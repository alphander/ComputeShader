#pragma once
#include "Resources.h"

int CreateComputeShader(ID3D11Device* device, ID3D11DeviceContext* c, ID3D11ComputeShader** shader, LPCWSTR file, char entryFunc[]);
void CreateDevice(ID3D11Device** inDevice, ID3D11DeviceContext** inContext);
int CreateConstants(ID3D11Device* device, ID3D11Buffer** inBuffer, Constant* c);
int CreateDynamicConstants(ID3D11Device* device, ID3D11Buffer** inBuffer, DynamicConstant* c);
int CreateBuffer(ID3D11Device* device, ID3D11Buffer** inBuffer, ID3D11UnorderedAccessView** inView, int size, int count, Data* init);
int CreateTexture(ID3D11Device* device, ID3D11Texture3D** inTexture, ID3D11UnorderedAccessView** inView, int width, int height, int depth, float* init);
int CreateAccess(ID3D11Device* device, ID3D11Buffer** inBuffer, int size, int count);
void UpdateDynamicConstants(ID3D11DeviceContext* context, ID3D11Buffer* buffer, DynamicConstant* data);
void DispatchComputeShader(unsigned int threadX, unsigned int threadY, unsigned int threadZ, ID3D11Device* device, ID3D11DeviceContext* context);