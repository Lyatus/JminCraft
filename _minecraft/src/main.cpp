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

	glEnable(GL_LIGHTING);

	//Speculaire
	GLfloat whiteSpecularMaterial[] = { 0.3, 0.3, 0.3, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteSpecularMaterial);
	GLfloat mShininess = 100;
	glMaterialf(GL_FRONT, GL_SHININESS, mShininess);

	//Emissive
	GLfloat black[] = { 0.0, 0.0, 0.0, 1.0 };

	//Ambient
	GLfloat materialAmbient[] = { 0.3, 0.3, 0.3, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	NYVert3Df lightPos(0, 0, 10), sunNorm(0, 1, 0);
	lightPos.rotate(sunNorm, NYRenderer::_DeltaTimeCumul / 10);
	float position[4] = { lightPos.X, lightPos.Y, lightPos.Z, 1 }; // w = 1 donc c'est une point light (w=0 -> directionelle, point à l'infini)
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	float diffuse[4] = { 0.5f, 0.5f, 0.5f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	glMaterialfv(GL_FRONT, GL_EMISSION, black);

	//A rajouter pour debug : rotation dans le temps
	glPushMatrix();
	glRotatef(NYRenderer::_DeltaTimeCumul * 100, g_slider->Value*10.0f, 1, cos(NYRenderer::_DeltaTimeCumul));
	glRotatef(45, 0, 0, 1);
	glTranslated(2, 0, 0);

	//Pour dessiner des quads
	glBegin(GL_QUADS);

	//Back face
	GLfloat backDiffuse[] = { 0, 1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, backDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, backDiffuse);
	glNormal3f(-1, 0, 0);
	glVertex3d(-1, -1, -1);
	glVertex3d(-1, -1, 1);
	glVertex3d(-1, 1, 1);
	glVertex3d(-1, 1, -1);


	//Front face
	GLfloat frontDiffuse[] = { 1.0, 0, 0, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, frontDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, frontDiffuse);
	glNormal3f(1, 0, 0);
	glVertex3d(1, -1, -1);
	glVertex3d(1, 1, -1);
	glVertex3d(1, 1, 1);
	glVertex3d(1, -1, 1);

	//Bottom face
	GLfloat bottomDiffuse[] = { 1.0, 1.0, 0, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, bottomDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, bottomDiffuse);
	glNormal3f(0, 0, -1);
	glVertex3d(-1, -1, -1);
	glVertex3d(-1, +1, -1);
	glVertex3d(+1, +1, -1);
	glVertex3d(+1, -1, -1);

	//Top face
	GLfloat topDiffuse[] = { 0, 0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, topDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, topDiffuse);
	glNormal3f(0, 0, 1);
	glVertex3d(-1, -1, +1);
	glVertex3d(+1, -1, +1);
	glVertex3d(+1, +1, +1);
	glVertex3d(-1, +1, +1);

	//Left face
	GLfloat leftDiffuse[] = { 1.0, 0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, leftDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, leftDiffuse);
	glNormal3f(0, -1, 0);
	glVertex3d(-1, -1, -1);
	glVertex3d(+1, -1, -1);
	glVertex3d(+1, -1, +1);
	glVertex3d(-1, -1, +1);

	//Right face
	GLfloat rightDiffuse[] = { 0, 1.0, 0, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, rightDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, rightDiffuse);
	glNormal3f(0, 1, 0);
	glVertex3d(-1, +1, -1);
	glVertex3d(-1, +1, +1);
	glVertex3d(+1, +1, +1);
	glVertex3d(+1, +1, -1);

	glEnd();

	GLfloat sphereDiffuse[] = { 1, 1, 1, 0.5 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, sphereDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, sphereDiffuse);
	glutSolidSphere(2, 16, 16);
	glPopMatrix();

	glPushMatrix();
	glTranslated(position[0], position[1], position[2]);
	GLfloat orange[] = { 1.0, 0.8, 0, 0.8 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, orange);
	glMaterialfv(GL_FRONT, GL_AMBIENT, orange);
	glMaterialfv(GL_FRONT, GL_EMISSION, orange);
	glutSolidSphere(.5, 8, 8);
	glPopMatrix();
}

void setLights(void) {
	//On active la light 0
	glEnable(GL_BLEND_SRC);
	glEnable(GL_LIGHT0);

	//On définit une lumière
	float specular[4] = { 0.5f, 0.5f, 0.5f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	float ambient[4] = { 0.1f, 0.1f, 0.1f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	NYColor skyColor(0, 181.f / 255.f, 221.f / 255.f, 1);
	g_renderer->setBackgroundColor(skyColor);
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
	else if (key == 'z') {
		g_renderer->_Camera->moveForward(.1f);
	}
}

void keyboardUpFunction(unsigned char key, int p1, int p2) {
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
		if (pressed) {
			g_renderer->_Camera->move(NYVert3Df((x - oldx) / 8.f, (y - oldy) / 8.f, 0));
		}
		else {
			g_renderer->_Camera->rotateAround(-(x - oldx) / 180.f);
			g_renderer->_Camera->rotateUpAround(-(y - oldy) / 180.f);
		}
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

	//Init Camera
	g_renderer->_Camera->setPosition(NYVert3Df(10, 10, 10));
	g_renderer->_Camera->setLookAt(NYVert3Df(0, 0, 0));


	//Fin init moteur

	//Init application



	//Init Timer
	g_timer = new NYTimer();

	//On start
	g_timer->start();

	glutMainLoop();

	return 0;
}

