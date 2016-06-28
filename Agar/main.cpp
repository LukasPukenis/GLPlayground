#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Drawable.h"
#include "Shader.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "Utils.h"
#include "BSP.h"
#include "Camera.h"

void APIENTRY openglCallbackFunction(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {

	std::cout << "---------------------opengl-callback-start------------" << std::endl;
	std::cout << "message: " << message << std::endl;
	std::cout << "type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		std::cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		std::cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		std::cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		std::cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		std::cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		std::cout << "OTHER";
		break;
	}
	std::cout << std::endl;

	std::cout << "id: " << id << std::endl;
	std::cout << "severity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		std::cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		std::cout << "HIGH";
		break;
	}
	std::cout << std::endl;
	std::cout << "---------------------opengl-callback-end--------------" << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

const GLuint WIDTH = 800, HEIGHT = 600;
GLFWwindow* window;
Camera camera;
double deltaTime, lastFrameTime;
int keysPressed[0xFF];

void setup() {
	camera.setScreenDimensions(WIDTH, HEIGHT);
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);

	// Create a GLFWwindow object that we can use for GLFW's functions
	window = glfwCreateWindow(WIDTH, HEIGHT, "BSP Map parser", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);
}

void processInputs() {
	if (keysPressed[GLFW_KEY_ESCAPE]) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (keysPressed[GLFW_KEY_S]) {
		camera.move(Camera::Forward);
	}

	if (keysPressed[GLFW_KEY_W]) {
		camera.move(Camera::Backward);
	}

	if (keysPressed[GLFW_KEY_A]) {
		camera.move(Camera::Left);
	}

	if (keysPressed[GLFW_KEY_D]) {
		camera.move(Camera::Right);
	}
}
/*
screenshots:
http://puu.sh/pB6Ob/18a49c192e.jpg
http://puu.sh/pB6Pr/6d0a1ff2ae.jpg
http://puu.sh/pB6RY/f1ae00a181.jpg
http://puu.sh/pB6VT/8d06cf59a8.jpg

http://puu.sh/pB7iF/9d489c22f7.jpg
*/

/*
BSP_vertex cubicBezier(const BSP_vertex & c0, const BSP_vertex & c1, const BSP_vertex & c2, float t) {
	auto _c0 = glm::vec3(c0.x, c0.y, c0.z);
	auto _c1 = glm::vec3(c1.x, c1.y, c1.z);
	auto _c2 = glm::vec3(c2.x, c2.y, c2.z);

	float b = 1.0 - t;
	auto dist = b * _c0 + 2 * b*t*_c1 + t*t*_c2;
	BSP_vertex final;
	final.x = dist.x;
	final.y = dist.y;
	final.z = dist.z;
	return final;
}
*/
glm::vec3 cubicBezier(const glm::vec3 & c0, const glm::vec3 & c1, const glm::vec3 & c2, float t) {
	float b = 1.0 - t;
	return b * c0 + 2*b*t*c1 + t*t*c2;
}

