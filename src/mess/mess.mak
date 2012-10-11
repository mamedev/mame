###########################################################################
#
#   mess.mak
#
#   MESS target makefile
#
###########################################################################

ifeq ($(TARGET),mess)
# In order to keep dependencies reasonable, we exclude objects in the base of
# $(SRC)/emu, as well as all the OSD objects and anything in the $(OBJ) tree
depend: maketree $(MAKEDEP_TARGET)
	@echo Rebuilding depend_$(TARGET).mak...
	$(MAKEDEP) -I. $(INCPATH) -X$(SRC)/emu -X$(SRC)/osd/... -X$(OBJ)/... src/$(TARGET) > depend_$(TARGET).mak
endif

# include MESS core defines
include $(SRC)/mess/messcore.mak


#-------------------------------------------------
# specify available CPU cores
#-------------------------------------------------

CPUS += Z80
CPUS += Z180
CPUS += I8085
CPUS += M6502
CPUS += H6280
CPUS += I86
CPUS += I386
CPUS += NEC
CPUS += V30MZ
CPUS += V60
CPUS += MCS48
CPUS += MCS51
CPUS += M6800
CPUS += M6805
CPUS += HD6309
CPUS += M6809
CPUS += KONAMI
CPUS += M680X0
CPUS += T11
CPUS += S2650
CPUS += TMS340X0
CPUS += TMS9900
CPUS += TMS9995
CPUS += TMS9900L
CPUS += TMS9995L
CPUS += Z8000
#CPUS += Z8001
CPUS += TMS32010
CPUS += TMS32025
CPUS += TMS32031
CPUS += TMS32051
CPUS += TMS57002
CPUS += CCPU
CPUS += ADSP21XX
CPUS += ASAP
#CPUS += AM29000
CPUS += UPD7810
CPUS += ARM
CPUS += ARM7
CPUS += JAGUAR
CPUS += CUBEQCPU
CPUS += ESRIP
CPUS += MIPS
CPUS += PSX
CPUS += SH2
CPUS += SH4
CPUS += DSP16A
CPUS += DSP32C
CPUS += PIC16C5X
CPUS += PIC16C62X
CPUS += G65816
CPUS += SPC700
CPUS += E1
CPUS += I860
CPUS += I960
CPUS += H83002
CPUS += H83334
CPUS += V810
CPUS += M37710
CPUS += POWERPC
CPUS += SE3208
CPUS += MC68HC11
CPUS += ADSP21062
CPUS += DSP56156
CPUS += RSP
CPUS += ALPHA8201
CPUS += COP400
CPUS += TLCS90
CPUS += TLCS900
CPUS += MB88XX
CPUS += MB86233
CPUS += SSP1601
CPUS += APEXC
CPUS += CP1610
CPUS += F8
CPUS += LH5801
CPUS += PDP1
CPUS += SATURN
CPUS += SC61860
CPUS += LR35902
CPUS += TMS7000
CPUS += SM8500
CPUS += MINX
CPUS += SSEM
CPUS += AVR8
CPUS += TMS0980
CPUS += I4004
CPUS += SUPERFX
CPUS += Z8
CPUS += I8008
CPUS += SCMP
#CPUS += MN10200
CPUS += COSMAC
CPUS += UNSP
CPUS += HCD62121
CPUS += PPS4
CPUS += UPD7725
CPUS += HD61700

#-------------------------------------------------
# specify available sound cores; some of these are
# only for MAME and so aren't included
#-------------------------------------------------

#SOUNDS += SAMPLES
SOUNDS += DAC
SOUNDS += DMADAC
SOUNDS += SPEAKER
SOUNDS += BEEP
SOUNDS += DISCRETE
SOUNDS += AY8910
SOUNDS += YM2151
SOUNDS += YM2203
SOUNDS += YM2413
SOUNDS += YM2608
SOUNDS += YM2610
SOUNDS += YM2610B
SOUNDS += YM2612
#SOUNDS += YM3438
SOUNDS += YM3812
SOUNDS += YM3526
#SOUNDS += Y8950
SOUNDS += YMF262
#SOUNDS += YMF271
#SOUNDS += YMF278B
#SOUNDS += YMZ280B
SOUNDS += SN76477
SOUNDS += SN76496
SOUNDS += POKEY
SOUNDS += TIA
SOUNDS += NES
SOUNDS += ASTROCADE
#SOUNDS += NAMCO
#SOUNDS += NAMCO_15XX
#SOUNDS += NAMCO_CUS30
#SOUNDS += NAMCO_52XX
#SOUNDS += NAMCO_63701X
SOUNDS += T6W28
#SOUNDS += SNKWAVE
#SOUNDS += C140
#SOUNDS += C352
#SOUNDS += TMS36XX
#SOUNDS += TMS3615
#SOUNDS += TMS5110
SOUNDS += TMS5220
SOUNDS += VLM5030
#SOUNDS += ADPCM
SOUNDS += MSM5205
#SOUNDS += MSM5232
SOUNDS += OKIM6258
SOUNDS += OKIM6295
#SOUNDS += OKIM6376
SOUNDS += UPD7759
SOUNDS += HC55516
#SOUNDS += K005289
#SOUNDS += K007232
SOUNDS += K051649
#SOUNDS += K053260
#SOUNDS += K054539
#SOUNDS += K056800
#SOUNDS += SEGAPCM
#SOUNDS += MULTIPCM
SOUNDS += SCSP
SOUNDS += AICA
SOUNDS += RF5C68
#SOUNDS += RF5C400
#SOUNDS += CEM3394
SOUNDS += QSOUND
SOUNDS += SAA1099
#SOUNDS += IREMGA20
SOUNDS += ES5503
SOUNDS += ES5505
SOUNDS += ES5506
#SOUNDS += BSMT2000
#SOUNDS += GAELCO_CG1V
#SOUNDS += GAELCO_GAE1
SOUNDS += C6280
#SOUNDS += SP0250
SOUNDS += SPU
SOUNDS += CDDA
#SOUNDS += ICS2115
#SOUNDS += ST0016
#SOUNDS += NILE
#SOUNDS += X1_010
#SOUNDS += VRENDER0
SOUNDS += VOTRAX
#SOUNDS += ES8712
SOUNDS += CDP1869
SOUNDS += S14001A
SOUNDS += WAVE
SOUNDS += SID6581
SOUNDS += SID8580
SOUNDS += SP0256
#SOUNDS += DIGITALKER
SOUNDS += CDP1863
SOUNDS += CDP1864
#SOUNDS += ZSG2
SOUNDS += MOS656X
#SOUNDS += S2636
SOUNDS += ASC
#SOUNDS += CUSTOM
SOUNDS += SOCRATES
SOUNDS += TMC0285
SOUNDS += TMS5200
#SOUNDS += CD2801
#SOUNDS += CD2802
#SOUNDS += M58817
#SOUNDS += TMC0281
#SOUNDS += TMS5100
#SOUNDS += TMS5110A
SOUNDS += LMC1992
SOUNDS += AWACS

#-------------------------------------------------
# this is the list of driver libraries that
# comprise MESS plus messdriv.o which contains
# the list of drivers
#-------------------------------------------------

DRVLIBS += \
	$(MESSOBJ)/acorn.a \
	$(MESSOBJ)/act.a \
	$(MESSOBJ)/alesis.a \
	$(MESSOBJ)/amiga.a \
	$(MESSOBJ)/amstrad.a \
	$(MESSOBJ)/apf.a \
	$(MESSOBJ)/apollo.a \
	$(MESSOBJ)/apple.a \
	$(MESSOBJ)/applied.a \
	$(MESSOBJ)/arcadia.a \
	$(MESSOBJ)/ascii.a \
	$(MESSOBJ)/at.a \
	$(MESSOBJ)/atari.a \
	$(MESSOBJ)/att.a \
	$(MESSOBJ)/bally.a \
	$(MESSOBJ)/bandai.a \
	$(MESSOBJ)/be.a \
	$(MESSOBJ)/bnpo.a \
	$(MESSOBJ)/bondwell.a \
	$(MESSOBJ)/booth.a \
	$(MESSOBJ)/camputers.a \
	$(MESSOBJ)/canon.a \
	$(MESSOBJ)/cantab.a \
	$(MESSOBJ)/casio.a \
	$(MESSOBJ)/cbm.a \
	$(MESSOBJ)/cccp.a \
	$(MESSOBJ)/cce.a \
	$(MESSOBJ)/ccs.a \
	$(MESSOBJ)/chromatics.a \
	$(MESSOBJ)/coleco.a \
	$(MESSOBJ)/cromemco.a \
	$(MESSOBJ)/comx.a \
	$(MESSOBJ)/concept.a \
	$(MESSOBJ)/conitec.a \
	$(MESSOBJ)/cybiko.a \
	$(MESSOBJ)/dai.a \
	$(MESSOBJ)/ddr.a \
	$(MESSOBJ)/dec.a \
	$(MESSOBJ)/dicksmth.a \
	$(MESSOBJ)/dms.a \
	$(MESSOBJ)/dragon.a \
	$(MESSOBJ)/drc.a \
	$(MESSOBJ)/eaca.a \
	$(MESSOBJ)/einis.a \
	$(MESSOBJ)/elektor.a \
	$(MESSOBJ)/elektrka.a \
	$(MESSOBJ)/ensoniq.a \
	$(MESSOBJ)/entex.a \
	$(MESSOBJ)/epoch.a \
	$(MESSOBJ)/epson.a \
	$(MESSOBJ)/exeltel.a \
	$(MESSOBJ)/exidy.a \
	$(MESSOBJ)/fairch.a \
	$(MESSOBJ)/fujitsu.a \
	$(MESSOBJ)/funtech.a \
	$(MESSOBJ)/galaxy.a \
	$(MESSOBJ)/gamepark.a \
	$(MESSOBJ)/grundy.a \
	$(MESSOBJ)/hartung.a \
	$(MESSOBJ)/heathkit.a \
	$(MESSOBJ)/hec2hrp.a \
	$(MESSOBJ)/hegener.a \
	$(MESSOBJ)/hitachi.a \
	$(MESSOBJ)/homebrew.a \
	$(MESSOBJ)/homelab.a \
	$(MESSOBJ)/hp.a \
	$(MESSOBJ)/intel.a \
	$(MESSOBJ)/intelgnt.a \
	$(MESSOBJ)/interton.a \
	$(MESSOBJ)/intv.a \
	$(MESSOBJ)/kaypro.a \
	$(MESSOBJ)/koei.a \
	$(MESSOBJ)/kyocera.a \
	$(MESSOBJ)/luxor.a \
	$(MESSOBJ)/magnavox.a \
	$(MESSOBJ)/matsushi.a \
	$(MESSOBJ)/mattel.a \
	$(MESSOBJ)/mchester.a \
	$(MESSOBJ)/memotech.a \
	$(MESSOBJ)/mgu.a \
	$(MESSOBJ)/microkey.a \
	$(MESSOBJ)/mit.a \
	$(MESSOBJ)/mits.a \
	$(MESSOBJ)/mitsubishi.a \
	$(MESSOBJ)/morrow.a \
	$(MESSOBJ)/mos.a \
	$(MESSOBJ)/motorola.a \
	$(MESSOBJ)/multitch.a \
	$(MESSOBJ)/nakajima.a \
	$(MESSOBJ)/nascom.a \
	$(MESSOBJ)/ne.a \
	$(MESSOBJ)/nec.a \
	$(MESSOBJ)/netronic.a \
	$(MESSOBJ)/next.a \
	$(MESSOBJ)/nintendo.a \
	$(MESSOBJ)/nokia.a \
	$(MESSOBJ)/novag.a \
	$(MESSOBJ)/olivetti.a \
	$(MESSOBJ)/omnibyte.a \
	$(MESSOBJ)/orion.a \
	$(MESSOBJ)/osborne.a \
	$(MESSOBJ)/osi.a \
	$(MESSOBJ)/palm.a \
	$(MESSOBJ)/parker.a \
	$(MESSOBJ)/pc.a \
	$(MESSOBJ)/pcshare.a \
	$(MESSOBJ)/pdp1.a \
	$(MESSOBJ)/pel.a \
	$(MESSOBJ)/philips.a \
	$(MESSOBJ)/pitronic.a \
	$(MESSOBJ)/poly88.a \
	$(MESSOBJ)/psion.a \
	$(MESSOBJ)/radio.a \
	$(MESSOBJ)/rca.a \
	$(MESSOBJ)/rm.a \
	$(MESSOBJ)/robotron.a \
	$(MESSOBJ)/rockwell.a \
	$(MESSOBJ)/samcoupe.a \
	$(MESSOBJ)/samsung.a \
	$(MESSOBJ)/sanyo.a \
	$(MESSOBJ)/sega.a \
	$(MESSOBJ)/sgi.a \
	$(MESSOBJ)/sharp.a \
	$(MESSOBJ)/sinclair.a \
	$(MESSOBJ)/skeleton.a \
	$(MESSOBJ)/snk.a \
	$(MESSOBJ)/sony.a \
	$(MESSOBJ)/sord.a \
	$(MESSOBJ)/special.a \
	$(MESSOBJ)/sun.a \
	$(MESSOBJ)/svi.a \
	$(MESSOBJ)/svision.a \
	$(MESSOBJ)/synertec.a \
	$(MESSOBJ)/tandberg.a \
	$(MESSOBJ)/tangerin.a \
	$(MESSOBJ)/tatung.a \
	$(MESSOBJ)/teamconc.a \
	$(MESSOBJ)/tektroni.a \
	$(MESSOBJ)/telenova.a \
	$(MESSOBJ)/telercas.a \
	$(MESSOBJ)/tem.a \
	$(MESSOBJ)/tesla.a \
	$(MESSOBJ)/test.a \
	$(MESSOBJ)/thomson.a \
	$(MESSOBJ)/ti.a \
	$(MESSOBJ)/tiger.a \
	$(MESSOBJ)/tigertel.a \
	$(MESSOBJ)/tiki.a \
	$(MESSOBJ)/tomy.a \
	$(MESSOBJ)/toshiba.a \
	$(MESSOBJ)/trs.a \
	$(MESSOBJ)/unisys.a \
	$(MESSOBJ)/veb.a \
	$(MESSOBJ)/vidbrain.a \
	$(MESSOBJ)/videoton.a \
	$(MESSOBJ)/visual.a \
	$(MESSOBJ)/votrax.a \
	$(MESSOBJ)/vtech.a \
	$(MESSOBJ)/wang.a \
	$(MESSOBJ)/wavemate.a \
	$(MESSOBJ)/xerox.a \
	$(MESSOBJ)/zpa.a \
	$(MESSOBJ)/zvt.a \
	$(MESSOBJ)/shared.a \
	$(MESSOBJ)/mame.a

