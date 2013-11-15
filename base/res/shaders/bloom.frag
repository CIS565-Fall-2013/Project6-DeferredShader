#version 330

uniform sampler2D u_Colortex;
uniform sampler2D u_Bloomtex;
in vec2 fs_Texcoord;
out vec4 out_Color;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

/////////////////////////////////////
//				UTILITY FUNCTIONS
/////////////////////////////////////

//Helper function to automicatlly sample and unpack colors
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Colortex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack bloom
float sampleBloom(vec2 texcoords) {
    return texture(u_Bloomtex,texcoords).x;
}

///////////////////////////////////
// MAIN
//////////////////////////////////

void main() {

	vec3 totcolor = vec3(0.0,0.0,0.0);
	vec2 texCord = fs_Texcoord;
	const float guassian[25] = {1,4,7,4,1,4,16,26,16,4,7,26,41,26,7,4,16,26,15,4,1,4,7,4,1};
	int k = 0;
	
	/*for (float i = texCord.x- 2.0/u_ScreenWidth ; i <= texCord.x + 2.0/u_ScreenWidth ; i= i + 1/u_ScreenWidth)
	{
		for(float j = texCord.y - 2.0/u_ScreenHeight ; j <= texCord.y + 2.0/u_ScreenHeight ; j= j + 1/u_ScreenHeight) 
		{
	    totcolor += sampleCol(vec2(i,j)) * guassian[k]/273.0 * sampleBloom(vec2(i,j));
		k++;
		}
    }*/

	float x = texCord.x - 2.0/u_ScreenWidth; 
	float y = texCord.y - 2.0/u_ScreenHeight ;

	for(int i = 0 ; i < 5 ; i++)
	{
		for(int j = 0 ; j < 5 ; j++)
		{
			totcolor += (sampleCol(vec2(x,y))  * sampleBloom(vec2(x,y)) * guassian[k]/273.0); //
			k++;
			y= y + 1.0/u_ScreenHeight;
		}
		y = texCord.y - 2.0/u_ScreenHeight;
		x = x + 1.0/u_ScreenWidth;
	}

	//out_Color = vec4(sampleCol(texCord)* sampleBloom(texCord),1.0) ;
	out_Color = vec4(totcolor,1.0) ;
	
	
}
