-- udf-wall.lua
-- Lua script for the user-defined functions 
-- called by the UserDefinedBC boundary condition.


function reflect_normal_velocity(ux, vy, cosX, cosY)
   -- Copied from cns_bc.h.
   un = ux * cosX + vy * cosY;     -- Normal velocity
   vt = -ux * cosY + vy * cosX;    -- Tangential velocity
   un = -un;                       -- Reflect normal component
   ux = un * cosX - vt * cosY;     -- Back to Cartesian coords
   vy = un * cosY + vt * cosX;
   return ux, vy
end

function ghost_cell(args)
   -- Function that returns the flow state for a ghost cell
   -- for use in the inviscid flux calculations.
   --
   -- args contains t, x, y, z, csX, csY, csZ, i, j, k, which_boundary
   i = args.i; j = args.j; k = args.k
   cell1 = sample_flow(block_id, i, j, k)
   cell1.u, cell1.v = reflect_normal_velocity(cell1.u, cell1.v, args.csX, args.csY)
   if args.which_boundary == NORTH then
      j = j - 1
   elseif args.which_boundary == EAST then
      i = i - 1
   elseif args.which_boundary == SOUTH then
      j = j + 1
   elseif args.which_boundary == WEST then
      i = i + 1
   end
   cell2 = sample_flow(block_id, i, j, k)
   cell2.u, cell2.v = reflect_normal_velocity(cell2.u, cell2.v, args.csX, args.csY)
   return cell1, cell2
end


function interface(args)
   -- Function that returns the conditions at the boundary 
   -- when viscous terms are active.
   --
   -- args contains t, x, y, z, csX, csY, csZ, i, j, k, which_boundary
   Tleft = 300
   Tright = 1500
   cell = sample_flow(block_id, args.i, args.j, args.k)
   cell.u, cell.v = 0
   cell.T = {}  -- temperatures, K (as a table)
   x = args.x
   if (x >= 0.0 and x <= 1e-3) then
   cell.T[0] = ((Tright-Tleft)*(1-math.exp(-1e4*(x-0.5e-3)))\
   /(1+math.exp(-1e4*(x-0.5e-3)))+(Tright+Tleft))/2 --hypertangent profile
   else
   cell.T[0] = Tright
   end
   return cell
end
