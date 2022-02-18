#include "Helper.h"
#include "VTKHelper.h"
#include <future>

const int width = 64, length = 64, height = 64;
const int threadX = 4, threadY = 4, threadZ = 4;
const int diffusionIters = 32;
const int pressureIters = 32;
const int simSteps = 16384;

const int fileSaveInterval = 1024;

char entry[] = "CSMain";
const char saveDirectory[] = "D:/Desktop/VTKs/data";

LPCWSTR diffusion = L"Diffusion.hlsl";
LPCWSTR advection = L"Advection.hlsl";
LPCWSTR calculateDivergence = L"CalculateDivergence.hlsl";
LPCWSTR calculatePressure = L"CalculatePressure.hlsl";
LPCWSTR clearDivergence = L"ClearDivergence.hlsl";

const int count = width * length * height;
const int size = sizeof(Data);
const int volume = count * size;
const float viscosity = 0.5f;
const float dt = 0.002f;

int main()
{
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
	dynamicConstant.side = 0;

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
	ID3D11UnorderedAccessView* viewsBC[2] = {bView, cView};
	ID3D11UnorderedAccessView* viewsCA[2] = {cView, aView};

	ID3D11UnorderedAccessView** pairs[3] = {viewsAB, viewsBC, viewsCA};
	ID3D11Buffer* buffers[3] = {aBuffer, bBuffer, cBuffer};

	float dx = (float)width, dy = (float)length, dz = (float)height;
	float tx = (float)threadX, ty = (float)threadY, tz = (float)threadZ;
	int x = ceil(dx / tx), y = ceil(dy / ty), z = ceil(dz / tz);

	cout << "Running Shader..." << endl;

	//Run-------------------------
	int side = 0;
	int step = 0;
	int saves = 0;
	int saveId = 0;
	int saveOk = 0;
	std::future<void> thread;
	context->CSSetUnorderedAccessViews(0, 2, pairs[0], 0);

	for (int i = 0; i < simSteps; i++)
	{
		context->CSSetShader(diffusionShader, nullptr, 0);
		for (int j = 0; j < diffusionIters; j++)
		{
			UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
			context->Dispatch(x, y, z);
			side++;
			step++;
			dynamicConstant.side = side;
			dynamicConstant.step = step;
		}

		context->CSSetShader(advectionShader, nullptr, 0);
		UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
		context->Dispatch(x, y, z);
		side++;
		dynamicConstant.side = side;

		context->CSSetShader(calculateDivergenceShader, nullptr, 0);
		UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
		context->Dispatch(x, y, z);
		side++;
		dynamicConstant.side = side;

		context->CSSetShader(calculatePressureShader, nullptr, 0);
		for (int j = 0; j < pressureIters; j++)
		{
			UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
			context->Dispatch(x, y, z);
			side++;
			step++;
			dynamicConstant.side = side;
			dynamicConstant.step = step;
		}

		context->CSSetShader(clearDivergenceShader, nullptr, 0);
		UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
		context->Dispatch(x, y, z);
		side++;
		dynamicConstant.side = side;

		cout << "Step: " << i << endl;

		if (saveOk >= fileSaveInterval)
		{
			cout << "Saving results..." << endl;
			ID3D11Buffer* current = buffers[saves % 3];

			if (saves == 0)
				context->CopyResource(cpuBuffer, current);

			cout << "Accessing results..." << endl;
			D3D11_MAPPED_SUBRESOURCE map;
			HRESULT hr = DXGI_ERROR_WAS_STILL_DRAWING;
			while (hr == DXGI_ERROR_WAS_STILL_DRAWING)
				hr = context->Map(cpuBuffer, 0, D3D11_MAP_READ, D3D11_MAP_FLAG_DO_NOT_WAIT, &map);

			const Data* outputData = reinterpret_cast<const Data*>(map.pData);
			context->Unmap(cpuBuffer, 0);
			cout << "Finished Accessing results..." << endl;

			cout << "Saving results..." << endl;
			if (!saves == 0) thread.wait();
			thread = std::async(std::launch::async, vtkBinary, width, length, height, count, outputData, saveDirectory, saves);

			context->CopyResource(cpuBuffer, current);
			cout << "Finished saving results..." << endl;

			side += side % 2;
			saves++;
			saveId++;
			saveOk = 0;
			context->CSSetUnorderedAccessViews(0, 2, pairs[saves % 3], 0);
		}
		saveOk++;
	}

	cout << "Finished running shader!" << endl;

	//End-----------------------------------

	aBuffer->Release();
	bBuffer->Release();
	cBuffer->Release();
	cpuBuffer->Release();
	aView->Release();
	bView->Release();
	cView->Release();
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