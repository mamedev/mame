---------------------------------------------------------------------------
--
--   bus.lua
--
--   Rules for building bus cores
--
--   Copyright Nicola Salmoria and the MAME Team.
--   Visit http://mamedev.org for licensing and usage restrictions.
--
---------------------------------------------------------------------------

-------------------------------------------------
--
--@src/emu/bus/a7800/a78_slot.h,BUSES += A7800
---------------------------------------------------

if (BUSES["A7800"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/a7800/a78_slot.*",
		MAME_DIR .. "src/emu/bus/a7800/rom.*",
		MAME_DIR .. "src/emu/bus/a7800/hiscore.*",
		MAME_DIR .. "src/emu/bus/a7800/xboard.*",
		MAME_DIR .. "src/emu/bus/a7800/cpuwiz.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/a800/a800_slot.h,BUSES += A800
---------------------------------------------------

if (BUSES["A800"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/a800/a800_slot.*",
		MAME_DIR .. "src/emu/bus/a800/rom.*",
		MAME_DIR .. "src/emu/bus/a800/oss.*",
		MAME_DIR .. "src/emu/bus/a800/sparta.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/a8sio/a8sio.h,BUSES += A8SIO
---------------------------------------------------

if (BUSES["A8SIO"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/a8sio/a8sio.*",
		MAME_DIR .. "src/emu/bus/a8sio/cassette.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/abcbus/abcbus.h,BUSES += ABCBUS
---------------------------------------------------

if (BUSES["ABCBUS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/abcbus/abcbus.*",
		MAME_DIR .. "src/emu/bus/abcbus/abc890.*",
		MAME_DIR .. "src/emu/bus/abcbus/dos.*",
		MAME_DIR .. "src/emu/bus/abcbus/fd2.*",
		MAME_DIR .. "src/emu/bus/abcbus/hdc.*",
		MAME_DIR .. "src/emu/bus/abcbus/lux10828.*",
		MAME_DIR .. "src/emu/bus/abcbus/lux21046.*",
		MAME_DIR .. "src/emu/bus/abcbus/lux21056.*",
		MAME_DIR .. "src/emu/bus/abcbus/lux4105.*",
		MAME_DIR .. "src/emu/bus/abcbus/uni800.*",
		MAME_DIR .. "src/emu/bus/abcbus/sio.*",
		MAME_DIR .. "src/emu/bus/abcbus/slutprov.*",
		MAME_DIR .. "src/emu/bus/abcbus/turbo.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/adam/exp.h,BUSES += ADAM
---------------------------------------------------

if (BUSES["ADAM"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/adam/exp.*",
		MAME_DIR .. "src/emu/bus/adam/adamlink.*",
		MAME_DIR .. "src/emu/bus/adam/ide.*",
		MAME_DIR .. "src/emu/bus/adam/ram.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/adamnet/adamnet.h,BUSES += ADAMNET
---------------------------------------------------

if (BUSES["ADAMNET"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/adamnet/adamnet.*",
		MAME_DIR .. "src/emu/bus/adamnet/ddp.*",
		MAME_DIR .. "src/emu/bus/adamnet/fdc.*",
		MAME_DIR .. "src/emu/bus/adamnet/kb.*",
		MAME_DIR .. "src/emu/bus/adamnet/printer.*",
		MAME_DIR .. "src/emu/bus/adamnet/spi.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/apf/slot.h,BUSES += APF
---------------------------------------------------

if (BUSES["APF"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/apf/slot.*",
		MAME_DIR .. "src/emu/bus/apf/rom.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/arcadia/slot.h,BUSES += ARCADIA
---------------------------------------------------

if (BUSES["ARCADIA"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/arcadia/slot.*",
		MAME_DIR .. "src/emu/bus/arcadia/rom.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/astrocde/slot.h,BUSES += ASTROCADE
---------------------------------------------------

if (BUSES["ASTROCADE"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/astrocde/slot.*",
		MAME_DIR .. "src/emu/bus/astrocde/rom.*",
		MAME_DIR .. "src/emu/bus/astrocde/exp.*",
		MAME_DIR .. "src/emu/bus/astrocde/ram.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/bw2/exp.h,BUSES += BW2
---------------------------------------------------

if (BUSES["BW2"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/bw2/exp.*",
		MAME_DIR .. "src/emu/bus/bw2/ramcard.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/c64/exp.h,BUSES += C64
--@src/emu/bus/c64/user.h,BUSES += C64
---------------------------------------------------

if (BUSES["C64"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/c64/exp.*",
		MAME_DIR .. "src/emu/bus/c64/c128_comal80.*",
		MAME_DIR .. "src/emu/bus/c64/comal80.*",
		MAME_DIR .. "src/emu/bus/c64/cpm.*",
		MAME_DIR .. "src/emu/bus/c64/currah_speech.*",
		MAME_DIR .. "src/emu/bus/c64/dela_ep256.*",
		MAME_DIR .. "src/emu/bus/c64/dela_ep64.*",
		MAME_DIR .. "src/emu/bus/c64/dela_ep7x8.*",
		MAME_DIR .. "src/emu/bus/c64/dinamic.*",
		MAME_DIR .. "src/emu/bus/c64/dqbb.*",
		MAME_DIR .. "src/emu/bus/c64/easy_calc_result.*",
		MAME_DIR .. "src/emu/bus/c64/easyflash.*",
		MAME_DIR .. "src/emu/bus/c64/epyx_fast_load.*",
		MAME_DIR .. "src/emu/bus/c64/exos.*",
		MAME_DIR .. "src/emu/bus/c64/fcc.*",
		MAME_DIR .. "src/emu/bus/c64/final.*",
		MAME_DIR .. "src/emu/bus/c64/final3.*",
		MAME_DIR .. "src/emu/bus/c64/fun_play.*",
		MAME_DIR .. "src/emu/bus/c64/georam.*",
		MAME_DIR .. "src/emu/bus/c64/ide64.*",
		MAME_DIR .. "src/emu/bus/c64/ieee488.*",
		MAME_DIR .. "src/emu/bus/c64/kingsoft.*",
		MAME_DIR .. "src/emu/bus/c64/mach5.*",
		MAME_DIR .. "src/emu/bus/c64/magic_desk.*",
		MAME_DIR .. "src/emu/bus/c64/magic_formel.*",
		MAME_DIR .. "src/emu/bus/c64/magic_voice.*",
		MAME_DIR .. "src/emu/bus/c64/midi_maplin.*",
		MAME_DIR .. "src/emu/bus/c64/midi_namesoft.*",
		MAME_DIR .. "src/emu/bus/c64/midi_passport.*",
		MAME_DIR .. "src/emu/bus/c64/midi_sci.*",
		MAME_DIR .. "src/emu/bus/c64/midi_siel.*",
		MAME_DIR .. "src/emu/bus/c64/mikro_assembler.*",
		MAME_DIR .. "src/emu/bus/c64/multiscreen.*",
		MAME_DIR .. "src/emu/bus/c64/music64.*",
		MAME_DIR .. "src/emu/bus/c64/neoram.*",
		MAME_DIR .. "src/emu/bus/c64/ocean.*",
		MAME_DIR .. "src/emu/bus/c64/pagefox.*",
		MAME_DIR .. "src/emu/bus/c64/partner.*",
		MAME_DIR .. "src/emu/bus/c64/prophet64.*",
		MAME_DIR .. "src/emu/bus/c64/ps64.*",
		MAME_DIR .. "src/emu/bus/c64/reu.*",
		MAME_DIR .. "src/emu/bus/c64/rex.*",
		MAME_DIR .. "src/emu/bus/c64/rex_ep256.*",
		MAME_DIR .. "src/emu/bus/c64/ross.*",
		MAME_DIR .. "src/emu/bus/c64/sfx_sound_expander.*",
		MAME_DIR .. "src/emu/bus/c64/silverrock.*",
		MAME_DIR .. "src/emu/bus/c64/simons_basic.*",
		MAME_DIR .. "src/emu/bus/c64/stardos.*",
		MAME_DIR .. "src/emu/bus/c64/std.*",
		MAME_DIR .. "src/emu/bus/c64/structured_basic.*",
		MAME_DIR .. "src/emu/bus/c64/super_explode.*",
		MAME_DIR .. "src/emu/bus/c64/super_games.*",
		MAME_DIR .. "src/emu/bus/c64/supercpu.*",
		MAME_DIR .. "src/emu/bus/c64/sw8k.*",
		MAME_DIR .. "src/emu/bus/c64/swiftlink.*",
		MAME_DIR .. "src/emu/bus/c64/system3.*",
		MAME_DIR .. "src/emu/bus/c64/tdos.*",
		MAME_DIR .. "src/emu/bus/c64/turbo232.*",
		MAME_DIR .. "src/emu/bus/c64/vizastar.*",
		MAME_DIR .. "src/emu/bus/c64/vw64.*",
		MAME_DIR .. "src/emu/bus/c64/warp_speed.*",
		MAME_DIR .. "src/emu/bus/c64/westermann.*",
		MAME_DIR .. "src/emu/bus/c64/xl80.*",
		MAME_DIR .. "src/emu/bus/c64/zaxxon.*",
		MAME_DIR .. "src/emu/bus/c64/user.*",
		MAME_DIR .. "src/emu/bus/c64/4dxh.*",
		MAME_DIR .. "src/emu/bus/c64/4ksa.*",
		MAME_DIR .. "src/emu/bus/c64/4tba.*",
		MAME_DIR .. "src/emu/bus/c64/16kb.*",
		MAME_DIR .. "src/emu/bus/c64/bn1541.*",
		MAME_DIR .. "src/emu/bus/c64/geocable.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/cbm2/exp.h,BUSES += CBM2
--@src/emu/bus/cbm2/user.h,BUSES += CBM2
---------------------------------------------------

if (BUSES["CBM2"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/cbm2/exp.*",
		MAME_DIR .. "src/emu/bus/cbm2/24k.*",
		MAME_DIR .. "src/emu/bus/cbm2/hrg.*",
		MAME_DIR .. "src/emu/bus/cbm2/std.*",
		MAME_DIR .. "src/emu/bus/cbm2/user.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/cbmiec/cbmiec.h,BUSES += CBMIEC
---------------------------------------------------

if (BUSES["CBMIEC"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/cbmiec/cbmiec.*",
		MAME_DIR .. "src/emu/bus/cbmiec/c1541.*",
		MAME_DIR .. "src/emu/bus/cbmiec/c1571.*",
		MAME_DIR .. "src/emu/bus/cbmiec/c1581.*",
		MAME_DIR .. "src/emu/bus/cbmiec/c64_nl10.*",
		MAME_DIR .. "src/emu/bus/cbmiec/cmdhd.*",
		MAME_DIR .. "src/emu/bus/cbmiec/diag264_lb_iec.*",
		MAME_DIR .. "src/emu/bus/cbmiec/fd2000.*",
		MAME_DIR .. "src/emu/bus/cbmiec/interpod.*",
		MAME_DIR .. "src/emu/bus/cbmiec/serialbox.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/chanf/slot.h,BUSES += CHANNELF
---------------------------------------------------

if (BUSES["CHANNELF"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/chanf/slot.*",
		MAME_DIR .. "src/emu/bus/chanf/rom.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/comx35/exp.h,BUSES += COMX35
---------------------------------------------------

if (BUSES["COMX35"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/comx35/exp.*",
		MAME_DIR .. "src/emu/bus/comx35/clm.*",
		MAME_DIR .. "src/emu/bus/comx35/expbox.*",
		MAME_DIR .. "src/emu/bus/comx35/eprom.*",
		MAME_DIR .. "src/emu/bus/comx35/fdc.*",
		MAME_DIR .. "src/emu/bus/comx35/joycard.*",
		MAME_DIR .. "src/emu/bus/comx35/printer.*",
		MAME_DIR .. "src/emu/bus/comx35/ram.*",
		MAME_DIR .. "src/emu/bus/comx35/thermal.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/coleco/ctrl.h,BUSES += COLECO
---------------------------------------------------

if (BUSES["COLECO"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/coleco/ctrl.*",
		MAME_DIR .. "src/emu/bus/coleco/hand.*",
		MAME_DIR .. "src/emu/bus/coleco/sac.*",
		MAME_DIR .. "src/emu/bus/coleco/exp.*",
		MAME_DIR .. "src/emu/bus/coleco/std.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/crvision/slot.h,BUSES += CRVISION
---------------------------------------------------

if (BUSES["CRVISION"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/crvision/slot.*",
		MAME_DIR .. "src/emu/bus/crvision/rom.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/dmv/dmv.h,BUSES += DMV
---------------------------------------------------

if (BUSES["DMV"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/dmv/dmvbus.*",
		MAME_DIR .. "src/emu/bus/dmv/k210.*",
		MAME_DIR .. "src/emu/bus/dmv/k220.*",
		MAME_DIR .. "src/emu/bus/dmv/k230.*",
		MAME_DIR .. "src/emu/bus/dmv/k233.*",
		MAME_DIR .. "src/emu/bus/dmv/k801.*",
		MAME_DIR .. "src/emu/bus/dmv/k803.*",
		MAME_DIR .. "src/emu/bus/dmv/k806.*",
		MAME_DIR .. "src/emu/bus/dmv/ram.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/ecbbus/ecbbus.h,BUSES += ECBBUS
---------------------------------------------------

if (BUSES["ECBBUS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/ecbbus/ecbbus.*",
		MAME_DIR .. "src/emu/bus/ecbbus/grip.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/econet/econet.h,BUSES += ECONET
---------------------------------------------------

if (BUSES["ECONET"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/econet/econet.*",
		MAME_DIR .. "src/emu/bus/econet/e01.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/ep64/exp.h,BUSES += EP64
---------------------------------------------------

if (BUSES["EP64"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/ep64/exp.*",
		MAME_DIR .. "src/emu/bus/ep64/exdos.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/generic/slot.h,BUSES += GENERIC
---------------------------------------------------

if (BUSES["GENERIC"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/generic/slot.*",
		MAME_DIR .. "src/emu/bus/generic/carts.*",
		MAME_DIR .. "src/emu/bus/generic/ram.*",
		MAME_DIR .. "src/emu/bus/generic/rom.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/ieee488/ieee488.h,BUSES += IEEE488
---------------------------------------------------

if (BUSES["IEEE488"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/ieee488/ieee488.*",
		MAME_DIR .. "src/emu/bus/ieee488/c2031.*",
		MAME_DIR .. "src/emu/bus/ieee488/c2040.*",
		MAME_DIR .. "src/emu/bus/ieee488/c2040fdc.*",
		MAME_DIR .. "src/emu/bus/ieee488/c8050.*",
		MAME_DIR .. "src/emu/bus/ieee488/c8050fdc.*",
		MAME_DIR .. "src/emu/bus/ieee488/c8280.*",
		MAME_DIR .. "src/emu/bus/ieee488/d9060.*",
		MAME_DIR .. "src/emu/bus/ieee488/softbox.*",
		MAME_DIR .. "src/emu/bus/ieee488/hardbox.*",
		MAME_DIR .. "src/emu/bus/ieee488/shark.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/iq151/iq151.h,BUSES += IQ151
---------------------------------------------------

if (BUSES["IQ151"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/iq151/iq151.*",
		MAME_DIR .. "src/emu/bus/iq151/disc2.*",
		MAME_DIR .. "src/emu/bus/iq151/grafik.*",
		MAME_DIR .. "src/emu/bus/iq151/minigraf.*",
		MAME_DIR .. "src/emu/bus/iq151/ms151a.*",
		MAME_DIR .. "src/emu/bus/iq151/rom.*",
		MAME_DIR .. "src/emu/bus/iq151/staper.*",
		MAME_DIR .. "src/emu/bus/iq151/video32.*",
		MAME_DIR .. "src/emu/bus/iq151/video64.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/isbx/isbx.h,BUSES += IMI7000
---------------------------------------------------

if (BUSES["IMI7000"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/imi7000/imi7000.*",
		MAME_DIR .. "src/emu/bus/imi7000/imi5000h.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/intv/slot.h,BUSES += INTV
---------------------------------------------------

if (BUSES["INTV"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/intv/slot.*",
		MAME_DIR .. "src/emu/bus/intv/rom.*",
		MAME_DIR .. "src/emu/bus/intv/voice.*",
		MAME_DIR .. "src/emu/bus/intv/ecs.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/isa/isa.h,BUSES += ISA
---------------------------------------------------

if (BUSES["ISA"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/isa/isa.*",
		MAME_DIR .. "src/emu/bus/isa/isa_cards.*",
		MAME_DIR .. "src/emu/bus/isa/mda.*",
		MAME_DIR .. "src/emu/bus/isa/wdxt_gen.*",
		MAME_DIR .. "src/emu/bus/isa/adlib.*",
		MAME_DIR .. "src/emu/bus/isa/com.*",
		MAME_DIR .. "src/emu/bus/isa/fdc.*",
		MAME_DIR .. "src/emu/bus/isa/mufdc.*",
		MAME_DIR .. "src/emu/bus/isa/finalchs.*",
		MAME_DIR .. "src/emu/bus/isa/gblaster.*",
		MAME_DIR .. "src/emu/bus/isa/gus.*",
		MAME_DIR .. "src/emu/bus/isa/sb16.*",
		MAME_DIR .. "src/emu/bus/isa/hdc.*",
		MAME_DIR .. "src/emu/bus/isa/ibm_mfc.*",
		MAME_DIR .. "src/emu/bus/isa/mpu401.*",
		MAME_DIR .. "src/emu/bus/isa/sblaster.*",
		MAME_DIR .. "src/emu/bus/isa/stereo_fx.*",
		MAME_DIR .. "src/emu/bus/isa/ssi2001.*",
		MAME_DIR .. "src/emu/bus/isa/ide.*",
		MAME_DIR .. "src/emu/bus/isa/xtide.*",
		MAME_DIR .. "src/emu/bus/isa/side116.*",
		MAME_DIR .. "src/emu/bus/isa/aha1542.*",
		MAME_DIR .. "src/emu/bus/isa/wd1002a_wx1.*",
		MAME_DIR .. "src/emu/bus/isa/dectalk.*",
		MAME_DIR .. "src/emu/bus/isa/pds.*",
		MAME_DIR .. "src/emu/bus/isa/omti8621.*",
		MAME_DIR .. "src/emu/bus/isa/cga.*",
		MAME_DIR .. "src/emu/bus/isa/svga_cirrus.*",
		MAME_DIR .. "src/emu/bus/isa/ega.*",
		MAME_DIR .. "src/emu/bus/isa/vga.*",
		MAME_DIR .. "src/emu/bus/isa/vga_ati.*",
		MAME_DIR .. "src/emu/bus/isa/mach32.*",
		MAME_DIR .. "src/emu/bus/isa/svga_tseng.*",
		MAME_DIR .. "src/emu/bus/isa/svga_s3.*",
		MAME_DIR .. "src/emu/bus/isa/s3virge.*",
		MAME_DIR .. "src/emu/bus/isa/pc1640_iga.*",
		MAME_DIR .. "src/emu/bus/isa/3c503.*",
		MAME_DIR .. "src/emu/bus/isa/ne1000.*",
		MAME_DIR .. "src/emu/bus/isa/ne2000.*",
		MAME_DIR .. "src/emu/bus/isa/3c505.*",
		MAME_DIR .. "src/emu/bus/isa/lpt.*",
		MAME_DIR .. "src/emu/bus/isa/p1_fdc.*",
		MAME_DIR .. "src/emu/bus/isa/p1_hdc.*",
		MAME_DIR .. "src/emu/bus/isa/p1_rom.*",
		MAME_DIR .. "src/emu/bus/isa/mc1502_fdc.*",
		MAME_DIR .. "src/emu/bus/isa/mc1502_rom.*",
		MAME_DIR .. "src/emu/bus/isa/xsu_cards.*",
		MAME_DIR .. "src/emu/bus/isa/sc499.*",
		MAME_DIR .. "src/emu/bus/isa/aga.*",
		MAME_DIR .. "src/emu/bus/isa/svga_trident.*",
		MAME_DIR .. "src/emu/bus/isa/num9rev.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/isbx/isbx.h,BUSES += ISBX
---------------------------------------------------

if (BUSES["ISBX"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/isbx/isbx.*",
		MAME_DIR .. "src/emu/bus/isbx/compis_fdc.*",
		MAME_DIR .. "src/emu/bus/isbx/isbc_218a.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/msx_slot/slot.h,BUSES += MSX_SLOT
---------------------------------------------------

if (BUSES["MSX_SLOT"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/msx_slot/bunsetsu.*",
		MAME_DIR .. "src/emu/bus/msx_slot/cartridge.*",
		MAME_DIR .. "src/emu/bus/msx_slot/disk.*",
		MAME_DIR .. "src/emu/bus/msx_slot/fs4600.*",
		MAME_DIR .. "src/emu/bus/msx_slot/music.*",
		MAME_DIR .. "src/emu/bus/msx_slot/panasonic08.*",
		MAME_DIR .. "src/emu/bus/msx_slot/rom.*",
		MAME_DIR .. "src/emu/bus/msx_slot/ram.*",
		MAME_DIR .. "src/emu/bus/msx_slot/ram_mm.*",
		MAME_DIR .. "src/emu/bus/msx_slot/slot.*",
		MAME_DIR .. "src/emu/bus/msx_slot/sony08.*",
		MAME_DIR .. "src/emu/bus/msx_cart/arc.*",
		MAME_DIR .. "src/emu/bus/msx_cart/ascii.*",
		MAME_DIR .. "src/emu/bus/msx_cart/bm_012.*",
		MAME_DIR .. "src/emu/bus/msx_cart/cartridge.*",
		MAME_DIR .. "src/emu/bus/msx_cart/crossblaim.*",
		MAME_DIR .. "src/emu/bus/msx_cart/disk.*",
		MAME_DIR .. "src/emu/bus/msx_cart/dooly.*",
		MAME_DIR .. "src/emu/bus/msx_cart/fmpac.*",
		MAME_DIR .. "src/emu/bus/msx_cart/halnote.*",
		MAME_DIR .. "src/emu/bus/msx_cart/hfox.*",
		MAME_DIR .. "src/emu/bus/msx_cart/holy_quran.*",
		MAME_DIR .. "src/emu/bus/msx_cart/konami.*",
		MAME_DIR .. "src/emu/bus/msx_cart/korean.*",
		MAME_DIR .. "src/emu/bus/msx_cart/majutsushi.*",
		MAME_DIR .. "src/emu/bus/msx_cart/msx_audio.*",
		MAME_DIR .. "src/emu/bus/msx_cart/msx_audio_kb.*",
		MAME_DIR .. "src/emu/bus/msx_cart/msxdos2.*",
		MAME_DIR .. "src/emu/bus/msx_cart/nomapper.*",
		MAME_DIR .. "src/emu/bus/msx_cart/rtype.*",
		MAME_DIR .. "src/emu/bus/msx_cart/superloderunner.*",
		MAME_DIR .. "src/emu/bus/msx_cart/super_swangi.*",
		MAME_DIR .. "src/emu/bus/msx_cart/yamaha.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/kc/kc.h,BUSES += KC
---------------------------------------------------

if (BUSES["KC"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/kc/kc.*",
		MAME_DIR .. "src/emu/bus/kc/d002.*",
		MAME_DIR .. "src/emu/bus/kc/d004.*",
		MAME_DIR .. "src/emu/bus/kc/ram.*",
		MAME_DIR .. "src/emu/bus/kc/rom.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/odyssey2/slot.h,BUSES += O2
---------------------------------------------------

if (BUSES["O2"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/odyssey2/slot.*",
		MAME_DIR .. "src/emu/bus/odyssey2/rom.*",
		MAME_DIR .. "src/emu/bus/odyssey2/chess.*",
		MAME_DIR .. "src/emu/bus/odyssey2/voice.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/pc_joy/pc_joy.h,BUSES += PC_JOY
---------------------------------------------------

if (BUSES["PC_JOY"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/pc_joy/pc_joy.*",
		MAME_DIR .. "src/emu/bus/pc_joy/pc_joy_sw.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/pc_kbd/pc_kbdc.h,BUSES += PC_KBD
---------------------------------------------------

if (BUSES["PC_KBD"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/pc_kbd/pc_kbdc.*",
		MAME_DIR .. "src/emu/bus/pc_kbd/keyboards.*",
		MAME_DIR .. "src/emu/bus/pc_kbd/ec1841.*",
		MAME_DIR .. "src/emu/bus/pc_kbd/iskr1030.*",
		MAME_DIR .. "src/emu/bus/pc_kbd/keytro.*",
		MAME_DIR .. "src/emu/bus/pc_kbd/msnat.*",
		MAME_DIR .. "src/emu/bus/pc_kbd/pc83.*",
		MAME_DIR .. "src/emu/bus/pc_kbd/pcat84.*",
		MAME_DIR .. "src/emu/bus/pc_kbd/pcxt83.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/pet/cass.h,BUSES += PET
--@src/emu/bus/pet/exp.h,BUSES += PET
--@src/emu/bus/pet/user.h,BUSES += PET
---------------------------------------------------

if (BUSES["PET"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/pet/cass.*",
		MAME_DIR .. "src/emu/bus/pet/c2n.*",
		MAME_DIR .. "src/emu/bus/pet/diag264_lb_tape.*",
		MAME_DIR .. "src/emu/bus/pet/exp.*",
		MAME_DIR .. "src/emu/bus/pet/64k.*",
		MAME_DIR .. "src/emu/bus/pet/hsg.*",
		MAME_DIR .. "src/emu/bus/pet/superpet.*",
		MAME_DIR .. "src/emu/bus/pet/user.*",
		MAME_DIR .. "src/emu/bus/pet/diag.*",
		MAME_DIR .. "src/emu/bus/pet/petuja.*",
		MAME_DIR .. "src/emu/bus/pet/cb2snd.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/plus4/exp.h,BUSES += PLUS4
--@src/emu/bus/plus4/user.h,BUSES += PLUS4
---------------------------------------------------

if (BUSES["PLUS4"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/plus4/exp.*",
		MAME_DIR .. "src/emu/bus/plus4/c1551.*",
		MAME_DIR .. "src/emu/bus/plus4/sid.*",
		MAME_DIR .. "src/emu/bus/plus4/std.*",
		MAME_DIR .. "src/emu/bus/plus4/user.*",
		MAME_DIR .. "src/emu/bus/plus4/diag264_lb_user.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/s100/s100.h,BUSES += S100
---------------------------------------------------

if (BUSES["S100"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/s100/s100.*",
		MAME_DIR .. "src/emu/bus/s100/dj2db.*",
		MAME_DIR .. "src/emu/bus/s100/djdma.*",
		MAME_DIR .. "src/emu/bus/s100/mm65k16s.*",
		MAME_DIR .. "src/emu/bus/s100/nsmdsa.*",
		MAME_DIR .. "src/emu/bus/s100/nsmdsad.*",
		MAME_DIR .. "src/emu/bus/s100/wunderbus.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/spc1000/exp.h,BUSES += SPC1000
---------------------------------------------------

if (BUSES["SPC1000"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/spc1000/exp.*",
		MAME_DIR .. "src/emu/bus/spc1000/fdd.*",
		MAME_DIR .. "src/emu/bus/spc1000/vdp.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/tvc/tvc.h,BUSES += TVC
---------------------------------------------------

if (BUSES["TVC"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/tvc/tvc.*",
		MAME_DIR .. "src/emu/bus/tvc/hbf.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vc4000/slot.h,BUSES += VC4000
---------------------------------------------------

if (BUSES["VC4000"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vc4000/slot.*",
		MAME_DIR .. "src/emu/bus/vc4000/rom.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vcs/vcs_slot.h,BUSES += VCS
---------------------------------------------------

if (BUSES["VCS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vcs/vcs_slot.*",
		MAME_DIR .. "src/emu/bus/vcs/rom.*",
		MAME_DIR .. "src/emu/bus/vcs/compumat.*",
		MAME_DIR .. "src/emu/bus/vcs/dpc.*",
		MAME_DIR .. "src/emu/bus/vcs/scharger.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vcs/ctrl.h,BUSES += VCS_CTRL
---------------------------------------------------

if (BUSES["VCS_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vcs_ctrl/ctrl.*",
		MAME_DIR .. "src/emu/bus/vcs_ctrl/joystick.*",
		MAME_DIR .. "src/emu/bus/vcs_ctrl/joybooster.*",
		MAME_DIR .. "src/emu/bus/vcs_ctrl/keypad.*",
		MAME_DIR .. "src/emu/bus/vcs_ctrl/lightpen.*",
		MAME_DIR .. "src/emu/bus/vcs_ctrl/paddles.*",
		MAME_DIR .. "src/emu/bus/vcs_ctrl/wheel.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vectrex/slot.h,BUSES += VECTREX
---------------------------------------------------

if (BUSES["VECTREX"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vectrex/slot.*",
		MAME_DIR .. "src/emu/bus/vectrex/rom.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vic10/exp.h,BUSES += VIC10
---------------------------------------------------

if (BUSES["VIC10"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vic10/exp.*",
		MAME_DIR .. "src/emu/bus/vic10/std.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vic20/exp.h,BUSES += VIC20
--@src/emu/bus/vic20/user.h,BUSES += VIC20
---------------------------------------------------

if (BUSES["VIC20"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vic20/exp.*",
		MAME_DIR .. "src/emu/bus/vic20/fe3.*",
		MAME_DIR .. "src/emu/bus/vic20/megacart.*",
		MAME_DIR .. "src/emu/bus/vic20/std.*",
		MAME_DIR .. "src/emu/bus/vic20/vic1010.*",
		MAME_DIR .. "src/emu/bus/vic20/vic1110.*",
		MAME_DIR .. "src/emu/bus/vic20/vic1111.*",
		MAME_DIR .. "src/emu/bus/vic20/vic1112.*",
		MAME_DIR .. "src/emu/bus/vic20/vic1210.*",
		MAME_DIR .. "src/emu/bus/vic20/user.*",
		MAME_DIR .. "src/emu/bus/vic20/4cga.*",
		MAME_DIR .. "src/emu/bus/vic20/vic1011.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vidbrain/exp.h,BUSES += VIDBRAIN
---------------------------------------------------

if (BUSES["VIDBRAIN"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vidbrain/exp.*",
		MAME_DIR .. "src/emu/bus/vidbrain/std.*",
		MAME_DIR .. "src/emu/bus/vidbrain/money_minder.*",
		MAME_DIR .. "src/emu/bus/vidbrain/timeshare.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vip/byteio.h,BUSES += VIP
--@src/emu/bus/vip/exp.h,BUSES += VIP
---------------------------------------------------

if (BUSES["VIP"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vip/byteio.*",
		MAME_DIR .. "src/emu/bus/vip/vp620.*",
		MAME_DIR .. "src/emu/bus/vip/exp.*",
		MAME_DIR .. "src/emu/bus/vip/vp550.*",
		MAME_DIR .. "src/emu/bus/vip/vp570.*",
		MAME_DIR .. "src/emu/bus/vip/vp575.*",
		MAME_DIR .. "src/emu/bus/vip/vp585.*",
		MAME_DIR .. "src/emu/bus/vip/vp590.*",
		MAME_DIR .. "src/emu/bus/vip/vp595.*",
		MAME_DIR .. "src/emu/bus/vip/vp700.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/wangpc/wangpc.h,BUSES += WANGPC
---------------------------------------------------

if (BUSES["WANGPC"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/wangpc/wangpc.*",
		MAME_DIR .. "src/emu/bus/wangpc/emb.*",
		MAME_DIR .. "src/emu/bus/wangpc/lic.*",
		MAME_DIR .. "src/emu/bus/wangpc/lvc.*",
		MAME_DIR .. "src/emu/bus/wangpc/mcc.*",
		MAME_DIR .. "src/emu/bus/wangpc/mvc.*",
		MAME_DIR .. "src/emu/bus/wangpc/rtc.*",
		MAME_DIR .. "src/emu/bus/wangpc/tig.*",
		MAME_DIR .. "src/emu/bus/wangpc/wdc.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/z88/z88.h,BUSES += Z88
---------------------------------------------------

if (BUSES["Z88"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/z88/z88.*",
		MAME_DIR .. "src/emu/bus/z88/flash.*",
		MAME_DIR .. "src/emu/bus/z88/ram.*",
		MAME_DIR .. "src/emu/bus/z88/rom.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/a2bus/a2bus.h,BUSES += A2BUS
---------------------------------------------------

if (BUSES["A2BUS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/a2bus/a2bus.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2lang.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2diskii.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2mockingboard.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2cffa.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2memexp.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2scsi.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2thunderclock.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2softcard.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2videoterm.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2ssc.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2swyft.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2eauxslot.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2themill.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2sam.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2alfam2.*",
		MAME_DIR .. "src/emu/bus/a2bus/laser128.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2echoii.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2arcadebd.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2midi.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2vulcan.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2zipdrive.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2applicard.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2hsscsi.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2ultraterm.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2pic.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2estd80col.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2eext80col.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2eramworks3.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2corvus.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2diskiing.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2mcms.*",
		MAME_DIR .. "src/emu/bus/a2bus/a2dx1.*",
		MAME_DIR .. "src/emu/bus/a2bus/timemasterho.*",
		MAME_DIR .. "src/emu/bus/a2bus/mouse.*",
		MAME_DIR .. "src/emu/bus/a2bus/corvfdc01.*",
		MAME_DIR .. "src/emu/bus/a2bus/corvfdc02.*",
		MAME_DIR .. "src/emu/bus/a2bus/ramcard16k.*",
		MAME_DIR .. "src/emu/bus/a2bus/ramcard128k.*",
		MAME_DIR .. "src/emu/bus/a2bus/ezcgi.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/nubus/nubus.h,BUSES += NUBUS
---------------------------------------------------

if (BUSES["NUBUS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/nubus/nubus.*",
		MAME_DIR .. "src/emu/bus/nubus/nubus_48gc.*",
		MAME_DIR .. "src/emu/bus/nubus/nubus_cb264.*",
		MAME_DIR .. "src/emu/bus/nubus/nubus_vikbw.*",
		MAME_DIR .. "src/emu/bus/nubus/nubus_specpdq.*",
		MAME_DIR .. "src/emu/bus/nubus/nubus_m2hires.*",
		MAME_DIR .. "src/emu/bus/nubus/nubus_spec8.*",
		MAME_DIR .. "src/emu/bus/nubus/nubus_radiustpd.*",
		MAME_DIR .. "src/emu/bus/nubus/nubus_m2video.*",
		MAME_DIR .. "src/emu/bus/nubus/nubus_asntmc3b.*",
		MAME_DIR .. "src/emu/bus/nubus/nubus_image.*",
		MAME_DIR .. "src/emu/bus/nubus/nubus_wsportrait.*",
		MAME_DIR .. "src/emu/bus/nubus/pds30_cb264.*",
		MAME_DIR .. "src/emu/bus/nubus/pds30_procolor816.*",
		MAME_DIR .. "src/emu/bus/nubus/pds30_sigmalview.*",
		MAME_DIR .. "src/emu/bus/nubus/pds30_30hr.*",
		MAME_DIR .. "src/emu/bus/nubus/pds30_mc30.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/centronics/ctronics.h,BUSES += CENTRONICS
---------------------------------------------------

if (BUSES["CENTRONICS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/centronics/ctronics.*",
		MAME_DIR .. "src/emu/bus/centronics/comxpl80.*",
		MAME_DIR .. "src/emu/bus/centronics/covox.*",
		MAME_DIR .. "src/emu/bus/centronics/dsjoy.*",
		MAME_DIR .. "src/emu/bus/centronics/epson_ex800.*",
		MAME_DIR .. "src/emu/bus/centronics/epson_lx800.*",
		MAME_DIR .. "src/emu/bus/centronics/epson_lx810l.*",
		MAME_DIR .. "src/emu/bus/centronics/printer.*",
		MAME_DIR .. "src/emu/bus/centronics/digiblst.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/rs232/rs232.h,BUSES += RS232
---------------------------------------------------

if (BUSES["RS232"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/rs232/keyboard.*",
		MAME_DIR .. "src/emu/bus/rs232/loopback.*",
		MAME_DIR .. "src/emu/bus/rs232/null_modem.*",
		MAME_DIR .. "src/emu/bus/rs232/printer.*",
		MAME_DIR .. "src/emu/bus/rs232/rs232.*",
		MAME_DIR .. "src/emu/bus/rs232/ser_mouse.*",
		MAME_DIR .. "src/emu/bus/rs232/terminal.*",
		MAME_DIR .. "src/emu/bus/rs232/xvd701.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/midi/midi.h,BUSES += MIDI
---------------------------------------------------

if (BUSES["MIDI"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/midi/midi.*",
		MAME_DIR .. "src/emu/bus/midi/midiinport.*",
		MAME_DIR .. "src/emu/bus/midi/midioutport.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/lpci/pci.h,BUSES += LPCI
---------------------------------------------------

if (BUSES["LPCI"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/lpci/pci.*",
		MAME_DIR .. "src/emu/bus/lpci/cirrus.*",
		MAME_DIR .. "src/emu/bus/lpci/i82371ab.*",
		MAME_DIR .. "src/emu/bus/lpci/i82371sb.*",
		MAME_DIR .. "src/emu/bus/lpci/i82439tx.*",
		MAME_DIR .. "src/emu/bus/lpci/northbridge.*",
		MAME_DIR .. "src/emu/bus/lpci/southbridge.*",
		MAME_DIR .. "src/emu/bus/lpci/mpc105.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/nes/nes_slot.h,BUSES += NES
---------------------------------------------------

if (BUSES["NES"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/nes/nes_slot.*",
		MAME_DIR .. "src/emu/bus/nes/nes_carts.*",
		MAME_DIR .. "src/emu/bus/nes/2a03pur.*",
		MAME_DIR .. "src/emu/bus/nes/act53.*",
		MAME_DIR .. "src/emu/bus/nes/aladdin.*",
		MAME_DIR .. "src/emu/bus/nes/ave.*",
		MAME_DIR .. "src/emu/bus/nes/bandai.*",
		MAME_DIR .. "src/emu/bus/nes/benshieng.*",
		MAME_DIR .. "src/emu/bus/nes/bootleg.*",
		MAME_DIR .. "src/emu/bus/nes/camerica.*",
		MAME_DIR .. "src/emu/bus/nes/cne.*",
		MAME_DIR .. "src/emu/bus/nes/cony.*",
		MAME_DIR .. "src/emu/bus/nes/datach.*",
		MAME_DIR .. "src/emu/bus/nes/discrete.*",
		MAME_DIR .. "src/emu/bus/nes/disksys.*",
		MAME_DIR .. "src/emu/bus/nes/event.*",
		MAME_DIR .. "src/emu/bus/nes/ggenie.*",
		MAME_DIR .. "src/emu/bus/nes/henggedianzi.*",
		MAME_DIR .. "src/emu/bus/nes/hes.*",
		MAME_DIR .. "src/emu/bus/nes/hosenkan.*",
		MAME_DIR .. "src/emu/bus/nes/irem.*",
		MAME_DIR .. "src/emu/bus/nes/jaleco.*",
		MAME_DIR .. "src/emu/bus/nes/jy.*",
		MAME_DIR .. "src/emu/bus/nes/kaiser.*",
		MAME_DIR .. "src/emu/bus/nes/karastudio.*",
		MAME_DIR .. "src/emu/bus/nes/konami.*",
		MAME_DIR .. "src/emu/bus/nes/legacy.*",
		MAME_DIR .. "src/emu/bus/nes/mmc1.*",
		MAME_DIR .. "src/emu/bus/nes/mmc2.*",
		MAME_DIR .. "src/emu/bus/nes/mmc3.*",
		MAME_DIR .. "src/emu/bus/nes/mmc3_clones.*",
		MAME_DIR .. "src/emu/bus/nes/mmc5.*",
		MAME_DIR .. "src/emu/bus/nes/multigame.*",
		MAME_DIR .. "src/emu/bus/nes/namcot.*",
		MAME_DIR .. "src/emu/bus/nes/nanjing.*",
		MAME_DIR .. "src/emu/bus/nes/ntdec.*",
		MAME_DIR .. "src/emu/bus/nes/nxrom.*",
		MAME_DIR .. "src/emu/bus/nes/pirate.*",
		MAME_DIR .. "src/emu/bus/nes/pt554.*",
		MAME_DIR .. "src/emu/bus/nes/racermate.*",
		MAME_DIR .. "src/emu/bus/nes/rcm.*",
		MAME_DIR .. "src/emu/bus/nes/rexsoft.*",
		MAME_DIR .. "src/emu/bus/nes/sachen.*",
		MAME_DIR .. "src/emu/bus/nes/somari.*",
		MAME_DIR .. "src/emu/bus/nes/sunsoft.*",
		MAME_DIR .. "src/emu/bus/nes/sunsoft_dcs.*",
		MAME_DIR .. "src/emu/bus/nes/taito.*",
		MAME_DIR .. "src/emu/bus/nes/tengen.*",
		MAME_DIR .. "src/emu/bus/nes/txc.*",
		MAME_DIR .. "src/emu/bus/nes/waixing.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/nes_ctrl/ctrl.h,BUSES += NES_CTRL
---------------------------------------------------

if (BUSES["NES_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/nes_ctrl/ctrl.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/joypad.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/4score.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/arkpaddle.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/bcbattle.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/ftrainer.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/fckeybrd.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/hori.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/konamihs.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/miracle.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/mjpanel.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/pachinko.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/partytap.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/powerpad.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/suborkey.*",
		MAME_DIR .. "src/emu/bus/nes_ctrl/zapper.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/snes/snes_slot.h,BUSES += SNES
---------------------------------------------------

if (BUSES["SNES"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/snes/snes_slot.*",
		MAME_DIR .. "src/emu/bus/snes/snes_carts.*",
		MAME_DIR .. "src/emu/bus/snes/bsx.*",
		MAME_DIR .. "src/emu/bus/snes/event.*",
		MAME_DIR .. "src/emu/bus/snes/rom.*",
		MAME_DIR .. "src/emu/bus/snes/rom21.*",
		MAME_DIR .. "src/emu/bus/snes/sa1.*",
		MAME_DIR .. "src/emu/bus/snes/sdd1.*",
		MAME_DIR .. "src/emu/bus/snes/sfx.*",
		MAME_DIR .. "src/emu/bus/snes/sgb.*",
		MAME_DIR .. "src/emu/bus/snes/spc7110.*",
		MAME_DIR .. "src/emu/bus/snes/sufami.*",
		MAME_DIR .. "src/emu/bus/snes/upd.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/snes_ctrl/ctrl.h,BUSES += SNES_CTRL
---------------------------------------------------

if (BUSES["SNES_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/snes_ctrl/ctrl.*",
		MAME_DIR .. "src/emu/bus/snes_ctrl/bcbattle.*",
		MAME_DIR .. "src/emu/bus/snes_ctrl/joypad.*",
		MAME_DIR .. "src/emu/bus/snes_ctrl/miracle.*",
		MAME_DIR .. "src/emu/bus/snes_ctrl/mouse.*",
		MAME_DIR .. "src/emu/bus/snes_ctrl/multitap.*",
		MAME_DIR .. "src/emu/bus/snes_ctrl/pachinko.*",
		MAME_DIR .. "src/emu/bus/snes_ctrl/sscope.*",
		MAME_DIR .. "src/emu/bus/snes_ctrl/twintap.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/vboy/slot.h,BUSES += VBOY
---------------------------------------------------
if (BUSES["VBOY"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vboy/slot.*",
		MAME_DIR .. "src/emu/bus/vboy/rom.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/megadrive/md_slot.h,BUSES += MEGADRIVE
---------------------------------------------------

if (BUSES["MEGADRIVE"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/megadrive/md_slot.*",
		MAME_DIR .. "src/emu/bus/megadrive/md_carts.*",
		MAME_DIR .. "src/emu/bus/megadrive/eeprom.*",
		MAME_DIR .. "src/emu/bus/megadrive/ggenie.*",
		MAME_DIR .. "src/emu/bus/megadrive/jcart.*",
		MAME_DIR .. "src/emu/bus/megadrive/rom.*",
		MAME_DIR .. "src/emu/bus/megadrive/sk.*",
		MAME_DIR .. "src/emu/bus/megadrive/stm95.*",
		MAME_DIR .. "src/emu/bus/megadrive/svp.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/neogeo/neogeo_slot.h,BUSES += NEOGEO
---------------------------------------------------

if (BUSES["NEOGEO"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/neogeo/neogeo_slot.*",
		MAME_DIR .. "src/emu/bus/neogeo/neogeo_intf.*",
		MAME_DIR .. "src/emu/bus/neogeo/neogeo_carts.*",
		MAME_DIR .. "src/emu/bus/neogeo/neogeo_helper.*",
		MAME_DIR .. "src/emu/bus/neogeo/banked_cart.*",
		MAME_DIR .. "src/emu/bus/neogeo/mslugx_prot.*",
		MAME_DIR .. "src/emu/bus/neogeo/mslugx_cart.*",
		MAME_DIR .. "src/emu/bus/neogeo/sma_prot.*",
		MAME_DIR .. "src/emu/bus/neogeo/sma_cart.*",
		MAME_DIR .. "src/emu/bus/neogeo/cmc_prot.*",
		MAME_DIR .. "src/emu/bus/neogeo/cmc_cart.*",
		MAME_DIR .. "src/emu/bus/neogeo/pcm2_prot.*",
		MAME_DIR .. "src/emu/bus/neogeo/pcm2_cart.*",
		MAME_DIR .. "src/emu/bus/neogeo/kof2002_prot.*",
		MAME_DIR .. "src/emu/bus/neogeo/kof2002_cart.*",
		MAME_DIR .. "src/emu/bus/neogeo/pvc_prot.*",
		MAME_DIR .. "src/emu/bus/neogeo/pvc_cart.*",
		MAME_DIR .. "src/emu/bus/neogeo/fatfury2_prot.*",
		MAME_DIR .. "src/emu/bus/neogeo/fatfury2_cart.*",
		MAME_DIR .. "src/emu/bus/neogeo/kof98_prot.*",
		MAME_DIR .. "src/emu/bus/neogeo/kof98_cart.*",
		MAME_DIR .. "src/emu/bus/neogeo/bootleg_prot.*",
		MAME_DIR .. "src/emu/bus/neogeo/bootleg_cart.*",
		MAME_DIR .. "src/emu/bus/neogeo/bootleg_hybrid_cart.*",
		MAME_DIR .. "src/emu/bus/neogeo/sbp_prot.*",
		MAME_DIR .. "src/emu/bus/neogeo/kog_prot.*",
		MAME_DIR .. "src/emu/bus/neogeo/rom.*",
	}
end


---------------------------------------------------
--
--@src/emu/bus/saturn/sat_slot.h,BUSES += SATURN
---------------------------------------------------

if (BUSES["SATURN"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/saturn/sat_slot.*",
		MAME_DIR .. "src/emu/bus/saturn/bram.*",
		MAME_DIR .. "src/emu/bus/saturn/dram.*",
		MAME_DIR .. "src/emu/bus/saturn/rom.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/sega8/sega8_slot.h,BUSES += SEGA8
---------------------------------------------------

if (BUSES["SEGA8"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/sega8/sega8_slot.*",
		MAME_DIR .. "src/emu/bus/sega8/rom.*",
		MAME_DIR .. "src/emu/bus/sega8/ccatch.*",
		MAME_DIR .. "src/emu/bus/sega8/mgear.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/sms_ctrl/smsctrl.h,BUSES += SMS_CTRL
---------------------------------------------------

if (BUSES["SMS_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/sms_ctrl/smsctrl.*",
		MAME_DIR .. "src/emu/bus/sms_ctrl/joypad.*",
		MAME_DIR .. "src/emu/bus/sms_ctrl/lphaser.*",
		MAME_DIR .. "src/emu/bus/sms_ctrl/paddle.*",
		MAME_DIR .. "src/emu/bus/sms_ctrl/rfu.*",
		MAME_DIR .. "src/emu/bus/sms_ctrl/sports.*",
		MAME_DIR .. "src/emu/bus/sms_ctrl/sportsjp.*",
		MAME_DIR .. "src/emu/bus/sms_ctrl/multitap.*",
		MAME_DIR .. "src/emu/bus/sms_ctrl/graphic.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/sms_exp/smsexp.h,BUSES += SMS_EXP
---------------------------------------------------

if (BUSES["SMS_EXP"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/sms_exp/smsexp.*",
		MAME_DIR .. "src/emu/bus/sms_exp/gender.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/ti99_peb/peribox.h,BUSES += TI99PEB
---------------------------------------------------

if (BUSES["TI99PEB"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/ti99_peb/peribox.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/bwg.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/evpc.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/hfdc.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/horizon.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/hsgpl.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/memex.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/myarcmem.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/pcode.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/samsmem.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/spchsyn.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/ti_32kmem.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/ti_fdc.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/ti_rs232.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/tn_ide.*",
		MAME_DIR .. "src/emu/bus/ti99_peb/tn_usbsm.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/gameboy/gb_slot.h,BUSES += GAMEBOY
---------------------------------------------------

if (BUSES["GAMEBOY"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/gameboy/gb_slot.*",
		MAME_DIR .. "src/emu/bus/gameboy/rom.*",
		MAME_DIR .. "src/emu/bus/gameboy/mbc.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/gamegear/ggext.h,BUSES += GAMEGEAR
---------------------------------------------------

if (BUSES["GAMEGEAR"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/gamegear/ggext.*",
		MAME_DIR .. "src/emu/bus/gamegear/smsctrladp.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/gba/gba_slot.h,BUSES += GBA
---------------------------------------------------

if (BUSES["GBA"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/gba/gba_slot.*",
		MAME_DIR .. "src/emu/bus/gba/rom.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/bml3/bml3bus.h,BUSES += BML3
---------------------------------------------------
if (BUSES["BML3"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/bml3/bml3bus.*",
		MAME_DIR .. "src/emu/bus/bml3/bml3mp1802.*",
		MAME_DIR .. "src/emu/bus/bml3/bml3mp1805.*",
		MAME_DIR .. "src/emu/bus/bml3/bml3kanji.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/coco/cococart.h,BUSES += COCO
---------------------------------------------------
if (BUSES["COCO"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/coco/cococart.*",
		MAME_DIR .. "src/emu/bus/coco/coco_232.*",
		MAME_DIR .. "src/emu/bus/coco/coco_orch90.*",
		MAME_DIR .. "src/emu/bus/coco/coco_pak.*",
		MAME_DIR .. "src/emu/bus/coco/coco_fdc.*",
		MAME_DIR .. "src/emu/bus/coco/coco_multi.*",
		MAME_DIR .. "src/emu/bus/coco/coco_dwsock.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/cpc/cpcexp.h,BUSES += CPC
---------------------------------------------------
if (BUSES["CPC"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/cpc/cpcexp.*",
		MAME_DIR .. "src/emu/bus/cpc/cpc_ssa1.*",
		MAME_DIR .. "src/emu/bus/cpc/cpc_rom.*",
		MAME_DIR .. "src/emu/bus/cpc/cpc_pds.*",
		MAME_DIR .. "src/emu/bus/cpc/cpc_rs232.*",
		MAME_DIR .. "src/emu/bus/cpc/mface2.*",
		MAME_DIR .. "src/emu/bus/cpc/symbfac2.*",
		MAME_DIR .. "src/emu/bus/cpc/amdrum.*",
		MAME_DIR .. "src/emu/bus/cpc/playcity.*",
		MAME_DIR .. "src/emu/bus/cpc/smartwatch.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/epson_sio/epson_sio.h,BUSES += EPSON_SIO
---------------------------------------------------
if (BUSES["EPSON_SIO"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/epson_sio/epson_sio.*",
		MAME_DIR .. "src/emu/bus/epson_sio/pf10.*",
		MAME_DIR .. "src/emu/bus/epson_sio/tf20.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/pce/pce_slot.h,BUSES += PCE
---------------------------------------------------
if (BUSES["PCE"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/pce/pce_slot.*",
		MAME_DIR .. "src/emu/bus/pce/pce_rom.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/scv/slot.h,BUSES += SCV
---------------------------------------------------
if (BUSES["SCV"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/scv/slot.*",
		MAME_DIR .. "src/emu/bus/scv/rom.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/x68k/x68kexp.h,BUSES += X68K
---------------------------------------------------
if (BUSES["X68K"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/x68k/x68kexp.*",
		MAME_DIR .. "src/emu/bus/x68k/x68k_neptunex.*",
		MAME_DIR .. "src/emu/bus/x68k/x68k_scsiext.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/abckb/abckb.h,BUSES += ABCKB
---------------------------------------------------
if (BUSES["ABCKB"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/abckb/abckb.*",
		MAME_DIR .. "src/emu/bus/abckb/abc77.*",
		MAME_DIR .. "src/emu/bus/abckb/abc99.*",
		MAME_DIR .. "src/emu/bus/abckb/abc800kb.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/compucolor/compclr_flp.h,BUSES += COMPUCOLOR
---------------------------------------------------
if (BUSES["COMPUCOLOR"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/compucolor/floppy.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/scsi/scsi.h,BUSES += SCSI
---------------------------------------------------
if (BUSES["SCSI"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/scsi/scsi.*",
		MAME_DIR .. "src/emu/bus/scsi/scsicd.*",
		MAME_DIR .. "src/emu/bus/scsi/scsihd.*",
		MAME_DIR .. "src/emu/bus/scsi/scsihle.*",
		MAME_DIR .. "src/emu/bus/scsi/cdu76s.*",
		MAME_DIR .. "src/emu/bus/scsi/acb4070.*",
		MAME_DIR .. "src/emu/bus/scsi/d9060hd.*",
		MAME_DIR .. "src/emu/bus/scsi/sa1403d.*",
		MAME_DIR .. "src/emu/bus/scsi/s1410.*",
		MAME_DIR .. "src/emu/bus/scsi/pc9801_sasi.*",
		MAME_DIR .. "src/emu/bus/scsi/omti5100.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/macpds/macpds.h,BUSES += MACPDS
---------------------------------------------------
if (BUSES["MACPDS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/macpds/macpds.*",
		MAME_DIR .. "src/emu/bus/macpds/pds_tpdfpd.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/oricext/oricext.h,BUSES += ORICEXT
---------------------------------------------------
if (BUSES["ORICEXT"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/oricext/oricext.*",
		MAME_DIR .. "src/emu/bus/oricext/jasmin.*",
		MAME_DIR .. "src/emu/bus/oricext/microdisc.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/a1bus/a1bus.h,BUSES += A1BUS
---------------------------------------------------

if (BUSES["A1BUS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/a1bus/a1bus.*",
		MAME_DIR .. "src/emu/bus/a1bus/a1cassette.*",
		MAME_DIR .. "src/emu/bus/a1bus/a1cffa.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/amiga/zorro/zorro.h,BUSES += ZORRO
---------------------------------------------------

if (BUSES["ZORRO"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/amiga/zorro/zorro.*",
		MAME_DIR .. "src/emu/bus/amiga/zorro/cards.*",
		MAME_DIR .. "src/emu/bus/amiga/zorro/a2052.*",
		MAME_DIR .. "src/emu/bus/amiga/zorro/a2232.*",
		MAME_DIR .. "src/emu/bus/amiga/zorro/a590.*",
		MAME_DIR .. "src/emu/bus/amiga/zorro/action_replay.*",
		MAME_DIR .. "src/emu/bus/amiga/zorro/buddha.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/ql/exp.h,BUSES += QL
---------------------------------------------------

if (BUSES["QL"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/ql/exp.*",
		MAME_DIR .. "src/emu/bus/ql/cst_qdisc.*",
		MAME_DIR .. "src/emu/bus/ql/cst_q_plus4.*",
		MAME_DIR .. "src/emu/bus/ql/cumana_fdi.*",
		MAME_DIR .. "src/emu/bus/ql/kempston_di.*",
		MAME_DIR .. "src/emu/bus/ql/miracle_gold_card.*",
		MAME_DIR .. "src/emu/bus/ql/mp_fdi.*",
		MAME_DIR .. "src/emu/bus/ql/opd_basic_master.*",
		MAME_DIR .. "src/emu/bus/ql/pcml_qdisk.*",
		MAME_DIR .. "src/emu/bus/ql/qubide.*",
		MAME_DIR .. "src/emu/bus/ql/sandy_superdisk.*",
		MAME_DIR .. "src/emu/bus/ql/sandy_superqboard.*",
		MAME_DIR .. "src/emu/bus/ql/trumpcard.*",
		MAME_DIR .. "src/emu/bus/ql/rom.*",
		MAME_DIR .. "src/emu/bus/ql/miracle_hd.*",
		MAME_DIR .. "src/emu/bus/ql/std.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/vtech/memexp/memexp.h,BUSES += VTECH_MEMEXP
---------------------------------------------------

if (BUSES["VTECH_MEMEXP"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vtech/memexp/memexp.*",
		MAME_DIR .. "src/emu/bus/vtech/memexp/carts.*",
		MAME_DIR .. "src/emu/bus/vtech/memexp/floppy.*",
		MAME_DIR .. "src/emu/bus/vtech/memexp/memory.*",
		MAME_DIR .. "src/emu/bus/vtech/memexp/rs232.*",
		MAME_DIR .. "src/emu/bus/vtech/memexp/wordpro.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/vtech/ioexp/ioexp.h,BUSES += VTECH_IOEXP
---------------------------------------------------

if (BUSES["VTECH_IOEXP"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vtech/ioexp/ioexp.*",
		MAME_DIR .. "src/emu/bus/vtech/ioexp/carts.*",
		MAME_DIR .. "src/emu/bus/vtech/ioexp/joystick.*",
		MAME_DIR .. "src/emu/bus/vtech/ioexp/printer.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/wswan/slot.h,BUSES += WSWAN
---------------------------------------------------

if (BUSES["WSWAN"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/wswan/slot.*",
		MAME_DIR .. "src/emu/bus/wswan/rom.*",
	}
end

---------------------------------------------------
--
--@src/emu/bus/psx/ctlrport.h,BUSES += PSX_CONTROLLER
---------------------------------------------------

if (BUSES["PSX_CONTROLLER"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/psx/ctlrport.*",
		MAME_DIR .. "src/emu/bus/psx/analogue.*",
		MAME_DIR .. "src/emu/bus/psx/multitap.*",
		MAME_DIR .. "src/emu/bus/psx/memcard.*",
	}
end
