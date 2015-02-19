#ifndef __AVATAR__
#define __AVATAR__

#include "engine/utils/types_3d.h"
#include "engine/render/camera.h"
#include "world.h"
#include "my_physics.h"

class NYAvatar
{
public:
	NYVert3Df Position;
	NYVert3Df Speed;

	NYVert3Df MoveDir;
	bool Move;
	bool Jump;
	float Height;
	float Width;
	float MoveSpeed;
	float AirMoveSpeed;
	float Gravity;
	float JumpImp;
	bool avance;
	bool recule;
	bool gauche;
	bool droite;
	bool Standing;

	NYCamera * Cam;
	NYWorld * World;

	NYAvatar(NYCamera * cam, NYWorld * world) {
		Cam = cam;
		World = world;
		Cam->setLookAt(Position = NYVert3Df(0, 0, 0));
		Cam->setPosition(NYVert3Df(32, 32, 32));
		Height = 5;
		Width = 5;
		MoveSpeed = 32;
		AirMoveSpeed = 4;
		Gravity = 9.8;
		JumpImp = 10;
		avance = false;
		recule = false;
		gauche = false;
		droite = false;
		Standing = false;
		Jump = false;
	}


	void render(void) {
		glPushMatrix();
		glTranslated(Position.X, Position.Y, Position.Z);
		glColor3f(1, 1, 0);
		glutSolidCube(Width);
		glPopMatrix();
	}

	void update(float elapsed) {
		//Par defaut, on applique la gravité (-100 sur Z)
		NYVert3Df force = NYVert3Df(0, 0, -1) * 100.0f;

		//Si l'avatar n'est pas au sol, alors il ne peut pas sauter
		if (!Standing)
			Jump = false;


		//Si il est au sol, on applique les controles "ground"
		if (Standing) {
			if (avance)
				force += Cam->_Direction * 400;
			if (recule)
				force += Cam->_Direction * -400;
			if (gauche)
				force += Cam->_NormVec * -400;
			if (droite)
				force += Cam->_NormVec * 400;
		}
		else //Si il est en l'air, c'est du air control
		{
			if (avance)
				force += Cam->_Direction * 50;
			if (recule)
				force += Cam->_Direction * -50;
			if (gauche)
				force += Cam->_NormVec * -50;
			if (droite)
				force += Cam->_NormVec * 50;
		}

		//On applique le jump
		if (Jump) {
			force += NYVert3Df(0, 0, 1) * 50.0f / elapsed; //(impulsion, pas fonction du temps)
			Jump = false;
		}

		//On applique les forces en fonction du temps écoulé
		Speed += force * elapsed;

		//On met une limite a sa vitesse horizontale
		NYVert3Df horSpeed = Speed;
		horSpeed.Z = 0;
		if (horSpeed.getSize() > 70.0f) {
			horSpeed.normalize();
			horSpeed *= 70.0f;
			Speed.X = horSpeed.X;
			Speed.Y = horSpeed.Y;
		}

		//On le déplace, en sauvegardant son ancienne position
		NYVert3Df oldPosition = Position;
		Position += (Speed * elapsed);

		Standing = false;

		for (int i = 0; i < 3; i++) {
			float valueColMin = 0;
			NYAxis axis = World->getMinCol(Position, Width, Height, valueColMin, i);
			if (axis != 0) {
				valueColMin = max(abs(valueColMin*1.05f), 0.0001f) * (valueColMin > 0 ? 1.0f : -1.0f);
				if (axis & NY_AXIS_X) {
					Position.X += valueColMin;
					Speed.X = 0;
				}
				if (axis & NY_AXIS_Y) {
					Position.Y += valueColMin;
					Speed.Y = 0;
				}
				if (axis & NY_AXIS_Z) {
					Speed.Z = 0;
					Position.Z += valueColMin;
					Speed *= pow(0.01f, elapsed);
					Standing = true;
				}
			}
		}
		// Placement de la camera
		Cam->moveLookAt(Position + NYVert3Df(0, 0, Height * .75f));
	}
	void pick() {
		NYVert3Df inter;
		int x, y, z;
		if (World->getRayCollision(Position, Position + Cam->_Direction * 128, inter, x, y, z)) {
			World->setCube(x, y, z, CUBE_AIR);
			World->updateCube(x, y, z);
		}
	}
	void jump() {
		if (!Jump) {
			Jump = true;
			Speed.Z = 10;
		}
	}
	void key(unsigned char key, bool down) {
		switch (key) {
			case 'z': avance = down; break;
			case 's': recule = down; break;
			case 'q': gauche = down; break;
			case 'd': droite = down; break;
			case ' ': jump(); break;
			case 'a': if (down) pick(); break;
		}
	}
};

#endif