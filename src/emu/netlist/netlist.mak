###########################################################################
#
#   netlist.mak
#
#   Rules for building netlist core and devices
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


NETLISTSRC = $(EMUSRC)/netlist
NETLISTOBJ = $(EMUOBJ)/netlist

#-------------------------------------------------
#  Netlist dirs and files
#-------------------------------------------------

OBJDIRS += \
	$(NETLISTOBJ) \
	$(NETLISTOBJ)/devices \
	$(NETLISTOBJ)/analog \

NETLISTOBJS+= \
	$(NETLISTOBJ)/nl_base.o \
	$(NETLISTOBJ)/nl_parser.o \
	$(NETLISTOBJ)/nl_setup.o \
	$(NETLISTOBJ)/pstring.o \
	$(NETLISTOBJ)/pstate.o \
	$(NETLISTOBJ)/analog/nld_bjt.o \
	$(NETLISTOBJ)/analog/nld_fourterm.o \
	$(NETLISTOBJ)/analog/nld_solver.o \
	$(NETLISTOBJ)/analog/nld_switches.o \
	$(NETLISTOBJ)/analog/nld_twoterm.o \
	$(NETLISTOBJ)/devices/nld_4066.o \
	$(NETLISTOBJ)/devices/nld_7400.o \
	$(NETLISTOBJ)/devices/nld_7402.o \
	$(NETLISTOBJ)/devices/nld_7404.o \
	$(NETLISTOBJ)/devices/nld_7410.o \
	$(NETLISTOBJ)/devices/nld_7420.o \
	$(NETLISTOBJ)/devices/nld_7425.o \
	$(NETLISTOBJ)/devices/nld_7427.o \
	$(NETLISTOBJ)/devices/nld_7430.o \
	$(NETLISTOBJ)/devices/nld_7448.o \
	$(NETLISTOBJ)/devices/nld_7450.o \
	$(NETLISTOBJ)/devices/nld_7474.o \
	$(NETLISTOBJ)/devices/nld_7483.o \
	$(NETLISTOBJ)/devices/nld_7486.o \
	$(NETLISTOBJ)/devices/nld_7490.o \
	$(NETLISTOBJ)/devices/nld_7493.o \
	$(NETLISTOBJ)/devices/nld_74107.o \
	$(NETLISTOBJ)/devices/nld_74153.o \
	$(NETLISTOBJ)/devices/nld_74ls629.o \
	$(NETLISTOBJ)/devices/nld_9316.o \
	$(NETLISTOBJ)/devices/nld_ne555.o \
	$(NETLISTOBJ)/devices/nld_legacy.o \
	$(NETLISTOBJ)/devices/net_lib.o \
	$(NETLISTOBJ)/devices/nld_log.o \
	$(NETLISTOBJ)/devices/nld_system.o \

