#include "Resources.h"

//https://docs.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-create

HRESULT CompileComputeShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ ID3D11Device* device, _Outptr_ ID3DBlob** blob)
{
	if (!srcFile || !entryPoint || !device || !blob) return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	LPCSTR profile = "cs_5_0";

	const D3D_SHADER_MACRO defines[] = { "EXAMPLE_DEFINE", "1", NULL, NULL };

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint, profile,
		flags, 0, &shaderBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();

		return hr;
	}

	*blob = shaderBlob;

	return hr;
}

int CreateComputeShader(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ComputeShader** shader, LPCWSTR file, char entryFunc[])
{
	ID3DBlob* csBlob = nullptr;
	HRESULT hr = CompileComputeShader(file, entryFunc, device, &csBlob);
	if (FAILED(hr)) return -1;

	ID3D11ComputeShader* computeShader = nullptr;
	hr = device->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, &computeShader);
	if (FAILED(hr)) return -1;

	csBlob->Release();

	*shader = computeShader;

	printf("Created Compute Shader\n");

	return 0;
}

void CreateDevice(ID3D11Device** inDevice, ID3D11DeviceContext** inContext)
{
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;

	const D3D_FEATURE_LEVEL lvl[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, lvl, _countof(lvl), D3D11_SDK_VERSION, &device, nullptr, nullptr);
	if (hr == E_INVALIDARG)
	{
		// DirectX 11.0 Runtime doesn't recognize D3D_FEATURE_LEVEL_11_1 as a valid value
		hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &lvl[1], _countof(lvl) - 1,
			D3D11_SDK_VERSION, &device, nullptr, nullptr);
	}

	device->GetImmediateContext(&context);

	*inDevice = device;
	*inContext = context;
}

int CreateConstants(ID3D11Device* device, ID3D11Buffer** inBuffer, Constant* c)
{
	ID3D11Buffer* buffer = nullptr;

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(Constant);
	cbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = c;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&cbDesc, &InitData, &buffer);
	if (FAILED(hr)) return -1;

	*inBuffer = buffer;

	return 0;
}

int CreateDynamicConstants(ID3D11Device* device, ID3D11Buffer** inBuffer, DynamicConstant* c)
{
	ID3D11Buffer* buffer = nullptr;

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(DynamicConstant);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = c;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&cbDesc, &InitData, &buffer);
	if (FAILED(hr)) return -1;

	*inBuffer = buffer;

	return 0;
}

int CreateBuffer(ID3D11Device* device, ID3D11Buffer** inBuffer, ID3D11UnorderedAccessView** inView, int size, int count, Data* init)
{
	ID3D11Buffer* buffer = nullptr;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = init;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC descBuffer;
	{
		descBuffer.Usage = D3D11_USAGE_DEFAULT;
		descBuffer.ByteWidth = size * count;
		descBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		descBuffer.CPUAccessFlags = 0;
		descBuffer.StructureByteStride = size;
		descBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	}
	HRESULT hr = device->CreateBuffer(&descBuffer, &data, &buffer);
	if (FAILED(hr)) return -1;

	ID3D11UnorderedAccessView* view = nullptr;
	D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
	{
		descView.Buffer.FirstElement = 0;
		descView.Buffer.Flags = 0;
		descView.Buffer.NumElements = count;
		descView.Format = DXGI_FORMAT_UNKNOWN;
		descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	}
	hr = device->CreateUnorderedAccessView(buffer, &descView, &view);
	if (FAILED(hr)) return -1;

	*inBuffer = buffer;
	*inView = view;

	return 0;
}

int CreateTexture(ID3D11Device* device, ID3D11Texture3D** inTexture, ID3D11UnorderedAccessView** inView, int width, int height, int depth, float* init)
{
	ID3D11Texture3D* texture = nullptr;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = init;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	D3D11_TEXTURE3D_DESC descTexture;
	{
		descTexture.Usage = D3D11_USAGE_DEFAULT;
		descTexture.MipLevels = 1;
		descTexture.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		descTexture.CPUAccessFlags = 0;
		descTexture.Width = width;
		descTexture.Height = height;
		descTexture.Depth = depth;
		descTexture.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		descTexture.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	}
	HRESULT hr = device->CreateTexture3D(&descTexture, &data, &texture);
	if (FAILED(hr)) return -1;

	ID3D11UnorderedAccessView* view = nullptr;
	D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
	{
		descView.Buffer.FirstElement = 0;
		descView.Buffer.Flags = 0;
		descView.Buffer.NumElements = width * height * depth;
		descView.Format = DXGI_FORMAT_UNKNOWN;
		descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	}
	hr = device->CreateUnorderedAccessView(texture, &descView, &view);
	if (FAILED(hr)) return -1;

	*inTexture = texture;
	*inView = view;

	return 0;
}

int CreateAccess(ID3D11Device* device, ID3D11Buffer** inBuffer, int size, int count)
{
	ID3D11Buffer* buffer = nullptr;

	D3D11_BUFFER_DESC descBuffer;
	{
		descBuffer.Usage = D3D11_USAGE_STAGING;
		descBuffer.ByteWidth = size * count;
		descBuffer.BindFlags = 0;
		descBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		descBuffer.StructureByteStride = size;
		descBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	}
	HRESULT hr = device->CreateBuffer(&descBuffer, 0, &buffer);
	if (FAILED(hr)) return -1;

	*inBuffer = buffer;

	return 0;
}

void UpdateDynamicConstants(ID3D11DeviceContext* context, ID3D11Buffer* buffer, DynamicConstant* data)
{
	D3D11_MAPPED_SUBRESOURCE map;
	ZeroMemory(&map, sizeof(D3D11_MAPPED_SUBRESOURCE));
	context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	memcpy(map.pData, data, sizeof(Constant));
	context->Unmap(buffer, 0);
}