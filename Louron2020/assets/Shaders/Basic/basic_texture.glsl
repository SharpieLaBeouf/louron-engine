#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {

	gl_Position = proj * view * model * vec4(aPos, 1.0);
	TexCoord = aTexCoord;

}

#SHADER FRAGMENT

#version 450 core

in vec2 TexCoord;
out vec4 fragColour;

uniform sampler2D ourTexture;
uniform vec4 ourColour;

void main() {

	fragColour = ourColour * texture(ourTexture, vec2(TexCoord.x, TexCoord.y));

}