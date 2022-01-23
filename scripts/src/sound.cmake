# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   sound.cmake
##
##   Rules for building sound cores
##
##########################################################################

set(SOUND_SRCS
	${MAME_DIR}/src/devices/sound/bbd.cpp
	${MAME_DIR}/src/devices/sound/bbd.h
	${MAME_DIR}/src/devices/sound/flt_biquad.cpp
	${MAME_DIR}/src/devices/sound/flt_biquad.h
	${MAME_DIR}/src/devices/sound/flt_vol.cpp
	${MAME_DIR}/src/devices/sound/flt_vol.h
	${MAME_DIR}/src/devices/sound/flt_rc.cpp
	${MAME_DIR}/src/devices/sound/flt_rc.h
	${MAME_DIR}/src/devices/sound/mixer.cpp
	${MAME_DIR}/src/devices/sound/mixer.h
	${MAME_DIR}/src/devices/sound/samples.cpp
	${MAME_DIR}/src/devices/sound/samples.h
)

###################################################
## DACs
##@src/devices/sound/dac.h,list(APPEND SOUNDS DAC)
##@src/devices/sound/dmadac.h,list(APPEND SOUNDS DMADAC)
##@src/devices/sound/spkrdev.h,list(APPEND SOUNDS SPEAKER)
##@src/devices/sound/beep.h,list(APPEND SOUNDS BEEP)
###################################################

if("DAC" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/dac.cpp
		${MAME_DIR}/src/devices/sound/dac.h
	)
endif()

if("DMADAC" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/dmadac.cpp
		${MAME_DIR}/src/devices/sound/dmadac.h
	)
endif()

if("SPEAKER" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/spkrdev.cpp
		${MAME_DIR}/src/devices/sound/spkrdev.h
	)
endif()

if("BEEP" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/beep.cpp
		${MAME_DIR}/src/devices/sound/beep.h
	)
endif()



###################################################
## CD audio
##@src/devices/sound/cdda.h,list(APPEND SOUNDS CDDA)
###################################################

if("CDDA" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/cdda.cpp
		${MAME_DIR}/src/devices/sound/cdda.h
	)
endif()



###################################################
## Discrete component audio
##@src/devices/sound/discrete.h,list(APPEND SOUNDS DISCRETE)
###################################################

if("DISCRETE" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/discrete.cpp
		${MAME_DIR}/src/devices/sound/discrete.h
		${MAME_DIR}/src/devices/sound/disc_cls.h
		${MAME_DIR}/src/devices/sound/disc_dev.h
		${MAME_DIR}/src/devices/sound/disc_dev.hxx
		${MAME_DIR}/src/devices/sound/disc_flt.h
		${MAME_DIR}/src/devices/sound/disc_flt.hxx
		${MAME_DIR}/src/devices/sound/disc_inp.hxx
		${MAME_DIR}/src/devices/sound/disc_mth.h
		${MAME_DIR}/src/devices/sound/disc_mth.hxx
		${MAME_DIR}/src/devices/sound/disc_sys.hxx
		${MAME_DIR}/src/devices/sound/disc_wav.h
		${MAME_DIR}/src/devices/sound/disc_wav.hxx
	)
endif()

###################################################
## AC97
##@src/devices/sound/pci-ac97.h,list(APPEND SOUNDS AC97)
###################################################

if("AC97" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/pci-ac97.cpp
		${MAME_DIR}/src/devices/sound/pci-ac97.h
	)
endif()



###################################################
## Apple custom sound chips
##@src/devices/sound/asc.h,list(APPEND SOUNDS ASC)
##@src/devices/sound/awacs.h,list(APPEND SOUNDS AWACS)
###################################################

if("ASC" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/asc.cpp
		${MAME_DIR}/src/devices/sound/asc.h
	)
endif()

if("AWACS" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/awacs.cpp
		${MAME_DIR}/src/devices/sound/awacs.h
	)
endif()


###################################################
## Atari custom sound chips
##@src/devices/sound/pokey.h,list(APPEND SOUNDS POKEY)
##@src/devices/sound/tiaintf.h,list(APPEND SOUNDS TIA)
###################################################

if("POKEY" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/pokey.cpp
		${MAME_DIR}/src/devices/sound/pokey.h
	)
endif()

if("TIA" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/tiasound.cpp
		${MAME_DIR}/src/devices/sound/tiasound.h
		${MAME_DIR}/src/devices/sound/tiaintf.cpp
		${MAME_DIR}/src/devices/sound/tiaintf.h
	)
endif()



###################################################
## Amiga audio hardware
##@src/devices/machine/8364_paula.h,list(APPEND SOUNDS PAULA_8364)
###################################################

if("PAULA_8364" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/machine/8364_paula.cpp
		${MAME_DIR}/src/devices/machine/8364_paula.h
	)
endif()



###################################################
## Bally Astrocade sound system
##@src/devices/sound/astrocde.h,list(APPEND SOUNDS ASTROCADE)
###################################################

