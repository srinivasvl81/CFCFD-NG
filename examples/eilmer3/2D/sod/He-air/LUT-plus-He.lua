-- Auto-generated by gasfile on: 08-Jul-2010 02:02:28
-- modified by create_gas_file() to add LUT
lut_file = 'cea-lut-air.lua.gz'
model = 'LUT-plus-composite'
equation_of_state = 'perfect gas'
thermal_behaviour = 'constant specific heats'
mixing_rule = 'Wilke'
sound_speed = 'equilibrium'
ignore_mole_fraction = 1.0e-15
species = {'He', }

He = {}
He.M = {
  value = 0.004002602,
  reference = "cea2::thermo.inp",
  description = "molecular mass",
  units = "kg/mol",
}
He.gamma = {
  value = 1.6666666666667,
  reference = "monatomic gas",
  description = "(ideal) ratio of specific heats at room temperature",
  units = "non-dimensional",
}
He.d = {
  value = 2.576e-10,
  reference = "Bird, Stewart and Lightfoot (2001), p. 864",
  description = "equivalent hard-sphere diameter, sigma from L-J parameters",
  units = "m",
}
He.e_zero = {
  value = 0,
  description = "reference energy",
  units = "J/kg",
}
He.q = {
  value = 0,
  description = "heat release",
  units = "J/kg",
}
He.viscosity = {
  parameters = {
    {
      A = 0.75015944,
      C = -2212.1291,
      B = 35.763243,
      T_high = 1000,
      T_low = 200,
      D = 0.92126352,
    },
    {
      A = 0.83394166,
      C = -52852.591,
      B = 220.82656,
      T_high = 5000,
      T_low = 1000,
      D = 0.20809361,
    },
    {
      A = 0.86316349,
      C = -1249870.5,
      B = 962.05176,
      T_high = 15000,
      T_low = 5000,
      D = -0.14115714,
    },
    ref = "from CEA2::trans.inp which cites Bich et al. (1990)",
  },
  model = "CEA",
}
He.thermal_conductivity = {
  parameters = {
    {
      A = 0.75007833,
      C = -2363.66,
      B = 36.577987,
      T_high = 1000,
      T_low = 200,
      D = 2.9766475,
    },
    {
      A = 0.83319259,
      C = -53304.53,
      B = 221.57417,
      T_high = 5000,
      T_low = 1000,
      D = 2.2684592,
    },
    {
      A = 0.85920953,
      C = -1106926.2,
      B = 898.73206,
      T_high = 15000,
      T_low = 5000,
      D = 1.9535742,
    },
    ref = "from CEA2::trans.inp which cites Bich et al. (1990)",
  },
  model = "CEA",
}
