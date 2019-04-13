#include <thread>
#include "Hooks.h"
#include "Utils\Utils.h"
#include "Features\Features.h"

Misc      g_Misc;
Hooks     g_Hooks;
Settings  g_Settings;


void Hooks::Init()
{
    // Get window handle
    while (!(g_Hooks.hCSGOWindow = FindWindowA("Valve001", nullptr)))
    {
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(50ms);
    }

    interfaces::Init();                         // Get interfaces
    g_pNetvars = std::make_unique<NetvarTree>();// Get netvars after getting interfaces as we use them

    Utils::Log("Hooking in progress...");

    // D3D Device pointer
    const uintptr_t d3dDevice = **reinterpret_cast<uintptr_t**>(Utils::FindSignature("shaderapidx9.dll", "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);

    if (g_Hooks.hCSGOWindow)        // Hook WNDProc to capture mouse / keyboard input
        g_Hooks.pOriginalWNDProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(g_Hooks.hCSGOWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Hooks::WndProc)));


    // VMTHooks
    g_Hooks.pD3DDevice9Hook     = std::make_unique<VMTHook>(reinterpret_cast<void*>(d3dDevice));
    g_Hooks.pClientModeHook     = std::make_unique<VMTHook>(g_pClientMode);
    g_Hooks.pSurfaceHook        = std::make_unique<VMTHook>(g_pSurface);
	g_Hooks.pDMEHook            = std::make_unique<VMTHook>(g_MdlRender);
	g_Hooks.pMaterialSystemHook = std::make_unique<VMTHook>( g_MatSystem );
	g_Hooks.pSceneEndHook       = std::make_unique<VMTHook>( g_pRenderView );
	g_Hooks.pVGuiHook           = std::make_unique<VMTHook>( g_VGuiPanel );
	//g_Hooks.pBspQueryHook        = std::make_unique<VMTHook>( g_pEngine->GetBSPTreeQuery() );


    // Hook the table functions
    g_Hooks.pD3DDevice9Hook    ->Hook(vtable_indexes::reset,            Hooks::Reset);
    g_Hooks.pD3DDevice9Hook    ->Hook(vtable_indexes::present,          Hooks::Present);
    g_Hooks.pClientModeHook    ->Hook(vtable_indexes::createMove,       Hooks::CreateMove);
    g_Hooks.pSurfaceHook       ->Hook(vtable_indexes::lockCursor,       Hooks::LockCursor);
	g_Hooks.pDMEHook           ->Hook(vtable_indexes::DME,              Hooks::DME);
	g_Hooks.pMaterialSystemHook->Hook( vtable_indexes::MSH,             Hooks::BeginFrame );
	g_Hooks.pSceneEndHook      ->Hook( vtable_indexes::sceneEnd,        Hooks::SceneEnd );
	g_Hooks.pClientModeHook    ->Hook( vtable_indexes::viewModelFOV,    Hooks::ViewModelFOV );
	g_Hooks.pVGuiHook          ->Hook( vtable_indexes::paint,           Hooks::hkPaintTraverse );
	//g_Hooks.pBspQueryHook      ->Hook( vtable_indexes::listLeavesInBox, Hooks::hkListLeavesInBox );


	// Cvar Stuff
	g_pCVar->FindVar( "mat_postprocess_enable" )->SetValue( 0 );
	g_pCVar->FindVar( "crosshair" )->SetValue( 0 );

    // Create event listener, no need for it now so it will remain commented.
    const std::vector<const char*> vecEventNames = { "player_hurt" };
    g_Hooks.pEventListener = std::make_unique<EventListener>(vecEventNames);

    Utils::Log("Hooking completed!");
}


void Hooks::Restore()
{
	Utils::Log("Unhooking in progress...");
    {   // Unhook every function we hooked and restore original one
        g_Hooks.pD3DDevice9Hook->Unhook(vtable_indexes::reset);
        g_Hooks.pD3DDevice9Hook->Unhook(vtable_indexes::present);
        g_Hooks.pClientModeHook->Unhook(vtable_indexes::createMove);
		g_Hooks.pSurfaceHook->Unhook( vtable_indexes::lockCursor );
		g_Hooks.pDMEHook->Unhook( vtable_indexes::DME );
		g_Hooks.pMaterialSystemHook->Unhook( vtable_indexes::MSH );
		g_Hooks.pSceneEndHook->Unhook( vtable_indexes::sceneEnd );
		g_Hooks.pVGuiHook->Unhook( vtable_indexes::paint );
		//g_Hooks.pBspQueryHook->Unhook( vtable_indexes::listLeavesInBox );

        SetWindowLongPtr(g_Hooks.hCSGOWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_Hooks.pOriginalWNDProc));

        g_pNetvars.reset();   /* Need to reset by-hand, global pointer so doesnt go out-of-scope */
    }
    Utils::Log("Unhooking succeded!");

    // Destroy fonts and all textures we created
    g_Render.Release();
}

