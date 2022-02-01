#include "Resources.hlsli"

RWStructuredBuffer<Data> Input : register(u0);
RWStructuredBuffer<Data> Output : register(u1);

uint get(uint3 id)
{
    return id.x + id.y * length + id.z * area;
}

[numthreads(8, 8, 8)]
void CSMain(uint3 id : SV_DispatchThreadID)
{   
    if ((id.x + id.z + id.y) % step == 0) return;
    
    float k = viscosity;
    
    uint2 o = uint2(1, 0);
    uint current = get(id);
    uint xp1 = get(id + o.xyy);
    uint xm1 = get(id - o.xyy);
    uint yp1 = get(id + o.yxy);
    uint ym1 = get(id - o.yxy);
    uint zp1 = get(id + o.yyx);
    uint zm1 = get(id - o.yyx);
    
    float a = Input[current].pressure;
    float b = Input[xp1].pressure;
    float c = Input[xm1].pressure;
    float d = Input[yp1].pressure;
    float e = Input[ym1].pressure;
    float f = Input[zp1].pressure;
    float g = Input[zm1].pressure;
    
    float s = (b + c + d + e + f + g) / 6;
    
    float value = (a + k * s) / (1.0 + k);
    
    Output[current].pressure = value;
}