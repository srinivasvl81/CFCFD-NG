// Author: Daniel F. Potter
// Date: 21-Sep-2009
// Place: Brisbane, Queendland, AUST

#include <iostream>
#include <sstream>
#include <cmath>
#include <stdlib.h>

#include "../../util/source/useful.h"
#include "../../util/source/lua_service.hh"

#include "noneq-thermal-behaviour.hh"
#include "chemical-species-library.hh"
#include "gas-model.hh"

using namespace std;

Noneq_thermal_behaviour * new_ntb_from_file( string inFile )
{
    lua_State *L = initialise_lua_State();
    
    if( do_gzfile(L, inFile) != 0 ) {
	ostringstream ost;
	ost << "new_ntb_from_file():\n";
	ost << "Error in input file: " << inFile << endl;
	input_error(ost);
    }
    
    return new Noneq_thermal_behaviour( L );
}

Noneq_thermal_behaviour::
Noneq_thermal_behaviour(lua_State *L)
    : Thermal_behaviour_model()
{
    // 0. minimum species mass-fraction
    lua_getglobal(L, "min_massf");
    if ( lua_isnil(L, -1) ) {
    	cout << "Noneq_thermal_behaviour::Noneq_thermal_behaviour()\n";
	cout << "Setting ignore_mole_fraction to default value: " 
	     << DEFAULT_MIN_MASS_FRACTION << endl;
	min_massf_ = DEFAULT_MIN_MASS_FRACTION;
    }
    else if ( lua_isnumber(L, -1) ) {
	min_massf_ = lua_tonumber(L, -1);
	if ( min_massf_ <= 0.0 ) {
	    ostringstream ost;
	    ost << "The min_massf is set to a value less than or equal to zero: " << min_massf_ << endl;
	    ost << "This must be a value between 0 and 1.\n";
	    input_error(ost); 
	}
	if ( min_massf_ > 1.0 ) {
	    ostringstream ost;
	    ost << "The min_massf is set to a value greater than 1.0: " << min_massf_ << endl;
	    ost << "This must be a value between 0 and 1.\n";
	    input_error(ost);
	}
    }
    else {
	ostringstream ost;
	ost << "The type min_massf is not a numeric type.\n";
	ost << "This must be a value between 0 and 1.\n";
	input_error(ost);
    }
    lua_pop(L,1);
    
    // 1. Create chemical species library if it has not yet been initialised
    if ( !chemical_species_library_initialised() ) {
	if ( initialise_chemical_species_library( min_massf_, L ) ) {
	    ostringstream ost;
	    ost << "Noneq_thermal_behaviour::Noneq_thermal_behaviour():\n";
	    ost << "Failed to initialise chemical species library.\n";
	    input_error(ost);
	}
    }
    
    // 2. Make vector of species pointers (declared as generic "Chemical_species")
    for ( int isp=0; isp<get_library_nsp(); ++isp )
    	species_.push_back( get_library_species_pointer(isp) );
    
    // 3. Create thermal modes
    lua_getglobal(L, "thermal_modes");
    
    if ( !lua_istable(L, -1) ) {
	ostringstream ost;
	ost << "Noneq_thermal_behaviour::Noneq_thermal_behaviour():\n";
	ost << "Error in the declaration of thermal_modes: a table is expected.\n";
	input_error(ost);
    }
    
    int ntm = lua_objlen(L, -1);
    
    cout << "Noneq_thermal_behaviour::Noneq_thermal_behaviour() - creating "
         << ntm << " thermal modes" << endl;
    
    for ( int itm = 0; itm < ntm; ++itm ) {
	lua_rawgeti(L, -1, itm+1);
	const char* mode = luaL_checkstring(L, -1);
	lua_pop(L, 1);

	// Now bring specific thermal mode table to TOS
	lua_getglobal(L, mode);
	if ( !lua_istable(L, -1) ) {
	    ostringstream ost;
	    ost << "Noneq_thermal_behaviour::Noneq_thermal_behaviour():\n";
	    ost << "Error locating information table for mode: " << mode << endl;
	    input_error(ost);
	}

	string mode_type = get_string( L, -1, "type" );
	if ( mode_type=="constant Cv" )
	    modes_.push_back( new Constant_Cv_energy_mode( mode, species_, L ) );
	else if ( mode_type=="variable Cv" )
	    modes_.push_back( new Variable_Cv_energy_mode( mode, species_, L ) );
	else {
	    ostringstream ost;
	    ost << "Noneq_thermal_behaviour::Noneq_thermal_behaviour():\n";
	    ost << "mode type: " << mode_type << " not understood.\n";
	    input_error(ost);
	}
	
	lua_pop(L, 1);	// pop 'mode'
     }
     
     lua_pop(L, 1);	// pop 'thermal_modes'
     
     // 3. Initialise chemical equilibrium system
#    if 0
     ces_ = new Chemical_equilibrium_system( min_massf_, species_ );
#    else
     ces_ = new No_chemical_equilibrium_system( min_massf_, species_ );
#    endif

     // 4. Complete initilisation of fully coupled diatomic species
     for ( int isp=0; isp<get_library_nsp(); ++isp ) {
     	 Chemical_species * X = get_library_species_pointer(isp);
     	 if ( X->get_type().find("fully coupled diatomic")!=string::npos )
     	     dynamic_cast<Fully_coupled_diatomic_species*>(X)->set_modal_temperature_indices();
     }
}

