#ifndef __WORLD_H__
#define __WORLD_H__

#include "gl/glew.h"
#include "gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "cube.h"
#include "chunk.h"
#include "perlin.h"
#include "FractalNoise.h"
#include <ctime>

typedef uint8 NYAxis;
#define NY_AXIS_X 0x01
#define NY_AXIS_Y 0x02
#define NY_AXIS_Z 0x04

#define MAT_SIZE 10 //en nombre de chunks
#define MAT_HEIGHT 5 //en nombre de chunks
#define MAT_SIZE_CUBES (MAT_SIZE * NYChunk::CHUNK_SIZE)
#define MAT_HEIGHT_CUBES (MAT_HEIGHT * NYChunk::CHUNK_SIZE)


class NYWorld
{
public :
	NYChunk * _Chunks[MAT_SIZE][MAT_SIZE][MAT_HEIGHT];
	int _MatriceHeights[MAT_SIZE_CUBES][MAT_SIZE_CUBES];
	int _MatriceHeightsTmp[MAT_SIZE_CUBES][MAT_SIZE_CUBES];
	float _FacteurGeneration;

	NYWorld()
	{
		_FacteurGeneration = 1.0;

		//On crée les chunks
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
					_Chunks[x][y][z] = new NYChunk();

		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
				{
					NYChunk * cxPrev = NULL;
					if(x > 0)
						cxPrev = _Chunks[x-1][y][z];
					NYChunk * cxNext = NULL;
					if(x < MAT_SIZE-1)
						cxNext = _Chunks[x+1][y][z];

					NYChunk * cyPrev = NULL;
					if(y > 0)
						cyPrev = _Chunks[x][y-1][z];
					NYChunk * cyNext = NULL;
					if(y < MAT_SIZE-1)
						cyNext = _Chunks[x][y+1][z];

					NYChunk * czPrev = NULL;
					if(z > 0)
						czPrev = _Chunks[x][y][z-1];
					NYChunk * czNext = NULL;
					if(z < MAT_HEIGHT-1)
						czNext = _Chunks[x][y][z+1];

					_Chunks[x][y][z]->setVoisins(cxPrev,cxNext,cyPrev,cyNext,czPrev,czNext);
				}			
	}

	inline NYCube * getCube(int x, int y, int z)
	{	
		if(x < 0)x = 0;
		if(y < 0)y = 0;
		if(z < 0)z = 0;
		if(x >= MAT_SIZE * NYChunk::CHUNK_SIZE) x = (MAT_SIZE * NYChunk::CHUNK_SIZE)-1;
		if(y >= MAT_SIZE * NYChunk::CHUNK_SIZE) y = (MAT_SIZE * NYChunk::CHUNK_SIZE)-1;
		if(z >= MAT_HEIGHT * NYChunk::CHUNK_SIZE) z = (MAT_HEIGHT * NYChunk::CHUNK_SIZE)-1;

		return &(_Chunks[x / NYChunk::CHUNK_SIZE][y / NYChunk::CHUNK_SIZE][z / NYChunk::CHUNK_SIZE]->_Cubes[x % NYChunk::CHUNK_SIZE][y % NYChunk::CHUNK_SIZE][z % NYChunk::CHUNK_SIZE]);
	}

	inline NYCube* getCubeWithWorldCoordinates(int x, int y, int z)
	{
		if (x < 0)x = 0;
		if (y < 0)y = 0;
		if (z < 0)z = 0;
		if (x >= MAT_SIZE * NYChunk::CHUNK_SIZE * NYCube::CUBE_SIZE) x = ( MAT_SIZE * NYChunk::CHUNK_SIZE * NYCube::CUBE_SIZE ) - 1;
		if (y >= MAT_SIZE * NYChunk::CHUNK_SIZE * NYCube::CUBE_SIZE) y = ( MAT_SIZE * NYChunk::CHUNK_SIZE * NYCube::CUBE_SIZE ) - 1;
		if (z >= MAT_HEIGHT * NYChunk::CHUNK_SIZE * NYCube::CUBE_SIZE) z = ( MAT_HEIGHT * NYChunk::CHUNK_SIZE * NYCube::CUBE_SIZE ) - 1;
		
		
		x = ( int )( x / NYCube::CUBE_SIZE );
		y = ( int )( y / NYCube::CUBE_SIZE );
		z = ( int )( z / NYCube::CUBE_SIZE );
		cout << x << " " << y << " " << z << endl;
		
		return &( _Chunks[x / NYChunk::CHUNK_SIZE][y / NYChunk::CHUNK_SIZE][z / NYChunk::CHUNK_SIZE]->_Cubes[x % NYChunk::CHUNK_SIZE][y % NYChunk::CHUNK_SIZE][z % NYChunk::CHUNK_SIZE] );

	}

