-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   sound.lua
--
--   Rules for building sound cores
--
----------------------------------------------------------------------------


---------------------------------------------------
-- DACs
--@src/devices/sound/dac.h,SOUNDS["DAC"] = true
--@src/devices/sound/dmadac.h,SOUNDS["DMADAC"] = true
--@src/devices/sound/speaker.h,SOUNDS["SPEAKER"] = true
--@src/devices/sound/beep.h,SOUNDS["BEEP"] = true
---------------------------------------------------

if (SOUNDS["DAC"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/dac.c",
		MAME_DIR .. "src/devices/sound/dac.h",
	}
end

if (SOUNDS["DMADAC"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/dmadac.c",
		MAME_DIR .. "src/devices/sound/dmadac.h",
	}
end

if (SOUNDS["SPEAKER"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/speaker.c",
		MAME_DIR .. "src/devices/sound/speaker.h",
	}
end

if (SOUNDS["BEEP"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/beep.c",
		MAME_DIR .. "src/devices/sound/beep.h",
	}
end



---------------------------------------------------
-- CD audio
--@src/devices/sound/cdda.h,SOUNDS["CDDA"] = true
---------------------------------------------------

if (SOUNDS["CDDA"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/cdda.c",
		MAME_DIR .. "src/devices/sound/cdda.h",
	}
end



---------------------------------------------------
-- Discrete component audio
--@src/devices/sound/discrete.h,SOUNDS["DISCRETE"] = true
---------------------------------------------------

if (SOUNDS["DISCRETE"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/discrete.c",
		MAME_DIR .. "src/devices/sound/discrete.h",
		MAME_DIR .. "src/devices/sound/disc_cls.h",
		MAME_DIR .. "src/devices/sound/disc_dev.h",
		MAME_DIR .. "src/devices/sound/disc_dev.inc",
		MAME_DIR .. "src/devices/sound/disc_flt.h",
		MAME_DIR .. "src/devices/sound/disc_flt.inc",
		MAME_DIR .. "src/devices/sound/disc_inp.inc",
		MAME_DIR .. "src/devices/sound/disc_mth.h",
		MAME_DIR .. "src/devices/sound/disc_mth.inc",
		MAME_DIR .. "src/devices/sound/disc_sys.inc",
		MAME_DIR .. "src/devices/sound/disc_wav.h",
		MAME_DIR .. "src/devices/sound/disc_wav.inc", 
	}
end

---------------------------------------------------
-- AC97
--@src/devices/sound/pic-ac97.h,SOUNDS["AC97"] = true
---------------------------------------------------

if (SOUNDS["AC97"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/pci-ac97.c",
		MAME_DIR .. "src/devices/sound/pci-ac97.h",
	}
end



---------------------------------------------------
-- Apple custom sound chips
--@src/devices/sound/asc.h,SOUNDS["ASC"] = true
--@src/devices/sound/awacs.h,SOUNDS["AWACS"] = true
---------------------------------------------------

if (SOUNDS["ASC"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/asc.c",
		MAME_DIR .. "src/devices/sound/asc.h",
	}
end

if (SOUNDS["AWACS"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/awacs.c",
		MAME_DIR .. "src/devices/sound/awacs.h",
	}
end


---------------------------------------------------
-- Atari custom sound chips
--@src/devices/sound/pokey.h,SOUNDS["POKEY"] = true
--@src/devices/sound/tiaintf.h,SOUNDS["TIA"] = true
---------------------------------------------------

if (SOUNDS["POKEY"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/pokey.c",
		MAME_DIR .. "src/devices/sound/pokey.h",
	}
end

if (SOUNDS["TIA"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tiasound.c", 
		MAME_DIR .. "src/devices/sound/tiasound.h", 
		MAME_DIR .. "src/devices/sound/tiaintf.c",
		MAME_DIR .. "src/devices/sound/tiaintf.h",
	}
end



---------------------------------------------------
-- Amiga audio hardware
--@src/devices/sound/amiga.h,SOUNDS["AMIGA"] = true
---------------------------------------------------

if (SOUNDS["AMIGA"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/amiga.c",
		MAME_DIR .. "src/devices/sound/amiga.h",
	}
end



---------------------------------------------------
-- Bally Astrocade sound system
--@src/devices/sound/astrocde.h,SOUNDS["ASTROCADE"] = true
---------------------------------------------------

if (SOUNDS["ASTROCADE"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/astrocde.c",
		MAME_DIR .. "src/devices/sound/astrocde.h",
	}
end



---------------------------------------------------
---------------------------------------------------
-- AC97
--@src/devices/sound/pic-ac97.h,SOUNDS["AC97"] = true
---------------------------------------------------

if (SOUNDS["AC97"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/pci-ac97.c",
		MAME_DIR .. "src/devices/sound/pci-ac97.h",
	}
end
-- CEM 3394 analog synthesizer chip
--@src/devices/sound/cem3394.h,SOUNDS["CEM3394"] = true
---------------------------------------------------

if (SOUNDS["CEM3394"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/cem3394.c",
		MAME_DIR .. "src/devices/sound/cem3394.h",
	}
end



---------------------------------------------------
-- Creative Labs SB0400 Audigy2 Value
--@src/devices/sound/sb0400.h,SOUNDS["SB0400"] = true
---------------------------------------------------

if (SOUNDS["SB0400"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/sb0400.c",
		MAME_DIR .. "src/devices/sound/sb0400.h",
	}
end


--------------------------------------------------
-- Creative Labs Ensonic AudioPCI97 ES1373
--@src/devices/sound/es1373.h,SOUNDS["ES1373"] = true
--------------------------------------------------

if (SOUNDS["ES1373"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/es1373.c",
		MAME_DIR .. "src/devices/sound/es1373.h",
	}
end

---------------------------------------------------
-- Data East custom sound chips
--@src/devices/sound/bsmt2000.h,SOUNDS["BSMT2000"] = true
---------------------------------------------------

if (SOUNDS["BSMT2000"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/bsmt2000.c",
		MAME_DIR .. "src/devices/sound/bsmt2000.h",
	}
end



---------------------------------------------------
-- Ensoniq 5503 (Apple IIgs)
--@src/devices/sound/es5503.h,SOUNDS["ES5503"] = true
---------------------------------------------------

if (SOUNDS["ES5503"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/es5503.c",
		MAME_DIR .. "src/devices/sound/es5503.h",
	}
end



---------------------------------------------------
-- Ensoniq 5505/5506
--@src/devices/sound/es5506.h,SOUNDS["ES5505"] = true
---------------------------------------------------

if (SOUNDS["ES5505"]~=null or SOUNDS["ES5506"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/es5506.c",
		MAME_DIR .. "src/devices/sound/es5506.h",
	}
end


---------------------------------------------------
-- Ensoniq "pump" device, interfaces 5505/5506 with 5510
--@src/devices/sound/esqpump.h,SOUNDS["ESQPUMP"] = true
---------------------------------------------------

if (SOUNDS["ESQPUMP"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/esqpump.c",
		MAME_DIR .. "src/devices/sound/esqpump.h",
	}
end


---------------------------------------------------
-- Excellent Systems ADPCM sound chip
--@src/devices/sound/es8712.h,SOUNDS["ES8712"] = true
---------------------------------------------------

if (SOUNDS["ES8712"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/es8712.c",
		MAME_DIR .. "src/devices/sound/es8712.h",
	}
end



---------------------------------------------------
-- Gaelco custom sound chips
--@src/devices/sound/gaelco.h,SOUNDS["GAELCO_CG1V"] = true
---------------------------------------------------

if (SOUNDS["GAELCO_CG1V"]~=null or SOUNDS["GAELCO_GAE1"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/gaelco.c",
		MAME_DIR .. "src/devices/sound/gaelco.h",
	}
end


---------------------------------------------------
-- RCA CDP1863
--@src/devices/sound/cdp1863.h,SOUNDS["CDP1863"] = true
---------------------------------------------------

if (SOUNDS["CDP1863"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/cdp1863.c",
		MAME_DIR .. "src/devices/sound/cdp1863.h",
	}
end



---------------------------------------------------
-- RCA CDP1864
--@src/devices/sound/cdp1864.h,SOUNDS["CDP1864"] = true
---------------------------------------------------

if (SOUNDS["CDP1864"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/cdp1864.c",
		MAME_DIR .. "src/devices/sound/cdp1864.h",
	}
end



---------------------------------------------------
-- RCA CDP1869
--@src/devices/sound/cdp1869.h,SOUNDS["CDP1869"] = true
---------------------------------------------------

if (SOUNDS["CDP1869"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/cdp1869.c",
		MAME_DIR .. "src/devices/sound/cdp1869.h",
	}
end



---------------------------------------------------
-- GI AY-8910
--@src/devices/sound/ay8910.h,SOUNDS["AY8910"] = true
---------------------------------------------------

if (SOUNDS["AY8910"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ay8910.c",
		MAME_DIR .. "src/devices/sound/ay8910.h",
	}
end



---------------------------------------------------
-- Harris HC55516 CVSD
--@src/devices/sound/hc55516.h,SOUNDS["HC55516"] = true
---------------------------------------------------

if (SOUNDS["HC55516"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/hc55516.c",
		MAME_DIR .. "src/devices/sound/hc55516.h",
	}
end



---------------------------------------------------
-- Hudsonsoft C6280 sound chip
--@src/devices/sound/c6280.h,SOUNDS["C6280"] = true
---------------------------------------------------

if (SOUNDS["C6280"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/c6280.c",
		MAME_DIR .. "src/devices/sound/c6280.h",
	}
end



---------------------------------------------------
-- ICS2115 sound chip
--@src/devices/sound/ics2115.h,SOUNDS["ICS2115"] = true
---------------------------------------------------

if (SOUNDS["ICS2115"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ics2115.c",
		MAME_DIR .. "src/devices/sound/ics2115.h",
	}
end



---------------------------------------------------
-- Imagetek I5000 sound
--@src/devices/sound/i5000.h,SOUNDS["I5000_SND"] = true
---------------------------------------------------

if (SOUNDS["I5000_SND"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/i5000.c",
		MAME_DIR .. "src/devices/sound/i5000.h",
	}
end



---------------------------------------------------
-- Irem custom sound chips
--@src/devices/sound/iremga20.h,SOUNDS["IREMGA20"] = true
---------------------------------------------------

if (SOUNDS["IREMGA20"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/iremga20.c",
		MAME_DIR .. "src/devices/sound/iremga20.h",
	}
end



---------------------------------------------------
-- Konami custom sound chips
--@src/devices/sound/k005289.h,SOUNDS["K005289"] = true
--@src/devices/sound/k007232.h,SOUNDS["K007232"] = true
--@src/devices/sound/k051649.h,SOUNDS["K051649"] = true
--@src/devices/sound/k053260.h,SOUNDS["K053260"] = true
--@src/devices/sound/k054539.h,SOUNDS["K054539"] = true
--@src/devices/sound/k056800.h,SOUNDS["K056800"] = true
---------------------------------------------------

if (SOUNDS["K005289"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/k005289.c",
		MAME_DIR .. "src/devices/sound/k005289.h",
	}
end

if (SOUNDS["K007232"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/k007232.c",
		MAME_DIR .. "src/devices/sound/k007232.h",
	}
end

if (SOUNDS["K051649"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/k051649.c",
		MAME_DIR .. "src/devices/sound/k051649.h",
	}
end

if (SOUNDS["K053260"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/k053260.c",
		MAME_DIR .. "src/devices/sound/k053260.h",
	}
end

if (SOUNDS["K054539"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/k054539.c",
		MAME_DIR .. "src/devices/sound/k054539.h",
	}
end

if (SOUNDS["K056800"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/k056800.c",
		MAME_DIR .. "src/devices/sound/k056800.h",
	}
end


---------------------------------------------------
-- L7A1045 L6028 DSP-A 
--@src/devices/sound/l7a1045_l6028_dsp_a.h,SOUNDS["L7A1045"] = true
---------------------------------------------------

if (SOUNDS["L7A1045"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/l7a1045_l6028_dsp_a.c",
		MAME_DIR .. "src/devices/sound/l7a1045_l6028_dsp_a.h",
	}
end


---------------------------------------------------
-- LMC1992 mixer chip
--@src/devices/sound/lmc1992.h,SOUNDS["LMC1992"] = true
---------------------------------------------------

if (SOUNDS["LMC1992"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/lmc1992.c",
		MAME_DIR .. "src/devices/sound/lmc1992.h",
	}
end



---------------------------------------------------
-- MAS 3507D MPEG 1/2 Layer 2/3 Audio Decoder
--@src/devices/sound/mas3507d.h,SOUNDS["MAS3507D"] = true
---------------------------------------------------

if (SOUNDS["MAS3507D"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/mas3507d.c",
		MAME_DIR .. "src/devices/sound/mas3507d.h",
	}
end



---------------------------------------------------
-- MOS 6560VIC
--@src/devices/sound/mos6560.h,SOUNDS["MOS656X"] = true
---------------------------------------------------

if (SOUNDS["MOS656X"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/mos6560.c",
		MAME_DIR .. "src/devices/sound/mos6560.h",
	}
end



---------------------------------------------------
-- MOS 7360 TED
--@src/devices/sound/mos7360.h,SOUNDS["MOS7360"] = true
---------------------------------------------------

if (SOUNDS["MOS7360"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/mos7360.c",
		MAME_DIR .. "src/devices/sound/mos7360.h",
	}
end



---------------------------------------------------
-- Namco custom sound chips
--@src/devices/sound/namco.h,SOUNDS["NAMCO"] = true
--@src/devices/sound/n63701x.h,SOUNDS["NAMCO_63701X"] = true
--@src/devices/sound/c140.h,SOUNDS["C140"] = true
--@src/devices/sound/c352.h,SOUNDS["C352"] = true
---------------------------------------------------

if (SOUNDS["NAMCO"]~=null or SOUNDS["NAMCO_15XX"]~=null or SOUNDS["NAMCO_CUS30"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/namco.c",
		MAME_DIR .. "src/devices/sound/namco.h",
	}
end

if (SOUNDS["NAMCO_63701X"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/n63701x.c",
		MAME_DIR .. "src/devices/sound/n63701x.h",
	}
end

if (SOUNDS["C140"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/c140.c",
		MAME_DIR .. "src/devices/sound/c140.h",
	}
end

if (SOUNDS["C352"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/c352.c",
		MAME_DIR .. "src/devices/sound/c352.h",
	}
end



---------------------------------------------------
-- National Semiconductor Digitalker
--@src/devices/sound/digitalk.h,SOUNDS["DIGITALKER"] = true
---------------------------------------------------

if (SOUNDS["DIGITALKER"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/digitalk.c",
		MAME_DIR .. "src/devices/sound/digitalk.h",
	}
end



---------------------------------------------------
-- Nintendo custom sound chips
--@src/devices/sound/nes_apu.h,SOUNDS["NES_APU"] = true
---------------------------------------------------

if (SOUNDS["NES_APU"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/nes_apu.c",
		MAME_DIR .. "src/devices/sound/nes_apu.h",
		MAME_DIR .. "src/devices/sound/nes_defs.h",
	}
end



---------------------------------------------------
-- NEC uPD7759 ADPCM sample player
--@src/devices/sound/upd7759.h,SOUNDS["UPD7759"] = true
---------------------------------------------------

if (SOUNDS["UPD7759"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/upd7759.c",
		MAME_DIR .. "src/devices/sound/upd7759.h",
		MAME_DIR .. "src/devices/sound/315-5641.c",
		MAME_DIR .. "src/devices/sound/315-5641.h",
	}
end



---------------------------------------------------
-- OKI ADPCM sample players
--@src/devices/sound/okim6258.h,SOUNDS["OKIM6258"] = true
--@src/devices/sound/msm5205.h,SOUNDS["MSM5205"] = true
--@src/devices/sound/msm5232.h,SOUNDS["MSM5232"] = true
--@src/devices/sound/okim6376.h,SOUNDS["OKIM6376"] = true
--@src/devices/sound/okim6295.h,SOUNDS["OKIM6295"] = true
--@src/devices/sound/okim9810.h,SOUNDS["OKIM9810"] = true
---------------------------------------------------

if (SOUNDS["OKIM6258"]~=null or SOUNDS["OKIM6295"]~=null or SOUNDS["OKIM9810"]~=null or SOUNDS["I5000_SND"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/okiadpcm.c",
		MAME_DIR .. "src/devices/sound/okiadpcm.h",
	}
end

if (SOUNDS["MSM5205"]~=null or SOUNDS["MSM6585"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/msm5205.c",
		MAME_DIR .. "src/devices/sound/msm5205.h",
	}
end

if (SOUNDS["MSM5232"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/msm5232.c",
		MAME_DIR .. "src/devices/sound/msm5232.h",
	}
end

if (SOUNDS["OKIM6376"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/okim6376.c",
		MAME_DIR .. "src/devices/sound/okim6376.h",
	}
end

if (SOUNDS["OKIM6295"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/okim6295.c",
		MAME_DIR .. "src/devices/sound/okim6295.h",
	}
end

if (SOUNDS["OKIM6258"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/okim6258.c",
		MAME_DIR .. "src/devices/sound/okim6258.h",
	}
end

if (SOUNDS["OKIM9810"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/okim9810.c",
		MAME_DIR .. "src/devices/sound/okim9810.h",
	}
end



---------------------------------------------------
-- Philips SAA1099
--@src/devices/sound/saa1099.h,SOUNDS["SAA1099"] = true
---------------------------------------------------

if (SOUNDS["SAA1099"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/saa1099.c",
		MAME_DIR .. "src/devices/sound/saa1099.h",
	}
end



---------------------------------------------------
-- AdMOS QS1000
--@src/devices/sound/qs1000.h,SOUNDS["QS1000"] = true
---------------------------------------------------

if (SOUNDS["QS1000"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/qs1000.c",
		MAME_DIR .. "src/devices/sound/qs1000.h",
	}
end



---------------------------------------------------
-- QSound sample player
--@src/devices/sound/qsound.h,SOUNDS["QSOUND"] = true
---------------------------------------------------

if (SOUNDS["QSOUND"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/qsound.c", 
		MAME_DIR .. "src/devices/sound/qsound.h", 
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16.c",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16.h",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16dis.c",
	}
end



---------------------------------------------------
-- Ricoh sample players
--@src/devices/sound/rf5c68.h,SOUNDS["RF5C68"] = true
--@src/devices/sound/rf5c400.h,SOUNDS["RF5C400"] = true
---------------------------------------------------

if (SOUNDS["RF5C68"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/rf5c68.c",
		MAME_DIR .. "src/devices/sound/rf5c68.h",
	}
end

if (SOUNDS["RF5C400"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/rf5c400.c",
		MAME_DIR .. "src/devices/sound/rf5c400.h",
	}
end



---------------------------------------------------
-- Sega custom sound chips
--@src/devices/sound/segapcm.h,SOUNDS["SEGAPCM"] = true
--@src/devices/sound/multipcm.h,SOUNDS["MULTIPCM"] = true
--@src/devices/sound/scsp.h,SOUNDS["SCSP"] = true
--@src/devices/sound/aica.h,SOUNDS["AICA"] = true
---------------------------------------------------

if (SOUNDS["SEGAPCM"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/segapcm.c",
		MAME_DIR .. "src/devices/sound/segapcm.h",
	}
end

if (SOUNDS["MULTIPCM"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/multipcm.c",
		MAME_DIR .. "src/devices/sound/multipcm.h",
	}
end

if (SOUNDS["SCSP"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/scsp.c",
		MAME_DIR .. "src/devices/sound/scsp.h",
		MAME_DIR .. "src/devices/sound/scspdsp.c",
		MAME_DIR .. "src/devices/sound/scspdsp.h",
	}
end

if (SOUNDS["AICA"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/aica.c",
		MAME_DIR .. "src/devices/sound/aica.h",
		MAME_DIR .. "src/devices/sound/aicadsp.c",
		MAME_DIR .. "src/devices/sound/aicadsp.h",
	}
end

---------------------------------------------------
-- Seta custom sound chips
--@src/devices/sound/st0016.h,SOUNDS["ST0016"] = true
--@src/devices/sound/nile.h,SOUNDS["NILE"] = true
--@src/devices/sound/x1_010.h,SOUNDS["X1_010"] = true
---------------------------------------------------

if (SOUNDS["ST0016"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/st0016.c",
		MAME_DIR .. "src/devices/sound/st0016.h",
	}
end

if (SOUNDS["NILE"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/nile.c",
		MAME_DIR .. "src/devices/sound/nile.h",
	}
end

if (SOUNDS["X1_010"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/x1_010.c",
		MAME_DIR .. "src/devices/sound/x1_010.h",
	}
end



---------------------------------------------------
-- SID custom sound chips
--@src/devices/sound/mos6581.h,SOUNDS["SID6581"] = true
---------------------------------------------------

if (SOUNDS["SID6581"]~=null or SOUNDS["SID8580"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/mos6581.c",
		MAME_DIR .. "src/devices/sound/mos6581.h",
		MAME_DIR .. "src/devices/sound/sid.c",
		MAME_DIR .. "src/devices/sound/sid.h",
		MAME_DIR .. "src/devices/sound/sidenvel.c",
		MAME_DIR .. "src/devices/sound/sidenvel.h",
		MAME_DIR .. "src/devices/sound/sidvoice.c",
		MAME_DIR .. "src/devices/sound/sidvoice.h",
		MAME_DIR .. "src/devices/sound/side6581.h",
		MAME_DIR .. "src/devices/sound/sidw6581.h",
		MAME_DIR .. "src/devices/sound/sidw8580.h",
	}
end


---------------------------------------------------
-- SNK(?) custom stereo sn76489a clone
--@src/devices/sound/t6w28.h,SOUNDS["T6W28"] = true
---------------------------------------------------

if (SOUNDS["T6W28"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/t6w28.c",
		MAME_DIR .. "src/devices/sound/t6w28.h",
	}
end



---------------------------------------------------
-- SNK custom wave generator
--@src/devices/sound/snkwave.h,SOUNDS["SNKWAVE"] = true
---------------------------------------------------

if (SOUNDS["SNKWAVE"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/snkwave.c",
		MAME_DIR .. "src/devices/sound/snkwave.h",
	}
end



---------------------------------------------------
-- Sony custom sound chips
--@src/devices/sound/spu.h,SOUNDS["SPU"] = true
---------------------------------------------------

if (SOUNDS["SPU"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/spu.c",
		MAME_DIR .. "src/devices/sound/spu.h",
		MAME_DIR .. "src/devices/sound/spu_tables.c",
		MAME_DIR .. "src/devices/sound/spureverb.c",
		MAME_DIR .. "src/devices/sound/spureverb.h",
	}
end


---------------------------------------------------
-- SP0256 speech synthesizer
--@src/devices/sound/sp0256.h,SOUNDS["SP0256"] = true
---------------------------------------------------

if (SOUNDS["SP0256"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/sp0256.c",
		MAME_DIR .. "src/devices/sound/sp0256.h",
	}
end



---------------------------------------------------
-- SP0250 speech synthesizer
--@src/devices/sound/sp0250.h,SOUNDS["SP0250"] = true
---------------------------------------------------

if (SOUNDS["SP0250"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/sp0250.c",
		MAME_DIR .. "src/devices/sound/sp0250.h",
	}
end


---------------------------------------------------
-- S14001A speech synthesizer
--@src/devices/sound/s14001a.h,SOUNDS["S14001A"] = true
---------------------------------------------------

if (SOUNDS["S14001A"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/s14001a.c",
		MAME_DIR .. "src/devices/sound/s14001a.h",
	}
end



---------------------------------------------------
-- Texas Instruments SN76477 analog chip
--@src/devices/sound/sn76477.h,SOUNDS["SN76477"] = true
---------------------------------------------------

if (SOUNDS["SN76477"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/sn76477.c",
		MAME_DIR .. "src/devices/sound/sn76477.h",
	}
end



---------------------------------------------------
-- Texas Instruments SN76496
--@src/devices/sound/sn76496.h,SOUNDS["SN76496"] = true
---------------------------------------------------

if (SOUNDS["SN76496"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/sn76496.c",
		MAME_DIR .. "src/devices/sound/sn76496.h",
	}
end



---------------------------------------------------
-- Texas Instruments TMS36xx doorbell chime
--@src/devices/sound/tms36xx.h,SOUNDS["TMS36XX"] = true
---------------------------------------------------

if (SOUNDS["TMS36XX"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tms36xx.c",
		MAME_DIR .. "src/devices/sound/tms36xx.h",
	}
end



---------------------------------------------------
-- Texas Instruments TMS3615 Octave Multiple Tone Synthesizer
--@src/devices/sound/tms3615.h,SOUNDS["TMS3615"] = true
---------------------------------------------------

if (SOUNDS["TMS3615"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tms3615.c",
		MAME_DIR .. "src/devices/sound/tms3615.h",
	}
end



---------------------------------------------------
-- Texas Instruments TMS5100-series speech synthesizers
--@src/devices/sound/tms5110.h,SOUNDS["TMS5110"] = true
---------------------------------------------------

if (SOUNDS["TMS5110"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tms5110.c",
		MAME_DIR .. "src/devices/sound/tms5110.h",
		MAME_DIR .. "src/devices/sound/tms5110r.inc",
	}
end

---------------------------------------------------
-- Texas Instruments TMS5200-series speech synthesizers
--@src/devices/sound/tms5220.h,SOUNDS["TMS5220"] = true
---------------------------------------------------
if (SOUNDS["TMS5220"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tms5220.c",
		MAME_DIR .. "src/devices/sound/tms5220.h",
		MAME_DIR .. "src/devices/sound/tms5110r.inc",
		MAME_DIR .. "src/devices/machine/spchrom.c",
		MAME_DIR .. "src/devices/machine/spchrom.h",
	}
end


---------------------------------------------------
-- Toshiba T6721A voice synthesizer
--@src/devices/sound/t6721a.h,SOUNDS["T6721A"] = true
---------------------------------------------------

if (SOUNDS["T6721A"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/t6721a.c",
		MAME_DIR .. "src/devices/sound/t6721a.h",
	}
end



---------------------------------------------------
-- Toshiba TC8830F sample player/recorder
--@src/devices/sound/tc8830f.h,SOUNDS["TC8830F"] = true
---------------------------------------------------

if (SOUNDS["TC8830F"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tc8830f.c",
		MAME_DIR .. "src/devices/sound/tc8830f.h",
	}
end


---------------------------------------------------
-- NEC uPD7752
--@src/devices/sound/upd7752.h,SOUNDS["UPD7752"] = true
---------------------------------------------------

if (SOUNDS["UPD7752"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/upd7752.c",
		MAME_DIR .. "src/devices/sound/upd7752.h",
	}
end


---------------------------------------------------
-- VLM5030 speech synthesizer
--@src/devices/sound/vlm5030.h,SOUNDS["VLM5030"] = true
---------------------------------------------------

if (SOUNDS["VLM5030"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/vlm5030.c",
		MAME_DIR .. "src/devices/sound/vlm5030.h",
		MAME_DIR .. "src/devices/sound/tms5110r.inc",
	}
end

---------------------------------------------------
-- Votrax speech synthesizer
--@src/devices/sound/votrax.h,SOUNDS["VOTRAX"] = true
---------------------------------------------------

if (SOUNDS["VOTRAX"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/votrax.c",
		MAME_DIR .. "src/devices/sound/votrax.h",
	}
end



---------------------------------------------------
-- VRender0 custom sound chip
--@src/devices/sound/vrender0.h,SOUNDS["VRENDER0"] = true
---------------------------------------------------

if (SOUNDS["VRENDER0"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/vrender0.c",
		MAME_DIR .. "src/devices/sound/vrender0.h",
	}
end



---------------------------------------------------
-- WAVE file (used for MESS cassette)
--@src/devices/sound/wave.h,SOUNDS["WAVE"] = true
---------------------------------------------------

if (SOUNDS["WAVE"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/wave.c",
		MAME_DIR .. "src/devices/sound/wave.h",
	}
end



---------------------------------------------------
-- Yamaha FM synthesizers
--@src/devices/sound/2151intf.h,SOUNDS["YM2151"] = true
--@src/devices/sound/2203intf.h,SOUNDS["YM2203"] = true
--@src/devices/sound/2413intf.h,SOUNDS["YM2413"] = true
--@src/devices/sound/2608intf.h,SOUNDS["YM2608"] = true
--@src/devices/sound/2610intf.h,SOUNDS["YM2610"] = true
--@src/devices/sound/2612intf.h,SOUNDS["YM2612"] = true
--@src/devices/sound/3812intf.h,SOUNDS["YM3812"] = true
--@src/devices/sound/3526intf.h,SOUNDS["YM3526"] = true
--@src/devices/sound/8950intf.h,SOUNDS["Y8950"] = true
--@src/devices/sound/ymf262.h,SOUNDS["YMF262"] = true
--@src/devices/sound/ymf271.h,SOUNDS["YMF271"] = true
--@src/devices/sound/ymf278b.h,SOUNDS["YMF278B"] = true
--@src/devices/sound/262intf.h,SOUNDS["YMF262"] = true
---------------------------------------------------

if (SOUNDS["YM2151"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/2151intf.c",
		MAME_DIR .. "src/devices/sound/2151intf.h",
		MAME_DIR .. "src/devices/sound/ym2151.c",
		MAME_DIR .. "src/devices/sound/ym2151.h",
	}
end

if (SOUNDS["YM2413"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/2413intf.c",
		MAME_DIR .. "src/devices/sound/2413intf.h",
		MAME_DIR .. "src/devices/sound/ym2413.c",
		MAME_DIR .. "src/devices/sound/ym2413.h",
	}
end

if (SOUNDS["YM2203"]~=null or SOUNDS["YM2608"]~=null or SOUNDS["YM2610"]~=null or SOUNDS["YM2610B"]~=null or SOUNDS["YM2612"]~=null or SOUNDS["YM3438"]~=null) then	
--if (SOUNDS["YM2203"]~=null) then	
	files {
		MAME_DIR .. "src/devices/sound/2203intf.c",
		MAME_DIR .. "src/devices/sound/2203intf.h",
		MAME_DIR .. "src/devices/sound/ay8910.c",
		MAME_DIR .. "src/devices/sound/ay8910.h",
		MAME_DIR .. "src/devices/sound/fm.c",
		MAME_DIR .. "src/devices/sound/fm.h",
	}
--end


--if (SOUNDS["YM2608"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/2608intf.c", 
		MAME_DIR .. "src/devices/sound/2608intf.h", 
		MAME_DIR .. "src/devices/sound/ay8910.c", 
		MAME_DIR .. "src/devices/sound/ay8910.h", 
		MAME_DIR .. "src/devices/sound/fm.c", 
		MAME_DIR .. "src/devices/sound/fm.h", 
		MAME_DIR .. "src/devices/sound/ymdeltat.c",
		MAME_DIR .. "src/devices/sound/ymdeltat.h",
	}
--end

--if (SOUNDS["YM2610"]~=null or SOUNDS["YM2610B"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/2610intf.c", 
		MAME_DIR .. "src/devices/sound/2610intf.h", 
		MAME_DIR .. "src/devices/sound/ay8910.c", 
		MAME_DIR .. "src/devices/sound/ay8910.h", 
		MAME_DIR .. "src/devices/sound/fm.c", 
		MAME_DIR .. "src/devices/sound/fm.h", 
		MAME_DIR .. "src/devices/sound/ymdeltat.c",
		MAME_DIR .. "src/devices/sound/ymdeltat.h",
	}
--end

--if (SOUNDS["YM2612"]~=null or SOUNDS["YM3438"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/2612intf.c", 
		MAME_DIR .. "src/devices/sound/2612intf.h", 
		MAME_DIR .. "src/devices/sound/ay8910.c", 
		MAME_DIR .. "src/devices/sound/ay8910.h", 
		MAME_DIR .. "src/devices/sound/fm2612.c",
	}
--end
end

if (SOUNDS["YM3812"]~=null or SOUNDS["YM3526"]~=null or SOUNDS["Y8950"]~=null) then
--if (SOUNDS["YM3812"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/3812intf.c", 
		MAME_DIR .. "src/devices/sound/3812intf.h", 
		MAME_DIR .. "src/devices/sound/fmopl.c", 
		MAME_DIR .. "src/devices/sound/fmopl.h", 
		MAME_DIR .. "src/devices/sound/ymdeltat.c",
		MAME_DIR .. "src/devices/sound/ymdeltat.h",
	}
--end

--if (SOUNDS["YM3526"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/3526intf.c", 
		MAME_DIR .. "src/devices/sound/3526intf.h", 
		MAME_DIR .. "src/devices/sound/fmopl.c", 
		MAME_DIR .. "src/devices/sound/fmopl.h", 
		MAME_DIR .. "src/devices/sound/ymdeltat.c",
		MAME_DIR .. "src/devices/sound/ymdeltat.h",
	}
--end

--if (SOUNDS["Y8950"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/8950intf.c", 
		MAME_DIR .. "src/devices/sound/8950intf.h", 
		MAME_DIR .. "src/devices/sound/fmopl.c", 
		MAME_DIR .. "src/devices/sound/fmopl.h", 
		MAME_DIR .. "src/devices/sound/ymdeltat.c",
		MAME_DIR .. "src/devices/sound/ymdeltat.h",
	}
--end
end

if (SOUNDS["YMF262"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ymf262.c", 
		MAME_DIR .. "src/devices/sound/ymf262.h", 
		MAME_DIR .. "src/devices/sound/262intf.c",
		MAME_DIR .. "src/devices/sound/262intf.h",
	}
end

if (SOUNDS["YMF271"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ymf271.c",
		MAME_DIR .. "src/devices/sound/ymf271.h",
	}
end

if (SOUNDS["YMF278B"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ymf278b.c",
		MAME_DIR .. "src/devices/sound/ymf278b.h",
	}
end



---------------------------------------------------
-- Yamaha YMZ280B ADPCM
--@src/devices/sound/ymz280b.h,SOUNDS["YMZ280B"] = true
---------------------------------------------------

if (SOUNDS["YMZ280B"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ymz280b.c",
		MAME_DIR .. "src/devices/sound/ymz280b.h",
	}
end

---------------------------------------------------
-- Yamaha YMZ770 AMM
--@src/devices/sound/ymz770.h,SOUNDS["YMZ770"] = true
---------------------------------------------------

if (SOUNDS["YMZ770"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ymz770.c", 
		MAME_DIR .. "src/devices/sound/ymz770.h", 
		MAME_DIR .. "src/devices/sound/mpeg_audio.c",
		MAME_DIR .. "src/devices/sound/mpeg_audio.h",
	}
end

---------------------------------------------------
-- ZOOM ZSG-2
--@src/devices/sound/zsg2.h,SOUNDS["ZSG2"] = true
---------------------------------------------------

if (SOUNDS["ZSG2"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/zsg2.c",
		MAME_DIR .. "src/devices/sound/zsg2.h",
	}
end

---------------------------------------------------
-- VRC6
--@src/devices/sound/vrc6.h,SOUNDS["VRC6"] = true
---------------------------------------------------

if (SOUNDS["VRC6"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/vrc6.c",
		MAME_DIR .. "src/devices/sound/vrc6.h",
	}
end

