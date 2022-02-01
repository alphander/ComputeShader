cbuffer constants : register(b0)
{
    int width, length, height;
    int area;
    float viscosity;
    int step;
};

struct Data
{
    float pressure;
    float3 velocity;
};