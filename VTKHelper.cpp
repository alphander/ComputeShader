#include "Resources.h"
#include <fstream>

void vtk(int x, int y, int z, unsigned int size, const Data* data, const char* filename)
{
    std::ofstream file;
    std::remove(filename);
    file.open(filename, std::ios::out | std::ios::app | std::ios::binary);
    file.clear();

    file << "# vtk DataFile Version 2.0" << std::endl
        << "Fluid data" << std::endl;

    file << "ASCII" << std::endl;
    file << "DATASET RECTILINEAR_GRID" << std::endl;
    file << "DIMENSIONS " << x << " " << y << " " << z << std::endl;
    file << "X_COORDINATES " << x << " float" << std::endl;
    for (int i = 1; i <= x; i++)
        file << i << " ";
    file << std::endl;
    file << "Y_COORDINATES " << y << " float" << std::endl;
    for (int i = 1; i <= y; i++)
        file << i << " ";
    file << std::endl;
    file << "Z_COORDINATES " << z << " float" << std::endl;
    for (int i = 1; i <= z; i++)
        file << i << " ";
    file << std::endl;

    file << "POINT_DATA " << size << std::endl;
    file << "SCALARS density float" << std::endl;
    file << "LOOKUP_TABLE default" << std::endl;
    for (int i = 0; i < size; i++)
        file << data[i].density << std::endl;

    file << "VECTORS velocity float" << std::endl;
    for (int i = 0; i < size; i++)
    {
        file << data[i].velocity.x << " ";
        file << data[i].velocity.y << " ";
        file << data[i].velocity.z << " ";
        file << std::endl;
    }

    file.close();
}