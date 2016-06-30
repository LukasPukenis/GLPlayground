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
	GLbyte r, g, b, a;
	GLint textureIndex;
	GLint lightmapIndex;
};

struct BSP_texture {
	char name[64];
	int flags;
	int contents;
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

struct BSP_lightmap {
	std::array<GLbyte, 128*128*3> rgb;
};

typedef std::vector<BSP_face> face_t;
typedef std::vector<BSP_vertex> vertex_t;
typedef std::vector<unsigned int> meshVertex_t;
typedef std::vector<BSP_texture> texture_t;
typedef std::vector<BSP_lightmap> lightmap_t;

class BSP {
public:	
	enum class STATUS {
		CANT_OPEN_FILE,
		NOT_A_BSP_FILE,
		OK
	};
	BSP::STATUS parse(const char * path);
	void debug();

	GLuint const getTextureHandle() {
		return textureHandle;
	}

	GLuint const getLightmapHandle() {
		return lightmapHandle;
	}

	face_t & getFaces() {
		return faces;
	};

	vertex_t & getVertexes() {
		return vertexes;
	}

	lightmap_t & getLightmaps() {
		return lightmaps;
	}

	meshVertex_t & getMeshVertexes() {
		return meshVertexes;
	}
private:
	unsigned int getIfstreamSize(std::ifstream & stream);
	bool validateHeader(std::ifstream & stream);
	void parseHeader();
	void readEntities();
	void readTextures();
	void readVertexes();
	void readLightmaps();
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
	lightmap_t lightmaps;
	face_t faces;
	texture_t textures;

	GLuint textureHandle, lightmapHandle;
	std::map<std::string, std::shared_ptr<BSPEntity_default*>> entities;
	std::map<std::string, std::string> entityTargets;
};