# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   video.cmake
##
##   Rules for building video cores
##
##########################################################################

set(VIDEO_SRCS
	${MAME_DIR}/src/devices/video/poly.h
	${MAME_DIR}/src/devices/video/sprite.cpp
	${MAME_DIR}/src/devices/video/sprite.h
	${MAME_DIR}/src/devices/video/vector.cpp
	${MAME_DIR}/src/devices/video/vector.h
)

##################################################
##
##@src/devices/video/315_5124.h,list(APPEND VIDEOS SEGA315_5124)
##################################################

if("SEGA315_5124" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/315_5124.cpp
		${MAME_DIR}/src/devices/video/315_5124.h
	)
endif()

##################################################
##
##@src/devices/video/315_5313.h,list(APPEND VIDEOS SEGA315_5313)
##################################################

if("SEGA315_5313" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/315_5313.cpp
		${MAME_DIR}/src/devices/video/315_5313.h
	)
endif()

##################################################
##
##@src/devices/video/am8052.h,list(APPEND VIDEOS AM8052)
##################################################

if("AM8052" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/am8052.cpp
		${MAME_DIR}/src/devices/video/am8052.h
	)
endif()

##################################################
##
##@src/devices/video/bufsprite.h,list(APPEND VIDEOS BUFSPRITE)
##################################################

if("BUFSPRITE" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/bufsprite.cpp
		${MAME_DIR}/src/devices/video/bufsprite.h
	)
endif()

##################################################
##
##@src/devices/video/cdp1861.h,list(APPEND VIDEOS CDP1861)
##################################################

if("CDP1861" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/cdp1861.cpp
		${MAME_DIR}/src/devices/video/cdp1861.h
	)
endif()

##################################################
##
##@src/devices/video/cdp1862.h,list(APPEND VIDEOS CDP1862)
##################################################

if("CDP1862" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/cdp1862.cpp
		${MAME_DIR}/src/devices/video/cdp1862.h
	)
endif()

##################################################
##
##@src/devices/video/cesblit.h,list(APPEND VIDEOS CESBLIT)
##################################################

if("CESBLIT" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/cesblit.cpp
		${MAME_DIR}/src/devices/video/cesblit.h
	)
endif()

##################################################
##
##@src/devices/video/crt9007.h,list(APPEND VIDEOS CRT9007)
##################################################

if("CRT9007" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/crt9007.cpp
		${MAME_DIR}/src/devices/video/crt9007.h
	)
endif()

##################################################
##
##@src/devices/video/crt9021.h,list(APPEND VIDEOS CRT9021)
##################################################

if("CRT9021" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/crt9021.cpp
		${MAME_DIR}/src/devices/video/crt9021.h
	)
endif()

##################################################
##
##@src/devices/video/crt9028.h,list(APPEND VIDEOS CRT9028)
##################################################

if("CRT9028" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/crt9028.cpp
		${MAME_DIR}/src/devices/video/crt9028.h
	)
endif()

##################################################
##
##@src/devices/video/crt9212.h,list(APPEND VIDEOS CRT9212)
##################################################

if("CRT9212" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/crt9212.cpp
		${MAME_DIR}/src/devices/video/crt9212.h
	)
endif()

##################################################
##
##@src/devices/video/dl1416.h,list(APPEND VIDEOS DL1416)
##################################################

if("DL1416" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/dl1416.cpp
		${MAME_DIR}/src/devices/video/dl1416.h
	)
endif()

##################################################
##
##@src/devices/video/dm9368.h,list(APPEND VIDEOS DM9368)
##################################################

if("DM9368" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/dm9368.cpp
		${MAME_DIR}/src/devices/video/dm9368.h
	)
endif()

##################################################
##
##@src/devices/video/dp8350.h,list(APPEND VIDEOS DP8350)
##################################################

if("DP8350" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/dp8350.cpp
		${MAME_DIR}/src/devices/video/dp8350.h
	)