#-------------------------------------------------
# the following files are MAME components and
# shared across a number of drivers
#-------------------------------------------------

$(MESSOBJ)/mame.a: \
	$(MAME_MACHINE)/pckeybrd.o	\
	$(MAME_MACHINE)/archimds.o	\
	$(MAME_VIDEO)/archimds.o	\
	$(MAME_VIDEO)/amiga.o		\
	$(MAME_VIDEO)/amigaaga.o	\
	$(MAME_MACHINE)/amiga.o		\
	$(MAME_AUDIO)/amiga.o		\
	$(MAME_MACHINE)/cd32.o	\
	$(MAME_VIDEO)/tia.o			\
	$(MAME_MACHINE)/atari.o		\
	$(MAME_VIDEO)/atari.o		\
	$(MAME_VIDEO)/antic.o		\
	$(MAME_VIDEO)/gtia.o		\
	$(MAME_DRIVERS)/jaguar.o	\
	$(MAME_AUDIO)/jaguar.o		\
	$(MAME_VIDEO)/jaguar.o		\
	$(MAME_VIDEO)/astrocde.o	\
	$(MAME_MACHINE)/kabuki.o	\
	$(MAME_VIDEO)/pk8000.o		\
	$(MAME_VIDEO)/vdc.o			\
	$(MAME_VIDEO)/ppu2c0x.o		\
	$(MAME_AUDIO)/snes_snd.o	\
	$(MAME_MACHINE)/snes.o		\
	$(MAME_VIDEO)/snes.o		\
	$(MAME_MACHINE)/n64.o		\
	$(MAME_VIDEO)/n64.o			\
	$(MAME_VIDEO)/rdpblend.o	\
	$(MAME_VIDEO)/rdptpipe.o	\
	$(MAME_VIDEO)/rdpspn16.o	\
	$(MAME_MACHINE)/pcshare.o	\
	$(MAME_MACHINE)/megadriv.o  \
	$(MAME_MACHINE)/megacd.o \
	$(MAME_MACHINE)/mega32x.o \
	$(MAME_MACHINE)/megasvp.o \
	$(MAME_MACHINE)/megavdp.o \
	$(MAME_MACHINE)/dc.o		\
	$(MAME_DRIVERS)/naomi.o 	\
	$(MAME_MACHINE)/dc.o		\
	$(MAME_VIDEO)/dc.o			\
	$(MAME_MACHINE)/naomi.o 	\
	$(MAME_MACHINE)/naomig1.o	\
	$(MAME_MACHINE)/naomibd.o	\
	$(MAME_MACHINE)/naomirom.o	\
	$(MAME_MACHINE)/naomigd.o	\
	$(MAME_MACHINE)/naomim1.o	\
	$(MAME_MACHINE)/naomim2.o	\
	$(MAME_MACHINE)/naomim4.o	\
	$(MAME_MACHINE)/awboard.o	\
	$(MAME_MACHINE)/mie.o		\
	$(MAME_MACHINE)/maple-dc.o	\
	$(MAME_MACHINE)/mapledev.o	\
	$(MAME_MACHINE)/dc-ctrl.o	\
	$(MAME_MACHINE)/jvs13551.o	\
	$(MAME_VIDEO)/dc.o			\
	$(MAME_VIDEO)/neogeo.o		\
	$(MAME_MACHINE)/neoprot.o	\
	$(MAME_MACHINE)/neocrypt.o	\
	$(MAME_MACHINE)/psx.o		\
	$(MAME_DRIVERS)/cdi.o		\
	$(MAME_MACHINE)/cdi070.o	\
	$(MAME_MACHINE)/cdicdic.o	\
	$(MAME_MACHINE)/cdislave.o	\
	$(MAME_VIDEO)/mcd212.o		\
	$(MAME_DRIVERS)/cd32.o  	\
	$(MAME_DRIVERS)/3do.o		\
	$(MAME_MACHINE)/3do.o		\
	$(MAME_DRIVERS)/vectrex.o	\
	$(MAME_VIDEO)/vectrex.o		\
	$(MAME_MACHINE)/vectrex.o	\
	$(MAME_DRIVERS)/saturn.o	\
	$(MAME_MACHINE)/stvcd.o		\
	$(MAME_MACHINE)/scudsp.o	\
	$(MAME_MACHINE)/stvprot.o	\
	$(MAME_MACHINE)/smpc.o		\
	$(MAME_VIDEO)/stvvdp1.o		\
	$(MAME_VIDEO)/stvvdp2.o		\
	$(MAME_DRIVERS)/cps1.o	\
	$(MAME_VIDEO)/cps1.o	\
	$(MAME_DRIVERS)/konamim2.o \


#-------------------------------------------------
# the following files are general components and
# shared across a number of drivers
#-------------------------------------------------

$(MESSOBJ)/shared.a: \
	$(MESS_AUDIO)/mea8000.o		\
	$(MESS_AUDIO)/spchroms.o	\
	$(MESS_DEVICES)/microdrv.o	\
	$(MESS_MACHINE)/74145.o		\
	$(MESS_MACHINE)/8530scc.o	\
	$(MESS_MACHINE)/at45dbxx.o	\
	$(MESS_MACHINE)/ay31015.o	\
	$(MESS_MACHINE)/er59256.o	\
	$(MESS_MACHINE)/hd63450.o	\
	$(MESS_MACHINE)/i8271.o		\
	$(MESS_MACHINE)/ieee488.o	\
	$(MESS_MACHINE)/isa.o		\
	$(MESS_MACHINE)/kb3600.o	\
	$(MESS_MACHINE)/keyboard.o	\
	$(MESS_MACHINE)/kr2376.o	\
	$(MESS_MACHINE)/mc6843.o	\
	$(MESS_MACHINE)/mc6846.o	\
	$(MESS_MACHINE)/mc6854.o	\
	$(MESS_MACHINE)/mm58274c.o	\
	$(MESS_MACHINE)/mpc105.o	\
	$(MESS_MACHINE)/mos6530.o	\
	$(MESS_MACHINE)/s100.o		\
	$(MESS_MACHINE)/serial.o	\
	$(MESS_MACHINE)/upd765.o	\
	$(MESS_MACHINE)/ncr5380.o	\
	$(MESS_MACHINE)/ncr5390.o	\
	$(MESS_MACHINE)/pc_kbdc.o	\
	$(MESS_MACHINE)/pc_lpt.o	\
	$(MESS_MACHINE)/cntr_covox.o \
	$(MESS_MACHINE)/pcf8593.o	\
	$(MESS_MACHINE)/smartmed.o	\
	$(MESS_MACHINE)/smc92x4.o	\
	$(MESS_MACHINE)/sst39vfx.o	\
	$(MESS_MACHINE)/terminal.o	\
	$(MESS_MACHINE)/teleprinter.o	\
	$(MESS_MACHINE)/upd7002.o	\
	$(MESS_MACHINE)/wd11c00_17.o	\
	$(MESS_MACHINE)/wd2010.o	\
	$(MESS_VIDEO)/dl1416.o		\
	$(MESS_VIDEO)/hd44780.o		\
	$(MESS_VIDEO)/hd66421.o		\
	$(MESS_VIDEO)/mc6847.o		\
	$(MESS_VIDEO)/tms3556.o		\
	$(MESS_VIDEO)/upd7220.o		\
	$(MESS_MACHINE)/applefdc.o	\
	$(MESS_DEVICES)/sonydriv.o	\
	$(MESS_DEVICES)/appldriv.o	\
	$(MESS_MACHINE)/dp8390.o	\
	$(MESS_MACHINE)/ne1000.o	\
	$(MESS_MACHINE)/ne2000.o	\
	$(MESS_MACHINE)/wd1772.o	\
	$(MESS_MACHINE)/3c503.o		\
	$(MESS_FORMATS)/z80bin.o	\
	$(MESS_MACHINE)/mb8795.o	\
	$(MESS_MACHINE)/null_modem.o	\
	$(MESS_MACHINE)/vcsctrl.o	\
	$(MESS_MACHINE)/vcs_joy.o	\
	$(MESS_MACHINE)/vcs_lightpen.o	\
	$(MESS_MACHINE)/vcs_paddles.o	\



#-------------------------------------------------
# manufacturer-specific groupings for drivers
#-------------------------------------------------

$(MESSOBJ)/acorn.a:				\
	$(MESS_DRIVERS)/a310.o		\
	$(MESS_DRIVERS)/a6809.o		\
	$(MESS_DRIVERS)/a7000.o		\
	$(MESS_DRIVERS)/acrnsys1.o	\
	$(MESS_DRIVERS)/atom.o		\
	$(MESS_FORMATS)/atom_atm.o	\
	$(MESS_VIDEO)/bbc.o			\
	$(MESS_MACHINE)/bbc.o		\
	$(MESS_DRIVERS)/bbc.o		\
	$(MESS_DRIVERS)/bbcbc.o		\
	$(MESS_MACHINE)/econet.o	\
	$(MESS_MACHINE)/e01.o		\
	$(MESS_VIDEO)/electron.o	\
	$(MESS_MACHINE)/electron.o	\
	$(MESS_DRIVERS)/electron.o	\
	$(MESS_DRIVERS)/z88.o		\
	$(MESS_VIDEO)/z88.o			\
	$(MESS_MACHINE)/z88cart.o	\
	$(MESS_MACHINE)/z88_flash.o	\
	$(MESS_MACHINE)/z88_ram.o	\
	$(MESS_MACHINE)/z88_rom.o	\
	$(MESS_MACHINE)/upd65031.o	\

$(MESSOBJ)/act.a:				\
	$(MESS_DRIVERS)/apricot.o	\
	$(MESS_DRIVERS)/apricotf.o	\
	$(MESS_DRIVERS)/apricotp.o	\
	$(MESS_MACHINE)/apricotkb.o	\
	$(MESS_DRIVERS)/victor9k.o	\
	$(MESS_MACHINE)/victor9kb.o	\

$(MESSOBJ)/alesis.a:			\
	$(MESS_DRIVERS)/alesis.o	\
	$(MESS_AUDIO)/alesis.o		\
	$(MESS_VIDEO)/alesis.o		\

$(MESSOBJ)/amiga.a:				\
	$(MESS_MACHINE)/amigacrt.o	\
	$(MESS_MACHINE)/amigacd.o	\
	$(MESS_MACHINE)/amigakbd.o	\
	$(MESS_DRIVERS)/amiga.o		\
	$(MESS_DRIVERS)/a3000.o		\
	$(MESS_DRIVERS)/ami1200.o	\

$(MESSOBJ)/amstrad.a:			\
	$(MESS_DRIVERS)/amstrad.o	\
	$(MESS_MACHINE)/amstrad.o	\
	$(MESS_MACHINE)/cpcexp.o	\
	$(MESS_MACHINE)/cpc_ssa1.o	\
	$(MESS_MACHINE)/cpc_rom.o	\
	$(MESS_MACHINE)/mface2.o	\
	$(MESS_DRIVERS)/amstr_pc.o	\
	$(MESS_MACHINE)/amstr_pc.o	\
	$(MESS_DRIVERS)/pc1512.o	\
	$(MESS_MACHINE)/pc1512kb.o	\
	$(MESS_VIDEO)/pc1512.o		\
	$(MESS_VIDEO)/pc1640.o		\
	$(MESS_MACHINE)/isa_wdxt_gen.o	\
	$(MESS_VIDEO)/nc.o			\
	$(MESS_DRIVERS)/nc.o		\
	$(MESS_MACHINE)/nc.o		\
	$(MESS_VIDEO)/pcw.o			\
	$(MESS_DRIVERS)/pcw.o		\
	$(MESS_VIDEO)/pcw16.o		\
	$(MESS_DRIVERS)/pda600.o	\
	$(MESS_MACHINE)/hd64610.o	\
	$(MESS_DRIVERS)/pcw16.o		\

$(MESSOBJ)/apf.a:				\
	$(MESS_DRIVERS)/apf.o		\

$(MESSOBJ)/apollo.a:			\
	$(MESS_DRIVERS)/apollo.o	\
	$(MESS_VIDEO)/apollo.o  	\
	$(MESS_MACHINE)/sc499.o 	\
	$(MESS_MACHINE)/omti8621.o  \
	$(MESS_MACHINE)/apollo_dbg.o  \
	$(MESS_MACHINE)/apollo_eth.o  \
	$(MESS_MACHINE)/apollo_net.o  \
	$(MESS_MACHINE)/apollo_kbd.o  \
	$(MESS_MACHINE)/3c505.o  \
	$(MESS_MACHINE)/apollo.o  \

