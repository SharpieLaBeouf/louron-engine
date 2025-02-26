#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoord;

struct VertexData {
    
    mat4 Proj;
    mat4 View;
    mat4 Model;

};

uniform VertexData u_VertexIn;

void main() {

	gl_Position = u_VertexIn.Proj * u_VertexIn.View * u_VertexIn.Model * vec4(aPos, 1.0);
	TexCoord = aTexCoord;
}

#SHADER FRAGMENT

#version 450 core

in vec2 TexCoord;

out vec4 out_FragColour;

struct Material{

	vec4 diffuse;
	sampler2D diffuseMap;

};
uniform Material u_Material;

void main() 
{
	out_FragColour = texture(u_Material.diffuseMap, vec2(TexCoord.x, TexCoord.y)) * u_Material.diffuse;
}