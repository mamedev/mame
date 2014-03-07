###########################################################################
#
#   emu.mak
#
#   MAME emulation core makefile
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


EMUSRC = $(SRC)/emu
EMUOBJ = $(OBJ)/emu

EMUAUDIO = $(EMUOBJ)/audio
EMUBUS = $(EMUOBJ)/bus
EMUDRIVERS = $(EMUOBJ)/drivers
EMULAYOUT = $(EMUOBJ)/layout
EMUMACHINE = $(EMUOBJ)/machine
EMUIMAGEDEV = $(EMUOBJ)/imagedev
EMUVIDEO = $(EMUOBJ)/video

OBJDIRS += \
	$(EMUOBJ)/cpu \
	$(EMUOBJ)/sound \
	$(EMUOBJ)/debug \
	$(EMUOBJ)/debugint \
	$(EMUOBJ)/audio \
	$(EMUOBJ)/bus \
	$(EMUOBJ)/bus/abcbus \
	$(EMUOBJ)/bus/adam \
	$(EMUOBJ)/bus/adamnet \
	$(EMUOBJ)/bus/bw2 \
	$(EMUOBJ)/bus/c64 \
	$(EMUOBJ)/bus/cbm2 \
	$(EMUOBJ)/bus/cbmiec \
	$(EMUOBJ)/bus/centronics \
	$(EMUOBJ)/bus/comx35 \
	$(EMUOBJ)/bus/ecbbus \
	$(EMUOBJ)/bus/econet \
	$(EMUOBJ)/bus/ep64 \
	$(EMUOBJ)/bus/ieee488 \
	$(EMUOBJ)/bus/imi7000 \
	$(EMUOBJ)/bus/isa \
	$(EMUOBJ)/bus/isbx \
	$(EMUOBJ)/bus/midi \
	$(EMUOBJ)/bus/pc_joy \
	$(EMUOBJ)/bus/pc_kbd \
	$(EMUOBJ)/bus/pet \
	$(EMUOBJ)/bus/plus4 \
	$(EMUOBJ)/bus/rs232 \
	$(EMUOBJ)/bus/s100 \
	$(EMUOBJ)/bus/vcs \
	$(EMUOBJ)/bus/vic10 \
	$(EMUOBJ)/bus/vic20 \
	$(EMUOBJ)/bus/vidbrain \
	$(EMUOBJ)/bus/vip \
	$(EMUOBJ)/bus/wangpc \
	$(EMUOBJ)/bus/a2bus \
	$(EMUOBJ)/bus/nubus \
	$(EMUOBJ)/bus/iq151 \
	$(EMUOBJ)/bus/kc \
	$(EMUOBJ)/bus/tvc \
	$(EMUOBJ)/bus/z88 \
	$(EMUOBJ)/drivers \
	$(EMUOBJ)/machine \
	$(EMUOBJ)/layout \
	$(EMUOBJ)/imagedev \
	$(EMUOBJ)/ui \
	$(EMUOBJ)/video \

OSDSRC = $(SRC)/osd
OSDOBJ = $(OBJ)/osd

OBJDIRS += \
	$(OSDOBJ)


#-------------------------------------------------
# emulator core objects
#-------------------------------------------------

