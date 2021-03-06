// Author: Daniel F. Potter
// Date: 18-Nov-2009

#include <iostream>
#include <sstream>
#include <numeric>
#include <math.h>
#include "../../util/source/lua_service.hh"
#include "../../util/source/useful.h"
#include "../../nm/source/no_fuss_linear_algebra.hh"

#include "energy-exchange-system.hh"
#include "energy-exchange-rate.hh"

using namespace std;

Energy_exchange_system::
Energy_exchange_system(lua_State *L, Gas_model &g, double error_tol)
    : OdeSystem(g.get_number_of_modes() - 1, true), g_(&g), err_tol_(error_tol)
{
    lua_getglobal(L, "rates");
    if ( !lua_istable(L, -1) ) {
	ostringstream ost;
	ost << "Energy_exchange_system::Energy_exchange_system()\n";
	ost << "Error interpreting 'rates'; a table of rates is expected.\n";
	input_error(ost);
    }
    int nrates = lua_objlen(L, -1);
    for ( size_t i = 1; i <= (size_t) nrates; ++i ) {
	lua_rawgeti(L, -1, i);
	ee_rate_.push_back(create_energy_exchange_rate(L));
	lua_pop(L, 1);
    }
    lua_pop(L, 1);
    
    int ndim = g_->get_number_of_modes() - 1;
    ydot_.resize(ndim);

    UNUSED_VARIABLE(err_tol_);
}

Energy_exchange_system::
~Energy_exchange_system()
{
    for ( size_t i = 0; i < ee_rate_.size(); ++i ) {
	delete ee_rate_[i];
    }
}

int
Energy_exchange_system::
eval(const vector<double> &y, vector<double> &ydot)
{
    // NOTE: updating the gas-data structure here as many mechanisms
    //       use temperature to evaluate the rate of energy exchange

    double e_total = accumulate(Q_->e.begin(), Q_->e.end(), 0.0);
    double e_other = 0.0;
    // 1. Update the gas-data structure based on the given y array
    for ( size_t itm = 1; itm < Q_->e.size(); ++itm ) {
        Q_->e[itm] = y[itm-1];
	e_other += Q_->e[itm];
    }
    Q_->e[0] = e_total - e_other;
    g_->eval_thermo_state_rhoe(*Q_); 
    
    if( ! called_at_least_once ) {
	for( size_t i = 0; i < ee_rate_.size(); ++i ) {
	    ee_rate_[i]->compute_all_relaxation_times(*Q_,*molef_);
	    
	}
	called_at_least_once = true;
    }

    for( size_t i = 0; i < ee_rate_.size(); ++i ) {
	ydot[i] = ee_rate_[i]->compute_rate(y, *Q_, *molef_);
    }
    
    return SUCCESS;
}

const double eps1 = 0.001;
const double therm_step_upper_limit = 1.0e-3;
const double therm_step_lower_limit = 1.0e-20;
const double zero_tol = 1.0e-30;

double
Energy_exchange_system::
stepsize_select(const vector<double> &y)
{
    eval(y, ydot_);
    double min_dt = therm_step_upper_limit; // to get us started
    double old_dt = 0.0;

    for ( size_t i = 0; i < y.size(); ++i ) {
	if( (y[i] > 0.0) && (fabs(ydot_[i]) > zero_tol) ) {
	    old_dt = fabs( y[i] / ydot_[i] );
	    if( old_dt < min_dt ) {
		min_dt = old_dt;
	    }
	}
    }

    double dt_therm = eps1 * min_dt;

    // Impose upper and lower chem_step limits
    if( dt_therm > therm_step_upper_limit )
	dt_therm = therm_step_upper_limit;

    if ( dt_therm < therm_step_lower_limit )
	dt_therm = therm_step_lower_limit;

    return dt_therm;
}

