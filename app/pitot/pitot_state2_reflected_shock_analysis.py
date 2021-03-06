#! /usr/bin/env python
"""
pitot_state2_reflected_shock_analysis.py: pitot end of shock tube reflected shock analysis tool

Sometimes, the thin diaphragm leaving the shock tube can be modelling with a shock reflection
at the diaphragm. This tool is build to do that and show what happens through the range from a
Mach wave (M = 1 ie nothing) up to a full reflected shock at the end of the shock tube.

Chris James (c.james4@uq.edu.au) - 22/12/15

"""

VERSION_STRING = "18-May-2016"

from pitot_condition_builder import stream_tee

import sys

from pitot import run_pitot
from pitot_input_utils import *

def check_new_inputs(cfg):
    """Takes the input file and checks that the extra inputs required for the
       gas giant diluent analysis are working..
    
       Returns the checked over input file and will tell the bigger program to 
      bail out if it finds an issue.
    
    """
    
    print "Starting check of end of shock tube reflected shock analysis specific inputs."
    
    cfg['bad_input'] = False
        
    if 'rs_out_of_st' not in cfg:
        print "'rs_out_of_st' variable is not in cfg. Added it = True now."
        cfg['rs_out_of_st'] = True
    
    if cfg['rs_out_of_st'] != True:
        print "'rs_out_of_st' variable is not set to True. Setting to = True now."
        cfg['rs_out_of_st'] = True
        
    if 'Mr_step' not in cfg:
        print "'Mr_step' variable is not in cfg. Setting to default value of 0.1 now."
        cfg['Mr_step'] = 0.1
                       
    if 'normalise_results_by' not in cfg:
        print "'normalise_results_by' not in cfg."
        print "Setting it to 'first value'."
        cfg['normalise_results_by'] = 'first value'  
        
    if 'cleanup_old_files' not in cfg:
        print "'cleanup_old_files' variable not set. Setting to default value of 'False'"
        cfg['cleanup_old_files'] = False
                
    if cfg['bad_input']: #bail out here if you end up having issues with your input
        print "Config failed check. Bailing out now."
        print '-'*60
        sys.exit(1)
        
    if not cfg['bad_input']:
        print "Extra input check completed."
        
    return cfg
    
def calculate_number_of_test_runs(cfg):
    """Function that uses a simple function to calculate the amount of test
       runs that the program has to perform.
    """   
    return len(cfg['Mr_st_list'])
    
def start_message(cfg):
    """Function that takes the cfg file and prints a start message for the
       program, detailing what it will do.
    """
    
    # print how many tests we're going to run, and the ranges.
    
    print '-'*60    
    print "{0} tests will be run.".format(cfg['number_of_test_runs'])
                 
    return cfg
    
def build_results_dict(cfg):
    """Function that looks at the cfg dictionary and works out what data needs
       to be stored for the type of test that we're running. Then it populates
       a dictionary called 'results' with empty lists for the data to be stored in.
       The dictionary is then returned.
    """
    
    # need to make a list to create a series of empty lists in the results
    # dictionary to store the data. the list is tailored to the test we're running
    
    full_list = []
    
    if cfg['secondary']:    
        basic_list = ['test number','Mr_st','psd1','p1','p5','Vsd',
                      'Vs1', 'Vs2', 'Ht','h','u_eq', 
                      'p2','T2','rho2','V2','M2', 'a2', 'gamma2', 'R2', 'Ht2',
                      'p2r','T2r','rho2r','V2r','M2r', 'a2r', 'gamma2r', 'R2r', 'Ht2r',                      
                      'p6','T6','rho6','V6','M6','p7','T7','rho7','V7','M7']
    else:
        basic_list = ['test number','Mr_st','p1','p5',
                      'Vs1', 'Vs2', 'Ht','h','u_eq',
                      'p2','T2','rho2','V2','M2', 'a2', 'gamma2', 'R2', 'Ht2',
                      'p2r','T2r','rho2r','V2r','M2r', 'a2r', 'gamma2r', 'R2r', 'Ht2r',    
                      'p6','T6','rho6','V6','M6','p7','T7','rho7','V7','M7']
    full_list += basic_list
    
    if cfg['nozzle']:
        nozzle_list = ['arearatio','p8','T8','rho8','V8','M8']
        full_list += nozzle_list
    if cfg['conehead']:
         conehead_list = ['p10c','T10c','rho10c','V10c']
         full_list += conehead_list
    if cfg['shock_over_model']:
        shock_over_model_list = ['p10f','T10f','rho10f','V10f',
                                 'p10e','T10e','rho10e','V10e']
        full_list += shock_over_model_list

    # now populate the dictionary with a bunch of empty lists based on that list

    results = {title : [] for title in full_list}
    
    # add the list of titles in case we want to use it in future
    
    results['full_list'] = full_list
        
    #add a list where we can store unsuccesful run numbers for analysis
    results['unsuccessful_runs'] = []
    
    return results
    
