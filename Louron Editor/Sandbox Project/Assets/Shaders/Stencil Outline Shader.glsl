#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 u_Proj;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
    TexCoords = aTexCoords;    
    gl_Position = u_Proj * u_View * u_Model * vec4(aPos, 1.0f);
}

#SHADER FRAGMENT

#version 450 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(0.949f, 0.549f, 0.157f, 1.0f);
}