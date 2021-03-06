# makefile for gas library.
#
# Author: Rowan J. Gollan
# Version: 14-May-2008, 26-Jun-2009

#----------------------------------------
# Some default compile options

TARGET ?= for_gnu
OPT ?= -O2
INSTALL_DIR ?= $(HOME)/e3bin
include ../../util/source/systems.mk

#----------------------------------------
# Directory variables

# Local to this area
MODELS          := ../models
KINETICS        := ../kinetics
SPECIES         := ../species
TOOLS           := ../tools
TEST            := ../test
CEA_CASES       := ../cea-cases
COLLISION_INTEGRALS := ../collision-integrals

# Other useful directory variables
UTIL_SRC        := ../../util/source
UTIL            := ../../util
LIBUTIL		:= $(UTIL)/build/libutil.a
NM_SRC          := ../../nm/source
NM              := ../../nm
LIBNM           := $(NM)/build/libnm.a
LUA             := ../../../extern/lua-5.1.4
LIBLUA          := $(LUA)/lib/liblua.a
LUA_INCLUDE_DIR := $(LUA)/include
LUALINK         := -ldl
ZLIB            := ../../../extern/zlib-1.2.3
LIBZLIB         := $(ZLIB)/libz.a
CEA2            := ../../../extern/cea2
REFPROP         := ../../../extern/refprop
ifeq ($(shell [ -d $(CEA2) ] && echo "true"), true)
  CEA2_EXISTS   := true
else
  CEA2_EXISTS   := false
endif
ifeq ($(shell [ -d $(REFPROP) ] && echo "true"), true)
  REFPROP_EXISTS  := true
else
  REFPROP_EXISTS  := false
endif

#---------------------------------------
# File name variables

LIBGAS := libgas.a

TEST_EXECUTABLES := user-defined-gas-model-test.x \
	composite-gas-model-test.x \
	generalised-Arrhenius-test.x \
	reaction-update-test.x \
	noneq-test.py \
	noneq-test.x \
	cpu-chem-test \
	gpu-chem-test \
	cpu-chem-step-test
#	gas-module-test.lua \
# 	perfect-gas-EOS-test.x \
# 	noble-abel-gas-EOS-test.x \

TOOL_EXECUTABLES := gasfile reac-lua2txt gasmodel.py
ifeq ($(CEA2_EXISTS), true)
  TOOL_EXECUTABLES += build-cea-lut.py cea2 thermo.inp trans.inp
  CEA2_MSG := "CEA2 has been included in the build."
else
  CEA2_MSG := "The CEA2 look-up-table utilities will NOT be functional\n\
  because the CEA2 program has not been found in $(CEA2)\n\
  Contact the NASA Glenn Research Center to request the software.\n\
  http://www.grc.nasa.gov/WWW/CEAWeb/"
endif
ifeq ($(REFPROP_EXISTS), true)
  TOOL_EXECUTABLES += build-REFPROP-lut.py refprop.so
  REFPROP_MSG := "REFPROP has been included in the build."
else
  REFPROP_MSG := "The REFPROP look-up-table and gas model will NOT be functional\n\
  because the REFPROP program has not been found in $(REFPROP)\n\
  Contact NIST to request the software. http://www.nist.gov/"
endif

LUA_MODULES := 	refprop.lua \
	four_temperature_gas.lua \
	three_temperature_gas.lua \
	two_temperature_gas.lua \
	one_temperature_gas.lua \
	fully_coupled_one_temperature_gas.lua \
	fully_coupled_two_temperature_gas.lua \
	fully_coupled_four_temperature_gas.lua \
	multi_temperature_gas.lua \
	reaction_parser.lua \
	reac.lua \
	reaction_rate.lua \
	tab_serialise.lua \
	energy_exchange_parser.lua \
	exch.lua \
	lex_elems.lua 

# Assume Unix/Linux
PYTHONGAS_MODULE := gaspy.py _gaspy.so
REFPROP_MODULE := refprop.so
ifeq ($(findstring Darwin, $(SYSTEM)), Darwin)
  # Darwin environment on a Mac OS X
  PYTHONGAS_MODULE := gaspy.py _gaspy.dylib
endif
ifeq ($(findstring MINGW32, $(SYSTEM)), MINGW32)
  # MINGW32 environment on MS-Windows
  PYTHONGAS_MODULE := gaspy.py _gaspy.dll
endif
ifeq ($(findstring CYGWIN, $(SYSTEM)), CYGWIN)
  # CYGWIN environment on MS-Windows
  PYTHONGAS_MODULE := gaspy.py _gaspy.dll
endif

#---------------------------------------
# main targets

build_new: $(LIBGAS) $(LUA_MODULES) $(TOOL_EXECUTABLES) $(PYTHONGAS_MODULE)
	@echo "Gas model library should be up to date."
	@echo -e $(subst \n ,\n,$(CEA2_MSG))
	@echo -e $(subst \n ,\n,$(REFPROP_MSG))

test: $(TEST_EXECUTABLES) cea2
	cp $(TEST_EXECUTABLES) $(TEST)
	# cp $(MODELS)/ltest.lua $(TEST)
	chmod +x $(TEST)/ltest.lua
	cd $(TEST); make test

noneq_test: noneq-test.x noneq-test.py cea2 $(PYTHONGAS_MODULE)
	cp noneq-test.py $(INSTALL_DIR)/
	cp cea2 $(INSTALL_DIR)/CEA2.exe
	cp thermo.inp trans.inp $(INSTALL_DIR)/
	cp noneq-test.x $(INSTALL_DIR)/

