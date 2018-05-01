#ifndef __YOCTO__ENGINE_TEST__
#define __YOCTO__ENGINE_TEST__

#include <list>
#include <chrono>

#include "engine/engine.h"

#include "avatar.h"
#include "world.h"
#include "boid.h"

#define FWD_KEY 'o'
#define BWD_KEY 'l'
#define LEFT_KEY 'k'
#define RIGHT_KEY 'm'
#define UP_KEY 'p'
#define DOWN_KEY 'i'


class MEngineMinicraft : public YEngine
{

	MAvatar * Avatar;
	MWorld * World;
	//int Seed = 1547852;
	int Seed = time(NULL);

	YVbo * VboCube = NULL;

	GLuint ShaderWorld = 0;
	GLuint ShaderSun = 0;
	GLuint ShaderCube = 0;
	GLuint ShaderCubeDebug = 0;
	GLuint ShaderPostProcess = 0;
	GLuint ShaderBoids = 0;

	YFbo * FboReflections;
	YFbo * FboShadows;
	YFbo * FboPostProcess;

	chrono::steady_clock::time_point startTime;
	float elapsed;

	YCamera CamLight;
	Mat44 VLight;
	Mat44 PLight;
	Mat44 MVPLight;
	YVec3f LightPos;

	float WaterHeight;

	// Camera controller
	float cameraSensitivity = 0.003f;   // The sensitivity of the camera
	float cameraSpeed = 10.f;			// Speed of the camera
	bool cameraLock;				// if the camera look is locked ( locked by default, space to switch)
	list<char> keysPressed;				// list of keys pressed
	bool freeCameraMovement;



	typedef enum
	{
		PASS_SHADOW,
		PASS_REFLECT,
		PASS_FINAL
	}PASS;

	PASS pass = PASS_SHADOW;

	YVec3f SunPosition;
	YVec3f SunDirection;
	YColor SunColor;
	YColor SkyColor;
	float BoostTime = 0.0f;
	bool BoostingTime = false;

	MBoidManager * Boids;

public:
	//Gestion singleton
	static YEngine * getInstance()
	{
		if (Instance == NULL)
			Instance = new MEngineMinicraft();
		return Instance;
	}

	/*HANDLERS GENERAUX*/
	void loadShaders() {
		ShaderWorld = Renderer->createProgram("shaders/world");
		ShaderSun = Renderer->createProgram("shaders/sun");
		ShaderCube = Renderer->createProgram("shaders/cube");
		ShaderCubeDebug = Renderer->createProgram("shaders/cube_debug");
		ShaderPostProcess = Renderer->createProgram("shaders/postprocess");
		//ShaderBoids = Renderer->createProgram("shaders/boids");
	}

	void init()
	{
		startTime = chrono::high_resolution_clock::now();

		YLog::log(YLog::ENGINE_INFO, "Minicraft Started : initialisation");

		VboCube = new YVbo(3, 36, YVbo::PACK_BY_ELEMENT_TYPE);

		VboCube->setElementDescription(0, YVbo::Element(3)); //Sommet
		VboCube->setElementDescription(1, YVbo::Element(3)); //Normale
		VboCube->setElementDescription(2, YVbo::Element(2)); //UV
		VboCube->createVboCpu();
		fillVBOCube(VboCube, MCube::CUBE_SIZE);
		VboCube->createVboGpu();
		VboCube->deleteVboCpu();

		FboReflections = new YFbo(false, 1, 2);
		FboShadows = new YFbo(true, 1, 1);
		FboPostProcess = new YFbo();

		World = new MWorld();
		Avatar = new MAvatar(Renderer->Camera, World);

		World->setPerlinZDecay(MWorld::MAT_HEIGHT_CUBES - 5, 0.5f);
		World->init_world(Seed);
		WaterHeight = 59;

		Renderer->setBackgroundColor(YColor(0.7f, 0.8f, 1.f, 1.f));
		Renderer->Camera->setPosition(YVec3f(10, 10, 10));
		Renderer->Camera->setLookAt(YVec3f(0, 0, 10));

		CamLight.setProjectionOrtho(-MWorld::MAT_SIZE_METERS / 1.2, MWorld::MAT_SIZE_METERS / 1.2,
			-MWorld::MAT_SIZE_METERS / 1.2, MWorld::MAT_SIZE_METERS / 1.2,
			-MWorld::MAT_HEIGHT_METERS / 1.1, MWorld::MAT_HEIGHT_METERS / 1.1);

		if (!cameraLock)
		{
			glutSetCursor(GLUT_CURSOR_NONE);
			glutWarpPointer(MiddleWidth, MiddleHeight);
		}

		Boids = new MBoidManager(World, Avatar);
	}

