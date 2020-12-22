#SHADER VERTEX

#version 330 core

layout (location = 0) in vec3 aPos;

void main() {

	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);

}

#SHADER FRAGMENT

#version 330 core

out vec4 fragColour;
uniform vec4 ourColour;

void main() {

	fragColour = ourColour;

}