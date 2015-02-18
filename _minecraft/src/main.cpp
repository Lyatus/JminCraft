//Includes application
#include <conio.h>
#include <vector>
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

#include "avatar.h"
#include "world.h"


NYRenderer * g_renderer = NULL;
NYTimer * g_timer = NULL;
NYWorld * g_world;
NYAvatar * g_avatar;
int g_nb_frames = 0;
float g_elapsed_fps = 0;
int g_main_window_id;
int g_mouse_btn_gui_state = 0;
bool g_fullscreen = false;

//Sun
NYVert3Df g_sun_dir;
NYColor g_sun_color;
float g_mn_lever = 6.0f * 60.0f;
float g_mn_coucher = 19.0f * 60.0f;
float g_tweak_time = 0;
bool g_fast_time = true;

//GUI 
GUIScreenManager * g_screen_manager = NULL;
GUIBouton * BtnParams = NULL;
GUIBouton * BtnClose = NULL;
GUILabel * LabelFps = NULL;
GUILabel * LabelCam = NULL;
GUIScreen * g_screen_params = NULL;
GUIScreen * g_screen_jeu = NULL;
GUISlider * g_slider;


//////////////////////////////////////////////////////////////////////////
// GESTION APPLICATION
//////////////////////////////////////////////////////////////////////////
void update(void) {
	float elapsed = g_timer->getElapsedSeconds(true);

	static float g_eval_elapsed = 0;

	//Calcul des fps
	g_elapsed_fps += elapsed;
	g_nb_frames++;
	if (g_elapsed_fps > 1.0) {
		LabelFps->Text = std::string("FPS : ") + toString(g_nb_frames);
		g_elapsed_fps -= 1.0f;
		g_nb_frames = 0;
	}

	//Avatar
	g_avatar->update(elapsed);


	
	//Rendu
	g_renderer->render(elapsed);

	
}


void render2d(void) {
	g_screen_manager->render();
}

void renderObjects(void) {
	

	//Rendu des axes
	glDisable(GL_LIGHTING);

	//Dessin des axes : on dessine des lignes
	glBegin(GL_LINES);

	//Première primitive : axe des x
	glColor3d(1, 0, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(10000, 0, 0);

	//Deuxième primitive : axe des y
	glColor3d(0, 1, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 10000, 0);

	//Troisième primitive : axe des z
	glColor3d(0, 0, 1);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, 10000);
	glEnd();

	//juste apres le rendu des axes

	//Active la lumière
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);

	//Rendu du soleil

	//On sauve la matrice
	glPushMatrix();

	//Position du soleil
	glTranslatef(g_renderer->_Camera->_Position.X, g_renderer->_Camera->_Position.Y, g_renderer->_Camera->_Position.Z);
	glTranslatef(g_sun_dir.X * 1000, g_sun_dir.Y * 1000, g_sun_dir.Z * 1000);

	//Material du soleil : de l'emissive
	GLfloat sunEmissionMaterial[] = { 0.0, 0.0, 0.0, 1.0 };
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

	//Rendu du monde
	g_world->render_world_vbo();

	//Rendu de l'avatar
	g_avatar->render();

	//Sphère blanche transparente pour bien voir le shading et le reflet du soleil
	GLfloat whiteSpecularMaterialSphere[] = { 0.3, 0.3, 0.3, 0.8 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteSpecularMaterialSphere);
	glMaterialf(GL_FRONT, GL_SHININESS, 100);

	GLfloat materialDiffuseSphere[] = { 0.7, 0.7, 0.7, 0.8 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuseSphere);
	GLfloat materialAmbientSphere[] = { 0.2, 0.2, 0.2, 0.8 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbientSphere);
}