$(MESSOBJ)/apple.a:				\
	$(MESS_VIDEO)/apple2.o		\
	$(MESS_MACHINE)/apple2.o	\
	$(MESS_DRIVERS)/apple2.o	\
	$(MESS_VIDEO)/apple2gs.o	\
	$(MESS_MACHINE)/apple2gs.o	\
	$(MESS_DRIVERS)/apple2gs.o	\
	$(MESS_MACHINE)/ay3600.o	\
	$(MESS_MACHINE)/a2bus.o     \
	$(MESS_MACHINE)/a2lang.o	\
	$(MESS_MACHINE)/a2diskii.o	\
	$(MESS_MACHINE)/a2mockingboard.o	\
	$(MESS_MACHINE)/a2cffa.o	\
	$(MESS_MACHINE)/a2memexp.o  \
	$(MESS_MACHINE)/a2scsi.o    \
	$(MESS_MACHINE)/a2thunderclock.o    \
	$(MESS_MACHINE)/a2softcard.o \
	$(MESS_MACHINE)/a2videoterm.o \
    $(MESS_MACHINE)/a2ssc.o \
    $(MESS_MACHINE)/a2swyft.o \
    $(MESS_MACHINE)/a2eauxslot.o \
    $(MESS_MACHINE)/a2themill.o \
    $(MESS_MACHINE)/a2sam.o \
    $(MESS_MACHINE)/a2alfam2.o \
    $(MESS_MACHINE)/laser128.o \
    $(MESS_MACHINE)/a2echoii.o \
    $(MESS_MACHINE)/a2arcadebd.o \
	$(MESS_MACHINE)/lisa.o		\
	$(MESS_DRIVERS)/lisa.o		\
	$(MESS_MACHINE)/nubus.o     \
	$(MESS_AUDIO)/mac.o			\
	$(MESS_VIDEO)/mac.o			\
	$(MESS_MACHINE)/mac.o		\
	$(MESS_MACHINE)/macrtc.o	\
	$(MESS_MACHINE)/macadb.o	\
	$(MESS_DRIVERS)/mac.o		\
	$(MESS_MACHINE)/macpci.o	\
	$(MESS_DRIVERS)/macpci.o	\
	$(MESS_VIDEO)/apple1.o		\
	$(MESS_MACHINE)/apple1.o	\
	$(MESS_DRIVERS)/apple1.o	\
	$(MESS_VIDEO)/apple3.o		\
	$(MESS_MACHINE)/apple3.o	\
	$(MESS_DRIVERS)/apple3.o	\
	$(MESS_MACHINE)/egret.o     \
	$(MESS_MACHINE)/cuda.o      \
	$(MESS_MACHINE)/mackbd.o    \
	$(MESS_VIDEO)/nubus_48gc.o	\
	$(MESS_VIDEO)/nubus_cb264.o \
	$(MESS_VIDEO)/nubus_vikbw.o \
	$(MESS_VIDEO)/nubus_specpdq.o \
	$(MESS_VIDEO)/nubus_m2hires.o \
	$(MESS_VIDEO)/nubus_spec8.o \
	$(MESS_VIDEO)/nubus_radiustpd.o \
	$(MESS_MACHINE)/nubus_asntmc3b.o \
	$(MESS_VIDEO)/nubus_wsportrait.o \

$(MESSOBJ)/applied.a:			\
	$(MESS_VIDEO)/mbee.o		\
	$(MESS_MACHINE)/mbee.o		\
	$(MESS_DRIVERS)/mbee.o		\

$(MESSOBJ)/arcadia.a:			\
	$(MESS_DRIVERS)/arcadia.o	\
	$(MESS_AUDIO)/arcadia.o		\
	$(MESS_VIDEO)/arcadia.o		\

$(MESSOBJ)/ascii.a:				\
	$(MESS_DRIVERS)/msx.o		\
	$(MESS_MACHINE)/msx.o		\
	$(MESS_MACHINE)/msx_slot.o	\

$(MESSOBJ)/at.a:				\
	$(MESS_MACHINE)/at_keybc.o	\
	$(MESS_MACHINE)/ps2.o		\
	$(MESS_MACHINE)/cs4031.o	\
	$(MESS_MACHINE)/cs8221.o	\
	$(MESS_MACHINE)/at.o		\
	$(MESS_DRIVERS)/at.o		\

$(MESSOBJ)/atari.a:				\
	$(MESS_MACHINE)/ataricrt.o	\
	$(MESS_MACHINE)/atarifdc.o	\
	$(MESS_DRIVERS)/atari400.o	\
	$(MESS_MACHINE)/a7800.o		\
	$(MESS_DRIVERS)/a7800.o		\
	$(MESS_VIDEO)/a7800.o		\
	$(MESS_DRIVERS)/a2600.o		\
	$(MESS_DRIVERS)/atarist.o	\
	$(MESS_VIDEO)/atarist.o 	\
	$(MESS_DRIVERS)/lynx.o		\
	$(MESS_AUDIO)/lynx.o		\
	$(MESS_MACHINE)/lynx.o		\
	$(MESS_DRIVERS)/portfoli.o	\

$(MESSOBJ)/att.a:				\
	$(MESS_DRIVERS)/unixpc.o	\

$(MESSOBJ)/bally.a:				\
	$(MESS_DRIVERS)/astrocde.o	\

$(MESSOBJ)/bandai.a:			\
	$(MESS_DRIVERS)/rx78.o		\
	$(MESS_DRIVERS)/wswan.o		\
	$(MESS_MACHINE)/wswan.o		\
	$(MESS_VIDEO)/wswan.o		\
	$(MESS_AUDIO)/wswan.o		\

$(MESSOBJ)/be.a:				\
	$(MESS_DRIVERS)/bebox.o		\
	$(MESS_MACHINE)/bebox.o		\
	$(MESS_VIDEO)/cirrus.o		\

$(MESSOBJ)/bnpo.a:				\
	$(MESS_DRIVERS)/b2m.o		\
	$(MESS_MACHINE)/b2m.o		\
	$(MESS_VIDEO)/b2m.o			\

$(MESSOBJ)/bondwell.a:			\
	$(MESS_DRIVERS)/bw2.o		\
	$(MESS_DRIVERS)/bw12.o		\

$(MESSOBJ)/booth.a:				\
	$(MESS_DRIVERS)/apexc.o		\

$(MESSOBJ)/camputers.a:			\
	$(MESS_DRIVERS)/camplynx.o	\

$(MESSOBJ)/canon.a:				\
	$(MESS_DRIVERS)/cat.o		\
	$(MESS_DRIVERS)/x07.o		\

$(MESSOBJ)/cantab.a:			\
	$(MESS_DRIVERS)/ace.o		\
	$(MESS_FORMATS)/ace_ace.o	\

$(MESSOBJ)/casio.a:				\
	$(MESS_DRIVERS)/casloopy.o	\
	$(MESS_DRIVERS)/cfx9850.o	\
	$(MESS_DRIVERS)/fp1100.o    \
	$(MESS_DRIVERS)/pv1000.o	\
	$(MESS_DRIVERS)/pv2000.o	\
	$(MESS_DRIVERS)/pb1000.o	\
	$(MESS_DRIVERS)/fp6000.o    \
	$(MESS_VIDEO)/hd44352.o		\

$(MESSOBJ)/cbm.a:				\
	$(MESS_VIDEO)/pet.o			\
	$(MESS_DRIVERS)/pet.o		\
	$(MESS_MACHINE)/pet.o		\
	$(MESS_DRIVERS)/c64.o		\
	$(MESS_MACHINE)/c64.o		\
	$(MESS_DRIVERS)/c64dtv.o	\
	$(MESS_MACHINE)/c64exp.o	\
	$(MESS_MACHINE)/c64user.o	\
	$(MESS_MACHINE)/c64_4cga.o	\
	$(MESS_MACHINE)/c64_4dxh.o	\
	$(MESS_MACHINE)/c64_4ksa.o	\
	$(MESS_MACHINE)/c64_4tba.o	\
	$(MESS_MACHINE)/c64_16kb.o	\
	$(MESS_MACHINE)/c64_bn1541.o	\
	$(MESS_MACHINE)/c64_comal80.o	\
	$(MESS_MACHINE)/c64_cpm.o	\
	$(MESS_MACHINE)/c64_currah_speech.o	\
	$(MESS_MACHINE)/c64_dela_ep256.o	\
	$(MESS_MACHINE)/c64_dela_ep64.o	\
	$(MESS_MACHINE)/c64_dela_ep7x8.o	\
	$(MESS_MACHINE)/c64_dinamic.o	\
	$(MESS_MACHINE)/c64_dqbb.o	\
	$(MESS_MACHINE)/c64_easy_calc_result.o	\
	$(MESS_MACHINE)/c64_easyflash.o	\
	$(MESS_MACHINE)/c64_epyx_fast_load.o	\
	$(MESS_MACHINE)/c64_exos.o	\
	$(MESS_MACHINE)/c64_final.o	\
	$(MESS_MACHINE)/c64_final3.o	\
	$(MESS_MACHINE)/c64_fun_play.o	\
	$(MESS_MACHINE)/c64_geocable.o	\
	$(MESS_MACHINE)/c64_georam.o	\
	$(MESS_MACHINE)/c64_ide64.o	\
	$(MESS_MACHINE)/c64_ieee488.o	\
	$(MESS_MACHINE)/c64_kingsoft.o	\
	$(MESS_MACHINE)/c64_mach5.o	\
	$(MESS_MACHINE)/c64_magic_desk.o	\
	$(MESS_MACHINE)/c64_magic_formel.o	\
	$(MESS_MACHINE)/c64_mikro_assembler.o	\
	$(MESS_MACHINE)/c64_multiscreen.o	\
	$(MESS_MACHINE)/c64_neoram.o	\
	$(MESS_MACHINE)/c64_ocean.o	\
	$(MESS_MACHINE)/c64_pagefox.o	\
	$(MESS_MACHINE)/c64_prophet64.o	\
	$(MESS_MACHINE)/c64_ps64.o	\
	$(MESS_MACHINE)/c64_reu.o	\
	$(MESS_MACHINE)/c64_rex.o	\
	$(MESS_MACHINE)/c64_rex_ep256.o	\
	$(MESS_MACHINE)/c64_ross.o	\
	$(MESS_MACHINE)/c64_sfx_sound_expander.o	\
	$(MESS_MACHINE)/c64_silverrock.o	\
	$(MESS_MACHINE)/c64_simons_basic.o	\
	$(MESS_MACHINE)/c64_stardos.o	\
	$(MESS_MACHINE)/c64_std.o	\
	$(MESS_MACHINE)/c64_structured_basic.o	\
	$(MESS_MACHINE)/c64_super_explode.o	\
	$(MESS_MACHINE)/c64_super_games.o	\
	$(MESS_MACHINE)/c64_sw8k.o	\
	$(MESS_MACHINE)/c64_system3.o	\
	$(MESS_MACHINE)/c64_tdos.o	\
	$(MESS_MACHINE)/c64_vw64.o	\
	$(MESS_MACHINE)/c64_warp_speed.o	\
	$(MESS_MACHINE)/c64_westermann.o	\
	$(MESS_MACHINE)/c64_xl80.o	\
	$(MESS_MACHINE)/c64_zaxxon.o	\
	$(MESS_MACHINE)/interpod.o	\
	$(MESS_DRIVERS)/vic10.o		\
	$(MESS_MACHINE)/vic10exp.o	\
	$(MESS_MACHINE)/vic10std.o	\
	$(MESS_DRIVERS)/vic20.o		\
	$(MESS_MACHINE)/vic20exp.o	\
	$(MESS_MACHINE)/vic20std.o	\
	$(MESS_MACHINE)/vic20user.o	\
	$(MESS_MACHINE)/vic20_megacart.o	\
	$(MESS_MACHINE)/vic1010.o	\
	$(MESS_MACHINE)/vic1110.o	\
	$(MESS_MACHINE)/vic1111.o	\
	$(MESS_MACHINE)/vic1112.o	\
	$(MESS_MACHINE)/vic1210.o	\
	$(MESS_AUDIO)/t6721.o		\
	$(MESS_AUDIO)/mos7360.o		\
	$(MESS_DRIVERS)/plus4.o		\
	$(MESS_MACHINE)/plus4exp.o	\
	$(MESS_MACHINE)/plus4user.o	\
	$(MESS_MACHINE)/plus4_sid.o	\
	$(MESS_MACHINE)/plus4_std.o	\
	$(MESS_MACHINE)/diag264_lb_iec.o	\
	$(MESS_MACHINE)/diag264_lb_tape.o	\
	$(MESS_MACHINE)/diag264_lb_user.o	\
	$(MESS_DRIVERS)/cbm2.o		\
	$(MESS_MACHINE)/cbm2exp.o	\
	$(MESS_MACHINE)/cbm2_std.o	\
	$(MESS_MACHINE)/cbm2_graphic.o	\
	$(MESS_DRIVERS)/c65.o		\
	$(MESS_MACHINE)/c65.o		\
	$(MESS_DRIVERS)/c128.o		\
	$(MESS_MACHINE)/c128.o		\
	$(MESS_MACHINE)/c128_comal80.o		\
	$(MESS_MACHINE)/cbmiec.o	\
	$(MESS_MACHINE)/c1541.o		\
	$(MESS_MACHINE)/c1551.o		\
	$(MESS_MACHINE)/c1571.o		\
	$(MESS_MACHINE)/c1581.o		\
	$(MESS_MACHINE)/c2031.o		\
	$(MESS_MACHINE)/c2040.o		\
	$(MESS_MACHINE)/c8280.o		\
	$(MESS_MACHINE)/d9060.o		\
	$(MESS_MACHINE)/d9060hd.o	\
	$(MESS_MACHINE)/softbox.o	\
	$(MESS_MACHINE)/serialbox.o	\
	$(MESS_MACHINE)/cmdhd.o	\
	$(MESS_MACHINE)/fd2000.o	\
	$(MESS_DRIVERS)/clcd.o		\
	$(MESS_MACHINE)/cbm.o		\
	$(MESS_MACHINE)/cbmipt.o	\
	$(MESS_MACHINE)/64h156.o	\
	$(MESS_MACHINE)/petcass.o	\
	$(MESS_MACHINE)/mos8722.o	\
	$(MESS_MACHINE)/mos8726.o	\
	$(MESS_MACHINE)/c2n.o		\
	$(MESS_VIDEO)/vdc8563.o		\
	$(MESS_VIDEO)/vic6567.o		\
	$(MESS_VIDEO)/vic4567.o		\
	$(MESS_VIDEO)/mos6566.o		\
	$(MESS_DRIVERS)/c900.o		\
	$(MESS_FORMATS)/cbm_snqk.o	\
	$(MESS_FORMATS)/cbm_crt.o	\

