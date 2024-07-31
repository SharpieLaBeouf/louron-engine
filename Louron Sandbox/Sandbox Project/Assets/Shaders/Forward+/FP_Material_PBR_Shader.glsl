#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3   aPos;
layout (location = 1) in vec3   aNormal;
layout (location = 2) in vec2   aTexCoord;
layout (location = 3) in vec3   aTangent;
layout (location = 4) in vec3   aBitangent;
layout (location = 5) in mat4   aInstanceMatrix;

out VS_OUT {

    vec3 FragPos;
    vec2 TexCoord;
	mat3 TBN_Matrix;
	vec3 TangentViewPos;
	vec3 TangentFragPos;

} vertex_out;

struct VertexData {
    
    mat4 Proj;
    mat4 View;
    mat4 Model;

};

uniform vec3 u_CameraPos;
uniform VertexData u_VertexIn;
uniform bool u_UseInstanceData = false;

void main() {

	vertex_out.TexCoord = aTexCoord;

    mat3 normal_matrix;

    if (!u_UseInstanceData) {
        gl_Position = u_VertexIn.Proj * u_VertexIn.View * u_VertexIn.Model * vec4(aPos, 1.0);
	    
        vertex_out.FragPos = vec3(u_VertexIn.Model * vec4(aPos, 1.0));

        normal_matrix = transpose(inverse(mat3(u_VertexIn.Model)));
    }
    else 
    {
        gl_Position = u_VertexIn.Proj * u_VertexIn.View * aInstanceMatrix * vec4(aPos, 1.0);
	   
        vertex_out.FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));
       
        normal_matrix = transpose(inverse(mat3(aInstanceMatrix)));
    }

    // Normal Mapping Functions - Construct TBN Matrix
	vec3 tangent = normalize(normal_matrix * aTangent);
	vec3 bitangent = normalize(normal_matrix * aBitangent);
	vec3 normal = normalize(normal_matrix * aNormal);
    
	mat3 TBN = transpose(mat3(tangent, bitangent, normal));
	vertex_out.TangentViewPos = TBN * u_CameraPos;
	vertex_out.TangentFragPos = TBN * vertex_out.FragPos;
	vertex_out.TBN_Matrix = TBN;
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
    vec4 colour;
    
    bool activeLight;
    float intensity;
    bool lastLight;
    
    // DO NOT USE - this is for SSBO alignment purposes ONLY
    float m_Padding1;
};

struct PointLight {
    vec4 position;

    vec4 colour;

    float radius;
    float intensity;

    bool activeLight;
    bool lastLight;
};

struct SpotLight {

    vec4 position;
    vec4 direction;

