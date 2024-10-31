
#pragma once

#include <cstdio>
#include <string>
#include <stdexcept>

#include <ida/ida.h>
#include <sunlinsol/sunlinsol_klu.h>
#include <sunlinsol/sunlinsol_dense.h>

#include <nvector/nvector_serial.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sundials/sundials_types.h>
#include <sundials/sundials_math.h>

template<typename T>
T* throwNull(T* ptr, const char *funcName)
{
    using namespace std;
    if(ptr==nullptr) throw runtime_error(string(funcName)+" returned nullptr");
    return ptr;
}

int throwRetval(int retval, const char *funcName)
{
    using namespace std;
    if(retval<0) throw runtime_error(string(funcName)+" returned "+to_string(retval));
    return retval;
}

#define NC(x) throwNull(x,#x)
#define RC(x) throwRetval(x,#x)

enum class Solver
{
    DLS,
    KLU
};

static void printFinalStats(void *mem, Solver solver)
{
    long int nst, nni, nje, nre, nreLS, netf, ncfn, nge;
    IDAGetNumSteps(mem, &nst);
    IDAGetNumResEvals(mem, &nre);
    switch(solver)
    {
        case Solver::DLS:
            
            IDAGetNumLinResEvals(mem, &nreLS);
            IDAGetNumJacEvals(mem, &nje);
            break;
        case Solver::KLU:
            nreLS=0;
            // IDASlsGetNumJacEvals(mem, &nje);
            break;
    }
    IDAGetNumNonlinSolvIters(mem, &nni);
    IDAGetNumErrTestFails(mem, &netf);
    IDAGetNumNonlinSolvConvFails(mem, &ncfn);
    IDAGetNumGEvals(mem, &nge);
    printf("\nFinal Run Statistics: \n\n");
    printf("Number of steps                    = %ld\n", nst);
    printf("Number of residual evaluations     = %ld\n", nre+nreLS);
    printf("Number of Jacobian evaluations     = %ld\n", nje);
    printf("Number of nonlinear iterations     = %ld\n", nni);
    printf("Number of error test failures      = %ld\n", netf);
    printf("Number of nonlinear conv. failures = %ld\n", ncfn);
    printf("Number of root fn. evaluations     = %ld\n", nge);
}