	void update(float elapsed)
	{
		updateLights(elapsed);

		if (freeCameraMovement)
		{
			YVec3f movement;
			if (std::find(keysPressed.begin(), keysPressed.end(), FWD_KEY) != keysPressed.end())
				movement += Avatar->Cam->Direction;
			if (std::find(keysPressed.begin(), keysPressed.end(), BWD_KEY) != keysPressed.end())
				movement -= Avatar->Cam->Direction;
			if (std::find(keysPressed.begin(), keysPressed.end(), LEFT_KEY) != keysPressed.end())
				movement -= Avatar->Cam->RightVec;
			if (std::find(keysPressed.begin(), keysPressed.end(), RIGHT_KEY) != keysPressed.end())
				movement += Avatar->Cam->RightVec;
			if (std::find(keysPressed.begin(), keysPressed.end(), UP_KEY) != keysPressed.end())
				movement += Avatar->Cam->UpRef;
			if (std::find(keysPressed.begin(), keysPressed.end(), DOWN_KEY) != keysPressed.end())
				movement -= Avatar->Cam->UpRef;
			if (movement.X != 0 || movement.Y != 0 || movement.Z != 0)
				Avatar->Cam->move(movement * cameraSpeed *elapsed);
		}
		else
		{
			Avatar->update(elapsed);
			Avatar->Run = GetKeyState(VK_LSHIFT) & 0x80;
			Renderer->Camera->moveTo(Avatar->Position + YVec3f(0, 0, Avatar->CurrentHeight / 2));
		}
		//Boids->updateBoids(elapsed);
	}

	YVec3f symetry(float zplane, YVec3f pos) {
		pos.Z -= (pos.Z - zplane) * 2;
		return pos;
	}

	void renderObjects()
	{
		//glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//Passe ombres
		
		FboShadows->setAsOutFBO(true);

		LightPos = YVec3f(MWorld::MAT_SIZE_METERS / 2.0f, MWorld::MAT_SIZE_METERS / 2.0f, MWorld::MAT_HEIGHT_METERS / 2.0f);
		//LightPos = YVec3f(MWorld::MAT_SIZE_METERS / 2.0f, MWorld::MAT_SIZE_METERS, MWorld::MAT_HEIGHT_METERS / 2.0f);

		CamLight.setPosition(LightPos);
		CamLight.setLookAt(CamLight.Position - (SunDirection));
		CamLight.look();
		Renderer->updateMatricesFromOgl(&CamLight);

		VLight = Renderer->MatV;
		PLight = Renderer->MatP;
		MVPLight = Renderer->MatMVP;

		glCullFace(GL_FRONT);
		renderScene(PASS::PASS_SHADOW);
		
		
		//Passe reflections
		
		FboReflections->setAsOutFBO(true);
		
		glEnable(GL_CLIP_DISTANCE0);
		YVec3f realcamPos = Renderer->Camera->Position;
		YVec3f realcamLookAt = Renderer->Camera->LookAt;

		Renderer->Camera->setPosition(symetry(WaterHeight, realcamPos));
		Renderer->Camera->setLookAt(symetry(WaterHeight, realcamLookAt));
		Renderer->Camera->look();
		Renderer->updateMatricesFromOgl();

		glCullFace(GL_BACK);
		renderScene(PASS::PASS_REFLECT);
		
		Renderer->Camera->setPosition(realcamPos);
		Renderer->Camera->setLookAt(realcamLookAt);
		Renderer->Camera->look();

		
		

		//Passe de rendu final
		FboPostProcess->setAsOutFBO(true);
		renderScene(PASS::PASS_FINAL);

		//Passe de post process
		
		
		FboPostProcess->setAsOutFBO(false);
		
		glUseProgram(ShaderPostProcess);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		FboPostProcess->setColorAsShaderInput(0, GL_TEXTURE0, "TexColor");
		FboPostProcess->setDepthAsShaderInput(GL_TEXTURE1, "TexDepth");

		GLint var = glGetUniformLocation(ShaderPostProcess, "sunPos");
		glUniform3f(var, SunPosition.X, SunPosition.Y, SunPosition.Z);

		var = glGetUniformLocation(ShaderPostProcess, "skyColor");
		glUniform3f(var, SkyColor.R, SkyColor.V, SkyColor.B);

		Renderer->sendNearFarToShader(ShaderPostProcess);
		Renderer->sendScreenSizeToShader(ShaderPostProcess);
		Renderer->sendMatricesToShader(ShaderPostProcess);
		Renderer->drawFullScreenQuad();
		
		

		
	}

