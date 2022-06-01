#include "Resources.h"

Cell* createCells(int width, int length, int height)
{
	const int size = sizeof(Cell);
	const int count = width * length * height;
	const int volume = count * size;

	//Creating volume
	Cell* cells = new Cell[volume];

	//Initializing volume
	for (int i = 0; i < volume; i++)
	{
		cells[i].type = 0;
		cells[i].velocity = DX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		cells[i].concentration = 0.0f;
		cells[i].divergence = 0.0f;
		cells[i].pressure = 0.0f;
	}

	//Adding boundary
	for (int i = 0; i < length; i++)
		for (int j = 0; j < height; j++)
			for (int k = 0; k < width; k++)
			{
				if (!(i == 0 || j == 0 || k == 0 || i == 63 || j == 63 || k == 63)) continue;

				int coord = i + j * length + k * length * height;

				cells[coord].type = 1;
			}

	//Adding sliding top
	for (int i = 0; i < length; i++)
		for (int j = 0; j < height; j++)
			for (int k = 0; k < width; k++)
			{
				if (!(k == 63)) continue;

				int coord = i + j * length + k * length * height;

				cells[coord].velocity = DX::XMFLOAT3(1.0, 0.0, 0.0);
			}

	return cells;
}