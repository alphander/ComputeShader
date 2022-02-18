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
    int side;
};

struct Data
{
    float3 velocity;
    float pressure;
    float divergence;
    float density;
    
    float padding1, padding2;
};

RWStructuredBuffer<Data> A : register(u0);
RWStructuredBuffer<Data> B : register(u1);

uint get(uint3 id)
{
    int3 dims = int3(width, length, height);
    id = mod(id, dims);
    return id.x + id.y * length + id.z * area;
}

Data getInput(uint3 id)
{
    uint index = get(id);
    Data data;
    if(side % 2 == 0)
        data = A[index];
    else
        data = B[index];
    return data;
}

Data getOutput(uint3 id)
{
    uint index = get(id);
    Data data;
    if (side % 2 == 0)
        data = B[index];
    else
        data = A[index];
    return data;
}

void setOutput(uint3 id, Data data)
{
    uint index = get(id);
    if(side % 2 == 0)
        B[index] = data;
    else
        A[index] = data;
}