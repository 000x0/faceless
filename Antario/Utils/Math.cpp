#include "Math.h"

namespace Math
{
	void VectorAngle( const Vector& forward, Vector& angles ) 
	{
		float tmp, yaw, pitch;

		if (forward[1] == 0 && forward[0] == 0) {
			yaw = 0;
			if (forward[2] > 0)
				pitch = 270;
			else
				pitch = 90;
		}
		else {
			yaw = (atan2( forward[1], forward[0] ) * 180 / M_PI);
			if (yaw < 0)
				yaw += 360;

			tmp = sqrt( forward[0] * forward[0] + forward[1] * forward[1] );
			pitch = (atan2( -forward[2], tmp ) * 180 / M_PI);
			if (pitch < 0)
				pitch += 360;
		}

		angles[0] = pitch;
		angles[1] = yaw;
		angles[2] = 0;
	}
	Vector CalcAngle( Vector src, Vector dst ) 
	{
		Vector ret;
		Math::VectorAngle( dst - src, ret );
		return ret;
	}
	void VectorTransform( const Vector in1, const matrix3x4_t in2, Vector &out ) 
	{
		out[0] = DotProduct( in1, Vector( in2[0][0], in2[0][1], in2[0][2] ) ) + in2[0][3];
		out[1] = DotProduct( in1, Vector( in2[1][0], in2[1][1], in2[1][2] ) ) + in2[1][3];
		out[2] = DotProduct( in1, Vector( in2[2][0], in2[2][1], in2[2][2] ) ) + in2[2][3];
	}
	Vector NormalizeAngle( Vector angle )
	{
		while (angle.x < -180.0f) angle.x += 360.0f;
		while (angle.x > 180.0f) angle.x -= 360.0f;

		while (angle.y < -180.0f) angle.y += 360.0f;
		while (angle.y > 180.0f) angle.y -= 360.0f;

		while (angle.z < -180.0f) angle.z += 360.0f;
		while (angle.z > 180.0f) angle.z -= 360.0f;

		return angle;
	}
	Vector ClampAngle( Vector angle )
	{
		while (angle.y > 180) angle.y -= 360;
		while (angle.y < -180) angle.y += 360;

		if (angle.x > 89.0f) angle.x = 89.0f;
		if (angle.x < -89.0f) angle.x = -89.0f;

		angle.z = 0.f;

		return angle;
	}
	float RandFloat( float min, float max )
	{
		static auto random_float = reinterpret_cast<float( *)(float, float)>(GetProcAddress( GetModuleHandleA( "vstdlib.dll" ), "RandomFloat" ));

		return random_float( min, max );
	}
	void Circle( int X, int Y, int radius, int numSides, Color Color )
	{
		D3DXVECTOR2 Line[128];
		float Step = 3.141 * 2.0 / numSides;
		int Count = 0;
		for (float a = 0; a < 3.141 * 2.0; a += Step)
		{
			float X1 = radius * cos( a ) + X;
			float Y1 = radius * sin( a ) + Y;
			float X2 = radius * cos( a + Step ) + X;
			float Y2 = radius * sin( a + Step ) + Y;
			g_Render.Line( X1, Y1, X2, Y2, Color );
			Count += 2;
		}
	}
}