/** \file post_shock_flow.cxx
 *
 *  \brief Defintions for the post-shock flow classes.
 *
 *  \author Rowan J. Gollan, Daniel F. Potter
 *  \version 07-Jan-07 : ported from Python version
 *           05-Jan-2012 : Loose and full coupling versions
 *
 **/

#include <iostream>
#include <iomanip>

#include <string>
#include <vector>
#include <sstream>

#include "../../../lib/gas/models/gas_data.hh"
#include "../../../lib/nm/source/no_fuss_linear_algebra.hh"
#include "../../../lib/util/source/useful.h"
#include "post_shock_flow.hh"

using namespace std;

double p2_p1(double M1, double g)
{
    return 1.0 + 2.0 * g / (g + 1.0) * (M1*M1 - 1.0);
}

double r2_r1(double M1, double g)
{
    double numer = (g + 1.0) * M1*M1;
    double denom = 2.0 + (g - 1.0) * M1*M1;
    return numer / denom;
}

double T2_T1(double M1, double g)
{
    return  p2_p1(M1, g) / r2_r1(M1, g);
}

double u2_u1(double M1, double g)
{
    return 1.0 / r2_r1(M1, g);
}

Post_shock_flow::
Post_shock_flow( Flow_state &ic, Gas_model * gm, Reaction_update * ru, 
    		 Energy_exchange_update * eeu,
    		 PoshaxRadiationTransportModel * rtm,
    		 bool apply_udpedx )
: gmodel_( gm ), rupdate_( ru ), eeupdate_( eeu ), rtmodel_( rtm ),
  apply_udpedx_( apply_udpedx )
{
    // Set icflow
    icflow.set_flow_state(*ic.Q,ic.u);
    
    // make a new gas-data structure with the initial conditions for the zero solver
    Gas_data Q(*ic.Q);
    
    //    Make some guesses about the flow behind shock
    //    based on normal shock relations (for ideal gas)
    //    then use Newton iterations for the the post-shock state.
    int status;
    double T_inf = icflow.Q->T[0];
    double rho_inf = icflow.Q->rho;
    double u_inf = icflow.u;
    double M_inf = u_inf / icflow.Q->a;
    double gamma = gmodel_->gamma(*icflow.Q,status);
    double T2 = T_inf * T2_T1(M_inf, gamma);
    double rho2 = rho_inf * r2_r1(M_inf, gamma);
    double u2 = u_inf * u2_u1(M_inf, gamma);
    
    vector<double> yguess(3), yout(3);
    yguess[0] = rho2; yguess[1] = T2; yguess[2] = u2;
    
    FrozenConservationSystem con_sys(gm,&Q,u_inf);
    zero_solver_.set_constants(3, 1.0e-6, 100, true);
    zero_solver_.solve(con_sys, yguess, yout);
    
    Q.rho = yout[0];
    Q.T[0] = yout[1];
    gmodel_->eval_thermo_state_rhoT( Q );
    double u_s = yout[2];

    psflow.set_flow_state(Q,u_s);

    // Initialise the electron pressure gradient
    dpe_dx = 0.0;
}

Post_shock_flow::
~Post_shock_flow() {}

void
Post_shock_flow::
write_reaction_rates_to_file( double x, ofstream &ofile )
{
    vector<double> w_f, w_b;
    rupdate_->get_directional_rates(w_f,w_b);
    ofile << setw(10) << x;
    for ( size_t ir=0; ir<w_f.size(); ++ir ) {
        ofile << setw(20) << w_f[ir] - w_b[ir];
    }
    ofile << endl;
}

/******************************************************************************/

Loosely_coupled_post_shock_flow::
Loosely_coupled_post_shock_flow( Flow_state &ic, Gas_model * gm, Reaction_update * ru, 
    		                 Energy_exchange_update * eeu,
    		                 PoshaxRadiationTransportModel * rtm,
    		                 bool apply_udpedx )
: Post_shock_flow( ic, gm, ru, eeu, rtm, apply_udpedx )
{
    yout_.resize( 3 );
    yguess_.resize( 3 );    
    
    con_sys_.initialise( gmodel_, psflow.Q, psflow.u );
}

Loosely_coupled_post_shock_flow::
~Loosely_coupled_post_shock_flow() {}

double 
Loosely_coupled_post_shock_flow::
increment_in_space(double x, double delta_x)
{
    // 0. Save the initial electron pressure
    double p_e0 = psflow.Q->p_e;

    // 1. Integrate the ODE system in space
    double new_x = ode_solve(x, delta_x);

    // 2. Solve for the flow state
    zero_solve();

    // 3. Store the electron pressure gradient
    dpe_dx = ( psflow.Q->p_e - p_e0 ) / delta_x;

    return new_x;
}

