// blob vertex shader: instanced billboard quad

#version 330 core

layout (location = 0) in vec2 rel;
layout (location = 1) in vec2 pos;

out vec2 vert_pos;

uniform mat4 view_projection;
uniform float point_radius;

void main(){
	gl_Position = view_projection * vec4(pos + rel * point_radius, 0, 1);
	vert_pos = rel;
};