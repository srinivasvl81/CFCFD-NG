-- this file has been automatically generated by chemkin2libgas.py

-- scaling factor, 1/R
S = 1.0/1.987

reaction{'H2 + O2 <=> 2OH',
   fr={'Arrhenius', A=1.70000e+13, n=0.00000e+00, T_a=4.47800e+04*S},
   label='r0'
}
reaction{'OH + H2 <=> H2O + H',
   fr={'Arrhenius', A=1.17000e+09, n=1.30000e+00, T_a=3.62600e+03*S},
   label='r1'
}
reaction{'O + OH <=> O2 + H',
   fr={'Arrhenius', A=4.00000e+14, n=-5.00000e-01, T_a=0.00000e+00*S},
   label='r2'
}
reaction{'O + H2 <=> OH + H',
   fr={'Arrhenius', A=5.06000e+04, n=2.67000e+00, T_a=6.29000e+03*S},
   label='r3'
}
reaction{'H + O2 + M <=> HO2 + M',
   fr={'Arrhenius', A=3.61000e+17, n=-7.20000e-01, T_a=0.00000e+00*S},
   efficiencies={H2O=18.60, H2=2.86, N2=1.26},
   label='r4'
}
reaction{'OH + HO2 <=> H2O + O2',
   fr={'Arrhenius', A=7.50000e+12, n=0.00000e+00, T_a=0.00000e+00*S},
   label='r5'
}
reaction{'H + HO2 <=> 2OH',
   fr={'Arrhenius', A=1.40000e+14, n=0.00000e+00, T_a=1.07300e+03*S},
   label='r6'
}
reaction{'O + HO2 <=> O2 + OH',
   fr={'Arrhenius', A=1.40000e+13, n=0.00000e+00, T_a=1.07300e+03*S},
   label='r7'
}
reaction{'2OH <=> O + H2O',
   fr={'Arrhenius', A=6.00000e+08, n=1.30000e+00, T_a=0.00000e+00*S},
   label='r8'
}
reaction{'H + H + M <=> H2 + M',
   fr={'Arrhenius', A=1.00000e+18, n=-1.00000e+00, T_a=0.00000e+00*S},
   efficiencies={H2O=0.00, H2=0.00},
   label='r9'
}
reaction{'H + H + H2 <=> H2 + H2',
   fr={'Arrhenius', A=9.20000e+16, n=-6.00000e-01, T_a=0.00000e+00*S},
   label='r10'
}
reaction{'H + H + H2O <=> H2 + H2O',
   fr={'Arrhenius', A=6.00000e+19, n=-1.25000e+00, T_a=0.00000e+00*S},
   label='r11'
}
reaction{'H + OH + M <=> H2O + M',
   fr={'Arrhenius', A=1.60000e+22, n=-2.00000e+00, T_a=0.00000e+00*S},
   efficiencies={H2O=5.00},
   label='r12'
}
reaction{'H + O + M <=> OH + M',
   fr={'Arrhenius', A=6.20000e+16, n=-6.00000e-01, T_a=0.00000e+00*S},
   efficiencies={H2O=5.00},
   label='r13'
}
reaction{'O + O + M <=> O2 + M',
   fr={'Arrhenius', A=1.89000e+13, n=0.00000e+00, T_a=-1.78800e+03*S},
   label='r14'
}
reaction{'H + HO2 <=> H2 + O2',
   fr={'Arrhenius', A=1.25000e+13, n=0.00000e+00, T_a=0.00000e+00*S},
   label='r15'
}
reaction{'HO2 + HO2 <=> H2O2 + O2',
   fr={'Arrhenius', A=2.00000e+12, n=0.00000e+00, T_a=0.00000e+00*S},
   label='r16'
}
reaction{'H2O2 + M <=> OH + OH + M',
   fr={'Arrhenius', A=1.30000e+17, n=0.00000e+00, T_a=4.55000e+04*S},
   label='r17'
}
reaction{'H2O2 + H <=> HO2 + H2',
   fr={'Arrhenius', A=1.60000e+12, n=0.00000e+00, T_a=3.80000e+03*S},
   label='r18'
}
reaction{'H2O2 + OH <=> H2O + HO2',
   fr={'Arrhenius', A=1.00000e+13, n=0.00000e+00, T_a=1.80000e+03*S},
   label='r19'
}
reaction{'O + N2 <=> NO + N',
   fr={'Arrhenius', A=1.40000e+14, n=0.00000e+00, T_a=7.58000e+04*S},
   label='r20'
}
reaction{'N + O2 <=> NO + O',
   fr={'Arrhenius', A=6.40000e+09, n=1.00000e+00, T_a=6.28000e+03*S},
   label='r21'
}
reaction{'OH + N <=> NO + H',
   fr={'Arrhenius', A=4.00000e+13, n=0.00000e+00, T_a=0.00000e+00*S},
   label='r22'
}
