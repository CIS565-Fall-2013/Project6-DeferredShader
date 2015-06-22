#include "ShaderCommon.glsl"

layout(binding = 1) uniform PerDraw_Object
{
    mat4 um4Model;
    mat4 um4InvTrans;
    vec3 uf3Color;
};

in vec3 fs_Normal;
in vec4 fs_Position;

out vec4 out_Normal;
out vec4 out_Position;
out vec4 out_Color;
out vec4 out_GlowMask;

void main()
{
    out_Normal = vec4(normalize(fs_Normal),0.0f);
    out_Position = vec4(fs_Position.xyz,1.0f); //Tuck position into 0 1 range
    out_Color = vec4(uf3Color,1.0);
	out_GlowMask = ufGlowmask * out_Color;
}
