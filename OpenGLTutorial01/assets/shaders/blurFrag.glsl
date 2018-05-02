#version 430 core

layout (location = 0) out vec4 FragColor;

uniform sampler2D blurTexture;

uniform bool horizontal;
uniform float scrWidth;
uniform float scrHeight;

float weights[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{
    vec2 scrCoords = vec2(gl_FragCoord.x / scrWidth, gl_FragCoord.y / scrHeight);
    vec2 texOffset = 1.0 / textureSize(blurTexture, 0);
    vec3 result = texture(blurTexture, scrCoords).rgb * weights[0];
    if(horizontal)
    {
        for(int i = 1; i != 5; ++i)
        {
            result += texture(blurTexture, scrCoords + vec2(texOffset.x * i, 0.0)).rgb * weights[i];
            result += texture(blurTexture, scrCoords - vec2(texOffset.x * i, 0.0)).rgb * weights[i];
        }
    }
    else
    {
        for(int i = 1; i != 5; ++i)
        {
            result += texture(blurTexture, scrCoords + vec2(texOffset.y * i, 0.0)).rgb * weights[i];
            result += texture(blurTexture, scrCoords - vec2(texOffset.y * i, 0.0)).rgb * weights[i];
        }
    }
    FragColor = vec4(result.rgb, 1.0);
}