def pitot_state2_reflected_shock_analysis_test_run(cfg, results):
    """Function that takes the fully built config dictionary
       and the text file that is being used for the program output
       and does the test run then adds a line to the output file.
    """
    
    condition_status = True #This will be turned to False if the condition fails
    
    cfg['filename'] = cfg['original_filename'] + '-test-{0}'.format(cfg['test_number'])
    
    # some code here to make a copy of the stdout printouts for each test and store it
    
    import sys
    
    test_log = open(cfg['filename']+'-log.txt',"w")
    sys.stdout = stream_tee(sys.stdout, test_log)
    
    print '-'*60
    print "Running test {0} of {1}.".format(cfg['test_number'], cfg['number_of_test_runs'])
    print "Current Mr_st = {0:.2f}.".format(cfg['Mr_st'])
    try:
        cfg, states, V, M = run_pitot(cfg = cfg)
    except Exception as e:
         print "Error {0}".format(str(e))
         print "Test {0} failed. Result will not be printed to csv output.".format(cfg['test_number'])
         condition_status = False
    if cfg['secondary'] and cfg['Vsd'] > cfg['Vs1']:
        print "Vsd is faster than Vs1, condition cannot be simulated by Pitot properly."
        print "Test {0} is considered failed, and result will not be printed to csv output.".format(cfg['test_number'])
        condition_status = False
    if condition_status:
        results = add_new_result_to_results_dict(cfg, states, V, M, results)
        # need to remove Vs values from the dictionary or it will bail out
        # on the next run
        if cfg['secondary']: cfg.pop('Vsd') 
        cfg.pop('Vs1'); cfg.pop('Vs2')
    else:
        # need to remove Vs values from the dictionary or it will bail out
        # on the next run         
        if cfg['secondary'] and 'Vsd' in cfg: cfg.pop('Vsd')
        if 'Vs1' in cfg: cfg.pop('Vs1') 
        if 'Vs2' in cfg: cfg.pop('Vs2')
        cfg['last_run_successful'] = False
        results['unsuccessful_runs'].append(cfg['test_number'])
        
    # now we end the stream teeing here by pulling out the original stdout object
    # and overwriting the stream tee with that, then closing the log file
    sys.stdout = sys.stdout.stream1   
    test_log.close()
            
    return condition_status, results
    
def results_csv_builder(results, test_name = 'pitot_run',  intro_line = None):
    """Function that takes the final results dictionary (which must include a 
       list called 'full_list' that tells this function what to print and in 
       what order) and then outputs a results csv. It will also add an intro line
       if a string with that is added. The name of the test is also required.
    """
    
    # open a file to start saving results
    with open(test_name + '-state2-reflected-shock-analysis.csv',"w") as condition_builder_output:
       
        # print a line explaining the results if the user gives it
        if intro_line:
            intro_line_optional = "# " + intro_line
            condition_builder_output.write(intro_line_optional + '\n')
        
        #now we'll make the code build us the second intro line
        intro_line = '#'
        for value in results['full_list']:
            if value != results['full_list'][-1]:
                intro_line += "{0},".format(value)
            else: #don't put the comma if it's the last value
                intro_line += "{0}".format(value)
            
        condition_builder_output.write(intro_line + '\n')
        
        # now we need to go through every test run and print the data.
        # we'll use 'full_list' to guide our way through
        
        # get the number of the test runs from the length of the first data list mentioned
        # in 'full_list'. need to assume the user hasn't screwed up and got lists of
        # different lengths
        number_of_test_runs = len(results[results['full_list'][0]])
        
        for i in range(0, number_of_test_runs, 1):
            output_line = ''
            for value in results['full_list']:
                if value != results['full_list'][-1]:
                    output_line += "{0},".format(results[value][i])
                else: #don't put the comma if it's the last value in the csv
                    output_line += "{0}".format(results[value][i])
            
            condition_builder_output.write(output_line + '\n')  
    
        condition_builder_output.close()              
                                  
    return
    