	void updateCube(int x, int y, int z)
	{	
		if(x < 0)x = 0;
		if(y < 0)y = 0;
		if(z < 0)z = 0;
		if(x >= MAT_SIZE * NYChunk::CHUNK_SIZE)x = (MAT_SIZE * NYChunk::CHUNK_SIZE)-1;
		if(y >= MAT_SIZE * NYChunk::CHUNK_SIZE)y = (MAT_SIZE * NYChunk::CHUNK_SIZE)-1;
		if(z >= MAT_HEIGHT * NYChunk::CHUNK_SIZE)z = (MAT_HEIGHT * NYChunk::CHUNK_SIZE)-1;
		_Chunks[x / NYChunk::CHUNK_SIZE][y / NYChunk::CHUNK_SIZE][z / NYChunk::CHUNK_SIZE]->disableHiddenCubes();
		_Chunks[x / NYChunk::CHUNK_SIZE][y / NYChunk::CHUNK_SIZE][z / NYChunk::CHUNK_SIZE]->toVbo();
	}

	void deleteCube(int x, int y, int z)
	{
		NYCube * cube = getCube(x,y,z);
		cube->_Draw = false;
		cube = getCube(x-1,y,z);
		updateCube(x,y,z);	
	}

	//Création d'une pile de cubes
	//only if zero permet de ne générer la  pile que si sa hauteur actuelle est de 0 (et ainsi de ne pas regénérer de piles existantes)
	void load_pile(int x, int y, int height, bool onlyIfZero = true)
	{
		if (height < 1)
			height = 1;
		if (height >= MAT_HEIGHT_CUBES)
			height = MAT_HEIGHT_CUBES - 1;

		if (_MatriceHeights[x][y] != 0 && onlyIfZero)
			return;

		getCube(x, y, 0)->_Draw = true;
		getCube(x, y, 0)->_Type = CUBE_EAU;
		for (int z = 1; z<height; z++)
		{
			getCube(x, y, z)->_Draw = true;
			getCube(x, y, z)->_Type = CUBE_TERRE;
		}

		if (height - 1>0)
		{
			getCube(x, y, height - 1)->_Draw = true;
			getCube(x, y, height - 1)->_Type = CUBE_HERBE;
		}

		for (int z = height; z<MAT_HEIGHT_CUBES; z++)
		{
			getCube(x, y, z)->_Draw = true;
			getCube(x, y, z)->_Type = CUBE_AIR;
		}

		_MatriceHeights[x][y] = height;
	}

	
	//Creation du monde entier, en utilisant le mouvement brownien fractionnaire
	void generate_piles(int x1, int y1,
		int x2, int y2,
		int x3, int y3,
		int x4, int y4, int prof, int profMax = -1)
	{
		if (( x3 - x1 ) <= 1 && ( y3 - y1 ) <= 1)
			return;

		int largeurRandom = ( int )( MAT_HEIGHT_CUBES / ( prof*_FacteurGeneration ) );
		if (largeurRandom == 0)
			largeurRandom = 1;

		if (profMax >= 0 && prof >= profMax)
		{
			Log::log(Log::ENGINE_INFO, ( "End of generation at prof " + toString(prof) ).c_str());
			return;
		}

		//On se met au milieu des deux coins du haut
		int xa = ( x1 + x2 ) / 2;
		int ya = ( y1 + y2 ) / 2;
		int heighta = ( _MatriceHeights[x1][y1] + _MatriceHeights[x2][y2] ) / 2;
		if (( x2 - x1 )>1)
		{
			heighta += ( rand() % largeurRandom ) - ( largeurRandom / 2 );
			load_pile(xa, ya, heighta);
		}
		else
			heighta = _MatriceHeights[xa][ya];

		//Au milieu des deux coins de droite
		int xb = ( x2 + x3 ) / 2;
		int yb = ( y2 + y3 ) / 2;
		int heightb = ( _MatriceHeights[x2][y2] + _MatriceHeights[x3][y3] ) / 2;
		if (( y3 - y2 )>1)
		{
			heightb += ( rand() % largeurRandom ) - ( largeurRandom / 2 );
			load_pile(xb, yb, heightb);
		}
		else
			heightb = _MatriceHeights[xb][yb];

		//Au milieu des deux coins du bas
		int xc = ( x3 + x4 ) / 2;
		int yc = ( y3 + y4 ) / 2;
		int heightc = ( _MatriceHeights[x3][y3] + _MatriceHeights[x4][y4] ) / 2;
		heightc += ( rand() % largeurRandom ) - ( largeurRandom / 2 );
		if (( x3 - x4 )>1)
		{
			load_pile(xc, yc, heightc);
		}
		else
			heightc = _MatriceHeights[xc][yc];

		//Au milieu des deux coins de gauche
		int xd = ( x4 + x1 ) / 2;
		int yd = ( y4 + y1 ) / 2;
		int heightd = ( _MatriceHeights[x4][y4] + _MatriceHeights[x1][y1] ) / 2;
		heightd += ( rand() % largeurRandom ) - ( largeurRandom / 2 );
		if (( y3 - y1 )>1)
		{
			load_pile(xd, yd, heightd);
		}
		else
			heightd = _MatriceHeights[xd][yd];

		//Au milieu milieu
		int xe = xa;
		int ye = yb;
		if (( x3 - x1 )>1 && ( y3 - y1 )>1)
		{
			int heighte = ( heighta + heightb + heightc + heightd ) / 4;
			heighte += ( rand() % largeurRandom ) - ( largeurRandom / 2 );
			load_pile(xe, ye, heighte);
		}

		//On genere les 4 nouveaux quads

		generate_piles(x1, y1, xa, ya, xe, ye, xd, yd, prof + 1, profMax);
		generate_piles(xa, ya, x2, y2, xb, yb, xe, ye, prof + 1, profMax);
		generate_piles(xe, ye, xb, yb, x3, y3, xc, yc, prof + 1, profMax);
		generate_piles(xd, yd, xe, ye, xc, yc, x4, y4, prof + 1, profMax);
	}
	/*
	//Creation du monde entier, en utilisant le mouvement brownien fractionnaire
	void generate_piles(int x1, int y1,
		int x2, int y2,
		int x3, int y3,
		int x4, int y4, int prof, int profMax = -1)
	{
		if (x3 - x1 <= 1 && y3 - y1 <= 1)
			return;

		int leftHeapX = ( x2 + x1 ) / 2;
		int leftHeapY = y2;
		int bottomHeapX = x2;
		int bottomHeapY = ( y2 + y3 ) / 2;
		int rightHeapX = ( x3 + x4 ) / 2;
		int rightHeapY = y3;
		int topHeapX = x4;
		int topHeapY = ( y4 + y1 ) / 2;

		int middleHeapX = leftHeapX;
		int middleHeapY = topHeapY;

		int leftHeapHeight = newHeight(_MatriceHeights[x1][y1], _MatriceHeights[x2][y2], prof);
		int bottomHeapHeight = newHeight(_MatriceHeights[x2][y2], _MatriceHeights[x3][y3], prof);
		int rightHeapHeight = newHeight(_MatriceHeights[x3][y3], _MatriceHeights[x4][y4], prof);
		int topHeapHeight = newHeight(_MatriceHeights[x4][y4], _MatriceHeights[x1][y1], prof);
		int middleHeapHeight = newHeight(_MatriceHeights[x4][y4], _MatriceHeights[x1][y1], prof);

		int height = ( _MatriceHeights[leftHeapX][leftHeapY] +
			_MatriceHeights[bottomHeapX][bottomHeapY] +
			_MatriceHeights[rightHeapX][rightHeapY] +
			_MatriceHeights[topHeapX][topHeapY] ) / 4;
		int random = rand() % ( prof - -prof + 1 ) + -prof;

		load_pile(topHeapX, topHeapY, topHeapHeight);
		load_pile(rightHeapX, rightHeapY, rightHeapHeight);
		load_pile(bottomHeapX, bottomHeapY, bottomHeapHeight);
		load_pile(leftHeapX, leftHeapY, leftHeapHeight);
		load_pile(middleHeapX, middleHeapY, middleHeapHeight);

		generate_piles(x1, y1,
			leftHeapX, leftHeapY,
			middleHeapX, middleHeapY,
			topHeapX, topHeapY, prof, profMax);
		generate_piles(leftHeapX, leftHeapY,
			x2, y2,
			bottomHeapX, bottomHeapY,
			middleHeapX, middleHeapY, prof, profMax);
		generate_piles(middleHeapX, middleHeapY,
			bottomHeapX, bottomHeapY,
			x3, y3,
			rightHeapX, rightHeapY, prof, profMax);
		generate_piles(topHeapX, topHeapY,
			middleHeapX, middleHeapY,
			rightHeapX, rightHeapY,
			x4, y4, prof, profMax);
	}
	*/
	int newHeight(int height1, int height2, int prof)
	{
		int height = ( height1 + height2 ) / 2;
		int random = rand() % ( prof - -prof + 1 ) + -prof;
		return height + random;
	}


