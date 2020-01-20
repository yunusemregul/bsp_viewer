#include <main.h>

template <class T>
void Map::readLump(int id, vector<T> &V)
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

void Map::loadLumps()
{
	// Reading lumps
	readLump<dplane_t>(LUMP_PLANES, planes);
	readLump<Vector>(LUMP_VERTEXES, vertexes);
	readLump<dedge_t>(LUMP_EDGES, edges);
	readLump<signed int>(LUMP_SURFEDGES, surfedges);
	readLump<dface_t>(LUMP_FACES,faces);
	readLump<dmodel_t>(LUMP_MODELS,models);
	readLump<texinfo_t>(LUMP_TEXINFO,texinfo);
	readLump<dtexdata_t>(LUMP_TEXDATA,texdata);
	readLump<dDispVert>(LUMP_DISP_VERTS,dispverts);
	readLump<ddispinfo_t>(LUMP_DISPINFO,dispinfos);
}

void Map::rescale()
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

	// window setup
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "bsp_viewer", NULL, NULL);
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

	// parsing the bsp
	map.file.read((char *)&map.header,sizeof(map.header));

	cout << "BSP file: " << argv[1] << endl;
	cout << "BSP version: " << map.header.version << endl;	

	map.loadLumps();
	map.rescale();

	// opengl side

	// compile shaders
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &vertexShader, NULL);
	glCompileShader(vertexShaderID);
	
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fragmentShader, NULL);
	glCompileShader(fragmentShaderID);
	
	GLuint programID = glCreateProgram();
	glAttachShader(programID, fragmentShaderID);
	glAttachShader(programID, vertexShaderID);
	glLinkProgram(programID);
	glUseProgram(programID);

	GLuint matrixID = glGetUniformLocation(programID, "MVP");
	GLuint vPosition_modelspaceID = glGetAttribLocation(programID, "vPosition_modelspace");
	GLuint vColorID = glGetAttribLocation(programID, "vColor");	

	// capture keys to control camera, quit window etc.
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW); 

	// VAOs
	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	vector<GLfloat> vertexBuffer;
	vector<GLfloat> colorBuffer;

	// faces
	for(int mdl=0; mdl<map.models.size(); mdl++)
	{
		for(int i=map.models[mdl].firstface; i<map.models[mdl].firstface+map.models[mdl].numfaces; i++)
		{
			vector<GLfloat> faceTris;

			// if the face is a displacement face
			if(map.faces[i].dispinfo!=-1)
			{
				// not supported right now
				// I can't get the logic, would appreciate any help that describes it
				continue;
			}

			texinfo_t faceTexInfo = map.texinfo[map.faces[i].texinfo];
			Vector faceColor = map.texdata[faceTexInfo.texdata].reflectivity;

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

					faceTris.insert(faceTris.end(),faceTris.begin(),faceTris.begin()+3);
					faceTris.insert(faceTris.end(),faceTris.end()-6,faceTris.end()-3);
				}

				faceTris.push_back(map.vertexes[vert].x);
				faceTris.push_back(map.vertexes[vert].y);
				faceTris.push_back(map.vertexes[vert].z);	
			}
			vertexBuffer.insert(vertexBuffer.end(),faceTris.begin(),faceTris.end());
			for(int j=0; j<faceTris.size()/3; j++)
			{
				colorBuffer.push_back(faceColor.x);
				colorBuffer.push_back(faceColor.y);
				colorBuffer.push_back(faceColor.z);
			}
		}		
	}

	int vBufferSize = vertexBuffer.size()*sizeof(GLfloat);
	int cBufferSize = colorBuffer.size()*sizeof(GLfloat);

	GLuint mapVertexBuffer;
	glGenBuffers(1, &mapVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mapVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vBufferSize, &vertexBuffer[0], GL_STATIC_DRAW);

	GLuint mapColorBuffer;
	glGenBuffers(2, &mapColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mapColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, cBufferSize, &colorBuffer[0], GL_STATIC_DRAW);	

	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// use the shader
		glUseProgram(programID);

		computeMatricesFromInputs();

		glm::mat4 projection = getProjectionMatrix();
		
		// camera matrix
		int _dist = 50;
		glm::vec3 camPos = glm::vec3(_dist,_dist,_dist);
		
		glm::mat4 view = getViewMatrix();
		// model matrix : an identity matrix (model will be at the origin)
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0,0.0,0.0));

		// ModelViewProjection
		glm::mat4 MVP = projection * view * model;
		glm::vec3 color = glm::vec3(1,1,1);

		// send MVP to the shader
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);

		// faces
		glBindBuffer(GL_ARRAY_BUFFER, mapVertexBuffer);
		glVertexAttribPointer(vPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, mapColorBuffer);
		glVertexAttribPointer(vColorID, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glEnableVertexAttribArray(vPosition_modelspaceID);
		glEnableVertexAttribArray(vColorID);
		
		glDrawArrays(GL_TRIANGLES, 0, vBufferSize/3);
		
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while(glfwGetKey(window, GLFW_KEY_ESCAPE )!=GLFW_PRESS && glfwWindowShouldClose(window)==0);

	// cleanup
	glDeleteBuffers(1, &mapVertexBuffer);
	glDeleteBuffers(1, &mapColorBuffer);
	glDeleteVertexArrays(1, &vertexArrayID);
	glDeleteProgram(programID);

	glfwTerminate();

	map.file.close();
	return 0;
}