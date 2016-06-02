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

void BSP::parseHeader() {
	//////// try to validate if directories are correct. 17 directories, each 2 ints, second int multiple of 4
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

	// as all the bsp vertex structures are as bytes inside vector, put them inside structures
	for (unsigned int i = 0; i < len; i+=44) {
		auto vertex = BSP_vertex();
		double K = 0.01;
		vertex.x = readFloatAndAdvance()*K;
		vertex.y = readFloatAndAdvance()*K;
		vertex.z = readFloatAndAdvance()*K;

		// swizzle
		auto temp = vertex.y;
		vertex.y = vertex.z;
		vertex.z = -temp;
		// end of swizzling

		vertex.texCoordX = readFloatAndAdvance();
		vertex.texCoordY = readFloatAndAdvance();

		vertex.lmCoordX = readFloatAndAdvance();
		vertex.lmCoordY = readFloatAndAdvance();

		vertex.normalX = readFloatAndAdvance();
		vertex.normalY = readFloatAndAdvance();
		vertex.normalZ = readFloatAndAdvance();

		vertex.r = readByteAndAdvance();
		vertex.g = readByteAndAdvance();
		vertex.b = readByteAndAdvance();
		vertex.a = readByteAndAdvance();

		vertexes.push_back(vertex);
	}
}

void BSP::readMeshVertexes() {
	auto index = 11;
	auto len = header[index].length;
	mapFile.seekg(header[index].offset);

	for (unsigned int i = 0; i < len; i+=4) {		
		meshVertexes.push_back(readIntAndAdvance());
	}
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

void BSP::readFaces() {
	int index = 13;
	auto len = header[index].length;
	mapFile.seekg(header[index].offset);

	auto s = sizeof(BSP_face);

	for (unsigned int i = 0; i < len; i += 104) {
		auto face = std::make_shared<BSP_face>();
		
		face->texture = readIntAndAdvance();
		face->effect  = readIntAndAdvance();
		face->type = readIntAndAdvance();
		face->vertex = readIntAndAdvance();
		face->n_vertexes = readIntAndAdvance();
		face->meshvert = readIntAndAdvance();
		face->n_meshverts = readIntAndAdvance();
		face->lm_index = readIntAndAdvance();
		face->lm_start[0] = readIntAndAdvance();
		face->lm_start[1] = readIntAndAdvance();
		face->lm_size[0] = readIntAndAdvance();
		face->lm_size[1] = readIntAndAdvance();
		face->lm_origin[0] = readFloatAndAdvance();
		face->lm_origin[1] = readFloatAndAdvance();
		face->lm_origin[2] = readFloatAndAdvance();

		face->lm_vecs[0][0] = readFloatAndAdvance();
		face->lm_vecs[0][1] = readFloatAndAdvance();
		face->lm_vecs[0][2] = readFloatAndAdvance();
		face->lm_vecs[1][0] = readFloatAndAdvance();
		face->lm_vecs[1][1] = readFloatAndAdvance();
		face->lm_vecs[1][2] = readFloatAndAdvance();

		face->normal[0] = readFloatAndAdvance();
		face->normal[1] = readFloatAndAdvance();
		face->normal[2] = readFloatAndAdvance();
		face->size[0] = readIntAndAdvance();
		face->size[1] = readIntAndAdvance();
		faces.push_back(face);
	}
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

	// parse lumps
	// readEntities();
	readVertexes();
	readMeshVertexes();
	readFaces();
	
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