	void lisse(void)
	{
		int sizeWidow = 4;
		memset(_MatriceHeightsTmp, 0x00, sizeof(int)*MAT_SIZE_CUBES*MAT_SIZE_CUBES);
		for (int x = 0; x<MAT_SIZE_CUBES; x++)
		{
			for (int y = 0; y<MAT_SIZE_CUBES; y++)
			{
				//on moyenne sur une distance
				int nb = 0;
				for (int i = ( x - sizeWidow < 0 ? 0 : x - sizeWidow );
					i < ( x + sizeWidow >= MAT_SIZE_CUBES ? MAT_SIZE_CUBES - 1 : x + sizeWidow ); i++)
				{
					for (int j = ( y - sizeWidow < 0 ? 0 : y - sizeWidow );
						j <( y + sizeWidow >= MAT_SIZE_CUBES ? MAT_SIZE_CUBES - 1 : y + sizeWidow ); j++)
					{
						_MatriceHeightsTmp[x][y] += _MatriceHeights[i][j];
						nb++;
					}
				}
				if (nb)
					_MatriceHeightsTmp[x][y] /= nb;
			}
		}

		//On reset les piles
		for (int x = 0; x<MAT_SIZE_CUBES; x++)
		{
			for (int y = 0; y<MAT_SIZE_CUBES; y++)
			{
				load_pile(x, y, _MatriceHeightsTmp[x][y], false);
			}
		}


	}

