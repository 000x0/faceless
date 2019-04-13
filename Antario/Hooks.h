#pragma once

#include "Utils\DrawManager.h"
#include "Utils\Interfaces.h"
#include "SDK\IClientMode.h"
#include "SDK\ISurface.h"
#include "EventListener.h"
#include "SDK\CInput.h"
#include "GUI\GUI.h"
#include "SDK\IVModelRender.h"
#include "SDK\IMaterialSystem.h"
#include "SDK\IVModelInfoClient.h"
#include "SDK\IRenderView.h"
#include "SDK\ICvar.h"
#include "SDK\IPanel.h"


namespace vtable_indexes
{
	constexpr auto reset           = 16;
	constexpr auto present         = 17;
	constexpr auto createMove      = 24;
	constexpr auto lockCursor      = 67;
	constexpr auto DME             = 21;
	constexpr auto MSH             = 42;
	constexpr auto sceneEnd        = 9;
	constexpr auto viewModelFOV    = 35;
	constexpr auto paint           = 41;
	constexpr auto listLeavesInBox = 6;
}

class VMTHook;
class Hooks
{
public:
    // Initialization setup, called on injection
    static void Init();
    static void Restore();

    /*---------------------------------------------*/
    /*-------------Hooked functions----------------*/
    /*---------------------------------------------*/

	static void     __fastcall  BeginFrame( void *thisptr, void*, float ft );
    static bool     __fastcall  CreateMove(IClientMode*, void*, float, CUserCmd*);
	static int      __fastcall  hkListLeavesInBox( void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax );
	static void     __fastcall  DME( void*, void*, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t* );
	static void     __fastcall  SceneEnd( void* thisptr, void* edx );
	static float    __stdcall   ViewModelFOV();
	static void     __stdcall   hkPaintTraverse( vgui::VPANEL panel, bool forceRepaint, bool allowForce );
    static void     __fastcall  LockCursor(ISurface*, void*);
    static HRESULT  __stdcall   Reset     (IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
    static HRESULT  __stdcall   Present   (IDirect3DDevice9* pDevice, const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion);
    static LRESULT  __stdcall   WndProc   (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    /*---------------------------------------------*/
    /*-------------VMT Hook pointers---------------*/
    /*---------------------------------------------*/

    std::unique_ptr<VMTHook> pD3DDevice9Hook;
    std::unique_ptr<VMTHook> pClientModeHook;
    std::unique_ptr<VMTHook> pSurfaceHook;
	std::unique_ptr<VMTHook> pDMEHook;
	std::unique_ptr<VMTHook> pMaterialSystemHook;
	std::unique_ptr<VMTHook> pSceneEndHook;
	std::unique_ptr<VMTHook> pVGuiHook;
	std::unique_ptr<VMTHook> pBspQueryHook;

    /*---------------------------------------------*/
    /*-------------Hook prototypes-----------------*/
    /*---------------------------------------------*/

	typedef void( __thiscall *BeginFrame_t )(void*, float);
    typedef bool (__fastcall* CreateMove_t) (IClientMode*, void*, float, CUserCmd*);
	typedef int( __thiscall * ListLeavesInBox )(void*, const Vector&, const Vector&, unsigned short*, int);
	typedef void( __thiscall* DrawModelExecute_t )(void*, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
	typedef void( __thiscall* SceneEnd_t )  (void*);
    typedef void (__fastcall* LockCursor_t) (ISurface*, void*);
    typedef long (__stdcall*  Reset_t)      (IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
    typedef long (__stdcall*  Present_t)    (IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
	using PaintTraverse = void( __thiscall* )(IPanel*, vgui::VPANEL, bool, bool);

private:
    ui::MenuMain                   nMenu;
    HWND                           hCSGOWindow             = nullptr; // CSGO window handle
    bool                           bInitializedDrawManager = false;   // Check if we initialized our draw manager
    WNDPROC                        pOriginalWNDProc        = nullptr; // Original CSGO window proc
    std::unique_ptr<EventListener> pEventListener          = nullptr; // Listens to csgo events, needs to be created
};

extern Hooks g_Hooks;


class VMTHook
{
public:
    VMTHook(void* ppClass)
    {
        this->ppBaseClass = static_cast<std::uintptr_t**>(ppClass);

        // loop through all valid class indexes. When it will hit invalid (not existing) it will end the loop
        while (static_cast<std::uintptr_t*>(*this->ppBaseClass)[this->indexCount])
            ++this->indexCount;

        const std::size_t kSizeTable = this->indexCount * sizeof(std::uintptr_t);

        this->pOriginalVMT = *this->ppBaseClass;
        this->pNewVMT      = std::make_unique<std::uintptr_t[]>(this->indexCount);

        // copy original vtable to our local copy of it
        std::memcpy(this->pNewVMT.get(), this->pOriginalVMT, kSizeTable);

        // replace original class with our local copy
        *this->ppBaseClass = this->pNewVMT.get();
    };
    ~VMTHook() { *this->ppBaseClass = this->pOriginalVMT; };

    template<class Type>
    Type GetOriginal(const std::size_t index)
    {
        return reinterpret_cast<Type>(this->pOriginalVMT[index]);
    };

    HRESULT Hook(const std::size_t index, void* fnNew)
    {
        if (index > this->indexCount)   // check if given index is valid
            return E_INVALIDARG;

        this->pNewVMT[index] = reinterpret_cast<std::uintptr_t>(fnNew);
        return S_OK;
    };

    HRESULT Unhook(const std::size_t index)
    {
        if (index > this->indexCount)
            return E_INVALIDARG;

        this->pNewVMT[index] = this->pOriginalVMT[index];
        return S_OK;
    };

private:
    std::unique_ptr<std::uintptr_t[]> pNewVMT      = nullptr;    // Actual used vtable
    std::uintptr_t** ppBaseClass  = nullptr; // Saved pointer to original class
    std::uintptr_t*  pOriginalVMT = nullptr; // Saved original pointer to the VMT
    std::size_t      indexCount   = 0;       // Count of indexes inside out f-ction
};