$(MESSOBJ)/cccp.a:				\
	$(MESS_DRIVERS)/argo.o		\
	$(MESS_DRIVERS)/cm1800.o	\
	$(MESS_VIDEO)/lviv.o		\
	$(MESS_DRIVERS)/lviv.o		\
	$(MESS_MACHINE)/lviv.o		\
	$(MESS_DRIVERS)/mikro80.o	\
	$(MESS_MACHINE)/mikro80.o	\
	$(MESS_VIDEO)/mikro80.o		\
	$(MESS_DRIVERS)/pk8000.o	\
	$(MESS_DRIVERS)/pk8020.o	\
	$(MESS_MACHINE)/pk8020.o	\
	$(MESS_VIDEO)/pk8020.o		\
	$(MESS_DRIVERS)/pyl601.o	\
	$(MESS_DRIVERS)/sm1800.o	\
	$(MESS_DRIVERS)/uknc.o		\
	$(MESS_DRIVERS)/ut88.o		\
	$(MESS_MACHINE)/ut88.o		\
	$(MESS_VIDEO)/ut88.o		\
	$(MESS_DRIVERS)/vector06.o	\
	$(MESS_MACHINE)/vector06.o	\
	$(MESS_VIDEO)/vector06.o	\

$(MESSOBJ)/cce.a:				\
	$(MESS_DRIVERS)/mc1000.o	\

$(MESSOBJ)/ccs.a:				\
	$(MESS_DRIVERS)/ccs2422.o	\
	$(MESS_DRIVERS)/ccs2810.o	\

$(MESSOBJ)/chromatics.a:		\
	$(MESS_DRIVERS)/cgc7900.o	\
	$(MESS_VIDEO)/cgc7900.o		\

$(MESSOBJ)/coleco.a:			\
	$(MESS_DRIVERS)/coleco.o	\
	$(MESS_MACHINE)/coleco.o	\
	$(MESS_DRIVERS)/adam.o		\

$(MESSOBJ)/cromemco.a:			\
	$(MESS_DRIVERS)/c10.o		\

$(MESSOBJ)/comx.a:				\
	$(MESS_DRIVERS)/comx35.o	\
	$(MESS_FORMATS)/comx35_comx.o	\
	$(MESS_MACHINE)/comxpl80.o	\
	$(MESS_VIDEO)/comx35.o		\
	$(MESS_MACHINE)/comxexp.o	\
	$(MESS_MACHINE)/comx_clm.o	\
	$(MESS_MACHINE)/comx_eb.o	\
	$(MESS_MACHINE)/comx_epr.o	\
	$(MESS_MACHINE)/comx_fd.o	\
	$(MESS_MACHINE)/comx_joy.o	\
	$(MESS_MACHINE)/comx_prn.o	\
	$(MESS_MACHINE)/comx_ram.o	\
	$(MESS_MACHINE)/comx_thm.o	\

$(MESSOBJ)/concept.a:			\
	$(MESS_DRIVERS)/concept.o   \
	$(MESS_MACHINE)/concept.o	\
	$(MESS_MACHINE)/corvushd.o	\

$(MESSOBJ)/conitec.a:			\
	$(MESS_DRIVERS)/prof80.o	\
	$(MESS_MACHINE)/ecbbus.o	\
	$(MESS_MACHINE)/ecb_grip.o	\
	$(MESS_DRIVERS)/prof180x.o	\

$(MESSOBJ)/cybiko.a:			\
	$(MESS_DRIVERS)/cybiko.o	\
	$(MESS_MACHINE)/cybiko.o	\

$(MESSOBJ)/dai.a:				\
	$(MESS_DRIVERS)/dai.o		\
	$(MESS_VIDEO)/dai.o			\
	$(MESS_AUDIO)/dai.o			\
	$(MESS_MACHINE)/tms5501.o	\
	$(MESS_MACHINE)/dai.o		\

$(MESSOBJ)/ddr.a:				\
	$(MESS_DRIVERS)/ac1.o		\
	$(MESS_MACHINE)/ac1.o		\
	$(MESS_VIDEO)/ac1.o			\
	$(MESS_DRIVERS)/bcs3.o		\
	$(MESS_DRIVERS)/c80.o		\
	$(MESS_DRIVERS)/huebler.o	\
	$(MESS_DRIVERS)/jtc.o		\
	$(MESS_MACHINE)/k7659kb.o	\
	$(MESS_DRIVERS)/kramermc.o	\
	$(MESS_MACHINE)/kramermc.o	\
	$(MESS_VIDEO)/kramermc.o	\
	$(MESS_DRIVERS)/llc.o		\
	$(MESS_VIDEO)/llc.o			\
	$(MESS_MACHINE)/llc.o		\
	$(MESS_DRIVERS)/nanos.o		\
	$(MESS_DRIVERS)/pcm.o		\
	$(MESS_DRIVERS)/vcs80.o		\

$(MESSOBJ)/dec.a:				\
	$(MESS_DRIVERS)/dct11em.o	\
	$(MESS_DRIVERS)/dectalk.o	\
	$(MESS_MACHINE)/rx01.o		\
	$(MESS_DRIVERS)/pdp11.o		\
	$(MESS_DRIVERS)/vax11.o		\
	$(MESS_DRIVERS)/rainbow.o	\
	$(MESS_DRIVERS)/vk100.o		\
	$(MESS_DRIVERS)/vt100.o		\
	$(MESS_DRIVERS)/vt220.o		\
	$(MESS_DRIVERS)/vt240.o		\
	$(MESS_DRIVERS)/vt320.o		\
	$(MESS_DRIVERS)/vt520.o		\
	$(MESS_VIDEO)/vtvideo.o		\
    $(MESS_MACHINE)/dec_lk201.o \

$(MESSOBJ)/dicksmth.a:			\
	$(MESS_DRIVERS)/super80.o	\
	$(MESS_MACHINE)/super80.o	\
	$(MESS_VIDEO)/super80.o		\

$(MESSOBJ)/dms.a:				\
	$(MESS_DRIVERS)/dms5000.o	\
	$(MESS_DRIVERS)/dms86.o		\
	$(MESS_DRIVERS)/zsbc3.o		\

$(MESSOBJ)/dragon.a:			\
	$(MESS_MACHINE)/dgn_beta.o	\
	$(MESS_VIDEO)/dgn_beta.o	\
	$(MESS_DRIVERS)/dgn_beta.o	\

$(MESSOBJ)/drc.a:				\
	$(MESS_DRIVERS)/zrt80.o		\

$(MESSOBJ)/eaca.a:				\
	$(MESS_DRIVERS)/cgenie.o	\
	$(MESS_VIDEO)/cgenie.o		\
	$(MESS_MACHINE)/cgenie.o	\

$(MESSOBJ)/einis.a:				\
	$(MESS_DRIVERS)/pecom.o		\
	$(MESS_MACHINE)/pecom.o		\
	$(MESS_VIDEO)/pecom.o		\

$(MESSOBJ)/elektrka.a:			\
	$(MESS_DRIVERS)/bk.o		\
	$(MESS_MACHINE)/bk.o		\
	$(MESS_VIDEO)/bk.o			\
	$(MESS_DRIVERS)/mk85.o		\
	$(MESS_DRIVERS)/mk90.o		\

$(MESSOBJ)/elektor.a:			\
	$(MESS_DRIVERS)/ec65.o		\
	$(MESS_DRIVERS)/elekscmp.o	\
	$(MESS_DRIVERS)/junior.o	\

$(MESSOBJ)/ensoniq.a:			\
	$(MESS_DRIVERS)/esq1.o		\
	$(MESS_DRIVERS)/mirage.o    \
	$(MESS_DRIVERS)/esq5505.o   \
    $(MESS_MACHINE)/esqvfd.o    \

$(MESSOBJ)/entex.a:				\
	$(MESS_VIDEO)/advision.o	\
	$(MESS_MACHINE)/advision.o	\
	$(MESS_DRIVERS)/advision.o	\

$(MESSOBJ)/epoch.a:				\
	$(MESS_DRIVERS)/gamepock.o	\
	$(MESS_MACHINE)/gamepock.o	\
	$(MESS_DRIVERS)/scv.o		\
	$(MESS_AUDIO)/upd1771.o		\

$(MESSOBJ)/epson.a:				\
	$(MESS_DRIVERS)/ex800.o		\
	$(MESS_DRIVERS)/hx20.o		\
	$(MESS_DRIVERS)/lx800.o		\
	$(MESS_MACHINE)/e05a03.o	\
	$(MESS_MACHINE)/pf10.o		\
	$(MESS_MACHINE)/tf20.o		\
	$(MESS_DRIVERS)/px4.o		\
	$(MESS_DRIVERS)/px8.o		\
	$(MESS_DRIVERS)/qx10.o		\

$(MESSOBJ)/exeltel.a:			\
	$(MESS_DRIVERS)/exelv.o		\

$(MESSOBJ)/exidy.a:				\
	$(MESS_MACHINE)/sorcerer.o	\
	$(MESS_DRIVERS)/sorcerer.o	\
	$(MESS_MACHINE)/micropolis.o	\

$(MESSOBJ)/fairch.a:			\
	$(MESS_VIDEO)/channelf.o	\
	$(MESS_AUDIO)/channelf.o	\
	$(MESS_DRIVERS)/channelf.o	\

$(MESSOBJ)/fujitsu.a:			\
	$(MESS_DRIVERS)/fmtowns.o	\
	$(MESS_VIDEO)/fmtowns.o		\
	$(MESS_MACHINE)/upd71071.o	\
	$(MESS_MACHINE)/fm_scsi.o	\
	$(MESS_DRIVERS)/fm7.o		\
	$(MESS_VIDEO)/fm7.o		\

$(MESSOBJ)/funtech.a:			\
	$(MESS_DRIVERS)/supracan.o	\

$(MESSOBJ)/galaxy.a:			\
	$(MESS_VIDEO)/galaxy.o		\
	$(MESS_DRIVERS)/galaxy.o	\
	$(MESS_MACHINE)/galaxy.o	\

$(MESSOBJ)/gamepark.a:			\
	$(MESS_DRIVERS)/gp32.o		\
	$(MESS_DRIVERS)/gp2x.o		\

$(MESSOBJ)/grundy.a:			\
	$(MESS_DRIVERS)/newbrain.o	\
	$(MESS_VIDEO)/newbrain.o	\

$(MESSOBJ)/hartung.a:			\
	$(MESS_DRIVERS)/gmaster.o	\
	$(MESS_AUDIO)/gmaster.o		\

$(MESSOBJ)/heathkit.a:			\
	$(MESS_DRIVERS)/et3400.o	\
	$(MESS_DRIVERS)/h8.o		\
	$(MESS_DRIVERS)/h19.o		\
	$(MESS_DRIVERS)/h89.o		\

$(MESSOBJ)/hegener.a:			\
	$(MESS_MACHINE)/mboard.o	\
	$(MESS_DRIVERS)/glasgow.o	\
	$(MESS_DRIVERS)/mephisto.o	\
        $(MESS_DRIVERS)/mmodular.o	\


$(MESSOBJ)/hitachi.a:			\
	$(MESS_DRIVERS)/bmjr.o		\
	$(MESS_DRIVERS)/bml3.o		\
	$(MESS_DRIVERS)/b16.o           \

$(MESSOBJ)/homebrew.a:			\
	$(MESS_DRIVERS)/4004clk.o	\
	$(MESS_DRIVERS)/68ksbc.o	\
	$(MESS_DRIVERS)/craft.o		\
	$(MESS_DRIVERS)/homez80.o	\
	$(MESS_DRIVERS)/p112.o		\
	$(MESS_DRIVERS)/phunsy.o	\
	$(MESS_DRIVERS)/pimps.o		\
	$(MESS_DRIVERS)/sbc6510.o	\
	$(MESS_DRIVERS)/z80dev.o	\
	$(MESS_DRIVERS)/uzebox.o	\

$(MESSOBJ)/homelab.a:			\
	$(MESS_DRIVERS)/homelab.o	\

$(MESSOBJ)/hp.a:				\
	$(MESS_MACHINE)/hp48.o		\
	$(MESS_VIDEO)/hp48.o		\
	$(MESS_DRIVERS)/hp48.o		\
	$(MESS_DRIVERS)/hp16500.o	\
	$(MESS_DRIVERS)/hp49gp.o	\
	$(MESS_DRIVERS)/hp9k.o	\

$(MESSOBJ)/hec2hrp.a:			\
	$(MESS_DRIVERS)/hec2hrp.o	\
	$(MESS_VIDEO)/hec2video.o	\
	$(MESS_MACHINE)/hecdisk2.o  \
	$(MESS_MACHINE)/hec2hrp.o   \
	$(MESS_DRIVERS)/interact.o	\

$(MESSOBJ)/intel.a:				\
	$(MESS_DRIVERS)/basic52.o	\
	$(MESS_DRIVERS)/isbc.o		\
	$(MESS_DRIVERS)/ipc.o		\
	$(MESS_DRIVERS)/ipds.o		\
	$(MESS_DRIVERS)/imds.o		\
	$(MESS_DRIVERS)/sdk85.o		\
	$(MESS_DRIVERS)/sdk86.o		\
	$(MESS_DRIVERS)/rex6000.o	\

$(MESSOBJ)/intelgnt.a:			\
	$(MESS_AUDIO)/dave.o		\
	$(MESS_VIDEO)/epnick.o		\
	$(MESS_DRIVERS)/enterp.o	\

$(MESSOBJ)/interton.a:			\
	$(MESS_AUDIO)/vc4000.o		\
	$(MESS_DRIVERS)/vc4000.o	\
	$(MESS_VIDEO)/vc4000.o		\

$(MESSOBJ)/intv.a:				\
	$(MESS_VIDEO)/intv.o		\
	$(MESS_VIDEO)/stic.o		\
	$(MESS_MACHINE)/intv.o		\
	$(MESS_DRIVERS)/intv.o		\