endif()

##################################################
##
##@src/devices/video/ef9340_1.h,list(APPEND VIDEOS EF9340_1)
##################################################

if("EF9340_1" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/ef9340_1.cpp
		${MAME_DIR}/src/devices/video/ef9340_1.h
	)
endif()

##################################################
##
##@src/devices/video/ef9345.h,list(APPEND VIDEOS EF9345)
##################################################

if("EF9345" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/ef9345.cpp
		${MAME_DIR}/src/devices/video/ef9345.h
	)
endif()

##################################################
##
##@src/devices/video/ef9364.h,list(APPEND VIDEOS EF9364)
##################################################

if("EF9364" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/ef9364.cpp
		${MAME_DIR}/src/devices/video/ef9364.h
	)
endif()

##################################################
##
##@src/devices/video/ef9365.h,list(APPEND VIDEOS EF9365)
##################################################

if("EF9365" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/ef9365.cpp
		${MAME_DIR}/src/devices/video/ef9365.h
	)
endif()

##################################################
##@src/devices/video/epic12.h,list(APPEND VIDEOS EPIC12)
##################################################

if("EPIC12" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/epic12.cpp
		${MAME_DIR}/src/devices/video/epic12.h
		${MAME_DIR}/src/devices/video/epic12_blit0.cpp
		${MAME_DIR}/src/devices/video/epic12_blit1.cpp
		${MAME_DIR}/src/devices/video/epic12_blit2.cpp
		${MAME_DIR}/src/devices/video/epic12_blit3.cpp
		${MAME_DIR}/src/devices/video/epic12_blit4.cpp
		${MAME_DIR}/src/devices/video/epic12_blit5.cpp
		${MAME_DIR}/src/devices/video/epic12_blit6.cpp
		${MAME_DIR}/src/devices/video/epic12_blit7.cpp
		${MAME_DIR}/src/devices/video/epic12_blit8.cpp
		${MAME_DIR}/src/devices/video/epic12in.hxx
		${MAME_DIR}/src/devices/video/epic12pixel.hxx
	)
endif()

##################################################
##
##@src/devices/video/fixfreq.h,list(APPEND VIDEOS FIXFREQ)
##################################################

if("FIXFREQ" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/fixfreq.cpp
		${MAME_DIR}/src/devices/video/fixfreq.h
	)
endif()

##################################################
##
##@src/devices/video/gf4500.h,list(APPEND VIDEOS GF4500)
##################################################

if("GF4500" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/gf4500.cpp
		${MAME_DIR}/src/devices/video/gf4500.h
	)
endif()

##################################################
##
##@src/devices/video/gf7600gs.h,list(APPEND VIDEOS GF7600GS)
##################################################

if("GF7600GS" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/gf7600gs.cpp
		${MAME_DIR}/src/devices/video/gf7600gs.h
	)
endif()

##################################################
##
##@src/devices/video/mga2064w.h,list(APPEND VIDEOS MGA2064W)
##################################################

if("MGA2064W" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/mga2064w.cpp
		${MAME_DIR}/src/devices/video/mga2064w.h
	)
endif()

##################################################
##
##@src/devices/video/nt7534.h,list(APPEND VIDEOS NT7534)
##################################################

if("NT7534" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/nt7534.cpp
		${MAME_DIR}/src/devices/video/nt7534.h
	)
endif()

##################################################
##
##@src/devices/video/hd44102.h,list(APPEND VIDEOS HD44102)
##################################################

if("HD44102" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hd44102.cpp
		${MAME_DIR}/src/devices/video/hd44102.h
	)
endif()

##################################################
##
##@src/devices/video/hd44352.h,list(APPEND VIDEOS HD44352)
##################################################

if("HD44352" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hd44352.cpp
		${MAME_DIR}/src/devices/video/hd44352.h
	)
endif()

