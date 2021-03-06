// bc_adiabatic.cxx

#include "../../../lib/util/source/useful.h"
#include "../../../lib/gas/models/gas_data.hh"
#include "../../../lib/gas/models/gas-model.hh"
#include "../../../lib/gas/models/physical_constants.hh"
#include "block.hh"
#include "kernel.hh"
#include "bc.hh"
#include "bc_adiabatic.hh"
#include "bc_menter_correction.hh"

//------------------------------------------------------------------------

AdiabaticBC::AdiabaticBC(Block *bdp, int which_boundary, double _emissivity)
    : BoundaryCondition(bdp, which_boundary, ADIABATIC)
{
    is_wall_flag = true;
    emissivity = _emissivity;
}

AdiabaticBC::AdiabaticBC(const AdiabaticBC &bc)
    : BoundaryCondition(bc.bdp, bc.which_boundary, bc.type_code)
{
    is_wall_flag = bc.is_wall_flag;
    emissivity = bc.emissivity;
}

AdiabaticBC::AdiabaticBC()
    : BoundaryCondition(0, 0, ADIABATIC)
{
    is_wall_flag = true;
    emissivity = 1.0;
}

AdiabaticBC&
AdiabaticBC::operator=(const AdiabaticBC &bc)
{
    BoundaryCondition::operator=(bc);
    return *this;
}

AdiabaticBC::~AdiabaticBC() {}

