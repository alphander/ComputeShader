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
    
    float a = Input[xp1].divergence;
    float b = Input[xm1].divergence;
    float c = Input[yp1].divergence;
    float d = Input[ym1].divergence;
    float e = Input[zp1].divergence;
    float f = Input[zm1].divergence;
    
    Output[current].divergence = ((a - b) + (c - d) + (e - f)) / 3.0;
}