##################################################
##
##@src/devices/video/hd44780.h,list(APPEND VIDEOS HD44780)
##################################################

if("HD44780" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hd44780.cpp
		${MAME_DIR}/src/devices/video/hd44780.h
	)
endif()

##################################################
##
##@src/devices/video/hd61202.h,list(APPEND VIDEOS HD61202)
##################################################

if("HD61202" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hd61202.cpp
		${MAME_DIR}/src/devices/video/hd61202.h
	)
endif()

##################################################
##
##@src/devices/video/hd61603.h,list(APPEND VIDEOS HD61603)
##################################################

if("HD61603" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hd61603.cpp
		${MAME_DIR}/src/devices/video/hd61603.h
	)
endif()

##################################################
##
##@src/devices/video/hd61830.h,list(APPEND VIDEOS HD61830)
##################################################

if("HD61830" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hd61830.cpp
		${MAME_DIR}/src/devices/video/hd61830.h
	)
endif()

##################################################
##
##@src/devices/video/hd63484.h,list(APPEND VIDEOS HD63484)
##################################################

if("HD63484" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hd63484.cpp
		${MAME_DIR}/src/devices/video/hd63484.h
	)
endif()

##################################################
##
##@src/devices/video/hd66421.h,list(APPEND VIDEOS HD66421)
##################################################

if("HD66421" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hd66421.cpp
		${MAME_DIR}/src/devices/video/hd66421.h
	)
endif()

##################################################
##
##@src/devices/video/hlcd0438.h,list(APPEND VIDEOS HLCD0438)
##################################################

if("HLCD0438" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hlcd0438.cpp
		${MAME_DIR}/src/devices/video/hlcd0438.h
	)
endif()

##################################################
##
##@src/devices/video/hlcd0488.h,list(APPEND VIDEOS HLCD0488)
##################################################

if("HLCD0488" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hlcd0488.cpp
		${MAME_DIR}/src/devices/video/hlcd0488.h
	)
endif()

##################################################
##
##@src/devices/video/hlcd0515.h,list(APPEND VIDEOS HLCD0515)
##################################################

if("HLCD0515" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hlcd0515.cpp
		${MAME_DIR}/src/devices/video/hlcd0515.h
	)
endif()

##################################################
##
##@src/devices/video/hlcd0538.h,list(APPEND VIDEOS HLCD0538)
##################################################

if("HLCD0538" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hlcd0538.cpp
		${MAME_DIR}/src/devices/video/hlcd0538.h
	)
endif()

##################################################
##
##@src/devices/video/hp1ll3.h,list(APPEND VIDEOS HP1LL3)
##################################################

if("HP1LL3" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/hp1ll3.cpp
		${MAME_DIR}/src/devices/video/hp1ll3.h
	)
endif()

##################################################
##
##@src/devices/video/huc6202.h,list(APPEND VIDEOS HUC6202)
##################################################

if("HUC6202" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/huc6202.cpp
		${MAME_DIR}/src/devices/video/huc6202.h
	)
endif()

##################################################
##
##@src/devices/video/huc6260.h,list(APPEND VIDEOS HUC6260)
##################################################

if("HUC6260" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/huc6260.cpp
		${MAME_DIR}/src/devices/video/huc6260.h
	)
endif()

##################################################
##
##@src/devices/video/huc6261.h,list(APPEND VIDEOS HUC6261)
##################################################

if("HUC6261" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/huc6261.cpp
		${MAME_DIR}/src/devices/video/huc6261.h
	)
endif()

##################################################
##
##@src/devices/video/huc6270.h,list(APPEND VIDEOS HUC6270)
##################################################

if("HUC6270" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/huc6270.cpp
		${MAME_DIR}/src/devices/video/huc6270.h
	)
endif()

##################################################
##
##@src/devices/video/huc6271.h,list(APPEND VIDEOS HUC6271)
##################################################

if("HUC6271" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/huc6271.cpp
		${MAME_DIR}/src/devices/video/huc6271.h
	)
