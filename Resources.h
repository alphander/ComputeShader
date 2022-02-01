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
	int width = 1, length = 1, height = 1;
	int area = 1;
	float viscosity = 0.01f;
	int step;
};

struct Data
{
	float pressure;
	DX::XMFLOAT3 velocity;
};