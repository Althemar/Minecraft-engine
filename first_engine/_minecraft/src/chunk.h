#pragma once

#include "engine/render/renderer.h"
#include "cube.h"

/**
  * On utilise des chunks pour que si on modifie juste un cube, on ait pas
  * besoin de recharger toute la carte dans le buffer, mais juste le chunk en question
  */
class NYChunk
{
	public :

		static const int CHUNK_SIZE = 16; ///< Taille d'un chunk en nombre de cubes (n*n*n)
		NYCube _Cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; ///< Cubes contenus dans le chunk

		GLuint _BufWorld; ///< Identifiant du VBO pour le monde
		
		static float _WorldVert[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3*4*6]; ///< Buffer pour les sommets
		static float _WorldCols[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3*4*6]; ///< Buffer pour les couleurs
		static float _WorldNorm[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3*4*6]; ///< Buffer pour les normales

		static const int SIZE_VERTICE = 3 * sizeof(float); ///< Taille en octets d'un vertex dans le VBO
		static const int SIZE_COLOR = 3 * sizeof(float);  ///< Taille d'une couleur dans le VBO
		static const int SIZE_NORMAL = 3 * sizeof(float);  ///< Taille d'une normale dans le VBO
		
		int _NbVertices; ///< Nombre de vertices dans le VBO (on ne met que les faces visibles)

		const float s_triangles[108] =
		{
			0.f, 0.f, 1.f,    0.f, 1.f, 0.f,     0.f, 0.f, 0.f,     0.f, 0.f, 1.0f,    0.f, 1.f, 1.f,    0.f, 1.f, 0.f, /* Face 1 */
			0.f, 1.f, 1.f,    1.f, 1.f, 0.f,     0.f, 1.f, 0.f,     0.f, 1.f, 1.0f,    1.f, 1.f, 1.f,    1.f, 1.f, 0.f, /* Face 2 */
			1.f, 1.f, 1.f,    1.f, 0.f, 0.f,     1.f, 1.f, 0.f,     1.f, 1.f, 1.0f,    1.f, 0.f, 1.f,    1.f, 0.f, 0.f, /* Face 3 */
			1.f, 0.f, 1.f,    0.f, 0.f, 0.f,     1.f, 0.f, 0.f,     1.f, 0.f, 1.0f,    0.f, 0.f, 1.f,    0.f, 0.f, 0.f, /* Face 4 */
			0.f, 0.f, 1.f,    1.f, 0.f, 1.f,     1.f, 1.f, 1.f,     1.f, 1.f, 1.0f,    0.f, 1.f, 1.f,    0.f, 0.f, 1.f, /* Face 5 */
			1.f, 1.f, 0.f,    1.f, 0.f, 0.f,     0.f, 0.f, 0.f,     0.f, 0.f, 0.f,     0.f, 1.f, 0.f,    1.f, 1.f, 0.f  /* Face 5 */
		};

		const float s_normals[108] =
		{
			-1.0f, 0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,
			0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,
			0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
		};

		NYChunk * Voisins[6];
		
		NYChunk()
		{
			_NbVertices = 0;
			_BufWorld = 0;
			memset(Voisins,0x00,sizeof(void*) * 6);
		}

		void setVoisins(NYChunk * xprev, NYChunk * xnext,NYChunk * yprev,NYChunk * ynext,NYChunk * zprev,NYChunk * znext)
		{
			Voisins[0] = xprev;
			Voisins[1] = xnext;
			Voisins[2] = yprev;
			Voisins[3] = ynext;
			Voisins[4] = zprev;
			Voisins[5] = znext;
		}

