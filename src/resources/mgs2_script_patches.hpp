#pragma once

// MGS2: small byte patches to a stage's script, applied as the game loads it. Features add theirs
// during init; one hook on GCL_LoadScript serves them all.
namespace MGS2_ScriptPatches
{
    // In `stage`, find these bytes and write `put` over them starting `at` bytes in.
    void Add(const char* stage, std::vector<uint8_t> find, size_t at, std::vector<uint8_t> put);
    void Initialize();   // after every feature that adds patches
}
