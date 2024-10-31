#include <chrono>
#include <cassert>
#include "ida_helper.h"

using namespace std;
using namespace std::chrono;

#define PRINT 2
#define NO_INTERPOLATE
#define SWAP

const int N = 2;
const int Nsteps = 100;
const double h = 0.05;

struct UserData
{
    UserData() {}
    double res = 0.0; //Time accumulator for profiling residual
};

int residualFunction(double t, N_Vector yy, N_Vector yp, N_Vector rr, void *userData)
{
    auto ra = steady_clock::now();
    
    double *yval =  N_VGetArrayPointer_Serial(yy);
    double *ypval = N_VGetArrayPointer_Serial(yp);
    double *rval =  N_VGetArrayPointer_Serial(rr);
    
    rval[0] = 10*sin(yval[1])-ypval[0];
    rval[1] = -yval[1]-ypval[1];
    
    auto rb = steady_clock::now();
    auto data = reinterpret_cast<UserData*>(userData);
    data->res += duration_cast<duration<double>>(rb-ra).count();

    return 0;
}

int jacobianFunction(double t, double alpha, N_Vector yy, N_Vector yp, N_Vector rr, SUNMatrix jac, void *userData, N_Vector temp1, N_Vector temp2, N_Vector temp3)
{
    auto ra = steady_clock::now();
    
    double *yval =  N_VGetArrayPointer_Serial(yy);
    double *ypval = N_VGetArrayPointer_Serial(yp);
    double *rval =  N_VGetArrayPointer_Serial(rr);
    double *jacval = SUNDenseMatrix_Data(jac);
    
    jacval[0] = -alpha;
    jacval[1] = 10*cos(yval[1]);
    jacval[2] = 0;
    jacval[3] = -1-alpha;
#ifdef SWAP
    swap(jacval[0],jacval[1]);
    swap(jacval[2],jacval[3]);
#endif
    
    auto rb = steady_clock::now();
    auto data = reinterpret_cast<UserData*>(userData);
    data->res += duration_cast<duration<double>>(rb-ra).count();

    return 0;
}

int main(int argc, char *argv[])
{
    auto ta = steady_clock::now();
    double ida = 0.0;  //Time accumulator for profiling IDASolve
    
    SUNContext ctx=nullptr;
    SUNContext_Create(nullptr,&ctx);
    N_Vector yy = NC(N_VNew_Serial(N,ctx));
    N_Vector yp = NC(N_VNew_Serial(N,ctx));
    double *yval = N_VGetArrayPointer_Serial(yy);
    yval[0] = 0;
    yval[1] = 10;
#ifdef SWAP
    swap(yval[0],yval[1]);
#endif
    double *ypval = N_VGetArrayPointer_Serial(yp);
    ypval[0] = 10*sin(10);
    ypval[1] = -10;
#ifdef SWAP
    swap(ypval[0],ypval[1]);
#endif
    
    void *mem=NC(IDACreate(ctx));
    double t0 = 0.0;
    RC(IDAInit(mem, residualFunction, t0, yy, yp));
    
    double rtol = 1.0e-6;
    double abstol = 1.0e-6;
    RC(IDASStolerances(mem, rtol, abstol));
    
    auto jac = NC(SUNDenseMatrix(N, N, ctx));
    //auto LS = NC(SUNLinSol_KLU(yy, jac, ctx));
    auto LS = NC(SUNLinSol_Dense(yy, jac, ctx));
    RC(IDASetLinearSolver(mem, LS, jac));
    RC(IDASetJacFn(mem, jacobianFunction));
    
    UserData *data = new UserData;
    RC(IDASetUserData(mem, data));
    
    // RC(IDACalcIC(mem, IDA_YA_YDP_INIT, h));

    FILE *fh=fopen("log.csv","w");
    for(double i = t0 + h; i <= h * Nsteps;)
    {
        auto ia = steady_clock::now();
        #ifdef NO_INTERPOLATE
        RC(IDASolve(mem, i, &i, yy, yp, IDA_ONE_STEP));
        double t = i;
        #else //NO_INTERPOLATE
        double tret;
        RC(IDASolve(mem, i, &tret, yy, yp, IDA_NORMAL));
        assert(i==tret);
        double t = i;
        i = i + h;
        #endif //NO_INTERPOLATE
        auto ib = steady_clock::now();
        ida += duration_cast<duration<double>>(ib-ia).count();
        
        #ifdef PRINT
        fprintf(fh,"%e,",t);
        double *yval = N_VGetArrayPointer_Serial(yy);
        for(int i = 0; i < min(N,PRINT); i++) fprintf(fh,"%e,",yval[i]);
        fprintf(fh,"\n");
        #endif //PRINT
        
    }
    
    printFinalStats(mem,Solver::DLS);
    IDAFree(&mem);
    N_VDestroy_Serial(yy);
    N_VDestroy_Serial(yp);
    delete data;

    auto tb = steady_clock::now();
    auto elapsed = duration_cast<duration<double>>(tb-ta).count();
    printf("Residual computation time = %fs [%f%%]\n"
           "Time spent in IDA solver  = %fs [%f%%]\n"
           "Total execution time      = %fs\n",
           data->res,(data->res/elapsed)*100.0,
           ida-data->res,((ida-data->res)/elapsed)*100.0,
           elapsed);

    fclose(fh);
}
