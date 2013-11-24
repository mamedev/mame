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
#  Core files
#-------------------------------------------------

NETLISTOBJS+= \
	$(NETLISTOBJ)/nl_base.o \
	$(NETLISTOBJ)/pstring.o \
	$(NETLISTOBJ)/nl_setup.o \
	$(NETLISTOBJ)/nl_parser.o \
	$(NETLISTOBJ)/devices/nld_system.o \
	$(NETLISTOBJ)/devices/net_lib.o \

