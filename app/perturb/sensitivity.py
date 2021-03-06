#!/usr/bin/env python
""" sensitivity.py

This script calculates the sensitivity and uncertainty in 
each nozzle exit flow property due to the various input
parameters. The calculation follows that of:
    Mee, D. (1993) 'Uncertainty Analysis of conditions 
    in the test section of the T4 Shock Tunnel', UQ 
    Mechanical Engineering Departmental Report No. 4/1993
and is based on a set of perturbed calculations 
generated by "perturb.py"
 
.. Author: Luke Doherty (luke.doherty@eng.ox.ac.uk)
           Osney Thermofluids Laboratory
           The University of Oxford
"""

VERSION_STRING = "04-January-2015"

import shlex, string
import sys, os
import optparse
import numpy as np #import array
np.seterr(divide='ignore',invalid='ignore')
from utils import run_command, read_case_summary
from sensitivity_input_utils import sensitivity_input_checker
#from nenzfr_utils import run_command, quote, read_case_summary, \
#     read_nenzfr_outfile, read_estcj_outfile
#from nenzfr_input_utils import input_checker, nenzfr_perturbed_input_checker, nenzfr_sensitivity_input_checker
E3BIN = os.path.expandvars("$HOME/e3bin")
sys.path.append(E3BIN)

