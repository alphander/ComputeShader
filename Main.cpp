#include "Helper.h"
#include "VTKHelper.h"
#include <time.h>

const int width = 64, length = 64, height = 64;
const int threadX = 8, threadY = 8, threadZ = 8;
const int diffusionGaussSeidelIters = 100;
const int pressureGaussSeidelIters = 100;
const int simulationSteps = 3;

char entry[] = "CSMain";

LPCWSTR diffusion = L"Diffusion.hlsl";
LPCWSTR advection = L"Advection.hlsl";
LPCWSTR calculateDivergence = L"CalculateDivergence.hlsl";
LPCWSTR calculatePressure = L"CalculatePressure.hlsl";
LPCWSTR clearDivergence = L"ClearDivergence.hlsl";

int main()
{
    const int size = sizeof(Data);
    const int count = width * length * height;
    const int volume = count * size;
    float viscosity = 0.1f;
    float dt = 0.01f;

    Data* initInput = new Data[volume];
    Data* initOutput = new Data[volume];
    for (int i = 0; i < volume; i++)
    {
        initInput[i].velocity = DX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        initInput[i].density = 0.0f;
        initInput[i].divergence = 0.0f;
        initInput[i].pressure = 0.0f;
    }
    initInput[32].velocity = DX::XMFLOAT3(10.0f, 2000.0f, 100.0f);
    initInput[32].density = 512.0f;

    Constant constant;
    constant.width = width;
    constant.length = length;
    constant.height = height;
    constant.area = length * height;

    DynamicConstant dynamicConstant;
    dynamicConstant.dt = dt;
    dynamicConstant.viscosity = viscosity;
    dynamicConstant.step = 0;

    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    ID3D11ComputeShader* diffusionShader = nullptr;
    ID3D11ComputeShader* advectionShader = nullptr;
    ID3D11ComputeShader* calculateDivergenceShader = nullptr;
    ID3D11ComputeShader* calculatePressureShader = nullptr;
    ID3D11ComputeShader* clearDivergenceShader = nullptr;

    ID3D11Buffer* constantBuffer = nullptr;
    ID3D11Buffer* dynamicConstantBuffer = nullptr;
    ID3D11Buffer* inputBuffer = nullptr;
    ID3D11Buffer* outputBuffer = nullptr;
    ID3D11Buffer* cpuBuffer = nullptr;

    ID3D11UnorderedAccessView* inputView = nullptr;
    ID3D11UnorderedAccessView* outputView = nullptr;

    CreateDevice(&device, &context);
    CreateComputeShader(device, context, &diffusionShader, diffusion, entry);
    CreateComputeShader(device, context, &advectionShader, advection, entry);
    CreateComputeShader(device, context, &calculateDivergenceShader, calculateDivergence, entry);
    CreateComputeShader(device, context, &calculatePressureShader, calculatePressure, entry);
    CreateComputeShader(device, context, &clearDivergenceShader, clearDivergence, entry);

    CreateConstants(device, &constantBuffer, &constant);
    CreateDynamicConstants(device, &dynamicConstantBuffer, &dynamicConstant);

    CreateBuffer(device, &inputBuffer, &inputView, size, count, initInput);
    CreateBuffer(device, &outputBuffer, &outputView, size, count, initOutput);
    CreateAccess(device, &cpuBuffer, size, count);

    ID3D11Buffer* constantBuffers[2] = { constantBuffer, dynamicConstantBuffer };

    context->CSSetConstantBuffers(0, 2, constantBuffers);

    ID3D11UnorderedAccessView* views1[2] = {inputView, outputView};
    ID3D11UnorderedAccessView* views2[2] = {outputView, inputView};

    float dx = (float)width, dy = (float)length, dz = (float)height;
    float tx = (float)threadX, ty = (float)threadY, tz = (float)threadZ;
    int x = ceil(dx / tx), y = ceil(dy / ty), z = ceil(dz / tz);

    //Run-------------------------
    int flip = 0;
    for (int i = 0; i < simulationSteps; i++)
    {
        context->CSSetShader(diffusionShader, nullptr, 0);
        for (int j = 0; j < diffusionGaussSeidelIters; j++)
        {
            UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
            context->CSSetUnorderedAccessViews(0, 2, flip % 2 == 0 ? views1 : views2, 0);
            context->Dispatch(x, y, z);
            dynamicConstant.step++;
            flip++;
        }

        context->CSSetShader(advectionShader, nullptr, 0);
        UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
        context->CSSetUnorderedAccessViews(0, 2, flip % 2 == 0 ? views1 : views2, 0);
        context->Dispatch(x, y, z);
        flip++;

        context->CSSetShader(calculateDivergenceShader, nullptr, 0);
        UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
        context->CSSetUnorderedAccessViews(0, 2, flip % 2 == 0 ? views1 : views2, 0);
        context->Dispatch(x, y, z);
        flip++;

        context->CSSetShader(calculatePressureShader, nullptr, 0);
        for (int j = 0; j < pressureGaussSeidelIters; j++)
        {
            UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
            context->CSSetUnorderedAccessViews(0, 2, flip % 2 == 0 ? views1 : views2, 0);
            context->Dispatch(x, y, z);
            dynamicConstant.step++;
            flip++;
        }

        context->CSSetShader(clearDivergenceShader, nullptr, 0);
        UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
        context->CSSetUnorderedAccessViews(0, 2, flip % 2 == 0 ? views1 : views2, 0);
        context->Dispatch(x, y, z);
        flip++;
        cout << "Step: " << i << std::endl;
    }
    //End-----------------------------------

    context->CopyResource(cpuBuffer, outputBuffer);

    D3D11_MAPPED_SUBRESOURCE map;
    HRESULT hr = context->Map(cpuBuffer, 0, D3D11_MAP_READ, 0, &map);

    const Data* outputData = reinterpret_cast<const Data*>(map.pData);

    context->Unmap(cpuBuffer, 0);

    vtk(width, length, height, count, outputData);

    inputBuffer->Release();
    outputBuffer->Release();
    cpuBuffer->Release();
    outputView->Release();
    inputView->Release();
    constantBuffer->Release();
    dynamicConstantBuffer->Release();
    context->Release();
    device->Release();
    diffusionShader->Release();
    advectionShader->Release();
    calculateDivergenceShader->Release();
    calculatePressureShader->Release();
    clearDivergenceShader->Release();

    cout << "Success!" << endl;
    return 0;
}