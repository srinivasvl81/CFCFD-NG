/** \file post_shock_flow.hh
 *
 *  \brief Declarations for the post-shock flow classes.
 *
 *  \author Rowan J. Gollan
 *  \verison 07-Jan-07 : ported from Python version
 *
 **/

#ifndef POST_SHOCK_FLOW_HH
#define POST_SHOCK_FLOW_HH

#include <string>
#include <fstream>


#include "../../../lib/nm/source/zero_finders.hh"
#include "../../../lib/nm/source/ode_solver.hh"
#include "../../../lib/nm/source/ode_system.hh"
#include "../../../lib/nm/source/no_fuss_linear_algebra.hh"

#include "../../../lib/gas/models/gas-model.hh"
#include "../../../lib/gas/kinetics/reaction-update.hh"
#include "../../../lib/gas/kinetics/energy-exchange-update.hh"

#include "poshax_radiation_transport.hh"
#include "conservation_systems.hh"
#include "flow_state.hh"

double p2_p1(double M1, double g);
double r2_r1(double M1, double g);
double T2_T1(double M1, double g);
double u2_u1(double M1, double g);

class Post_shock_flow {
public:
    Post_shock_flow(Flow_state &ic, Gas_model * gm, Reaction_update * ru, 
    		    Energy_exchange_update * eeu,
    		    PoshaxRadiationTransportModel * rtm,
    		    bool apply_udpedx);

    virtual ~Post_shock_flow();

public:
    virtual double increment_in_space(double x, double delta_x) = 0;
    
    void write_reaction_rates_to_file( double x, std::ofstream &oss );

public:
    Flow_state psflow;
    Flow_state icflow;

    double dpe_dx;

protected:
    std::string type;

    Gas_model * gmodel_;
    Reaction_update * rupdate_;
    Energy_exchange_update * eeupdate_;
    PoshaxRadiationTransportModel * rtmodel_;
    
    std::vector<double> yout_;
    std::vector<double> yguess_;

    NewtonRaphsonZF zero_solver_;

    bool apply_udpedx_;
};

class Loosely_coupled_post_shock_flow : public Post_shock_flow {
public:
    Loosely_coupled_post_shock_flow(Flow_state &ic, Gas_model * gm,
    	                            Reaction_update * ru, 
    		                    Energy_exchange_update * eeu,
    		                    PoshaxRadiationTransportModel * rtm,
    		                    bool apply_udpedx);

    ~Loosely_coupled_post_shock_flow();
    
public:
    double increment_in_space(double x, double delta_x); 
    
private:   
    double ode_solve(double x, double delta_x);
    void zero_solve();
    
private:
    double dt_suggest_;
    
    FrozenConservationSystem con_sys_;
};

class Fully_coupled_post_shock_flow : public Post_shock_flow, public OdeSystem {
public:
    Fully_coupled_post_shock_flow(Flow_state &ic, Gas_model * gm, 
    	                          Reaction_update * ru, 
    	                          Energy_exchange_update * eeu,
    		                  PoshaxRadiationTransportModel * rtm,
    		                    bool apply_udpedx);

    ~Fully_coupled_post_shock_flow();
    
    int eval( const std::vector<double> &y, std::vector<double> &ydot );

public:
    double increment_in_space(double x, double delta_x);
    
private:
    double ode_solve(double x, double delta_x);
    void zero_solve( const std::vector<double> &A );
    
    int nsp_;
    int ntm_;

    std::vector<double> dcdt_;
    std::vector<double> dedt_;
    
    std::vector<double> yin_;

    OdeSolver ode_solver_;
    OdeSystem * ode_sys_ptr_;
    
    NoneqConservationSystem con_sys_;
};

#endif