install: $(LIBGAS) $(LUA_MODULES) $(TOOL_EXECUTABLES) $(PYTHONGAS_MODULE)
	- mkdir -p $(INSTALL_DIR)
	cd ../../; rsync -av --exclude=".svn" --exclude="*~" cfpylib $(INSTALL_DIR)
	- cp $(TOOL_EXECUTABLES) $(INSTALL_DIR)/
	- mkdir -p $(INSTALL_DIR)/cea-cases
	- cp $(CEA_CASES)/*.lua.gz $(INSTALL_DIR)/cea-cases/
ifeq ($(CEA2_EXISTS), true)
	- cp thermo.inp trans.inp $(INSTALL_DIR)/cea-cases/
endif
	- cp $(LUA)/bin/lua $(INSTALL_DIR)/
	chmod +x $(INSTALL_DIR)/gasfile
	chmod +x $(INSTALL_DIR)/gasmodel.py
	- cp $(LUA_MODULES) $(INSTALL_DIR)/
	- mkdir -p $(INSTALL_DIR)/species
	- cp $(SPECIES)/*.lua $(INSTALL_DIR)/species/
ifeq ($(REFPROP_EXISTS), true)
	- rsync -av $(SPECIES)/species-with-chemkin-thermo-data $(INSTALL_DIR)/species/
	- mkdir -p $(INSTALL_DIR)/species/refprop/fluids
	- rsync -av $(REFPROP)/REFPROP/fluids/ $(INSTALL_DIR)/species/refprop/fluids
	- mkdir -p $(INSTALL_DIR)/species/refprop/mixtures
	- rsync -av $(REFPROP)/REFPROP/mixtures/ $(INSTALL_DIR)/species/refprop/mixtures
endif
	- mkdir -p $(INSTALL_DIR)/collision-integrals
	rsync -a --exclude=.svn $(COLLISION_INTEGRALS)/* $(INSTALL_DIR)/collision-integrals/
	- cp $(PYTHONGAS_MODULE) $(INSTALL_DIR)/
ifeq ($(findstring Darwin, $(SYSTEM)), Darwin)
	- cp $(INSTALL_DIR)/_gaspy.dylib $(INSTALL_DIR)/_gaspy.so
endif

tags:
	cd ../models; etags ./*.cxx ./*.hh $(KINETICS)/*.cxx $(KINETICS)/*.hh \
		$(UTIL_SRC)/*.cxx $(UTIL_SRC)/*.hh $(UTIL_SRC)/*.c $(UTIL_SRC)/*.h

clean:
	- rm $(LIBGAS)
	- rm *.o
	- rm $(TEST_EXECUTABLES)
	- rm $(TOOL_EXECUTABLES)
	- rm thermo.inp trans.inp
	- rm $(LUA_MODULES)
ifeq ($(findstring Darwin, $(SYSTEM)), Darwin)
	- rm *.so
endif
	- rm $(PYTHONGAS_MODULE) $(MODELS)/gaspy_wrap.cxx $(MODELS)/gaspy.py
	- rm noneq-test.py $(TEST)/noneq-test.py
	- cd $(LUA); make clean
	- cd $(ZLIB); make clean
	- cd $(NM)/build; make clean
	- cd $(TEST); make clean
	- cd $(UTIL)/build; make clean

clean_swig :
	- rm $(LIBGAS)
	- rm $(PYTHONGAS_MODULE) $(MODELS)/gaspy_wrap.cxx $(MODELS)/gaspy.py
	- rm gaspy_wrap.o

#----------------------------------------
# Filename variables
# - convenient ways to refer to some
#   special files or collections of files

MODELS_OBJECTS := CEA-Cp-functor.o \
	CEA-h-functor.o \
	CEA-s-functor.o \
	CEA-viscosity.o \
	gas_data.o \
	gas-model.o \
	lua_service.o \
	Richardson_extrapolation.o \
	user-defined-gas-model.o \
	lservice_gas_data.o \
	look-up-table.o \
	LUT-plus-composite-gas-model.o \
	composite-gas-model.o \
	REFPROP-gas-model.o \
	perfect-gas-EOS.o \
	simple-gas-EOS.o \
	noble-abel-gas-EOS.o \
	van-der-waals-gas-EOS.o \
	Bender-EOS.o \
	MBWR-EOS.o \
	thermal-behaviour-model.o \
	perfect-thermal-behaviour.o \
	real-thermal-behaviour.o \
	dense-real-thermal-behaviour.o \
	constant-specific-heats.o \
	Wilke-mixing-rule.o \
	segmented-functor.o \
	Sutherland-viscosity.o \
	Sutherland-thermal-conductivity.o \
	no-diffusion-coefficients.o \
	hard-sphere-dcm.o \
	CEA_curves.o \
	CEA-thermal-conductivity.o \
	sound-speed-model.o \
	eq-sound-speed.o \
	noneq-sound-speed.o \
	noneq-gas-EOS.o \
	noneq-thermal-behaviour.o \
	thermal-energy-modes.o \
	chemical-species.o \
	species-energy-modes.o \
	diatom-electronic-level.o \
	polyatom-electronic-level.o \
	coupled-diatom-LUT.o \
	no-transport-coefficients.o \
	chemical-equilibrium-system.o \
	chemical-species-library.o \
	zero_system.o \
	zero_finders.o \
	CI-functor.o \
	collision-integral.o \
	GuptaYos-mixing-rule.o \
	GuptaYos-dcm.o \
	binary-interaction.o \
	Armaly-Sutton-mixing-rule.o \
	Blottner-viscosity.o

KINETICS_OBJECTS := chemical-kinetic-ODE-update.o \
	chemical-kinetic-system.o \
	chemical-kinetic-ODE-MC-update.o \
	chemical-kinetic-MC-system.o \
	chemistry-energy-coupling.o \
	coupling-component.o \
	species-pieces.o \
	eq-const-from-thermo.o \
	eq-const-from-CEA-curves.o \
	eq-const-from-partition-functions.o \
	equilibrium-constant.o \
	generalised-Arrhenius.o \
	Park-nonequilibrium.o \
	Macheret-dissociation.o \
	MarroneTreanor-dissociation.o \
	Knab-molecular-reaction.o \
	no_fuss_linear_algebra.o \
	normal-reaction.o \
	ode_solver.o \
	ode_step.o \
	ode_setup.o \
	ode_system.o \
	pressure-dependent-rate.o \
	reaction.o \
	reaction-rate-coeff.o \
	reaction-update.o \
	third-body-reaction.o \
	energy-exchange-ODE-update.o \
	energy-exchange-system.o \
	energy-exchange-rate.o \
	energy-exchange-mechanism.o \
	energy-exchange-relaxation-time.o \
	energy-exchange-update.o \
	thermal-equilibrium-mechanism.o

#-----------------------------------------
# Archive library
#

$(LIBGAS) : $(MODELS_OBJECTS) $(KINETICS_OBJECTS) $(LIBUTIL)
	- rm -f $(LIBGAS)
	$(AR) rv $(LIBGAS) $(MODELS_OBJECTS) $(KINETICS_OBJECTS) $(LIBUTIL)
	- ranlib $(LIBGAS)

#-----------------------------------------
# Loadable library for Python.
#

CXXFLAG3 := -c -fPIC

$(MODELS)/gaspy_wrap.cxx : $(MODELS)/gaspy.i $(MODELS)/libgas.i $(LIBGAS)
	swig -python -c++ -w512 $(MODELS)/gaspy.i

gaspy_wrap.o : $(MODELS)/gaspy_wrap.cxx $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG3) $(MODELS)/gaspy_wrap.cxx \
	-I$(PYTHON_INCLUDE_DIR) -I$(LUA_INCLUDE_DIR)

$(MODELS)/gaspy.py : $(MODELS)/gaspy.i $(MODELS)/libgas.i $(LIBGAS)
	swig -python -c++ -w512 $(MODELS)/gaspy.i

gaspy.py : $(MODELS)/gaspy.py
	cp $(MODELS)/gaspy.py .

# Shared object for Unix/Linux
_gaspy.so : gaspy.py gaspy_wrap.o $(LIBUTIL) $(LIBGAS) $(LIBZLIB) $(LIBLUA)
	$(CXXLINK) $(LDFLAG) -shared -o _gaspy.so gaspy_wrap.o \
	$(LIBUTIL) $(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LLIB)

# Dynamic library for Mac OS X
_gaspy.dylib : gaspy.py gaspy_wrap.o $(LIBUTIL) $(LIBGAS) $(LIBZLIB) $(LIBLUA)
	$(CXXLINK) $(LDFLAG) -o _gaspy.dylib gaspy_wrap.o \
	$(LIBUTIL) $(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LLIB) -framework Python \
	-bundle -bundle_loader $(PYTHON_BIN_DIR)/python

# Dynamic link library for MINGW32 environment on MS-Windows
_gaspy.dll : gaspy_wrap.o $(LIBUTIL) $(LIBGAS) $(LIBPYTHON) $(LIBLUA) $(LIBZLIB)
	$(CXXLINK) $(LDFLAG) -shared -o _gaspy.dll gaspy_wrap.o \
		-L$(PYTHON_DIR)/libs \
		$(LIBUTIL) $(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LIBPYTHON) $(LLIB)

#----------------------------------------
# Loadable library for Lua
#
# Shared object for Unix/Linux
gas.so : $(LUA_OBJECTS) $(LIBUTIL) $(LIBGAS) $(LIBLUA) $(LIBZLIB)
	$(CXXLINK) $(LDFLAG) -shared -o gas.so $(LUA_OBJECTS) $(LIBGAS) \
	$(LIBUTIL) $(LIBLUA) $(LIBZLIB) $(LLIB)

# Dynamic link library for MINGW32 environment on MS-Windows
gas.dll : $(LUA_OBJECTS) $(LIBUTIL) $(LIBGAS) $(LIBLUA) $(LIBZLIB)
	$(CXXLINK) $(LDFLAG) -shared -o gas.dll $(LUA_OBJECTS) $(LIBGAS) \
	$(LIBUTIL) $(LIBLUA) $(LIBZLIB) $(LLIB)

# Dynamic library for Mac OS X
gas.dylib : $(LUA_OBJECTS) $(LIBUTIL) $(LIBGAS) $(LIBLUA) $(LIBZLIB)
	$(CXXLINK) $(LDFLAG) -o gas.dylib $(LUA_OBJECTS) $(LIBGAS) $(LIBUTIL) \
	$(LIBLUA) $(LIBZLIB) $(LLIB) \
	-framework Python -bundle -bundle_loader $(PYTHON_BIN_DIR)/python

#----------------------------------------
# Libraries (external dependencies)

# Assume linux
PLAT := linux
ifeq ($(findstring Darwin, $(SYSTEM)), Darwin)
    PLAT := macosx
endif
ifeq ($(findstring CYGWIN, $(SYSTEM)), CYGWIN)
    PLAT := cygwin
endif
# Even if we are using the Mingw compiler, assume that we are on cygwin.
ifeq ($(findstring MINGW32, $(SYSTEM)), MINGW32)
    PLAT := cygwin
endif
$(LIBLUA):
	cd $(LUA); make $(PLAT) local

$(LUA_INCLUDE_DIR): $(LIBLUA)

$(LIBZLIB):
	cd $(ZLIB); make libz.a

$(LIBNM) :
	cd $(NM)/build; make TARGET=$(TARGET) OPT=$(OPT) libnm.a

$(LIBUTIL) :
	cd $(UTIL)/build; make TARGET=$(TARGET) OPT=$(OPT) libutil.a

#----------------------------------------
# Object files


#
# models objects
#
CEA-Cp-functor.o : $(MODELS)/CEA-Cp-functor.cxx $(MODELS)/CEA-Cp-functor.hh \
	$(NM_SRC)/functor.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/CEA-Cp-functor.cxx -I$(LUA_INCLUDE_DIR)

CEA-h-functor.o : $(MODELS)/CEA-h-functor.cxx $(MODELS)/CEA-h-functor.hh \
	$(NM_SRC)/functor.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/CEA-h-functor.cxx -I$(LUA_INCLUDE_DIR)

CEA-s-functor.o : $(MODELS)/CEA-s-functor.cxx $(MODELS)/CEA-s-functor.hh \
	$(NM_SRC)/functor.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/CEA-s-functor.cxx -I$(LUA_INCLUDE_DIR)

CEA-viscosity.o : $(MODELS)/CEA-viscosity.cxx $(MODELS)/CEA-viscosity.hh \
	$(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/CEA-viscosity.cxx -I$(LUA_INCLUDE_DIR)

gas_data.o : $(MODELS)/gas_data.cxx $(MODELS)/gas_data.hh
	$(CXXCOMPILE) $(CXXFLAG) -I$(LUA_INCLUDE_DIR) $(MODELS)/gas_data.cxx

lua_service.o : $(UTIL_SRC)/lua_service.hh $(UTIL_SRC)/lua_service.cxx $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(UTIL_SRC)/lua_service.cxx -I$(LUA_INCLUDE_DIR) -I$(ZLIB)

look-up-table.o : $(MODELS)/look-up-table.cxx $(MODELS)/look-up-table.cxx \
	$(MODELS)/gas_data.hh $(MODELS)/gas-model.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/look-up-table.cxx -I$(LUA_INCLUDE_DIR)

LUT-plus-composite-gas-model.o : $(MODELS)/LUT-plus-composite-gas-model.hh $(MODELS)/LUT-plus-composite-gas-model.cxx \
	$(MODELS)/gas_data.hh $(MODELS)/gas-model.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/LUT-plus-composite-gas-model.cxx -I$(LUA_INCLUDE_DIR)

lservice_gas_data.o : $(MODELS)/lservice_gas_data.cxx $(MODELS)/lservice_gas_data.hh \
	$(MODELS)/gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/lservice_gas_data.cxx -I$(LUA_INCLUDE_DIR)

gas-model.o : $(MODELS)/gas-model.cxx $(MODELS)/gas-model.hh \
	$(MODELS)/gas_data.hh $(NM_SRC)/functor.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/gas-model.cxx -I$(LUA_INCLUDE_DIR)

Richardson_extrapolation.o : $(NM_SRC)/Richardson_extrapolation.cxx $(NM_SRC)/Richardson_extrapolation.hh \
	$(NM_SRC)/functor.hh
	$(CXXCOMPILE) $(CXXFLAG) $(NM_SRC)/Richardson_extrapolation.cxx

user-defined-gas-model.o : $(MODELS)/user-defined-gas-model.cxx $(MODELS)/user-defined-gas-model.hh \
	$(MODELS)/gas_data.hh $(MODELS)/lservice_gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/user-defined-gas-model.cxx -I$(LUA_INCLUDE_DIR)

composite-gas-model.o : $(MODELS)/composite-gas-model.cxx $(MODELS)/composite-gas-model.hh \
	$(MODELS)/gas-model.hh $(MODELS)/gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/composite-gas-model.cxx -I$(LUA_INCLUDE_DIR)

REFPROP-gas-model.o : $(MODELS)/REFPROP-gas-model.cxx $(MODELS)/REFPROP-gas-model.hh \
	$(MODELS)/gas-model.hh $(MODELS)/gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/REFPROP-gas-model.cxx -I$(LUA_INCLUDE_DIR)

perfect-gas-EOS.o : $(MODELS)/perfect-gas-EOS.cxx $(MODELS)/perfect-gas-EOS.hh \
	$(MODELS)/equation-of-state.hh $(MODELS)/gas_data.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/perfect-gas-EOS.cxx -I$(LUA_INCLUDE_DIR)

simple-gas-EOS.o : $(MODELS)/simple-gas-EOS.cxx $(MODELS)/simple-gas-EOS.hh \
	$(MODELS)/equation-of-state.hh $(MODELS)/gas_data.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/simple-gas-EOS.cxx -I$(LUA_INCLUDE_DIR)

noble-abel-gas-EOS.o : $(MODELS)/noble-abel-gas-EOS.cxx $(MODELS)/noble-abel-gas-EOS.hh \
	$(MODELS)/equation-of-state.hh $(MODELS)/gas_data.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/noble-abel-gas-EOS.cxx -I$(LUA_INCLUDE_DIR)

van-der-waals-gas-EOS.o : $(MODELS)/van-der-waals-gas-EOS.cxx $(MODELS)/van-der-waals-gas-EOS.hh \
	$(MODELS)/equation-of-state.hh $(MODELS)/gas_data.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/van-der-waals-gas-EOS.cxx -I$(LUA_INCLUDE_DIR)

Bender-EOS.o : $(MODELS)/Bender-EOS.cxx $(MODELS)/Bender-EOS.hh \
	$(MODELS)/equation-of-state.hh $(MODELS)/gas_data.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/Bender-EOS.cxx -I$(LUA_INCLUDE_DIR)


MBWR-EOS.o : $(MODELS)/MBWR-EOS.cxx $(MODELS)/MBWR-EOS.hh \
	$(MODELS)/equation-of-state.hh $(MODELS)/gas_data.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/MBWR-EOS.cxx -I$(LUA_INCLUDE_DIR)

thermal-behaviour-model.o : $(MODELS)/thermal-behaviour-model.cxx $(MODELS)/thermal-behaviour-model.hh \
	$(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/thermal-behaviour-model.cxx -I$(LUA_INCLUDE_DIR)

perfect-thermal-behaviour.o : $(MODELS)/perfect-thermal-behaviour.cxx $(MODELS)/perfect-thermal-behaviour.hh \
	$(MODELS)/thermal-behaviour-model.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/perfect-thermal-behaviour.cxx -I$(LUA_INCLUDE_DIR)

real-thermal-behaviour.o : $(MODELS)/real-thermal-behaviour.cxx $(MODELS)/real-thermal-behaviour.hh \
	$(MODELS)/thermal-behaviour-model.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/real-thermal-behaviour.cxx -I$(LUA_INCLUDE_DIR)

dense-real-thermal-behaviour.o : $(MODELS)/dense-real-thermal-behaviour.cxx $(MODELS)/dense-real-thermal-behaviour.hh \
	$(MODELS)/thermal-behaviour-model.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/dense-real-thermal-behaviour.cxx -I$(LUA_INCLUDE_DIR)

constant-specific-heats.o : $(MODELS)/constant-specific-heats.cxx $(MODELS)/constant-specific-heats.hh \
	$(MODELS)/thermal-behaviour-model.hh $(MODELS)/gas_data.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/constant-specific-heats.cxx -I$(LUA_INCLUDE_DIR)

segmented-functor.o : $(NM_SRC)/segmented-functor.cxx $(NM_SRC)/segmented-functor.hh \
	$(NM_SRC)/functor.hh
	$(CXXCOMPILE) $(CXXFLAG) $(NM_SRC)/segmented-functor.cxx

Sutherland-viscosity.o : $(MODELS)/Sutherland-viscosity.cxx $(MODELS)/Sutherland-viscosity.hh \
	$(MODELS)/viscosity-model.hh $(MODELS)/gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/Sutherland-viscosity.cxx -I$(LUA_INCLUDE_DIR)

Sutherland-thermal-conductivity.o : $(MODELS)/Sutherland-thermal-conductivity.cxx $(MODELS)/Sutherland-thermal-conductivity.hh \
	$(MODELS)/Sutherland-thermal-conductivity.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/Sutherland-thermal-conductivity.cxx -I$(LUA_INCLUDE_DIR)

Wilke-mixing-rule.o : $(MODELS)/Wilke-mixing-rule.cxx $(MODELS)/Wilke-mixing-rule.hh \
	$(MODELS)/transport-coefficients-model.hh $(MODELS)/gas_data.hh \
	$(MODELS)/viscosity-model.hh $(MODELS)/thermal-conductivity-model.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/Wilke-mixing-rule.cxx -I$(LUA_INCLUDE_DIR)

no-diffusion-coefficients.o : $(MODELS)/no-diffusion-coefficients.cxx $(MODELS)/no-diffusion-coefficients.hh \
	$(MODELS)/gas_data.hh
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/no-diffusion-coefficients.cxx

hard-sphere-dcm.o : $(MODELS)/hard-sphere-dcm.cxx $(MODELS)/hard-sphere-dcm.hh \
	$(MODELS)/diffusion-coefficients-model.hh $(MODELS)/gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/hard-sphere-dcm.cxx -I$(LUA_INCLUDE_DIR)

CEA_curves.o : $(MODELS)/CEA_curves.cxx $(MODELS)/CEA_curves.hh \
	$(MODELS)/gas_data.hh
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/CEA_curves.cxx

CEA-thermal-conductivity.o : $(MODELS)/CEA-thermal-conductivity.cxx $(MODELS)/CEA-thermal-conductivity.hh \
	$(MODELS)/CEA-thermal-conductivity.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/CEA-thermal-conductivity.cxx -I$(LUA_INCLUDE_DIR)

sound-speed-model.o : $(MODELS)/sound-speed-model.cxx $(MODELS)/sound-speed-model.hh \
	$(MODELS)/gas_data.hh $(MODELS)/gas-model.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/sound-speed-model.cxx -I$(LUA_INCLUDE_DIR)

eq-sound-speed.o : $(MODELS)/eq-sound-speed.cxx $(MODELS)/eq-sound-speed.hh \
	$(MODELS)/gas_data.hh $(MODELS)/gas-model.hh $(MODELS)/sound-speed-model.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/eq-sound-speed.cxx -I$(LUA_INCLUDE_DIR)

noneq-sound-speed.o : $(MODELS)/noneq-sound-speed.cxx $(MODELS)/noneq-sound-speed.hh \
	$(MODELS)/gas_data.hh $(MODELS)/gas-model.hh $(MODELS)/sound-speed-model.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/noneq-sound-speed.cxx -I$(LUA_INCLUDE_DIR)

noneq-gas-EOS.o : $(MODELS)/noneq-gas-EOS.cxx $(MODELS)/noneq-gas-EOS.hh \
	$(MODELS)/equation-of-state.hh $(MODELS)/gas_data.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/noneq-gas-EOS.cxx -I$(LUA_INCLUDE_DIR)

noneq-thermal-behaviour.o : $(MODELS)/noneq-thermal-behaviour.cxx $(MODELS)/noneq-thermal-behaviour.hh \
	$(MODELS)/thermal-behaviour-model.hh $(MODELS)/thermal-energy-modes.hh \
	$(MODELS)/chemical-species.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/noneq-thermal-behaviour.cxx -I$(LUA_INCLUDE_DIR)

thermal-energy-modes.o : $(MODELS)/thermal-energy-modes.cxx $(MODELS)/thermal-energy-modes.hh \
	$(MODELS)/species-energy-modes.hh $(MODELS)/chemical-species.hh \
	$(MODELS)/gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/thermal-energy-modes.cxx -I$(LUA_INCLUDE_DIR)

chemical-species.o : $(MODELS)/chemical-species.cxx $(MODELS)/chemical-species.hh \
	$(MODELS)/species-energy-modes.hh $(MODELS)/gas_data.hh \
	$(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/chemical-species.cxx -I$(LUA_INCLUDE_DIR)

species-energy-modes.o : $(MODELS)/species-energy-modes.cxx $(MODELS)/species-energy-modes.hh \
	$(MODELS)/diatom-electronic-level.hh $(MODELS)/polyatom-electronic-level.hh $(MODELS)/coupled-diatom-LUT.hh
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/species-energy-modes.cxx

diatom-electronic-level.o : $(MODELS)/diatom-electronic-level.cxx $(MODELS)/diatom-electronic-level.hh
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/diatom-electronic-level.cxx

polyatom-electronic-level.o : $(MODELS)/polyatom-electronic-level.cxx $(MODELS)/polyatom-electronic-level.hh
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/polyatom-electronic-level.cxx

coupled-diatom-LUT.o : $(MODELS)/coupled-diatom-LUT.cxx $(MODELS)/coupled-diatom-LUT.hh
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/coupled-diatom-LUT.cxx

no-transport-coefficients.o : $(MODELS)/no-transport-coefficients.cxx \
	$(MODELS)/no-transport-coefficients.hh $(MODELS)/gas_data.hh \
	$(MODELS)/transport-coefficients-model.hh
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/no-transport-coefficients.cxx -I$(LUA_INCLUDE_DIR)

chemical-equilibrium-system.o : $(MODELS)/chemical-equilibrium-system.cxx \
	$(MODELS)/chemical-equilibrium-system.hh $(MODELS)/chemical-species.hh \
	$(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/chemical-equilibrium-system.cxx -I$(LUA_INCLUDE_DIR)

chemical-species-library.o : $(MODELS)/chemical-species-library.cxx $(MODELS)/chemical-species-library.hh \
	$(MODELS)/chemical-species.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/chemical-species-library.cxx -I$(LUA_INCLUDE_DIR)

zero_system.o : $(NM_SRC)/zero_system.cxx $(NM_SRC)/zero_system.hh \
        $(NM_SRC)/no_fuss_linear_algebra.hh
	$(CXXCOMPILE) $(CXXFLAG) $(NM_SRC)/zero_system.cxx

zero_finders.o : $(NM_SRC)/zero_finders.cxx $(NM_SRC)/zero_finders.hh \
        $(NM_SRC)/zero_system.hh $(NM_SRC)/no_fuss_linear_algebra.hh
	$(CXXCOMPILE) $(CXXFLAG) $(NM_SRC)/zero_finders.cxx

CI-functor.o : $(MODELS)/CI-functor.cxx $(MODELS)/CI-functor.hh \
	$(NM_SRC)/functor.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/CI-functor.cxx -I$(LUA_INCLUDE_DIR)

collision-integral.o : $(MODELS)/collision-integral.cxx $(MODELS)/collision-integral.hh \
        $(MODELS)/CI-functor.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/collision-integral.cxx -I$(LUA_INCLUDE_DIR)

GuptaYos-mixing-rule.o : $(MODELS)/GuptaYos-mixing-rule.cxx $(MODELS)/GuptaYos-mixing-rule.hh \
        $(MODELS)/binary-interaction.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/GuptaYos-mixing-rule.cxx -I$(LUA_INCLUDE_DIR)

GuptaYos-dcm.o : $(MODELS)/GuptaYos-dcm.cxx $(MODELS)/GuptaYos-dcm.hh \
        $(MODELS)/binary-interaction.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/GuptaYos-dcm.cxx -I$(LUA_INCLUDE_DIR)

binary-interaction.o : $(MODELS)/binary-interaction.cxx $(MODELS)/binary-interaction.hh \
        $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/binary-interaction.cxx -I$(LUA_INCLUDE_DIR)

Armaly-Sutton-mixing-rule.o : $(MODELS)/Armaly-Sutton-mixing-rule.cxx $(MODELS)/Armaly-Sutton-mixing-rule.hh \
        $(MODELS)/binary-interaction.hh $(UTIL_SRC)/lua_service.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/Armaly-Sutton-mixing-rule.cxx -I$(LUA_INCLUDE_DIR)

Blottner-viscosity.o : $(MODELS)/Blottner-viscosity.cxx $(MODELS)/Blottner-viscosity.hh \
	$(MODELS)/viscosity-model.hh $(MODELS)/gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(MODELS)/Blottner-viscosity.cxx -I$(LUA_INCLUDE_DIR)

#
# kinetics objects
#

chemical-kinetic-ODE-update.o : $(KINETICS)/chemical-kinetic-ODE-update.cxx $(KINETICS)/chemical-kinetic-ODE-update.hh \
	$(KINETICS)/reaction-update.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/chemical-kinetic-ODE-update.cxx -I$(LUA_INCLUDE_DIR)

chemical-kinetic-system.o : $(KINETICS)/chemical-kinetic-system.hh $(KINETICS)/chemical-kinetic-system.cxx \
	$(NM_SRC)/ode_system.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/chemical-kinetic-system.cxx -I$(LUA_INCLUDE_DIR)

chemical-kinetic-ODE-MC-update.o : $(KINETICS)/chemical-kinetic-ODE-MC-update.cxx $(KINETICS)/chemical-kinetic-ODE-MC-update.hh \
	$(KINETICS)/reaction-update.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/chemical-kinetic-ODE-MC-update.cxx -I$(LUA_INCLUDE_DIR)

chemical-kinetic-MC-system.o : $(KINETICS)/chemical-kinetic-MC-system.hh $(KINETICS)/chemical-kinetic-MC-system.cxx \
	$(NM_SRC)/ode_system.hh $(KINETICS)/species-pieces.hh $(KINETICS)/chemistry-energy-coupling.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/chemical-kinetic-MC-system.cxx -I$(LUA_INCLUDE_DIR)

chemistry-energy-coupling.o : $(KINETICS)/chemistry-energy-coupling.hh $(KINETICS)/chemistry-energy-coupling.cxx \
	$(KINETICS)/coupling-component.hh $(MODELS)/chemical-species-library.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/chemistry-energy-coupling.cxx -I$(LUA_INCLUDE_DIR)

coupling-component.o : $(KINETICS)/coupling-component.hh $(KINETICS)/coupling-component.cxx \
	$(KINETICS)/reaction.hh $(MODELS)/chemical-species-library.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/coupling-component.cxx -I$(LUA_INCLUDE_DIR)

species-pieces.o : $(KINETICS)/species-pieces.hh $(KINETICS)/species-pieces.cxx 
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/species-pieces.cxx

eq-const-from-thermo.o : $(KINETICS)/eq-const-from-thermo.hh $(KINETICS)/eq-const-from-thermo.cxx \
	$(KINETICS)/equilibrium-constant.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/eq-const-from-thermo.cxx -I$(LUA_INCLUDE_DIR)

eq-const-from-CEA-curves.o : $(KINETICS)/eq-const-from-CEA-curves.hh $(KINETICS)/eq-const-from-CEA-curves.cxx \
	$(KINETICS)/equilibrium-constant.hh $(MODELS)/chemical-species.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/eq-const-from-CEA-curves.cxx -I$(LUA_INCLUDE_DIR)

eq-const-from-partition-functions.o : $(KINETICS)/eq-const-from-partition-functions.hh $(KINETICS)/eq-const-from-partition-functions.cxx \
	$(KINETICS)/equilibrium-constant.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/eq-const-from-partition-functions.cxx -I$(LUA_INCLUDE_DIR)

equilibrium-constant.o : $(KINETICS)/equilibrium-constant.hh $(KINETICS)/equilibrium-constant.cxx \
	$(MODELS)/gas-model.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/equilibrium-constant.cxx -I$(LUA_INCLUDE_DIR)

generalised-Arrhenius.o : $(KINETICS)/generalised-Arrhenius.cxx $(KINETICS)/generalised-Arrhenius.hh \
	$(KINETICS)/reaction-rate-coeff.hh $(MODELS)/gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/generalised-Arrhenius.cxx -I$(LUA_INCLUDE_DIR)

Park-nonequilibrium.o : $(KINETICS)/Park-nonequilibrium.cxx $(KINETICS)/Park-nonequilibrium.hh \
	$(KINETICS)/reaction-rate-coeff.hh $(MODELS)/gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/Park-nonequilibrium.cxx -I$(LUA_INCLUDE_DIR)

Macheret-dissociation.o : $(KINETICS)/Macheret-dissociation.cxx $(KINETICS)/Macheret-dissociation.hh \
	$(KINETICS)/reaction-rate-coeff.hh $(MODELS)/gas_data.hh $(KINETICS)/generalised-Arrhenius.hh \
	$(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/Macheret-dissociation.cxx -I$(LUA_INCLUDE_DIR)

MarroneTreanor-dissociation.o : $(KINETICS)/MarroneTreanor-dissociation.cxx $(KINETICS)/MarroneTreanor-dissociation.hh \
	$(KINETICS)/reaction-rate-coeff.hh $(MODELS)/gas_data.hh $(KINETICS)/generalised-Arrhenius.hh \
	$(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/MarroneTreanor-dissociation.cxx -I$(LUA_INCLUDE_DIR)

Knab-molecular-reaction.o : $(KINETICS)/Knab-molecular-reaction.cxx $(KINETICS)/Knab-molecular-reaction.hh \
	$(KINETICS)/reaction-rate-coeff.hh $(MODELS)/gas_data.hh $(KINETICS)/generalised-Arrhenius.hh \
	$(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/Knab-molecular-reaction.cxx -I$(LUA_INCLUDE_DIR)

no_fuss_linear_algebra.o : $(NM_SRC)/no_fuss_linear_algebra.cxx $(NM_SRC)/no_fuss_linear_algebra.hh
	$(CXXCOMPILE) $(CXXFLAG) $(NM_SRC)/no_fuss_linear_algebra.cxx

normal-reaction.o : $(KINETICS)/normal-reaction.hh $(KINETICS)/normal-reaction.cxx \
	$(KINETICS)/reaction.hh $(MODELS)/gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/normal-reaction.cxx -I$(LUA_INCLUDE_DIR)

ode_solver.o : $(NM_SRC)/ode_solver.hh $(NM_SRC)/ode_solver.cxx
	$(CXXCOMPILE) $(CXXFLAG) $(NM_SRC)/ode_solver.cxx

ode_step.o : $(NM_SRC)/ode_step.hh $(NM_SRC)/ode_step.cxx
	$(CXXCOMPILE) $(CXXFLAG) $(NM_SRC)/ode_step.cxx

ode_setup.o : $(KINETICS)/ode_setup.cxx $(KINETICS)/ode_setup.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/ode_setup.cxx -I$(LUA_INCLUDE_DIR)

ode_system.o : $(NM_SRC)/ode_system.hh $(NM_SRC)/ode_system.cxx
	$(CXXCOMPILE) $(CXXFLAG) $(NM_SRC)/ode_system.cxx

pressure-dependent-rate.o : $(KINETICS)/pressure-dependent-rate.cxx $(KINETICS)/pressure-dependent-rate.hh \
	$(KINETICS)/reaction-rate-coeff.hh $(MODELS)/gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/pressure-dependent-rate.cxx -I$(LUA_INCLUDE_DIR)


reaction.o : $(KINETICS)/reaction.hh $(KINETICS)/reaction.cxx \
	$(MODELS)/gas_data.hh $(MODELS)/gas-model.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/reaction.cxx -I$(LUA_INCLUDE_DIR)

reaction-rate-coeff.o : $(KINETICS)/reaction-rate-coeff.cxx $(KINETICS)/reaction-rate-coeff.hh \
	$(MODELS)/gas_data.hh $(MODELS)/gas-model.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/reaction-rate-coeff.cxx -I$(LUA_INCLUDE_DIR)

reaction-update.o : $(KINETICS)/reaction-update.cxx $(KINETICS)/reaction-update.hh \
	$(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/reaction-update.cxx -I$(LUA_INCLUDE_DIR)

third-body-reaction.o : $(KINETICS)/third-body-reaction.hh $(KINETICS)/third-body-reaction.cxx \
	$(KINETICS)/normal-reaction.hh $(KINETICS)/reaction.hh $(MODELS)/gas_data.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/third-body-reaction.cxx -I$(LUA_INCLUDE_DIR)

energy-exchange-ODE-update.o : $(KINETICS)/energy-exchange-ODE-update.cxx \
	$(KINETICS)/energy-exchange-ODE-update.hh $(KINETICS)/energy-exchange-update.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/energy-exchange-ODE-update.cxx -I$(LUA_INCLUDE_DIR)

energy-exchange-system.o : $(KINETICS)/energy-exchange-system.cxx \
	$(KINETICS)/energy-exchange-system.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/energy-exchange-system.cxx -I$(LUA_INCLUDE_DIR)

energy-exchange-rate.o : $(KINETICS)/energy-exchange-rate.cxx \
	$(KINETICS)/energy-exchange-rate.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/energy-exchange-rate.cxx -I$(LUA_INCLUDE_DIR)

energy-exchange-mechanism.o : $(KINETICS)/energy-exchange-mechanism.cxx \
	$(KINETICS)/energy-exchange-mechanism.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/energy-exchange-mechanism.cxx -I$(LUA_INCLUDE_DIR)

energy-exchange-relaxation-time.o : $(KINETICS)/energy-exchange-relaxation-time.cxx \
	$(KINETICS)/energy-exchange-relaxation-time.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/energy-exchange-relaxation-time.cxx -I$(LUA_INCLUDE_DIR)

energy-exchange-update.o : $(KINETICS)/energy-exchange-update.cxx $(KINETICS)/energy-exchange-update.hh \
	$(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/energy-exchange-update.cxx -I$(LUA_INCLUDE_DIR)

thermal-equilibrium-mechanism.o : $(KINETICS)/thermal-equilibrium-mechanism.cxx \
	$(KINETICS)/thermal-equilibrium-mechanism.hh $(LUA_INCLUDE_DIR)
	$(CXXCOMPILE) $(CXXFLAG) $(KINETICS)/thermal-equilibrium-mechanism.cxx -I$(LUA_INCLUDE_DIR)

#------------------------------------------
# Test executables
#

user-defined-gas-model-test.x : $(MODELS)/user-defined-gas-model-test.cxx \
	$(LIBGAS) $(LIBLUA) $(LIBZLIB)
	$(CXXLINK) $(LFLAG) -o user-defined-gas-model-test.x $(MODELS)/user-defined-gas-model-test.cxx \
	$(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LUALINK) -I$(LUA_INCLUDE_DIR)

composite-gas-model-test.x : $(MODELS)/composite-gas-model-test.cxx $(LIBGAS) \
        $(LIBLUA)
	$(CXXLINK) $(LFLAG) -o composite-gas-model-test.x $(MODELS)/composite-gas-model-test.cxx \
	$(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LUALINK) -I$(LUA_INCLUDE_DIR)

# perfect-gas-EOS-test.x : $(MODELS)/perfect-gas-EOS-test.cxx $(LIBGAS) $(LIBLUA)
# 	$(CXXLINK) $(LFLAG) -o perfect-gas-EOS-test.x $(MODELS)/perfect-gas-EOS-test.cxx \
# 	$(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LUALINK) -I$(LUA_INCLUDE_DIR)

# noble-abel-gas-EOS-test.x : $(MODELS)/noble-abel-gas-EOS-test.cxx $(LIBGAS) $(LIBLUA)
# 	$(CXXLINK) $(LFLAG) -o noble-abel-gas-EOS-test.x $(MODELS)/noble-abel-gas-EOS-test.cxx \
# 	$(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LUALINK) -I$(LUA_INCLUDE_DIR)

generalised-Arrhenius-test.x : $(KINETICS)/generalised-Arrhenius-test.cxx $(LIBGAS) $(LIBLUA)
	$(CXXLINK) $(LFLAG) -o generalised-Arrhenius-test.x $(KINETICS)/generalised-Arrhenius-test.cxx \
	$(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LUALINK) -I$(LUA_INCLUDE_DIR)

reaction-update-test.x : $(KINETICS)/reaction-update-test.cxx $(LIBGAS) $(LIBLUA)
	$(CXXLINK) $(LFLAG) -o reaction-update-test.x $(KINETICS)/reaction-update-test.cxx \
	$(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LUALINK) -I$(LUA_INCLUDE_DIR)

cpu-chem-test : $(KINETICS)/cpu-chem-test.cxx $(LIBGAS) $(LIBLUA)
	$(CXXLINK) $(LFLAG) -o cpu-chem-test $(KINETICS)/cpu-chem-test.cxx \
	$(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LUALINK) -I$(LUA_INCLUDE_DIR)

cpu-qss-step-test : $(KINETICS)/cpu-qss-step-test.cxx $(LIBGAS) $(LIBLUA)
	$(CXXLINK) $(LFLAG) -o cpu-qss-step-test $(KINETICS)/cpu-qss-step-test.cxx \
	$(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LUALINK) -I$(LUA_INCLUDE_DIR)

gpu-chem-test : $(KINETICS)/gpu-chem-test.cxx $(LIBGAS) $(LIBLUA)
	$(CXXLINK) $(LFLAG) -o gpu-chem-test $(KINETICS)/gpu-chem-test.cxx \
	$(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LUALINK) -I$(LUA_INCLUDE_DIR) 


minimal-implementation-gas-model-test.x : $(MODELS)/minimal-implementation-gas-model.hh $(MODELS)/minimal-implementation-gas-model.cxx \
	$(MODELS)/gas_data.hh $(MODELS)/gas-model.hh gas-model.o
	$(CXXLINK) $(LFLAG) -o minimal-implementation-gas-model-test.x $(MODELS)/minimal-implementation-gas-model-test.cxx $(MODELS)/minimal-implementation-gas-model.cxx \
	gas-model.o

gas-module-test.lua : $(MODELS)/gas-module-test.lua
	cp $(MODELS)/gas-module-test.lua .

noneq-test.py : $(MODELS)/noneq-test.py
	cp $(MODELS)/noneq-test.py .

noneq-test.x : $(MODELS)/noneq-test.cxx $(LIBGAS) $(LIBLUA)
	$(CXXLINK) $(LFLAG) -o noneq-test.x $(MODELS)/noneq-test.cxx \
	$(LIBGAS) $(LIBLUA) $(LIBZLIB) $(LUALINK) -I$(LUA_INCLUDE_DIR)

#------------------------------------------
# Lua modules
#


ideal_gas.lua : $(TOOLS)/ideal_gas.lua
	cp $(TOOLS)/ideal_gas.lua .

Noble_Abel_gas.lua : $(TOOLS)/Noble_Abel_gas.lua
	cp $(TOOLS)/Noble_Abel_gas.lua .

van_der_Waals_gas.lua : $(TOOLS)/van_der_Waals_gas.lua
	cp $(TOOLS)/van_der_Waals_gas.lua .

real_gas_Bender.lua : $(TOOLS)/real_gas_Bender.lua
	cp $(TOOLS)/real_gas_Bender.lua .

simple_gas.lua : $(TOOLS)/simple_gas.lua
	cp $(TOOLS)/simple_gas.lua .

real_gas_MBWR.lua : $(TOOLS)/real_gas_MBWR.lua
	cp $(TOOLS)/real_gas_MBWR.lua .

refprop.lua : $(TOOLS)/refprop.lua
	cp $(TOOLS)/refprop.lua .

thermally_perfect_gas.lua : $(TOOLS)/thermally_perfect_gas.lua
	cp $(TOOLS)/thermally_perfect_gas.lua .

four_temperature_gas.lua : $(TOOLS)/four_temperature_gas.lua
	cp $(TOOLS)/four_temperature_gas.lua .

three_temperature_gas.lua : $(TOOLS)/three_temperature_gas.lua
	cp $(TOOLS)/three_temperature_gas.lua .

two_temperature_gas.lua : $(TOOLS)/two_temperature_gas.lua
	cp $(TOOLS)/two_temperature_gas.lua .

one_temperature_gas.lua : $(TOOLS)/one_temperature_gas.lua
	cp $(TOOLS)/one_temperature_gas.lua .

fully_coupled_one_temperature_gas.lua : $(TOOLS)/fully_coupled_one_temperature_gas.lua
	cp $(TOOLS)/fully_coupled_one_temperature_gas.lua .

fully_coupled_two_temperature_gas.lua : $(TOOLS)/fully_coupled_two_temperature_gas.lua
	cp $(TOOLS)/fully_coupled_two_temperature_gas.lua .

fully_coupled_four_temperature_gas.lua : $(TOOLS)/fully_coupled_four_temperature_gas.lua
	cp $(TOOLS)/fully_coupled_four_temperature_gas.lua .

multi_temperature_gas.lua : $(TOOLS)/multi_temperature_gas.lua
	cp $(TOOLS)/multi_temperature_gas.lua .

tab_serialise.lua : $(TOOLS)/tab_serialise.lua
	cp $(TOOLS)/tab_serialise.lua .

reaction_parser.lua : $(KINETICS)/reaction_parser.lua
	cp $(KINETICS)/reaction_parser.lua .

reac.lua : $(KINETICS)/reac.lua
	cp $(KINETICS)/reac.lua .

reaction_rate.lua : $(KINETICS)/reaction_rate.lua
	cp $(KINETICS)/reaction_rate.lua .

energy_exchange_parser.lua : $(KINETICS)/energy_exchange_parser.lua
	cp $(KINETICS)/energy_exchange_parser.lua .

exch.lua : $(KINETICS)/exch.lua
	cp $(KINETICS)/exch.lua .

lex_elems.lua : $(KINETICS)/lex_elems.lua
	cp $(KINETICS)/lex_elems.lua .
#------------------------------------------
# Tools
#

gasfile : $(TOOLS)/gasfile.lua
	cp $(TOOLS)/gasfile.lua gasfile
	chmod +x gasfile

reac-lua2txt : $(TOOLS)/reac-lua2txt.lua
	cp $(TOOLS)/reac-lua2txt.lua reac-lua2txt
	chmod +x reac-lua2txt

build-cea-lut.py : $(TOOLS)/build-cea-lut.py
	cp $(TOOLS)/build-cea-lut.py .

# If the following command is attempted and fails,
# try going to the cea2 source area and manually compile.
# The -m32 option sometimes gives grief 
# (on 64-bit linux systems with dodgy compatability libraries) 
# but it seems to be needed to make a well behaved executable. 
cea2 :
	- gfortran -m32 -std=legacy -o cea2 $(CEA2)/cea2.f
	cp $(CEA2)/thermo.inp .
	cp $(CEA2)/trans.inp .

thermo.inp : $(CEA2)/thermo.inp
	cp $(CEA2)/thermo.inp .

trans.inp : $(CEA2)/trans.inp
	cp $(CEA2)/trans.inp .

gasmodel.py : $(TOOLS)/gasmodel.py
	cp $(TOOLS)/gasmodel.py gasmodel.py
	chmod +x gasmodel.py

build-REFPROP-lut.py : $(TOOLS)/build-REFPROP-lut.py
	cp $(TOOLS)/build-REFPROP-lut.py .

refprop.so :
	cd $(REFPROP)/build; make
	cp $(REFPROP)/build/$@ .
	cd $(REFPROP)/build; make all-clean
