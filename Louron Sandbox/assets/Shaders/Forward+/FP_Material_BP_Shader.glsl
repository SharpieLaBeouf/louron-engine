#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in mat4 instanceMatrix;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform bool u_UseInstanceData = false;

void main() {

	TexCoord = aTexCoord;

    if (!u_UseInstanceData) {
        gl_Position = proj * view * model * vec4(aPos, 1.0);
	    Normal = mat3(transpose(inverse(model))) * aNormal;
	    FragPos = vec3(model * vec4(aPos, 1.0));
    }
    else 
    {
        gl_Position = proj * view * instanceMatrix * vec4(aPos, 1.0);
	    Normal = mat3(transpose(inverse(instanceMatrix))) * aNormal;
	    FragPos = vec3(instanceMatrix * vec4(aPos, 1.0));
    }
}

#SHADER FRAGMENT

#version 450 core

struct Material {

	vec4 diffuse;
    sampler2D diffuseMap;

	vec4 specular;
    sampler2D specularMap;

    sampler2D normalMap;

    float shine;
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
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
out vec4 fragColour;

// Standard Uniform Variables
uniform mat4 proj;
uniform vec3 u_CameraPos;
uniform Material u_Material;

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

// Lighting Functions
vec3 CalcDirLights(vec3 normal, vec3 viewDir);
vec3 CalcPointLights(vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLights(vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcNewSpotLights(vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
    
	// Determine which tile this pixel belongs to
	index = tileID.y * u_TilesX + tileID.x;
	
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(u_CameraPos - FragPos);

    vec3 result = vec3(0.0);
    
    result += CalcDirLights(norm, viewDir);
    result += CalcPointLights(norm, FragPos, viewDir);
    result += CalcSpotLights(norm, FragPos, viewDir);

    fragColour = vec4(result, 1.0);
}

// Calculate Directional Light Lighting in Scene (MAX 10)
vec3 CalcDirLights(vec3 normal, vec3 viewDir) {
    
    vec3 dirResult = vec3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < DL_Buffer_Data.data.length(); i++) {

        if (DL_Buffer_Data.data[i].lastLight == true)
            break;

        DirLight light = DL_Buffer_Data.data[i];

        vec3 lightDir = normalize(-light.direction.xyz);

        // diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);

        // specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shine);

        // Apply distance-based attenuation to specular
        float lightDistance = length(light.direction.xyz - FragPos);
        float distanceAttenuationFactor = 1.0 / (1.0 + 0.1 * lightDistance + 0.01 * (lightDistance * lightDistance));
        spec *= distanceAttenuationFactor;

        // combine results
        vec3 ambient = light.ambient.xyz * vec3(texture(u_Material.diffuseMap, TexCoord))               * u_Material.diffuse.xyz;
        vec3 diffuse = light.diffuse.xyz * diff * vec3(texture(u_Material.diffuseMap, TexCoord))        * u_Material.diffuse.xyz;
        vec3 specular = light.specular.xyz * spec * vec3(texture(u_Material.specularMap, TexCoord))     * u_Material.specular.xyz;

        dirResult += (ambient + diffuse + specular);
    }

    return dirResult;
}

// Calculate Point Light Lighting in Scene
vec3 CalcPointLights(vec3 normal, vec3 fragPos, vec3 viewDir) {

    vec3 pointResult = vec3(0.0f, 0.0f, 0.0f);
    
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
        
            // diffuse shading
            float diff = max(dot(normal, lightDir), 0.0);
        
            // specular shading
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shine);
        
            // attenuation
            float attenuation = light.intensity * pow(max(0.0, 1.0 - abs(dist) / light.radius), 2.0);

            // combine results
            vec3 ambient = light.ambient.xyz * texture(u_Material.diffuseMap, TexCoord).rgb             * u_Material.diffuse.xyz;
            vec3 diffuse = light.diffuse.xyz * diff * texture(u_Material.diffuseMap, TexCoord).rgb      * u_Material.diffuse.xyz;
            vec3 specular = light.specular.xyz * spec * texture(u_Material.specularMap, TexCoord).rgb   * u_Material.specular.xyz;
        
            ambient *= attenuation;
            diffuse *= attenuation;
            specular *= attenuation;

            pointResult += (ambient + diffuse + specular);
        }
    }

    return pointResult;
}

// Calculate Spot Light Lighting in Scene
vec3 CalcSpotLights(vec3 normal, vec3 fragPos, vec3 viewDir) {
    
    vec3 spotResult = vec3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < SL_Buffer_Data.data.length(); i++) {
    
        if (SL_Buffer_Data.data[i].lastLight == true)
            break;
            
        if (SL_Buffer_Data.data[i].activeLight == false)
            continue;
                
        // Setup light
        SpotLight light = SL_Buffer_Data.data[i];
        vec3 lightDir = normalize(light.position.xyz - fragPos);

        // Fragment culling
        float distance = length(light.position.xyz - fragPos);
        float theta = dot(lightDir, normalize(-light.direction.xyz));

        // Test if fragment is within range and radius of cone angle
        if (distance < light.range && theta > cos(radians(light.angle * 0.5))) {
            
            // Diffuse shading
            float diff = max(dot(normal, lightDir), 0.0);
        
            // Specular shading
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shine);

            // Calculate the angle factor to determine the falloff from the center to the edge of the cone
            float angleFactor = 1.0 - smoothstep(cos(radians(light.angle * 0.5 - light.angle * 0.25)), cos(radians(light.angle * 0.5)), theta);
            // Calculate the spotlight intensity based on the angle factor
            float spotlightIntensity = light.intensity * angleFactor;
            // Calculate the attenuation based on distance (similar to point light attenuation)
            float attenuation = spotlightIntensity * pow(max(0.0, 1.0 - (distance / light.range)), 2.0);
    
            // Combine results
            vec3 ambient = light.ambient.xyz * vec3(texture(u_Material.diffuseMap, TexCoord))               * u_Material.diffuse.xyz;
            vec3 diffuse = light.diffuse.xyz * diff * vec3(texture(u_Material.diffuseMap, TexCoord))        * u_Material.diffuse.xyz;
            vec3 specular = light.specular.xyz * spec * vec3(texture(u_Material.specularMap, TexCoord))     * u_Material.specular.xyz;
        
            ambient *= attenuation;
            diffuse *= attenuation;
            specular *= attenuation;
        
            spotResult += (ambient + diffuse + specular);
        }
     }

    return spotResult;
}