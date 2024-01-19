#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;


void main() {

	gl_Position = proj * view * model * vec4(aPos, 1.0);
	TexCoord = aTexCoord;
}

#SHADER FRAGMENT

#version 450 core

in vec2 TexCoord;
out vec4 fragColour;

struct Material{

	vec4 diffuse;
	sampler2D diffuseMap;

};
uniform Material u_Material;

void main() {
	fragColour = texture(u_Material.diffuseMap, vec2(TexCoord.x, TexCoord.y)) * u_Material.diffuse;
}