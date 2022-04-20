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
	float nu_dt;
	float kappa_dt;
	float dt_rho;
	float dx, dy, dz;
	int step;
};

struct Data
{
	int type;
	DX::XMFLOAT3 velocity;
	float pressure;
	float divergence;
	float concentration;

	int padding1;
};