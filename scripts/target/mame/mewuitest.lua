-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   mewuitest.lua
--
--   Mewui test target makefile
--
---------------------------------------------------------------------------

--dofile("arcade.lua")
--dofile("mess.lua")

--------------------------------------------------
-- specify available BUSES cores
--------------------------------------------------

BUSES["C64"] = true
BUSES["CBM2"] = true
BUSES["CBMIEC"] = true
BUSES["CENTRONICS"] = true
BUSES["CPC"] = true
BUSES["GAMEBOY"] = true
BUSES["GBA"] = true
BUSES["GENERIC"] = true
BUSES["IEEE488"] = true
BUSES["ISA"] = true
BUSES["MIDI"] = true
BUSES["NEOGEO"] = true
BUSES["NES"] = true
BUSES["NES_CTRL"] = true
BUSES["PC_KBD"] = true
BUSES["PET"] = true
BUSES["PLUS4"] = true
BUSES["RS232"] = true
BUSES["SCSI"] = true
BUSES["SNES"] = true
BUSES["SNES_CTRL"] = true
BUSES["VCS"] = true
BUSES["VCS_CTRL"] = true
BUSES["VIC10"] = true
BUSES["VIC20"] = true
BUSES["ZORRO"] = true

--------------------------------------------------
-- specify available CPUS cores
--------------------------------------------------

CPUS["ARM7"] = true
CPUS["G65816"] = true
CPUS["I8085"] = true
CPUS["I86"] = true
CPUS["LR35902"] = true
CPUS["M6502"] = true
CPUS["M6800"] = true
CPUS["M6809"] = true
CPUS["M6805"] = true
CPUS["M680X0"] = true
CPUS["MCS48"] = true
CPUS["MCS51"] = true
CPUS["MINX"] = true
CPUS["MIPS"] = true
CPUS["S2650"] = true
CPUS["SPC700"] = true
CPUS["SUPERFX"] = true
CPUS["RSP"] = true
CPUS["UPD7725"] = true
CPUS["UPD7810"] = true
CPUS["Z80"] = true

--------------------------------------------------
-- specify available MACHINES cores
--------------------------------------------------

MACHINES["6522VIA"] = true
MACHINES["6821PIA"] = true
MACHINES["6840PTM"] = true
MACHINES["ACIA6850"] = true
MACHINES["AKIKO"] = true
MACHINES["AMIGAFDC"] = true
MACHINES["AT_KEYBC"] = true
MACHINES["AUTOCONFIG"] = true
MACHINES["BANKDEV"] = true
MACHINES["CMOS40105"] = true
MACHINES["COM8116"] = true
MACHINES["CORVUSHD"] = true
MACHINES["CR511B"] = true
MACHINES["DMAC"] = true
MACHINES["DS1302"] = true
MACHINES["DS1315"] = true
MACHINES["DS75160A"] = true
MACHINES["DS75161A"] = true
MACHINES["E05A30"] = true
MACHINES["E05A03"] = true
MACHINES["EEPROMDEV"] = true
MACHINES["GAYLE"] = true
MACHINES["I8255"] = true
MACHINES["I8257"] = true
MACHINES["I8251"] = true
MACHINES["I2CMEM"] = true
MACHINES["IDE"] = true
MACHINES["INS8250"] = true
MACHINES["INTELFLASH"] = true
MACHINES["LATCH8"] = true
MACHINES["MC146818"] = true
MACHINES["MC6852"] = true
MACHINES["MIOT6530"] = true
MACHINES["MOS6526"] = true
MACHINES["MOS6529"] = true
MACHINES["MOS6551"] = true
MACHINES["MOS6702"] = true
MACHINES["MOS8706"] = true
MACHINES["MOS8722"] = true
MACHINES["MOS8726"] = true
MACHINES["MSM58321"] = true
MACHINES["MSM6242"] = true
MACHINES["NETLIST"] = true
MACHINES["PC_LPT"] = true
MACHINES["PCKEYBRD"] = true
MACHINES["PIC8259"] = true
MACHINES["PIT8253"] = true
MACHINES["PLA"] = true
MACHINES["R64H156"] = true
MACHINES["RP5C01"] = true
MACHINES["RP5H01"] = true
MACHINES["STEPPERS"] = true
MACHINES["TIMEKPR"] = true
MACHINES["TMS6100"] = true
MACHINES["TPI6525"] = true
MACHINES["TTL74145"] = true
MACHINES["UPD1990A"] = true
MACHINES["UPD765"] = true
MACHINES["V3021"] = true
MACHINES["WD_FDC"] = true
MACHINES["WD33C93"] = true
MACHINES["Z80DMA"] = true
MACHINES["Z80PIO"] = true
MACHINES["Z80DART"] = true
MACHINES["Z80CTC"] = true

