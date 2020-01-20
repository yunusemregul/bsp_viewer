#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <fstream>
#include <time.h>

// OpenGL
	// Include GLEW
	#include <GL/glew.h>

	// Include GLFW
	#include <GLFW/glfw3.h>
	GLFWwindow* window;

	// Include GLM
	#include <glm/glm.hpp>
	#include <glm/gtc/matrix_transform.hpp>
	using namespace glm;

	#include "controls.cpp"
//

// BSP
	#include "sourcesdk/bsp.h"
	#include "sourcesdk/lump.h"

	#define MAX_MAP_VERTS 65536
	#define MAX_MAP_PLANES 65536
	#define MAX_MAP_EDGES 256000	
	#define MAX_MAP_SURFEDGES 512000
	#define MAX_MAP_MODELS 1024
	#define MAX_MAP_FACES 65536
//

using std::fstream;
using std::cout;
using std::endl;
using std::cerr; 

using std::pow;
using std::sqrt;

using std::vector;
using std::ios;

// The struct that will hold the Map data
struct Map{
	float scale = 25;

	fstream file;

	dheader_t header;

	// lumps
	vector <dplane_t> planes;
	vector <Vector> vertexes;
	vector <dedge_t> edges;
	vector <signed int> surfedges;
	vector <dface_t> faces;
	vector <dmodel_t> models;

	vector <dDispVert> dispverts;
	vector <ddispinfo_t> dispinfos;

	vector <texinfo_t> texinfo;
	vector <dtexdata_t> texdata;

	template <class T>
	void readLump(int id, vector<T> &V);
	void loadLumps();
	void rescale();
};

const char *vertexShader = R"(
#version 330 core

uniform mat4 MVP;
in vec3 vPosition_modelspace;
in vec3 vColor;
out vec3 color;

void main()
{
	gl_Position =  MVP * vec4(vPosition_modelspace,1);
	color = vColor;
}
)";

const char *fragmentShader = R"(
#version 330 core
in vec3 color;

void main()
{ 
	gl_FragColor = vec4(color,1);
}
)";