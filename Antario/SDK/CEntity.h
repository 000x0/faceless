#pragma once
#include "Definitions.h"
#include "IClientUnknown.h"
#include "IClientEntityList.h"
#include "..\Utils\Utils.h"
#include "..\Utils\NetvarManager.h"
#include "..\SDK\Studio.h"
#include "IEngineTrace.h"

extern WeaponInfo_t g_WeaponInfoCopy[255];

template <typename t>
static t GetVFunc( void* class_pointer, size_t index ) {
	return (*(t**)class_pointer)[index];
}

// class predefinition
class C_BaseCombatWeapon;

class C_BaseEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable
{
private:
    template<class T>
    T GetPointer(const int offset)
    {
        return reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }
    // To get value from the pointer itself
    template<class T>
    T GetValue(const int offset)
    {
        return *reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }

public:
    C_BaseCombatWeapon* GetActiveWeapon()
    {
        static int m_hActiveWeapon = g_pNetvars->GetOffset("DT_BaseCombatCharacter", "m_hActiveWeapon");
        const auto weaponData      = GetValue<CBaseHandle>(m_hActiveWeapon);
        return reinterpret_cast<C_BaseCombatWeapon*>(g_pEntityList->GetClientEntityFromHandle(weaponData));
    }

    int GetTeam()
    {
        static int m_iTeamNum = g_pNetvars->GetOffset("DT_BaseEntity", "m_iTeamNum");
        return GetValue<int>(m_iTeamNum);
    }

    EntityFlags GetFlags()
    {
        static int m_fFlags = g_pNetvars->GetOffset("DT_BasePlayer", "m_fFlags");
        return GetValue<EntityFlags>(m_fFlags);
    }

    MoveType_t GetMoveType()
    {
        static int m_Movetype = g_pNetvars->GetOffset("DT_BaseEntity", "m_nRenderMode") + 1;
        return GetValue<MoveType_t>(m_Movetype);
    }

    bool GetLifeState()
    {
        static int m_lifeState = g_pNetvars->GetOffset("DT_BasePlayer", "m_lifeState");
        return GetValue<bool>(m_lifeState);
    }

    int GetHealth()
    {
        static int m_iHealth = g_pNetvars->GetOffset("DT_BasePlayer", "m_iHealth");
        return GetValue<int>(m_iHealth);
    }

    bool IsAlive() { return this->GetHealth() > 0 && this->GetLifeState() == 0; }

    bool IsImmune()
    {
        static int m_bGunGameImmunity = g_pNetvars->GetOffset("DT_CSPlayer", "m_bGunGameImmunity");
        return GetValue<bool>(m_bGunGameImmunity);
    }

    int GetTickBase()
    {
        static int m_nTickBase = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_nTickBase");
        return GetValue<int>(m_nTickBase);
    }

    Vector GetOrigin()
    {
        static int m_vecOrigin = g_pNetvars->GetOffset("DT_BaseEntity", "m_vecOrigin");
        return GetValue<Vector>(m_vecOrigin);
    }

    Vector GetViewOffset()
    {
        static int m_vecViewOffset = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_vecViewOffset[0]");
        return GetValue<Vector>(m_vecViewOffset);
    }

    Vector GetEyePosition() { return this->GetOrigin() + this->GetViewOffset(); }
	Vector GetPunchAngles() { return *reinterpret_cast<Vector*>(uintptr_t( this ) + 0x302C); }

	Vector GetBonePosition( int i ) {
		matrix3x4_t boneMatrix[128];
		if (this->SetupBones( boneMatrix, 128, 0x00000100, static_cast<float>(GetTickCount64()) ))
			return Vector( boneMatrix[i][0][3], boneMatrix[i][1][3], boneMatrix[i][2][3] );

		return Vector( 0, 0, 0 );
	}

	matrix3x4_t GetBoneMatrix( int BoneID )
	{
		matrix3x4_t matrix;

		auto offset = *reinterpret_cast<uintptr_t*>(uintptr_t( this ) + 0x26A8);
		if (offset)
			matrix = *reinterpret_cast<matrix3x4_t*>(offset + 0x30 * BoneID);

		return matrix;
	}

	float GetSimTime() {
		static int m_flSimulationtime = g_pNetvars->GetOffset( "DT_BaseEntity", "m_flSimulationTime" );
		return GetValue<float>( m_flSimulationtime );
	}

	float GetOldSimTime() {
		static int m_flSimulationTime = 0x264;
		return *(float*)((DWORD)this + (m_flSimulationTime + 0x4));
	}

