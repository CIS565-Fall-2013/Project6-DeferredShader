#version 330


uniform mat4x4 u_Model;
uniform mat4x4 u_View;
uniform mat4x4 u_Persp;
uniform mat4x4 u_InvTrans;

in  vec3 Position;
in  vec3 Normal;
in  vec4 Tangent;
in  vec2 Texcoord;

out vec3 fs_Normal;
out vec3 fs_Tangent;
out vec3 fs_Binormal;
out vec4 fs_Position;
out vec2 fs_Texcoord;

void main(void) {
	
    fs_Normal =normalize((u_InvTrans*vec4(Normal,0.0f)).xyz);
	fs_Tangent =normalize((u_InvTrans*vec4(Tangent.xyz,0.0f)).xyz);
	fs_Binormal =normalize((u_InvTrans*vec4(normalize(cross(Normal, Tangent.xyz)*Tangent.w),0.0f)).xyz);
	
    vec4 world = u_Model * vec4(Position, 1.0);
    vec4 camera = u_View * world;
    fs_Position = camera;
    gl_Position = u_Persp * camera;
	fs_Texcoord = Texcoord;
}
