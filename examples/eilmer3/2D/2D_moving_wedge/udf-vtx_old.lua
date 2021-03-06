-- Lua script for the vertex velocity
--
-- Author: Jason Kan Qin
-- Date: 13-Dec-2014

-- dummy functions to keep eilmer3 happy

function at_timestep_start(args) return nil end
function at_timestep_end(args) return nil end

local R = 23e-3
local thickness = 0.5e-3
local Inertia = 2700 * thickness * R^3 /3
local Moment_retard = 1.25
local Angular_velocity = 0
local Angle = 30.
local T_last = 0

function vtx_velocity(args)

   src = {}
   
   i = args.i -- vertex index (includes ghost vtx)
   j = args.j -- vertex index (includes ghost vtx)
   -- k = args.k -- vertex index (includes ghost vtx)
   x = args.x -- vertex location
   y = args.y -- vertex location
   -- z = args.z -- vertex location
   t = args.t -- current simulation time 
   -- dt = args.dt --current time step
   b_id = args.bdp_id -- block ID (number generated by e3prep.py)  

   if b_id == 0 or b_id == 1 then
      -- vtx for 1st block are kept stationary
      src.vel_x = 0
      src.vel_y = 0
      src.vel_z = 0
  
   elseif b_id == 2 then
      -- move vtx on second block
      local imin = 2
      local jmin = 2
      local imax = 52
      local jmax = 52

      -- Obtain position of corner points
      A0 = sample_vtx(b_id,imin,jmin,0) -- south-west corner
      A1 = sample_vtx(b_id,imax,jmin,0) -- south-east corner
      A2 = sample_vtx(b_id,imax,jmax,0) -- north-east corner
      A3 = sample_vtx(b_id,imin,jmax,0) -- north west corner
      Ab = sample_vtx(b_id,i,jmin,0) -- corresponding point on south edge
      At = sample_vtx(b_id,i,jmax,0) -- corresponding point on south edge


      if (i == imin and j == jmin) then     
         local Moment = 0
         local P_mean = 0
         Delta_t = t - T_last
         T_last = t
    
         for i = imin,(imax-1) do 
            Dat = sample_flow(b_id,i,jmin,0)
            arm = ((Dat.x - A0.x)^2 + (Dat.y - A0.y)^2 )^0.5
            p = (Dat.p-5955) * Dat.iLength
            P_mean = P_mean + Dat.p
            --print (Dat.p-5955)
            --print (Dat.iLength)
            --print (Moment)
            Moment = Moment + arm*p
         end
         -- check if caroner bending moment is overcome
         if Moment > Moment_retard then
            Moment = Moment - Moment_retard
         else
            Moment = 0
         end 
         -- use moment to calculate angular acceleration and movement
         Accel = Moment / Inertia
         Angular_velocity = Angular_velocity - Accel * Delta_t
         Angle = Angle + Angular_velocity / math.pi *180. * Delta_t 
         --print (Moment)
         --print (Inertia)
         print ("Mean Pressure on Plate (kPa)", P_mean/50. *1/1000)
         --print (Accel *Delta_t)
         --print (Delta_t)
         print ("Angle_dot (rad/s)", Angular_velocity)
         print ("Angle (deg)", Angle)
      end

     
      A12x = - Angular_velocity * R * ((A1.y-A0.y) / R) -- x velocity at point 1 and 2
      A1y = Angular_velocity * R * ((A1.x-A0.x) / R) -- y velocity at point 1 
      
      -- linearly interpolate velocities based on x and y position  
      src.vel_x = (x - A0.x) / (A1.x-A0.x) * A12x
      src.vel_y = (x - A0.x) / (A1.x-A0.x) * A1y * (1 - (y - Ab.y) / (At.y - Ab.y))  
      src.vel_z = 0.0

   elseif b_id == 3 then
      -- block added to ensure no interference with downstream boundary
      local imin = 2
      local jmin = 2
      local imax = 7
      local jmax = 52

      -- Obtain position of corner points
      A0 = sample_vtx(b_id,imin,jmin,0) -- south-west corner
      -- A1 = sample_vtx(b_id,imax,jmin,0) -- south-east corner
      -- A2 = sample_vtx(b_id,imax,jmax,0) -- north-east corner
      A3 = sample_vtx(b_id,imin,jmax,0) -- north west corner
      Ab = sample_vtx(b_id,i,jmin,0) -- corresponding point on south edge
      Ahinge = sample_vtx(2,imin,jmin,0) -- hinge point
            

      A12x = - Angular_velocity * R * ((A0.y-Ahinge.y) / R)
      A1y = Angular_velocity * R * ((A0.x-Ahinge.x) / R)

      src.vel_x = A12x
      src.vel_y = A1y * (1 - (y - Ab.y) / (A3.y - Ab.y))
      src.vel_z = 0.0
   else
      -- block added to ensure expansion fan captured
      local imin = 2
      local jmin = 2
      local imax = 7  
      local jmax = 12
      --A0 = sample_vtx(b_id,imin,jmin,0)
      --A1 = sample_vtx(b_id,imax,jmin,0)
      --A2 = sample_vtx(b_id,imax,jmax,0)
      A3 = sample_vtx(b_id,imin,jmax,0)
      --Ab = sample_vtx(b_id,i,jmin,0)
      Ahinge = sample_vtx(2,imin,jmin,0) -- hinge point
      
      A12x = - Angular_velocity * R * ((A3.y-Ahinge.y) / R)
      A1y = Angular_velocity * R * ((A3.x-Ahinge.x) / R)

      src.vel_x = A12x
      src.vel_y = A1y
      src.vel_z = 0.0   
   end

   return src
   
end