	void renderScene(PASS pass) {

		glUseProgram(0);
		GLuint shader = 0;

		if (pass == PASS::PASS_FINAL) {
			//Rendu des axes
			glDisable(GL_LIGHTING);
			glBegin(GL_LINES);
			glColor3d(1, 0, 0);
			glVertex3d(0, 0, 0);
			glVertex3d(10000, 0, 0);
			glColor3d(0, 1, 0);
			glVertex3d(0, 0, 0);
			glVertex3d(0, 10000, 0);
			glColor3d(0, 0, 1);
			glVertex3d(0, 0, 0);
			glVertex3d(0, 0, 10000);
			glEnd();
		}

		//Rendu du world
		if (pass != PASS::PASS_SHADOW) {

			
			glUseProgram(ShaderWorld);
			shader = ShaderWorld;
			
			FboShadows->setDepthAsShaderInput(GL_TEXTURE0, "TexShadow");
			if (pass != PASS::PASS_REFLECT)
				FboReflections->setColorAsShaderInput(0, GL_TEXTURE1, "TexReflect");

			GLint var = glGetUniformLocation(shader, "V_light");
			glUniformMatrix4fv(var, 1, true, VLight.Mat.t);
			var = glGetUniformLocation(shader, "P_light");
			glUniformMatrix4fv(var, 1, true, PLight.Mat.t);

			var = glGetUniformLocation(shader, "reflexionMapPass");
			glUniform1i(var, pass == PASS_REFLECT ? 1 : 0);

			var = glGetUniformLocation(shader, "water_height");
			glUniform1f(var, WaterHeight);
			
			var = glGetUniformLocation(shader, "lightDir");
			glUniform3f(var, SunDirection.X, SunDirection.Y, SunDirection.Z);

			var = glGetUniformLocation(shader, "lightPos");
			glUniform3f(var, LightPos.X, LightPos.Y, LightPos.Z);

			var = glGetUniformLocation(shader, "camPos");
			glUniform3f(var, Renderer->Camera->Position.X, Renderer->Camera->Position.Y, Renderer->Camera->Position.Z);

			var = glGetUniformLocation(shader, "sunColor");
			glUniform3f(var, SunColor.R, SunColor.V, SunColor.B);

			var = glGetUniformLocation(shader, "skyColor");
			glUniform3f(var, SkyColor.R, SkyColor.V, SkyColor.B);

			var = glGetUniformLocation(shader, "world_size");
			glUniform1f(var, MWorld::MAT_SIZE_METERS);

			var = glGetUniformLocation(shader, "time");
			float time = TimeSinceStart().count() / 100;
			glUniform1f(var, time);

			var = glGetUniformLocation(shader, "elapsed");;
			//float elapsed = 
			//glUniform1f(var, elapsed);


			Renderer->updateMatricesFromOgl();
			Renderer->sendScreenSizeToShader(shader);
			Renderer->sendMatricesToShader(shader);
			this->sendTimeToShader(shader);
			this->sendAllTexToShader(shader);
		}

		bool drawTransparent = true;
		if (pass == PASS::PASS_REFLECT)
			drawTransparent = false;

		World->render_world_vbo(shader, false, drawTransparent);

		if (pass != PASS::PASS_SHADOW) {
			glPushMatrix();
			glUseProgram(ShaderSun);
			GLuint var = glGetUniformLocation(ShaderSun, "sun_color");
			glUniform3f(var, SunColor.R, SunColor.V, SunColor.B);
			glTranslatef(SunPosition.X, SunPosition.Y, SunPosition.Z);
			glScalef(10, 10, 10);
			Renderer->updateMatricesFromOgl();
			Renderer->sendMatricesToShader(ShaderSun);
			VboCube->render();
			glPopMatrix();
		}

		//glUseProgram(ShaderBoids);
		//Boids->render(ShaderBoids,VboCube);
	}

