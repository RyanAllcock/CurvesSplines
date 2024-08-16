// line vertex shader: line joint vertices

#version 330 core

layout (location = 0) in vec2 quad;
layout (location = 1) in vec2 pos0;
layout (location = 2) in vec2 dir0;
layout (location = 3) in vec2 pos1;
layout (location = 4) in vec2 dir1;

uniform mat4 view_projection;
uniform float line_thickness;

out vec2 vert_joint;

void main(){
	vec2 position = mix(pos0, pos1, (quad.x + 1) / 2);
	vec2 direction = mix(dir0, dir1, (quad.x + 1) / 2);
	vec2 joint = line_thickness * vec2(-direction.y, direction.x) * quad.y;
	gl_Position = view_projection * vec4(position + joint, 0, 1);
};