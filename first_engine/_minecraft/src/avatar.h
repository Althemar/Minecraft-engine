#ifndef __AVATAR__
#define __AVATAR__

#include "engine/utils/types_3d.h"
#include "engine/render/camera.h"
#include "world.h"

class NYAvatar
{
	public :
		NYVert3Df Position;
		NYVert3Df Speed;

		NYVert3Df MoveDir;
		bool Move;
		bool Jump;
		float Height;
		float Width;
		bool avance;
		bool recule;
		bool gauche;
		bool droite;
		bool Standing;
		

		NYCamera * Cam;
		NYWorld * World;

		float moveSpeed;
		float speedMax;
		float fallSpeed;
		float jumpSpeed;
		float jumpHeight;

		bool canJump;

		NYAvatar(NYCamera * cam,NYWorld * world)
		{
			Position = NYVert3Df(0,0,0);
			Height = 25;
			Width = 3;
			moveSpeed = 1000;
			fallSpeed = -50;
			jumpSpeed = 50;
			Cam = cam;
			avance = false;
			recule = false;
			gauche = false;
			droite = false;
			Standing = false;
			Jump = false;
			canJump = false;
			World = world;
		}


		void render(void)
		{
			glutSolidCube(Width/2);
		}

		void jump()
		{
			if (!canJump)
				return;
			Speed.Z += jumpSpeed;
			Jump = true;
		}

		NYVert3Df pickingRay()
		{
			NYVert3Df origin = Cam->_Direction;
			NYCube* cube;
			NYVert3Df cubePosition = (Cam->_Position + origin) / NYCube::CUBE_SIZE;
			cube = World->getCube(cubePosition.X, cubePosition.Y, cubePosition.Z);
			while (cube->_Type == NYCubeType::CUBE_AIR && origin.getSize() < 50)
			{
				cubePosition = ( Cam->_Position + origin ) / NYCube::CUBE_SIZE;
				cube = World->getCube(cubePosition.X, cubePosition.Y, cubePosition.Z);
				origin += Cam->_Direction;
			}
			return cubePosition;
		}

		void removeCube()
		{
			NYVert3Df origin = Cam->_Direction;
			NYCube* cube;
			NYVert3Df cubePosition = ( Cam->_Position + origin ) / NYCube::CUBE_SIZE;
			cube = World->getCube(cubePosition.X, cubePosition.Y, cubePosition.Z);
			while (cube->_Type == NYCubeType::CUBE_AIR && origin.getSize() < 50)
			{
				cubePosition = ( Cam->_Position + origin ) / NYCube::CUBE_SIZE;
				cube = World->getCube(cubePosition.X, cubePosition.Y, cubePosition.Z);
				origin += Cam->_Direction;
			}
			if (cube->_Type != NYCubeType::CUBE_AIR && cube != NULL)
			{
				cube->_Type = NYCubeType::CUBE_AIR;
				cube->_Draw = false;
				World->updateCube(cubePosition.X, cubePosition.Y, cubePosition.Z);
			}
		}

		void addCube()
		{
			NYVert3Df origin = Cam->_Direction;
			NYCube* cube;
			NYVert3Df cubePosition = ( Cam->_Position + origin ) / NYCube::CUBE_SIZE;
			cube = World->getCube(cubePosition.X, cubePosition.Y, cubePosition.Z);
			while (cube->_Type == NYCubeType::CUBE_AIR && origin.getSize() < 50)
			{
				cubePosition = ( Cam->_Position + origin ) / NYCube::CUBE_SIZE;
				cube = World->getCube(cubePosition.X, cubePosition.Y, cubePosition.Z);
				origin += Cam->_Direction;
			}
			if (cube->_Type != NYCubeType::CUBE_AIR && cube != NULL)
			{
				// Modify x, y, or z
				
				NYCube* newCube = World->getCube(cubePosition.X, cubePosition.Y, cubePosition.Z + 1);
				newCube->_Draw = true;
				newCube->_Type = NYCubeType::CUBE_TERRE;
				World->updateCube(cubePosition.X, cubePosition.Y, cubePosition.Z + 1);
			}

		}

		void update(float elapsed)
		{
			elapsed = max(elapsed, 1.f / 60.f);

			// Deplacement
			NYVert3Df movementForce;
			if (avance)
				movementForce += Cam->_Direction;
			if (recule)
				movementForce -= Cam->_Direction;
			if (gauche)
				movementForce -= Cam->_NormVec;
			if (droite)
				movementForce += Cam->_NormVec;
			movementForce.Z = 0;
			movementForce *= moveSpeed;

			
			Speed += movementForce * elapsed;
			
			

			NYVert3Df verticalForce;
			verticalForce = NYVert3Df(0.f, 0.f, -80.f);
			Speed.Z += verticalForce.Z * elapsed;


			if (Jump && Speed.Z == 0)
			{
				Jump = false;
			}

			// Damping
			float zSpeed = Speed.Z;
			Speed -= Speed * 15 * elapsed;
			Speed.Z = zSpeed;
			
			NYVert3Df nextPosition = Position + Speed * elapsed;

			bool isOnGround = false;

			// Check collision
			for (int i = 0; i < 6; i++)
			{
				float depassement = 0.f;
				NYAxis axis = World->getMinCol(nextPosition, Speed, Width, Height, depassement, false);

				switch (axis)
				{
				case NY_AXIS_X:
					if (Speed.X < 0)
						nextPosition.X += depassement;
					else if (Speed.X > 0)
						nextPosition.X -= depassement;
					Speed.X = 0;
					break;
				case NY_AXIS_Y:
					if (Speed.Y < 0)
						nextPosition.Y += depassement;
					else if (Speed.Y > 0)
						nextPosition.Y -= depassement;
					Speed.Y = 0;
					break;
				case NY_AXIS_Z:
					isOnGround = true;
					if (Speed.Z < 0)
						nextPosition.Z += depassement;
					else if (Speed.Z > 0)
						nextPosition.Z -= depassement;
					Speed.Z = 0;
					if (Jump)
						Jump = false;
					break;
				}
			}
			if (!isOnGround)
				canJump = false;
			else
				canJump = true;

			
			Position = nextPosition;
			Cam->moveTo(Position);
		}
		
	
};

#endif