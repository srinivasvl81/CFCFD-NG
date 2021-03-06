#!/usr/bin/env python
"""
test_libgas_gas.py -- test script
"""

import sys, os
sys.path.append(os.path.expandvars("$HOME/e3bin"))

import unittest
from cfpylib.gasdyn.libgas_gas import Gas, make_gas_from_name


class TestLibGasGas(unittest.TestCase):
    def test_REFPROP(self):
        # Supercritical (Peter Blyton)
        testGas = make_gas_from_name('r134a-refprop')
        testGas.set_pT(4.2e6, 400.0)
        self.assertAlmostEqual(testGas.rho, 201.900034379, places=9)
        self.assertAlmostEqual(testGas.e/1e3, 452.009183568, places=9)
        self.assertAlmostEqual(testGas.a, 135.781351458, places=9)
        self.assertEqual(testGas.quality, 999) # 999 means supercritical
        # Test calculation of transport coefficients
        self.assertAlmostEqual(testGas.mu/1e6, 19.4290589130, places=9)
        self.assertAlmostEqual(testGas.k*1e3, 27.0975847050, places=9)

    def test_Bender(self):
        # Supercritical (Peter Blyton)
        testGas = make_gas_from_name('co2-bender')
        testGas.set_ps(1.0e6, 1.7737e3)
        self.assertAlmostEqual(testGas.rho, 1.0/0.05379, places=2)
        self.assertAlmostEqual(testGas.h/1e3, 419.95, places=1)
        self.assertAlmostEqual(testGas.T, 300.0, places=1)
        self.assertAlmostEqual(testGas.a, 262.42, delta=0.05)
        self.assertAlmostEqual(testGas.C_v, 682.17, delta=2.0)
        self.assertAlmostEqual(testGas.C_p, 920.89, delta=2.0)
        # Test cloning
        secondGas = testGas.clone()
        self.assertAlmostEqual(secondGas.C_p, 920.89, delta=2.0)

    def test_thermally_perfect_air(self):
        # Peter J.
        testGas = make_gas_from_name('air-thermally-perfect')
        p = 100.0e3 # Pa
        T = 300.0 # degrees K
        testGas.set_pT(p, T)
        gam = testGas.gam
        R = testGas.R
        Cp = testGas.C_p
        self.assertAlmostEqual(gam, 1.4, delta=0.01)
        self.assertAlmostEqual(R, 287.0, delta=2)
        rho = testGas.rho
        self.assertAlmostEqual(rho, p/(R*T), delta=0.01)
        import math
        self.assertAlmostEqual(testGas.a, math.sqrt(gam*R*T), delta=2.0)
        s = testGas.s
        p2 = p + 50.0e3
        T2 = T + 50.0
        testGas.set_pT(p2, T2)
        s2 = testGas.s
        delta_s = Cp*math.log(T2/T) - R*math.log(p2/p)
        self.assertAlmostEqual(s2-s, delta_s, delta=0.3)
        testGas.set_ps(p2, s)
        rho2 = testGas.rho
        self.assertAlmostEqual(p2/pow(rho2,gam), p/pow(rho,gam), delta=100.0)

    def test_cea_lut_air_low_T(self):
        # Peter J.
        # 300 degrees is below the low-T limit of the table and it has to extrapolate.
        if not os.path.exists('cea-lut-air-ions.lua.gz'):
            os.system('cp ~/cfcfd3/lib/gas/cea-cases/cea-lut-air-ions.lua.gz .')
        testGas = Gas('cea-lut-air-ions.lua.gz')
        p = 100.0e3 # Pa
        T = 300.0 # degrees K
        testGas.set_pT(p, T)
        gam = testGas.gam
        R = testGas.R
        Cp = testGas.C_p # For the look-up table this Cp is for computing entropy
        self.assertAlmostEqual(Cp, R*gam/(gam-1.0), delta=10.0) # fairly coarse
        self.assertAlmostEqual(gam, 1.4, delta=0.02)
        self.assertAlmostEqual(R, 287.0, delta=2)
        rho = testGas.rho
        self.assertAlmostEqual(rho, p/(R*T), delta=0.01)
        import math
        self.assertAlmostEqual(testGas.a, math.sqrt(gam*R*T), delta=2.0)
        s = testGas.s
        p2 = p + 50.0e3
        T2 = T + 50.0
        testGas.set_pT(p2, T2)
        s2 = testGas.s
        delta_s = Cp*math.log(T2/T) - R*math.log(p2/p)
        self.assertAlmostEqual(s2-s, delta_s, delta=3.0) # fairly coarse
        testGas.set_ps(p2, s)
        rho2 = testGas.rho
        self.assertAlmostEqual(p2/pow(rho2,gam), p/pow(rho,gam), delta=200.0)

    def test_cea_lut_air_warm_T(self):
        # Peter J.
        # 600 degreesis just into the table so we should be interpolating.
        if not os.path.exists('cea-lut-air-ions.lua.gz'):
            os.system('cp ~/cfcfd3/lib/gas/cea-cases/cea-lut-air-ions.lua.gz .')
        testGas = Gas('cea-lut-air-ions.lua.gz')
        p = 100.0e3 # Pa
        T = 600.0 # degrees K
        testGas.set_pT(p, T)
        gam = testGas.gam
        R = testGas.R
        Cp = testGas.C_p # For the look-up table this Cp is for computing entropy
        self.assertAlmostEqual(Cp, R*gam/(gam-1.0), delta=10.0) # fairly coarse
        self.assertAlmostEqual(gam, 1.4, delta=0.03)
        self.assertAlmostEqual(R, 287.0, delta=2)
        rho = testGas.rho
        self.assertAlmostEqual(rho, p/(R*T), delta=0.01)
        import math
        self.assertAlmostEqual(testGas.a, math.sqrt(gam*R*T), delta=2.0)
        s = testGas.s
        p2 = p + 50.0e3
        T2 = T + 50.0
        testGas.set_pT(p2, T2)
        s2 = testGas.s
        delta_s = Cp*math.log(T2/T) - R*math.log(p2/p)
        self.assertAlmostEqual(s2-s, delta_s, delta=3.0) # fairly coarse
        testGas.set_ps(p2, s)
        rho2 = testGas.rho
        self.assertAlmostEqual(p2/pow(rho2,gam), p/pow(rho,gam), delta=300.0)
       
if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(TestLibGasGas)
    unittest.TextTestRunner(verbosity=2).run(suite)
