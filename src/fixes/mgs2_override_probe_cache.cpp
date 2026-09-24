#include "stdafx.h"
#include "mgs2_override_probe_cache.hpp"

#include "common.hpp"
#include "logging.hpp"
#include "expand_bp_assets.hpp"

#include <atomic>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

// Every asset a stage loads is first looked for in up to five override folders. Nearly all miss, and under Proton
// each miss makes Wine scan the whole folder, seconds per load. Read each folder once and answer from that.

namespace
{
    SafetyHookInline gProbeHook {};
    SafetyHookInline gExistsHook {};

    thread_local bool tInProbe = false;
    thread_local bool tBackground = false;
    std::atomic<ULONGLONG> gLastGameProbe { 0 };

    std::mutex gLock;
    std::unordered_map<std::string, std::unordered_set<std::string>> gFolders;

    std::string Lower(std::string_view s)
    {
        std::string out(s);
        for (char& c : out)
        {
            c = (c == '\\') ? '/' : static_cast<char>(tolower(static_cast<unsigned char>(c)));
        }
        return out;
    }

    bool InFolder(const std::string& dir, const std::string& name)
    {
        {
            const std::lock_guard<std::mutex> guard(gLock);
            const auto it = gFolders.find(dir);
            if (it != gFolders.end())
            {
                return it->second.contains(name);
            }
        }

        // list outside the lock, so the game never waits on the prefetch thread's scan
        std::unordered_set<std::string> names;
        const auto list = [&](const std::string& folder)
        {
            WIN32_FIND_DATAA fd {};
            const HANDLE find = FindFirstFileExA((folder.empty() ? std::string("*") : folder + "/*").c_str(), FindExInfoBasic, &fd,
                FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
            if (find == INVALID_HANDLE_VALUE)
            {
                return;
            }
            do
            {
                names.insert(Lower(fd.cFileName));
            } while (FindNextFileA(find, &fd));
            FindClose(find);
        };
        list(dir);
        // the ASI loader's overload folder counts too: its files open as if they were here
        if (const auto& overlay = BP_FileSys::LoaderOverloadRoot(); !overlay.empty())
        {
            list((overlay / dir).string());
        }
        const bool found = names.contains(name);
        const std::lock_guard<std::mutex> guard(gLock);
        gFolders.try_emplace(dir, std::move(names));
        return found;
    }

    // BOOL FileExists( const char *path )
    int __fastcall Exists_hooked(const char* path)
    {
        if (!tInProbe || path == nullptr)
        {
            return gExistsHook.fastcall<int>(path);
        }

        const std::string full = Lower(path);
        const size_t slash = full.rfind('/');
        const std::string dir = slash == std::string::npos ? "" : full.substr(0, slash);
        const std::string name = slash == std::string::npos ? full : full.substr(slash + 1);

        return InFolder(dir, name) ? 1 : 0;
    }

    // void ProbeOverride( char *path, void *unused )
    void __fastcall Probe_hooked(char* path, void* arg)
    {
        if (!tBackground)
        {
            gLastGameProbe = GetTickCount64();
        }
        tInProbe = true;
        gProbeHook.fastcall<void>(path, arg);
        tInProbe = false;
    }
}

void MGS2_OverrideProbeCache::HandleLevelTransition()
{
    // override files dropped in while the game runs show up on the next load
    const std::lock_guard<std::mutex> guard(gLock);
    gFolders.clear();
}

void MGS2_OverrideProbeCache::Resolve(char* path)
{
    // uncached, the probe is a slow file check per candidate: leave the default path
    if (gProbeHook)
    {
        tBackground = true;
        Probe_hooked(path, nullptr);
        tBackground = false;
    }
}

ULONGLONG MGS2_OverrideProbeCache::LastGameProbe()
{
    return gLastGameProbe.load();
}

void MGS2_OverrideProbeCache::Setup()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    uint8_t* probe = Memory::PatternScan(baseModule, "48 89 5C 24 ?? 55 41 56 41 57 48 81 EC",
        "MGS2: Override Probe Cache: BP override file probe");
    uint8_t* exists = Memory::PatternScan(baseModule,
        "48 83 EC ?? 0F 57 C0 4C 8D 44 24 ?? 33 C0 33 D2 0F 11 44 24 ?? 89 44 24 ?? 0F 11 44 24 ?? FF 15 ?? ?? ?? ?? 33 C9",
        "MGS2: Override Probe Cache: BP file exists");
    if (!probe || !exists)
    {
        return;
    }

    gExistsHook = safetyhook::create_inline(exists, reinterpret_cast<void*>(Exists_hooked));
    LOG_HOOK(gExistsHook, "MGS2: Override Probe Cache: BP file exists");
    gProbeHook = safetyhook::create_inline(probe, reinterpret_cast<void*>(Probe_hooked));
    LOG_HOOK(gProbeHook, "MGS2: Override Probe Cache: BP override file probe");
}
