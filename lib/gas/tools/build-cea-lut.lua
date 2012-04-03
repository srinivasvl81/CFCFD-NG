#!/usr/bin/env lua
-- Author: Rowan J. Gollan
-- Date: 10-Nov-2008
-- Place: Hampton, Virginia, USA
-- Notes:
--   Some code is ported from
--   'cea_driver.tcl' written by PJ.
--

local pow, sqrt, abs = math.pow, math.sqrt, math.abs

local CEA_PROGRAM = 'cea2'
local CEA_THERMO_PFX = 'thermo'
local CEA_THERMO_FILE = 'thermo.inp'
local CEA_TRANS_PFX = 'trans'
local CEA_TRANS_FILE = 'trans.inp'
local tmp_stdin = 'input'
local tmp_inp_file = 'tmp.inp'
local tmp_out_file = 'tmp.out'
local tmp_plt_file = 'tmp.plt'

local log_rho_min = -6.0
local log_rho_max = 2.0
local irpoints = 51
local dlr = (log_rho_max - log_rho_min) / (irpoints-1)

local ne = 401

local u_offset = 0.0

local T_range = {}
T_range[1] = "200.0 250.0 300.0 400.0 500.0 600.0 700.0 800.0 900.0 1000.0 1100.0"
T_range[2] = "1200.0 1400.0 1600.0 1800.0 2000.0 2200.0 2400.0 2600.0 2800.0 3000.0"
T_range[3] = "3200.0 3400.0 3600.0 3800.0 4000.0 4200.0 4400.0 4600.0 4800.0 5000.0"
T_range[4] = "5200.0 5400.0 5600.0 5800.0 6000.0 6200.0 6400.0 6600.0 6800.0 7000.0"
T_range[5] = "7200.0 7400.0 7600.0 7800.0 8000.0 8200.0 8400.0 8600.0 8800.0 9000.0"
T_range[6] = "9200.0 9400.0 9600.0 9800.0 10000.0 10200.0 10400.0 10600.0 10800.0 11000.0"
T_range[7] = "11200.0 11400.0 11600.0 11800.0 12000.0 12200.0 12400.0 12600.0 12800.0 13000.0"
T_range[8] = "13200.0 13400.0 13600.0 13800.0 14000.0 14200.0 14400.0 14600.0 14800.0 15000.0"
T_range[9] = "15200.0 15400.0 15600.0 15800.0 16000.0 16200.0 16400.0 16600.0 16800.0 17000.0"
T_range[10] = "17200.0 17400.0 17600.0 17800.0 18000.0 18200.0 18400.0 18600.0 18800.0 19000.0"
T_range[11] = "19200.0 19400.0 19600.0 19800.0 20000.0 20200.0 20400.0 20600.0 20800.0 21000.0"

