#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {

	gl_Position = proj * view * model * vec4(aPos, 1.0);

}

#SHADER FRAGMENT

#version 450 core

layout(location = 0) out vec4 fragColour;

uniform vec4 ourColour;

void main() {

	fragColour = ourColour;

}