void __fastcall Hooks::BeginFrame( void *thisptr, void*, float ft )
{
	static auto oBeginFrame = g_Hooks.pMaterialSystemHook->GetOriginal<BeginFrame_t>( vtable_indexes::MSH );

	oBeginFrame( thisptr, ft );
}

bool __fastcall Hooks::CreateMove(IClientMode* thisptr, void* edx, float sample_frametime, CUserCmd* pCmd)
{
    // Call original createmove before we start screwing with it
    static auto oCreateMove = g_Hooks.pClientModeHook->GetOriginal<CreateMove_t>(24);
    oCreateMove(thisptr, edx, sample_frametime, pCmd);

    if (!pCmd || !pCmd->command_number)
        return oCreateMove;

    // Get globals
    g::pCmd         = pCmd;
    g::pLocalEntity = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
    if (!g::pLocalEntity)
        return oCreateMove;

	// run shit outside enginepred
	pCmd->buttons |= IN_BULLRUSH;
    g_Misc.OnCreateMove();

    engine_prediction::RunEnginePred();
    // run shit in enginepred

	g_Legitbot.OnCreateMove();

    engine_prediction::EndEnginePred();

	Math::ClampAngle( pCmd->viewangles );

    return false;
}

//#define MAX_COORD_FLOAT ( 16384.0f )
//#define MIN_COORD_FLOAT ( -MAX_COORD_FLOAT )
//int __fastcall Hooks::hkListLeavesInBox( void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax )
//{

//}

void __fastcall Hooks::DME( void* ecx, void* edx, IMatRenderContext* context, const DrawModelState_t& state, const ModelRenderInfo_t& render_info, matrix3x4_t* matrix )
{
	static auto oDME = g_Hooks.pDMEHook->GetOriginal<DrawModelExecute_t>( vtable_indexes::DME );

	if (!g::pLocalEntity || !g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return oDME( ecx, context, state, render_info, matrix );

	//static IMaterial* material = g_MatSystem->FindMaterial( "debug/debugambientcube", "Model textures" );

	//if (material || !material->IsErrorMaterial())
	//{
	//	for (int i = 1; i <= g_pEngine->GetMaxClients(); ++i)
	//	{
	//		auto entity = g_pEntityList->GetClientEntity( render_info.entity_index );

	//		if (entity && entity->IsAlive() && !entity->IsDormant())
	//		{
	//			if (strstr( g_MdlInfo->GetModelName( render_info.pModel ), "models/player" ))
	//			{
	//				float r = (255 - 2.55 * entity->GetHealth()) / 255.0f;
	//				float g = (2.55 * entity->GetHealth()) / 255.0f;
	//				float b = 0.0f;
	//				g_pRenderView->SetColorModulation( r, g, b );

	//				material->IncrementReferenceCount();
	//				material->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, true );

	//				g_MdlRender->ForcedMaterialOverride( material );

	//				//oDME( ecx, context, state, render_info, matrix );
	//			}
	//		}
	//	}
	//}

	oDME( ecx, context, state, render_info, matrix );
	g_MdlRender->ForcedMaterialOverride( nullptr );
}


void __fastcall Hooks::SceneEnd( void* thisptr, void* edx )
{
	static auto oSceneEnd = g_Hooks.pSceneEndHook->GetOriginal<SceneEnd_t>( vtable_indexes::sceneEnd );

	if (!g::pLocalEntity || !g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return oSceneEnd( thisptr );

	static IMaterial *material = g_MatSystem->FindMaterial( "chams", "Model textures", true, NULL );

	if (!material || material->IsErrorMaterial())
		return;

	for (int i = 1; i <= g_pEngine->GetMaxClients(); ++i)
	{
		auto entity = g_pEntityList->GetClientEntity( i );

		if (entity && entity != g::pLocalEntity && !entity->IsDormant() && entity->GetTeam() != g::pLocalEntity->GetTeam())
		{
			//invisible
			g_pRenderView->SetColorModulation( 255, 255, 255 );

			material->IncrementReferenceCount();
			material->AlphaModulate( 0.1f );
			material->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, true );

			g_MdlRender->ForcedMaterialOverride( material );

			entity->DrawModel( 0x1, 255 );
			g_MdlRender->ForcedMaterialOverride( nullptr );

			//visible
			float r = (255 - 2.55 * entity->GetHealth()) / 255.0f;
			float g = (2.55 * entity->GetHealth()) / 255.0f;
			float b = 0.0f;
			g_pRenderView->SetColorModulation( r, g, b );

			material->IncrementReferenceCount();
			material->AlphaModulate( 1.f );
			material->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, false );

			g_MdlRender->ForcedMaterialOverride( material );

			entity->DrawModel( 0x1, 255 );
			g_MdlRender->ForcedMaterialOverride( nullptr );
		}
	}

	oSceneEnd( thisptr );
}