def normalised_results_csv_builder(results, test_name = 'pitot_run',  
                                   intro_line = None, normalised_by = 'first value'):
    """Function that takes the final results dictionary (which must include a 
       list called 'full_list' that tells this function what to print and in 
       what order) and then outputs a normalised version of the results csv.
       You can tell it to normalise by other values, but 'first value' is default.
       
       It will also add an intro line if a string with that is added. 
       The name of the test is also required.
    """
    
    # open a file to start saving results
    with open(test_name + '-state2-reflected-shock-analysis-normalised.csv',"w") as condition_builder_output: #csv_output file creation
    
        # print a line explaining the results if the user gives it
        if intro_line:
            intro_line_optional = "# " + intro_line
            condition_builder_output.write(intro_line_optional + '\n')
            
        normalised_intro_line = "# all variables normalised by {0}".format(normalised_by)
        condition_builder_output.write(normalised_intro_line + '\n')
        
        # 'test number' and 'diluent percentage' and the species concentrations
        # will not be normalised
        normalise_exceptions = ['test number', 'Mr_st']
        
        #now we'll make the code build us the second intro line
        intro_line = '#'
        for value in results['full_list']:
            if value != results['full_list'][-1]:
                if value in normalise_exceptions:
                    intro_line += "{0},".format(value)
                else:
                    intro_line += "{0} normalised,".format(value)
            else: #don't put the comma if it's the last value
                if value in normalise_exceptions:
                    intro_line += "{0}".format(value)
                else:
                    intro_line += "{0} normalised".format(value)
            
        condition_builder_output.write(intro_line + '\n')
        
        # now we need to go through every test run and print the data.
        # we'll use 'full_list' to guide our way through
        
        # get the number of the test runs from the length of the first data list mentioned
        # in 'full_list'. need to assume the user hasn't screwed up and got lists of
        # different lengths
        number_of_test_runs = len(results[results['full_list'][0]])
        
        # build a dictionary to store all of our normalisation values
        normalising_value_dict = {}
        
        for value in  results['full_list']:
            if normalised_by == 'first value':
                normalising_value_dict[value] = results[value][0]
            elif normalised_by == 'maximum value':
                normalising_value_dict[value] = max(results[value])
            elif normalised_by == 'last value':
                normalising_value_dict[value] = results[value][-1]       
        
        for i in range(0, number_of_test_runs, 1):
            output_line = ''
            for value in results['full_list']:
                if value != results['full_list'][-1]:
                    # don't normalise selected exceptions, or values that are not numbers
                    # or a value that is not a number
                    if value in normalise_exceptions or \
                    not isinstance(results[value][i], (int, float)):
                        output_line += "{0},".format(results[value][i])
                    else:
                        output_line += "{0},".format(results[value][i]/normalising_value_dict[value])
                else: #don't put the comma if it's the last value in the csv
                    if value in normalise_exceptions or \
                    not isinstance(results[value][i], (int, float)):
                        output_line += "{0},".format(results[value][i])
                    else:  # only normalise if the value is a number
                        output_line += "{0}".format(results[value][i]/normalising_value_dict[value])
            
            condition_builder_output.write(output_line + '\n')  
    
        condition_builder_output.close()              
                                  
    return     
    
