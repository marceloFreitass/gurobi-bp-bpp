#pragma once

#include "Data.h"
#include <iostream>
#include <vector>

#define bigM 1000000
#define EPSILON 0.00000001

using namespace std;

class Master{

    private:


        Data* data;        

    public:

        GRBModel model;
        std::vector<GRBVar> lambdas;
        std::vector<GRBConstr> constraints;

        vector<vector<bool>> A; // 1 se o item j ta no padrao i


        Master(Data* data, GRBEnv& env);

};