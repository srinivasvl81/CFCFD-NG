/// \file block.cxx
/// \ingroup eilmer3
/// \brief Functions that apply to the block as a whole.
///
/// \author PJ
/// \version 23-Jun-2006 Abstracted from cns_tstp.cxx
///

#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
extern "C" {
#include <zlib.h>
}
#include "cell.hh"
#include "kernel.hh"
#include "block.hh"
#include "bc.hh"

//-----------------------------------------------------------------------------

Block::Block()
    : bcp(NI,NULL) // Let everything else default initialize.
{}

Block::Block(const Block &b)
    : id(b.id), active(b.active), omegaz(b.omegaz),
      dt_allow(b.dt_allow),
      cfl_min(b.cfl_min), cfl_max(b.cfl_max),
      mass_residual(b.mass_residual),
      energy_residual(b.energy_residual),
      mass_residual_loc(b.mass_residual_loc),
      energy_residual_loc(b.energy_residual_loc),
      hncell(b.hncell),
      hicell(b.hicell), hjcell(b.hjcell), hkcell(b.hkcell),
      nidim(b.nidim), njdim(b.njdim), nkdim(b.nkdim),
      nni(b.nni), nnj(b.nnj), nnk(b.nnk),
      imin(b.imin), imax(b.imax),
      jmin(b.jmin), jmax(b.jmax),
      kmin(b.kmin), kmax(b.kmax),
      active_cells(b.active_cells),
      baldwin_lomax_iturb(b.baldwin_lomax_iturb),
      bcp(b.bcp),
      ctr_(b.ctr_),
      ifi_(b.ifi_), ifj_(b.ifj_), ifk_(b.ifk_),
      vtx_(b.vtx_),
      sifi_(b.sifi_), sifj_(b.sifj_), sifk_(b.sifk_)
{}

Block & Block::operator=(const Block &b)
{
    if ( this != &b ) { // Avoid aliasing
	id = b.id; active = b.active; omegaz = b.omegaz;
	dt_allow = b.dt_allow;
	cfl_min = b.cfl_min; cfl_max = b.cfl_max;
	mass_residual = b.mass_residual;
	energy_residual = b.energy_residual;
	mass_residual_loc = b.mass_residual_loc;
	energy_residual_loc = b.energy_residual_loc;
	hncell = b.hncell;
	hicell = b.hicell; hjcell = b.hjcell; hkcell = b.hkcell;
	nidim = b.nidim; njdim = b.njdim; nkdim = b.nkdim;
	nni = b.nni; nnj = b.nnj; nnk = b.nnk;
	imin = b.imin; imax = b.imax;
	jmin = b.jmin; jmax = b.jmax;
	kmin = b.kmin; kmax = b.kmax;
	active_cells = b.active_cells;
	baldwin_lomax_iturb = b.baldwin_lomax_iturb;
	bcp = b.bcp;
	ctr_ = b.ctr_;
	ifi_ = b.ifi_; ifj_ = b.ifj_; ifk_ = b.ifk_;
	vtx_ = b.vtx_;
	sifi_ = b.sifi_; sifj_ = b.sifj_; sifk_ = b.sifk_;
    }
    return *this;
}

Block::~Block() 
{
    for ( size_t i = 0; i < bcp.size(); ++i )
	delete bcp[i];
    // for ( size_t i = 0; i < shock_iface_pos.size(); ++i )
    //	delete shock_iface_pos[i];
}

/// \brief Allocate memory for the internal arrays of the block.
/// Returns 0 if successful, 1 otherwise.
int Block::array_alloc(size_t dimensions)
{
    if ( get_verbose_flag() ) cout << "array_alloc(): Begin for block " <<  id << endl;
    // Check for obvious errors.
    if ( nidim <= 0 || njdim <= 0 || nkdim <= 0 ) {
        cerr << "array_alloc(): Declared dimensions are zero or negative: " 
	     << nidim << ", " << njdim << ", " << nkdim << endl;
	exit( VALUE_ERROR );
    }
    Gas_model *gm = get_gas_model_ptr();

    // Allocate vectors of pointers for the entire block.
    size_t ntot = nidim * njdim * nkdim;
    ctr_.resize(ntot); 
    ifi_.resize(ntot);
    ifj_.resize(ntot);
    if ( dimensions == 3 ) ifk_.resize(ntot);
    vtx_.resize(ntot);
    sifi_.resize(ntot);
    sifj_.resize(ntot);
    if ( dimensions == 3 ) sifk_.resize(ntot);
    // Now, create the actual objects.
    for (size_t gid = 0; gid < ntot; ++gid) {
        ctr_[gid] = new FV_Cell(gm);
	ctr_[gid]->id = gid;
	std::vector<size_t> ijk = to_ijk_indices(gid);
	size_t i = ijk[0]; size_t j = ijk[1]; size_t k = ijk[2];
	if ( i >= imin && i <= imax && j >= jmin && j <= jmax && k >= kmin && k <= kmax ) {
	    active_cells.push_back(ctr_[gid]);
	}
        ifi_[gid] = new FV_Interface(gm);
	ifi_[gid]->id = gid;
        ifj_[gid] = new FV_Interface(gm);
	ifj_[gid]->id = gid;
        if ( dimensions == 3 ) {
	    ifk_[gid] = new FV_Interface(gm);
	    ifk_[gid]->id = gid;
	}
        vtx_[gid] = new FV_Vertex(gm);
	vtx_[gid]->id = gid;
        sifi_[gid] = new FV_Interface(gm);
	sifi_[gid]->id = gid;
        sifj_[gid] = new FV_Interface(gm);
	sifj_[gid]->id = gid;
        if ( dimensions == 3 ) {
	    sifk_[gid] = new FV_Interface(gm);
	    sifk_[gid]->id = gid;
	}
    } // gid loop

    if ( get_verbose_flag() || id == 0 ) {
	cout << "Block " << id << ": finished creating " << ntot << " cells." << endl;
    }
    return SUCCESS;
} // end of array_alloc()


