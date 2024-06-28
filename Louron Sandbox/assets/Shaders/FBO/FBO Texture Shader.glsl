#SHADER VERTEX

#version 450 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {

	gl_Position = vec4(aPos, 0.0, 1.0);
	TexCoord = aTexCoord;

}

#SHADER FRAGMENT

#version 450 core

in vec2 TexCoord;
out vec4 fragColour;

uniform sampler2D u_FBOTexture;

void main() {

    fragColour = texture(u_FBOTexture, TexCoord);

}