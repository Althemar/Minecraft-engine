#include "boid.h"

MBoid::MBoid(MWorld * world)
{
	Position = YVec3f((MWorld::MAT_SIZE_CUBES*MCube::CUBE_SIZE) / 2, (MWorld::MAT_SIZE_CUBES*MCube::CUBE_SIZE) / 2, (MWorld::MAT_HEIGHT_CUBES*MCube::CUBE_SIZE) * 2);
	McrftWorld = world;
	Target = NULL;
	TargetSec = NULL;
	Perturbation = YVec3f(randf(), randf(), randf()) * 5;
	DTarget = 0;
	Still = false;
	UseNavField = true;
}

void MBoid::render(GLuint shader, YVbo * VboCube)
{
	glPushMatrix();
	glTranslatef(Position.X, Position.Y, Position.Z);
	YVec3f axis = YVec3f(1, 0, 0).cross(Dir);
	float angle = acos(YVec3f(1, 0, 0).dot(Dir));
	glRotatef(angle / 6.28 * 360, axis.X, axis.Y, axis.Z);
	glScalef(0.2f, 0.2f, 0.2f);
	YRenderer::getInstance()->updateMatricesFromOgl(NULL,shader, false, false);
	YRenderer::getInstance()->sendMatricesToShader(YRenderer::CURRENT_SHADER);
	VboCube->render();
	glPopMatrix();
}

void MBoid::renderDebug() {
	if (Target != NULL) {
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3f(0, 1, 0);
		glVertex3f(Position.X, Position.Y, Position.Z);
		glVertex3f(Target->X, Target->Y, Target->Z);
		glEnd();
	}
	else {
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3f(0, 1, 1);
		glVertex3f(Position.X, Position.Y, Position.Z);
		glVertex3f(TargetConst.X, TargetConst.Y, TargetConst.Z);
		glEnd();
	}

	if (TargetSec != NULL) {
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3f(1, 1, 0);
		glVertex3f(Position.X, Position.Y, Position.Z);
		glVertex3f(TargetSec->X, TargetSec->Y, TargetSec->Z);
		glEnd();
	}
}

void MBoid::update(float elapsed)
{
	YVec3f Force;
	DTarget = 0;
	if (Target != NULL) {

		YVec3f pertTarget = Perturbation + *Target;
		DTarget = (pertTarget - Position).getSize();
		Force += (pertTarget - Position) * 20 / DTarget;
	}
	else {
		DTarget = (TargetConst - Position).getSize();
		Force += (TargetConst - Position) * 30 / DTarget;
	}

	if (TargetSec != NULL) {
		YVec3f pertTarget = Perturbation + *TargetSec;
		Force += (pertTarget - Position).normalize() * 10;
	}


	if (UseNavField)
		Force += McrftWorld->getNavDir(Position) * 30;
	Speed += Force * elapsed;

	Dir = Speed;
	Dir.normalize();

	if (Speed.getSqrSize() > (12 * 12))
		Speed = Dir * 12;

	if (!Still)
		Position += Speed * elapsed;

}

MBoidManager::MBoidManager(MWorld * world, MAvatar * avatar) {
	World = world;
	Avatar = avatar;
}

void MBoidManager::createBoids()
{
	for (int i = 0; i < 1000; i++) {
		MBoid * b = new MBoid(World);
		b->Position = Avatar->Cam->Position + Avatar->Cam->Direction * 20.0f;
		b->Target = &(Avatar->Position);
		if (Boids.size() > 0)
			b->TargetSec = &(Boids[rand() % Boids.size()]->Position);
		Boids.push_back(b);
	}
	YLog::log(YLog::ENGINE_INFO, "Added boids");
}

