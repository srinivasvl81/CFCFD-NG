#! /usr/bin/env python
"""
pitot.py: Equilibrium expansion tube simulator

This program can be used to estimate the flow conditions for a shock-processed 
flow for an expansion tube. It is currently setup to work primarily with the 
X2 and X3 expansion tube's at the University of QLD, but I daresay it could be
expanded (excuse the pun) to use other facilities without too much effort.

The gas is assumed to remain in thermochemical equilibrium and the flow 
processing is done in decoupled quasi-one-dimensional wave processes such as 
shock waves and expansion fans.

The aim of the program is for it to be versatile, and it can do a number of
different calculations and configurations using different command line arguments:

* expansion tube with and without a pure He secondary driver tube and/or nozzle
* different driver conditions currently (100%He,80%He:20%Ar,90%He:10%Ar for X2, 60%He:40%Ar for X3)
* choosing shock speeds to give certain fill pressures in the tube
* choosing fill pressures to give certain shock speeds
* inputing experimental data of fill pressures and shock speeds to 'fill in the gaps' so to speak
* the code has been setup to do high total pressure conditions where a shock
  propagates into the shock tube from the secondary driver (instead of the usual
  expansion). I don't think this feature is working very well at the moment though.. 
 
When run as an application, this program takes its input as
command line arguments, performs the requested calculations and prints a table
with all of the results to the screen. Any changes you want to make can then 
be made to certain parameters, or you can quit the program. 
These results are also conveniently printed to a text file.

To see what specific inputs are required, start the program as::

$ pitot.py --help

Which particular input parameters you need to supply depends on the
chosen task, however, a fully theoretical basic x2 condition can be ran by using::

$ pitot.py --facility x2 --test fulltheory-pressure --config nozzle --driver_gas He:1.0 --test_gas air --p1 3000.0 --p5 10.0

due to the some of the default values this can be shortened down to this once 
you get the feel of it everything:

$ pitot.py --driver_gas He:1.0 --test_gas air --p1 3000.0 --p5 10.0
    
The full txt_output is a bit too much to include here, but you should see that the
stagnation enthalpy leaving the nozzle is 62.87 MJ/kg, and the velocity exiting
the nozzle (V in state 8) should be 10639.5 m/s.

A brief description of the states can be found below:
    
State 4: primary driver at the burst of the steel diaphragm (27.9MPa,2700K), 
    taken to be basically stagnated (M~0)
State 3s: primary driver after steady expansion to the throat mach number 
    (M=1 for 80%He, M=1.59 for 90%He,M=2.15 for 100%He, based on area ratio stuff done by Richard)

If secondary driver is used:
State sd1 : secondary driver fill condition (pure He)
State sd2: shocked secondary driver gas
State sd3: state 11 unsteadily expanding into the secondary driver tube

State 1: shock tube fill condition (test gas)
State 2: shocked test gas
State 3: state 11 or state sd2 unsteadily expanding into the shock tube

State 5: acceleration tube fill condition (air)
State 6: shocked acceleration tube gas
State 7: test gas unsteadily expanding into the acceleration tube

State 8: nozzle exit condition after steady expansion through the nozzle
State 10: post shock condition at the stagnation point of the model


Getting the program set up
--------------------------
pitot.py is not a stand-alone file.
It comes as part of the cfcfd3 compressible-flow collection and
depends upon functions from the cfpylib library to do the specific 
calculations.

Someday I'll make a proper makefile for this guy, but for now,
just make sure cfpylib is in your Python $PATH.

You may then call upon pitot.py as long as you have suitable
enviroment variables set, as per the installation instructions
for Eilmer3.


Some History
------------
Since the dawn of time, Richard Morgan used the program pitot to perform
basic calculations and create new expansion tube conditions.
The code was originally written in GWBasic, and as an exercise in 
programming (and to hopefully make the life of my colleagues and I
easier) I learnt some basic GWBasic syntax and ported the program to
Python at the start of 2012. It has since become an ongoing project
and this latest version has completely rebuilt the code from the 
ground up as a new program that makes use of the classes and functions
available to me as part of cfpylib inside the cfcfd code collection.

.. Author: Chris James (c.james4@uq.edu.au)
   The Centre for Hypersonics
   The University of QLD, St Lucia.

.. Versions:
   when dinosaurs were still around: RGM wrote the original code in GWBasic
   Summer 2011 - 2012: ported to Python as a direct copy by myself
   first half of 2012: added new parts to the code to make it more friendly
   October 2012: started full scale rebuild of code as an exercise in expansion
       tubes for myself at the start of my postgrad, and to build a program
       that is more useful for myself and others
    24-Oct-2012: changed notation of each state to fall in line with the notation
        used by David Gildfind in his work, I thought it made more sense as
        it means the shock tube and acc tubes always have the same notation
        (regardless of whether or not secondary driver is in place). Added one of
        my own conditions as an example too. Also added ability to use pressures
        and shock speeds from testing with pitot.
    30-Oct-2012: modified some of the notation to add x3 as a facility. Tested that all
        of the x2 simulations still worked, and then uploaded the new version.
    8-Nov-2012: added the final stuff for the scramjet conditions, I don't know
        how well it's working now, but for better or for worse, it's in there.
    29-Nov-2012: Did some little changes to the printout at the end of the test
        to make it clearer and some of the basic syntax.
    17-Dec-2012: Added two more test gases to the code.
    15-Jan-2013: added basic test time calculation from the work of Paull and Stalker (1992) to the code
    16-Jan-2013: added a second csv output to the code that should be handy for pulling the data into spreadsheets etc.
    17-Jan-2013: started implementing changes suggested by Peter Jacobs and Fabian Zander:
        Changed the variable 'state_over_model' to 'test_section_state' so it's less ambiguous.
        Did a general tidy up of my print statements and comments.
        Cut out the section at the end of the code where variables can be changed, the user can do this externally.
    22-Jan-2013: added perfect gas solver to the code so it can now do equilibrium and perfect gas calculations
    29-Jan-2013: added non-reflected shock tunnel mode to the code.
    29-Jan-2013: added a 'cea-printout' mode that does some of the default cfcfd condition printouts
    11-Feb-2013: turned off the clean-up lines (that remove temp files) as default and added
        a switch to turn it back on. (it will run faster if the program is not adding and removing 
        these files everytime it runs)
    12-Mar-2013: re-added CO2 to the code for perfect gas calculations, added Mars
        and Venus at the same time for various things. had to add extra values to their dictionaries 
        to make it work.
    14-Mar-2013: fixed a mistake at the end of the code where a total condition was being calculated 
        instead of a pitot one.
    15-Mar-2013: Added new solver condition 'pg'eq' that allows eq calculations to be done for CO2 
        based test gases that have issues with a fully equilibrium calculation. Also added Michael
        Scott's original x2 single stage piston as a selectable piston condition.
    05-Apr-2013: Did a major rethink of the program and changed the input
        so the program accepts input as either a configuration dictionary
        or variable specified in an external file that is then ran through Python.
        I also started cleaning up the code and pulling chunks of code out into
        functions so that the main() function is easier to follow.
    07-May-2013: Moved all of the functions out into separate python files that 
        are now imported. This was done to make this main file a lot cleaner.
    20-Aug-2013: Added new area ratio check mode that will run through various
        nozzle area ratios and output the changing freestream conditions in
        a csv file for analysis in a plotting program.
    02-Jan-2014: Added a skeleton version of a new condition builder mode that
        will run a series of the conditions and output a csv detailing
        the fill conditions and the resulting shock speeds and freestream
        properties. Also tidied up the code in general so that the printouts
        during each test run are easier to look at and understand.
"""

