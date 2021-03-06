/// \file one_d_interp.hh

#ifndef ONE_D_INTERP_HH
#define ONE_D_INTERP_HH

#include "cell.hh"

enum thermo_interp_t {INTERP_PT, INTERP_RHOE, INTERP_RHOP, INTERP_RHOT};
thermo_interp_t set_thermo_interpolator(thermo_interp_t interp);
thermo_interp_t get_thermo_interpolator();
std::string get_thermo_interpolator_name(thermo_interp_t interp);
bool set_apply_limiter_flag(bool bflag);
bool get_apply_limiter_flag();
bool set_extrema_clipping_flag(bool bflag);
bool get_extrema_clipping_flag();
double set_overshoot_factor(double value);
double get_overshoot_factor();
bool set_interpolate_in_local_frame_flag(bool bflag);
bool get_interpolate_in_local_frame_flag();

int one_d_interp_both(const FV_Interface &IFace,
		      const FV_Cell &cL1, const FV_Cell &cL0, 
		      const FV_Cell &cR0, const FV_Cell &cR1, 
		      double cL1Length, double cL0Length, 
		      double cR0Length, double cR1Length, 
		      FlowState &Lft, FlowState &Rght);

int one_d_interp_left(const FV_Interface &IFace,
		      const FV_Cell &cL1, const FV_Cell &cL0, const FV_Cell &cR0,
		      double cL1Length, double cL0Length, double cR0Length,
		      FlowState &Lft, FlowState &Rght);

int one_d_interp_right(const FV_Interface &IFace,
		       const FV_Cell &cL0, const FV_Cell &cR0, const FV_Cell &cR1,
		       double cL0Length, double cR0Length, double cR1Length,
		       FlowState &Lft, FlowState &Rght);

#endif