float __stdcall Hooks::ViewModelFOV()
{
	return 95.f;
}

void __stdcall Hooks::hkPaintTraverse( vgui::VPANEL panel, bool forceRepaint, bool allowForce )
{
	static auto oPaint = g_Hooks.pVGuiHook->GetOriginal<PaintTraverse>( vtable_indexes::paint );

	if (g::pLocalEntity && g_pEngine->IsInGame() && g_pEngine->IsConnected())
	{
		if (!strcmp( "HudZoom", g_VGuiPanel->GetName( panel ) )) //noscope
				return;
	}
	oPaint( g_VGuiPanel, panel, forceRepaint, allowForce );
}

void __fastcall Hooks::LockCursor(ISurface* thisptr, void* edx)
{
    static auto oLockCursor = g_Hooks.pSurfaceHook->GetOriginal<LockCursor_t>(vtable_indexes::lockCursor);

    if (!g_Settings.bMenuOpened)
        return oLockCursor(thisptr, edx);

    g_pSurface->UnlockCursor();
}


HRESULT __stdcall Hooks::Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    static auto oReset = g_Hooks.pD3DDevice9Hook->GetOriginal<Reset_t>(vtable_indexes::reset);

    if (g_Hooks.bInitializedDrawManager)
    {
        Utils::Log("Reseting draw manager.");
        g_Render.OnLostDevice();
        HRESULT hr = oReset(pDevice, pPresentationParameters);
        g_Render.OnResetDevice(pDevice);
        Utils::Log("DrawManager reset succeded.");
        return hr;
    }

    return oReset(pDevice, pPresentationParameters);
}


HRESULT __stdcall Hooks::Present(IDirect3DDevice9* pDevice, const RECT* pSourceRect, const RECT* pDestRect, 
                                 HWND hDestWindowOverride,  const RGNDATA* pDirtyRegion)
{
    IDirect3DStateBlock9* stateBlock     = nullptr;
    IDirect3DVertexDeclaration9* vertDec = nullptr;

    pDevice->GetVertexDeclaration(&vertDec);
    pDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &stateBlock);

    [pDevice]()
    {
        if (!g_Hooks.bInitializedDrawManager)
        {
            Utils::Log("Initializing Draw manager");
            g_Render.InitDeviceObjects(pDevice);
            g_Hooks.nMenu.Initialize();
            g_Hooks.bInitializedDrawManager = true;
            Utils::Log("Draw manager initialized");
        }
        else
        {
            g_Render.SetupRenderStates(); // Sets up proper render states for our state block

            static std::string szWatermark = "gansta hook";
          
            /* Put your draw calls here */
            g_ESP.Render();            
            /* ------------------------ */
          
	    	// Render menu after ESP so menu overlaps ESP
            // g_Render.String(8, 8, FONT_DROPSHADOW, Color(250, 150, 200, 180), g_Fonts.vecFonts[FONT_TAHOMA_8], szWatermark.c_str());

            if (g_Settings.bMenuOpened)
            {
                g_Hooks.nMenu.Render();             // Render our menu
                g_Hooks.nMenu.mouseCursor->Render();// Render mouse cursor in the end so its not overlapped
            }
        }
    }();

    stateBlock->Apply();
    stateBlock->Release();
    pDevice   ->SetVertexDeclaration(vertDec);

    static auto oPresent = g_Hooks.pD3DDevice9Hook->GetOriginal<Present_t>(17);
    return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}


LRESULT Hooks::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // for now as a lambda, to be transfered somewhere
    // Thanks uc/WasserEsser for pointing out my mistake!
    // Working when you HOLD th button, not when you press it.
    const auto getButtonHeld = [uMsg, wParam](bool& bButton, int vKey)
    {
        if (wParam != vKey) return;

        if (uMsg == WM_KEYDOWN)
            bButton = true;
        else if (uMsg == WM_KEYUP)
            bButton = false;
    };

    const auto getButtonToggle = [uMsg, wParam](bool& bButton, int vKey)
    {
        if (wParam != vKey) return;

        if (uMsg == WM_KEYUP)
            bButton = !bButton;
    };

	getButtonToggle( g_Settings.bCheatActive, VK_DELETE );
    //getButtonToggle(g_Settings.bMenuOpened, VK_INSERT);

    if (g_Hooks.bInitializedDrawManager)
    {
        // our wndproc capture fn
        if (g_Settings.bMenuOpened)
        {
            g_Hooks.nMenu.MsgProc(uMsg, wParam, lParam);
            return true;
        }
    }

    // Call original wndproc to make game use input again
    return CallWindowProcA(g_Hooks.pOriginalWNDProc, hWnd, uMsg, wParam, lParam);
}
