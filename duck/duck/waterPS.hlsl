float4 camPos;
sampler samp;
textureCUBE envMap;
Texture2D bumpMap;

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 localPos : POSITION0;
    float3 worldPos : POSITION1;
};

float intersectRay(float3 p, float3 d)
{
    float3 t1 = (1.0 - p) / d;
    float3 t2 = (-1.0 - p) / d;
    float3 t = max(t1, t2);
    
    return min(min(t.x, t.y), t.z);
}

float fresnel(float3 N, float3 V)
{
    float c = max(dot(N, V), 0.0);
    float F0 = 0.14;
    return F0 + (1.0 - F0) * pow(1.0 - c, 5.0);
}

float4 main(PSInput i) : SV_TARGET
{   
    float3 norm = bumpMap.Sample(samp, i.localPos.xz).rgb;
    norm = normalize(norm * 2.0 - 1.0);
    float3 viewVec = normalize(camPos.xyz - i.worldPos);
    float n = 3.0 / 4.0;
    if (dot(norm, viewVec) < 0)
    {
        norm = -norm;
        n = 4.0 / 3.0;
    }
    float f = fresnel(norm, viewVec);
    float3 color;
    
    float3 reflection = reflect(-viewVec, norm);
    float t_reflection = intersectRay(i.localPos, reflection);
    float3 tex1 = i.localPos + reflection * t_reflection;
    float3 col1 = envMap.Sample(samp, tex1).rgb;
    
    float3 refraction = refract(-viewVec, norm, n);
    if (any(refraction))
    {
        float t_refraction = intersectRay(i.localPos, refraction);
        float3 tex2 = i.localPos + refraction * t_refraction;
        float3 col2 = envMap.Sample(samp, tex2).rgb;
        color = lerp(col2, col1, f);
    }
    else
    {
        color = col1;
    }
    
    color = pow(color, 0.4545f);
    return float4(color, 1);
}