		/**
		  * Raz de l'état des cubes (a draw = false)
		  */
		void reset(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z]._Draw = true;
						_Cubes[x][y][z]._Type = CUBE_AIR;
					}
		}

		int fillWorld()
		{
			float * ptVert = _WorldVert;
			float * ptCols = _WorldCols;
			float * ptNorm = _WorldNorm;
			int vertices = 0;
			for (int x = 0; x < CHUNK_SIZE; x++)
				for (int y = 0; y < CHUNK_SIZE; y++)
					for (int z = 0; z < CHUNK_SIZE; z++)
					{
						NYCube* cube = &( _Cubes[x][y][z] );
						if (cube->_Draw && cube->isSolid())
						{
							float R, G, B;
							if (cube->_Type == CUBE_EAU)
							{
								R = 0.0f / 255.0f;
								G = 48.0f / 255.0f;
								B = 255.0f / 255.0;
							}
							else if (cube->_Type == CUBE_HERBE)
							{
								R = 1.f / 255.0f;
								G = 112.0f / 255.0f;
								B = 12.0f / 255.0;
							}
							else if (cube->_Type == CUBE_TERRE)
							{
								R = 101.0f / 255.0f;
								G = 74.0f / 255.0f;
								B = 0.0f / 255.0;
							}

							for (int i = 0; i < 108; i += 3)
							{
								*ptVert = NYCube::CUBE_SIZE * x + ( s_triangles[i] * NYCube::CUBE_SIZE );
								ptVert++;
								*ptVert = NYCube::CUBE_SIZE * y + ( s_triangles[i + 1] * NYCube::CUBE_SIZE );
								ptVert++;
								*ptVert = NYCube::CUBE_SIZE * z + ( s_triangles[i + 2] * NYCube::CUBE_SIZE );
								ptVert++;
								vertices += 3;

								*ptCols = R + ( rand() % 100 ) / 2000.0f;
								ptCols++;
								*ptCols = G + ( rand() % 100 ) / 2000.0f;
								ptCols++;
								*ptCols = B + ( rand() % 100 ) / 2000.0f;
								ptCols++;

								*ptNorm = s_normals[i] + x;
								ptNorm++;
								*ptNorm = s_normals[i + 1] + y;
								ptNorm++;
								*ptNorm = s_normals[i + 2] + z;
								ptNorm++;
							}
						}
					}

			return vertices;
		}

		
		/*
		//On met le chunk ddans son VBO
		void toVbo(void)
		{
			if (_BufWorld != 0)
				glDeleteBuffers(1, &_BufWorld);

			_NbVertices = fillWorld();

			glGenBuffers(1, &_BufWorld);


			glBindBuffer(GL_ARRAY_BUFFER, _BufWorld);


			glBufferData(GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE +
				_NbVertices * SIZE_COLOR +
				_NbVertices * SIZE_VERTICE,
				NULL,
				GL_STREAM_DRAW);

			//On copie les vertices
			glBufferSubData(GL_ARRAY_BUFFER,
				0,
				_NbVertices * SIZE_VERTICE,
				_WorldVert);

			//On copie les couleurs
			
			glBufferSubData(GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE,
				_NbVertices * SIZE_COLOR,
				_WorldCols);

			//On copie les normales
			glBufferSubData(GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE +
				_NbVertices * SIZE_COLOR,
				_NbVertices * SIZE_NORMAL,
				_WorldNorm);

			//On debind le buffer pour eviter une modif accidentelle par le reste du code
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		*/
		void render(void)
		{
			glEnable(GL_COLOR_MATERIAL);
			glEnable(GL_LIGHTING);

			//On bind le buuffer
			glBindBuffer(GL_ARRAY_BUFFER, _BufWorld);
			NYRenderer::checkGlError("glBindBuffer");

			//On active les datas que contiennent le VBO
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);

			//On place les pointeurs sur les datas, aux bons offsets
			glVertexPointer(3, GL_FLOAT, 0, ( void* )( 0 ));
			glColorPointer(3, GL_FLOAT, 0, ( void* )( _NbVertices*SIZE_VERTICE ));
			glNormalPointer(GL_FLOAT, 0, ( void* )( _NbVertices*SIZE_VERTICE + _NbVertices*SIZE_COLOR ));
		
			//On demande le dessin
			//glDrawArrays(GL_TRIANGLES, 0, _NbVertices);
			glDrawArrays(GL_QUADS, 0, _NbVertices);

			//On cleane
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);
		    glDisableClientState(GL_NORMAL_ARRAY);
		}

		/**
		  * On verifie si le cube peut être vu
		  */
		bool test_hidden(int x, int y, int z)
		{
			NYCube * cubeXPrev = NULL; 
			NYCube * cubeXNext = NULL; 
			NYCube * cubeYPrev = NULL; 
			NYCube * cubeYNext = NULL; 
			NYCube * cubeZPrev = NULL; 
			NYCube * cubeZNext = NULL; 

			if(x == 0 && Voisins[0] != NULL)
				cubeXPrev = &(Voisins[0]->_Cubes[CHUNK_SIZE-1][y][z]);
			else if(x > 0)
				cubeXPrev = &(_Cubes[x-1][y][z]);

			if(x == CHUNK_SIZE-1 && Voisins[1] != NULL)
				cubeXNext = &(Voisins[1]->_Cubes[0][y][z]);
			else if(x < CHUNK_SIZE-1)
				cubeXNext = &(_Cubes[x+1][y][z]);

			if(y == 0 && Voisins[2] != NULL)
				cubeYPrev = &(Voisins[2]->_Cubes[x][CHUNK_SIZE-1][z]);
			else if(y > 0)
				cubeYPrev = &(_Cubes[x][y-1][z]);

			if(y == CHUNK_SIZE-1 && Voisins[3] != NULL)
				cubeYNext = &(Voisins[3]->_Cubes[x][0][z]);
			else if(y < CHUNK_SIZE-1)
				cubeYNext = &(_Cubes[x][y+1][z]);

			if(z == 0 && Voisins[4] != NULL)
				cubeZPrev = &(Voisins[4]->_Cubes[x][y][CHUNK_SIZE-1]);
			else if(z > 0)
				cubeZPrev = &(_Cubes[x][y][z-1]);

			if(z == CHUNK_SIZE-1 && Voisins[5] != NULL)
				cubeZNext = &(Voisins[5]->_Cubes[x][y][0]);
			else if(z < CHUNK_SIZE-1)
				cubeZNext = &(_Cubes[x][y][z+1]);

			if( cubeXPrev == NULL || cubeXNext == NULL ||
				cubeYPrev == NULL || cubeYNext == NULL ||
				cubeZPrev == NULL || cubeZNext == NULL )
				return false;

			if( cubeXPrev->isSolid() == true && //droite
				cubeXNext->isSolid() == true && //gauche
				cubeYPrev->isSolid() == true && //haut
				cubeYNext->isSolid() == true && //bas
				cubeZPrev->isSolid() == true && //devant
				cubeZNext->isSolid() == true )  //derriere
				return true;
			return false;
		}

		void disableHiddenCubes(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z]._Draw = true;
						if(test_hidden(x,y,z))
							_Cubes[x][y][z]._Draw = false;
					}
		}

		
		void toVbo(void)
		{
			//On utilise les buffers temporaires pour préparer nos datas
			float * ptVert = _WorldVert;
			float * ptCols = _WorldCols;
			float * ptNorm = _WorldNorm;
			_NbVertices = 0;

			//On parcourt tous nos cubes
			for (int x = 0; x<CHUNK_SIZE; x++)
			{
				for (int y = 0; y<CHUNK_SIZE; y++)
				{
					for (int z = 0; z<CHUNK_SIZE; z++)
					{
						if (_Cubes[x][y][z]._Draw && _Cubes[x][y][z]._Type != CUBE_AIR)
						{
							float color[3];

							switch (_Cubes[x][y][z]._Type)
							{
							case CUBE_TERRE:
								color[0] = 101.0f / 255.0f;
								color[1] = 74.0f / 255.0f;
								color[2] = 0.0f / 255.0f;
								break;
							case CUBE_HERBE:
								color[0] = 1.0f / 255.0f;
								color[1] = 112.0f / 255.0f;
								color[2] = 12.0f / 255.0f;
								break;
							case CUBE_EAU:
								color[0] = 0.0f / 255.0f;
								color[1] = 48.0f / 255.0f;
								color[2] = 255.0f / 255.0f;
								break;
							}

							//Position du cube (coin bas gauche face avant)
							float xPos = x*( float )NYCube::CUBE_SIZE;
							float yPos = y*( float )NYCube::CUBE_SIZE;
							float zPos = z*( float )NYCube::CUBE_SIZE;

							//Premier QUAD
							*ptVert = xPos; ptVert++;
							*ptVert = yPos; ptVert++;
							*ptVert = zPos; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = -1; ptNorm++;

							*ptVert = xPos; ptVert++;
							*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = zPos; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = -1; ptNorm++;

							*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = zPos; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = -1; ptNorm++;

							*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = yPos; ptVert++;
							*ptVert = zPos; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = -1; ptNorm++;

							//Second QUAD (droite)
							*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = yPos; ptVert++;
							*ptVert = zPos; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 1; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;

							*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = zPos; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 1; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;

							*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 1; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;

							*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = yPos; ptVert++;
							*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 1; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;

							//Troisieme QUAD (gauche)
							*ptVert = xPos; ptVert++;
							*ptVert = yPos; ptVert++;
							*ptVert = zPos; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = -1; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;

							*ptVert = xPos; ptVert++;
							*ptVert = yPos; ptVert++;
							*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = -1; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;

							*ptVert = xPos; ptVert++;
							*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = -1; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;

							*ptVert = xPos; ptVert++;
							*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = zPos; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = -1; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;

							//Quatrieme QUAD (Top)
							*ptVert = xPos; ptVert++;
							*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = zPos; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = 1; ptNorm++;
							*ptNorm = 0; ptNorm++;

							*ptVert = xPos; ptVert++;
							*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = 1; ptNorm++;
							*ptNorm = 0; ptNorm++;

							*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = 1; ptNorm++;
							*ptNorm = 0; ptNorm++;

							*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = zPos; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = 1; ptNorm++;
							*ptNorm = 0; ptNorm++;

							//Cinquieme QUAD (Bottom)
							*ptVert = xPos; ptVert++;
							*ptVert = yPos; ptVert++;
							*ptVert = zPos; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = -1; ptNorm++;
							*ptNorm = 0; ptNorm++;

							*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = yPos; ptVert++;
							*ptVert = zPos; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = -1; ptNorm++;
							*ptNorm = 0; ptNorm++;

							*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = yPos; ptVert++;
							*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = -1; ptNorm++;
							*ptNorm = 0; ptNorm++;

							*ptVert = xPos; ptVert++;
							*ptVert = yPos; ptVert++;
							*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = -1; ptNorm++;
							*ptNorm = 0; ptNorm++;

							//Sixieme QUAD (Fond)
							*ptVert = xPos; ptVert++;
							*ptVert = yPos; ptVert++;
							*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = 1; ptNorm++;

							*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = yPos; ptVert++;
							*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = 1; ptNorm++;

							*ptVert = xPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = 1; ptNorm++;

							*ptVert = xPos; ptVert++;
							*ptVert = yPos + NYCube::CUBE_SIZE; ptVert++;
							*ptVert = zPos + NYCube::CUBE_SIZE; ptVert++; _NbVertices++;

							*ptCols = color[0] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[1] + ( rand() % 100 ) / 2000.0f; ptCols++;
							*ptCols = color[2] + ( rand() % 100 ) / 2000.0f; ptCols++;

							*ptNorm = 0; ptNorm++;
							*ptNorm = 0; ptNorm++;
							*ptNorm = 1; ptNorm++;

						}
					}
				}
			}

			if (_BufWorld != 0)
				glDeleteBuffers(1, &_BufWorld);

			glGenBuffers(1, &_BufWorld);

			glBindBuffer(GL_ARRAY_BUFFER, _BufWorld);

			glBufferData(GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE +
				_NbVertices * SIZE_COLOR +
				_NbVertices * SIZE_NORMAL,
				NULL,
				GL_STREAM_DRAW);

			GLenum error = glGetError();

			glBufferSubData(GL_ARRAY_BUFFER,
				0,
				_NbVertices * SIZE_VERTICE,
				_WorldVert);

			error = glGetError();

			glBufferSubData(GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE,
				_NbVertices * SIZE_COLOR,
				_WorldCols);

			error = glGetError();

			glBufferSubData(GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE +
				_NbVertices * SIZE_COLOR,
				_NbVertices * SIZE_NORMAL,
				_WorldNorm);

			error = glGetError();

			glBindBuffer(GL_ARRAY_BUFFER, 0);

		}

		

};