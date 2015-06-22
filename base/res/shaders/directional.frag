#include "ShaderCommon.glsl"
#include "LightingCommon.glsl"

// Textures
uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

in vec2 fs_Texcoord;
out vec4 out_Color;

const float occlusion_strength = 1.5f;
void main() 
{
    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth);

    vec3 normal = SampleTexture(u_Normaltex, fs_Texcoord);
    vec3 position = SampleTexture(u_Positiontex, fs_Texcoord);
    vec3 color = SampleTexture(u_Colortex, fs_Texcoord);
    vec3 light = uf4Light.xyz;
    float lightRadius = uf4Light.w;

    float diffuse = max(0.0, dot(normalize(light), normal));
    out_Color = vec4(color*ufLightIl*diffuse, 1.0f);
}
