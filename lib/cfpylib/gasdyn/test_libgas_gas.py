#!/usr/bin/env python
"""
test_libgas_gas.py -- test script
"""

import sys, os
sys.path.append(os.path.expandvars("$HOME/e3bin"))

import unittest
from cfpylib.gasdyn.libgas_gas import Gas


class TestLibGasGas(unittest.TestCase):
    def test_REFPROP(self):
        # Supercritical
        testGas = Gas(name='R134A.FLD', gasModelType='real gas REFPROP')
        testGas.set_pT(4.2e6, 400.0)
        self.assertAlmostEqual(testGas.rho, 201.900034379, places=9)
        self.assertAlmostEqual(testGas.e/1e3, 452.009183568, places=9)
        self.assertAlmostEqual(testGas.a, 135.781351458, places=9)
        self.assertEqual(testGas.quality, 999) # 999 means supercritical
        # Test calculation of transport coefficients
        self.assertAlmostEqual(testGas.mu/1e6, 19.4290589130, places=9)
        self.assertAlmostEqual(testGas.k*1e3, 27.0975847050, places=9)

    def test_Bender(self):
        # Supercritical
        testGas = Gas(name='CO2', gasModelType='real gas Bender')
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

if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(TestLibGasGas)
    unittest.TextTestRunner(verbosity=2).run(suite)
