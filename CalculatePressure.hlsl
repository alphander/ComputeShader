#include "Resources.hlsli"

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
    
    uint2 w = uint2(1, 0);
    Data xp1 = getInput(id + w.xyy);
    Data xm1 = getInput(id - w.xyy);
    Data yp1 = getInput(id + w.yxy);
    Data ym1 = getInput(id - w.yxy);
    Data zp1 = getInput(id + w.yyx);
    Data zm1 = getInput(id - w.yyx);
    
    float a = xp1.pressure;
    float b = xm1.pressure;
    float c = yp1.pressure;
    float d = ym1.pressure;
    float e = zp1.pressure;
    float f = zm1.pressure;
    
    output.pressure = (((a + b + c + d + e + f) - input.divergence)) / 6.0;
    setOutput(id, output);
}