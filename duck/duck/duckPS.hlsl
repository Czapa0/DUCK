sampler samp;
Texture2D duckTex;

float4 lightPos;
float3 lightColor;
float ks, kd, ka, m;

float4 phong(float3 surfaceColor, float3 worldPos, float3 norm, float3 view)
{
    view = normalize(view);
    norm = normalize(norm);
    float3 color = surfaceColor * ka; //ambient
    float3 lightVec = normalize(lightPos.xyz - worldPos);
    float3 halfVec = normalize(view + lightVec);
    color += lightColor * kd * surfaceColor * saturate(dot(norm, lightVec)); //diffuse
    color += lightColor * ks * pow(saturate(dot(norm, halfVec)), m); //specular
    return saturate(float4(color, 1.0f));
}

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float3 norm : NORMAL0;
    float3 view : VIEWVEC0;
    float2 tex : TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
    float3 color = duckTex.Sample(samp, i.tex).rgb;
    //return float4(color, 1.0f);
    return phong(color, i.worldPos, i.norm, i.view);
}