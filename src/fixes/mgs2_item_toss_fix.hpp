#pragma once

// MGS2: dropped items and dog tags leave a held-up guard in a wider cone than on PS2, so a tag can
// hop a metre and fall off a ledge. The launch pitch reads a random number as signed; PS2 read it
// unsigned. This puts the PS2 cone back.
namespace MGS2ItemTossFix
{
    inline bool bEnabled = true;
    void Initialize();
}
