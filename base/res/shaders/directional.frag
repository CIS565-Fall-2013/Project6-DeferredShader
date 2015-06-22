#include "ShaderCommon.glsl"
#include "LightingCommon.glsl"

// Textures
uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

in vec2 vo_f2TexCoord;
out vec4 out_f4Colour;

const float occlusion_strength = 1.5f;
void main() 
{
    float exp_depth = texture(u_Depthtex, vo_f2TexCoord).r;
    float lin_depth = linearizeDepth(exp_depth);

    vec3 normal = SampleTexture(u_Normaltex, vo_f2TexCoord);
    vec3 position = SampleTexture(u_Positiontex, vo_f2TexCoord);
    vec3 color = SampleTexture(u_Colortex, vo_f2TexCoord);
    vec3 light = uf4Light.xyz;
    float lightRadius = uf4Light.w;

    float diffuse = max(0.0, dot(normalize(light), normal));
    out_f4Colour = vec4(color*ufLightIl*diffuse, 1.0f);
}
