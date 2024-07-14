#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3   aPos;
layout (location = 1) in vec3   aNormal;
layout (location = 2) in vec2   aTexCoord;
layout (location = 3) in vec3   aTangent;
layout (location = 4) in vec3   aBitangent;
layout (location = 5) in mat4   aInstanceMatrix;

layout (location = 0) out vec2  TexCoord;
layout (location = 1) out vec3  Normal;
layout (location = 2) out vec3  FragPos;

struct VertexData {
    
    mat4 Proj;
    mat4 View;
    mat4 Model;

};

uniform VertexData u_VertexIn;
uniform bool u_UseInstanceData = false;

void main() {

	TexCoord = aTexCoord;

    if (!u_UseInstanceData) {
        gl_Position = u_VertexIn.Proj * u_VertexIn.View * u_VertexIn.Model * vec4(aPos, 1.0);
	    Normal = mat3(transpose(inverse(u_VertexIn.Model))) * aNormal;
	    FragPos = vec3(u_VertexIn.Model * vec4(aPos, 1.0));
    }
    else 
    {
        gl_Position = u_VertexIn.Proj * u_VertexIn.View * aInstanceMatrix * vec4(aPos, 1.0);
	    Normal = mat3(transpose(inverse(aInstanceMatrix))) * aNormal;
	    FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));
    }
}

#SHADER FRAGMENT

#version 450 core

struct PBRMaterial {
    vec4 albedoTint;
    sampler2D albedoTexture;

    float metallicScale;
    sampler2D metallicTexture;

    sampler2D normalTexture;

    float roughness;
};

struct DirLight {
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    bool lastLight;
    
    // DO NOT USE - this is for SSBO alignment purposes ONLY (8 BYTES)
    float m_Padding1;
    float m_Padding2;
    float m_Padding3;
};

struct PointLight {
    vec4 position;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float radius;
    float intensity;

    bool activeLight;
    bool lastLight;
};

struct SpotLight {
    vec4 position;
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    
    float range;
    float angle;
    float intensity;
    
    bool activeLight;
    bool lastLight;
   
    // DO NOT USE - this is for SSBO alignment purposes ONLY (8 BYTES)
    float m_Padding1;
    float m_Padding2;
    float m_Padding3;
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

// Shader In/Out Variables
layout (location = 0) in vec2       TexCoord;
layout (location = 1) in vec3       Normal;
layout (location = 2) in vec3       FragPos;

layout (location = 0) out vec4      o_FragColour;

// Standard Uniform Variables
uniform vec3 u_CameraPos;
uniform PBRMaterial u_Material;

// Forward Plus Uniform Variables
uniform int u_TilesX;
uniform ivec2 u_ScreenSize;
uniform sampler2D u_Depth;

// Forward Plus Local Variables
uint index;
ivec2 location = ivec2(gl_FragCoord.xy);
ivec2 tileID = location / ivec2(16, 16);

uint MAX_DIRECTIONAL_LIGHTS = 10;
uint MAX_POINT_LIGHTS = 1024;
uint MAX_SPOT_LIGHTS = 1024;

const float PI = 3.14159265359;

// PBR Functions
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);

// Lighting Functions
vec3 CalcDirLights(vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness);
vec3 CalcPointLights(vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float metallic, float roughness);
vec3 CalcSpotLights(vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float metallic, float roughness);

void main() {
    
    // Determine which tile this pixel belongs to
    index = tileID.y * u_TilesX + tileID.x;
    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(u_CameraPos - FragPos);

    // Retrieve material properties
    vec3 albedo = texture(u_Material.albedoTexture, TexCoord).rgb * u_Material.albedoTint.rgb;
    float metallic = texture(u_Material.metallicTexture, TexCoord).r * u_Material.metallicScale;
    float roughness = u_Material.roughness;

    vec3 result = vec3(0.0);
    
    result += CalcDirLights(norm, viewDir, albedo, metallic, roughness);
    result += CalcPointLights(norm, FragPos, viewDir, albedo, metallic, roughness);
    result += CalcSpotLights(norm, FragPos, viewDir, albedo, metallic, roughness);
    
    result = result / (result + vec3(1.0));

    // Gamma correction
    result = pow(result / (result + vec3(1.0)), vec3(1.0/2.2)); 

    o_FragColour = vec4(result, 1.0);
}

