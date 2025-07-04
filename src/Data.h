#include "gurobi_c++.h"

#pragma once
#include <string>
#include <fstream>
#include <vector>

using namespace std;

class Data{
    private:
    public:
        size_t quantItems;
        size_t binCapacity;
        vector<int> itemWeight;

    Data(char* instance);
};