int Block::array_cleanup(size_t dimensions)
{
    if ( get_verbose_flag() || id == 0 ) {
	cout << "array_cleanup(): Begin for block " <<  id << endl;
    }
    // Need to clean up allocated memory.
    size_t ntot = nidim * njdim * nkdim;
    for (size_t gid = 0; gid < ntot; ++gid) {
	delete ctr_[gid];
	delete ifi_[gid];
	delete ifj_[gid];
	if ( dimensions == 3 ) delete ifk_[gid];
	delete vtx_[gid];
	delete sifi_[gid];
	delete sifj_[gid];
	if ( dimensions == 3 ) delete sifk_[gid];
    } // gid loop
    ctr_.clear(); 
    ifi_.clear();
    ifj_.clear();
    ifk_.clear();
    vtx_.clear();
    sifi_.clear();
    sifj_.clear();
    sifk_.clear();
    active_cells.clear();
    return SUCCESS;
} // end of array_cleanup()

//-----------------------------------------------------------------------------

int Block::bind_interfaces_to_cells( size_t dimensions )
{
    FV_Cell *cellp;
    size_t kstart, kend;

    if ( dimensions == 3 ) {
	kstart = kmin-1;
	kend = kmax+1;
    } else {
	kstart = 0;
	kend = 0;
    }
    for ( size_t k = kstart; k <= kend; ++k ) {
	for ( size_t j = jmin-1; j <= jmax+1; ++j ) {
	    for ( size_t i = imin-1; i <= imax+1; ++i ) {
		cellp = get_cell(i,j,k);
		cellp->iface[NORTH] = get_ifj(i,j+1,k);
		cellp->iface[EAST] = get_ifi(i+1,j,k);
		cellp->iface[SOUTH] = get_ifj(i,j,k);
		cellp->iface[WEST] = get_ifi(i,j,k);
		cellp->vtx[0] = get_vtx(i,j,k);
		cellp->vtx[1] = get_vtx(i+1,j,k);
		cellp->vtx[2] = get_vtx(i+1,j+1,k);
		cellp->vtx[3] = get_vtx(i,j+1,k);
		if ( dimensions == 3 ) {
		    cellp->iface[TOP] = get_ifk(i,j,k+1);
		    cellp->iface[BOTTOM] = get_ifk(i,j,k);
		    cellp->vtx[4] = get_vtx(i,j,k+1);
		    cellp->vtx[5] = get_vtx(i+1,j,k+1);
		    cellp->vtx[6] = get_vtx(i+1,j+1,k+1);
		    cellp->vtx[7] = get_vtx(i,j+1,k+1);
		} else {
		    cellp->iface[TOP] = NULL;
		    cellp->iface[BOTTOM] = NULL;
		    cellp->vtx[4] = NULL;
		    cellp->vtx[5] = NULL;
		    cellp->vtx[6] = NULL;
		    cellp->vtx[7] = NULL;
		} // end if
	    }
	}
    }
    return SUCCESS;
} // end bind_interfaces_to_cells()


/// \brief Set the base heat source values for this block.
int Block::set_base_qdot( global_data &gd )
{
    double total_qdot_for_block = 0.0;
    CHeatZone *hzp;
    FV_Cell *cellp;

    total_qdot_for_block = 0.0;
    for ( size_t k = kmin; k <= kmax; ++k ) {
	for ( size_t i = imin; i <= imax; ++i ) {
	    for ( size_t j = jmin; j <= jmax; ++j ) {
		cellp = get_cell(i,j,k);
		cellp->base_qdot = 0.0;
		for ( size_t indx = 0; indx < gd.n_heat_zone; ++indx ) {
		    hzp = &(gd.heat_zone[indx]);
		    if ( cellp->pos.x >= hzp->x0 && cellp->pos.x <= hzp->x1 &&
			 cellp->pos.y >= hzp->y0 && cellp->pos.y <= hzp->y1 &&
			 (gd.dimensions == 2 || 
			  (cellp->pos.z >= hzp->z0 && cellp->pos.z <= hzp->z1)) ) {
			cellp->base_qdot += hzp->qdot;
		    }
		} // for indx
		total_qdot_for_block += cellp->base_qdot * cellp->volume;
	    } // for j
	} // for i
    } // for k
    if ( total_qdot_for_block > 1.0e-10 ) {
	cout << "set_base_qdot(): block " << id
	     << " base_qdot= " << total_qdot_for_block << " Watts" << endl;
    }
    return SUCCESS;
} // end set_base_qdot()