def add_new_result_to_results_dict(cfg, states, V, M, results):
    """Function that takes a completed test run and adds the tunnel
       configuration and results to the results dictionary.
    """ 
    
    results['test number'].append(cfg['test_number'])
    results['Mr_st'].append(cfg['Mr_st'])
    if cfg['secondary']:
        results['psd1'].append(cfg['psd1'])
    results['p1'].append(cfg['p1']) 
    results['p5'].append(cfg['p5'])
    if cfg['secondary']:
        results['Vsd'].append(cfg['Vsd'])
    results['Vs1'].append(cfg['Vs1'])
    results['Vs2'].append(cfg['Vs2'])
    
    if cfg['stagnation_enthalpy']:
        results['Ht'].append(cfg['stagnation_enthalpy']/10**6)
    else:
        results['Ht'].append('did not solve')
    results['h'].append(cfg['freestream_enthalpy']/10**6)
    
    if cfg['u_eq']:
        results['u_eq'].append(cfg['u_eq'])
    else:
        results['u_eq'].append('did not solve')
       
    results['p2'].append(states['s2'].p)
    results['T2'].append(states['s2'].T)
    results['rho2'].append(states['s2'].rho)
    results['V2'].append(V['s2'])
    results['M2'].append(M['s2'])
    results['a2'].append(states['s2'].a)
    results['gamma2'].append(states['s2'].gam)
    results['R2'].append(states['s2'].R)   
    if cfg['Ht2']:
        results['Ht2'].append(cfg['Ht2']/10**6)
    else:
        results['Ht2'].append('did not solve')
    if cfg['Mr_st'] == 1.0: # then we don't make state 2r and this will be the same as state 2  
        results['p2r'].append(states['s2'].p)
        results['T2r'].append(states['s2'].T)
        results['rho2r'].append(states['s2'].rho)
        results['V2r'].append(V['s2'])
        results['M2r'].append(M['s2'])
        results['a2r'].append(states['s2'].a)
        results['gamma2r'].append(states['s2'].gam)
        results['R2r'].append(states['s2'].R)   
        if cfg['Ht2']:
            results['Ht2r'].append(cfg['Ht2']/10**6)
        else:
            results['Ht2r'].append('did not solve')   
    else:
        results['p2r'].append(states['s2r'].p)
        results['T2r'].append(states['s2r'].T)
        results['rho2r'].append(states['s2'].rho)
        results['V2r'].append(V['s2r'])
        results['M2r'].append(M['s2r'])
        results['a2r'].append(states['s2r'].a)
        results['gamma2r'].append(states['s2r'].gam)
        results['R2r'].append(states['s2r'].R)   
        if cfg['Ht2r']:
            results['Ht2r'].append(cfg['Ht2r']/10**6)
        else:
            results['Ht2r'].append('did not solve')   
        
    if cfg['tunnel_mode'] == 'expansion-tube':
        results['p6'].append(states['s6'].p)
        results['T6'].append(states['s6'].T)
        results['rho6'].append(states['s6'].rho)
        results['V6'].append(V['s6'])
        results['M6'].append(M['s6'])
        
        results['p7'].append(states['s7'].p)
        results['T7'].append(states['s7'].T)
        results['rho7'].append(states['s7'].rho)
        results['V7'].append(V['s7'])
        results['M7'].append(M['s7'])
    
    if cfg['nozzle']:
        results['arearatio'].append(cfg['area_ratio'])
        results['p8'].append(states['s8'].p)
        results['T8'].append(states['s8'].T)
        results['rho8'].append(states['s8'].rho)
        results['V8'].append(V['s8'])
        results['M8'].append(M['s8'])

    if cfg['conehead']:
        results['p10c'].append(states['s10c'].p)
        results['T10c'].append(states['s10c'].T)
        results['rho10c'].append(states['s10c'].rho)
        results['V10c'].append(V['s10c'])
        
    if cfg['shock_over_model']:
        if 's10f' in states.keys():
            results['p10f'].append(states['s10f'].p)
            results['T10f'].append(states['s10f'].T)
            results['rho10f'].append(states['s10f'].rho)
            results['V10f'].append(V['s10f'])
        else:
            results['p10f'].append('did not solve')
            results['T10f'].append('did not solve')
            results['rho10f'].append('did not solve')
            results['V10f'].append('did not solve')
        if 's10e' in states.keys():
            results['p10e'].append(states['s10e'].p)
            results['T10e'].append(states['s10e'].T)
            results['rho10e'].append(states['s10e'].rho)
            results['V10e'].append(V['s10e'])
        else:
            results['p10e'].append('did not solve')
            results['T10e'].append('did not solve')
            results['rho10e'].append('did not solve')
            results['V10e'].append('did not solve')  
            
    return results
    
