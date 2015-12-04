#include "ShaderCommon.glsl"

layout(binding = 1) uniform PerDraw_Object
{
    mat4 um4Model;
    mat4 um4InvTrans;
    vec3 uf3Color;
};

in vec3 in_f3Position;
in vec3 in_f3Normal;
in vec2 in_f2Texcoord;
in vec3 in_f3Tangent;

out vec3 vo_f3Normal;
out vec4 vo_f4Position;
out vec2 vo_f2Texcoord;
out vec3 vo_f3Tangent;
out vec3 vo_f3Bitangent;

void main() 
{
    vo_f3Normal = in_f3Normal;
    vec4 f4Camera = um4View * um4Model * vec4(in_f3Position, 1.0);
    vo_f4Position = f4Camera;
    vo_f2Texcoord = in_f2Texcoord;
    vo_f3Tangent = in_f3Tangent;
    vo_f3Bitangent = cross(in_f3Normal, in_f3Tangent);

    gl_Position = um4Persp * f4Camera;
}
