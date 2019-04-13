#pragma once
#include <vector>
#include "SDK\IGameEvent.h"
#include "SDK\CEntity.h"
#include "Utils\GlobalVars.h"

class EventListener : public IGameEventListener2
{
public:
    EventListener(std::vector<const char*> events)
    {
        for (auto& it : events)
            g_pEventManager->AddListener(this, it, false);
    }

    ~EventListener()
    {
        g_pEventManager->RemoveListener(this);
    }

    void FireGameEvent(IGameEvent* event) override
    {
        // call functions here
		auto target = g_pEngine->GetPlayerForUserID( event->GetInt( "userid" ) );
		auto attacker = g_pEntityList->GetClientEntity( g_pEngine->GetPlayerForUserID( event->GetInt( "attacker" ) ) );
		if (target && attacker && attacker == g::pLocalEntity)
		{
			if (!strcmp( event->GetName(), "player_hurt" )) {
				g_pSurface->IPlaySound( "buttons\\arena_switch_press_02.wav" );
			}
			g::pHitmarkerAlpha = 1.f;
		}
    }

    int GetEventDebugID() override
    {
        return EVENT_DEBUG_ID_INIT;
    }
};