def state2_reflected_shock_analysis_summary(cfg, results):
    """Function that takes the config dictionary and results dictionary 
       made throughout the running of the program and prints a summary of 
       the run to the screen and to a summary text file
    """
    
    print '-'*60
    print "Printing summary to screen and to a text document."
    
    with open(cfg['original_filename']+'-state2-reflected-shock-analysis-summary.txt',"w") as summary_file:
       
        # print lines explaining the results
        summary_line_1 = "# Summary of pitot state 2 reflected shock analysis program."
        summary_file.write(summary_line_1 + '\n')
        summary_line_2 = "# Summary performed using Version {0} of the state 2 reflected shock analysis program.".format(VERSION_STRING)
        summary_file.write(summary_line_2 + '\n')
        
        summary_line_3 = "{0} tests ran. {1} ({2:.1f}%) were successful."\
            .format(cfg['number_of_test_runs'], len(results['test number']),
            float(len(results['test number']))/float(cfg['number_of_test_runs'])*100.0)
        print summary_line_3
        summary_file.write(summary_line_3 + '\n')
    
        if results['unsuccessful_runs']: 
            summary_line_4 = "Unsucessful runs were run numbers {0}.".format(results['unsuccessful_runs'])
            print summary_line_4
            summary_file.write(summary_line_4 + '\n')          
    
        for variable in results['full_list']:
            # first check it's not a variable that doesn't need to be summarised
            if variable not in ['test number', 'driver condition', 'arearatio']:
                # now check the list and remove any string values
                data_list = []
                for value in results[variable]:
                    if isinstance(value, (float,int)): data_list.append(value)
                if len(data_list) > 0: #ie. don't bother summarising if there is no numerical data there
                    max_value = max(data_list)
                    min_value = min(data_list)
                    if variable[0] == 'p':
                        summary_line = "Variable {0} varies from {1:.2f} - {2:.2f} Pa."\
                        .format(variable, min_value, max_value)
                    elif variable[0] == 'V':
                        summary_line = "Variable {0} varies from {1:.2f} - {2:.2f} m/s."\
                        .format(variable, min_value, max_value)
                    elif variable[0] == 'T':
                        summary_line = "Variable {0} varies from {1:.2f} - {2:.2f} K."\
                        .format(variable, min_value, max_value)
                    elif variable[0] == 'r':
                        summary_line = "Variable {0} varies from {1:.7f} - {2:.7f} kg/m**3."\
                        .format(variable, min_value, max_value)
                    elif variable[0] == 'H' or variable[0] == 'h':
                        summary_line = "Variable {0} varies from {1:.7f} - {2:.7f} MJ/kg."\
                        .format(variable, min_value, max_value)                    
                    else:
                        summary_line = "Variable {0} varies from {1:.1f} - {2:.1f}."\
                        .format(variable, min_value, max_value)
                    print summary_line
                    summary_file.write(summary_line + '\n')
                    
        summary_file.close()   
                
    return
            