--------------------------------------------------
-- specify available VIDEOS cores
--------------------------------------------------

VIDEOS["BUFSPRITE"] = true
VIDEOS["DL1416"] = true
VIDEOS["MC6845"] = true
VIDEOS["MOS6566"] = true
VIDEOS["SNES_PPU"] = true

--------------------------------------------------
-- specify available SOUNDS cores
--------------------------------------------------

SOUNDS["AY8910"] = true
SOUNDS["AMIGA"] = true
SOUNDS["BEEP"] = true
SOUNDS["CDDA"] = true
SOUNDS["DAC"] = true
SOUNDS["DISCRETE"] = true
SOUNDS["DMADAC"] = true
SOUNDS["ICS2115"] = true
SOUNDS["MOS7360"] = true
SOUNDS["MOS656X"] = true
SOUNDS["M58817"] = true
SOUNDS["MSM5205"] = true
SOUNDS["NES_APU"] = true
SOUNDS["OKIM6295"] = true
SOUNDS["QSOUND"] = true
SOUNDS["SID6581"] = true
SOUNDS["SID8580"] = true
SOUNDS["SN76477"] = true
SOUNDS["SN76496"] = true
SOUNDS["SP0256"] = true
SOUNDS["SPEAKER"] = true
SOUNDS["T6721A"] = true
SOUNDS["TMS5110"] = true
SOUNDS["VLM5030"] = true
SOUNDS["VRC6"] = true
SOUNDS["WAVE"] = true
SOUNDS["YM2151"] = true
SOUNDS["YM2203"] = true
SOUNDS["YM2413"] = true
SOUNDS["YM2608"] = true
SOUNDS["YM2610"] = true
SOUNDS["YM3526"] = true


