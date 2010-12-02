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
EMUDRIVERS = $(EMUOBJ)/drivers
EMULAYOUT = $(EMUOBJ)/layout
EMUMACHINE = $(EMUOBJ)/machine
EMUVIDEO = $(EMUOBJ)/video

OBJDIRS += \
	$(EMUOBJ)/cpu \
	$(EMUOBJ)/sound \
	$(EMUOBJ)/debug \
	$(EMUOBJ)/debugint \
	$(EMUOBJ)/audio \
	$(EMUOBJ)/drivers \
	$(EMUOBJ)/machine \
	$(EMUOBJ)/layout \
	$(EMUOBJ)/video \

OSDSRC = $(SRC)/osd
OSDOBJ = $(OBJ)/osd

OBJDIRS += \
	$(OSDOBJ)


#-------------------------------------------------
# emulator core objects
#-------------------------------------------------

EMUOBJS = \
	$(EMUOBJ)/addrmap.o \
	$(EMUOBJ)/attotime.o \
	$(EMUOBJ)/audit.o \
	$(EMUOBJ)/cheat.o \
	$(EMUOBJ)/clifront.o \
	$(EMUOBJ)/config.o \
	$(EMUOBJ)/crsshair.o \
	$(EMUOBJ)/debugger.o \
	$(EMUOBJ)/delegate.o \
	$(EMUOBJ)/devcb.o \
	$(EMUOBJ)/devcpu.o \
	$(EMUOBJ)/devimage.o \
	$(EMUOBJ)/devlegcy.o \
	$(EMUOBJ)/devintrf.o \
	$(EMUOBJ)/didisasm.o \
	$(EMUOBJ)/diexec.o \
	$(EMUOBJ)/diimage.o \
	$(EMUOBJ)/dimemory.o \
	$(EMUOBJ)/dinvram.o \
	$(EMUOBJ)/disound.o \
	$(EMUOBJ)/distate.o \
	$(EMUOBJ)/drawgfx.o \
	$(EMUOBJ)/driver.o \
	$(EMUOBJ)/emualloc.o \
	$(EMUOBJ)/emucore.o \
	$(EMUOBJ)/emuopts.o \
	$(EMUOBJ)/emupal.o \
	$(EMUOBJ)/fileio.o \
	$(EMUOBJ)/hash.o \
	$(EMUOBJ)/hashfile.o \
	$(EMUOBJ)/image.o \
	$(EMUOBJ)/info.o \
	$(EMUOBJ)/input.o \
	$(EMUOBJ)/inputseq.o \
	$(EMUOBJ)/inptport.o \
	$(EMUOBJ)/ioprocs.o \
	$(EMUOBJ)/mame.o \
	$(EMUOBJ)/machine.o \
	$(EMUOBJ)/mconfig.o \
	$(EMUOBJ)/memory.o \
	$(EMUOBJ)/output.o \
	$(EMUOBJ)/render.o \
	$(EMUOBJ)/rendfont.o \
	$(EMUOBJ)/rendlay.o \
	$(EMUOBJ)/rendutil.o \
	$(EMUOBJ)/romload.o \
	$(EMUOBJ)/schedule.o \
	$(EMUOBJ)/screen.o \
	$(EMUOBJ)/softlist.o \
	$(EMUOBJ)/sound.o \
	$(EMUOBJ)/state.o \
	$(EMUOBJ)/streams.o \
	$(EMUOBJ)/tilemap.o \
	$(EMUOBJ)/timer.o \
	$(EMUOBJ)/ui.o \
	$(EMUOBJ)/uigfx.o \
	$(EMUOBJ)/uiimage.o \
	$(EMUOBJ)/uiinput.o \
	$(EMUOBJ)/uimenu.o \
	$(EMUOBJ)/validity.o \
	$(EMUOBJ)/video.o \
	$(EMUOBJ)/watchdog.o \
	$(EMUOBJ)/debug/debugcmd.o \
	$(EMUOBJ)/debug/debugcon.o \
	$(EMUOBJ)/debug/debugcpu.o \
	$(EMUOBJ)/debug/debughlp.o \
	$(EMUOBJ)/debug/debugvw.o \
	$(EMUOBJ)/debug/dvdisasm.o \
	$(EMUOBJ)/debug/dvmemory.o \
	$(EMUOBJ)/debug/dvstate.o \
	$(EMUOBJ)/debug/dvtext.o \
	$(EMUOBJ)/debug/express.o \
	$(EMUOBJ)/debug/textbuf.o \
	$(EMUOBJ)/debugint/debugint.o \
	$(EMUOBJ)/profiler.o \
	$(OSDOBJ)/osdepend.o

