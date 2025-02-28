-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   video.lua
--
--   Rules for building video cores
--
---------------------------------------------------------------------------

files {
	MAME_DIR .. "src/devices/video/cgapal.cpp",
	MAME_DIR .. "src/devices/video/cgapal.h",
	MAME_DIR .. "src/devices/video/poly.h",
	MAME_DIR .. "src/devices/video/sprite.cpp",
	MAME_DIR .. "src/devices/video/sprite.h",
	MAME_DIR .. "src/devices/video/vector.cpp",
	MAME_DIR .. "src/devices/video/vector.h",
}

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
--@src/devices/video/am8052.h,VIDEOS["AM8052"] = true
--------------------------------------------------

if (VIDEOS["AM8052"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/am8052.cpp",
		MAME_DIR .. "src/devices/video/am8052.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/atirage.h,VIDEOS["ATIRAGE"] = true
--------------------------------------------------

if (VIDEOS["ATIRAGE"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/atirage.cpp",
		MAME_DIR .. "src/devices/video/atirage.h",
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
--@src/devices/video/cesblit.h,VIDEOS["CESBLIT"] = true
--------------------------------------------------

if (VIDEOS["CESBLIT"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/cesblit.cpp",
		MAME_DIR .. "src/devices/video/cesblit.h",
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
--@src/devices/video/crt9028.h,VIDEOS["CRT9028"] = true
--------------------------------------------------

if (VIDEOS["CRT9028"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/crt9028.cpp",
		MAME_DIR .. "src/devices/video/crt9028.h",
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
--@src/devices/video/dp8350.h,VIDEOS["DP8350"] = true
--------------------------------------------------

if (VIDEOS["DP8350"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/dp8350.cpp",
		MAME_DIR .. "src/devices/video/dp8350.h",
	}
end

---------------------------------------------------
--
--@src/devices/video/ds8874.h,VIDEOS["DS8874"] = true
---------------------------------------------------

if (VIDEOS["DS8874"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ds8874.cpp",
		MAME_DIR .. "src/devices/video/ds8874.h",
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
--@src/devices/video/ef9364.h,VIDEOS["EF9364"] = true
--------------------------------------------------

if (VIDEOS["EF9364"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ef9364.cpp",
		MAME_DIR .. "src/devices/video/ef9364.h",
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
--@src/devices/video/nt7534.h,VIDEOS["NT7534"] = true
--------------------------------------------------

if (VIDEOS["NT7534"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/nt7534.cpp",
		MAME_DIR .. "src/devices/video/nt7534.h",
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
--@src/devices/video/hd61202.h,VIDEOS["HD61202"] = true
--------------------------------------------------

if (VIDEOS["HD61202"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hd61202.cpp",
		MAME_DIR .. "src/devices/video/hd61202.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/hd61603.h,VIDEOS["HD61603"] = true
--------------------------------------------------

if (VIDEOS["HD61603"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hd61603.cpp",
		MAME_DIR .. "src/devices/video/hd61603.h",
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
--@src/devices/video/hlcd0438.h,VIDEOS["HLCD0438"] = true
--------------------------------------------------

if (VIDEOS["HLCD0438"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hlcd0438.cpp",
		MAME_DIR .. "src/devices/video/hlcd0438.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/hlcd0488.h,VIDEOS["HLCD0488"] = true
--------------------------------------------------

if (VIDEOS["HLCD0488"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hlcd0488.cpp",
		MAME_DIR .. "src/devices/video/hlcd0488.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/hlcd0515.h,VIDEOS["HLCD0515"] = true
--------------------------------------------------

if (VIDEOS["HLCD0515"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hlcd0515.cpp",
		MAME_DIR .. "src/devices/video/hlcd0515.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/hlcd0538.h,VIDEOS["HLCD0538"] = true
--------------------------------------------------

if (VIDEOS["HLCD0538"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hlcd0538.cpp",
		MAME_DIR .. "src/devices/video/hlcd0538.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/hp1ll3.h,VIDEOS["HP1LL3"] = true
--------------------------------------------------

if (VIDEOS["HP1LL3"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/hp1ll3.cpp",
		MAME_DIR .. "src/devices/video/hp1ll3.h",
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
--@src/devices/video/huc6271.h,VIDEOS["HUC6271"] = true
--------------------------------------------------

if (VIDEOS["HUC6271"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/huc6271.cpp",
		MAME_DIR .. "src/devices/video/huc6271.h",
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
--@src/devices/video/ims_cvc.h,VIDEOS["IMS_CVC"] = true
--------------------------------------------------

if (VIDEOS["IMS_CVC"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ims_cvc.cpp",
		MAME_DIR .. "src/devices/video/ims_cvc.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/k051316.h,VIDEOS["K051316"] = true
--------------------------------------------------

if (VIDEOS["K051316"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/k051316.cpp",
		MAME_DIR .. "src/devices/video/k051316.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/k053936.h,VIDEOS["K053936"] = true
--------------------------------------------------

if (VIDEOS["K053936"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/k053936.cpp",
		MAME_DIR .. "src/devices/video/k053936.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/lc7580.h,VIDEOS["LC7580"] = true
--------------------------------------------------

if (VIDEOS["LC7580"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/lc7580.cpp",
		MAME_DIR .. "src/devices/video/lc7580.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/lc7985.h,VIDEOS["LC7985"] = true
--------------------------------------------------

if (VIDEOS["LC7985"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/lc7985.cpp",
		MAME_DIR .. "src/devices/video/lc7985.h",
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

---------------------------------------------------
--
--@src/devices/video/mb86292.h,VIDEOS["MB86292"] = true
---------------------------------------------------

if (VIDEOS["MB86292"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/mb86292.cpp",
		MAME_DIR .. "src/devices/video/mb86292.h",
	}
end

---------------------------------------------------
--
--@src/devices/video/mb88303.h,VIDEOS["MB88303"] = true
---------------------------------------------------

if (VIDEOS["MB88303"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/mb88303.cpp",
		MAME_DIR .. "src/devices/video/mb88303.h",
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
--@src/devices/video/mc68328lcd.h,VIDEOS["MC68328LCD"] = true
--------------------------------------------------

if (VIDEOS["MC68328LCD"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/mc68328lcd.cpp",
		MAME_DIR .. "src/devices/video/mc68328lcd.h",
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
--@src/devices/video/md4330b.h,VIDEOS["MD4330B"] = true
--------------------------------------------------

if (VIDEOS["MD4330B"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/md4330b.cpp",
		MAME_DIR .. "src/devices/video/md4330b.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/mm5445.h,VIDEOS["MM5445"] = true
--------------------------------------------------

if (VIDEOS["MM5445"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/mm5445.cpp",
		MAME_DIR .. "src/devices/video/mm5445.h",
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

--------------------------------------------------
--
--@src/devices/video/mos8563.h,VIDEOS["MOS8563"] = true
--------------------------------------------------

if (VIDEOS["MOS8563"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/mos8563.cpp",
		MAME_DIR .. "src/devices/video/mos8563.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/mn1252.h,VIDEOS["MN1252"] = true
--------------------------------------------------

if (VIDEOS["MN1252"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/mn1252.cpp",
		MAME_DIR .. "src/devices/video/mn1252.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga.h,VIDEOS["PC_VGA"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga.cpp",
		MAME_DIR .. "src/devices/video/pc_vga.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga_alliance.h,VIDEOS["PC_VGA_ALLIANCE"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA_ALLIANCE"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_alliance.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_alliance.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga_ati.h,VIDEOS["PC_VGA_ATI"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA_ATI"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_ati.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_ati.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/ibm8514a.h,VIDEOS["IBM8514A"] = true
--------------------------------------------------

if (VIDEOS["IBM8514A"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ibm8514a.cpp",
		MAME_DIR .. "src/devices/video/ibm8514a.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/ati_mach8.h,VIDEOS["ATI_MACH8"] = true
--------------------------------------------------

if (VIDEOS["ATI_MACH8"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ati_mach8.cpp",
		MAME_DIR .. "src/devices/video/ati_mach8.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/ati_mach8.h,VIDEOS["ATI_MACH32"] = true
--------------------------------------------------

if (VIDEOS["ATI_MACH32"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ati_mach32.cpp",
		MAME_DIR .. "src/devices/video/ati_mach32.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga_cirrus.h,VIDEOS["PC_VGA_CIRRUS"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA_CIRRUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_cirrus.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_cirrus.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga_matrox.h,VIDEOS["PC_VGA_MATROX"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA_MATROX"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_matrox.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_matrox.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga_mediagx.h,VIDEOS["PC_VGA_MEDIAGX"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA_MEDIAGX"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_mediagx.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_mediagx.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga_nvidia.h,VIDEOS["PC_VGA_NVIDIA"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA_NVIDIA"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_nvidia.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_nvidia.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga_oak.h,VIDEOS["PC_VGA_OAK"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA_OAK"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_oak.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_oak.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga_paradise.h,VIDEOS["PC_VGA_PARADISE"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA_PARADISE"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_paradise.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_paradise.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/wd90c26.h,VIDEOS["WD90C26"] = true
--------------------------------------------------

if (VIDEOS["WD90C26"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/wd90c26.cpp",
		MAME_DIR .. "src/devices/video/wd90c26.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga_s3.h,VIDEOS["PC_VGA_S3"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA_S3"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_s3.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_s3.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/s3virge.h,VIDEOS["S3VIRGE"] = true
--------------------------------------------------

if (VIDEOS["S3VIRGE"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/s3virge.cpp",
		MAME_DIR .. "src/devices/video/s3virge.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga_sis.h,VIDEOS["PC_VGA_SIS"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA_SIS"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_sis.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_sis.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga_trident.h,VIDEOS["PC_VGA_TRIDENT"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA_TRIDENT"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_trident.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_trident.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_vga_tseng.h,VIDEOS["PC_VGA_TSENG"] = true
--------------------------------------------------

if (VIDEOS["PC_VGA_TSENG"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_tseng.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_tseng.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pc_xga.h,VIDEOS["PC_XGA"] = true
--------------------------------------------------

if (VIDEOS["PC_XGA"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pc_xga.cpp",
		MAME_DIR .. "src/devices/video/pc_xga.h",
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
--@src/devices/video/pcf2100.h,VIDEOS["PCF2100"] = true
--------------------------------------------------

if (VIDEOS["PCF2100"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pcf2100.cpp",
		MAME_DIR .. "src/devices/video/pcf2100.h",
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
--@src/devices/video/saa5240.h,VIDEOS["SAA5240"] = true
--------------------------------------------------

if (VIDEOS["SAA5240"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/saa5240.cpp",
		MAME_DIR .. "src/devices/video/saa5240.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/saa7110.h,VIDEOS["SAA7110"] = true
--------------------------------------------------

if (VIDEOS["SAA7110"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/saa7110.cpp",
		MAME_DIR .. "src/devices/video/saa7110.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/sn74s262.h,VIDEOS["SN74S262"] = true
--------------------------------------------------

if (VIDEOS["SN74S262"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/sn74s262.cpp",
		MAME_DIR .. "src/devices/video/sn74s262.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/pwm.h,VIDEOS["PWM_DISPLAY"] = true
--------------------------------------------------
if (VIDEOS["PWM_DISPLAY"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/pwm.cpp",
		MAME_DIR .. "src/devices/video/pwm.h",
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
--@src/devices/video/sed1356.h,VIDEOS["SED1356"] = true
--------------------------------------------------
if (VIDEOS["SED1356"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/sed1356.cpp",
		MAME_DIR .. "src/devices/video/sed1356.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/sed1375.h,VIDEOS["SED1375"] = true
--------------------------------------------------
if (VIDEOS["SED1375"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/sed1375.cpp",
		MAME_DIR .. "src/devices/video/sed1375.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/sed1500.h,VIDEOS["SED1500"] = true
--------------------------------------------------
if (VIDEOS["SED1500"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/sed1500.cpp",
		MAME_DIR .. "src/devices/video/sed1500.h",
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
--@src/devices/video/sda5708.h,VIDEOS["SDA5708"] = true
--------------------------------------------------
if (VIDEOS["SDA5708"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/sda5708.cpp",
		MAME_DIR .. "src/devices/video/sda5708.h",
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
--@src/devices/video/t6963c.h,VIDEOS["T6963C"] = true
--------------------------------------------------

if (VIDEOS["T6963C"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/t6963c.cpp",
		MAME_DIR .. "src/devices/video/t6963c.h",
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
--@src/devices/video/tmap038.h,VIDEOS["TMAP038"] = true
--------------------------------------------------

if (VIDEOS["TMAP038"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/tmap038.cpp",
		MAME_DIR .. "src/devices/video/tmap038.h",
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
--@src/devices/video/zeus2.h,VIDEOS["ZEUS2"] = true
--------------------------------------------------

if (VIDEOS["ZEUS2"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/zeus2.cpp",
		MAME_DIR .. "src/devices/video/zeus2.h",
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
		MAME_DIR .. "src/devices/video/voodoo_2.cpp",
		MAME_DIR .. "src/devices/video/voodoo_2.h",
		MAME_DIR .. "src/devices/video/voodoo_banshee.cpp",
		MAME_DIR .. "src/devices/video/voodoo_banshee.h",
		MAME_DIR .. "src/devices/video/voodoo_regs.h",
		MAME_DIR .. "src/devices/video/voodoo_render.cpp",
		MAME_DIR .. "src/devices/video/voodoo_render.h",
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

--------------------------------------------------
--
--@src/devices/video/jangou_blitter.h,VIDEOS["JANGOU_BLITTER"] = true
--------------------------------------------------

if (VIDEOS["JANGOU_BLITTER"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/jangou_blitter.cpp",
		MAME_DIR .. "src/devices/video/jangou_blitter.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/gb_lcd.h,VIDEOS["GB_LCD"] = true
--------------------------------------------------

if (VIDEOS["GB_LCD"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/gb_lcd.cpp",
		MAME_DIR .. "src/devices/video/gb_lcd.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/gba_lcd.h,VIDEOS["GBA_LCD"] = true
--------------------------------------------------

if (VIDEOS["GBA_LCD"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/gba_lcd.cpp",
		MAME_DIR .. "src/devices/video/gba_lcd.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/ef9369.h,VIDEOS["EF9369"] = true
--------------------------------------------------

if (VIDEOS["EF9369"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ef9369.cpp",
		MAME_DIR .. "src/devices/video/ef9369.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/ppu2c0x.h,VIDEOS["PPU2C0X"] = true
--------------------------------------------------

if (VIDEOS["PPU2C0X"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ppu2c0x.cpp",
		MAME_DIR .. "src/devices/video/ppu2c0x.h",
		MAME_DIR .. "src/devices/video/ppu2c0x_vt.cpp",
		MAME_DIR .. "src/devices/video/ppu2c0x_vt.h",
		MAME_DIR .. "src/devices/video/ppu2c0x_sh6578.cpp",
		MAME_DIR .. "src/devices/video/ppu2c0x_sh6578.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/bt431.h,VIDEOS["BT431"] = true
--------------------------------------------------

if (VIDEOS["BT431"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/bt431.cpp",
		MAME_DIR .. "src/devices/video/bt431.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/bt459.h,VIDEOS["BT459"] = true
--------------------------------------------------

if (VIDEOS["BT459"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/bt459.cpp",
		MAME_DIR .. "src/devices/video/bt459.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/bt45x.h,VIDEOS["BT45X"] = true
--------------------------------------------------

if (VIDEOS["BT45X"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/bt45x.cpp",
		MAME_DIR .. "src/devices/video/bt45x.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/bt47x.h,VIDEOS["BT47X"] = true
--------------------------------------------------

if (VIDEOS["BT47X"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/bt47x.cpp",
		MAME_DIR .. "src/devices/video/bt47x.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/bt48x.h,VIDEOS["BT48X"] = true
--------------------------------------------------

if (VIDEOS["BT48X"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/bt48x.cpp",
		MAME_DIR .. "src/devices/video/bt48x.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/imagetek_i4100.h,VIDEOS["I4100"] = true
--------------------------------------------------

if (VIDEOS["I4100"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/imagetek_i4100.cpp",
		MAME_DIR .. "src/devices/video/imagetek_i4100.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/dp8510.h,VIDEOS["DP8510"] = true
--------------------------------------------------

if (VIDEOS["DP8510"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/dp8510.cpp",
		MAME_DIR .. "src/devices/video/dp8510.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/topcat.h,VIDEOS["TOPCAT"] = true
--------------------------------------------------
if (VIDEOS["TOPCAT"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/topcat.cpp",
		MAME_DIR .. "src/devices/video/topcat.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/catseye.h,VIDEOS["CATSEYE"] = true
--------------------------------------------------
if (VIDEOS["CATSEYE"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/catseye.cpp",
		MAME_DIR .. "src/devices/video/catseye.h",
	}
end


--------------------------------------------------
--
--@src/devices/video/nereid.h,VIDEOS["NEREID"] = true
--------------------------------------------------
if (VIDEOS["NEREID"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/nereid.cpp",
		MAME_DIR .. "src/devices/video/nereid.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/ps2gif.h,VIDEOS["PS2GIF"] = true
--------------------------------------------------
if (VIDEOS["PS2GIF"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ps2gif.cpp",
		MAME_DIR .. "src/devices/video/ps2gif.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/ps2gs.h,VIDEOS["PS2GS"] = true
--------------------------------------------------
if (VIDEOS["PS2GS"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/ps2gs.cpp",
		MAME_DIR .. "src/devices/video/ps2gs.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/vrender0.h,VIDEOS["VRENDER0"] = true
--------------------------------------------------

if (VIDEOS["VRENDER0"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/vrender0.cpp",
		MAME_DIR .. "src/devices/video/vrender0.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/avgdvg.h,VIDEOS["AVGDVG"] = true
--------------------------------------------------

if (VIDEOS["AVGDVG"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/avgdvg.cpp",
		MAME_DIR .. "src/devices/video/avgdvg.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/x1_001.h,VIDEOS["X1_001"] = true
--------------------------------------------------

if (VIDEOS["X1_001"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/x1_001.cpp",
		MAME_DIR .. "src/devices/video/x1_001.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/zr36060.h,VIDEOS["ZR36060"] = true
--------------------------------------------------

if (VIDEOS["ZR36060"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/zr36060.cpp",
		MAME_DIR .. "src/devices/video/zr36060.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/zr36110.h,VIDEOS["ZR36110"] = true
--------------------------------------------------

if (VIDEOS["ZR36110"]~=null) then
	files {
		MAME_DIR .. "src/devices/video/zr36110.cpp",
		MAME_DIR .. "src/devices/video/zr36110.h",
	}
end
