//Includes application
#include <conio.h>
#include <vector>
#include <list>
#include <string>
#include <windows.h>

#include "external/gl/glew.h"
#include "external/gl/freeglut.h"

//Moteur
#include "engine/utils/types_3d.h"
#include "engine/timer.h"
#include "engine/log/log_console.h"
#include "engine/render/renderer.h"
#include "engine/gui/screen.h"
#include "engine/gui/screen_manager.h"

#include "world.h"
#include "avatar.h"

#define FWD_KEY 'o'
#define BWD_KEY 'l'
#define LEFT_KEY 'k'
#define RIGHT_KEY 'm'
#define UP_KEY 'p'
#define DOWN_KEY 'i'

int g_screen_width = 800, g_screen_height = 600;
float screen_center_x, screen_center_y;

NYRenderer * g_renderer = NULL;
NYTimer * g_timer = NULL;
int g_nb_frames = 0;
float g_elapsed_fps = 0;
int g_main_window_id;
int g_mouse_btn_gui_state = 0;
bool g_fullscreen = false;

//GUI 
GUIScreenManager * g_screen_manager = NULL;
GUIBouton * BtnParams = NULL;
GUIBouton * BtnClose = NULL;
GUILabel * LabelFps = NULL;
GUILabel * LabelCam = NULL;
GUIScreen * g_screen_params = NULL;
GUIScreen * g_screen_jeu = NULL;
GUISlider * g_slider;

// Camera controller
float cameraSensitivity = 0.01f;   // The sensitivity of the camera
float cameraSpeed = 100.f;			// Speed of the camera
bool firstPersonCamera = true;		// if the camera is first or third person ( first by default, 't' to switch)
bool cameraLock = true;				// if the camera look is locked ( locked by default, space to switch)
list<char> keysPressed;				// list of keys pressed
bool avatarCamera;


//Soleil
NYVert3Df g_sun_dir;
NYColor g_sun_color;
float g_mn_lever = 6.0f * 60.0f;
float g_mn_coucher = 19.0f * 60.0f;
float g_tweak_time = 0;
bool g_fast_time = false;

// World
NYWorld * g_world;
NYAvatar* g_avatar;


//////////////////////////////////////////////////////////////////////////
// GESTION APPLICATION
//////////////////////////////////////////////////////////////////////////
void update(void)
{
	float elapsed = g_timer->getElapsedSeconds(true);


	static float g_eval_elapsed = 0;

	//Calcul des fps
	g_elapsed_fps += elapsed;
	g_nb_frames++;
	if(g_elapsed_fps > 1.0)
	{
		LabelFps->Text = std::string("FPS : ") + toString(g_nb_frames);
		g_elapsed_fps -= 1.0f;
		g_nb_frames = 0;
	}

	if (g_fast_time)
		g_tweak_time += elapsed * 120.0f;

	//Rendu
	g_renderer->render(elapsed);

	if (!avatarCamera)
	{
		NYVert3Df movement;
		if (std::find(keysPressed.begin(), keysPressed.end(), FWD_KEY) != keysPressed.end())
			movement += g_renderer->_Camera->_Direction;
		if (std::find(keysPressed.begin(), keysPressed.end(), BWD_KEY) != keysPressed.end())
			movement -= g_renderer->_Camera->_Direction;
		if (std::find(keysPressed.begin(), keysPressed.end(), LEFT_KEY) != keysPressed.end())
			movement -= g_renderer->_Camera->_NormVec;
		if (std::find(keysPressed.begin(), keysPressed.end(), RIGHT_KEY) != keysPressed.end())
			movement += g_renderer->_Camera->_NormVec;
		if (std::find(keysPressed.begin(), keysPressed.end(), UP_KEY) != keysPressed.end())
			movement += g_renderer->_Camera->_UpRef;
		if (std::find(keysPressed.begin(), keysPressed.end(), DOWN_KEY) != keysPressed.end())
			movement -= g_renderer->_Camera->_UpRef;
		if (movement.X != 0 || movement.Y != 0 || movement.Z != 0)
			g_renderer->_Camera->move(movement * cameraSpeed *elapsed);
	}
	else
	{
		g_avatar->update(elapsed);
	}
	
}


