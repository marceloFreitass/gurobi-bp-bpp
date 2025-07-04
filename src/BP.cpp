#include "BP.h"
#include "combo.c"

BP::BP(Data* data) : env_master(), env_price(), mp(data, env_master), pp(data, env_price){
    this->data = data;
}

void BP::print_integer_solution()
{
    size_t count = 0;
    for(size_t p = 0; p < mp.A.size(); p++)
    {
        if(fabs(mp.lambdas[p].get(GRB_DoubleAttr_X)  - 1.0) < EPSILON)
        {
            count++;
            std::cout <<  "CAIXA " << count << ": ";
            for(size_t i = 0; i < data->quantItems; i++)
            {
                if(mp.A[p][i])
                {
                    std::cout << i << " ";
                }
            }
            std::cout << std::endl;
        }
    }
}

void BP::reset(vector<size_t>& disabled)
{
    //ativando padroes desativados
    for(size_t i = 0; i < disabled.size(); i++)
    {
        mp.lambdas[disabled[i]].set(GRB_DoubleAttr_UB, GRB_INFINITY);
    }
    disabled.clear();

    //tirando restricoes colocadas no pricing
    size_t n_constraints = pp.constraints.size();
    for(size_t i = n_constraints - 1; i >= 1; i--)
    {
        pp.model.remove(pp.constraints[i]);
        pp.constraints.erase(pp.constraints.begin() + i);
    }
}
std::pair<int, int> BP::get_most_fractional()
{
    size_t n_patterns = mp.A.size();
    int i1 = -1;
    int i2 = -1;
    double most_frac = 0.5;
    for(size_t i = 0; i < data->quantItems; i++)
    {
        for(size_t j = i + 1; j < data->quantItems; j++)
        {
            double frac = 0;
            for(size_t p = 0; p < n_patterns; p++)
            {
                if(mp.A[p][i] && mp.A[p][j])
                {
                    frac += mp.lambdas[p].get(GRB_DoubleAttr_X);
                }
            }
            if(fabs(frac - 0.5) + EPSILON < most_frac)
            {
                i1 = i;
                i2 = j;
                most_frac = fabs(frac - 0.5);
            }

        }
    }

    return {i1, i2};
    

}
double BP::solve_gc_mink()
{
    double cost;
    double sub_cost;
    item combo_data[data->quantItems];
    while(true)
    {   
        mp.model.optimize();
        cost = mp.model.get(GRB_DoubleAttr_ObjVal);
        for(size_t i = 0; i < data->quantItems; i++)
        {
            combo_data[i] = {long(bigM * mp.constraints[i].get(GRB_DoubleAttr_Pi)), data->itemWeight[i], false, int(i)};
        }

        long z = combo(combo_data, combo_data + data->quantItems - 1, data->binCapacity, 0, data->quantItems * bigM * bigM, true, false);
        sub_cost = (double)z/bigM;

        if(1 - sub_cost < -EPSILON)
        {
            GRBColumn new_col;
            std::vector<bool> new_col_vec(data->quantItems, false);
            
            for(size_t i = 0; i < data->quantItems; i++)
            {
                new_col.addTerm(combo_data[i].x, mp.constraints[combo_data[i].index]);
                if(combo_data[i].x)
                {
                    new_col_vec[combo_data[i].index] = true;
                }
            }
            mp.A.push_back(new_col_vec);
            std::string name = "λ";
            name += std::to_string(mp.A.size() - 1);

            mp.lambdas.push_back(mp.model.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS, new_col, name));
        }
        else
        {
            break;
        }
    }

    return cost;
}
double BP::solve_gc()
{

    double cost;
    double sub_cost;
    while(true)
    {   
        mp.model.optimize();
        cost = mp.model.get(GRB_DoubleAttr_ObjVal);
        for(size_t i = 0; i < data->quantItems; i++)
        {
            pp.x[i].set(GRB_DoubleAttr_Obj, mp.constraints[i].get(GRB_DoubleAttr_Pi));
        }
        pp.model.optimize();
        sub_cost = pp.model.get(GRB_DoubleAttr_ObjVal);
        if(1 - sub_cost < -EPSILON)
        {
            GRBColumn new_col;
            std::vector<bool> new_col_vec(data->quantItems, false);
            for(size_t i = 0; i < data->quantItems; i++)
            {
                new_col.addTerm(pp.x[i].get(GRB_DoubleAttr_X), mp.constraints[i]);
                if(pp.x[i].get(GRB_DoubleAttr_X))
                {
                    new_col_vec[i] = true;
                }
            }
            
            mp.A.push_back(new_col_vec);
            std::string name = "λ";
            name += std::to_string(mp.A.size() - 1);

            mp.lambdas.push_back(mp.model.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS, new_col, name));

        }
        
        else
        {
            break;
        }
    }
    return cost;
}

