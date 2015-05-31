/** main.d
 * Eilmer4 compressible-flow simulation code, top-level function.
 *
 * Author: Peter J. and Rowan G. 
 * First code: 2015-02-05
 */

import core.memory;
import std.stdio;
import std.string;
import std.file;
import std.path;
import std.getopt;
import std.conv;
import std.parallelism;
import std.algorithm.comparison;

import geom;
import gas;
import globalconfig;
import simcore;

import util.lua;
import luaglobalconfig;
import luaflowstate;
import luageom;
import luagpath;
import luasurface;
import luavolume;
import luaunifunction;
import luasgrid;
import luasolidprops;

void main(string[] args)
{
    writeln("Eilmer4 compressible-flow simulation code.");
    writeln("Revision: PUT_REVISION_STRING_HERE");

    string msg = "Usage:                               Comment:\n";
    msg       ~= "e4shared [--job=<string>]            file names built from this string\n";
    msg       ~= "         [--prep]                    prepare config, grid and flow files\n";
    msg       ~= "         [--run]                     run the simulation over time\n";
    msg       ~= "         [--tindx=<int>]             defaults to 0\n";
    msg       ~= "         [--max-cpus=<int>]          defaults to ";
    msg       ~= to!string(totalCPUs) ~" on this machine\n";
    msg       ~= "         [--verbosity=<int>]         defaults to 0\n";
    msg       ~= "         [--max-wall-clock=<int>]    in seconds\n";
    msg       ~= "         [--help]                    writes this message\n";
    if ( args.length < 2 ) {
	writeln("Too few arguments.");
	write(msg);
	exit(1);
    }
    string jobName = "";
    int verbosityLevel = 0;
    bool prepFlag = false;
    bool runFlag = false;
    int tindxStart = 0;
    int maxWallClock = 5*24*3600; // 5 days default
    bool helpWanted = false;
    int maxCPUs = totalCPUs;
    try {
	getopt(args,
	       "job", &jobName,
	       "verbosity", &verbosityLevel,
	       "prep", &prepFlag,
	       "run", &runFlag,
	       "tindx", &tindxStart,
	       "max-wall-clock", &maxWallClock,
	       "max-cpus", &maxCPUs,
	       "help", &helpWanted
	       );
    } catch (Exception e) {
	writeln("Problem parsing command-line options.");
	writeln("Arguments not processed: ");
	args = args[1 .. $]; // Dispose of program name in first argument.
	foreach (myarg; args) writeln("    arg: ", myarg);
	write(msg);
	exit(1);
    }
    if (helpWanted) {
	write(msg);
	exit(0);
    }
    if (jobName.length == 0) {
	writeln("Need to specify a job name.");
	write(msg);
	exit(1);
    }
    if (prepFlag) {
	writeln("Start lua connection.");
	// GC.disable(); // To avoid segfaults, just for the preparation via Lua.
	auto L = luaL_newstate();
	luaL_openlibs(L);
	registerVector3(L);
	registerGlobalConfig(L);
	registerFlowState(L);
	registerPaths(L);
	registerSurfaces(L);
	registerVolumes(L);
	registerUnivariateFunctions(L);
	registerStructuredGrid(L);
	registerSolidProps(L);
	luaL_dofile(L, toStringz(dirName(thisExePath())~"/prep.lua"));
	luaL_dofile(L, toStringz(jobName~".lua"));
	luaL_dostring(L, toStringz("build_job_files(\""~jobName~"\")"));
	writeln("Done.");
	// GC.enable();
    }
    if (runFlag) {
	GlobalConfig.base_file_name = jobName;
	maxCPUs = min(max(maxCPUs, 1), totalCPUs); // don't ask for more than available
	if (verbosityLevel > 0) {
	    writeln("Begin simulation with command-line arguments.");
	    writeln("  jobName: ", jobName);
	    writeln("  tindxStart: ", tindxStart);
	    writeln("  maxWallClock: ", maxWallClock);
	    writeln("  verbosityLevel: ", verbosityLevel);
	    writeln("  maxCPUs: ", maxCPUs);
	}
	GlobalConfig.verbosity_level = verbosityLevel;
	
	init_simulation(tindxStart, maxCPUs, maxWallClock);
	writeln("starting simulation time= ", simcore.sim_time);
	if (GlobalConfig.block_marching) {
	    march_over_blocks();
	} else {
	    integrate_in_time(GlobalConfig.max_time);
	}
	finalize_simulation();
    }
    writeln("Done.");
} // end main()


