#ifndef __BOID__
#define __BOID__

#include "engine/utils/types_3d.h"
#include "engine/utils/timer.h"
#include "world.h"
#include "avatar.h"

class MBoid
{
public:
	YVec3f Position;
	YVec3f Speed;
	YVec3f Dir;
	MWorld * McrftWorld;
	YVec3f TargetConst;
	YVec3f * Target;
	YVec3f * TargetSec;
	YVec3f Perturbation;

	bool Still;
	bool UseNavField;

	float DTarget;

	MBoid(MWorld * world);

	void render(GLuint shader, YVbo * VboCube);

	void renderDebug();
	void update(float elapsed);
};

class MBoidManager
{

public:

	MWorld * World;
	MAvatar * Avatar;
	float BoidsDodoElapsed;
	bool BoidsDodo;
	bool BoidsFreeFlight;
	YVec3f BoidsTarget;
	vector<MBoid*> Boids;

	MBoidManager(MWorld * world, MAvatar * avatar);

	void createBoids();

	void setNewTargetBoids();

	void goToNestBoids();

	void updateBoids(float elapsed);

	void render(GLuint shader, YVbo * vboCube);
};


#endif