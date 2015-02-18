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
		Cam->setPosition(NYVert3Df(5, 5, 5));
		Height = 1;
		Width = 1;
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
		glutSolidCube(Width);
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glVertex3d(0, 0, Height*.75f);
		glVertex3d(Cam->_Direction.X * 5, Cam->_Direction.Y * 5, Cam->_Direction.Z * 5);
		glEnd();
		glEnable(GL_LIGHTING);
		glPopMatrix();
	}

	void update(float elapsed) {
		//Par defaut, on applique la gravité (-100 sur Z)
		NYVert3Df force = NYVert3Df(0, 0, -1) * Gravity;

		//Si l'avatar n'est pas au sol, alors il ne peut pas sauter
		if (!Standing)
			Jump = false;


		//Si il est au sol, on applique les controles "ground"
		if (Standing) {
			if (avance)
				force += Cam->_Direction * MoveSpeed;
			if (recule)
				force += Cam->_Direction * -MoveSpeed;
			if (gauche)
				force += Cam->_NormVec * -MoveSpeed;
			if (droite)
				force += Cam->_NormVec * MoveSpeed;
		}
		else //Si il est en l'air, c'est du air control
		{
			if (avance)
				force += Cam->_Direction * AirMoveSpeed;
			if (recule)
				force += Cam->_Direction * -AirMoveSpeed;
			if (gauche)
				force += Cam->_NormVec * -AirMoveSpeed;
			if (droite)
				force += Cam->_NormVec * AirMoveSpeed;
		}

		//On applique le jump
		if (Jump) {
			force += NYVert3Df(0, 0, 1) * JumpImp / elapsed; //(impulsion, pas fonction du temps)
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

		//On recup la collision a la nouvelle position
		NYCollision collidePrinc = 0x00;
		NYCollision collide = World->collide_with_world(Position, Width, Height, collidePrinc);
		if (collide & NY_COLLIDE_BOTTOM && Speed.Z < 0) {
			Position.Z = oldPosition.Z;
			Speed *= pow(0.01f, elapsed);
			Speed.Z = 0;
			Standing = true;
		}
		else
			Standing = false;

		if (collide & NY_COLLIDE_UP && !Standing && Speed.Z > 0) {
			Position.Z = oldPosition.Z;
			Speed.Z = 0;
		}

		//On a regle le probleme du bottom et up, on gère les collision sur le plan (x,y)
		collide = World->collide_with_world(Position, Width, Height, collidePrinc);

		//En fonction des cotés, on annule une partie des déplacements
		if (collide & NY_COLLIDE_BACK && collide & NY_COLLIDE_RIGHT && collide & NY_COLLIDE_LEFT) {
			Position.Y = oldPosition.Y;
			Speed.Y = 0;
		}

		if (collide & NY_COLLIDE_FRONT && collide & NY_COLLIDE_RIGHT && collide & NY_COLLIDE_LEFT) {
			Position.Y = oldPosition.Y;
			Speed.Y = 0;
		}

		if (collide & NY_COLLIDE_RIGHT && collide & NY_COLLIDE_FRONT && collide & NY_COLLIDE_BACK) {
			Position.X = oldPosition.X;
			Speed.X = 0;
		}

		if (collide & NY_COLLIDE_LEFT && collide & NY_COLLIDE_FRONT && collide & NY_COLLIDE_BACK) {
			Position.X = oldPosition.X;
			Speed.X = 0;
		}

		//Si je collide sur un angle
		if (!(collide & NY_COLLIDE_BACK && collide & NY_COLLIDE_FRONT) && !(collide & NY_COLLIDE_LEFT && collide & NY_COLLIDE_RIGHT))
			if (collide & (NY_COLLIDE_BACK | NY_COLLIDE_FRONT | NY_COLLIDE_RIGHT | NY_COLLIDE_LEFT)) {
				Position.Y = oldPosition.Y;
				Position.X = oldPosition.X;
			}
		// Placement de la camera
		Cam->moveTo(Position + NYVert3Df(0, 0, Height * .75f));
	}
	void pick() {
		NYVert3Df point = 
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
		}
	}
};

#endif