function createProjects_mame_mewuitest(_target, _subtarget)
	project ("mame_mewuitest")
	targetsubdir(_target .."_" .. _subtarget)
	kind "StaticLib"
	uuid (os.uuid("drv-mame_mewuitest"))

	options {
		"ForceCPP",
	}

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/mess",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/emu/netlist",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/zlib",
		GEN_DIR  .. "mess/layout",
		GEN_DIR  .. "mame/layout",
	}

	files{
		-- Capcom
		MAME_DIR .. "src/mame/drivers/1942.c",
		MAME_DIR .. "src/mame/video/1942.c",
		MAME_DIR .. "src/mame/drivers/1943.c",
		MAME_DIR .. "src/mame/video/1943.c",
		MAME_DIR .. "src/mame/drivers/alien.c",
		MAME_DIR .. "src/mame/drivers/bionicc.c",
		MAME_DIR .. "src/mame/video/bionicc.c",
		MAME_DIR .. "src/mame/drivers/supduck.c",
		MAME_DIR .. "src/mame/video/tigeroad_spr.c",
		MAME_DIR .. "src/mame/drivers/blktiger.c",
		MAME_DIR .. "src/mame/video/blktiger.c",
		MAME_DIR .. "src/mame/drivers/cbasebal.c",
		MAME_DIR .. "src/mame/video/cbasebal.c",
		MAME_DIR .. "src/mame/drivers/commando.c",
		MAME_DIR .. "src/mame/video/commando.c",
		MAME_DIR .. "src/mame/drivers/cps1.c",
		MAME_DIR .. "src/mame/video/cps1.c",
		MAME_DIR .. "src/mame/drivers/kenseim.c",
		MAME_DIR .. "src/mame/drivers/cps2.c",
		MAME_DIR .. "src/mame/machine/cps2crpt.c",
		MAME_DIR .. "src/mame/drivers/cps3.c",
		MAME_DIR .. "src/mame/audio/cps3.c",
		MAME_DIR .. "src/mame/drivers/egghunt.c",
		MAME_DIR .. "src/mame/drivers/exedexes.c",
		MAME_DIR .. "src/mame/video/exedexes.c",
		MAME_DIR .. "src/mame/drivers/fcrash.c",
		MAME_DIR .. "src/mame/drivers/gng.c",
		MAME_DIR .. "src/mame/video/gng.c",
		MAME_DIR .. "src/mame/drivers/gunsmoke.c",
		MAME_DIR .. "src/mame/video/gunsmoke.c",
		MAME_DIR .. "src/mame/drivers/higemaru.c",
		MAME_DIR .. "src/mame/video/higemaru.c",
		MAME_DIR .. "src/mame/drivers/lastduel.c",
		MAME_DIR .. "src/mame/video/lastduel.c",
		MAME_DIR .. "src/mame/drivers/lwings.c",
		MAME_DIR .. "src/mame/video/lwings.c",
		MAME_DIR .. "src/mame/drivers/mitchell.c",
		MAME_DIR .. "src/mame/video/mitchell.c",
		MAME_DIR .. "src/mame/drivers/sf.c",
		MAME_DIR .. "src/mame/video/sf.c",
		MAME_DIR .. "src/mame/drivers/sidearms.c",
		MAME_DIR .. "src/mame/video/sidearms.c",
		MAME_DIR .. "src/mame/drivers/sonson.c",
		MAME_DIR .. "src/mame/video/sonson.c",
		MAME_DIR .. "src/mame/drivers/srumbler.c",
		MAME_DIR .. "src/mame/video/srumbler.c",
		MAME_DIR .. "src/mame/drivers/tigeroad.c",
		MAME_DIR .. "src/mame/video/tigeroad.c",
		MAME_DIR .. "src/mame/machine/tigeroad.c",
		MAME_DIR .. "src/mame/drivers/vulgus.c",
		MAME_DIR .. "src/mame/video/vulgus.c",
		MAME_DIR .. "src/mame/machine/kabuki.c",

		-- IGS
		MAME_DIR .. "src/mame/drivers/cabaret.c",
		MAME_DIR .. "src/mame/drivers/ddz.c",
		MAME_DIR .. "src/mame/drivers/dunhuang.c",
		MAME_DIR .. "src/mame/drivers/goldstar.c",
		MAME_DIR .. "src/mame/video/goldstar.c",
		MAME_DIR .. "src/mame/drivers/jackie.c",
		MAME_DIR .. "src/mame/drivers/igspoker.c",
		MAME_DIR .. "src/mame/drivers/igs009.c",
		MAME_DIR .. "src/mame/drivers/igs011.c",
		MAME_DIR .. "src/mame/drivers/igs017.c",
		MAME_DIR .. "src/mame/drivers/igs_m027.c",
		MAME_DIR .. "src/mame/drivers/igs_m036.c",
		MAME_DIR .. "src/mame/drivers/iqblock.c",
		MAME_DIR .. "src/mame/video/iqblock.c",
		MAME_DIR .. "src/mame/drivers/lordgun.c",
		MAME_DIR .. "src/mame/video/lordgun.c",
		MAME_DIR .. "src/mame/drivers/pgm.c",
		MAME_DIR .. "src/mame/video/pgm.c",
		MAME_DIR .. "src/mame/machine/pgmprot_igs027a_type1.c",
		MAME_DIR .. "src/mame/machine/pgmprot_igs027a_type2.c",
		MAME_DIR .. "src/mame/machine/pgmprot_igs027a_type3.c",
		MAME_DIR .. "src/mame/machine/pgmprot_igs025_igs012.c",
		MAME_DIR .. "src/mame/machine/pgmprot_igs025_igs022.c",
		MAME_DIR .. "src/mame/machine/pgmprot_igs025_igs028.c",
		MAME_DIR .. "src/mame/machine/pgmprot_orlegend.c",
		MAME_DIR .. "src/mame/drivers/pgm2.c",
		MAME_DIR .. "src/mame/drivers/spoker.c",
		MAME_DIR .. "src/mame/machine/igs036crypt.c",
		MAME_DIR .. "src/mame/machine/pgmcrypt.c",
		MAME_DIR .. "src/mame/machine/igs025.c",
		MAME_DIR .. "src/mame/machine/igs022.c",
		MAME_DIR .. "src/mame/machine/igs028.c",

		-- Amstrad
		MAME_DIR .. "src/mess/drivers/amstrad.c",
		MAME_DIR .. "src/mess/machine/amstrad.c",
		MAME_DIR .. "src/mess/drivers/amstr_pc.c",
		MAME_DIR .. "src/mess/drivers/nc.c",
		MAME_DIR .. "src/mess/machine/nc.c",
		MAME_DIR .. "src/mess/video/nc.c",
		MAME_DIR .. "src/mess/drivers/pc1512.c",
		MAME_DIR .. "src/mess/machine/pc1512kb.c",
		MAME_DIR .. "src/mess/video/pc1512.c",
		MAME_DIR .. "src/mess/drivers/pcw.c",
		MAME_DIR .. "src/mess/video/pcw.c",
		MAME_DIR .. "src/mess/drivers/pcw16.c",
		MAME_DIR .. "src/mess/video/pcw16.c",
		MAME_DIR .. "src/mess/drivers/pda600.c",

		-- CBM
		MAME_DIR .. "src/mess/drivers/c128.c",
		MAME_DIR .. "src/mess/drivers/c64.c",
		MAME_DIR .. "src/mess/drivers/c64dtv.c",
		MAME_DIR .. "src/mess/drivers/c65.c",
		MAME_DIR .. "src/mess/drivers/c900.c",
		MAME_DIR .. "src/mess/drivers/cbm2.c",
		MAME_DIR .. "src/mess/drivers/clcd.c",
		MAME_DIR .. "src/mess/drivers/pet.c",
		MAME_DIR .. "src/mess/drivers/plus4.c",
		MAME_DIR .. "src/mess/drivers/vic10.c",
		MAME_DIR .. "src/mess/drivers/vic20.c",
		MAME_DIR .. "src/mess/machine/cbm_snqk.c",

		-- Amiga
		MAME_DIR .. "src/mess/drivers/amiga.c",
		MAME_DIR .. "src/mess/machine/amigakbd.c",

		-- Rockwell
		MAME_DIR .. "src/mess/drivers/aim65.c",
		MAME_DIR .. "src/mess/machine/aim65.c",
		MAME_DIR .. "src/mess/drivers/aim65_40.c",

		-- MOS
		MAME_DIR .. "src/mess/drivers/kim1.c",

		-- Synertec
		MAME_DIR .. "src/mess/drivers/sym1.c",

		-- Nintendo
		MAME_DIR .. "src/mess/drivers/gb.c",
		MAME_DIR .. "src/mess/audio/gb.c",
		MAME_DIR .. "src/mess/machine/gb.c",
		MAME_DIR .. "src/mess/video/gb_lcd.c",
		MAME_DIR .. "src/mess/drivers/gba.c",
		MAME_DIR .. "src/mess/video/gba.c",
		MAME_DIR .. "src/mess/drivers/n64.c",
		MAME_DIR .. "src/mess/drivers/nes.c",
		MAME_DIR .. "src/mess/machine/nes.c",
		MAME_DIR .. "src/mess/video/nes.c",
		MAME_DIR .. "src/mess/drivers/pokemini.c",
		MAME_DIR .. "src/mess/drivers/snes.c",
		MAME_DIR .. "src/mess/machine/snescx4.c",
		MAME_DIR .. "src/mess/drivers/vboy.c",
		MAME_DIR .. "src/mess/audio/vboy.c",

		MAME_DIR .. "src/mame/drivers/cham24.c",
		MAME_DIR .. "src/mame/drivers/dkong.c",
		MAME_DIR .. "src/mame/audio/dkong.c",
		MAME_DIR .. "src/mame/video/dkong.c",
		MAME_DIR .. "src/mame/drivers/mario.c",
		MAME_DIR .. "src/mame/audio/mario.c",
		MAME_DIR .. "src/mame/video/mario.c",
		MAME_DIR .. "src/mame/drivers/multigam.c",
		MAME_DIR .. "src/mame/drivers/n8080.c",
		MAME_DIR .. "src/mame/audio/n8080.c",
		MAME_DIR .. "src/mame/video/n8080.c",
		MAME_DIR .. "src/mame/drivers/nss.c",
		MAME_DIR .. "src/mame/machine/snes.c",
		MAME_DIR .. "src/mame/audio/snes_snd.c",
		MAME_DIR .. "src/mame/drivers/playch10.c",
		MAME_DIR .. "src/mame/machine/playch10.c",
		MAME_DIR .. "src/mame/video/playch10.c",
		MAME_DIR .. "src/mame/drivers/popeye.c",
		MAME_DIR .. "src/mame/video/popeye.c",
		MAME_DIR .. "src/mame/drivers/punchout.c",
		MAME_DIR .. "src/mame/video/punchout.c",
		MAME_DIR .. "src/mame/drivers/famibox.c",
		MAME_DIR .. "src/mame/drivers/sfcbox.c",
		MAME_DIR .. "src/mame/drivers/snesb.c",
		MAME_DIR .. "src/mame/drivers/spacefb.c",
		MAME_DIR .. "src/mame/audio/spacefb.c",
		MAME_DIR .. "src/mame/video/spacefb.c",
		MAME_DIR .. "src/mame/drivers/vsnes.c",
		MAME_DIR .. "src/mame/machine/vsnes.c",
		MAME_DIR .. "src/mame/video/vsnes.c",
		MAME_DIR .. "src/mame/video/ppu2c0x.c",

		-- MAME shared
		MAME_DIR .. "src/mame/machine/archimds.c",
		MAME_DIR .. "src/mame/video/archimds.c",
		MAME_DIR .. "src/mame/machine/amiga.c",
		MAME_DIR .. "src/mame/video/amiga.c",
		MAME_DIR .. "src/mame/video/amigaaga.c",
		MAME_DIR .. "src/mame/video/tia.c",
		MAME_DIR .. "src/mame/machine/atari.c",
		MAME_DIR .. "src/mame/video/atari.c",
		MAME_DIR .. "src/mame/video/antic.c",
		MAME_DIR .. "src/mame/video/gtia.c",
		MAME_DIR .. "src/mame/drivers/jaguar.c",
		MAME_DIR .. "src/mame/audio/jaguar.c",
		MAME_DIR .. "src/mame/video/jaguar.c",
		MAME_DIR .. "src/mame/audio/gorf.c",
		MAME_DIR .. "src/mame/audio/wow.c",
		MAME_DIR .. "src/mame/drivers/astrocde.c",
		MAME_DIR .. "src/mame/video/astrocde.c",
		MAME_DIR .. "src/mame/machine/kabuki.c",
		MAME_DIR .. "src/mame/video/pk8000.c",
		MAME_DIR .. "src/mame/video/ppu2c0x.c",
		MAME_DIR .. "src/mame/machine/snes.c",
		MAME_DIR .. "src/mame/audio/snes_snd.c",
		MAME_DIR .. "src/mame/machine/n64.c",
		MAME_DIR .. "src/mame/video/n64.c",
		MAME_DIR .. "src/mame/video/rdpblend.c",
		MAME_DIR .. "src/mame/video/rdptpipe.c",
		MAME_DIR .. "src/mame/machine/megadriv.c",
		MAME_DIR .. "src/mame/drivers/naomi.c",
		MAME_DIR .. "src/mame/machine/awboard.c",
		MAME_DIR .. "src/mame/machine/dc.c",
		MAME_DIR .. "src/mame/machine/dc-ctrl.c",
		MAME_DIR .. "src/mame/machine/gdrom.c",
		MAME_DIR .. "src/mame/machine/jvs13551.c",
		MAME_DIR .. "src/mame/machine/maple-dc.c",
		MAME_DIR .. "src/mame/machine/mapledev.c",
		MAME_DIR .. "src/mame/machine/mie.c",
		MAME_DIR .. "src/mame/machine/naomi.c",
		MAME_DIR .. "src/mame/machine/naomibd.c",
		MAME_DIR .. "src/mame/machine/naomig1.c",
		MAME_DIR .. "src/mame/machine/naomigd.c",
		MAME_DIR .. "src/mame/machine/naomim1.c",
		MAME_DIR .. "src/mame/machine/naomim2.c",
		MAME_DIR .. "src/mame/machine/naomim4.c",
		MAME_DIR .. "src/mame/machine/naomirom.c",
		MAME_DIR .. "src/mame/machine/315-5881_crypt.c",
		MAME_DIR .. "src/mame/video/powervr2.c",
		MAME_DIR .. "src/mame/drivers/neogeo.c",
		MAME_DIR .. "src/mame/machine/neocrypt.c",
		MAME_DIR .. "src/mame/machine/ng_memcard.c",
		MAME_DIR .. "src/mame/video/neogeo.c",
		MAME_DIR .. "src/mame/video/neogeo_spr.c",
		MAME_DIR .. "src/mame/drivers/cdi.c",
		MAME_DIR .. "src/mame/machine/cdi070.c",
		MAME_DIR .. "src/mame/machine/cdicdic.c",
		MAME_DIR .. "src/mame/machine/cdislave.c",
		MAME_DIR .. "src/mame/video/mcd212.c",
		MAME_DIR .. "src/mame/drivers/3do.c",
		MAME_DIR .. "src/mame/machine/3do.c",
		MAME_DIR .. "src/mame/drivers/konamim2.c",
		MAME_DIR .. "src/mame/drivers/vectrex.c",
		MAME_DIR .. "src/mame/machine/vectrex.c",
		MAME_DIR .. "src/mame/video/vectrex.c",
		MAME_DIR .. "src/mame/drivers/cps1.c",
		MAME_DIR .. "src/mame/video/cps1.c",
		MAME_DIR .. "src/mame/video/chihiro.c",
		MAME_DIR .. "src/mame/machine/xbox.c",


		-- Neogeo
		MAME_DIR .. "src/mame/drivers/neogeo.c",
		MAME_DIR .. "src/mame/video/neogeo.c",
		MAME_DIR .. "src/mame/drivers/neogeo_noslot.c",
		MAME_DIR .. "src/mame/video/neogeo_spr.c",
		MAME_DIR .. "src/mame/machine/neocrypt.c",
		MAME_DIR .. "src/mame/machine/ng_memcard.c",

	}
end

function linkProjects_mame_mewuitest(_target, _subtarget)
	links {
		"mame_mewuitest",
	}
end
