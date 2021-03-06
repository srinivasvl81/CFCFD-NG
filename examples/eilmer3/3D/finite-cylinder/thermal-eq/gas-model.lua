-- Auto-generated by gasfile on: 23-Jan-2010 23:21:55
model = 'composite gas'
equation_of_state = 'perfect gas'
thermal_behaviour = 'thermally perfect'
mixing_rule = 'Wilke'
sound_speed = 'equilibrium'
diffusion_coefficients = 'hard sphere'
ignore_mole_fraction = 1.0e-15
T_COLD = 20.0
species = {'N2', 'N', 'N2_plus', 'N_plus', 'e_minus', }

N2 = {}
N2.M = {
  value = 0.028013,
  reference = "from CEA2::thermo.inp",
  description = "molecular mass",
  units = "kg/mol",
}
N2.d = {
  value = 3.667e-10,
  reference = "Bird, Stewart and Lightfoot (2001), p. 864",
  description = "equivalent hard sphere diameter, based on L-J parameters",
  units = "m",
}
N2.viscosity = {
  parameters = {
    T_ref = 273,
    ref = "Table 1-2, White (2006)",
    S = 107,
    mu_ref = 1.663e-05,
  },
  model = "Sutherland",
}
N2.thermal_conductivity = {
  parameters = {
    S = 150,
    ref = "Table 1-3, White (2006)",
    k_ref = 0.0242,
    T_ref = 273,
  },
  model = "Sutherland",
}
N2.CEA_coeffs = {
  {
    T_high = 1000,
    T_low = 200,
    coeffs = {
      22103.71497,
      -381.846182,
      6.08273836,
      -0.00853091441,
      1.384646189e-05,
      -9.62579362e-09,
      2.519705809e-12,
      710.846086,
      -10.76003744,
    },
  },
  {
    T_high = 6000,
    T_low = 1000,
    coeffs = {
      587712.406,
      -2239.249073,
      6.06694922,
      -0.00061396855,
      1.491806679e-07,
      -1.923105485e-11,
      1.061954386e-15,
      12832.10415,
      -15.86640027,
    },
  },
  {
    T_high = 20000,
    T_low = 6000,
    coeffs = {
      831013916,
      -642073.354,
      202.0264635,
      -0.03065092046,
      2.486903333e-06,
      -9.70595411e-11,
      1.437538881e-15,
      4938707.04,
      -1672.09974,
    },
  },
  ref = "from CEA2::thermo.inp",
}
N = {}
N.M = {
  value = 0.0140067,
  reference = "CEA2::thermo.inp",
  description = "molecular mass",
  units = "kg/mol",
}
N.d = {
  value = 3.617e-10,
  reference = "value for air: Bird, Stewart and Lightfoot (2001), p. 864",
  description = "equivalent hard-sphere diameter, sigma from L-J parameters",
  units = "m",
}
N.viscosity = {
  parameters = {
    {
      A = 0.83724737,
      C = -174507.53,
      B = 439.9715,
      T_high = 5000,
      T_low = 1000,
      D = 0.10365689,
    },
    {
      A = 0.89986588,
      C = -1820047.8,
      B = 1411.2801,
      T_high = 15000,
      T_low = 5000,
      D = -0.55811716,
    },
    ref = "from CEA2::trans.inp which cites Levin et al (1990)",
  },
  model = "CEA",
}
N.thermal_conductivity = {
  parameters = {
    {
      A = 0.83771661,
      C = -175784.46,
      B = 442.4327,
      T_high = 5000,
      T_low = 1000,
      D = 0.89942915,
    },
    {
      A = 0.9000171,
      C = -1826240.3,
      B = 1414.1175,
      T_high = 15000,
      T_low = 5000,
      D = 0.24048513,
    },
    ref = "from CEA2::trans.inp which cites Levin et al (1990)",
  },
  model = "CEA",
}
N.CEA_coeffs = {
  {
    T_high = 1000,
    T_low = 200,
    coeffs = {
      0,
      0,
      2.5,
      0,
      0,
      0,
      0,
      56104.6378,
      4.193905036,
    },
  },
  {
    T_high = 6000,
    T_low = 1000,
    coeffs = {
      88765.0138,
      -107.12315,
      2.362188287,
      0.0002916720081,
      -1.7295151e-07,
      4.01265788e-11,
      -2.677227571e-15,
      56973.5133,
      4.865231506,
    },
  },
  {
    T_high = 20000,
    T_low = 6000,
    coeffs = {
      547518105,
      -310757.498,
      69.1678274,
      -0.00684798813,
      3.8275724e-07,
      -1.098367709e-11,
      1.277986024e-16,
      2550585.618,
      -584.8769753,
    },
  },
  ref = "from CEA2::thermo.inp",
}
N2_plus = {}
N2_plus.M = {
  value = 0.0280128514,
  reference = "from CEA2::thermo.inp",
  description = "molecular mass",
  units = "kg/mol",
}
N2_plus.d = {
  value = 3.617e-10,
  reference = "value for air: Bird, Stewart and Lightfoot (2001), p. 864",
  description = "equivalent hard-sphere diameter, sigma from L-J parameters",
  units = "m",
}
N2_plus.viscosity = {
  parameters = {
    T_ref = 273,
    ref = "Table 1-2, White (2006)",
    S = 111,
    mu_ref = 1.716e-05,
  },
  model = "Sutherland",
}
N2_plus.thermal_conductivity = {
  parameters = {
    S = 194,
    ref = "Table 1-3, White (2006)",
    k_ref = 0.0241,
    T_ref = 273,
  },
  model = "Sutherland",
}
N2_plus.CEA_coeffs = {
  {
    T_high = 1000,
    T_low = 298.15,
    coeffs = {
      -34740.4747,
      269.6222703,
      3.16491637,
      -0.002132239781,
      6.7304764e-06,
      -5.63730497e-09,
      1.621756e-12,
      179000.4424,
      6.832974166,
    },
  },
  {
    T_high = 6000,
    T_low = 1000,
    coeffs = {
      -2845599.002,
      7058.89303,
      -2.884886385,
      0.003068677059,
      -4.36165231e-07,
      2.102514545e-11,
      5.41199647e-16,
      134038.8483,
      50.90897022,
    },
  },
  {
    T_high = 20000,
    T_low = 6000,
    coeffs = {
      -371282977,
      313928.7234,
      -96.0351805,
      0.01571193286,
      -1.175065525e-06,
      4.14444123e-11,
      -5.62189309e-16,
      -2217361.867,
      843.6270947,
    },
  },
}
N_plus = {}
N_plus.M = {
  value = 0.0140061514,
  reference = "CEA2::thermo.inp",
  description = "molecular mass",
  units = "kg/mol",
}
N_plus.d = {
  value = 3.617e-10,
  reference = "value for air: Bird, Stewart and Lightfoot (2001), p. 864",
  description = "equivalent hard-sphere diameter, sigma from L-J parameters",
  units = "m",
}
N_plus.viscosity = {
  parameters = {
    T_ref = 273,
    ref = "Table 1-2, White (2006)",
    S = 111,
    mu_ref = 1.716e-05,
  },
  model = "Sutherland",
}
N_plus.thermal_conductivity = {
  parameters = {
    S = 194,
    ref = "Table 1-3, White (2006)",
    k_ref = 0.0241,
    T_ref = 273,
  },
  model = "Sutherland",
}
N_plus.CEA_coeffs = {
  {
    T_high = 1000,
    T_low = 298.15,
    coeffs = {
      5237.07921,
      2.299958315,
      2.487488821,
      2.737490756e-05,
      -3.134447576e-08,
      1.850111332e-11,
      -4.447350984e-15,
      225628.4738,
      5.076830786,
    },
  },
  {
    T_high = 6000,
    T_low = 1000,
    coeffs = {
      290497.0374,
      -855.790861,
      3.47738929,
      -0.000528826719,
      1.352350307e-07,
      -1.389834122e-11,
      5.046166279e-16,
      231080.9984,
      -1.994146545,
    },
  },
  {
    T_high = 20000,
    T_low = 6000,
    coeffs = {
      16460921.48,
      -11131.65218,
      4.97698664,
      -0.0002005393583,
      1.022481356e-08,
      -2.691430863e-13,
      3.539931593e-18,
      313628.4696,
      -17.0664638,
    },
  },
}
e_minus = {}
e_minus.M = {
  value = 5.48579903e-07,
  reference = "CEA2::therm.inp",
  description = "molecular mass",
  units = "kg/mol",
}
e_minus.d = {
  value = 3.617e-10,
  reference = "value for air: Bird, Stewart and Lightfoot (2001), p. 864",
  description = "equivalent hard-sphere diameter, sigma from L-J parameters",
  units = "m",
}
e_minus.viscosity = {
  parameters = {
    T_ref = 273,
    ref = "Table 1-2, White (2006)",
    S = 111,
    mu_ref = 1.716e-05,
  },
  model = "Sutherland",
}
e_minus.thermal_conductivity = {
  parameters = {
    S = 194,
    ref = "Table 1-3, White (2006)",
    k_ref = 0.0241,
    T_ref = 273,
  },
  model = "Sutherland",
}
e_minus.CEA_coeffs = {
  {
    T_high = 1000,
    T_low = 298.15,
    coeffs = {
      0,
      0,
      2.5,
      0,
      0,
      0,
      0,
      -745.375,
      -11.72081224,
    },
  },
  {
    T_high = 6000,
    T_low = 1000,
    coeffs = {
      0,
      0,
      2.5,
      0,
      0,
      0,
      0,
      -745.375,
      -11.72081224,
    },
  },
  {
    T_high = 20000,
    T_low = 6000,
    coeffs = {
      0,
      0,
      2.5,
      0,
      0,
      0,
      0,
      -745.375,
      -11.72081224,
    },
  },
}
