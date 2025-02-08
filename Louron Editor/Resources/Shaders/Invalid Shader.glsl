#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in mat4 aInstanceMatrix; // Use this as model matrix when engine instances the mesh opposed to u_VertexIn.Model

struct VertexData {
    
    mat4 Proj;
    mat4 View;
    mat4 Model;

};

uniform VertexData u_VertexIn;
uniform bool u_UseInstanceData = false;

out vec2 TexCoord;

void main() {

    mat4 model_matrix = (u_UseInstanceData ? aInstanceMatrix : u_VertexIn.Model);
	gl_Position = u_VertexIn.Proj * u_VertexIn.View * model_matrix * vec4(aPos, 1.0);
	TexCoord = aTexCoord;
}

#SHADER FRAGMENT

#version 450 core

in vec2 TexCoord;

out vec4 out_FragColour;

uniform sampler2D u_InvalidTexture;

void main() 
{
	out_FragColour = texture(u_InvalidTexture, vec2(TexCoord.x, TexCoord.y)) * vec4(1.0, 0.0, 1.0, 1.0);
}