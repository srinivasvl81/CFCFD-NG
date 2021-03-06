// bc_standard_wall_fucntion.cxx
/// \brief apply wall fucntion to the cells near solid walls.
/// Reference: R.H. Nichols and C.C. Nelson, 
///            Wall function boundary conditions including heat transfer and compressibility
///            AIAA Journal, Vol.42 No.6, June 2004.
/// Kan Qin, September 2015

#include "../../../lib/util/source/useful.h"
#include "../../../lib/gas/models/gas_data.hh"
#include "../../../lib/gas/models/gas-model.hh"
#include "../../../lib/gas/models/physical_constants.hh"
#include "block.hh"
#include "kernel.hh"
#include "bc.hh"
#include <math.h>

// Wall function correction for adiabatic wall, 2D
void correction_adiabatic_wall_2D(FV_Cell *cell, FV_Cell *cell2, FV_Interface *IFace, size_t bc_type) 
{
    Gas_model *gmodel = get_gas_model_ptr();
        
    // local variables
    double cp, Pr, gas_constant, recovery;
    double cell_tangent, face_tangent;
    double rho_wall, T_wall, T_normal, P_wall;    
    double wall_dist, wall_dist2;
    double du;            
    double dudy;
    double mu_lam, mu_lam_1, mu_t, mu_coeff, y_white_y_plus;
    double tau_wall_old, tau_wall=0.0;
    double diff_tau, tolerance = 1.0e-10;
    double u_tau=0.0, u_plus=0.0;
    double Gam=0.0, Beta=0.0, Q=0.0, Phi=0.0;
    double y_plus_white=0.0, y_plus;
    size_t counter = 0;
    double tke, omega_i, omega_o, omega;    
    double reverse_flag = 1.0; 
    Vector3 local_tau_wall;     
    
    // Typical constants from boundary layer theory
    double kappa = 0.4;
    double B = 5.5;
    double C_mu = 0.09;       
    
    // Compute the reconvery factor
    cp = gmodel->Cp(*(cell->fs->gas));          
    Pr = cell->fs->gas->mu*cp/cell->fs->gas->k[0];
    gas_constant = gmodel->R(*(cell->fs->gas));
    recovery = pow(Pr,(1.0/3.0));   
     
    // Compute tangent velocity at the nearest interior point and wall interface
    cell->fs->vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);
    IFace->fs->vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);                
    cell_tangent = sqrt( pow(cell->fs->vel.y, 2.0) + pow(cell->fs->vel.z, 2.0) );                
    face_tangent = sqrt( pow(IFace->fs->vel.y, 2.0) + pow(IFace->fs->vel.z, 2.0) );                
    cell->fs->vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
    IFace->fs->vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
    
    // Compute normal distance of the interior point from the wall
    wall_dist = fabs( dot((cell->pos[0] - IFace->pos), IFace->n) );
    
    // compute the shear stess at the wall in the regular fashion by using the stress tensor on the surface.
    // This will provide the initial values to solve tau_wall iteratively
    du = fabs(cell_tangent - face_tangent);
    dudy = du/wall_dist;
    mu_lam = IFace->fs->gas->mu;
    mu_lam_1 = cell->fs->gas->mu;
    tau_wall_old = mu_lam * dudy;
    
    // Compute the wall temperature using the Crocco-Buseman equation
    P_wall = cell->fs->gas->p;
    T_normal = cell->fs->gas->T[0];
    T_wall = T_normal + recovery * pow(du,2.0) / ( 2.0*cp );
        // Calculate wall density using perfect equation of state
        // TODO: replace with non-ideal equation of state
    rho_wall = P_wall/(gas_constant*T_wall); 
    
    // Iteratively solve the corrected shear stress
    counter = 0; 
    diff_tau = 100.0;
    while (diff_tau > tolerance ) {
        
        // friction velocity and u+
        u_tau = sqrt(tau_wall_old/rho_wall);
        u_plus = du / u_tau;
        
        //  Gamma, Beta, Qm and Phi defined by Nichols & Nelson 2004
        Gam = recovery * u_tau * u_tau / ( 2.0*cp*T_wall );
        Beta = 0.0;  // for adiabatic wall only
        Q = sqrt( Beta*Beta + 4.0*Gam );
        Phi = asin(-1.0*Beta/Q);
        
        // Y+ defined by White and Christoph (compressibility and heat transfer)
        y_plus_white = exp((kappa/sqrt(Gam))*(asin((2.0*Gam*u_plus - Beta)/Q) - Phi))*exp(-1.0*kappa*B);
        
        // Spalding's universal form for the BL velocity with the outer velocity form of White & Christoph above
        y_plus = u_plus + y_plus_white - exp(-1.0*kappa*B)*( 1.0 + kappa*u_plus
                                                                 + pow((kappa*u_plus),2.0)/2.0
                                                                 + pow((kappa*u_plus),3.0)/6.0 );
         
        // Calculate an updated value for the wall shear stress                                                                 
        tau_wall = (1.0/rho_wall) * pow( y_plus*mu_lam/wall_dist,2.0 );
        
        // Difference between the old and new Tau. Update old value
        diff_tau = fabs(tau_wall-tau_wall_old);
        tau_wall_old += 0.25*(tau_wall-tau_wall_old);
        
        counter++;
        if (counter > 500) break;   
    }    
    
    reverse_flag = 1.0;
    if ( face_tangent  > cell_tangent ) reverse_flag = -1.0;
    // store this wall shear stress, will be used to replace the viscos stress
    if ( bc_type == 0 || bc_type == 1 || bc_type == 4 ) { // North, East and Top
        IFace->tau_wall = -1.0 * reverse_flag * tau_wall;                       
    } else { // South, West and Bottom
        IFace->tau_wall = reverse_flag * tau_wall; 
    }    
    IFace->q_wall = 0.0; // For adiabatic wall only    
    // transform wall shear stress back to the global frame of reference
    IFace->tau_wall_x = IFace->tau_wall * IFace->n.y;
    IFace->tau_wall_y = IFace->tau_wall * IFace->n.x;
    IFace->tau_wall_z = 0.0;     
                
    // Turbulence model boundary conditions
    y_white_y_plus = 2.0 * y_plus_white * kappa*sqrt(Gam)/Q
                     * pow( (1.0 - pow(2.0*Gam*u_plus-Beta,2.0)/(Q*Q)), -0.5 ); 
    mu_coeff = 1.0 + y_white_y_plus
               - kappa*exp(-1.0*kappa*B) * ( 1.0 + kappa*u_plus + kappa*u_plus*kappa*u_plus/2.0 )
               - mu_lam_1/mu_lam;
    mu_t = mu_lam * mu_coeff;
       // omega
    omega_i = 6.0*mu_lam / ( 0.075*rho_wall*wall_dist*wall_dist );
    omega_o = u_tau / ( sqrt(C_mu)*kappa*wall_dist );
    omega = sqrt( omega_i*omega_i + omega_o*omega_o );
       // tke
    tke = omega*mu_t/cell->fs->gas->rho; // negative tke values might happends at the initial stage
    if ( tke < 0.0 || isnan(tke) != 0 ) { 
        wall_dist2 = fabs( dot((cell2->pos[0] - IFace->pos), IFace->n) );
        tke = wall_dist*wall_dist / ( wall_dist2*wall_dist2 ) * cell2->fs->tke;
    }
                
    // store tke and omega for the first cell off the wall
    IFace->tke = tke;
    IFace->omega = omega;
    
    // store friction velocity u_tau and wall density rho_wall for post-processing
    IFace->u_tau = u_tau;
    IFace->rho_wall = rho_wall;          
}

