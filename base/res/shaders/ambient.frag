#include "ShaderCommon.glsl"
#include "LightingCommon.glsl"

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
	vec4 f4FinalColour = vec4(0.0, 0.0, 0.0, 1.0);

    if (fLinearDepth < 0.99f) 
    {
        float fDiffuse = max(0.0, dot(uf4Light.xyz, f3Normal));
		if (ubToonOn)
		{
			if (fDiffuse >= 1.0)
				fDiffuse = 1.0;
			else if (fDiffuse >= 0.8)
				fDiffuse = 0.8;
			else if (fDiffuse >= 0.6)
				fDiffuse = 0.6;
			else if (fDiffuse >= 0.4)
				fDiffuse = 0.4;
			else if (fDiffuse >= 0.2)
				fDiffuse = 0.2;
			else
				fDiffuse = 0.0;

			float dp = dot (normalize(f3Normal), normalize(-f3Position));
			if (dp < 0.1)
				f4FinalColour = vec4 (0.0, 0.0, 0.0, 1.0);
			else
				f4FinalColour = vec4(f3Colour * (uf4Light.w * fDiffuse + ufLightIl), 1.0f);
		}
		else
			f4FinalColour = vec4(f3Colour * clamp((uf4Light.w * fDiffuse), ufLightIl, 1.0f), 1.0f);
		
    }	

	out_f4Colour = f4FinalColour;
}

