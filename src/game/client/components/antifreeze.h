#pragma once
#include <game/client/component.h>

class CAntiFreeze : public CComponent
{
public:
    CAntiFreeze();
    void OnRender() override;
    void OnConsoleInit() override;
    void OnReset() override;
    void OnRelease() override;

    // НАСТРОЙКИ
    bool m_Enabled = true;
    int m_MinTicks = 1;
    int m_MaxTicks = 4;
    int m_Hotkey = KEY_X;
    bool m_IgnoreHookedPlayer = true;
    bool m_IgnoreOnGround = false;
    bool m_IgnoreVertical = true;

    // СОСТОЯНИЯ
    bool m_WasReleased = false;
    bool m_ToggleKeyPressed = false;   // для бинда на V

private:
    void ReleaseHook();
    int GetAdaptiveTicks(float speed);
};
