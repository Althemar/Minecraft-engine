#pragma once

#include "engine/utils/types_3d.h"


class NYPerlin
{
public:
	NYVert3Df * _Dirs;
	int _Width;
	int _Size;

	NYPerlin()
	{
		_Width = 50;
		_Size = _Width*_Width*_Width;
		_Dirs = new NYVert3Df[_Size];
		for (int i = 0; i < _Size; i++)
		{
			_Dirs[i].X = randf();
			_Dirs[i].Y = randf();
			_Dirs[i].Z = randf();
		}
	}

	float lerp(float a, float b, float alpha)
	{
		float alphaSmooth = alpha * ( 3 * alpha - 2 * alpha*alpha );
		return ( 1 - alphaSmooth )*a + alphaSmooth * b;
	}
public:

	float sample(float xBase, float yBase, float zBase)
	{
		float x = xBase;
		float y = yBase;
		float z = zBase;

		while (x > _Width)
			x -= _Width;
		while (y > _Width)
			y -= _Width;
		while (z > _Width)
			z -= _Width;


		int x1 = floor(x);
		int x2 = floor(x) + 1;
		int y1 = floor(y);
		int y2 = floor(y) + 1;
		int z1 = floor(z);
		int z2 = floor(z) + 1;
		float dx = x - x1;
		float dy = y - y1;
		float dz = z - z1;

		NYVert3Df pos(x, y, z);
		NYVert3Df sommets[8];
		//plan X2
		sommets[0] = NYVert3Df(x2, y1, z1);
		sommets[1] = NYVert3Df(x2, y1, z2);
		sommets[2] = NYVert3Df(x2, y2, z2);
		sommets[3] = NYVert3Df(x2, y2, z1);

		//plan X1
		sommets[4] = NYVert3Df(x1, y1, z1);
		sommets[5] = NYVert3Df(x1, y1, z2);
		sommets[6] = NYVert3Df(x1, y2, z2);
		sommets[7] = NYVert3Df(x1, y2, z1);

		float angles[8];
		for (int i = 0; i < 8; i++)
		{
			angles[i] = ( pos - sommets[i] ).scalProd(
				_Dirs[( int )( sommets[i].X*_Width*_Width + sommets[i].Y*_Width + sommets[i].Z )]);
		}

		//plan X2
		float ybas = lerp(angles[0], angles[3], dy);
		float yhaut = lerp(angles[1], angles[2], dy);
		float mid2 = lerp(ybas, yhaut, dz);

		//plan X1
		ybas = lerp(angles[4], angles[7], dy);
		yhaut = lerp(angles[5], angles[6], dy);
		float mid1 = lerp(ybas, yhaut, dz);

		float res = lerp(mid1, mid2, dx);

		res = ( res + 1 ) / 2.0f;


		return res;
	}
};