// Wall function correction for fixed temperature wall, 2D
void correction_fixedt_wall_2D(FV_Cell *cell, FV_Cell *cell2, FV_Interface *IFace, size_t bc_type) 
{
    Gas_model *gmodel = get_gas_model_ptr();
        
    // local variables
    double cp, Pr, recovery;
    double cell_tangent, face_tangent;
    double rho_wall, T_wall, T_normal;    
    double wall_dist, wall_dist2;
    double du, dT;            
    double dudy, dTdy;
    double mu_lam, mu_lam_1, mu_t, mu_coeff, y_white_y_plus, k_lam;
    double tau_wall_old, tau_wall=0.0;
    double q_wall_old, q_wall=0.0;    
    double diff_tau, diff_q, tolerance = 1.0e-10;
    double u_tau=0.0, u_plus=0.0;
    double Gam=0.0, Beta=0.0, Q=0.0, Phi=0.0;
    double y_plus_white=0.0, y_plus=0.0;
    size_t counter = 0;
    double tke, omega_i, omega_o, omega; 
    double reverse_flag = 1.0; 
    Vector3 local_tau_wall;        
    
    // Typical constants from boundary layer theory
    double kappa = 0.4;
    double B = 5.5;
    double C_mu = 0.09;       
    
    // Compute the reconvery factor
    cp = gmodel->Cp(*(cell->fs->gas));          
    Pr = cell->fs->gas->mu*cp/cell->fs->gas->k[0];
    recovery = pow(Pr,(1.0/3.0));
    
    // Compute tangent velocity at the nearest interior point and wall interface
    cell->fs->vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);
    IFace->fs->vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);                
    cell_tangent = sqrt( pow(cell->fs->vel.y, 2.0) + pow(cell->fs->vel.z, 2.0) );                
    face_tangent = sqrt( pow(IFace->fs->vel.y, 2.0) + pow(IFace->fs->vel.z, 2.0) ); 
    cell->fs->vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
    IFace->fs->vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
    
    // Compute normal distance of the interior point from the wall
    wall_dist = fabs( dot((cell->pos[0] - IFace->pos), IFace->n) );
    
    // compute the shear stess at the wall in the regular fashion by using the stress tensor on the surface.
    // This will provide the initial values to solve tau_wall iteratively
    du = fabs(cell_tangent - face_tangent);
    dudy = du/wall_dist;
    mu_lam = IFace->fs->gas->mu;
    mu_lam_1 = cell->fs->gas->mu;
    tau_wall_old = mu_lam * dudy;               
    
    // Compute the wall temperature using the Crocco-Buseman equation
    T_normal = cell->fs->gas->T[0];
    T_wall = IFace->fs->gas->T[0];
    rho_wall = IFace->fs->gas->rho;
    
    // compute the heat flux at the wall in the regular fashion by using the stress tensor on the surface.
    // This will provide the initial values to solve q_wall iteratively
    dT = fabs(T_normal - T_wall);
    dTdy = dT/wall_dist;
    k_lam = IFace->fs->gas->k[0];
    q_wall_old = k_lam * dTdy;    
    
    // Iteratively solve the corrected shear stress and heat flux
    counter = 0; 
    diff_tau = 100.0; 
    diff_q = 100.0;
    while (diff_tau > tolerance && diff_q > tolerance) {
       
       // friction velocity and u+
       u_tau = sqrt(tau_wall_old/rho_wall);
       u_plus = du / u_tau;
       
       //  Gamma, Beta, Qm and Phi defined by Nichols & Nelson 2004
       Gam = recovery * u_tau * u_tau / ( 2.0*cp*T_wall );
       Beta = q_wall_old * mu_lam / ( rho_wall*T_wall*k_lam*u_tau );
       Q = sqrt( Beta*Beta + 4.0*Gam );
       Phi = asin(-1.0*Beta/Q);
       
       // Y+ defined by White and Christoph (compressibility and heat transfer)
       y_plus_white = exp((kappa/sqrt(Gam))*(asin((2.0*Gam*u_plus - Beta)/Q) - Phi))*exp(-1.0*kappa*B);
       
       // Spalding's universal form for the BL velocity with the outer velocity form of White & Christoph above
       y_plus = u_plus + y_plus_white - exp(-1.0*kappa*B)*( 1.0 + kappa*u_plus
                                                                + pow((kappa*u_plus),2.0)/2.0
                                                                + pow((kappa*u_plus),3.0)/6.0 );
       
       // Calculate an updated value for the wall shear stress                                                                 
       tau_wall = (1.0/rho_wall) * pow( y_plus*mu_lam/wall_dist,2.0 );
       
       // Difference between the old and new Tau. Update old value
       diff_tau = fabs(tau_wall-tau_wall_old);
       tau_wall_old += 0.25*(tau_wall-tau_wall_old);
       
       // Calculate an updated value for the wall heat flux
       Beta = ( T_normal/T_wall - 1.0 + Gam*u_plus*u_plus ) / u_plus;
       q_wall = Beta * ( rho_wall*T_wall*k_lam*u_tau ) / mu_lam;
       
       // Difference between the old and new q_wall, Update old value
       diff_q = fabs(q_wall-q_wall_old);
       q_wall_old += 0.25*(q_wall-q_wall_old);                    
       
       counter++;
       if (counter > 500) break;   
    }
    
    reverse_flag = 1.0;
    if ( face_tangent  > cell_tangent ) reverse_flag = -1.0;
    // store this wall shear stress, will be used to replace the viscos stress
    if ( bc_type == 0 || bc_type == 1 || bc_type == 4 ) { // North, East and Top
        IFace->tau_wall = -1.0 * reverse_flag * tau_wall;
        IFace->q_wall = -1.0 * q_wall;                        
    } else { // South, West and Bottom
        IFace->tau_wall = reverse_flag * tau_wall;
        IFace->q_wall = q_wall;            
    }
    // transform wall shear stress back to the global frame of reference
    // 2D
    IFace->tau_wall_x = IFace->tau_wall * IFace->n.y;
    IFace->tau_wall_y = IFace->tau_wall * IFace->n.x;
    IFace->tau_wall_z = 0.0;    
                
    // Turbulence model boundary conditions
    y_white_y_plus = 2.0 * y_plus_white * kappa*sqrt(Gam)/Q
                     * pow( (1.0 - pow(2.0*Gam*u_plus-Beta,2.0)/(Q*Q)), -0.5 ); 
    mu_coeff = 1.0 + y_white_y_plus
               - kappa*exp(-1.0*kappa*B) * ( 1.0 + kappa*u_plus + kappa*u_plus*kappa*u_plus/2.0 )
               - mu_lam_1/mu_lam;
    mu_t = mu_lam * mu_coeff;
       // omega
    omega_i = 6.0*mu_lam / ( 0.075*rho_wall*wall_dist*wall_dist );
    omega_o = u_tau / ( sqrt(C_mu)*kappa*wall_dist );
    omega = sqrt( omega_i*omega_i + omega_o*omega_o );
       // tke
    tke = omega*mu_t/cell->fs->gas->rho; // negative tke values might happends at the initial stage
    if ( tke < 0.0 || isnan(tke) != 0 ) { 
        wall_dist2 = fabs( dot((cell2->pos[0] - IFace->pos), IFace->n) );
        tke = wall_dist*wall_dist / ( wall_dist2*wall_dist2 ) * cell2->fs->tke;
    } 
                
    // store tke and omega for the first cell off the wall
    IFace->tke = tke;
    IFace->omega = omega;  
    
    // store friction velocity u_tau and wall density rho_wall for post-processing
    IFace->u_tau = u_tau;
    IFace->rho_wall = rho_wall;    
}

