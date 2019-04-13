#pragma once
#include "..\Utils\Utils.h"

class ISurface
{
public:
	void UnlockCursor()
	{
		return Utils::CallVFunc<66, void>(this);
	}
	void IPlaySound( const char* fileName )
	{
		typedef void( __thiscall* OriginalFn )(void*, const char*);
		return Utils::CallVFunc<82, void>( this, fileName );
	}
};
extern ISurface* g_pSurface;