#--------------------- intro stuff --------------------------------------

#this import pulled from estcj with some additions

import sys, os, math
sys.path.append(os.path.expandvars("$HOME/e3bin")) # installation directory
sys.path.append("") # so that we can find user's scripts in current directory
from cfpylib.nm.zero_solvers import secant
# We base our calculation of gas properties upon calls to the NASA Glenn CEA code.
from cfpylib.gasdyn.cea2_gas import Gas, make_gas_from_name
from cfpylib.gasdyn.gas_flow import *
from cfpylib.gasdyn.ideal_gas_flow import p0_p, pitot_p

#to do some perfect gas stuff

import cfpylib.gasdyn.ideal_gas as pg

#import all of the other parts of pitot

from pitot_input_utils import *
from pitot_flow_functions import *
from pitot_output_utils import *
from pitot_area_ratio_check import *


VERSION_STRING = "08-Jan-2014"

DEBUG_PITOT = False

PRINT_STATUS = 1 #if print status is 1, some basic printouts are done

   
#----------------------------------------------------------------------------
    
def run_pitot(cfg = {}, config_file = None):
    """
    
    I pulled the guts of pitot out of the main() and into this run_pitot
    function so that pitot can more easily be modulated. A user can feed
    a cfg dictionary into the run_pitot function and run pitot without needing
    a cfg file at all.
    
    Chris James (c.james4@uq.edu.au) 13/05/13
    
    run_pitot(dict) - > depends
    
    """
    
    #---------------------- getting the inputs set up --------------------------
    
    if config_file:
        cfg = config_loader(config_file)
        
    #----------------- check inputs ----------------------------------------
    
    cfg = input_checker(cfg)
    
    start_message(cfg)
    
    cfg['VERSION_STRING'] = VERSION_STRING #add the version string the cfg dictionary
                                                        
    #---------------- building initial states ---------------------------------- 
            
    cfg, states, V, M = state_builder(cfg) #function above that builds all of the initial states based on info in the cfg dictionary
       
    #--------- unsteady expansion of driver gas-----------------------------
    if cfg['secondary']:
        print "Starting unsteady expansion of the driver gas into the secondary driver tube."
    else:
        print "Starting unsteady expansion of the driver gas into the shock tube."
    # For the unsteady expansion of the test driver into the tube, regulation of the amount
    # of expansion is determined by the shock-processed gas in the next section.
    # Across the contact surface between these gases, the pressure and velocity
    # have to match so we set up some trials of various pressures and check 
    # that velocities match.
    
    #----------------secondary driver calculation (if required)---------------
        
    if cfg['secondary']:
        cfg, states, V, M = secondary_driver_calculation(cfg, states, V, M)
           
    if cfg['secondary']:
        cfg['shock_tube_expansion'] = 'sd2' #state sd2 expands into shock tube
    else:
        cfg['shock_tube_expansion'] = 's3s' #otherwise state 3s is expanding into shock tube
        
    #--------------------- shock tube-------------------------------------------
    
    cfg, states, V, M = shock_tube_calculation(cfg, states, V, M)
    
    #--------------------- acceleration tube -----------------------------------
    
    if cfg['tunnel_mode'] == 'expansion-tube':
        try:
            cfg, states, V, M = acceleration_tube_calculation(cfg, states, V, M)
        except Exception as e:
            print "Error {0}".format(str(e))
            print "Acceleration tube calculation failed. Going to try it again with 'state7_no_ions' turned on."
            cfg['state7_no_ions'] = True
            print '-'*60
            try:
                cfg, states, V, M = acceleration_tube_calculation(cfg, states, V, M)
            except Exception as e:
                print "Error {0}".format(str(e))
                raise Exception, "pitot.run_pitot() Run of pitot has failed in the acceleration tube calculation."            
        
    #------------------- finishing off any other needed calculations -------------

    #setup the test section state (and do the nozzle expansion if required)
    cfg, states, V, M = test_section_setup(cfg, states, V, M)     
    
    #------------- do conehead calculation if asked to do so -----------
    
    if cfg['conehead']:
        cfg, states, V, M = conehead_calculation(cfg, states, V, M)
    
    #--------- do normal shock over model calculations if asked to ----------
    
    if cfg['shock_over_model']:
        cfg, states, V, M = shock_over_model_calculation(cfg, states, V, M)
            
    #-------------- if the normal shock thing was done, fix it up before print out ----------

    if cfg['test'] == "fulltheory-pressure" and cfg['shock_switch']: #restore the fill pressure's back to normal
        states['s1'].p = states['s1'].p*2.0
        if cfg['tunnel_mode'] == 'expansion-tube':
            states['s5'].p = states['s5'].p*2.0
        
    if cfg['test'] == "fulltheory-shock" and cfg['shock_switch']: #cut fill pressure's in half at the end
        states['s1'].p = states['s1'].p*2.0
        if cfg['tunnel_mode'] == 'expansion-tube':
            states['s5'].p = states['s5'].p*2.0
        
    #----------- test time calculations -------------------------------
    
    #see if we're able to calculate test time
    
    cfg = test_time_calculator(cfg, states, V)
    
    #------------------- perform output work ---------------------------
    if 'printout' in cfg['mode']:
        if PRINT_STATUS: print "Test completed. Printing output now."
        if PRINT_STATUS: print '-'*60
    if cfg['mode'] == 'printout' or cfg['mode'] == 'cea-printout':
        #do txt_output and csv output with cfcfd cea style printouts added if required
        cfg, states, V, M = txt_file_output(cfg, states, V, M)
        cfg, states, V, M = csv_file_output(cfg, states, V, M)
    elif cfg['mode'] == 'txt-printout' or cfg['mode'] == 'cea-txt-printout': 
        #just do txt printout
        cfg, states, V, M = txt_file_output(cfg, states, V, M)
    elif cfg['mode'] == 'csv-printout': #just do the csv printout
        cfg, states, V, M = csv_file_output(cfg, states, V, M)
    
    #-------------------- run the area ratio check if used to --------------

    if cfg['area_ratio_check']:
        cfg, states, V, M = area_ratio_check(cfg, states, V, M)
    
    #cleanup if we've been asked to

    if cfg['cleanup'] and cfg['solver'] == 'eq' \
    or cfg['cleanup'] and cfg['solver'] == 'pg-eq':
        cleanup_function()     
        
    return cfg, states, V, M               
    
