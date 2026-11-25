#=====================================================================
# ZUBoard 1CG - Fixed Pin Constraints
#=====================================================================

# Op2_0[0] connected to Push-Button SW3
set_property PACKAGE_PIN A8 [get_ports {Op2_0[0]}]
set_property IOSTANDARD LVCMOS18 [get_ports {Op2_0[0]}]

# Res_0[0] connected to RGB LED (Blue)
set_property PACKAGE_PIN B5 [get_ports {Res_0[0]}]
set_property IOSTANDARD LVCMOS18 [get_ports {Res_0[0]}]
