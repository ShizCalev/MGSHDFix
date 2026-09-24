#include "stdafx.h"
#include "mgs2_area_prefetch.hpp"

#include "common.hpp"
#include "gamevars.hpp"
#include "logging.hpp"
#include "mgs2_override_probe_cache.hpp"
#include "expand_bp_assets.hpp"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_set>

// A first visit through a door mostly waits on cold files. The stage script names every area its doors lead to,
// so once an area is up, read those areas' files in the background, picked the way the loader picks them.

namespace
{
    constexpr auto kPoll   = std::chrono::milliseconds(100);
    constexpr auto kSettle = std::chrono::milliseconds(500);
    constexpr ULONGLONG kQuietMs = 1500;
    constexpr int kRadioOrStream = 0x00300400;   // MENU_RADIO_ON | MENU_STREAM_CH_0 | MENU_STREAM_CH_1
    ULONGLONG gLastBusy = 0;

    std::mutex gLock;
    std::condition_variable gWake;
    std::atomic<uint32_t> gGeneration { 0 };
    std::string gArea;
    std::unordered_set<std::string> gWarmed;

    bool Stale(uint32_t gen) { return gGeneration.load() != gen; }

    // the game goes first: its own loading, a codec call, any voice stream (a disk stall there hitches the frame
    // loop, and the sound driver ticks from it)
    void WaitForQuiet(uint32_t gen)
    {
        while (!Stale(gen))
        {
            const ULONGLONG now = GetTickCount64();
            if (g_GameVars.GM_MenuStatus() & kRadioOrStream) { gLastBusy = now; }
            const ULONGLONG busy = std::max(gLastBusy, MGS2_OverrideProbeCache::LastGameProbe());
            if (now - busy >= kQuietMs) { return; }
            std::this_thread::sleep_for(kPoll);
        }
    }

    // this region first; JP plays the US scenario
    std::vector<std::string> Regions(bool scripts)
    {
        const std::string region = g_GameVars.MGS2_RegionDir();
        std::vector<std::string> out { scripts && region == "jp" ? "us" : region };
        for (const char* r : { "eu", "us", "jp" })
        {
            if (std::find(out.begin(), out.end(), r) == out.end()) { out.push_back(r); }
        }
        return out;
    }

    std::filesystem::path FirstInRegion(bool scripts, const std::function<std::filesystem::path(const std::string&)>& path)
    {
        std::error_code ec;
        for (const auto& r : Regions(scripts))
        {
            const auto p = path(r);
            if (std::filesystem::is_regular_file(p, ec)) { return p; }
        }
        return {};
    }

    // load "w01b" = the load command's strcode, then a string operand
    std::vector<std::string> DoorTargets(const std::string& area)
    {
        const auto gcx = FirstInRegion(true, [&](const std::string& r) { return sExePath / "assets/gcx" / r / "_bp" / ("scenerio_stage_" + area + ".gcx"); });
        std::vector<std::string> out;
        if (gcx.empty()) { return out; }

        std::ifstream f(gcx, std::ios::binary);
        const std::vector<uint8_t> b((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        for (size_t i = 0; i + 6 < b.size(); ++i)
        {
            if (b[i] != 0x84 || b[i + 1] != 0xC8 || b[i + 2] != 0x37 || b[i + 3] != 0x07 || b[i + 4] != 0x07) { continue; }
            const size_t len = b[i + 5];
            if (len < 2 || i + 6 + len > b.size() || b[i + 5 + len] != 0) { continue; }
            const std::string target(reinterpret_cast<const char*>(&b[i + 6]), len - 1);
            if (target != area && std::find(out.begin(), out.end(), target) == out.end()) { out.push_back(target); }
        }
        return out;
    }

    // textures go to the probe in their _win form, like the loader passes them
    std::vector<std::filesystem::path> AreaFiles(const std::string& area)
    {
        std::vector<std::filesystem::path> out;
        for (const char* list : { "manifest.txt", "bp_assets.txt" })
        {
            const auto p = FirstInRegion(false, [&](const std::string& r) { return sExePath / r / "stage" / area / list; });
            if (p.empty()) { continue; }
            const bool textures = list[0] == 'b';
            std::ifstream f(p);
            for (std::string line; std::getline(f, line);)
            {
                std::string rel = line.substr(0, line.find(','));
                if (rel.empty() || rel.size() > 200) { continue; }
                if (textures)
                {
                    const size_t slash = rel.rfind('/');
                    rel.insert(slash == std::string::npos ? 0 : slash + 1, "_win/");
                }
                char path[512] {};
                memcpy(path, rel.c_str(), rel.size());
                MGS2_OverrideProbeCache::Resolve(path);
                std::error_code ec;
                const auto& overlay = BP_FileSys::LoaderOverloadRoot();
                if (!overlay.empty() && std::filesystem::is_regular_file(overlay / path, ec)) { out.push_back(overlay / path); }
                else if (std::filesystem::is_regular_file(sExePath / path, ec)) { out.push_back(sExePath / path); }
            }
        }
        return out;
    }

    uint64_t Warm(const std::filesystem::path& file, uint32_t gen)
    {
        static std::vector<char> buffer(1 << 20);
        const HANDLE h = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (h == INVALID_HANDLE_VALUE) { return 0; }
        uint64_t total = 0;
        DWORD got = 0;
        while (!Stale(gen) && ReadFile(h, buffer.data(), static_cast<DWORD>(buffer.size()), &got, nullptr) && got)
        {
            total += got;
            WaitForQuiet(gen);
        }
        CloseHandle(h);
        return total;
    }

    void Worker()
    {
        SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
        uint32_t seen = 0;
        while (true)
        {
            std::string area;
            {
                std::unique_lock<std::mutex> lock(gLock);
                gWake.wait(lock, [&] { return gGeneration.load() != seen; });
                seen = gGeneration.load();
                area = gArea;
            }
            const uint32_t gen = seen;
            while (!Stale(gen) && g_GameVars.DG_UnDrawFrameCount() != 0) { std::this_thread::sleep_for(kPoll); }
            std::this_thread::sleep_for(kSettle);
            if (Stale(gen) || area.empty()) { continue; }

            const auto started = std::chrono::steady_clock::now();
            const auto targets = DoorTargets(area);
            size_t files = 0;
            uint64_t bytes = 0;
            for (const auto& target : targets)
            {
                if (Stale(gen)) { break; }
                for (const auto& file : AreaFiles(target))
                {
                    WaitForQuiet(gen);
                    if (Stale(gen)) { break; }
                    const std::string key = file.string();
                    if (gWarmed.contains(key)) { continue; }
                    bytes += Warm(file, gen);
                    if (Stale(gen)) { break; }
                    gWarmed.insert(key);
                    files++;
                }
            }
            const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started).count();
            spdlog::info("MGS2: Area Prefetch: {} ({}) -> {} doors, {} files, {:.1f} MB in {} ms{}", area, g_GameVars.MGS2_RegionDir(), targets.size(), files,
                static_cast<double>(bytes) / (1024.0 * 1024.0), ms, Stale(gen) ? " (cut short by the next load)" : "");
        }
    }
}

void MGS2_AreaPrefetch::HandleLevelTransition()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }
    {
        const std::lock_guard<std::mutex> lock(gLock);
        gArea = g_GameVars.GetCurrentStage();
        gGeneration++;
    }
    gWake.notify_one();
}

void MGS2_AreaPrefetch::Setup()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }
    std::thread(Worker).detach();
    spdlog::info("MGS2: Area Prefetch: worker started");
}
