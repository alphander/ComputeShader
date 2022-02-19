#define mod(x,y) (x-y*floor(x/y))

cbuffer Constant : register(b0)
{
    int width, length, height;
    int area;
};

cbuffer DynamicConstant : register(b1)
{
    float dt;
    float viscosity;
    int step;
    
    int padding1;
};

struct Data
{
    float3 velocity;
    float pressure;
    float divergence;
    float density;
    
    int padding1, padding2;
};

RWStructuredBuffer<Data> Input : register(u0);
RWStructuredBuffer<Data> Output : register(u1);

uint get(uint3 id)
{
    int3 dims = int3(width, length, height);
    id = mod(id, dims);
    return id.x + id.y * length + id.z * area;
}