void render2d(void)
{
	g_screen_manager->render();
}

void renderObjects(void)
{
	// RENDU DES AXES
	// --------------

	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);
	glColor3d(1,0,0);
	glVertex3d(0,0,0);
	glVertex3d(10000,0,0);
	glColor3d(0,1,0);
	glVertex3d(0,0,0);
	glVertex3d(0,10000,0);
	glColor3d(0,0,1);
	glVertex3d(0,0,0);
	glVertex3d(0,0,10000);
	glEnd();

	// RENDU DU SOLEIL 
	// ---------------

	glEnable(GL_LIGHTING);

	//On sauve la matrice
	glPushMatrix();

	//Position du soleil
	glTranslatef(g_renderer->_Camera->_Position.X, g_renderer->_Camera->_Position.Y, g_renderer->_Camera->_Position.Z);
	glTranslatef(g_sun_dir.X * 1000, g_sun_dir.Y * 1000, g_sun_dir.Z * 1000);

	//Material du soleil : de l'emissive
	GLfloat sunEmissionMaterial[] = { 0.0, 0.0, 0.0,1.0 };
	sunEmissionMaterial[0] = g_sun_color.R;
	sunEmissionMaterial[1] = g_sun_color.V;
	sunEmissionMaterial[2] = g_sun_color.B;
	glMaterialfv(GL_FRONT, GL_EMISSION, sunEmissionMaterial);

	//On dessine un cube pour le soleil
	glutSolidCube(50.0f);

	//On reset le material emissive pour la suite
	sunEmissionMaterial[0] = 0.0f;
	sunEmissionMaterial[1] = 0.0f;
	sunEmissionMaterial[2] = 0.0f;
	glMaterialfv(GL_FRONT, GL_EMISSION, sunEmissionMaterial);

	//Reset de la matrice
	glPopMatrix();

	glPushMatrix();
	// g_world->render_world_old_school();
	g_world->render_world_vbo();
	glPopMatrix();

	if (avatarCamera)
		g_avatar->render();
}

bool getSunDirection(NYVert3Df & sun, float mnLever, float mnCoucher)
{
	bool nuit = false;

	SYSTEMTIME t;
	GetLocalTime(&t);

	//On borne le tweak time à une journée (cyclique)
	while (g_tweak_time > 24 * 60)
		g_tweak_time -= 24 * 60;

	//Temps écoulé depuis le début de la journée
	float fTime = ( float )( t.wHour * 60 + t.wMinute );
	fTime += g_tweak_time;
	while (fTime > 24 * 60)
		fTime -= 24 * 60;

	//Si c'est la nuit
	if (fTime < mnLever || fTime > mnCoucher)
	{
		nuit = true;
		if (fTime < mnLever)
			fTime += 24 * 60;
		fTime -= mnCoucher;
		fTime /= ( mnLever + 24 * 60 - mnCoucher );
		fTime *= M_PI;
	}
	else
	{
		//c'est le jour
		nuit = false;
		fTime -= mnLever;
		fTime /= ( mnCoucher - mnLever );
		fTime *= M_PI;
	}

	//Position en fonction de la progression dans la journée
	sun.X = cos(fTime);
	sun.Y = 0.2f;
	sun.Z = sin(fTime);
	sun.normalize();

	return nuit;
}

