-- Author: Daniel F. Potter
-- Date: 30-Sep-2009
-- Place: UQ, St Lucia, QLD

module(..., package.seeall)

-----
-- Adapted from Listing 12.2 in
-- Programming in Lua, 2nd edition
-----
function serialise(o, f, indent)
   indent = indent or ""
   if type(o) == "number" then
      f:write(o)
   elseif type(o) == "string" then
      f:write(string.format("%q", o))
   elseif type(o) == "table" then
      local old_indent = indent
      indent = indent.."  "
      f:write("{\n")
      for k,v in pairs(o) do
	 if type(k) == "string" then
	    f:write(string.format("%s%s = ", indent, k))
	 else
	    f:write(string.format("%s", indent))
	 end
	 serialise(v, f, indent)
	 f:write(",\n")
      end
      indent = old_indent
      f:write(string.format("%s}", indent))
   else
      error("cannot serialise a " .. type(o))
   end
end

function list_available_species()
   e3bin = os.getenv("E3BIN") or os.getenv("HOME").."/e3bin"
   dir = e3bin.."/species"
   tmpname = os.tmpname()
   os.execute(string.format("ls -1 %s/*.lua > %s", dir, tmpname))
   tmpfile = assert(io.open(tmpname, "r"))
   species = {}
   for line in tmpfile:lines() do
      sp = string.match(line, "([%a%d_plus]+).lua")
      species[sp] = true
   end
   tmpfile:close()
   os.execute(string.format("rm %s", tmpname))
   return species
end

local CI_folders = { "GuptaYos", "Wright", "Bruno", "Mars", "None" }
CI_folders["GuptaYos"] = "gupta-yos"
CI_folders["Wright"] = "wright"
CI_folders["Bruno"] = "bruno"
CI_folders["Mars"] = "mars"
CI_folders["None"] = "none"

function list_available_collision_integrals( ref )
   e3bin = os.getenv("E3BIN") or os.getenv("HOME").."/e3bin"
   dir = e3bin.."/collision-integrals/"..CI_folders[ref]
   tmpname = os.tmpname()
   os.execute(string.format("ls -1 %s/*.lua > %s", dir, tmpname))
   tmpfile = assert(io.open(tmpname, "r"))
   CIs = {}
   for line in tmpfile:lines() do
      species_pairs = string.sub( line, string.len( dir )+2, string.len( line )-4 )
      hyphen = string.find(species_pairs,'-')
      sp_i = string.sub( species_pairs, 1, hyphen-1 )
      sp_j = string.sub( species_pairs, hyphen+1 )
      if not CIs[sp_i] then CIs[sp_i] = {} end
      CIs[sp_i][sp_j]=true
      -- print(string.format("sp_i = %s, sp_j = %s",sp_i,sp_j))
   end
   tmpfile:close()
   os.execute(string.format("rm %s", tmpname))
   return CIs
end

local monatomic_type_list = { "species_type" }
local electron_type_list = { "species_type" }
local diatomic_type_list = { "species_type", "oscillator_type" }
local polyatomic_type_list = { "species_type", "oscillator_type" }
local base_value_list = { "M", "s_0", "h_f", "I", "Z", "eps0", "sigma" }
local diatomic_value_list = { "M", "s_0", "h_f", "I", "Z", "eps0", "sigma", "r0", "r_eq", "f_m", "mu", "alpha", "mu_B" }

local default = {}
default.min_massf = 1.0e-10
default.T_min = 20.0
default.T_max = 100000.0
default.iterative_method = 'NewtonRaphson'
default.convergence_tolerance = 1.0e-6
default.max_iterations = 100
default.oscillator_type = 'truncated harmonic'
default.n_levels = 2

