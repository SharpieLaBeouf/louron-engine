#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3   aPos;
layout (location = 1) in mat4   aInstanceMatrix;

struct VertexData {
    
    mat4 Proj;
    mat4 View;
    mat4 Model;

};

uniform VertexData u_VertexIn;
uniform bool u_UseInstanceData = false;

void main() {

    if (!u_UseInstanceData) {
        gl_Position = u_VertexIn.Proj * u_VertexIn.View * u_VertexIn.Model * vec4(aPos, 1.0);
    }
    else{
        gl_Position = u_VertexIn.Proj * u_VertexIn.View * aInstanceMatrix * vec4(aPos, 1.0);
    }
}

#SHADER FRAGMENT

#version 450 core

uniform vec4 u_LineColor;

layout (location = 0) out vec4 out_FragColour;

void main() {
    out_FragColour = u_LineColor;
}