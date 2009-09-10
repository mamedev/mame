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
# Core sound types
#-------------------------------------------------

SOUNDDEFS += -DHAS_SAMPLES=$(if $(filter SAMPLES,$(SOUNDS)),1,0)

ifneq ($(filter SAMPLES,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/samples.o
endif



#-------------------------------------------------
# DACs
#-------------------------------------------------

SOUNDDEFS += -DHAS_DAC=$(if $(filter DAC,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_DMADAC=$(if $(filter DMADAC,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_SPEAKER=$(if $(filter SPEAKER,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_BEEP=$(if $(filter BEEP,$(SOUNDS)),1,0)

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

SOUNDDEFS += -DHAS_CDDA=$(if $(filter CDDA,$(SOUNDS)),1,0)

ifneq ($(filter CDDA,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cdda.o
endif



#-------------------------------------------------
# Discrete component audio
#-------------------------------------------------

SOUNDDEFS += -DHAS_DISCRETE=$(if $(filter DISCRETE,$(SOUNDS)),1,0)

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
# Atari custom sound chips
#-------------------------------------------------

SOUNDDEFS += -DHAS_POKEY=$(if $(filter POKEY,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_TIA=$(if $(filter TIA,$(SOUNDS)),1,0)

ifneq ($(filter POKEY,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/pokey.o
endif

ifneq ($(filter TIA,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tiasound.o $(SOUNDOBJ)/tiaintf.o
endif



#-------------------------------------------------
# Bally Astrocade sound system
#-------------------------------------------------

SOUNDDEFS += -DHAS_ASTROCADE=$(if $(filter ASTROCADE,$(SOUNDS)),1,0)

ifneq ($(filter ASTROCADE,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/astrocde.o
endif



#-------------------------------------------------
# CEM 3394 analog synthesizer chip
#-------------------------------------------------

SOUNDDEFS += -DHAS_CEM3394=$(if $(filter CEM3394,$(SOUNDS)),1,0)

ifneq ($(filter CEM3394,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cem3394.o
endif



#-------------------------------------------------
# Data East custom sound chips
#-------------------------------------------------

SOUNDDEFS += -DHAS_BSMT2000=$(if $(filter BSMT2000,$(SOUNDS)),1,0)

ifneq ($(filter BSMT2000,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/bsmt2000.o
endif



#-------------------------------------------------
# Ensoniq 5503 (Apple IIgs)
#-------------------------------------------------

SOUNDDEFS += -DHAS_ES5503=$(if $(filter ES5503,$(SOUNDS)),1,0)

ifneq ($(filter ES5503,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/es5503.o
endif



#-------------------------------------------------
# Ensoniq 5505/5506
#-------------------------------------------------

SOUNDDEFS += -DHAS_ES5505=$(if $(filter ES5505,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_ES5506=$(if $(filter ES5506,$(SOUNDS)),1,0)

ifneq ($(filter ES5505 ES5506,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/es5506.o
endif



#-------------------------------------------------
# Excellent Systems ADPCM sound chip
#-------------------------------------------------

SOUNDDEFS += -DHAS_ES8712=$(if $(filter ES8712,$(SOUNDS)),1,0)

ifneq ($(filter ES8712,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/es8712.o
endif



#-------------------------------------------------
# Gaelco custom sound chips
#-------------------------------------------------

SOUNDDEFS += -DHAS_GAELCO_CG1V=$(if $(filter GAELCO_CG1V,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_GAELCO_GAE1=$(if $(filter GAELCO_GAE1,$(SOUNDS)),1,0)

ifneq ($(filter GAELCO_CG1V,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/gaelco.o
endif

ifneq ($(filter GAELCO_GAE1,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/gaelco.o
endif



#-------------------------------------------------
# RCA CDP1863
#-------------------------------------------------

SOUNDDEFS += -DHAS_CDP1863=$(if $(filter CDP1863,$(SOUNDS)),1,0)

ifneq ($(filter CDP1863,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cdp1863.o
endif



#-------------------------------------------------
# RCA CDP1869
#-------------------------------------------------

SOUNDDEFS += -DHAS_CDP1869=$(if $(filter CDP1869,$(SOUNDS)),1,0)

ifneq ($(filter CDP1869,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cdp1869.o
endif



#-------------------------------------------------
# GI AY-8910
#-------------------------------------------------

SOUNDDEFS += -DHAS_AY8910=$(if $(filter AY8910,$(SOUNDS)),1,0)

ifneq ($(filter AY8910,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ay8910.o
endif



#-------------------------------------------------
# Harris HC55516 CVSD
#-------------------------------------------------

SOUNDDEFS += -DHAS_HC55516=$(if $(filter HC55516,$(SOUNDS)),1,0)

ifneq ($(filter HC55516,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/hc55516.o
endif



#-------------------------------------------------
# Hudsonsoft C6280 sound chip
#-------------------------------------------------

SOUNDDEFS += -DHAS_C6280=$(if $(filter C6280,$(SOUNDS)),1,0)

ifneq ($(filter C6280,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/c6280.o
endif



#-------------------------------------------------
# ICS2115 sound chip
#-------------------------------------------------

SOUNDDEFS += -DHAS_ICS2115=$(if $(filter ICS2115,$(SOUNDS)),1,0)

ifneq ($(filter ICS2115,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ics2115.o
endif



#-------------------------------------------------
# Irem custom sound chips
#-------------------------------------------------

SOUNDDEFS += -DHAS_IREMGA20=$(if $(filter IREMGA20,$(SOUNDS)),1,0)

ifneq ($(filter IREMGA20,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/iremga20.o
endif



#-------------------------------------------------
# Konami custom sound chips
#-------------------------------------------------

SOUNDDEFS += -DHAS_K005289=$(if $(filter K005289,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_K007232=$(if $(filter K007232,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_K051649=$(if $(filter K051649,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_K053260=$(if $(filter K053260,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_K054539=$(if $(filter K054539,$(SOUNDS)),1,0)

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



#-------------------------------------------------
# Namco custom sound chips
#-------------------------------------------------

SOUNDDEFS += -DHAS_NAMCO=$(if $(filter NAMCO,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_NAMCO_15XX=$(if $(filter NAMCO_15XX,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_NAMCO_CUS30=$(if $(filter NAMCO_CUS30,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_NAMCO_63701X=$(if $(filter NAMCO_63701X,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_C140=$(if $(filter C140,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_C352=$(if $(filter C352,$(SOUNDS)),1,0)

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

SOUNDDEFS += -DHAS_DIGITALKER=$(if $(filter DIGITALKER,$(SOUNDS)),1,0)

ifneq ($(filter DIGITALKER,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/digitalk.o
endif



#-------------------------------------------------
# Nintendo custom sound chips
#-------------------------------------------------

SOUNDDEFS += -DHAS_NES=$(if $(filter NES,$(SOUNDS)),1,0)

ifneq ($(filter NES,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/nes_apu.o
endif



#-------------------------------------------------
# NEC uPD7759 ADPCM sample player
#-------------------------------------------------

SOUNDDEFS += -DHAS_UPD7759=$(if $(filter UPD7759,$(SOUNDS)),1,0)

ifneq ($(filter UPD7759,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/upd7759.o
endif



#-------------------------------------------------
# OKI ADPCM sample players
#-------------------------------------------------

SOUNDDEFS += -DHAS_MSM5205=$(if $(filter MSM5205,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_MSM5232=$(if $(filter MSM5232,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_OKIM6376=$(if $(filter OKIM6376,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_OKIM6295=$(if $(filter OKIM6295,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_OKIM6258=$(if $(filter OKIM6258,$(SOUNDS)),1,0)

ifneq ($(filter MSM5205,$(SOUNDS)),)
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



#-------------------------------------------------
# Philips SAA1099
#-------------------------------------------------

SOUNDDEFS += -DHAS_SAA1099=$(if $(filter SAA1099,$(SOUNDS)),1,0)

ifneq ($(filter SAA1099,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/saa1099.o
endif



#-------------------------------------------------
# QSound sample player
#-------------------------------------------------

SOUNDDEFS += -DHAS_QSOUND=$(if $(filter QSOUND,$(SOUNDS)),1,0)

ifneq ($(filter QSOUND,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/qsound.o
endif



#-------------------------------------------------
# Ricoh sample players
#-------------------------------------------------

SOUNDDEFS += -DHAS_RF5C68=$(if $(filter RF5C68,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_RF5C400=$(if $(filter RF5C400,$(SOUNDS)),1,0)

ifneq ($(filter RF5C68,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/rf5c68.o
endif

ifneq ($(filter RF5C400,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/rf5c400.o
endif



#-------------------------------------------------
# Sega custom sound chips
#-------------------------------------------------

SOUNDDEFS += -DHAS_SEGAPCM=$(if $(filter SEGAPCM,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_MULTIPCM=$(if $(filter MULTIPCM,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_SCSP=$(if $(filter SCSP,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_AICA=$(if $(filter AICA,$(SOUNDS)),1,0)

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


#-------------------------------------------------
# Seta custom sound chips
#-------------------------------------------------

SOUNDDEFS += -DHAS_ST0016=$(if $(filter ST0016,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_NILE=$(if $(filter NILE,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_X1_010=$(if $(filter X1_010,$(SOUNDS)),1,0)

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

SOUNDDEFS += -DHAS_SID6581=$(if $(filter SID6581,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_SID8580=$(if $(filter SID8580,$(SOUNDS)),1,0)

ifneq ($(filter SID6581,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sid6581.o $(SOUNDOBJ)/sid.o $(SOUNDOBJ)/sidenvel.o $(SOUNDOBJ)/sidvoice.o
endif

ifneq ($(filter SID8580,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sid6581.o $(SOUNDOBJ)/sid.o $(SOUNDOBJ)/sidenvel.o $(SOUNDOBJ)/sidvoice.o
endif



#-------------------------------------------------
# SNK(?) custom stereo sn76489a clone
#-------------------------------------------------

SOUNDDEFS += -DHAS_T6W28=$(if $(filter T6W28,$(SOUNDS)),1,0)

ifneq ($(filter T6W28,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/t6w28.o
endif



#-------------------------------------------------
# SNK custom wave generator
#-------------------------------------------------

SOUNDDEFS += -DHAS_SNKWAVE=$(if $(filter SNKWAVE,$(SOUNDS)),1,0)

ifneq ($(filter SNKWAVE,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/snkwave.o
endif



#-------------------------------------------------
# Sony custom sound chips
#-------------------------------------------------

SOUNDDEFS += -DHAS_PSXSPU=$(if $(filter PSXSPU,$(SOUNDS)),1,0)

ifneq ($(filter PSXSPU,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/psx.o
endif



#-------------------------------------------------
# SP0256 speech synthesizer
#-------------------------------------------------

SOUNDDEFS += -DHAS_SP0256=$(if $(filter SP0256,$(SOUNDS)),1,0)

ifneq ($(filter SP0256,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sp0256.o
endif



#-------------------------------------------------
# SP0250 speech synthesizer
#-------------------------------------------------

SOUNDDEFS += -DHAS_SP0250=$(if $(filter SP0250,$(SOUNDS)),1,0)

ifneq ($(filter SP0250,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sp0250.o
endif


#-------------------------------------------------
# S14001A speech synthesizer
#-------------------------------------------------

SOUNDDEFS += -DHAS_S14001A=$(if $(filter S14001A,$(SOUNDS)),1,0)

ifneq ($(filter S14001A,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/s14001a.o
endif

#-------------------------------------------------
# Texas Instruments SN76477 analog chip
#-------------------------------------------------

SOUNDDEFS += -DHAS_SN76477=$(if $(filter SN76477,$(SOUNDS)),1,0)

ifneq ($(filter SN76477,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sn76477.o
endif



#-------------------------------------------------
# Texas Instruments SN76496
#-------------------------------------------------

SOUNDDEFS += -DHAS_SN76496=$(if $(filter SN76496,$(SOUNDS)),1,0)

ifneq ($(filter SN76496,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sn76496.o
endif



#-------------------------------------------------
# Texas Instruments TMS36xx doorbell chime
#-------------------------------------------------

SOUNDDEFS += -DHAS_TMS36XX=$(if $(filter TMS36XX,$(SOUNDS)),1,0)

ifneq ($(filter TMS36XX,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tms36xx.o
endif



#-------------------------------------------------
# Texas Instruments TMS3615 Octave Multiple Tone Synthesizer
#-------------------------------------------------

SOUNDDEFS += -DHAS_TMS3615=$(if $(filter TMS3615,$(SOUNDS)),1,0)

ifneq ($(filter TMS3615,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tms3615.o
endif



#-------------------------------------------------
# Texas Instruments TMS5110 speech synthesizers
#-------------------------------------------------

SOUNDDEFS += -DHAS_TMS5110=$(if $(filter TMS5110,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_TMS5220=$(if $(filter TMS5220,$(SOUNDS)),1,0)

ifneq ($(filter TMS5110,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tms5110.o
endif

ifneq ($(filter TMS5220,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tms5220.o
endif



#-------------------------------------------------
# VLM5030 speech synthesizer
#-------------------------------------------------

SOUNDDEFS += -DHAS_VLM5030=$(if $(filter VLM5030,$(SOUNDS)),1,0)

ifneq ($(filter VLM5030,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/vlm5030.o
endif



#-------------------------------------------------
# Votrax speech synthesizer
#-------------------------------------------------

SOUNDDEFS += -DHAS_VOTRAX=$(if $(filter VOTRAX,$(SOUNDS)),1,0)

ifneq ($(filter VOTRAX,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/votrax.o $(SOUNDOBJ)/samples.o
endif



#-------------------------------------------------
# VRender0 custom sound chip
#-------------------------------------------------

SOUNDDEFS += -DHAS_VRENDER0=$(if $(filter VRENDER0,$(SOUNDS)),1,0)

ifneq ($(filter VRENDER0,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/vrender0.o
endif



#-------------------------------------------------
# WAVE file (used for MESS cassette)
#-------------------------------------------------

SOUNDDEFS += -DHAS_WAVE=$(if $(filter WAVE,$(SOUNDS)),1,0)

ifneq ($(filter WAVE,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/wave.o
endif



#-------------------------------------------------
# Yamaha FM synthesizers
#-------------------------------------------------

SOUNDDEFS += -DHAS_YM2151=$(if $(filter YM2151,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_YM2203=$(if $(filter YM2203,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_YM2413=$(if $(filter YM2413,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_YM2608=$(if $(filter YM2608,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_YM2610=$(if $(filter YM2610,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_YM2610B=$(if $(filter YM2610B,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_YM2612=$(if $(filter YM2612,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_YM3438=$(if $(filter YM3438,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_YM3812=$(if $(filter YM3812,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_YM3526=$(if $(filter YM3526,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_Y8950=$(if $(filter Y8950,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_YMF262=$(if $(filter YMF262,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_YMF271=$(if $(filter YMF271,$(SOUNDS)),1,0)
SOUNDDEFS += -DHAS_YMF278B=$(if $(filter YMF278B,$(SOUNDS)),1,0)

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
SOUNDOBJS += $(SOUNDOBJ)/3812intf.o $(SOUNDOBJ)/fmopl.o
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

SOUNDDEFS += -DHAS_YMZ280B=$(if $(filter YMZ280B,$(SOUNDS)),1,0)

ifneq ($(filter YMZ280B,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ymz280b.o
endif
