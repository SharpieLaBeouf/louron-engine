#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

struct VertexData {
    
    mat4 Proj;
    mat4 View;

};

uniform VertexData u_VertexIn;

void main()
{
    TexCoords = aPos;
    vec4 pos = u_VertexIn.Proj * u_VertexIn.View * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  

#SHADER FRAGMENT

#version 450 core

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube u_Skybox;

void main()
{    
    FragColor = texture(u_Skybox, TexCoords);
}