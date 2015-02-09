// bc.d
// Base class for boundary condition objects, for use in Eilmer4
//
// Peter J. 2014-07-20 first cut.

import std.conv;
import geom;
import fvcore;
import globalconfig;
import fvinterface;
import fvcell;
import block;
import sblock;

enum BCCode 
{ 
    slip_wall,
    adiabatic_wall,
    fixed_t_wall,
    full_face_exchange,
    mapped_cell,
    supersonic_in,
    subsonic_in,
    static_profile_in,
    fixed_p_out,
    extrapolate_out
}

BCCode type_code_from_name(string name)
{
    switch ( name ) {
    case "slip_wall", "slip-wall", "SlipWall":
	return BCCode.slip_wall;
    case "adiabatic", "adiabatic_wall", "adiabatic-wall", "AdiabaticWall":
	return BCCode.adiabatic_wall;
    case "adjacent", "Adjacent", "full_face_exchange", "full-face-exchange",
	"FullFaceExchange":
	return BCCode.full_face_exchange;
    case "mapped_cell", "mapped-cell", "MappedCell":
	return BCCode.mapped_cell;
    case "supersonic_in", "supersonic-in", "sup_in", "sup-in", "SupersonicIn":
	return BCCode.supersonic_in;
    case "subsonic_in", "subsonic-in", "sub_in", "sub-in", "SubsonicIn":
	return BCCode.subsonic_in;
    case "static_profile_in", "static-profile-in", "static-profile", 
	"StaticProfileIn":
	return BCCode.static_profile_in;
    case "fixed_p_out", "fixed-p-out", "FixedPOut":
	return BCCode.fixed_p_out;
    case "extrapolate_out", "extrapolate-out", "ExtrapolateOut":
	return BCCode.extrapolate_out;
    default: return BCCode.slip_wall;
    } // end switch
} // end type_code_from_name()


void reflect_normal_velocity(ref FVCell cell, in FVInterface IFace)
// Reflects normal velocity with respect to the cell interface.
//
// The process is to rotate the velocity vector into the local frame of
// the interface, negate the normal (local x-component) velocity and
// rotate back to the global frame.
{
    cell.fs.vel.transform_to_local_frame(IFace.n, IFace.t1, IFace.t2);
    cell.fs.vel.refx = -(cell.fs.vel.x);
    cell.fs.vel.transform_to_global_frame(IFace.n, IFace.t1, IFace.t2);
}

void reflect_normal_magnetic_field(ref FVCell cell, in FVInterface IFace)
{
    cell.fs.B.transform_to_local_frame(IFace.n, IFace.t1, IFace.t2);
    cell.fs.B.refx = -(cell.fs.B.x);
    cell.fs.B.transform_to_global_frame(IFace.n, IFace.t1, IFace.t2);
}

class BoundaryCondition {
    // [TODO] we need to redesign this so that we can use unstructured-grid blocks, eventually.
    // Presently, there are a lot of assumptions built in that are specific for structured-grid blocks.
public:
    // Location of the boundary condition.
    SBlock blk; // reference to the structured-grid block to which this BC is applied
    int which_boundary; // identity/index of the relevant boundary

    // Nature of the boundary condition that may be checked 
    // by other parts of the CFD code.
    BCCode type_code = BCCode.slip_wall;
    bool is_wall = true;
    bool ghost_cell_data_available = true;
    bool sets_conv_flux_directly = false;
    bool sets_visc_flux_directly = false;
    double emissivity = 0.0;

    override string toString() const
    {
	char[] repr;
	repr ~= "BoundaryCondition(";
	repr ~= "type_code=" ~ to!string(type_code);
	repr ~= ")";
	return to!string(repr);
    }

