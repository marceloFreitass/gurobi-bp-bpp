
#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <stack>
#include <math.h>
#include "gurobi_c++.h"

#include "Data.h"
#include "Master.h"
#include "Price.h"

using namespace std;

typedef struct
{
    std::vector<pair<int, int>> together;
    std::vector<pair<int, int>> separate;
}Node;

class BP{

    private:
        Data* data;
        GRBEnv env_master;
        GRBEnv env_price;
        Master mp;
        Price pp;
    public:

    
    BP(Data* data);
    std::pair<int, int> get_most_fractional();
    double solve_gc();
    double solve_gc_mink();
    void solve_bp();
    void reset(vector<size_t>& disabled);
    void print_integer_solution();
};