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
	vec3 ViewPos;

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
    vertex_out.ViewPos = u_CameraPos;

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
    
    float[5] shadowCascadePlaneDistances;

    uint shadowCastingType;
	uint shadowLightIndex;

    uint m_Padding1;
    uint m_Padding2;
};

struct PointLight {
    vec4 position;

    vec4 colour;

    float radius;
    float intensity;

    bool activeLight;
    bool lastLight;

	uint shadowCastingType;
	uint shadowLayerIndex;
    
    // DO NOT USE - this is for SSBO alignment purposes ONLY (12 BYTES)
	float m_Padding2;
	float m_Padding3;
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
    
	uint shadowCastingType;
	
    // DO NOT USE - this is for SSBO alignment purposes ONLY (8 BYTES)
	float m_Padding1;
	float m_Padding2;
   
};

struct VisibleIndex {
    uint index;
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

layout(std430, binding = 5) readonly buffer DL_Shadow_LightSpaceMatrices_Buffer {
    mat4 data[];
} DL_Shadow_LightSpaceMatrices_Buffer_Data;

// Shader In/Out Variables
in VS_OUT {

    vec3 FragPos;
    vec2 TexCoord;
	vec3 ViewPos;

	mat3 TBN_Matrix;
	vec3 TangentViewPos;
	vec3 TangentFragPos;

} fragment_in;

struct VertexData {
    
    mat4 Proj;
    mat4 View;
    mat4 Model;

};

uniform VertexData u_VertexIn;

layout (location = 0) out vec4 out_FragColour;

// Standard Uniform Variables
uniform PBRMaterial u_Material;

// Forward Plus Uniform Variables
uniform int u_TilesX;
uniform ivec2 u_ScreenSize;
uniform sampler2D u_Depth;
uniform bool u_ShowLightComplexity;
uniform sampler2DArray u_DL_ShadowMapArray;
uniform samplerCubeArray u_PL_ShadowCubeMapArray;

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

float Attenuation_InverseSquareLawWithFalloff(float light_radius, float distance_from_light) {
    float attenuation = 1.0 / (distance_from_light * distance_from_light); // Inverse square law
    attenuation = 2.0 / ((distance_from_light * distance_from_light) + (light_radius * light_radius) + (distance_from_light * sqrt((distance_from_light * distance_from_light) + (light_radius * light_radius))));

    // Normalize the distance to the light radius and create a smooth falloff
    float normalised_distance = distance_from_light / light_radius;
    if (normalised_distance > 1.0) {
        attenuation = 0.0; // Outside the light radius
    } else {
        float smooth_falloff = pow(1.0 - normalised_distance, 2.0); // Quadratic smooth falloff
        attenuation *= smooth_falloff;
    }

    return attenuation * 100.0;
}

vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);


float DirectionalLightShadowCalculation(vec3 normal, vec3 light_direction, uint shadow_light_index, float[5] cascade_plane_distances, bool soft_shadow)
{    
    // Transform fragment position to view space
    vec4 frag_pos_view_space = u_VertexIn.View * vec4(fragment_in.FragPos, 1.0);
    float depth_value = abs(frag_pos_view_space.z);

    // Find the active cascade layer and compute blend factor
    int cascade_layer = 4; // Default to the last cascade
    for (int i = 0; i < 5; ++i)
    {
        if (depth_value < cascade_plane_distances[i])
        {
            cascade_layer = i;
            break;
        }
    }
    
    vec4 frag_pos_light_space = DL_Shadow_LightSpaceMatrices_Buffer_Data.data[int(shadow_light_index * 5) + cascade_layer] * vec4(fragment_in.FragPos, 1.0);

    // Perform perspective divide and transform to [0,1] range
    vec3 projection_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    projection_coords = projection_coords * 0.5 + 0.5;

    // Get depth of current fragment from light's perspective
    float current_depth = projection_coords.z;

    // Prevent gaps by relaxing the out-of-bounds check
    if (current_depth > 1.00) 
        return 0.0; 

    // Calculate bias (based on normal and light direction)
    float bias = max(0.05 * (1.0 - dot(normal, light_direction)), 0.005);
    const float bias_mod = 0.5;
    bias *= 1 / (cascade_plane_distances[cascade_layer] * bias_mod);

    // Percentage Closer Filtering (PCF)
    float shadow = 0.0;

    if (soft_shadow)
    {
        vec2 texel_size = 1.0 / vec2(textureSize(u_DL_ShadowMapArray, 0));
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcf_depth = texture(u_DL_ShadowMapArray, vec3(projection_coords.xy + vec2(x, y) * texel_size, int(shadow_light_index * 5) + cascade_layer)).r;
                shadow += (current_depth - bias) > pcf_depth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0; // Average over 3x3 region
    } 
    else
    {
        // Hard shadow sampling (no softening or filtering)
        float shadow_depth = texture(u_DL_ShadowMapArray, vec3(projection_coords.xy, int(shadow_light_index * 5) + cascade_layer)).r;
        shadow = (current_depth - bias) > shadow_depth ? 1.0 : 0.0;  // If current depth is greater than shadow depth, it's in shadow
    }

    return shadow;
}