void run() {
	// Build and compile our shader program
	Shader ourShader("basic.vert", "basic.frag");

	BSP bsp;
	auto result = bsp.parse("map/maps/q3dm2.bsp");
	auto & faces = bsp.getFaces();
	auto & meshVertexes = bsp.getMeshVertexes();

	typedef GLuint element_t;
	std::vector<element_t> elements;

	bsp.debug();

	auto & vertexes = bsp.getVertexes();

	// extract polygons from faces and draw them
	auto total = 0;
	auto polygon = faces.begin();
	auto faceCnt = 0;

	std::vector<unsigned int> indexes;
	
	std::sort(faces.begin(), faces.end(), [&](const auto & a, const auto & b) {
		return a->type < b->type;
	});

	while (polygon != faces.end()) {
		faceCnt++;
		polygon = std::find_if(polygon + 1, faces.end(), [&](const auto & item) {
			return item->type == 1 || item->type == 2;
		});
		
		if (polygon == faces.end()) break;
		auto face = *polygon;
		auto type = face->type;

		int vertex = face->vertex;
		int length = face->n_meshverts;
		int offset = face->meshvert;
		auto _off = vertexes.size();

		if (type == 1 || type == 3) {
			for (auto i = 0; i < length; i++) {
				auto index = vertex + meshVertexes[i + offset];
				vertexes[index].textureIndex = face->texture;
				vertexes[index].lightmapIndex = face->lm_index;
				indexes.push_back(index);
			}

			total += length;
			elements.push_back(offset);
		}	
		else if (type == 2) {			
			for (int py = 0; py < face->size[1] - 2; py += 2) {				
				for (int px = 0; px < face->size[0] - 2; px += 2) {
					auto indexOffset = vertexes.size();
					int rowOff = face->size[0] * py;
					int off = face->vertex;

					auto c1 = off + rowOff + px;
					auto c2 = off + rowOff + px + 1;
					auto c3 = off + rowOff + px + 2;

					rowOff += face->size[0];

					auto c4 = off + rowOff + px;
					auto c5 = off + rowOff + px + 1;
					auto c6 = off + rowOff + px + 2;

					rowOff += face->size[0];

					auto c7 = off + rowOff + px;
					auto c8 = off + rowOff + px + 1;
					auto c9 = off + rowOff + px + 2;
					
					std::array<BSP_vertex, 9> controls;

					controls[0] = vertexes[c1];
					controls[1] = vertexes[c2];
					controls[2] = vertexes[c3];

					controls[3] = vertexes[c4];
					controls[4] = vertexes[c5];
					controls[5] = vertexes[c6];

					controls[6] = vertexes[c7];
					controls[7] = vertexes[c8];
					controls[8] = vertexes[c9];

					auto L = 8;
					auto L1 = L + 1;

					for (auto i = 0; i < L1; ++i) {
						float a = (float)i / L;
						float b = 1 - a;
						
						BSP_vertex v;

						glm::vec3 _pos1 = glm::vec3(controls[0].x, controls[0].y, controls[0].z);
						glm::vec3 _pos2 = glm::vec3(controls[3].x, controls[3].y, controls[3].z);
						glm::vec3 _pos3 = glm::vec3(controls[6].x, controls[6].y, controls[6].z);
						
						glm::vec3 _tex1 = glm::vec3(controls[0].texCoordX, controls[0].texCoordY, 0.0);
						glm::vec3 _tex2 = glm::vec3(controls[3].texCoordX, controls[3].texCoordY, 0.0);
						glm::vec3 _tex3 = glm::vec3(controls[6].texCoordX, controls[6].texCoordY, 0.0);

						glm::vec3 _lm1 = glm::vec3(controls[0].lmCoordX, controls[0].lmCoordY, 0.0);
						glm::vec3 _lm2 = glm::vec3(controls[3].lmCoordX, controls[3].lmCoordY, 0.0);
						glm::vec3 _lm3 = glm::vec3(controls[6].lmCoordX, controls[6].lmCoordY, 0.0);

						auto pos = Utils::bezier3(_pos1, _pos2, _pos3, a);
						auto tex = Utils::bezier3(_tex1, _tex2, _tex3, a);
						auto lm = Utils::bezier3(_lm1, _lm2, _lm3, a);

						v.x = pos.x;
						v.y = pos.y;
						v.z = pos.z;

						v.texCoordX = tex.x;
						v.texCoordY = tex.y;

						v.lmCoordX = lm.x;
						v.lmCoordY = lm.y;

						v.textureIndex = face->texture;
						v.lightmapIndex = face->lm_index;

						vertexes.push_back(v);
					}

					for (auto i = 1; i < L1; ++i) {
						float a = (float)i / L;
						float b = 1.0 - a;

						BSP_vertex temp[3];

						int j;
						for (j = 0; j < 3; ++j) {
							int k = 3 * j;
							
							BSP_vertex v;

							glm::vec3 _pos1 = glm::vec3(controls[k+0].x, controls[k+0].y, controls[k+0].z);
							glm::vec3 _pos2 = glm::vec3(controls[k+1].x, controls[k + 1].y, controls[k + 1].z);
							glm::vec3 _pos3 = glm::vec3(controls[k + 2].x, controls[k + 2].y, controls[k + 2].z);

							glm::vec3 _tex1 = glm::vec3(controls[k + 0].texCoordX, controls[k + 0].texCoordY, 0.0);
							glm::vec3 _tex2 = glm::vec3(controls[k + 1].texCoordX, controls[k + 1].texCoordY, 0.0);
							glm::vec3 _tex3 = glm::vec3(controls[k + 2].texCoordX, controls[k + 2].texCoordY, 0.0);

							glm::vec3 _lm1 = glm::vec3(controls[k + 0].lmCoordX, controls[k + 0].lmCoordY, 0.0);
							glm::vec3 _lm2 = glm::vec3(controls[k + 1].lmCoordX, controls[k + 1].lmCoordY, 0.0);
							glm::vec3 _lm3 = glm::vec3(controls[k + 2].lmCoordX, controls[k + 2].lmCoordY, 0.0);

							auto pos = Utils::bezier3(_pos1, _pos2, _pos3, a);
							auto tex = Utils::bezier3(_tex1, _tex2, _tex3, a);
							auto lm = Utils::bezier3(_lm1, _lm2, _lm3, a);

							v.x = pos.x;
							v.y = pos.y;
							v.z = pos.z;

							v.texCoordX = tex.x;
							v.texCoordY = tex.y;

							v.lmCoordX = lm.x;
							v.lmCoordY = lm.y;

							temp[j] = v;
						}

						for (j = 0; j < L1; ++j) {
							float a = (float)j / L;
							float b = 1.0 - a;
							
							BSP_vertex v;

							glm::vec3 _pos1 = glm::vec3(temp[0].x, temp[0].y, temp[0].z);
							glm::vec3 _pos2 = glm::vec3(temp[1].x, temp[1].y, temp[1].z);
							glm::vec3 _pos3 = glm::vec3(temp[2].x, temp[2].y, temp[2].z);

							glm::vec3 _tex1 = glm::vec3(temp[0].texCoordX, temp[0].texCoordY, 0.0);
							glm::vec3 _tex2 = glm::vec3(temp[1].texCoordX, temp[1].texCoordY, 0.0);
							glm::vec3 _tex3 = glm::vec3(temp[2].texCoordX, temp[2].texCoordY, 0.0);

							glm::vec3 _lm1 = glm::vec3(temp[0].lmCoordX, temp[0].lmCoordY, 0.0);
							glm::vec3 _lm2 = glm::vec3(temp[1].lmCoordX, temp[1].lmCoordY, 0.0);
							glm::vec3 _lm3 = glm::vec3(temp[2].lmCoordX, temp[2].lmCoordY, 0.0);

							auto pos = Utils::bezier3(_pos1, _pos2, _pos3, a);
							auto tex = Utils::bezier3(_tex1, _tex2, _tex3, a);
							auto lm = Utils::bezier3(_lm1, _lm2, _lm3, a);

							v.x = pos.x;
							v.y = pos.y;
							v.z = pos.z;

							v.texCoordX = tex.x;
							v.texCoordY = tex.y;

							v.lmCoordX = lm.x;
							v.lmCoordY = lm.y;

							v.textureIndex = face->texture;
							v.lightmapIndex = face->lm_index;

							vertexes.push_back(v);
						}
					}
					
					for (int row = 0; row < L; ++row) {
						for (int col = 0; col < L; ++col) {
							indexes.push_back(indexOffset + (row + 1) * L1 + col);
							indexes.push_back(indexOffset + row * L1 + col);
							indexes.push_back(indexOffset + row * L1 + (col + 1));

							indexes.push_back(indexOffset + (row + 1) * L1 + col);
							indexes.push_back(indexOffset + row * L1 + (col + 1));
							indexes.push_back(indexOffset + (row + 1) * L1 + (col + 1));
						}
					}
					
					///////////////////////////////////
				}
			}
		}
	}

	/////////////////////////////////////
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(openglCallbackFunction, nullptr);
	
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indexes.size(), &indexes[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(BSP_vertex), &vertexes[0], GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BSP_vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Color attribute
	glVertexAttribPointer(1, 4, GL_BYTE, GL_FALSE, sizeof(BSP_vertex), (GLvoid*)((10 * sizeof(GL_FLOAT))));
	glEnableVertexAttribArray(1);
	
	// Texel attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BSP_vertex), (GLvoid*)((3 * sizeof(GL_FLOAT))));
	glEnableVertexAttribArray(2);

	// Texture array depth/id attribute
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(BSP_vertex), (GLvoid*)((4 * sizeof(GLbyte)) + (10 * sizeof(GL_FLOAT))));
	glEnableVertexAttribArray(3);

	// Lightmap array depth/id attribute
	glVertexAttribIPointer(4, 1, GL_INT, sizeof(BSP_vertex), (GLvoid*)((1*sizeof(GLint))+(4 * sizeof(GLbyte)) + (10 * sizeof(GL_FLOAT))));
	glEnableVertexAttribArray(4);

	// Lightmap uv
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(BSP_vertex), (GLvoid*)(5 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(5);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(0); // Unbind VAO

	glm::mat4 projection = camera.getProjection();
	camera.setPosition(glm::vec3(0.0, 0.0, 40.0));

	ourShader.Use();
	auto texLocation = glGetUniformLocation(ourShader.Program, "_texture");
	auto lmLocation = glGetUniformLocation(ourShader.Program, "_lightmap");

	glUniform1i(texLocation, 0);
	glUniform1i(lmLocation, 1);

	//assert(texLocation != -1, "texLocation uniform was optimized");
	//assert(lmLocation != -1, "lmLocation uniform was optimized");

	while (!glfwWindowShouldClose(window))
	{		
		deltaTime = glfwGetTime() - lastFrameTime;
		lastFrameTime = glfwGetTime();

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		processInputs();

		// Render
		// Clear the colorbuffer
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, bsp.getTextureHandle());

		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D_ARRAY, bsp.getLightmapHandle());
		
		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
		GLint angleLoc = glGetUniformLocation(ourShader.Program, "angle");

		glm::mat4 view;
		camera.setDeltaTime(deltaTime);
		view = camera.getView();

		// Get their uniform location
		glUniform1f(angleLoc, glfwGetTime());
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glm::mat4 model = glm::mat4(1.0);	// just identity

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		glDrawElements(GL_TRIANGLES, indexes.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	// Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	keysPressed[key] = action;
}

bool firstMouse = true;
double lastMouseX, lastMouseY;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) {
		lastMouseX = xpos;
		lastMouseY = ypos;
		firstMouse = false;
	}

	auto sensitivity = 0.5;

	auto diffX = xpos - lastMouseX;
	auto diffY = lastMouseY - ypos;
	lastMouseX = xpos;
	lastMouseY = ypos;

	diffX *= sensitivity;
	diffY *= sensitivity;

	std::cout << diffY << std::endl;
	camera.adjustPitch(diffY);
	camera.adjustYaw(diffX);
}

int main()
{
	setup();
	run();

	return 0;
}