def run_pitot_state2_reflected_shock_analysis(cfg = {}, config_file = None):
    """
    
    Chris James (c.james4@uq.edu.au) 23/12/14
    
    run_pitot_state2_reflected_shock_analysis(dict) - > depends
    
    """
    
    import time
    
    print '-' * 60
    print "Starting pitot state 2 reflected shock analysis."
    print '-' * 60
    
    #---------------------- get the inputs set up --------------------------
    
    if config_file:
        cfg = config_loader(config_file)   
        
    #----------------- check inputs ----------------------------------------

    cfg = check_new_inputs(cfg)
    
    cfg = input_checker(cfg)
          
    # clean up any old files if the user has asked for it
    
    if cfg['cleanup_old_files']:
        from pitot_condition_builder import cleanup_old_files
        cleanup_old_files()
        
    #------------------ do initial run to get maximum Mr ---------------------
    print "Performing initital run to find maximum reflected shock strength."
    cfg['Mr_st'] = 'Maximum'
    try:
        cfg, states, V, M = run_pitot(cfg = cfg)
        # use thise first run to get some guesses too...
        cfg['Vs1_guess_1'] = cfg['Vs1'] - 20.0
        cfg['Vs1_guess_2'] = cfg['Vs1'] - 10.0
        cfg['Vs2_guess_1'] = cfg['Vs2'] - 20.0
        cfg['Vs2_guess_2'] = cfg['Vs2'] - 10.0       
        # pop these values so we can then run PITOT later
        if cfg['secondary'] and 'Vsd' in cfg: cfg.pop('Vsd')
        if 'Vs1' in cfg: cfg.pop('Vs1') 
        if 'Vs2' in cfg: cfg.pop('Vs2')
    except Exception:
         print "Initial run of Pitot to try and find maximum Mr has failed."
         print "Please check inputs."
         raise Exception()
    
    print "Maximum reflected shock strength is M = {0:.3f}.".format(cfg['Mr-st'])
    
    # now that the test run has been completed, we can build the list of Mr_st values
    import numpy
    cfg['Mr_st_list'] = numpy.arange(1.0, cfg['Mr-st'], cfg['Mr_step'])
    # now append the last value to the array so we can calculate that too
    cfg['Mr_st_list'] = numpy.append(cfg['Mr_st_list'], cfg['Mr-st'])
         
    # and now get the number of test runs     
    cfg['number_of_test_runs'] = calculate_number_of_test_runs(cfg)
    
    import copy
    cfg['original_filename'] = copy.copy(cfg['filename'])

    # make a counter so we can work out what test we're running
    # also make one to store how many runs are successful
   
    counter = 0
    good_counter = 0
    
    # print a start message to get us going
    
    cfg = start_message(cfg)
    
    # work out what we need in our results dictionary and make the dictionary
    
    results = build_results_dict(cfg)
    
    have_checked_time = False
    
    #now start up the for loops and get running    
    
    for Mr_st in cfg['Mr_st_list']:
        # need to work out what diluent we're using and whether we're using
        # mole or mass fractions, then get this going
        
        # when Mr_st = 1.0 we want to turn the code off (as a Mach wave has no strength anyway)        
        if Mr_st == 1.0:
            cfg['rs_out_of_st'] = False
        else:
            cfg['rs_out_of_st'] = True
        
        cfg['Mr_st'] = Mr_st

        counter += 1
        cfg['test_number'] = counter
        if not have_checked_time:
            start_time = time.time()
        run_status, results = pitot_state2_reflected_shock_analysis_test_run(cfg, results) 
        if run_status:
            good_counter += 1
            if not have_checked_time:
                test_time = time.time() - start_time
                print '-'*60
                print "Time to complete first test was {0:.2f} seconds."\
                .format(test_time)
                print "If every test takes this long. It will take roughly {0:.2f} hours to perform all {1} tests."\
                .format(test_time*cfg['number_of_test_runs']/3600.0, cfg['number_of_test_runs'])
                have_checked_time = True

    # now that we're done we can dump the results to the results csv 
    intro_line = "Output of pitot end of shock tube reflected shock analysis program Version {0}."\
                 .format(VERSION_STRING)            
    results_csv_builder(results, test_name = cfg['original_filename'],  
                        intro_line = intro_line)
                        
    #and a normalised csv also
    normalised_results_csv_builder(results, test_name = cfg['original_filename'],  
                        intro_line = intro_line, 
                        normalised_by = cfg['normalise_results_by'])  
                        
    # and pull in the pickle function from pitot_condition_builder.py
    # so we can pick the results and config dictionaries                  
    from pitot_condition_builder import pickle_data
    
    pickle_data(cfg, results)
    
    # now analyse results dictionary and print some results to the screen
    # and another external file
    
    state2_reflected_shock_analysis_summary(cfg, results) 
    
    return
                                
#----------------------------------------------------------------------------

def main():
    
    import optparse  
    op = optparse.OptionParser(version=VERSION_STRING)   
    op.add_option('-c', '--config_file', '--config-file', dest='config_file',
                  help=("filename where the configuration file is located"))    

    opt, args = op.parse_args()
    config_file = opt.config_file
           
    run_pitot_state2_reflected_shock_analysis(cfg = {}, config_file = config_file)
    
    return
    
#----------------------------------------------------------------------------

if __name__ == '__main__':
    if len(sys.argv) <= 1:
        print "pitot_state2_reflected_shock_analysis.py - Pitot Equilibrium expansion tube simulator reflected shock at end of the shock tube analysis tool"
        print "start with --help for help with inputs"
        
    else:
        main()