	void genPerlin()
	{
		NYPerlin* perlin = new NYPerlin();
		
		for (int x = 0; x<MAT_SIZE_CUBES; x++)
			for (int y = 0; y<MAT_SIZE_CUBES; y++)
				for (int z = 0; z < MAT_HEIGHT_CUBES; z++)
				{
					NYCube* cube = getCube(x, y, z);

					float modif = 0.04;

					float sample = perlin->sample(modif*x, modif*y, modif*z);
					//sample /= pow(( float )z / ( float )MAT_HEIGHT_CUBES, TweakA) / TweakB + TweakC;

					if (sample < 0.5f)
					{
						cube->_Type = CUBE_AIR;
						cube->_Draw = false;
					}
					else
					{
						cube->_Type = CUBE_TERRE;
						cube->_Draw = true;
					}
				}

	}

	


	void init_world(int profmax = -1)
	{
		_cprintf("Creation du monde %f \n",_FacteurGeneration);

		srand(time(NULL));

		//Reset du monde
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
					_Chunks[x][y][z]->reset();
		memset(_MatriceHeights,0x00,MAT_SIZE_CUBES*MAT_SIZE_CUBES*sizeof(int));

		//On charge les 4 coins
		
		/*
		load_pile(0, 0, MAT_HEIGHT_CUBES / 2);
		load_pile(MAT_SIZE_CUBES - 1, 0, MAT_HEIGHT_CUBES / 2);
		load_pile(MAT_SIZE_CUBES - 1, MAT_SIZE_CUBES - 1, MAT_HEIGHT_CUBES / 2);
		load_pile(0, MAT_SIZE_CUBES - 1, MAT_HEIGHT_CUBES / 2);
		*/
		
		load_pile(0,0, rand() % ( MAT_HEIGHT_CUBES ));
		load_pile(MAT_SIZE_CUBES-1,0, rand() % ( MAT_HEIGHT_CUBES ));
		load_pile(MAT_SIZE_CUBES-1,MAT_SIZE_CUBES-1, rand() % ( MAT_HEIGHT_CUBES ));
		load_pile(0,MAT_SIZE_CUBES-1, rand() % ( MAT_HEIGHT_CUBES ));
		

		//On génère a partir des 4 coins
		
		generate_piles(0, 0,
			MAT_SIZE_CUBES - 1, 0,
			MAT_SIZE_CUBES - 1, MAT_SIZE_CUBES - 1,
			0, MAT_SIZE_CUBES - 1, 1, profmax);

		lisse();
	    

		//genPerlin();
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
					_Chunks[x][y][z]->disableHiddenCubes();

		add_world_to_vbo();
	}

