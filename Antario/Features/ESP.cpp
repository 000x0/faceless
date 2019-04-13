#include "ESP.h"
#include "..\Settings.h"
#include "..\Utils\Utils.h"
#include "..\SDK\IVEngineClient.h"
#include "..\SDK\PlayerInfo.h"
#include "..\SDK\CGlobalVarsBase.h"

ESP g_ESP;

void ESP::PlayerName( C_BaseEntity* entity, int iterator )
{
	Vector vecScreenOrigin, vecOrigin = entity->GetRenderOrigin();

	if (!Utils::WorldToScreen( vecOrigin, vecScreenOrigin ))
		return;

	Vector vecScreenBottom, vecBottom = vecOrigin;
	vecBottom.z += (entity->GetFlags() & FL_DUCKING) ? 54.f : 72.f;
	if (!Utils::WorldToScreen( vecBottom, vecScreenBottom ))
		return;

	PlayerInfo_t pInfo;
	g_pEngine->GetPlayerInfo( iterator, &pInfo );
	
	/* convert char* to string */
	std::string str( pInfo.szName );
	/* All lowercase because lower case gang */
	std::transform( str.begin(), str.end(), str.begin(), ::tolower );

	const auto sx = int( std::roundf( vecScreenOrigin.x ) ), sy = int( std::roundf( vecScreenOrigin.y ) ), h = int( std::roundf( vecScreenBottom.y - vecScreenOrigin.y ) );

	g_Render.String( sx, sy + h - 16, FONT_CENTERED_X | FONT_DROPSHADOW, Color( 255, 255, 255, 255 ), g_Fonts.vecFonts[FONT_TAHOMA_8], str.c_str() );
}

void ESP::RenderWeaponName( C_BaseEntity* entity )
{
	Vector vecScreenOrigin, vecOrigin = entity->GetRenderOrigin();
	if (!Utils::WorldToScreen( vecOrigin, vecScreenOrigin ))
		return;

	auto weapon = entity->GetActiveWeapon();

	if (!weapon)
		return;

	auto strWeaponName = weapon->GetName();
	int ammo = weapon->GetAmmo();
	int clip = weapon->GetCSWpnData()->iMaxClip1;

	/* Remove "weapon_" prefix */
	strWeaponName.erase( 0, 7 );
	/* All lowercase */
	std::transform( strWeaponName.begin(), strWeaponName.end(), strWeaponName.begin(), ::tolower );
	/* Final string */
	std::string str = strWeaponName + "[" + std::to_string( ammo ) + "/" + std::to_string( clip ) + "]";

	g_Render.String( int( vecScreenOrigin.x ), int( vecScreenOrigin.y ) + 5, FONT_CENTERED_X | FONT_DROPSHADOW, Color( 255, 255, 255, 255 ), g_Fonts.vecFonts[FONT_TAHOMA_8], str.c_str() );
}

void ESP::RenderHealth( C_BaseEntity* entity )
{
	Vector vecScreenOrigin, vecOrigin = entity->GetRenderOrigin();
	if (!Utils::WorldToScreen( vecOrigin, vecScreenOrigin ))
		return;

	Vector vecScreenBottom, vecBottom = vecOrigin;
	vecBottom.z += (entity->GetFlags() & FL_DUCKING) ? 54.f : 72.f;
	if (!Utils::WorldToScreen( vecBottom, vecScreenBottom ))
		return;

	int enemyHp = entity->GetHealth();

	std::string str = "HP[" + std::to_string( enemyHp ) + "]";

	const auto sx = int( std::roundf( vecScreenOrigin.x ) ), sy = int( std::roundf( vecScreenOrigin.y ) ), h = int( std::roundf( vecScreenBottom.y - vecScreenOrigin.y ) ), w = int( std::roundf( h * 0.25f ) );

	g_Render.String( sx - w + 5, sy + h, FONT_DROPSHADOW, Color( 255, 255, 255, 255 ), g_Fonts.vecFonts[FONT_TAHOMA_8], str.c_str() );

}

void ESP::Radar( C_BaseEntity* entity ) //worth its own func
{ 
	entity->Spotted( true );
}