    vec4 colour;
    
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
in VS_OUT {

    vec3 FragPos;
    vec2 TexCoord;
	mat3 TBN_Matrix;
	vec3 TangentViewPos;
	vec3 TangentFragPos;

} fragment_in;

layout (location = 0) out vec4 out_FragColour;

// Standard Uniform Variables
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
vec3 CalcDirLights(vec3 normal, vec3 view_direction, vec3 albedo, float metallic, float roughness);
vec3 CalcPointLights(vec3 normal, vec3 fragPos, vec3 view_direction, vec3 albedo, float metallic, float roughness);
vec3 CalcSpotLights(vec3 normal, vec3 fragPos, vec3 view_direction, vec3 albedo, float metallic, float roughness);

float Attenuation_InverseSquareLawWithFalloff(float light_intensity, float light_radius, float distance_from_light) {
    float attenuation = 1.0 / (distance_from_light * distance_from_light); // Inverse square law

    // Normalize the distance to the light radius and create a smooth falloff
    float normalised_distance = distance_from_light / light_radius;
    if (normalised_distance > 1.0) {
        attenuation = 0.0; // Outside the light radius
    } else {
        float smooth_falloff = pow(1.0 - normalised_distance, 2.0); // Quadratic smooth falloff
        attenuation *= smooth_falloff;
    }

    return light_intensity * attenuation;
}

float Attenuation_QuadraticWithFalloff(float light_intensity, float light_radius, float distance_from_light) {
    return light_intensity * pow(max(1.0 - abs(distance_from_light) / light_radius, 0.0), 2.0);
}

void main() {
    
    // Determine which tile this pixel belongs to
    index = tileID.y * u_TilesX + tileID.x;
    
	vec3 view_direction = normalize(fragment_in.TangentViewPos - fragment_in.TangentFragPos);

    // All textures are loaded using GL_RGBA internal format, 
    // therefore for albedo textures ONLY we need to convert 
    // this from sRGB to linear space
    vec3 albedo = pow(texture(u_Material.albedoTexture, fragment_in.TexCoord).rgb * u_Material.albedoTint.rgb, vec3(2.2));
    float metallic = texture(u_Material.metallicTexture, fragment_in.TexCoord).r * u_Material.metallicScale;
    float roughness = u_Material.roughness;

	vec3 normal = texture(u_Material.normalTexture, fragment_in.TexCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
    
    vec3 result = vec3(0.0);
    
    result += CalcDirLights(normal, view_direction, albedo, metallic, roughness);
    result += CalcPointLights(normal, fragment_in.TangentFragPos, view_direction, albedo, metallic, roughness);
    result += CalcSpotLights(normal, fragment_in.TangentFragPos, view_direction, albedo, metallic, roughness);
    
    // HDR
    result = result / (result + vec3(1.0));

    // Gamma correction
    result = pow(result, vec3(1.0/2.2)); 

    out_FragColour = vec4(result, 1.0);
}

// Calculate Directional Light Lighting in Scene (MAX 10)
vec3 CalcDirLights(vec3 normal, vec3 view_direction, vec3 albedo, float metallic, float roughness) {
    
    vec3 directional_result = vec3(0.0);

    for (int i = 0; i < DL_Buffer_Data.data.length(); i++) {

        if (DL_Buffer_Data.data[i].lastLight == true)
            break;

        if (DL_Buffer_Data.data[i].activeLight == false)
            continue;

        DirLight light = DL_Buffer_Data.data[i];
        
        vec3 light_direction = fragment_in.TBN_Matrix * -light.direction.xyz;
        
        if (dot(normal, light_direction) <= 0.01)
            continue;

        vec3 radiance = light.colour.rgb * light.intensity;
        
        // Cook-Torrance BRDF
        vec3 halfway_direction = normalize(light_direction + view_direction + vec3(0.005));
        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 F = FresnelSchlick(max(dot(halfway_direction, view_direction), 0.0), F0);
        float NDF = DistributionGGX(normal, halfway_direction, roughness);
        float G = GeometrySmith(normal, view_direction, light_direction, roughness);

        float NdotV = max(dot(normal, view_direction), 0.0);
        float NdotL = max(dot(normal, light_direction), 0.0);
        float denominator = 4.0 * NdotV * NdotL + 0.0001;
                
        vec3 numerator = NDF * G * F;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        directional_result += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    return directional_result;
}

// Calculate Point Light Lighting in Scene
vec3 CalcPointLights(vec3 normal, vec3 fragPos, vec3 view_direction, vec3 albedo, float metallic, float roughness) {
    vec3 point_result = vec3(0.0);
    
    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint index = tileID.y * u_TilesX + tileID.x;
    uint offset = index * MAX_POINT_LIGHTS;

    for (int i = 0;  i < MAX_POINT_LIGHTS && PL_IndiciesBuffer_Data.data[offset + i].index != -1; i++) {
        uint lightIndex = PL_IndiciesBuffer_Data.data[offset + i].index;

        if (!PL_Buffer_Data.data[lightIndex].activeLight)
            continue;
        
        vec3 tang_light_pos = fragment_in.TBN_Matrix * PL_Buffer_Data.data[lightIndex].position.xyz;
        float dist = length(tang_light_pos - fragPos);

        if (dist < PL_Buffer_Data.data[lightIndex].radius) {
            
            PointLight light = PL_Buffer_Data.data[lightIndex];
            vec3 light_direction = normalize(tang_light_pos - fragPos);
           
            vec3 radiance = light.colour.rgb * light.intensity;
            float attenuation = Attenuation_InverseSquareLawWithFalloff(light.intensity, light.radius, dist);
            
            // Cook-Torrance BRDF
            vec3 halfway_direction = normalize(light_direction + view_direction);
            vec3 F0 = mix(vec3(0.04), albedo, metallic);
            vec3 F = FresnelSchlick(max(dot(halfway_direction, view_direction), 0.0), F0);
            float NDF = DistributionGGX(normal, halfway_direction, roughness);
            float G = GeometrySmith(normal, view_direction, light_direction, roughness);

            float NdotV = max(dot(normal, view_direction), 0.0);
            float NdotL = max(dot(normal, light_direction), 0.0);
            float denominator = 4.0 * NdotV * NdotL + 0.0001;
            
            vec3 numerator = NDF * G * F;
            vec3 specular = numerator / denominator;

            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;
            
            point_result += (kD * albedo / PI + specular) * radiance * NdotL * attenuation;
        }
    }

    return point_result;
}

// Calculate Spot Light Lighting in Scene
vec3 CalcSpotLights(vec3 normal, vec3 fragPos, vec3 view_direction, vec3 albedo, float metallic, float roughness) {

    vec3 spot_result = vec3(0.0);
    
    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint index = tileID.y * u_TilesX + tileID.x;
    uint offset = index * MAX_SPOT_LIGHTS;

    for (int i = 0;  i < MAX_SPOT_LIGHTS && SL_IndiciesBuffer_Data.data[offset + i].index != -1; i++) {

        uint lightIndex = SL_IndiciesBuffer_Data.data[offset + i].index;

        if (SL_Buffer_Data.data[lightIndex].activeLight == false)
            continue;
        
		vec3 tang_light_pos = fragment_in.TBN_Matrix * SL_Buffer_Data.data[lightIndex].position.xyz;
        
        float dist = length(tang_light_pos - fragPos);
        
        if (dist < SL_Buffer_Data.data[lightIndex].range) {

            SpotLight light = SL_Buffer_Data.data[lightIndex];
            
            vec3 light_direction = normalize(tang_light_pos - fragPos);
            
            float theta = dot(light_direction, normalize(fragment_in.TBN_Matrix * -light.direction.xyz));
            float outer_cut_off = cos(radians(light.angle * 0.5));

            // Check if frag is within the influence of the cone of the spot light.
            if (theta > outer_cut_off) {
                
                float inner_cut_off = cos(radians(light.angle * 0.25));

                float epsilon = cos(inner_cut_off - outer_cut_off);
                
                // Calculate the angle factor to determine the falloff from the center to the edge of the cone
                float angle_factor = 1.0 - smoothstep(epsilon, outer_cut_off, theta);
                // Calculate the spotlight intensity based on the angle factor
                float spot_light_intensity = light.intensity * angle_factor;

                float attenuation = Attenuation_InverseSquareLawWithFalloff(spot_light_intensity, light.range, dist);

                vec3 radiance = light.colour.rgb * spot_light_intensity;
                
                // Cook-Torrance BRDF
                vec3 halfway_direction = normalize(light_direction + view_direction);
                vec3 F0 = mix(vec3(0.04), albedo, metallic);
                vec3 F = FresnelSchlick(max(dot(halfway_direction, view_direction), 0.0), F0);
                float NDF = DistributionGGX(normal, halfway_direction, roughness);
                float G = GeometrySmith(normal, view_direction, light_direction, roughness);

                float NdotV = max(dot(normal, view_direction), 0.0);
                float NdotL = max(dot(normal, light_direction), 0.0);
                float denominator = 4.0 * NdotV * NdotL + 0.0001;
            
                vec3 numerator = NDF * G * F;
                vec3 specular = numerator / denominator;

                vec3 kS = F;
                vec3 kD = vec3(1.0) - kS;
                kD *= 1.0 - metallic;
                
                spot_result += (kD * albedo / PI + specular) * radiance * NdotL * attenuation;

            }
        }
    }

    return spot_result;
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
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// GGX Geometry function using Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// Smith's method for both geometric shadowing
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
