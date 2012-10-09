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
EMUIMAGEDEV = $(EMUOBJ)/imagedev
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
	$(EMUOBJ)/imagedev \
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
	$(EMUOBJ)/devcpu.o \
	$(EMUOBJ)/device.o \
	$(EMUOBJ)/didisasm.o \
	$(EMUOBJ)/diexec.o \
	$(EMUOBJ)/diimage.o \
	$(EMUOBJ)/dimemory.o \
	$(EMUOBJ)/dinetwork.o \
	$(EMUOBJ)/dinvram.o \
	$(EMUOBJ)/dirtc.o \
	$(EMUOBJ)/diserial.o \
	$(EMUOBJ)/dislot.o \
	$(EMUOBJ)/disound.o \
	$(EMUOBJ)/distate.o \
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
	$(EMUOBJ)/mame.o \
	$(EMUOBJ)/machine.o \
	$(EMUOBJ)/mconfig.o \
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
	$(EMUOBJ)/ui.o \
	$(EMUOBJ)/uigfx.o \
	$(EMUOBJ)/uiimage.o \
	$(EMUOBJ)/uiinput.o \
	$(EMUOBJ)/uiswlist.o \
	$(EMUOBJ)/uimain.o \
	$(EMUOBJ)/uimenu.o \
	$(EMUOBJ)/validity.o \
	$(EMUOBJ)/video.o \
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
	$(OSDOBJ)/osdepend.o \
	$(OSDOBJ)/osdnet.o

EMUSOUNDOBJS = \
	$(EMUOBJ)/sound/filter.o \
	$(EMUOBJ)/sound/flt_vol.o \
	$(EMUOBJ)/sound/flt_rc.o \
	$(EMUOBJ)/sound/wavwrite.o \

EMUAUDIOOBJS = \

EMUDRIVEROBJS = \
	$(EMUDRIVERS)/empty.o \
	$(EMUDRIVERS)/testcpu.o \

