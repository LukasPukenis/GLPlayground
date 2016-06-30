#include "BSP.h"
#include <iostream>
#include <memory>
#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <array>
#include <tuple>
#include <regex>
#include "utils.h"
#include <SOIL.h>

void BSP::parseHeader() {
	// try to validate if directories are correct. 17 directories, each 2 ints, second int multiple of 4
	mapFile.seekg(4 + 4, mapFile.beg);
	for (auto i = 0; i < 17; i++) {		
		auto offset = readIntAndAdvance();
		auto len = readIntAndAdvance();

		header[i].length = len;
		header[i].offset = offset;
	}
}

// vertex offset
void BSP::readVertexes() {
	auto index = 10;
	auto len = header[index].length;	
	mapFile.seekg(header[index].offset);

	// we can't read vertexes as a whole because BSP_vertex structure contains additional fields, so we read it and parse it
	typedef struct {
		float a[3];
		float b[2][2];
		float c[3];
		unsigned char d[4];
	} bsp_vert_original;

	std::vector<bsp_vert_original> data;
	data.reserve(len/44);

	// as all the bsp vertex structures are as bytes inside vector, put them inside structures
	auto c = sizeof(BSP_vertex);
	for (unsigned int i = 0; i < len; i+=44) {
		auto vertex = bsp_vert_original();
		mapFile.read(reinterpret_cast<char*>(&vertex), sizeof(bsp_vert_original));
		data.push_back(vertex);
	}

	for (auto v : data) {
		auto & vert = BSP_vertex();
		vert = reinterpret_cast<BSP_vertex&>(v);
		vertexes.push_back(vert);
	}
}

void BSP::readMeshVertexes() {
	auto index = 11;
	auto len = header[index].length;
	mapFile.seekg(header[index].offset);

	meshVertexes.resize(len);
	mapFile.read(reinterpret_cast<char*>(&meshVertexes[0]), len);
}

int BSP::readIntAndAdvance() {
	unsigned char data[4];
	mapFile.read(reinterpret_cast<char*>(data), 4);
	auto value = reinterpret_cast<int&>(data);
	return value;
}

float BSP::readFloatAndAdvance() {
	unsigned char data[4];
	mapFile.read(reinterpret_cast<char*>(data), 4);
	auto value = reinterpret_cast<float&>(data);
	return value;
}

unsigned char BSP::readByteAndAdvance() {
	unsigned char data[1];
	mapFile.read(reinterpret_cast<char*>(data), 1);
	auto value = reinterpret_cast<unsigned char&>(data);
	return value;
}

void BSP::readLightmaps() {
	int index = 14;
	auto len = header[index].length;
	mapFile.seekg(header[index].offset);

	// by spec lightmaps are 128x128 always
	int const maxW = 128;
	int const maxH = 128;	

	glActiveTexture(GL_TEXTURE0+1);
	glGenTextures(1, &lightmapHandle);
	glBindTexture(GL_TEXTURE_2D_ARRAY, lightmapHandle);

	auto depth = len / sizeof(BSP_lightmap);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, GL_RGB, maxW, maxH, depth);	
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// mipmap parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	auto count = len / sizeof(BSP_lightmap);
	for (auto i = 0; i < count; i++) {
		BSP_lightmap lightmap;
		mapFile.read(reinterpret_cast<char*>(&lightmap), maxW*maxH*3);
		lightmaps.push_back(lightmap);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, maxW, maxH, 1, GL_RGB, GL_UNSIGNED_BYTE, lightmap.rgb.data());
	}

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void BSP::readTextures() {
	unsigned char data[64];
	int index = 1;
	auto len = header[index].length;
	mapFile.seekg(header[index].offset);

	int maxW = 256;
	int maxH = 256;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureHandle);	

	auto depth = len / sizeof(BSP_texture);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, GL_RGB, maxW, maxH, depth);	
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// mipmap parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	for (unsigned int i = 0; i < depth; i ++) {
		auto texture = BSP_texture();
		mapFile.read(reinterpret_cast<char*>(&texture), sizeof(BSP_texture));
		textures.push_back(texture);

		// textures are without extensions in a map, but there can be only jpg or tga so we check for both
		std::fstream texFile;
		std::string jpeg_filename = (std::string("./map/") + texture.name + ".jpg");
		std::string tga_filename = (std::string("./map/") + texture.name + ".tga");

		int width, height, channels;
		channels = 0;
		unsigned char * image = nullptr;
		texFile.open(jpeg_filename.c_str(), std::ios::in);
		bool isGoodJPEG = texFile.good();
		texFile.close();
		
		texFile.open(tga_filename.c_str(), std::ios::in);
		bool isGoodTGA = texFile.good();
		texFile.close();

		bool isRealFile = false;
		if (isGoodJPEG) {
			isRealFile = true;
			image = SOIL_load_image(jpeg_filename.c_str(), &width, &height, &channels, SOIL_LOAD_RGB);
		}
		else if(isGoodTGA) {
			isRealFile = true;
			image = SOIL_load_image(tga_filename.c_str(), &width, &height, &channels, SOIL_LOAD_RGB);
		}
		else {
			std::cout << texture.name << "[texture not found]" << std::endl;
		}
		
		if (isRealFile) {
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, image);			
			SOIL_free_image_data(image);
		}
	}
	
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	std::cout << std::endl;
}

