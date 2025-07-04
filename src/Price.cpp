#include "Price.h"

GRBModel createModel_sub(GRBEnv& env)
{
    return GRBModel(env);
}

Price::Price(Data* data, GRBEnv& env) : model(createModel_sub(env)){

    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set(GRB_IntParam_Method, 1);
    data = data;

    for(size_t i = 0; i < data->quantItems; i++)
    {
        std::string name = "x";
        name += std::to_string(i);
        x.push_back(model.addVar(0.0, 1.0, 1.0, GRB_INTEGER, name));
    }
    GRBLinExpr sum_weight = 0;
    for(size_t i = 0; i < data->quantItems; i++)
    {
        sum_weight += data->itemWeight[i] * x[i];
    }
    constraints.push_back(model.addConstr(sum_weight <= data->binCapacity));

}