/// \brief Set the reactions-allowed flag for cells in this block.
int Block::identify_reaction_zones( global_data &gd )
{
    size_t total_cells_in_reaction_zones = 0;
    size_t total_cells = 0;
    CReactionZone *rzp;
    FV_Cell *cellp;

    for ( size_t k = kmin; k <= kmax; ++k ) {
	for ( size_t i = imin; i <= imax; ++i ) {
	    for ( size_t j = jmin; j <= jmax; ++j ) {
		cellp = get_cell(i,j,k);
		if ( gd.n_reaction_zone > 0 ) {
		    // User-specified reaction zones; mask off reacting/nonreacting zones.
		    cellp->fr_reactions_allowed = 0;
		    for ( size_t indx = 0; indx < gd.n_reaction_zone; ++indx ) {
			rzp = &(gd.reaction_zone[indx]);
			if ( cellp->pos.x >= rzp->x0 && cellp->pos.x <= rzp->x1 &&
			     cellp->pos.y >= rzp->y0 && cellp->pos.y <= rzp->y1 &&
			     (gd.dimensions == 2 || 
			      (cellp->pos.z >= rzp->z0 && cellp->pos.z <= rzp->z1)) ) {
			    cellp->fr_reactions_allowed = 1;
			}
		    } // end for( indx
		} else {
		    // No user-specified zones; always allow reactions.
		    cellp->fr_reactions_allowed = 1;
		}
		total_cells_in_reaction_zones += cellp->fr_reactions_allowed;
		total_cells += 1;
	    } // for j
	} // for i
    } // for k
    if ( get_reacting_flag() ) {
	cout << "identify_reaction_zones(): block " << id
	     << " cells inside zones = " << total_cells_in_reaction_zones 
	     << " out of " << total_cells << endl;
	if ( gd.n_reaction_zone == 0 ) {
	    cout << "Note that for no user-specified zones,"
		 << " the whole domain is allowed to be reacting." << endl;
	}
    }
    return SUCCESS;
} // end identify_reaction_zones()


/// \brief Set the in-turbulent-zone flag for cells in this block.
int Block::identify_turbulent_zones( global_data &gd )
{
    size_t total_cells_in_turbulent_zones = 0;
    size_t total_cells = 0;
    CTurbulentZone *tzp;
    FV_Cell *cellp;
    
    for ( size_t k = kmin; k <= kmax; ++k ) {
	for ( size_t i = imin; i <= imax; ++i ) {
	    for ( size_t j = jmin; j <= jmax; ++j ) {
		cellp = get_cell(i,j,k);
		if ( gd.n_turbulent_zone > 0 ) {
		    cellp->in_turbulent_zone = 0;
		    for ( size_t indx = 0; indx < gd.n_turbulent_zone; ++indx ) {
			tzp = &(gd.turbulent_zone[indx]);
			if ( cellp->pos.x >= tzp->x0 && cellp->pos.x <= tzp->x1 &&
			     cellp->pos.y >= tzp->y0 && cellp->pos.y <= tzp->y1 &&
			     (gd.dimensions == 2 || 
			      (cellp->pos.z >= tzp->z0 && cellp->pos.z <= tzp->z1)) ) {
			    cellp->in_turbulent_zone = 1;
			}
		    } // for indx
		} else {
		    cellp->in_turbulent_zone = 1;
		}
		total_cells_in_turbulent_zones += cellp->in_turbulent_zone;
		total_cells += 1;
	    } // for j
	} // for i
    } // for k
    if ( get_turbulence_flag() ) {
	cout << "identify_turbulent_zones(): block " << id
	     << " cells inside zones = " << total_cells_in_turbulent_zones 
	     << " out of " << total_cells << endl;
	if ( gd.n_turbulent_zone == 0 ) {
	    cout << "Note that for no user-specified zones,"
		 << " the whole domain is allowed to be turbulent." << endl;
	}
    }
    return SUCCESS;
} // end identify_turbulent_zones()


int Block::clear_fluxes_of_conserved_quantities( size_t dimensions )
{
    FV_Interface *IFace;

    for ( size_t k = kmin; k <= kmax; ++k ) {
	for (size_t j = jmin; j <= jmax; ++j) {
	    for (size_t i = imin; i <= imax+1; ++i) {
		IFace = get_ifi(i,j,k);
		IFace->F->clear_values();
	    } // for i
	} // for j
    } // for k
    for ( size_t k = kmin; k <= kmax; ++k ) {
	for (size_t j = jmin; j <= jmax+1; ++j) {
	    for (size_t i = imin; i <= imax; ++i) {
		IFace = get_ifj(i,j,k);
		IFace->F->clear_values();
	    } // for i
	} // for j
    } // for k
    if ( dimensions == 3 ) {
	for ( size_t k = kmin; k <= kmax+1; ++k ) {
	    for (size_t j = jmin; j <= jmax; ++j) {
		for (size_t i = imin; i <= imax; ++i) {
		    IFace = get_ifk(i,j,k);
		    IFace->F->clear_values();
		} // for i
	    } // for j
	} // for k
    } // end if G.dimensions == 3
    return SUCCESS;
}

