#pragma once

void vtkAscii(int x, int y, int z, const Cell* data, const char* filename, int count);

void vtkBinary(const int x, const int y, const int z, const Cell* data, const char* filename, int saves);