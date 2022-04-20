#include "Resources.hlsli"
#define diffuse(a, b, c, d, e, f, g, k) (a + k * (b + c + d + e + f + g)) / (1.0 + 6.0 * k);

[numthreads(8, 8, 8)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    uint current = get(id);
    if ((id.x + id.z + id.y) % step == 0)
    {
        Output[current] = Input[current];
        return;
    }
    
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
    {
        float a = Input[current].concentration;
        float b = Input[xp1].concentration;
        float c = Input[xm1].concentration;
        float d = Input[yp1].concentration;
        float e = Input[ym1].concentration;
        float f = Input[zp1].concentration;
        float g = Input[zm1].concentration;
        Output[current].concentration = diffuse(a, b, c, d, e, f, g, kappa_dt);
    }
    {
        float3 a = Input[current].velocity;
        float3 b = Input[xp1].velocity;
        float3 c = Input[xm1].velocity;
        float3 d = Input[yp1].velocity;
        float3 e = Input[ym1].velocity;
        float3 f = Input[zp1].velocity;
        float3 g = Input[zm1].velocity;
        Output[current].velocity = diffuse(a, b, c, d, e, f, g, nu_dt);
    }
}