int Block::propagate_data_west_to_east( size_t dimensions )
// Propagate data from the west ghost cell, right across the block.
// This is a useful starting state for the block-sequenced calculation
// where the final flow is expected to be steady-state.
{
    FV_Cell *src, *dest;
    Gas_model *gm = get_gas_model_ptr();
    for ( size_t k = kmin; k <= kmax; ++k ) {
	for ( size_t j = jmin; j <= jmax; ++j) {
	    src = get_cell(imin-1,j);
	    for ( size_t i = imin; i <= imax; ++i ) {
		dest = get_cell(i,j,k);
		dest->copy_values_from(*src, COPY_FLOW_STATE);
		Gas_data *gas = dest->fs->gas;
		if ( gm->eval_thermo_state_pT(*gas) != SUCCESS ||
		     gm->eval_transport_coefficients(*gas) != SUCCESS ) {
		    printf( "propagate_data_west_to_east(): Duff call to thermo model.\n" );
		    printf( "   i=%d, j=%d, k=%d\n", static_cast<int>(i),
			    static_cast<int>(j), static_cast<int>(k) );
		    gas->print_values();
		    exit(DUFF_EOS_ERROR);  /* Might as well quit early. */
		}
	    } // for i
	} // for j
    } // for k
    return SUCCESS;
} // end propagate_data_west_to_east()


/** \brief Computes the (pressure and shear) forces applied by the gas 
 *         to the bounding surface.
 *
 * We make use of geometric quantities stored at 
 * the cell interfaces.  
 * (Area is area per unit radian for axisymmetric calculations.)
 */
void Block::compute_x_forces( char *text_string, int ibndy, size_t dimensions )
{
    double fx_p, fx_v, x1, y1, cosX, cosY, area;
    double xc, yc, d, vt, mu;
    size_t i, j, ivisc;
    FV_Cell *cell;
    FV_Interface *IFace;
    
    if ( dimensions == 3 ) {
	printf( "X-Force calculations not implemented for 3D geometries, yet." );
	exit( NOT_IMPLEMENTED_ERROR );
    }

    fx_p = 0.0;
    fx_v = 0.0;
    ivisc = get_viscous_flag();

    if ( ibndy == NORTH ) {
	j = jmax;
	for ( i = imin; i <= imax; ++i ) {
	    cell = get_cell(i,j);
	    IFace = cell->iface[NORTH];
	    cosX = IFace->n.x;
	    cosY = IFace->n.y;
	    area = IFace->area;
	    mu   = IFace->fs->gas->mu;
	    fx_p += cell->fs->gas->p * area * cosX;
	    if ( ivisc ) {
		/* pieces needed to reconstruct the local velocity gradient */
		x1 = cell->vtx[0]->pos.x; y1 = cell->vtx[0]->pos.y;
		xc = cell->pos.x;   yc = cell->pos.y;
		d = -(xc - x1) * cosX + -(yc - y1) * cosY;
		vt = cell->fs->vel.x * cosY - cell->fs->vel.y * cosX;
		/* x-component of the shear force, assuming no-slip wall */
		fx_v += mu * vt / d * area * cosY;
	    }
	}
    } else if ( ibndy == SOUTH ) {
	j = jmin;
	for ( i = imin; i <= imax; ++i ) {
	    cell = get_cell(i,j);
	    IFace = cell->iface[SOUTH];
	    cosX = IFace->n.x;
	    cosY = IFace->n.y;
	    area = IFace->area;
	    mu   = IFace->fs->gas->mu;
	    fx_p -= cell->fs->gas->p * area * cosX;
	    if ( ivisc ) {
		/* pieces needed to reconstruct the local velocity gradient */
		x1 = cell->vtx[0]->pos.x; y1 = cell->vtx[0]->pos.y;
		xc = cell->pos.x;   yc = cell->pos.y;
		d = (xc - x1) * cosX + (yc - y1) * cosY;
		vt = cell->fs->vel.x * cosY - cell->fs->vel.y * cosX;
		/* x-component of the shear force, assuming no-slip wall */
		fx_v += mu * vt / d * area * cosY;
	    }
	}
    } else if ( ibndy == EAST ) {
	i = imax;
	for ( j = jmin; j <= jmax; ++j ) {
	    cell = get_cell(i,j);
	    IFace = cell->iface[EAST];
	    cosX = IFace->n.x;
	    cosY = IFace->n.y;
	    area = IFace->area;
	    mu   = IFace->fs->gas->mu;
	    fx_p += cell->fs->gas->p * area * cosX;
	    if ( ivisc ) {
		/* pieces needed to reconstruct the local velocity gradient */
		x1 = cell->vtx[1]->pos.x; y1 = cell->vtx[1]->pos.y;
		xc = cell->pos.x;   yc = cell->pos.y;
		d = -(xc - x1) * cosX + -(yc - y1) * cosY;
		vt = -(cell->fs->vel.x) * cosY + cell->fs->vel.y * cosX;
		/* x-component of the shear force, assuming no-slip wall */
		fx_v -= mu * vt / d * area * cosY;
	    }
	}
    } else if ( ibndy == WEST ) {
	i = imin;
	for ( j = jmin; j <= jmax; ++j ) {
	    cell = get_cell(i,j);
	    IFace = cell->iface[WEST];
	    cosX = IFace->n.x;
	    cosY = IFace->n.y;
	    area = IFace->area;
	    mu   = IFace->fs->gas->mu;
	    fx_p -= cell->fs->gas->p * area * cosX;
	    if ( ivisc ) {
		/* pieces needed to reconstruct the local velocity gradient */
		x1 = cell->vtx[0]->pos.x; y1 = cell->vtx[0]->pos.y;
		xc = get_cell(i,j)->pos.x;  yc = get_cell(i,j)->pos.y;
		d = (xc - x1) * cosX + (yc - y1) * cosY;
		vt = -(cell->fs->vel.x) * cosY + cell->fs->vel.y * cosX;
		/* x-component of the shear force, assuming no-slip wall */
		fx_v -= mu * vt / d * area * cosY;
	    }
	}
    }   /* end if: boundary selection */

    if ( get_axisymmetric_flag() == 1 ) {
	fx_p *= (2.0 * 3.1415927);
	fx_v *= (2.0 * 3.1415927);
    }

    sprintf( text_string, "FX_P %e FX_V %e ", fx_p, fx_v );
} // end compute_x_forces()


