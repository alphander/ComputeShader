#include "Resources.hlsli"
#define lerps(x, y, z, a, b, c, d, e, f, g, h) lerp(lerp(lerp(a,b,x),lerp(c,d,x),y),lerp(lerp(e,f,x),lerp(g,h,x),y),z);

[numthreads(8, 8, 8)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    int index = get(id);
    float3 pos = id - Input[index].velocity * dt;
    float3 fr = frac(pos);
    float3 fl = floor(pos);
    
    uint2 w = uint2(1, 0);
    uint a = get(fl + w.yyy);
    uint b = get(fl + w.xyy);
    uint c = get(fl + w.yxy);
    uint d = get(fl + w.xxy);
    uint e = get(fl + w.yyx);
    uint f = get(fl + w.xyx);
    uint g = get(fl + w.yxx);
    uint h = get(fl + w.xxx);
    {
        float i = Input[a].density;
        float j = Input[b].density;
        float k = Input[c].density;
        float l = Input[d].density;
        float m = Input[e].density;
        float n = Input[f].density;
        float o = Input[g].density;
        float p = Input[h].density;
        Output[index].density = lerps(fr.x, fr.y, fr.z, i, j, k, l, m, n, o, p);
    }
    {
        float3 i = Input[a].velocity;
        float3 j = Input[b].velocity;
        float3 k = Input[c].velocity;
        float3 l = Input[d].velocity;
        float3 m = Input[e].velocity;
        float3 n = Input[f].velocity;
        float3 o = Input[g].velocity;
        float3 p = Input[h].velocity;
        Output[index].velocity = lerps(fr.x, fr.y, fr.z, i, j, k, l, m, n, o, p);
    }
}