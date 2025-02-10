#SHADER CUSTOM_MATERIAL_UNIFORMS

// -------------------------------------------
//    Custom Material Property Uniforms 
// -------------------------------------------

// Please Note: You Must Not Leave This Empty. 
// If you do not wish to have Material Properties, then remove this entire block.

struct MaterialUniforms
{
    // Enter Any Custom Variables That Can Be Modified via Editor and Scripting. 
    // For Example: 
    //
    //   float DissolveMaskThrehsold;
    //   sampler2D NoiseTexture; 

    float MyCustomProperty;
};

#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3   aPos;
layout (location = 1) in vec3   aNormal;
layout (location = 2) in vec2   aTexCoord;
layout (location = 3) in vec3   aTangent;
layout (location = 4) in vec3   aBitangent;
layout (location = 5) in mat4   aInstanceMatrix; // Use this as model matrix when engine instances the mesh opposed to u_VertexIn.Model

out VS_OUT {

    vec3 FragPos;
	vec3 ViewPos;
    vec2 TexCoord;

	vec3 TangentFragPos;
	vec3 TangentViewPos;
	mat3 TBN_Matrix;

} vertex_out;

struct VertexData {
    
    mat4 Proj;
    mat4 View;
    mat4 Model;

};

uniform vec3 u_CameraPos;
uniform VertexData u_VertexIn;
uniform bool u_UseInstanceData = false;

// Do Not Change Name ("u_MaterialUniforms")
// The Engine will bind all uniforms in this block, using this name as the key. 
uniform MaterialUniforms u_MaterialUniforms;

void main() {
    
    mat4 model_matrix = (u_UseInstanceData ? aInstanceMatrix : u_VertexIn.Model);
    gl_Position = u_VertexIn.Proj * u_VertexIn.View * model_matrix * vec4(aPos, 1.0);

    // - For Non-Uniform Scaling -
    // Inverse to Correct Distortion of Non-Uniform Scaling
    // Transpose to Ensure Normals are Perpendicular After Inverse
    mat3 normal_matrix = transpose(inverse(mat3(model_matrix))); 

    // Normal Mapping Functions - Construct TBN Matrix
	vec3 T = normalize(normal_matrix * aTangent);
	vec3 N = normalize(normal_matrix * aNormal);
    
    // Gram-Schmidt process
    T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	mat3 TBN = transpose(mat3(T, B, N));

    // Set VertexOut Data
    vertex_out.FragPos = vec3(model_matrix * vec4(aPos, 1.0));
	vertex_out.TexCoord = aTexCoord;
    vertex_out.ViewPos = u_CameraPos;
	vertex_out.TangentFragPos = TBN * vertex_out.FragPos;
	vertex_out.TangentViewPos = TBN * u_CameraPos;
	vertex_out.TBN_Matrix = TBN;
}

#SHADER FRAGMENT

#version 450 core

class PBRMaterial {
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

    // DO NOT USE - this is for SSBO alignment purposes ONLY (8 BYTES)
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
    
    // DO NOT USE - this is for SSBO alignment purposes ONLY (8 BYTES)
	uint m_Padding1;
	uint m_Padding2;
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
	uint shadowLayerIndex;
	
    // DO NOT USE - this is for SSBO alignment purposes ONLY (4 BYTES)
	uint m_Padding1;
};

struct VisibleIndex { uint index; };

// Point Lights SSBO
layout(std430, binding = 0) readonly buffer PL_Buffer           { PointLight data[];    } PL_Buffer_Data;
layout(std430, binding = 1) readonly buffer PL_IndiciesBuffer   { VisibleIndex data[];  } PL_IndiciesBuffer_Data;

// Spot Lights SSBO
layout(std430, binding = 2) readonly buffer SL_Buffer           { SpotLight data[];     } SL_Buffer_Data;
layout(std430, binding = 3) readonly buffer SL_IndiciesBuffer   { VisibleIndex data[];  } SL_IndiciesBuffer_Data;
    
// Directional Lights SSBO
layout(std430, binding = 4) readonly buffer DL_Buffer           { DirLight data[];      } DL_Buffer_Data;