/// \brief Assemble the x-force numbers for each block into a single
///        (string) report and send it to the logfile. 
int Block::print_forces( FILE *fp, double t, size_t dimensions )
{
    char msg_text[512], small_text[132];

    if ( bcp[NORTH]->xforce_flag == 1 ) {
	sprintf( small_text, "XFORCE: TIME %e BLOCK %d BNDY %d ",
		 t, static_cast<int>(id), NORTH );
	strcpy( msg_text, small_text );
	this->compute_x_forces( small_text, NORTH, dimensions );
	strcat( msg_text, small_text );
	fprintf( fp, "%s\n",  msg_text );
    }
    if ( bcp[EAST]->xforce_flag == 1 ) {
	sprintf( small_text, "XFORCE: TIME %e BLOCK %d BNDY %d ",
		 t, static_cast<int>(id), EAST );
	strcpy( msg_text, small_text );
	this->compute_x_forces( small_text, EAST, dimensions );
	strcat( msg_text, small_text );
	fprintf( fp, "%s\n",  msg_text );
    }
    if ( bcp[SOUTH]->xforce_flag == 1 ) {
	sprintf( small_text, "XFORCE: TIME %e BLOCK %d BNDY %d ",
		 t, static_cast<int>(id), SOUTH );
	strcpy( msg_text, small_text );
	this->compute_x_forces( small_text, SOUTH, dimensions );
	strcat( msg_text, small_text );
	fprintf( fp, "%s\n",  msg_text );
    }
    if ( bcp[WEST]->xforce_flag == 1 ) {
	sprintf( small_text, "XFORCE: TIME %e BLOCK %d BNDY %d ",
		 t, static_cast<int>(id), WEST );
	strcpy( msg_text, small_text );
	this->compute_x_forces( small_text, WEST, dimensions );
	strcat( msg_text, small_text );
	fprintf( fp, "%s\n",  msg_text );
    }

    return SUCCESS;
} // end print_forces()


int Block::count_invalid_cells( size_t dimensions )
/// \brief Returns the number of cells that contain invalid data.
///
/// This data can be identified by the density of internal energy 
/// being on the minimum limit or the velocity being very large.
//
// To do: We should probably make this function more 3D friendly, however,
// it should not be invoked (ever) if the code is working well!
{
    FV_Cell *cell;
    size_t  number_of_invalid_cells;
    Gas_model *gmodel = get_gas_model_ptr();
    FV_Cell dummy_cell(gmodel);
    
    number_of_invalid_cells = 0;

    for ( size_t k = kmin; k <= kmax; ++k ) {
	for ( size_t i = imin; i <= imax; ++i ) {
	    for ( size_t j = jmin; j <= jmax; ++j ) {
		cell = get_cell(i,j,k);
		if ( cell->check_flow_data() != 1 ) {
		    ++number_of_invalid_cells;
		    if ( get_bad_cell_complain_flag() ) {
			printf("count_invalid_cells: block_id = %d, cell[%d,%d,%d]\n", 
			       static_cast<int>(id), static_cast<int>(i), 
			       static_cast<int>(j), static_cast<int>(k));
			cell->print();
		    }
		    if ( adjust_invalid_cell_data ) {
			// We shall set the cell data to something that
			// is valid (and self consistent).
			FV_Cell *other_cell;
			std::vector<FV_Cell *> neighbours;
			if (prefer_copy_from_left ) {
			    if ( get_bad_cell_complain_flag() ) {
				printf( "Adjusting cell data by copying data from left.\n" );
			    }
			    // Presently, only searches around in the i,j plane
			    other_cell = get_cell(i-1,j,k);
			    if ( other_cell->check_flow_data() ) neighbours.push_back(other_cell);
			} else {
			    if ( get_bad_cell_complain_flag() ) {
				printf( "Adjusting cell data to a local average.\n" );
			    }
			    other_cell = get_cell(i+1,j,k);
			    if ( other_cell->check_flow_data() ) neighbours.push_back(other_cell);
			    other_cell = get_cell(i,j-1,k);
			    if ( other_cell->check_flow_data() ) neighbours.push_back(other_cell);
			    other_cell = get_cell(i,j+1,k);
			    if ( other_cell->check_flow_data() ) neighbours.push_back(other_cell);
			    if ( neighbours.size() == 0 ) {
				printf( "It seems that there were no valid neighbours, I give up.\n" );
				exit( BAD_CELLS_ERROR );
			    }
			    cell->replace_flow_data_with_average(neighbours);
			}
			cell->encode_conserved(omegaz);
			cell->decode_conserved(omegaz);
			if ( get_bad_cell_complain_flag() ) {
			    printf("after flow-data replacement: block_id = %d, cell[%d,%d,%d]\n", 
				   static_cast<int>(id), static_cast<int>(i),
				   static_cast<int>(j), static_cast<int>(k));
			    cell->print();
			}
		    } // end adjust_invalid_cell_data 
		} // end of if invalid data...
	    } // j loop
	} // i loop
    } // k loop
    return number_of_invalid_cells;
} // end of count_invalid_cells()


