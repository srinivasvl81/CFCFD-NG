# makefile for pitot
# lines to install the gas library and cfpylib taken from the nenzfr / estcj make file
#
# Chris James (c.james4@uq.edu.au) - 07-Apr-2014

TARGET ?= for_gnu
OPT ?= -O2

INSTALL_DIR ?= $(HOME)/e3bin
GAS     := ../../lib/gas
PROGRAMS := pitot.py pitot_input_utils.py pitot_flow_functions.py pitot_output_utils.py pitot_area_ratio_check.py pitot_condition_builder.py pitot_multiple.py test_pitot.py pitot_contamination_analysis.py pitot_gg_differing_diluent_analysis.py pitot_differing_compression_ratio_analysis.py pitot_state2_reflected_shock_analysis.py pitot_bootstrapper.py

install : $(INSTALL_DIR) $(PROGRAMS)
	@echo Copying cfpylib files to $(INSTALL_DIR)
	cd $(GAS)/build; make TARGET=$(TARGET) OPT=$(OPT) INSTALL_DIR=$(INSTALL_DIR) install
	cd ../../lib/; rsync -av --exclude=".svn" --exclude="*~" cfpylib $(INSTALL_DIR)
	@echo Copying pitot files to $(INSTALL_DIR)
	- cp -f $(PROGRAMS) $(INSTALL_DIR)
	- chmod +x $(INSTALL_DIR)/pitot.py
	- chmod +x $(INSTALL_DIR)/pitot_condition_builder.py
	- chmod +x $(INSTALL_DIR)/pitot_multiple.py
	- chmod +x $(INSTALL_DIR)/pitot_contamination_analysis.py
	- chmod +x $(INSTALL_DIR)/test_pitot.py
	- chmod +x $(INSTALL_DIR)/pitot_gg_differing_diluent_analysis.py
	- chmod +x $(INSTALL_DIR)/pitot_differing_compression_ratio_analysis.py
	- chmod +x $(INSTALL_DIR)/pitot_state2_reflected_shock_analysis.py
	- chmod +x $(INSTALL_DIR)/pitot_bootstrapper.py
	@echo -----------------------------------------
	@echo pitot should be now installed.
	@echo -----------------------------------------
	
$(INSTALL_DIR):
	mkdir $(INSTALL_DIR)
