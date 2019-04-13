#pragma once
#include "Math.h"
#include <d3dx9math.h>
#include "DrawManager.h"

namespace Math
{
	void VectorAngle( const Vector& forward, Vector& angles );
	Vector CalcAngle( Vector src, Vector dst );
	Vector NormalizeAngle( Vector angle );
	Vector ClampAngle( Vector angles );
	void VectorTransform( const Vector in1, const matrix3x4_t in2, Vector &out );
	float RandFloat( float min, float max );
	void Circle( int X, int Y, int radius, int numSides, Color Color );
}