if("ASTROCADE" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/astrocde.cpp
		${MAME_DIR}/src/devices/sound/astrocde.h
	)
endif()



###################################################
###################################################
## AC97
##@src/devices/sound/pic-ac97.h,list(APPEND SOUNDS AC97)
###################################################

if("AC97" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/pci-ac97.cpp
		${MAME_DIR}/src/devices/sound/pci-ac97.h
	)
endif()
## CEM 3394 analog synthesizer chip
##@src/devices/sound/cem3394.h,list(APPEND SOUNDS CEM3394)
###################################################

if("CEM3394" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/cem3394.cpp
		${MAME_DIR}/src/devices/sound/cem3394.h
	)
endif()



###################################################
## Creative Labs SB0400 Audigy2 Value
##@src/devices/sound/sb0400.h,list(APPEND SOUNDS SB0400)
###################################################

if("SB0400" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/sb0400.cpp
		${MAME_DIR}/src/devices/sound/sb0400.h
	)
endif()


##################################################
## Creative Labs Ensonic AudioPCI97 ES1373
##@src/devices/sound/es1373.h,list(APPEND SOUNDS ES1373)
##################################################

if("ES1373" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/es1373.cpp
		${MAME_DIR}/src/devices/sound/es1373.h
	)
endif()

###################################################
## Data East custom sound chips
##@src/devices/sound/bsmt2000.h,list(APPEND SOUNDS BSMT2000)
###################################################

if("BSMT2000" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/bsmt2000.cpp
		${MAME_DIR}/src/devices/sound/bsmt2000.h
	)
endif()



###################################################
## Ensoniq 5503 (Apple IIgs)
##@src/devices/sound/es5503.h,list(APPEND SOUNDS ES5503)
###################################################

if("ES5503" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/es5503.cpp
		${MAME_DIR}/src/devices/sound/es5503.h
	)
endif()



###################################################
## Ensoniq 5505/5506
##@src/devices/sound/es5506.h,list(APPEND SOUNDS ES5505)
###################################################

if(("ES5505" IN_LIST SOUNDS) OR ("ES5506" IN_LIST SOUNDS))
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/es5506.cpp
		${MAME_DIR}/src/devices/sound/es5506.h
	)
endif()


###################################################
## Ensoniq "pump" device, interfaces 5505/5506 with 5510
##@src/devices/sound/esqpump.h,list(APPEND SOUNDS ESQPUMP)
###################################################

if("ESQPUMP" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/esqpump.cpp
		${MAME_DIR}/src/devices/sound/esqpump.h
	)
endif()


###################################################
## Excellent Systems ADPCM sound chip
##@src/devices/sound/es8712.h,list(APPEND SOUNDS ES8712)
###################################################

if("ES8712" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/es8712.cpp
		${MAME_DIR}/src/devices/sound/es8712.h
	)
endif()



###################################################
## Gaelco custom sound chips
##@src/devices/sound/gaelco.h,list(APPEND SOUNDS GAELCO_CG1V)
###################################################

if(("GAELCO_CG1V" IN_LIST SOUNDS) OR ("GAELCO_GAE1" IN_LIST SOUNDS))
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/gaelco.cpp
		${MAME_DIR}/src/devices/sound/gaelco.h
	)
endif()


###################################################
## RCA CDP1863
##@src/devices/sound/cdp1863.h,list(APPEND SOUNDS CDP1863)
###################################################

if("CDP1863" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/cdp1863.cpp
		${MAME_DIR}/src/devices/sound/cdp1863.h
	)
endif()



###################################################
## RCA CDP1864
##@src/devices/sound/cdp1864.h,list(APPEND SOUNDS CDP1864)
###################################################

if("CDP1864" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/cdp1864.cpp
		${MAME_DIR}/src/devices/sound/cdp1864.h
	)
endif()



###################################################
## RCA CDP1869
##@src/devices/sound/cdp1869.h,list(APPEND SOUNDS CDP1869)
###################################################

if("CDP1869" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/cdp1869.cpp
		${MAME_DIR}/src/devices/sound/cdp1869.h
	)
endif()



###################################################
## GI AY-8910
##@src/devices/sound/ay8910.h,list(APPEND SOUNDS AY8910)
###################################################

if("AY8910" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ay8910.cpp
		${MAME_DIR}/src/devices/sound/ay8910.h
	)
endif()



###################################################
## Harris HC55516 CVSD
##@src/devices/sound/hc55516.h,list(APPEND SOUNDS HC55516)
###################################################

if("HC55516" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/hc55516.cpp
		${MAME_DIR}/src/devices/sound/hc55516.h
	)
endif()



###################################################
## Hudsonsoft HuC6230 SoundBox
##@src/devices/sound/huc6230.h,list(APPEND SOUNDS HUC6230)
###################################################

if("HUC6230" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/huc6230.cpp
		${MAME_DIR}/src/devices/sound/huc6230.h
	)
endif()