// Shadow Light Space Matrix SSBO
layout(std430, binding = 5) readonly buffer DL_Shadow_LightSpaceMatrices_Buffer { mat4 data[]; } DL_Shadow_LightSpaceMatrices_Buffer_Data;
layout(std430, binding = 6) readonly buffer SL_Shadow_LightSpaceMatrices_Buffer { mat4 data[]; } SL_Shadow_LightSpaceMatrices_Buffer_Data;

// Shader In/Out Variables
in VS_OUT {
    vec3 FragPos;
	vec3 ViewPos;
    vec2 TexCoord;
	vec3 TangentFragPos;
	vec3 TangentViewPos;
	mat3 TBN_Matrix;
} fragment_in;

struct VertexData {
    mat4 Proj;
    mat4 View;
    mat4 Model;
};

// Standard Uniform Variables
uniform VertexData u_VertexIn;
uniform PBRMaterial u_Material;
uniform sampler2DArray u_DL_ShadowMapArray;
uniform sampler2DArray u_SL_ShadowMapArray;
uniform samplerCubeArray u_PL_ShadowCubeMapArray;

// Forward Plus Uniform Variables
uniform int u_TilesX;
uniform ivec2 u_ScreenSize;
uniform bool u_ShowLightComplexity;

// Camera
uniform float u_Near;
uniform float u_Far;

// Depth Sampling
uniform int u_Samples; // Number of Samples Per Pixel
bool IsMultiSampled() { return (u_Samples > 1); }
float LouronSampleDepthTexture(vec2 frag_coord); // Pass gl_FragCoord.xy, or any other frag coordinate you are trying to sample
float LouronLineariseDepth(float depth);

// Forward Plus Local Variables
uint index;
ivec2 location = ivec2(gl_FragCoord.xy);
ivec2 tileID = location / ivec2(16, 16);
const uint MAX_DIRECTIONAL_LIGHTS = 10;
const uint MAX_POINT_LIGHTS = 1024;
const uint MAX_SPOT_LIGHTS = 1024;

const float PI = 3.14159265359;

// -------------------------------------------
//     Forward Declare Lighting Functions 
// -------------------------------------------
vec3 LouronCalculatePBR_Lighting_All(vec3 normal, vec3 fragPos, vec3 view_direction, vec3 albedo, float metallic, float roughness);
vec3 LouronCalculatePBR_Lighting_Directional(vec3 normal, vec3 view_direction, vec3 albedo, float metallic, float roughness);
vec3 LouronCalculatePBR_Lighting_Point(vec3 normal, vec3 fragPos, vec3 view_direction, vec3 albedo, float metallic, float roughness);
vec3 LouronCalculatePBR_Lighting_Spot(vec3 normal, vec3 fragPos, vec3 view_direction, vec3 albedo, float metallic, float roughness);
vec3 LouronCalculateHDR_Reinhard(vec3 colour);
vec3 LouronCalculateHDR_Exposure(vec3 colour, float exposure);
vec3 LouronCalculateGammaCorrection(vec3 colour, float gamma);

float PointLightShadowCalculation(vec3 light_pos, float point_light_far_plane, uint cubemap_array_index, int samples);
float SpotLightShadowCalculation(vec3 normal, vec3 light_pos, vec3 light_direction, uint shadow_light_index, bool soft_shadow);
float DirectionalLightShadowCalculation(vec3 normal, vec3 light_direction, uint shadow_light_index, float[5] cascade_plane_distances, bool soft_shadow);

vec3 LouronGetNormal();

layout (location = 0) out vec4 out_FragColour;

// Do Not Change Name ("u_MaterialUniforms")
// The Engine will bind all uniforms in this block, using this name as the key. 
uniform MaterialUniforms u_MaterialUniforms; 

vec3 CalculateToonLighting(vec3 normal, vec3 radiance, float shadow_factor, vec3 light_direction, vec3 view_direction, vec3 albedo) {
    float NdotL = max(dot(normal, light_direction), 0.0);

    // Quantize NdotL into discrete steps (Cel shading effect)
    float toonShading = smoothstep(0.0, 0.1, NdotL) * 0.3 +
                        smoothstep(0.1, 0.3, NdotL) * 0.6 +
                        smoothstep(0.3, 1.0, NdotL) * 1.0;

    // Simple Rim Light effect (adds stylized highlight around edges)
    float rim = 1.0 - max(dot(normal, view_direction), 1.0);
    rim = smoothstep(0.5, 1.0, rim) * (1.0 - u_Material.roughness); // Controls rim strength

    // Specular highlight (simplified with a toonish step)
    vec3 halfway_direction = normalize(light_direction + view_direction);
    float specular_intensity = pow(max(dot(normal, halfway_direction), 0.0), 64.0) * (1.0 - u_Material.roughness);
    specular_intensity = specular_intensity > 0.5 ? 1.0 : 0.0; // Toon specular

    return shadow_factor * radiance * (toonShading * albedo + rim + specular_intensity);
}