void MBoidManager::setNewTargetBoids()
{
	BoidsDodo = false;
	BoidsFreeFlight = true;
	if (randf() > 0.9)
		BoidsTarget = YVec3f(rand() % 2 > 0 ? MWorld::MAT_SIZE_CUBES * MCube::CUBE_SIZE + 100 : -100,
			rand() % 2 > 0 ? MWorld::MAT_SIZE_CUBES * MCube::CUBE_SIZE + 100 : -100,
			rand() % 2 > 0 ? ((MWorld::MAT_HEIGHT_CUBES / 2) + 10) * MCube::CUBE_SIZE : MWorld::MAT_HEIGHT_CUBES * MCube::CUBE_SIZE);
	else
		BoidsTarget = YVec3f(randf() *  MWorld::MAT_SIZE_CUBES * MCube::CUBE_SIZE,
			randf() *  MWorld::MAT_SIZE_CUBES * MCube::CUBE_SIZE,
			rand() % 2 > 0 ? ((MWorld::MAT_HEIGHT_CUBES / 2) + 10) * MCube::CUBE_SIZE : MWorld::MAT_HEIGHT_CUBES * MCube::CUBE_SIZE);

	for (int i = 0; i < Boids.size(); i++)
	{
		Boids[i]->TargetSec = &(Boids[rand() % Boids.size()]->Position);
		Boids[i]->Target = &(BoidsTarget);
		Boids[i]->Still = false;
		Boids[i]->UseNavField = true;
	}
}

void MBoidManager::goToNestBoids()
{
	BoidsDodoElapsed = 0;
	BoidsFreeFlight = false;
	BoidsDodo = true;
	int iBoid = 0;
	while (iBoid < Boids.size()) {
		for (int z = MWorld::MAT_HEIGHT_CUBES; z >= 0; z--)
			for (int x = 0; x < MWorld::MAT_SIZE_CUBES; x++)
				for (int y = 0; y < MWorld::MAT_SIZE_CUBES; y++) {
					if (World->getCube(x, y, z)->getType() == MCube::CUBE_BRANCHES &&
						World->getCube(x, y, z + 1)->getType() == MCube::CUBE_AIR
						&& iBoid < Boids.size()) {
						for (int i = 0; i < 2; i++) {
							if (iBoid < Boids.size()) {
								Boids[iBoid]->TargetConst = YVec3f((x + 0.5)*MCube::CUBE_SIZE + ((randf() - 0.5f) * MCube::CUBE_SIZE),
									(y + 0.5)*MCube::CUBE_SIZE + ((randf() - 0.5f) * MCube::CUBE_SIZE),
									(z + 1.5)*MCube::CUBE_SIZE + ((randf() - 0.5f) * MCube::CUBE_SIZE));
								Boids[iBoid]->Target = NULL;
								Boids[iBoid]->TargetSec = NULL;
								Boids[iBoid]->Still = false;
								Boids[iBoid]->UseNavField = true;
								iBoid++;
							}
						}
					}
				}
	}
}


void MBoidManager::updateBoids(float elapsed)
{
	BoidsDodoElapsed += elapsed;
	for (int i = 0; i < Boids.size(); i++) {
		Boids[i]->update(elapsed);
		if (BoidsFreeFlight) {
			if (Boids[i]->DTarget <= MCube::CUBE_SIZE) {
				if (randf() > 0.2f)
					setNewTargetBoids();
				else
					goToNestBoids();

			}
		}
		if (BoidsDodo) {

			if (Boids[i]->DTarget <= MCube::CUBE_SIZE * 5) {
				Boids[i]->UseNavField = false;
				Boids[i]->Speed *= pow(0.5, elapsed);
			}
			else {
				Boids[i]->UseNavField = true;
			}

			if (Boids[i]->DTarget <= MCube::CUBE_SIZE / 5.0)
				Boids[i]->Still = true;

			if (BoidsDodoElapsed > 30) {
				setNewTargetBoids();
			}
		}
	}
}

void MBoidManager::render(GLuint shader, YVbo * vboCube)
{
	for (int i = 0; i<Boids.size(); i++)
		Boids[i]->render(shader, vboCube);
}