$(MESSOBJ)/kaypro.a:			\
	$(MESS_DRIVERS)/kaypro.o	\
	$(MESS_MACHINE)/kaypro.o	\
	$(MESS_VIDEO)/kaypro.o		\
	$(MESS_MACHINE)/kay_kbd.o	\

$(MESSOBJ)/koei.a:				\
	$(MESS_DRIVERS)/pasogo.o	\

$(MESSOBJ)/kyocera.a:			\
	$(MESS_DRIVERS)/kyocera.o	\
	$(MESS_VIDEO)/kyocera.o		\

$(MESSOBJ)/luxor.a:				\
	$(MESS_DRIVERS)/abc80.o		\
	$(MESS_VIDEO)/abc80.o		\
	$(MESS_MACHINE)/abc80kb.o	\
	$(MESS_DRIVERS)/abc80x.o	\
	$(MESS_VIDEO)/abc800.o		\
	$(MESS_MACHINE)/abc800kb.o	\
	$(MESS_VIDEO)/abc802.o		\
	$(MESS_VIDEO)/abc806.o		\
	$(MESS_MACHINE)/abcbus.o	\
	$(MESS_MACHINE)/abc_dos.o	\
	$(MESS_MACHINE)/abc_fd2.o	\
	$(MESS_MACHINE)/abc_hdc.o	\
	$(MESS_MACHINE)/abc_uni800.o	\
	$(MESS_MACHINE)/abc_sio.o	\
	$(MESS_MACHINE)/abc_slutprov.o	\
	$(MESS_MACHINE)/abc_turbo.o	\
	$(MESS_MACHINE)/abc_xebec.o	\
	$(MESS_MACHINE)/abc77.o		\
	$(MESS_MACHINE)/abc99.o		\
	$(MESS_MACHINE)/lux10828.o	\
	$(MESS_MACHINE)/lux21046.o	\
	$(MESS_MACHINE)/abc830.o	\
	$(MESS_MACHINE)/abc890.o	\
	$(MESS_DRIVERS)/abc1600.o	\
	$(MESS_MACHINE)/abc1600_bus.o	\
	$(MESS_MACHINE)/lux4105.o	\
	$(MESS_VIDEO)/abc1600.o		\
	$(MESS_MACHINE)/s1410.o		\

$(MESSOBJ)/magnavox.a:			\
	$(MESS_MACHINE)/odyssey2.o	\
	$(MESS_VIDEO)/odyssey2.o	\
	$(MESS_DRIVERS)/odyssey2.o	\

$(MESSOBJ)/mattel.a:			\
	$(MESS_DRIVERS)/aquarius.o	\
	$(MESS_VIDEO)/aquarius.o	\
	$(MESS_DRIVERS)/juicebox.o	\
	$(MESS_MACHINE)/s3c44b0.o	\

$(MESSOBJ)/matsushi.a:			\
	$(MESS_DRIVERS)/jr100.o		\
	$(MESS_DRIVERS)/jr200.o		\
	$(MESS_DRIVERS)/myb3k.o         \

$(MESSOBJ)/mchester.a:			\
	$(MESS_DRIVERS)/ssem.o		\

$(MESSOBJ)/memotech.a:			\
	$(MESS_DRIVERS)/mtx.o		\
	$(MESS_MACHINE)/mtx.o		\

$(MESSOBJ)/mgu.a:				\
	$(MESS_DRIVERS)/irisha.o	\
	$(MESS_MACHINE)/irisha.o	\
	$(MESS_VIDEO)/irisha.o		\

$(MESSOBJ)/microkey.a:			\
	$(MESS_DRIVERS)/primo.o		\
	$(MESS_MACHINE)/primo.o		\
	$(MESS_VIDEO)/primo.o		\

$(MESSOBJ)/mit.a:				\
	$(MESS_VIDEO)/crt.o			\
	$(MESS_DRIVERS)/tx0.o		\
	$(MESS_VIDEO)/tx0.o			\

$(MESSOBJ)/mits.a:				\
	$(MESS_DRIVERS)/altair.o	\
	$(MESS_DRIVERS)/mits680b.o	\

$(MESSOBJ)/mitsubishi.a:		\
	$(MESS_DRIVERS)/multi8.o	\
	$(MESS_DRIVERS)/multi16.o	\

$(MESSOBJ)/morrow.a:			\
	$(MESS_DRIVERS)/mpz80.o		\
	$(MESS_MACHINE)/s100_dj2db.o		\
	$(MESS_MACHINE)/s100_djdma.o		\
	$(MESS_MACHINE)/s100_mm65k16s.o		\
	$(MESS_MACHINE)/s100_wunderbus.o	\

$(MESSOBJ)/mos.a:				\
	$(MESS_DRIVERS)/kim1.o		\

$(MESSOBJ)/motorola.a:			\
	$(MESS_DRIVERS)/mekd2.o		\

$(MESSOBJ)/multitch.a:			\
	$(MESS_DRIVERS)/mpf1.o		\

$(MESSOBJ)/nakajima.a:			\
	$(MESS_DRIVERS)/nakajies.o	\

$(MESSOBJ)/nascom.a:			\
	$(MESS_VIDEO)/nascom1.o		\
	$(MESS_MACHINE)/nascom1.o	\
	$(MESS_DRIVERS)/nascom1.o	\

$(MESSOBJ)/ne.a:				\
	$(MESS_DRIVERS)/z80ne.o     \
	$(MESS_MACHINE)/z80ne.o     \

$(MESSOBJ)/nec.a:				\
	$(MESS_MACHINE)/pce.o		\
	$(MESS_DRIVERS)/pce.o		\
	$(MESS_DRIVERS)/pcfx.o		\
	$(MESS_DRIVERS)/pc6001.o	\
	$(MESS_DRIVERS)/pc8401a.o	\
	$(MESS_VIDEO)/pc8401a.o		\
	$(MESS_DRIVERS)/pc8001.o	\
	$(MESS_DRIVERS)/pc8801.o	\
	$(MESS_DRIVERS)/pc88va.o	\
	$(MESS_DRIVERS)/pc100.o		\
	$(MESS_DRIVERS)/pc9801.o	\
	$(MESS_DRIVERS)/tk80bs.o	\

$(MESSOBJ)/netronic.a:			\
	$(MESS_DRIVERS)/elf.o		\
	$(MESS_DRIVERS)/exp85.o		\

$(MESSOBJ)/next.a:				\
	$(MESS_DRIVERS)/next.o		\
	$(MESS_MACHINE)/nextkbd.o	\
	$(MESS_MACHINE)/nextmo.o	\

$(MESSOBJ)/nintendo.a:			\
	$(MESS_MACHINE)/nes_mmc.o	\
	$(MESS_VIDEO)/nes.o			\
	$(MESS_MACHINE)/nes.o		\
	$(MESS_DRIVERS)/nes.o		\
	$(MESS_MACHINE)/snescart.o	\
	$(MESS_DRIVERS)/snes.o		\
	$(MESS_DRIVERS)/n64.o		\
	$(MESS_AUDIO)/gb.o			\
	$(MESS_VIDEO)/gb.o			\
	$(MESS_MACHINE)/gb.o		\
	$(MESS_DRIVERS)/gb.o		\
	$(MESS_MACHINE)/pokemini.o	\
	$(MESS_DRIVERS)/pokemini.o	\
	$(MESS_DRIVERS)/vboy.o		\
    $(MESS_AUDIO)/vboy.o        \
	$(MESS_DRIVERS)/gba.o		\
	$(MESS_VIDEO)/gba.o			\

$(MESSOBJ)/nokia.a:				\
	$(MESS_DRIVERS)/mikromik.o	\
	$(MESS_VIDEO)/mikromik.o	\

$(MESSOBJ)/novag.a:				\
	$(MESS_DRIVERS)/mk1.o		\
	$(MESS_DRIVERS)/mk2.o		\
	$(MESS_VIDEO)/ssystem3.o	\
	$(MESS_DRIVERS)/ssystem3.o	\
	$(MESS_DRIVERS)/supercon.o	\

$(MESSOBJ)/olivetti.a:			\
	$(MESS_DRIVERS)/m20.o		\

$(MESSOBJ)/omnibyte.a:			\
	$(MESS_DRIVERS)/msbc1.o		\
	$(MESS_DRIVERS)/ob68k1a.o	\

$(MESSOBJ)/orion.a:				\
	$(MESS_DRIVERS)/orion.o		\
	$(MESS_MACHINE)/orion.o		\
	$(MESS_VIDEO)/orion.o		\

$(MESSOBJ)/osborne.a:			\
	$(MESS_DRIVERS)/osborne1.o	\
	$(MESS_MACHINE)/osborne1.o	\
	$(MESS_DRIVERS)/osbexec.o	\
	$(MESS_DRIVERS)/vixen.o		\

$(MESSOBJ)/osi.a:				\
	$(MESS_DRIVERS)/osi.o		\
	$(MESS_VIDEO)/osi.o			\

$(MESSOBJ)/palm.a:				\
	$(MESS_DRIVERS)/palm.o		\
	$(MESS_MACHINE)/mc68328.o	\
	$(MESS_VIDEO)/mc68328.o		\
	$(MESS_DRIVERS)/palmz22.o	\

$(MESSOBJ)/parker.a:			\
	$(MESS_DRIVERS)/stopthie.o	\

$(MESSOBJ)/pitronic.a:			\
	$(MESS_DRIVERS)/beta.o		\

$(MESSOBJ)/pc.a:				\
	$(MESS_VIDEO)/pc_aga.o		\
	$(MESS_MACHINE)/tandy1t.o	\
	$(MESS_MACHINE)/europc.o	\
	$(MESS_MACHINE)/pc.o		\
	$(MESS_DRIVERS)/pc.o		\
	$(MESS_DRIVERS)/genpc.o		\
	$(MESS_MACHINE)/genpc.o		\
	$(MESS_DRIVERS)/ibmpc.o		\
	$(MESS_VIDEO)/pc_t1t.o		\

$(MESSOBJ)/pcshare.a:			\
	$(MESS_MACHINE)/pc_turbo.o	\
	$(MESS_MACHINE)/pc_fdc.o	\
	$(MESS_MACHINE)/pc_joy.o	\
	$(MESS_MACHINE)/pc_keyboards.o \
	$(MESS_MACHINE)/kb_keytro.o	\
	$(MESS_MACHINE)/kb_msnat.o \
	$(MESS_MACHINE)/isa_adlib.o	\
	$(MESS_MACHINE)/ser_mouse.o	\
	$(MESS_MACHINE)/isa_com.o	\
	$(MESS_MACHINE)/isa_fdc.o	\
	$(MESS_MACHINE)/isa_finalchs.o	\
	$(MESS_MACHINE)/isa_gblaster.o	\
	$(MESS_MACHINE)/isa_gus.o	\
	$(MESS_MACHINE)/isa_hdc.o	\
	$(MESS_MACHINE)/isa_ibm_mfc.o	\
	$(MESS_MACHINE)/isa_mpu401.o	\
	$(MESS_MACHINE)/isa_sblaster.o	\
	$(MESS_MACHINE)/isa_stereo_fx.o	\
	$(MESS_MACHINE)/isa_ide.o	\
	$(MESS_MACHINE)/isa_ide_cd.o	\
	$(MESS_MACHINE)/isa_aha1542.o	\
	$(MESS_VIDEO)/isa_cga.o		\
	$(MESS_VIDEO)/isa_mda.o		\
	$(MESS_VIDEO)/crtc_ega.o	\
	$(MESS_VIDEO)/isa_ega.o		\
	$(MESS_VIDEO)/isa_vga.o		\
	$(MESS_VIDEO)/isa_vga_ati.o	\
	$(MESS_VIDEO)/isa_svga_tseng.o		\
	$(MESS_VIDEO)/isa_svga_s3.o	\
	$(MESS_VIDEO)/isa_svga_cirrus.o	\
	$(MESS_MACHINE)/i82371ab.o	\
	$(MESS_MACHINE)/i82371sb.o	\
	$(MESS_MACHINE)/i82439tx.o	\
	$(MESS_MACHINE)/northbridge.o \
	$(MESS_MACHINE)/southbridge.o \

$(MESSOBJ)/pdp1.a:				\
	$(MESS_VIDEO)/pdp1.o		\
	$(MESS_DRIVERS)/pdp1.o		\

$(MESSOBJ)/pel.a:				\
	$(MESS_DRIVERS)/galeb.o		\
	$(MESS_VIDEO)/galeb.o		\
	$(MESS_DRIVERS)/orao.o		\
	$(MESS_MACHINE)/orao.o		\
	$(MESS_VIDEO)/orao.o		\

$(MESSOBJ)/philips.a:			\
	$(MESS_VIDEO)/p2000m.o		\
	$(MESS_DRIVERS)/p2000t.o	\
	$(MESS_MACHINE)/p2000t.o	\
	$(MESS_DRIVERS)/vg5k.o		\
	$(MESS_VIDEO)/ef9345.o		\

$(MESSOBJ)/poly88.a:			\
	$(MESS_DRIVERS)/poly88.o	\
	$(MESS_MACHINE)/poly88.o	\
	$(MESS_VIDEO)/poly88.o		\

$(MESSOBJ)/psion.a:				\
	$(MESS_DRIVERS)/psion.o		\
	$(MESS_VIDEO)/psion.o		\
	$(MESS_MACHINE)/psion_pack.o	\

$(MESSOBJ)/radio.a:				\
	$(MESS_DRIVERS)/radio86.o	\
	$(MESS_MACHINE)/radio86.o	\
	$(MESS_VIDEO)/radio86.o		\
	$(MESS_DRIVERS)/apogee.o	\
	$(MESS_DRIVERS)/partner.o	\
	$(MESS_MACHINE)/partner.o	\
	$(MESS_DRIVERS)/mikrosha.o	\