int Block::init_residuals( size_t dimensions )
/// \brief Initialization of data for later computing residuals.
{
    FV_Cell *cellp;
    mass_residual = 0.0;
    mass_residual_loc = Vector3(0.0, 0.0, 0.0);
    energy_residual = 0.0;
    energy_residual_loc = Vector3(0.0, 0.0, 0.0);
    for ( size_t k = kmin; k <= kmax; ++k ) {
	for ( size_t j = jmin; j <= jmax; ++j ) {
	    for (size_t  i = imin; i <= imax; ++i ) {
		cellp = get_cell(i,j,k);
		cellp->rho_at_start_of_step = cellp->fs->gas->rho;
		cellp->rE_at_start_of_step = cellp->U->total_energy;
	    } // for i
	} // for j
    } // for k
    return SUCCESS;
} // end of check_residual()


int Block::compute_residuals( size_t dimensions )
/// \brief Compute the residuals using previously stored data.
///
/// The largest residual of density for all cells was the traditional way
/// mbcns/Elmer estimated the approach to steady state.
/// However, with the splitting up of the increments for different physical
/// processes, this residual calculation code needed a bit of an update.
/// Noting that the viscous-stress, chemical and radiation increments
/// do not affect the mass within a cell, we now compute the residuals 
/// for both mass and (total) energy for all cells, the record the largest
/// with their location. 
{
    double local_residual;
    mass_residual = 0.0;
    mass_residual_loc = Vector3(0.0, 0.0, 0.0);
    energy_residual = 0.0;
    energy_residual_loc = Vector3(0.0, 0.0, 0.0);
    for ( FV_Cell *cellp: active_cells ) {
	local_residual = ( cellp->fs->gas->rho - cellp->rho_at_start_of_step ) / cellp->fs->gas->rho;
	local_residual = FABS(local_residual);
	if ( local_residual > mass_residual ) {
	    mass_residual = local_residual;
	    mass_residual_loc.x = cellp->pos.x;
	    mass_residual_loc.y = cellp->pos.y;
	    mass_residual_loc.z = cellp->pos.z;
	}
	local_residual = ( cellp->U->total_energy - cellp->rE_at_start_of_step ) / cellp->U->total_energy;
	local_residual = FABS(local_residual);
	if ( local_residual > energy_residual ) {
	    energy_residual = local_residual;
	    energy_residual_loc.x = cellp->pos.x;
	    energy_residual_loc.y = cellp->pos.y;
	    energy_residual_loc.z = cellp->pos.z;
	}
    } // for cellp
    return SUCCESS;
} // end of compute_residuals()


int Block::determine_time_step_size( double cfl_target, size_t dimensions )
/// \brief Compute the local time step limit for all cells in the block.
///
/// The overall time step is limited by the worst-case cell.
/// \returns 0 on success, DT_SEARCH_FAILED otherwise.
///
/// \verbatim
/// Some Definitions...
/// ----------------
/// dt_global  : global time step for the block
/// cfl_target : desired CFL number
/// cfl_min  : approximate minimum CFL number in the block
/// cfl_max  : approximate maximum CFL number in the block
/// dt_allow : allowable time step (i.e. the maximum dt that
///            satisfies both the CFL target and the viscous
///            time step limit)
/// cfl_allow : allowable CFL number, t_order dependent
/// \endverbatim
{
    global_data *gdp = get_global_data_ptr();
    bool first;
    double dt_local, cfl_local, signal, cfl_allow;
    if ( get_Torder_flag() < 3 ) {
	cfl_allow = 0.9;
    } else {
	cfl_allow = 1.1;
    }

    first = true;
    for ( FV_Cell *cp: active_cells ) {
	signal = cp->signal_frequency(dimensions);
	cfl_local = gdp->dt_global * signal; // Current (Local) CFL number
	dt_local = cfl_target / signal; // Recommend a time step size.
	if ( first ) {
	    cfl_min = cfl_local;
	    cfl_max = cfl_local;
	    dt_allow = dt_local;
	    first = false;
	} else {
	    if (cfl_local < cfl_min) cfl_min = cfl_local;
	    if (cfl_local > cfl_max) cfl_max = cfl_local;
	    if (dt_local < dt_allow) dt_allow = dt_local;
	}
    } // for cp
    if ( cfl_max > 0.0 && cfl_max < cfl_allow ) {
	return SUCCESS;
    } else {
	printf( "determine_time_step_size(): bad CFL number was encountered\n" );
	printf( "    cfl_max = %e for Block %d\n", cfl_max, static_cast<int>(id) );
	printf( "    If this cfl_max value is not much larger than 1.0,\n" );
	printf( "    your simulation could probably be restarted successfully\n" );
	printf( "    with some minor tweaking." );
	printf( "    That tweaking should probably include a reduction\n");
	printf( "    in the size of the initial time-step, dt\n");
	printf( "    If this job is a restart/continuation of an old job, look in\n");
	printf( "    the old-job.finish file for the value of dt at termination.\n");
	return DT_SEARCH_FAILED;
    }
} // end of determine_time_step_size()


