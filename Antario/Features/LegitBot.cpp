#include "legitbot.h"
#include "..\Utils\GlobalVars.h"
#include "..\Settings.h"
#include "..\SDK\CGlobalVarsBase.h"
#include "..\Utils\Math.h"
#include "..\SDK\Studio.h"
#include "..\SDK\IVModelInfoClient.h"
#include "..\SDK\ICvar.h"
#include "..\SDK\IEngineTrace.h"

Legitbot g_Legitbot;

bool C_BaseEntity::IsVisible( Vector origin ) //oops shouldnt be here
{
	if (g::pLocalEntity->IsAlive())
	{
		trace_t tr;
		Ray_t ray;
		CTraceFilter Filter;
		Filter.pSkip = g::pLocalEntity;

		ray.Init( g::pLocalEntity->GetEyePosition(), origin );

		g_pTrace->TraceRay( ray, MASK_VISIBLE, &Filter, &tr );

		return tr.IsVisible();
	}
	return false;
}

mstudiobbox_t* GetHitbox( C_BaseEntity* entity, int hitbox_index )
{
	if (entity->IsDormant() || entity->GetHealth() <= 0)
		return NULL;

	const auto pModel = entity->GetModel();
	if (!pModel)
		return NULL;

	auto pStudioHdr = g_MdlInfo->GetStudiomodel( pModel );
	if (!pStudioHdr)
		return NULL;

	auto pSet = pStudioHdr->pHitboxSet( 0 );
	if (!pSet)
		return NULL;

	if (hitbox_index >= pSet->numhitboxes || hitbox_index < 0)
		return NULL;

	return pSet->pHitbox( hitbox_index );
}

Vector Legitbot::HitBoxPosition( C_BaseEntity* entity, int hitbox_id )
{
	auto pHitbox = GetHitbox( entity, hitbox_id );

	if (!pHitbox)
		return Vector( 0, 0, 0 );

	auto pBoneMatrix = entity->GetBoneMatrix( pHitbox->bone );

	Vector bbmin, bbmax;
	Math::VectorTransform( pHitbox->bbmin, pBoneMatrix, bbmin );
	Math::VectorTransform( pHitbox->bbmax, pBoneMatrix, bbmax );

	return (bbmin + bbmax) * 0.5f;
}

template<typename T>
static T CubicInterpolate( T const& p1, T const& p2, T const& p3, T const& p4, float t )
{
	return p1 * (1 - t) * (1 - t) * (1 - t) +
		p2 * 3 * t * (1 - t) * (1 - t) +
		p3 * 3 * t * t * (1 - t) +
		p4 * t * t * t;
}

void Legitbot::OnCreateMove()
{
	static C_BaseEntity* preTarget = nullptr;
	static C_BaseEntity* curTarget = nullptr;
	static float t = 0.f;

	float bestFOV = 999.f;
	Vector viewAngles, engineAngles, angles;

	g_pEngine->GetViewAngles( engineAngles );
	Vector punchAngles = g::pLocalEntity->GetPunchAngles();
	static float recoil = g_pCVar->FindVar( "weapon_recoil_scale" )->GetFloat();

	if (GetAsyncKeyState( VK_LBUTTON ))
	{
		for (int it = 1; it <= g_pEngine->GetMaxClients(); ++it)
		{
			C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity( it );

			if (!pPlayerEntity || pPlayerEntity->IsDormant() || !pPlayerEntity->IsAlive() || pPlayerEntity == g::pLocalEntity || pPlayerEntity->GetTeam() == g::pLocalEntity->GetTeam())
				continue;

			if (!pPlayerEntity->IsVisible( HitBoxPosition( pPlayerEntity, HITBOX_HEAD ) ))
				continue;

			angles = Math::ClampAngle( Math::NormalizeAngle( Math::CalcAngle( g::pLocalEntity->GetEyePosition(), HitBoxPosition( pPlayerEntity, HITBOX_HEAD ) ) ) );

			if (g::pCmd->buttons & IN_ATTACK) 
				angles -= punchAngles * recoil; //we want to see that recoil so we feel legit baby

			float fov = (engineAngles - angles).Length2D();

			if (fov < bestFOV)
			{
				bestFOV = fov;
				viewAngles = angles;
				curTarget = pPlayerEntity;
			}
		}

		if (preTarget != curTarget || preTarget == nullptr)
		{
			t = 0.0f;
			preTarget = curTarget;
		} 
		else if (preTarget == curTarget)
		{
			t += 0.03f;
		}

		if (bestFOV != 999.f)
		{
			if (t < 1.f && bestFOV > 1.f) //not really necessary to add a curve if we are this close
			{
				Vector src = engineAngles;
				Vector dst = viewAngles;

				Vector delta = src - dst;
				Math::ClampAngle( delta );

				float randValPt1 = 15.0f + Math::RandFloat( 0.0f, 15.0f );
				float finalRandValPt1 = 3.f / randValPt1;
				Vector point1 = src + (delta * finalRandValPt1);
				Math::ClampAngle( point1 );

				float randValPt2 = 40.0f + Math::RandFloat( 0.0f, 15.0f );
				float finalRandValPt2 = 1.0f / randValPt2;
				Vector point2 = dst * (1.0f + finalRandValPt2);
				Math::ClampAngle( point2 );

				Vector angle = CubicInterpolate( src, point1, point2, dst, t );
				Math::ClampAngle( angle );

				g::pCmd->viewangles = angle;
				g_pEngine->SetViewAngles( angle );
			}
			else
			{
				Vector smoothAngle = Math::NormalizeAngle( viewAngles - engineAngles );
				viewAngles = engineAngles + smoothAngle / 4.f; //smooth amount
				Math::ClampAngle( viewAngles );
				g::pCmd->viewangles = viewAngles;
				g_pEngine->SetViewAngles( viewAngles );
			}
		}
	}
	else
	{
		t = 0.f;
	}

	g::pCurrentFOV = t;
}