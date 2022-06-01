#include "Resources.h"
#include <fstream>
#include <string>

using std::string;
using std::to_string;
using std::ios;

char* SwapEndian(float in)
{
    float t = 0.0f;
    char* from = (char*)&in;
    char* to = (char*) &t;

    to[0] = from[3];
    to[1] = from[2];
    to[2] = from[1];
    to[3] = from[0];

    return to;
}

void vtkAscii(const int x, const int y, const int z, const Cell* data, const char* filename, int saves)
{
    const int size = x * y * z;
    string str = filename + to_string(saves) + ".vtk";
    std::ofstream file(str, ios::binary);

    file << "# vtk DataFile Version 2.0\n";
    file << "Fluid data\n";

    file << "ASCII\n";
    file << "DATASET RECTILINEAR_GRID\n";
    file << "DIMENSIONS " << x << " " << y << " " << z << "\n";

    file << "X_COORDINATES " << x << " float\n";
    for (float i = 1; i <= x; i++)
        file << i << " ";
    file << "\n";

    file << "Y_COORDINATES " << y << " float\n";
    for (float i = 1; i <= y; i++)
        file << i << " ";
    file << "\n";

    file << "Z_COORDINATES " << z << " float\n";
    for (float i = 1; i <= z; i++)
        file << i << " ";
    file << "\n";

    file << "POINT_DATA " << size << "\n";
    file << "SCALARS concentration float\n";
    file << "LOOKUP_TABLE default\n";

    for (int i = 0; i < size; i++)
        file << data[i].concentration << "\n";

    file << "VECTORS velocity float\n";

    for (int i = 0; i < size; i++)
    {
        file << data[i].velocity.x << " ";
        file << data[i].velocity.y << " ";
        file << data[i].velocity.z << "\n";
    }

    file.close();
    cout << "Finished making ascii vtk..." << endl;
}

void vtkBinary(const int x, const int y, const int z, const Cell* data, const char* filename, int saves)
{
    const int count = x * y * z;
    string str = filename + to_string(saves) + ".vtk";
    std::ofstream file(str, ios::binary);

    file << "# vtk DataFile Version 2.0\n";
    file << "Fluid data\n";

    file << "BINARY\n";
    file << "DATASET RECTILINEAR_GRID\n";
    file << "DIMENSIONS " << x << " " << y << " " << z << endl;

    file << "X_COORDINATES " << x << " float" << endl;
    for (float i = 1; i <= x; i++)
        file.write(SwapEndian(i), sizeof(float));
    file << endl;

    file << "Y_COORDINATES " << y << " float" << endl;
    for (float i = 1; i <= y; i++)
        file.write(SwapEndian(i), sizeof(float));
    file << endl;

    file << "Z_COORDINATES " << z << " float" << endl;
    for (float i = 1; i <= z; i++)
        file.write(SwapEndian(i), sizeof(float));
    file << endl;

    file << "POINT_DATA " << count << endl;
    file << "SCALARS concentration float" << endl;
    file << "LOOKUP_TABLE default" << endl;
    for (int i = 0; i < count; i++)
        file.write(SwapEndian(data[i].concentration), sizeof(float));
    file << endl;

    file << "SCALARS divergence float" << endl;
    file << "LOOKUP_TABLE default" << endl;
    for (int i = 0; i < count; i++)
        file.write(SwapEndian(data[i].divergence), sizeof(float));
    file << endl;

    file << "VECTORS velocity float" << endl;
    for (int i = 0; i < count; i++)
    {
        file.write(SwapEndian(data[i].velocity.x), sizeof(float));
        file.write(SwapEndian(data[i].velocity.y), sizeof(float));
        file.write(SwapEndian(data[i].velocity.z), sizeof(float));
    }
    file << endl;

    file.close();
    cout << "Finished making binary vtk..." << endl;
}