bool getSunDirection(NYVert3Df & sun, float mnLever, float mnCoucher) {
	bool nuit = false;

	SYSTEMTIME t;
	GetLocalTime(&t);

	//On borne le tweak time à une journée (cyclique)
	while (g_tweak_time > 24 * 60)
		g_tweak_time -= 24 * 60;

	//Temps écoulé depuis le début de la journée
	float fTime = (float)(t.wHour * 60 + t.wMinute);
	fTime += g_tweak_time;
	while (fTime > 24 * 60)
		fTime -= 24 * 60;

	//Si c'est la nuit
	if (fTime < mnLever || fTime > mnCoucher) {
		nuit = true;
		if (fTime < mnLever)
			fTime += 24 * 60;
		fTime -= mnCoucher;
		fTime /= (mnLever + 24 * 60 - mnCoucher);
		fTime *= M_PI;
	}
	else {
		//c'est le jour
		nuit = false;
		fTime -= mnLever;
		fTime /= (mnCoucher - mnLever);
		fTime *= M_PI;
	}

	//Position en fonction de la progression dans la journée
	sun.X = cos(fTime);
	sun.Y = 0.2f;
	sun.Z = sin(fTime);
	sun.normalize();

	return nuit;
}
void setLights(void) {
	//On active la light 0
	glEnable(GL_BLEND);
	glEnable(GL_LIGHT0);

	//On recup la direciton du soleil
	bool nuit = getSunDirection(g_sun_dir, g_mn_lever, g_mn_coucher);

	//On définit une lumière directionelle (un soleil)
	float position[4] = { g_sun_dir.X, g_sun_dir.Y, g_sun_dir.Z, 0 }; ///w = 0 donc c'est une position a l'infini
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	//Pendant la journée
	if (!nuit) {
		//On definit la couleur
		NYColor sunColor(1, 1, 0.8, 1);
		NYColor skyColor(0, 181.f / 255.f, 221.f / 255.f, 1);
		NYColor downColor(0.9, 0.5, 0.1, 1);
		sunColor = sunColor.interpolate(downColor, (abs(g_sun_dir.X)));
		skyColor = skyColor.interpolate(downColor, (abs(g_sun_dir.X)));

		g_renderer->setBackgroundColor(skyColor);

		float color[4] = { sunColor.R, sunColor.V, sunColor.B, 1 };
		glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
		float color2[4] = { sunColor.R, sunColor.V, sunColor.B, 1 };
		glLightfv(GL_LIGHT0, GL_AMBIENT, color2);
		g_sun_color = sunColor;
	}
	else {
		//La nuit : lune blanche et ciel noir
		NYColor sunColor(1, 1, 1, 1);
		NYColor skyColor(0, 0, 0, 1);
		g_renderer->setBackgroundColor(skyColor);

		float color[4] = { sunColor.R / 3.f, sunColor.V / 3.f, sunColor.B / 3.f, 1 };
		glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
		float color2[4] = { sunColor.R / 7.f, sunColor.V / 7.f, sunColor.B / 7.f, 1 };
		glLightfv(GL_LIGHT0, GL_AMBIENT, color2);
		g_sun_color = sunColor;
	}
}

void resizeFunction(int width, int height) {
	glViewport(0, 0, width, height);
	g_renderer->resize(width, height);
}

//////////////////////////////////////////////////////////////////////////
// GESTION CLAVIER SOURIS
//////////////////////////////////////////////////////////////////////////

void specialDownFunction(int key, int p1, int p2) {
	//On change de mode de camera
	if (key == GLUT_KEY_LEFT) {
	}

}

void specialUpFunction(int key, int p1, int p2) {

}

void keyboardDownFunction(unsigned char key, int p1, int p2) {
	if (key == VK_ESCAPE) {
		glutDestroyWindow(g_main_window_id);
		exit(0);
	}

	if (key == 'f') {
		if (!g_fullscreen) {
			glutFullScreen();
			g_fullscreen = true;
		}
		else if (g_fullscreen) {
			glutLeaveGameMode();
			glutLeaveFullScreen();
			glutReshapeWindow(g_renderer->_ScreenWidth, g_renderer->_ScreenWidth);
			glutPositionWindow(0, 0);
			g_fullscreen = false;
		}
	}
	g_avatar->key(key, true);
}

void keyboardUpFunction(unsigned char key, int p1, int p2) {
	g_avatar->key(key, false);
}

void mouseWheelFunction(int wheel, int dir, int x, int y) {
	g_renderer->_Camera->move(NYVert3Df(0, 0, (wheel*dir) / 2.f));
}

void mouseFunction(int button, int state, int x, int y) {
	//Gestion de la roulette de la souris
	if ((button & 0x07) == 3 && state)
		mouseWheelFunction(button, 1, x, y);
	if ((button & 0x07) == 4 && state)
		mouseWheelFunction(button, -1, x, y);

	//GUI
	g_mouse_btn_gui_state = 0;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		g_mouse_btn_gui_state |= GUI_MLBUTTON;

	bool mouseTraite = false;
	mouseTraite = g_screen_manager->mouseCallback(x, y, g_mouse_btn_gui_state, 0, 0);
}

