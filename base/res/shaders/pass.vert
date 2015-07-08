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

out vec3 vo_f3Normal;
out vec4 vo_f3Position;
out vec2 vo_f2Texcoord;

void main() 
{
    vo_f3Normal = (um4InvTrans * vec4(in_f3Normal,0.0f)).xyz;
    vec4 f4World = um4Model * vec4(in_f3Position, 1.0);
    vec4 f4Camera = um4View * f4World;
    vo_f3Position = f4Camera;
    vo_f2Texcoord = in_f2Texcoord;
    
    gl_Position = um4Persp * f4Camera;
}
