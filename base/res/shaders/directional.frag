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
    vec3 f3Normal = SampleFragmentNormal(u_Normaltex, vo_f2TexCoord);
    vec3 f3Colour = SampleTexture(u_Colortex, vo_f2TexCoord);
    float fDiffuse = max(0.0, dot(uf4DirecLightDir.xyz, f3Normal));
    out_f4Colour = vec4(f3Colour * uf4DirecLightDir.w * fDiffuse, 1.0f);
}