EMUOBJS = \
	$(EMUOBJ)/hashfile.o \
	$(EMUOBJ)/addrmap.o \
	$(EMUOBJ)/attotime.o \
	$(EMUOBJ)/audit.o \
	$(EMUOBJ)/cheat.o \
	$(EMUOBJ)/clifront.o \
	$(EMUOBJ)/config.o \
	$(EMUOBJ)/crsshair.o \
	$(EMUOBJ)/debugger.o \
	$(EMUOBJ)/delegate.o \
	$(EMUOBJ)/devdelegate.o \
	$(EMUOBJ)/devcb.o \
	$(EMUOBJ)/devcb2.o \
	$(EMUOBJ)/devcpu.o \
	$(EMUOBJ)/devfind.o \
	$(EMUOBJ)/device.o \
	$(EMUOBJ)/didisasm.o \
	$(EMUOBJ)/diexec.o \
	$(EMUOBJ)/diimage.o \
	$(EMUOBJ)/dimemory.o \
	$(EMUOBJ)/dinetwork.o \
	$(EMUOBJ)/dinvram.o \
	$(EMUOBJ)/dioutput.o \
	$(EMUOBJ)/dirtc.o \
	$(EMUOBJ)/diserial.o \
	$(EMUOBJ)/dislot.o \
	$(EMUOBJ)/disound.o \
	$(EMUOBJ)/distate.o \
	$(EMUOBJ)/divideo.o \
	$(EMUOBJ)/drawgfx.o \
	$(EMUOBJ)/driver.o \
	$(EMUOBJ)/drivenum.o \
	$(EMUOBJ)/emualloc.o \
	$(EMUOBJ)/emucore.o \
	$(EMUOBJ)/emuopts.o \
	$(EMUOBJ)/emupal.o \
	$(EMUOBJ)/fileio.o \
	$(EMUOBJ)/hash.o \
	$(EMUOBJ)/image.o \
	$(EMUOBJ)/info.o \
	$(EMUOBJ)/input.o \
	$(EMUOBJ)/ioport.o \
	$(EMUOBJ)/luaengine.o \
	$(EMUOBJ)/mame.o \
	$(EMUOBJ)/machine.o \
	$(EMUOBJ)/mconfig.o \
	$(EMUOBJ)/memarray.o \
	$(EMUOBJ)/memory.o \
	$(EMUOBJ)/network.o \
	$(EMUOBJ)/output.o \
	$(EMUOBJ)/render.o \
	$(EMUOBJ)/rendfont.o \
	$(EMUOBJ)/rendlay.o \
	$(EMUOBJ)/rendutil.o \
	$(EMUOBJ)/romload.o \
	$(EMUOBJ)/save.o \
	$(EMUOBJ)/schedule.o \
	$(EMUOBJ)/screen.o \
	$(EMUOBJ)/softlist.o \
	$(EMUOBJ)/sound.o \
	$(EMUOBJ)/speaker.o \
	$(EMUOBJ)/sprite.o \
	$(EMUOBJ)/tilemap.o \
	$(EMUOBJ)/timer.o \
	$(EMUOBJ)/uiinput.o \
	$(EMUOBJ)/ui/ui.o \
	$(EMUOBJ)/ui/swlist.o \
	$(EMUOBJ)/ui/menu.o \
	$(EMUOBJ)/ui/mainmenu.o \
	$(EMUOBJ)/ui/miscmenu.o \
	$(EMUOBJ)/ui/selgame.o \
	$(EMUOBJ)/ui/filemngr.o \
	$(EMUOBJ)/ui/filesel.o \
	$(EMUOBJ)/ui/imginfo.o \
	$(EMUOBJ)/ui/bbcontrl.o \
	$(EMUOBJ)/ui/barcode.o \
	$(EMUOBJ)/ui/tapectrl.o \
	$(EMUOBJ)/ui/viewgfx.o \
	$(EMUOBJ)/validity.o \
	$(EMUOBJ)/video.o \
	$(EMUOBJ)/debug/debugcmd.o \
	$(EMUOBJ)/debug/debugcon.o \
	$(EMUOBJ)/debug/debugcpu.o \
	$(EMUOBJ)/debug/debughlp.o \
	$(EMUOBJ)/debug/debugvw.o \
	$(EMUOBJ)/debug/dvdisasm.o \
	$(EMUOBJ)/debug/dvmemory.o \
	$(EMUOBJ)/debug/dvbpoints.o \
	$(EMUOBJ)/debug/dvwpoints.o \
	$(EMUOBJ)/debug/dvstate.o \
	$(EMUOBJ)/debug/dvtext.o \
	$(EMUOBJ)/debug/express.o \
	$(EMUOBJ)/debug/textbuf.o \
	$(EMUOBJ)/debugint/debugint.o \
	$(EMUOBJ)/profiler.o \
	$(EMUOBJ)/webengine.o \
	$(OSDOBJ)/osdepend.o \
	$(OSDOBJ)/osdnet.o

EMUSOUNDOBJS = \
	$(EMUOBJ)/sound/filter.o \
	$(EMUOBJ)/sound/flt_vol.o \
	$(EMUOBJ)/sound/flt_rc.o \
	$(EMUOBJ)/sound/wavwrite.o \
	$(EMUOBJ)/sound/samples.o   \

