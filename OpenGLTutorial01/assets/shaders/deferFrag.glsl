#version 430 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragBrightness;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D normalTexture;
uniform sampler2D vertexTexture;
uniform sampler2D motionTexture;
uniform sampler2D shadowTexture;
uniform sampler2D depthTexture;
uniform samplerCube omniShadowTexture;
uniform mat4 lightSpaceMatrix;
uniform float screenWidth;
uniform float screenHeight;
uniform float nearPlane;
uniform float farPlane;

uniform mat4 viewProjectionMatrix;
uniform mat4 prevViewProjectionMatrix;

uniform int drawType;

const float offset = 1.0 / 300.0; 


struct DirLight
{
	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct PointLight
{
	vec3 position;
	
	float constant;
	float linear;
	float quadratic;
	float radius;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct SpotLight
{
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;
	
	float constant;
	float linear;
	float quadratic;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform int numPointLights;
uniform DirLight dirLight;
uniform PointLight pointLights[100];
uniform SpotLight spotLight;
uniform bool checkDistance;
uniform bool isOmniDirectionalShadow;
uniform float omniFarPlane;

uniform bool isNormal;
uniform vec3 cameraPos;

vec4 FragPosLightSpace;

const int levels = 5;
const float scaleFactor = 1.0 / levels;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color, bool first);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color);
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowTexture, projCoords.xy).r;
    float currentDepth = projCoords.z;

    //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float bias = 0.05;

    // float shadow = 0.0;
    // vec2 texelSize = 1.0 / textureSize(shadowTexture, 0);
    // for(int x = -1; x <= 1; ++x)
    // {
    //     for(int y = -1; y <= 1; ++y)
    //     {
    //         float pcfDepth = texture(shadowTexture, projCoords.xy + vec2(x, y) * texelSize).r;
    //         shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    //     }
    // }
    // shadow /= 9.0;

    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

float OmniShadowCalculation(vec3 FragPos)
{
	vec3 fragToLight = FragPos - pointLights[0].position;
	float closestDepth = texture(omniShadowTexture, fragToLight).r;
	closestDepth *= omniFarPlane;

	float currentDepth = length(fragToLight);
	float bias = 0.05;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	return shadow;
}

