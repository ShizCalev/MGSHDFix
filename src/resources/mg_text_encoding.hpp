#pragma once

struct MGTextCharacter
{
    wchar_t nUnicode;
    uint8_t nByte;
};

inline constexpr MGTextCharacter astMG1WesternCharacters[] =
{
    { 0x00A1, 0xDC },
    { 0x00BF, 0xDD },
    { 0x00C0, 0xB4 },
    { 0x00C1, 0xB5 },
    { 0x00C4, 0xB7 },
    { 0x00C9, 0xBD },
    { 0x00CC, 0xC4 },
    { 0x00CD, 0xC5 },
    { 0x00D3, 0xCD },
    { 0x00D6, 0xCF },
    { 0x00D9, 0xD4 },
    { 0x00DA, 0xD5 },
    { 0x00DC, 0xD7 },
    { 0x00DF, 0xDA },
    { 0x00E0, 0xB0 },
    { 0x00E1, 0xB1 },
    { 0x00E2, 0xB2 },
    { 0x00E4, 0xB3 },
    { 0x00E7, 0xD9 },
    { 0x00E8, 0xB8 },
    { 0x00E9, 0xB9 },
    { 0x00EA, 0xBA },
    { 0x00EC, 0xC0 },
    { 0x00ED, 0xC1 },
    { 0x00EE, 0xC2 },
    { 0x00F1, 0xD8 },
    { 0x00F2, 0xC8 },
    { 0x00F3, 0xC9 },
    { 0x00F4, 0xCA },
    { 0x00F6, 0xCB },
    { 0x00F9, 0xD0 },
    { 0x00FA, 0xD1 },
    { 0x00FB, 0xD2 },
    { 0x00FC, 0xD3 },
};

inline constexpr MGTextCharacter astMG2WesternCharacters[] =
{
    { 0x00A1, 0xDC },
    { 0x00B0, 0xDF },
    { 0x00BF, 0xDD },
    { 0x00C0, 0xB4 },
    { 0x00C1, 0xB5 },
    { 0x00C4, 0xB7 },
    { 0x00C7, 0xDB },
    { 0x00C8, 0xBC },
    { 0x00C9, 0xBD },
    { 0x00CC, 0xC4 },
    { 0x00CD, 0xC5 },
    { 0x00D3, 0xCD },
    { 0x00D6, 0xCF },
    { 0x00D9, 0xD4 },
    { 0x00DA, 0xD5 },
    { 0x00DC, 0xD7 },
    { 0x00DF, 0xDA },
    { 0x00E0, 0xB0 },
    { 0x00E1, 0xB1 },
    { 0x00E2, 0xB2 },
    { 0x00E4, 0xB3 },
    { 0x00E7, 0xD9 },
    { 0x00E8, 0xB8 },
    { 0x00E9, 0xB9 },
    { 0x00EA, 0xBA },
    { 0x00EB, 0xBB },
    { 0x00EC, 0xC0 },
    { 0x00ED, 0xC1 },
    { 0x00EE, 0xC2 },
    { 0x00EF, 0xC3 },
    { 0x00F1, 0xD8 },
    { 0x00F2, 0xC8 },
    { 0x00F3, 0xC9 },
    { 0x00F4, 0xCA },
    { 0x00F6, 0xCB },
    { 0x00F9, 0xD0 },
    { 0x00FA, 0xD1 },
    { 0x00FB, 0xD2 },
    { 0x00FC, 0xD3 },
    { 0x00FD, 0xAB },
    { 0x010D, 0xA7 },
    { 0x011B, 0xA8 },
    { 0x0159, 0xAA },
    { 0x016F, 0xA9 },
    { 0x017E, 0xAC },
    { 0xFF6D, 0xAD },
};

inline constexpr MGTextCharacter astMG2JapaneseCharacters[] =
{
    { 0x00E1, 0xB1 },
    { 0x010D, 0xB2 },
    { 0x011B, 0xB3 },
    { 0x00ED, 0xB4 },
    { 0x00FA, 0xB5 },
    { 0x016F, 0xB6 },
    { 0x0159, 0xB7 },
    { 0x00FD, 0xB8 },
    { 0x017E, 0xB9 },
    // old StringDB spellings for the same glyphs.
    { 0x00E2, 0xB2 },
    { 0x00E4, 0xB3 },
    { 0x00C0, 0xB4 },
    { 0x00C1, 0xB5 },
    { 0x00C2, 0xB6 },
    { 0x00C4, 0xB7 },
    { 0x00E8, 0xB8 },
    { 0x00E9, 0xB9 },
};


namespace MGTextExtended
{
    constexpr uint8_t nFirstLead = 0xF0;
    constexpr uint8_t nLastLead = 0xF9;
    constexpr size_t nTrailBytes = 188; // 0x40-0x7E and 0x80-0xFC
    inline std::vector<wchar_t> vCharacters;

    inline bool IsAsian(wchar_t nCharacter)
    {
        return (nCharacter >= 0x2E80 && nCharacter <= 0x9FFF) || (nCharacter >= 0xA960 && nCharacter <= 0xA97F) || (nCharacter >= 0xAC00 && nCharacter <= 0xD7FF)
            || (nCharacter >= 0xD800 && nCharacter <= 0xDFFF) || (nCharacter >= 0xF900 && nCharacter <= 0xFAFF) || (nCharacter >= 0xFF00 && nCharacter <= 0xFFEF);
    }

    // 0 when refused or full. CJK is only allowed when gm_lang == Japanese
    inline uint16_t Allocate(wchar_t nCharacter, bool bJapanese)
    {
        if (nCharacter < 0x80 || (nCharacter >= 0xD800 && nCharacter <= 0xDFFF) || (!bJapanese && IsAsian(nCharacter)))
        {
            return 0;
        }
        auto itCharacter = std::ranges::find(vCharacters, nCharacter);
        if (itCharacter == vCharacters.end())
        {
            if (vCharacters.size() >= (nLastLead - nFirstLead + 1) * nTrailBytes)
            {
                return 0;
            }
            vCharacters.push_back(nCharacter);
            itCharacter = vCharacters.end() - 1;
        }
        const size_t nIndex = static_cast<size_t>(itCharacter - vCharacters.begin());
        const size_t nTrail = nIndex % nTrailBytes;
        const uint8_t nTrailByte = static_cast<uint8_t>(nTrail < 63 ? 0x40 + nTrail : 0x41 + nTrail);
        return static_cast<uint16_t>(((nFirstLead + nIndex / nTrailBytes) << 8) | nTrailByte);
    }

    inline wchar_t Lookup(uint16_t nCode)
    {
        const uint8_t nLead = static_cast<uint8_t>(nCode >> 8);
        const uint8_t nTrailByte = static_cast<uint8_t>(nCode & 0xFF);
        if (nLead < nFirstLead || nLead > nLastLead || nTrailByte < 0x40 || nTrailByte == 0x7F || nTrailByte > 0xFC)
        {
            return 0;
        }
        const size_t nIndex = static_cast<size_t>(nLead - nFirstLead) * nTrailBytes + (nTrailByte < 0x7F ? nTrailByte - 0x40 : nTrailByte - 0x41);
        return nIndex < vCharacters.size() ? vCharacters[nIndex] : 0;
    }

    inline bool Contains(wchar_t nCharacter)
    {
        return std::ranges::find(vCharacters, nCharacter) != vCharacters.end();
    }
}
