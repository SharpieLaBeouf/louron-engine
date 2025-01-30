#SHADER VERTEX

#version 450 core
layout (location = 0) in vec3 aPos;

uniform mat4 u_Model;

void main()
{
    gl_Position = u_Model * vec4(aPos, 1.0);
}

#SHADER GEOMETRY

#version 450 core

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std430, binding = 5) readonly buffer DL_Shadow_LightSpaceMatrices_Buffer {
    mat4 data[];
} DL_Shadow_LightSpaceMatrices_Buffer_Data;

uniform uint u_LightIndex;

void main()
{          
	for (int i = 0; i < 3; ++i)
	{
		gl_Position = DL_Shadow_LightSpaceMatrices_Buffer_Data.data[int(u_LightIndex) * 5 + gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = int(u_LightIndex) * 5 + gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}  

#SHADER FRAGMENT

#version 450 core

void main()
{
} 