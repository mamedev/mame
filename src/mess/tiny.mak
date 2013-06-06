###########################################################################
#
#   tiny.mak
#
#   Small driver-specific example makefile
#   Use make TARGET=mess SUBTARGET=tiny to build
#
#   As an example this makefile builds MESS with the three Colecovision
#   drivers enabled only.
#
#   Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
#   Visit  http://mamedev.org for licensing and usage restrictions.
#
###########################################################################

# include MESS core defines
include $(SRC)/mess/messcore.mak

#-------------------------------------------------
# Specify all the CPU cores necessary for the
# drivers referenced in tiny.c.
#-------------------------------------------------

CPUS += Z80



#-------------------------------------------------
# Specify all the sound cores necessary for the
# drivers referenced in tiny.c.
#-------------------------------------------------

SOUNDS += SN76496



#-------------------------------------------------
# specify available video cores
#-------------------------------------------------

VIDEOS += TMS9928A


#-------------------------------------------------
# specify available machine cores
#-------------------------------------------------



#-------------------------------------------------
# This is the list of files that are necessary
# for building all of the drivers referenced
# in tiny.c
#-------------------------------------------------

DRVLIBS = \
	$(MESS_DRIVERS)/coleco.o \
	$(MESS_MACHINE)/coleco.o \