endif()

##################################################
##
##@src/devices/video/huc6272.h,list(APPEND VIDEOS HUC6272)
##################################################

if("HUC6272" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/huc6272.cpp
		${MAME_DIR}/src/devices/video/huc6272.h
	)
endif()

##################################################
##
##@src/devices/video/i8244.h,list(APPEND VIDEOS I8244)
##################################################

if("I8244" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/i8244.cpp
		${MAME_DIR}/src/devices/video/i8244.h
	)
endif()

##################################################
##
##@src/devices/video/i82730.h,list(APPEND VIDEOS I82730)
##################################################

if("I82730" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/i82730.cpp
		${MAME_DIR}/src/devices/video/i82730.h
	)
endif()

##################################################
##
##@src/devices/video/i8275.h,list(APPEND VIDEOS I8275)
##################################################

if("I8275" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/i8275.cpp
		${MAME_DIR}/src/devices/video/i8275.h
	)
endif()

##################################################
##
##@src/devices/video/ims_cvc.h,list(APPEND VIDEOS IMS_CVC)
##################################################

if("IMS_CVC" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/ims_cvc.cpp
		${MAME_DIR}/src/devices/video/ims_cvc.h
	)
endif()

##################################################
##
##@src/devices/video/lc7582.h,list(APPEND VIDEOS LC7582)
##################################################

if("LC7582" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/lc7582.cpp
		${MAME_DIR}/src/devices/video/lc7582.h
	)
endif()

##################################################
##
##@src/devices/video/lc7985.h,list(APPEND VIDEOS LC7985)
##################################################

if("LC7985" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/lc7985.cpp
		${MAME_DIR}/src/devices/video/lc7985.h
	)
endif()

##################################################
##
##@src/devices/video/m50458.h,list(APPEND VIDEOS M50458)
##################################################

if("M50458" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/m50458.cpp
		${MAME_DIR}/src/devices/video/m50458.h
	)
endif()

###################################################
##
##@src/devices/video/mb88303.h,list(APPEND VIDEOS MB88303)
###################################################

if("MB88303" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/mb88303.cpp
		${MAME_DIR}/src/devices/video/mb88303.h
	)
endif()

##################################################
##
##@src/devices/video/mb90082.h,list(APPEND VIDEOS MB90082)
##################################################

if("MB90082" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/mb90082.cpp
		${MAME_DIR}/src/devices/video/mb90082.h
	)
endif()

##################################################
##
##@src/devices/video/mb_vcu.h,list(APPEND VIDEOS MB_VCU)
##################################################

if("MB_VCU" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/mb_vcu.cpp
		${MAME_DIR}/src/devices/video/mb_vcu.h
	)
endif()

##################################################
##
##@src/devices/video/mc6845.h,list(APPEND VIDEOS MC6845)
##################################################

if("MC6845" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/mc6845.cpp
		${MAME_DIR}/src/devices/video/mc6845.h
	)
endif()

##################################################
##
##@src/devices/video/mc6847.h,list(APPEND VIDEOS MC6847)
##################################################

if("MC6847" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/mc6847.cpp
		${MAME_DIR}/src/devices/video/mc6847.h
	)
endif()

##################################################
##
##@src/devices/video/md4330b.h,list(APPEND VIDEOS MD4330B)
##################################################

if("MD4330B" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/md4330b.cpp
		${MAME_DIR}/src/devices/video/md4330b.h
	)
endif()

##################################################
##
##@src/devices/video/mm5445.h,list(APPEND VIDEOS MM5445)
##################################################

if("MM5445" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/mm5445.cpp
		${MAME_DIR}/src/devices/video/mm5445.h
	)
endif()

##################################################
##
##@src/devices/video/msm6222b.h,list(APPEND VIDEOS MSM6222B)
##################################################

