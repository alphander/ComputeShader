#include "Resources.hlsli"

[numthreads(4, 4, 4)]
void CSMain(uint3 id : SV_DispatchThreadID)
{   
    Data input = getInput(id);
    Data output = getOutput(id);
    
    uint2 w = uint2(1, 0);
    Data xp1 = getInput(id + w.xyy);
    Data xm1 = getInput(id - w.xyy);
    Data yp1 = getInput(id + w.yxy);
    Data ym1 = getInput(id - w.yxy);
    Data zp1 = getInput(id + w.yyx);
    Data zm1 = getInput(id - w.yyx);
    
    float3 a = xp1.velocity;
    float3 b = xm1.velocity;
    float3 c = yp1.velocity;
    float3 d = ym1.velocity;
    float3 e = zp1.velocity;
    float3 f = zm1.velocity;
    
    output.divergence = ((a.x - b.x) + (c.y - d.y) + (e.z - f.z)) / 3.0;
    setOutput(id, output);
}