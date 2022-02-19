#include "Resources.hlsli"
#define diffuse(a, b, c, d, e, f, g) (a + k * ((b + c + d + e + f + g) / 6)) / (1.0 + k);

[numthreads(8, 8, 8)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    uint current = get(id);
    if ((id.x + id.z + id.y) % step == 0)
    {
        Output[current] = Input[current];
        return;
    }
    
    float k = viscosity;
    
    uint2 w = uint2(1, 0);
    uint xp1 = get(id + w.xyy);
    uint xm1 = get(id - w.xyy);
    uint yp1 = get(id + w.yxy);
    uint ym1 = get(id - w.yxy);
    uint zp1 = get(id + w.yyx);
    uint zm1 = get(id - w.yyx);
    {
        float a = Input[current].density;
        float b = Input[xp1].density;
        float c = Input[xm1].density;
        float d = Input[yp1].density;
        float e = Input[ym1].density;
        float f = Input[zp1].density;
        float g = Input[zm1].density;
        Output[current].density = diffuse(a, b, c, d, e, f, g);
    }
    {
        float3 a = Input[current].velocity;
        float3 b = Input[xp1].velocity;
        float3 c = Input[xm1].velocity;
        float3 d = Input[yp1].velocity;
        float3 e = Input[ym1].velocity;
        float3 f = Input[zp1].velocity;
        float3 g = Input[zm1].velocity;
        Output[current].velocity = diffuse(a, b, c, d, e, f, g);
    }
}