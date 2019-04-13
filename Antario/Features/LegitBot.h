#pragma once
#include "..\SDK\CEntity.h"

class Legitbot {
private:
	Vector HitBoxPosition( C_BaseEntity* entity, int hitbox_id );
public:
	void OnCreateMove();
};

extern Legitbot g_Legitbot;