vec3 LouronCalculateToonLighting_Directional(vec3 normal, vec3 view_direction, vec3 albedo) {
    vec3 directional_result = vec3(0.0);

    for (int i = 0; i < DL_Buffer_Data.data.length() && !DL_Buffer_Data.data[i].lastLight; i++) {
        if (!DL_Buffer_Data.data[i].activeLight)
            continue;

        DirLight light = DL_Buffer_Data.data[i];
        vec3 light_direction = fragment_in.TBN_Matrix * -light.direction.xyz;

        if (dot(normal, light_direction) <= 0.01)
            continue;

        vec3 radiance = light.colour.rgb * light.intensity;
        float shadow_factor = 1.0;
        
        if (light.shadowLightIndex != -1)
            shadow_factor -= DirectionalLightShadowCalculation(normal, -light.direction.xyz, light.shadowLightIndex, light.shadowCascadePlaneDistances, (light.shadowCastingType == 2));
        
        shadow_factor = shadow_factor > 0.5 ? 1.0 : 0.0;
        directional_result += CalculateToonLighting(normal, radiance, shadow_factor, light_direction, view_direction, albedo);
    }


    return directional_result;
}

// ------------------------------------------------------------------------------------------
//                                  Main Shader Function 
// ------------------------------------------------------------------------------------------
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

	vec3 normal = LouronGetNormal();
    
    vec3 result;
    result = LouronCalculateToonLighting_Directional(normal, view_direction, albedo);
    result = LouronCalculateGammaCorrection(result, 2.2);

    float edge = max(dot(normal, view_direction), 0.0);
    edge = smoothstep(0.1, 0.2, edge);
    result *= edge;

    out_FragColour = vec4(result, 1.0);
}

// -------------------------------------------
//              PBR Functions 
// -------------------------------------------

