#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <array>
#include <tuple>
#include <regex>
#include <map>

/*
Entities are:
worldspawn
info_player_deathmatch
info_player_intermission
target_speaker
trigger_teleport
info_notnull
light
item_armor_shard
misc_model
target_relay
trigger_multiple
item_armor_combat
item_health
ammo_lightning
weapon_lightning
item_health_large
ammo_shells
ammo_rockets
trigger_hurt
weapon_shotgun
trigger_push
misc_teleporter_dest
weapon_rocketlauncher
trigger_always
target_delay
*/

typedef unsigned char byte;

struct BSPEntity {
	std::string key;
};

struct BSPEntity_default: public BSPEntity {	
	std::string val;
};

struct BSPEntity_float : public BSPEntity {
	double val;
};

struct BSPEntity_vec3 : public BSPEntity {
	double x, y, z;
};

struct BSPHeader {
	unsigned int length;
	unsigned int offset;
};

// BSP vertex is not your usual vector of 3 components, nooo
struct BSP_vertex {
	GLfloat x, y, z;
	GLfloat texCoordX, texCoordY;
	GLfloat lmCoordX, lmCoordY;
	GLfloat normalX, normalY, normalZ;
	byte r, g, b, a;
};

struct BSP_face {
	int texture;	//	Texture index.
	int effect;	//	Index into lump 12 (Effects), or -1.
	int type;	//	Face type. 1 = polygon, 2 = patch, 3 = mesh, 4 = billboard
	int vertex;	//	Index of first vertex.
	int n_vertexes;	//	Number of vertices.
	int meshvert;	//	Index of first meshvert.
	int n_meshverts;	//	Number of meshverts.
	int lm_index;	//	Lightmap index.
	int lm_start[2];	//	Corner of this face's lightmap image in lightmap.
	int lm_size[2];	//	Size of this face's lightmap image in lightmap.
	float lm_origin[3];	//	World space origin of lightmap.
	float lm_vecs[2][3];	//	World space lightmap s and t unit vectors.
	float normal[3];	//	Surface normal.
	int size[2];	//	Patch dimensions.
};

typedef std::vector<std::shared_ptr<BSP_face>> face_t;
typedef std::vector<BSP_vertex> vertex_t;
typedef std::vector<unsigned int> meshVertex_t;

class BSP {
public:	
	enum class STATUS {
		CANT_OPEN_FILE,
		NOT_A_BSP_FILE,
		OK
	};
	BSP::STATUS parse(const char * path);
	void debug();

	face_t & getFaces() {
		return faces;
	};

	vertex_t & getVertexes() {
		return vertexes;
	}

	meshVertex_t & getMeshVertexes() {
		return meshVertexes;
	}
private:
	unsigned int getIfstreamSize(std::ifstream & stream);
	bool validateHeader(std::ifstream & stream);
	void parseHeader();
	void readEntities();
	void readVertexes();
	void readMeshVertexes();
	void readFaces();
	
	int readIntAndAdvance();
	float readFloatAndAdvance();
	unsigned char readByteAndAdvance();

	std::ifstream mapFile;
	unsigned int binaryLength;
	std::array<BSPHeader, 17> header;
	vertex_t vertexes;
	meshVertex_t meshVertexes;
	face_t faces;

	std::map<std::string, std::shared_ptr<BSPEntity_default*>> entities;
	std::map<std::string, std::string> entityTargets;
};