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
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 u_ShadowMatrices[6];
uniform int u_LayerOffset; // Offset for the current light's cube map layer in the array.

out vec4 FragPos;

void main()
{
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = u_LayerOffset + face; // Offset by light index.
        for (int i = 0; i < 3; ++i)
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = u_ShadowMatrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}

#SHADER FRAGMENT

#version 450 core
in vec4 FragPos;

uniform vec3 u_LightPosition;
uniform float u_FarPlane;

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - u_LightPosition);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / u_FarPlane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
} 