void BSP::readFaces() {
	int index = 13;
	auto len = header[index].length;
	mapFile.seekg(header[index].offset);

	faces.resize(len/104);
	mapFile.read(reinterpret_cast<char*>(&faces[0]), len);
}

void BSP::readEntities() {
	std::vector<char> data(10 * 1024 * 1024);
	int i = 0;
	unsigned int offset = header[i].offset;
	unsigned int len = header[i].length;

	mapFile.seekg(offset, mapFile.beg);
	mapFile.read(&data[0], len);

	std::string ss(data.begin(), data.begin() + len);

	std::regex json_record_regex("\\{([^}]*)\\}", std::regex_constants::ECMAScript | std::regex_constants::icase);

	auto match_begin = std::sregex_iterator(ss.begin(), ss.end(), json_record_regex);

	std::regex key_value_regex("\"(.*)\" \"(.*)\"$", std::regex_constants::ECMAScript | std::regex_constants::icase);

	for (std::sregex_iterator i = match_begin; i != std::sregex_iterator(); ++i) {
		std::cout << "Match: " << (*i).str() << std::endl;

		auto m = (*i).str();
		auto key_val_match_begin = std::sregex_iterator(m.begin(), m.end(), key_value_regex);
		for (auto j = key_val_match_begin; j != std::sregex_iterator(); ++j) {
			auto keyval = (*j).str();

			std::regex keyval_regex("\"(.*?)\"", std::regex_constants::ECMAScript);
			std::regex float_regex("(\\d*[.]?\\d+)", std::regex_constants::ECMAScript);

			auto keyMatch = std::sregex_iterator(keyval.begin(), keyval.end(), keyval_regex);
			
			auto key = (*keyMatch)[1].str();

			std::string val1, val2, val3;

			if (key == "origin" || key == "_color") {
				auto match = std::sregex_iterator(keyval.begin(), keyval.end(), float_regex);
				val1 = (*match)[1].str();
				match++;
				val2 = (*match)[1].str();
				match++;
				val3 = (*match)[1].str();
				auto x = std::atof(val1.c_str());
				auto y = std::atof(val2.c_str());
				auto z = std::atof(val3.c_str());
				//entities[]
			}
			else if (key == "angle") {
				auto match = std::sregex_iterator(keyval.begin(), keyval.end(), float_regex);
				val1 = (*match)[1].str();
			}
			else {
				auto match = std::sregex_iterator(keyval.begin(), keyval.end(), keyval_regex);				
				val1 = (*match).str();
			}
			
			std::cout << "->" << key << " -> " << val1 << ":" << val2 << ":" << val3 << std::endl;
		}
	}
}

void BSP::debug() {
	std::cout << "<DEBUG>" << std::endl;
	std::cout << "Vertexes count " << std::distance(vertexes.begin(), vertexes.end()) << std::endl;
	std::cout << "</DEBUG>" << std::endl;
}

void out3(std::string & out, int x, int y, int z) {
	out += "[";
	out += std::to_string(x);
	out += ", ";
	out += std::to_string(y);
	out += ", ";
	out += std::to_string(z);
	out += "], ";
}

BSP::STATUS BSP::parse(const char * path) {
	mapFile.open(path, std::ios::binary);
	if (mapFile.fail()) return BSP::STATUS::CANT_OPEN_FILE;

	auto isValid = validateHeader(mapFile);
	if (!isValid) return BSP::STATUS::NOT_A_BSP_FILE;

	binaryLength = getIfstreamSize(mapFile);
	parseHeader();

	// readEntities();
	readVertexes();
	readMeshVertexes();
	readFaces();
	readTextures();
	readLightmaps();
	
	return BSP::STATUS::OK;
}

bool BSP::validateHeader(std::ifstream & stream) {
	char header[3];

	// ignore first byte, read "BSP", reset file pointer
	stream.seekg(1);
	stream.read(header, 3);
	stream.seekg(0);

	if (std::memcmp(header, "BSP", 3) == 0)	return true;

	return false;
}

unsigned int BSP::getIfstreamSize(std::ifstream & stream) {
	stream.seekg(0, std::ios::end);
	auto len = stream.tellg();

	stream.seekg(0);
	return binaryLength;
}