###################################################
## Hudsonsoft C6280 sound chip
##@src/devices/sound/c6280.h,list(APPEND SOUNDS C6280)
###################################################

if("C6280" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/c6280.cpp
		${MAME_DIR}/src/devices/sound/c6280.h
	)
endif()



###################################################
## ICS2115 sound chip
##@src/devices/sound/ics2115.h,list(APPEND SOUNDS ICS2115)
###################################################

if("ICS2115" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ics2115.cpp
		${MAME_DIR}/src/devices/sound/ics2115.h
	)
endif()



###################################################
## Imagetek I5000 sound
##@src/devices/sound/i5000.h,list(APPEND SOUNDS I5000_SND)
###################################################

if("I5000_SND" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/i5000.cpp
		${MAME_DIR}/src/devices/sound/i5000.h
	)
endif()



###################################################
## Irem custom sound chips
##@src/devices/sound/iremga20.h,list(APPEND SOUNDS IREMGA20)
###################################################

if("IREMGA20" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/iremga20.cpp
		${MAME_DIR}/src/devices/sound/iremga20.h
	)
endif()



###################################################
## Konami custom sound chips
##@src/devices/sound/k005289.h,list(APPEND SOUNDS K005289)
##@src/devices/sound/k007232.h,list(APPEND SOUNDS K007232)
##@src/devices/sound/k051649.h,list(APPEND SOUNDS K051649)
##@src/devices/sound/k053260.h,list(APPEND SOUNDS K053260)
##@src/devices/sound/k054539.h,list(APPEND SOUNDS K054539)
##@src/devices/sound/k056800.h,list(APPEND SOUNDS K056800)
###################################################

if("K005289" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/k005289.cpp
		${MAME_DIR}/src/devices/sound/k005289.h
	)
endif()

if("K007232" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/k007232.cpp
		${MAME_DIR}/src/devices/sound/k007232.h
	)
endif()

if("K051649" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/k051649.cpp
		${MAME_DIR}/src/devices/sound/k051649.h
	)
endif()

if("K053260" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/k053260.cpp
		${MAME_DIR}/src/devices/sound/k053260.h
	)
endif()

if("K054539" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/k054539.cpp
		${MAME_DIR}/src/devices/sound/k054539.h
	)
endif()

if("K056800" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/k056800.cpp
		${MAME_DIR}/src/devices/sound/k056800.h
	)
endif()


###################################################
## L7A1045 L6028 DSP-A
##@src/devices/sound/l7a1045_l6028_dsp_a.h,list(APPEND SOUNDS L7A1045)
###################################################

if("L7A1045" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/l7a1045_l6028_dsp_a.cpp
		${MAME_DIR}/src/devices/sound/l7a1045_l6028_dsp_a.h
	)
endif()


###################################################
## LMC1992 mixer chip
##@src/devices/sound/lmc1992.h,list(APPEND SOUNDS LMC1992)
###################################################

if("LMC1992" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/lmc1992.cpp
		${MAME_DIR}/src/devices/sound/lmc1992.h
	)
endif()



###################################################
## MAS 3507D MPEG 1/2 Layer 2/3 Audio Decoder
##@src/devices/sound/mas3507d.h,list(APPEND SOUNDS MAS3507D)
###################################################

if("MAS3507D" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/mas3507d.cpp
		${MAME_DIR}/src/devices/sound/mas3507d.h
	)
endif()



###################################################
## MEA8000 Voice Synthesizer
##@src/devices/sound/mea8000.h,list(APPEND SOUNDS MEA8000)
###################################################

if("MEA8000" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/mea8000.cpp
		${MAME_DIR}/src/devices/sound/mea8000.h
	)
endif()



###################################################
## MOS 6560VIC
##@src/devices/sound/mos6560.h,list(APPEND SOUNDS MOS656X)
###################################################

if("MOS656X" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/mos6560.cpp
		${MAME_DIR}/src/devices/sound/mos6560.h
	)
endif()



###################################################
## MOS 7360 TED
##@src/devices/sound/mos7360.h,list(APPEND SOUNDS MOS7360)
###################################################

if("MOS7360" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/mos7360.cpp
		${MAME_DIR}/src/devices/sound/mos7360.h
	)
endif()



###################################################
## Namco custom sound chips
##@src/devices/sound/namco.h,list(APPEND SOUNDS NAMCO)
##@src/devices/sound/namco_163.h,list(APPEND SOUNDS NAMCO_163)
##@src/devices/sound/n63701x.h,list(APPEND SOUNDS NAMCO_63701X)
##@src/devices/sound/c140.h,list(APPEND SOUNDS C140)
##@src/devices/sound/c352.h,list(APPEND SOUNDS C352)
###################################################

if(("NAMCO" IN_LIST SOUNDS) OR ("NAMCO_15XX" IN_LIST SOUNDS) OR ("NAMCO_CUS30" IN_LIST SOUNDS))
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/namco.cpp
		${MAME_DIR}/src/devices/sound/namco.h
	)
endif()

