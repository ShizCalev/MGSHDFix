#pragma once

class MuteWarning final
{
private:
    int* muteWarningAddress = nullptr;

public:
    void Setup();
    void CheckStatus() const;
    bool bEnabled = false;

};

inline MuteWarning g_MuteWarning;