void setLightsBasedOnDayTime(void)
{
	//On active la light 0
	glEnable(GL_LIGHT0);

	//On recup la direciton du soleil
	bool nuit = getSunDirection(g_sun_dir, g_mn_lever, g_mn_coucher);

	//On définit une lumière directionelle (un soleil)
	float position[4] = { g_sun_dir.X,g_sun_dir.Y,g_sun_dir.Z,0 }; ///w = 0 donc c'est une position a l'infini
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	//Pendant la journée
	if (!nuit)
	{
		//On definit la couleur
		NYColor sunColor(1, 1, 0.8, 1);
		NYColor skyColor(0, 181.f / 255.f, 221.f / 255.f, 1);
		NYColor downColor(0.9, 0.5, 0.1, 1);
		sunColor = sunColor.interpolate(downColor, ( abs(g_sun_dir.X) ));
		skyColor = skyColor.interpolate(downColor, ( abs(g_sun_dir.X) ));

		g_renderer->setBackgroundColor(skyColor);

		float color[4] = { sunColor.R,sunColor.V,sunColor.B,1 };
		glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
		float color2[4] = { sunColor.R,sunColor.V,sunColor.B,1 };
		glLightfv(GL_LIGHT0, GL_AMBIENT, color2);
		g_sun_color = sunColor;
	}
	else
	{
		//La nuit : lune blanche et ciel noir
		NYColor sunColor(1, 1, 1, 1);
		NYColor skyColor(0, 0, 0, 1);
		g_renderer->setBackgroundColor(skyColor);

		float color[4] = { sunColor.R / 3.f,sunColor.V / 3.f,sunColor.B / 3.f,1 };
		glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
		float color2[4] = { sunColor.R / 7.f,sunColor.V / 7.f,sunColor.B / 7.f,1 };
		glLightfv(GL_LIGHT0, GL_AMBIENT, color2);
		g_sun_color = sunColor;
	}
}

void resizeFunction(int width, int height)
{
	glViewport(0, 0, width, height);
	g_renderer->resize(width,height);
	g_screen_width = width;
	g_screen_height = height; 
	screen_center_x = g_screen_width / 2;
	screen_center_y = g_screen_height / 2;
}



//////////////////////////////////////////////////////////////////////////
// GESTION CLAVIER SOURIS
//////////////////////////////////////////////////////////////////////////

void specialDownFunction(int key, int p1, int p2)
{
}

void specialUpFunction(int key, int p1, int p2)
{
}

void keyboardDownFunction(unsigned char key, int p1, int p2)
{
	if(key == VK_ESCAPE)
	{
		glutDestroyWindow(g_main_window_id);	
		exit(0);
	}

	

	if(key == 'f')
	{
		if(!g_fullscreen){
			glutFullScreen();
			g_fullscreen = true;
		} else if(g_fullscreen){
			glutLeaveGameMode();
			glutLeaveFullScreen();
			glutReshapeWindow(g_renderer->_ScreenWidth, g_renderer->_ScreenWidth);
			glutPositionWindow(0,0);
			g_fullscreen = false;
		}
	}	

	if (key == 't')
	{
		firstPersonCamera = !firstPersonCamera;
	}
	if (key == 'g')
	{
		g_fast_time = !g_fast_time;
	}
	if (key == 'r')
	{
		g_world->init_world();
	}
	if (key == 'c')
	{
		avatarCamera = !avatarCamera;
		g_avatar->Position = g_renderer->_Camera->_Position;
		g_avatar->Speed =  NYVert3Df();
		keysPressed.clear();

	}

	if (avatarCamera)
	{
		switch (key)
		{
		case FWD_KEY:
			g_avatar->avance = true;
			break;
		case BWD_KEY:
			g_avatar->recule = true;
			break;
		case LEFT_KEY:
			g_avatar->gauche = true;
			break;
		case RIGHT_KEY:
			g_avatar->droite = true;
			break;
		case VK_SPACE:
			g_avatar->Jump = true;
			break;
		}
	}
	else
	{
		if (key == FWD_KEY || key == BWD_KEY || key == LEFT_KEY || key == RIGHT_KEY || key == UP_KEY || key == DOWN_KEY)
			keysPressed.push_back(key);

		if (key == VK_SPACE)
		{
			cameraLock = !cameraLock;
			if (cameraLock)
			{
				glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
			}
			else
			{
				glutSetCursor(GLUT_CURSOR_NONE);
				glutWarpPointer(screen_center_x, screen_center_y);
			}
		}
	}
		
}