int Block::detect_shock_points( size_t dimensions )
/// \brief Detects shocks by looking for compression between adjacent cells.
///
/// The velocity component normal to the cell interfaces
/// is used as the indicating variable.
{
    FV_Cell *cL, *cR, *cell;
    FV_Interface *IFace;
    double uL, uR, aL, aR, a_min;

    // Change in normalised velocity to indicate a shock.
    // A value of -0.05 has been found suitable to detect the levels of
    // shock compression observed in the "sod" and "cone20" test cases.
    // It may need to be tuned for other situations, especially when
    // viscous effects are important.
    double tol = get_compression_tolerance();

    // First, work across North interfaces and
    // locate shocks using the (local) normal velocity.
    for ( size_t k = kmin; k <= kmax; ++k ) {
	for ( size_t i = imin; i <= imax; ++i ) {
	    for ( size_t j = jmin-1; j <= jmax; ++j ) {
		cL = get_cell(i,j,k);
		cR = get_cell(i,j+1,k);
		IFace = cL->iface[NORTH];
		uL = cL->fs->vel.x * IFace->n.x + cL->fs->vel.y * IFace->n.y + cL->fs->vel.z * IFace->n.z;
		uR = cR->fs->vel.x * IFace->n.x + cR->fs->vel.y * IFace->n.y + cR->fs->vel.z * IFace->n.z;
		aL = cL->fs->gas->a;
		aR = cR->fs->gas->a;
		if (aL < aR)
		    a_min = aL;
		else
		    a_min = aR;
		IFace->fs->S = ((uR - uL) / a_min < tol);
	    } // j loop
	} // i loop
    } // for k
    
    // Second, work across East interfaces and
    // locate shocks using the (local) normal velocity.
    for ( size_t k = kmin; k <= kmax; ++k ) {
	for ( size_t i = imin-1; i <= imax; ++i ) {
	    for ( size_t j = jmin; j <= jmax; ++j ) {
		cL = get_cell(i,j,k);
		cR = get_cell(i+1,j,k);
		IFace = cL->iface[EAST];
		uL = cL->fs->vel.x * IFace->n.x + cL->fs->vel.y * IFace->n.y + cL->fs->vel.z * IFace->n.z;
		uR = cR->fs->vel.x * IFace->n.x + cR->fs->vel.y * IFace->n.y + cR->fs->vel.z * IFace->n.z;
		aL = cL->fs->gas->a;
		aR = cR->fs->gas->a;
		if (aL < aR)
		    a_min = aL;
		else
		    a_min = aR;
		IFace->fs->S = ((uR - uL) / a_min < tol);
	    } // j loop
	} // i loop
    } // for k
    
    if ( dimensions == 3 ) {
	// Third, work across Top interfaces.
	for ( size_t i = imin; i <= imax; ++i ) {
	    for ( size_t j = jmin; j <= jmax; ++j ) {
		for ( size_t k = kmin-1; k <= kmax; ++k ) {
		    cL = get_cell(i,j,k);
		    cR = get_cell(i,j,k+1);
		    IFace = cL->iface[TOP];
		    uL = cL->fs->vel.x * IFace->n.x + cL->fs->vel.y * IFace->n.y + cL->fs->vel.z * IFace->n.z;
		    uR = cR->fs->vel.x * IFace->n.x + cR->fs->vel.y * IFace->n.y + cR->fs->vel.z * IFace->n.z;
		    aL = cL->fs->gas->a;
		    aR = cR->fs->gas->a;
		    if (aL < aR)
			a_min = aL;
		    else
			a_min = aR;
		    IFace->fs->S = ((uR - uL) / a_min < tol);
		} // for k
	    } // j loop
	} // i loop
    } // if ( dimensions == 3 )
    
    // Finally, mark cells as shock points if any of their
    // interfaces are shock points.
    for ( size_t k = kmin; k <= kmax; ++k ) {
	for ( size_t i = imin; i <= imax; ++i ) {
	    for ( size_t j = jmin; j <= jmax; ++j ) {
		cell = get_cell(i,j,k);
		cell->fs->S = cell->iface[EAST]->fs->S || cell->iface[WEST]->fs->S ||
		    cell->iface[NORTH]->fs->S || cell->iface[SOUTH]->fs->S ||
		    ( dimensions == 3 && (cell->iface[BOTTOM]->fs->S || cell->iface[TOP]->fs->S) );
	    } // j loop
	} // i loop
    } // for k
    
    return SUCCESS;
} // end of detect_shock_points()

