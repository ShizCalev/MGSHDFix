#pragma once

// Ending cast roll (script 142, sheet 16)
namespace MG2_EndingTiming
{
    inline bool bEnabled = false;

    constexpr int nVideoFramesPerTick = 4;

    constexpr size_t nCueSizeOffset = 0x70;
    constexpr int nVanillaCueTick = 1715;

    constexpr int nVanillaLastTick = 1793;

    struct Anchor
    {
        int nTick;
        int nFrame;
    };

    inline constexpr Anchor astAnchors[] =
    {
        { 0, 0 },          // song start, CAST title card
        { 56, 220 },       // title card off
        { 75, 299 },       { 194, 737 },   // Roy
        { 198, 751 },      { 313, 1192 },  // Holly
        { 317, 1205 },     { 438, 1659 },  // George
        { 443, 1673 },     { 571, 2162 },  // Drago
        { 576, 2174 },     { 691, 2622 },  // Yosef
        { 695, 2635 },     { 816, 3090 },  // Miller
        { 820, 3104 },     { 935, 3519 },  // Kio
        { 938, 3533 },     { 1077, 4036 }, // Fox
        { 1081, 4052 },    { 1211, 4525 }, // Natasha
        { 1215, 4536 },    { 1328, 4972 }, // Big Boss
        { 1329, 4975 },    { 1434, 5383 }, // Hind D
        { 1436, 5387 },    { 1550, 5832 }, // Metal Gear D
        { 1608, 6060 },    // Snake
        { 1715, 6464 },    // music fade
        { 1725, 6467 },    // white flash
        { 1726, 6468 },    // Snake off
        { 1793, 6762 },    // title screen
    };

    [[nodiscard]] constexpr int RetimeTick(int nTick)
    {
        const Anchor* pPrevious = &astAnchors[0];
        int nFrame = pPrevious->nFrame + (nTick - pPrevious->nTick) * nVideoFramesPerTick;
        for (const Anchor& stAnchor : astAnchors)
        {
            if (nTick <= stAnchor.nTick)
            {
                const int nSpan = stAnchor.nTick - pPrevious->nTick;
                nFrame = nSpan == 0 ? stAnchor.nFrame : pPrevious->nFrame + ((nTick - pPrevious->nTick) * (stAnchor.nFrame - pPrevious->nFrame) + nSpan / 2) / nSpan;
                return (nFrame + nVideoFramesPerTick / 2) / nVideoFramesPerTick;
            }
            pPrevious = &stAnchor;
        }
        nFrame = pPrevious->nFrame + (nTick - pPrevious->nTick) * nVideoFramesPerTick;
        return (nFrame + nVideoFramesPerTick / 2) / nVideoFramesPerTick;
    }

    constexpr int nRestoredCueTick = RetimeTick(nVanillaCueTick);
    constexpr int nRestoredLastTick = RetimeTick(nVanillaLastTick);
    static_assert(nRestoredLastTick != nVanillaLastTick, "same timings");
}