float DistributionGGX(vec3 N, vec3 H, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

vec3 CalculatePBR(vec3 normal, vec3 radiance, float shadow_factor, vec3 light_direction, vec3 view_direction, vec3 albedo, float metallic, float roughness)
{
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

    return shadow_factor * ((kD * albedo / PI + specular) * radiance * NdotL);
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

// -------------------------------------------
//              Attenuation 
// -------------------------------------------
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

// -------------------------------------------
//                Shadows 
// -------------------------------------------
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

float SpotLightShadowCalculation(vec3 normal, vec3 light_pos, vec3 light_direction, uint shadow_light_index, bool soft_shadow)
{
    // Transform fragment position to light space
    vec4 frag_pos_light_space = SL_Shadow_LightSpaceMatrices_Buffer_Data.data[int(shadow_light_index)] * vec4(fragment_in.FragPos, 1.0);
    
    // Perform perspective divide and transform to [0,1] range
    vec3 projection_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    projection_coords = projection_coords * 0.5 + 0.5;

    // Get depth of current fragment from light's perspective
    float current_depth = projection_coords.z;

    // Prevent gaps by relaxing the out-of-bounds check
    if (current_depth > 1.00)
        return 0.0;
    
    // Percentage Closer Filtering (PCF)
    float shadow = 0.0;
    if (soft_shadow)
    {
        vec2 texel_size = 1.0 / vec2(textureSize(u_SL_ShadowMapArray, 0));
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcf_depth = texture(u_SL_ShadowMapArray, vec3(projection_coords.xy + vec2(x, y) * texel_size, int(shadow_light_index))).r;
                shadow += current_depth > pcf_depth ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0; // Average over 3x3 region
    }
    else
    {
        // Hard shadow sampling
        float shadow_depth = texture(u_SL_ShadowMapArray, vec3(projection_coords.xy, int(shadow_light_index))).r;
        shadow = current_depth > shadow_depth ? 1.0 : 0.0;
    }

    return shadow;
}

vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

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

// -------------------------------------------
//        InBuilt Lighting Calculations 
// -------------------------------------------

// Calculate Directional Light Lighting in Scene (MAX 10)
vec3 LouronCalculatePBR_Lighting_Directional(vec3 normal, vec3 view_direction, vec3 albedo, float metallic, float roughness) {
    
    vec3 directional_result = vec3(0.0);

    for (int i = 0; i < DL_Buffer_Data.data.length() && !DL_Buffer_Data.data[i].lastLight; i++) {

        if (DL_Buffer_Data.data[i].activeLight == false)
            continue;

        DirLight light = DL_Buffer_Data.data[i];
        
        vec3 light_direction = fragment_in.TBN_Matrix * -light.direction.xyz;
        
        if (dot(normal, light_direction) <= 0.01)
            continue;

        vec3 radiance = light.colour.rgb * light.intensity;
        float shadow_factor = 1.0;
        
        if(light.shadowLightIndex != -1)
            shadow_factor -= DirectionalLightShadowCalculation(normal, -light.direction.xyz, light.shadowLightIndex, light.shadowCascadePlaneDistances, (light.shadowCastingType == 2));

        directional_result += CalculatePBR(normal, radiance, shadow_factor, light_direction, view_direction, albedo, metallic, roughness);
    }

    return directional_result;
}

// Calculate Point Light Lighting in Scene
vec3 LouronCalculatePBR_Lighting_Point(vec3 normal, vec3 fragPos, vec3 view_direction, vec3 albedo, float metallic, float roughness) {
    vec3 point_result = vec3(0.0);
    
    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint index = tileID.y * u_TilesX + tileID.x;
    uint offset = index * MAX_POINT_LIGHTS;

    int lightCount = 0;

    for (int i = 0;  i < MAX_POINT_LIGHTS && PL_IndiciesBuffer_Data.data[offset + i].index != -1; i++) {

        uint lightIndex = PL_IndiciesBuffer_Data.data[offset + i].index;

        if (!PL_Buffer_Data.data[lightIndex].activeLight)
            continue;
        
        vec3 tang_light_pos = fragment_in.TBN_Matrix * PL_Buffer_Data.data[lightIndex].position.xyz;
        float dist = length(PL_Buffer_Data.data[lightIndex].position.xyz - fragment_in.FragPos);
        if (dist < PL_Buffer_Data.data[lightIndex].radius) {
            
            PointLight light = PL_Buffer_Data.data[lightIndex];
            vec3 light_direction = normalize(tang_light_pos - fragPos);
           
            vec3 radiance = light.colour.rgb * light.intensity * Attenuation_InverseSquareLawWithFalloff(light.radius, dist);
            float shadow_factor = 1.0;
            
            if (light.shadowLayerIndex != -1)
                shadow_factor -= PointLightShadowCalculation(light.position.xyz, light.radius, light.shadowLayerIndex, (light.shadowCastingType == 1) ? 1 : 20);

            point_result += CalculatePBR(normal, radiance, shadow_factor, light_direction, view_direction, albedo, metallic, roughness);
            
            lightCount++; // For Light Complexity Heat Map
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
vec3 LouronCalculatePBR_Lighting_Spot(vec3 normal, vec3 fragPos, vec3 view_direction, vec3 albedo, float metallic, float roughness) {

    vec3 spot_result = vec3(0.0);
    
    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint index = tileID.y * u_TilesX + tileID.x;
    uint offset = index * MAX_SPOT_LIGHTS;

    int lightCount = 0;

    for (int i = 0;  i < MAX_SPOT_LIGHTS && SL_IndiciesBuffer_Data.data[offset + i].index != -1; i++) {

        uint lightIndex = SL_IndiciesBuffer_Data.data[offset + i].index;

        if (SL_Buffer_Data.data[lightIndex].activeLight == false)
            continue;
        
		vec3 tang_light_pos = fragment_in.TBN_Matrix * SL_Buffer_Data.data[lightIndex].position.xyz;
        float dist = length(tang_light_pos - fragPos);
        if (dist < SL_Buffer_Data.data[lightIndex].range) {

            SpotLight light = SL_Buffer_Data.data[lightIndex];
            vec3 light_direction = normalize(tang_light_pos - fragPos);

            // Check if frag is within the influence of the cone of the spot light.
            float theta = dot(light_direction, normalize(fragment_in.TBN_Matrix * -light.direction.xyz));
            float outer_cut_off = cos(radians(light.angle * 0.5));
            if (theta <= outer_cut_off)
                continue;

            float inner_cut_off = cos(radians(light.angle * 0.25)); // Inner cutoff is 25%
            float epsilon = cos(inner_cut_off - outer_cut_off);
            
            float angle_factor = 1.0 - smoothstep(epsilon, outer_cut_off, theta); // Calculate the angle factor to determine the falloff from the center to the edge of the cone
            float spot_light_intensity = light.intensity * angle_factor; // Calculate the spotlight intensity based on the angle factor
            
            vec3 radiance = light.colour.rgb * spot_light_intensity * Attenuation_InverseSquareLawWithFalloff(light.range, dist);
            float shadow_factor = 1.0;
            
            if (light.shadowLayerIndex != -1) 
                shadow_factor -= SpotLightShadowCalculation(normal, light.position.xyz, -light.direction.xyz, light.shadowLayerIndex, (light.shadowCastingType == 2));
            
            spot_result += CalculatePBR(normal, radiance, shadow_factor, light_direction, view_direction, albedo, metallic, roughness);
            
            lightCount++; // For Light Complexity Heat Map
        }
    }
    
    // If heatmap is enabled, blend the original result with the heatmap color based on light count
    if (u_ShowLightComplexity && lightCount > 0) {
        float heatmapValue = float(lightCount) / 10.0;
        vec3 heatmapColor = vec3(heatmapValue, 0.0, 1.0 - heatmapValue);
        spot_result = mix(spot_result, heatmapColor, 0.5); // Adjust blend factor as needed
    }

    return spot_result;
}

vec3 LouronCalculatePBR_Lighting_All(vec3 normal, vec3 fragPos, vec3 view_direction, vec3 albedo, float metallic, float roughness)
{
    vec3 result = vec3(0.0);

    result += LouronCalculatePBR_Lighting_Directional(normal, view_direction, albedo, metallic, roughness);
    result += LouronCalculatePBR_Lighting_Point(normal, fragPos, view_direction, albedo, metallic, roughness);
    result += LouronCalculatePBR_Lighting_Spot(normal, fragPos, view_direction, albedo, metallic, roughness);

    return result;
}

vec3 LouronCalculateHDR_Reinhard(vec3 colour)
{
    return colour / (colour + vec3(1.0));
}

vec3 LouronCalculateHDR_Exposure(vec3 colour, float exposure)
{
    return vec3(1.0) - exp(-colour * exposure);
}

vec3 LouronCalculateGammaCorrection(vec3 colour, float gamma)
{
    return pow(colour, vec3(1.0/gamma)); 
}

vec3 LouronGetNormal()
{
	return normalize((texture(u_Material.normalTexture, fragment_in.TexCoord).rgb) * 2.0 - 1.0);
}

// -------------------------------------------
//        InBuilt Helper Functions
// -------------------------------------------

// Declared Depth Samplers Down 
// Here to Avoid Direct Access.
// Please prefer to use the LouronSampleDepthTexture Method.
uniform sampler2D u_Depth;
uniform sampler2DMS u_Depth_MS;
float LouronSampleDepthTexture(vec2 frag_coord)
{
    vec2 depthUV = frag_coord / u_ScreenSize;

    if(IsMultiSampled())
    {
        float sum = 0.0;
        for (int i = 0; i < u_Samples; ++i) 
        {
            sum += texelFetch(u_Depth_MS, ivec2(frag_coord), i).r;
        }
        return sum / float(u_Samples);
    }
    
    return texture(u_Depth, depthUV).r;
}

// Depth Texture is Non-Linearised, Use This to Linearise.
float LouronLineariseDepth(float depth) {
	float zNdc = 2 * depth - 1;
	float zEye = (2 * u_Far * u_Near) / ((u_Far + u_Near) - zNdc * (u_Far - u_Near));
	float linearDepth = (zEye - u_Near) / (u_Far - u_Near);
	return 1.0 - linearDepth;
}