void BP::solve_bp()
{
    Node root = {{}, {}};
    std::stack<Node> tree;
    std::vector<size_t> disabled_patterns;
    double UB = GRB_INFINITY;
    double LB;

    //assumindo que nao vai resolver na raiz
    LB = solve_gc_mink();
    std::cout << "LB: " << LB << std::endl;
    std::pair<int , int> new_pair = get_most_fractional();
    Node new_node_1 = root;
    new_node_1.together.push_back(new_pair);
    Node new_node_2 = root;
    new_node_2.separate.push_back(new_pair);

    tree.push(new_node_1);
    tree.push(new_node_2);

    int count = 0;

    while(!tree.empty())
    {
        Node no = tree.top();
        tree.pop();

        for(size_t p = 0; p < mp.A.size(); p++)
        {
            for(size_t i = 0; i < no.together.size(); i++)
            {
                std::pair<size_t, size_t> pt = no.together[i];
                if((mp.A[p][pt.first] && !mp.A[p][pt.second]) || (!mp.A[p][pt.first] && mp.A[p][pt.second]))
                {
                    mp.lambdas[p].set(GRB_DoubleAttr_UB, 0);
                    disabled_patterns.push_back(p);
                }
            }
            for(size_t i = 0; i < no.separate.size(); i++)
            {
                std::pair<size_t, size_t> ps = no.separate[i];
                if(mp.A[p][ps.first] && mp.A[p][ps.second])
                {
                    mp.lambdas[p].set(GRB_DoubleAttr_UB, 0);
                    disabled_patterns.push_back(p);  
                }
            }

        }
        for(size_t i = 0; i < no.together.size(); i++)
        {
            std::pair<size_t, size_t> pt = no.together[i];
            pp.constraints.push_back(pp.model.addConstr(pp.x[pt.first] == pp.x[pt.second]));
        }
        for(size_t i = 0; i < no.separate.size(); i++)
        {
            std::pair<size_t, size_t> ps = no.separate[i];
            pp.constraints.push_back(pp.model.addConstr(pp.x[ps.first] + pp.x[ps.second] <= 1));   
        }
        try
        {
            LB = solve_gc();
        }
        catch(GRBException e) //inviavel
        {
            reset(disabled_patterns);
            continue;
        }
        
        std::cout << "LB: " << LB << std::endl;
        if(std::ceil(LB - EPSILON) - UB >= 0) //poda por limitate
        {
            reset(disabled_patterns);
            continue;
        }
        std::pair<int , int> new_pair = get_most_fractional();
        if(new_pair.first == -1)
        {
            std::cout << "NOVA SOLUCAO INTEIRA" << std::endl;

            if(LB + EPSILON < UB)
            {
                UB = LB;
                print_integer_solution();
            }
            reset(disabled_patterns);
            continue;
        }

        reset(disabled_patterns);

        Node new_node_1 = no;
        new_node_1.together.push_back(new_pair);
        Node new_node_2 = no;
        new_node_2.separate.push_back(new_pair);

        
        tree.push(new_node_2);
        tree.push(new_node_1);
    }
    std::cout << "OTIMO: " << UB << std::endl;
}