double 
Loosely_coupled_post_shock_flow::
ode_solve(double x, double delta_x)
{
    double dx = delta_x;
    double dt = dx/psflow.u;
    double dt_suggest = dt;

    // Apply reaction update if it is present
    if ( rupdate_ ) {
    	rupdate_->update_state(*psflow.Q, dt, dt_suggest, gmodel_);
	gmodel_->eval_thermo_state_rhoe(*psflow.Q);
    	// dt_new = dt_suggest;
    }
    
    // Apply energy exchange update if it is present
    if ( eeupdate_ ) {
    	eeupdate_->update_state( *psflow.Q, dt, dt_suggest, gmodel_ );
    	gmodel_->eval_thermo_state_rhoe( *psflow.Q );
    	// if ( dt_suggest < dt_new ) dt_new = dt_suggest;
    }
    
    // Apply radiative source term if it is present
    // NOTE:  - assuming the electronic mode is at the back of the vector
    //        - now also allowing the fraction of div(q) applied to the electronic
    //          mode to be other than 1.0 (this allows for static specific models where
    //          a factor of zero will often be used)
    if ( rtmodel_ ) {
    	psflow.Q_rad = rtmodel_->eval_Q_rad( *psflow.Q );
    	double delta_Q_rad_kg = ( psflow.Q_rad / psflow.Q->rho ) * dt;
	if ( gmodel_->get_number_of_modes() == 1 ) {
	    psflow.Q->e[0] += delta_Q_rad_kg;
	}
	else {
    	    psflow.Q->e.back() += delta_Q_rad_kg * rtmodel_->get_electronic_mode_factor();
	}
    	gmodel_->eval_thermo_state_rhoe(*psflow.Q);

    	// Update total energy in the conservation system
    	con_sys_.set_C( con_sys_.get_C() + con_sys_.get_A() * delta_Q_rad_kg );
    }

    // Apply work on electrons due to presence of electric field ( u dpe/dx )
    if ( apply_udpedx_ ) {
        psflow.Q->e.back() += ( psflow.u * dpe_dx / psflow.Q->rho ) * dt;
        gmodel_->eval_thermo_state_rhoe(*psflow.Q);
    }

    return dt_suggest*psflow.u;
}

void
Loosely_coupled_post_shock_flow::
zero_solve()
{
    //    cout << "ENTERING ZERO_SOLVE:\n";
    //    psflow.Q->print_values();
    // 1. Create a guess
    yguess_[0] = psflow.Q->rho;
    yguess_[1] = psflow.Q->T[0];
    yguess_[2] = psflow.u;

    // 2. Solve
    zero_solver_.solve(con_sys_, yguess_, yout_);

    // 3. Map
    psflow.Q->rho = yout_[0];
    psflow.Q->T[0] = yout_[1];
    psflow.u = yout_[2];
    
    // 4. Equation of state evaluation
    gmodel_->eval_thermo_state_rhoT(*psflow.Q);
    
    return;
}

/*************************************************************************/

Fully_coupled_post_shock_flow::
Fully_coupled_post_shock_flow( Flow_state &ic, Gas_model * gm, Reaction_update * ru, 
    		               Energy_exchange_update * eeu,
    		               PoshaxRadiationTransportModel * rtm,
    	                       bool apply_udpedx )
: Post_shock_flow( ic, gm, ru, eeu, rtm, apply_udpedx )
{
    // gas-model dimensions
    nsp_ = gmodel_->get_number_of_species();
    ntm_ = gmodel_->get_number_of_modes();
    
    // Initialise the ODE solver pieces
    dcdt_.resize( nsp_, 0.0 );
    dedt_.resize( ntm_, 0.0 );
    int ndim = nsp_ + 1 + ntm_;
    yin_.resize( ndim, 0.0 );
    yout_.resize( ndim, 0.0 );
    yguess_.resize( ndim, 0.0 );
    ode_solver_.set_constants( "poshax3 noneq ODE system", ndim,
    	                       "rkf", 20, 1.15, 1.0e-2, 0.333 );
    ode_sys_ptr_ = dynamic_cast<OdeSystem*>(this);
    
    // Initialise the zero solver and conservation system
    zero_solver_.set_constants(ndim, 1.0e-6, 100, true);
    con_sys_.initialise( gmodel_, psflow.Q, psflow.u );
}

