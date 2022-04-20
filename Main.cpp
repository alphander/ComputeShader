#include "Helper.h"
#include "VTKHelper.h"
#include <future>

const int width = 64, length = 64, height = 64;
const int threadX = 8, threadY = 8, threadZ = 8;
const int diffusionIters = 128;
const int pressureIters = 128;
const int simSteps = 25000;

const int fileSaveInterval = 250;

char entry[] = "CSMain";
const char saveDirectory[] = "VTKs/data";

LPCWSTR diffusion = L"Diffusion.hlsl";
LPCWSTR advection = L"Advection.hlsl";
LPCWSTR calculateDivergence = L"CalculateDivergence.hlsl";
LPCWSTR calculatePressure = L"CalculatePressure.hlsl";
LPCWSTR clearDivergence = L"ClearDivergence.hlsl";

const int count = width * length * height;
const int size = sizeof(Data);
const int volume = count * size;
const float nu = 0.16f;
const float kappa = 0.16f;
const float dt = 0.0005f;
const float rho = 1.0f;
const float dx = 1.0f, dy = 1.0f, dz = 1.0f;

Data* createData()
{
	//Creating volume
	Data* init = new Data[volume];

	//Initializing volume
	for (int i = 0; i < volume; i++)
	{
		init[i].type = 0;
		init[i].velocity = DX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		init[i].concentration = 0.0f;
		init[i].divergence = 0.0f;
		init[i].pressure = 0.0f;
	}

	//Initializing initial velocities
	/*int a = 16 + 32 * length + 32 * length * height;
	init[a].concentration = 100.0f;
	init[a].velocity = DX::XMFLOAT3(1.0f, 0, 0);

	int b = 48 + 32 * length + 32 * length * height;
	init[b].concentration = 100.0f;
	init[b].velocity = DX::XMFLOAT3(-1.0f, 0, 0);*/

	//Adding boundary
	for (int i = 0; i < length; i++)
		for (int j = 0; j < height; j++)
			for (int k = 0; k < width; k++)
			{
				if (!(i == 0 || j == 0 || k == 0 || i == 63 || j == 63 || k == 63)) continue;

				int coord = i + j * length + k * length * height;

				init[coord].type = 1;
			}

	//Adding box
	/*for (int i = 0; i < length; i++)
		for (int j = 0; j < height; j++)
			for (int k = 0; k < width; k++)
			{
				if (!(i >= 16 && j >= 0 && k >= 16 && i < 48 && j < 63 && k < 48)) continue;

				int coord = i + j * length + k * length * height;

				init[coord].type = 1;
			}*/

	//Adding sliding top
	for (int i = 0; i < length; i++)
		for (int j = 0; j < height; j++)
			for (int k = 0; k < width; k++)
			{
				if (!(k == 63)) continue;

				int coord = i + j * length + k * length * height;

				init[coord].velocity = DX::XMFLOAT3(1.0, 0.0, 0.0);
			}

	return init;
}

int main()
{
	Data* init = createData();

	Constant constant;
	constant.width = width;
	constant.length = length;
	constant.height = height;
	constant.area = length * height;

	DynamicConstant dynamicConstant;
	dynamicConstant.dt = dt;
	dynamicConstant.nu_dt = nu * dt;
	dynamicConstant.kappa_dt = kappa * dt;
	dynamicConstant.dt_rho = dt / rho;
	dynamicConstant.dx = dx;
	dynamicConstant.dy = dy;
	dynamicConstant.dz = dz;
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

	ID3D11UnorderedAccessView* viewsAB[2] = { aView, bView };
	ID3D11UnorderedAccessView* viewsBA[2] = { bView, aView };
	ID3D11UnorderedAccessView* viewsBC[2] = { bView, cView };
	ID3D11UnorderedAccessView* viewsCB[2] = { cView, bView };
	ID3D11UnorderedAccessView* viewsCA[2] = { cView, aView };
	ID3D11UnorderedAccessView* viewsAC[2] = { aView, cView };

	ID3D11UnorderedAccessView** pairs1[2] = { viewsAB, viewsBA };
	ID3D11UnorderedAccessView** pairs2[2] = { viewsBC, viewsCB };
	ID3D11UnorderedAccessView** pairs3[2] = { viewsCA, viewsAC };

	ID3D11UnorderedAccessView*** pairs[3] = {pairs1, pairs2, pairs3};

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

	for (int i = 0; i < simSteps; i++)
	{
		context->CSSetShader(diffusionShader, nullptr, 0);
		for (int j = 0; j < diffusionIters; j++)
		{
			UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
			context->CSSetUnorderedAccessViews(0, 2, pairs[saves % 3][side % 2], 0);
			context->Dispatch(x, y, z);
			dynamicConstant.step++;
			side++;
		}

		context->CSSetShader(advectionShader, nullptr, 0);
		context->CSSetUnorderedAccessViews(0, 2, pairs[saves % 3][side % 2], 0);
		UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
		context->Dispatch(x, y, z);
		side++;

		context->CSSetShader(calculateDivergenceShader, nullptr, 0);
		context->CSSetUnorderedAccessViews(0, 2, pairs[saves % 3][side % 2], 0);
		UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
		context->Dispatch(x, y, z);
		side++;

		context->CSSetShader(calculatePressureShader, nullptr, 0);
		for (int j = 0; j < pressureIters; j++)
		{
			context->CSSetUnorderedAccessViews(0, 2, pairs[saves % 3][side % 2], 0);
			UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
			context->Dispatch(x, y, z);
			dynamicConstant.step++;
			side++;
		}

		context->CSSetShader(clearDivergenceShader, nullptr, 0);
		context->CSSetUnorderedAccessViews(0, 2, pairs[saves % 3][side % 2], 0);
		UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);
		context->Dispatch(x, y, z);
		side++;

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