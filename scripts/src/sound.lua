---------------------------------------------------------------------------
--
--   sound.lua
--
--   Rules for building sound cores
--
--   Copyright Nicola Salmoria and the MAME Team.
--   Visit http://mamedev.org for licensing and usage restrictions.
--
----------------------------------------------------------------------------


---------------------------------------------------
-- DACs
--@src/emu/sound/dac.h,SOUNDS += DAC
--@src/emu/sound/dmadac.h,SOUNDS += DMADAC
--@src/emu/sound/speaker.h,SOUNDS += SPEAKER
--@src/emu/sound/beep.h,SOUNDS += BEEP
---------------------------------------------------

if (SOUNDS["DAC"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/dac.*",
	}
end

if (SOUNDS["DMADAC"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/dmadac.*",
	}
end

if (SOUNDS["SPEAKER"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/speaker.*",
	}
end

if (SOUNDS["BEEP"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/beep.*",
	}
end



---------------------------------------------------
-- CD audio
--@src/emu/sound/cdda.h,SOUNDS += CDDA
---------------------------------------------------

if (SOUNDS["CDDA"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/cdda.*",
	}
end



---------------------------------------------------
-- Discrete component audio
--@src/emu/sound/discrete.h,SOUNDS += DISCRETE
---------------------------------------------------

if (SOUNDS["DISCRETE"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/discrete.*",
	}
end

---------------------------------------------------
-- AC97
--@src/emu/sound/pic-ac97.h,SOUNDS += AC97
---------------------------------------------------

if (SOUNDS["AC97"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/pci-ac97.*",
	}
end



---------------------------------------------------
-- Apple custom sound chips
--@src/emu/sound/asc.h,SOUNDS += ASC
--@src/emu/sound/awacs.h,SOUNDS += AWACS
---------------------------------------------------

if (SOUNDS["ASC"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/asc.*",
	}
end

if (SOUNDS["AWACS"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/awacs.*",
	}
end


---------------------------------------------------
-- Atari custom sound chips
--@src/emu/sound/pokey.h,SOUNDS += POKEY
--@src/emu/sound/tiaintf.h,SOUNDS += TIA
---------------------------------------------------

if (SOUNDS["POKEY"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/pokey.*",
	}
end

if (SOUNDS["TIA"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/tiasound.*", 
		MAME_DIR .. "src/emu/sound/tiaintf.*",
	}
end



---------------------------------------------------
-- Amiga audio hardware
--@src/emu/sound/amiga.h,SOUNDS += AMIGA
---------------------------------------------------

if (SOUNDS["AMIGA"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/amiga.*",
	}
end



---------------------------------------------------
-- Bally Astrocade sound system
--@src/emu/sound/astrocde.h,SOUNDS += ASTROCADE
---------------------------------------------------

if (SOUNDS["ASTROCADE"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/astrocde.*",
	}
end



---------------------------------------------------
---------------------------------------------------
-- AC97
--@src/emu/sound/pic-ac97.h,SOUNDS += AC97
---------------------------------------------------

if (SOUNDS["AC97"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/pci-ac97.*",
	}
end
-- CEM 3394 analog synthesizer chip
--@src/emu/sound/cem3394.h,SOUNDS += CEM3394
---------------------------------------------------

if (SOUNDS["CEM3394"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/cem3394.*",
	}
end



---------------------------------------------------
-- Creative Labs SB0400 Audigy2 Value
--@src/emu/sound/sb0400.h,SOUNDS += AC97
---------------------------------------------------

if (SOUNDS["SB0400"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/sb0400.*",
	}
end


--------------------------------------------------
-- Creative Labs Ensonic AudioPCI97 ES1373
--@src/emu/sound/es1373.h,SOUNDS += ES1373
--------------------------------------------------

if (SOUNDS["ES1373"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/es1373.*",
	}
end

---------------------------------------------------
-- Data East custom sound chips
--@src/emu/sound/bsmt2000.h,SOUNDS += BSMT2000
---------------------------------------------------

if (SOUNDS["BSMT2000"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/bsmt2000.*",
	}
end



---------------------------------------------------
-- Ensoniq 5503 (Apple IIgs)
--@src/emu/sound/es5503.h,SOUNDS += ES5503
---------------------------------------------------

if (SOUNDS["ES5503"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/es5503.*",
	}
end



---------------------------------------------------
-- Ensoniq 5505/5506
--@src/emu/sound/es5506.h,SOUNDS += ES5505
---------------------------------------------------

if (SOUNDS["ES5505"]~=null or SOUNDS["ES5506"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/es5506.*",
	}
end


---------------------------------------------------
-- Ensoniq "pump" device, interfaces 5505/5506 with 5510
--@src/emu/sound/esqpump.h,SOUNDS += ESQPUMP
---------------------------------------------------

if (SOUNDS["ESQPUMP"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/esqpump.*",
	}
end


---------------------------------------------------
-- Excellent Systems ADPCM sound chip
--@src/emu/sound/es8712.h,SOUNDS += ES8712
---------------------------------------------------

if (SOUNDS["ES8712"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/es8712.*",
	}
end



---------------------------------------------------
-- Gaelco custom sound chips
--@src/emu/sound/gaelco.h,SOUNDS += GAELCO_CG1V
---------------------------------------------------

if (SOUNDS["GAELCO_CG1V"]~=null or SOUNDS["GAELCO_GAE1"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/gaelco.*",
	}
end


---------------------------------------------------
-- RCA CDP1863
--@src/emu/sound/cdp1863.h,SOUNDS += CDP1863
---------------------------------------------------

if (SOUNDS["CDP1863"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/cdp1863.*",
	}
end



---------------------------------------------------
-- RCA CDP1864
--@src/emu/sound/cdp1864.h,SOUNDS += CDP1864
---------------------------------------------------

if (SOUNDS["CDP1864"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/cdp1864.*",
	}
end



---------------------------------------------------
-- RCA CDP1869
--@src/emu/sound/cdp1869.h,SOUNDS += CDP1869
---------------------------------------------------

if (SOUNDS["CDP1869"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/cdp1869.*",
	}
end



---------------------------------------------------
-- GI AY-8910
--@src/emu/sound/ay8910.h,SOUNDS += AY8910
---------------------------------------------------

if (SOUNDS["AY8910"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/ay8910.*",
	}
end



---------------------------------------------------
-- Harris HC55516 CVSD
--@src/emu/sound/hc55516.h,SOUNDS += HC55516
---------------------------------------------------

if (SOUNDS["HC55516"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/hc55516.*",
	}
end



---------------------------------------------------
-- Hudsonsoft C6280 sound chip
--@src/emu/sound/c6280.h,SOUNDS += C6280
---------------------------------------------------

if (SOUNDS["C6280"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/c6280.*",
	}
end



---------------------------------------------------
-- ICS2115 sound chip
--@src/emu/sound/ics2115.h,SOUNDS += ICS2115
---------------------------------------------------

if (SOUNDS["ICS2115"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/ics2115.*",
	}
end



---------------------------------------------------
-- Imagetek I5000 sound
--@src/emu/sound/i5000.h,SOUNDS += I5000_SND
---------------------------------------------------

if (SOUNDS["I5000_SND"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/i5000.*",
	}
end



---------------------------------------------------
-- Irem custom sound chips
--@src/emu/sound/iremga20.h,SOUNDS += IREMGA20
---------------------------------------------------

if (SOUNDS["IREMGA20"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/iremga20.*",
	}
end



---------------------------------------------------
-- Konami custom sound chips
--@src/emu/sound/k005289.h,SOUNDS += K005289
--@src/emu/sound/k007232.h,SOUNDS += K007232
--@src/emu/sound/k051649.h,SOUNDS += K051649
--@src/emu/sound/k053260.h,SOUNDS += K053260
--@src/emu/sound/k054539.h,SOUNDS += K054539
--@src/emu/sound/k056800.h,SOUNDS += K056800
---------------------------------------------------

if (SOUNDS["K005289"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/k005289.*",
	}
end

if (SOUNDS["K007232"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/k007232.*",
	}
end

if (SOUNDS["K051649"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/k051649.*",
	}
end

if (SOUNDS["K053260"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/k053260.*",
	}
end

if (SOUNDS["K054539"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/k054539.*",
	}
end

if (SOUNDS["K056800"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/k056800.*",
	}
end



---------------------------------------------------
-- LMC1992 mixer chip
--@src/emu/sound/lmc1992.h,SOUNDS += LMC1992
---------------------------------------------------

if (SOUNDS["LMC1992"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/lmc1992.*",
	}
end



---------------------------------------------------
-- MAS 3507D MPEG 1/2 Layer 2/3 Audio Decoder
--@src/emu/sound/mas3507d.h,SOUNDS += MAS3507D
---------------------------------------------------

if (SOUNDS["MAS3507D"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/mas3507d.*",
	}
end



---------------------------------------------------
-- MOS 6560VIC
--@src/emu/sound/mos6560.h,SOUNDS += MOS656X
---------------------------------------------------

if (SOUNDS["MOS656X"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/mos6560.*",
	}
end



---------------------------------------------------
-- MOS 7360 TED
--@src/emu/sound/mos7360.h,SOUNDS += MOS7360
---------------------------------------------------

if (SOUNDS["MOS7360"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/mos7360.*",
	}
end



---------------------------------------------------
-- Namco custom sound chips
--@src/emu/sound/namco.h,SOUNDS += NAMCO
--@src/emu/sound/n63701x.h,SOUNDS += NAMCO_63701X
--@src/emu/sound/c140.h,SOUNDS += C140
--@src/emu/sound/c352.h,SOUNDS += C352
---------------------------------------------------

if (SOUNDS["NAMCO"]~=null or SOUNDS["NAMCO_15XX"]~=null or SOUNDS["NAMCO_CUS30"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/namco.*",
	}
end

if (SOUNDS["NAMCO_63701X"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/n63701x.*",
	}
end

if (SOUNDS["C140"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/c140.*",
	}
end

if (SOUNDS["C352"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/c352.*",
	}
end



---------------------------------------------------
-- National Semiconductor Digitalker
--@src/emu/sound/digitalk.h,SOUNDS += DIGITALKER
---------------------------------------------------

if (SOUNDS["DIGITALKER"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/digitalk.*",
	}
end



---------------------------------------------------
-- Nintendo custom sound chips
--@src/emu/sound/nes_apu.h,SOUNDS += NES_APU
---------------------------------------------------

if (SOUNDS["NES_APU"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/nes_apu.*",
	}
end



---------------------------------------------------
-- NEC uPD7759 ADPCM sample player
--@src/emu/sound/upd7759.h,SOUNDS += UPD7759
---------------------------------------------------

if (SOUNDS["UPD7759"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/upd7759.*",
	}
end



---------------------------------------------------
-- OKI ADPCM sample players
--@src/emu/sound/okim6258.h,SOUNDS += OKIM6258
--@src/emu/sound/msm5205.h,SOUNDS += MSM5205
--@src/emu/sound/msm5232.h,SOUNDS += MSM5232
--@src/emu/sound/okim6376.h,SOUNDS += OKIM6376
--@src/emu/sound/okim6295.h,SOUNDS += OKIM6295
--@src/emu/sound/okim9810.h,SOUNDS += OKIM9810
---------------------------------------------------

if (SOUNDS["OKIM6258"]~=null or SOUNDS["OKIM6295"]~=null or SOUNDS["OKIM9810"]~=null or SOUNDS["I5000_SND"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/okiadpcm.*",
	}
end

if (SOUNDS["MSM5205"]~=null or SOUNDS["MSM6585"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/msm5205.*",
	}
end

if (SOUNDS["MSM5232"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/msm5232.*",
	}
end

if (SOUNDS["OKIM6376"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/okim6376.*",
	}
end

if (SOUNDS["OKIM6295"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/okim6295.*",
	}
end

if (SOUNDS["OKIM6258"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/okim6258.*",
	}
end

if (SOUNDS["OKIM9810"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/okim9810.*",
	}
end



---------------------------------------------------
-- Philips SAA1099
--@src/emu/sound/saa1099.h,SOUNDS += SAA1099
---------------------------------------------------

if (SOUNDS["SAA1099"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/saa1099.*",
	}
end



---------------------------------------------------
-- AdMOS QS1000
--@src/emu/sound/qs1000.h,SOUNDS += QS1000
---------------------------------------------------

if (SOUNDS["QS1000"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/qs1000.*",
	}
end



---------------------------------------------------
-- QSound sample player
--@src/emu/sound/qsound.h,SOUNDS += QSOUND
---------------------------------------------------

if (SOUNDS["QSOUND"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/qsound.*", 
		MAME_DIR .. "src/emu/cpu/dsp16/dsp16.*",
		MAME_DIR .. "src/emu/cpu/dsp16/dsp16dis.*",
	}
end



---------------------------------------------------
-- Ricoh sample players
--@src/emu/sound/rf5c68.h,SOUNDS += RF5C68
--@src/emu/sound/rf5c400.h,SOUNDS += RF5C400
---------------------------------------------------

if (SOUNDS["RF5C68"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/rf5c68.*",
	}
end

if (SOUNDS["RF5C400"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/rf5c400.*",
	}
end



---------------------------------------------------
-- Sega custom sound chips
--@src/emu/sound/segapcm.h,SOUNDS += SEGAPCM
--@src/emu/sound/multipcm.h,SOUNDS += MULTIPCM
--@src/emu/sound/scsp.h,SOUNDS += SCSP
--@src/emu/sound/aica.h,SOUNDS += AICA
---------------------------------------------------

if (SOUNDS["SEGAPCM"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/segapcm.*",
	}
end

if (SOUNDS["MULTIPCM"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/multipcm.*",
	}
end

if (SOUNDS["SCSP"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/scsp.*",
		MAME_DIR .. "src/emu/sound/scspdsp.*",
	}
end

if (SOUNDS["AICA"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/aica.*",
		MAME_DIR .. "src/emu/sound/aicadsp.*",
	}
end

---------------------------------------------------
-- Seta custom sound chips
--@src/emu/sound/st0016.h,SOUNDS += ST0016
--@src/emu/sound/nile.h,SOUNDS += NILE
--@src/emu/sound/x1_010.h,SOUNDS += X1_010
---------------------------------------------------

if (SOUNDS["ST0016"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/st0016.*",
	}
end

if (SOUNDS["NILE"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/nile.*",
	}
end

if (SOUNDS["X1_010"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/x1_010.*",
	}
end



---------------------------------------------------
-- SID custom sound chips
--@src/emu/sound/mos6581.h,SOUNDS += SID6581
---------------------------------------------------

if (SOUNDS["SID6581"]~=null or SOUNDS["SID8580"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/mos6581.*",
		MAME_DIR .. "src/emu/sound/sid.*",
		MAME_DIR .. "src/emu/sound/sidenvel.*",
		MAME_DIR .. "src/emu/sound/sidvoice.*",
	}
end


---------------------------------------------------
-- SNK(?) custom stereo sn76489a clone
--@src/emu/sound/t6w28.h,SOUNDS += T6W28
---------------------------------------------------

if (SOUNDS["T6W28"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/t6w28.*",
	}
end



---------------------------------------------------
-- SNK custom wave generator
--@src/emu/sound/snkwave.h,SOUNDS += SNKWAVE
---------------------------------------------------

if (SOUNDS["SNKWAVE"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/snkwave.*",
	}
end



---------------------------------------------------
-- Sony custom sound chips
--@src/emu/sound/spu.h,SOUNDS += SPU
---------------------------------------------------

if (SOUNDS["SPU"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/spu.*",
		MAME_DIR .. "src/emu/sound/spu_tables.*",
		MAME_DIR .. "src/emu/sound/spureverb.*",
	}
end


---------------------------------------------------
-- SP0256 speech synthesizer
--@src/emu/sound/sp0256.h,SOUNDS += SP0256
---------------------------------------------------

if (SOUNDS["SP0256"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/sp0256.*",
	}
end



---------------------------------------------------
-- SP0250 speech synthesizer
--@src/emu/sound/sp0250.h,SOUNDS += SP0250
---------------------------------------------------

if (SOUNDS["SP0250"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/sp0250.*",
	}
end


---------------------------------------------------
-- S14001A speech synthesizer
--@src/emu/sound/s14001a.h,SOUNDS += S14001A
---------------------------------------------------

if (SOUNDS["S14001A"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/s14001a.*",
	}
end



---------------------------------------------------
-- Texas Instruments SN76477 analog chip
--@src/emu/sound/sn76477.h,SOUNDS += SN76477
---------------------------------------------------

if (SOUNDS["SN76477"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/sn76477.*",
	}
end



---------------------------------------------------
-- Texas Instruments SN76496
--@src/emu/sound/sn76496.h,SOUNDS += SN76496
---------------------------------------------------

if (SOUNDS["SN76496"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/sn76496.*",
	}
end



---------------------------------------------------
-- Texas Instruments TMS36xx doorbell chime
--@src/emu/sound/tms36xx.h,SOUNDS += TMS36XX
---------------------------------------------------

if (SOUNDS["TMS36XX"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/tms36xx.*",
	}
end



---------------------------------------------------
-- Texas Instruments TMS3615 Octave Multiple Tone Synthesizer
--@src/emu/sound/tms3615.h,SOUNDS += TMS3615
---------------------------------------------------

if (SOUNDS["TMS3615"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/tms3615.*",
	}
end



---------------------------------------------------
-- Texas Instruments TMS5100-series speech synthesizers
--@src/emu/sound/tms5110.h,SOUNDS += TMS5110
---------------------------------------------------

if (SOUNDS["TMS5110"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/tms5110.*",
	}
end

---------------------------------------------------
-- Texas Instruments TMS5200-series speech synthesizers
--@src/emu/sound/tms5220.h,SOUNDS += TMS5220
---------------------------------------------------
if (SOUNDS["TMS5220"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/tms5220.*",
		MAME_DIR .. "src/emu/machine/spchrom.*",
	}
end


---------------------------------------------------
-- Toshiba T6721A voice synthesizer
--@src/emu/sound/t6721a.h,SOUNDS += T6721A
---------------------------------------------------

if (SOUNDS["T6721A"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/t6721a.*",
	}
end



---------------------------------------------------
-- Toshiba TC8830F sample player/recorder
--@src/emu/sound/tc8830f.h,SOUNDS += TC8830F
---------------------------------------------------

if (SOUNDS["TC8830F"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/tc8830f.*",
	}
end


---------------------------------------------------
-- NEC uPD7752
--@src/emu/sound/upd7752.h,SOUNDS += UPD7752
---------------------------------------------------

if (SOUNDS["UPD7752"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/upd7752.*",
	}
end


---------------------------------------------------
-- VLM5030 speech synthesizer
--@src/emu/sound/vlm5030.h,SOUNDS += VLM5030
---------------------------------------------------

if (SOUNDS["VLM5030"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/vlm5030.*",
	}
end

---------------------------------------------------
-- Votrax speech synthesizer
--@src/emu/sound/votrax.h,SOUNDS += VOTRAX
---------------------------------------------------

if (SOUNDS["VOTRAX"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/votrax.*",
		MAME_DIR .. "src/emu/sound/samples.*",
	}
end



---------------------------------------------------
-- VRender0 custom sound chip
--@src/emu/sound/vrender0.h,SOUNDS += VRENDER0
---------------------------------------------------

if (SOUNDS["VRENDER0"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/vrender0.*",
	}
end



---------------------------------------------------
-- WAVE file (used for MESS cassette)
--@src/emu/sound/wave.h,SOUNDS += WAVE
---------------------------------------------------

if (SOUNDS["WAVE"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/wave.*",
	}
end



---------------------------------------------------
-- Yamaha FM synthesizers
--@src/emu/sound/2151intf.h,SOUNDS += YM2151
--@src/emu/sound/2203intf.h,SOUNDS += YM2203
--@src/emu/sound/2413intf.h,SOUNDS += YM2413
--@src/emu/sound/2608intf.h,SOUNDS += YM2608
--@src/emu/sound/2610intf.h,SOUNDS += YM2610
--@src/emu/sound/2612intf.h,SOUNDS += YM2612
--@src/emu/sound/3812intf.h,SOUNDS += YM3812
--@src/emu/sound/3526intf.h,SOUNDS += YM3526
--@src/emu/sound/8950intf.h,SOUNDS += Y8950
--@src/emu/sound/ymf262.h,SOUNDS += YMF262
--@src/emu/sound/ymf271.h,SOUNDS += YMF271
--@src/emu/sound/ymf278b.h,SOUNDS += YMF278B
---------------------------------------------------

if (SOUNDS["YM2151"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/2151intf.*",
		MAME_DIR .. "src/emu/sound/ym2151.*",
	}
end

if (SOUNDS["YM2203"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/2203intf.*",
		MAME_DIR .. "src/emu/sound/ay8910.*",
		MAME_DIR .. "src/emu/sound/fm.*",
	}
end

if (SOUNDS["YM2413"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/2413intf.*",
		MAME_DIR .. "src/emu/sound/ym2413.*",
	}
end

if (SOUNDS["YM2608"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/2608intf.*", 
		MAME_DIR .. "src/emu/sound/ay8910.*", 
		MAME_DIR .. "src/emu/sound/fm.*", 
		MAME_DIR .. "src/emu/sound/ymdeltat.*",
	}
end

if (SOUNDS["YM2610"]~=null or SOUNDS["YM2610B"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/2610intf.*", 
		MAME_DIR .. "src/emu/sound/ay8910.*", 
		MAME_DIR .. "src/emu/sound/fm.*", 
		MAME_DIR .. "src/emu/sound/ymdeltat.*",
	}
end

if (SOUNDS["YM2612"]~=null or SOUNDS["YM3438"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/2612intf.*", 
		MAME_DIR .. "src/emu/sound/ay8910.*", 
		MAME_DIR .. "src/emu/sound/fm2612.*",
	}
end

if (SOUNDS["YM3812"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/3812intf.*", 
		MAME_DIR .. "src/emu/sound/fmopl.*", 
		MAME_DIR .. "src/emu/sound/ymdeltat.*",
	}
end

if (SOUNDS["YM3526"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/3526intf.*", 
		MAME_DIR .. "src/emu/sound/fmopl.*", 
		MAME_DIR .. "src/emu/sound/ymdeltat.*",
	}
end

if (SOUNDS["Y8950"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/8950intf.*", 
		MAME_DIR .. "src/emu/sound/fmopl.*", 
		MAME_DIR .. "src/emu/sound/ymdeltat.*",
	}
end

if (SOUNDS["YMF262"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/ymf262.*", 
		MAME_DIR .. "src/emu/sound/262intf.*",
	}
end

if (SOUNDS["YMF271"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/ymf271.*",
	}
end

if (SOUNDS["YMF278B"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/ymf278b.*",
	}
end



---------------------------------------------------
-- Yamaha YMZ280B ADPCM
--@src/emu/sound/ymz280b.h,SOUNDS += YMZ280B
---------------------------------------------------

if (SOUNDS["YMZ280B"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/ymz280b.*",
	}
end

---------------------------------------------------
-- Yamaha YMZ770 AMM
--@src/emu/sound/ymz770.h,SOUNDS += YMZ770
---------------------------------------------------

if (SOUNDS["YMZ770"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/ymz770.*", 
		MAME_DIR .. "src/emu/sound/mpeg_audio.*",
	}
end

---------------------------------------------------
-- ZOOM ZSG-2
--@src/emu/sound/zsg2.h,SOUNDS += ZSG2
---------------------------------------------------

if (SOUNDS["ZSG2"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/zsg2.*",
	}
end

---------------------------------------------------
-- VRC6
--@src/emu/sound/vrc6.h,SOUNDS += VRC6
---------------------------------------------------

if (SOUNDS["VRC6"]~=null) then
	files {
		MAME_DIR .. "src/emu/sound/vrc6.*",
	}
end