if("MSM6222B" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/msm6222b.cpp
		${MAME_DIR}/src/devices/video/msm6222b.h
	)
endif()

##################################################
##
##@src/devices/video/msm6255.h,list(APPEND VIDEOS MSM6255)
##################################################

if("MSM6255" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/msm6255.cpp
		${MAME_DIR}/src/devices/video/msm6255.h
	)
endif()

##################################################
##
##@src/devices/video/mos6566.h,list(APPEND VIDEOS MOS6566)
##################################################

if("MOS6566" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/mos6566.cpp
		${MAME_DIR}/src/devices/video/mos6566.h
	)
endif()


list(APPEND VIDEO_SRCS
	${MAME_DIR}/src/devices/video/cgapal.cpp
	${MAME_DIR}/src/devices/video/cgapal.h
)

##################################################
##
##@src/devices/video/pc_vga.h,list(APPEND VIDEOS PC_VGA)
##################################################

if("PC_VGA" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/pc_vga.cpp
		${MAME_DIR}/src/devices/video/pc_vga.h
		${MAME_DIR}/src/devices/bus/isa/trident.cpp
		${MAME_DIR}/src/devices/bus/isa/trident.h
		${MAME_DIR}/src/devices/video/clgd542x.cpp
		${MAME_DIR}/src/devices/video/clgd542x.h
	)
endif()

##################################################
##
##@src/devices/video/virge_pci.h,list(APPEND VIDEOS VIRGE_PCI)
##################################################

if("VIRGE_PCI" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/virge_pci.cpp
		${MAME_DIR}/src/devices/video/virge_pci.h
	)
endif()

##################################################
##
##@src/devices/video/pcd8544.h,list(APPEND VIDEOS PCD8544)
##################################################

if("PCD8544" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/pcd8544.cpp
		${MAME_DIR}/src/devices/video/pcd8544.h
	)
endif()

##################################################
##
##@src/devices/video/pcf2100.h,list(APPEND VIDEOS PCF2100)
##################################################

if("PCF2100" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/pcf2100.cpp
		${MAME_DIR}/src/devices/video/pcf2100.h
	)
endif()

##################################################
##
##@src/devices/video/psx.h,list(APPEND VIDEOS PSX)
##################################################

if("PSX" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/psx.cpp
		${MAME_DIR}/src/devices/video/psx.h
	)
endif()

##################################################
##
##@src/devices/video/ramdac.h,list(APPEND VIDEOS RAMDAC)
##################################################

if("RAMDAC" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/ramdac.cpp
		${MAME_DIR}/src/devices/video/ramdac.h
	)
endif()

##################################################
##
##@src/devices/video/saa5050.h,list(APPEND VIDEOS SAA5050)
##################################################

if("SAA5050" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/saa5050.cpp
		${MAME_DIR}/src/devices/video/saa5050.h
	)
endif()

##################################################
##
##@src/devices/video/saa5240.h,list(APPEND VIDEOS SAA5240)
##################################################

if("SAA5240" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/saa5240.cpp
		${MAME_DIR}/src/devices/video/saa5240.h
	)
endif()

##################################################
##
##@src/devices/video/pwm.h,list(APPEND VIDEOS PWM_DISPLAY)
##################################################
if("PWM_DISPLAY" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/pwm.cpp
		${MAME_DIR}/src/devices/video/pwm.h
	)
endif()

##################################################
##
##@src/devices/video/sed1200.h,list(APPEND VIDEOS SED1200)
##################################################
if("SED1200" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/sed1200.cpp
		${MAME_DIR}/src/devices/video/sed1200.h
	)
endif()

##################################################
##
##@src/devices/video/sed1330.h,list(APPEND VIDEOS SED1330)
##################################################
if("SED1330" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/sed1330.cpp
		${MAME_DIR}/src/devices/video/sed1330.h
	)
endif()

