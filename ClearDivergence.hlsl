#include "Resources.hlsli"

[numthreads(8, 8, 8)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    uint current = get(id);
    
    if (Input[current].type)
    {
        Output[current] = Input[current];
        return;
    }
    
    uint2 w = uint2(1, 0);
    uint xp1 = get(id + w.xyy);
    uint xm1 = get(id - w.xyy);
    uint yp1 = get(id + w.yxy);
    uint ym1 = get(id - w.yxy);
    uint zp1 = get(id + w.yyx);
    uint zm1 = get(id - w.yyx);
    
    float a = Input[xp1].pressure;
    float b = Input[xm1].pressure;
    float c = Input[yp1].pressure;
    float d = Input[ym1].pressure;
    float e = Input[zp1].pressure;
    float f = Input[zm1].pressure;
    
    Output[current].velocity = Input[current].velocity - float3((a - b) / 2.0, (c - d) / 2.0, (e - f) / 2.0) * dt_rho;
}