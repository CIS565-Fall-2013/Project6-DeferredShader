#version 430

// Shader constants
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

in vec3 fs_Normal;
in vec4 fs_Position;

out vec4 out_Normal;
out vec4 out_Position;
out vec4 out_Color;
out vec4 out_GlowMask;

void main()
{
    out_Normal = vec4(normalize(fs_Normal),0.0f);
    out_Position = vec4(fs_Position.xyz,1.0f); //Tuck position into 0 1 range
    out_Color = vec4(u_Color,1.0);
	out_GlowMask = glowmask * out_Color;
}