//-----------------------------------------------------------------------------

int find_nearest_cell( double x, double y, double z, 
		       size_t *jb_near,
		       size_t *i_near, size_t *j_near, size_t *k_near )
/// \brief Given an (x,y,z) position, locate the nearest cell centre.  
///
/// @param x, y, z : coordinates of the desired point
/// @param i_near, j_near, k_near : pointers to indices of the cell centre are stored here
/// @returns 1 for a close match, 0 if there were no close-enough cells.
{
    global_data *gd = get_global_data_ptr();
    Block *bdp;
    size_t ig, jg, kg, jbg;
    double dx, dy, dz, nearest, distance;

    jbg = 0;
    bdp = get_block_data_ptr(jbg);
    ig = bdp->imin; jg = bdp->jmin; kg = bdp->kmin;
    FV_Cell *mycp = bdp->get_cell(ig,jg,kg);
    dx = x - mycp->pos.x; dy = y - mycp->pos.y; dz = z - mycp->pos.z;
    nearest = sqrt(dx*dx + dy*dy + dz*dz);

    for ( size_t jb = 0; jb < gd->nblock; jb++ ) {
	bdp = get_block_data_ptr(jb);
	for ( FV_Cell *cp: bdp->active_cells ) {
	    dx = x - cp->pos.x; dy = y - cp->pos.y; dz = z - cp->pos.z;
	    distance = sqrt(dx*dx + dy*dy + dz*dz);
	    if (distance < nearest) {
		std::vector<size_t> ijk = bdp->to_ijk_indices(cp->id);
		ig = ijk[0]; jg = ijk[1]; kg = ijk[2];
		nearest = distance; jbg = jb;
	    }
	} // for cp
    } // for jb
    bdp = get_block_data_ptr(jbg);
    *jb_near = jbg; *i_near = ig; *j_near = jg; *k_near = kg;
    if ( nearest > bdp->get_ifi(ig,jg,kg)->length || 
	 nearest > bdp->get_ifj(ig,jg,kg)->length || 
	 (gd->dimensions == 3 && (nearest > bdp->get_ifk(ig,jg,kg)->length)) ) {
        printf("We really did not get close to (%e, %e, %e)\n", x, y, z);
        printf("Nearest %e lengths %e %e %e \n", nearest, 
	       bdp->get_ifi(ig,jg,kg)->length, bdp->get_ifj(ig,jg,kg)->length, 
	       bdp->get_ifk(ig,jg,kg)->length);
	return 0;
    } else {
        return 1;
    }
} // end find_nearest_cell()


// Some global data for locate_cell().
// To shorten the search, starting_block is the first block searched
// when requested locate_cell() is requested to locate the cell
// containing a specified point.
size_t starting_block = 0;

int locate_cell(double x, double y, double z,
	        size_t *jb_found, size_t *i_found, size_t *j_found, size_t *k_found)
// Returns 1 if a cell containing the sample point (x,y,z) is found, else 0.
// The indices of the containing cell are recorded, if found.
//
// To consider: maybe we should use *jb_found as the starting_block value.
{
    global_data *gd = get_global_data_ptr();
    Block *bdp;
    Vector3 p = Vector3(x,y,z);
    *i_found = 0; *j_found = 0; *k_found = 0; *jb_found = 0;
    // Search the blocks, starting from the block in which the last point was found.
    for ( size_t jb = starting_block; jb < gd->nblock; jb++ ) {
	bdp = get_block_data_ptr(jb);
	for ( FV_Cell *cp: bdp->active_cells ) {
	    if ( cp->point_is_inside(p, gd->dimensions) ) {
		std::vector<size_t> ijk = bdp->to_ijk_indices(cp->id);
		*i_found = ijk[0]; *j_found = ijk[1]; *k_found = ijk[2]; 
		*jb_found = jb;
		starting_block = jb; // remember for next time
		return 1;
	    }
	} // for cp
    } // jb-loop
    // If we reach this point, then the point may be in one of the other blocks.
    for ( size_t jb = 0; jb < starting_block; jb++ ) {
	bdp = get_block_data_ptr(jb);
	for ( FV_Cell *cp: bdp->active_cells ) {
	    if ( cp->point_is_inside(p, gd->dimensions) ) {
		std::vector<size_t> ijk = bdp->to_ijk_indices(cp->id);
		*i_found = ijk[0]; *j_found = ijk[1]; *k_found = ijk[2]; 
		*jb_found = jb;
		starting_block = jb; // remember for next time
		return 1;
	    }
	} // for cp
    } // for jb
    // If we arrive here, we have not located the containing cell.
    return 0;
} // end locate_cell()
