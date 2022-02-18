#include "Resources.hlsli"
#define diffuse(a, b, c, d, e, f, g) (a + k * ((b + c + d + e + f + g) / 6)) / (1.0 + k);

[numthreads(4, 4, 4)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    Data input = getInput(id);
    Data output = getOutput(id);
    
    if ((id.x + id.z + id.y) % step == 0)
    {
        setOutput(id, input);
        return;
    }
    
    float k = viscosity;
    
    uint2 w = uint2(1, 0);
    Data xp1 = getInput(id + w.xyy);
    Data xm1 = getInput(id - w.xyy);
    Data yp1 = getInput(id + w.yxy);
    Data ym1 = getInput(id - w.yxy);
    Data zp1 = getInput(id + w.yyx);
    Data zm1 = getInput(id - w.yyx);
    {
        float a = input.density;
        float b = xp1.density;
        float c = xm1.density;
        float d = yp1.density;
        float e = ym1.density;
        float f = zp1.density;
        float g = zm1.density;
        output.density = diffuse(a, b, c, d, e, f, g);
    }
    {
        float3 a = input.velocity;
        float3 b = xp1.velocity;
        float3 c = xm1.velocity;
        float3 d = yp1.velocity;
        float3 e = ym1.velocity;
        float3 f = zp1.velocity;
        float3 g = zm1.velocity;
        output.velocity = diffuse(a, b, c, d, e, f, g);
    }
    setOutput(id, output);
}