// Wall function correction for adiabatic wall, 3D
void correction_adiabatic_wall_3D(FV_Cell *cell, FV_Cell *cell2, FV_Interface *IFace, size_t bc_type) 
{
    Gas_model *gmodel = get_gas_model_ptr();
        
    // local variables
    double cp, Pr, gas_constant, recovery;
    double cell_tangent0, face_tangent0, cell_tangent1, face_tangent1;
    double rho_wall, T_wall, T_normal, P_wall;    
    double wall_dist, wall_dist2;
    double du, dudy;
    double mu_lam, mu_lam_1, mu_t, mu_coeff, y_white_y_plus;
    double tau_wall_old, tau_wall=0.0;    
    double diff_tau, tolerance = 1.0e-10;
    double u_tau=0.0, u_plus=0.0;
    double Gam=0.0, Beta=0.0, Q=0.0, Phi=0.0;
    double y_plus_white=0.0, y_plus=0.0;
    size_t counter = 0;
    double tke, omega_i, omega_o, omega; 
    double reverse_flag0 = 1.0, reverse_flag1 = 1.0; 
    double vt1_2_angle;
    Vector3 local_tau_wall;        
    
    // Typical constants from boundary layer theory
    double kappa = 0.4;
    double B = 5.5;
    double C_mu = 0.09;       
    
    // Compute the reconvery factor
    cp = gmodel->Cp(*(cell->fs->gas));          
    Pr = cell->fs->gas->mu*cp/cell->fs->gas->k[0];
    gas_constant = gmodel->R(*(cell->fs->gas));
    recovery = pow(Pr,(1.0/3.0));
    
    // Compute tangent velocity at the nearest interior point and wall interface
    cell->fs->vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);
    IFace->fs->vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);                
    cell_tangent0 = cell->fs->vel.y;
    cell_tangent1 = cell->fs->vel.z;
    face_tangent0 = IFace->fs->vel.y;
    face_tangent1 = IFace->fs->vel.z;
    cell->fs->vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
    IFace->fs->vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
    
    // Compute normal distance of the interior point from the wall
    wall_dist = fabs( dot((cell->pos[0] - IFace->pos), IFace->n) );
    
    // compute the shear stess at the wall in the regular fashion by using the stress tensor on the surface.
    // This will provide the initial values to solve tau_wall iteratively
    du = sqrt( pow((cell_tangent0-face_tangent0),2.0) + pow((cell_tangent1-face_tangent1),2.0) );
    dudy = du/wall_dist;
    mu_lam = IFace->fs->gas->mu;
    mu_lam_1 = cell->fs->gas->mu;
    tau_wall_old = mu_lam * dudy; 
    
    // Compute the wall temperature using the Crocco-Buseman equation
    P_wall = cell->fs->gas->p;
    T_normal = cell->fs->gas->T[0];
    T_wall = T_normal + recovery * pow(du,2.0) / ( 2.0*cp );
        // Calculate wall density using perfect equation of state
        // TODO: replace with non-ideal equation of state    
    rho_wall = P_wall/(gas_constant*T_wall);
    
    // Iteratively solve the corrected shear stress and heat flux
    counter = 0; 
    diff_tau = 100.0; 
    while (diff_tau > tolerance) {
       
       // friction velocity and u+
       u_tau = sqrt(tau_wall_old/rho_wall);
       u_plus = du / u_tau;
       
       //  Gamma, Beta, Qm and Phi defined by Nichols & Nelson 2004
       Gam = recovery * u_tau * u_tau / ( 2.0*cp*T_wall );
       Beta = 0.0; // for adiabatic wall
       Q = sqrt( Beta*Beta + 4.0*Gam );
       Phi = asin(-1.0*Beta/Q);
       
       // Y+ defined by White and Christoph (compressibility and heat transfer)
       y_plus_white = exp((kappa/sqrt(Gam))*(asin((2.0*Gam*u_plus - Beta)/Q) - Phi))*exp(-1.0*kappa*B);
       
       // Spalding's universal form for the BL velocity with the outer velocity form of White & Christoph above
       y_plus = u_plus + y_plus_white - exp(-1.0*kappa*B)*( 1.0 + kappa*u_plus
                                                                + pow((kappa*u_plus),2.0)/2.0
                                                                + pow((kappa*u_plus),3.0)/6.0 );
       
       // Calculate an updated value for the wall shear stress                                                                 
       tau_wall = (1.0/rho_wall) * pow( y_plus*mu_lam/wall_dist,2.0 );
       
       // Difference between the old and new Tau. Update old value
       diff_tau = fabs(tau_wall-tau_wall_old);
       tau_wall_old += 0.25*(tau_wall-tau_wall_old);              
       
       counter++;
       if (counter > 500) break;   
    }
    
    reverse_flag0 = 1.0;
    reverse_flag1 = 1.0;    
    if ( face_tangent0  > cell_tangent0 ) reverse_flag0 = -1.0;
    if ( face_tangent1  > cell_tangent1 ) reverse_flag1 = -1.0;    
    // store this wall shear stress, will be used to replace the viscos stress
    if ( bc_type == 0 || bc_type == 1 || bc_type == 4 ) { // North, East and Top
        IFace->tau_wall = -1.0 * tau_wall;              
    } else { // South, West and Bottom
        IFace->tau_wall = tau_wall; 
    }
    IFace->q_wall = 0.0; // adiabatic wall
    
    // transform wall shear stress back to the local frame of reference
    cell->fs->vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);
    IFace->fs->vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);    
    vt1_2_angle = atan2(fabs(cell->fs->vel.z - IFace->fs->vel.z), fabs(cell->fs->vel.y - IFace->fs->vel.y));
    cell->fs->vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
    IFace->fs->vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);       
         
    local_tau_wall.x = 0.0;        
    local_tau_wall.y = reverse_flag0 * IFace->tau_wall * cos(vt1_2_angle);
    local_tau_wall.z = reverse_flag1 * IFace->tau_wall * sin(vt1_2_angle); 
    local_tau_wall.transform_to_global(IFace->n, IFace->t1, IFace->t2);   
               
    IFace->tau_wall_x = local_tau_wall.x;
    IFace->tau_wall_y = local_tau_wall.y;    
    IFace->tau_wall_z = local_tau_wall.z;
                
    // Turbulence model boundary conditions
    y_white_y_plus = 2.0 * y_plus_white * kappa*sqrt(Gam)/Q
                     * pow( (1.0 - pow(2.0*Gam*u_plus-Beta,2.0)/(Q*Q)), -0.5 ); 
    mu_coeff = 1.0 + y_white_y_plus
               - kappa*exp(-1.0*kappa*B) * ( 1.0 + kappa*u_plus + kappa*u_plus*kappa*u_plus/2.0 )
               - mu_lam_1/mu_lam;
    mu_t = mu_lam * mu_coeff;
       // omega
    omega_i = 6.0*mu_lam / ( 0.075*rho_wall*wall_dist*wall_dist );
    omega_o = u_tau / ( sqrt(C_mu)*kappa*wall_dist );
    omega = sqrt( omega_i*omega_i + omega_o*omega_o );
       // tke
    tke = omega*mu_t/cell->fs->gas->rho; // negative tke values might happends at the initial stage
    if ( tke < 0.0 || isnan(tke) != 0 ) { 
        wall_dist2 = fabs( dot((cell2->pos[0] - IFace->pos), IFace->n) );
        tke = wall_dist*wall_dist / ( wall_dist2*wall_dist2 ) * cell2->fs->tke;
    }   
                
    // store tke and omega for the first cell off the wall
    IFace->tke = tke;
    IFace->omega = omega;  
    
    // store friction velocity u_tau and wall density rho_wall for post-processing
    IFace->u_tau = u_tau;
    IFace->rho_wall = rho_wall;
}

