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
in vec3 vo_f3Tangent;
in vec3 vo_f3Bitangent;

out vec4 out_f4Normal;
out vec4 out_f4Position;
out vec4 out_f4Colour;

uniform sampler2D t2DDiffuse;
uniform sampler2D t2DNormal;
uniform sampler2D t2DSpecular;

void main()
{
    vec3 f3Tangent = (dot(vo_f3Tangent, vo_f3Tangent) > 1e-6) ? normalize(vo_f3Tangent) : vec3(0);
    vec3 f3Bitangent = (dot(vo_f3Bitangent, vo_f3Bitangent) > 1e-6) ? normalize(vo_f3Bitangent) : vec3(0);
    vec3 f3Normal = GetNormalMappedNormal(t2DNormal, vo_f2Texcoord, f3Tangent, f3Bitangent, normalize(vo_f3Normal));

    out_f4Normal = vec4(StoreFragmentNormal((um4InvTrans * vec4(f3Normal, 0.0f)).xyz), ufGlowmask);
    out_f4Position = vo_f4Position;
    out_f4Colour = texture(t2DDiffuse, vo_f2Texcoord);
}
