// bc_ablating.hh

#include "bc.hh"

class AblatingBC : public BoundaryCondition, public ZeroSystem {
public:
    double Twall;
private:
    std::vector<double> mdot;
    std::vector<double> TProfile;
    unsigned int ncell_for_profile;
    double mdot_total;
    Gas_model *gmodel;
    Gas_data *Q;
    size_t u0_index;
    size_t rho_index;
    size_t max_iterations;
    double tol;
    std::vector<double> cell_massf;
    // double cell_rho; unused, I believe.
    double cell_un;
    double cell_mass_flux;
    double cell_momentum_flux;
    double nsp;
    Vector3 cell_local_vel;
    
    ZeroFinder * zero_solver;
    double f_jac;
    
    std::vector<double> y_guess;
    std::vector<double> y_out;
    std::vector<double> N;
public:
    AblatingBC();
    AblatingBC(Block *bdp, int which_boundary, double Twall, double emissivity);
    AblatingBC(const AblatingBC &bc);
    AblatingBC & operator=(const AblatingBC &bc);
    virtual ~AblatingBC();
    virtual void print_info(std::string lead_in);
    virtual int apply_convective(double t);	// sets ghost cell flow conditions
    virtual int apply_viscous(double t); 	// sets wall temperature (same as FixedTBC)
private:
    int char_mass_flow();
    int create_y_guess(FV_Cell *cell1, FV_Interface *wall, FV_Cell *cell0);
    int solve_system(FV_Cell *cell1, FV_Interface *wall, FV_Cell *cell0);
// The following are for the zero system
public:
    int f(const vector<double> &y_guess, vector<double> &G);
    int Jac(const std::vector<double> &y_guess, Valmatrix &dGdy);
};