local function tally_str(s)
   tokens = {}
   for tk in string.gmatch(s, "%S+") do
      tokens[#tokens+1] = tk
   end
   return #tokens
end

local function split_str(s)
   tokens = {}
   for tk in string.gmatch(s, "%S+") do
      tokens[#tokens+1] = tk
   end
   return tokens
end


local itmax = 0
for i,Tstr in ipairs(T_range) do
   itmax = itmax + tally_str(Tstr)
end

function setup_cea()
   -- Copy thermo.inp and trans.inp to working directory
   file = os.getenv("HOME").."/e3bin/cea-cases/"..CEA_THERMO_FILE
   cmd = "cp "..file.." ."
   os.execute(cmd)

   file = os.getenv("HOME").."/e3bin/cea-cases/"..CEA_TRANS_FILE
   cmd = "cp "..file.." ."
   os.execute(cmd)

   -- Process thermo.inp and trans.inp
   inp = assert(io.open(tmp_stdin, 'w'))
   inp:write(string.format("%s\n", CEA_THERMO_PFX))
   inp:close()
   cmd = CEA_PROGRAM.." < "..tmp_stdin.." > junk"
   os.execute(cmd)

   inp = assert(io.open(tmp_stdin, 'w'))
   inp:write(string.format("%s\n", CEA_TRANS_PFX))
   inp:close()
   cmd = CEA_PROGRAM.." < "..tmp_stdin.." > junk"
   os.execute(cmd)

   -- cleanup
   cmd = "rm "..tmp_stdin.." junk"
   os.execute(cmd)
end

function cleanup_cea()
   local files = {CEA_THERMO_FILE,
		  CEA_THERMO_PFX..".out",
		  CEA_THERMO_PFX..".lib",
		  CEA_TRANS_FILE,
		  CEA_TRANS_PFX..".out",
		  CEA_TRANS_PFX..".lib",
	       }
   
   for _,f in ipairs(files) do
      cmd = "rm "..f
      os.execute(cmd)
   end
end

function write_inp_file(fname, reactants, rho, T_str, with_ions, only_list)
   f = assert(io.open(fname, 'w'))
   f:write(string.format("# %s generated by build-cea-lut.lua\n", fname))
   f:write("# assigned temperature and density\n")
   f:write("problem tv")
   if with_ions then
      f:write(" ions")
   end
   f:write("\n")

   f:write(string.format("  rho,kg/m**3 %s\n", rho))
   f:write(string.format("  t,k         %s\n", T_str))

   f:write("reac\n")


   for k,v in pairs(reactants) do
      f:write(string.format("   name = %s  ", k))
      if units == 'massf' then
	 f:write(string.format("wt%% = %g\n", v))
      else
	 f:write(string.format("moles = %g\n", v))
      end
   end

   f:write("output transport\n")
   f:write("   plot rho t u p son gam cp vis cond\n")
   
   if only_list then
      f:write(string.format("   only %s\n", only_list))
   end

   f:write("end\n")
   f:close()
end

function list_available_cases()
   dir = os.getenv("HOME").."/e3bin/cea-cases"
   tmpname = os.tmpname()
   os.execute(string.format("ls -1 %s/*.lua > %s", dir, tmpname))
   tmpfile = assert(io.open(tmpname, "r"))
   cases = {}
   for line in tmpfile:lines() do
      case = string.match(line, "([%a%d%-]+).lua")
      cases[case] = true
   end
   tmpfile:close()
   os.execute(string.format("rm %s", tmpname))
   return cases
end

local function pairsByKeys(t)
   local a = {}
   for n in pairs(t) do a[#a + 1] = n end
   table.sort(a)
   local i = 0
   return function ()
	     i = i + 1
	     return a[i], t[a[i]]
	  end
end

function print_usage()
   print "Usage: build-cea-lut (--case=caseLabel | --user-defined=inp.lua) [--extrapolate]"
   print "       where caseLabel is one of"
   cases = list_available_cases()
   for k,_ in pairsByKeys(cases) do
      print(string.format("           %s", k))
   end
   print ""
   print "  <OR> where inp.lua is a user-defined input template file."
   print ""
   print "       If '--extrapolate' is present, then use a linear extrapolation"
   print "       to fill in the edges of the look-up table."
   os.exit(1)
end

function add_data_to_table(t, plt_file)
   f = assert(io.open(plt_file, 'r'))
   
   while true do
      line = f:read("*line")
      if not line then break end
      tks = split_str(line)

      t[#t+1] = {rho=tonumber(tks[1]),
		 T=tonumber(tks[2]),
		 u=tonumber(tks[3]),
		 p=tonumber(tks[4]),
		 a=tonumber(tks[5]),
		 gamma=tonumber(tks[6]),
		 Cp=tonumber(tks[7]),
		 mu=tonumber(tks[8]),
		 k=tonumber(tks[9])
	      }
      -- Fix small vicosities and thermal conductivities
      if t[#t].mu == nil or t[#t].mu < 1.0e-10 then
	 t[#t].mu = 1.0e-10
      end
      
      if t[#t].k == nil or t[#t].k < 1.0e-10 then
	 t[#t].k = 1.0e-10
      end

   end
end

function set_u_offset(data)
   -- locate closest density to 1.0
   rho_target = 1.0
   ir_found = 0
   best_relative_error = 100.0
   drho = 0.0
   for ir=1,irpoints do
      drho = rho_target - data[ir][1].rho
      relative_error = abs(drho) / rho_target
      if relative_error < best_relative_error then
	 ir_found = ir
	 best_relative_error = relative_error
      end
   end

   if best_relative_error > 1.0e-4 then
      print("Density not found close enough:")
      print("index=", ir_found, " cea_rho=", data[ir_found][1].rho, " target_rho=", rho_target)
   end

   ir = ir_found
   it = 1
   T = data[ir][it].T
   p = data[ir][it].p * 100000.0 -- to get Pa
   Cp = data[ir][it].Cp * 1000.0 -- to get J/(kg.K)
   rho = data[ir][it].rho
   e1 = Cp * T - (p / rho)
   u_cea = data[ir][it].u
   -- u_offset if global in this module
   u_offset = e1 * 0.001 - u_cea
   print("u_offset= ", u_offset, " kJ/kg")

end

function collate_table_data(reactants, with_ions, only_list)
   setup_cea()

   data = {}
   print("Collating table data...")
   for ir=1,irpoints do
      local log_rho = log_rho_min + (ir-1)*dlr
      local rho = pow(10, log_rho)
      print("rho= ", rho)
      data[ir] = {}
      for _,Tstr in ipairs(T_range) do
	 write_inp_file(tmp_inp_file, reactants, rho, Tstr, with_ions, only_list)
	 inp = assert(io.open(tmp_stdin, 'w'))
	 inp:write("tmp\n")
	 inp:close()
	 cmd = CEA_PROGRAM.." < "..tmp_stdin.." > junk"
	 os.execute(cmd)
	 add_data_to_table(data[ir], tmp_plt_file)
      end
   end
   print("Done.\n")

   cmd = "rm "..tmp_inp_file.." "..tmp_plt_file.." "..tmp_stdin.." "..tmp_out_file.." junk"
   os.execute(cmd)
   cleanup_cea()

   set_u_offset(data)

   return data
end

function find_e_limits(data, extrapolate)
   -- Find minimum and maximum energy values.
   u_lower = 0.0
   u_upper = 0.0
   if extrapolate then
      u_lower = 1.0e9
      u_upper = -1.0e9
      for ir,_ in ipairs(data) do
	 u = data[1][ir].u
	 if u < u_lower then
	    u_lower = u
	 end

	 u = data[ir][#data[ir]].u
	 if u > u_upper then
	    u_upper = u
	 end
      end
   else
      u_lower = -1.0e9
      u_upper = 1.0e9
      for ir,_ in ipairs(data) do
	 u = data[ir][1].u
	 if u < u_lower then
	    u_lower = u
	 end

	 u = data[ir][#data[ir]].u
	 if u > u_upper then
	    u_upper = u
	 end
      end
   end

   emin = 1.0001 * (u_lower + u_offset) * 1000.0
   emax = 0.9999 * (u_upper + u_offset) * 1000.0

   return emin, emax
end

function interpolate_properties(data, e, ir)
   -- Locate energy
   
   u_kJkg = e * 0.001 - u_offset

   if u_kJkg < data[ir][1].u then
      print("Warning: energy too low: rho=", data[ir][1].rho, " cea_u=", data[ir][1].u, " u_kJkg=", u_kJkg)
   end

   it_save = 0
   for it=2,#data[ir] do
      if u_kJkg < data[ir][it].u then
	 it_save = it
	 break
      end
   end
   
   if it_save == 0 then
      -- We haven't found any energy value in table
      print("Warning: energy too high: rho=", data[ir][1].rho, " cea_u=", data[ir][#data[ir]].u, " u_kJkg=", e)
      it_save = #data[ir]
   end

   alpha = (u_kJkg - data[ir][it_save-1].u) / (data[ir][it_save].u - data[ir][it_save-1].u)

   t = {}

   if alpha >= 0.0 and alpha <= 1.0 then
      -- Interpolate
      p_bars = (1.0 - alpha)*data[ir][it_save-1].p + alpha*data[ir][it_save].p
      T_K = (1.0 - alpha)*data[ir][it_save-1].T + alpha*data[ir][it_save].T
      Cp = (1.0 - alpha)*data[ir][it_save-1].Cp + alpha*data[ir][it_save].Cp
      gammma = (1.0 - alpha)*data[ir][it_save-1].gamma + alpha*data[ir][it_save].gamma
      a = (1.0 - alpha)*data[ir][it_save-1].a + alpha*data[ir][it_save].a
      mu = (1.0 - alpha)*data[ir][it_save-1].mu + alpha*data[ir][it_save].mu
      k = (1.0 - alpha)*data[ir][it_save-1].k + alpha*data[ir][it_save].k
      
      p = p_bars * 100000.0  -- units in Pa
      rho = data[ir][it_save].rho
      R_hat = p / (rho*T_K)
      gamma_hat = a * a / (R_hat * T_K)
      Cv = Cp*1000.0 - R_hat
      
      t.Cv_hat = e / T_K
      t.Cv = Cv
      t.R_hat = R_hat
      t.g_hat = gamma_hat
      t.mu_hat = mu*1.0e-4 -- convert to kg/(m.s)
      t.k_hat = k*1.0e-1   -- convert to W/(m.K)
   else
      -- Extrpolate by building an equivalent ideal gas 
      -- model in the region and scaling to desired energy.
      p_limit = data[ir][it_save].p*100000.0
      T_limit = data[ir][it_save].T
      rho = data[ir][it_save].rho
      R_hat = p_limit / (rho * T_limit)
      Cv = data[ir][it_save].Cp*1000.0 - R_hat
      a_limit = data[ir][it_save].a
      e_limit = data[ir][it_save].u

      T_K = e / e_limit * T_limit
      p = (T_K/T_limit) * p_limit
      a = a_limit*(sqrt(T_K/T_limit))
      gamma_hat = a * a / (R_hat * T_K)

      t.Cv_hat = e / T_K
      t.Cv = Cv*1000.0
      t.R_hat = R_hat
      t.g_hat = gamma_hat
      t.mu_hat = data[ir][it_save].mu*1.0e-4*sqrt(T_K/T_limit)
      t.k_hat = data[ir][it_save].k*1.0e-1*sqrt(T_K/T_limit)
   end

   return t

end

function reorder_table_by_energy(data, extrapolate)
   data2 = {}

   e_min, e_max = find_e_limits(data, extrapolate)

   de = (e_max - e_min)/(ne-1)

   for ie=1,ne do
      data2[ie] = {}
      e = e_min +(ie-1)*de
      
      for ir=1,irpoints do
	 d = interpolate_properties(data, e, ir)
	 data2[ie][ir] = d
      end

   end

   return data2, e_min, de
end

function build_table(fname, reactants, with_ions, only_list, extrapolate)
   data = collate_table_data(reactants, with_ions, only_list)
   data2, e_min, de = reorder_table_by_energy(data, extrapolate)

   print(string.format("Writing out look-up table: %s", fname))
   f = assert(io.open(fname, 'w'))

   f:write(string.format("-- Auto-generated by build-cea-lut on: %s\n",
			 os.date("%d-%b-%Y %X")))
   f:write("model = 'look-up table'\n")
   f:write(string.format("iesteps = %d\n", #data2 - 1))
   f:write(string.format("irsteps = %d\n", #data2[1] - 1))
   f:write(string.format("emin = %g\n", e_min))
   f:write(string.format("de = %g\n", de))
   f:write(string.format("lrmin = %g\n", log_rho_min))
   f:write(string.format("dlr = %g\n", dlr))

   f:write("data = {\n")
   
   for ie=1,#data2 do
      f:write("{\n")
      for ir=1,#data2[ie] do
	 t = data2[ie][ir]
	 f:write(string.format("{%g, %g, %g, %g, %g, %g},\n",
			       t.Cv_hat, t.Cv, t.R_hat, t.g_hat, t.mu_hat, t.k_hat))
      end
      f:write("},\n")
   end
   f:write("}\n\n")

   f:close()
   print("Done.")

   print(string.format("Gzipping look-up table: %s", fname))
   cmd = "gzip -f "..fname
   os.execute(cmd)
   print(string.format("Created: %s", fname..".gz"))
   print("Done")

end

function main()
   if not (#arg == 1 or #arg == 2) then
      print_usage()
   end


   opt, val = string.match(arg[1], "%-%-([%a%-]+)=([%a%d.%-]+)")

   extrapolate = false
   if arg[2] then
      if arg[2] == "--extrapolate" then
	 extrapolate = true
      else
	 print("Second argument is unknown: ", arg[2])
	 print("The only valid argument is an optional '--exatrapolate'")
	 print_usage()
      end
   end

   if not opt or not val then
      print_usage()
   end
   
   file = ""
   if opt == 'case' then
      cases = list_available_cases()
      if not cases[val] then
	 print(string.format("The case: %s cannot be found in the collection.\n", val))
	 print("Check for the appropriate file in:\n")
	 dir = os.getenv("HOME").."/e3bin/cea-cases"
	 print("   ", dir)
	 print("Bailing out!\n")
	 os.exit(1)
      end
      file = os.getenv("HOME") .. "/e3bin/cea-cases/" .. val .. ".lua"
   elseif opt == "user-defined" then
      file = val
   else
      print(string.format("Option: '%s' is unknown.\n", opt))
      print_usage()
   end

   dofile(file)

   build_table(_G.fname, _G.reactants, _G.with_ions, _G.only_list, extrapolate)
   
end

main()

