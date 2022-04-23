#define mod(x,y) (x-y*floor(x/y))

cbuffer Constant : register(b0)
{
    int width, length, height;
    int area;
};

cbuffer DynamicConstant : register(b1)
{
    float dt;
    float nu_dt;
    float kappa_dt;
    float dt_rho;
    float dx, dy, dz;
    int step;
};

struct Cell
{
    int type;
    float3 velocity;
    float pressure;
    float divergence;
    float concentration;
    
    int padding1;
};

RWStructuredBuffer<Cell> Input : register(u0);
RWStructuredBuffer<Cell> Output : register(u1);

uint get(uint3 id)
{
    int3 dims = int3(width, length, height);
    id = mod(id, dims);
    return id.x + id.y * length + id.z * area;
}