	void resize(int width, int height) {
		FboReflections->resize(width, height);
		FboShadows->resize(width, height);
		FboPostProcess->resize(width, height);
		
	}

	/*FONCTION MINECRAFT*/

	//On commence par le point en UV 0,0 et on tourne en CCW
	int addQuadToVbo(YVbo * vbo, int iVertice, YVec3f & a, YVec3f & b, YVec3f & c, YVec3f & d) {
		YVec3f normal = (b - a).cross(d - a);
		normal.normalize();

		vbo->setElementValue(0, iVertice, a.X, a.Y, a.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 0, 0);
	

		iVertice++;
		
		vbo->setElementValue(0, iVertice, b.X, b.Y, b.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 1, 0);

		iVertice++;

		vbo->setElementValue(0, iVertice, c.X, c.Y, c.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 1, 1);

		iVertice++;

		vbo->setElementValue(0, iVertice, a.X, a.Y, a.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 0, 0);

		iVertice++;

		vbo->setElementValue(0, iVertice, c.X, c.Y, c.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 1, 1);

		iVertice++;

		vbo->setElementValue(0, iVertice, d.X, d.Y, d.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 0, 1);

		iVertice++;

		return 6;

	}

	void fillVBOCube(YVbo * vbo, float size = 5.0f)
	{
		int iVertice = 0;

		YVec3f a(size / 2.0f, -size / 2.0f, -size / 2.0f);
		YVec3f b(size / 2.0f, size / 2.0f, -size / 2.0f);
		YVec3f c(size / 2.0f, size / 2.0f, size / 2.0f);
		YVec3f d(size / 2.0f, -size / 2.0f, size / 2.0f);
		YVec3f e(-size / 2.0f, -size / 2.0f, -size / 2.0f);
		YVec3f f(-size / 2.0f, size / 2.0f, -size / 2.0f);
		YVec3f g(-size / 2.0f, size / 2.0f, size / 2.0f);
		YVec3f h(-size / 2.0f, -size / 2.0f, size / 2.0f);

		iVertice += addQuadToVbo(vbo, iVertice, a, b, c, d); //x+
		iVertice += addQuadToVbo(vbo, iVertice, f, e, h, g); //x-
		iVertice += addQuadToVbo(vbo, iVertice, b, f, g, c); //y+
		iVertice += addQuadToVbo(vbo, iVertice, e, a, d, h); //y-
		iVertice += addQuadToVbo(vbo, iVertice, c, g, h, d); //z+
		iVertice += addQuadToVbo(vbo, iVertice, e, f, b, a); //z-
	}

	bool getSunDirFromDayTime(YVec3f & sunDir, float mnLever, float mnCoucher, float boostTime)
	{
		bool nuit = false;

		SYSTEMTIME t;
		GetLocalTime(&t);

		//On borne le tweak time à une journée (cyclique)
		while (boostTime > 24 * 60)
			boostTime -= 24 * 60;

		//Temps écoulé depuis le début de la journée
		float fTime = (float)(t.wHour * 60 + t.wMinute);
		fTime += boostTime;
		while (fTime > 24 * 60)
			fTime -= 24 * 60;

		//Si c'est la nuit
		if (fTime < mnLever || fTime > mnCoucher) {
			nuit = true;
			if (fTime < mnLever)
				fTime += 24 * 60;
			fTime -= mnCoucher;
			fTime /= (mnLever + 24 * 60 - mnCoucher);
			fTime *= (float)M_PI;
		} else {
			//c'est le jour
			nuit = false;
			fTime -= mnLever;
			fTime /= (mnCoucher - mnLever);
			fTime *= (float)M_PI;
		}

		//Direction du soleil en fonction de l'heure
		sunDir.X = cos(fTime);
		sunDir.Y = 0.2f;
		sunDir.Z = sin(fTime);
		sunDir.normalize();

		return nuit;
	}