float PointLightShadowCalculation(vec3 light_pos, float point_light_far_plane, uint cubemap_array_index, int samples) {
    
    vec3 fragToLight = fragment_in.FragPos - light_pos; 
    float currentDepth = length(fragToLight);
    
    float shadow = 0.0;
    float bias = 0.15;

    float view_distance = length(fragment_in.ViewPos - fragment_in.FragPos);
    float disk_radius = (1.0 + (view_distance / point_light_far_plane)) / 25.0;
    
    for (int i = 0; i < samples; ++i) {
        vec3 sampleOffset = fragToLight + gridSamplingDisk[i] * disk_radius;
        vec4 shadowCoord = vec4(sampleOffset, float(cubemap_array_index));
        
        // Sample shadow map using array index
        float closestDepth = float(texture(u_PL_ShadowCubeMapArray, shadowCoord));
        
        // Compare depths
        if (currentDepth - bias > closestDepth * point_light_far_plane) {
            shadow += 1.0;
        }
    }

    shadow /= float(samples);
    
    return shadow;
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
    //result = result / (result + vec3(1.0)); // Reinhard Tone Mapping
    result = vec3(1.0) - exp(-result * 5.0); // Exposure Tone Mapping

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
        
        float shadow_factor = 1.0;

        if (light.shadowCastingType == 1)
        {
            shadow_factor -= DirectionalLightShadowCalculation(normal, -light.direction.xyz, light.shadowLightIndex, light.shadowCascadePlaneDistances, false);
        } 
        else if (light.shadowCastingType == 2)
        {
            shadow_factor -= DirectionalLightShadowCalculation(normal, -light.direction.xyz, light.shadowLightIndex, light.shadowCascadePlaneDistances, true);
        }

        directional_result += shadow_factor * ((kD * albedo / PI + specular) * radiance * NdotL);
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

    int lightCount = 0;

    for (int i = 0;  i < MAX_POINT_LIGHTS && PL_IndiciesBuffer_Data.data[offset + i].index != -1; i++) {
        uint lightIndex = PL_IndiciesBuffer_Data.data[offset + i].index;
        lightCount++;

        if (!PL_Buffer_Data.data[lightIndex].activeLight)
            continue;
        
        vec3 tang_light_pos = fragment_in.TBN_Matrix * PL_Buffer_Data.data[lightIndex].position.xyz;
        float dist = length(tang_light_pos - fragPos);

        if (dist < PL_Buffer_Data.data[lightIndex].radius) {
            
            PointLight light = PL_Buffer_Data.data[lightIndex];
            vec3 light_direction = normalize(tang_light_pos - fragPos);
           
            float attenuation = Attenuation_InverseSquareLawWithFalloff(light.radius, dist);
            vec3 radiance = light.colour.rgb * light.intensity * attenuation;

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
            
            float shadow_calculation = 0.0;
            if(light.shadowCastingType == 1) // HARD Shadows
                shadow_calculation = PointLightShadowCalculation(PL_Buffer_Data.data[lightIndex].position.xyz, light.radius, light.shadowLayerIndex, 1);
            if(light.shadowCastingType == 2) // SOFT Shadows
                shadow_calculation = PointLightShadowCalculation(PL_Buffer_Data.data[lightIndex].position.xyz, light.radius, light.shadowLayerIndex, 20);

            float shadow_factor = 1.0 - shadow_calculation; // Reduce contribution based on shadow

            // Final contribution, reduced by shadow factor
            point_result += shadow_factor * ((kD * albedo / PI + specular) * radiance * NdotL);

        }
    }
    
    // If heatmap is enabled, blend the original result with the heatmap color based on light count
    if (u_ShowLightComplexity && lightCount > 0) {
        float heatmapValue = float(lightCount) / 10.0;
        vec3 heatmapColor = vec3(heatmapValue, 0.0, 1.0 - heatmapValue);
        point_result = mix(point_result, heatmapColor, 0.5); // Adjust blend factor as needed
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

                float attenuation = Attenuation_InverseSquareLawWithFalloff(light.range, dist);
                vec3 radiance = light.colour.rgb * spot_light_intensity * attenuation;
                
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
                
                spot_result += (kD * albedo / PI + specular) * radiance * NdotL;

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