int AdiabaticBC::apply_viscous(double t)
// Notes:
// We make the wall non-catalytic to ensure no heat transfer.
// This is not strictly correct to set the species here,
// rather qx and qy should be set to 0, however,
// it gives the identical effect on flow variables.
//
// Menter's slightly-rough-surface boundary condition as described
// in Wilcox 2006 text, eqn 7.36.
// We assume that the y2 in eqn 7.16 is the same as
// the height of our finite-volume cell.
{
    size_t i, j, k;
    FV_Cell *cell;
    FV_Interface *IFace;
    Block & bd = *bdp;
    global_data &G = *get_global_data_ptr();     

    switch ( which_boundary ) {
    case NORTH:
	j = bd.jmax;
        for (k = bd.kmin; k <= bd.kmax; ++k) {
	    for (i = bd.imin; i <= bd.imax; ++i) {
		cell = bd.get_cell(i,j,k);
		IFace = cell->iface[NORTH];
		FlowState &fs = *(IFace->fs);
		fs.copy_values_from(*(cell->fs));
		fs.vel.x = 0.0; fs.vel.y = 0.0; fs.vel.z = 0.0;
		if ( G.moving_grid ) {
		    IFace->ivel.transform_to_local(IFace->n, IFace->t1, IFace->t2);
		    fs.vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);		    
		    fs.vel.y = IFace->ivel.y;
		    fs.vel.z = IFace->ivel.z;
                    fs.vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);		    
                    IFace->ivel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
                }			
		fs.tke = 0.0;
		fs.omega = ideal_omega_at_wall(cell);
	    } // end i loop
	} // end for k
	break;
    case EAST:
	i = bd.imax;
        for (k = bd.kmin; k <= bd.kmax; ++k) {
	    for (j = bd.jmin; j <= bd.jmax; ++j) {
		cell = bd.get_cell(i,j,k);
		IFace = cell->iface[EAST];
		FlowState &fs = *(IFace->fs);
		fs.copy_values_from(*(cell->fs));
		fs.vel.x = 0.0; fs.vel.y = 0.0; fs.vel.z = 0.0;
		if ( G.moving_grid ) {
		    IFace->ivel.transform_to_local(IFace->n, IFace->t1, IFace->t2);
		    fs.vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);		    
		    fs.vel.y = IFace->ivel.y;
		    fs.vel.z = IFace->ivel.z;
                    fs.vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);		    
                    IFace->ivel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
                }		
		fs.tke = 0.0;
		fs.omega = ideal_omega_at_wall(cell);
	    } // end j loop
	} // end for k
	break;
    case SOUTH:
	j = bd.jmin;
        for (k = bd.kmin; k <= bd.kmax; ++k) {
	    for (i = bd.imin; i <= bd.imax; ++i) {
		cell = bd.get_cell(i,j,k);
		IFace = cell->iface[SOUTH];
		FlowState &fs = *(IFace->fs);
		fs.copy_values_from(*(cell->fs));
		fs.vel.x = 0.0; fs.vel.y = 0.0; fs.vel.z = 0.0;
		if ( G.moving_grid ) {
		    IFace->ivel.transform_to_local(IFace->n, IFace->t1, IFace->t2);
		    fs.vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);		    
		    fs.vel.y = IFace->ivel.y;
		    fs.vel.z = IFace->ivel.z;
                    fs.vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);		    
                    IFace->ivel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
                }			
		fs.tke = 0.0;
		fs.omega = ideal_omega_at_wall(cell);
	    } // end i loop
	} // end for k
	break;
    case WEST:
	i = bd.imin;
        for (k = bd.kmin; k <= bd.kmax; ++k) {
	    for (j = bd.jmin; j <= bd.jmax; ++j) {
		cell = bd.get_cell(i,j,k);
		IFace = cell->iface[WEST];
		FlowState &fs = *(IFace->fs);
		fs.copy_values_from(*(cell->fs));
		fs.vel.x = 0.0; fs.vel.y = 0.0; fs.vel.z = 0.0;
		if ( G.moving_grid ) {
		    IFace->ivel.transform_to_local(IFace->n, IFace->t1, IFace->t2);
		    fs.vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);		    
		    fs.vel.y = IFace->ivel.y;
		    fs.vel.z = IFace->ivel.z;
                    fs.vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);		    
                    IFace->ivel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
                }			
		fs.tke = 0.0;
		fs.omega = ideal_omega_at_wall(cell);
	    } // end j loop
	} // end for k
 	break;
    case TOP:
	k = bd.kmax;
        for (i = bd.imin; i <= bd.imax; ++i) {
	    for (j = bd.jmin; j <= bd.jmax; ++j) {
		cell = bd.get_cell(i,j,k);
		IFace = cell->iface[TOP];
		FlowState &fs = *(IFace->fs);
		fs.copy_values_from(*(cell->fs));
		fs.vel.x = 0.0; fs.vel.y = 0.0; fs.vel.z = 0.0;
		if ( G.moving_grid ) {
		    IFace->ivel.transform_to_local(IFace->n, IFace->t1, IFace->t2);
		    fs.vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);		    
		    fs.vel.y = IFace->ivel.y;
		    fs.vel.z = IFace->ivel.z;
                    fs.vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);		    
                    IFace->ivel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
                }			
		fs.tke = 0.0;
		fs.omega = ideal_omega_at_wall(cell);
	    } // end j loop
	} // end for i
	break;
    case BOTTOM:
	k = bd.kmin;
        for (i = bd.imin; i <= bd.imax; ++i) {
	    for (j = bd.jmin; j <= bd.jmax; ++j) {
		cell = bd.get_cell(i,j,k);
		IFace = cell->iface[BOTTOM];
		FlowState &fs = *(IFace->fs);
		fs.copy_values_from(*(cell->fs));
		fs.vel.x = 0.0; fs.vel.y = 0.0; fs.vel.z = 0.0;
		if ( G.moving_grid ) {
		    IFace->ivel.transform_to_local(IFace->n, IFace->t1, IFace->t2);
		    fs.vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);		    
		    fs.vel.y = IFace->ivel.y;
		    fs.vel.z = IFace->ivel.z;
                    fs.vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);		    
                    IFace->ivel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
                }			
		fs.tke = 0.0;
		fs.omega = ideal_omega_at_wall(cell);
	    } // end j loop
	} // end for i
 	break;
    default:
	printf( "Error: apply_viscous not implemented for boundary %d\n", 
		which_boundary );
	return NOT_IMPLEMENTED_ERROR;
    }
    return SUCCESS;
} // end AdiabaticBC::apply_viscous()