	NYCube * pick(NYVert3Df  pos, NYVert3Df  dir, NYPoint3D * point)
	{
		return NULL;
	}

	//Boites de collisions plus petites que deux cubes
	NYAxis getMinCol(NYVert3Df pos, NYVert3Df dir, float width, float height, float & valueColMin, bool oneShot)
	{
		int x = ( int )( pos.X / NYCube::CUBE_SIZE );
		int y = ( int )( pos.Y / NYCube::CUBE_SIZE );
		int z = ( int )( pos.Z / NYCube::CUBE_SIZE );

		int xNext = ( int )( ( pos.X + width / 2.0f ) / NYCube::CUBE_SIZE );
		int yNext = ( int )( ( pos.Y + width / 2.0f ) / NYCube::CUBE_SIZE );
		int zNext = ( int )( ( pos.Z + height / 2.0f ) / NYCube::CUBE_SIZE );

		int xPrev = ( int )( ( pos.X - width / 2.0f ) / NYCube::CUBE_SIZE );
		int yPrev = ( int )( ( pos.Y - width / 2.0f ) / NYCube::CUBE_SIZE );
		int zPrev = ( int )( ( pos.Z - height / 2.0f ) / NYCube::CUBE_SIZE );

		if (x < 0)	x = 0;
		if (y < 0)	y = 0;
		if (z < 0)	z = 0;

		if (xPrev < 0)	xPrev = 0;
		if (yPrev < 0)	yPrev = 0;
		if (zPrev < 0)	zPrev = 0;

		if (xNext < 0)	xNext = 0;
		if (yNext < 0)	yNext = 0;
		if (zNext < 0)	zNext = 0;

		if (x >= MAT_SIZE_CUBES)	x = MAT_SIZE_CUBES - 1;
		if (y >= MAT_SIZE_CUBES)	y = MAT_SIZE_CUBES - 1;
		if (z >= MAT_HEIGHT_CUBES)	z = MAT_HEIGHT_CUBES - 1;

		if (xPrev >= MAT_SIZE_CUBES)	xPrev = MAT_SIZE_CUBES - 1;
		if (yPrev >= MAT_SIZE_CUBES)	yPrev = MAT_SIZE_CUBES - 1;
		if (zPrev >= MAT_HEIGHT_CUBES)	zPrev = MAT_HEIGHT_CUBES - 1;

		if (xNext >= MAT_SIZE_CUBES)	xNext = MAT_SIZE_CUBES - 1;
		if (yNext >= MAT_SIZE_CUBES)	yNext = MAT_SIZE_CUBES - 1;
		if (zNext >= MAT_HEIGHT_CUBES)	zNext = MAT_HEIGHT_CUBES - 1;

		//On fait chaque axe
		NYAxis axis = 0x00;
		valueColMin = oneShot ? 0.5 : 10000.0f;
		float seuil = 0.00001;
		float prodScalMin = 1.0f;
		if (dir.getMagnitude() > 1)
			dir.normalize();

		//On verif tout les 4 angles de gauche
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!( getCube(xPrev + 1, yPrev, zPrev)->isSolid() ||
				getCube(xPrev + 1, yPrev, zNext)->isSolid() ||
				getCube(xPrev + 1, yNext, zPrev)->isSolid() ||
				getCube(xPrev + 1, yNext, zNext)->isSolid() ) || !oneShot)
			{
				float depassement = ( ( xPrev + 1 ) * NYCube::CUBE_SIZE ) - ( pos.X - width / 2.0f );
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_X;
					}
			}
		}

		//float depassementx2 = (xNext * NYCube::CUBE_SIZE) - (pos.X + width / 2.0f);

		//On verif tout les 4 angles de droite
		if (getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!( getCube(xNext - 1, yPrev, zPrev)->isSolid() ||
				getCube(xNext - 1, yPrev, zNext)->isSolid() ||
				getCube(xNext - 1, yNext, zPrev)->isSolid() ||
				getCube(xNext - 1, yNext, zNext)->isSolid() ) || !oneShot)
			{
				float depassement = ( xNext * NYCube::CUBE_SIZE ) - ( pos.X + width / 2.0f );
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_X;
					}
			}
		}

		//float depassementy1 = (yNext * NYCube::CUBE_SIZE) - (pos.Y + width / 2.0f);

		//On verif tout les 4 angles de devant
		if (getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!( getCube(xPrev, yNext - 1, zPrev)->isSolid() ||
				getCube(xPrev, yNext - 1, zNext)->isSolid() ||
				getCube(xNext, yNext - 1, zPrev)->isSolid() ||
				getCube(xNext, yNext - 1, zNext)->isSolid() ) || !oneShot)
			{
				float depassement = ( yNext * NYCube::CUBE_SIZE ) - ( pos.Y + width / 2.0f );
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_Y;
					}
			}
		}

		//float depassementy2 = ((yPrev + 1) * NYCube::CUBE_SIZE) - (pos.Y - width / 2.0f);

		//On verif tout les 4 angles de derriere
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!( getCube(xPrev, yPrev + 1, zPrev)->isSolid() ||
				getCube(xPrev, yPrev + 1, zNext)->isSolid() ||
				getCube(xNext, yPrev + 1, zPrev)->isSolid() ||
				getCube(xNext, yPrev + 1, zNext)->isSolid() ) || !oneShot)
			{
				float depassement = ( ( yPrev + 1 ) * NYCube::CUBE_SIZE ) - ( pos.Y - width / 2.0f );
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_Y;
					}
			}
		}

		//On verif tout les 4 angles du haut
		if (getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!( getCube(xPrev, yPrev, zNext - 1)->isSolid() ||
				getCube(xPrev, yNext, zNext - 1)->isSolid() ||
				getCube(xNext, yPrev, zNext - 1)->isSolid() ||
				getCube(xNext, yNext, zNext - 1)->isSolid() ) || !oneShot)
			{
				float depassement = ( zNext * NYCube::CUBE_SIZE ) - ( pos.Z + height / 2.0f );
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_Z;
					}
			}
		}

		//On verif tout les 4 angles du bas
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!( getCube(xPrev, yPrev, zPrev + 1)->isSolid() ||
				getCube(xPrev, yNext, zPrev + 1)->isSolid() ||
				getCube(xNext, yPrev, zPrev + 1)->isSolid() ||
				getCube(xNext, yNext, zPrev + 1)->isSolid() ) || !oneShot)
			{
				float depassement = ( ( zPrev + 1 ) * NYCube::CUBE_SIZE ) - ( pos.Z - height / 2.0f );
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_Z;
					}
			}
		}
		return axis;
	}


	void render_world_vbo(void)
	{
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
				{
					glPushMatrix();
					glTranslatef((float)(x*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE),(float)(y*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE),(float)(z*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE));
					_Chunks[x][y][z]->render();	
					glPopMatrix();
				}
	}

	void add_world_to_vbo(void)
	{
		int totalNbVertices = 0;
		
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
				{
					_Chunks[x][y][z]->toVbo();
					totalNbVertices += _Chunks[x][y][z]->_NbVertices;
				}

		Log::log(Log::ENGINE_INFO,(toString(totalNbVertices) + " vertices in VBO").c_str());
	}

	void render_world_old_school(void)
	{

		//Flemme des materials
		glEnable(GL_COLOR_MATERIAL);

		//Pour tous les cubes de notre matrice
		for (int x = 0; x<MAT_SIZE_CUBES; x++)
		{
			for (int y = 0; y<MAT_SIZE_CUBES; y++)
			{
				for (int z = 0; z<MAT_HEIGHT_CUBES; z++)
				{
					//On recup le cube (se charge d'aller chercher dans le bon chunk)
					NYCube * cube = getCube(x, y, z);

					//On le dessine en fonction de son type
					if (cube->_Draw && cube->_Type != CUBE_AIR)
					{
						switch (cube->_Type)
						{
						case CUBE_TERRE:
							glColor3f(101.0f / 255.0f, 74.0f / 255.0f, 0.0f / 255.0f);
							break;
						case CUBE_HERBE:
							glColor3f(1.0f / 255.0f, 112.0f / 255.0f, 12.0f / 255.0f);
							break;
						case CUBE_EAU:
							glColor3f(0.0f / 255.0f, 48.0f / 255.0f, 255.0f / 255.0f);
							break;
						}

						//On se deplace pour chaque cube... bcp de push et pop
						glPushMatrix();
						glTranslated(x*NYCube::CUBE_SIZE, y*NYCube::CUBE_SIZE, z*NYCube::CUBE_SIZE);
						glutSolidCube(NYCube::CUBE_SIZE);
						glPopMatrix();
					}
				}
			}
		}
		glDisable(GL_COLOR_MATERIAL);
	}	
};



#endif