#----------------------------------------------------------------------------
     
def main():
    
    import optparse  
    op = optparse.OptionParser(version=VERSION_STRING)   
    op.add_option('-c', '--config_file', dest='config_file',
                  help=("filename where the configuration file is located"))    

    opt, args = op.parse_args()
    config_file = opt.config_file
           
    run_pitot(cfg = {}, config_file = config_file)

#------------------------ running stuff----------------------------------------
                           
if __name__ == '__main__':
    if len(sys.argv) <= 1:
        print "pitot - Equilibrium expansion tube simulator"
        print "start with --help for help with inputs"
        print "There are a few demos here that you can run to see how the program works:"
        print "demo-s-eq: demo of Umar's high speed air condition where shock speeds are guessed to find the fill pressures used."
        print "demo-p-eq: equilibrium demo of Umar's high speed air condition where the fill pressures are specified."
        print "demo-p-pg: perfect gas demo of Umar's high speed air condition where the fill pressures are specified."
        print "hadas85-full-theory-eq: equilibrium fully theoretical demo of Hadas' 8.5km/s titan condition where fill pressures are specified."
        print "hadas85-full-theory-pg: perfect gas fully theoretical demo of Hadas' 8.5km/s titan condition where fill pressures are specified."
        print "hadas85-experiment-eq: equilibrium run through of Hadas' 8.5km/s titan condition with both fill pressures and shock speeds from xpt specified."
        print "hadas85-experiment-pg: perfect gas run through of Hadas' 8.5km/s titan condition with both fill pressures and shock speeds from xpt specified."
        print "chrishe-full-theory-eq: fully theoretical equilibrium demo of one of my He conditions with secondary driver."
        print "chrishe-full-theory-pg: fully theoretical perfect gas demo of one of my He conditions with secondary driver."
        print "dave-scramjet-p: a fully theoretical test of one of David Gildfind's scramjet conditions that iterates through fill pressures."
        print "dave-scramjet-s: a fully theoretical test of one of David Gildfind's scramjet conditions that iterates through shock speeds."
        print "dave-scramjet-tunnel: a recreation of one of Dave's scramjet conditions using test data."
        print "x3: basic x3 scramjet condition example."
        demo = raw_input("Type one of the demo commands for a demo run or anything else to quit ")
        
        if demo == 'demo-s-eq':
            print "This is an equilibrium demo of pitot recreating Umar Sheikh's high speed air condition where shock speeds are guessed to find the fill pressures used"
            print "Fill pressure's we are aiming for are p1 = 3000 Pa, p5 = 10 Pa"
            print " "
            cfg = {'solver':'eq', 'test':'fulltheory-shock', 
                   'facility':'x2', 'nozzle':True, 'secondary': False,
                   'driver_gas':'He:1.0', 'test_gas':'air',
                   'Vs1':5645.0,'Vs2':11600.0, 'filename':demo}
            run_pitot(cfg=cfg)
            
        elif demo == "demo-p-eq":
            print "This is equilibrium demo of pitot recreating Umar Sheikh's high speed air condition where fill pressures are specified."
            print " "   
            cfg = {'solver':'eq', 'test':'fulltheory-pressure', 
                   'facility':'x2', 'nozzle':True, 'secondary': False,
                   'driver_gas':'He:1.0', 'test_gas':'air',
                   'p1':3000.0,'p5':10.0, 'filename':demo}
            run_pitot(cfg=cfg)
            
        elif demo == "demo-p-pg":
            print "This is perfect gas demo of pitot recreating Umar Sheikh's high speed air condition where fill pressures are specified."
            print " "            
            cfg = {'solver':'pg', 'test':'fulltheory-pressure', 
                   'facility':'x2', 'nozzle':True, 'secondary': False,
                   'driver_gas':'He:1.0', 'test_gas':'air',
                   'p1':3000.0,'p5':10.0, 'filename':demo}
            run_pitot(cfg=cfg)
            
        elif demo == 'hadas85-full-theory-eq':
            print "This is the equilibrium fully theoretical demo of pitot recreating Hadas' 8.5 km/s titan condition."
            print " "
            cfg = {'solver':'eq', 'test':'fulltheory-pressure',
                   'mode':'cea-printout','conehead':True, 'area_ratio':3.0,
                   'facility':'x2', 'nozzle':True, 'secondary': False,
                   'driver_gas':'He:0.80,Ar:0.20', 'test_gas':'titan',
                   'p1':3200.0,'p5':10.0, 'filename':demo}
            run_pitot(cfg=cfg)
            
        elif demo == 'hadas85-full-theory-pg':
            print "This is the perfect gas fully theoretical demo of pitot recreating Hadas' 8.5 km/s titan condition."
            print " "
            cfg = {'solver':'pg', 'test':'fulltheory-pressure',
                   'mode':'cea-printout','conehead':True, 'area_ratio':3.0,
                   'facility':'x2', 'nozzle':True, 'secondary': False,
                   'driver_gas':'He:0.80,Ar:0.20', 'test_gas':'titan',
                   'p1':3200.0,'p5':10.0, 'filename':demo}
            run_pitot(cfg=cfg) 
            
        elif demo == 'hadas85-experiment-eq':
            print "This is the equilibrium demo of pitot recreating Hadas' 8.5 km/s titan condition with experimental shock speeds specified."
            print " "
            cfg = {'solver':'eq', 'test':'experiment',
                   'mode':'printout','conehead':False, 'area_ratio':3.0,
                   'facility':'x2', 'nozzle':True, 'secondary': False,
                   'driver_gas':'He:0.80,Ar:0.20', 'test_gas':'titan',
                   'p1':3200.0,'p5':10.0, 'Vs1': 4100.0, 'Vs2': 8620.0,
                   'filename':demo, 'expand_to':'shock-speed', 'expansion_factor':0.95}
            run_pitot(cfg=cfg) 
            
        elif demo == 'hadas85-experiment-pg':
            print "This is the perfect gas demo of pitot recreating Hadas' 8.5 km/s titan condition with experimental shock speeds specified."
            print " "
            cfg = {'solver':'pg', 'test':'experiment',
                   'mode':'printout','conehead':True, 'area_ratio':3.0,
                   'facility':'x2', 'nozzle':True, 'secondary': False,
                   'driver_gas':'He:0.80,Ar:0.20', 'test_gas':'titan',
                   'p1':3200.0,'p5':10.0, 'Vs1': 4100.0, 'Vs2': 8620.0,
                   'filename':demo}
            run_pitot(cfg=cfg) 
            
        elif demo == 'chrishe-full-theory-eq':
            print "This is the equilibrium demo of pitot recreating my 16 km/s 85%H2:15%He condition fully theoretically."
            print " "
            cfg = {'solver':'eq', 'test':'fulltheory-pressure',
                   'mode':'printout','conehead':True, 'area_ratio':2.5,
                   'facility':'x2', 'nozzle':True, 'secondary': True,
                   'driver_gas':'He:1.0', 'test_gas':'gasgiant_h215he',
                   'psd1':17500.0, 'p1':4700.0, 'p5':6.37, 
                   'shock_over_model':True, 'filename':demo}
            run_pitot(cfg=cfg)             
            
        elif demo == 'chrishe-full-theory-pg':
            print "This is the perfect gas demo of pitot recreating my 16 km/s 85%H2:15%He condition fully theoretically."
            print " "
            cfg = {'solver':'pg', 'test':'fulltheory-pressure',
                   'mode':'printout','conehead':True, 'area_ratio':2.5,
                   'facility':'x2', 'nozzle':True, 'secondary': True,
                   'driver_gas':'He:1.0', 'test_gas':'gasgiant_h215he',
                   'psd1':17500.0, 'p1':4700.0, 'p5':6.37, 
                   'shock_over_model':True, 'filename':demo}
            run_pitot(cfg=cfg)   
            
        elif demo == 'dave-scramjet-p':
            print "This is the demo of pitot recreating one of Dave Gildfind's scramjet conditions that iterates through fill pressures."
            print " "
            cfg = {'solver':'eq', 'test':'fulltheory-pressure',
                   'mode':'printout', 'area_ratio':2.5,
                   'facility':'x2', 'nozzle':True, 'secondary': True,
                   'driver_gas':'He:0.80,Ar:0.20', 'test_gas':'air',
                   'psd1':100000.0, 'p1':486000.0, 'p5':1500, 
                   'shock_switch':True, 'filename':demo}
            run_pitot(cfg=cfg)          
                    
        elif demo == 'dave-scramjet-s':
            print "This is the demo of pitot recreating one of Dave Gildfind's scramjet conditions that iterates through shock speeds."
            print " "
            cfg = {'solver':'eq', 'test':'fulltheory-shock',
                   'mode':'printout', 'area_ratio':2.5,
                   'facility':'x2', 'nozzle':True, 'secondary': True,
                   'driver_gas':'He:0.80,Ar:0.20', 'test_gas':'air',
                   'Vsd':4290.0, 'Vs1':1588.0, 'Vs2':3424.0, 
                   'filename':demo}
            run_pitot(cfg=cfg)     
                    
        elif demo == 'dave-scramjet-tunnel':
            print "This is the demo of pitot recreating one of Dave Gildfind's scramjet condition from test data."
            print " "
            
            cfg = {'solver':'eq', 'test':'experiment',
                   'mode':'printout', 'area_ratio':2.5,
                   'facility':'x2', 'nozzle':False, 'secondary': True,
                   'driver_gas':'He:0.80,Ar:0.20', 'test_gas':'air',
                   'Vsd':4178.0, 'Vs1':1417.0, 'Vs2':3264.0,
                   'psd1':100000.0, 'p1':690800.0, 'p5':288.2, 
                   'shock_switch':True, 'filename':demo}
            run_pitot(cfg=cfg)     
            
        elif demo == 'x3':
            print "This is the demo of pitot recreating one of the basic x3 conditions."
            print " "
            cfg = {'solver':'eq', 'test':'fulltheory-pressure',
                   'mode':'printout', 'area_ratio':2.5,
                   'facility':'x3', 'nozzle':False, 'secondary': True,
                   'driver_gas':'He:0.60,Ar:0.40', 'test_gas':'air',
                   'psd1':133000.0, 'p1':73000.0, 'p5':210.0, 
                   'shock_switch':True, 'filename':demo}
            run_pitot(cfg=cfg)              
                
    else:
        main()
