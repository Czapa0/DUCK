matrix modelMtx, viewProjMtx;
float waterLevel;

struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 localPos : POSITION0;
    float3 worldPos : POSITION1;
};

VSOutput main(float3 pos : POSITION0)
{
    VSOutput o;
    o.localPos = pos;
    o.localPos.y = waterLevel;
    
    float4 p = float4(o.localPos, 1);
    p = mul(modelMtx, p);
    o.worldPos = p;
    
    p = mul(viewProjMtx, p);
    o.pos = p;
    
    return o;
}
