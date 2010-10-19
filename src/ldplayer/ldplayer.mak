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
# this is the list of driver libraries that
# comprise MAME plus mamedriv.o which contains
# the list of drivers
#-------------------------------------------------

DRVLIBS = \
	$(LDPOBJ)/ldpdriv.o \
	$(EMUDRIVERS)/emudummy.o \
	$(LDPOBJ)/ldplayer.o \


#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(LDPOBJ)/ldplayer.o:	$(LAYOUT)/pr8210.lh \