void keyboardUpFunction(unsigned char key, int p1, int p2)
{
	if (avatarCamera)
	{
		if (key == FWD_KEY)
			g_avatar->avance = false;
		if (key == BWD_KEY)
			g_avatar->recule = false;
		if (key == LEFT_KEY)
			g_avatar->gauche = false;
		if (key == RIGHT_KEY)
			g_avatar->droite = false;
		if (key == VK_SPACE)
			g_avatar->jump();
	}
	else
		if (key == FWD_KEY || key == BWD_KEY || key == LEFT_KEY || key == RIGHT_KEY || key == UP_KEY || key == DOWN_KEY)
			keysPressed.remove(key);
}

void mouseWheelFunction(int wheel, int dir, int x, int y)
{
	
}

void mouseFunction(int button, int state, int x, int y)
{
	//Gestion de la roulette de la souris
	if((button & 0x07) == 3 && state)
		mouseWheelFunction(button,1,x,y);
	if((button & 0x07) == 4 && state)
		mouseWheelFunction(button,-1,x,y);

	//GUI
	g_mouse_btn_gui_state = 0;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (avatarCamera)
			g_avatar->addCube();
		g_mouse_btn_gui_state |= GUI_MLBUTTON;
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		if (avatarCamera)
			g_avatar->removeCube();
	}
	
	g_screen_manager->mouseCallback(x,y,g_mouse_btn_gui_state,0,0);
}

void mouseMoveFunction(int x, int y, bool pressed)
{
	g_screen_manager->mouseCallback(x, y, g_mouse_btn_gui_state, 0, 0);

	if(!cameraLock)
	{
		glutWarpPointer(screen_center_x, screen_center_y);

		float xOffset = screen_center_x - x;
		float yOffset = screen_center_y - y;

		if (firstPersonCamera)
		{
			g_renderer->_Camera->rotate(xOffset * cameraSensitivity);
			g_renderer->_Camera->rotateUp(yOffset * cameraSensitivity);
		}
		else
		{
			g_renderer->_Camera->rotateAround(xOffset * cameraSensitivity);
			g_renderer->_Camera->rotateUpAround(yOffset * cameraSensitivity);
		}
	}
}

void mouseMoveActiveFunction(int x, int y)
{
	mouseMoveFunction(x,y,true);
}
void mouseMovePassiveFunction(int x, int y)
{
	mouseMoveFunction(x,y,false);
}


void clickBtnParams (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_params);
}

void clickBtnCloseParam (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_jeu);
}

/**
  * POINT D'ENTREE PRINCIPAL
  **/