$(MESSOBJ)/rca.a:				\
	$(MESS_DRIVERS)/studio2.o	\
	$(MESS_FORMATS)/studio2_st2.o	\
	$(MESS_DRIVERS)/vip.o		\
	$(MESS_MACHINE)/vip_byteio.o	\
	$(MESS_MACHINE)/vip_exp.o	\
	$(MESS_MACHINE)/vp550.o		\
	$(MESS_MACHINE)/vp570.o		\
	$(MESS_MACHINE)/vp575.o		\
	$(MESS_MACHINE)/vp585.o		\
	$(MESS_MACHINE)/vp590.o		\
	$(MESS_MACHINE)/vp595.o		\
	$(MESS_MACHINE)/vp620.o		\
	$(MESS_MACHINE)/vp700.o		\

$(MESSOBJ)/rm.a:				\
	$(MESS_DRIVERS)/rm380z.o	\
	$(MESS_MACHINE)/rm380z.o	\
	$(MESS_VIDEO)/rm380z.o		\
	$(MESS_DRIVERS)/rmnimbus.o	\
	$(MESS_MACHINE)/rmnimbus.o  \
	$(MESS_VIDEO)/rmnimbus.o    \
	$(MESS_MACHINE)/acb4070.o	\

$(MESSOBJ)/robotron.a:			\
	$(MESS_DRIVERS)/a5105.o		\
	$(MESS_DRIVERS)/a51xx.o		\
	$(MESS_DRIVERS)/a7150.o		\
	$(MESS_DRIVERS)/k1003.o		\
	$(MESS_DRIVERS)/k8915.o		\
	$(MESS_DRIVERS)/rt1715.o	\
	$(MESS_DRIVERS)/z1013.o		\
	$(MESS_DRIVERS)/z9001.o		\

$(MESSOBJ)/rockwell.a:			\
	$(MESS_MACHINE)/aim65.o		\
	$(MESS_DRIVERS)/aim65.o		\
	$(MESS_DRIVERS)/aim65_40.o	\

$(MESSOBJ)/samcoupe.a:			\
	$(MESS_VIDEO)/samcoupe.o	\
	$(MESS_DRIVERS)/samcoupe.o	\
	$(MESS_MACHINE)/samcoupe.o	\

$(MESSOBJ)/samsung.a:			\
	$(MESS_DRIVERS)/spc1000.o	\

$(MESSOBJ)/sanyo.a:				\
	$(MESS_DRIVERS)/phc25.o		\
	$(MESS_DRIVERS)/mbc55x.o	\
	$(MESS_MACHINE)/mbc55x.o	\
	$(MESS_VIDEO)/mbc55x.o  	\

$(MESSOBJ)/sega.a:				\
	$(MESS_DRIVERS)/sg1000.o	\
	$(MAME_MACHINE)/md_cart.o	\
	$(MESS_DRIVERS)/megadriv.o  \
	$(MESS_DRIVERS)/dccons.o	\
	$(MAME_MACHINE)/gdrom.o 	\
	$(MESS_MACHINE)/dccons.o	\
	$(MESS_MACHINE)/sms.o	\
	$(MESS_DRIVERS)/sms.o	\

$(MESSOBJ)/sgi.a:				\
	$(MESS_MACHINE)/sgi.o		\
	$(MESS_DRIVERS)/sgi_ip2.o	\
	$(MESS_DRIVERS)/sgi_ip6.o	\
	$(MESS_DRIVERS)/ip20.o		\
	$(MESS_DRIVERS)/ip22.o		\
	$(MESS_VIDEO)/newport.o		\

$(MESSOBJ)/sharp.a:				\
	$(MESS_VIDEO)/mz700.o		\
	$(MESS_DRIVERS)/mz700.o		\
	$(MESS_MACHINE)/lh5810.o	\
	$(MESS_DRIVERS)/pc1500.o	\
	$(MESS_DRIVERS)/pocketc.o	\
	$(MESS_VIDEO)/pc1401.o		\
	$(MESS_MACHINE)/pc1401.o	\
	$(MESS_VIDEO)/pc1403.o		\
	$(MESS_MACHINE)/pc1403.o	\
	$(MESS_VIDEO)/pc1350.o		\
	$(MESS_MACHINE)/pc1350.o	\
	$(MESS_VIDEO)/pc1251.o		\
	$(MESS_MACHINE)/pc1251.o	\
	$(MESS_VIDEO)/pocketc.o		\
	$(MESS_MACHINE)/mz700.o		\
	$(MESS_DRIVERS)/x68k.o		\
	$(MESS_VIDEO)/x68k.o		\
	$(MESS_MACHINE)/x68kexp.o	\
	$(MESS_MACHINE)/x68k_neptunex.o	\
	$(MESS_MACHINE)/x68k_scsiext.o	\
	$(MESS_MACHINE)/x68k_hdc.o	\
	$(MESS_MACHINE)/mb89352.o	\
	$(MESS_DRIVERS)/mz80.o		\
	$(MESS_VIDEO)/mz80.o		\
	$(MESS_MACHINE)/mz80.o		\
	$(MESS_DRIVERS)/mz2000.o	\
	$(MESS_DRIVERS)/x1.o		\
	$(MESS_MACHINE)/x1.o		\
	$(MESS_DRIVERS)/x1twin.o	\
	$(MESS_DRIVERS)/mz2500.o	\
	$(MESS_DRIVERS)/pce220.o	\
	$(MESS_MACHINE)/pce220_ser.o	\
	$(MESS_DRIVERS)/mz6500.o	\

$(MESSOBJ)/sinclair.a:			\
	$(MESS_VIDEO)/spectrum.o	\
	$(MESS_VIDEO)/timex.o		\
	$(MESS_VIDEO)/zx.o		\
	$(MESS_DRIVERS)/zx.o		\
	$(MESS_MACHINE)/zx.o		\
	$(MESS_DRIVERS)/spectrum.o	\
	$(MESS_DRIVERS)/spec128.o	\
	$(MESS_DRIVERS)/timex.o		\
	$(MESS_DRIVERS)/specpls3.o	\
	$(MESS_DRIVERS)/scorpion.o	\
	$(MESS_DRIVERS)/atm.o		\
	$(MESS_DRIVERS)/pentagon.o	\
	$(MESS_MACHINE)/beta.o		\
	$(MESS_FORMATS)/spec_snqk.o	\
	$(MESS_FORMATS)/timex_dck.o	\
	$(MESS_DRIVERS)/ql.o		\
	$(MESS_VIDEO)/zx8301.o		\
	$(MESS_MACHINE)/zx8302.o	\


$(MESSOBJ)/snk.a:				\
	$(MESS_DRIVERS)/ng_aes.o	\
	$(MAME_MACHINE)/neocrypt.o	\
	$(MAME_MACHINE)/neoprot.o	\
	$(MAME_MACHINE)/neoboot.o	\
	$(MAME_DRIVERS)/neogeo.o	\
	$(MESS_DRIVERS)/ngp.o		\
	$(MESS_VIDEO)/k1ge.o		\

$(MESSOBJ)/sony.a:				\
	$(MESS_DRIVERS)/psx.o		\
	$(MESS_MACHINE)/psxcd.o		\
	$(MESS_MACHINE)/psxcddrv.o	\
	$(MESS_MACHINE)/psxcard.o	\
	$(MESS_DRIVERS)/pockstat.o	\
	$(MESS_DRIVERS)/smc777.o	\

$(MESSOBJ)/sord.a:				\
	$(MESS_DRIVERS)/m5.o		\

$(MESSOBJ)/special.a:			\
	$(MESS_AUDIO)/special.o		\
	$(MESS_DRIVERS)/special.o	\
	$(MESS_MACHINE)/special.o	\
	$(MESS_VIDEO)/special.o		\

$(MESSOBJ)/sun.a:				\
	$(MESS_DRIVERS)/sun1.o		\
	$(MESS_DRIVERS)/sun2.o		\
	$(MESS_DRIVERS)/sun3.o		\
	$(MESS_DRIVERS)/sun4.o		\

$(MESSOBJ)/svi.a:				\
	$(MESS_MACHINE)/svi318.o	\
	$(MESS_DRIVERS)/svi318.o	\

$(MESSOBJ)/svision.a:			\
	$(MESS_DRIVERS)/svision.o	\
	$(MESS_AUDIO)/svision.o		\

$(MESSOBJ)/synertec.a:			\
	$(MESS_MACHINE)/sym1.o		\
	$(MESS_DRIVERS)/sym1.o		\

$(MESSOBJ)/tandberg.a:			\
	$(MESS_DRIVERS)/tdv2324.o	\

$(MESSOBJ)/tangerin.a:			\
	$(MESS_VIDEO)/microtan.o	\
	$(MESS_MACHINE)/microtan.o	\
	$(MESS_DRIVERS)/microtan.o	\
	$(MESS_FORMATS)/m65_snqk.o	\
	$(MESS_DRIVERS)/oric.o		\
	$(MESS_VIDEO)/oric.o		\
	$(MESS_MACHINE)/oric.o		\

$(MESSOBJ)/tatung.a:			\
	$(MESS_DRIVERS)/einstein.o	\
	$(MESS_MACHINE)/einstein.o	\

$(MESSOBJ)/teamconc.a:			\
	$(MESS_VIDEO)/comquest.o	\
	$(MESS_DRIVERS)/comquest.o	\

$(MESSOBJ)/tektroni.a:			\
	$(MESS_DRIVERS)/tek405x.o	\
	$(MESS_DRIVERS)/tek410x.o	\

$(MESSOBJ)/telenova.a:			\
	$(MESS_DRIVERS)/compis.o	\
	$(MESS_MACHINE)/compis.o	\
	$(MESS_MACHINE)/compiskb.o	\

$(MESSOBJ)/telercas.a:			\
	$(MESS_DRIVERS)/tmc1800.o	\
	$(MESS_VIDEO)/tmc1800.o		\
	$(MESS_DRIVERS)/tmc600.o	\
	$(MESS_VIDEO)/tmc600.o		\
	$(MESS_DRIVERS)/tmc2000e.o	\

$(MESSOBJ)/tem.a:			\
	$(MESS_DRIVERS)/tec1.o		\

$(MESSOBJ)/tesla.a:			\
	$(MESS_DRIVERS)/ondra.o		\
	$(MESS_MACHINE)/ondra.o		\
	$(MESS_VIDEO)/ondra.o		\
	$(MESS_VIDEO)/pmd85.o		\
	$(MESS_DRIVERS)/pmd85.o		\
	$(MESS_MACHINE)/pmd85.o		\
	$(MESS_DRIVERS)/pmi80.o		\
	$(MESS_DRIVERS)/sapi1.o		\
	$(MESS_MACHINE)/sapi1.o		\
	$(MESS_VIDEO)/sapi1.o		\

$(MESSOBJ)/thomson.a:			\
	$(MESS_DRIVERS)/thomson.o	\
	$(MESS_MACHINE)/thomson.o	\
	$(MESS_VIDEO)/thomson.o		\
	$(MESS_MACHINE)/thomflop.o	\

$(MESSOBJ)/ti.a:				\
	$(MESS_DRIVERS)/ti85.o		\
	$(MESS_VIDEO)/ti85.o		\
	$(MESS_VIDEO)/t6a04.o		\
	$(MESS_MACHINE)/ti85.o		\
	$(MESS_DRIVERS)/ti89.o		\
	$(MESS_MACHINE)/990_hd.o	\
	$(MESS_MACHINE)/990_tap.o	\
	$(MESS_MACHINE)/ti990.o		\
	$(MESS_MACHINE)/at29040a.o	\
	$(MESS_MACHINE)/ti99/datamux.o	\
	$(MESS_MACHINE)/ti99/videowrp.o \
	$(MESS_MACHINE)/ti99/grom.o	\
	$(MESS_MACHINE)/ti99/gromport.o  \
	$(MESS_MACHINE)/ti99/joyport.o  \
	$(MESS_MACHINE)/ti99/mecmouse.o  \
	$(MESS_MACHINE)/ti99/handset.o  \
	$(MESS_MACHINE)/ti99/peribox.o  \
	$(MESS_MACHINE)/ti99/ti32kmem.o  \
	$(MESS_MACHINE)/ti99/ti_fdc.o  \
	$(MESS_MACHINE)/ti99/bwg.o  \
	$(MESS_MACHINE)/ti99/hfdc.o  \
	$(MESS_MACHINE)/ti99/ti99_hd.o	\
	$(MESS_MACHINE)/ti99/p_code.o \
	$(MESS_MACHINE)/ti99/myarcmem.o \
	$(MESS_MACHINE)/ti99/samsmem.o \
	$(MESS_MACHINE)/ti99/tn_ide.o \
	$(MESS_MACHINE)/ti99/tn_usbsm.o \
	$(MESS_MACHINE)/ti99/evpc.o	\
	$(MESS_MACHINE)/ti99/hsgpl.o	\
	$(MESS_MACHINE)/ti99/ti_rs232.o	\
	$(MESS_MACHINE)/ti99/spchsyn.o	\
	$(MESS_MACHINE)/ti99/speech8.o	\
	$(MESS_MACHINE)/ti99/mapper8.o	\
	$(MESS_MACHINE)/ti99/genboard.o	\
	$(MESS_MACHINE)/ti99/memex.o	\
	$(MESS_MACHINE)/ti99/horizon.o	\
	$(MESS_MACHINE)/strata.o	\
	$(MESS_MACHINE)/990_dk.o	\
	$(MESS_DRIVERS)/ti990_4.o	\
	$(MESS_DRIVERS)/ti99_4x.o	\
	$(MESS_DRIVERS)/ti99_4p.o	\
	$(MESS_DRIVERS)/ti99_8.o	\
	$(MESS_DRIVERS)/geneve.o	\
	$(MESS_DRIVERS)/tm990189.o	\
	$(MESS_VIDEO)/911_vdt.o		\
	$(MESS_VIDEO)/733_asr.o		\
	$(MESS_DRIVERS)/ti990_10.o	\
	$(MESS_DRIVERS)/ti99_2.o	\
	$(MESS_VIDEO)/avigo.o		\
	$(MESS_DRIVERS)/avigo.o		\

