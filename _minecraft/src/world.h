#ifndef __WORLD_H__
#define __WORLD_H__

#include "gl/glew.h"
#include "gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "cube.h"
#include "chunk.h"


typedef uint8 NYCollision;
#define NY_COLLIDE_UP     0x01
#define NY_COLLIDE_BOTTOM 0x02
#define NY_COLLIDE_LEFT   0x04
#define NY_COLLIDE_RIGHT  0x08
#define NY_COLLIDE_FRONT  0x10
#define NY_COLLIDE_BACK   0x20
#define NY_COLLIDE_IN     0x40

#define MAT_SIZE 16 //en nombre de chunks
#define MAT_HEIGHT 1 //en nombre de chunks
#define MAT_SIZE_CUBES (MAT_SIZE * NYChunk::CHUNK_SIZE)
#define MAT_HEIGHT_CUBES (MAT_HEIGHT * NYChunk::CHUNK_SIZE)


class NYWorld
{
public:
	NYChunk * _Chunks[MAT_SIZE][MAT_SIZE][MAT_HEIGHT];
	int _MatriceHeights[MAT_SIZE_CUBES][MAT_SIZE_CUBES];
	float _FacteurGeneration;

	NYWorld() {
		_FacteurGeneration = 1.0;

		//On crée les chunks
		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++)
					_Chunks[x][y][z] = new NYChunk();

		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++) {
					NYChunk * cxPrev = NULL;
					if (x > 0)
						cxPrev = _Chunks[x - 1][y][z];
					NYChunk * cxNext = NULL;
					if (x < MAT_SIZE - 1)
						cxNext = _Chunks[x + 1][y][z];

					NYChunk * cyPrev = NULL;
					if (y > 0)
						cyPrev = _Chunks[x][y - 1][z];
					NYChunk * cyNext = NULL;
					if (y < MAT_SIZE - 1)
						cyNext = _Chunks[x][y + 1][z];

					NYChunk * czPrev = NULL;
					if (z > 0)
						czPrev = _Chunks[x][y][z - 1];
					NYChunk * czNext = NULL;
					if (z < MAT_HEIGHT - 1)
						czNext = _Chunks[x][y][z + 1];

					_Chunks[x][y][z]->setVoisins(cxPrev, cxNext, cyPrev, cyNext, czPrev, czNext);
				}


	}

	inline NYCube * getCube(int x, int y, int z) {
		if (x < 0)x = 0;
		if (y < 0)y = 0;
		if (z < 0)z = 0;
		if (x >= MAT_SIZE * NYChunk::CHUNK_SIZE) x = (MAT_SIZE * NYChunk::CHUNK_SIZE) - 1;
		if (y >= MAT_SIZE * NYChunk::CHUNK_SIZE) y = (MAT_SIZE * NYChunk::CHUNK_SIZE) - 1;
		if (z >= MAT_HEIGHT * NYChunk::CHUNK_SIZE) z = (MAT_HEIGHT * NYChunk::CHUNK_SIZE) - 1;

		return &(_Chunks[x / NYChunk::CHUNK_SIZE][y / NYChunk::CHUNK_SIZE][z / NYChunk::CHUNK_SIZE]->_Cubes[x % NYChunk::CHUNK_SIZE][y % NYChunk::CHUNK_SIZE][z % NYChunk::CHUNK_SIZE]);
	}

	void updateCube(int x, int y, int z) {
		if (x < 0)x = 0;
		if (y < 0)y = 0;
		if (z < 0)z = 0;
		if (x >= MAT_SIZE * NYChunk::CHUNK_SIZE)x = (MAT_SIZE * NYChunk::CHUNK_SIZE) - 1;
		if (y >= MAT_SIZE * NYChunk::CHUNK_SIZE)y = (MAT_SIZE * NYChunk::CHUNK_SIZE) - 1;
		if (z >= MAT_HEIGHT * NYChunk::CHUNK_SIZE)z = (MAT_HEIGHT * NYChunk::CHUNK_SIZE) - 1;
		_Chunks[x / NYChunk::CHUNK_SIZE][y / NYChunk::CHUNK_SIZE][z / NYChunk::CHUNK_SIZE]->toVbo();
	}
	void setCube(int x, int y, int z, NYCubeType type, bool draw = true) {
		NYCube *cube = getCube(x, y, z);
		cube->_Draw = draw;
		cube->_Type = type;
	}
	void deleteCube(int x, int y, int z) {
		NYCube * cube = getCube(x, y, z);
		cube->_Draw = false;
		cube = getCube(x - 1, y, z);
	}

	//Création d'une pile de cubes
	//only if zero permet de ne générer la  pile que si sa hauteur actuelle est de 0 (et ainsi de ne pas regénérer de piles existantes)
	void load_pile(int x, int y, int height, bool onlyIfZero = true) {
		if (onlyIfZero && _MatriceHeights[x][y] > 0) return;
		else _MatriceHeights[x][y] = height;
		for (int z = 0; z < MAT_HEIGHT_CUBES; z++)
			if (z == 0)
				setCube(x, y, z, CUBE_EAU);
			else if (z < height)
				setCube(x, y, z, CUBE_TERRE);
			else if (z == height)
				setCube(x, y, z, CUBE_HERBE);
			else
				setCube(x, y, z, CUBE_AIR);

	}

	//Creation du monde entier, en utilisant le mouvement brownien fractionnaire
	void generate_piles(int x1, int y1,
		int x2, int y2,
		int x3, int y3,
		int x4, int y4, int prof, int profMax = -1) {
		if (x1 + 1 >= x2) return;

		int x12 = (x1 + x2) / 2;
		int x23 = (x2 + x3) / 2;
		int x34 = (x3 + x4) / 2;
		int x14 = (x1 + x4) / 2;
		int y12 = (y1 + y2) / 2;
		int y23 = (y2 + y3) / 2;
		int y34 = (y3 + y4) / 2;
		int y14 = (y1 + y4) / 2;
		int xc = x12;
		int yc = y14;

		load_pile(x12, y12, inter_piles(_MatriceHeights[x1][y1], _MatriceHeights[x2][y2], abs(x1 - x2)));
		load_pile(x23, y23, inter_piles(_MatriceHeights[x2][y2], _MatriceHeights[x3][y3], abs(y2 - y3)));
		load_pile(x34, y34, inter_piles(_MatriceHeights[x3][y3], _MatriceHeights[x4][y4], abs(x3 - x4)));
		load_pile(x14, y14, inter_piles(_MatriceHeights[x1][y1], _MatriceHeights[x4][y4], abs(y1 - y4)));
		load_pile(xc, yc, inter_piles(_MatriceHeights[x12][y12], _MatriceHeights[x34][y34], abs(y12 - y34)));

		if (prof == profMax) return;

		generate_piles(x1, y1, x12, y12, xc, yc, x14, y14, prof + 1, profMax);
		generate_piles(x12, y12, x2, y2, x23, y23, xc, yc, prof + 1, profMax);
		generate_piles(xc, yc, x23, y23, x3, y3, x34, y34, prof + 1, profMax);
		generate_piles(x14, y14, xc, yc, x34, y34, x4, y4, prof + 1, profMax);
	}
	int inter_piles(int h1, int h2, int dist) {
		int wtr = (h1 + h2) / 2;
		int mod = min(dist / 3, MAT_HEIGHT_CUBES / 2);
		if (mod) wtr += (rand() % mod) - mod / 2;
		return wtr;
	}

	int _NewHeights[MAT_SIZE_CUBES][MAT_SIZE_CUBES];
	void lisse(void) {
		int s = 4;
		for (int x = 0; x < MAT_SIZE_CUBES; x++)
			for (int y = 0; y < MAT_SIZE_CUBES; y++) {
				int newh = 0, c = 0;
				for (int xi = x - s; xi <= x + s; xi++)
					for (int yi = y - s; yi <= y + s; yi++)
						if (xi != x && yi != y
							&& xi >= 0 && yi >= 0
							&& xi < MAT_SIZE_CUBES && yi < MAT_SIZE_CUBES) {
							newh += _MatriceHeights[xi][yi];
							c++;
						}
				newh /= c;
				_NewHeights[x][y] = newh;
			}
		for (int x = 0; x < MAT_SIZE_CUBES; x++)
			for (int y = 0; y < MAT_SIZE_CUBES; y++)
				load_pile(x, y, _NewHeights[x][y], false);
	}




	void init_world(int profmax = -1) {
		_cprintf("Creation du monde %f \n", _FacteurGeneration);

		srand(6665);

		//Reset du monde
		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++)
					_Chunks[x][y][z]->reset();
		memset(_MatriceHeights, 0x00, MAT_SIZE_CUBES*MAT_SIZE_CUBES*sizeof(int));

		//On charge les 4 coins
		load_pile(0, 0, MAT_HEIGHT_CUBES / 2);
		load_pile(MAT_SIZE_CUBES - 1, 0, MAT_HEIGHT_CUBES / 2);
		load_pile(MAT_SIZE_CUBES - 1, MAT_SIZE_CUBES - 1, MAT_HEIGHT_CUBES / 2);
		load_pile(0, MAT_SIZE_CUBES - 1, MAT_HEIGHT_CUBES / 2);

		//On génère a partir des 4 coins
		generate_piles(0, 0,
			MAT_SIZE_CUBES - 1, 0,
			MAT_SIZE_CUBES - 1, MAT_SIZE_CUBES - 1,
			0, MAT_SIZE_CUBES - 1, 1, profmax);

		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++)
					_Chunks[x][y][z]->disableHiddenCubes();
	}

	NYCube * pick(NYVert3Df  pos, NYVert3Df  dir, NYPoint3D * point) {
		return NULL;
	}

	//Boites de collisions plus petites que deux cubes
	NYCollision collide_with_world(NYVert3Df pos, float width, float height, NYCollision & collisionPrincipale) {
		NYCollision collision = 0;
		return collision;
	}

	void render_world_vbo(void) {
		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++) {
					glPushMatrix();
					glTranslatef((float)(x*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE), (float)(y*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE), (float)(z*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE));
					_Chunks[x][y][z]->render();
					glPopMatrix();
				}
	}

	void add_world_to_vbo(void) {
		int totalNbVertices = 0;

		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++) {
					_Chunks[x][y][z]->toVbo();
					totalNbVertices += _Chunks[x][y][z]->_NbVertices;
				}

		Log::log(Log::ENGINE_INFO, (toString(totalNbVertices) + " vertices in VBO").c_str());
	}

	void render_world_old_school(void) {
		for (int x = 0; x < MAT_SIZE_CUBES; x++)
			for (int y = 0; y < MAT_HEIGHT_CUBES; y++)
				for (int z = 0; z < MAT_SIZE_CUBES; z++) {
					NYCube * cube = getCube(x, y, z);
					if (cube->_Draw && cube->isSolid()) {
						glPushMatrix();
						glTranslated(x, y, z);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, cube->getColor());
						glutSolidCube(1);
						glPopMatrix();
					}
				}
	}
};



#endif