void ESP::Crosshair()
{
	static int center_x = g_Render.GetScreenCenter().x;
	static int center_y = g_Render.GetScreenCenter().y;

	g_Render.Line( center_x - 5, center_y, center_x + 5, center_y, Color( 255, 0, 0, 255 ) );
	g_Render.Line( center_x, center_y - 5, center_x, center_y + 5, Color( 255, 0, 0, 255 ) );
}

void ESP::Inaccuracy()
{
	auto weapon = g::pLocalEntity->GetActiveWeapon();

	if (!weapon)
		return;

	static int center_x = g_Render.GetScreenCenter().x;
	static int center_y = g_Render.GetScreenCenter().y;

	const float spread_distance = (weapon->GetInaccuracy() + weapon->GetSpread()) * 320.f / std::tan( ( 110/*size of circle*/ * ( M_PI / 180.f ) ) * 0.5f );
	const float spread_radius = center_y * 4 * 0.002083 * spread_distance;

	Math::Circle( center_x, center_y, spread_radius, 10, Color( 255, 255, 255, 25 ) );
}

void ESP::ScopeLines()
{
	auto weapon = g::pLocalEntity->GetActiveWeapon();

	if (weapon)
	{
		float spread = weapon->GetInaccuracy() * 50;
		int height = std::clamp( spread, 1.f, 7.5f );
		int alpha = 255 - (height * 15.f);

		static int center_x = g_Render.GetScreenCenter().x;
		static int center_y = g_Render.GetScreenCenter().y;

		g_Render.RectFilled( 0, center_y - height, center_x * 2, center_y + height, Color( 0, 0, 0, alpha ) );
		g_Render.RectFilled( center_x - height, 0, center_x + height, center_y * 2, Color( 0, 0, 0, alpha ) );
	}

}

void ESP::Hitmarker()
{
	static int center_x = g_Render.GetScreenCenter().x;
	static int center_y = g_Render.GetScreenCenter().y;

	if (g::pHitmarkerAlpha > 0.f) 
	{
		float alpha = g::pHitmarkerAlpha * 255;
		g_Render.Line( center_x - 8, center_y - 8, center_x - 3, center_y - 3, Color( 255, 255, 255, alpha ) );
		g_Render.Line( center_x - 8, center_y + 8, center_x - 3, center_y + 3, Color( 255, 255, 255, alpha ) );
		g_Render.Line( center_x + 8, center_y - 8, center_x + 3, center_y - 3, Color( 255, 255, 255, alpha ) );
		g_Render.Line( center_x + 8, center_y + 8, center_x + 3, center_y + 3, Color( 255, 255, 255, alpha ) );

		g::pHitmarkerAlpha -= 1.f / 0.5f * g_pGlobalVars->frametime;
	}
}

void ESP::NightMode()
{
	for (int i = 1; i < g_pEntityList->GetMaxEntities(); ++i)
	{
		auto entity = g_pEntityList->GetClientEntity( i );
		if (entity)
		{
			if (entity->GetClientClass()->ClassID == EClassIds::CEnvTonemapController)
			{
				entity->set_m_bUseCustomAutoExposureMin( 1 );
				entity->set_m_bUseCustomAutoExposureMax( 1 );
				entity->set_m_flCustomAutoExposureMin( 0.2 );
				entity->set_m_flCustomAutoExposureMax( 0.2 );
			}
		}
	}
}

void ESP::Render()
{
	if (g::pLocalEntity && g_pEngine->IsInGame())
	{
		this->Crosshair();
		this->Hitmarker();
		this->NightMode();

		if (g::pLocalEntity->IsAlive())
			this->Inaccuracy();

		if (g::pLocalEntity->Scoped())
			this->ScopeLines();

		//g_Render.String( g_Render.GetScreenCenter().x, g_Render.GetScreenCenter().y - 20, FONT_DROPSHADOW, Color( 255, 255, 255, 255 ), g_Fonts.vecFonts[FONT_TAHOMA_8], (std::string( std::to_string( g::pCurrentFOV ) ).c_str() ) );

		for (int i = 1; i <= g_pEngine->GetMaxClients(); ++i)
		{
			auto entity = g_pEntityList->GetClientEntity( i );

			if (entity && entity->IsAlive() && !entity->IsDormant() && entity->GetTeam() != g::pLocalEntity->GetTeam())
			{
				this->PlayerName( entity, i );
				this->RenderWeaponName( entity );
				this->RenderHealth( entity );
				this->Radar( entity );
			}
		}
	}
}
