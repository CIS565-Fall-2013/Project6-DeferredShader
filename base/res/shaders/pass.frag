#include "ShaderCommon.glsl"

layout(binding = 1) uniform PerDraw_Object
{
    mat4 um4Model;
    mat4 um4InvTrans;
    vec3 uf3Color;
};

in vec3 vo_f3Normal;
in vec4 vo_f4Position;
in vec2 vo_f2Texcoord;

out vec4 out_f4Normal;
out vec4 out_f4Position;
out vec4 out_f4Colour;

uniform sampler2D t2DDiffuse;
uniform sampler2D t2DNormal;
uniform sampler2D t2DSpecular;

void main()
{
    out_f4Normal = vec4(StoreFragmentNormal(vo_f3Normal), ufGlowmask);
    out_f4Position = vo_f4Position;
    out_f4Colour = texture(t2DDiffuse, vo_f2Texcoord);
}
