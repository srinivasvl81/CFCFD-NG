# 1. transport model
gdata.transport_model = "monte carlo"
gdata.nrays = 32768
gdata.clustering = "by area"
gdata.absorption = "partitioned energy"

# 2. spectral model
gdata.spectral_model = "photaura"
gdata.lambda_min = 180.
gdata.lambda_max = 6000.
gdata.spectral_points = int ( ( 1.0e7 / gdata.lambda_min - 1.0e7 / gdata.lambda_max ) * 0.1 )
gdata.adaptive_spectral_grid = False

params = {
"species"               : [ 'Ar', 'Ar_plus', 'e_minus' ],
"radiators"             : [ 'Ar', 'Ar_plus', 'e_minus' ],
"QSS_radiators"         : [ 'Ar' ],
"no_emission_radiators" : [],
"iTe"                   : 1,
"atomic_level_source"   : "NIST_ASD",
"atomic_line_source"    : "NIST_ASD",
"atomic_PICS_source"    : "TOPBase",
"allow_inexact_Stark_matches" : True,
"require_PICS_term_match" : False
}

declare_radiators( params, gdata )