float SobelNormalHorizontal(sampler2D texMap, vec2 texCoord, vec2 pixelSize)
{
	vec3 sum = vec3(0.0);

	sum -= texture(texMap, vec2(texCoord.x - pixelSize.x, texCoord.y + pixelSize.y)).rgb * 3.0;
	sum -= texture(texMap, vec2(texCoord.x - pixelSize.x, texCoord.y)).rgb * 10.0;
	sum -= texture(texMap, vec2(texCoord.x - pixelSize.x, texCoord.y - pixelSize.y)).rgb * 3.0;

	sum += texture(texMap, vec2(texCoord.x + pixelSize.x, texCoord.y + pixelSize.y)).rgb * 3.0;
	sum += texture(texMap, vec2(texCoord.x + pixelSize.x, texCoord.y)).rgb * 10.0;
	sum += texture(texMap, vec2(texCoord.x + pixelSize.x, texCoord.y - pixelSize.y)).rgb * 3.0;

	float edgeFactor = dot(sum, sum);

	if(edgeFactor < 0.2)
		return 1.0;
	else
		return 0.0;
}
float SobelNormalVertical(sampler2D texMap, vec2 texCoord, vec2 pixelSize)
{
	vec3 sum = vec3(0.0);

	sum += texture(texMap, vec2(texCoord.x - pixelSize.x, texCoord.y + pixelSize.y)).rgb * 3.0;
	sum += texture(texMap, vec2(texCoord.x, texCoord.y + pixelSize.y)).rgb * 10.0;
	sum += texture(texMap, vec2(texCoord.x + pixelSize.x, texCoord.y + pixelSize.y)).rgb * 3.0;

	sum -= texture(texMap, vec2(texCoord.x - pixelSize.x, texCoord.y - pixelSize.y)).rgb * 3.0;
	sum -= texture(texMap, vec2(texCoord.x, texCoord.y - pixelSize.y)).rgb * 10.0;
	sum -= texture(texMap, vec2(texCoord.x + pixelSize.x, texCoord.y - pixelSize.y)).rgb * 3.0;

	float edgeFactor = dot(sum, sum);

	if(edgeFactor < 0.5)
		return 1.0;
	else
		return 0.0;
}
float SobelDepthHorizontal(sampler2D texMap, vec2 texCoord, vec2 pixelSize)
{
	float sum = 0.0;

	sum -= texture(texMap, vec2(texCoord.x - pixelSize.x, texCoord.y + pixelSize.y)).r * 3.0;
	sum -= texture(texMap, vec2(texCoord.x - pixelSize.x, texCoord.y)).r * 10.0;
	sum -= texture(texMap, vec2(texCoord.x - pixelSize.x, texCoord.y - pixelSize.y)).r * 3.0;

	sum += texture(texMap, vec2(texCoord.x + pixelSize.x, texCoord.y + pixelSize.y)).r * 3.0;
	sum += texture(texMap, vec2(texCoord.x + pixelSize.x, texCoord.y)).r * 10.0;
	sum += texture(texMap, vec2(texCoord.x + pixelSize.x, texCoord.y - pixelSize.y)).r * 3.0;

	if(sum < 0.05)
		return 1.0;
	else
		return 0.0;
}
float SobelDepthVertical(sampler2D texMap, vec2 texCoord, vec2 pixelSize)
{
	float sum = 0.0;

	sum += texture(texMap, vec2(texCoord.x - pixelSize.x, texCoord.y + pixelSize.y)).r * 3.0;
	sum += texture(texMap, vec2(texCoord.x, texCoord.y + pixelSize.y)).r * 10.0;
	sum += texture(texMap, vec2(texCoord.x + pixelSize.x, texCoord.y + pixelSize.y)).r * 3.0;

	sum -= texture(texMap, vec2(texCoord.x - pixelSize.x, texCoord.y - pixelSize.y)).r * 3.0;
	sum -= texture(texMap, vec2(texCoord.x, texCoord.y - pixelSize.y)).r * 10.0;
	sum -= texture(texMap, vec2(texCoord.x + pixelSize.x, texCoord.y - pixelSize.y)).r * 3.0;

	if(sum < 0.05)
		return 1.0;
	else
		return 0.0;
}




float linearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main()
{ 
	vec2 scrCoords = vec2(gl_FragCoord.x / screenWidth, gl_FragCoord.y / screenHeight);

    vec4 colorPixel = vec4(texture(screenTexture, scrCoords).rgb, 1.0);
	vec4 normalPixel = vec4(0.0);

	normalPixel= vec4(texture(normalTexture, scrCoords).rgb, 1.0);

    vec4 vertexPixel = vec4(texture(vertexTexture, scrCoords).rgb, 1.0);
	// vec2 motionVector = texture(motionTexture, scrCoords).xy / 10.0;

	// float depthValue = texture(depthTexture, scrCoords).r;
	// vec3 depthColor = vec3(linearizeDepth(depthValue) / farPlane);

	// //vec2 velocity = perPixelVelocity(scrCoords);

    FragPosLightSpace = lightSpaceMatrix * vertexPixel;

    vec3 norm = normalize(normalPixel.rgb);
    vec3 viewDir = normalize(cameraPos - vertexPixel.rgb);

  
    
   
    


    // float brightness = dot(result.rgb, vec3(0.2126, 0.7152, 0.0722));
    // if(brightness > 1.0)
    //     FragBrightness = vec4(result.rgb, 1.0);
    // else
    //     FragBrightness = vec4(0.0, 0.0, 0.0, 1.0);



	if (drawType == 1) // Draw without any effects
	{
		  vec3 result = vec3(0.0f);

		if(isNormal)
			result = CalcDirLight(dirLight, norm, viewDir, colorPixel.rgb);
		
		bool first = false;
		for (int i = 0; i < numPointLights; i++)
		{
			float distance = length(pointLights[i].position - vertexPixel.rgb);

			if(i == 0)
				first = true;
			else
				first = false;

			if(checkDistance)
			{
				if(distance < pointLights[i].radius)
				{
					
					result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, colorPixel.rgb, first);
				}
			}
			else
			{
				result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, colorPixel.rgb, first);
			}
		}
		FragColor = vec4(result, 1.0);
	}
	else if (drawType == 2) // Gray-scale
	{
		//   vec3 result = vec3(0.0f);
		//  result = CalcDirLight(dirLight, norm, viewDir, colorPixel.rgb);
		// 		bool first = false;
		// for (int i = 0; i < numPointLights; i++)
		// {
		// 	float distance = length(pointLights[i].position - vertexPixel.rgb);

		// 	if(i == 0)
		// 		first = true;
		// 	else
		// 		first = false;

		// 	if(checkDistance)
		// 	{
		// 		if(distance < pointLights[i].radius)
		// 		{
					
		// 			result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, colorPixel.rgb, first);
		// 		}
		// 	}
		// 	else
		// 	{
		// 		result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, colorPixel.rgb, first);
		// 	}
		// }
		// float average = 0.2126 * result.r + 0.7152 * result.g + 0.0722 * result.b;
		// FragColor = vec4(average, average, average, 1.0);
		float depthValue = texture(shadowTexture, scrCoords).r;
		FragColor = vec4(vec3(depthValue), 1.0);
	}
	else if (drawType == 3) // Inverse
	{
		  vec3 result = vec3(0.0f);
		 result = CalcDirLight(dirLight, norm, viewDir, colorPixel.rgb);
				bool first = false;
		for (int i = 0; i < numPointLights; i++)
		{
			float distance = length(pointLights[i].position - vertexPixel.rgb);

			if(i == 0)
				first = true;
			else
				first = false;

			if(checkDistance)
			{
				if(distance < pointLights[i].radius)
				{
					
					result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, colorPixel.rgb, first);
				}
			}
			else
			{
				result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, colorPixel.rgb, first);
			}
		}
		FragColor = vec4(1.0) - vec4(result, 0.0);
	}	
	else if (drawType == 4)
	{
		const float offset = 1.0f / 300.0f;
		vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    	);
		float kernel[9] = float[](
    	1.0, 1.0, 1.0,
    	1.0, -8.0, 1.0,
		1.0, 1.0, 1.0
		);

		vec3 sampleTex[9];
		for(int i = 0; i != 9; i++)
			sampleTex[i] = vec3(texture(screenTexture, scrCoords.xy + offsets[i]));

		vec4 color = vec4(0.0f);
		for(int i = 0; i != 9; i++)
			color.rgb += sampleTex[i] * kernel[i];
		color.a = 1.0f;
		vec3 result = vec3(0.0, 0.0, 0.0);
		result = CalcDirLight(dirLight, norm, viewDir, color.rgb);
		bool first = false;
		for (int i = 0; i < numPointLights; i++)
		{
			float distance = length(pointLights[i].position - vertexPixel.rgb);

			if(i == 0)
				first = true;
			else
				first = false;

			if(checkDistance)
			{
				if(distance < pointLights[i].radius)
				{
					
					result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, color.rgb, first);
				}
			}
			else
			{
				result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, color.rgb, first);
			}
		}
		FragColor = vec4(result, 1.0);
	}
    else if (drawType == 5)
	{
		vec4 top = texture(screenTexture, vec2(scrCoords.x, scrCoords.y + 1.0 / screenHeight)).rgba;
		vec4 bottom = texture(screenTexture, vec2(scrCoords.x, scrCoords.y - 1.0 / screenHeight)).rgba;
		vec4 left = texture(screenTexture, vec2(scrCoords.x - 1.0 / screenWidth, scrCoords.y)).rgba;
		vec4 right = texture(screenTexture, vec2(scrCoords.x + 1.0 / screenWidth, scrCoords.y)).rgba;
		vec4 topLeft = texture(screenTexture, vec2(scrCoords.x - 1.0 / screenWidth, scrCoords.y + 1.0 / screenHeight)).rgba;
		vec4 topRight = texture(screenTexture, vec2(scrCoords.x + 1.0 / screenWidth, scrCoords.y + 1.0 / screenHeight)).rgba;
		vec4 bottomLeft = texture(screenTexture, vec2(scrCoords.x - 1.0 / screenWidth, scrCoords.y - 1.0 / screenHeight)).rgba;
		vec4 bottomRight = texture(screenTexture, vec2(scrCoords.x + 1.0 / screenWidth, scrCoords.y - 1.0 / screenHeight)).rgba;
		vec4 sx = -topLeft - 2 * left - bottomLeft + topRight + 2 * right + bottomRight;
		vec4 sy = -topLeft - 2 * top - topRight + bottomLeft + 2 * bottom + bottomRight;
		vec4 color = sqrt(sx * sx + sy * sy);


		vec3 result = vec3(0.0, 0.0, 0.0);
		result = CalcDirLight(dirLight, norm, viewDir, color.rgb);
		bool first = false;
		for (int i = 0; i < numPointLights; i++)
		{
			float distance = length(pointLights[i].position - vertexPixel.rgb);

			if(i == 0)
				first = true;
			else
				first = false;

			if(checkDistance)
			{
				if(distance < pointLights[i].radius)
				{
					
					result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, color.rgb, first);
				}
			}
			else
			{
				result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, color.rgb, first);
			}
		}
		FragColor = vec4(result, 1.0);
	}
	else if (drawType == 6)
	{
		  vec3 result = vec3(0.0f);
		 result = CalcDirLight(dirLight, norm, viewDir, colorPixel.rgb);
				bool first = false;
		for (int i = 0; i < numPointLights; i++)
		{
			float distance = length(pointLights[i].position - vertexPixel.rgb);

			if(i == 0)
				first = true;
			else
				first = false;

			if(checkDistance)
			{
				if(distance < pointLights[i].radius)
				{
					
					result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, colorPixel.rgb, first);
				}
			}
			else
			{
				result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, colorPixel.rgb, first);
			}
		}

		vec2 pixelSize = vec2(1.0 / screenWidth, 1.0 / screenHeight);
		float SobelDepth = SobelDepthHorizontal(depthTexture, scrCoords, pixelSize) * SobelDepthVertical(depthTexture, scrCoords, pixelSize);
		float SobelNormal = SobelNormalHorizontal(normalTexture, scrCoords, pixelSize) * SobelNormalVertical(normalTexture, scrCoords, pixelSize);

		FragColor = vec4(result.rgb * SobelDepth * SobelNormal, 1.0);
	}
	else if(drawType == 7)
	{
		int samples = 16;
		float intensity = 0.1;
		float decay = 0.95875;
		vec2 texCoords = scrCoords;
		vec2 direction = vec2(0.5) - texCoords;
		direction /= samples;
		vec3 color = texture(screenTexture, texCoords).rgb;


		for(int i = 0; i != samples; i++)
		{
			color += texture(screenTexture, texCoords).rgb * intensity;
			intensity *= decay;
			texCoords += direction;
		}


		vec3 result = vec3(0.0, 0.0, 0.0);
		result = CalcDirLight(dirLight, norm, viewDir, color.rgb);
		bool first = false;
		for (int i = 0; i < numPointLights; i++)
		{
			float distance = length(pointLights[i].position - vertexPixel.rgb);

			if(i == 0)
				first = true;
			else
				first = false;

			if(checkDistance)
			{
				if(distance < pointLights[i].radius)
				{
					
					result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, color.rgb, first);
				}
			}
			else
			{
				result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, color.rgb, first);
			}
		}
		FragColor = vec4(color, 1.0);
	}
	else if(drawType == 8)
	{
		vec4 top = texture(screenTexture, vec2(scrCoords.x, scrCoords.y + 1.0 / screenHeight)).rgba;
		vec4 bottom = texture(screenTexture, vec2(scrCoords.x, scrCoords.y - 1.0 / screenHeight)).rgba;
		vec4 left = texture(screenTexture, vec2(scrCoords.x - 1.0 / screenWidth, scrCoords.y)).rgba;
		vec4 right = texture(screenTexture, vec2(scrCoords.x + 1.0 / screenWidth, scrCoords.y)).rgba;
		vec4 topLeft = texture(screenTexture, vec2(scrCoords.x - 1.0 / screenWidth, scrCoords.y + 1.0 / screenHeight)).rgba;
		vec4 topRight = texture(screenTexture, vec2(scrCoords.x + 1.0 / screenWidth, scrCoords.y + 1.0 / screenHeight)).rgba;
		vec4 bottomLeft = texture(screenTexture, vec2(scrCoords.x - 1.0 / screenWidth, scrCoords.y - 1.0 / screenHeight)).rgba;
		vec4 bottomRight = texture(screenTexture, vec2(scrCoords.x + 1.0 / screenWidth, scrCoords.y - 1.0 / screenHeight)).rgba;
		vec4 sx = -topLeft - 2 * left - bottomLeft + topRight + 2 * right + bottomRight;
		vec4 sy = -topLeft - 2 * top - topRight + bottomLeft + 2 * bottom + bottomRight;
		vec4 color = sqrt(sx * sx + sy * sy);

		vec3 result = vec3(0.0f);
		result = CalcDirLight(dirLight, norm, viewDir, color.rgb);
		bool first = false;
		for (int i = 0; i < numPointLights; i++)
		{
			float distance = length(pointLights[i].position - vertexPixel.rgb);

			if(i == 0)
				first = true;
			else
				first = false;

			if(checkDistance)
			{
				if(distance < pointLights[i].radius)
				{
					
					result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, color.rgb, first);
				}
			}
			else
			{
				result += CalcPointLight(pointLights[i], norm, vertexPixel.rgb, viewDir, color.rgb, first);
			}
		}
		FragColor = vec4(result, 1.0);		
	}
	else if(drawType == 9)
	{
	}
	else
	{
		//If something gets messed up, just draw like normal
		FragColor = texture(screenTexture, scrCoords);
	}
}


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color)
{
	vec3 lightDir = normalize(-light.direction);
    vec3 halfDir = normalize(lightDir + viewDir);
	//diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	//specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfDir), 0.0), 64);
	//combine results
    float shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);

	if(drawType == 8)
	{
		vec3 ambient = light.ambient * color;
		vec3 diffuse = light.diffuse * floor(diff * levels) * scaleFactor * color;

		vec3 outColor = vec3(0.0f);
		if(!isOmniDirectionalShadow)
		{
			outColor = (ambient + (1.0 - shadow) * (diffuse));
		}
		else
		{
			outColor = ambient + diffuse;
		}
		
		//vec3 justDiff = light.diffuse;
		return outColor;
	}
	else
	{
		vec3 ambient = light.ambient;
		vec3 diffuse = light.diffuse * diff;
		vec3 specular = light.specular * spec;

		vec3 outColor = vec3(0.0f);
		if(!isOmniDirectionalShadow)
		{
			outColor = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
			//outColor = (ambient + diffuse + specular) * color;
		}
		else
		{
			outColor = (ambient + diffuse + specular) * color;
		}
		
		//vec3 justDiff = light.diffuse;
		return outColor;
	}
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color, bool first)
{
	vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfDir = normalize(lightDir - viewDir);
	//diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	//specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfDir), 0.0), 16);
	//attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
						light.quadratic * (distance * distance));
	//combine result

	if(drawType == 8)
	{
		vec3 ambient = vec3(0.0f);
		vec3 diffuse = vec3(0.0f);
		vec3 specular = vec3(0.0f);

		ambient = 0.5 * light.ambient * color;
		diffuse = light.diffuse * floor(diff * levels) * scaleFactor * color;
		specular = light.specular * spec * color;
		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;

		return ambient + diffuse;
	}
	else
	{
		
	vec3 ambient = vec3(0.0f);
	vec3 diffuse = vec3(0.0f);
	vec3 specular = vec3(0.0f);

	ambient = 0.25 * light.ambient * color;
	diffuse = light.diffuse * diff * color;
	specular = light.specular * spec * color;
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return ambient + diffuse + specular;
	}

}

// vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
// {
// 	vec3 lightDir = normalize(light.position - fragPos);
// 	//diffuse shading
// 	float diff = max(dot(normal, lightDir), 0.0);
// 	//specular shading
// 	vec3 reflectDir = reflect(-lightDir, normal);
// 	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
// 	//attenuation
// 	float distance = length(light.position - fragPos);
// 	float attenuation = 1.0 / (light.constant + light.linear * distance + 
// 						light.quadratic * (distance * distance));
// 	//spotlight intensity
// 	float theta = dot(lightDir, normalize(-light.direction));
// 	float epsilon = light.cutOff - light.outerCutOff;
// 	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
// 	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
// 	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
// 	//vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
// 	vec3 specular = vec3(0.0);
// 	ambient *= attenuation * intensity;
// 	diffuse *= attenuation * intensity;
// 	specular *= attenuation * intensity;
	
// 	return (ambient + diffuse + specular);
// }
