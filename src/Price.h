#pragma once

#include "Data.h"
#include <iostream>
#include <vector>

#define bigM 1000000
#define EPSILON 0.00000001

using namespace std;

class Price{

    private:         
    public:
        Data* data;  
        GRBModel model;
        std::vector<GRBVar> x;
        std::vector<GRBConstr> constraints;

        Price(Data* data, GRBEnv& env);

};