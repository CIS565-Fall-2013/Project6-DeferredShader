#include "ShaderCommon.glsl"

// Textures
uniform sampler2D u_Posttex;
uniform sampler2D u_GlowMask;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;
uniform sampler2D u_normalTex;
uniform sampler2D u_positionTex;
uniform sampler2D u_depthTex;

in vec2 vo_f2TexCoord;
out vec4 out_f4Colour;

const float occlusion_strength = 1.5f;
const mat3 GaussianMat3 = mat3(vec3 (1,2,1), 
							   vec3 (2,4,2),
							   vec3 (1,2,1)) / 16.0;
const float GaussianMat5 [] = {	1/273.0, 4/273.0, 7/273.0, 4/273.0, 1/273.0,
								4/273.0, 16/273.0, 26/273.0, 16/273.0, 4/273.0,
							    7/273.0, 26/273.0, 41/273.0, 26/273.0, 7/273.0,
							    4/273.0, 16/273.0, 26/273.0, 16/273.0, 4/273.0,
							    1/273.0, 4/273.0, 7/273.0, 4/273.0, 1/273.0	};
								
void main() 
{
    vec3 color = SampleTexture(u_Posttex, vo_f2TexCoord);
	if (ubBloomOn)
	{
//		if (SampleTexture(u_GlowMask, vo_f2TexCoord).r)
//		{
			vec3 bloomColour = vec3(0);
			for (int i = -1; i < 2; ++i)
			{
				int j = -1;
				bloomColour += (texture(u_GlowMask, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight), 2).xyz * GaussianMat3 [i+1].x);
				++ j;
				bloomColour += (texture(u_GlowMask, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight), 2).xyz * GaussianMat3 [i+1].y);
				++ j;
				bloomColour += (texture(u_GlowMask, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight), 2).xyz * GaussianMat3 [i+1].z);
				++ j;
			}  
			color += color*bloomColour;  
//		}
	}

	if (ubToonOn)
	{
		float dotPdt = dot (texture(u_normalTex, vo_f2TexCoord).xyz, -(texture(u_positionTex, vo_f2TexCoord).xyz));
		if (dotPdt < 0.1)
			color = vec3(0.0, 0.0, 0.0);
	}

	if (ubDOFOn)
	{
		float depth = texture(u_depthTex, vo_f2TexCoord).x;
		depth = linearizeDepth(depth);

		float focalLen = texture(u_depthTex, vec2 (ufMouseTexX, ufMouseTexY)).x;
		focalLen = linearizeDepth(focalLen);

		float lenQuant = 0.01;
		depth = abs (focalLen - depth); 
		depth /= lenQuant;

		vec3 bloomColour = vec3(0);
		if (depth >= 3.0)
		{
			for (int i = -2; i < 3; ++i)
			{
				int j = -2;
				bloomColour += (texture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * GaussianMat5 [(j+2)*5+ (i+2)]);
				++ j;
				bloomColour += (texture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * GaussianMat5 [(j+2)*5+ (i+2)]);
				++ j;
				bloomColour += (texture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * GaussianMat5 [(j+2)*5+ (i+2)]);
				++ j;
				bloomColour += (texture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * GaussianMat5 [(j+2)*5+ (i+2)]);
				++ j;
				bloomColour += (texture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * GaussianMat5 [(j+2)*5+ (i+2)]);
				++ j;
			}

			if (!ubDOFDebug)
				color = bloomColour;
			else
				color = vec3 (1.0, 0.0, 0.0);
		}

		else if (depth >= 2.0)
		{
			for (int i = -1; i < 2; ++i)
			{
				int j = -1;
				bloomColour += (texture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * GaussianMat3 [i+1].x);
				++ j;
				bloomColour += (texture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * GaussianMat3 [i+1].y);
				++ j;
				bloomColour += (texture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz * GaussianMat3 [i+1].z);
				++ j;
			}
			if (!ubDOFDebug)
				color = bloomColour;
			else
				color = vec3(0.0, 1.0, 0.0);
		}
		else if (depth >= 1.0)
		{
			for (int i = 0; i < 2; ++ i)
			{			
				for (int j = 0; j < 2; ++ j)
				{
					bloomColour += texture(u_Posttex, vec2(vo_f2TexCoord.x + i*ufInvScrWidth, vo_f2TexCoord.y + j*ufInvScrHeight)).xyz;
				}
			}
			bloomColour /= 4.0;
			
			if (!ubDOFDebug)
				color = bloomColour;
			else
				color = vec3(0.0, 0.0, 1.0);
		}
	}

//    float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
//    float vin = min(2*distance(vec2(0.5), vo_f2TexCoord), 1.0);
    out_f4Colour = vec4(color,1.0);//vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
}