function create_one_temperature_gas(species, f)
   f:write(string.format("-- Auto-generated by gasfile on: %s\n",
			 os.date("%d-%b-%Y %X")))
   f:write("model = 'composite gas'\n")
   f:write("equation_of_state = 'perfect gas'\n")
   f:write("thermal_behaviour = 'thermal nonequilibrium'\n")
   f:write("mixing_rule = 'GuptaYos'\n")
   f:write("sound_speed = 'equilibrium'\n")
   f:write("diffusion_coefficients = 'GuptaYos'\n")
   f:write(string.format("min_massf = %e\n\n", default.min_massf))
   
   f:write("thermal_modes = { 'all' }\n\n")
   f:write("all = {}\n")
   f:write("all.type = 'variable Cv'\n")
   f:write("all.iT = 0\n")
   f:write("all.components = { 'all-translation', 'all-rotation', 'all-vibration', 'all-electronic' }\n\n")
   f:write(string.format("all.T_min = %f\n",default.T_min))
   f:write(string.format("all.T_max = %f\n",default.T_max))
   f:write(string.format("all.iterative_method = '%s'\n",default.iterative_method))
   f:write(string.format("all.convergence_tolerance = %e\n",default.convergence_tolerance))
   f:write(string.format("all.max_iterations = %d\n\n",default.max_iterations))
   
   species_avail = list_available_species()
   f:write("species = {")
   for _,sp in ipairs(species) do
      if not species_avail[sp] then
	 print(string.format("Species: %s cannot be found in the collection of species.\n", sp))
	 print("Check for an appropriate file in:\n")
	 e3bin = os.getenv("E3BIN") or os.getenv("HOME").."/e3bin"
	 dir = e3bin.."/species"
	 print("   ", dir)
	 print("Bailing out!\n")
	 os.exit(1)
      end
      f:write(string.format("'%s', ", sp))
   end
   f:write("}\n\n")
   
   for _,sp in ipairs(species) do
      e3bin = os.getenv("E3BIN") or os.getenv("HOME").."/e3bin"
      dir = e3bin.."/species/"
      file = dir..sp..".lua"
      dofile(file)
      
      f:write(string.format("%s = {}\n", sp))
      
      if string.find(_G[sp]["species_type"],"diatomic") then
         type_list = diatomic_type_list
         value_list = diatomic_value_list
      elseif string.find(_G[sp]["species_type"],"monatomic") then
         type_list = monatomic_type_list
         value_list = base_value_list
      elseif string.find(_G[sp]["species_type"],"free electron") then
         type_list = electron_type_list
         value_list = base_value_list
      elseif string.find(_G[sp]["species_type"],"polyatomic") then
         type_list = polyatomic_type_list
         value_list = base_value_list
      else
         print(string.format("Could not decode the 'type' given for species: %s\n", sp))
         print("Bailing out!\n")
         os.exit(1)
      end
      
      for __,val in ipairs(type_list) do
	 var = sp.."."..val
	 f:write(string.format("%s = ", var))
	 if _G[sp][val] then
	    serialise(_G[sp][val], f)
	 else
	    serialise(default[val], f)
	 end
	 f:write("\n")
      end

      for __,val in ipairs(value_list) do
         if _G[sp][val] then
            var = sp.."."..val
	    f:write(string.format("%s = ", var))
            serialise(_G[sp][val], f)
	    f:write("\n")
	 end
      end
      
      -- electronic levels
      val = "electronic_levels"
      f:write(string.format("%s = {\n", sp.."."..val))
      n_levels = _G[sp][val]["n_levels"]
      -- n_levels = default.n_levels
      -- some special species have a fixed number of levels
      if sp=="e_minus" or sp=="H_plus" then
         n_levels = 1
      end
      if _G[sp][val]["n_levels"] < n_levels then
         print(string.format("Requested number of electronic levels (%d) for species: %s\n", n_levels, sp))
         print(string.format("exceeds the levels for which there is data (%s).\n", _G[sp][val]["n_levels"] ))
         print("Bailing out!\n")
         os.exit(1)
      end
      f:write(string.format("  n_levels = %d,\n", n_levels ))
      f:write(string.format("  ref = %q,\n", _G[sp][val]["ref"] ))
      for ilev=0,(n_levels-1) do
         ilev_str = string.format("ilev_%d", ilev)
         f:write(string.format("  %s = ", ilev_str))
         serialise(_G[sp][val][ilev_str], f, "  ")
         f:write(",\n")
      end
      f:write("}\n")
      
      -- CEA coefficients
      val = "CEA_coeffs"
      f:write(string.format("%s = ", sp.."."..val))
      serialise(_G[sp].CEA_coeffs, f)
      f:write(string.format("\n"))
      
      -- transport coefficient models
      val = "viscosity"
      f:write(string.format("%s = {\n", sp.."."..val))
      f:write(string.format("  model = 'collision integrals'\n}\n"))
      val = "thermal_conductivity"
      f:write(string.format("%s = {\n", sp.."."..val))
      f:write(string.format("  model = 'collision integrals'\n}\n"))
   end
      
   -- collision integrals
   CI_ref = "Mars"
   e3bin = os.getenv("E3BIN") or os.getenv("HOME").."/e3bin"
   dir = e3bin.."/collision-integrals/"..CI_folders[CI_ref]
   available_CIs = list_available_collision_integrals( CI_ref )
   f:write(string.format("collision_integrals = {\n"))
   for isp,sp_i in ipairs(species) do
      if isp==1 then start=nil else start=isp-1 end
      for jsp,sp_j in next,species,start do
         file="not found"
         if available_CIs[sp_i] then
            if available_CIs[sp_i][sp_j] then
               file = dir.."/"..sp_i.."-"..sp_j..".lua"
            end
         end
         if file=="not found" and available_CIs[sp_j] then
            if available_CIs[sp_j][sp_i] then
               file = dir.."/"..sp_j.."-"..sp_i..".lua"
            end
         end
         -- print(string.format("sp_i = %s, sp_j = %s, file = %s",sp_i,sp_j,file))
         if file=="not found" then
            print(string.format("No %s-%s or %s-%s collision integral data available.", sp_i, sp_j, sp_j, sp_i))
            print("Setting all coefficients to 0.0.\n")
            e3bin = os.getenv("E3BIN") or os.getenv("HOME").."/e3bin"
	    file = e3bin.."/collision-integrals/none-none.lua"
         end
         dofile(file)
         f:write(string.format("  {\n"))
         f:write(string.format("    i = "))
         -- serialise(_G["CI"]["i"],f)
         serialise(sp_i,f)
         f:write(string.format(",\n"))
         f:write(string.format("    j = "))
         -- serialise(_G["CI"]["j"],f)
         serialise(sp_j,f)
         f:write(string.format(",\n"))
         f:write(string.format("    reference = "))
         serialise(_G["CI"]["reference"],f)
         f:write(string.format(",\n"))
         f:write(string.format("    model = "))
         serialise(_G["CI"]["model"],f)
         f:write(string.format(",\n"))
         f:write(string.format("    parameters = "))
         serialise(_G["CI"]["parameters"],f,"    ")
         f:write(string.format(",\n  },\n"))
         -- print(string.format("sp_i[%d] = %s, sp_j[%d] = %s\n", isp, sp_i, jsp, sp_j ))
      end
   end
   f:write(string.format("\n}\n"))

end
