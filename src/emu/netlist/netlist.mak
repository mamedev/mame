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

NETLISTOBJS+= $(NETLISTOBJ)/net_lib.o

