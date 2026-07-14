#include "antifreeze.h"
#include <game/client/gameclient.h>
#include <base/vmath.h>
#include <engine/input.h>

CAntiFreeze::CAntiFreeze() = default;

void CAntiFreeze::OnConsoleInit()
{
    Console()->Register("af_enable", "i", CFGFLAG_CLIENT, "Enable anti-freeze", 0, this);
    Console()->Register("af_minticks", "i", CFGFLAG_CLIENT, "Min prediction ticks", 0, this);
    Console()->Register("af_maxticks", "i", CFGFLAG_CLIENT, "Max prediction ticks", 0, this);
    Console()->Register("af_hotkey", "i", CFGFLAG_CLIENT, "Hotkey for manual release", 0, this);
}

void CAntiFreeze::OnReset()
{
    m_WasReleased = false;
    m_ToggleKeyPressed = false;
}

void CAntiFreeze::OnRelease()
{
    m_WasReleased = false;
}

int CAntiFreeze::GetAdaptiveTicks(float speed)
{
    if (speed <= 5.0f) return m_MaxTicks;
    if (speed >= 15.0f) return m_MinTicks;
    float t = (speed - 5.0f) / 10.0f;
    return m_MaxTicks - (int)(t * (m_MaxTicks - m_MinTicks));
}

void CAntiFreeze::OnRender()
{
    if (!GameClient()->m_Snap.m_pLocalCharacter) return;
    if (GameClient()->m_Snap.m_SpecInfo.m_Active) return;

    // ===== БИНД НА V (вкл/выкл) =====
    if (Input()->KeyIsPressed(KEY_V) && !m_ToggleKeyPressed)
    {
        m_Enabled = !m_Enabled;
        m_ToggleKeyPressed = true;
    }
    if (!Input()->KeyIsPressed(KEY_V))
    {
        m_ToggleKeyPressed = false;
    }

    if (!m_Enabled) return;

    // ===== ХОТКЕЙ (ручной отпуск) =====
    if (Input()->KeyIsPressed(m_Hotkey))
    {
        ReleaseHook();
        m_WasReleased = true;
        return;
    }

    const CCharacterCore *pCore = &GameClient()->m_PredictedChar;
    if (!pCore) return;

    if (pCore->m_HookState != HOOK_FLYING)
    {
        m_WasReleased = false;
        return;
    }

    if (m_IgnoreHookedPlayer && pCore->HookedPlayer() != -1)
        return;

    if (m_IgnoreOnGround && pCore->IsGrounded())
        return;

    float speed;
    if (m_IgnoreVertical)
        speed = fabs(pCore->m_Vel.x);
    else
        speed = pCore->m_Vel.len();

    int ticks = GetAdaptiveTicks(speed);
    float hookLen = distance(pCore->m_Pos, pCore->m_HookPos);
    vec2 futurePos = pCore->m_Pos + pCore->m_Vel * (float)ticks;
    float futureHookLen = distance(futurePos, pCore->m_HookPos);

    float limit = 380.0f;

    if ((hookLen > limit * 0.95f || futureHookLen > limit) && !m_WasReleased)
    {
        ReleaseHook();
        m_WasReleased = true;
    }
}

void CAntiFreeze::ReleaseHook()
{
    GameClient()->m_PredictedChar.m_HookState = HOOK_RETRACTED;
    GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Hook = 0;
}
