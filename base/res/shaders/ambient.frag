#include "ShaderCommon.glsl"
#include "LightingCommon.glsl"

// Textures
uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

in vec2 fs_Texcoord;
out vec4 out_Color;

const float occlusion_strength = 1.5f;
void main() 
{
    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth, u_Near, u_Far);

    vec3 normal = sampleNrm(fs_Texcoord);
    vec3 position = samplePos(fs_Texcoord);
    vec3 color = sampleCol(fs_Texcoord);
    vec3 light = u_Light.xyz;
    float strength = u_Light.w;
	vec4 finalcolour = vec4(0.0, 0.0, 0.0, 1.0);
    if (lin_depth > 0.99f) 
    {
        out_Color = vec4(vec3(0.0), 1.0);
    } 
    else 
    {
        float ambient = u_LightIl;
        float diffuse = max(0.0, dot(normalize(light),normal));
		if (u_toonOn)
		{
			if (diffuse >= 1.0)
				diffuse = 1.0;
			else if (diffuse >= 0.8)
				diffuse = 0.8;
			else if (diffuse >= 0.6)
				diffuse = 0.6;
			else if (diffuse >= 0.4)
				diffuse = 0.4;
			else if (diffuse >= 0.2)
				diffuse = 0.2;
			else
				diffuse = 0.0;

			float dp = dot (normalize(normal), normalize(-position));
			if (dp < 0.1)
				finalcolour = vec4 (0.0, 0.0, 0.0, 1.0);
			else
				finalcolour = vec4(color*(strength*diffuse + ambient), 1.0f);
		}
		else
			finalcolour = vec4(color*(strength*diffuse + ambient), 1.0f);
		
		out_Color = finalcolour;
    }	
}