$(MESSOBJ)/tiger.a:				\
	$(MESS_DRIVERS)/gamecom.o	\
	$(MESS_MACHINE)/gamecom.o	\
	$(MESS_VIDEO)/gamecom.o		\

$(MESSOBJ)/tigertel.a:			\
	$(MESS_DRIVERS)/gizmondo.o	\
	$(MESS_MACHINE)/docg3.o		\
	$(MESS_VIDEO)/gf4500.o		\

$(MESSOBJ)/tiki.a:				\
	$(MESS_DRIVERS)/tiki100.o	\

$(MESSOBJ)/tomy.a:				\
	$(MESS_DRIVERS)/tutor.o		\

$(MESSOBJ)/toshiba.a:			\
	$(MESS_DRIVERS)/pasopia.o	\
	$(MESS_DRIVERS)/pasopia7.o	\
	$(MESS_DRIVERS)/paso1600.o      \

$(MESSOBJ)/trs.a:				\
	$(MESS_MACHINE)/6883sam.o	\
	$(MESS_MACHINE)/ds1315.o	\
	$(MESS_MACHINE)/coco.o		\
	$(MESS_MACHINE)/coco12.o		\
	$(MESS_DRIVERS)/coco12.o		\
	$(MESS_MACHINE)/coco3.o		\
	$(MESS_DRIVERS)/coco3.o		\
	$(MESS_VIDEO)/gime.o			\
	$(MESS_MACHINE)/dragon.o		\
	$(MESS_DRIVERS)/dragon.o		\
	$(MESS_MACHINE)/dgnalpha.o		\
	$(MESS_MACHINE)/coco_vhd.o      \
	$(MESS_MACHINE)/cococart.o      \
	$(MESS_MACHINE)/coco_232.o      \
	$(MESS_MACHINE)/coco_orch90.o\
	$(MESS_MACHINE)/coco_pak.o      \
	$(MESS_MACHINE)/coco_fdc.o      \
	$(MESS_MACHINE)/coco_multi.o    \
	$(MESS_DRIVERS)/mc10.o		\
	$(MESS_MACHINE)/trs80.o		\
	$(MESS_VIDEO)/trs80.o		\
	$(MESS_FORMATS)/trs_cmd.o	\
	$(MESS_DRIVERS)/trs80.o		\
	$(MESS_DRIVERS)/trs80m2.o	\
	$(MESS_MACHINE)/trs80m2kb.o	\
	$(MESS_DRIVERS)/tandy2k.o	\
	$(MESS_MACHINE)/tandy2kb.o	\

$(MESSOBJ)/unisys.a:			\
	$(MESS_DRIVERS)/univac.o	\

$(MESSOBJ)/veb.a:				\
	$(MESS_DRIVERS)/chessmst.o	\
	$(MESS_VIDEO)/kc.o			\
	$(MESS_DRIVERS)/kc.o		\
	$(MESS_MACHINE)/kc.o		\
	$(MESS_MACHINE)/kc_keyb.o	\
	$(MESS_MACHINE)/kcexp.o		\
	$(MESS_MACHINE)/kc_ram.o	\
	$(MESS_MACHINE)/kc_rom.o	\
	$(MESS_MACHINE)/kc_d002.o	\
	$(MESS_MACHINE)/kc_d004.o	\
	$(MESS_DRIVERS)/lc80.o		\
	$(MESS_DRIVERS)/mc80.o		\
	$(MESS_VIDEO)/mc80.o		\
	$(MESS_MACHINE)/mc80.o		\
	$(MESS_DRIVERS)/poly880.o	\
	$(MESS_DRIVERS)/sc1.o		\
	$(MESS_DRIVERS)/sc2.o		\

$(MESSOBJ)/vidbrain.a:			\
	$(MESS_DRIVERS)/vidbrain.o	\
	$(MESS_MACHINE)/vidbrain_exp.o	\
	$(MESS_MACHINE)/vb_std.o	\
	$(MESS_MACHINE)/vb_money_minder.o	\
	$(MESS_MACHINE)/vb_timeshare.o	\
	$(MESS_VIDEO)/uv201.o		\

$(MESSOBJ)/videoton.a:			\
	$(MESS_DRIVERS)/tvc.o		\
	$(MESS_AUDIO)/tvc.o			\
	$(MESS_MACHINE)/tvcexp.o	\
	$(MESS_MACHINE)/tvc_hbf.o	\

$(MESSOBJ)/visual.a:			\
	$(MESS_DRIVERS)/v1050.o		\
	$(MESS_VIDEO)/v1050.o		\
	$(MESS_MACHINE)/v1050kb.o	\

$(MESSOBJ)/votrax.a:			\
	$(MESS_DRIVERS)/votrpss.o	\
	$(MESS_DRIVERS)/votrtnt.o	\

$(MESSOBJ)/vtech.a:				\
	$(MESS_DRIVERS)/lcmate2.o	\
	$(MESS_DRIVERS)/pc4.o		\
	$(MESS_VIDEO)/pc4.o			\
	$(MESS_DRIVERS)/vtech1.o	\
	$(MESS_VIDEO)/vtech2.o		\
	$(MESS_MACHINE)/vtech2.o	\
	$(MESS_DRIVERS)/vtech2.o	\
	$(MESS_DRIVERS)/crvision.o	\
	$(MESS_AUDIO)/socrates.o	\
	$(MESS_DRIVERS)/socrates.o	\
	$(MESS_DRIVERS)/pc2000.o	\
	$(MESS_DRIVERS)/prestige.o	\
	$(MESS_DRIVERS)/geniusiq.o	\

$(MESSOBJ)/wang.a:				\
	$(MESS_DRIVERS)/wangpc.o	\
	$(MESS_MACHINE)/wangpcbus.o	\
	$(MESS_MACHINE)/wangpckb.o	\
	$(MESS_MACHINE)/wangpc_emb.o	\
	$(MESS_MACHINE)/wangpc_lic.o	\
	$(MESS_MACHINE)/wangpc_lvc.o	\
	$(MESS_MACHINE)/wangpc_mcc.o	\
	$(MESS_MACHINE)/wangpc_mvc.o	\
	$(MESS_MACHINE)/wangpc_rtc.o	\
	$(MESS_MACHINE)/wangpc_tig.o	\
	$(MESS_MACHINE)/wangpc_wdc.o	\

$(MESSOBJ)/wavemate.a:			\
	$(MESS_DRIVERS)/bullet.o	\
	$(MESS_DRIVERS)/jupiter.o	\

$(MESSOBJ)/xerox.a:				\
	$(MESS_DRIVERS)/xerox820.o	\
	$(MESS_DRIVERS)/bigbord2.o	\

$(MESSOBJ)/zpa.a:				\
	$(MESS_DRIVERS)/iq151.o		\
	$(MESS_MACHINE)/iq151cart.o	\
	$(MESS_MACHINE)/iq151_rom.o	\
	$(MESS_MACHINE)/iq151_disc2.o	\
	$(MESS_MACHINE)/iq151_minigraf.o	\
	$(MESS_MACHINE)/iq151_ms151a.o	\
	$(MESS_MACHINE)/iq151_staper.o	\
	$(MESS_VIDEO)/iq151_grafik.o	\
	$(MESS_VIDEO)/iq151_video32.o	\
	$(MESS_VIDEO)/iq151_video64.o	\

$(MESSOBJ)/zvt.a:				\
	$(MESS_DRIVERS)/pp01.o		\
	$(MESS_MACHINE)/pp01.o		\
	$(MESS_VIDEO)/pp01.o		\

$(MESSOBJ)/test.a:				\
	$(MESS_DRIVERS)/test_t400.o	\
	$(MESS_DRIVERS)/zexall.o	\

$(MESSOBJ)/skeleton.a:			\
	$(MESS_DRIVERS)/alphasma.o	\
	$(MESS_DRIVERS)/alphatro.o	\
	$(MESS_DRIVERS)/amico2k.o	\
	$(MESS_DRIVERS)/applix.o	\
	$(MESS_DRIVERS)/ax20.o		\
	$(MESS_DRIVERS)/babbage.o	\
	$(MESS_DRIVERS)/beehive.o	\
	$(MESS_DRIVERS)/bob85.o		\
	$(MESS_DRIVERS)/busicom.o	\
	$(MESS_VIDEO)/busicom.o		\
	$(MESS_DRIVERS)/chaos.o 	\
	$(MESS_DRIVERS)/chesstrv.o	\
	$(MESS_DRIVERS)/cd2650.o	\
	$(MESS_DRIVERS)/codata.o	\
	$(MESS_DRIVERS)/cortex.o	\
	$(MESS_DRIVERS)/cosmicos.o	\
	$(MESS_DRIVERS)/cp1.o		\
	$(MESS_DRIVERS)/csc.o		\
	$(MESS_DRIVERS)/cvicny.o	\
	$(MESS_DRIVERS)/cxhumax.o	\
	$(MESS_DRIVERS)/czk80.o		\
	$(MESS_DRIVERS)/d6800.o		\
	$(MESS_DRIVERS)/d6809.o		\
	$(MESS_DRIVERS)/digel804.o	\
	$(MESS_DRIVERS)/dim68k.o	\
	$(MESS_DRIVERS)/dm7000.o	\
	$(MESS_DRIVERS)/dmv.o		\
	$(MESS_DRIVERS)/dolphunk.o	\
	$(MESS_DRIVERS)/dual68.o	\
	$(MESS_DRIVERS)/eacc.o		\
	$(MESS_DRIVERS)/elwro800.o	\
	$(MESS_DRIVERS)/eti660.o	\
	$(MESS_DRIVERS)/fk1.o		\
	$(MESS_DRIVERS)/fidelz80.o	\
	$(MESS_DRIVERS)/horizon.o	\
	$(MESS_DRIVERS)/hpz80unk.o	\
	$(MESS_DRIVERS)/ht68k.o		\
	$(MESS_DRIVERS)/if800.o		\
	$(MESS_DRIVERS)/indiana.o	\
	$(MESS_DRIVERS)/instruct.o	\
	$(MESS_DRIVERS)/itt3030.o	\
	$(MESS_DRIVERS)/konin.o		\
	$(MESS_DRIVERS)/m79152pc.o	\
	$(MESS_DRIVERS)/mbc200.o	\
	$(MESS_DRIVERS)/mccpm.o		\
	$(MESS_DRIVERS)/mes.o		\
	$(MESS_DRIVERS)/microdec.o	\
	$(MESS_DRIVERS)/micronic.o	\
	$(MESS_DRIVERS)/mini2440.o	\
	$(MESS_DRIVERS)/mk14.o		\
	$(MESS_DRIVERS)/mmd1.o		\
	$(MESS_DRIVERS)/mod8.o		\
	$(MESS_DRIVERS)/ms0515.o	\
	$(MESS_DRIVERS)/mstation.o	\
	$(MESS_DRIVERS)/mycom.o		\
	$(MESS_DRIVERS)/okean240.o	\
	$(MESS_DRIVERS)/p8k.o		\
	$(MESS_DRIVERS)/pegasus.o	\
	$(MESS_DRIVERS)/pes.o		\
	$(MESS_DRIVERS)/pipbug.o	\
	$(MESS_DRIVERS)/plan80.o	\
	$(MESS_DRIVERS)/poly.o		\
	$(MESS_DRIVERS)/pro80.o		\
	$(MESS_DRIVERS)/pt68k4.o	\
	$(MESS_DRIVERS)/ptcsol.o	\
	$(MESS_DRIVERS)/pv9234.o	\
	$(MESS_DRIVERS)/qtsbc.o		\
	$(MESS_DRIVERS)/rvoice.o	\
	$(MESS_DRIVERS)/sacstate.o	\
	$(MESS_DRIVERS)/sage2.o		\
	$(MESS_DRIVERS)/savia84.o	\
	$(MESS_DRIVERS)/selz80.o	\
	$(MESS_DRIVERS)/sitcom.o	\
	$(MESS_DRIVERS)/slc1.o		\
	$(MESS_DRIVERS)/super6.o	\
	$(MESS_DRIVERS)/swtpc.o		\
	$(MESS_DRIVERS)/sys2900.o	\
	$(MESS_DRIVERS)/systec.o	\
	$(MESS_DRIVERS)/terak.o		\
	$(MESS_DRIVERS)/tim011.o	\
	$(MESS_DRIVERS)/tricep.o	\
	$(MESS_DRIVERS)/tsispch.o	\
	$(MESS_DRIVERS)/unior.o		\
	$(MESS_DRIVERS)/unistar.o	\
	$(MESS_DRIVERS)/v6809.o		\
	$(MESS_DRIVERS)/vector4.o	\
	$(MESS_DRIVERS)/vii.o		\
	$(MESS_DRIVERS)/vta2000.o	\
	$(MESS_DRIVERS)/xor100.o	\
	$(MESS_DRIVERS)/z100.o		\




#-------------------------------------------------
# miscellaneous dependencies
#-------------------------------------------------

$(MAME_MACHINE)/snes.o: $(MAMESRC)/machine/snesobc1.c \
				$(MAMESRC)/machine/snescx4.c \
				$(MAMESRC)/machine/cx4ops.c \
				$(MAMESRC)/machine/cx4oam.c \
				$(MAMESRC)/machine/cx4fn.c \
				$(MAMESRC)/machine/cx4data.c \
				$(MAMESRC)/machine/snesrtc.c \
				$(MAMESRC)/machine/snessdd1.c \
				$(MAMESRC)/machine/snes7110.c \
				$(MAMESRC)/machine/snesbsx.c

$(MESS_VIDEO)/gba.o:		$(MESSSRC)/video/gbamode0.c \
				$(MESSSRC)/video/gbamode1.c \
				$(MESSSRC)/video/gbamode2.c \
				$(MESSSRC)/video/gbam345.c

$(MESS_MACHINE)/nes_mmc.o:	$(MESSSRC)/machine/nes_ines.c \
				$(MESSSRC)/machine/nes_pcb.c \
				$(MESSSRC)/machine/nes_unif.c \

