###########################################################################
#
#   video.mak
#
#   Rules for building video cores
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


VIDEOSRC = $(EMUSRC)/video
VIDEOOBJ = $(EMUOBJ)/video


#-------------------------------------------------
# Core video types
#-------------------------------------------------

VIDEOOBJS+= $(VIDEOOBJ)/generic.o   	\
			$(VIDEOOBJ)/resnet.o    	\
			$(VIDEOOBJ)/rgbutil.o   	\

VIDEOOBJS+= $(VIDEOOBJ)/315_5124.o      \
			$(VIDEOOBJ)/bufsprite.o     \
			$(VIDEOOBJ)/cdp1861.o       \
			$(VIDEOOBJ)/cdp1862.o       \
			$(VIDEOOBJ)/cgapal.o        \
			$(VIDEOOBJ)/crt9007.o       \
			$(VIDEOOBJ)/crt9021.o       \
			$(VIDEOOBJ)/crt9212.o       \
			$(VIDEOOBJ)/dl1416.o        \
			$(VIDEOOBJ)/dm9368.o        \
			$(VIDEOOBJ)/ef9340_1.o      \
			$(VIDEOOBJ)/h63484.o        \
			$(VIDEOOBJ)/hd44102.o       \
			$(VIDEOOBJ)/hd44352.o       \
			$(VIDEOOBJ)/hd44780.o       \
			$(VIDEOOBJ)/hd61830.o       \
			$(VIDEOOBJ)/hd63484.o       \
			$(VIDEOOBJ)/hd66421.o       \
			$(VIDEOOBJ)/huc6202.o       \
			$(VIDEOOBJ)/huc6260.o       \
			$(VIDEOOBJ)/huc6261.o       \
			$(VIDEOOBJ)/huc6270.o       \
			$(VIDEOOBJ)/huc6272.o       \
			$(VIDEOOBJ)/i8244.o         \
			$(VIDEOOBJ)/i8275.o         \
			$(VIDEOOBJ)/i8275x.o        \
			$(VIDEOOBJ)/k053250.o       \
			$(VIDEOOBJ)/m50458.o        \
			$(VIDEOOBJ)/mb90082.o       \
			$(VIDEOOBJ)/mc6845.o        \
			$(VIDEOOBJ)/mc6847.o        \
			$(VIDEOOBJ)/msm6255.o       \
			$(VIDEOOBJ)/pc_cga.o        \
			$(VIDEOOBJ)/pc_vga.o        \
			$(VIDEOOBJ)/poly.o          \
			$(VIDEOOBJ)/psx.o           \
			$(VIDEOOBJ)/ramdac.o        \
			$(VIDEOOBJ)/s2636.o         \
			$(VIDEOOBJ)/saa5050.o       \
			$(VIDEOOBJ)/sed1330.o       \
			$(VIDEOOBJ)/stvvdp1.o       \
			$(VIDEOOBJ)/stvvdp2.o       \
			$(VIDEOOBJ)/tlc34076.o      \
			$(VIDEOOBJ)/tms34061.o      \
			$(VIDEOOBJ)/tms3556.o       \
			$(VIDEOOBJ)/tms9927.o       \
			$(VIDEOOBJ)/tms9928a.o      \
			$(VIDEOOBJ)/upd3301.o       \
			$(VIDEOOBJ)/upd7220.o       \
			$(VIDEOOBJ)/upd7227.o       \
			$(VIDEOOBJ)/v9938.o         \
			$(VIDEOOBJ)/vector.o        \
			$(VIDEOOBJ)/voodoo.o        \
