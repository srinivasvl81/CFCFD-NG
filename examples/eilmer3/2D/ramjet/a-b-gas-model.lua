-- Auto-generated by gasfile on: 17-May-2009 20:49:48
-- and edited manually by PJ, 21-Jan-2010 for Rowan's oblique detonation case.
-- Further edited for the ramjet example brought over from mbcns2 22-Apr-2011.
model = 'composite gas'
equation_of_state = 'perfect gas'
thermal_behaviour = 'constant specific heats'
sound_speed = 'equilibrium'
mixing_rule = 'Wilke'
diffusion_coefficients = 'hard sphere'
ignore_mole_fraction = 1.0e-15
species = {'AA', 'BB', 'AB', }

AA = {}
AA.M = {
  value = 8.314/287.0,
  reference = "Air (reactant A).",
  description = "molecular mass",
  units = "kg/mol",
}
AA.gamma = {
  value = 1.4,
  reference = "Air (reactant A).",
  description = "(ideal) ratio of specific heats at room temperature",
  units = "non-dimensional",
}
AA.d = {
  value = 3.617e-10,
  reference = "Bird, Stewart and Lightfoot (2001), p. 864",
  description = "air value: equivalent hard-sphere diameter, sigma from L-J parameters",
  units = "m",
}
AA.e_zero = {
  value = 0,
  description = "reference energy",
  units = "J/kg",
}
AA.q = {
  value = 0,
  description = "heat release",
  units = "J/kg",
}
AA.viscosity = {
  parameters = {
    T_ref = 273,
    ref = "Table 1-2, White (2006)",
    S = 111,
    mu_ref = 1.716e-05,
  },
  model = "Sutherland",
}
AA.thermal_conductivity = {
  parameters = {
    S = 194,
    ref = "Table 1-3, White (2006)",
    k_ref = 0.0241,
    T_ref = 273,
  },
  model = "Sutherland",
}

BB = {}
BB.M = {
  value = 8.314/287.0,
  reference = "Fuel (reactant B).",
  description = "molecular mass",
  units = "kg/mol",
}
BB.gamma = {
  value = 1.4,
  reference = "Fuel (reactant B).",
  description = "(ideal) ratio of specific heats at room temperature",
  units = "non-dimensional",
}
BB.d = {
  value = 3.617e-10,
  reference = "Bird, Stewart and Lightfoot (2001), p. 864",
  description = "air value: equivalent hard-sphere diameter, sigma from L-J parameters",
  units = "m",
}
BB.e_zero = {
  value = 0,
  description = "reference energy",
  units = "J/kg",
}
BB.q = {
  value = 0,
  description = "heat release",
  units = "J/kg",
}
BB.viscosity = {
  parameters = {
    T_ref = 273,
    ref = "Table 1-2, White (2006)",
    S = 111,
    mu_ref = 1.716e-05,
  },
  model = "Sutherland",
}
BB.thermal_conductivity = {
  parameters = {
    S = 194,
    ref = "Table 1-3, White (2006)",
    k_ref = 0.0241,
    T_ref = 273,
  },
  model = "Sutherland",
}

AB = {}
AB.M = {
  value = 8.314/287.0,
  reference = "Burnt air-fuel mixture (products).",
  description = "molecular mass",
  units = "kg/mol",
}
AB.gamma = {
  value = 1.2,
  reference = "Burnt air-fuel mixture (products).",
  description = "(ideal) ratio of specific heats at room temperature",
  units = "non-dimensional",
}
AB.d = {
  value = 3.617e-10,
  reference = "Bird, Stewart and Lightfoot (2001), p. 864",
  description = "air value: equivalent hard-sphere diameter, sigma from L-J parameters",
  units = "m",
}
AB.e_zero = {
  value = 0,
  description = "reference energy",
  units = "J/kg",
}
AB.q = {
  value = 3.0e6,
  description = "heat release",
  units = "J/kg",
}
AB.viscosity = {
  parameters = {
    T_ref = 273,
    ref = "Table 1-2, White (2006)",
    S = 111,
    mu_ref = 1.716e-05,
  },
  model = "Sutherland",
}
AB.thermal_conductivity = {
  parameters = {
    S = 194,
    ref = "Table 1-3, White (2006)",
    k_ref = 0.0241,
    T_ref = 273,
  },
  model = "Sutherland",
}