	void updateLights(float elapsed) {

		if (BoostingTime)
			BoostTime += elapsed * 60.0f;

		//On recup la direciton du soleil
		bool nuit = getSunDirFromDayTime(SunDirection, 6.0f * 60.0f, 19.0f * 60.0f, BoostTime);
		SunPosition = Renderer->Camera->Position + SunDirection * 500.0f;

		//Pendant la journée
		if (!nuit) {
			//On definit la couleur
			SunColor = YColor(1.0f, 1.0f, 0.8f, 1.0f);
			SkyColor = YColor(0.0f, 181.f / 255.f, 221.f / 255.f, 1.0f);
			YColor downColor(0.9f, 0.5f, 0.1f, 1);

			SunColor = SunColor.interpolate(downColor, (abs(SunDirection.X)));
			SkyColor = SkyColor.interpolate(downColor, (abs(SunDirection.X)));
		} else {
			//La nuit : lune blanche et ciel noir
			SunColor = YColor(1, 1, 1, 1);
			SkyColor = YColor(0, 0, 0, 1);
		}

		Renderer->setBackgroundColor(SkyColor);
	}

	chrono::milliseconds TimeSinceStart() {
		chrono::steady_clock::time_point currentTime = chrono::steady_clock::now();
		return chrono::duration_cast<chrono::milliseconds>(currentTime - startTime);
	}

	chrono::seconds TimeSinceStartSeconds() {
		chrono::steady_clock::time_point currentTime = chrono::steady_clock::now();
		return chrono::duration_cast<chrono::seconds>(currentTime - startTime);
	}


	/*INPUTS*/

	void keyPressed(int key, bool special, bool down, int p1, int p2) {

		if (key == 'c' && down)
		{
			freeCameraMovement = !freeCameraMovement;
			Avatar->Position = Avatar->Cam->Position;
			Avatar->Speed = YVec3f();
			if (!freeCameraMovement)
			{
				SetCameraLock(false);
			}
		}
		if (key == 'g')
			BoostingTime = down;
		/*
		if (key == 'v' && down)
			Boids->setNewTargetBoids();
		if (key == 'b' && down)
			Boids->createBoids();
		if (key == 'n' && down)
			Boids->goToNestBoids();
		*/

		// Free movement controller
		if (freeCameraMovement)
		{
			if (key == FWD_KEY || key == BWD_KEY || key == LEFT_KEY || key == RIGHT_KEY || key == UP_KEY || key == DOWN_KEY)
			{
				if (down)
					keysPressed.push_back(key);
				else
					keysPressed.remove(key);
			}
			
			if (key == VK_SPACE && down)
			{
				SetCameraLock(!cameraLock);
			}
			
		}
		// FPS controller
		else
		{
			switch (key){
				case FWD_KEY:
					Avatar->avance = down;
					break;
				case BWD_KEY:
					Avatar->recule = down;
					break;
				case LEFT_KEY:
					Avatar->gauche = down;
					break;
				case RIGHT_KEY:
					Avatar->droite = down;
					break;
				case VK_SPACE:
					Avatar->Jump = down;
					break;
			}
		}

	}

	void mouseWheel(int wheel, int dir, int x, int y, bool inUi)
	{
		Renderer->Camera->move(Renderer->Camera->Direction * 10.0f * dir);
	}

	void mouseClick(int button, int state, int x, int y, bool inUi)
	{
		//if (!freeCameraMovement)
		//{
			if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
			{
				int xC, yC, zC;
				YVec3f inter;
				World->getRayCollision(Renderer->Camera->Position,
					Renderer->Camera->Position + Renderer->Camera->Direction * 30,
					inter, xC, yC, zC);
				World->deleteCube(xC, yC, zC);
			}
			else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
			{
				
			}
		//}
		
	}

	void mouseMove(int x, int y, bool pressed, bool inUi)
	{
		if (!cameraLock)
		{
			glutWarpPointer(MiddleWidth, MiddleHeight);

			float xOffset = MiddleWidth - x;
			float yOffset = MiddleHeight - y;
			Avatar->Cam->rotate(xOffset * cameraSensitivity );
			Avatar->Cam->rotateUp(yOffset * cameraSensitivity );
		}
	}

	void SetCameraLock(bool lock)
	{
		cameraLock = lock;
		if (cameraLock)
		{
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		}
		else
		{
			glutSetCursor(GLUT_CURSOR_NONE);
			glutWarpPointer(MiddleWidth, MiddleHeight);
		}
	}
};


#endif