if("NAMCO_163" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/namco_163.cpp
		${MAME_DIR}/src/devices/sound/namco_163.h
	)
endif()

if("NAMCO_63701X" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/n63701x.cpp
		${MAME_DIR}/src/devices/sound/n63701x.h
	)
endif()

if("C140" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/c140.cpp
		${MAME_DIR}/src/devices/sound/c140.h
	)
endif()

if("C352" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/c352.cpp
		${MAME_DIR}/src/devices/sound/c352.h
	)
endif()



###################################################
## National Semiconductor Digitalker
##@src/devices/sound/digitalk.h,list(APPEND SOUNDS DIGITALKER)
###################################################

if("DIGITALKER" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/digitalk.cpp
		${MAME_DIR}/src/devices/sound/digitalk.h
	)
endif()



###################################################
## Nintendif()o custom sound chips
##@src/devices/sound/nes_apu.h,list(APPEND SOUNDS NES_APU)
###################################################

if("NES_APU" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/nes_apu.cpp
		${MAME_DIR}/src/devices/sound/nes_apu.h
		${MAME_DIR}/src/devices/sound/nes_defs.h
		${MAME_DIR}/src/devices/sound/nes_apu_vt.cpp
		${MAME_DIR}/src/devices/sound/nes_apu_vt.h
	)
endif()



###################################################
## NEC uPD7759 ADPCM sample player
##@src/devices/sound/upd7759.h,list(APPEND SOUNDS UPD7759)
###################################################

if("UPD7759" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/upd7759.cpp
		${MAME_DIR}/src/devices/sound/upd7759.h
		${MAME_DIR}/src/devices/sound/315-5641.cpp
		${MAME_DIR}/src/devices/sound/315-5641.h
	)
endif()



###################################################
## OKI ADPCM sample players
##@src/devices/sound/okim6258.h,list(APPEND SOUNDS OKIM6258)
##@src/devices/sound/msm5205.h,list(APPEND SOUNDS MSM5205)
##@src/devices/sound/msm5232.h,list(APPEND SOUNDS MSM5232)
##@src/devices/sound/okim6376.h,list(APPEND SOUNDS OKIM6376)
##@src/devices/sound/okim6295.h,list(APPEND SOUNDS OKIM6295)
##@src/devices/sound/okim9810.h,list(APPEND SOUNDS OKIM9810)
##@src/devices/sound/okiadpcm.h,list(APPEND SOUNDS OKIADPCM)
###################################################

if(("OKIM6258" IN_LIST SOUNDS) OR ("OKIM6295" IN_LIST SOUNDS) OR ("OKIM9810" IN_LIST SOUNDS) OR ("I5000_SND" IN_LIST SOUNDS) OR ("OKIADPCM" IN_LIST SOUNDS))
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/okiadpcm.cpp
		${MAME_DIR}/src/devices/sound/okiadpcm.h
	)
endif()

if(("MSM5205" IN_LIST SOUNDS) OR ("MSM6585" IN_LIST SOUNDS))
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/msm5205.cpp
		${MAME_DIR}/src/devices/sound/msm5205.h
	)
endif()

if("MSM5232" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/msm5232.cpp
		${MAME_DIR}/src/devices/sound/msm5232.h
	)
endif()

if("OKIM6376" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/okim6376.cpp
		${MAME_DIR}/src/devices/sound/okim6376.h
	)
endif()

if("OKIM6295" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/okim6295.cpp
		${MAME_DIR}/src/devices/sound/okim6295.h
	)
endif()

if("OKIM6258" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/okim6258.cpp
		${MAME_DIR}/src/devices/sound/okim6258.h
	)
endif()

if("OKIM9810" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/okim9810.cpp
		${MAME_DIR}/src/devices/sound/okim9810.h
	)
endif()



###################################################
## Philips SAA1099
##@src/devices/sound/saa1099.h,list(APPEND SOUNDS SAA1099)
###################################################

if("SAA1099" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/saa1099.cpp
		${MAME_DIR}/src/devices/sound/saa1099.h
	)
endif()



###################################################
## AdMOS QS1000
##@src/devices/sound/qs1000.h,list(APPEND SOUNDS QS1000)
###################################################

if("QS1000" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/qs1000.cpp
		${MAME_DIR}/src/devices/sound/qs1000.h
	)
endif()



###################################################
## QSound sample player
##@src/devices/sound/qsound.h,list(APPEND SOUNDS QSOUND)
###################################################

if("QSOUND" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/qsound.cpp
		${MAME_DIR}/src/devices/sound/qsound.h
		${MAME_DIR}/src/devices/sound/qsoundhle.cpp
		${MAME_DIR}/src/devices/sound/qsoundhle.h
	)
endif()



###################################################
## Ricoh sample players
##@src/devices/sound/rf5c68.h,list(APPEND SOUNDS RF5C68)
##@src/devices/sound/rf5c400.h,list(APPEND SOUNDS RF5C400)
###################################################

if("RF5C68" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/rf5c68.cpp
		${MAME_DIR}/src/devices/sound/rf5c68.h
	)
