/** \file l_hist.cxx
 * \ingroup l1d3
 * \brief History Postprocessor for l1d.cxx.
 *
 * This program is used to read the x-location and 
 * individual-cell history data 
 * generated by the One-Dimensional Lagrangian code 
 * and then generate a GNU-Plot file.
 *
 * \author PA Jacobs
 *
 * \version 1.0  13-Jan-92
 */

/*-----------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "../../../lib/util/source/useful.h"
#include "l1d.hh"
#include "l_kernel.hh"
#include "l_io.hh"

// Don't try to fix these array issues...
// We really must replace these post-processing files with a python equivalent.
#ifndef NSPECD
#   define NSPECD 20
#endif
#define MAX_HIST_LOC 100

/*-----------------------------------------------------------------*/

int main(int argc, char **argv)
{
    double tstart, tstop;
    int i, j, maxsol;
    int itype;  /* 0 = x-location histories   */
                /* 1 = cell histories         */
    /*
     * x-location data
     */
    int hnloc;         /* number of history locations */
    double hxloc[MAX_HIST_LOC];  /* history locations */
    int ixloc;         /* which location              */
    double xx[MAX_HIST_LOC], rhox[MAX_HIST_LOC];
    double ux[MAX_HIST_LOC], ex[MAX_HIST_LOC];
    double px[MAX_HIST_LOC], ax[MAX_HIST_LOC];
    double Tx[MAX_HIST_LOC], tau0x[MAX_HIST_LOC];
    double qx[MAX_HIST_LOC], entropyx[MAX_HIST_LOC];
    double fx[NSPECD][MAX_HIST_LOC], f_sum;
    char pname[40];
    char oname[40], iname[40], base_file_name[32], name_tag[10];
    FILE * infile,  /* history file                 */
        *outfile;   /* plot file                    */

    double *sim_time;
    double *x, *rho, *u, *e, *p;
    double *a, *T, *pitot;
    double *h, *tau0, *q, *entropy;
    double *f[NSPECD];

    double M, gam, t1, t2;
    int count, c_temp;
    int ilogr, ilogp;   /* postprocessing options    */
#   define NCHAR 3200
    char line[NCHAR];   /* buffer for input lines    */
    int command_line_error;

    UNUSED_VARIABLE(hxloc);

    /*
     * Initialise and set defaults.
     */
    printf("\n-----------------------------------");
    printf("\nLagrangian 1D History Postprocessor");
    printf("\n-----------------------------------");
    printf("\n\n");

    strcpy(base_file_name, "default");
    maxsol = 2000000;
    tstart = 0.0;
    tstop = 0.0;
    ixloc = 0;
    hnloc = 1;
    ilogr = 0;
    ilogp = 0;
#   define  XLOCATION        0
#   define  PARTICULAR_CELL  1
    itype = XLOCATION;

    /*
     * Decode command line arguments.
     */
    command_line_error = 0;
    if (argc < 2) {
        /* no arguments supplied */
        command_line_error = 1;
        goto usage;
    }   /* end if */

    i = 1;
    while (i < argc) {
        /* process the next command-line argument */

        if (strcmp(argv[i], "-f") == 0) {
            /* Set the base file name. */
            i++;
            if (i >= argc) {
                command_line_error = 1;
                goto usage;
            }
            strcpy(base_file_name, argv[i]);
            i++;
            printf("Setting base_file_name = %s\n", base_file_name);
        } else if (strcmp(argv[i], "-tstart") == 0) {
            /* Set the target solution time. */
            i++;
            if (i >= argc) {
                command_line_error = 1;
                goto usage;
            }
            sscanf(argv[i], "%lf", &tstart);
            i++;
            printf("Setting tstart = %e\n", tstart);
        } else if (strcmp(argv[i], "-tstop") == 0) {
            /* Set the target solution time. */
            i++;
            if (i >= argc) {
                command_line_error = 1;
                goto usage;
            }
            sscanf(argv[i], "%lf", &tstop);
            i++;
            printf("Setting tstop = %e\n", tstop);
        } else if (strcmp(argv[i], "-maxsol") == 0) {
            /* Set the maximum number of data points. */
            i++;
            if (i >= argc) {
                command_line_error = 1;
                goto usage;
            }
            sscanf(argv[i], "%d", &maxsol);
            i++;
            printf("Set maxsol to %d\n", maxsol);
        } else if (strcmp(argv[i], "-xloc") == 0) {
            /* Select a particular x-location. */
            itype = XLOCATION;
            i++;
            if (i >= argc) {
                command_line_error = 1;
                goto usage;
            }
            sscanf(argv[i], "%d", &ixloc);
            i++;
            printf("Select xloc[%d]\n", ixloc);
        } else if (strcmp(argv[i], "-cell") == 0) {
            itype = PARTICULAR_CELL;
            i++;
            if (i >= argc) {
                command_line_error = 1;
                goto usage;
            }
            sscanf(argv[i], "%d", &ixloc);
            i++;
            printf("Select icell[%d]\n", ixloc);
        } else if (strcmp(argv[i], "-logrho") == 0) {
            ilogr = 1;
            i++;
            printf("Take logarithm of density.\n");
        } else if (strcmp(argv[i], "-logp") == 0) {
            ilogp = 1;
            i++;
            printf("Take logarithm of pressure.\n");
        } else if (strcmp(argv[i], "-help") == 0) {
            command_line_error = 1;
            printf("Printing usage message...\n\n");
            goto usage;
        } else {
            printf("Unknown option: %s\n", argv[i]);
            command_line_error = 1;
            goto usage;
        }   /* end if */
    }   /* end while */

    /*
     * If the command line arguments are incorrect,
     * write some advice to the console then finish.
     */
    usage:
    if (command_line_error == 1) {
        printf("Purpose:\n");
        printf("Pick up a history file <base_file_name>.hx,\n");
        printf("gather the data for one particular x-location or cell\n");
        printf("and write that data in a format suitable for GNU-Plot.\n");
        printf("Time is written to the first column, followed by\n");
        printf("x-position, density, etc.\n");
        printf("When reformatting, the density and pressure fields\n");
        printf("can be converted to logarithmic scales.\n");
        printf("Look at the head of the output file to see what\n");
        printf("other flow quantities are written.\n");
        printf("\n");

        printf("Command-line options: (defaults are shown in parentheses)\n");
        printf("-f <base_file_name>       (default)\n");
        printf("-tstart <time>            (0.0)\n");
        printf("-tstop  <time>            (0.0)\n");
        printf("-maxsol <n>               (20000)\n");
        printf("-xloc <ixloc>             (0)\n");
        printf("-cell <icell>             (0)\n");
        printf("-logrho                   (linear)\n");
        printf("-logp                     (linear)\n");
        printf("-help                     (print this message)\n");
        exit(1);
    }   /* end if command_line_error */

    strcpy(pname, base_file_name); strcat(pname, ".Lp");
    SimulationData SD = SimulationData(pname, 0);
    Gas_model *gmodel = get_gas_model_ptr();
    int nsp = gmodel->get_number_of_species();
    int nmodes = gmodel->get_number_of_modes();

    strcpy(iname, base_file_name);
    if (itype == PARTICULAR_CELL) {
        strcat(iname, ".hc");
    } else {
        strcat(iname, ".hx");
    }   /* end if */
    if ((infile = fopen(iname, "r")) == NULL) {
        printf("\nCould not open %s; BAILING OUT\n", iname);
        exit(1);
    }   /* end if */

    /*
     * Allocate memory in order to accumulate the history. 
     */
    printf( "Allocate memory for %d samples.\n", maxsol );
    sim_time = (double *) calloc(maxsol, sizeof(double));
    if ( sim_time == NULL ) {
        printf( "Failed to allocate memory for sim_time[]\n" );
        exit( -1 );
    }
    x = (double *) calloc(maxsol, sizeof(double));
    if ( x == NULL ) {
        printf( "Failed to allocate memory for x[]\n" );
        exit( -1 );
    }
    rho = (double *) calloc(maxsol, sizeof(double));
    if ( rho == NULL ) {
        printf( "Failed to allocate memory for rho[]\n" );
        exit( -1 );
    }
    u = (double *) calloc(maxsol, sizeof(double));
    if ( u == NULL ) {
        printf( "Failed to allocate memory for u[]\n" );
        exit( -1 );
    }
    e = (double *) calloc(maxsol, sizeof(double));
    if ( e == NULL ) {
        printf( "Failed to allocate memory for e[]\n" );
        exit( -1 );
    }
    p = (double *) calloc(maxsol, sizeof(double));
    if ( p == NULL ) {
        printf( "Failed to allocate memory for p[]\n" );
        exit( -1 );
    }
    a = (double *) calloc(maxsol, sizeof(double));
    if ( a == NULL ) {
        printf( "Failed to allocate memory for a[]\n" );
        exit( -1 );
    }
    T = (double *) calloc(maxsol, sizeof(double));
    if ( T == NULL ) {
        printf( "Failed to allocate memory for T[]\n" );
        exit( -1 );
    }
    pitot = (double *) calloc(maxsol, sizeof(double));
    if ( pitot == NULL ) {
        printf( "Failed to allocate memory for pitot[]\n" );
        exit( -1 );
    }
    h = (double *) calloc(maxsol, sizeof(double));
    if ( h == NULL ) {
        printf( "Failed to allocate memory for h[]\n" );
        exit( -1 );
    }
    tau0 = (double *) calloc(maxsol, sizeof(double));
    if ( tau0 == NULL ) {
        printf( "Failed to allocate memory for tau0[]\n" );
        exit( -1 );
    }
    q = (double *) calloc(maxsol, sizeof(double));
    if ( q == NULL ) {
        printf( "Failed to allocate memory for q[]\n" );
        exit( -1 );
    }
    entropy = (double *) calloc(maxsol, sizeof(double));
    if ( entropy == NULL ) {
        printf( "Failed to allocate memory for entropy[]\n" );
        exit( -1 );
    }
    for ( int isp = 0; isp < NSPECD; ++isp ) {
	f[isp] = (double *) calloc(maxsol, sizeof(double));
	if ( f[isp] == NULL ) {
	    printf( "Failed to allocate memory for f[%d][]\n", isp );
	    exit( -1 );
	}
    }

    /*
     * MAIN LOOP
     * 
     * Read the history file and store the requested data.
     */
    printf( "Begin main loop.\n" );
    count = 0;
    for (i = 1; i <= maxsol; ++i) {
        /* 
         * Read the x-location or cell data from a single line.
         * Test the reading of the time-stamp in case we try to
         * go past the end of the file.
         */
        if (fgets(line, NCHAR, infile) == NULL) {
            printf("Premature end of file.\n");
            break;
        }   /* end if */
        if (sscanf(line, "%lf %d %d %d", &(sim_time[count]), &hnloc, &nsp, &nmodes) != 4) {
            printf( "Invalid time-stamp, sol[%d].\n", i );
	    printf( "Stop reading with line:\n%s\n", line );
            break;
        }   /* end if */
	if ( nsp > NSPECD ) {
	    printf( "There are too many species for this program, as currently built.\n" );
	    printf( "NSPECD=%d nsp=%d\n", NSPECD, nsp );
	    printf( "Rebuild with larger NSPECD.\n" );
	    exit( BAD_INPUT_ERROR );
	}
        if (ixloc >= hnloc && i == 1) {
            printf("WARNING: Maximum cells/xlocns in file is %d.\n", hnloc);
            printf("Resetting selection to last one.\n");
            ixloc = hnloc - 1;
        }   /* end if */

        /*
         * If we reached this point, the time-stamp was OK so
         * assume that all of the following lines are present. 
         */
	LCell* icell = new LCell(gmodel);
        for (j = 0; j < hnloc; ++j) {
	    if ( fgets(line, NCHAR, infile) == NULL ) {
		printf("Problem reading file.\n");
		exit(BAD_INPUT_ERROR);
	    }
	    if ( icell->scan_cell_values_from_string(line) ) {
		printf("Cell failed to read from line:\n" );
		printf("%s\n", line);
		return FAILURE;
	    }
	    xx[j] = icell->xmid;
	    rhox[j] = icell->gas->rho;
	    ux[j] = icell->u;
	    ex[j] = icell->gas->e[0]; // FIX-ME -- should also process other modes
	    px[j] = icell->gas->p;
	    ax[j] = icell->gas->a;
	    Tx[j] = icell->gas->T[0]; // FIX-ME -- should also process other modes
	    tau0x[j] = icell->shear_stress;
	    qx[j] = icell->heat_flux;
	    entropyx[j] = icell->entropy;
	    fx[0][j] = icell->gas->massf[0];
	    f_sum = fx[0][j];
	    for ( int isp = 1; isp < nsp; ++isp ) {
		fx[isp][j] = icell->gas->massf[isp];
		f_sum += fx[isp][j];
	    }
            if ( rhox[j] > 1.0e-12 && fabs(f_sum - 1.0) > 0.001 ) {
		printf( "Mass fractions don't add up: %e.\n", f_sum );
            }
        } // end for (j = 0...

        x[count] = xx[ixloc];
        rho[count] = rhox[ixloc];
        u[count] = ux[ixloc];
        e[count] = ex[ixloc];
        p[count] = px[ixloc];
        a[count] = ax[ixloc];
        T[count] = Tx[ixloc];
        tau0[count] = tau0x[ixloc];
        q[count] = qx[ixloc];
        entropy[count] = entropyx[ixloc];
	for ( int isp = 0; isp < nsp; ++isp ) {
	    f[isp][count] = fx[isp][ixloc];
	}

        /*
         * Compute Pitot pressure.
         */
	if ( a[count] > 1.0e-12 ) {
	    M = fabs(u[count]) / a[count];
	} else {
	    M = 0.0;
	}
        gam = p[count] / (rho[count] * e[count]) + 1.0;
#       define  APPROX  0
#       if (APPROX == 1)
        /* Approximate pitot formula for high T air */
        pitot[count] = 0.92 * rho[count] * u[count] * u[count];
#       else
        /* Rayleigh Pitot formula */
        if (M > 1.0) {
            /* go through the shock */
            t1 = (gam + 1.0) * M * M / 2.0;
            t2 = (gam + 1.0) / (2.0 * gam * M * M - (gam - 1.0));
            pitot[count] = p[count] * pow(t1, (gam / (gam - 1.0))) *
                pow(t2, (1.0 / (gam - 1.0)));
        } else {
            /* Isentropic compression only */
            t1 = 1.0 + 0.5 * (gam - 1.0) * M * M;
            pitot[count] = p[count] * pow(t1, (gam / (gam - 1.0)));
        }   /* end if */
#       endif

        if (ilogp == 1)
            p[count] = log10(p[count]);
        if (ilogr == 1)
            rho[count] = log10(rho[count]);
        if (rho[count] > 1.0e-6) {
            h[count] = e[count] + p[count] / rho[count];
        } else {
            h[count] = 0.0;
        } // end if

        /*
         * Print some sort of status report every 100 time increments.
         */
        c_temp = count / 100;
        if ((c_temp * 100 == count) && (count != 0)) {
            printf("soln=%d, count=%d, t=%e\n", i - 1,
                   count, sim_time[count]);
        }

        /* 
         * We increment count now to indicate how many
         * history points have been stored.
         */
        ++count;

        /*
         * We shall normally stop on one of two conditions;
         * Reaching the final time or over filling the arrays.
         */
        if (sim_time[count - 1] >= tstop)
            break;
        if (count >= maxsol)
            break;

        /*
         * If we have not reached the specified time to start recording,
         * reset count so that the next data read overwrites the data
         * recently stored.
         */
        if (sim_time[count - 1] < tstart) {
            if (count == 1) {
                count = 0;
            } else {
                printf("Invalid attempt to restart count.\n");
                printf("There is something wrong with the history file.\n");
                printf("sim_time = %e, count = %d, i = %d\n",
                       sim_time[count - 1], count, i);
                exit(-1);
            }   /* end if */
        }   /* end if */
    }   /* end for i... */

    printf("Event count = %d, final t = %e\n", count, sim_time[count - 1]);

    if (infile != NULL)
        fclose(infile);

    /*
     * Build the output filename so that one set of data
     * is distinct from another.
     */
    strcpy(oname, base_file_name);
    if (itype == XLOCATION) {
        sprintf(name_tag, "_hx%d", ixloc);
    } else {
        sprintf(name_tag, "_hc%d", ixloc);
    }   /* end if */
    strcat(oname, name_tag);
    strcat(oname, ".dat");
    printf("Output file: %s\n", oname);
    if ((outfile = fopen(oname, "w")) == NULL) {
        printf("\nCould not open %s; BAILING OUT\n", oname);
        exit(1);
    }   /* end if */

    /* 
     * Write out the history data for a particular cell or x-location.
     * The first few lines indicate the contents of the file and,
     * at one time, was used in one of my plotting programs.
     * These lines are now comments so that the file can be used
     * directly by GNU-Plot.
     */
    fprintf( outfile, "# History Data for cell or x-location.\n");
    fprintf( outfile, "# %d          <== number of columns\n", (13 + nsp) );
    fprintf( outfile, "# t,s         <== col_1\n");
    fprintf( outfile, "# x,m         <== col_2\n");
    if (ilogr == 1) {
        fprintf( outfile, "# log10(rho)  <== col_3\n");
    } else {
        fprintf( outfile, "# rho,kg/m**3 <== col_3\n");
    }   /* end if */
    fprintf( outfile, "# u,m/s       <== col_4\n# e,J/k       <== col_5\n");
    if (ilogp == 1) {
        fprintf( outfile, "# log10(p)    <== col_6\n");
    } else {
        fprintf( outfile, "# p,Pa        <== col_6\n");
    }   /* end if */
    fprintf( outfile, "# a,m/s       <== col_7\n");
    fprintf( outfile, "# T,K         <== col_8\n");
    fprintf( outfile, "# h,J/kg      <== col_9\n");
    fprintf( outfile, "# Ppitot,Pa   <== col_10\n");
    fprintf( outfile, "# tau0,Pa     <== col_11\n");
    fprintf( outfile, "# q,W/m**2    <== col_12\n");
    fprintf( outfile, "# S2,J/kg/K   <== col_13\n");
    for ( int isp = 0; isp < nsp; ++isp ) {
        fprintf( outfile, "# f[%d]        <== col_%0d\n", isp, (13 + isp + 1) );
    }
    fprintf( outfile, "# %d           <== number of plotting blocks\n", 1);
    fprintf( outfile, "# %d        <== number of samples\n", count);
    for (i = 0; i < count; ++i) {
        fprintf(outfile, 
		"%e %e %e %e %e %e %e %e %e %e %e %e %e ",
                sim_time[i], x[i], rho[i], u[i], e[i],
                p[i], a[i], T[i], h[i], pitot[i], 
		tau0[i], q[i], entropy[i]);
	for ( int isp = 0; isp < nsp; ++isp ) {
	    fprintf( outfile, "%e ", f[isp][i] );
	}
	fprintf( outfile, "\n" );
    }   /* end for i... */

    if (outfile != NULL) fclose(outfile);

    /*
     * Clean up memory.
     */
    printf( "Free memory.\n" );
    if ( sim_time != NULL ) free( sim_time );
    if ( x != NULL ) free( x );
    if ( rho != NULL ) free( rho );
    if ( u != NULL ) free( u );
    if ( e != NULL ) free( e );
    if ( p != NULL ) free( p );
    if ( a != NULL ) free( a );
    if ( T != NULL ) free( T );
    if ( pitot != NULL ) free( pitot );
    if ( h != NULL ) free( h );
    if ( tau0 != NULL ) free( tau0 );
    if ( q != NULL ) free( q );
    if ( entropy != NULL ) free( entropy );
    for ( int isp = 0; isp < NSPECD; ++isp ) {
	if ( f[isp] != NULL ) free( f[isp] );
    }

    printf( "End program.\n" );
    return SUCCESS;
} // end main

