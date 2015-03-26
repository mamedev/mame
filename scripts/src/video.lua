---------------------------------------------------------------------------
--
--   video.lua
--
--   Rules for building video cores
--
--   Copyright Nicola Salmoria and the MAME Team.
--   Visit http://mamedev.org for licensing and usage restrictions.
--
---------------------------------------------------------------------------

--------------------------------------------------
--
--@src/emu/video/315_5124.h,VIDEOS += SEGA315_5124
--------------------------------------------------

if (VIDEOS["SEGA315_5124"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/315_5124.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/315_5313.h,VIDEOS += SEGA315_5313
--------------------------------------------------

if (VIDEOS["SEGA315_5313"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/315_5313.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/bufsprite.h,VIDEOS += BUFSPRITE
--------------------------------------------------

if (VIDEOS["BUFSPRITE"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/bufsprite.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/cdp1861.h,VIDEOS += CDP1861
--------------------------------------------------

if (VIDEOS["CDP1861"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/cdp1861.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/cdp1862.h,VIDEOS += CDP1862
--------------------------------------------------

if (VIDEOS["CDP1862"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/cdp1862.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/crt9007.h,VIDEOS += CRT9007
--------------------------------------------------

if (VIDEOS["CRT9007"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/crt9007.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/crt9021.h,VIDEOS += CRT9021
--------------------------------------------------

if (VIDEOS["CRT9021"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/crt9021.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/crt9212.h,VIDEOS += CRT9212
--------------------------------------------------

if (VIDEOS["CRT9212"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/crt9212.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/dl1416.h,VIDEOS += DL1416
--------------------------------------------------

if (VIDEOS["DL1416"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/dl1416.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/dm9368.h,VIDEOS += DM9368
--------------------------------------------------

if (VIDEOS["DM9368"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/dm9368.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/ef9340_1.h,VIDEOS += EF9340_1
--------------------------------------------------

if (VIDEOS["EF9340_1"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/ef9340_1.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/ef9345.h,VIDEOS += EF9345
--------------------------------------------------

if (VIDEOS["EF9345"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/ef9345.*",
	}
end

--------------------------------------------------
--@src/emu/video/epic12.h,VIDEOS += EPIC12
--------------------------------------------------

if (VIDEOS["EPIC12"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/epic12.*",
		MAME_DIR .. "src/emu/video/epic12_blit0.*",
		MAME_DIR .. "src/emu/video/epic12_blit1.*",
		MAME_DIR .. "src/emu/video/epic12_blit2.*",
		MAME_DIR .. "src/emu/video/epic12_blit3.*",
		MAME_DIR .. "src/emu/video/epic12_blit4.*",
		MAME_DIR .. "src/emu/video/epic12_blit5.*",
		MAME_DIR .. "src/emu/video/epic12_blit6.*",
		MAME_DIR .. "src/emu/video/epic12_blit7.*",
		MAME_DIR .. "src/emu/video/epic12_blit8.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/fixfreq.h,VIDEOS += FIXFREQ
--------------------------------------------------

if (VIDEOS["FIXFREQ"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/fixfreq.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/gf4500.h,VIDEOS += GF4500
--------------------------------------------------

if (VIDEOS["GF4500"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/gf4500.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/gf7600gs.h,VIDEOS += GF7600GS
--------------------------------------------------

if (VIDEOS["GF7600GS"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/gf7600gs.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/h63484.h,VIDEOS += H63484
--------------------------------------------------

if (VIDEOS["H63484"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/h63484.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/hd44102.h,VIDEOS += HD44102
--------------------------------------------------

if (VIDEOS["HD44102"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/hd44102.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/hd44352.h,VIDEOS += HD44352
--------------------------------------------------

if (VIDEOS["HD44352"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/hd44352.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/hd44780.h,VIDEOS += HD44780
--------------------------------------------------

if (VIDEOS["HD44780"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/hd44780.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/hd61830.h,VIDEOS += HD61830
--------------------------------------------------

if (VIDEOS["HD61830"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/hd61830.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/hd63484.h,VIDEOS += HD63484
--------------------------------------------------

if (VIDEOS["HD63484"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/hd63484.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/hd66421.h,VIDEOS += HD66421
--------------------------------------------------

if (VIDEOS["HD66421"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/hd66421.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/huc6202.h,VIDEOS += HUC6202
--------------------------------------------------

if (VIDEOS["HUC6202"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/huc6202.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/huc6260.h,VIDEOS += HUC6260
--------------------------------------------------

if (VIDEOS["HUC6260"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/huc6260.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/huc6261.h,VIDEOS += HUC6261
--------------------------------------------------

if (VIDEOS["HUC6261"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/huc6261.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/huc6270.h,VIDEOS += HUC6270
--------------------------------------------------

if (VIDEOS["HUC6270"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/huc6270.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/huc6272.h,VIDEOS += HUC6272
--------------------------------------------------

if (VIDEOS["HUC6272"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/huc6272.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/i8244.h,VIDEOS += I8244
--------------------------------------------------

if (VIDEOS["I8244"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/i8244.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/i8275.h,VIDEOS += I8275
--------------------------------------------------

if (VIDEOS["I8275"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/i8275.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/m50458.h,VIDEOS += M50458
--------------------------------------------------

if (VIDEOS["M50458"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/m50458.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/mb90082.h,VIDEOS += MB90082
--------------------------------------------------

if (VIDEOS["MB90082"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/mb90082.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/mb_vcu.h,VIDEOS += MB_VCU
--------------------------------------------------

if (VIDEOS["MB_VCU"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/mb_vcu.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/mc6845.h,VIDEOS += MC6845
--------------------------------------------------

if (VIDEOS["MC6845"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/mc6845.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/mc6847.h,VIDEOS += MC6847
--------------------------------------------------

if (VIDEOS["MC6847"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/mc6847.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/msm6222b.h,VIDEOS += MSM6222B
--------------------------------------------------

if (VIDEOS["MSM6222B"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/msm6222b.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/msm6255.h,VIDEOS += MSM6255
--------------------------------------------------

if (VIDEOS["MSM6255"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/msm6255.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/mos6566.h,VIDEOS += MOS6566
--------------------------------------------------

if (VIDEOS["MOS6566"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/mos6566.*",
	}
end


files {	
	MAME_DIR .. "src/emu/video/cgapal.*",
}

--------------------------------------------------
--
--@src/emu/video/pc_vga.h,VIDEOS += PC_VGA
--------------------------------------------------

if (VIDEOS["PC_VGA"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/pc_vga.*",
		MAME_DIR .. "src/emu/bus/isa/trident.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/polylgcy.h,VIDEOS += POLY
--------------------------------------------------

if (VIDEOS["POLY"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/polylgcy.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/psx.h,VIDEOS += PSX
--------------------------------------------------

if (VIDEOS["PSX"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/psx.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/ramdac.h,VIDEOS += RAMDAC
--------------------------------------------------

if (VIDEOS["RAMDAC"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/ramdac.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/saa5050.h,VIDEOS += SAA5050
--------------------------------------------------

if (VIDEOS["SAA5050"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/saa5050.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/sed1200.h,VIDEOS += SED1200
--------------------------------------------------
if (VIDEOS["SED1200"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/sed1200.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/sed1330.h,VIDEOS += SED1330
--------------------------------------------------
if (VIDEOS["SED1330"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/sed1330.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/sed1520.h,VIDEOS += SED1520
--------------------------------------------------
if (VIDEOS["SED1520"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/sed1520.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/scn2674.h,VIDEOS += SCN2674
--------------------------------------------------
if (VIDEOS["SCN2674"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/scn2674.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/snes_ppu.h,VIDEOS += SNES_PPU
--------------------------------------------------
if (VIDEOS["SNES_PPU"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/snes_ppu.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/stvvdp1.h,VIDEOS += STVVDP
--@src/emu/video/stvvdp2.h,VIDEOS += STVVDP
--------------------------------------------------

if (VIDEOS["STVVDP"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/stvvdp1.*",
		MAME_DIR .. "src/emu/video/stvvdp2.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/t6a04.h,VIDEOS += T6A04
--------------------------------------------------

if (VIDEOS["T6A04"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/t6a04.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/tea1002.h,VIDEOS += TEA1002
--------------------------------------------------

if (VIDEOS["TEA1002"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/tea1002.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/tlc34076.h,VIDEOS += TLC34076
--------------------------------------------------

if (VIDEOS["TLC34076"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/tlc34076.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/tms34061.h,VIDEOS += TMS34061
--------------------------------------------------

if (VIDEOS["TMS34061"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/tms34061.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/tms3556.h,VIDEOS += TMS3556
--------------------------------------------------

if (VIDEOS["TMS3556"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/tms3556.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/tms9927.h,VIDEOS += TMS9927
--------------------------------------------------

if (VIDEOS["TMS9927"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/tms9927.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/tms9928a.h,VIDEOS += TMS9928A
--------------------------------------------------

if (VIDEOS["TMS9928A"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/tms9928a.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/upd3301.h,VIDEOS += UPD3301
--------------------------------------------------

if (VIDEOS["UPD3301"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/upd3301.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/upd7220.h,VIDEOS += UPD7220
--------------------------------------------------

if (VIDEOS["UPD7220"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/upd7220.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/upd7227.h,VIDEOS += UPD7227
--------------------------------------------------

if (VIDEOS["UPD7227"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/upd7227.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/vic4567.h,VIDEOS += VIC4567
--------------------------------------------------

if (VIDEOS["VIC4567"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/vic4567.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/v9938.h,VIDEOS += V9938
--------------------------------------------------

if (VIDEOS["V9938"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/v9938.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/voodoo.h,VIDEOS += VOODOO
--------------------------------------------------

if (VIDEOS["VOODOO"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/voodoo.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/voodoo_pci.h,VIDEOS += VOODOO_PCI
--------------------------------------------------

if (VIDEOS["VOODOO_PCI"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/voodoo_pci.*",
	}
end

--------------------------------------------------
--
--@src/emu/video/crtc_ega.h,VIDEOS += CRTC_EGA
--------------------------------------------------

if (VIDEOS["CRTC_EGA"]~=null) then
	files {
		MAME_DIR .. "src/emu/video/crtc_ega.*",
	}
end