// Calculate Directional Light Lighting in Scene (MAX 10)
vec3 CalcDirLights(vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness) {
    
    vec3 dirResult = vec3(0.0);

    for (int i = 0; i < DL_Buffer_Data.data.length(); i++) {

        if (DL_Buffer_Data.data[i].lastLight == true)
            break;

        DirLight light = DL_Buffer_Data.data[i];

        vec3 lightDir = normalize(-light.direction.xyz);
        vec3 radiance = light.diffuse.rgb;

        // Cook-Torrance BRDF
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float distance = length(light.direction.xyz - FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 F = FresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
        float NDF = DistributionGGX(normal, halfwayDir, roughness);
        float G = GeometrySmith(normal, viewDir, lightDir, roughness);
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(normal, lightDir), 0.0);
        dirResult += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    return dirResult;
}

// Calculate Point Light Lighting in Scene
vec3 CalcPointLights(vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float metallic, float roughness) {

    vec3 pointResult = vec3(0.0);
    
    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint index = tileID.y * u_TilesX + tileID.x;
    uint offset = index * MAX_POINT_LIGHTS;

    for (int i = 0;  i < MAX_POINT_LIGHTS && PL_IndiciesBuffer_Data.data[offset + i].index != -1; i++) {

        uint lightIndex = PL_IndiciesBuffer_Data.data[offset + i].index;

        if (PL_Buffer_Data.data[lightIndex].activeLight == false)
            continue;
    
        float dist = length(PL_Buffer_Data.data[lightIndex].position.xyz - fragPos);
        
        if (dist < PL_Buffer_Data.data[lightIndex].radius) {

            PointLight light = PL_Buffer_Data.data[lightIndex];

            vec3 lightDir = normalize(light.position.xyz - fragPos);
            vec3 radiance = light.diffuse.rgb * light.intensity;

            // Cook-Torrance BRDF
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float distance = length(light.position.xyz - FragPos);
            float attenuation = 1.0 / (distance * distance);
            vec3 F0 = mix(vec3(0.04), albedo, metallic);
            vec3 F = FresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
            float NDF = DistributionGGX(normal, halfwayDir, roughness);
            float G = GeometrySmith(normal, viewDir, lightDir, roughness);
            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.001;
            vec3 specular = numerator / denominator;

            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;

            float NdotL = max(dot(normal, lightDir), 0.0);
            pointResult += (kD * albedo / PI + specular) * radiance * NdotL * attenuation;
        }
    }

    return pointResult;
}

// Calculate Spot Light Lighting in Scene
vec3 CalcSpotLights(vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float metallic, float roughness) {

    vec3 spotResult = vec3(0.0);
    
    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint index = tileID.y * u_TilesX + tileID.x;
    uint offset = index * MAX_SPOT_LIGHTS;

    for (int i = 0;  i < MAX_SPOT_LIGHTS && SL_IndiciesBuffer_Data.data[offset + i].index != -1; i++) {

        uint lightIndex = SL_IndiciesBuffer_Data.data[offset + i].index;

        if (SL_Buffer_Data.data[lightIndex].activeLight == false)
            continue;

        float dist = length(SL_Buffer_Data.data[lightIndex].position.xyz - fragPos);
        
        if (dist < SL_Buffer_Data.data[lightIndex].range) {

            SpotLight light = SL_Buffer_Data.data[lightIndex];

            vec3 lightDir = normalize(light.position.xyz - fragPos);
            vec3 radiance = light.diffuse.rgb * light.intensity;

            // Cook-Torrance BRDF
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float distance = length(light.position.xyz - FragPos);
            float attenuation = 1.0 / (distance * distance);
            vec3 F0 = mix(vec3(0.04), albedo, metallic);
            vec3 F = FresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
            float NDF = DistributionGGX(normal, halfwayDir, roughness);
            float G = GeometrySmith(normal, viewDir, lightDir, roughness);
            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.001;
            vec3 specular = numerator / denominator;

            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;

            float NdotL = max(dot(normal, lightDir), 0.0);
            spotResult += (kD * albedo / PI + specular) * radiance * NdotL * attenuation;
        }
    }

    return spotResult;
}

// GGX Normal Distribution Function
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// Schlick's approximation of Fresnel factor
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX Geometry function using Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// Smith's method for both geometric shadowing
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