$(MESS_AUDIO)/mac.o:		$(MESSSRC)/audio/mac.c \
				$(MESSSRC)/includes/mac.h $(MESSSRC)/machine/egret.h $(MESSSRC)/machine/cuda.h

$(MESS_VIDEO)/mac.o:		$(MESSSRC)/video/mac.c \
				$(MESSSRC)/includes/mac.h $(MESSSRC)/machine/egret.h $(MESSSRC)/machine/cuda.h

$(MESS_MACHINE)/mac.o:		$(MESSSRC)/machine/mac.c \
				$(MESSSRC)/includes/mac.h $(MESSSRC)/machine/egret.h $(MESSSRC)/machine/cuda.h

$(MESS_MACHINE)/macadb.o:	$(MESSSRC)/machine/macadb.c \
				$(MESSSRC)/includes/mac.h $(MESSSRC)/machine/egret.h $(MESSSRC)/machine/cuda.h

$(MESS_MACHINE)/macrtc.o:	$(MESSSRC)/machine/macrtc.c \
				$(MESSSRC)/includes/mac.h $(MESSSRC)/machine/egret.h $(MESSSRC)/machine/cuda.h

$(MESS_DRIVERS)/mac.o:		$(MESSSRC)/drivers/mac.c \
				$(MESSSRC)/includes/mac.h $(MESSSRC)/machine/egret.h $(MESSSRC)/machine/cuda.h

$(MESS_MACHINE)/egret.o:	$(MESSSRC)/machine/egret.c\
                        	$(MESSSRC)/machine/egret.h

$(MESS_DRIVERS)/apple2.o:   $(MESSSRC)/includes/apple2.h
$(MESS_MACHINE)/apple2.o:   $(MESSSRC)/includes/apple2.h
$(MESS_VIDEO)/apple2.o:     $(MESSSRC)/includes/apple2.h
$(MESS_DRIVERS)/apple2gs.o: $(MESSSRC)/includes/apple2.h $(MESSSRC)/includes/apple2gs.h
$(MESS_MACHINE)/apple2gs.o: $(MESSSRC)/includes/apple2.h $(MESSSRC)/includes/apple2gs.h
$(MESS_VIDEO)/apple2gs.o:   $(MESSSRC)/includes/apple2.h $(MESSSRC)/includes/apple2gs.h


# $(MESSSRC)/drivers/apollo.c includes m68kcpu.h and m68kcpu.h now includes m68kops.h
$(MESS_DRIVERS)/apollo.o:	$(EMUSRC)/cpu/m68000/m68kcpu.h
$(MESS_MACHINE)/apollo_dbg.o:	$(EMUSRC)/cpu/m68000/m68kcpu.h

# when we compile source files we need to include generated files from the OBJ directory
$(MESS_DRIVERS)/apollo.o:	$(MESSSRC)/drivers/apollo.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -I$(CPUOBJ)/m68000 -c $< -o $@

$(MESS_MACHINE)/apollo_dbg.o: $(MESSSRC)/machine/apollo_dbg.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -I$(CPUOBJ)/m68000 -c $< -o $@

#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(MESS_DRIVERS)/4004clk.o:	$(MESS_LAYOUT)/4004clk.lh
$(MESS_VIDEO)/abc1600.o:	$(MESS_LAYOUT)/abc1600.lh
$(MESS_DRIVERS)/acrnsys1.o:	$(MESS_LAYOUT)/acrnsys1.lh
$(MESS_DRIVERS)/aim65.o:	$(MESS_LAYOUT)/aim65.lh
$(MESS_DRIVERS)/aim65_40.o:	$(MESS_LAYOUT)/aim65_40.lh
$(MESS_DRIVERS)/alesis.o:	$(MESS_LAYOUT)/sr16.lh
$(MESS_DRIVERS)/amico2k.o:	$(MESS_LAYOUT)/amico2k.lh
$(MESS_VIDEO)/apollo.o:		$(MESS_LAYOUT)/apollo.lh
$(MESS_VIDEO)/apollo.o:		$(MESS_LAYOUT)/apollo_15i.lh
$(MESS_DRIVERS)/apollo.o:	$(MESS_LAYOUT)/apollo_dsp.lh
$(MESS_DRIVERS)/apricotp.o:	$(MESS_LAYOUT)/apricotp.lh
$(MESS_DRIVERS)/avigo.o:	$(MESS_LAYOUT)/avigo.lh
$(MESS_DRIVERS)/babbage.o:	$(MESS_LAYOUT)/babbage.lh
$(MESS_DRIVERS)/beta.o:		$(MESS_LAYOUT)/beta.lh
$(MESS_DRIVERS)/bob85.o:	$(MESS_LAYOUT)/bob85.lh
$(MAME_DRIVERS)/cdi.o:		$(MAME_LAYOUT)/cdi.lh
$(MESS_DRIVERS)/chessmst.o:	$(MESS_LAYOUT)/chessmst.lh
$(MESS_DRIVERS)/chesstrv.o:	$(MESS_LAYOUT)/chesstrv.lh \
							$(MESS_LAYOUT)/borisdpl.lh
$(MESS_DRIVERS)/cvicny.o:	$(MESS_LAYOUT)/cvicny.lh
$(MESS_DRIVERS)/coco.o:		$(MESS_LAYOUT)/coco3.lh
$(MESS_DRIVERS)/coco3.o:	$(MESS_LAYOUT)/coco3.lh
$(MESS_DRIVERS)/cosmicos.o:	$(MESS_LAYOUT)/cosmicos.lh
$(MESS_DRIVERS)/c80.o:		$(MESS_LAYOUT)/c80.lh
$(MESS_DRIVERS)/dectalk.o:	$(MESS_LAYOUT)/dectalk.lh
$(MESS_DRIVERS)/digel804.o:	$(MESS_LAYOUT)/digel804.lh
$(MESS_DRIVERS)/dmv.o:		$(MESS_LAYOUT)/dmv.lh
$(MESS_DRIVERS)/dolphunk.o:	$(MESS_LAYOUT)/dolphunk.lh
$(MESS_DRIVERS)/eacc.o:		$(MESS_LAYOUT)/eacc.lh
$(MESS_DRIVERS)/elf.o:		$(MESS_LAYOUT)/elf2.lh
$(MESS_DRIVERS)/elekscmp.o:	$(MESS_LAYOUT)/elekscmp.lh
$(MESS_MACHINE)/esqvfd.o:   $(MESS_LAYOUT)/esq2by40.lh \
                            $(MESS_LAYOUT)/esq1by22.lh
$(MESS_DRIVERS)/et3400.o:	$(MESS_LAYOUT)/et3400.lh
$(MESS_DRIVERS)/ex800.o:	$(MESS_LAYOUT)/ex800.lh
$(MESS_DRIVERS)/fidelz80.o:	$(MESS_LAYOUT)/fidelz80.lh \
							$(MESS_LAYOUT)/abc.lh \
							$(MESS_LAYOUT)/vsc.lh
$(MESS_DRIVERS)/glasgow.o:	$(MESS_LAYOUT)/glasgow.lh
$(MESS_DRIVERS)/h8.o:		$(MESS_LAYOUT)/h8.lh
$(MESS_DRIVERS)/instruct.o:	$(MESS_LAYOUT)/instruct.lh
$(MESS_DRIVERS)/k1003.o:	$(MESS_LAYOUT)/k1003.lh
$(MESS_DRIVERS)/kim1.o:		$(MESS_LAYOUT)/kim1.lh
$(MESS_DRIVERS)/junior.o:	$(MESS_LAYOUT)/junior.lh
$(MESS_DRIVERS)/lc80.o:		$(MESS_LAYOUT)/lc80.lh
$(MESS_DRIVERS)/llc.o:		$(MESS_LAYOUT)/llc1.lh
$(MESS_DRIVERS)/lynx.o:		$(MESS_LAYOUT)/lynx.lh
$(MESS_DRIVERS)/lx800.o:	$(MESS_LAYOUT)/lx800.lh
$(MESS_DRIVERS)/mac.o:		$(MESS_LAYOUT)/mac.lh
$(MAME_MACHINE)/megacd.o:	$(MAME_LAYOUT)/megacd.lh
$(MAME_MACHINE)/megadriv.o:	$(MAME_LAYOUT)/megacd.lh
$(MESS_DRIVERS)/megadriv.o:	$(MAME_LAYOUT)/megacd.lh
$(MESS_DRIVERS)/mekd2.o:	$(MESS_LAYOUT)/mekd2.lh
$(MESS_DRIVERS)/mephisto.o:	$(MESS_LAYOUT)/mephisto.lh
$(MESS_DRIVERS)/mikrolab.o:	$(MESS_LAYOUT)/mikrolab.lh
$(MESS_DRIVERS)/mk1.o:		$(MESS_LAYOUT)/mk1.lh
$(MESS_DRIVERS)/mk14.o:		$(MESS_LAYOUT)/mk14.lh
$(MESS_DRIVERS)/mk2.o:		$(MESS_LAYOUT)/mk2.lh
$(MESS_DRIVERS)/mmd1.o:		$(MESS_LAYOUT)/mmd1.lh \
							$(MESS_LAYOUT)/mmd2.lh
$(MESS_DRIVERS)/mpf1.o:		$(MESS_LAYOUT)/mpf1.lh \
							$(MESS_LAYOUT)/mpf1b.lh \
							$(MESS_LAYOUT)/mpf1p.lh
$(MESS_VIDEO)/newbrain.o:	$(MESS_LAYOUT)/newbrain.lh
$(MAME_DRIVERS)/neogeo.o:	$(MAME_LAYOUT)/neogeo.lh
$(MESS_DRIVERS)/pc1500.o:	$(MESS_LAYOUT)/pc1500.lh
$(MESS_VIDEO)/pc8401a.o:	$(MESS_LAYOUT)/pc8500.lh
$(MESS_DRIVERS)/pcw.o:		$(MESS_LAYOUT)/pcw.lh
$(MESS_DRIVERS)/pmi80.o:	$(MESS_LAYOUT)/pmi80.lh
$(MESS_DRIVERS)/poly880.o:	$(MESS_LAYOUT)/poly880.lh
$(MESS_DRIVERS)/pro80.o:	$(MESS_LAYOUT)/pro80.lh
$(MESS_DRIVERS)/px4.o:		$(MESS_LAYOUT)/px4.lh
$(MESS_DRIVERS)/px8.o:		$(MESS_LAYOUT)/px8.lh
$(MESS_DRIVERS)/savia84.o:	$(MESS_LAYOUT)/savia84.lh
$(MESS_DRIVERS)/sc1.o:		$(MESS_LAYOUT)/sc1.lh
$(MESS_DRIVERS)/sc2.o:		$(MESS_LAYOUT)/sc2.lh
$(MESS_DRIVERS)/sdk85.o:	$(MESS_LAYOUT)/sdk85.lh
$(MESS_DRIVERS)/sdk86.o:	$(MESS_LAYOUT)/sdk86.lh
$(MESS_DRIVERS)/selz80.o:	$(MESS_LAYOUT)/selz80.lh
$(MESS_DRIVERS)/sitcom.o:	$(MESS_LAYOUT)/sitcom.lh
$(MESS_DRIVERS)/slc1.o:		$(MESS_LAYOUT)/slc1.lh
$(MESS_DRIVERS)/sms.o:		$(MAME_LAYOUT)/sms1.lh
$(MESS_DRIVERS)/stopthie.o:	$(MESS_LAYOUT)/stopthie.lh
$(MESS_DRIVERS)/super80.o:	$(MESS_LAYOUT)/super80.lh
$(MESS_DRIVERS)/supercon.o:	$(MESS_LAYOUT)/supercon.lh
$(MESS_DRIVERS)/svision.o:	$(MESS_LAYOUT)/svision.lh
$(MESS_DRIVERS)/sym1.o:		$(MESS_LAYOUT)/sym1.lh
$(MESS_DRIVERS)/tec1.o:		$(MESS_LAYOUT)/tec1.lh
$(MESS_DRIVERS)/tk80bs.o:	$(MESS_LAYOUT)/tk80.lh
$(MESS_DRIVERS)/tm990189.o:	$(MESS_LAYOUT)/tm990189.lh $(MESS_LAYOUT)/tm990189v.lh
$(MESS_DRIVERS)/unixpc.o:	$(MESS_LAYOUT)/unixpc.lh
$(MESS_DRIVERS)/ut88.o:		$(MESS_LAYOUT)/ut88mini.lh
$(MESS_DRIVERS)/vboy.o:		$(MESS_LAYOUT)/vboy.lh
$(MESS_DRIVERS)/vcs80.o:	$(MESS_LAYOUT)/vcs80.lh
$(MESS_DRIVERS)/vidbrain.o:	$(MESS_LAYOUT)/vidbrain.lh
$(MESS_DRIVERS)/votrpss.o:	$(MESS_LAYOUT)/votrpss.lh
$(MESS_DRIVERS)/votrtnt.o:	$(MESS_LAYOUT)/votrtnt.lh
$(MESS_DRIVERS)/vk100.o:	$(MESS_LAYOUT)/vk100.lh
$(MESS_DRIVERS)/vt100.o:	$(MESS_LAYOUT)/vt100.lh
$(MESS_DRIVERS)/wswan.o:	$(MESS_LAYOUT)/wswan.lh
$(MESS_DRIVERS)/x68k.o:		$(MESS_LAYOUT)/x68000.lh
$(MESS_DRIVERS)/z80dev.o:	$(MESS_LAYOUT)/z80dev.lh
$(MESS_DRIVERS)/z80ne.o:	$(MESS_LAYOUT)/z80ne.lh   \
							$(MESS_LAYOUT)/z80net.lh  \
							$(MESS_LAYOUT)/z80netb.lh \
							$(MESS_LAYOUT)/z80netf.lh


#-------------------------------------------------
# MESS-specific tools
#-------------------------------------------------

include $(MESSSRC)/tools/tools.mak

