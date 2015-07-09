#include "ShaderCommon.glsl"

layout(binding = 1) uniform PerDraw_Object
{
    mat4 um4Model;
    mat4 um4InvTrans;
    vec3 uf3Color;
};

in vec3 vo_f3Normal;
in vec4 vo_f3Position;
in vec2 vo_f2Texcoord;

out vec4 out_f4Normal;
out vec4 out_f4Position;
out vec4 out_f4Colour;
out vec4 out_f4GlowMask;

uniform sampler2D t2DDiffuse;
uniform sampler2D t2DNormal;
uniform sampler2D t2DSpecular;

void main()
{
    out_f4Normal = vec4(normalize(vo_f3Normal), 0.0f);
    out_f4Position = vec4(vo_f3Position.xyz, 1.0f); //Tuck position into 0 1 range
    out_f4Colour = texture(t2DDiffuse, vo_f2Texcoord);
    out_f4Colour.xyz = pow(out_f4Colour.xyz, vec3(1.0f/2.2f));  // Gamma correction.
	out_f4GlowMask = ufGlowmask * out_f4Colour;
}
