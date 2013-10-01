#!/bin/bash

# 1. Prepare gas file
gasfile argon-3sp-2T.inp argon-3sp-2T.lua

# 2. Run simulation
poshax3.x argon.cfg

# 3. Plot results
gnuplot profiles.gplot

# 4. Convert to png files
convert -density 600x600 -quality 90 temperature_profiles.eps temperature_profiles.png 
convert -density 600x600 -quality 90 ionization_fraction_profile.eps ionization_fraction_profile.png

# 5. Compute RMS error with UTIAS shock tube measurements
python compute_errors.py 
