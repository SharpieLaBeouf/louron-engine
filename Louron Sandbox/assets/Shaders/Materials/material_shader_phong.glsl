#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main() {

	gl_Position = proj * view * model * vec4(aPos, 1.0);

	TexCoord = aTexCoord;
	Normal = mat3(transpose(inverse(model))) * aNormal;
	FragPos = vec3(model * vec4(aPos, 1.0));

}

#SHADER FRAGMENT

#version 450 core


struct Light {
    vec3 position;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct Material{

	float shine;
	vec4 diffuse;
	vec4 specular;

	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D normalMap;
	sampler2D heightMap;

};

uniform Light u_Light;
uniform Material u_Material;
uniform vec3 u_CameraPos;

out vec4 fragColour;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

void main() {
	
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(u_Light.position - FragPos);
	vec3 viewDir = normalize(u_CameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	
	vec4 ambient  = texture(u_Material.diffuseMap,  TexCoord) * u_Material.diffuse * min((u_Light.ambient), 0.25);
	vec4 diffuse  = u_Material.diffuse  * texture(u_Material.diffuseMap,  TexCoord) * u_Light.diffuse * max(dot(norm, lightDir), 0.0);
	vec4 specular = texture(u_Material.specularMap, TexCoord) * u_Light.specular * pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shine);

	fragColour = ambient + diffuse + specular;
	
}