#version 430 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;

uniform float screenWidth;
uniform float screenHeight;

uniform bool checkGamma;
uniform bool checkHDR;
uniform int drawType;
uniform float exposure;

const float offset = 1.0 / 300.0; 

void main()
{ 
	vec2 scrCoords = vec2(gl_FragCoord.x / screenWidth, gl_FragCoord.y / screenHeight);
	vec4 texColor = texture(screenTexture, scrCoords);
	vec4 bloomColor = texture(bloomTexture, scrCoords);
	vec3 result = vec3(0.0);

	if(checkHDR)
	{
		if(checkGamma)
		{
			result = texColor.rgb;// + bloomColor.rgb;
			//result = vec3(1.0) - exp(-result.rgb * exposure);
			result = result / (result + vec3(1.0));
			//result = pow(result, vec3(1.0 / 2.8));
			FragColor = vec4(result, 1.0);
		}
		else
		{
			result = texColor.rgb, bloomColor.rgb;
			result = vec3(1.0) - exp(-result.rgb * exposure);
			FragColor = vec4(result, 1.0);
		}
	}
	else
	{
		if(checkGamma)
		{
			FragColor = vec4(texColor.rgb, 1.0);
			FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / 2.2));
		}
		else
		{
			FragColor = vec4(texColor.rgb, 1.0);
		}
	}


}