    void apply_convective(double t)
    {
	// The default convective boundary condition is to reflect
	// the normal component of the velocity at the ghost-cell centres.
	// Effectively a slip-wall. 
	size_t i, j, k;
	FVCell src_cell, dest_cell;
	FVInterface IFace;
	auto opt = CopyDataOption.minimal_flow;

	final switch ( which_boundary ) {
	case Face.north:
	    j = blk.jmax;
	    for (k = blk.kmin; k <= blk.kmax; ++k) {
		for (i = blk.imin; i <= blk.imax; ++i) {
		    // ghost cell 1.
		    src_cell = blk.get_cell(i,j,k);
		    IFace = src_cell.iface[Face.north];
		    dest_cell = blk.get_cell(i,j+1,k);
		    dest_cell.copy_values_from(src_cell, opt);
		    reflect_normal_velocity(dest_cell, IFace);
		    if ( GlobalConfig.MHD ) {
			reflect_normal_magnetic_field(dest_cell, IFace);
		    }
		    // ghost cell 2.
		    src_cell = blk.get_cell(i,j-1,k);
		    dest_cell = blk.get_cell(i,j+2,k);
		    dest_cell.copy_values_from(src_cell, opt);
		    reflect_normal_velocity(dest_cell, IFace);
		    if ( GlobalConfig.MHD ) {
			reflect_normal_magnetic_field(dest_cell, IFace);
		    }
		} // end i loop
	    } // for k
	    break;
	case Face.east:
	    i = blk.imax;
	    for (k = blk.kmin; k <= blk.kmax; ++k) {
		for (j = blk.jmin; j <= blk.jmax; ++j) {
		    // ghost cell 1.
		    src_cell = blk.get_cell(i,j,k);
		    IFace = src_cell.iface[Face.east];
		    dest_cell = blk.get_cell(i+1,j,k);
		    dest_cell.copy_values_from(src_cell, opt);
		    reflect_normal_velocity(dest_cell, IFace);
		    if ( GlobalConfig.MHD ) {
			reflect_normal_magnetic_field(dest_cell, IFace);
		    }
		    // ghost cell 2.
		    src_cell = blk.get_cell(i-1,j,k);
		    dest_cell = blk.get_cell(i+2,j,k);
		    dest_cell.copy_values_from(src_cell, opt);
		    reflect_normal_velocity(dest_cell, IFace);
		    if ( GlobalConfig.MHD ) {
			reflect_normal_magnetic_field(dest_cell, IFace);
		    }
		} // end j loop
	    } // for k
	    break;
	case Face.south:
	    j = blk.jmin;
	    for (k = blk.kmin; k <= blk.kmax; ++k) {
		for (i = blk.imin; i <= blk.imax; ++i) {
		    // ghost cell 1.
		    src_cell = blk.get_cell(i,j,k);
		    IFace = src_cell.iface[Face.south];
		    dest_cell = blk.get_cell(i,j-1,k);
		    dest_cell.copy_values_from(src_cell, opt);
		    reflect_normal_velocity(dest_cell, IFace);
		    if ( GlobalConfig.MHD ) {
			reflect_normal_magnetic_field(dest_cell, IFace);
		    }
		    // ghost cell 2.
		    src_cell = blk.get_cell(i,j+1,k);
		    dest_cell = blk.get_cell(i,j-2,k);
		    dest_cell.copy_values_from(src_cell, opt);
		    reflect_normal_velocity(dest_cell, IFace);
		    if ( GlobalConfig.MHD ) {
			reflect_normal_magnetic_field(dest_cell, IFace);
		    }
		} // end i loop
	    } // for k
	    break;
	case Face.west:
	    i = blk.imin;
	    for (k = blk.kmin; k <= blk.kmax; ++k) {
		for (j = blk.jmin; j <= blk.jmax; ++j) {
		    // ghost cell 1.
		    src_cell = blk.get_cell(i,j,k);
		    IFace = src_cell.iface[Face.west];
		    dest_cell = blk.get_cell(i-1,j,k);
		    dest_cell.copy_values_from(src_cell, opt);
		    reflect_normal_velocity(dest_cell, IFace);
		    if ( GlobalConfig.MHD ) {
			reflect_normal_magnetic_field(dest_cell, IFace);
		    }
		    // ghost cell 2.
		    src_cell = blk.get_cell(i+1,j,k);
		    dest_cell = blk.get_cell(i-2,j,k);
		    dest_cell.copy_values_from(src_cell, opt);
		    reflect_normal_velocity(dest_cell, IFace);
		    if ( GlobalConfig.MHD ) {
			reflect_normal_magnetic_field(dest_cell, IFace);
		    }
		} // end j loop
	    } // for k
	    break;
	case Face.top:
	    k = blk.kmax;
	    for (i = blk.imin; i <= blk.imax; ++i) {
		for (j = blk.jmin; j <= blk.jmax; ++j) {
		    // ghost cell 1.
		    src_cell = blk.get_cell(i,j,k);
		    IFace = src_cell.iface[Face.top];
		    dest_cell = blk.get_cell(i,j,k+1);
		    dest_cell.copy_values_from(src_cell, opt);
		    reflect_normal_velocity(dest_cell, IFace);
		    if ( GlobalConfig.MHD ) {
			reflect_normal_magnetic_field(dest_cell, IFace);
		    }
		    // ghost cell 2.
		    src_cell = blk.get_cell(i,j,k-1);
		    dest_cell = blk.get_cell(i,j,k+2);
		    dest_cell.copy_values_from(src_cell, opt);
		    reflect_normal_velocity(dest_cell, IFace);
		    if ( GlobalConfig.MHD ) {
			reflect_normal_magnetic_field(dest_cell, IFace);
		    }
		} // end j loop
	    } // for i
	    break;
	case Face.bottom:
	    k = blk.kmin;
	    for (i = blk.imin; i <= blk.imax; ++i) {
		for (j = blk.jmin; j <= blk.jmax; ++j) {
		    // ghost cell 1.
		    src_cell = blk.get_cell(i,j,k);
		    IFace = src_cell.iface[Face.bottom];
		    dest_cell = blk.get_cell(i,j,k-1);
		    dest_cell.copy_values_from(src_cell, opt);
		    reflect_normal_velocity(dest_cell, IFace);
		    if ( GlobalConfig.MHD ) {
			reflect_normal_magnetic_field(dest_cell, IFace);
		    }
		    // ghost cell 2.
		    src_cell = blk.get_cell(i,j,k+1);
		    dest_cell = blk.get_cell(i,j,k-2);
		    dest_cell.copy_values_from(src_cell, opt);
		    reflect_normal_velocity(dest_cell, IFace);
		    if ( GlobalConfig.MHD ) {
			reflect_normal_magnetic_field(dest_cell, IFace);
		    }
		} // end j loop
	    } // for i
	    break;
	} // end switch which_boundary
    } // end apply_convective()

    void apply_viscous(double t) {}  // does nothing

} // end class BoundaryCondition
