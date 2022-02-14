#pragma once

void vtkAscii(int x, int y, int z, unsigned int size, const Data* data, const char* filename, int count);

void vtkBinary(const int x, const int y, const int z, unsigned int size, const Data* data, const char* filename, int count);