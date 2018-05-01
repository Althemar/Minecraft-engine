#pragma once

#include "engine/render/renderer.h"
#include "engine/render/vbo.h"
#include "cube.h"

/**
  * On utilise des chunks pour que si on modifie juste un cube, on ait pas
  * besoin de recharger toute la carte dans le buffer, mais juste le chunk en question
  */
class MChunk
{
	public :

		static const int CHUNK_SIZE = 64; ///< Taille d'un chunk en nombre de cubes (n*n*n)
		MCube _Cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; ///< Cubes contenus dans le chunk

		YVbo * VboOpaque = NULL;
		YVbo * VboTransparent = NULL;

		MChunk * Voisins[6];

		int _XPos, _YPos, _ZPos; ///< Position du chunk dans le monde

		int _NbVertices_Opaque;
		int _NbVertices_Transparent;
		
		MChunk(int x, int y, int z)
		{
			memset(Voisins, 0x00, sizeof(void*)* 6);
			_XPos = x;
			_YPos = y;
			_ZPos = z;
			_NbVertices_Opaque = 0;
			_NbVertices_Transparent = 0;
		}

		void setVoisins(MChunk * xprev, MChunk * xnext, MChunk * yprev, MChunk * ynext, MChunk * zprev, MChunk * znext)
		{
			Voisins[0] = xprev;
			Voisins[1] = xnext;
			Voisins[2] = yprev;
			Voisins[3] = ynext;
			Voisins[4] = zprev;
			Voisins[5] = znext;
		}

