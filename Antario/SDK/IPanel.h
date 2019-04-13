#pragma once
#include "..\Utils\Utils.h"

class IPanel
{
public:
	const char *GetName( unsigned int vguiPanel )
	{
		return Utils::CallVFunc<36, const char*>( this, vguiPanel );
	}
};

namespace vgui
{
	typedef unsigned long HFont;
	typedef unsigned int VPANEL;

	enum font_flags {
		none,
		italic = 1,
		underline = 2,
		strike = 4,
		symbol = 8,
		anti_alias = 16,
		gaussian_blur = 32,
		rotary = 64,
		drop_shadow = 128,
		additive = 256,
		outline = 512,
		custom = 1024,
		bitmap = 2048
	};

};

extern IPanel* g_VGuiPanel;