// vector vertex shader: elongated quad shader

#version 330 core

layout (location = 0) in vec2 quad_position;
layout (location = 1) in vec2 position0;
layout (location = 2) in vec2 position1;

uniform mat4 view_projection;
uniform float vector_thickness;
uniform float vector_length;

void main(){
	vec2 direction = normalize(position1 - position0);
	vec2 perpendicular = vec2(-direction.y, direction.x);
	vec2 vertex_position = position0 + 
		vector_length * mix(-direction, direction, (quad_position.x + 1) / 2) + 
		vector_thickness * mix(-perpendicular, perpendicular, (quad_position.y + 1) / 2);
	gl_Position = view_projection * vec4(vertex_position, 0, 1);
};