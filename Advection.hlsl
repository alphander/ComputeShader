#include "Resources.hlsli"
#define lerps(x, y, z, a, b, c, d, e, f, g, h) lerp(lerp(lerp(a,b,x),lerp(c,d,x),y),lerp(lerp(e,f,x),lerp(g,h,x),y),z);

[numthreads(4, 4, 4)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    Data input = getInput(id);
    Data output = getOutput(id);
    float3 pos = id - input.velocity * dt;
    float3 fr = frac(pos);
    float3 fl = floor(pos);
    
    uint2 w = uint2(1, 0);
    Data a = getInput(fl + w.yyy);
    Data b = getInput(fl + w.xyy);
    Data c = getInput(fl + w.yxy);
    Data d = getInput(fl + w.xxy);
    Data e = getInput(fl + w.yyx);
    Data f = getInput(fl + w.xyx);
    Data g = getInput(fl + w.yxx);
    Data h = getInput(fl + w.xxx);
    {
        float i = a.density;
        float j = b.density;
        float k = c.density;
        float l = d.density;
        float m = e.density;
        float n = f.density;
        float o = g.density;
        float p = h.density;
        output.density = lerps(fr.x, fr.y, fr.z, i, j, k, l, m, n, o, p);
    }
    {
        float3 i = a.velocity;
        float3 j = b.velocity;
        float3 k = c.velocity;
        float3 l = d.velocity;
        float3 m = e.velocity;
        float3 n = f.velocity;
        float3 o = g.velocity;
        float3 p = h.velocity;
        output.velocity = lerps(fr.x, fr.y, fr.z, i, j, k, l, m, n, o, p);
    }
    setOutput(id, output);
}