##################################################
##
##@src/devices/video/sed1356.h,list(APPEND VIDEOS SED1356)
##################################################
if("SED1356" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/sed1356.cpp
		${MAME_DIR}/src/devices/video/sed1356.h
	)
endif()

##################################################
##
##@src/devices/video/sed1500.h,list(APPEND VIDEOS SED1500)
##################################################
if("SED1500" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/sed1500.cpp
		${MAME_DIR}/src/devices/video/sed1500.h
	)
endif()

##################################################
##
##@src/devices/video/sed1520.h,list(APPEND VIDEOS SED1520)
##################################################
if("SED1520" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/sed1520.cpp
		${MAME_DIR}/src/devices/video/sed1520.h
	)
endif()

##################################################
##
##@src/devices/video/scn2674.h,list(APPEND VIDEOS SCN2674)
##################################################
if("SCN2674" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/scn2674.cpp
		${MAME_DIR}/src/devices/video/scn2674.h
	)
endif()

##################################################
##
##@src/devices/video/sda5708.h,list(APPEND VIDEOS SDA5708)
##################################################
if("SDA5708" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/sda5708.cpp
		${MAME_DIR}/src/devices/video/sda5708.h
	)
endif()

##################################################
##
##@src/devices/video/snes_ppu.h,list(APPEND VIDEOS SNES_PPU)
##################################################
if("SNES_PPU" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/snes_ppu.cpp
		${MAME_DIR}/src/devices/video/snes_ppu.h
	)
endif()

##################################################
##
##@src/devices/video/stvvdp1.h,list(APPEND VIDEOS STVVDP)
##@src/devices/video/stvvdp2.h,list(APPEND VIDEOS STVVDP)
##################################################

if("STVVDP" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/stvvdp1.cpp
		${MAME_DIR}/src/devices/video/stvvdp1.h
		${MAME_DIR}/src/devices/video/stvvdp2.cpp
		${MAME_DIR}/src/devices/video/stvvdp2.h
	)
endif()

##################################################
##
##@src/devices/video/t6963c.h,list(APPEND VIDEOS T6963C)
##################################################

if("T6963C" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/t6963c.cpp
		${MAME_DIR}/src/devices/video/t6963c.h
	)
endif()

##################################################
##
##@src/devices/video/t6a04.h,list(APPEND VIDEOS T6A04)
##################################################

if("T6A04" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/t6a04.cpp
		${MAME_DIR}/src/devices/video/t6a04.h
	)
endif()

##################################################
##
##@src/devices/video/tea1002.h,list(APPEND VIDEOS TEA1002)
##################################################

if("TEA1002" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/tea1002.cpp
		${MAME_DIR}/src/devices/video/tea1002.h
	)
endif()

##################################################
##
##@src/devices/video/tlc34076.h,list(APPEND VIDEOS TLC34076)
##################################################

if("TLC34076" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/tlc34076.cpp
		${MAME_DIR}/src/devices/video/tlc34076.h
	)
endif()

##################################################
##
##@src/devices/video/tms34061.h,list(APPEND VIDEOS TMS34061)
##################################################

if("TMS34061" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/tms34061.cpp
		${MAME_DIR}/src/devices/video/tms34061.h
	)
endif()

##################################################
##
##@src/devices/video/tms3556.h,list(APPEND VIDEOS TMS3556)
##################################################

if("TMS3556" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/tms3556.cpp
		${MAME_DIR}/src/devices/video/tms3556.h
	)
endif()

##################################################
##
##@src/devices/video/tms9927.h,list(APPEND VIDEOS TMS9927)
##################################################

if("TMS9927" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/tms9927.cpp
		${MAME_DIR}/src/devices/video/tms9927.h
	)
endif()

##################################################
##
##@src/devices/video/tms9928a.h,list(APPEND VIDEOS TMS9928A)
##################################################

if("TMS9928A" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/tms9928a.cpp
		${MAME_DIR}/src/devices/video/tms9928a.h
	)
endif()

