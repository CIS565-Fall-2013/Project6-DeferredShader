#include "ShaderCommon.glsl"

layout(binding = 1) uniform PerDraw_Object
{
    mat4 um4Model;
    mat4 um4InvTrans;
    vec3 uf3Color;
};

in  vec3 in_f3Position;
in  vec3 in_f3Normal;

out vec3 vo_f3Normal;
out vec4 vo_f3Position;

void main() 
{
    vo_f3Normal = (um4InvTrans*vec4(in_f3Normal,0.0f)).xyz;
    vec4 world = um4Model * vec4(in_f3Position, 1.0);
    vec4 camera = um4View * world;
    vo_f3Position = camera;
    gl_Position = um4Persp * camera;
}