int oldx = 0, oldy = 0;
void mouseMoveFunction(int x, int y, bool pressed) {
	bool mouseTraite = false;

	mouseTraite = g_screen_manager->mouseCallback(x, y, g_mouse_btn_gui_state, 0, 0);
	if (pressed && mouseTraite) {
		//Mise a jour des variables liées aux sliders
	}

	if (oldx || oldy) {
		g_renderer->_Camera->rotate(-(x - oldx) / 180.f);
		g_renderer->_Camera->rotateUp(-(y - oldy) / 180.f);
	}
	oldx = x;
	oldy = y;
}

void mouseMoveActiveFunction(int x, int y) {
	mouseMoveFunction(x, y, true);
}
void mouseMovePassiveFunction(int x, int y) {
	mouseMoveFunction(x, y, false);
}


void clickBtnParams(GUIBouton * bouton) {
	g_screen_manager->setActiveScreen(g_screen_params);
}

void clickBtnCloseParam(GUIBouton * bouton) {
	g_screen_manager->setActiveScreen(g_screen_jeu);
}

/**
  * POINT D'ENTREE PRINCIPAL
  **/
int main(int argc, char* argv[]) {
	LogConsole::createInstance();

	int screen_width = 800;
	int screen_height = 600;

	glutInit(&argc, argv);
	glutInitContextVersion(3, 0);
	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
		);

	glutInitWindowSize(screen_width, screen_height);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glEnable(GL_MULTISAMPLE);

	Log::log(Log::ENGINE_INFO, (toString(argc) + " arguments en ligne de commande.").c_str());
	bool gameMode = true;
	for (int i = 0; i < argc; i++) {
		if (argv[i][0] == 'w') {
			Log::log(Log::ENGINE_INFO, "Arg w mode fenetre.\n");
			gameMode = false;
		}
	}

	if (gameMode) {
		int width = glutGet(GLUT_SCREEN_WIDTH);
		int height = glutGet(GLUT_SCREEN_HEIGHT);

		char gameModeStr[200];
		sprintf(gameModeStr, "%dx%d:32@60", width, height);
		glutGameModeString(gameModeStr);
		g_main_window_id = glutEnterGameMode();
	}
	else {
		g_main_window_id = glutCreateWindow("MyNecraft");
		glutReshapeWindow(screen_width, screen_height);
	}

	if (g_main_window_id < 1) {
		Log::log(Log::ENGINE_ERROR, "Erreur creation de la fenetre.");
		exit(EXIT_FAILURE);
	}

	GLenum glewInitResult = glewInit();

	if (glewInitResult != GLEW_OK) {
		Log::log(Log::ENGINE_ERROR, ("Erreur init glew " + std::string((char*)glewGetErrorString(glewInitResult))).c_str());
		_cprintf("ERROR : %s", glewGetErrorString(glewInitResult));
		exit(EXIT_FAILURE);
	}

	//Affichage des capacités du système
	Log::log(Log::ENGINE_INFO, ("OpenGL Version : " + std::string((char*)glGetString(GL_VERSION))).c_str());

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
	g_renderer->setLightsFun(setLights);
	g_renderer->setBackgroundColor(NYColor());
	g_renderer->initialise();

	//On applique la config du renderer
	glViewport(0, 0, g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);
	g_renderer->resize(g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);

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
	y += 10;
	x += 10;

	GUILabel * label = new GUILabel();
	label->X = x;
	label->Y = y;
	label->Text = "Param :";
	g_screen_params->addElement(label);

	y += label->Height + 1;

	g_slider = new GUISlider();
	g_slider->setPos(x, y);
	g_slider->setMaxMin(1, 0);
	g_slider->Visible = true;
	g_screen_params->addElement(g_slider);

	y += g_slider->Height + 1;
	y += 10;

	//Ecran a rendre
	g_screen_manager->setActiveScreen(g_screen_jeu);

	//Fin init moteur

	//Init application

	g_world = new NYWorld();
	g_world->_FacteurGeneration = 5;
	g_world->init_world();
	g_world->add_world_to_vbo();

	//Init Avatar
	g_avatar = new NYAvatar(g_renderer->_Camera, g_world);
	g_avatar->Position = NYVert3Df(10, 10, 40);

	//Init Timer
	g_timer = new NYTimer();

	//On start
	g_timer->start();

	glutMainLoop();

	return 0;
}

