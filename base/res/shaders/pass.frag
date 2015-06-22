#include "ShaderCommon.glsl"

layout(binding = 1) uniform PerDraw_Object
{
    mat4 um4Model;
    mat4 um4InvTrans;
    vec3 uf3Color;
};

in vec3 fs_Normal;
in vec4 fs_Position;

out vec4 out_f4Normal;
out vec4 out_f4Position;
out vec4 out_f4Colour;
out vec4 out_f4GlowMask;

void main()
{
    out_f4Normal = vec4(normalize(fs_Normal),0.0f);
    out_f4Position = vec4(fs_Position.xyz,1.0f); //Tuck position into 0 1 range
    out_f4Colour = vec4(uf3Color,1.0);
	out_f4GlowMask = ufGlowmask * out_f4Colour;
}
