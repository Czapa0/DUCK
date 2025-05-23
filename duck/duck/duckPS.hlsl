sampler samp;
Texture2D duckTex;

float4 lightPos;
float3 lightColor;
float ks, kd, ka, m;

float4 phong(float3 surfaceColor, float3 worldPos, float3 norm, float3 view, float3 specVec)
{
    view = normalize(view);
    norm = normalize(norm);
    float3 color = surfaceColor * ka; //ambient
    float3 lightVec = normalize(lightPos.xyz - worldPos);
    color += lightColor * kd * surfaceColor * saturate(dot(norm, lightVec)); //diffuse
    color += lightColor * ks * pow(saturate(dot(norm, specVec)), m); //specular
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
    //Tangent
    float3 dPdx = ddx(i.worldPos);
    float3 dPdy = ddy(i.worldPos);
    float2 dtdx = ddx(i.tex);
    float2 dtdy = ddy(i.tex);
    float3 T = normalize(-dPdx * dtdy.y + dPdy * dtdx.y);
    //Bitangent
    float3 B = normalize(cross(i.norm, T));
    //View
    float view = i.view;
    view = normalize(i.view - dot(i.view, B) * B);
    //Light
    float3 lightVec = normalize(lightPos.xyz - i.worldPos);
    lightVec = normalize(lightVec - dot(lightVec, B) * B);
    float3 halfVec = normalize(view + lightVec);
    //Sample color
    float3 color = duckTex.Sample(samp, i.tex).rgb;
    //Final color
    return phong(color, i.worldPos, i.norm, i.view, halfVec);
}