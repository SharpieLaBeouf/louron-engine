#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

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

struct Material {

	vec4 diffuse;
    sampler2D diffuseTex;

	vec4 specular;
    sampler2D specularTex;

    sampler2D normal;

    float shininess;
}; 

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

struct VisibleIndex {
    int index;
};

// Point Lights SSBO
    layout(std430, binding = 0) readonly buffer PL_Buffer {
        PointLight data[];
    } PL_Buffer_Data;

    layout(std430, binding = 1) readonly buffer PL_IndiciesBuffer {
        VisibleIndex data[];
    } PL_IndiciesBuffer_Data;

// Spot Lights SSBO
    layout(std430, binding = 2) readonly buffer SL_Buffer {
        SpotLight data[];
    } SL_Buffer_Data;

    layout(std430, binding = 3) readonly buffer SL_IndiciesBuffer {
        VisibleIndex data[];
    } SL_IndiciesBuffer_Data;
    
// Directional Lights SSBO
    layout(std430, binding = 4) readonly buffer DL_Buffer {
        DirLight data[];
    } DL_Buffer_Data;

in vec3 FragPos;
in vec2 TexCoord;
in vec3 Normal;

uniform vec3 u_CameraPos;
uniform Material u_Material;

out vec4 fragColour;

vec3 CalcDirLights(vec3 normal, vec3 viewDir);
vec3 CalcPointLights(vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLights(vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
	
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(u_CameraPos - FragPos);

    vec3 result = CalcDirLights(norm, viewDir);
    result += CalcPointLights(norm, FragPos, viewDir);
    result += CalcSpotLights(norm, FragPos, viewDir);

	fragColour = vec4(result, 1.0f);
	
}

vec3 CalcDirLights(vec3 normal, vec3 viewDir){
    return vec3(1.0f);
}

vec3 CalcPointLights(vec3 normal, vec3 fragPos, vec3 viewDir){
    return vec3(1.0f);
}

vec3 CalcSpotLights(vec3 normal, vec3 fragPos, vec3 viewDir){
    return vec3(1.0f);
}