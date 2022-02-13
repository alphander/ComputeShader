#include "Resources.h"
#include <fstream>
#include <string>
#include <array>
#include "intrin.h"

using std::string;
using std::array;
using std::to_string;
using std::ios;

void vtk(const int x, const int y, const int z, unsigned int size, const Data* data, const char* filename, int count)
{
    cout << "Writing to file..." << endl;
    string str = filename + to_string(count) + ".vtk";
    std::ofstream file(str, ios::binary);

    file << "# vtk DataFile Version 2.0" << endl;
    file << "Fluid data" << endl;

    file << "ASCII" << endl;
    file << "DATASET RECTILINEAR_GRID" << endl;
    file << "DIMENSIONS " << x << " " << y << " " << z << endl;

    file << "X_COORDINATES " << x << " float" << endl;
    for (float i = 1; i <= x; i++)
        file << i << " ";
    file << endl;

    file << "Y_COORDINATES " << y << " float" << endl;
    for (float i = 1; i <= y; i++)
        file << i << " ";
    file << endl;

    file << "Z_COORDINATES " << z << " float" << endl;
    for (float i = 1; i <= z; i++)
        file << i << " ";
    file << endl;

    file << "POINT_DATA " << size << endl;
    file << "SCALARS density float" << endl;
    file << "LOOKUP_TABLE default" << endl;

    for (int i = 0; i < size; i++)
        file << data[i].density << endl;

    file << "VECTORS velocity float" << endl;

    for (int i = 0; i < size; i++)
    {
        file << data[i].velocity.x << " ";
        file << data[i].velocity.y << " ";
        file << data[i].velocity.z << " ";
    }
    cout << "Finished Writing to file!" << endl;

    file.close();
}