##################################################
##
##@src/devices/video/upd3301.h,list(APPEND VIDEOS UPD3301)
##################################################

if("UPD3301" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/upd3301.cpp
		${MAME_DIR}/src/devices/video/upd3301.h
	)
endif()

##################################################
##
##@src/devices/video/upd7220.h,list(APPEND VIDEOS UPD7220)
##################################################

if("UPD7220" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/upd7220.cpp
		${MAME_DIR}/src/devices/video/upd7220.h
	)
endif()

##################################################
##
##@src/devices/video/upd7227.h,list(APPEND VIDEOS UPD7227)
##################################################

if("UPD7227" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/upd7227.cpp
		${MAME_DIR}/src/devices/video/upd7227.h
	)
endif()

##################################################
##
##@src/devices/video/vic4567.h,list(APPEND VIDEOS VIC4567)
##################################################

if("VIC4567" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/vic4567.cpp
		${MAME_DIR}/src/devices/video/vic4567.h
	)
endif()

##################################################
##
##@src/devices/video/v9938.h,list(APPEND VIDEOS V9938)
##################################################

if("V9938" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/v9938.cpp
		${MAME_DIR}/src/devices/video/v9938.h
	)
endif()

##################################################
##
##@src/devices/video/zeus2.h,list(APPEND VIDEOS ZEUS2)
##################################################

if("ZEUS2" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/zeus2.cpp
		${MAME_DIR}/src/devices/video/zeus2.h
	)
endif()

##################################################
##
##@src/devices/video/voodoo.h,list(APPEND VIDEOS VOODOO)
##################################################

if("VOODOO" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/voodoo.cpp
		${MAME_DIR}/src/devices/video/voodoo.h
		${MAME_DIR}/src/devices/video/voodoo_2.cpp
		${MAME_DIR}/src/devices/video/voodoo_2.h
		${MAME_DIR}/src/devices/video/voodoo_banshee.cpp
		${MAME_DIR}/src/devices/video/voodoo_banshee.h
		${MAME_DIR}/src/devices/video/voodoo_regs.h
		${MAME_DIR}/src/devices/video/voodoo_render.cpp
		${MAME_DIR}/src/devices/video/voodoo_render.h
	)
endif()

##################################################
##
##@src/devices/video/voodoo_pci.h,list(APPEND VIDEOS VOODOO_PCI)
##################################################

if("VOODOO_PCI" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/voodoo_pci.cpp
		${MAME_DIR}/src/devices/video/voodoo_pci.h
	)
endif()

##################################################
##
##@src/devices/video/crtc_ega.h,list(APPEND VIDEOS CRTC_EGA)
##################################################

if("CRTC_EGA" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/crtc_ega.cpp
		${MAME_DIR}/src/devices/video/crtc_ega.h
	)
endif()

##################################################
##
##@src/devices/video/jangou_blitter.h,list(APPEND VIDEOS JANGOU_BLITTER)
##################################################

if("JANGOU_BLITTER" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/jangou_blitter.cpp
		${MAME_DIR}/src/devices/video/jangou_blitter.h
	)
endif()

##################################################
##
##@src/devices/video/gb_lcd.h,list(APPEND VIDEOS GB_LCD)
##################################################

if("GB_LCD" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/gb_lcd.cpp
		${MAME_DIR}/src/devices/video/gb_lcd.h
	)
endif()

##################################################
##
##@src/devices/video/gba_lcd.h,list(APPEND VIDEOS GBA_LCD)
##################################################

if("GBA_LCD" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/gba_lcd.cpp
		${MAME_DIR}/src/devices/video/gba_lcd.h
	)
endif()

##################################################
##
##@src/devices/video/ef9369.h,list(APPEND VIDEOS EF9369)
##################################################

if("EF9369" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/ef9369.cpp
		${MAME_DIR}/src/devices/video/ef9369.h
	)
endif()