EMUMACHINEOBJS = \
	$(EMUMACHINE)/53c810.o		\
	$(EMUMACHINE)/6522via.o		\
	$(EMUMACHINE)/6525tpi.o		\
	$(EMUMACHINE)/6526cia.o		\
	$(EMUMACHINE)/6532riot.o	\
	$(EMUMACHINE)/6551acia.o	\
	$(EMUMACHINE)/6821pia.o		\
	$(EMUMACHINE)/6840ptm.o		\
	$(EMUMACHINE)/6850acia.o	\
	$(EMUMACHINE)/68681.o		\
	$(EMUMACHINE)/74123.o		\
	$(EMUMACHINE)/74148.o		\
	$(EMUMACHINE)/74153.o		\
	$(EMUMACHINE)/74181.o		\
	$(EMUMACHINE)/7474.o		\
	$(EMUMACHINE)/8042kbdc.o	\
	$(EMUMACHINE)/8237dma.o		\
	$(EMUMACHINE)/8257dma.o		\
	$(EMUMACHINE)/adc0808.o		\
	$(EMUMACHINE)/adc083x.o		\
	$(EMUMACHINE)/adc1038.o		\
	$(EMUMACHINE)/adc1213x.o	\
	$(EMUMACHINE)/am53cf96.o	\
	$(EMUMACHINE)/am9517a.o		\
	$(EMUMACHINE)/amigafdc.o	\
	$(EMUMACHINE)/at28c16.o		\
	$(EMUMACHINE)/cdp1852.o		\
	$(EMUMACHINE)/cdp1871.o		\
	$(EMUMACHINE)/com8116.o		\
	$(EMUMACHINE)/cr589.o		\
	$(EMUMACHINE)/ctronics.o	\
	$(EMUMACHINE)/ds1302.o		\
	$(EMUMACHINE)/ds2401.o		\
	$(EMUMACHINE)/ds2404.o		\
	$(EMUMACHINE)/ds75160a.o	\
	$(EMUMACHINE)/ds75161a.o	\
	$(EMUMACHINE)/e0516.o		\
	$(EMUMACHINE)/eeprom.o		\
	$(EMUMACHINE)/er2055.o		\
	$(EMUMACHINE)/f3853.o		\
	$(EMUMACHINE)/generic.o		\
	$(EMUMACHINE)/i2cmem.o		\
	$(EMUMACHINE)/i8155.o		\
	$(EMUMACHINE)/i8212.o		\
	$(EMUMACHINE)/i8214.o		\
	$(EMUMACHINE)/i8243.o		\
	$(EMUMACHINE)/i8251.o		\
	$(EMUMACHINE)/i8255.o		\
	$(EMUMACHINE)/i8279.o		\
	$(EMUMACHINE)/i8355.o		\
	$(EMUMACHINE)/idectrl.o		\
	$(EMUMACHINE)/im6402.o		\
	$(EMUMACHINE)/ins8154.o		\
	$(EMUMACHINE)/ins8250.o		\
	$(EMUMACHINE)/intelfsh.o	\
	$(EMUMACHINE)/jvsdev.o		\
	$(EMUMACHINE)/jvshost.o		\
	$(EMUMACHINE)/k033906.o		\
	$(EMUMACHINE)/k053252.o 	\
	$(EMUMACHINE)/k056230.o		\
	$(EMUMACHINE)/latch8.o		\
	$(EMUMACHINE)/laserdsc.o	\
	$(EMUMACHINE)/lc89510.o		\
	$(EMUMACHINE)/ldstub.o		\
	$(EMUMACHINE)/ldpr8210.o	\
	$(EMUMACHINE)/ldv1000.o		\
	$(EMUMACHINE)/ldvp931.o		\
	$(EMUMACHINE)/m6m80011ap.o	\
	$(EMUMACHINE)/matsucd.o		\
	$(EMUMACHINE)/mb14241.o		\
	$(EMUMACHINE)/mb3773.o		\
	$(EMUMACHINE)/mb87078.o		\
	$(EMUMACHINE)/mc146818.o	\
	$(EMUMACHINE)/mc2661.o		\
	$(EMUMACHINE)/mc6852.o		\
	$(EMUMACHINE)/mc68901.o		\
	$(EMUMACHINE)/mccs1850.o	\
	$(EMUMACHINE)/mm74c922.o	\
	$(EMUMACHINE)/microtch.o	\
	$(EMUMACHINE)/mos6526.o		\
	$(EMUMACHINE)/mos6529.o		\
	$(EMUMACHINE)/msm5832.o		\
	$(EMUMACHINE)/msm58321.o	\
	$(EMUMACHINE)/msm6242.o		\
	$(EMUMACHINE)/ncr539x.o 	\
	$(EMUMACHINE)/netlist.o		\
	$(EMUMACHINE)/net_lib.o		\
	$(EMUMACHINE)/nmc9306.o		\
	$(EMUMACHINE)/nscsi_bus.o   \
	$(EMUMACHINE)/nscsi_cd.o    \
	$(EMUMACHINE)/nscsi_hd.o    \
	$(EMUMACHINE)/nvram.o		\
	$(EMUMACHINE)/pc16552d.o	\
	$(EMUMACHINE)/pci.o			\
	$(EMUMACHINE)/pd4990a.o		\
	$(EMUMACHINE)/pic8259.o		\
	$(EMUMACHINE)/pit8253.o		\
	$(EMUMACHINE)/pla.o			\
	$(EMUMACHINE)/ram.o			\
	$(EMUMACHINE)/roc10937.o	\
	$(EMUMACHINE)/rp5c01.o		\
	$(EMUMACHINE)/rp5c15.o		\
	$(EMUMACHINE)/rp5h01.o		\
	$(EMUMACHINE)/rtc4543.o		\
	$(EMUMACHINE)/rtc65271.o	\
	$(EMUMACHINE)/rtc9701.o		\
	$(EMUMACHINE)/s3c2400.o		\
	$(EMUMACHINE)/s3c2410.o		\
	$(EMUMACHINE)/s3c2440.o		\
	$(EMUMACHINE)/s3520cf.o		\
	$(EMUMACHINE)/scsibus.o		\
	$(EMUMACHINE)/scsicb.o		\
	$(EMUMACHINE)/scsicd.o		\
	$(EMUMACHINE)/scsidev.o		\
	$(EMUMACHINE)/scsihd.o		\
	$(EMUMACHINE)/scsihle.o		\
	$(EMUMACHINE)/secflash.o	\
	$(EMUMACHINE)/seibu_cop.o	\
	$(EMUMACHINE)/smc91c9x.o	\
	$(EMUMACHINE)/tc009xlvc.o	\
	$(EMUMACHINE)/timekpr.o		\
	$(EMUMACHINE)/tmp68301.o	\
	$(EMUMACHINE)/tms6100.o		\
	$(EMUMACHINE)/tms9901.o		\
	$(EMUMACHINE)/tms9902.o		\
	$(EMUMACHINE)/upd1990a.o	\
	$(EMUMACHINE)/upd4701.o		\
	$(EMUMACHINE)/upd7201.o		\
	$(EMUMACHINE)/v3021.o		\
	$(EMUMACHINE)/wd17xx.o		\
	$(EMUMACHINE)/wd33c93.o		\
	$(EMUMACHINE)/x2212.o		\
	$(EMUMACHINE)/x76f041.o		\
	$(EMUMACHINE)/x76f100.o		\
	$(EMUMACHINE)/z80ctc.o		\
	$(EMUMACHINE)/z80dart.o		\
	$(EMUMACHINE)/z80dma.o		\
	$(EMUMACHINE)/z80pio.o		\
	$(EMUMACHINE)/z80sio.o		\
	$(EMUMACHINE)/z80sti.o		\
	$(EMUMACHINE)/z8536.o		\