Fully_coupled_post_shock_flow::
~Fully_coupled_post_shock_flow() {}

double 
Fully_coupled_post_shock_flow::
increment_in_space(double x, double delta_x)
{
    // 0. Save the initial electron pressure
    double p_e0 = psflow.Q->p_e;

    // 1. Integrate the ODE system in space
    double new_x = ode_solve(x, delta_x);
    
    // 2. Solve for the flow state
    zero_solve(yout_);
    
    // 3. Store the electron pressure gradient
    dpe_dx = ( psflow.Q->p_e - p_e0 ) / delta_x;

    return new_x;
}

double 
Fully_coupled_post_shock_flow::
ode_solve(double x, double delta_x)
{
    double dx = delta_x;
    double dx_suggest = dx;
    
    // 1.  Encode the flux quantities from the current flow-state
    con_sys_.encode_conserved( yin_, *psflow.Q, psflow.u );
    
    // 2. Submit to ODE solver
    int flag = ode_solver_.solve_over_interval( *(ode_sys_ptr_), 0.0, dx, 
    	                                        &dx_suggest, yin_, yout_ );
    
    if ( ! flag ) {
    	cout << "Post_shock_flow::ode_solve()" << endl
    	     << "The ODE solver failed with the following flow-state:" << endl
    	     << psflow.str( bool(rtmodel_) ) << endl
    	     << "Bailing out!" << endl;
    	exit( FAILURE );
    }
    
    return dx_suggest;
}

void
Fully_coupled_post_shock_flow::
zero_solve( const vector<double> &A )
{
    // 0. Set constants in the conservation system
    con_sys_.set_constants( A );
    
    // 1. Create a guess
    for ( int isp=0; isp<nsp_; ++isp )
    	yguess_[isp] = psflow.Q->rho * psflow.Q->massf[isp];
    for ( int itm=0; itm<ntm_; ++itm )
    	yguess_[nsp_+itm] = psflow.Q->T[itm];
    yguess_[nsp_+ntm_] = psflow.u;
    
    // 2. Solve
    zero_solver_.solve(con_sys_, yguess_, yout_);
    
    // 3. Map
    psflow.Q->rho = 0.0;
    for ( int isp=0; isp<nsp_; ++isp ) {
    	if ( yout_[isp] < 0.0 ) yout_[isp] = 0.0;
    	psflow.Q->rho += yout_[isp];
    }
    for ( int isp=0; isp<nsp_; ++isp )
    	psflow.Q->massf[isp] = yout_[isp] / psflow.Q->rho;
    for ( int itm=0; itm<ntm_; ++itm )
    	psflow.Q->T[itm] = yout_[nsp_+itm];
    psflow.u = yout_[nsp_+ntm_];

    // 4. Rescale the mass-fractions to sum to one
    scale_mass_fractions( psflow.Q->massf );
    
    // 5. Equation of state evaluation
    gmodel_->eval_thermo_state_rhoT(*psflow.Q);
}

int
Fully_coupled_post_shock_flow::
eval( const vector<double> &y, vector<double> &ydot )
{
    // NOTE: may need to omit this step if its too slow
    // 1. Evaluate the flow state from the y vector
    zero_solve( y );
    
    // 2. Fill out the ydot vector
    int iy=0;
    // 2a. Species mass production
    if ( rupdate_ ) rupdate_->rate_of_change( *psflow.Q, dcdt_ );
    for ( int isp=0; isp<nsp_; ++isp ) {
	ydot[iy] = dcdt_[isp] * gmodel_->molecular_weight(isp);
	++iy;
    }
    // 2b. Total momentum
    ydot[iy] = 0.0;
    ++iy;
    // 2c. Total energy
    psflow.Q_rad = 0.0;
    if ( rtmodel_ ) psflow.Q_rad = rtmodel_->eval_Q_rad( *psflow.Q );
    ydot[iy] = psflow.Q_rad;
    ++iy;
    // 2d. Modal energies
    if ( eeupdate_ ) eeupdate_->rate_of_change( *psflow.Q, dedt_ );
    if ( rupdate_ )
    	rupdate_->eval_chemistry_energy_coupling_source_terms( *psflow.Q, dedt_ );
    for ( int itm=1; itm<ntm_; ++itm ) {
    	ydot[iy] = dedt_[itm] * psflow.Q->rho;
    	++iy;
    }
    ydot[iy-1] += psflow.Q_rad;
    if ( apply_udpedx_ )
        ydot[iy-1] += psflow.u * dpe_dx;
    
    return 0;
}