	bool Scoped()
	{
		static int m_bIsScoped = g_pNetvars->GetOffset( "DT_CSPlayer", "m_bIsScoped" );
		return GetValue<bool>( m_bIsScoped );
	}

	void Spotted( bool value )
	{
		static int m_bSpotted = g_pNetvars->GetOffset( "DT_BaseEntity", "m_bSpotted" );
		*reinterpret_cast<bool*>(uintptr_t( this ) + m_bSpotted) = value;
	}

	float GetInaccuracy()
	{
		typedef float( __thiscall* oInaccuracy )(PVOID);
		return GetVFunc< oInaccuracy >( this, 471 )(this);
	}

	float GetSpread()
	{
		typedef float( __thiscall* oWeaponSpread )(PVOID);
		return GetVFunc< oWeaponSpread >( this, 439 )(this);
	}

	void set_m_bUseCustomBloomScale( byte value )
	{
		static int m_bUseCustomBloomScale = g_pNetvars->GetOffset( "DT_EnvTonemapController", "m_bUseCustomBloomScale" );
		*reinterpret_cast<byte*>(uintptr_t( this ) + m_bUseCustomBloomScale) = value;
	}

	void set_m_bUseCustomAutoExposureMin( byte value ) 
	{
		static int m_bUseCustomAutoExposureMin = g_pNetvars->GetOffset( "DT_EnvTonemapController", "m_bUseCustomAutoExposureMin" );
		*reinterpret_cast<byte*>(uintptr_t( this ) + m_bUseCustomAutoExposureMin) = value;
	}

	void set_m_bUseCustomAutoExposureMax( byte value ) 
	{
		static int m_bUseCustomAutoExposureMax = g_pNetvars->GetOffset( "DT_EnvTonemapController", "m_bUseCustomAutoExposureMax" );
		*reinterpret_cast<byte*>(uintptr_t( this ) + m_bUseCustomAutoExposureMax) = value;
	}

	void set_m_flCustomBloomScale( float value ) 
	{
		static int m_flCustomBloomScale = g_pNetvars->GetOffset( "DT_EnvTonemapController", "m_flCustomBloomScale" );
		*reinterpret_cast<float*>(uintptr_t( this ) + m_flCustomBloomScale) = value;
	}

	void set_m_flCustomAutoExposureMin( float value ) 
	{
		static int m_flCustomAutoExposureMin = g_pNetvars->GetOffset( "DT_EnvTonemapController", "m_flCustomAutoExposureMin" );
		*reinterpret_cast<float*>(uintptr_t( this ) + m_flCustomAutoExposureMin) = value;
	}

	void set_m_flCustomAutoExposureMax( float value ) 
	{
		static int m_flCustomAutoExposureMax = g_pNetvars->GetOffset( "DT_EnvTonemapController", "m_flCustomAutoExposureMax" );
		*reinterpret_cast<float*>(uintptr_t( this ) + m_flCustomAutoExposureMax) = value;
	}

	bool IsVisible( Vector origin );
};


class C_BaseCombatWeapon : public C_BaseEntity
{
private:
    template<class T>
    T GetPointer(const int offset)
    {
        return reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }
    // To get value from the pointer itself
    template<class T>
    T GetValue(const int offset)
    {
        return *reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }

public:
    ItemDefinitionIndex GetItemDefinitionIndex()
    {
        static int m_iItemDefinitionIndex = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_AttributeManager", "m_Item", "m_iItemDefinitionIndex");
        return GetValue<ItemDefinitionIndex>(m_iItemDefinitionIndex);
    }

    float GetNextPrimaryAttack()
    {
        static int m_flNextPrimaryAttack = g_pNetvars->GetOffset("DT_BaseCombatWeapon", "LocalActiveWeaponData", "m_flNextPrimaryAttack");
        return GetValue<float>(m_flNextPrimaryAttack);
    }

    int GetAmmo()
    {
        static int m_iClip1 = g_pNetvars->GetOffset("DT_BaseCombatWeapon", "m_iClip1");
        return GetValue<int>(m_iClip1);
    }

	WeaponInfo_t* GetCSWpnData()
	{
        static auto system = *reinterpret_cast<CWeaponSystem**>(Utils::FindSignature("client_panorama.dll", "8B 35 ? ? ? ? FF 10 0F B7 C0") + 0x2u);
        return system->GetWpnData(this->GetItemDefinitionIndex());
	}

    std::string GetName()
    {
        ///TODO: Test if szWeaponName returns proper value for m4a4 / m4a1-s or it doesnt recognize them.
        return std::string(this->GetCSWpnData()->szWeaponName);
    }
};