// Wall function correction for fixed temperature wall
void correction_fixedt_wall_3D(FV_Cell *cell, FV_Cell *cell2, FV_Interface *IFace, size_t bc_type) 
{
    Gas_model *gmodel = get_gas_model_ptr();
        
    // local variables
    double cp, Pr, recovery;
    double cell_tangent0, face_tangent0, cell_tangent1, face_tangent1;
    double rho_wall, T_wall, T_normal;    
    double wall_dist, wall_dist2;
    double du, dT;            
    double dudy, dTdy;
    double mu_lam, mu_lam_1, mu_t, mu_coeff, y_white_y_plus, k_lam;
    double tau_wall_old, tau_wall=0.0;
    double q_wall_old, q_wall=0.0;    
    double diff_tau, diff_q, tolerance = 1.0e-10;
    double u_tau=0.0, u_plus=0.0;
    double Gam=0.0, Beta=0.0, Q=0.0, Phi=0.0;
    double y_plus_white=0.0, y_plus=0.0;
    size_t counter = 0;
    double tke, omega_i, omega_o, omega; 
    double reverse_flag0 = 1.0, reverse_flag1 = 1.0; 
    double vt1_2_angle;
    Vector3 local_tau_wall;        
    
    // Typical constants from boundary layer theory
    double kappa = 0.4;
    double B = 5.5;
    double C_mu = 0.09;       
    
    // Compute the reconvery factor
    cp = gmodel->Cp(*(cell->fs->gas));          
    Pr = cell->fs->gas->mu*cp/cell->fs->gas->k[0];
    recovery = pow(Pr,(1.0/3.0));
    
    // Compute tangent velocity at the nearest interior point and wall interface
    cell->fs->vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);
    IFace->fs->vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);                
    cell_tangent0 = cell->fs->vel.y;
    cell_tangent1 = cell->fs->vel.z;
    face_tangent0 = IFace->fs->vel.y;
    face_tangent1 = IFace->fs->vel.z;
    cell->fs->vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
    IFace->fs->vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
    
    // Compute normal distance of the interior point from the wall
    wall_dist = fabs( dot((cell->pos[0] - IFace->pos), IFace->n) );
    
    // compute the shear stess at the wall in the regular fashion by using the stress tensor on the surface.
    // This will provide the initial values to solve tau_wall iteratively
    du = sqrt( pow((cell_tangent0-face_tangent0),2.0) + pow((cell_tangent1-face_tangent1),2.0) );
    dudy = du/wall_dist;
    mu_lam = IFace->fs->gas->mu;
    mu_lam_1 = cell->fs->gas->mu;
    tau_wall_old = mu_lam * dudy;               
    
    // Compute the wall temperature using the Crocco-Buseman equation
    T_normal = cell->fs->gas->T[0];
    T_wall = IFace->fs->gas->T[0];
    rho_wall = IFace->fs->gas->rho;
    
    // compute the heat flux at the wall in the regular fashion by using the stress tensor on the surface.
    // This will provide the initial values to solve q_wall iteratively
    dT = fabs(T_normal - T_wall);
    dTdy = dT/wall_dist;
    k_lam = IFace->fs->gas->k[0];
    q_wall_old = k_lam * dTdy;    
    
    // Iteratively solve the corrected shear stress and heat flux
    counter = 0; 
    diff_tau = 100.0; 
    diff_q = 100.0;
    while (diff_tau > tolerance && diff_q > tolerance) {
       
       // friction velocity and u+
       u_tau = sqrt(tau_wall_old/rho_wall);
       u_plus = du / u_tau;
       
       //  Gamma, Beta, Qm and Phi defined by Nichols & Nelson 2004
       Gam = recovery * u_tau * u_tau / ( 2.0*cp*T_wall );
       Beta = q_wall_old * mu_lam / ( rho_wall*T_wall*k_lam*u_tau );
       Q = sqrt( Beta*Beta + 4.0*Gam );
       Phi = asin(-1.0*Beta/Q);
       
       // Y+ defined by White and Christoph (compressibility and heat transfer)
       y_plus_white = exp((kappa/sqrt(Gam))*(asin((2.0*Gam*u_plus - Beta)/Q) - Phi))*exp(-1.0*kappa*B);
       
       // Spalding's universal form for the BL velocity with the outer velocity form of White & Christoph above
       y_plus = u_plus + y_plus_white - exp(-1.0*kappa*B)*( 1.0 + kappa*u_plus
                                                                + pow((kappa*u_plus),2.0)/2.0
                                                                + pow((kappa*u_plus),3.0)/6.0 );
       
       // Calculate an updated value for the wall shear stress                                                                 
       tau_wall = (1.0/rho_wall) * pow( y_plus*mu_lam/wall_dist,2.0 );
       
       // Difference between the old and new Tau. Update old value
       diff_tau = fabs(tau_wall-tau_wall_old);
       tau_wall_old += 0.25*(tau_wall-tau_wall_old);
       
       // Calculate an updated value for the wall heat flux
       Beta = ( T_normal/T_wall - 1.0 + Gam*u_plus*u_plus ) / u_plus;
       q_wall = Beta * ( rho_wall*T_wall*k_lam*u_tau ) / mu_lam;
       
       // Difference between the old and new q_wall, Update old value
       diff_q = fabs(q_wall-q_wall_old);
       q_wall_old += 0.25*(q_wall-q_wall_old);                    
       
       counter++;
       if (counter > 500) break;   
    }
    
    reverse_flag0 = 1.0;
    reverse_flag1 = 1.0;    
    if ( face_tangent0  > cell_tangent0 ) reverse_flag0 = -1.0;
    if ( face_tangent1  > cell_tangent1 ) reverse_flag1 = -1.0;    
    // store this wall shear stress, will be used to replace the viscos stress
    if ( bc_type == 0 || bc_type == 1 || bc_type == 4 ) { // North, East and Top
        IFace->tau_wall = -1.0 * tau_wall;
        IFace->q_wall = -1.0 * q_wall;                        
    } else { // South, West and Bottom
        IFace->tau_wall = tau_wall;
        IFace->q_wall = q_wall;            
    }
    // transform wall shear stress back to the local frame of reference
    cell->fs->vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);
    IFace->fs->vel.transform_to_local(IFace->n, IFace->t1, IFace->t2);    
    vt1_2_angle = atan2(fabs(cell->fs->vel.z - IFace->fs->vel.z), fabs(cell->fs->vel.y - IFace->fs->vel.y));
    cell->fs->vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);
    IFace->fs->vel.transform_to_global(IFace->n, IFace->t1, IFace->t2);       
         
    local_tau_wall.x = 0.0;        
    local_tau_wall.y = reverse_flag0 * IFace->tau_wall * cos(vt1_2_angle);
    local_tau_wall.z = reverse_flag1 * IFace->tau_wall * sin(vt1_2_angle); 
    local_tau_wall.transform_to_global(IFace->n, IFace->t1, IFace->t2);   
               
    IFace->tau_wall_x = local_tau_wall.x;
    IFace->tau_wall_y = local_tau_wall.y;    
    IFace->tau_wall_z = local_tau_wall.z;
                
    // Turbulence model boundary conditions
    y_white_y_plus = 2.0 * y_plus_white * kappa*sqrt(Gam)/Q
                     * pow( (1.0 - pow(2.0*Gam*u_plus-Beta,2.0)/(Q*Q)), -0.5 ); 
    mu_coeff = 1.0 + y_white_y_plus
               - kappa*exp(-1.0*kappa*B) * ( 1.0 + kappa*u_plus + kappa*u_plus*kappa*u_plus/2.0 )
               - mu_lam_1/mu_lam;
    mu_t = mu_lam * mu_coeff;
       // omega
    omega_i = 6.0*mu_lam / ( 0.075*rho_wall*wall_dist*wall_dist );
    omega_o = u_tau / ( sqrt(C_mu)*kappa*wall_dist );
    omega = sqrt( omega_i*omega_i + omega_o*omega_o );
       // tke
    tke = omega*mu_t/cell->fs->gas->rho; // negative tke values might happends at the initial stage
    if ( tke < 0.0 || isnan(tke) != 0 ) { 
        wall_dist2 = fabs( dot((cell2->pos[0] - IFace->pos), IFace->n) );
        tke = wall_dist*wall_dist / ( wall_dist2*wall_dist2 ) * cell2->fs->tke;
    }   
                
    // store tke and omega for the first cell off the wall
    IFace->tke = tke;
    IFace->omega = omega;  
    
    // store friction velocity u_tau and wall density rho_wall for post-processing
    IFace->u_tau = u_tau;
    IFace->rho_wall = rho_wall;
}

