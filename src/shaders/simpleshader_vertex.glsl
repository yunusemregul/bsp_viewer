#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;

out vec4 vertexColor;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform vec4 Color;

void main()
{
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	vertexColor = vec4(sin(vertexPosition_modelspace.x),sin(vertexPosition_modelspace.y),Color.b,Color.w);
}

