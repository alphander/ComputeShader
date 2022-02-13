#include "Resources.hlsli"

[numthreads(8, 8, 8)]
void CSMain(uint3 id : SV_DispatchThreadID)
{   
    uint current = get(id);
    
    uint2 w = uint2(1, 0);
    uint xp1 = get(id + w.xyy);
    uint xm1 = get(id - w.xyy);
    uint yp1 = get(id + w.yxy);
    uint ym1 = get(id - w.yxy);
    uint zp1 = get(id + w.yyx);
    uint zm1 = get(id - w.yyx);
    
    float3 a = Input[xp1].velocity;
    float3 b = Input[xm1].velocity;
    float3 c = Input[yp1].velocity;
    float3 d = Input[ym1].velocity;
    float3 e = Input[zp1].velocity;
    float3 f = Input[zm1].velocity;
    
    Output[current].divergence = ((a.x - b.x) + (c.y - d.y) + (e.z - f.z)) / 3.0;
}