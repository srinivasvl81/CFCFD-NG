/**
 * rate_constant.d
 * Implements the calculation of reaction rate coefficients.
 *
 * Author: Rowan G.
 */

module kinetics.rate_constant;

import std.math;
import luad.all;
import util.lua_service;
import gas;

/++
  RateConstant is a class for computing the rate constant of
  a chemical reaction.
+/

interface RateConstant {
    RateConstant dup() const;
    double eval(in GasState Q) const;
}

/++
 User interface to configure ArrheniusRateConstant.
 +/
struct ArrheniusParams {
    double A; // frequency factor, units vary depending on reaction order
    double n; // power for temperature dependecy
    double C; // activation temperature
}

/++
 ArrheniusRateConstant uses the Arrhenius model to compute
 a chemical rate constant.

 Strictly speaking, this class implements a modified or
 generalised version of the Arrhenius rate constant.
 Arrhenius expression contains no temperature-dependent
 pre-exponential term. That is, the Arrhenius expression for
 rate constant is:
   k = A exp(-C/T))
 The modified expression is:
   k = A T^n exp(-C/T))

 This class implements that latter expression. It seems overly
 pedantic to provide two separate classes: one for the original
 Arrhenius expression and one for the modified version. Particularly
 given that almost all modern reaction schemes for gas phase chemistry
 provide rate constant inputs in terms of the modified expression.
 Simply setting n=0 gives the original Arrhenius form of the expression.
+/
class ArrheniusRateConstant : RateConstant {
public:
    this(double A, double n, double C)
    {
	_A = A;
	_n = n;
	_C = C;
    }
    this(LuaTable t)
    {
	_A = t.get!double("A");
	_n = t.get!double("n");
	_C = t.get!double("C");
    }
    this(ArrheniusParams input)
    {
	_A = input.A;
	_n = input.n;
	_C = input.C;
    }
    ArrheniusRateConstant dup() const
    {
	return new ArrheniusRateConstant(_A, _n, _C);
    }
    override double eval(in GasState Q) const
    {
	double T = Q.T[0];
	return _A*pow(T, _n)*exp(-_C/T);
    }
private:
    double _A, _n, _C;
}

static ArrheniusRateConstant[] rates;

void Arrhenius0(LuaTable t)
{
    rates ~= new ArrheniusRateConstant(t.toStruct!ArrheniusParams());
}
void Arrhenius1(double A, double n, double C)
{
    rates ~= new ArrheniusRateConstant(A, n, C);
}

unittest {
    import util.msg_service;
    // Test 1. Rate constant for H2 + I2 reaction.
    auto rc = new ArrheniusRateConstant(1.94e14, 0.0, 20620.0);
    auto gd = new GasState(1, 1);
    gd.T[0] = 700.0;
    assert(approxEqual(31.24116, rc.eval(gd)), failedUnitTest());
    // Test 2. Read rate constant parameters for nitrogen dissociation
    // from Lua input and compute rate constant at 4000.0 K
    auto lua = initLuaState("sample-input/N2-diss.lua");
    auto rc2 = new ArrheniusRateConstant(lua.get!LuaTable("rate"));
    gd.T[0] = 4000.0;
    assert(approxEqual(1594.39, rc2.eval(gd)), failedUnitTest());
    // Test 3. Read rate constant parameters for nitrogen dissocation
    // from a Lua table.
    lua = new LuaState;
    lua.openLibs();
    // Register Arrhenius functions with Lua
    lua["Arrhenius0"] = &Arrhenius0;
    lua["Arrhenius1"] = &Arrhenius1;
    lua.doFile("sample-input/rate-input-test.lua");
    assert(approxEqual(1594.39, rates[0].eval(gd)), failedUnitTest());
    assert(approxEqual(1594.39, rates[1].eval(gd)), failedUnitTest());
}