EMUVIDEOOBJS = \
	$(EMUVIDEO)/315_5124.o		\
	$(EMUVIDEO)/bufsprite.o		\
	$(EMUVIDEO)/cdp1861.o		\
	$(EMUVIDEO)/cdp1862.o		\
	$(EMUVIDEO)/crt9007.o		\
	$(EMUVIDEO)/crt9021.o		\
	$(EMUVIDEO)/crt9212.o		\
	$(EMUVIDEO)/dm9368.o		\
	$(EMUVIDEO)/generic.o		\
	$(EMUVIDEO)/h63484.o		\
	$(EMUVIDEO)/hd44102.o		\
	$(EMUVIDEO)/hd61830.o		\
	$(EMUVIDEO)/hd63484.o		\
	$(EMUVIDEO)/huc6202.o		\
	$(EMUVIDEO)/huc6260.o		\
	$(EMUVIDEO)/huc6261.o		\
	$(EMUVIDEO)/huc6270.o		\
	$(EMUVIDEO)/huc6272.o		\
	$(EMUVIDEO)/i8275.o			\
	$(EMUVIDEO)/k053250.o		\
	$(EMUVIDEO)/m50458.o		\
	$(EMUVIDEO)/mb90082.o		\
	$(EMUVIDEO)/mc6845.o		\
	$(EMUVIDEO)/msm6255.o		\
	$(EMUVIDEO)/pc_cga.o		\
	$(EMUVIDEO)/cgapal.o		\
	$(EMUVIDEO)/pc_vga.o		\
	$(EMUVIDEO)/poly.o			\
	$(EMUVIDEO)/psx.o			\
	$(EMUVIDEO)/ramdac.o		\
	$(EMUVIDEO)/resnet.o		\
	$(EMUVIDEO)/rgbutil.o		\
	$(EMUVIDEO)/s2636.o			\
	$(EMUVIDEO)/saa5050.o		\
	$(EMUVIDEO)/sed1330.o		\
	$(EMUVIDEO)/tlc34076.o		\
	$(EMUVIDEO)/tms34061.o		\
	$(EMUVIDEO)/tms9927.o		\
	$(EMUVIDEO)/tms9928a.o		\
	$(EMUVIDEO)/upd3301.o		\
	$(EMUVIDEO)/v9938.o			\
	$(EMUVIDEO)/vector.o		\
	$(EMUVIDEO)/voodoo.o		\

EMUIMAGEDEVOBJS = \
	$(EMUIMAGEDEV)/bitbngr.o	\
	$(EMUIMAGEDEV)/cartslot.o	\
	$(EMUIMAGEDEV)/cassette.o	\
	$(EMUIMAGEDEV)/chd_cd.o		\
	$(EMUIMAGEDEV)/flopdrv.o	\
	$(EMUIMAGEDEV)/floppy.o		\
	$(EMUIMAGEDEV)/harddriv.o	\
	$(EMUIMAGEDEV)/printer.o	\
	$(EMUIMAGEDEV)/serial.o		\
	$(EMUIMAGEDEV)/snapquik.o	\


LIBEMUOBJS = $(EMUOBJS) $(EMUSOUNDOBJS) $(EMUAUDIOOBJS) $(EMUDRIVEROBJS) $(EMUMACHINEOBJS) $(EMUIMAGEDEVOBJS) $(EMUVIDEOOBJS)

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

$(EMUMACHINE)/s3c2400.o:	$(EMUSRC)/machine/s3c24xx.c
$(EMUMACHINE)/s3c2410.o:	$(EMUSRC)/machine/s3c24xx.c
$(EMUMACHINE)/s3c2440.o:	$(EMUSRC)/machine/s3c24xx.c


#-------------------------------------------------
# core layouts
#-------------------------------------------------

$(EMUOBJ)/rendlay.o:	$(EMULAYOUT)/dualhovu.lh \
						$(EMULAYOUT)/dualhsxs.lh \
						$(EMULAYOUT)/dualhuov.lh \
						$(EMULAYOUT)/horizont.lh \
						$(EMULAYOUT)/triphsxs.lh \
						$(EMULAYOUT)/quadhsxs.lh \
						$(EMULAYOUT)/vertical.lh \
						$(EMULAYOUT)/ho1880ff.lh \
						$(EMULAYOUT)/ho20ffff.lh \
						$(EMULAYOUT)/ho2eff2e.lh \
						$(EMULAYOUT)/ho4f893d.lh \
						$(EMULAYOUT)/ho88ffff.lh \
						$(EMULAYOUT)/hoa0a0ff.lh \
						$(EMULAYOUT)/hoffe457.lh \
						$(EMULAYOUT)/hoffff20.lh \
						$(EMULAYOUT)/voffff20.lh \
						$(EMULAYOUT)/lcd.lh \
						$(EMULAYOUT)/lcd_rot.lh \
						$(EMULAYOUT)/noscreens.lh \

$(EMUOBJ)/video.o:		$(EMULAYOUT)/snap.lh
