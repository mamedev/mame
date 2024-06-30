-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   sound.lua
--
--   Rules for building sound cores
--
----------------------------------------------------------------------------

files {
	MAME_DIR .. "src/devices/sound/bbd.cpp",
	MAME_DIR .. "src/devices/sound/bbd.h",
	MAME_DIR .. "src/devices/sound/flt_biquad.cpp",
	MAME_DIR .. "src/devices/sound/flt_biquad.h",
	MAME_DIR .. "src/devices/sound/flt_vol.cpp",
	MAME_DIR .. "src/devices/sound/flt_vol.h",
	MAME_DIR .. "src/devices/sound/flt_rc.cpp",
	MAME_DIR .. "src/devices/sound/flt_rc.h",
	MAME_DIR .. "src/devices/sound/mixer.cpp",
	MAME_DIR .. "src/devices/sound/mixer.h",
	MAME_DIR .. "src/devices/sound/samples.cpp",
	MAME_DIR .. "src/devices/sound/samples.h",
}

---------------------------------------------------
-- DACs
--@src/devices/sound/dac.h,SOUNDS["DAC"] = true
--@src/devices/sound/dmadac.h,SOUNDS["DMADAC"] = true
--@src/devices/sound/spkrdev.h,SOUNDS["SPEAKER"] = true
--@src/devices/sound/beep.h,SOUNDS["BEEP"] = true
---------------------------------------------------

