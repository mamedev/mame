###########################################################################
#
#   sound.mak
#
#   Rules for building sound cores
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


SOUNDSRC = $(EMUSRC)/sound
SOUNDOBJ = $(EMUOBJ)/sound


#-------------------------------------------------
# Core sound types; samples always required
#-------------------------------------------------

SOUNDOBJS += $(SOUNDOBJ)/samples.o



#-------------------------------------------------
# DACs
#-------------------------------------------------

ifneq ($(filter DAC,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/dac.o
endif

ifneq ($(filter DMADAC,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/dmadac.o
endif

ifneq ($(filter SPEAKER,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/speaker.o
endif

ifneq ($(filter BEEP,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/beep.o
endif



#-------------------------------------------------
# CD audio
#-------------------------------------------------

ifneq ($(filter CDDA,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cdda.o
endif



#-------------------------------------------------
# Discrete component audio
#-------------------------------------------------

ifneq ($(filter DISCRETE,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/discrete.o
endif

$(SOUNDOBJ)/discrete.o:	$(SOUNDSRC)/discrete.c \
						$(SOUNDSRC)/discrete.h \
						$(SOUNDSRC)/disc_dev.c \
						$(SOUNDSRC)/disc_sys.c \
						$(SOUNDSRC)/disc_flt.c \
						$(SOUNDSRC)/disc_inp.c \
						$(SOUNDSRC)/disc_mth.c \
						$(SOUNDSRC)/disc_wav.c


#-------------------------------------------------
# Apple custom sound chips
#-------------------------------------------------

ifneq ($(filter ASC,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/asc.o
endif

ifneq ($(filter AWACS,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/awacs.o
endif


#-------------------------------------------------
# Atari custom sound chips
#-------------------------------------------------

ifneq ($(filter POKEY,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/pokey.o
endif

ifneq ($(filter TIA,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tiasound.o $(SOUNDOBJ)/tiaintf.o
endif



#-------------------------------------------------
# Bally Astrocade sound system
#-------------------------------------------------

ifneq ($(filter ASTROCADE,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/astrocde.o
endif



#-------------------------------------------------
# CEM 3394 analog synthesizer chip
#-------------------------------------------------

ifneq ($(filter CEM3394,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cem3394.o
endif



#-------------------------------------------------
# Data East custom sound chips
#-------------------------------------------------

ifneq ($(filter BSMT2000,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/bsmt2000.o
endif



#-------------------------------------------------
# Ensoniq 5503 (Apple IIgs)
#-------------------------------------------------

ifneq ($(filter ES5503,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/es5503.o
endif



#-------------------------------------------------
# Ensoniq 5505/5506
#-------------------------------------------------

ifneq ($(filter ES5505 ES5506,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/es5506.o
endif



#-------------------------------------------------
# Excellent Systems ADPCM sound chip
#-------------------------------------------------

ifneq ($(filter ES8712,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/es8712.o
endif



#-------------------------------------------------
# Gaelco custom sound chips
#-------------------------------------------------

ifneq ($(filter GAELCO_CG1V,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/gaelco.o
endif

ifneq ($(filter GAELCO_GAE1,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/gaelco.o
endif



#-------------------------------------------------
# RCA CDP1863
#-------------------------------------------------

ifneq ($(filter CDP1863,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cdp1863.o
endif



#-------------------------------------------------
# RCA CDP1864
#-------------------------------------------------

ifneq ($(filter CDP1864,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cdp1864.o
endif



#-------------------------------------------------
# RCA CDP1869
#-------------------------------------------------

ifneq ($(filter CDP1869,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cdp1869.o
endif



#-------------------------------------------------
# GI AY-8910
#-------------------------------------------------

ifneq ($(filter AY8910,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ay8910.o
endif



#-------------------------------------------------
# Harris HC55516 CVSD
#-------------------------------------------------

ifneq ($(filter HC55516,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/hc55516.o
endif



#-------------------------------------------------
# Hudsonsoft C6280 sound chip
#-------------------------------------------------

ifneq ($(filter C6280,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/c6280.o
endif



#-------------------------------------------------
# ICS2115 sound chip
#-------------------------------------------------

ifneq ($(filter ICS2115,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ics2115.o
endif



#-------------------------------------------------
# Imagetek I5000 sound
#-------------------------------------------------

ifneq ($(filter I5000_SND,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/i5000.o
endif



#-------------------------------------------------
# Irem custom sound chips
#-------------------------------------------------

ifneq ($(filter IREMGA20,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/iremga20.o
endif



#-------------------------------------------------
# Konami custom sound chips
#-------------------------------------------------

ifneq ($(filter K005289,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/k005289.o
endif

ifneq ($(filter K007232,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/k007232.o
endif

ifneq ($(filter K051649,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/k051649.o
endif

ifneq ($(filter K053260,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/k053260.o
endif

ifneq ($(filter K054539,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/k054539.o
endif

ifneq ($(filter K056800,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/k056800.o
endif



#-------------------------------------------------
# LMC1992 mixer chip
#-------------------------------------------------

ifneq ($(filter LMC1992,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/lmc1992.o
endif



#-------------------------------------------------
# MAS 3507D MPEG 1/2 Layer 2/3 Audio Decoder
#-------------------------------------------------

ifneq ($(filter MAS3507D,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/mas3507d.o
endif



#-------------------------------------------------
# MOS 6560VIC
#-------------------------------------------------

ifneq ($(filter MOS656X,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/mos6560.o
endif



#-------------------------------------------------
# Namco custom sound chips
#-------------------------------------------------

ifneq ($(filter NAMCO NAMCO_15XX NAMCO_CUS30,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/namco.o
endif

ifneq ($(filter NAMCO_63701X,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/n63701x.o
endif

ifneq ($(filter C140,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/c140.o
endif

ifneq ($(filter C352,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/c352.o
endif



#-------------------------------------------------
# National Semiconductor Digitalker
#-------------------------------------------------

ifneq ($(filter DIGITALKER,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/digitalk.o
endif



#-------------------------------------------------
# Nintendo custom sound chips
#-------------------------------------------------

ifneq ($(filter NES,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/nes_apu.o
endif



#-------------------------------------------------
# NEC uPD7759 ADPCM sample player
#-------------------------------------------------

ifneq ($(filter UPD7759,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/upd7759.o
endif



#-------------------------------------------------
# OKI ADPCM sample players
#-------------------------------------------------

ifneq ($(filter OKIM6258 OKIM6295 OKIM9810 I5000_SND,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/okiadpcm.o
endif

ifneq ($(filter MSM5205 MSM6585,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/msm5205.o
endif

ifneq ($(filter MSM5232,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/msm5232.o
endif

ifneq ($(filter OKIM6376,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/okim6376.o
endif

ifneq ($(filter OKIM6295,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/okim6295.o
endif

ifneq ($(filter OKIM6258,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/okim6258.o
endif

ifneq ($(filter OKIM9810,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/okim9810.o
endif



#-------------------------------------------------
# Philips SAA1099
#-------------------------------------------------

ifneq ($(filter SAA1099,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/saa1099.o
endif



#-------------------------------------------------
# AdMOS QS1000
#-------------------------------------------------

#ifneq ($(filter QS1000,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/qs1000.o
#endif



#-------------------------------------------------
# QSound sample player
#-------------------------------------------------

ifneq ($(filter QSOUND,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/qsound.o
endif



#-------------------------------------------------
# Ricoh sample players
#-------------------------------------------------

ifneq ($(filter RF5C68,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/rf5c68.o
endif

ifneq ($(filter RF5C400,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/rf5c400.o
endif



#-------------------------------------------------
# S2636 wave generator
#-------------------------------------------------

ifneq ($(filter S2636,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/s2636.o
endif



#-------------------------------------------------
# Sega custom sound chips
#-------------------------------------------------

ifneq ($(filter SEGAPCM,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/segapcm.o
endif

ifneq ($(filter MULTIPCM,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/multipcm.o
endif

ifneq ($(filter SCSP,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/scsp.o $(SOUNDOBJ)/scspdsp.o
endif

ifneq ($(filter AICA,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/aica.o $(SOUNDOBJ)/aicadsp.o
endif

$(SOUNDOBJ)/scsp.o:	$(SOUNDSRC)/scsplfo.c
$(SOUNDOBJ)/aica.o:	$(SOUNDSRC)/aicalfo.c



#-------------------------------------------------
# Seta custom sound chips
#-------------------------------------------------

ifneq ($(filter ST0016,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/st0016.o
endif

ifneq ($(filter NILE,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/nile.o
endif

ifneq ($(filter X1_010,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/x1_010.o
endif



#-------------------------------------------------
# SID custom sound chips
#-------------------------------------------------

ifneq ($(filter SID6581,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sid6581.o $(SOUNDOBJ)/sid.o $(SOUNDOBJ)/sidenvel.o $(SOUNDOBJ)/sidvoice.o
endif

ifneq ($(filter SID8580,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sid6581.o $(SOUNDOBJ)/sid.o $(SOUNDOBJ)/sidenvel.o $(SOUNDOBJ)/sidvoice.o
endif



#-------------------------------------------------
# SNK(?) custom stereo sn76489a clone
#-------------------------------------------------

ifneq ($(filter T6W28,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/t6w28.o
endif



#-------------------------------------------------
# SNK custom wave generator
#-------------------------------------------------

ifneq ($(filter SNKWAVE,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/snkwave.o
endif



#-------------------------------------------------
# Sony custom sound chips
#-------------------------------------------------

ifneq ($(filter SPU,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/spu.o $(SOUNDOBJ)/spu_tables.o $(SOUNDOBJ)/spureverb.o
endif


#-------------------------------------------------
# SP0256 speech synthesizer
#-------------------------------------------------

ifneq ($(filter SP0256,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sp0256.o
endif



#-------------------------------------------------
# SP0250 speech synthesizer
#-------------------------------------------------

ifneq ($(filter SP0250,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sp0250.o
endif


#-------------------------------------------------
# S14001A speech synthesizer
#-------------------------------------------------

ifneq ($(filter S14001A,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/s14001a.o
endif



#-------------------------------------------------
# Texas Instruments SN76477 analog chip
#-------------------------------------------------

ifneq ($(filter SN76477,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sn76477.o
endif



#-------------------------------------------------
# Texas Instruments SN76496
#-------------------------------------------------

ifneq ($(filter SN76496,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sn76496.o
endif



#-------------------------------------------------
# Texas Instruments TMS36xx doorbell chime
#-------------------------------------------------

ifneq ($(filter TMS36XX,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tms36xx.o
endif



#-------------------------------------------------
# Texas Instruments TMS3615 Octave Multiple Tone Synthesizer
#-------------------------------------------------

ifneq ($(filter TMS3615,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tms3615.o
endif



#-------------------------------------------------
# Texas Instruments TMS5110 speech synthesizers
#-------------------------------------------------

ifneq ($(filter TMS5110,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tms5110.o
endif

ifneq ($(filter TMS5220,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tms5220.o
endif

$(SOUNDOBJ)/tms5110.o:	$(SOUNDSRC)/tms5110r.c
$(SOUNDOBJ)/tms5220.o:	$(SOUNDSRC)/tms5220r.c



#-------------------------------------------------
# Toshiba TC8830F sample player/recorder
#-------------------------------------------------

ifneq ($(filter TC8830F,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tc8830f.o
endif



#-------------------------------------------------
# VLM5030 speech synthesizer
#-------------------------------------------------

ifneq ($(filter VLM5030,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/vlm5030.o
endif



#-------------------------------------------------
# Votrax speech synthesizer
#-------------------------------------------------

ifneq ($(filter VOTRAX,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/votrax.o $(SOUNDOBJ)/samples.o
endif



#-------------------------------------------------
# VRender0 custom sound chip
#-------------------------------------------------

ifneq ($(filter VRENDER0,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/vrender0.o
endif



#-------------------------------------------------
# WAVE file (used for MESS cassette)
#-------------------------------------------------

ifneq ($(filter WAVE,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/wave.o
endif



#-------------------------------------------------
# Yamaha FM synthesizers
#-------------------------------------------------

ifneq ($(filter YM2151,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/2151intf.o $(SOUNDOBJ)/ym2151.o
endif

ifneq ($(filter YM2203,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/2203intf.o $(SOUNDOBJ)/ay8910.o $(SOUNDOBJ)/fm.o
endif

ifneq ($(filter YM2413,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/2413intf.o $(SOUNDOBJ)/ym2413.o
endif

ifneq ($(filter YM2608,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/2608intf.o $(SOUNDOBJ)/ay8910.o $(SOUNDOBJ)/fm.o $(SOUNDOBJ)/ymdeltat.o
endif

ifneq ($(filter YM2610 YM2610B,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/2610intf.o $(SOUNDOBJ)/ay8910.o $(SOUNDOBJ)/fm.o $(SOUNDOBJ)/ymdeltat.o
endif

ifneq ($(filter YM2612 YM3438,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/2612intf.o $(SOUNDOBJ)/ay8910.o $(SOUNDOBJ)/fm2612.o
endif

ifneq ($(filter YM3812,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/3812intf.o $(SOUNDOBJ)/fmopl.o $(SOUNDOBJ)/ymdeltat.o
endif

ifneq ($(filter YM3526,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/3526intf.o $(SOUNDOBJ)/fmopl.o
endif

ifneq ($(filter Y8950,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/8950intf.o $(SOUNDOBJ)/fmopl.o $(SOUNDOBJ)/ymdeltat.o
endif

ifneq ($(filter YMF262,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ymf262.o $(SOUNDOBJ)/262intf.o
endif

ifneq ($(filter YMF271,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ymf271.o
endif

ifneq ($(filter YMF278B,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ymf278b.o
endif



#-------------------------------------------------
# Yamaha YMZ280B ADPCM
#-------------------------------------------------

ifneq ($(filter YMZ280B,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ymz280b.o
endif

#-------------------------------------------------
# Yamaha YMZ770 AMM
#-------------------------------------------------

ifneq ($(filter YMZ770,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ymz770.o
endif

#-------------------------------------------------
# ZOOM ZSG-2
#-------------------------------------------------

ifneq ($(filter ZSG2,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/zsg2.o
endif