##################################################
##
##@src/devices/video/ppu2c0x.h,list(APPEND VIDEOS PPU2C0X)
##################################################

if("PPU2C0X" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/ppu2c0x.cpp
		${MAME_DIR}/src/devices/video/ppu2c0x.h
		${MAME_DIR}/src/devices/video/ppu2c0x_vt.cpp
		${MAME_DIR}/src/devices/video/ppu2c0x_vt.h
		${MAME_DIR}/src/devices/video/ppu2c0x_sh6578.cpp
		${MAME_DIR}/src/devices/video/ppu2c0x_sh6578.h
	)
endif()

##################################################
##
##@src/devices/video/bt459.h,list(APPEND VIDEOS BT459)
##################################################

if("BT459" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/bt459.cpp
		${MAME_DIR}/src/devices/video/bt459.h
	)
endif()

##################################################
##
##@src/devices/video/imagetek_i4100.h,list(APPEND VIDEOS I4100)
##################################################

if("I4100" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/imagetek_i4100.cpp
		${MAME_DIR}/src/devices/video/imagetek_i4100.h
	)
endif()

##################################################
##
##@src/devices/video/dp8510.h,list(APPEND VIDEOS DP8510)
##################################################

if("DP8510" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/dp8510.cpp
		${MAME_DIR}/src/devices/video/dp8510.h
	)
endif()

##################################################
##
##@src/devices/video/bt45x.h,list(APPEND VIDEOS BT45X)
##################################################

if("BT45X" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/bt45x.cpp
		${MAME_DIR}/src/devices/video/bt45x.h
	)
endif()

##################################################
##
##@src/devices/video/topcat.h,list(APPEND VIDEOS TOPCAT)
##################################################
if("TOPCAT" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/topcat.cpp
		${MAME_DIR}/src/devices/video/topcat.h
	)
endif()

##################################################
##
##@src/devices/video/catseye.h,list(APPEND VIDEOS CATSEYE)
##################################################
if("CATSEYE" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/catseye.cpp
		${MAME_DIR}/src/devices/video/catseye.h
	)
endif()


##################################################
##
##@src/devices/video/nereid.h,list(APPEND VIDEOS NEREID)
##################################################
if("NEREID" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/nereid.cpp
		${MAME_DIR}/src/devices/video/nereid.h
	)
endif()

##################################################
##
##@src/devices/video/ps2gif.h,list(APPEND VIDEOS PS2GIF)
##################################################
if("PS2GIF" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/ps2gif.cpp
		${MAME_DIR}/src/devices/video/ps2gif.h
	)
endif()

##################################################
##
##@src/devices/video/ps2gs.h,list(APPEND VIDEOS PS2GS)
##################################################
if("PS2GS" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/ps2gs.cpp
		${MAME_DIR}/src/devices/video/ps2gs.h
	)
endif()

##################################################
##
##@src/devices/video/decsfb.h,list(APPEND VIDEOS DECSFB)
##################################################
if("DECSFB" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/decsfb.cpp
		${MAME_DIR}/src/devices/video/decsfb.h
	)
endif()

##################################################
##
##@src/devices/video/bt47x.h,list(APPEND VIDEOS BT47X)
##################################################

if("BT47X" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/bt47x.cpp
		${MAME_DIR}/src/devices/video/bt47x.h
	)
endif()

##################################################
##
##@src/devices/video/bt431.h,list(APPEND VIDEOS BT431)
##################################################

if("BT431" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/bt431.cpp
		${MAME_DIR}/src/devices/video/bt431.h
	)
endif()

##################################################
##
##@src/devices/video/vrender0.h,list(APPEND VIDEOS VRENDER0)
##################################################

if("VRENDER0" IN_LIST VIDEOS)
	list(APPEND VIDEO_SRCS
		${MAME_DIR}/src/devices/video/vrender0.cpp
		${MAME_DIR}/src/devices/video/vrender0.h
	)
endif()