endif()

if("RF5C400" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/rf5c400.cpp
		${MAME_DIR}/src/devices/sound/rf5c400.h
	)
endif()



###################################################
## Sega custom sound chips
##@src/devices/sound/segapcm.h,list(APPEND SOUNDS SEGAPCM)
##@src/devices/sound/multipcm.h,list(APPEND SOUNDS MULTIPCM)
##@src/devices/sound/scsp.h,list(APPEND SOUNDS SCSP)
##@src/devices/sound/aica.h,list(APPEND SOUNDS AICA)
###################################################

if("SEGAPCM" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/segapcm.cpp
		${MAME_DIR}/src/devices/sound/segapcm.h
	)
endif()

if("MULTIPCM" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/multipcm.cpp
		${MAME_DIR}/src/devices/sound/multipcm.h
	)
endif()

if("SCSP" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/scsp.cpp
		${MAME_DIR}/src/devices/sound/scsp.h
		${MAME_DIR}/src/devices/sound/scspdsp.cpp
		${MAME_DIR}/src/devices/sound/scspdsp.h
	)
endif()

if("AICA" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/aica.cpp
		${MAME_DIR}/src/devices/sound/aica.h
		${MAME_DIR}/src/devices/sound/aicadsp.cpp
		${MAME_DIR}/src/devices/sound/aicadsp.h
	)
endif()

###################################################
## Seta custom sound chips
##@src/devices/sound/st0016.h,list(APPEND SOUNDS ST0016)
##@src/devices/sound/setapcm.h,list(APPEND SOUNDS SETAPCM)
##@src/devices/sound/x1_010.h,list(APPEND SOUNDS X1_010)
###################################################

if("ST0016" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/st0016.cpp
		${MAME_DIR}/src/devices/sound/st0016.h
	)
endif()

if("SETAPCM" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/setapcm.cpp
		${MAME_DIR}/src/devices/sound/setapcm.h
	)
endif()

if("X1_010" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/x1_010.cpp
		${MAME_DIR}/src/devices/sound/x1_010.h
	)
endif()



###################################################
## SID custom sound chips
##@src/devices/sound/mos6581.h,list(APPEND SOUNDS SID6581)
###################################################

if(("SID6581" IN_LIST SOUNDS) OR ("SID8580" IN_LIST SOUNDS))
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/mos6581.cpp
		${MAME_DIR}/src/devices/sound/mos6581.h
		${MAME_DIR}/src/devices/sound/sid.cpp
		${MAME_DIR}/src/devices/sound/sid.h
		${MAME_DIR}/src/devices/sound/sidenvel.cpp
		${MAME_DIR}/src/devices/sound/sidenvel.h
		${MAME_DIR}/src/devices/sound/sidvoice.cpp
		${MAME_DIR}/src/devices/sound/sidvoice.h
		${MAME_DIR}/src/devices/sound/side6581.h
		${MAME_DIR}/src/devices/sound/sidw6581.h
		${MAME_DIR}/src/devices/sound/sidw8580.h
	)
endif()


###################################################
## SNK(?) custom stereo sn76489a clone
##@src/devices/sound/t6w28.h,list(APPEND SOUNDS T6W28)
###################################################

if("T6W28" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/t6w28.cpp
		${MAME_DIR}/src/devices/sound/t6w28.h
	)
endif()



###################################################
## SNK custom wave generator
##@src/devices/sound/snkwave.h,list(APPEND SOUNDS SNKWAVE)
###################################################

if("SNKWAVE" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/snkwave.cpp
		${MAME_DIR}/src/devices/sound/snkwave.h
	)
endif()



###################################################
## Sony custom sound chips
##@src/devices/sound/spu.h,list(APPEND SOUNDS SPU)
###################################################

if("SPU" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/spu.cpp
		${MAME_DIR}/src/devices/sound/spu.h
		${MAME_DIR}/src/devices/sound/spu_tables.cpp
		${MAME_DIR}/src/devices/sound/spureverb.cpp
		${MAME_DIR}/src/devices/sound/spureverb.h
	)
endif()


###################################################
## SP0256 speech synthesizer
##@src/devices/sound/sp0256.h,list(APPEND SOUNDS SP0256)
###################################################

if("SP0256" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/sp0256.cpp
		${MAME_DIR}/src/devices/sound/sp0256.h
	)
endif()



###################################################
## SP0250 speech synthesizer
##@src/devices/sound/sp0250.h,list(APPEND SOUNDS SP0250)
###################################################

if("SP0250" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/sp0250.cpp
		${MAME_DIR}/src/devices/sound/sp0250.h
	)
endif()



###################################################
## S14001A speech synthesizer
##@src/devices/sound/s14001a.h,list(APPEND SOUNDS S14001A)
###################################################

if("S14001A" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/s14001a.cpp
		${MAME_DIR}/src/devices/sound/s14001a.h
	)
endif()



