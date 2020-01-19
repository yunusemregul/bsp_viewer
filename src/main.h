#include <iostream>
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

	#include <shaders/shader.cpp>
	#include <controls.cpp>
//

// BSP
	#include <sourcesdk/bsp.h>
	#include <sourcesdk/lump.h>

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

// The struct that will hold the Map data
struct Map{
	float scale = 100;

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

	template <class T>
	void readLump(int id, vector<T> &V);
	void loadLumps();
	void rescale();
};