#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;

struct VertexData {
    
    mat4 Proj;
    mat4 View;
    mat4 Model;

};

uniform VertexData u_VertexIn;

void main() {

	gl_Position = u_VertexIn.Proj * u_VertexIn.View * u_VertexIn.Model * vec4(aPos, 1.0);
	
}

#SHADER FRAGMENT

#version 450 core

layout(location = 0) out vec4 fragColour;

uniform vec4 u_OurColour;

void main() {

	fragColour = u_OurColour;

}