###################################################
## Texas Instruments SN76477 analog chip
##@src/devices/sound/sn76477.h,list(APPEND SOUNDS SN76477)
###################################################

if("SN76477" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/sn76477.cpp
		${MAME_DIR}/src/devices/sound/sn76477.h
	)
endif()



###################################################
## Texas Instruments SN76496
##@src/devices/sound/sn76496.h,list(APPEND SOUNDS SN76496)
###################################################

if("SN76496" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/sn76496.cpp
		${MAME_DIR}/src/devices/sound/sn76496.h
	)
endif()



###################################################
## Texas Instruments TMS36xx doorbell chime
##@src/devices/sound/tms36xx.h,list(APPEND SOUNDS TMS36XX)
###################################################

if("TMS36XX" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/tms36xx.cpp
		${MAME_DIR}/src/devices/sound/tms36xx.h
	)
endif()



###################################################
## Texas Instruments TMS3615 Octave Multiple Tone Synthesizer
##@src/devices/sound/tms3615.h,list(APPEND SOUNDS TMS3615)
###################################################

if("TMS3615" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/tms3615.cpp
		${MAME_DIR}/src/devices/sound/tms3615.h
	)
endif()



###################################################
## Texas Instruments TMS5100-series speech synthesizers
##@src/devices/sound/tms5110.h,list(APPEND SOUNDS TMS5110)
###################################################

if("TMS5110" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/tms5110.cpp
		${MAME_DIR}/src/devices/sound/tms5110.h
		${MAME_DIR}/src/devices/sound/tms5110r.hxx
	)
endif()

###################################################
## Texas Instruments TMS5200-series speech synthesizers
##@src/devices/sound/tms5220.h,list(APPEND SOUNDS TMS5220)
###################################################
if("TMS5220" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/tms5220.cpp
		${MAME_DIR}/src/devices/sound/tms5220.h
		${MAME_DIR}/src/devices/sound/tms5110r.hxx
		${MAME_DIR}/src/devices/machine/spchrom.cpp
		${MAME_DIR}/src/devices/machine/spchrom.h
	)
endif()


###################################################
## Toshiba T6721A voice synthesizer
##@src/devices/sound/t6721a.h,list(APPEND SOUNDS T6721A)
###################################################

if("T6721A" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/t6721a.cpp
		${MAME_DIR}/src/devices/sound/t6721a.h
	)
endif()



###################################################
## Toshiba TC8830F sample player/recorder
##@src/devices/sound/tc8830f.h,list(APPEND SOUNDS TC8830F)
###################################################

if("TC8830F" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/tc8830f.cpp
		${MAME_DIR}/src/devices/sound/tc8830f.h
	)
endif()


###################################################
## NEC uPD7752
##@src/devices/sound/upd7752.h,list(APPEND SOUNDS UPD7752)
###################################################

if("UPD7752" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/upd7752.cpp
		${MAME_DIR}/src/devices/sound/upd7752.h
	)
endif()


###################################################
## VLM5030 speech synthesizer
##@src/devices/sound/vlm5030.h,list(APPEND SOUNDS VLM5030)
###################################################

if("VLM5030" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/vlm5030.cpp
		${MAME_DIR}/src/devices/sound/vlm5030.h
		${MAME_DIR}/src/devices/sound/tms5110r.hxx
	)
endif()

###################################################
## Votrax speech synthesizer
##@src/devices/sound/votrax.h,list(APPEND SOUNDS VOTRAX)
###################################################

if("VOTRAX" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/votrax.cpp
		${MAME_DIR}/src/devices/sound/votrax.h
	)
endif()



###################################################
## VRender0 custom sound chip
##@src/devices/sound/vrender0.h,list(APPEND SOUNDS VRENDER0)
###################################################

if("VRENDER0" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/vrender0.cpp
		${MAME_DIR}/src/devices/sound/vrender0.h
	)
endif()



###################################################
## WAVE file (used for MESS cassette)
##@src/devices/sound/wave.h,list(APPEND SOUNDS WAVE)
###################################################

if("WAVE" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/wave.cpp
		${MAME_DIR}/src/devices/sound/wave.h
	)
endif()


###################################################
## Yamaha FM synthesizers
##@src/devices/sound/ym2154.h,list(APPEND SOUNDS YM2154)
##@src/devices/sound/ymopm.h,list(APPEND SOUNDS YM2151)
##@src/devices/sound/ymopz.h,list(APPEND SOUNDS YM2414)
##@src/devices/sound/ymopq.h,list(APPEND SOUNDS YM3806)
##@src/devices/sound/ymopn.h,list(APPEND SOUNDS YM2203)
##@src/devices/sound/ymopl.h,list(APPEND SOUNDS YM2413)
##@src/devices/sound/ymopn.h,list(APPEND SOUNDS YM2608)
##@src/devices/sound/ymopn.h,list(APPEND SOUNDS YM2610)
##@src/devices/sound/ymopn.h,list(APPEND SOUNDS YM2612)
##@src/devices/sound/ymopl.h,list(APPEND SOUNDS YM3526)
##@src/devices/sound/ymopl.h,list(APPEND SOUNDS YM3812)
##@src/devices/sound/ymopl.h,list(APPEND SOUNDS YMF262)
##@src/devices/sound/ymopl.h,list(APPEND SOUNDS YMF278B)
##@src/devices/sound/ymf271.h,list(APPEND SOUNDS YMF271)
##@src/devices/sound/ymopl.h,list(APPEND SOUNDS Y8950)
###################################################

