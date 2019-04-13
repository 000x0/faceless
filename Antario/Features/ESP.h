#pragma once
#include "..\Utils\GlobalVars.h"
#include "..\Utils\DrawManager.h"
#include "..\SDK\ClientClass.h"
#include "..\Utils\Math.h"

class ESP
{
public:
    void Render();
private:
	void PlayerName( C_BaseEntity* entity, int iterator );
	void RenderWeaponName( C_BaseEntity* entity );
	void RenderHealth( C_BaseEntity* entity );
	void Radar( C_BaseEntity* entity );

	void Crosshair();
	void Inaccuracy();
	void Hitmarker();
	void NightMode();
	void ScopeLines();
};
extern ESP g_ESP;