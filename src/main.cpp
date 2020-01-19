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
	cout << ">		count: " << V.size() << endl;
}

void Map::LoadLumps()
{
	// Reading lumps
	for(int id=0; id < HEADER_LUMPS; id++)
	{
		if(header.lumps[id].fileofs==0)
			continue;

		switch(id)
		{
		case(LUMP_PLANES):
		{
			cout << ">	LUMP_PLANES" << endl;
			ReadLump<dplane_t>(id, planes);

			break;
		}
		case(LUMP_VERTEXES):
		{
			cout << ">	LUMP_VERTEXES" << endl;
			ReadLump<Vector>(id, vertexes);

			break;
		}
		case(LUMP_EDGES):
		{
			cout << ">	LUMP_EDGES" << endl;
			ReadLump<dedge_t>(id, edges);

			break;
		}
		case(LUMP_SURFEDGES):
		{
			cout << ">	LUMP_SURFEDGES" << endl;
			ReadLump<signed int>(id, surfedges);

			break;
		}
		case(LUMP_FACES):
		{
			cout << ">	LUMP_FACES" << endl;
			ReadLump<dface_t>(id,faces);

			break;		
		}		
		case(LUMP_MODELS):
		{
			cout << ">	LUMP_MODELS" << endl;
			ReadLump<dmodel_t>(id,models);

			break;		
		}
		case(LUMP_TEXINFO):
		{
			cout << ">	LUMP_TEXINFO" << endl;
			ReadLump<texinfo_t>(id,texinfo);

			break;
		}
		case(LUMP_DISP_VERTS):
		{
			cout << ">	LUMP_DISP_VERTS" << endl;
			ReadLump<dDispVert>(id,dispverts);

			break;
		}
		case(LUMP_DISPINFO):
		{
			cout << ">	LUMP_DISPINFO" << endl;
			ReadLump<ddispinfo_t>(id,dispinfos);
			
			break;		
		}

		default:
			break;
		}
 	}
}

