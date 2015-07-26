#include "ShaderCommon.glsl"

// Textures
uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

in vec2 vo_f2TexCoord;
out vec4 out_f4Colour;

const float occlusion_strength = 1.5f;
void main() 
{
    float fLinearDepth = texture(u_Depthtex, vo_f2TexCoord).r;
    fLinearDepth = linearizeDepth(fLinearDepth);

    vec3 f3Normal = SampleTexture(u_Normaltex, vo_f2TexCoord);
    vec3 f3Position = SampleTexture(u_Positiontex, vo_f2TexCoord);
    vec3 f3Colour = SampleTexture(u_Colortex, vo_f2TexCoord);

    switch (uiDisplayType) 
    {
        case DISPLAY_DEPTH:
            out_f4Colour = vec4(vec3(fLinearDepth), 1.0f);
            break;
        case DISPLAY_NORMAL:
            out_f4Colour = vec4(abs(f3Normal), 1.0f);
            break;
        case DISPLAY_POSITION:
            out_f4Colour = vec4(abs(f3Position) / ufFar, 1.0f);
            break;
        case DISPLAY_COLOR:
            out_f4Colour = vec4(f3Colour, 1.0);
            break;
        case DISPLAY_LIGHTS:
        case DISPLAY_TOTAL:
            break;
    }	
}

