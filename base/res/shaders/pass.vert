#version 430

uniform PerFrame
{
    mat4 u_View;
    mat4 u_Persp;
    float u_Far;
    float u_Near;
    float u_InvScrHeight;
    float u_InvScrWidth;
    float u_mouseTexX;
    float u_mouseTexY;
    float glowmask;
    int u_OcclusionType;
    int u_DisplayType;
    int u_ScreenWidth;
    int u_ScreenHeight;
    bool u_BloomOn;
    bool u_toonOn;
    bool u_DOFOn;
    bool u_DOFDebug;
};

uniform PerDraw_Object
{
    mat4 u_Model;
    mat4 u_InvTrans;
    vec3 u_Color;
};

in  vec3 Position;
in  vec3 Normal;

out vec3 fs_Normal;
out vec4 fs_Position;

void main() 
{
    fs_Normal = (u_InvTrans*vec4(Normal,0.0f)).xyz;
    vec4 world = u_Model * vec4(Position, 1.0);
    vec4 camera = u_View * world;
    fs_Position = camera;
    gl_Position = u_Persp * camera;
}
