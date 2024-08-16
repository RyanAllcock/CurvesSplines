// blob fragment shader: basic circle/disk

#version 330 core

in vec2 vert_pos;

out vec4 frag_colour;

void main(){
	vec2 uv = vert_pos;
	float distance = 1.0 - length(uv);
	vec3 radius = vec3(step(0.0, distance));
	vec3 colour = vec3(0, 0.5, 1);
	frag_colour.rgb = colour * radius;
	frag_colour.a = 1;
	if(distance < 0) discard;
};