if (SOUNDS["DAC"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/dac.cpp",
		MAME_DIR .. "src/devices/sound/dac.h",
	}
end

if (SOUNDS["DMADAC"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/dmadac.cpp",
		MAME_DIR .. "src/devices/sound/dmadac.h",
	}
end

if (SOUNDS["SPEAKER"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/spkrdev.cpp",
		MAME_DIR .. "src/devices/sound/spkrdev.h",
	}
end

if (SOUNDS["BEEP"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/beep.cpp",
		MAME_DIR .. "src/devices/sound/beep.h",
	}
end



---------------------------------------------------
-- CD audio
--@src/devices/sound/cdda.h,SOUNDS["CDDA"] = true
---------------------------------------------------

if (SOUNDS["CDDA"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/cdda.cpp",
		MAME_DIR .. "src/devices/sound/cdda.h",
	}
end



---------------------------------------------------
-- Discrete component audio
--@src/devices/sound/discrete.h,SOUNDS["DISCRETE"] = true
---------------------------------------------------

if (SOUNDS["DISCRETE"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/discrete.cpp",
		MAME_DIR .. "src/devices/sound/discrete.h",
		MAME_DIR .. "src/devices/sound/disc_cls.h",
		MAME_DIR .. "src/devices/sound/disc_dev.h",
		MAME_DIR .. "src/devices/sound/disc_dev.hxx",
		MAME_DIR .. "src/devices/sound/disc_flt.h",
		MAME_DIR .. "src/devices/sound/disc_flt.hxx",
		MAME_DIR .. "src/devices/sound/disc_inp.hxx",
		MAME_DIR .. "src/devices/sound/disc_mth.h",
		MAME_DIR .. "src/devices/sound/disc_mth.hxx",
		MAME_DIR .. "src/devices/sound/disc_sys.hxx",
		MAME_DIR .. "src/devices/sound/disc_wav.h",
		MAME_DIR .. "src/devices/sound/disc_wav.hxx",
	}
end

---------------------------------------------------
-- AC97
--@src/devices/sound/pci-ac97.h,SOUNDS["AC97"] = true
---------------------------------------------------

if (SOUNDS["AC97"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/pci-ac97.cpp",
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
		MAME_DIR .. "src/devices/sound/asc.cpp",
		MAME_DIR .. "src/devices/sound/asc.h",
	}
end

if (SOUNDS["AWACS"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/awacs.cpp",
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
		MAME_DIR .. "src/devices/sound/pokey.cpp",
		MAME_DIR .. "src/devices/sound/pokey.h",
	}
end

if (SOUNDS["TIA"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tiasound.cpp",
		MAME_DIR .. "src/devices/sound/tiasound.h",
		MAME_DIR .. "src/devices/sound/tiaintf.cpp",
		MAME_DIR .. "src/devices/sound/tiaintf.h",
	}
end



---------------------------------------------------
-- Amiga audio hardware
--@src/devices/machine/8364_paula.h,SOUNDS["PAULA_8364"] = true
---------------------------------------------------

if (SOUNDS["PAULA_8364"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/8364_paula.cpp",
		MAME_DIR .. "src/devices/machine/8364_paula.h",
	}
end



---------------------------------------------------
-- Bally Astrocade sound system
--@src/devices/sound/astrocde.h,SOUNDS["ASTROCADE"] = true
---------------------------------------------------

if (SOUNDS["ASTROCADE"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/astrocde.cpp",
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
		MAME_DIR .. "src/devices/sound/pci-ac97.cpp",
		MAME_DIR .. "src/devices/sound/pci-ac97.h",
	}
end
-- CEM 3394 analog synthesizer chip
--@src/devices/sound/cem3394.h,SOUNDS["CEM3394"] = true
---------------------------------------------------

if (SOUNDS["CEM3394"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/cem3394.cpp",
		MAME_DIR .. "src/devices/sound/cem3394.h",
	}
end



---------------------------------------------------
-- Creative Labs SB0400 Audigy2 Value
--@src/devices/sound/sb0400.h,SOUNDS["SB0400"] = true
---------------------------------------------------

if (SOUNDS["SB0400"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/sb0400.cpp",
		MAME_DIR .. "src/devices/sound/sb0400.h",
	}
end


--------------------------------------------------
-- Creative Labs Ensonic AudioPCI97 ES1373
--@src/devices/sound/es1373.h,SOUNDS["ES1373"] = true
--------------------------------------------------

if (SOUNDS["ES1373"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/es1373.cpp",
		MAME_DIR .. "src/devices/sound/es1373.h",
	}
end

---------------------------------------------------
-- Data East custom sound chips
--@src/devices/sound/bsmt2000.h,SOUNDS["BSMT2000"] = true
---------------------------------------------------

if (SOUNDS["BSMT2000"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/bsmt2000.cpp",
		MAME_DIR .. "src/devices/sound/bsmt2000.h",
	}
end



---------------------------------------------------
-- Ensoniq 5503 (Apple IIgs)
--@src/devices/sound/es5503.h,SOUNDS["ES5503"] = true
---------------------------------------------------

if (SOUNDS["ES5503"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/es5503.cpp",
		MAME_DIR .. "src/devices/sound/es5503.h",
	}
end



---------------------------------------------------
-- Ensoniq 5505/5506
--@src/devices/sound/es5506.h,SOUNDS["ES5505"] = true
---------------------------------------------------

if (SOUNDS["ES5505"]~=null or SOUNDS["ES5506"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/es5506.cpp",
		MAME_DIR .. "src/devices/sound/es5506.h",
	}
end


---------------------------------------------------
-- Ensoniq "pump" device, interfaces 5505/5506 with 5510
--@src/devices/sound/esqpump.h,SOUNDS["ESQPUMP"] = true
---------------------------------------------------

if (SOUNDS["ESQPUMP"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/esqpump.cpp",
		MAME_DIR .. "src/devices/sound/esqpump.h",
	}
end


---------------------------------------------------
-- Excellent Systems ADPCM sound chip
--@src/devices/sound/es8712.h,SOUNDS["ES8712"] = true
---------------------------------------------------

if (SOUNDS["ES8712"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/es8712.cpp",
		MAME_DIR .. "src/devices/sound/es8712.h",
	}
end



---------------------------------------------------
-- Gaelco custom sound chips
--@src/devices/sound/gaelco.h,SOUNDS["GAELCO_CG1V"] = true
---------------------------------------------------

if (SOUNDS["GAELCO_CG1V"]~=null or SOUNDS["GAELCO_GAE1"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/gaelco.cpp",
		MAME_DIR .. "src/devices/sound/gaelco.h",
	}
end


---------------------------------------------------
-- RCA CDP1863
--@src/devices/sound/cdp1863.h,SOUNDS["CDP1863"] = true
---------------------------------------------------

if (SOUNDS["CDP1863"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/cdp1863.cpp",
		MAME_DIR .. "src/devices/sound/cdp1863.h",
	}
end



---------------------------------------------------
-- RCA CDP1864
--@src/devices/sound/cdp1864.h,SOUNDS["CDP1864"] = true
---------------------------------------------------

if (SOUNDS["CDP1864"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/cdp1864.cpp",
		MAME_DIR .. "src/devices/sound/cdp1864.h",
	}
end



---------------------------------------------------
-- RCA CDP1869
--@src/devices/sound/cdp1869.h,SOUNDS["CDP1869"] = true
---------------------------------------------------

if (SOUNDS["CDP1869"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/cdp1869.cpp",
		MAME_DIR .. "src/devices/sound/cdp1869.h",
	}
end



---------------------------------------------------
-- GI AY-8910
--@src/devices/sound/ay8910.h,SOUNDS["AY8910"] = true
---------------------------------------------------

if (SOUNDS["AY8910"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ay8910.cpp",
		MAME_DIR .. "src/devices/sound/ay8910.h",
	}
end



---------------------------------------------------
-- Harris HC55516 CVSD
--@src/devices/sound/hc55516.h,SOUNDS["HC55516"] = true
---------------------------------------------------

if (SOUNDS["HC55516"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/hc55516.cpp",
		MAME_DIR .. "src/devices/sound/hc55516.h",
	}
end



---------------------------------------------------
-- Hudsonsoft HuC6230 SoundBox
--@src/devices/sound/huc6230.h,SOUNDS["HUC6230"] = true
---------------------------------------------------

if (SOUNDS["HUC6230"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/huc6230.cpp",
		MAME_DIR .. "src/devices/sound/huc6230.h",
	}
end



---------------------------------------------------
-- Hudsonsoft C6280 sound chip
--@src/devices/sound/c6280.h,SOUNDS["C6280"] = true
---------------------------------------------------

if (SOUNDS["C6280"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/c6280.cpp",
		MAME_DIR .. "src/devices/sound/c6280.h",
	}
end



---------------------------------------------------
-- ICS2115 sound chip
--@src/devices/sound/ics2115.h,SOUNDS["ICS2115"] = true
---------------------------------------------------

if (SOUNDS["ICS2115"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ics2115.cpp",
		MAME_DIR .. "src/devices/sound/ics2115.h",
	}
end



---------------------------------------------------
-- Imagetek I5000 sound
--@src/devices/sound/i5000.h,SOUNDS["I5000_SND"] = true
---------------------------------------------------

if (SOUNDS["I5000_SND"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/i5000.cpp",
		MAME_DIR .. "src/devices/sound/i5000.h",
	}
end



---------------------------------------------------
-- Irem custom sound chips
--@src/devices/sound/iremga20.h,SOUNDS["IREMGA20"] = true
---------------------------------------------------

if (SOUNDS["IREMGA20"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/iremga20.cpp",
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
		MAME_DIR .. "src/devices/sound/k005289.cpp",
		MAME_DIR .. "src/devices/sound/k005289.h",
	}
end

if (SOUNDS["K007232"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/k007232.cpp",
		MAME_DIR .. "src/devices/sound/k007232.h",
	}
end

if (SOUNDS["K051649"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/k051649.cpp",
		MAME_DIR .. "src/devices/sound/k051649.h",
	}
end

if (SOUNDS["K053260"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/k053260.cpp",
		MAME_DIR .. "src/devices/sound/k053260.h",
	}
end

if (SOUNDS["K054539"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/k054539.cpp",
		MAME_DIR .. "src/devices/sound/k054539.h",
	}
end

if (SOUNDS["K056800"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/k056800.cpp",
		MAME_DIR .. "src/devices/sound/k056800.h",
	}
end


---------------------------------------------------
-- L7A1045 L6028 DSP-A
--@src/devices/sound/l7a1045_l6028_dsp_a.h,SOUNDS["L7A1045"] = true
---------------------------------------------------

if (SOUNDS["L7A1045"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/l7a1045_l6028_dsp_a.cpp",
		MAME_DIR .. "src/devices/sound/l7a1045_l6028_dsp_a.h",
	}
end


---------------------------------------------------
-- LMC1992 mixer chip
--@src/devices/sound/lmc1992.h,SOUNDS["LMC1992"] = true
---------------------------------------------------

if (SOUNDS["LMC1992"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/lmc1992.cpp",
		MAME_DIR .. "src/devices/sound/lmc1992.h",
	}
end



---------------------------------------------------
-- MAS 3507D MPEG 1/2 Layer 2/3 Audio Decoder
--@src/devices/sound/mas3507d.h,SOUNDS["MAS3507D"] = true
---------------------------------------------------

if (SOUNDS["MAS3507D"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/mas3507d.cpp",
		MAME_DIR .. "src/devices/sound/mas3507d.h",
	}
end



---------------------------------------------------
-- Micronas DAC 3550A Stereo Audio DAC
--@src/devices/sound/dac3350a.h,SOUNDS["DAC3350A"] = true
---------------------------------------------------

if (SOUNDS["DAC3350A"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/dac3350a.cpp",
		MAME_DIR .. "src/devices/sound/dac3350a.h",
	}
end



---------------------------------------------------
-- MEA8000 Voice Synthesizer
--@src/devices/sound/mea8000.h,SOUNDS["MEA8000"] = true
---------------------------------------------------

if (SOUNDS["MEA8000"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/mea8000.cpp",
		MAME_DIR .. "src/devices/sound/mea8000.h",
	}
end



---------------------------------------------------
-- MOS 6560VIC
--@src/devices/sound/mos6560.h,SOUNDS["MOS656X"] = true
---------------------------------------------------

if (SOUNDS["MOS656X"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/mos6560.cpp",
		MAME_DIR .. "src/devices/sound/mos6560.h",
	}
end



---------------------------------------------------
-- MOS 7360 TED
--@src/devices/sound/mos7360.h,SOUNDS["MOS7360"] = true
---------------------------------------------------

if (SOUNDS["MOS7360"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/mos7360.cpp",
		MAME_DIR .. "src/devices/sound/mos7360.h",
	}
end



---------------------------------------------------
-- Namco custom sound chips
--@src/devices/sound/namco.h,SOUNDS["NAMCO"] = true
--@src/devices/sound/namco_163.h,SOUNDS["NAMCO_163"] = true
--@src/devices/sound/n63701x.h,SOUNDS["NAMCO_63701X"] = true
--@src/devices/sound/c140.h,SOUNDS["C140"] = true
--@src/devices/sound/c352.h,SOUNDS["C352"] = true
---------------------------------------------------

if (SOUNDS["NAMCO"]~=null or SOUNDS["NAMCO_15XX"]~=null or SOUNDS["NAMCO_CUS30"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/namco.cpp",
		MAME_DIR .. "src/devices/sound/namco.h",
	}
end

if (SOUNDS["NAMCO_163"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/namco_163.cpp",
		MAME_DIR .. "src/devices/sound/namco_163.h",
	}
end

if (SOUNDS["NAMCO_63701X"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/n63701x.cpp",
		MAME_DIR .. "src/devices/sound/n63701x.h",
	}
end

if (SOUNDS["C140"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/c140.cpp",
		MAME_DIR .. "src/devices/sound/c140.h",
	}
end

if (SOUNDS["C352"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/c352.cpp",
		MAME_DIR .. "src/devices/sound/c352.h",
	}
end



---------------------------------------------------
-- National Semiconductor Digitalker
--@src/devices/sound/digitalk.h,SOUNDS["DIGITALKER"] = true
---------------------------------------------------

if (SOUNDS["DIGITALKER"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/digitalk.cpp",
		MAME_DIR .. "src/devices/sound/digitalk.h",
	}
end



---------------------------------------------------
-- Nintendo custom sound chips
--@src/devices/sound/nes_apu.h,SOUNDS["NES_APU"] = true
---------------------------------------------------

if (SOUNDS["NES_APU"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/nes_apu.cpp",
		MAME_DIR .. "src/devices/sound/nes_apu.h",
		MAME_DIR .. "src/devices/sound/nes_defs.h",
		MAME_DIR .. "src/devices/sound/nes_apu_vt.cpp",
		MAME_DIR .. "src/devices/sound/nes_apu_vt.h",
	}
end



---------------------------------------------------
-- NEC uPD7759 ADPCM sample player
--@src/devices/sound/upd7759.h,SOUNDS["UPD7759"] = true
---------------------------------------------------

if (SOUNDS["UPD7759"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/upd7759.cpp",
		MAME_DIR .. "src/devices/sound/upd7759.h",
		MAME_DIR .. "src/devices/sound/315-5641.cpp",
		MAME_DIR .. "src/devices/sound/315-5641.h",
	}
end



---------------------------------------------------
-- IMA ADPCM sample player
--@src/devices/sound/imaadpcm.h,SOUNDS["IMAADPCM"] = true
---------------------------------------------------

if (SOUNDS["IMAADPCM"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/imaadpcm.cpp",
		MAME_DIR .. "src/devices/sound/imaadpcm.h",
	}
end



---------------------------------------------------
-- OKI ADPCM sample players
--@src/devices/sound/okim6258.h,SOUNDS["OKIM6258"] = true
--@src/devices/sound/msm5205.h,SOUNDS["MSM5205"] = true
--@src/devices/sound/msm5232.h,SOUNDS["MSM5232"] = true
--@src/devices/sound/okim6376.h,SOUNDS["OKIM6376"] = true
--@src/devices/sound/okim6295.h,SOUNDS["OKIM6295"] = true
--@src/devices/sound/okim6588.h,SOUNDS["OKIM6588"] = true
--@src/devices/sound/okim9810.h,SOUNDS["OKIM9810"] = true
--@src/devices/sound/okiadpcm.h,SOUNDS["OKIADPCM"] = true
---------------------------------------------------

if (SOUNDS["OKIM6258"]~=null or SOUNDS["OKIM6295"]~=null or SOUNDS["OKIM6588"]~=null or SOUNDS["OKIM9810"]~=null or SOUNDS["I5000_SND"]~=null or SOUNDS["OKIADPCM"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/okiadpcm.cpp",
		MAME_DIR .. "src/devices/sound/okiadpcm.h",
	}
end

if (SOUNDS["MSM5205"]~=null or SOUNDS["MSM6585"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/msm5205.cpp",
		MAME_DIR .. "src/devices/sound/msm5205.h",
	}
end

if (SOUNDS["MSM5232"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/msm5232.cpp",
		MAME_DIR .. "src/devices/sound/msm5232.h",
	}
end

if (SOUNDS["OKIM6376"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/okim6376.cpp",
		MAME_DIR .. "src/devices/sound/okim6376.h",
	}
end

if (SOUNDS["OKIM6295"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/okim6295.cpp",
		MAME_DIR .. "src/devices/sound/okim6295.h",
	}
end

if (SOUNDS["OKIM6258"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/okim6258.cpp",
		MAME_DIR .. "src/devices/sound/okim6258.h",
	}
end

if (SOUNDS["OKIM6588"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/okim6588.cpp",
		MAME_DIR .. "src/devices/sound/okim6588.h",
	}
end

if (SOUNDS["OKIM9810"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/okim9810.cpp",
		MAME_DIR .. "src/devices/sound/okim9810.h",
	}
end



---------------------------------------------------
-- Philips SAA1099
--@src/devices/sound/saa1099.h,SOUNDS["SAA1099"] = true
---------------------------------------------------

if (SOUNDS["SAA1099"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/saa1099.cpp",
		MAME_DIR .. "src/devices/sound/saa1099.h",
	}
end



---------------------------------------------------
-- AdMOS QS1000
--@src/devices/sound/qs1000.h,SOUNDS["QS1000"] = true
---------------------------------------------------

if (SOUNDS["QS1000"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/qs1000.cpp",
		MAME_DIR .. "src/devices/sound/qs1000.h",
	}
end



---------------------------------------------------
-- QSound sample player
--@src/devices/sound/qsound.h,SOUNDS["QSOUND"] = true
---------------------------------------------------

if (SOUNDS["QSOUND"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/qsound.cpp",
		MAME_DIR .. "src/devices/sound/qsound.h",
		MAME_DIR .. "src/devices/sound/qsoundhle.cpp",
		MAME_DIR .. "src/devices/sound/qsoundhle.h",
	}
end



---------------------------------------------------
-- Ricoh sample players
--@src/devices/sound/rf5c68.h,SOUNDS["RF5C68"] = true
--@src/devices/sound/rf5c400.h,SOUNDS["RF5C400"] = true
---------------------------------------------------

if (SOUNDS["RF5C68"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/rf5c68.cpp",
		MAME_DIR .. "src/devices/sound/rf5c68.h",
	}
end

if (SOUNDS["RF5C400"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/rf5c400.cpp",
		MAME_DIR .. "src/devices/sound/rf5c400.h",
	}
end



---------------------------------------------------
-- Sega custom sound chips
--@src/devices/sound/segapcm.h,SOUNDS["SEGAPCM"] = true
--@src/devices/sound/scsp.h,SOUNDS["SCSP"] = true
--@src/devices/sound/aica.h,SOUNDS["AICA"] = true
---------------------------------------------------

if (SOUNDS["SEGAPCM"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/segapcm.cpp",
		MAME_DIR .. "src/devices/sound/segapcm.h",
	}
end

if (SOUNDS["SCSP"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/scsp.cpp",
		MAME_DIR .. "src/devices/sound/scsp.h",
		MAME_DIR .. "src/devices/sound/scspdsp.cpp",
		MAME_DIR .. "src/devices/sound/scspdsp.h",
	}
end

if (SOUNDS["AICA"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/aica.cpp",
		MAME_DIR .. "src/devices/sound/aica.h",
		MAME_DIR .. "src/devices/sound/aicadsp.cpp",
		MAME_DIR .. "src/devices/sound/aicadsp.h",
	}
end

---------------------------------------------------
-- Seta custom sound chips
--@src/devices/sound/st0016.h,SOUNDS["ST0016"] = true
--@src/devices/sound/setapcm.h,SOUNDS["SETAPCM"] = true
--@src/devices/sound/x1_010.h,SOUNDS["X1_010"] = true
---------------------------------------------------

if (SOUNDS["ST0016"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/st0016.cpp",
		MAME_DIR .. "src/devices/sound/st0016.h",
	}
end

if (SOUNDS["SETAPCM"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/setapcm.cpp",
		MAME_DIR .. "src/devices/sound/setapcm.h",
	}
end

if (SOUNDS["X1_010"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/x1_010.cpp",
		MAME_DIR .. "src/devices/sound/x1_010.h",
	}
end



---------------------------------------------------
-- SID custom sound chips
--@src/devices/sound/mos6581.h,SOUNDS["SID6581"] = true
---------------------------------------------------

if (SOUNDS["SID6581"]~=null or SOUNDS["SID8580"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/mos6581.cpp",
		MAME_DIR .. "src/devices/sound/mos6581.h",
		MAME_DIR .. "src/devices/sound/sid.cpp",
		MAME_DIR .. "src/devices/sound/sid.h",
		MAME_DIR .. "src/devices/sound/sidenvel.cpp",
		MAME_DIR .. "src/devices/sound/sidenvel.h",
		MAME_DIR .. "src/devices/sound/sidvoice.cpp",
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
		MAME_DIR .. "src/devices/sound/t6w28.cpp",
		MAME_DIR .. "src/devices/sound/t6w28.h",
	}
end



---------------------------------------------------
-- SNK custom wave generator
--@src/devices/sound/snkwave.h,SOUNDS["SNKWAVE"] = true
---------------------------------------------------

if (SOUNDS["SNKWAVE"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/snkwave.cpp",
		MAME_DIR .. "src/devices/sound/snkwave.h",
	}
end



---------------------------------------------------
-- Sony custom sound chips
--@src/devices/sound/spu.h,SOUNDS["SPU"] = true
---------------------------------------------------

if (SOUNDS["SPU"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/spu.cpp",
		MAME_DIR .. "src/devices/sound/spu.h",
		MAME_DIR .. "src/devices/sound/spu_tables.cpp",
		MAME_DIR .. "src/devices/sound/spureverb.cpp",
		MAME_DIR .. "src/devices/sound/spureverb.h",
	}
end


---------------------------------------------------
-- SP0256 speech synthesizer
--@src/devices/sound/sp0256.h,SOUNDS["SP0256"] = true
---------------------------------------------------

if (SOUNDS["SP0256"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/sp0256.cpp",
		MAME_DIR .. "src/devices/sound/sp0256.h",
	}
end



---------------------------------------------------
-- SP0250 speech synthesizer
--@src/devices/sound/sp0250.h,SOUNDS["SP0250"] = true
---------------------------------------------------

if (SOUNDS["SP0250"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/sp0250.cpp",
		MAME_DIR .. "src/devices/sound/sp0250.h",
	}
end


---------------------------------------------------
-- ST-Techno custom sound chip
--@src/devices/sound/stt_sa1.h,SOUNDS["STT_SA1"] = true
---------------------------------------------------

if (SOUNDS["STT_SA1"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/stt_sa1.cpp",
		MAME_DIR .. "src/devices/sound/stt_sa1.h",
	}
end


---------------------------------------------------
-- S14001A speech synthesizer
--@src/devices/sound/s14001a.h,SOUNDS["S14001A"] = true
---------------------------------------------------

if (SOUNDS["S14001A"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/s14001a.cpp",
		MAME_DIR .. "src/devices/sound/s14001a.h",
	}
end



---------------------------------------------------
-- Texas Instruments SN76477 analog chip
--@src/devices/sound/sn76477.h,SOUNDS["SN76477"] = true
---------------------------------------------------

if (SOUNDS["SN76477"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/sn76477.cpp",
		MAME_DIR .. "src/devices/sound/sn76477.h",
	}
end



---------------------------------------------------
-- Texas Instruments SN76496
--@src/devices/sound/sn76496.h,SOUNDS["SN76496"] = true
---------------------------------------------------

if (SOUNDS["SN76496"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/sn76496.cpp",
		MAME_DIR .. "src/devices/sound/sn76496.h",
	}
end



---------------------------------------------------
-- Silicon Systems SSI-263A HLE
--@src/devices/sound/ssi263hle.h,SOUNDS["SSI263HLE"] = true
---------------------------------------------------

if (SOUNDS["SSI263HLE"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ssi263hle.cpp",
		MAME_DIR .. "src/devices/sound/ssi263hle.h",
	}
end



---------------------------------------------------
-- Texas Instruments TMS36xx doorbell chime
--@src/devices/sound/tms36xx.h,SOUNDS["TMS36XX"] = true
---------------------------------------------------

if (SOUNDS["TMS36XX"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tms36xx.cpp",
		MAME_DIR .. "src/devices/sound/tms36xx.h",
	}
end



---------------------------------------------------
-- Texas Instruments TMS3615 Octave Multiple Tone Synthesizer
--@src/devices/sound/tms3615.h,SOUNDS["TMS3615"] = true
---------------------------------------------------

if (SOUNDS["TMS3615"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tms3615.cpp",
		MAME_DIR .. "src/devices/sound/tms3615.h",
	}
end



---------------------------------------------------
-- Texas Instruments TMS5100-series speech synthesizers
--@src/devices/sound/tms5110.h,SOUNDS["TMS5110"] = true
---------------------------------------------------

if (SOUNDS["TMS5110"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tms5110.cpp",
		MAME_DIR .. "src/devices/sound/tms5110.h",
		MAME_DIR .. "src/devices/sound/tms5110r.hxx",
	}
end

---------------------------------------------------
-- Texas Instruments TMS5200-series speech synthesizers
--@src/devices/sound/tms5220.h,SOUNDS["TMS5220"] = true
---------------------------------------------------
if (SOUNDS["TMS5220"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tms5220.cpp",
		MAME_DIR .. "src/devices/sound/tms5220.h",
		MAME_DIR .. "src/devices/sound/tms5110r.hxx",
		MAME_DIR .. "src/devices/machine/spchrom.cpp",
		MAME_DIR .. "src/devices/machine/spchrom.h",
	}
end


---------------------------------------------------
-- Toshiba T6721A voice synthesizer
--@src/devices/sound/t6721a.h,SOUNDS["T6721A"] = true
---------------------------------------------------

if (SOUNDS["T6721A"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/t6721a.cpp",
		MAME_DIR .. "src/devices/sound/t6721a.h",
	}
end



---------------------------------------------------
-- Toshiba TC8830F sample player/recorder
--@src/devices/sound/tc8830f.h,SOUNDS["TC8830F"] = true
---------------------------------------------------

if (SOUNDS["TC8830F"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tc8830f.cpp",
		MAME_DIR .. "src/devices/sound/tc8830f.h",
	}
end


---------------------------------------------------
-- NEC uPD7752
--@src/devices/sound/upd7752.h,SOUNDS["UPD7752"] = true
---------------------------------------------------

if (SOUNDS["UPD7752"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/upd7752.cpp",
		MAME_DIR .. "src/devices/sound/upd7752.h",
	}
end


---------------------------------------------------
-- VLM5030 speech synthesizer
--@src/devices/sound/vlm5030.h,SOUNDS["VLM5030"] = true
---------------------------------------------------

if (SOUNDS["VLM5030"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/vlm5030.cpp",
		MAME_DIR .. "src/devices/sound/vlm5030.h",
		MAME_DIR .. "src/devices/sound/tms5110r.hxx",
	}
end

---------------------------------------------------
-- Votrax SC-01[-A] speech synthesizer
--@src/devices/sound/votrax.h,SOUNDS["VOTRAX_SC01"] = true
--@src/devices/sound/votrax.h,SOUNDS["VOTRAX_SC01A"] = true
---------------------------------------------------

if (SOUNDS["VOTRAX_SC01"]~=null or SOUNDS["VOTRAX_SC01A"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/votrax.cpp",
		MAME_DIR .. "src/devices/sound/votrax.h",
	}
end



---------------------------------------------------
-- VRender0 custom sound chip
--@src/devices/sound/vrender0.h,SOUNDS["VRENDER0"] = true
---------------------------------------------------

if (SOUNDS["VRENDER0"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/vrender0.cpp",
		MAME_DIR .. "src/devices/sound/vrender0.h",
	}
end



---------------------------------------------------
-- WAVE file (used for cassette)
--@src/devices/sound/wave.h,SOUNDS["WAVE"] = true
---------------------------------------------------

if (SOUNDS["WAVE"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/wave.cpp",
		MAME_DIR .. "src/devices/sound/wave.h",
	}
end



---------------------------------------------------
-- Yamaha FM synthesizers
--@src/devices/sound/ym2154.h,SOUNDS["YM2154"] = true
--@src/devices/sound/ymopm.h,SOUNDS["YM2151"] = true
--@src/devices/sound/ymopz.h,SOUNDS["YM2414"] = true
--@src/devices/sound/ymopq.h,SOUNDS["YM3806"] = true
--@src/devices/sound/ymopn.h,SOUNDS["YM2203"] = true
--@src/devices/sound/ymopl.h,SOUNDS["YM2413"] = true
--@src/devices/sound/ymopn.h,SOUNDS["YM2608"] = true
--@src/devices/sound/ymopn.h,SOUNDS["YM2610"] = true
--@src/devices/sound/ymopn.h,SOUNDS["YM2612"] = true
--@src/devices/sound/ymopl.h,SOUNDS["YM3526"] = true
--@src/devices/sound/ymopl.h,SOUNDS["YM3812"] = true
--@src/devices/sound/ymopl.h,SOUNDS["YMF262"] = true
--@src/devices/sound/ymopl.h,SOUNDS["YMF278B"] = true
--@src/devices/sound/ymf271.h,SOUNDS["YMF271"] = true
--@src/devices/sound/ymopl.h,SOUNDS["Y8950"] = true
---------------------------------------------------

if (SOUNDS["YM2154"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ym2154.cpp",
		MAME_DIR .. "src/devices/sound/ym2154.h",
	}
end

if (SOUNDS["YM2151"]~=null or SOUNDS["YM2164"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ymopm.cpp",
		MAME_DIR .. "src/devices/sound/ymopm.h",
	}
end

if (SOUNDS["YM2414"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ymopz.cpp",
		MAME_DIR .. "src/devices/sound/ymopz.h",
	}
end

if (SOUNDS["YM3806"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ymopq.cpp",
		MAME_DIR .. "src/devices/sound/ymopq.h",
	}
end

if (SOUNDS["YM2203"]~=null or SOUNDS["YM2608"]~=null or SOUNDS["YM2610"]~=null or SOUNDS["YM2610B"]~=null or SOUNDS["YM2612"]~=null or SOUNDS["YM3438"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ay8910.cpp",
		MAME_DIR .. "src/devices/sound/ay8910.h",
		MAME_DIR .. "src/devices/sound/ymopn.cpp",
		MAME_DIR .. "src/devices/sound/ymopn.h",
	}
end

if (SOUNDS["YM3526"]~=null or SOUNDS["Y8950"]~=null or SOUNDS["YM3812"]~=null or SOUNDS["YMF262"]~=null or SOUNDS["YMF278B"]~=null or SOUNDS["YM2413"]~=null or SOUNDS["YM2423"]~=null or SOUNDS["YMF281"]~=null or SOUNDS["DS1001"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ymopl.cpp",
		MAME_DIR .. "src/devices/sound/ymopl.h",
	}
end

if (SOUNDS["YMF271"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ymf271.cpp",
		MAME_DIR .. "src/devices/sound/ymf271.h",
	}
end



---------------------------------------------------
-- Yamaha YMZ280B ADPCM
--@src/devices/sound/ymz280b.h,SOUNDS["YMZ280B"] = true
---------------------------------------------------

if (SOUNDS["YMZ280B"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ymz280b.cpp",
		MAME_DIR .. "src/devices/sound/ymz280b.h",
	}
end

---------------------------------------------------
-- Yamaha YMZ770 AMM
--@src/devices/sound/ymz770.h,SOUNDS["YMZ770"] = true
---------------------------------------------------

if (SOUNDS["YMZ770"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ymz770.cpp",
		MAME_DIR .. "src/devices/sound/ymz770.h",
	}
end

---------------------------------------------------
-- Yamaha GEW series PCM
--@src/devices/sound/gew7.h,SOUNDS["GEW7"] = true
--@src/devices/sound/multipcm.h,SOUNDS["MULTIPCM"] = true
---------------------------------------------------

if (SOUNDS["GEW7"]~=null or SOUNDS["MULTIPCM"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/gew.cpp",
		MAME_DIR .. "src/devices/sound/gew.h",
	}
end

if (SOUNDS["GEW7"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/gew7.cpp",
		MAME_DIR .. "src/devices/sound/gew7.h",
	}
end

if (SOUNDS["MULTIPCM"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/multipcm.cpp",
		MAME_DIR .. "src/devices/sound/multipcm.h",
	}
end

---------------------------------------------------
-- MP3 AUDIO
--@src/devices/sound/mp3_audio.h,SOUNDS["MP3_AUDIO"] = true
---------------------------------------------------

if (SOUNDS["MP3_AUDIO"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/mp3_audio.cpp",
		MAME_DIR .. "src/devices/sound/mp3_audio.h",
	}
end

---------------------------------------------------
-- MPEG AUDIO
--@src/devices/sound/mpeg_audio.h,SOUNDS["MPEG_AUDIO"] = true
---------------------------------------------------

if (SOUNDS["MPEG_AUDIO"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/mpeg_audio.cpp",
		MAME_DIR .. "src/devices/sound/mpeg_audio.h",
	}
end

---------------------------------------------------
-- ZOOM ZSG-2
--@src/devices/sound/zsg2.h,SOUNDS["ZSG2"] = true
---------------------------------------------------

if (SOUNDS["ZSG2"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/zsg2.cpp",
		MAME_DIR .. "src/devices/sound/zsg2.h",
	}
end

---------------------------------------------------
-- VRC6
--@src/devices/sound/vrc6.h,SOUNDS["VRC6"] = true
---------------------------------------------------

if (SOUNDS["VRC6"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/vrc6.cpp",
		MAME_DIR .. "src/devices/sound/vrc6.h",
	}
end

---------------------------------------------------
-- AD1848
--@src/devices/sound/ad1848.h,SOUNDS["AD1848"] = true
---------------------------------------------------

if (SOUNDS["AD1848"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ad1848.cpp",
		MAME_DIR .. "src/devices/sound/ad1848.h",
	}
end

---------------------------------------------------
-- UPD1771
--@src/devices/sound/upd1771.h,SOUNDS["UPD1771"] = true
---------------------------------------------------

if (SOUNDS["UPD1771"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/upd1771.cpp",
		MAME_DIR .. "src/devices/sound/upd1771.h",
	}
end

---------------------------------------------------
-- GB_SOUND
--@src/devices/sound/gb.h,SOUNDS["GB_SOUND"] = true
---------------------------------------------------

if (SOUNDS["GB_SOUND"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/gb.cpp",
		MAME_DIR .. "src/devices/sound/gb.h",
	}
end

---------------------------------------------------
-- PCD3311
--@src/devices/sound/pcd3311.h,SOUNDS["PCD3311"] = true
---------------------------------------------------

if (SOUNDS["PCD3311"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/pcd3311.cpp",
		MAME_DIR .. "src/devices/sound/pcd3311.h",
	}
end

---------------------------------------------------
-- DAC-76 COMDAC
--@src/devices/sound/dac76.h,SOUNDS["DAC76"] = true
---------------------------------------------------
if (SOUNDS["DAC76"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/dac76.cpp",
		MAME_DIR .. "src/devices/sound/dac76.h",
	}
end

---------------------------------------------------
-- MM5837 Noise Generator
--@src/devices/sound/mm5837.h,SOUNDS["MM5837"] = true
---------------------------------------------------

if (SOUNDS["MM5837"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/mm5837.cpp",
		MAME_DIR .. "src/devices/sound/mm5837.h",
	}
end

---------------------------------------------------
-- Intelligent Designs DAVE
--@src/devices/sound/dave.h,SOUNDS["DAVE"] = true
---------------------------------------------------

if (SOUNDS["DAVE"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/dave.cpp",
		MAME_DIR .. "src/devices/sound/dave.h",
	}
end

---------------------------------------------------
-- Toshiba TA7630
--@src/devices/sound/ta7630.h,SOUNDS["TA7630"] = true
---------------------------------------------------

if (SOUNDS["TA7630"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ta7630.cpp",
		MAME_DIR .. "src/devices/sound/ta7630.h",
	}
end

---------------------------------------------------
-- Sanyo LC7535
--@src/devices/sound/lc7535.h,SOUNDS["LC7535"] = true
---------------------------------------------------

if (SOUNDS["LC7535"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/lc7535.cpp",
		MAME_DIR .. "src/devices/sound/lc7535.h",
	}
end

---------------------------------------------------
-- Sanyo LC78836M
--@src/devices/sound/lc78836m.h,SOUNDS["LC78836M"] = true
---------------------------------------------------

if (SOUNDS["LC78836M"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/lc78836m.cpp",
		MAME_DIR .. "src/devices/sound/lc78836m.h",
	}
end

---------------------------------------------------
-- Sanyo LC82310
--@src/devices/sound/lc82310.h,SOUNDS["LC82310"] = true
---------------------------------------------------

if (SOUNDS["LC82310"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/lc82310.cpp",
		MAME_DIR .. "src/devices/sound/lc82310.h",
	}
end

---------------------------------------------------
-- NEC uPD933
--@src/devices/sound/upd933.h,SOUNDS["UPD933"] = true
---------------------------------------------------

if (SOUNDS["UPD933"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/upd933.cpp",
		MAME_DIR .. "src/devices/sound/upd933.h",
	}
end

---------------------------------------------------
-- NEC uPD934G
--@src/devices/sound/upd934g.h,SOUNDS["UPD934G"] = true
---------------------------------------------------

if (SOUNDS["UPD934G"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/upd934g.cpp",
		MAME_DIR .. "src/devices/sound/upd934g.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/iopspu.h,SOUNDS["IOPSPU"] = true
---------------------------------------------------

if (SOUNDS["IOPSPU"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/iopspu.cpp",
		MAME_DIR .. "src/devices/sound/iopspu.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/swp00.h,SOUNDS["SWP00"] = true
---------------------------------------------------

if (SOUNDS["SWP00"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/swp00.cpp",
		MAME_DIR .. "src/devices/sound/swp00.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/swp20.h,SOUNDS["SWP20"] = true
---------------------------------------------------

if (SOUNDS["SWP20"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/swp20.cpp",
		MAME_DIR .. "src/devices/sound/swp20.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/swx00.h,SOUNDS["SWX00"] = true
---------------------------------------------------

if (SOUNDS["SWX00"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/swx00.cpp",
		MAME_DIR .. "src/devices/sound/swx00.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/meg.h,SOUNDS["MEG"] = true
---------------------------------------------------

if (SOUNDS["MEG"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/meg.cpp",
		MAME_DIR .. "src/devices/sound/meg.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/xt446.h,SOUNDS["XT446"] = true
---------------------------------------------------

if (SOUNDS["XT446"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/xt446.cpp",
		MAME_DIR .. "src/devices/sound/xt446.h",
	}
end

---------------------------------------------------
-- Roland sample players
--@src/devices/sound/rolandpcm.h,SOUNDS["ROLANDPCM"] = true
---------------------------------------------------

if (SOUNDS["ROLANDPCM"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/rolandpcm.cpp",
		MAME_DIR .. "src/devices/sound/rolandpcm.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/vgm_visualizer.h,SOUNDS["VGMVIZ"] = true
---------------------------------------------------

if (SOUNDS["VGMVIZ"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/vgm_visualizer.cpp",
		MAME_DIR .. "src/devices/sound/vgm_visualizer.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/s_dsp.h,SOUNDS["S_DSP"] = true
---------------------------------------------------

if (SOUNDS["S_DSP"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/s_dsp.cpp",
		MAME_DIR .. "src/devices/sound/s_dsp.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/ks0164.h,SOUNDS["KS0164"] = true
---------------------------------------------------

if (SOUNDS["KS0164"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ks0164.cpp",
		MAME_DIR .. "src/devices/sound/ks0164.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/rp2c33_snd.h,SOUNDS["RP2C33_SOUND"] = true
---------------------------------------------------

if (SOUNDS["RP2C33_SOUND"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/rp2c33_snd.cpp",
		MAME_DIR .. "src/devices/sound/rp2c33_snd.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/tt5665.h,SOUNDS["TT5665"] = true
---------------------------------------------------

if (SOUNDS["TT5665"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/tt5665.cpp",
		MAME_DIR .. "src/devices/sound/tt5665.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/uda1344.h,SOUNDS["UDA1344"] = true
---------------------------------------------------

if (SOUNDS["UDA1344"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/uda1344.cpp",
		MAME_DIR .. "src/devices/sound/uda1344.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/lynx.h,SOUNDS["LYNX"] = true
---------------------------------------------------

if (SOUNDS["LYNX"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/lynx.cpp",
		MAME_DIR .. "src/devices/sound/lynx.h",
	}
end

---------------------------------------------------
--
--@src/devices/sound/nn71003f.h,SOUNDS["NN71003F"] = true
---------------------------------------------------

if (SOUNDS["NN71003F"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/nn71003f.cpp",
		MAME_DIR .. "src/devices/sound/nn71003f.h",
	}
end

---------------------------------------------------
-- AP2010
--@src/devices/sound/ap2010pcm.h,SOUNDS["AP2010"] = true
---------------------------------------------------

if (SOUNDS["AP2010"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/ap2010pcm.cpp",
		MAME_DIR .. "src/devices/sound/ap2010pcm.h",
	}
end

---------------------------------------------------
-- Texas Instruments CF61909
--@src/devices/sound/cf61909.h,SOUNDS["CF61909"] = true
---------------------------------------------------

if (SOUNDS["CF61909"]~=null) then
	files {
		MAME_DIR .. "src/devices/sound/cf61909.cpp",
		MAME_DIR .. "src/devices/sound/cf61909.h",
	}
end