if("YM2154" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ym2154.cpp
		${MAME_DIR}/src/devices/sound/ym2154.h
)
endif()

if(("YM2151" IN_LIST SOUNDS) OR ("YM2164" IN_LIST SOUNDS))
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ymopm.cpp
		${MAME_DIR}/src/devices/sound/ymopm.h
)
endif()

if("YM2414" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ymopz.cpp
		${MAME_DIR}/src/devices/sound/ymopz.h
)
endif()

if("YM3806" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ymopq.cpp
		${MAME_DIR}/src/devices/sound/ymopq.h
)
endif()

if(("YM2203" IN_LIST SOUNDS) OR ("YM2608" IN_LIST SOUNDS) OR ("YM2610" IN_LIST SOUNDS) OR ("YM2610B" IN_LIST SOUNDS) OR ("YM2612" IN_LIST SOUNDS) OR ("YM3438" IN_LIST SOUNDS))
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ay8910.cpp
		${MAME_DIR}/src/devices/sound/ay8910.h
		${MAME_DIR}/src/devices/sound/ymopn.cpp
		${MAME_DIR}/src/devices/sound/ymopn.h
)
endif()

if(("YM3526" IN_LIST SOUNDS) OR ("Y8950" IN_LIST SOUNDS) OR ("YM3812" IN_LIST SOUNDS) OR ("YMF262" IN_LIST SOUNDS) OR ("YMF278B" IN_LIST SOUNDS) OR ("YM2413" IN_LIST SOUNDS) OR ("YM2423" IN_LIST SOUNDS) OR ("YMF281" IN_LIST SOUNDS) OR ("DS1001" IN_LIST SOUNDS))
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ymopl.cpp
		${MAME_DIR}/src/devices/sound/ymopl.h
)
endif()

if("YMF271" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ymf271.cpp
		${MAME_DIR}/src/devices/sound/ymf271.h
)
endif()

###################################################
## Yamaha YMZ280B ADPCM
##@src/devices/sound/ymz280b.h,list(APPEND SOUNDS YMZ280B)
###################################################

if("YMZ280B" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ymz280b.cpp
		${MAME_DIR}/src/devices/sound/ymz280b.h
	)
endif()

###################################################
## Yamaha YMZ770 AMM
##@src/devices/sound/ymz770.h,list(APPEND SOUNDS YMZ770)
###################################################

if("YMZ770" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ymz770.cpp
		${MAME_DIR}/src/devices/sound/ymz770.h
	)
endif()

###################################################
## MPEG AUDIO
##@src/devices/sound/mpeg_audio.h,list(APPEND SOUNDS MPEG_AUDIO)
###################################################

if("MPEG_AUDIO" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/mpeg_audio.cpp
		${MAME_DIR}/src/devices/sound/mpeg_audio.h
	)
endif()

###################################################
## ZOOM ZSG-2
##@src/devices/sound/zsg2.h,list(APPEND SOUNDS ZSG2)
###################################################

if("ZSG2" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/zsg2.cpp
		${MAME_DIR}/src/devices/sound/zsg2.h
	)
endif()

###################################################
## VRC6
##@src/devices/sound/vrc6.h,list(APPEND SOUNDS VRC6)
###################################################

if("VRC6" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/vrc6.cpp
		${MAME_DIR}/src/devices/sound/vrc6.h
	)
endif()

###################################################
## AD1848
##@src/devices/sound/ad1848.h,list(APPEND SOUNDS AD1848)
###################################################

if("AD1848" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ad1848.cpp
		${MAME_DIR}/src/devices/sound/ad1848.h
	)
endif()

###################################################
## UPD1771
##@src/devices/sound/upd1771.h,list(APPEND SOUNDS UPD1771)
###################################################

if("UPD1771" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/upd1771.cpp
		${MAME_DIR}/src/devices/sound/upd1771.h
	)
endif()

###################################################
## GB_SOUND
##@src/devices/sound/gb.h,list(APPEND SOUNDS GB_SOUND)
###################################################

if("GB_SOUND" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/gb.cpp
		${MAME_DIR}/src/devices/sound/gb.h
	)
endif()

###################################################
## PCD3311
##@src/devices/sound/pcd3311.h,list(APPEND SOUNDS PCD3311)
###################################################

if("PCD3311" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/pcd3311.cpp
		${MAME_DIR}/src/devices/sound/pcd3311.h
	)
endif()