void Map::Rescale()
{
	// scaling down the map
	for(int i=0;i<vertexes.size();i++)
	{
		vertexes[i] = vertexes[i]/scale;
	}

	for(int i=0;i<planes.size();i++)
	{
		planes[i].dist /= scale;
		//planes[i].normal = planes[i].normal/scale; 
	}

	for(int i=0; i<faces.size(); i++)
	{
		faces[i].area /= pow(scale,2);
	}

	for(int i=0; i<dispinfos.size(); i++)
	{
		dispinfos[i].startPosition = dispinfos[i].startPosition / scale;
	}
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
	if( window == NULL ){
		cerr << "Failed to create GLFW window!" << endl;
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
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

	static const GLfloat square_buffer_data[] = { // 2x2 square
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
				dplane_t plane = map.planes[map.faces[i].planenum];

				//cout << plane.normal.x << " " << plane.normal.y << " " << plane.normal.z << endl;

				ddispinfo_t dinfo = map.dispinfos[map.faces[i].dispinfo];
				
				// surfedges array
				int *surfedges = &map.surfedges[map.faces[i].firstedge];

				float xlen;
				float ylen;

				// ind = 0, x stable
				// ind = 1, y stable
				for(int ind=0;ind<2;ind++)
				{
					for(int ind2=0;ind2<2;ind2++)
					{
						Vector v = map.vertexes[map.edges[abs(surfedges[ind])].v[ind2]];

						if(ind==0)
						{
							if(ind2==0)
							{
								ylen = v.y;
							}
							else
							{
								ylen = abs(v.y-ylen);
							}
						}
						else
						{
							if(ind2==0)
							{
								xlen = v.x;
							}
							else
							{
								xlen = abs(v.x-xlen);
							}	
						}
					}
				}

				float len = sqrt(map.faces[i].area);
				xlen = xlen==0 ? len : xlen;
				ylen = ylen==0 ? len : ylen;
				
				int num = (pow(2,dinfo.power)+1);
				
				// y?
				for(int j=0; j<num; j++)
				{
					// x?
					for(int k=0; k<num; k++)
					{
						dDispVert vert = map.dispverts[dinfo.DispVertStart+j*num+k];

						char buf[32];
						vert.vec.tostring(buf);

						Vector start(k*xlen/num, j*ylen/num, 0);
						Vector end(k*xlen/num+(vert.dist+1)/map.scale*vert.vec.x, j*ylen/num+(vert.dist+1)/map.scale*vert.vec.y,(vert.dist+1)/map.scale*vert.vec.z);
						start = end-Vector(0,0,.05);

						start = start + dinfo.startPosition;
						end = end + dinfo.startPosition;

						//start = start * plane.normal;
						//end = end & plane.normal;

						wbuffer.push_back(start.x);
						wbuffer.push_back(start.y);
						wbuffer.push_back(start.z);

						wbuffer.push_back(end.x);
						wbuffer.push_back(end.y);
						wbuffer.push_back(end.z);
					}
				}
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

	//edges
	/*for(int i=0; i<map.surfedges.size(); i++)
	//for(int i=4; i<4+5; i++)
	{
		int surfedgefirst = map.surfedges[i];
		//int surfedgesecond = map.surfedges[i+1];
		dedge_t first = map.edges[surfedgefirst > 0 ? surfedgefirst : -surfedgefirst];
		//dedge_t second = map.edges[surfedgesecond > 0 ? surfedgesecond : -surfedgesecond];

		wbuffer.push_back(map.vertexes[first.v[surfedgefirst>0 ? 0 : 1]].x);
		wbuffer.push_back(map.vertexes[first.v[surfedgefirst>0 ? 0 : 1]].y);
		wbuffer.push_back(map.vertexes[first.v[surfedgefirst>0 ? 0 : 1]].z);	

		wbuffer.push_back(map.vertexes[first.v[surfedgefirst>0 ? 1 : 0]].x);
		wbuffer.push_back(map.vertexes[first.v[surfedgefirst>0 ? 1 : 0]].y);
		wbuffer.push_back(map.vertexes[first.v[surfedgefirst>0 ? 1 : 0]].z);
	}*/

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

	/*for(int i=0;i<map.faces.size();i++)
	{
		if(map.faces[i].numedges==4)
			continue;
		cout << "face " << i << " firstedge: " << map.surfedges[map.faces[i].firstedge] << " numedges: " << map.faces[i].numedges << endl;
		for(int i2=0;i2<map.faces[i].numedges;i2++)
		{
			cout << "	surfedge "<< i2 << ": " << map.surfedges[map.faces[i].firstedge+i2] << endl;
			cout << "		edge verts: " << map.edges[map.surfedges[map.faces[i].firstedge+i2]].v[0] << ", " << map.edges[map.surfedges[map.faces[i].firstedge+i2]].v[1] << endl;
		}
		cout << endl;
	}*/

	// backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW); 

	do{

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

		// DRAW X-Y-Z LINES
		/*for(int i=0;i<3;i++)
		{
			glm::mat4 Line = glm::mat4(1.0f);
			glm::vec4 Color;

			switch(i)
			{
				case 0: //x red
				{
					Color = glm::vec4(1,0,0,1);
					Line = glm::rotate(Line, glm::radians(90.0f), glm::vec3(1,0,0));
					Line = glm::scale(Line, glm::vec3(500, .1, 0));
					break;
				}
				case 1: //y green
				{
					Color = glm::vec4(0,1,0,1);
					Line = glm::rotate(Line, glm::radians(90.0f), glm::vec3(0,1,0));
					Line = glm::scale(Line, glm::vec3(.1, 500, 0));
					break;
				}
				case 2: //z blue
				{
					Color = glm::vec4(0,0,1,1);
					Line = glm::rotate(Line, glm::radians(90.0f), glm::vec3(1,0,0));
					Line = glm::scale(Line, glm::vec3(.1, 500, 0));
					break;
				}
			}
			glm::mat4 MVP = Projection*View*Line;

			// Send our transformation and color to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniform4f(ColorID, Color.x, Color.y, Color.z, Color.w);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, squarevertexbuffer);
				glVertexAttribPointer(
					0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
				);

				// Draw the triangle !
				glDrawArrays(GL_TRIANGLES, 0, 6); // 3 indices starting at 0 -> 1 triangle
			glDisableVertexAttribArray(0);	
		}	*/	
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
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &squarevertexbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	map.file.close();
	return 0;
}