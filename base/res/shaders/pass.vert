#include "ShaderCommon.glsl"

layout(binding = 1) uniform PerDraw_Object
{
    mat4 um4Model;
    mat4 um4InvTrans;
    vec3 uf3Color;
};

in  vec3 Position;
in  vec3 Normal;

out vec3 fs_Normal;
out vec4 fs_Position;

void main() 
{
    fs_Normal = (um4InvTrans*vec4(Normal,0.0f)).xyz;
    vec4 world = um4Model * vec4(Position, 1.0);
    vec4 camera = um4View * world;
    fs_Position = camera;
    gl_Position = um4Persp * camera;
}
