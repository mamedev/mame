-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   video.lua
--
--   Rules for building video cores
--
---------------------------------------------------------------------------

--------------------------------------------------
--
--@src/devices/video/315_5124.h,VIDEOS["SEGA315_5124"] = true
--------------------------------------------------

if (VIDEOS["SEGA315_5124"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/315_5124.cpp",
		MAME_DIR .. "src/devices/video/315_5124.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/315_5313.h,VIDEOS["SEGA315_5313"] = true
--------------------------------------------------

if (VIDEOS["SEGA315_5313"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/315_5313.cpp",
		MAME_DIR .. "src/devices/video/315_5313.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/bufsprite.h,VIDEOS["BUFSPRITE"] = true
--------------------------------------------------

if (VIDEOS["BUFSPRITE"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/bufsprite.cpp",
		MAME_DIR .. "src/devices/video/bufsprite.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/cdp1861.h,VIDEOS["CDP1861"] = true
--------------------------------------------------

if (VIDEOS["CDP1861"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/cdp1861.cpp",
		MAME_DIR .. "src/devices/video/cdp1861.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/cdp1862.h,VIDEOS["CDP1862"] = true
--------------------------------------------------

if (VIDEOS["CDP1862"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/cdp1862.cpp",
		MAME_DIR .. "src/devices/video/cdp1862.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/crt9007.h,VIDEOS["CRT9007"] = true
--------------------------------------------------

if (VIDEOS["CRT9007"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/crt9007.cpp",
		MAME_DIR .. "src/devices/video/crt9007.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/crt9021.h,VIDEOS["CRT9021"] = true
--------------------------------------------------

if (VIDEOS["CRT9021"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/crt9021.cpp",
		MAME_DIR .. "src/devices/video/crt9021.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/crt9212.h,VIDEOS["CRT9212"] = true
--------------------------------------------------

if (VIDEOS["CRT9212"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/crt9212.cpp",
		MAME_DIR .. "src/devices/video/crt9212.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/dl1416.h,VIDEOS["DL1416"] = true
--------------------------------------------------

if (VIDEOS["DL1416"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/dl1416.cpp",
		MAME_DIR .. "src/devices/video/dl1416.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/dm9368.h,VIDEOS["DM9368"] = true
--------------------------------------------------

if (VIDEOS["DM9368"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/dm9368.cpp",
		MAME_DIR .. "src/devices/video/dm9368.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/ef9340_1.h,VIDEOS["EF9340_1"] = true
--------------------------------------------------

if (VIDEOS["EF9340_1"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ef9340_1.cpp",
		MAME_DIR .. "src/devices/video/ef9340_1.h",
		MAME_DIR .. "src/devices/video/ef9341_chargen.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/ef9345.h,VIDEOS["EF9345"] = true
--------------------------------------------------

if (VIDEOS["EF9345"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ef9345.cpp",
		MAME_DIR .. "src/devices/video/ef9345.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/ef9365.h,VIDEOS["EF9365"] = true
--------------------------------------------------

if (VIDEOS["EF9365"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ef9365.cpp",
		MAME_DIR .. "src/devices/video/ef9365.h",
	}
end

--------------------------------------------------
--@src/devices/video/epic12.h,VIDEOS["EPIC12"] = true
--------------------------------------------------

if (VIDEOS["EPIC12"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/epic12.cpp",
		MAME_DIR .. "src/devices/video/epic12.h",
		MAME_DIR .. "src/devices/video/epic12_blit0.cpp",
		MAME_DIR .. "src/devices/video/epic12_blit1.cpp",
		MAME_DIR .. "src/devices/video/epic12_blit2.cpp",
		MAME_DIR .. "src/devices/video/epic12_blit3.cpp",
		MAME_DIR .. "src/devices/video/epic12_blit4.cpp",
		MAME_DIR .. "src/devices/video/epic12_blit5.cpp",
		MAME_DIR .. "src/devices/video/epic12_blit6.cpp",
		MAME_DIR .. "src/devices/video/epic12_blit7.cpp",
		MAME_DIR .. "src/devices/video/epic12_blit8.cpp",
		MAME_DIR .. "src/devices/video/epic12in.inc",
		MAME_DIR .. "src/devices/video/epic12pixel.inc",
	}
end

--------------------------------------------------
--
--@src/devices/video/fixfreq.h,VIDEOS["FIXFREQ"] = true
--------------------------------------------------

if (VIDEOS["FIXFREQ"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/fixfreq.cpp",
		MAME_DIR .. "src/devices/video/fixfreq.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/gf4500.h,VIDEOS["GF4500"] = true
--------------------------------------------------

if (VIDEOS["GF4500"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/gf4500.cpp",
		MAME_DIR .. "src/devices/video/gf4500.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/gf7600gs.h,VIDEOS["GF7600GS"] = true
--------------------------------------------------

if (VIDEOS["GF7600GS"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/gf7600gs.cpp",
		MAME_DIR .. "src/devices/video/gf7600gs.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/h63484.h,VIDEOS["H63484"] = true
--------------------------------------------------

if (VIDEOS["H63484"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/h63484.cpp",
		MAME_DIR .. "src/devices/video/h63484.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/hd44102.h,VIDEOS["HD44102"] = true
--------------------------------------------------

if (VIDEOS["HD44102"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hd44102.cpp",
		MAME_DIR .. "src/devices/video/hd44102.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/hd44352.h,VIDEOS["HD44352"] = true
--------------------------------------------------

if (VIDEOS["HD44352"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hd44352.cpp",
		MAME_DIR .. "src/devices/video/hd44352.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/hd44780.h,VIDEOS["HD44780"] = true
--------------------------------------------------

if (VIDEOS["HD44780"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hd44780.cpp",
		MAME_DIR .. "src/devices/video/hd44780.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/hd61830.h,VIDEOS["HD61830"] = true
--------------------------------------------------

if (VIDEOS["HD61830"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hd61830.cpp",
		MAME_DIR .. "src/devices/video/hd61830.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/hd63484.h,VIDEOS["HD63484"] = true
--------------------------------------------------

if (VIDEOS["HD63484"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hd63484.cpp",
		MAME_DIR .. "src/devices/video/hd63484.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/hd66421.h,VIDEOS["HD66421"] = true
--------------------------------------------------

if (VIDEOS["HD66421"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hd66421.cpp",
		MAME_DIR .. "src/devices/video/hd66421.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/huc6202.h,VIDEOS["HUC6202"] = true
--------------------------------------------------

if (VIDEOS["HUC6202"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/huc6202.cpp",
		MAME_DIR .. "src/devices/video/huc6202.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/huc6260.h,VIDEOS["HUC6260"] = true
--------------------------------------------------

if (VIDEOS["HUC6260"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/huc6260.cpp",
		MAME_DIR .. "src/devices/video/huc6260.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/huc6261.h,VIDEOS["HUC6261"] = true
--------------------------------------------------

if (VIDEOS["HUC6261"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/huc6261.cpp",
		MAME_DIR .. "src/devices/video/huc6261.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/huc6270.h,VIDEOS["HUC6270"] = true
--------------------------------------------------

if (VIDEOS["HUC6270"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/huc6270.cpp",
		MAME_DIR .. "src/devices/video/huc6270.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/huc6272.h,VIDEOS["HUC6272"] = true
--------------------------------------------------

if (VIDEOS["HUC6272"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/huc6272.cpp",
		MAME_DIR .. "src/devices/video/huc6272.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/i8244.h,VIDEOS["I8244"] = true
--------------------------------------------------

if (VIDEOS["I8244"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/i8244.cpp",
		MAME_DIR .. "src/devices/video/i8244.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/i82730.h,VIDEOS["I82730"] = true
--------------------------------------------------

if (VIDEOS["I82730"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/i82730.cpp",
		MAME_DIR .. "src/devices/video/i82730.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/i8275.h,VIDEOS["I8275"] = true
--------------------------------------------------

if (VIDEOS["I8275"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/i8275.cpp",
		MAME_DIR .. "src/devices/video/i8275.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/m50458.h,VIDEOS["M50458"] = true
--------------------------------------------------

if (VIDEOS["M50458"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/m50458.cpp",
		MAME_DIR .. "src/devices/video/m50458.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/mb90082.h,VIDEOS["MB90082"] = true
--------------------------------------------------

if (VIDEOS["MB90082"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/mb90082.cpp",
		MAME_DIR .. "src/devices/video/mb90082.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/mb_vcu.h,VIDEOS["MB_VCU"] = true
--------------------------------------------------

if (VIDEOS["MB_VCU"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/mb_vcu.cpp",
		MAME_DIR .. "src/devices/video/mb_vcu.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/mc6845.h,VIDEOS["MC6845"] = true
--------------------------------------------------

if (VIDEOS["MC6845"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/mc6845.cpp",
		MAME_DIR .. "src/devices/video/mc6845.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/mc6847.h,VIDEOS["MC6847"] = true
--------------------------------------------------

if (VIDEOS["MC6847"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/mc6847.cpp",
		MAME_DIR .. "src/devices/video/mc6847.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/msm6222b.h,VIDEOS["MSM6222B"] = true
--------------------------------------------------

if (VIDEOS["MSM6222B"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/msm6222b.cpp",
		MAME_DIR .. "src/devices/video/msm6222b.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/msm6255.h,VIDEOS["MSM6255"] = true
--------------------------------------------------

if (VIDEOS["MSM6255"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/msm6255.cpp",
		MAME_DIR .. "src/devices/video/msm6255.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/mos6566.h,VIDEOS["MOS6566"] = true
--------------------------------------------------

if (VIDEOS["MOS6566"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/mos6566.cpp",
		MAME_DIR .. "src/devices/video/mos6566.h",
	}
end


files {	
	MAME_DIR .. "src/devices/video/cgapal.cpp",
	MAME_DIR .. "src/devices/video/cgapal.h",	
}

--------------------------------------------------
--
--@src/devices/video/pc_vga.h,VIDEOS["PC_VGA"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga.cpp",
		MAME_DIR .. "src/devices/video/pc_vga.h",
		MAME_DIR .. "src/devices/bus/isa/trident.cpp",
		MAME_DIR .. "src/devices/bus/isa/trident.h",
		MAME_DIR .. "src/devices/video/clgd542x.cpp",
		MAME_DIR .. "src/devices/video/clgd542x.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pcd8544.h,VIDEOS["PCD8544"] = true
--------------------------------------------------

if (VIDEOS["PCD8544"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pcd8544.cpp",
		MAME_DIR .. "src/devices/video/pcd8544.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/polylgcy.h,VIDEOS["POLY"] = true
--------------------------------------------------

if (VIDEOS["POLY"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/polylgcy.cpp",
		MAME_DIR .. "src/devices/video/polylgcy.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/psx.h,VIDEOS["PSX"] = true
--------------------------------------------------

if (VIDEOS["PSX"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/psx.cpp",
		MAME_DIR .. "src/devices/video/psx.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/ramdac.h,VIDEOS["RAMDAC"] = true
--------------------------------------------------

if (VIDEOS["RAMDAC"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ramdac.cpp",
		MAME_DIR .. "src/devices/video/ramdac.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/saa5050.h,VIDEOS["SAA5050"] = true
--------------------------------------------------

if (VIDEOS["SAA5050"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/saa5050.cpp",
		MAME_DIR .. "src/devices/video/saa5050.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/sed1200.h,VIDEOS["SED1200"] = true
--------------------------------------------------
if (VIDEOS["SED1200"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/sed1200.cpp",
		MAME_DIR .. "src/devices/video/sed1200.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/sed1330.h,VIDEOS["SED1330"] = true
--------------------------------------------------
if (VIDEOS["SED1330"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/sed1330.cpp",
		MAME_DIR .. "src/devices/video/sed1330.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/sed1520.h,VIDEOS["SED1520"] = true
--------------------------------------------------
if (VIDEOS["SED1520"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/sed1520.cpp",
		MAME_DIR .. "src/devices/video/sed1520.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/scn2674.h,VIDEOS["SCN2674"] = true
--------------------------------------------------
if (VIDEOS["SCN2674"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/scn2674.cpp",
		MAME_DIR .. "src/devices/video/scn2674.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/snes_ppu.h,VIDEOS["SNES_PPU"] = true
--------------------------------------------------
if (VIDEOS["SNES_PPU"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/snes_ppu.cpp",
		MAME_DIR .. "src/devices/video/snes_ppu.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/stvvdp1.h,VIDEOS["STVVDP"] = true
--@src/devices/video/stvvdp2.h,VIDEOS["STVVDP"] = true
--------------------------------------------------

if (VIDEOS["STVVDP"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/stvvdp1.cpp",
		MAME_DIR .. "src/devices/video/stvvdp2.cpp",
	}
end

--------------------------------------------------
--
--@src/devices/video/t6a04.h,VIDEOS["T6A04"] = true
--------------------------------------------------

if (VIDEOS["T6A04"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/t6a04.cpp",
		MAME_DIR .. "src/devices/video/t6a04.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/tea1002.h,VIDEOS["TEA1002"] = true
--------------------------------------------------

if (VIDEOS["TEA1002"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/tea1002.cpp",
		MAME_DIR .. "src/devices/video/tea1002.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/tlc34076.h,VIDEOS["TLC34076"] = true
--------------------------------------------------

if (VIDEOS["TLC34076"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/tlc34076.cpp",
		MAME_DIR .. "src/devices/video/tlc34076.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/tms34061.h,VIDEOS["TMS34061"] = true
--------------------------------------------------

if (VIDEOS["TMS34061"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/tms34061.cpp",
		MAME_DIR .. "src/devices/video/tms34061.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/tms3556.h,VIDEOS["TMS3556"] = true
--------------------------------------------------

if (VIDEOS["TMS3556"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/tms3556.cpp",
		MAME_DIR .. "src/devices/video/tms3556.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/tms9927.h,VIDEOS["TMS9927"] = true
--------------------------------------------------

if (VIDEOS["TMS9927"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/tms9927.cpp",
		MAME_DIR .. "src/devices/video/tms9927.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/tms9928a.h,VIDEOS["TMS9928A"] = true
--------------------------------------------------

if (VIDEOS["TMS9928A"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/tms9928a.cpp",
		MAME_DIR .. "src/devices/video/tms9928a.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/upd3301.h,VIDEOS["UPD3301"] = true
--------------------------------------------------

if (VIDEOS["UPD3301"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/upd3301.cpp",
		MAME_DIR .. "src/devices/video/upd3301.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/upd7220.h,VIDEOS["UPD7220"] = true
--------------------------------------------------

if (VIDEOS["UPD7220"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/upd7220.cpp",
		MAME_DIR .. "src/devices/video/upd7220.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/upd7227.h,VIDEOS["UPD7227"] = true
--------------------------------------------------

if (VIDEOS["UPD7227"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/upd7227.cpp",
		MAME_DIR .. "src/devices/video/upd7227.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/vic4567.h,VIDEOS["VIC4567"] = true
--------------------------------------------------

if (VIDEOS["VIC4567"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/vic4567.cpp",
		MAME_DIR .. "src/devices/video/vic4567.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/v9938.h,VIDEOS["V9938"] = true
--------------------------------------------------

if (VIDEOS["V9938"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/v9938.cpp",
		MAME_DIR .. "src/devices/video/v9938.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/voodoo.h,VIDEOS["VOODOO"] = true
--------------------------------------------------

if (VIDEOS["VOODOO"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/voodoo.cpp",
		MAME_DIR .. "src/devices/video/voodoo.h",
		MAME_DIR .. "src/devices/video/vooddefs.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/voodoo_pci.h,VIDEOS["VOODOO_PCI"] = true
--------------------------------------------------

if (VIDEOS["VOODOO_PCI"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/voodoo_pci.cpp",
		MAME_DIR .. "src/devices/video/voodoo_pci.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/crtc_ega.h,VIDEOS["CRTC_EGA"] = true
--------------------------------------------------

if (VIDEOS["CRTC_EGA"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/crtc_ega.cpp",
		MAME_DIR .. "src/devices/video/crtc_ega.h",
	}
end
