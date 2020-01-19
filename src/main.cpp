/*
	notes:
		* displacements does not work as expected, having problems with rotation and scale	
*/

#include <main.h>

template <class T>
void Map::ReadLump(int id, vector<T> &V)
{
	for(int i=0; i<header.lumps[id].filelen/sizeof(T); i++)
	{
		T to_push;
		file.seekg(header.lumps[id].fileofs+i*sizeof(T),ios::beg);
		file.read((char *)&to_push, sizeof(T));
		V.push_back(to_push);
	}
	cout << "> LUMP ID: "<< id << "\tcount: " << V.size() << endl;
}

void Map::LoadLumps()
{
	// Reading lumps
	ReadLump<dplane_t>(LUMP_PLANES, planes);
	ReadLump<Vector>(LUMP_VERTEXES, vertexes);
	ReadLump<dedge_t>(LUMP_EDGES, edges);
	ReadLump<signed int>(LUMP_SURFEDGES, surfedges);
	ReadLump<dface_t>(LUMP_FACES,faces);
	ReadLump<dmodel_t>(LUMP_MODELS,models);
	ReadLump<texinfo_t>(LUMP_TEXINFO,texinfo);
	ReadLump<dDispVert>(LUMP_DISP_VERTS,dispverts);
	ReadLump<ddispinfo_t>(LUMP_DISPINFO,dispinfos);
}

void Map::Rescale()
{
	// scaling down the map
	for(int i=0;i<vertexes.size();i++)
		vertexes[i] = vertexes[i]/scale;

	for(int i=0;i<planes.size();i++)
		planes[i].dist /= scale;
		//planes[i].normal = planes[i].normal/scale; 

	for(int i=0; i<faces.size(); i++)
		faces[i].area /= pow(scale,2);

	for(int i=0; i<dispinfos.size(); i++)
		dispinfos[i].startPosition = dispinfos[i].startPosition / scale;
}

int main(int argc, char *argv[])
{
	if(argc<2)
	{
		cerr << "Usage: " << endl << argv[0] << " gm_flatgrass.bsp" << endl;
		return -1;
	}

	char *ext = strrchr(argv[1],'.');
	if(ext==NULL || strcmp(ext,".bsp")!=0)
	{
		cerr << "File extension must be bsp!" << endl;
		return -1;
	}

	Map map;

	map.file.open(argv[1], fstream::in | fstream::binary);

	if(!map.file.good())
	{
		cerr << "BSP file not found!" << endl;
		return -1;
	}
	
	if(!glfwInit())
	{
		cerr << "Failed to init GLFW!" << endl;
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "bsp-viewer", NULL, NULL);
	if( window == NULL )
	{
		cerr << "Failed to create GLFW window!" << endl;
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		cerr << "Failed to initialize GLEW!" << endl;
		glfwTerminate();
		return -1;
	}

	// BSP part
	map.file.read((char *)&map.header,sizeof(map.header));

	cout << "BSP file: " << argv[1] << endl;
	cout << "BSP version: " << map.header.version << endl;	

	map.LoadLumps();
	map.Rescale();

	// OPENGL

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "./shaders/simpleshader_vertex.glsl", "./shaders/simpleshader_fragments.glsl" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ColorID = glGetUniformLocation(programID, "Color");

	static const GLfloat square_buffer_data[] = // 2x2 square
	{ 
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
	};

	GLuint squarevertexbuffer;
	glGenBuffers(1, &squarevertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, squarevertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square_buffer_data), square_buffer_data, GL_STATIC_DRAW);
	
	vector<GLfloat>  vbuffer;
	vector<GLfloat>  wbuffer; // for wireframe view

	// faces
	for(int mdl=0; mdl<map.models.size(); mdl++)
	{
		for(int i=map.models[mdl].firstface; i<map.models[mdl].firstface+map.models[mdl].numfaces; i++)
		{
			vector<GLfloat> facetris;

			// if the face is a displacement face
			if(map.faces[i].dispinfo!=-1)
			{
				// not supported right now
				// I can't get the logic of it
				continue;
			}

			for(int i2=0; i2<map.faces[i].numedges;i2++)
			{
				int edge = map.surfedges[map.faces[i].firstedge+i2];

				int vert;
				vert = map.edges[abs(edge)].v[edge<0 ? 1 : 0];
				if(i2>2)
				{
					// insert 
					// first, current-1, current
					// vertices of this face to triangulate it

					facetris.insert(facetris.end(),facetris.begin(),facetris.begin()+3);
					facetris.insert(facetris.end(),facetris.end()-6,facetris.end()-3);

					facetris.push_back(map.vertexes[vert].x);
					facetris.push_back(map.vertexes[vert].y);
					facetris.push_back(map.vertexes[vert].z);		
				}
				else
				{
					facetris.push_back(map.vertexes[vert].x);
					facetris.push_back(map.vertexes[vert].y);
					facetris.push_back(map.vertexes[vert].z);
				}	
			}
			vbuffer.insert(vbuffer.end(),facetris.begin(),facetris.end());
		}		
	}

	int vbuffersize = vbuffer.size()*sizeof(GLfloat);
	int wbuffersize = wbuffer.size()*sizeof(GLfloat);

	GLuint mapvertexbuffer;
	glGenBuffers(1, &mapvertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mapvertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vbuffersize, &vbuffer[0], GL_STATIC_DRAW);

	GLuint mapwbuffer;
	glGenBuffers(1, &mapwbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mapwbuffer);
	glBufferData(GL_ARRAY_BUFFER, wbuffersize, &wbuffer[0], GL_STATIC_DRAW);

	// backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW); 

	do
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		computeMatricesFromInputs();

		glm::mat4 Projection = getProjectionMatrix();
		
		// Camera matrix
		int _dist = 50;
		glm::vec3 campos = glm::vec3(_dist,_dist,_dist);
		
		glm::mat4 View = getViewMatrix();
		// Model matrix : an identity matrix (model will be at the origin)
		glm::mat4 Model      = glm::mat4(1.0f);
		Model = glm::rotate(Model, glm::radians(-90.0f), glm::vec3(1.0,0.0,0.0));
		//Model = glm::lookAt(Model,campos,glm::vec3(0,1,0));

		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
		glm::vec4 Color = glm::vec4(1,1,1,1);

		// Send our transformation and color to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform4f(ColorID, Color.x, Color.y, Color.z, Color.w);

		// faces
		glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, mapvertexbuffer);
			
			glVertexAttribPointer(
				0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			glDrawArrays(GL_TRIANGLES, 0, vbuffersize/3);
		glDisableVertexAttribArray(0);

		// wireframe
		glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, mapwbuffer);
			
			glVertexAttribPointer(
				0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			glDrawArrays(GL_LINES, 0, wbuffersize/3);
		glDisableVertexAttribArray(0);	

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE )!=GLFW_PRESS && glfwWindowShouldClose(window)==0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &squarevertexbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	map.file.close();
	return 0;
}