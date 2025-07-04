#include "Master.h"

GRBModel createModel(GRBEnv& env)
{
    return GRBModel(env);
}

Master::Master(Data* data, GRBEnv& env) : model(createModel(env)){

    model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set(GRB_IntParam_Method, 1);
    A = std::vector<std::vector<bool>>(data->quantItems, vector<bool>(data->quantItems, 0));
    
    //modelo inicial (identidade)

    for(size_t i = 0; i < data->quantItems; i++)
    {
        std::string name = "λ";
        name += std::to_string(i);
        lambdas.push_back(model.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS, name));
        A[i][i] = true;
    }
    for(size_t i = 0; i < data->quantItems; i++)
    {
        constraints.push_back(model.addConstr(lambdas[i] >= 1.0));
    }

}

