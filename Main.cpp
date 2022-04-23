#include "DXUtil.h"
#include "VTKUtil.h"
#include "CellUtil.h"
#include <future>

//Simulation parameters
const int simulationSteps = 25000;
const int width = 64, length = 64, height = 64;

const float nu = 0.16f;
const float kappa = 0.16f;
const float dt = 0.0005f;
const float rho = 1.0f;
const float dx = 1.0f, dy = 1.0f, dz = 1.0f;

//File save parameters
const int fileSaveInterval = 250;
const char saveDirectory[] = "VTKs/data";

//GPU parameters
#define shaderCount 5
#define bufferCount 3
const int threadX = 8, threadY = 8, threadZ = 8;

char entry[] = "CSMain";
LPCWSTR shaderFiles[shaderCount] = {
	L"Diffusion.hlsl",
	L"Advection.hlsl",
	L"CalculateDivergence.hlsl",
	L"CalculatePressure.hlsl",
	L"ClearDivergence.hlsl"
};
const int shaderIterations[shaderCount] = {
	128,
	1,
	1,
	128,
	1
};

ID3D11Device* device = nullptr;
ID3D11DeviceContext* context = nullptr;
ID3D11ComputeShader* shaders[shaderCount];

ID3D11Buffer* constantBuffer = nullptr; //Data that is constant throughout the program.
ID3D11Buffer* dynamicConstantBuffer = nullptr; //Data that is constant throughout a program cycle.
ID3D11Buffer* cpuBuffer = nullptr; //The readback buffer. Lives on RAM

ID3D11Buffer* buffers[bufferCount]; //Rotating buffers allowing us to readback information. Lives on VRAM
ID3D11UnorderedAccessView* bufferViews[bufferCount]; //Views for buffers

Constant constant;
DynamicConstant dynamicConstant;

void enable()
{
	const int size = sizeof(Cell);
	const int count = width * length * height;
	const int volume = count * size;

	//Creating GPU device and context objects. Everything is done with these.
	CreateDevice(&device, &context);

	//Initializing Cell space on RAM
	Cell* cells = createCells(width, height, length);
	for (int i = 0; i < bufferCount; i++)
		CreateBuffer(device, &buffers[i], &bufferViews[i], size, count, cells);
	delete[] cells; //This tends to be pretty large. Free it asap.

	//Initializing data that is constant throughout the program.
	constant.width = width;
	constant.length = length;
	constant.height = height;
	constant.area = length * height;

	//Initializing data that is constant throughout a program cycle.
	dynamicConstant.dt = dt;
	dynamicConstant.nu_dt = nu * dt; 
	dynamicConstant.kappa_dt = kappa * dt;
	dynamicConstant.dt_rho = dt / rho;
	dynamicConstant.dx = dx;
	dynamicConstant.dy = dy;
	dynamicConstant.dz = dz;
	dynamicConstant.step = 0;

	//Creating other buffers
	CreateConstants(device, &constantBuffer, &constant);
	CreateDynamicConstants(device, &dynamicConstantBuffer, &dynamicConstant);
	CreateAccess(device, &cpuBuffer, size, count);

	//Compiling and creating shader objects.
	for (int i = 0; i < shaderCount; i++)
		CreateComputeShader(device, context, &shaders[i], shaderFiles[i], entry);

	ID3D11Buffer* constantBuffers[2] = { constantBuffer, dynamicConstantBuffer };
	context->CSSetConstantBuffers(0, 2, constantBuffers);
}

void runSimulation(int& saves, int& side, int& step)
{
	//Calculating thread dispatch groups.
	float dx = (float)width, dy = (float)length, dz = (float)height;
	float tx = (float)threadX, ty = (float)threadY, tz = (float)threadZ;
	int x = ceil(dx / tx), y = ceil(dy / ty), z = ceil(dz / tz);

	for (int i = 0; i < shaderCount; i++)
	{
		ID3D11ComputeShader* shader = shaders[i];
		context->CSSetShader(shader, nullptr, 0);

		int iterations = shaderIterations[i];
		for (int j = 0; j < iterations; j++)
		{
			UpdateDynamicConstants(context, dynamicConstantBuffer, &dynamicConstant);

			int a = (saves + side % 2) % bufferCount;//Getting A buffer index
			int b = (saves + (side + 1) % 2) % bufferCount;//Getting B buffer index
			ID3D11UnorderedAccessView* pair[2] = { bufferViews[a], bufferViews[b] }; //Putting A and B into array for for loading

			context->CSSetUnorderedAccessViews(0, 2, pair, 0);
			context->Dispatch(x, y, z);
			dynamicConstant.step = step;
			side++;
		}
	}
}

void saveSimulation(int& saves, int& side)
{
	static int saveOk = 0;
	static std::future<void> thread;

	if (saveOk >= fileSaveInterval)
	{
		ID3D11Buffer* current = buffers[saves % bufferCount];

		if (saves == 0) context->CopyResource(cpuBuffer, current); //First iteration loading.

		cout << "Accessing results..." << endl;
		D3D11_MAPPED_SUBRESOURCE map;
		HRESULT hr = DXGI_ERROR_WAS_STILL_DRAWING;
		while (hr == DXGI_ERROR_WAS_STILL_DRAWING)
			hr = context->Map(cpuBuffer, 0, D3D11_MAP_READ, D3D11_MAP_FLAG_DO_NOT_WAIT, &map);

		const Cell* outputData = reinterpret_cast<const Cell*>(map.pData);
		context->Unmap(cpuBuffer, 0);

		cout << "Finished Accessing results..." << endl;
		cout << "Saving results..." << endl;

		if (!saves == 0) thread.wait();

		thread = std::async(std::launch::async, vtkBinary, width, length, height, outputData, saveDirectory, saves);

		context->CopyResource(cpuBuffer, current);
		cout << "Finished saving results..." << endl;

		side += side % 2;
		saves++;
		saveOk = 0;
	}
	saveOk++;
}

void disable()
{
	//Releasing shaders
	for (int i = 0; i < shaderCount; i++)
		shaders[i]->Release();

	//Releasing buffers
	for (int i = 0; i < bufferCount; i++)
		buffers[i]->Release();
	for (int i = 0; i < bufferCount; i++)
		bufferViews[i]->Release();

	//Releasing other buffers and objects
	cpuBuffer->Release();
	constantBuffer->Release();
	dynamicConstantBuffer->Release();
	context->Release();
	device->Release();
}

int main()
{
	int step = 0; //Simulation steps count
	int side = 0; //Buffer side
	int saves = 0; //Number of saves

	cout << "Loading data..." << endl;
	enable();

	cout << "Running Shaders..." << endl;
	for (int i = 0; i < simulationSteps; i++)
	{
		runSimulation(saves, side, step);
		saveSimulation(saves, side);
		cout << "Step: " << i << endl;
	}
	cout << "Finished running shaders!" << endl;
	cout << "Unloading data..." << endl;

	disable();
	cout << "Program ended!" << endl;
}