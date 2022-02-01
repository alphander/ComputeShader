#include "Helper.h"
#include <time.h>

const int width = 8, length = 8, height = 8;
const int threadX = 8, threadY = 8, threadZ = 8;
const float viscosity = 0.01f;

LPCWSTR diffusion = L"Diffusion.hlsl";

char entry[] = "CSMain";

int main()
{
    const int size = sizeof(Data);
    const int count = width * length * height;
    const int volume = count * size;
    Data* initInput = new Data[volume];
    Data* initOutput = new Data[volume];
    for (int i = 0; i < volume; i++)
    {
        initInput[i].pressure = 0.0f;
        initInput[i].velocity = DX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    Constant constants;
    constants.width = width;
    constants.length = length;
    constants.height = height;
    constants.area = length * height;
    constants.viscosity = viscosity;

    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    ID3D11ComputeShader* shader = nullptr;

    ID3D11Buffer* constantsBuffer = nullptr;
    ID3D11Buffer* inputBuffer = nullptr;
    ID3D11UnorderedAccessView* inputView = nullptr;
    ID3D11Buffer* outputBuffer = nullptr;
    ID3D11UnorderedAccessView* outputView = nullptr;
    ID3D11Buffer* cpuBuffer = nullptr;

    CreateDevice(&device, &context);
    CreateComputeShader(device, context, &shader, diffusion, entry);
    CreateConstants(device, &constantsBuffer, &constants);
    CreateBuffer(device, &inputBuffer, &inputView, size, count, initInput);
    CreateBuffer(device, &outputBuffer, &outputView, size, count, initOutput);
    CreateAccess(device, cpuBuffer, size, count);

    context->CSSetConstantBuffers(0, 1, &constantsBuffer);

    ID3D11UnorderedAccessView* views1[2] = {inputView, outputView};
    ID3D11UnorderedAccessView* views2[2] = {outputView, inputView};

    float dx = (float)width, dy = (float)length, dz = (float)height;
    float tx = (float)threadX, ty = (float)threadY, tz = (float)threadZ;
    int x = ceil(dx / tx), y = ceil(dy / ty), z = ceil(dz / tz);

    for (int i = 0; i < 100; i++)
    {
        context->CSSetUnorderedAccessViews(0, 2, i % 2 == 0 ? views1 : views2, 0);
        context->Dispatch(x, y, z);
    }

    context->CopyResource(cpuBuffer, outputBuffer);

    D3D11_MAPPED_SUBRESOURCE map;
    HRESULT hr = context->Map(cpuBuffer, 0, D3D11_MAP_READ, 0, &map);

    if (SUCCEEDED(hr))
    {
        const auto values = reinterpret_cast<const Data*>(map.pData);
        for (int i = 0; i < count; i++)
        {
            float c = values[i].pressure;
            cout << c << endl;
        }
        context->Unmap(cpuBuffer, 0);
    }

    inputBuffer->Release();
    outputBuffer->Release();
    cpuBuffer->Release();
    outputView->Release();
    inputView->Release();
    constantsBuffer->Release();
    context->Release();
    device->Release();
    shader->Release();

    cout << "Success!" << endl;
    return 0;
}