int wall_function_correction(Block &bd, size_t ftl)
{
    size_t i, j, k;
    FV_Cell *cell, *cell2;
    FV_Interface *IFace;
    global_data &G = *get_global_data_ptr();

    // NORTH boundary
    j =  bd.jmax;
    if ( bd.bcp[NORTH]->is_wall() && bd.bcp[NORTH]->type_code != SLIP_WALL ) {
	for ( i = bd.imin; i <= bd.imax; ++i ) {
            for ( k = bd.kmin; k <= bd.kmax; ++k ) {
 		cell = bd.get_cell(i,j,k);
 		cell2 = bd.get_cell(i,j-1,k); 		
 		IFace = cell->iface[NORTH];
 		if ( bd.bcp[NORTH]->type_code == ADIABATIC ) { // determine adiabatic wall
 		    if ( G.dimensions == 2 )  correction_adiabatic_wall_2D(cell, cell2, IFace, 0);
 		    if ( G.dimensions == 3 )  correction_adiabatic_wall_3D(cell, cell2, IFace, 0); 		    
 		} else { // fixed temperature wall
 		    if ( G.dimensions == 2 )  correction_fixedt_wall_2D(cell, cell2, IFace, 0);
 		    if ( G.dimensions == 3 )  correction_fixedt_wall_3D(cell, cell2, IFace, 0); 		    
 		}
            } // k-loop
	} // i-loop
    }
    
    // EAST boundary
    i = bd.imax;    
    if ( bd.bcp[EAST]->is_wall() && bd.bcp[EAST]->type_code != SLIP_WALL ) {
	for ( j = bd.jmin; j <= bd.jmax; ++j ) {
            for ( k = bd.kmin; k <= bd.kmax; ++k ) {
 		cell = bd.get_cell(i,j,k);
 		cell2 = bd.get_cell(i-1,j,k); 		
 		IFace = cell->iface[EAST];
 		if ( bd.bcp[EAST]->type_code == ADIABATIC ) { // determine adiabatic wall
 		    if ( G.dimensions == 2 )  correction_adiabatic_wall_2D(cell, cell2, IFace, 1);
 		    if ( G.dimensions == 3 )  correction_adiabatic_wall_3D(cell, cell2, IFace, 1); 		    
 		} else { // fixed temperature wall
 		    if ( G.dimensions == 2 )  correction_fixedt_wall_2D(cell, cell2, IFace, 1);
 		    if ( G.dimensions == 3 )  correction_fixedt_wall_3D(cell, cell2, IFace, 1); 		    
 		} 		
            } // k-loop
	} // j-loop
    }    

    // SOUTH boundary
    j =  bd.jmin;    
    if ( bd.bcp[SOUTH]->is_wall() && bd.bcp[SOUTH]->type_code != SLIP_WALL ) {
	for ( i = bd.imin; i <= bd.imax; ++i ) {
            for ( k = bd.kmin; k <= bd.kmax; ++k ) {
 		cell = bd.get_cell(i,j,k);
 		cell2 = bd.get_cell(i,j+1,k); 		
 		IFace = cell->iface[SOUTH];
 		if ( bd.bcp[SOUTH]->type_code == ADIABATIC ) { // determine adiabatic wall
 		    if ( G.dimensions == 2 )  correction_adiabatic_wall_2D(cell, cell2, IFace, 2);
 		    if ( G.dimensions == 3 )  correction_adiabatic_wall_3D(cell, cell2, IFace, 3); 		    
 		} else { // fixed temperature wall
 		    if ( G.dimensions == 2 )  correction_fixedt_wall_2D(cell, cell2, IFace, 2);
 		    if ( G.dimensions == 3 )  correction_fixedt_wall_3D(cell, cell2, IFace, 2); 		    
 		}
	    } // k-loop
	} // i-loop
    }

    // WEST boundary
    i = bd.imin;    
    if ( bd.bcp[WEST]->is_wall() && bd.bcp[WEST]->type_code != SLIP_WALL ) {
	for (j = bd.jmin; j <= bd.jmax; ++j) {
            for ( k = bd.kmin; k <= bd.kmax; ++k ) {
 		cell = bd.get_cell(i,j,k);
 		cell2 = bd.get_cell(i+1,j,k); 		
 		IFace = cell->iface[WEST];
 		if ( bd.bcp[WEST]->type_code == ADIABATIC ) { // determine adiabatic wall
 		    if ( G.dimensions == 2 )  correction_adiabatic_wall_2D(cell, cell2, IFace, 3);
 		    if ( G.dimensions == 3 )  correction_adiabatic_wall_3D(cell, cell2, IFace, 3); 		    
 		} else { // fixed temperature wall
 		    if ( G.dimensions == 2 )  correction_fixedt_wall_2D(cell, cell2, IFace, 3);
 		    if ( G.dimensions == 3 )  correction_fixedt_wall_3D(cell, cell2, IFace, 3);
 		} 		
            } // k-loop
	} // j-loop
    }

    if ( G.dimensions == 3 ) {
	// TOP boundary
	k = bd.kmax;	
	if ( bd.bcp[TOP]->is_wall() && bd.bcp[TOP]->type_code != SLIP_WALL ) {
	    for ( i = bd.imin; i <= bd.imax; ++i ) {
		for ( j = bd.jmin; j <= bd.jmax; ++j ) {
 		    cell = bd.get_cell(i,j,k);
 		    cell2 = bd.get_cell(i,j,k-1); 		    
 		    IFace = cell->iface[TOP];
 		    if ( bd.bcp[TOP]->type_code == ADIABATIC ) { // determine adiabatic wall
 		        if ( G.dimensions == 2 )  correction_adiabatic_wall_2D(cell, cell2, IFace, 4);
 		        if ( G.dimensions == 3 )  correction_adiabatic_wall_3D(cell, cell2, IFace, 4); 		        
 		    } else { // fixed temperature wall
 		        //correction_fixedt_wall(cell, cell2, IFace, 4);
 		        if ( G.dimensions == 2 )  correction_fixedt_wall_2D(cell, cell2, IFace, 4);
 		        if ( G.dimensions == 3 )  correction_fixedt_wall_3D(cell, cell2, IFace, 4); 		        
 		}  		    
		} // j-loop
	    } // i-loop
	}
        
	// BOTTOM boundary
	k = bd.kmin;	
	if ( bd.bcp[BOTTOM]->is_wall() && bd.bcp[BOTTOM]->type_code != SLIP_WALL ) {
	    for ( i = bd.imin; i <= bd.imax; ++i ) {
		for ( j = bd.jmin; j <= bd.jmax; ++j ) {
 		    cell = bd.get_cell(i,j,k);
 		    cell2 = bd.get_cell(i,j,k+1); 		    
 		    IFace = cell->iface[BOTTOM];
 		    if ( bd.bcp[BOTTOM]->type_code == ADIABATIC ) { // determine adiabatic wall
 		        if ( G.dimensions == 2 )  correction_adiabatic_wall_2D(cell, cell2, IFace, 5);
 		        if ( G.dimensions == 3 )  correction_adiabatic_wall_3D(cell, cell2, IFace, 5); 		        
 		    } else { // fixed temperature wall
 		        //correction_fixedt_wall(cell, cell2, IFace, 5);
 		        if ( G.dimensions == 2 )  correction_fixedt_wall_2D(cell, cell2, IFace, 5);
 		        if ( G.dimensions == 3 )  correction_fixedt_wall_3D(cell, cell2, IFace, 5); 		        
 		    }  		    
		} // j-loop
	    } // i-loop
	}
    } // end if G.dimensions == 3
    return SUCCESS;
} // end of wall_function_correction()

