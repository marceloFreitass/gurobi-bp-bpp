#include "Data.h"
#include "BP.h"


int main(int argc, char** argv){

    Data data(argv[1]);
    try
    {
        BP bp(&data);
        bp.solve_bp();

    }
    catch(GRBException e)
    {
        std::cout << "ERRO: " << e.getMessage() << std::endl;
    }

}