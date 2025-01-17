#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat3 normalToWorld = mat3(1.0f);

struct VertexData {
    
    mat4 Proj;
    mat4 View;
    mat4 Model;

};

uniform VertexData u_VertexIn;

void main() {

	gl_Position = u_VertexIn.Proj * u_VertexIn.View * u_VertexIn.Model * vec4(aPos, 1.0);
	
	TexCoord = aTexCoord;
	Normal = normalToWorld * aNormal;;
	FragPos = vec3(u_VertexIn.Model * vec4(aPos, 1.0));

}

#SHADER FRAGMENT

#version 450 core

out vec4 fragColour;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 u_ViewPos;

uniform vec3 u_LightPos;
uniform vec4 u_LightColour;

uniform vec4 u_TargetColour;

float ambientStrength = 0.1;
float specularStrength = 1.0;

//uniform sampler2D ourTexture;

void main() {
	
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(u_LightPos - FragPos);
	vec3 viewDir = normalize(u_ViewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	
	vec4 ambient  = ambientStrength * u_LightColour;

	vec4 diffuse  = u_LightColour * max(dot(norm, lightDir), 0.0);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec4 specular = specularStrength * spec * u_LightColour;  

	fragColour = u_TargetColour * (ambient + diffuse + specular);
	
	//fragColour = texture(ourTexture, vec2(TexCoord.x, TexCoord.y));
}