EMUSOUNDOBJS = \
	$(EMUOBJ)/sound/filter.o \
	$(EMUOBJ)/sound/flt_vol.o \
	$(EMUOBJ)/sound/flt_rc.o \
	$(EMUOBJ)/sound/wavwrite.o \

EMUAUDIOOBJS = \
	$(EMUAUDIO)/generic.o \

EMUDRIVEROBJS = \
	$(EMUDRIVERS)/empty.o \

EMUMACHINEOBJS = \
	$(EMUMACHINE)/53c810.o \
	$(EMUMACHINE)/6532riot.o \
	$(EMUMACHINE)/6522via.o \
	$(EMUMACHINE)/6526cia.o \
	$(EMUMACHINE)/6821pia.o \
	$(EMUMACHINE)/6840ptm.o \
	$(EMUMACHINE)/6850acia.o \
	$(EMUMACHINE)/68681.o \
	$(EMUMACHINE)/7474.o \
	$(EMUMACHINE)/74123.o \
	$(EMUMACHINE)/74148.o \
	$(EMUMACHINE)/74153.o \
	$(EMUMACHINE)/74181.o \
	$(EMUMACHINE)/8042kbdc.o \
	$(EMUMACHINE)/8237dma.o \
	$(EMUMACHINE)/8257dma.o \
	$(EMUMACHINE)/8255ppi.o \
	$(EMUMACHINE)/adc083x.o \
	$(EMUMACHINE)/adc1038.o \
	$(EMUMACHINE)/adc1213x.o \
	$(EMUMACHINE)/am53cf96.o \
	$(EMUMACHINE)/at28c16.o \
	$(EMUMACHINE)/cdp1852.o \
	$(EMUMACHINE)/ds1302.o \
	$(EMUMACHINE)/ds2401.o \
	$(EMUMACHINE)/ds2404.o \
	$(EMUMACHINE)/eeprom.o \
	$(EMUMACHINE)/er2055.o \
	$(EMUMACHINE)/f3853.o \
	$(EMUMACHINE)/generic.o \
	$(EMUMACHINE)/i8243.o \
	$(EMUMACHINE)/i8255a.o \
	$(EMUMACHINE)/i2cmem.o \
	$(EMUMACHINE)/idectrl.o \
	$(EMUMACHINE)/ins8154.o	\
	$(EMUMACHINE)/ins8250.o \
	$(EMUMACHINE)/intelfsh.o \
	$(EMUMACHINE)/k033906.o \
	$(EMUMACHINE)/k056230.o \
	$(EMUMACHINE)/latch8.o \
	$(EMUMACHINE)/ldcore.o \
	$(EMUMACHINE)/ldpr8210.o \
	$(EMUMACHINE)/ldv1000.o \
	$(EMUMACHINE)/ldvp931.o \
	$(EMUMACHINE)/mb14241.o \
	$(EMUMACHINE)/mb3773.o \
	$(EMUMACHINE)/mb87078.o \
	$(EMUMACHINE)/mc146818.o \
	$(EMUMACHINE)/microtch.o \
	$(EMUMACHINE)/msm6242.o \
	$(EMUMACHINE)/nvram.o \
	$(EMUMACHINE)/pc16552d.o \
	$(EMUMACHINE)/pci.o \
	$(EMUMACHINE)/pic8259.o \
	$(EMUMACHINE)/pit8253.o \
	$(EMUMACHINE)/pd4990a.o \
	$(EMUMACHINE)/roc10937.o \
	$(EMUMACHINE)/rp5h01.o \
	$(EMUMACHINE)/rtc65271.o \
	$(EMUMACHINE)/scsi.o \
	$(EMUMACHINE)/scsicd.o \
	$(EMUMACHINE)/scsidev.o \
	$(EMUMACHINE)/scsihd.o \
	$(EMUMACHINE)/cr589.o \
	$(EMUMACHINE)/smc91c9x.o \
	$(EMUMACHINE)/timekpr.o \
	$(EMUMACHINE)/tmp68301.o \
	$(EMUMACHINE)/tms6100.o \
	$(EMUMACHINE)/upd4701.o \
	$(EMUMACHINE)/wd33c93.o \
	$(EMUMACHINE)/x2212.o \
	$(EMUMACHINE)/x76f041.o \
	$(EMUMACHINE)/x76f100.o \
	$(EMUMACHINE)/z80ctc.o \
	$(EMUMACHINE)/z80dart.o \
	$(EMUMACHINE)/z80dma.o \
	$(EMUMACHINE)/z80pio.o \
	$(EMUMACHINE)/z80sio.o \
	$(EMUMACHINE)/z80sti.o \

