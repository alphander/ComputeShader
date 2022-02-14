#include "Helper.h"
#include "VTKHelper.h"
#include <thread>

const int width = 64, length = 64, height = 64;
const int threadX = 8, threadY = 8, threadZ = 8;
const int diffusionIters = 64;
const int pressureIters = 64;
const int simSteps = 64;

const int fileSaveInterval = 64;

char entry[] = "CSMain";
const char saveDirectory[] = "D:/Desktop/VTKs/test";

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
    float viscosity = 0.5f;
    float dt = 0.01f;

    Data* init = new Data[volume];
    for (int i = 0; i < volume; i++)
    {
        init[i].velocity = DX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        init[i].density = 0.0f;
        init[i].divergence = 0.0f;
        init[i].pressure = 0.0f;
    }
    int k = 16 + 32 * length + 32 * length * height;
    init[k].density = 1000.0f;
    init[k].velocity = DX::XMFLOAT3(128.0f, 0, 0);

    int l = 48 + 32 * length + 32 * length * height;
    init[l].density = 1000.0f;
    init[l].velocity = DX::XMFLOAT3(-128.0f, 0, 0);

    Constant constant;
    constant.width = width;
    constant.length = length;
    constant.height = height;
    constant.area = length * height;

    DynamicConstant dynamicConstant;
    dynamicConstant.dt = dt;
    dynamicConstant.viscosity = viscosity * dt;
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
    ID3D11Buffer* aBuffer = nullptr;
    ID3D11Buffer* bBuffer = nullptr;
    ID3D11Buffer* cBuffer = nullptr;
    ID3D11Buffer* cpuBuffer = nullptr;

    ID3D11UnorderedAccessView* aView = nullptr;
    ID3D11UnorderedAccessView* bView = nullptr;
    ID3D11UnorderedAccessView* cView = nullptr;

    CreateDevice(&device, &context);
    CreateComputeShader(device, context, &diffusionShader, diffusion, entry);
    CreateComputeShader(device, context, &advectionShader, advection, entry);
    CreateComputeShader(device, context, &calculateDivergenceShader, calculateDivergence, entry);
    CreateComputeShader(device, context, &calculatePressureShader, calculatePressure, entry);
    CreateComputeShader(device, context, &clearDivergenceShader, clearDivergence, entry);

    CreateConstants(device, &constantBuffer, &constant);
    CreateDynamicConstants(device, &dynamicConstantBuffer, &dynamicConstant);

    CreateBuffer(device, &aBuffer, &aView, size, count, init);
    CreateBuffer(device, &bBuffer, &bView, size, count, init);
    CreateBuffer(device, &cBuffer, &cView, size, count, init);
    delete[] init;
    CreateAccess(device, &cpuBuffer, size, count);

    ID3D11Buffer* constantBuffers[2] = { constantBuffer, dynamicConstantBuffer };

    context->CSSetConstantBuffers(0, 2, constantBuffers);

    ID3D11UnorderedAccessView* viewsAB[2] = {aView, bView};
    ID3D11UnorderedAccessView* viewsBA[2] = {bView, aView};

    ID3D11UnorderedAccessView** pairs[2] = {viewsAB, viewsBA};

    float dx = (float)width, dy = (float)length, dz = (float)height;
    float tx = (float)threadX, ty = (float)threadY, tz = (float)threadZ;
    int x = ceil(dx / tx), y = ceil(dy / ty), z = ceil(dz / tz);

    cout << "Running Shader..." << endl;

    //Run-------------------------
    int side = 0;
    int pair = 0;
    for (int i = 0; i < simSteps; i++)
    {
        context->CSSetShader(diffusionShader, nullptr, 0);
        for (int j = 0; j < diffusionIters; j++)
        {
            UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
            context->CSSetUnorderedAccessViews(0, 2, pairs[side % 2], 0);
            context->Dispatch(x, y, z);
            dynamicConstant.step++;
            side++;
        }

        context->CSSetShader(advectionShader, nullptr, 0);
        UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
        context->CSSetUnorderedAccessViews(0, 2, pairs[side % 2], 0);
        context->Dispatch(x, y, z);
        side++;

        context->CSSetShader(calculateDivergenceShader, nullptr, 0);
        UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
        context->CSSetUnorderedAccessViews(0, 2, pairs[side % 2], 0);
        context->Dispatch(x, y, z);
        side++;

        context->CSSetShader(calculatePressureShader, nullptr, 0);
        for (int j = 0; j < pressureIters; j++)
        {
            UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
            context->CSSetUnorderedAccessViews(0, 2, pairs[side % 2], 0);
            context->Dispatch(x, y, z);
            dynamicConstant.step++;
            side++;
        }

        context->CSSetShader(clearDivergenceShader, nullptr, 0);
        UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
        context->CSSetUnorderedAccessViews(0, 2, pairs[side % 2], 0);
        context->Dispatch(x, y, z);
        side++;

        cout << "Step: " << i << endl;

        if (i % fileSaveInterval != fileSaveInterval - 1)
            continue;

        cout << "Accessing results..." << endl;
        context->CopyResource(cpuBuffer, cBuffer);

        D3D11_MAPPED_SUBRESOURCE map;
        HRESULT hr = context->Map(cpuBuffer, 0, D3D11_MAP_READ, 0, &map);
        const Data* outputData = reinterpret_cast<const Data*>(map.pData);
        context->Unmap(cpuBuffer, 0);

        cout << "Finished accessing results!" << endl;

        cout << "Writing to file..." << endl;

        vtkBinary(width, length, height, count, outputData, saveDirectory, i / fileSaveInterval);
        cout << "Finished Writing to file!" << endl;

        cout << "Finished running shader!" << endl;

        
    }

    //End-----------------------------------

    aBuffer->Release();
    bBuffer->Release();
    cpuBuffer->Release();
    aView->Release();
    bView->Release();
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