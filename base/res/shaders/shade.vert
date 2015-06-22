#include "ShaderCommon.glsl"

in vec3 in_f3Position;
in vec2 in_f2Texcoord;

out vec2 fs_Texcoord;

void main() 
{
    fs_Texcoord = in_f2Texcoord;
    gl_Position = vec4(in_f3Position, 1.0f);
}