int main(int argc, char* argv[])
{ 
	LogConsole::createInstance();
	
	screen_center_x = g_screen_width / 2;
	screen_center_y = g_screen_height / 2;

	glutInit(&argc, argv); 
	glutInitContextVersion(3,0);
	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
		);

	glutInitWindowSize(g_screen_width, g_screen_height);
	glutInitWindowPosition (0, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE );

	glEnable(GL_MULTISAMPLE);

	Log::log(Log::ENGINE_INFO, (toString(argc) + " arguments en ligne de commande.").c_str());	
	bool gameMode = true;
	for(int i=0;i<argc;i++)
	{
		if(argv[i][0] == 'w')
		{
			Log::log(Log::ENGINE_INFO,"Arg w mode fenetre.\n");
			gameMode = false;
		}
	}

	if(gameMode)
	{
		int width = glutGet(GLUT_SCREEN_WIDTH);
		int height = glutGet(GLUT_SCREEN_HEIGHT);
		
		char gameModeStr[200];
		sprintf(gameModeStr,"%dx%d:32@60",width,height);
		glutGameModeString(gameModeStr);
		g_main_window_id = glutEnterGameMode();
	}
	else
	{
		g_main_window_id = glutCreateWindow("MyNecraft");
		glutReshapeWindow(g_screen_width, g_screen_height);
	}

	if(g_main_window_id < 1) 
	{
		Log::log(Log::ENGINE_ERROR,"Erreur creation de la fenetre.");
		exit(EXIT_FAILURE);
	}
	
	GLenum glewInitResult = glewInit();

	if (glewInitResult != GLEW_OK)
	{
		Log::log(Log::ENGINE_ERROR,("Erreur init glew " + std::string((char*)glewGetErrorString(glewInitResult))).c_str());
		_cprintf("ERROR : %s",glewGetErrorString(glewInitResult));
		exit(EXIT_FAILURE);
	}

	//Affichage des capacités du système
	Log::log(Log::ENGINE_INFO,("OpenGL Version : " + std::string((char*)glGetString(GL_VERSION))).c_str());

	glutDisplayFunc(update);
	glutReshapeFunc(resizeFunction);
	glutKeyboardFunc(keyboardDownFunction);
	glutKeyboardUpFunc(keyboardUpFunction);
	glutSpecialFunc(specialDownFunction);
	glutSpecialUpFunc(specialUpFunction);
	glutMouseFunc(mouseFunction);
	glutMotionFunc(mouseMoveActiveFunction);
	glutPassiveMotionFunc(mouseMovePassiveFunction);
	glutIgnoreKeyRepeat(1);

	//Initialisation du renderer
	g_renderer = NYRenderer::getInstance();
	g_renderer->setRenderObjectFun(renderObjects);
	g_renderer->setRender2DFun(render2d);
	g_renderer->setLightsFun(setLightsBasedOnDayTime);
	g_renderer->setBackgroundColor(NYColor());
	g_renderer->initialise();

	//On applique la config du renderer
	glViewport(0, 0, g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);
	g_renderer->resize(g_renderer->_ScreenWidth,g_renderer->_ScreenHeight);
	
	//Ecran de jeu
	uint16 x = 10;
	uint16 y = 10;
	g_screen_jeu = new GUIScreen(); 

	g_screen_manager = new GUIScreenManager();
		
	//Bouton pour afficher les params
	BtnParams = new GUIBouton();
	BtnParams->Titre = std::string("Params");
	BtnParams->X = x;
	BtnParams->setOnClick(clickBtnParams);
	g_screen_jeu->addElement(BtnParams);

	y += BtnParams->Height + 1;

	LabelFps = new GUILabel();
	LabelFps->Text = "FPS";
	LabelFps->X = x;
	LabelFps->Y = y;
	LabelFps->Visible = true;
	g_screen_jeu->addElement(LabelFps);

	//Ecran de parametrage
	x = 10;
	y = 10;
	g_screen_params = new GUIScreen();

	GUIBouton * btnClose = new GUIBouton();
	btnClose->Titre = std::string("Close");
	btnClose->X = x;
	btnClose->setOnClick(clickBtnCloseParam);
	g_screen_params->addElement(btnClose);

	y += btnClose->Height + 1;
	y+=10;
	x+=10;

	GUILabel * label = new GUILabel();
	label->X = x;
	label->Y = y;
	label->Text = "Param :";
	g_screen_params->addElement(label);

	y += label->Height + 1;

	g_slider = new GUISlider();
	g_slider->setPos(x,y);
	g_slider->setMaxMin(1,0);
	g_slider->Visible = true;
	g_screen_params->addElement(g_slider);

	y += g_slider->Height + 1;
	y+=10;

	//Ecran a rendre
	g_screen_manager->setActiveScreen(g_screen_jeu);
	
	//Init Camera
	g_renderer->_Camera->setPosition(NYVert3Df(10,10,10));
	g_renderer->_Camera->setLookAt(NYVert3Df(0,0,0));
	

	//Fin init moteur

	//Init application

	g_world = new NYWorld();
	g_world->_FacteurGeneration = 5;
	g_world->init_world();

	g_avatar = new NYAvatar(g_renderer->_Camera, g_world);

	//Init Timer
	g_timer = new NYTimer();
	
	//On start
	g_timer->start();

	glutMainLoop(); 

	return 0;
}

