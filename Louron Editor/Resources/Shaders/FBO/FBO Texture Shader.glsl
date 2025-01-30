#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoords;

void main() {

    gl_Position = vec4(aPos, 1.0);
    TexCoords = aTexCoord;

}

#SHADER FRAGMENT

#version 450 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_ScreenTexture;

void main() {
    FragColor = texture(u_ScreenTexture, TexCoords);
}