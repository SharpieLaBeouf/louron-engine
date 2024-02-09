#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

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
out vec4 fragColour;

uniform sampler2D u_OurTexture;
uniform vec4 u_OurColour;

void main() {

	fragColour = u_OurColour * texture(u_OurTexture, vec2(TexCoord.x, TexCoord.y));

}