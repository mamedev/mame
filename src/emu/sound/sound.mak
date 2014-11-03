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
# DACs
#@src/emu/sound/dac.h,SOUNDS += DAC
#@src/emu/sound/dmadac.h,SOUNDS += DMADAC
#@src/emu/sound/speaker.h,SOUNDS += SPEAKER
#@src/emu/sound/beep.h,SOUNDS += BEEP
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
#@src/emu/sound/cdda.h,SOUNDS += CDDA
#-------------------------------------------------

ifneq ($(filter CDDA,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cdda.o
endif



#-------------------------------------------------
# Discrete component audio
#@src/emu/sound/discrete.h,SOUNDS += DISCRETE
#-------------------------------------------------

ifneq ($(filter DISCRETE,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/discrete.o
endif

$(SOUNDOBJ)/discrete.o: $(SOUNDSRC)/discrete.c \
						$(SOUNDSRC)/discrete.h \
						$(SOUNDSRC)/disc_dev.inc \
						$(SOUNDSRC)/disc_sys.inc \
						$(SOUNDSRC)/disc_flt.inc \
						$(SOUNDSRC)/disc_inp.inc \
						$(SOUNDSRC)/disc_mth.inc \
						$(SOUNDSRC)/disc_wav.inc


#-------------------------------------------------
# AC97
#@src/emu/sound/pic-ac97.h,SOUNDS += AC97
#-------------------------------------------------

ifneq ($(filter AC97,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/pci-ac97.o
endif



#-------------------------------------------------
# Apple custom sound chips
#@src/emu/sound/asc.h,SOUNDS += ASC
#@src/emu/sound/awacs.h,SOUNDS += AWACS
#-------------------------------------------------

ifneq ($(filter ASC,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/asc.o
endif

ifneq ($(filter AWACS,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/awacs.o
endif


#-------------------------------------------------
# Atari custom sound chips
#@src/emu/sound/pokey.h,SOUNDS += POKEY
#@src/emu/sound/tiaintf.h,SOUNDS += TIA
#-------------------------------------------------

ifneq ($(filter POKEY,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/pokey.o
endif

ifneq ($(filter TIA,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tiasound.o $(SOUNDOBJ)/tiaintf.o
endif



#-------------------------------------------------
# Amiga audio hardware
#@src/emu/sound/amiga.h,SOUNDS += AMIGA
#-------------------------------------------------

ifneq ($(filter AMIGA,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/amiga.o
endif



#-------------------------------------------------
# Bally Astrocade sound system
#@src/emu/sound/astrocde.h,SOUNDS += ASTROCADE
#-------------------------------------------------

ifneq ($(filter ASTROCADE,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/astrocde.o
endif



#-------------------------------------------------
#-------------------------------------------------
# AC97
#@src/emu/sound/pic-ac97.h,SOUNDS += AC97
#-------------------------------------------------

ifneq ($(filter AC97,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/pci-ac97.o
endif
# CEM 3394 analog synthesizer chip
#@src/emu/sound/cem3394.h,SOUNDS += CEM3394
#-------------------------------------------------

ifneq ($(filter CEM3394,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cem3394.o
endif



#-------------------------------------------------
# Creative Labs SB0400 Audigy2 Value
#@src/emu/sound/sb0400.h,SOUNDS += AC97
#-------------------------------------------------

ifneq ($(filter SB0400,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sb0400.o
endif



#-------------------------------------------------
# Data East custom sound chips
#@src/emu/sound/bsmt2000.h,SOUNDS += BSMT2000
#-------------------------------------------------

ifneq ($(filter BSMT2000,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/bsmt2000.o
endif



#-------------------------------------------------
# Ensoniq 5503 (Apple IIgs)
#@src/emu/sound/es5503.h,SOUNDS += ES5503
#-------------------------------------------------

ifneq ($(filter ES5503,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/es5503.o
endif



#-------------------------------------------------
# Ensoniq 5505/5506
#@src/emu/sound/es5506.h,SOUNDS += ES5505
#-------------------------------------------------

ifneq ($(filter ES5505 ES5506,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/es5506.o
endif


#-------------------------------------------------
# Ensoniq "pump" device, interfaces 5505/5506 with 5510
#@src/emu/sound/esqpump.h,SOUNDS += ESQPUMP
#-------------------------------------------------

ifneq ($(filter ESQPUMP,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/esqpump.o
endif


#-------------------------------------------------
# Excellent Systems ADPCM sound chip
#@src/emu/sound/es8712.h,SOUNDS += ES8712
#-------------------------------------------------

ifneq ($(filter ES8712,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/es8712.o
endif



#-------------------------------------------------
# Gaelco custom sound chips
#@src/emu/sound/gaelco.h,SOUNDS += GAELCO_CG1V
#-------------------------------------------------

ifneq ($(filter GAELCO_CG1V GAELCO_GAE1,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/gaelco.o
endif


#-------------------------------------------------
# RCA CDP1863
#@src/emu/sound/cdp1863.h,SOUNDS += CDP1863
#-------------------------------------------------

ifneq ($(filter CDP1863,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cdp1863.o
endif



#-------------------------------------------------
# RCA CDP1864
#@src/emu/sound/cdp1864.h,SOUNDS += CDP1864
#-------------------------------------------------

ifneq ($(filter CDP1864,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cdp1864.o
endif



#-------------------------------------------------
# RCA CDP1869
#@src/emu/sound/cdp1869.h,SOUNDS += CDP1869
#-------------------------------------------------

ifneq ($(filter CDP1869,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/cdp1869.o
endif



#-------------------------------------------------
# GI AY-8910
#@src/emu/sound/ay8910.h,SOUNDS += AY8910
#-------------------------------------------------

ifneq ($(filter AY8910,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ay8910.o
endif



#-------------------------------------------------
# Harris HC55516 CVSD
#@src/emu/sound/hc55516.h,SOUNDS += HC55516
#-------------------------------------------------

ifneq ($(filter HC55516,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/hc55516.o
endif



#-------------------------------------------------
# Hudsonsoft C6280 sound chip
#@src/emu/sound/c6280.h,SOUNDS += C6280
#-------------------------------------------------

ifneq ($(filter C6280,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/c6280.o
endif



#-------------------------------------------------
# ICS2115 sound chip
#@src/emu/sound/ics2115.h,SOUNDS += ICS2115
#-------------------------------------------------

ifneq ($(filter ICS2115,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ics2115.o
endif



#-------------------------------------------------
# Imagetek I5000 sound
#@src/emu/sound/i5000.h,SOUNDS += I5000_SND
#-------------------------------------------------

ifneq ($(filter I5000_SND,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/i5000.o
endif



#-------------------------------------------------
# Irem custom sound chips
#@src/emu/sound/iremga20.h,SOUNDS += IREMGA20
#-------------------------------------------------

ifneq ($(filter IREMGA20,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/iremga20.o
endif



#-------------------------------------------------
# Konami custom sound chips
#@src/emu/sound/k005289.h,SOUNDS += K005289
#@src/emu/sound/k007232.h,SOUNDS += K007232
#@src/emu/sound/k051649.h,SOUNDS += K051649
#@src/emu/sound/k053260.h,SOUNDS += K053260
#@src/emu/sound/k054539.h,SOUNDS += K054539
#@src/emu/sound/k056800.h,SOUNDS += K056800
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
#@src/emu/sound/lmc1992.h,SOUNDS += LMC1992
#-------------------------------------------------

ifneq ($(filter LMC1992,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/lmc1992.o
endif



#-------------------------------------------------
# MAS 3507D MPEG 1/2 Layer 2/3 Audio Decoder
#@src/emu/sound/mas3507d.h,SOUNDS += MAS3507D
#-------------------------------------------------

ifneq ($(filter MAS3507D,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/mas3507d.o
endif



#-------------------------------------------------
# MOS 6560VIC
#@src/emu/sound/mos6560.h,SOUNDS += MOS656X
#-------------------------------------------------

ifneq ($(filter MOS656X,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/mos6560.o
endif



#-------------------------------------------------
# MOS 7360 TED
#@src/emu/sound/mos7360.h,SOUNDS += MOS7360
#-------------------------------------------------

ifneq ($(filter MOS7360,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/mos7360.o
endif



#-------------------------------------------------
# Namco custom sound chips
#@src/emu/sound/namco.h,SOUNDS += NAMCO
#@src/emu/sound/n63701x.h,SOUNDS += NAMCO_63701X
#@src/emu/sound/c140.h,SOUNDS += C140
#@src/emu/sound/c352.h,SOUNDS += C352
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
#@src/emu/sound/digitalk.h,SOUNDS += DIGITALKER
#-------------------------------------------------

ifneq ($(filter DIGITALKER,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/digitalk.o
endif



#-------------------------------------------------
# Nintendo custom sound chips
#@src/emu/sound/nes_apu.h,SOUNDS += NES_APU
#-------------------------------------------------

ifneq ($(filter NES_APU,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/nes_apu.o
endif



#-------------------------------------------------
# NEC uPD7759 ADPCM sample player
#@src/emu/sound/upd7759.h,SOUNDS += UPD7759
#-------------------------------------------------

ifneq ($(filter UPD7759,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/upd7759.o
endif



#-------------------------------------------------
# OKI ADPCM sample players
#@src/emu/sound/okim6258.h,SOUNDS += OKIM6258
#@src/emu/sound/msm5205.h,SOUNDS += MSM5205
#@src/emu/sound/msm5232.h,SOUNDS += MSM5232
#@src/emu/sound/okim6376.h,SOUNDS += OKIM6376
#@src/emu/sound/okim6295.h,SOUNDS += OKIM6295
#@src/emu/sound/okim9810.h,SOUNDS += OKIM9810
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
#@src/emu/sound/saa1099.h,SOUNDS += SAA1099
#-------------------------------------------------

ifneq ($(filter SAA1099,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/saa1099.o
endif



#-------------------------------------------------
# AdMOS QS1000
#@src/emu/sound/qs1000.h,SOUNDS += QS1000
#-------------------------------------------------

ifneq ($(filter QS1000,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/qs1000.o
endif



#-------------------------------------------------
# QSound sample player
#@src/emu/sound/qsound.h,SOUNDS += QSOUND
#-------------------------------------------------

ifneq ($(filter QSOUND,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/qsound.o $(CPUOBJ)/dsp16/dsp16.o $(CPUOBJ)/dsp16/dsp16dis.o
endif



#-------------------------------------------------
# Ricoh sample players
#@src/emu/sound/rf5c68.h,SOUNDS += RF5C68
#@src/emu/sound/rf5c400.h,SOUNDS += RF5C400
#-------------------------------------------------

ifneq ($(filter RF5C68,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/rf5c68.o
endif

ifneq ($(filter RF5C400,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/rf5c400.o
endif



#-------------------------------------------------
# Sega custom sound chips
#@src/emu/sound/segapcm.h,SOUNDS += SEGAPCM
#@src/emu/sound/multipcm.h,SOUNDS += MULTIPCM
#@src/emu/sound/scsp.h,SOUNDS += SCSP
#@src/emu/sound/aica.h,SOUNDS += AICA
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

#-------------------------------------------------
# Seta custom sound chips
#@src/emu/sound/st0016.h,SOUNDS += ST0016
#@src/emu/sound/nile.h,SOUNDS += NILE
#@src/emu/sound/x1_010.h,SOUNDS += X1_010
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
#@src/emu/sound/mos6581.h,SOUNDS += SID6581
#-------------------------------------------------

ifneq ($(filter SID6581 SID8580,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/mos6581.o $(SOUNDOBJ)/sid.o $(SOUNDOBJ)/sidenvel.o $(SOUNDOBJ)/sidvoice.o
endif


#-------------------------------------------------
# SNK(?) custom stereo sn76489a clone
#@src/emu/sound/t6w28.h,SOUNDS += T6W28
#-------------------------------------------------

ifneq ($(filter T6W28,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/t6w28.o
endif



#-------------------------------------------------
# SNK custom wave generator
#@src/emu/sound/snkwave.h,SOUNDS += SNKWAVE
#-------------------------------------------------

ifneq ($(filter SNKWAVE,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/snkwave.o
endif



#-------------------------------------------------
# Sony custom sound chips
#@src/emu/sound/spu.h,SOUNDS += SPU
#-------------------------------------------------

ifneq ($(filter SPU,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/spu.o $(SOUNDOBJ)/spu_tables.o $(SOUNDOBJ)/spureverb.o
endif


#-------------------------------------------------
# SP0256 speech synthesizer
#@src/emu/sound/sp0256.h,SOUNDS += SP0256
#-------------------------------------------------

ifneq ($(filter SP0256,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sp0256.o
endif



#-------------------------------------------------
# SP0250 speech synthesizer
#@src/emu/sound/sp0250.h,SOUNDS += SP0250
#-------------------------------------------------

ifneq ($(filter SP0250,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sp0250.o
endif


#-------------------------------------------------
# S14001A speech synthesizer
#@src/emu/sound/s14001a.h,SOUNDS += S14001A
#-------------------------------------------------

ifneq ($(filter S14001A,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/s14001a.o
endif



#-------------------------------------------------
# Texas Instruments SN76477 analog chip
#@src/emu/sound/sn76477.h,SOUNDS += SN76477
#-------------------------------------------------

ifneq ($(filter SN76477,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sn76477.o
endif



#-------------------------------------------------
# Texas Instruments SN76496
#@src/emu/sound/sn76496.h,SOUNDS += SN76496
#-------------------------------------------------

ifneq ($(filter SN76496,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/sn76496.o
endif



#-------------------------------------------------
# Texas Instruments TMS36xx doorbell chime
#@src/emu/sound/tms36xx.h,SOUNDS += TMS36XX
#-------------------------------------------------

ifneq ($(filter TMS36XX,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tms36xx.o
endif



#-------------------------------------------------
# Texas Instruments TMS3615 Octave Multiple Tone Synthesizer
#@src/emu/sound/tms3615.h,SOUNDS += TMS3615
#-------------------------------------------------

ifneq ($(filter TMS3615,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tms3615.o
endif



#-------------------------------------------------
# Texas Instruments TMS5100-series speech synthesizers
#@src/emu/sound/tms5110.h,SOUNDS += TMS5110
#-------------------------------------------------

ifneq ($(filter TMS5110,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tms5110.o
endif

$(SOUNDOBJ)/tms5110.o:  $(SOUNDSRC)/tms5110r.inc



#-------------------------------------------------
# Texas Instruments TMS5200-series speech synthesizers
#@src/emu/sound/tms5220.h,SOUNDS += TMS5220
#-------------------------------------------------
ifneq ($(filter TMS5220,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tms5220.o $(EMUMACHINE)/spchrom.o
endif

$(SOUNDOBJ)/tms5220.o:  $(SOUNDSRC)/tms5110r.inc



#-------------------------------------------------
# Toshiba T6721A voice synthesizer
#@src/emu/sound/t6721a.h,SOUNDS += T6721A
#-------------------------------------------------

ifneq ($(filter T6721A,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/t6721a.o
endif



#-------------------------------------------------
# Toshiba TC8830F sample player/recorder
#@src/emu/sound/tc8830f.h,SOUNDS += TC8830F
#-------------------------------------------------

ifneq ($(filter TC8830F,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/tc8830f.o
endif


#-------------------------------------------------
# NEC uPD7752
#@src/emu/sound/upd7752.h,SOUNDS += UPD7752
#-------------------------------------------------

ifneq ($(filter UPD7752,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/upd7752.o
endif


#-------------------------------------------------
# VLM5030 speech synthesizer
#@src/emu/sound/vlm5030.h,SOUNDS += VLM5030
#-------------------------------------------------

ifneq ($(filter VLM5030,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/vlm5030.o
endif

$(SOUNDOBJ)/vlm5030.o:  $(SOUNDSRC)/tms5110r.inc



#-------------------------------------------------
# Votrax speech synthesizer
#@src/emu/sound/votrax.h,SOUNDS += VOTRAX
#-------------------------------------------------

ifneq ($(filter VOTRAX,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/votrax.o $(SOUNDOBJ)/samples.o
endif



#-------------------------------------------------
# VRender0 custom sound chip
#@src/emu/sound/vrender0.h,SOUNDS += VRENDER0
#-------------------------------------------------

ifneq ($(filter VRENDER0,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/vrender0.o
endif



#-------------------------------------------------
# WAVE file (used for MESS cassette)
#@src/emu/sound/wave.h,SOUNDS += WAVE
#-------------------------------------------------

ifneq ($(filter WAVE,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/wave.o
endif



#-------------------------------------------------
# Yamaha FM synthesizers
#@src/emu/sound/2151intf.h,SOUNDS += YM2151
#@src/emu/sound/2203intf.h,SOUNDS += YM2203
#@src/emu/sound/2413intf.h,SOUNDS += YM2413
#@src/emu/sound/2608intf.h,SOUNDS += YM2608
#@src/emu/sound/2610intf.h,SOUNDS += YM2610
#@src/emu/sound/2612intf.h,SOUNDS += YM2612
#@src/emu/sound/3812intf.h,SOUNDS += YM3812
#@src/emu/sound/3526intf.h,SOUNDS += YM3526
#@src/emu/sound/8950intf.h,SOUNDS += Y8950
#@src/emu/sound/ymf262.h,SOUNDS += YMF262
#@src/emu/sound/ymf271.h,SOUNDS += YMF271
#@src/emu/sound/ymf278b.h,SOUNDS += YMF278B
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
SOUNDOBJS += $(SOUNDOBJ)/3526intf.o $(SOUNDOBJ)/fmopl.o $(SOUNDOBJ)/ymdeltat.o
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
#@src/emu/sound/ymz280b.h,SOUNDS += YMZ280B
#-------------------------------------------------

ifneq ($(filter YMZ280B,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ymz280b.o
endif

#-------------------------------------------------
# Yamaha YMZ770 AMM
#@src/emu/sound/ymz770.h,SOUNDS += YMZ770
#-------------------------------------------------

ifneq ($(filter YMZ770,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/ymz770.o $(SOUNDOBJ)/mpeg_audio.o
endif

#-------------------------------------------------
# ZOOM ZSG-2
#@src/emu/sound/zsg2.h,SOUNDS += ZSG2
#-------------------------------------------------

ifneq ($(filter ZSG2,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/zsg2.o
endif

#-------------------------------------------------
# VRC6
#@src/emu/sound/vrc6.h,SOUNDS += VRC6
#-------------------------------------------------

ifneq ($(filter VRC6,$(SOUNDS)),)
SOUNDOBJS += $(SOUNDOBJ)/vrc6.o
endif

