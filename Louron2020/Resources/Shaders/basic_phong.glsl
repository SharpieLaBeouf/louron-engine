#SHADER VERTEX

#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform mat3 normalToWorld = mat3(1.0f);

void main() {

	gl_Position = proj * view * model * vec4(aPos, 1.0);

	TexCoord = aTexCoord;
	Normal = normalToWorld * aNormal;;
	FragPos = vec3(model * vec4(aPos, 1.0));

}

#SHADER FRAGMENT

#version 330 core

out vec4 fragColour;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;

uniform vec3 lightPos;
uniform vec4 lightColour;

uniform vec4 targetColour;

float ambientStrength = 0.1;
float specularStrength = 1.0;

//uniform sampler2D ourTexture;

void main() {
	
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	
	vec4 ambient  = ambientStrength * lightColour;

	vec4 diffuse  = lightColour * max(dot(norm, lightDir), 0.0);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec4 specular = specularStrength * spec * lightColour;  

	fragColour = targetColour * (ambient + diffuse + specular);
	
	//fragColour = texture(ourTexture, vec2(TexCoord.x, TexCoord.y));
}