###################################################
## DAC-76 COMDAC
##@src/devices/sound/dac76.h,list(APPEND SOUNDS DAC76)
###################################################
if("DAC76" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/dac76.cpp
		${MAME_DIR}/src/devices/sound/dac76.h
	)
endif()

###################################################
## MM5837 Noise Generator
##@src/devices/sound/mm5837.h,list(APPEND SOUNDS MM5837)
###################################################

if("MM5837" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/mm5837.cpp
		${MAME_DIR}/src/devices/sound/mm5837.h
	)
endif()

###################################################
## Intelligent Designs DAVE
##@src/devices/sound/dave.h,list(APPEND SOUNDS DAVE)
###################################################

if("DAVE" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/dave.cpp
		${MAME_DIR}/src/devices/sound/dave.h
	)
endif()

###################################################
## Toshiba TA7630
##@src/devices/sound/ta7630.h,list(APPEND SOUNDS TA7630)
###################################################

if("TA7630" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ta7630.cpp
		${MAME_DIR}/src/devices/sound/ta7630.h
	)
endif()

###################################################
## Sanyo LC7535
##@src/devices/sound/lc7535.h,list(APPEND SOUNDS LC7535)
###################################################

if("LC7535" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/lc7535.cpp
		${MAME_DIR}/src/devices/sound/lc7535.h
	)
endif()

###################################################
## NEC uPD934G
##@src/devices/sound/upd934g.h,list(APPEND SOUNDS UPD934G)
###################################################

if("UPD934G" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/upd934g.cpp
		${MAME_DIR}/src/devices/sound/upd934g.h
	)
endif()

###################################################
##
##@src/devices/sound/iopspu.h,list(APPEND SOUNDS IOPSPU)
###################################################

if("IOPSPU" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/iopspu.cpp
		${MAME_DIR}/src/devices/sound/iopspu.h
	)
endif()

###################################################
##
##@src/devices/sound/swp00.h,list(APPEND SOUNDS SWP00)
###################################################

if("SWP00" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/swp00.cpp
		${MAME_DIR}/src/devices/sound/swp00.h
	)
endif()

###################################################
##
##@src/devices/sound/swp20.h,list(APPEND SOUNDS SWP20)
###################################################

if("SWP20" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/swp20.cpp
		${MAME_DIR}/src/devices/sound/swp20.h
	)
endif()

###################################################
##
##@src/devices/sound/swp30.h,list(APPEND SOUNDS SWP30)
###################################################

if("SWP30" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/swp30.cpp
		${MAME_DIR}/src/devices/sound/swp30.h
	)
endif()

###################################################
##
##@src/devices/sound/xt446.h,list(APPEND SOUNDS XT446)
###################################################

if("XT446" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/xt446.cpp
		${MAME_DIR}/src/devices/sound/xt446.h
	)
endif()

###################################################
## Roland sample players
##@src/devices/sound/rolandpcm.h,list(APPEND SOUNDS ROLANDPCM)
###################################################

if("ROLANDPCM" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/rolandpcm.cpp
		${MAME_DIR}/src/devices/sound/rolandpcm.h
	)
endif()

###################################################
##
##@src/devices/sound/vgm_visualizer.h,list(APPEND SOUNDS VGMVIZ)
###################################################

if("VGMVIZ" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/vgm_visualizer.cpp
		${MAME_DIR}/src/devices/sound/vgm_visualizer.h
	)
endif()

###################################################
##
##@src/devices/sound/s_dsp.h,list(APPEND SOUNDS S_DSP)
###################################################

if("S_DSP" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/s_dsp.cpp
		${MAME_DIR}/src/devices/sound/s_dsp.h
	)
endif()

###################################################
##
##@src/devices/sound/ks0164.h,list(APPEND SOUNDS KS0164)
###################################################

if("KS0164" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/ks0164.cpp
		${MAME_DIR}/src/devices/sound/ks0164.h
	)
endif()

###################################################
##
##@src/devices/sound/rp2c33_snd.h,list(APPEND SOUNDS RP2C33_SOUND)
###################################################

if("RP2C33_SOUND" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/rp2c33_snd.cpp
		${MAME_DIR}/src/devices/sound/rp2c33_snd.h
	)
endif()

###################################################
##
##@src/devices/sound/tt5665.h,list(APPEND SOUNDS TT5665)
###################################################

if("TT5665" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/tt5665.cpp
		${MAME_DIR}/src/devices/sound/tt5665.h
	)
endif()

###################################################
##
##@src/devices/sound/uda1344.h,list(APPEND SOUNDS UDA1344)
###################################################

if("UDA1344" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/uda1344.cpp
		${MAME_DIR}/src/devices/sound/uda1344.h
	)
endif()

###################################################
##
##@src/devices/sound/lynx.h,list(APPEND SOUNDS LYNX)
###################################################

if("LYNX" IN_LIST SOUNDS)
	list(APPEND SOUND_SRCS
		${MAME_DIR}/src/devices/sound/lynx.cpp
		${MAME_DIR}/src/devices/sound/lynx.h
	)
endif()
