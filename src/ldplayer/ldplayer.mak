###########################################################################
#
#   ldplayer.mak
#
#   Small makefile to build a standalone laserdisc player
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################

# add some additional include libraries for the mame files
INCPATH += \
	-I$(SRC)/mame \

LDPSRC = $(SRC)/ldplayer
LDPOBJ = $(OBJ)/ldplayer

LAYOUT = $(LDPOBJ)/layout

OBJDIRS += \
	$(LDPOBJ) \
	$(LAYOUT) \



#-------------------------------------------------
# specify required CPU cores (none)
#-------------------------------------------------

CPUS += MCS48
CPUS += Z80



#-------------------------------------------------
# specify required sound cores
#-------------------------------------------------

SOUNDS += WAVE


#-------------------------------------------------
# specify available video cores
#-------------------------------------------------

#-------------------------------------------------
# specify available machine cores
#-------------------------------------------------

MACHINES += LDV1000
MACHINES += LDPR8210

#-------------------------------------------------
# specify available bus cores
#
# MIDI is here as dummy bus to allow libbus.a to
# be created on OSX.
#-------------------------------------------------

BUSES += MIDI

#-------------------------------------------------
# this is the list of driver libraries that
# comprise MAME plus mamedriv.o which contains
# the list of drivers
#-------------------------------------------------

DRVLIBS = \
	$(EMUDRIVERS)/emudummy.o


#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(LDPOBJ)/ldplayer.o:   $(LAYOUT)/pr8210.lh
