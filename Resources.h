#pragma once

#include <stdio.h>
#include <iostream>

#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

using std::cout;
using std::endl;
namespace DX = DirectX;

struct Constant
{
	int width, length, height;
	int area;
};

struct DynamicConstant
{
	float dt;
	float viscosity;
	int step;

	float padding1;
};

struct Data
{
	DX::XMFLOAT3 velocity;
	float pressure;
	float divergence;
	float density;

	float padding1, padding2;
};