EMUDRIVEROBJS = \
	$(EMUDRIVERS)/empty.o \
	$(EMUDRIVERS)/testcpu.o \

EMUMACHINEOBJS = \
	$(EMUMACHINE)/bcreader.o    \
	$(EMUMACHINE)/buffer.o      \
	$(EMUMACHINE)/clock.o       \
	$(EMUMACHINE)/generic.o     \
	$(EMUMACHINE)/keyboard.o    \
	$(EMUMACHINE)/laserdsc.o    \
	$(EMUMACHINE)/latch.o       \
	$(EMUMACHINE)/netlist.o     \
	$(EMUMACHINE)/nvram.o       \
	$(EMUMACHINE)/ram.o         \
	$(EMUMACHINE)/terminal.o    \

EMUIMAGEDEVOBJS = \
	$(EMUIMAGEDEV)/bitbngr.o    \
	$(EMUIMAGEDEV)/cartslot.o   \
	$(EMUIMAGEDEV)/cassette.o   \
	$(EMUIMAGEDEV)/chd_cd.o     \
	$(EMUIMAGEDEV)/flopdrv.o    \
	$(EMUIMAGEDEV)/floppy.o     \
	$(EMUIMAGEDEV)/harddriv.o   \
	$(EMUIMAGEDEV)/midiin.o     \
	$(EMUIMAGEDEV)/midiout.o    \
	$(EMUIMAGEDEV)/printer.o    \
	$(EMUIMAGEDEV)/snapquik.o   \


EMUVIDEOOBJS = \
	$(EMUVIDEO)/generic.o       \
	$(EMUVIDEO)/resnet.o        \
	$(EMUVIDEO)/rgbutil.o       \
	$(EMUVIDEO)/vector.o        \


LIBEMUOBJS = $(EMUOBJS) $(EMUSOUNDOBJS) $(EMUDRIVEROBJS) $(EMUMACHINEOBJS) $(EMUIMAGEDEVOBJS) $(EMUVIDEOOBJS)

$(LIBEMU): $(LIBEMUOBJS)



#-------------------------------------------------
# CPU core objects
#-------------------------------------------------

include $(EMUSRC)/cpu/cpu.mak

$(LIBDASM): $(DASMOBJS)


#-------------------------------------------------
# sound core objects
#-------------------------------------------------

include $(EMUSRC)/sound/sound.mak

#-------------------------------------------------
# netlist core objects
#-------------------------------------------------

include $(EMUSRC)/netlist/netlist.mak

#-------------------------------------------------
# video core objects
#-------------------------------------------------

include $(EMUSRC)/video/video.mak

#-------------------------------------------------
# machine core objects
#-------------------------------------------------

include $(EMUSRC)/machine/machine.mak

#-------------------------------------------------
# bus core objects
#-------------------------------------------------

include $(EMUSRC)/bus/bus.mak

#-------------------------------------------------
# core optional library
#-------------------------------------------------

$(LIBOPTIONAL): $(CPUOBJS) $(SOUNDOBJS) $(VIDEOOBJS) $(MACHINEOBJS) $(BUSOBJS) $(NETLISTOBJS)

#-------------------------------------------------
# additional dependencies
#-------------------------------------------------

$(EMUOBJ)/rendfont.o:   $(EMUOBJ)/uismall.fh

$(EMUOBJ)/video.o:  $(EMUSRC)/rendersw.c

#-------------------------------------------------
# core layouts
#-------------------------------------------------

$(EMUOBJ)/rendlay.o:    $(EMULAYOUT)/dualhovu.lh \
						$(EMULAYOUT)/dualhsxs.lh \
						$(EMULAYOUT)/dualhuov.lh \
						$(EMULAYOUT)/horizont.lh \
						$(EMULAYOUT)/triphsxs.lh \
						$(EMULAYOUT)/quadhsxs.lh \
						$(EMULAYOUT)/vertical.lh \
						$(EMULAYOUT)/lcd.lh \
						$(EMULAYOUT)/lcd_rot.lh \
						$(EMULAYOUT)/noscreens.lh \

$(EMUOBJ)/video.o:      $(EMULAYOUT)/snap.lh
