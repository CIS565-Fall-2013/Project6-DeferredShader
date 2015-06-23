#include "ShaderCommon.glsl"

layout(binding = 1) uniform PerDraw_Object
{
    mat4 um4Model;
    mat4 um4InvTrans;
    vec3 uf3Color;
};

in vec3 vo_f3Normal;
in vec4 vo_f3Position;

out vec4 out_f4Normal;
out vec4 out_f4Position;
out vec4 out_f4Colour;
out vec4 out_f4GlowMask;

void main()
{
    out_f4Normal = vec4(normalize(vo_f3Normal), 0.0f);
    out_f4Position = vec4(vo_f3Position.xyz, 1.0f); //Tuck position into 0 1 range
    out_f4Colour = vec4(uf3Color, 1.0f);
	out_f4GlowMask = ufGlowmask * out_f4Colour;
}