#---------------------------------------------------------------
def run_sensitivity(cfg):
    """
    Function that accepts a config dictionary and runs sensitivity
    """
    
    #check inputs using original nenzfr input checker first

    cfg['bad_input'] = False

    cfg = input_checker(cfg)

    # add default pertubations and check new nenzfr perturbed inputs

    # Set the default relative perturbation values (as percentages)
    cfg['defaultPerturbations'] = {'p1':2.5, 'T1':2.5, 'Vs':2.5, 'pe':2.5,
                           'Tw':2.5, 'BLTrans':2.5, 'TurbVisRatio':2.5,
                           'TurbInten':2.5, 'CoreRadiusFraction':2.5}

    cfg = nenzfr_perturbed_input_checker(cfg)

    #Relative uncertainties below based on uncertainties in Rainer's thesis.

    cfg['default_rel_uncertainties'] = {'p1':0.0325, 'T1':0.02, 'Vs':0.05, 'pe':0.025,
                           'Tw':0.04, 'BLTrans':1.00, 'TurbVisRatio':1.00,
                           'TurbInten':0.8, 'CoreRadiusFraction':0.05}

    cfg = nenzfr_sensitivity_input_checker(cfg)

    #bail out here if there is an issue
    if cfg['bad_input']:
        return -2

    # Read the sensitivity_case_summary file to get the perturbed
    # variables and their various values
    perturbedVariables, DictOfCases = read_case_summary()

    # Pull the variable of the uncertainties for each perturbed variable from
    # the config dictionary.
    inputUncertainties = cfg['inputUncertainties']

    # Define the name of the nominal case and load the exit plane data
    nominal = 'case000'
    nominalData, exitVar = read_nenzfr_outfile('./'+nominal+'/'+\
                                               cfg['jobName']+'-exit.stats')
    # Load the nozzle supply data
    nominalSupply = read_estcj_outfile('./'+nominal+'/'+cfg['jobName']+'-estcj.dat')
    # Now add the relevant supply data (T, h) to the nominalData dictionary
    nominalData['supply_rho'] = nominalSupply['rho']
    nominalData['supply_T'] = nominalSupply['T']
    nominalData['supply_h'] = nominalSupply['h']
    # Add the supply variables to the exitVar list
    exitVar.insert(0,'supply_rho')
    exitVar.insert(1,'supply_T')
    exitVar.insert(2,'supply_h')
    # Add extra variables of interest (q, Re_u, m_dot)
    nominalData, exitVar = add_extra_variables(nominalData, exitVar)

    nominalValues = get_values(nominalData, exitVar)
    #print nominalValues

    # Loop through each of the perturbed variables
    sensitivity = {}
    sensitivity_abs = {}
    uncertainty = {}
    for k in range(len(perturbedVariables)):
        var = perturbedVariables[k]

        if var == 'CoreRadiusFraction':
            perturb_CoreRadiusFraction(var, perturbedVariables,\
                                       DictOfCases, cfg['levels'])
        # Define the name of the relevant perturbed cases and load the
        # associated data
        high = 'case'+"{0:02}".format(k)+'1'
        highData, dontNeed  = read_nenzfr_outfile('./'+high+'/'+\
                                                  cfg['jobName']+'-exit.stats')
        highSupply = read_estcj_outfile('./'+high+'/'+cfg['jobName']+'-estcj.dat')
        highData['supply_rho'] = highSupply['rho']
        highData['supply_T'] = highSupply['T']
        highData['supply_h'] = highSupply['h']
        highData, dontNeed = add_extra_variables(highData, [])

        low = 'case'+"{0:02}".format(k)+'2'
        lowData, dontNeed = read_nenzfr_outfile('./'+low+'/'+\
                                                cfg['jobName']+'-exit.stats')
        lowSupply = read_estcj_outfile('./'+low+'/'+cfg['jobName']+'-estcj.dat')
        lowData['supply_rho'] = lowSupply['rho']
        lowData['supply_T'] = lowSupply['T']
        lowData['supply_h'] = lowSupply['h']
        lowData, dontNeed = add_extra_variables(lowData, [])

        #print low, nominal, high

        # Values of the freestream properties at the perturbed conditions
        highValues = get_values(highData,exitVar)
        lowValues = get_values(lowData,exitVar)
        # Values of the perturbed input values
        highX = DictOfCases[high][perturbedVariables.index(var)]
        lowX = DictOfCases[low][perturbedVariables.index(var)]
        nominalX = DictOfCases[nominal][perturbedVariables.index(var)]

        if cfg['levels'] == 3:
            # As the perturbations may not be centered on the nominal
            # condition we caculate the gradient by taking a weighted
            # average of the forward and backward derivatives. The
            # weightings are such that the truncation error associated
            # with this gradient estimate is O(3) or higher (i.e. the
            # weightings are such that the second order terms in the Taylor
            # series expansion cancel. Thanks to D.Petty for this theory.)
            highWeighting = (nominalX - lowX)/(highX - lowX)
            lowWeighting = (highX - nominalX)/(highX - lowX)

            #sensitivity[var] = ( highWeighting*(np.array(highValues)-\
            #                                    np.array(nominalValues))/\
            #                                   (highX - nominalX) + \
            #                     lowWeighting*(np.array(nominalValues)-\
            #                                   np.array(lowValues))/\
            #                                  (nominalX - lowX)     )*\
            #                    nominalX/np.array(nominalValues)
            sensitivity_abs[var] = ( highWeighting*(np.array(highValues)-\
                                                np.array(nominalValues))/\
                                               (highX - nominalX) + \
                                 lowWeighting*(np.array(nominalValues)-\
                                               np.array(lowValues))/\
                                              (nominalX - lowX)     )

            sensitivity[var] = sensitivity_abs[var]*nominalX/np.array(nominalValues)
            #print sensitivity[var]
        elif cfg['levels'] == 5:
            # For 5 levels per variable we have additional cases
            # that need to be loaded. Again we do not assume that
            # the levels are equally spaced around the nominal.
            # The weightings are such that the truncation error
            # associated with this estimate is O(4) or higher.
            tooHigh = 'case'+"{0:02}".format(k)+'3'
            tooHighData,dontNeed = \
                  read_nenzfr_outfile('./'+tooHigh+'/'+cfg['jobName']+'-exit.stats')
            tooHighSupply = read_estcj_outfile('./'+tooHigh+'/'+cfg['jobName']+'-estcj.dat')
            tooHighData['supply_rho'] = tooHighSupply['rho']
            tooHighData['supply_T'] = tooHighSupply['T']
            tooHighData['supply_h'] = tooHighSupply['h']
            tooHighData, dontNeed = add_extra_variables(tooHighData, [])

            tooLow = 'case'+"{0:02}".format(k)+'4'
            tooLowData,dontNeed = \
                  read_nenzfr_outfile('./'+tooLow+'/'+cfg['jobName']+'-exit.stats')
            tooLowSupply = read_estcj_outfile('./'+tooLow+'/'+cfg['jobName']+'-estcj.dat')
            tooLowData['supply_rho'] = tooLowSupply['rho']
            tooLowData['supply_T'] = tooLowSupply['T']
            tooLowData['supply_h'] = tooLowSupply['h']
            tooLowData, dontNeed = add_extra_variables(tooLowData, [])

            #print tooHigh, high, nominal, low, tooLow

            # Values of the freestream properties at the perturbed conditions
            tooHighValues = get_values(tooHighData,exitVar)
            tooLowValues = get_values(tooLowData,exitVar)
            # Values of the perturbed input values
            tooHighX = DictOfCases[tooHigh][perturbedVariables.index(var)]
            tooLowX = DictOfCases[tooLow][perturbedVariables.index(var)]
            tooHighDeltaX = tooHighX - nominalX
            highDeltaX = highX - nominalX
            tooLowDeltaX = tooLowX - nominalX
            lowDeltaX = lowX - nominalX
            weighting = (tooHighX - tooLowX)/(highX - lowX)

            denom = 1/tooHighDeltaX - 1/tooLowDeltaX -\
                   (1/highDeltaX - 1/lowDeltaX)*weighting
            numer =  np.array(tooHighValues)/tooHighDeltaX**2 -\
                     np.array(tooLowValues)/tooLowDeltaX**2 -\
                    ( np.array(highValues)/highDeltaX**2 -\
                      np.array(lowValues)/lowDeltaX**2 )*weighting -\
                     np.array(nominalValues)*\
                       ( 1/tooHighDeltaX**2 - 1/tooLowDeltaX**2 - \
                         ( 1/highDeltaX**2 - 1/lowDeltaX**2 )*weighting )

            sensitivity_abs[var] = numer/denom
            sensitivity[var] = sensitivity_abs[var]*nominalX/np.array(nominalValues)

        # Now calculate the uncertainty in each exit flow variable
        # due to the uncertainty in the current (perturbed)
        # input variable
        uncertainty[var] = sensitivity[var]*inputUncertainties[var]

    # Write out a file of the sensitivities
    write_sensitivity_summary(sensitivity, perturbedVariables, exitVar, 'relative')
    write_sensitivity_summary(sensitivity_abs, perturbedVariables, exitVar, 'absolute')

    # Write out a file of the uncertainties
    write_uncertainty_summary(uncertainty, perturbedVariables, exitVar,\
                              inputUncertainties)
    
    return



def main():
    """
    Examine the command-line options and then calculate the sensitivties
    and uncertainties of each property based on a set of results generated 
    by "perturb.py".
    """
    op = optparse.OptionParser(version=VERSION_STRING)
    op.add_option('-c', '--config_file', dest='config_file',
                  help=("filename for the config file"))
    opt, args = op.parse_args()
    config_file = opt.config_file
    #   
    cfg = {}
    #
    if not cfg: #if the configuration dictionary has not been filled up already, load it from a file
        try: #from Rowan's onedval program
            execfile(config_file, globals(), cfg)
        except IOError as e:
	    print "Error {0}".format(str(e)) 
            print "There was a problem reading the config file: '{0}'".format(config_file)
            print "Check that it conforms to Python syntax."
            print "Bailing out!"
            sys.exit(1)
    #
    run_sensitivity(cfg)
    #         
    return

#---------------------------------------------------------------

if __name__ == '__main__':
    if len(sys.argv) <= 1:
        print "sensitivity:\n Calculate Sensitivity of Shock Tunnel Test Flow Conditions for a varying inputs"
        print "   Version:", VERSION_STRING
        print "   To get some useful hints, invoke the program with option --help."
        sys.exit(0)
    return_flag = main()
    sys.exit(return_flag)
