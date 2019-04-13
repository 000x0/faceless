#include "Interfaces.h"
#include "Utils.h"

#include "..\SDK\IClientMode.h"
#include "..\SDK\IBaseClientDll.h"
#include "..\SDK\IClientEntityList.h"
#include "..\SDK\IVEngineClient.h"
#include "..\SDK\CPrediction.h"
#include "..\SDK\IGameEvent.h"
#include "..\SDK\ISurface.h"
#include "..\SDK\IVModelRender.h"
#include "..\SDK\IMaterialSystem.h"
#include "..\SDK\IVModelInfoClient.h"
#include "..\SDK\IRenderView.h"
#include "..\SDK\ICvar.h"
#include "..\SDK\IPanel.h"
#include "..\SDK\IEngineTrace.h"

// Initializing global interfaces

IBaseClientDLL*     g_pClientDll    = nullptr;
IClientMode*        g_pClientMode   = nullptr;
IClientEntityList*  g_pEntityList   = nullptr;
IVEngineClient*     g_pEngine       = nullptr;
CPrediction*        g_pPrediction   = nullptr;
IGameMovement*      g_pMovement     = nullptr;
IMoveHelper*        g_pMoveHelper   = nullptr;
CGlobalVarsBase*    g_pGlobalVars   = nullptr;
IGameEventManager2* g_pEventManager = nullptr;
ISurface*           g_pSurface      = nullptr;
IVModelRender*		g_MdlRender     = nullptr;
IMaterialSystem*    g_MatSystem     = nullptr;
IVModelInfoClient*  g_MdlInfo       = nullptr;
IVRenderView*       g_pRenderView   = nullptr;
ICVar*				g_pCVar         = nullptr;
IPanel*             g_VGuiPanel     = nullptr;
IEngineTrace*       g_pTrace        = nullptr;

namespace interfaces
{
    template<typename T>
    T* CaptureInterface(const char* szModuleName, const char* szInterfaceVersion)
    {
        HMODULE moduleHandle = GetModuleHandleA(szModuleName);
        if (moduleHandle)   /* In case of not finding module handle, throw an error. */
        {
            CreateInterfaceFn pfnFactory = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(moduleHandle, "CreateInterface"));
            return reinterpret_cast<T*>(pfnFactory(szInterfaceVersion, nullptr));
        }
        Utils::Log("Error getting interface %", szInterfaceVersion);
        return nullptr;
    }


    void Init()
    {
        g_pClientDll    = CaptureInterface<IBaseClientDLL>("client_panorama.dll", "VClient018");                 // Get IBaseClientDLL
        g_pClientMode   = **reinterpret_cast<IClientMode***>    ((*reinterpret_cast<uintptr_t**>(g_pClientDll))[10] + 0x5u);                // Get IClientMode
        g_pGlobalVars   = **reinterpret_cast<CGlobalVarsBase***>((*reinterpret_cast<uintptr_t**>(g_pClientDll))[0]  + 0x1Bu);               // Get CGlobalVarsBase
        g_pEntityList   = CaptureInterface<IClientEntityList>("client_panorama.dll", "VClientEntityList003");    // Get IClientEntityList
        g_pEngine       = CaptureInterface<IVEngineClient>("engine.dll", "VEngineClient014");			            // Get IVEngineClient
        g_pPrediction   = CaptureInterface<CPrediction>("client_panorama.dll", "VClientPrediction001");          // Get CPrediction
        g_pMovement     = CaptureInterface<IGameMovement>("client_panorama.dll", "GameMovement001");             // Get IGameMovement
        g_pMoveHelper   = **reinterpret_cast<IMoveHelper***>((Utils::FindSignature("client_panorama.dll", "8B 0D ? ? ? ? 8B 46 08 68") + 0x2));  // Get IMoveHelper
        g_pEventManager = CaptureInterface<IGameEventManager2>("engine.dll", "GAMEEVENTSMANAGER002");            // Get IGameEventManager2
        g_pSurface      = CaptureInterface<ISurface>("vguimatsurface.dll", "VGUI_Surface031");		            // Get ISurface
		g_MdlRender     = CaptureInterface<IVModelRender>( "engine.dll", "VEngineModel016" );					// Get IVMofelRender
		g_MatSystem     = CaptureInterface<IMaterialSystem>( "materialsystem.dll", "VMaterialSystem080" );          // Get IMaterialSystem
		g_MdlInfo       = CaptureInterface<IVModelInfoClient>( "engine.dll", "VModelInfoClient004" );				// Get IVModelInfoClient
		g_pRenderView   = CaptureInterface<IVRenderView>( "engine.dll", "VEngineRenderView014" );					// Get IVRenderView
		g_pCVar         = CaptureInterface<ICVar>( "vstdlib.dll", "VEngineCvar007" );							// Get Convars
		g_VGuiPanel     = CaptureInterface<IPanel>( "vgui2.dll", "VGUI_Panel009" );							// Get Panel
		g_pTrace        = CaptureInterface<IEngineTrace>( "engine.dll", "EngineTraceClient004" );            //Get Trace
    }
}