Noneq_thermal_behaviour::
~Noneq_thermal_behaviour()
{
    // species is just a vector of pointers to the chemical species library
    clear_chemical_species_library();
    
    for ( size_t itm=0; itm<modes_.size(); ++itm )
    	delete modes_[itm];
    
    if ( ces_ ) delete ces_;
}

double
Noneq_thermal_behaviour::
s_dhdT_const_p(const Gas_data &Q, Equation_of_state *EOS_, int &status)
{
    double Cp = 0.0;
    for ( size_t isp=0; isp<species_.size(); ++isp ) {
    	if ( Q.massf[isp]>min_massf_ ) 
    	    Cp += Q.massf[isp]*species_[isp]->eval_Cp(Q);
    }
    
    status = SUCCESS;
    
    return Cp;
}

double
Noneq_thermal_behaviour::
s_dedT_const_v(const Gas_data &Q, Equation_of_state *EOS_, int &status)
{
    double Cv = 0.0;
    for ( size_t isp=0; isp<species_.size(); ++isp ) {
    	if ( Q.massf[isp]>min_massf_ ) 
    	    Cv += Q.massf[isp]*species_[isp]->eval_Cv(Q);
    }
    
    status = SUCCESS;
    
    return Cv;
}

int
Noneq_thermal_behaviour::
s_eval_energy(Gas_data &Q, Equation_of_state *EOS_)
{
    for ( size_t itm = 0; itm < modes_.size(); ++itm ) {
    	Q.e[itm] = modes_[itm]->eval_energy(Q);
    }
    // And the contribution of chemical energy (due to enthalpy of formation)
    for ( size_t isp = 0; isp < species_.size(); ++isp ) {
    	Q.e[0] += Q.massf[isp]*species_[isp]->get_h_f();
    }
    return SUCCESS;
}

int
Noneq_thermal_behaviour::
s_eval_temperature(Gas_data &Q, Equation_of_state *EOS_)
{
    // Subtract chemical energy from mode 0
    double e_save = Q.e[0];
    for ( size_t isp = 0; isp < species_.size(); ++isp ) {
    	Q.e[0] -= Q.massf[isp]*species_[isp]->get_h_f();
    }
    for ( size_t itm = 0; itm < modes_.size(); ++itm ) {
    	Q.T[itm] = modes_[itm]->eval_temperature(Q);
    }
    // Re-instate e[0]
    Q.e[0] = e_save;
    return SUCCESS;
}

double
Noneq_thermal_behaviour::
s_eval_energy_isp(const Gas_data &Q, Equation_of_state *EOS_, int isp)
{
    return species_[isp]->eval_energy(Q);
}

double
Noneq_thermal_behaviour::
s_eval_enthalpy_isp(const Gas_data &Q, Equation_of_state *EOS_, int isp)
{
    return species_[isp]->eval_enthalpy(Q);
}

double
Noneq_thermal_behaviour::
s_eval_entropy_isp(const Gas_data &Q, Equation_of_state *EOS_, int isp)
{
    return species_[isp]->eval_entropy(Q);
}

double
Noneq_thermal_behaviour::
s_eval_modal_enthalpy_isp(const Gas_data &Q, Equation_of_state *EOS_, int isp, int itm )
{
    return species_[isp]->eval_modal_enthalpy(itm,Q);
}

double
Noneq_thermal_behaviour::
s_eval_modal_Cv(Gas_data &Q, Equation_of_state *EOS_, int itm )
{
    return modes_[itm]->eval_Cv(Q);
}