int apply_turbulent_model_for_wall_function(Block &bd)
{
    size_t i, j, k;
    FV_Cell *cell;
    FV_Interface *IFace;
    global_data &G = *get_global_data_ptr();

    // NORTH boundary
    j =  bd.jmax;    
    if ( bd.bcp[NORTH]->is_wall() && bd.bcp[NORTH]->type_code != SLIP_WALL ) {
	for ( i = bd.imin; i <= bd.imax; ++i ) {
            for ( k = bd.kmin; k <= bd.kmax; ++k ) {
 		cell = bd.get_cell(i,j,k);
 		IFace = cell->iface[NORTH];
                // set turbulence model boundary conditions for wall function
                cell->fs->tke = IFace->tke;
                cell->fs->omega = IFace->omega;                
            } // k-loop
	} // i-loop
    }

    // EAST boundary
    i = bd.imax;    
    if ( bd.bcp[EAST]->is_wall() && bd.bcp[EAST]->type_code != SLIP_WALL ) {
	for ( j = bd.jmin; j <= bd.jmax; ++j ) {
            for ( k = bd.kmin; k <= bd.kmax; ++k ) {
 		cell = bd.get_cell(i,j,k);
 		IFace = cell->iface[EAST];
                // set turbulence model boundary conditions for wall function
                cell->fs->tke = IFace->tke;
                cell->fs->omega = IFace->omega;  		
            } // k-loop
	} // j-loop
    }
    
    // SOUTH boundary
    j =  bd.jmin;    
    if ( bd.bcp[SOUTH]->is_wall() && bd.bcp[SOUTH]->type_code != SLIP_WALL ) {
	for ( i = bd.imin; i <= bd.imax; ++i ) {
            for ( k = bd.kmin; k <= bd.kmax; ++k ) {
 		cell = bd.get_cell(i,j,k);
 		IFace = cell->iface[SOUTH];
                // set turbulence model boundary conditions for wall function
                cell->fs->tke = IFace->tke;
                cell->fs->omega = IFace->omega;  		
	    } // k-loop
	} // i-loop
    }

    // WEST boundary
    i = bd.imin;    
    if ( bd.bcp[WEST]->is_wall() && bd.bcp[WEST]->type_code != SLIP_WALL ) {
	for (j = bd.jmin; j <= bd.jmax; ++j) {
            for ( k = bd.kmin; k <= bd.kmax; ++k ) {
 		cell = bd.get_cell(i,j,k);
 		IFace = cell->iface[WEST];
                // set turbulence model boundary conditions for wall function
                cell->fs->tke = IFace->tke;
                cell->fs->omega = IFace->omega;  		
            } // k-loop
	} // j-loop
    }

    if ( G.dimensions == 3 ) {
	// TOP boundary
	k = bd.kmax;	
	if ( bd.bcp[TOP]->is_wall() && bd.bcp[TOP]->type_code != SLIP_WALL ) {
	    for ( i = bd.imin; i <= bd.imax; ++i ) {
		for ( j = bd.jmin; j <= bd.jmax; ++j ) {
 		    cell = bd.get_cell(i,j,k);
 		    IFace = cell->iface[TOP];
                    // set turbulence model boundary conditions for wall function
                    cell->fs->tke = IFace->tke;
                    cell->fs->omega = IFace->omega;  		    
		} // j-loop
	    } // i-loop
	}
        
	// BOTTOM boundary
	k = bd.kmin;	
	if ( bd.bcp[BOTTOM]->is_wall() && bd.bcp[BOTTOM]->type_code != SLIP_WALL ) {
	    for ( i = bd.imin; i <= bd.imax; ++i ) {
		for ( j = bd.jmin; j <= bd.jmax; ++j ) {
 		    cell = bd.get_cell(i,j,k);
 		    IFace = cell->iface[BOTTOM];
                    // set turbulence model boundary conditions for wall function
                    cell->fs->tke = IFace->tke;
                    cell->fs->omega = IFace->omega;  		    
		} // j-loop
	    } // i-loop
	}
    } // end if G.dimensions == 3
    return SUCCESS;
} // end of apply_turbulent_model_for_wall_function()