EMUVIDEOOBJS = \
	$(EMUVIDEO)/generic.o \
	$(EMUVIDEO)/hd63484.o \
	$(EMUVIDEO)/mc6845.o \
	$(EMUVIDEO)/pc_vga.o \
	$(EMUVIDEO)/pc_video.o \
	$(EMUVIDEO)/poly.o \
	$(EMUVIDEO)/resnet.o \
	$(EMUVIDEO)/rgbutil.o \
	$(EMUVIDEO)/s2636.o \
	$(EMUVIDEO)/saa5050.o \
	$(EMUVIDEO)/tlc34076.o \
	$(EMUVIDEO)/tms34061.o \
	$(EMUVIDEO)/tms9927.o \
	$(EMUVIDEO)/tms9928a.o \
	$(EMUVIDEO)/v9938.o \
	$(EMUVIDEO)/vector.o \
	$(EMUVIDEO)/voodoo.o \

LIBEMUOBJS = $(EMUOBJS) $(EMUSOUNDOBJS) $(EMUAUDIOOBJS) $(EMUDRIVEROBJS) $(EMUMACHINEOBJS) $(EMUVIDEOOBJS)

$(LIBEMU): $(LIBEMUOBJS)



#-------------------------------------------------
# CPU core objects
#-------------------------------------------------

include $(EMUSRC)/cpu/cpu.mak

$(LIBCPU): $(CPUOBJS)

$(LIBDASM): $(DASMOBJS)



#-------------------------------------------------
# sound core objects
#-------------------------------------------------

include $(EMUSRC)/sound/sound.mak

$(LIBSOUND): $(SOUNDOBJS)



#-------------------------------------------------
# additional dependencies
#-------------------------------------------------

$(EMUOBJ)/rendfont.o:	$(EMUOBJ)/uismall.fh

$(EMUOBJ)/video.o:	$(EMUSRC)/rendersw.c
$(EMUVIDEO)/v9938.o:	$(EMUSRC)/video/v9938mod.c


#-------------------------------------------------
# core layouts
#-------------------------------------------------

$(EMUOBJ)/rendlay.o:	$(EMULAYOUT)/dualhovu.lh \
						$(EMULAYOUT)/dualhsxs.lh \
						$(EMULAYOUT)/dualhuov.lh \
						$(EMULAYOUT)/horizont.lh \
						$(EMULAYOUT)/triphsxs.lh \
						$(EMULAYOUT)/vertical.lh \
						$(EMULAYOUT)/ho20ffff.lh \
						$(EMULAYOUT)/ho2eff2e.lh \
						$(EMULAYOUT)/ho4f893d.lh \
						$(EMULAYOUT)/ho88ffff.lh \
						$(EMULAYOUT)/hoa0a0ff.lh \
						$(EMULAYOUT)/hoffe457.lh \
						$(EMULAYOUT)/hoffff20.lh \
						$(EMULAYOUT)/voffff20.lh \

$(EMUOBJ)/video.o:		$(EMULAYOUT)/snap.lh