		void reset(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z].setDraw(false);
						_Cubes[x][y][z].setType(MCube::CUBE_AIR);
					}
		}

		float get_cube_type(MCube* cube) {
			if (cube == NULL) {
				return MCube::MCubeType::CUBE_EAU;
			}
			else {
				return cube->getType();
			}
		}

		void get_surrounding_cubes(int x, int y, int z, MCube ** cubeXPrev, MCube ** cubeXNext,
			MCube ** cubeYPrev, MCube ** cubeYNext,
			MCube ** cubeZPrev, MCube ** cubeZNext)
		{

			*cubeXPrev = NULL;
			*cubeXNext = NULL;
			*cubeYPrev = NULL;
			*cubeYNext = NULL;
			*cubeZPrev = NULL;
			*cubeZNext = NULL;

			if (x == 0 && Voisins[0] != NULL)
				*cubeXPrev = &(Voisins[0]->_Cubes[CHUNK_SIZE - 1][y][z]);
			else if (x > 0)
				*cubeXPrev = &(_Cubes[x - 1][y][z]);

			if (x == CHUNK_SIZE - 1 && Voisins[1] != NULL)
				*cubeXNext = &(Voisins[1]->_Cubes[0][y][z]);
			else if (x < CHUNK_SIZE - 1)
				*cubeXNext = &(_Cubes[x + 1][y][z]);

			if (y == 0 && Voisins[2] != NULL)
				*cubeYPrev = &(Voisins[2]->_Cubes[x][CHUNK_SIZE - 1][z]);
			else if (y > 0)
				*cubeYPrev = &(_Cubes[x][y - 1][z]);

			if (y == CHUNK_SIZE - 1 && Voisins[3] != NULL)
				*cubeYNext = &(Voisins[3]->_Cubes[x][0][z]);
			else if (y < CHUNK_SIZE - 1)
				*cubeYNext = &(_Cubes[x][y + 1][z]);

			if (z == 0 && Voisins[4] != NULL)
				*cubeZPrev = &(Voisins[4]->_Cubes[x][y][CHUNK_SIZE - 1]);
			else if (z > 0)
				*cubeZPrev = &(_Cubes[x][y][z - 1]);

			if (z == CHUNK_SIZE - 1 && Voisins[5] != NULL)
				*cubeZNext = &(Voisins[5]->_Cubes[x][y][0]);
			else if (z < CHUNK_SIZE - 1)
				*cubeZNext = &(_Cubes[x][y][z + 1]);
		}

		// Renvoie en plus les cubes en diagonales sur le même plan horizontal
		void get_more_surrounding_cubes(int x, int y, int z, MCube ** cubeXPrev, MCube ** cubeXNext,
			MCube ** cubeYPrev, MCube ** cubeYNext,
			MCube ** cubeZPrev, MCube ** cubeZNext, 
			MCube ** cubeXNextYNext, MCube ** cubeXNextYPrev, MCube ** cubeXPrevYNext, MCube ** cubeXPrevYPrev
		)
		{
			*cubeXPrev = NULL;
			*cubeXNext = NULL;
			*cubeYPrev = NULL;
			*cubeYNext = NULL;
			*cubeZPrev = NULL;
			*cubeZNext = NULL;
			*cubeXNextYNext = NULL;
			*cubeXNextYPrev = NULL;
			*cubeXPrevYNext = NULL;
			*cubeXPrevYPrev = NULL;

			get_surrounding_cubes(x, y, z, cubeXPrev, cubeXNext, cubeYPrev, cubeYNext, cubeZPrev, cubeZPrev);

			if (x < CHUNK_SIZE - 1 && y < CHUNK_SIZE - 1)
				*cubeXNextYNext = &(_Cubes[x + 1][y + 1][z]);

			if (x < CHUNK_SIZE - 1 && y > 0)
				*cubeXNextYPrev = &(_Cubes[x + 1][y - 1][z]);

			if (x > 0 && y < CHUNK_SIZE - 1)
				*cubeXPrevYNext = &(_Cubes[x - 1][y + 1][z]);

			if (x > 0 && y > 0)
				*cubeXPrevYPrev = &(_Cubes[x - 1][y - 1][z]);
		}

		//On commence par le point en UV 0,0 et on tourne en CCW
		int addQuadToVbo(YVbo * vbo, int iVertice, YVec3f & a, YVec3f & b, YVec3f & c, YVec3f & d, float type, float typeVoisinGauche, float typeVoisinFace, float typeVoisinDroite, float typeVoisinGaucheFace, float typeVoisinDroiteFace) {
			YVec3f normal = (b - a).cross(d - a);
			normal.normalize();
			
			addVerticeToVbo(vbo, iVertice++, a, normal, YVec2f(0, 0), type, typeVoisinFace, typeVoisinDroiteFace, typeVoisinDroite);
			addVerticeToVbo(vbo, iVertice++, b, normal, YVec2f(1, 0), type, typeVoisinGauche, typeVoisinGaucheFace, typeVoisinFace);
			addVerticeToVbo(vbo, iVertice++, c, normal, YVec2f(1, 1), type, typeVoisinGauche, typeVoisinGaucheFace, typeVoisinFace);
			addVerticeToVbo(vbo, iVertice++, a, normal, YVec2f(0, 0), type, typeVoisinFace, typeVoisinDroiteFace, typeVoisinDroite);
			addVerticeToVbo(vbo, iVertice++, c, normal, YVec2f(1, 1), type, typeVoisinGauche, typeVoisinGaucheFace, typeVoisinFace);
			addVerticeToVbo(vbo, iVertice++, d, normal, YVec2f(0, 1), type, typeVoisinFace, typeVoisinDroiteFace, typeVoisinDroite);
		
			return 6;
		}

		// for face oriented down or up
		int addQuadToVbo(YVbo * vbo, int iVertice, YVec3f & a, YVec3f & b, YVec3f & c, YVec3f & d, float type, float typeXNext, float typeYNext, float typeXPrev, float typeYPrev, float typeXNextYNext, float typeXNextYPrev, float typeXPrevYNext, float typeXPrevYPrev) {
			YVec3f normal = (b - a).cross(d - a);
			normal.normalize();

			addVerticeToVbo(vbo, iVertice++, a, normal, YVec2f(0, 0), type, typeYNext, typeXNextYNext, typeXNext);
			addVerticeToVbo(vbo, iVertice++, b, normal, YVec2f(1, 0), type, typeXPrev, typeXPrevYNext, typeYNext);
			addVerticeToVbo(vbo, iVertice++, c, normal, YVec2f(1, 1), type, typeYPrev, typeXPrevYPrev, typeXPrev);
			addVerticeToVbo(vbo, iVertice++, a, normal, YVec2f(0, 0), type, typeYNext, typeXNextYNext, typeXNext);
			addVerticeToVbo(vbo, iVertice++, c, normal, YVec2f(1, 1), type, typeYPrev, typeXPrevYPrev, typeXPrev);
			addVerticeToVbo(vbo, iVertice++, d, normal, YVec2f(0, 1), type, typeXNext, typeXNextYPrev, typeYPrev);

			return 6;
		}

		void addVerticeToVbo(YVbo * vbo, int iVertice, YVec3f position, YVec3f normal, YVec2f uv, float type, float typeVoisinGauche, float typeVoisinOppose, float typeVoisinDroite) {
			vbo->setElementValue(0, iVertice, position.X, position.Y, position.Z);
			vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
			vbo->setElementValue(2, iVertice, uv.X, uv.Y);
			vbo->setElementValue(3, iVertice, type);
			vbo->setElementValue(4, iVertice, typeVoisinGauche);
			vbo->setElementValue(5, iVertice, typeVoisinOppose);
			vbo->setElementValue(6, iVertice, typeVoisinDroite);
		}

		int foreachVisibleTriangle(bool countOnly, int * nbVertOpaque, int * nbVertTransp, YVbo * VboOpaque, YVbo * VboTrasparent) {
			int type = 0;
			MCube * cube;
			int nbVertices = 0;
			int iVerticeOpaque = 0;
			int iVerticeTransp = 0;
			int * iVertice = &iVerticeOpaque;

			*nbVertOpaque = 0;
			*nbVertTransp = 0;
			//On parcourt tous nos cubes
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				for (int y = 0; y < CHUNK_SIZE; y++)
				{
					for (int z = 0; z < CHUNK_SIZE; z++)
					{
						cube = &(_Cubes[x][y][z]);
						type = cube->getType();

						if (cube->getDraw() && type != MCube::CUBE_AIR)
						{
							//Position du cube (coin bas gauche face avant)
							float xPos = x * (float)MCube::CUBE_SIZE;
							float yPos = y * (float)MCube::CUBE_SIZE;
							float zPos = z * (float)MCube::CUBE_SIZE;

							YVec3f a(xPos + MCube::CUBE_SIZE, yPos, zPos);
							YVec3f b(xPos + MCube::CUBE_SIZE, yPos + MCube::CUBE_SIZE, zPos);
							YVec3f c(xPos + MCube::CUBE_SIZE, yPos + MCube::CUBE_SIZE, zPos + MCube::CUBE_SIZE);
							YVec3f d(xPos + MCube::CUBE_SIZE, yPos, zPos + MCube::CUBE_SIZE);
							YVec3f e(xPos, yPos, zPos);
							YVec3f f(xPos, yPos + MCube::CUBE_SIZE, zPos);
							YVec3f g(xPos, yPos + MCube::CUBE_SIZE, zPos + MCube::CUBE_SIZE);
							YVec3f h(xPos, yPos, zPos + MCube::CUBE_SIZE);

							MCube * cubeXPrev = NULL;
							MCube * cubeXNext = NULL;
							MCube * cubeYPrev = NULL;
							MCube * cubeYNext = NULL;
							MCube * cubeZPrev = NULL;
							MCube * cubeZNext = NULL;
							MCube * cubeXNextYNext = NULL;
							MCube * cubeXNextYPrev = NULL;
							MCube * cubeXPrevYNext = NULL;
							MCube * cubeXPrevYPrev = NULL;
							/*
							get_surrounding_cubes(
								x, y, z,
								&cubeXPrev, &cubeXNext,
								&cubeYPrev, &cubeYNext,
								&cubeZPrev, &cubeZNext);
							*/
							
							get_more_surrounding_cubes(
								x, y, z,
								&cubeXPrev, &cubeXNext,
								&cubeYPrev, &cubeYNext,
								&cubeZPrev, &cubeZNext,
								&cubeXNextYNext, &cubeXNextYPrev, &cubeXPrevYNext, &cubeXPrevYPrev);
								
							
							float typeXNext = get_cube_type(cubeXNext);
							float typeYNext = get_cube_type(cubeYNext);
							float typeXPrev = get_cube_type(cubeXPrev);
							float typeYPrev = get_cube_type(cubeYPrev);
							float typeXNextYNext = get_cube_type(cubeXNextYNext);
							float typeXNextYPrev = get_cube_type(cubeXNextYPrev);
							float typeXPrevYNext = get_cube_type(cubeXPrevYNext);
							float typeXPrevYPrev = get_cube_type(cubeXPrevYPrev);

							iVertice = &iVerticeTransp;
							if (countOnly)
								iVertice = nbVertTransp;								
							YVbo * vbo = VboTransparent;
							if (cube->isOpaque()) {
								iVertice = &iVerticeOpaque;
								if (countOnly)
									iVertice = nbVertOpaque;
								vbo = VboOpaque;
							}
							
							
							

							//Premier QUAD (x+)
							if (cubeXNext == NULL ||
								(cube->isOpaque() && !cubeXNext->isOpaque()) || //Je suis un cube opaque et le cube a cote de moi est transparent
								(!cube->isOpaque() && cubeXNext->getType() == MCube::CUBE_AIR)) //Je suis un cube transparent et le cube a cote de moi est de l'air (on rend le transparent qu'au contact de l'air)
							{
								if (!countOnly)
									addQuadToVbo(vbo, *iVertice, a, b, c, d, type, typeYNext, typeXNext, typeYPrev, typeXNextYNext, typeXNextYPrev); //x+
								*iVertice += 6;
							}
							//Second QUAD (x-)
							if (cubeXPrev == NULL ||
								(cube->isOpaque() && !cubeXPrev->isOpaque()) || //Je suis un cube opaque et le cube a cote de moi est transparent
								(!cube->isOpaque() && cubeXPrev->getType() == MCube::CUBE_AIR)) //Je suis un cube transparent et le cube a cote de moi est de l'air (on rend le transparent qu'au contact de l'air)
							{
								if (!countOnly)
									addQuadToVbo(vbo, *iVertice, f, e, h, g, type, typeYPrev, typeXPrev, typeYNext, typeXPrevYPrev, typeXPrevYNext); //x-
								*iVertice += 6;
							}

							//Troisieme QUAD (y+)
							if (cubeYNext == NULL ||
								(cube->isOpaque() && !cubeYNext->isOpaque()) || //Je suis un cube opaque et le cube a cote de moi est transparent
								(!cube->isOpaque() && cubeYNext->getType() == MCube::CUBE_AIR)) //Je suis un cube transparent et le cube a cote de moi est de l'air (on rend le transparent qu'au contact de l'air)
							{
								if (!countOnly)
									addQuadToVbo(vbo, *iVertice, b, f, g, c, type, typeXPrev, typeYNext, typeXNext, typeXPrevYNext, typeXNextYNext); //y+
								*iVertice += 6;
							}

							//Quatrieme QUAD (y-)
							if (cubeYPrev == NULL ||
								(cube->isOpaque() && !cubeYPrev->isOpaque()) || //Je suis un cube opaque et le cube a cote de moi est transparent
								(!cube->isOpaque() && cubeYPrev->getType() == MCube::CUBE_AIR)) //Je suis un cube transparent et le cube a cote de moi est de l'air (on rend le transparent qu'au contact de l'air)
							{
								if (!countOnly)
									addQuadToVbo(vbo, *iVertice, e, a, d, h, type, typeXNext, typeYPrev, typeXPrev, typeXNextYPrev, typeXPrevYPrev); //y-
								*iVertice += 6;
							}
			
   						    //Cinquieme QUAD (z+) (on traite la condition != pour avoir les faces du haut quand on fera des vagues pour l'eau
							if (cubeZNext == NULL ||
								(cube->isOpaque() && !cubeZNext->isOpaque()) || //Je suis un cube opaque et le cube a cote de moi est transparent
								(!cube->isOpaque() && cubeZNext->getType() != type)) //Je suis un cube transparent et le cube a cote de moi est de l'air (on rend le transparent qu'au contact de l'air)
							{
								if (!countOnly)
									addQuadToVbo(vbo, *iVertice, c, g, h, d, type, typeXNext, typeYNext, typeXPrev, typeYPrev, typeXNextYNext, typeXNextYPrev, typeXPrevYNext, typeXPrevYPrev); //z+
								*iVertice += 6;
							}

	
							if (cubeZPrev == NULL ||
								(cube->isOpaque() && !cubeZPrev->isOpaque()) || //Je suis un cube opaque et le cube a cote de moi est transparent
								(!cube->isOpaque() && cubeZPrev->getType() == MCube::CUBE_AIR)) //Je suis un cube transparent et le cube a cote de moi est de l'air (on rend le transparent qu'au contact d'un cube !=)
							{
								if (!countOnly)
									addQuadToVbo(vbo, *iVertice, e, f, b, a, type, typeXNext, typeYNext, typeXPrev, typeYPrev, typeXNextYNext, typeXNextYPrev, typeXPrevYNext, typeXPrevYPrev); //z-
								*iVertice += 6;
							}
						}
					}
				}
			}

			return nbVertices;
		}

		void toVbo()
		{
			SAFEDELETE(VboOpaque);
			SAFEDELETE(VboTransparent);

			//On compte juste
			foreachVisibleTriangle(true, &_NbVertices_Opaque, &_NbVertices_Transparent,VboOpaque,VboTransparent);

			//Creation des vbo
			VboOpaque = new YVbo(7, _NbVertices_Opaque, YVbo::PACK_BY_ELEMENT_TYPE);
			VboTransparent = new YVbo(7, _NbVertices_Transparent, YVbo::PACK_BY_ELEMENT_TYPE);

			VboOpaque->setElementDescription(0, YVbo::Element(3)); //Sommet
			VboOpaque->setElementDescription(1, YVbo::Element(3)); //Normale
			VboOpaque->setElementDescription(2, YVbo::Element(2)); //Uv
			VboOpaque->setElementDescription(3, YVbo::Element(1)); //Type
			VboOpaque->setElementDescription(4, YVbo::Element(1)); //Type Voisin Gauche
			VboOpaque->setElementDescription(5, YVbo::Element(1)); //Type Voisin Opposé
			VboOpaque->setElementDescription(6, YVbo::Element(1)); //Type Vosiin Droite
			
			VboTransparent->setElementDescription(0, YVbo::Element(3)); //Sommet
			VboTransparent->setElementDescription(1, YVbo::Element(3)); //Normale
			VboTransparent->setElementDescription(2, YVbo::Element(2)); //Uv
			VboTransparent->setElementDescription(3, YVbo::Element(1)); //Type
			VboTransparent->setElementDescription(4, YVbo::Element(1)); //Type Voisin Gauche
			VboTransparent->setElementDescription(5, YVbo::Element(1)); //Type Voisin Opposé
			VboTransparent->setElementDescription(6, YVbo::Element(1)); //Type Vosiin Droite
			
			VboOpaque->createVboCpu();
			VboTransparent->createVboCpu();

			//On remplit les VBOs
			foreachVisibleTriangle(false, &_NbVertices_Opaque, &_NbVertices_Transparent, VboOpaque, VboTransparent);

			VboOpaque->createVboGpu();
			VboTransparent->createVboGpu();

			VboOpaque->deleteVboCpu();
			VboTransparent->deleteVboCpu();
		}


		void render(bool transparent,bool debug=true)
		{
			if (transparent)
				VboTransparent->render();
			else
				VboOpaque->render();
		}

		/**
		  * On verifie si le cube peut être vu
		  */
		bool test_hidden(int x, int y, int z)
		{
			MCube * cubeXPrev = NULL; 
			MCube * cubeXNext = NULL; 
			MCube * cubeYPrev = NULL; 
			MCube * cubeYNext = NULL; 
			MCube * cubeZPrev = NULL; 
			MCube * cubeZNext = NULL; 

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

			if (cubeXPrev->isOpaque() == true && //droite
				cubeXNext->isOpaque() == true && //gauche
				cubeYPrev->isOpaque() == true && //haut
				cubeYNext->isOpaque() == true && //bas
				cubeZPrev->isOpaque() == true && //devant
				cubeZNext->isOpaque() == true)  //derriere
				return true;
			return false;
		}

		void disableHiddenCubes(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z].setDraw(true);
						if(test_hidden(x,y,z))
							_Cubes[x][y][z].setDraw(false);
					}
		}


};