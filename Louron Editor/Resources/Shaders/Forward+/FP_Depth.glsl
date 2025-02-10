#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;

uniform mat4 u_Proj;
uniform mat4 u_View;
uniform mat4 u_Model;

void main() {
	gl_Position = u_Proj * u_View * u_Model * vec4(aPos, 1.0);
}

#SHADER FRAGMENT

#version 450 core

struct Entity
{
    uint entity_id;
    float depth;
};

layout(std430, binding = 10) buffer EntityBuffer { Entity data[]; } EntityBuffer_Data;

uniform uint u_EntityID;
uniform ivec2 u_ScreenSize;

void main() 
{
    uint index = uint(gl_FragCoord.y) * uint(u_ScreenSize.x) + uint(gl_FragCoord.x);
    
    float existingDepth = EntityBuffer_Data.data[index].depth;
    if (gl_FragCoord.z < existingDepth) {
        EntityBuffer_Data.data[index].entity_id = u_EntityID;
        EntityBuffer_Data.data[index].depth = gl_FragCoord.z;
    }
}