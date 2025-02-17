-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   machine.lua
--
--   Rules for building machine cores
--
----------------------------------------------------------------------------

files {
	MAME_DIR .. "src/devices/machine/bcreader.cpp",
	MAME_DIR .. "src/devices/machine/bcreader.h",
	MAME_DIR .. "src/devices/machine/buffer.cpp",
	MAME_DIR .. "src/devices/machine/buffer.h",
	MAME_DIR .. "src/devices/machine/clock.cpp",
	MAME_DIR .. "src/devices/machine/clock.h",
	MAME_DIR .. "src/devices/machine/keyboard.cpp",
	MAME_DIR .. "src/devices/machine/keyboard.h",
	MAME_DIR .. "src/devices/machine/keyboard.ipp",
	MAME_DIR .. "src/devices/machine/laserdsc.cpp",
	MAME_DIR .. "src/devices/machine/laserdsc.h",
	MAME_DIR .. "src/devices/machine/nvram.cpp",
	MAME_DIR .. "src/devices/machine/nvram.h",
	MAME_DIR .. "src/devices/machine/ram.cpp",
	MAME_DIR .. "src/devices/machine/ram.h",
	MAME_DIR .. "src/devices/machine/legscsi.cpp",
	MAME_DIR .. "src/devices/machine/legscsi.h",
	MAME_DIR .. "src/devices/machine/sdlc.cpp",
	MAME_DIR .. "src/devices/machine/sdlc.h",
	MAME_DIR .. "src/devices/machine/terminal.cpp",
	MAME_DIR .. "src/devices/machine/terminal.h",
	MAME_DIR .. "src/devices/machine/timer.cpp",
	MAME_DIR .. "src/devices/machine/timer.h",
}
files {
	MAME_DIR .. "src/devices/imagedev/bitbngr.cpp",
	MAME_DIR .. "src/devices/imagedev/bitbngr.h",
	MAME_DIR .. "src/devices/imagedev/cartrom.cpp",
	MAME_DIR .. "src/devices/imagedev/cartrom.h",
	MAME_DIR .. "src/devices/imagedev/cassette.cpp",
	MAME_DIR .. "src/devices/imagedev/cassette.h",
	MAME_DIR .. "src/devices/imagedev/cdromimg.cpp",
	MAME_DIR .. "src/devices/imagedev/cdromimg.h",
	MAME_DIR .. "src/devices/imagedev/diablo.cpp",
	MAME_DIR .. "src/devices/imagedev/diablo.h",
	MAME_DIR .. "src/devices/imagedev/flopdrv.cpp",
	MAME_DIR .. "src/devices/imagedev/flopdrv.h",
	MAME_DIR .. "src/devices/imagedev/floppy.cpp",
	MAME_DIR .. "src/devices/imagedev/floppy.h",
	MAME_DIR .. "src/devices/imagedev/harddriv.cpp",
	MAME_DIR .. "src/devices/imagedev/harddriv.h",
	MAME_DIR .. "src/devices/imagedev/magtape.cpp",
	MAME_DIR .. "src/devices/imagedev/magtape.h",
	MAME_DIR .. "src/devices/imagedev/memcard.cpp",
	MAME_DIR .. "src/devices/imagedev/memcard.h",
	MAME_DIR .. "src/devices/imagedev/mfmhd.cpp",
	MAME_DIR .. "src/devices/imagedev/mfmhd.h",
	MAME_DIR .. "src/devices/imagedev/microdrv.cpp",
	MAME_DIR .. "src/devices/imagedev/microdrv.h",
	MAME_DIR .. "src/devices/imagedev/midiin.cpp",
	MAME_DIR .. "src/devices/imagedev/midiin.h",
	MAME_DIR .. "src/devices/imagedev/midiout.cpp",
	MAME_DIR .. "src/devices/imagedev/midiout.h",
	MAME_DIR .. "src/devices/imagedev/papertape.cpp",
	MAME_DIR .. "src/devices/imagedev/papertape.h",
	MAME_DIR .. "src/devices/imagedev/picture.cpp",
	MAME_DIR .. "src/devices/imagedev/picture.h",
	MAME_DIR .. "src/devices/imagedev/printer.cpp",
	MAME_DIR .. "src/devices/imagedev/printer.h",
	MAME_DIR .. "src/devices/imagedev/simh_tape_image.cpp",
	MAME_DIR .. "src/devices/imagedev/simh_tape_image.h",
	MAME_DIR .. "src/devices/imagedev/snapquik.cpp",
	MAME_DIR .. "src/devices/imagedev/snapquik.h",
	MAME_DIR .. "src/devices/imagedev/wafadrive.cpp",
	MAME_DIR .. "src/devices/imagedev/wafadrive.h",
	MAME_DIR .. "src/devices/imagedev/avivideo.cpp",
	MAME_DIR .. "src/devices/imagedev/avivideo.h",
}


---------------------------------------------------
--
--@src/devices/machine/acorn_bmu.h,MACHINES["ACORN_BMU"] = true
---------------------------------------------------

if (MACHINES["ACORN_BMU"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/acorn_bmu.cpp",
		MAME_DIR .. "src/devices/machine/acorn_bmu.h",
	}
end

--------------------------------------------------
--
--@src/devices/machine/acorn_ioc.h,MACHINES["ACORN_IOC"] = true
--------------------------------------------------

if (MACHINES["ACORN_IOC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/acorn_ioc.cpp",
		MAME_DIR .. "src/devices/machine/acorn_ioc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/acorn_lc.h,MACHINES["ACORN_LC"] = true
---------------------------------------------------

if (MACHINES["ACORN_LC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/acorn_lc.cpp",
		MAME_DIR .. "src/devices/machine/acorn_lc.h",
	}
end

--------------------------------------------------
--
--@src/devices/machine/acorn_memc.h,MACHINES["ACORN_MEMC"] = true
--------------------------------------------------

if (MACHINES["ACORN_MEMC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/acorn_memc.cpp",
		MAME_DIR .. "src/devices/machine/acorn_memc.h",
	}
end

--------------------------------------------------
--
--@src/devices/machine/acorn_vidc.h,MACHINES["ACORN_VIDC"] = true
--------------------------------------------------

if (MACHINES["ACORN_VIDC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/acorn_vidc.cpp",
		MAME_DIR .. "src/devices/machine/acorn_vidc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/am2901b.h,MACHINES["AM2901B"] = true
---------------------------------------------------

if (MACHINES["AM2901B"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/am2901b.cpp",
		MAME_DIR .. "src/devices/machine/am2901b.h",
	}
end

--------------------------------------------------
--
--@src/devices/machine/arm_iomd.h,MACHINES["ARM_IOMD"] = true
--------------------------------------------------

if (MACHINES["ARM_IOMD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/arm_iomd.cpp",
		MAME_DIR .. "src/devices/machine/arm_iomd.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/autoconfig.h,MACHINES["AUTOCONFIG"] = true
---------------------------------------------------

if (MACHINES["AUTOCONFIG"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/autoconfig.cpp",
		MAME_DIR .. "src/devices/machine/autoconfig.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/cop452.h,MACHINES["COP452"] = true
---------------------------------------------------

if (MACHINES["COP452"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/cop452.cpp",
		MAME_DIR .. "src/devices/machine/cop452.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/cr511b.h,MACHINES["CR511B"] = true
---------------------------------------------------

if (MACHINES["CR511B"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/cr511b.cpp",
		MAME_DIR .. "src/devices/machine/cr511b.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/dmac.h,MACHINES["DMAC"] = true
---------------------------------------------------

if (MACHINES["DMAC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/dmac.cpp",
		MAME_DIR .. "src/devices/machine/dmac.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/40105.h,MACHINES["CMOS40105"] = true
---------------------------------------------------

if (MACHINES["CMOS40105"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/40105.cpp",
		MAME_DIR .. "src/devices/machine/40105.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/53c7xx.h,MACHINES["NCR53C7XX"] = true
---------------------------------------------------

if (MACHINES["NCR53C7XX"]~=null) then
	MACHINES["NSCSI"] = true
	files {
		MAME_DIR .. "src/devices/machine/53c7xx.cpp",
		MAME_DIR .. "src/devices/machine/53c7xx.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ncr5385.h,MACHINES["NCR5385"] = true
---------------------------------------------------

if (MACHINES["NCR5385"]~=null) then
	MACHINES["NSCSI"] = true
	files {
		MAME_DIR .. "src/devices/machine/ncr5385.cpp",
		MAME_DIR .. "src/devices/machine/ncr5385.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/53c810.h,MACHINES["LSI53C810"] = true
---------------------------------------------------

if (MACHINES["LSI53C810"]~=null) then
	MACHINES["SCSI"] = true
	files {
		MAME_DIR .. "src/devices/machine/53c810.cpp",
		MAME_DIR .. "src/devices/machine/53c810.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/2812fifo.h,MACHINES["2812FIFO"] = true
---------------------------------------------------

if (MACHINES["2812FIFO"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/2812fifo.cpp",
		MAME_DIR .. "src/devices/machine/2812fifo.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/6522via.h,MACHINES["6522VIA"] = true
---------------------------------------------------

if (MACHINES["6522VIA"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/6522via.cpp",
		MAME_DIR .. "src/devices/machine/6522via.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/6525tpi.h,MACHINES["TPI6525"] = true
---------------------------------------------------

if (MACHINES["TPI6525"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/6525tpi.cpp",
		MAME_DIR .. "src/devices/machine/6525tpi.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/6821pia.h,MACHINES["6821PIA"] = true
---------------------------------------------------

if (MACHINES["6821PIA"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/6821pia.cpp",
		MAME_DIR .. "src/devices/machine/6821pia.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/6840ptm.h,MACHINES["6840PTM"] = true
---------------------------------------------------

if (MACHINES["6840PTM"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/6840ptm.cpp",
		MAME_DIR .. "src/devices/machine/6840ptm.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/6850acia.h,MACHINES["ACIA6850"] = true
---------------------------------------------------

if (MACHINES["ACIA6850"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/6850acia.cpp",
		MAME_DIR .. "src/devices/machine/6850acia.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/6883sam.h,MACHINES["6883SAM"] = true
---------------------------------------------------

if (MACHINES["6883SAM"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/6883sam.cpp",
		MAME_DIR .. "src/devices/machine/6883sam.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/68153bim.h,MACHINES["BIM68153"] = true
---------------------------------------------------

if (MACHINES["BIM68153"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/68153bim.cpp",
		MAME_DIR .. "src/devices/machine/68153bim.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/68230pit.h,MACHINES["PIT68230"] = true
---------------------------------------------------

if (MACHINES["PIT68230"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/68230pit.cpp",
		MAME_DIR .. "src/devices/machine/68230pit.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/68561mpcc.h,MACHINES["MPCC68561"] = true
---------------------------------------------------

if (MACHINES["MPCC68561"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/68561mpcc.cpp",
		MAME_DIR .. "src/devices/machine/68561mpcc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mc68681.h,MACHINES["68681"] = true
---------------------------------------------------

if (MACHINES["68681"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mc68681.cpp",
		MAME_DIR .. "src/devices/machine/mc68681.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/7200fifo.h,MACHINES["7200FIFO"] = true
---------------------------------------------------

if (MACHINES["7200FIFO"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/7200fifo.cpp",
		MAME_DIR .. "src/devices/machine/7200fifo.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/7404.h,MACHINES["TTL7404"] = true
---------------------------------------------------

if (MACHINES["TTL7404"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/7404.cpp",
		MAME_DIR .. "src/devices/machine/7404.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74123.h,MACHINES["TTL74123"] = true
---------------------------------------------------

if (MACHINES["TTL74123"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74123.cpp",
		MAME_DIR .. "src/devices/machine/74123.h",
		MAME_DIR .. "src/devices/machine/rescap.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74145.h,MACHINES["TTL74145"] = true
---------------------------------------------------

if (MACHINES["TTL74145"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74145.cpp",
		MAME_DIR .. "src/devices/machine/74145.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74148.h,MACHINES["TTL74148"] = true
---------------------------------------------------

if (MACHINES["TTL74148"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74148.cpp",
		MAME_DIR .. "src/devices/machine/74148.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74153.h,MACHINES["TTL74153"] = true
---------------------------------------------------

if (MACHINES["TTL74153"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74153.cpp",
		MAME_DIR .. "src/devices/machine/74153.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74157.h,MACHINES["TTL74157"] = true
---------------------------------------------------

if (MACHINES["TTL74157"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74157.cpp",
		MAME_DIR .. "src/devices/machine/74157.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74161.h,MACHINES["TTL74161"] = true
---------------------------------------------------

if (MACHINES["TTL74161"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74161.cpp",
		MAME_DIR .. "src/devices/machine/74161.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74165.h,MACHINES["TTL74165"] = true
---------------------------------------------------

if (MACHINES["TTL74165"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74165.cpp",
		MAME_DIR .. "src/devices/machine/74165.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74166.h,MACHINES["TTL74166"] = true
---------------------------------------------------

if (MACHINES["TTL74166"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74166.cpp",
		MAME_DIR .. "src/devices/machine/74166.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74175.h,MACHINES["TTL74175"] = true
---------------------------------------------------

if (MACHINES["TTL74175"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74175.cpp",
		MAME_DIR .. "src/devices/machine/74175.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74181.h,MACHINES["TTL74181"] = true
---------------------------------------------------

if (MACHINES["TTL74181"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74181.cpp",
		MAME_DIR .. "src/devices/machine/74181.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74259.h,MACHINES["TTL74259"] = true
---------------------------------------------------

if (MACHINES["TTL74259"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74259.cpp",
		MAME_DIR .. "src/devices/machine/74259.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74381.h,MACHINES["TTL74381"] = true
---------------------------------------------------

if (MACHINES["TTL74381"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74381.cpp",
		MAME_DIR .. "src/devices/machine/74381.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/74543.h,MACHINES["TTL74543"] = true
---------------------------------------------------

if (MACHINES["TTL74543"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/74543.cpp",
		MAME_DIR .. "src/devices/machine/74543.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/7474.h,MACHINES["TTL7474"] = true
---------------------------------------------------

if (MACHINES["TTL7474"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/7474.cpp",
		MAME_DIR .. "src/devices/machine/7474.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/82s129.h,MACHINES["PROM82S129"] = true
---------------------------------------------------

if (MACHINES["PROM82S129"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/82s129.cpp",
		MAME_DIR .. "src/devices/machine/82s129.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/8042kbdc.h,MACHINES["KBDC8042"] = true
---------------------------------------------------

if (MACHINES["KBDC8042"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/8042kbdc.cpp",
		MAME_DIR .. "src/devices/machine/8042kbdc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/adc0804.h,MACHINES["ADC0804"] = true
---------------------------------------------------

if (MACHINES["ADC0804"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/adc0804.cpp",
		MAME_DIR .. "src/devices/machine/adc0804.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/adc0808.h,MACHINES["ADC0808"] = true
---------------------------------------------------

if (MACHINES["ADC0808"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/adc0808.cpp",
		MAME_DIR .. "src/devices/machine/adc0808.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/adc083x.h,MACHINES["ADC083X"] = true
---------------------------------------------------

if (MACHINES["ADC083X"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/adc083x.cpp",
		MAME_DIR .. "src/devices/machine/adc083x.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/adc1038.h,MACHINES["ADC1038"] = true
---------------------------------------------------

if (MACHINES["ADC1038"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/adc1038.cpp",
		MAME_DIR .. "src/devices/machine/adc1038.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/adc1213x.h,MACHINES["ADC1213X"] = true
---------------------------------------------------

if (MACHINES["ADC1213X"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/adc1213x.cpp",
		MAME_DIR .. "src/devices/machine/adc1213x.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/aicartc.h,MACHINES["AICARTC"] = true
---------------------------------------------------

if (MACHINES["AICARTC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/aicartc.cpp",
		MAME_DIR .. "src/devices/machine/aicartc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/am25s55x.h,MACHINES["AM25S55X"] = true
---------------------------------------------------

if (MACHINES["AM25S55X"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/am25s55x.cpp",
		MAME_DIR .. "src/devices/machine/am25s55x.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/am2847.h,MACHINES["AM2847"] = true
---------------------------------------------------

if (MACHINES["AM2847"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/am2847.cpp",
		MAME_DIR .. "src/devices/machine/am2847.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/am2910.h,MACHINES["AM2910"] = true
---------------------------------------------------

if (MACHINES["AM2910"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/am2910.cpp",
		MAME_DIR .. "src/devices/machine/am2910.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/am79c30.h,MACHINES["AM79C30"] = true
---------------------------------------------------

if (MACHINES["AM79C30"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/am79c30.cpp",
		MAME_DIR .. "src/devices/machine/am79c30.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/am79c90.h,MACHINES["AM79C90"] = true
---------------------------------------------------

if (MACHINES["AM79C90"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/am79c90.cpp",
		MAME_DIR .. "src/devices/machine/am79c90.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/am9513.h,MACHINES["AM9513"] = true
---------------------------------------------------

if (MACHINES["AM9513"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/am9513.cpp",
		MAME_DIR .. "src/devices/machine/am9513.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/am9517a.h,MACHINES["AM9517A"] = true
---------------------------------------------------

if (MACHINES["AM9517A"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/am9517a.cpp",
		MAME_DIR .. "src/devices/machine/am9517a.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/am9519.h,MACHINES["AM9519"] = true
---------------------------------------------------

if (MACHINES["AM9519"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/am9519.cpp",
		MAME_DIR .. "src/devices/machine/am9519.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/applepic.h,MACHINES["APPLEPIC"] = true
---------------------------------------------------

if (MACHINES["APPLEPIC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/applepic.cpp",
		MAME_DIR .. "src/devices/machine/applepic.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/at28c16.h,MACHINES["AT28C16"] = true
---------------------------------------------------

if (MACHINES["AT28C16"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/at28c16.cpp",
		MAME_DIR .. "src/devices/machine/at28c16.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/at28c64b.h,MACHINES["AT28C64B"] = true
---------------------------------------------------

if (MACHINES["AT28C64B"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/at28c64b.cpp",
		MAME_DIR .. "src/devices/machine/at28c64b.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/at29x.h,MACHINES["AT29X"] = true
---------------------------------------------------

if (MACHINES["AT29X"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/at29x.cpp",
		MAME_DIR .. "src/devices/machine/at29x.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/at45dbxx.h,MACHINES["AT45DBXX"] = true
---------------------------------------------------

if (MACHINES["AT45DBXX"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/at45dbxx.cpp",
		MAME_DIR .. "src/devices/machine/at45dbxx.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/atahle.h,MACHINES["ATAHLE"] = true
---------------------------------------------------

if (MACHINES["ATAHLE"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/atahle.cpp",
		MAME_DIR .. "src/devices/machine/atahle.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/atastorage.h,MACHINES["ATASTORAGE"] = true
---------------------------------------------------

if (MACHINES["ATASTORAGE"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/atastorage.cpp",
		MAME_DIR .. "src/devices/machine/atastorage.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/atmel_arm_aic.h,MACHINES["ARM_AIC"] = true
---------------------------------------------------

if (MACHINES["ARM_AIC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/atmel_arm_aic.cpp",
		MAME_DIR .. "src/devices/machine/atmel_arm_aic.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ay31015.h,MACHINES["AY31015"] = true
---------------------------------------------------

if (MACHINES["AY31015"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ay31015.cpp",
		MAME_DIR .. "src/devices/machine/ay31015.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/bankdev.h,MACHINES["BANKDEV"] = true
---------------------------------------------------

if (MACHINES["BANKDEV"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/bankdev.cpp",
		MAME_DIR .. "src/devices/machine/bankdev.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/bq4847.h,MACHINES["BQ4847"] = true
---------------------------------------------------

if (MACHINES["BQ4847"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/bq4847.cpp",
		MAME_DIR .. "src/devices/machine/bq4847.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/bq48x2.h,MACHINES["BQ4852"] = true
---------------------------------------------------

if (MACHINES["BQ4852"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/bq48x2.cpp",
		MAME_DIR .. "src/devices/machine/bq48x2.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/busmouse.h,MACHINES["BUSMOUSE"] = true
---------------------------------------------------

if (MACHINES["BUSMOUSE"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/busmouse.cpp",
		MAME_DIR .. "src/devices/machine/busmouse.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/cdp1852.h,MACHINES["CDP1852"] = true
---------------------------------------------------

if (MACHINES["CDP1852"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/cdp1852.cpp",
		MAME_DIR .. "src/devices/machine/cdp1852.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/cdp1871.h,MACHINES["CDP1871"] = true
---------------------------------------------------

if (MACHINES["CDP1871"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/cdp1871.cpp",
		MAME_DIR .. "src/devices/machine/cdp1871.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/cdp1879.h,MACHINES["CDP1879"] = true
---------------------------------------------------

if (MACHINES["CDP1879"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/cdp1879.cpp",
		MAME_DIR .. "src/devices/machine/cdp1879.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ch376.h,MACHINES["CH376"] = true
---------------------------------------------------

if (MACHINES["CH376"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ch376.cpp",
		MAME_DIR .. "src/devices/machine/ch376.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/chessmachine.h,MACHINES["CHESSMACHINE"] = true
---------------------------------------------------

if (MACHINES["CHESSMACHINE"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/chessmachine.cpp",
		MAME_DIR .. "src/devices/machine/chessmachine.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/com52c50.h,MACHINES["COM52C50"] = true
---------------------------------------------------

if (MACHINES["COM52C50"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/com52c50.cpp",
		MAME_DIR .. "src/devices/machine/com52c50.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/com8116.h,MACHINES["COM8116"] = true
---------------------------------------------------

if (MACHINES["COM8116"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/com8116.cpp",
		MAME_DIR .. "src/devices/machine/com8116.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/cs4031.h,MACHINES["CS4031"] = true
---------------------------------------------------

if (MACHINES["CS4031"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/cs4031.cpp",
		MAME_DIR .. "src/devices/machine/cs4031.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/cs8221.h,MACHINES["CS8221"] = true
---------------------------------------------------

if (MACHINES["CS8221"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/cs8221.cpp",
		MAME_DIR .. "src/devices/machine/cs8221.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/cs8900a.h,MACHINES["CS8900A"] = true
---------------------------------------------------

if (MACHINES["CS8900A"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/cs8900a.cpp",
		MAME_DIR .. "src/devices/machine/cs8900a.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/cxd1095.h,MACHINES["CXD1095"] = true
---------------------------------------------------

if (MACHINES["CXD1095"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/cxd1095.cpp",
		MAME_DIR .. "src/devices/machine/cxd1095.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/dimm_spd.h,MACHINES["DIMM_SPD"] = true
---------------------------------------------------

if (MACHINES["DIMM_SPD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/dimm_spd.cpp",
		MAME_DIR .. "src/devices/machine/dimm_spd.h",
	}
end

--@src/devices/machine/dl11.h,MACHINES["DL11"] = true
---------------------------------------------------

if (MACHINES["DL11"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/dl11.cpp",
		MAME_DIR .. "src/devices/machine/dl11.h",
	}
end

--@src/devices/machine/ds1204.h,MACHINES["DS1204"] = true
---------------------------------------------------

if (MACHINES["DS1204"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds1204.cpp",
		MAME_DIR .. "src/devices/machine/ds1204.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds1205.h,MACHINES["DS1205"] = true
---------------------------------------------------

if (MACHINES["DS1205"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds1205.cpp",
		MAME_DIR .. "src/devices/machine/ds1205.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds1207.h,MACHINES["DS1207"] = true
---------------------------------------------------

if (MACHINES["DS1207"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds1207.cpp",
		MAME_DIR .. "src/devices/machine/ds1207.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds1302.h,MACHINES["DS1302"] = true
---------------------------------------------------

if (MACHINES["DS1302"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds1302.cpp",
		MAME_DIR .. "src/devices/machine/ds1302.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds1215.h,MACHINES["DS1215"] = true
---------------------------------------------------

if (MACHINES["DS1215"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds1215.cpp",
		MAME_DIR .. "src/devices/machine/ds1215.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds1386.h,MACHINES["DS1386"] = true
---------------------------------------------------

if (MACHINES["DS1386"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds1386.cpp",
		MAME_DIR .. "src/devices/machine/ds1386.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds17x85.h,MACHINES["DS17X85"] = true
---------------------------------------------------

if (MACHINES["DS17X85"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds17x85.cpp",
		MAME_DIR .. "src/devices/machine/ds17x85.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds1994.h,MACHINES["DS1994"] = true
---------------------------------------------------

if (MACHINES["DS1994"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds1994.cpp",
		MAME_DIR .. "src/devices/machine/ds1994.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds2401.h,MACHINES["DS2401"] = true
---------------------------------------------------

if (MACHINES["DS2401"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds2401.cpp",
		MAME_DIR .. "src/devices/machine/ds2401.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds2404.h,MACHINES["DS2404"] = true
---------------------------------------------------

if (MACHINES["DS2404"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds2404.cpp",
		MAME_DIR .. "src/devices/machine/ds2404.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds2430a.h,MACHINES["DS2430A"] = true
---------------------------------------------------

if (MACHINES["DS2430A"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds2430a.cpp",
		MAME_DIR .. "src/devices/machine/ds2430a.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds6417.h,MACHINES["DS6417"] = true
---------------------------------------------------

if (MACHINES["DS6417"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds6417.cpp",
		MAME_DIR .. "src/devices/machine/ds6417.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds75160a.h,MACHINES["DS75160A"] = true
---------------------------------------------------

if (MACHINES["DS75160A"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds75160a.cpp",
		MAME_DIR .. "src/devices/machine/ds75160a.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ds75161a.h,MACHINES["DS75161A"] = true
---------------------------------------------------

if (MACHINES["DS75161A"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds75161a.cpp",
		MAME_DIR .. "src/devices/machine/ds75161a.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/e0516.h,MACHINES["E0516"] = true
---------------------------------------------------

if (MACHINES["E0516"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/e0516.cpp",
		MAME_DIR .. "src/devices/machine/e0516.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/e05a03.h,MACHINES["E05A03"] = true
---------------------------------------------------

if (MACHINES["E05A03"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/e05a03.cpp",
		MAME_DIR .. "src/devices/machine/e05a03.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/e05a30.h,MACHINES["E05A30"] = true
---------------------------------------------------

if (MACHINES["E05A30"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/e05a30.cpp",
		MAME_DIR .. "src/devices/machine/e05a30.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/eeprom.h,MACHINES["EEPROMDEV"] = true
--@src/devices/machine/eepromser.h,MACHINES["EEPROMDEV"] = true
--@src/devices/machine/eeprompar.h,MACHINES["EEPROMDEV"] = true
---------------------------------------------------

if (MACHINES["EEPROMDEV"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/eeprom.cpp",
		MAME_DIR .. "src/devices/machine/eeprom.h",
		MAME_DIR .. "src/devices/machine/eepromser.cpp",
		MAME_DIR .. "src/devices/machine/eepromser.h",
		MAME_DIR .. "src/devices/machine/eeprompar.cpp",
		MAME_DIR .. "src/devices/machine/eeprompar.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/er1400.h,MACHINES["ER1400"] = true
---------------------------------------------------

if (MACHINES["ER1400"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/er1400.cpp",
		MAME_DIR .. "src/devices/machine/er1400.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/er2055.h,MACHINES["ER2055"] = true
---------------------------------------------------

if (MACHINES["ER2055"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/er2055.cpp",
		MAME_DIR .. "src/devices/machine/er2055.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/exorterm.h,MACHINES["EXORTERM"] = true
---------------------------------------------------

if (MACHINES["EXORTERM"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/exorterm.cpp",
		MAME_DIR .. "src/devices/machine/exorterm.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/machine/exorterm.cpp", GEN_DIR .. "emu/layout/exorterm155.lh" },
	}

	custombuildtask {
		layoutbuildtask("emu/layout", "exorterm155"),
	}
end

---------------------------------------------------
--
--@src/devices/machine/f3853.h,MACHINES["F3853"] = true
---------------------------------------------------

if (MACHINES["F3853"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/f3853.cpp",
		MAME_DIR .. "src/devices/machine/f3853.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/f4702.h,MACHINES["F4702"] = true
---------------------------------------------------

if (MACHINES["F4702"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/f4702.cpp",
		MAME_DIR .. "src/devices/machine/f4702.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/fga002.h,MACHINES["FGA002"] = true
---------------------------------------------------

if (MACHINES["FGA002"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/fga002.cpp",
		MAME_DIR .. "src/devices/machine/fga002.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/gt913_io.h,MACHINES["GT913"] = true
--@src/devices/machine/gt913_kbd.h,MACHINES["GT913"] = true
--@src/devices/machine/gt913_snd.h,MACHINES["GT913"] = true
---------------------------------------------------

if (MACHINES["GT913"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/gt913_io.cpp",
		MAME_DIR .. "src/devices/machine/gt913_io.h",
		MAME_DIR .. "src/devices/machine/gt913_kbd.cpp",
		MAME_DIR .. "src/devices/machine/gt913_kbd.h",
		MAME_DIR .. "src/devices/machine/gt913_snd.cpp",
		MAME_DIR .. "src/devices/machine/gt913_snd.h",
	}
end

--------------------------------------------------
--
--@src/devices/machine/generic_spi_flash.h,MACHINES["GENERIC_SPI_FLASH"] = true
--------------------------------------------------

if (MACHINES["GENERIC_SPI_FLASH"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/generic_spi_flash.cpp",
		MAME_DIR .. "src/devices/machine/generic_spi_flash.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/hd63450.h,MACHINES["HD63450"] = true
---------------------------------------------------

if (MACHINES["HD63450"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/hd63450.cpp",
		MAME_DIR .. "src/devices/machine/hd63450.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/hd64610.h,MACHINES["HD64610"] = true
---------------------------------------------------

if (MACHINES["HD64610"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/hd64610.cpp",
		MAME_DIR .. "src/devices/machine/hd64610.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/hp_dc100_tape.h,MACHINES["HP_DC100_TAPE"] = true
---------------------------------------------------

if (MACHINES["HP_DC100_TAPE"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/hp_dc100_tape.cpp",
		MAME_DIR .. "src/devices/machine/hp_dc100_tape.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/hp_taco.h,MACHINES["HP_TACO"] = true
---------------------------------------------------

if (MACHINES["HP_TACO"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/hp_taco.cpp",
		MAME_DIR .. "src/devices/machine/hp_taco.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/1ma6.h,MACHINES["1MA6"] = true
---------------------------------------------------

if (MACHINES["1MA6"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/1ma6.cpp",
		MAME_DIR .. "src/devices/machine/1ma6.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/1mb5.h,MACHINES["1MB5"] = true
---------------------------------------------------

if (MACHINES["1MB5"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/1mb5.cpp",
		MAME_DIR .. "src/devices/machine/1mb5.h",
	}
end

---------------------------------------------------
--@src/devices/machine/i2chle.h,MACHINES["I2CHLE"] = true
---------------------------------------------------

if (MACHINES["I2CHLE"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i2chle.cpp",
		MAME_DIR .. "src/devices/machine/i2chle.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i2cmem.h,MACHINES["I2CMEM"] = true
---------------------------------------------------

if (MACHINES["I2CMEM"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i2cmem.cpp",
		MAME_DIR .. "src/devices/machine/i2cmem.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i7220.h,MACHINES["I7220"] = true
---------------------------------------------------

if (MACHINES["I7220"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i7220.cpp",
		MAME_DIR .. "src/devices/machine/i7220.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i8087.h,MACHINES["I8087"] = true
---------------------------------------------------

if (MACHINES["I8087"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i8087.cpp",
		MAME_DIR .. "src/devices/machine/i8087.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i8155.h,MACHINES["I8155"] = true
---------------------------------------------------

if (MACHINES["I8155"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i8155.cpp",
		MAME_DIR .. "src/devices/machine/i8155.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i8212.h,MACHINES["I8212"] = true
---------------------------------------------------

if (MACHINES["I8212"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i8212.cpp",
		MAME_DIR .. "src/devices/machine/i8212.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i8214.h,MACHINES["I8214"] = true
---------------------------------------------------

if (MACHINES["I8214"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i8214.cpp",
		MAME_DIR .. "src/devices/machine/i8214.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i82355.h,MACHINES["I82355"] = true
---------------------------------------------------

if (MACHINES["I82355"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i82355.cpp",
		MAME_DIR .. "src/devices/machine/i82355.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i8243.h,MACHINES["I8243"] = true
---------------------------------------------------

if (MACHINES["I8243"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i8243.cpp",
		MAME_DIR .. "src/devices/machine/i8243.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i8251.h,MACHINES["I8251"] = true
---------------------------------------------------

if (MACHINES["I8251"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i8251.cpp",
		MAME_DIR .. "src/devices/machine/i8251.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i8257.h,MACHINES["I8257"] = true
---------------------------------------------------

if (MACHINES["I8257"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i8257.cpp",
		MAME_DIR .. "src/devices/machine/i8257.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/i8271.h,MACHINES["I8271"] = true
---------------------------------------------------

if (MACHINES["I8271"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i8271.cpp",
		MAME_DIR .. "src/devices/machine/i8271.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i8279.h,MACHINES["I8279"] = true
---------------------------------------------------

if (MACHINES["I8279"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i8279.cpp",
		MAME_DIR .. "src/devices/machine/i8279.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i8355.h,MACHINES["I8355"] = true
---------------------------------------------------

if (MACHINES["I8355"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i8355.cpp",
		MAME_DIR .. "src/devices/machine/i8355.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i80130.h,MACHINES["I80130"] = true
---------------------------------------------------

if (MACHINES["I80130"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i80130.cpp",
		MAME_DIR .. "src/devices/machine/i80130.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ibm21s850.h,MACHINES["IBM21S850"] = true
---------------------------------------------------

if (MACHINES["IBM21S850"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ibm21s850.cpp",
		MAME_DIR .. "src/devices/machine/ibm21s850.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/icd2061a.h,MACHINES["ICD2061A"] = true
---------------------------------------------------

if (MACHINES["ICD2061A"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/icd2061a.cpp",
		MAME_DIR .. "src/devices/machine/icd2061a.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/icm7170.h,MACHINES["ICM7170"] = true
---------------------------------------------------

if (MACHINES["ICM7170"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/icm7170.cpp",
		MAME_DIR .. "src/devices/machine/icm7170.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/idectrl.h,MACHINES["IDECTRL"] = true
--@src/devices/machine/vt83c461.h,MACHINES["IDECTRL"] = true
---------------------------------------------------

if (MACHINES["IDECTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/idectrl.cpp",
		MAME_DIR .. "src/devices/machine/idectrl.h",
		MAME_DIR .. "src/devices/machine/vt83c461.cpp",
		MAME_DIR .. "src/devices/machine/vt83c461.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ie15.h,MACHINES["IE15"] = true
---------------------------------------------------

if (MACHINES["IE15"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ie15.cpp",
		MAME_DIR .. "src/devices/machine/ie15.h",
		MAME_DIR .. "src/devices/machine/ie15_kbd.cpp",
		MAME_DIR .. "src/devices/machine/ie15_kbd.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/machine/ie15.cpp", GEN_DIR .. "emu/layout/ie15.lh" },
	}

	custombuildtask {
		layoutbuildtask("emu/layout", "ie15"),
	}
end

---------------------------------------------------
--
--@src/devices/machine/im6402.h,MACHINES["IM6402"] = true
---------------------------------------------------

if (MACHINES["IM6402"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/im6402.cpp",
		MAME_DIR .. "src/devices/machine/im6402.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ins8154.h,MACHINES["INS8154"] = true
---------------------------------------------------

if (MACHINES["INS8154"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ins8154.cpp",
		MAME_DIR .. "src/devices/machine/ins8154.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ins8250.h,MACHINES["INS8250"] = true
---------------------------------------------------

if (MACHINES["INS8250"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ins8250.cpp",
		MAME_DIR .. "src/devices/machine/ins8250.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/intelfsh.h,MACHINES["INTELFLASH"] = true
---------------------------------------------------

if (MACHINES["INTELFLASH"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/intelfsh.cpp",
		MAME_DIR .. "src/devices/machine/intelfsh.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/jvsdev.h,MACHINES["JVS"] = true
--@src/devices/machine/jvshost.h,MACHINES["JVS"] = true
---------------------------------------------------

if (MACHINES["JVS"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/jvsdev.cpp",
		MAME_DIR .. "src/devices/machine/jvsdev.h",
		MAME_DIR .. "src/devices/machine/jvshost.cpp",
		MAME_DIR .. "src/devices/machine/jvshost.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/k033906.h,MACHINES["K033906"] = true
---------------------------------------------------

if (MACHINES["K033906"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/k033906.cpp",
		MAME_DIR .. "src/devices/machine/k033906.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/k053252.h,MACHINES["K053252"] = true
---------------------------------------------------

if (MACHINES["K053252"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/k053252.cpp",
		MAME_DIR .. "src/devices/machine/k053252.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/k056230.h,MACHINES["K056230"] = true
---------------------------------------------------

if (MACHINES["K056230"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/k056230.cpp",
		MAME_DIR .. "src/devices/machine/k056230.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/1801vp128.h,MACHINES["K1801VP128"] = true
---------------------------------------------------

if (MACHINES["K1801VP128"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/1801vp128.cpp",
		MAME_DIR .. "src/devices/machine/1801vp128.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/kb3600.h,MACHINES["KB3600"] = true
---------------------------------------------------

if (MACHINES["KB3600"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/kb3600.cpp",
		MAME_DIR .. "src/devices/machine/kb3600.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/keytronic_l2207.h,MACHINES["KEYTRONIC_L2207"] = true
---------------------------------------------------

if (MACHINES["KEYTRONIC_L2207"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/keytronic_l2207.cpp",
		MAME_DIR .. "src/devices/machine/keytronic_l2207.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/kr1601rr1.h,MACHINES["KR1601RR1"] = true
---------------------------------------------------

if (MACHINES["KR1601RR1"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/kr1601rr1.cpp",
		MAME_DIR .. "src/devices/machine/kr1601rr1.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/kr2376.h,MACHINES["KR2376"] = true
---------------------------------------------------

if (MACHINES["KR2376"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/kr2376.cpp",
		MAME_DIR .. "src/devices/machine/kr2376.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/latch8.h,MACHINES["LATCH8"] = true
---------------------------------------------------

if (MACHINES["LATCH8"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/latch8.cpp",
		MAME_DIR .. "src/devices/machine/latch8.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/lc89510.h,MACHINES["LC89510"] = true
---------------------------------------------------

if (MACHINES["LC89510"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/lc89510.cpp",
		MAME_DIR .. "src/devices/machine/lc89510.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ldpr8210.h,MACHINES["LDPR8210"] = true
---------------------------------------------------

if (MACHINES["LDPR8210"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ldpr8210.cpp",
		MAME_DIR .. "src/devices/machine/ldpr8210.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ldstub.h,MACHINES["LDSTUB"] = true
---------------------------------------------------

if (MACHINES["LDSTUB"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ldstub.cpp",
		MAME_DIR .. "src/devices/machine/ldstub.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ldv1000.h,MACHINES["LDV1000"] = true
---------------------------------------------------

if (MACHINES["LDV1000"]~=null) then
	MACHINES["Z80CTC"] = true
	MACHINES["I8255"] = true
	files {
		MAME_DIR .. "src/devices/machine/ldv1000.cpp",
		MAME_DIR .. "src/devices/machine/ldv1000.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ldv1000hle.h,MACHINES["LDV1000HLE"] = true
---------------------------------------------------

if (MACHINES["LDV1000HLE"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ldv1000hle.cpp",
		MAME_DIR .. "src/devices/machine/ldv1000hle.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ldv4200hle.h,MACHINES["LDV4200HLE"] = true
---------------------------------------------------

if (MACHINES["LDV4200HLE"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ldv4200hle.cpp",
		MAME_DIR .. "src/devices/machine/ldv4200hle.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ldp1000.h,MACHINES["LDP1000"] = true
---------------------------------------------------

if (MACHINES["LDP1000"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ldp1000.cpp",
		MAME_DIR .. "src/devices/machine/ldp1000.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ldp1450.h,MACHINES["LDP1450"] = true
---------------------------------------------------

if (MACHINES["LDP1450"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ldp1450.cpp",
		MAME_DIR .. "src/devices/machine/ldp1450.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ldp1450hle.h,MACHINES["LDP1450HLE"] = true
---------------------------------------------------

if (MACHINES["LDP1450HLE"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ldp1450hle.cpp",
		MAME_DIR .. "src/devices/machine/ldp1450hle.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ldvp931.h,MACHINES["LDVP931"] = true
---------------------------------------------------

if (MACHINES["LDVP931"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ldvp931.cpp",
		MAME_DIR .. "src/devices/machine/ldvp931.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/lh5810.h,MACHINES["LH5810"] = true
---------------------------------------------------

if (MACHINES["LH5810"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/lh5810.cpp",
		MAME_DIR .. "src/devices/machine/lh5810.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/locomo.h,MACHINES["LOCOMO"] = true
---------------------------------------------------

if (MACHINES["LOCOMO"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/locomo.cpp",
		MAME_DIR .. "src/devices/machine/locomo.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/m3002.h,MACHINES["M3002"] = true
---------------------------------------------------

if (MACHINES["M3002"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/m3002.cpp",
		MAME_DIR .. "src/devices/machine/m3002.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/m68sfdc.h,MACHINES["M68SFDC"] = true
---------------------------------------------------

if (MACHINES["M68SFDC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/m68sfdc.cpp",
		MAME_DIR .. "src/devices/machine/m68sfdc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/m6m80011ap.h,MACHINES["M6M80011AP"] = true
---------------------------------------------------

if (MACHINES["M6M80011AP"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/m6m80011ap.cpp",
		MAME_DIR .. "src/devices/machine/m6m80011ap.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/m950x0.h,MACHINES["M950X0"] = true
---------------------------------------------------

if (MACHINES["M950X0"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/m950x0.cpp",
		MAME_DIR .. "src/devices/machine/m950x0.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mb14241.h,MACHINES["MB14241"] = true
---------------------------------------------------

if (MACHINES["MB14241"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mb14241.cpp",
		MAME_DIR .. "src/devices/machine/mb14241.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mb3773.h,MACHINES["MB3773"] = true
---------------------------------------------------

if (MACHINES["MB3773"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mb3773.cpp",
		MAME_DIR .. "src/devices/machine/mb3773.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mb8421.h,MACHINES["MB8421"] = true
---------------------------------------------------

if (MACHINES["MB8421"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mb8421.cpp",
		MAME_DIR .. "src/devices/machine/mb8421.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mb87030.h,MACHINES["MB87030"] = true
---------------------------------------------------

if (MACHINES["MB87030"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mb87030.cpp",
		MAME_DIR .. "src/devices/machine/mb87030.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mb87078.h,MACHINES["MB87078"] = true
---------------------------------------------------

if (MACHINES["MB87078"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mb87078.cpp",
		MAME_DIR .. "src/devices/machine/mb87078.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mb8795.h,MACHINES["MB8795"] = true
---------------------------------------------------

if (MACHINES["MB8795"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mb8795.cpp",
		MAME_DIR .. "src/devices/machine/mb8795.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mb89371.h,MACHINES["MB89371"] = true
---------------------------------------------------

if (MACHINES["MB89371"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mb89371.cpp",
		MAME_DIR .. "src/devices/machine/mb89371.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mb89374.h,MACHINES["MB89374"] = true
---------------------------------------------------

if (MACHINES["MB89374"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mb89374.cpp",
		MAME_DIR .. "src/devices/machine/mb89374.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mc146818.h,MACHINES["MC146818"] = true
---------------------------------------------------

if (MACHINES["MC146818"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mc146818.cpp",
		MAME_DIR .. "src/devices/machine/mc146818.h",
		MAME_DIR .. "src/devices/machine/ds128x.cpp",
		MAME_DIR .. "src/devices/machine/ds128x.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mc14411.h,MACHINES["MC14411"] = true
---------------------------------------------------

if (MACHINES["MC14411"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mc14411.cpp",
		MAME_DIR .. "src/devices/machine/mc14411.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mc6843.h,MACHINES["MC6843"] = true
---------------------------------------------------

if (MACHINES["MC6843"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mc6843.cpp",
		MAME_DIR .. "src/devices/machine/mc6843.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mc6844.h,MACHINES["MC6844"] = true
---------------------------------------------------

if (MACHINES["MC6844"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mc6844.cpp",
		MAME_DIR .. "src/devices/machine/mc6844.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mc6846.h,MACHINES["MC6846"] = true
---------------------------------------------------

if (MACHINES["MC6846"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mc6846.cpp",
		MAME_DIR .. "src/devices/machine/mc6846.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mc6852.h,MACHINES["MC6852"] = true
---------------------------------------------------

if (MACHINES["MC6852"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mc6852.cpp",
		MAME_DIR .. "src/devices/machine/mc6852.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mc6854.h,MACHINES["MC6854"] = true
---------------------------------------------------

if (MACHINES["MC6854"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mc6854.cpp",
		MAME_DIR .. "src/devices/machine/mc6854.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mc68328.h,MACHINES["MC68328"] = true
---------------------------------------------------

if (MACHINES["MC68328"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mc68328.cpp",
		MAME_DIR .. "src/devices/machine/mc68328.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mc68901.h,MACHINES["MC68901"] = true
---------------------------------------------------

if (MACHINES["MC68901"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mc68901.cpp",
		MAME_DIR .. "src/devices/machine/mc68901.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mccs1850.h,MACHINES["MCCS1850"] = true
---------------------------------------------------

if (MACHINES["MCCS1850"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mccs1850.cpp",
		MAME_DIR .. "src/devices/machine/mccs1850.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/68307.h,MACHINES["M68307"] = true
---------------------------------------------------

if (MACHINES["M68307"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/68307.cpp",
		MAME_DIR .. "src/devices/machine/68307.h",
		MAME_DIR .. "src/devices/machine/68307sim.cpp",
		MAME_DIR .. "src/devices/machine/68307sim.h",
		MAME_DIR .. "src/devices/machine/68307bus.cpp",
		MAME_DIR .. "src/devices/machine/68307bus.h",
		MAME_DIR .. "src/devices/machine/68307tmu.cpp",
		MAME_DIR .. "src/devices/machine/68307tmu.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/68340.h,MACHINES["M68340"] = true
---------------------------------------------------

if (MACHINES["M68340"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/68340.cpp",
		MAME_DIR .. "src/devices/machine/68340.h",
		MAME_DIR .. "src/devices/machine/68340sim.cpp",
		MAME_DIR .. "src/devices/machine/68340sim.h",
		MAME_DIR .. "src/devices/machine/68340dma.cpp",
		MAME_DIR .. "src/devices/machine/68340dma.h",
		MAME_DIR .. "src/devices/machine/68340ser.cpp",
		MAME_DIR .. "src/devices/machine/68340ser.h",
		MAME_DIR .. "src/devices/machine/68340tmu.cpp",
		MAME_DIR .. "src/devices/machine/68340tmu.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mcf5206e.h,MACHINES["MCF5206E"] = true
---------------------------------------------------

if (MACHINES["MCF5206E"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mcf5206e.cpp",
		MAME_DIR .. "src/devices/machine/mcf5206e.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mdcr.h,MACHINES["MDCR"] = true
---------------------------------------------------

if (MACHINES["MDCR"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mdcr.cpp",
		MAME_DIR .. "src/devices/machine/mdcr.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/meters.h,MACHINES["METERS"] = true
---------------------------------------------------

if (MACHINES["METERS"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/meters.cpp",
		MAME_DIR .. "src/devices/machine/meters.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/microtch.h,MACHINES["MICROTOUCH"] = true
---------------------------------------------------

if (MACHINES["MICROTOUCH"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/microtch.cpp",
		MAME_DIR .. "src/devices/machine/microtch.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mm5307.h,MACHINES["MM5307"] = true
---------------------------------------------------

if (MACHINES["MM5307"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mm5307.cpp",
		MAME_DIR .. "src/devices/machine/mm5307.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mm5740.h,MACHINES["MM5740"] = true
---------------------------------------------------

if (MACHINES["MM5740"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mm5740.cpp",
		MAME_DIR .. "src/devices/machine/mm5740.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mm58274c.h,MACHINES["MM58274C"] = true
---------------------------------------------------

if (MACHINES["MM58274C"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mm58274c.cpp",
		MAME_DIR .. "src/devices/machine/mm58274c.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mm74c922.h,MACHINES["MM74C922"] = true
---------------------------------------------------

if (MACHINES["MM74C922"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mm74c922.cpp",
		MAME_DIR .. "src/devices/machine/mm74c922.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mos6526.h,MACHINES["MOS6526"] = true
---------------------------------------------------

if (MACHINES["MOS6526"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mos6526.cpp",
		MAME_DIR .. "src/devices/machine/mos6526.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mos6529.h,MACHINES["MOS6529"] = true
---------------------------------------------------

if (MACHINES["MOS6529"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mos6529.cpp",
		MAME_DIR .. "src/devices/machine/mos6529.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mos6530.h,MACHINES["MOS6530"] = true
---------------------------------------------------

if (MACHINES["MOS6530"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mos6530.cpp",
		MAME_DIR .. "src/devices/machine/mos6530.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mos6702.h,MACHINES["MOS6702"] = true
---------------------------------------------------

if (MACHINES["MOS6702"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mos6702.cpp",
		MAME_DIR .. "src/devices/machine/mos6702.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mos8706.h,MACHINES["MOS8706"] = true
---------------------------------------------------

if (MACHINES["MOS8706"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mos8706.cpp",
		MAME_DIR .. "src/devices/machine/mos8706.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mos8722.h,MACHINES["MOS8722"] = true
---------------------------------------------------

if (MACHINES["MOS8722"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mos8722.cpp",
		MAME_DIR .. "src/devices/machine/mos8722.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mos8726.h,MACHINES["MOS8726"] = true
---------------------------------------------------

if (MACHINES["MOS8726"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mos8726.cpp",
		MAME_DIR .. "src/devices/machine/mos8726.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mos6551.h,MACHINES["MOS6551"] = true
---------------------------------------------------

if (MACHINES["MOS6551"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mos6551.cpp",
		MAME_DIR .. "src/devices/machine/mos6551.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/msm5001n.h,MACHINES["MSM5001N"] = true
---------------------------------------------------

if (MACHINES["MSM5001N"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/msm5001n.cpp",
		MAME_DIR .. "src/devices/machine/msm5001n.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/msm5832.h,MACHINES["MSM5832"] = true
---------------------------------------------------

if (MACHINES["MSM5832"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/msm5832.cpp",
		MAME_DIR .. "src/devices/machine/msm5832.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/msm58321.h,MACHINES["MSM58321"] = true
---------------------------------------------------

if (MACHINES["MSM58321"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/msm58321.cpp",
		MAME_DIR .. "src/devices/machine/msm58321.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/msm6200.h,MACHINES["MSM6200"] = true
---------------------------------------------------

if (MACHINES["MSM6200"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/msm6200.cpp",
		MAME_DIR .. "src/devices/machine/msm6200.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/msm6242.h,MACHINES["MSM6242"] = true
---------------------------------------------------

if (MACHINES["MSM6242"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/msm6242.cpp",
		MAME_DIR .. "src/devices/machine/msm6242.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/msm6253.h,MACHINES["MSM6253"] = true
---------------------------------------------------

if (MACHINES["MSM6253"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/msm6253.cpp",
		MAME_DIR .. "src/devices/machine/msm6253.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/myb3k_kbd.h,MACHINES["MYB3K_KEYBOARD"] = true
---------------------------------------------------

if (MACHINES["MYB3K_KEYBOARD"]~=null) then
	files {
	MAME_DIR .. "src/devices/machine/myb3k_kbd.cpp",
	MAME_DIR .. "src/devices/machine/myb3k_kbd.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/nandflash.h,MACHINES["NANDFLASH"] = true
---------------------------------------------------

if (MACHINES["NANDFLASH"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/nandflash.cpp",
		MAME_DIR .. "src/devices/machine/nandflash.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/nmc9306.h,MACHINES["NMC9306"] = true
---------------------------------------------------

if (MACHINES["NMC9306"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/nmc9306.cpp",
		MAME_DIR .. "src/devices/machine/nmc9306.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/nscsi_bus.h,MACHINES["NSCSI"] = true
--@src/devices/machine/nscsi_cb.h,MACHINES["NSCSI"] = true
---------------------------------------------------

if (MACHINES["NSCSI"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/nscsi_bus.cpp",
		MAME_DIR .. "src/devices/machine/nscsi_bus.h",
		MAME_DIR .. "src/devices/machine/nscsi_cb.cpp",
		MAME_DIR .. "src/devices/machine/nscsi_cb.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pcf8573.h,MACHINES["PCF8573"] = true
---------------------------------------------------

if (MACHINES["PCF8573"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pcf8573.cpp",
		MAME_DIR .. "src/devices/machine/pcf8573.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pcf8583.h,MACHINES["PCF8583"] = true
---------------------------------------------------

if (MACHINES["PCF8583"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pcf8583.cpp",
		MAME_DIR .. "src/devices/machine/pcf8583.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pcf8584.h,MACHINES["PCF8584"] = true
---------------------------------------------------

if (MACHINES["PCF8584"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pcf8584.cpp",
		MAME_DIR .. "src/devices/machine/pcf8584.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pcf8593.h,MACHINES["PCF8593"] = true
---------------------------------------------------

if (MACHINES["PCF8593"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pcf8593.cpp",
		MAME_DIR .. "src/devices/machine/pcf8593.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/lpci.h,MACHINES["LPCI"] = true
---------------------------------------------------

if (MACHINES["LPCI"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/lpci.cpp",
		MAME_DIR .. "src/devices/machine/lpci.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pci.h,MACHINES["PCI"] = true
---------------------------------------------------

if (MACHINES["PCI"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pci.cpp",
		MAME_DIR .. "src/devices/machine/pci.h",
		MAME_DIR .. "src/devices/machine/pci-usb.cpp",
		MAME_DIR .. "src/devices/machine/pci-usb.h",
		MAME_DIR .. "src/devices/machine/pci-sata.cpp",
		MAME_DIR .. "src/devices/machine/pci-sata.h",
		MAME_DIR .. "src/devices/machine/pci-ide.cpp",
		MAME_DIR .. "src/devices/machine/pci-ide.h",
		MAME_DIR .. "src/devices/machine/pci-apic.cpp",
		MAME_DIR .. "src/devices/machine/pci-apic.h",
		MAME_DIR .. "src/devices/machine/pci-smbus.cpp",
		MAME_DIR .. "src/devices/machine/pci-smbus.h",
		MAME_DIR .. "src/devices/machine/i82541.cpp",
		MAME_DIR .. "src/devices/machine/i82541.h",
		MAME_DIR .. "src/devices/machine/i82875p.cpp",
		MAME_DIR .. "src/devices/machine/i82875p.h",
		MAME_DIR .. "src/devices/machine/i6300esb.cpp",
		MAME_DIR .. "src/devices/machine/i6300esb.h",
		MAME_DIR .. "src/devices/machine/i82439hx.cpp",
		MAME_DIR .. "src/devices/machine/i82439hx.h",
		MAME_DIR .. "src/devices/machine/i82439tx.cpp",
		MAME_DIR .. "src/devices/machine/i82439tx.h",
		MAME_DIR .. "src/devices/machine/i82443bx_host.cpp",
		MAME_DIR .. "src/devices/machine/i82443bx_host.h",
		MAME_DIR .. "src/devices/machine/i82371sb.cpp",
		MAME_DIR .. "src/devices/machine/i82371sb.h",
		MAME_DIR .. "src/devices/machine/i82371eb_isa.cpp",
		MAME_DIR .. "src/devices/machine/i82371eb_isa.h",
		MAME_DIR .. "src/devices/machine/i82371eb_ide.cpp",
		MAME_DIR .. "src/devices/machine/i82371eb_ide.h",
		MAME_DIR .. "src/devices/machine/i82371eb_acpi.cpp",
		MAME_DIR .. "src/devices/machine/i82371eb_acpi.h",
		MAME_DIR .. "src/devices/machine/i82371eb_usb.cpp",
		MAME_DIR .. "src/devices/machine/i82371eb_usb.h",
		MAME_DIR .. "src/devices/machine/lpc.h",
		MAME_DIR .. "src/devices/machine/lpc-acpi.cpp",
		MAME_DIR .. "src/devices/machine/lpc-acpi.h",
		MAME_DIR .. "src/devices/machine/lpc-rtc.cpp",
		MAME_DIR .. "src/devices/machine/lpc-rtc.h",
		MAME_DIR .. "src/devices/machine/lpc-pit.cpp",
		MAME_DIR .. "src/devices/machine/lpc-pit.h",
		MAME_DIR .. "src/devices/machine/mpc106.cpp",
		MAME_DIR .. "src/devices/machine/mpc106.h",
		MAME_DIR .. "src/devices/machine/mv6436x.cpp",
		MAME_DIR .. "src/devices/machine/mv6436x.h",
		MAME_DIR .. "src/devices/machine/vrc4373.cpp",
		MAME_DIR .. "src/devices/machine/vrc4373.h",
		MAME_DIR .. "src/devices/machine/vrc5074.cpp",
		MAME_DIR .. "src/devices/machine/vrc5074.h",
		MAME_DIR .. "src/devices/machine/gt64xxx.cpp",
		MAME_DIR .. "src/devices/machine/gt64xxx.h",
		MAME_DIR .. "src/devices/machine/sis5513_ide.cpp",
		MAME_DIR .. "src/devices/machine/sis5513_ide.h",
		MAME_DIR .. "src/devices/machine/sis630_host.cpp",
		MAME_DIR .. "src/devices/machine/sis630_host.h",
		MAME_DIR .. "src/devices/machine/sis630_gui.cpp",
		MAME_DIR .. "src/devices/machine/sis630_gui.h",
		MAME_DIR .. "src/devices/machine/sis7001_usb.cpp",
		MAME_DIR .. "src/devices/machine/sis7001_usb.h",
		MAME_DIR .. "src/devices/machine/sis7018_audio.cpp",
		MAME_DIR .. "src/devices/machine/sis7018_audio.h",
		MAME_DIR .. "src/devices/machine/sis900_eth.cpp",
		MAME_DIR .. "src/devices/machine/sis900_eth.h",
		MAME_DIR .. "src/devices/machine/sis950_lpc.cpp",
		MAME_DIR .. "src/devices/machine/sis950_lpc.h",
		MAME_DIR .. "src/devices/machine/sis950_smbus.cpp",
		MAME_DIR .. "src/devices/machine/sis950_smbus.h",
		MAME_DIR .. "src/devices/machine/sis85c496.cpp",
		MAME_DIR .. "src/devices/machine/sis85c496.h",
		MAME_DIR .. "src/devices/machine/vt8231_isa.cpp",
		MAME_DIR .. "src/devices/machine/vt8231_isa.h",
		MAME_DIR .. "src/devices/machine/mediagx_cs5530_bridge.cpp",
		MAME_DIR .. "src/devices/machine/mediagx_cs5530_bridge.h",
		MAME_DIR .. "src/devices/machine/mediagx_cs5530_ide.cpp",
		MAME_DIR .. "src/devices/machine/mediagx_cs5530_ide.h",
		MAME_DIR .. "src/devices/machine/mediagx_cs5530_video.cpp",
		MAME_DIR .. "src/devices/machine/mediagx_cs5530_video.h",
		MAME_DIR .. "src/devices/machine/mediagx_host.cpp",
		MAME_DIR .. "src/devices/machine/mediagx_host.h",
		MAME_DIR .. "src/devices/machine/zfmicro_usb.cpp",
		MAME_DIR .. "src/devices/machine/zfmicro_usb.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pckeybrd.h,MACHINES["PCKEYBRD"] = true
---------------------------------------------------

if (MACHINES["PCKEYBRD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pckeybrd.cpp",
		MAME_DIR .. "src/devices/machine/pckeybrd.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/phi.h,MACHINES["PHI"] = true
---------------------------------------------------

if (MACHINES["PHI"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/phi.cpp",
		MAME_DIR .. "src/devices/machine/phi.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pic8259.h,MACHINES["PIC8259"] = true
---------------------------------------------------

if (MACHINES["PIC8259"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pic8259.cpp",
		MAME_DIR .. "src/devices/machine/pic8259.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pit8253.h,MACHINES["PIT8253"] = true
---------------------------------------------------

if (MACHINES["PIT8253"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pit8253.cpp",
		MAME_DIR .. "src/devices/machine/pit8253.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pla.h,MACHINES["PLA"] = true
---------------------------------------------------

if (MACHINES["PLA"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pla.cpp",
		MAME_DIR .. "src/devices/machine/pla.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/psion_asic1.h,MACHINES["PSION_ASIC"] = true
--@src/devices/machine/psion_asic2.h,MACHINES["PSION_ASIC"] = true
--@src/devices/machine/psion_asic3.h,MACHINES["PSION_ASIC"] = true
--@src/devices/machine/psion_asic4.h,MACHINES["PSION_ASIC"] = true
--@src/devices/machine/psion_asic5.h,MACHINES["PSION_ASIC"] = true
--@src/devices/machine/psion_asic9.h,MACHINES["PSION_ASIC"] = true
---------------------------------------------------

if (MACHINES["PSION_ASIC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/psion_asic1.cpp",
		MAME_DIR .. "src/devices/machine/psion_asic1.h",
		MAME_DIR .. "src/devices/machine/psion_asic2.cpp",
		MAME_DIR .. "src/devices/machine/psion_asic2.h",
		MAME_DIR .. "src/devices/machine/psion_asic3.cpp",
		MAME_DIR .. "src/devices/machine/psion_asic3.h",
		MAME_DIR .. "src/devices/machine/psion_asic4.cpp",
		MAME_DIR .. "src/devices/machine/psion_asic4.h",
		MAME_DIR .. "src/devices/machine/psion_asic5.cpp",
		MAME_DIR .. "src/devices/machine/psion_asic5.h",
		MAME_DIR .. "src/devices/machine/psion_asic9.cpp",
		MAME_DIR .. "src/devices/machine/psion_asic9.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/psion_condor.h,MACHINES["PSION_CONDOR"] = true
---------------------------------------------------

if (MACHINES["PSION_CONDOR"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/psion_condor.cpp",
		MAME_DIR .. "src/devices/machine/psion_condor.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/psion_ssd.h,MACHINES["PSION_SSD"] = true
---------------------------------------------------

if (MACHINES["PSION_SSD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/psion_ssd.cpp",
		MAME_DIR .. "src/devices/machine/psion_ssd.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pxa255.h,MACHINES["PXA255"] = true
---------------------------------------------------

if (MACHINES["PXA255"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pxa255.cpp",
		MAME_DIR .. "src/devices/machine/pxa255.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/r10696.h,MACHINES["R10696"] = true
---------------------------------------------------

if (MACHINES["R10696"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/r10696.cpp",
		MAME_DIR .. "src/devices/machine/r10696.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/r10788.h,MACHINES["R10788"] = true
---------------------------------------------------

if (MACHINES["R10788"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/r10788.cpp",
		MAME_DIR .. "src/devices/machine/r10788.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/r65c52.h,MACHINES["R65C52"] = true
---------------------------------------------------

if (MACHINES["R65C52"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/r65c52.cpp",
		MAME_DIR .. "src/devices/machine/r65c52.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ra17xx.h,MACHINES["RA17XX"] = true
---------------------------------------------------

if (MACHINES["RA17XX"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ra17xx.cpp",
		MAME_DIR .. "src/devices/machine/ra17xx.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/rf5c296.h,MACHINES["RF5C296"] = true
---------------------------------------------------

if (MACHINES["RF5C296"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/rf5c296.cpp",
		MAME_DIR .. "src/devices/machine/rf5c296.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ripple_counter.h,MACHINES["RIPPLE_COUNTER"] = true
---------------------------------------------------

if (MACHINES["RIPPLE_COUNTER"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ripple_counter.cpp",
		MAME_DIR .. "src/devices/machine/ripple_counter.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/roc10937.h,MACHINES["ROC10937"] = true
---------------------------------------------------

if (MACHINES["ROC10937"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/roc10937.cpp",
		MAME_DIR .. "src/devices/machine/roc10937.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/rp5c01.h,MACHINES["RP5C01"] = true
---------------------------------------------------

if (MACHINES["RP5C01"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/rp5c01.cpp",
		MAME_DIR .. "src/devices/machine/rp5c01.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/rp5c15.h,MACHINES["RP5C15"] = true
---------------------------------------------------

if (MACHINES["RP5C15"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/rp5c15.cpp",
		MAME_DIR .. "src/devices/machine/rp5c15.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/rp5h01.h,MACHINES["RP5H01"] = true
---------------------------------------------------

if (MACHINES["RP5H01"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/rp5h01.cpp",
		MAME_DIR .. "src/devices/machine/rp5h01.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/64h156.h,MACHINES["R64H156"] = true
---------------------------------------------------

if (MACHINES["R64H156"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/64h156.cpp",
		MAME_DIR .. "src/devices/machine/64h156.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/rstbuf.h,MACHINES["RSTBUF"] = true
---------------------------------------------------

if (MACHINES["RSTBUF"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/rstbuf.cpp",
		MAME_DIR .. "src/devices/machine/rstbuf.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/rtc4543.h,MACHINES["RTC4543"] = true
---------------------------------------------------

if (MACHINES["RTC4543"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/rtc4543.cpp",
		MAME_DIR .. "src/devices/machine/rtc4543.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/rtc65271.h,MACHINES["RTC65271"] = true
---------------------------------------------------

if (MACHINES["RTC65271"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/rtc65271.cpp",
		MAME_DIR .. "src/devices/machine/rtc65271.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/rtc9701.h,MACHINES["RTC9701"] = true
---------------------------------------------------

if (MACHINES["RTC9701"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/rtc9701.cpp",
		MAME_DIR .. "src/devices/machine/rtc9701.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/s2350.h,MACHINES["S2350"] = true
---------------------------------------------------

if (MACHINES["S2350"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/s2350.cpp",
		MAME_DIR .. "src/devices/machine/s2350.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/s2636.h,MACHINES["S2636"] = true
---------------------------------------------------

if (MACHINES["S2636"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/s2636.cpp",
		MAME_DIR .. "src/devices/machine/s2636.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/s3520cf.h,MACHINES["S3520CF"] = true
---------------------------------------------------

if (MACHINES["S3520CF"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/s3520cf.cpp",
		MAME_DIR .. "src/devices/machine/s3520cf.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/s3c2400.h,MACHINES["S3C24XX"] = true
--@src/devices/machine/s3c2410.h,MACHINES["S3C24XX"] = true
--@src/devices/machine/s3c2440.h,MACHINES["S3C24XX"] = true
---------------------------------------------------

if (MACHINES["S3C24XX"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/s3c2400.cpp",
		MAME_DIR .. "src/devices/machine/s3c2400.h",
		MAME_DIR .. "src/devices/machine/s3c2410.cpp",
		MAME_DIR .. "src/devices/machine/s3c2410.h",
		MAME_DIR .. "src/devices/machine/s3c2440.cpp",
		MAME_DIR .. "src/devices/machine/s3c2440.h",
		MAME_DIR .. "src/devices/machine/s3c24xx.cpp",
		MAME_DIR .. "src/devices/machine/s3c24xx.h",
		MAME_DIR .. "src/devices/machine/s3c24xx.hxx",
	}
end

---------------------------------------------------
--
--@src/devices/machine/s3c44b0.h,MACHINES["S3C44B0"] = true
---------------------------------------------------

if (MACHINES["S3C44B0"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/s3c44b0.cpp",
		MAME_DIR .. "src/devices/machine/s3c44b0.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/sa1110.h,MACHINES["SA1110"] = true
---------------------------------------------------

if (MACHINES["SA1110"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/sa1110.cpp",
		MAME_DIR .. "src/devices/machine/sa1110.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/sa1111.h,MACHINES["SA1111"] = true
---------------------------------------------------

if (MACHINES["SA1111"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/sa1111.cpp",
		MAME_DIR .. "src/devices/machine/sa1111.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/saa1043.h,MACHINES["SAA1043"] = true
---------------------------------------------------

if (MACHINES["SAA1043"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/saa1043.cpp",
		MAME_DIR .. "src/devices/machine/saa1043.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/saa5070.h,MACHINES["SAA5070"] = true
---------------------------------------------------

if (MACHINES["SAA5070"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/saa5070.cpp",
		MAME_DIR .. "src/devices/machine/saa5070.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/sc16is741.h,MACHINES["SC16IS741"] = true
---------------------------------------------------
if (MACHINES["SC16IS741"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/sc16is741.cpp",
		MAME_DIR .. "src/devices/machine/sc16is741.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/scc66470.h,MACHINES["SCC66470"] = true
---------------------------------------------------
if (MACHINES["SCC66470"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/scc66470.cpp",
		MAME_DIR .. "src/devices/machine/scc66470.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/scc68070.h,MACHINES["SCC68070"] = true
---------------------------------------------------
if (MACHINES["SCC68070"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/scc68070.cpp",
		MAME_DIR .. "src/devices/machine/scc68070.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/scn_pci.h,MACHINES["SCN_PCI"] = true
---------------------------------------------------

if (MACHINES["SCN_PCI"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/scn_pci.cpp",
		MAME_DIR .. "src/devices/machine/scn_pci.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/scoop.h,MACHINES["SCOOP"] = true
---------------------------------------------------
if (MACHINES["SCOOP"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/scoop.cpp",
		MAME_DIR .. "src/devices/machine/scoop.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/scnxx562.h,MACHINES["DUSCC"] = true
---------------------------------------------------

if (MACHINES["DUSCC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/scnxx562.cpp",
		MAME_DIR .. "src/devices/machine/scnxx562.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/sda2006.h,MACHINES["SDA2006"] = true
---------------------------------------------------

if (MACHINES["SDA2006"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/sda2006.cpp",
		MAME_DIR .. "src/devices/machine/sda2006.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/sensorboard.h,MACHINES["SENSORBOARD"] = true
---------------------------------------------------

if (MACHINES["SENSORBOARD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/sensorboard.cpp",
		MAME_DIR .. "src/devices/machine/sensorboard.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/smc91c9x.h,MACHINES["SMC91C9X"] = true
---------------------------------------------------

if (MACHINES["SMC91C9X"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/smc91c9x.cpp",
		MAME_DIR .. "src/devices/machine/smc91c9x.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/smpc.h,MACHINES["SMPC"] = true
---------------------------------------------------

if (MACHINES["SMPC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/smpc.cpp",
		MAME_DIR .. "src/devices/machine/smpc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/sega_scu.h,MACHINES["SEGA_SCU"] = true
---------------------------------------------------

if (MACHINES["SEGA_SCU"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/sega_scu.cpp",
		MAME_DIR .. "src/devices/machine/sega_scu.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/spg2xx.h,MACHINES["SPG2XX"] = true
--@src/devices/machine/spg110.h,MACHINES["SPG2XX"] = true
--@src/devices/machine/generalplus_gpl16250soc.h,MACHINES["SPG2XX"] = true
---------------------------------------------------

if (MACHINES["SPG2XX"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/spg2xx.cpp",
		MAME_DIR .. "src/devices/machine/spg2xx.h",
		MAME_DIR .. "src/devices/machine/spg2xx_audio.cpp",
		MAME_DIR .. "src/devices/machine/spg2xx_audio.h",
		MAME_DIR .. "src/devices/machine/spg2xx_io.cpp",
		MAME_DIR .. "src/devices/machine/spg2xx_io.h",
		MAME_DIR .. "src/devices/machine/spg2xx_sysdma.cpp",
		MAME_DIR .. "src/devices/machine/spg2xx_sysdma.h",
		MAME_DIR .. "src/devices/machine/spg2xx_video.cpp",
		MAME_DIR .. "src/devices/machine/spg2xx_video.h",
		MAME_DIR .. "src/devices/machine/spg110.cpp",
		MAME_DIR .. "src/devices/machine/spg110.h",
		MAME_DIR .. "src/devices/machine/spg110_video.cpp",
		MAME_DIR .. "src/devices/machine/spg110_video.h",
		MAME_DIR .. "src/devices/machine/generalplus_gpl16250soc.cpp",
		MAME_DIR .. "src/devices/machine/generalplus_gpl16250soc.h",
		MAME_DIR .. "src/devices/machine/generalplus_gpl16250soc_video.cpp",
		MAME_DIR .. "src/devices/machine/generalplus_gpl16250soc_video.h",
		MAME_DIR .. "src/devices/machine/spg_renderer.cpp",
		MAME_DIR .. "src/devices/machine/spg_renderer.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/spg290_cdservo.h,MACHINES["SPG290"] = true
--@src/devices/machine/spg290_timer.h,MACHINES["SPG290"] = true
--@src/devices/machine/spg290_i2c.h,MACHINES["SPG290"] = true
--@src/devices/machine/spg290_ppu.h,MACHINES["SPG290"] = true
---------------------------------------------------

if (MACHINES["SPG290"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/spg290_cdservo.cpp",
		MAME_DIR .. "src/devices/machine/spg290_cdservo.h",
		MAME_DIR .. "src/devices/machine/spg290_timer.cpp",
		MAME_DIR .. "src/devices/machine/spg290_timer.h",
		MAME_DIR .. "src/devices/machine/spg290_i2c.cpp",
		MAME_DIR .. "src/devices/machine/spg290_i2c.h",
		MAME_DIR .. "src/devices/machine/spg290_ppu.cpp",
		MAME_DIR .. "src/devices/machine/spg290_ppu.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/swtpc8212.h,MACHINES["SWTPC8212"] = true
---------------------------------------------------

if (MACHINES["SWTPC8212"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/swtpc8212.cpp",
		MAME_DIR .. "src/devices/machine/swtpc8212.h",
	}
end

---------------------------------------------------
--
--
---------------------------------------------------

if (BUSES["ATA"]~=null) or (BUSES["SCSI"]~=null) then
	MACHINES["T10"] = true
end

if (MACHINES["T10"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/t10mmc.cpp",
		MAME_DIR .. "src/devices/machine/t10mmc.h",
		MAME_DIR .. "src/devices/machine/t10sbc.cpp",
		MAME_DIR .. "src/devices/machine/t10sbc.h",
		MAME_DIR .. "src/devices/machine/t10spc.cpp",
		MAME_DIR .. "src/devices/machine/t10spc.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/smartboard.h,MACHINES["TASC_SB30"] = true
---------------------------------------------------

if (MACHINES["TASC_SB30"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/smartboard.cpp",
		MAME_DIR .. "src/devices/machine/smartboard.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tc009xlvc.h,MACHINES["TC0091LVC"] = true
---------------------------------------------------

if (MACHINES["TC0091LVC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tc009xlvc.cpp",
		MAME_DIR .. "src/devices/machine/tc009xlvc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tdc1008.h,MACHINES["TDC1008"] = true
---------------------------------------------------

if (MACHINES["TDC1008"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tdc1008.cpp",
		MAME_DIR .. "src/devices/machine/tdc1008.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/te7750.h,MACHINES["TE7750"] = true
---------------------------------------------------

if (MACHINES["TE7750"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/te7750.cpp",
		MAME_DIR .. "src/devices/machine/te7750.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/thmfc1.h,MACHINES["THMFC1"] = true
---------------------------------------------------

if (MACHINES["THMFC1"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/thmfc1.cpp",
		MAME_DIR .. "src/devices/machine/thmfc1.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ticket.h,MACHINES["TICKET"] = true
---------------------------------------------------

if (MACHINES["TICKET"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ticket.cpp",
		MAME_DIR .. "src/devices/machine/ticket.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/timekpr.h,MACHINES["TIMEKPR"] = true
---------------------------------------------------

if (MACHINES["TIMEKPR"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/timekpr.cpp",
		MAME_DIR .. "src/devices/machine/timekpr.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tmc0430.h,MACHINES["TMC0430"] = true
---------------------------------------------------

if (MACHINES["TMC0430"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tmc0430.cpp",
		MAME_DIR .. "src/devices/machine/tmc0430.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tmc0999.h,MACHINES["TMC0999"] = true
---------------------------------------------------

if (MACHINES["TMC0999"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tmc0999.cpp",
		MAME_DIR .. "src/devices/machine/tmc0999.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tmc208k.h,MACHINES["TMC208K"] = true
---------------------------------------------------

if (MACHINES["TMC208K"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tmc208k.cpp",
		MAME_DIR .. "src/devices/machine/tmc208k.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tms1024.h,MACHINES["TMS1024"] = true
---------------------------------------------------

if (MACHINES["TMS1024"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tms1024.cpp",
		MAME_DIR .. "src/devices/machine/tms1024.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tms5501.h,MACHINES["TMS5501"] = true
---------------------------------------------------

if (MACHINES["TMS5501"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tms5501.cpp",
		MAME_DIR .. "src/devices/machine/tms5501.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tms6100.h,MACHINES["TMS6100"] = true
---------------------------------------------------

if (MACHINES["TMS6100"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tms6100.cpp",
		MAME_DIR .. "src/devices/machine/tms6100.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tms9901.h,MACHINES["TMS9901"] = true
---------------------------------------------------

if (MACHINES["TMS9901"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tms9901.cpp",
		MAME_DIR .. "src/devices/machine/tms9901.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tms9902.h,MACHINES["TMS9902"] = true
---------------------------------------------------

if (MACHINES["TMS9902"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tms9902.cpp",
		MAME_DIR .. "src/devices/machine/tms9902.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tms9914.h,MACHINES["TMS9914"] = true
---------------------------------------------------

if (MACHINES["TMS9914"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tms9914.cpp",
		MAME_DIR .. "src/devices/machine/tms9914.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tsb12lv01a.h,MACHINES["TSB12LV01A"] = true
---------------------------------------------------

if (MACHINES["TSB12LV01A"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tsb12lv01a.cpp",
		MAME_DIR .. "src/devices/machine/tsb12lv01a.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tube.h,MACHINES["TUBE"] = true
---------------------------------------------------

if (MACHINES["TUBE"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tube.cpp",
		MAME_DIR .. "src/devices/machine/tube.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ucb1200.h,MACHINES["UCB1200"] = true
---------------------------------------------------

if (MACHINES["UCB1200"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ucb1200.cpp",
		MAME_DIR .. "src/devices/machine/ucb1200.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/upc82c710.h,MACHINES["UPC82C710"] = true
---------------------------------------------------

if (MACHINES["UPC82C710"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/upc82c710.cpp",
		MAME_DIR .. "src/devices/machine/upc82c710.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/upc82c711.h,MACHINES["UPC82C711"] = true
---------------------------------------------------

if (MACHINES["UPC82C711"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/upc82c711.cpp",
		MAME_DIR .. "src/devices/machine/upc82c711.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/upd1990a.h,MACHINES["UPD1990A"] = true
---------------------------------------------------

if (MACHINES["UPD1990A"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/upd1990a.cpp",
		MAME_DIR .. "src/devices/machine/upd1990a.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/upd4991a.h,MACHINES["UPD4991A"] = true
---------------------------------------------------

if (MACHINES["UPD4991A"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/upd4991a.cpp",
		MAME_DIR .. "src/devices/machine/upd4991a.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/upd4992.h,MACHINES["UPD4992"] = true
---------------------------------------------------

if (MACHINES["UPD4992"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/upd4992.cpp",
		MAME_DIR .. "src/devices/machine/upd4992.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/upd4701.h,MACHINES["UPD4701"] = true
---------------------------------------------------

if (MACHINES["UPD4701"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/upd4701.cpp",
		MAME_DIR .. "src/devices/machine/upd4701.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/upd7001.h,MACHINES["UPD7001"] = true
---------------------------------------------------

if (MACHINES["UPD7001"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/upd7001.cpp",
		MAME_DIR .. "src/devices/machine/upd7001.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/upd7002.h,MACHINES["UPD7002"] = true
---------------------------------------------------

if (MACHINES["UPD7002"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/upd7002.cpp",
		MAME_DIR .. "src/devices/machine/upd7002.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/upd7004.h,MACHINES["UPD7004"] = true
---------------------------------------------------

if (MACHINES["UPD7004"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/upd7004.cpp",
		MAME_DIR .. "src/devices/machine/upd7004.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/upd71071.h,MACHINES["UPD71071"] = true
---------------------------------------------------

if (MACHINES["UPD71071"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/upd71071.cpp",
		MAME_DIR .. "src/devices/machine/upd71071.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/upd765.h,MACHINES["UPD765"] = true
---------------------------------------------------

if (MACHINES["UPD765"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/upd765.cpp",
		MAME_DIR .. "src/devices/machine/upd765.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/v3021.h,MACHINES["V3021"] = true
---------------------------------------------------

if (MACHINES["V3021"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/v3021.cpp",
		MAME_DIR .. "src/devices/machine/v3021.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/vic_pl192.h,MACHINES["VIC_PL192"] = true
---------------------------------------------------

if (MACHINES["VIC_PL192"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/vic_pl192.cpp",
		MAME_DIR .. "src/devices/machine/vic_pl192.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/wd_fdc.h,MACHINES["WD_FDC"] = true
---------------------------------------------------

if (MACHINES["WD_FDC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/wd_fdc.cpp",
		MAME_DIR .. "src/devices/machine/wd_fdc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/wd1000.h,MACHINES["WD1000"] = true
---------------------------------------------------

if (MACHINES["WD1000"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/wd1000.cpp",
		MAME_DIR .. "src/devices/machine/wd1000.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/wd1010.h,MACHINES["WD1010"] = true
---------------------------------------------------

if (MACHINES["WD1010"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/wd1010.cpp",
		MAME_DIR .. "src/devices/machine/wd1010.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/wd11c00_17.h,MACHINES["WD11C00_17"] = true
---------------------------------------------------

if (MACHINES["WD11C00_17"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/wd11c00_17.cpp",
		MAME_DIR .. "src/devices/machine/wd11c00_17.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/wd2010.h,MACHINES["WD2010"] = true
---------------------------------------------------

if (MACHINES["WD2010"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/wd2010.cpp",
		MAME_DIR .. "src/devices/machine/wd2010.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/wd33c9x.h,MACHINES["WD33C9X"] = true
---------------------------------------------------

if (MACHINES["WD33C9X"]~=null) then
	MACHINES["SCSI"] = true
	files {
		MAME_DIR .. "src/devices/machine/wd33c9x.cpp",
		MAME_DIR .. "src/devices/machine/wd33c9x.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/wd7600.h,MACHINES["WD7600"] = true
---------------------------------------------------

if (MACHINES["WD7600"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/wd7600.cpp",
		MAME_DIR .. "src/devices/machine/wd7600.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/x2201.h,MACHINES["X2201"] = true
---------------------------------------------------

if (MACHINES["X2201"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/x2201.cpp",
		MAME_DIR .. "src/devices/machine/x2201.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/x2212.h,MACHINES["X2212"] = true
---------------------------------------------------

if (MACHINES["X2212"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/x2212.cpp",
		MAME_DIR .. "src/devices/machine/x2212.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/x76f041.h,MACHINES["X76F041"] = true
---------------------------------------------------

if (MACHINES["X76F041"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/x76f041.cpp",
		MAME_DIR .. "src/devices/machine/x76f041.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/x76f100.h,MACHINES["X76F100"] = true
---------------------------------------------------

if (MACHINES["X76F100"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/x76f100.cpp",
		MAME_DIR .. "src/devices/machine/x76f100.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ym2148.h,MACHINES["YM2148"] = true
---------------------------------------------------

if (MACHINES["YM2148"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ym2148.cpp",
		MAME_DIR .. "src/devices/machine/ym2148.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ym3802.h,MACHINES["YM3802"] = true
---------------------------------------------------

if (MACHINES["YM3802"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ym3802.cpp",
		MAME_DIR .. "src/devices/machine/ym3802.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/z80ctc.h,MACHINES["Z80CTC"] = true
---------------------------------------------------

if (MACHINES["Z80CTC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/z80ctc.cpp",
		MAME_DIR .. "src/devices/machine/z80ctc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/z80sio.h,MACHINES["Z80SIO"] = true
---------------------------------------------------

if (MACHINES["Z80SIO"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/z80sio.cpp",
		MAME_DIR .. "src/devices/machine/z80sio.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/z80scc.h,MACHINES["Z80SCC"] = true
---------------------------------------------------

if (MACHINES["Z80SCC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/z80scc.cpp",
		MAME_DIR .. "src/devices/machine/z80scc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/z80dma.h,MACHINES["Z80DMA"] = true
---------------------------------------------------

if (MACHINES["Z80DMA"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/z80dma.cpp",
		MAME_DIR .. "src/devices/machine/z80dma.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/z80pio.h,MACHINES["Z80PIO"] = true
---------------------------------------------------

if (MACHINES["Z80PIO"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/z80pio.cpp",
		MAME_DIR .. "src/devices/machine/z80pio.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/z80sti.h,MACHINES["Z80STI"] = true
---------------------------------------------------

if (MACHINES["Z80STI"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/z80sti.cpp",
		MAME_DIR .. "src/devices/machine/z80sti.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/z8536.h,MACHINES["Z8536"] = true
---------------------------------------------------

if (MACHINES["Z8536"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/z8536.cpp",
		MAME_DIR .. "src/devices/machine/z8536.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i8255.h,MACHINES["I8255"] = true
---------------------------------------------------

if (MACHINES["I8255"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i8255.cpp",
		MAME_DIR .. "src/devices/machine/i8255.h",
		MAME_DIR .. "src/devices/machine/mb89363b.cpp",
		MAME_DIR .. "src/devices/machine/mb89363b.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ncr5380.h,MACHINES["NCR5380"] = true
---------------------------------------------------

if (MACHINES["NCR5380"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ncr5380.cpp",
		MAME_DIR .. "src/devices/machine/ncr5380.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ncr53c90.h,MACHINES["NCR53C90"] = true
---------------------------------------------------

if (MACHINES["NCR53C90"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ncr53c90.cpp",
		MAME_DIR .. "src/devices/machine/ncr53c90.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mm58167.h,MACHINES["MM58167"] = true
---------------------------------------------------

if (MACHINES["MM58167"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mm58167.cpp",
		MAME_DIR .. "src/devices/machine/mm58167.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mm58174.h,MACHINES["MM58174"] = true
---------------------------------------------------

if (MACHINES["MM58174"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mm58174.cpp",
		MAME_DIR .. "src/devices/machine/mm58174.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/dp8390.h,MACHINES["DP8390"] = true
---------------------------------------------------

if (MACHINES["DP8390"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/dp8390.cpp",
		MAME_DIR .. "src/devices/machine/dp8390.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/dp83932c.h,MACHINES["DP83932C"] = true
---------------------------------------------------

if (MACHINES["DP83932C"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/dp83932c.cpp",
		MAME_DIR .. "src/devices/machine/dp83932c.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/dp8573a.h,MACHINES["DP8573A"] = true
---------------------------------------------------

if (MACHINES["DP8573A"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/dp8573a.cpp",
		MAME_DIR .. "src/devices/machine/dp8573a.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pc_lpt.h,MACHINES["PC_LPT"] = true
---------------------------------------------------

if (MACHINES["PC_LPT"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pc_lpt.cpp",
		MAME_DIR .. "src/devices/machine/pc_lpt.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pc_fdc.h,MACHINES["PC_FDC"] = true
---------------------------------------------------

if (MACHINES["PC_FDC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pc_fdc.cpp",
		MAME_DIR .. "src/devices/machine/pc_fdc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mpu401.h,MACHINES["MPU401"] = true
---------------------------------------------------

if (MACHINES["MPU401"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mpu401.cpp",
		MAME_DIR .. "src/devices/machine/mpu401.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/at_keybc.h,MACHINES["AT_KEYBC"] = true
---------------------------------------------------

if (MACHINES["AT_KEYBC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/at_keybc.cpp",
		MAME_DIR .. "src/devices/machine/at_keybc.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/hdc92x4.h,MACHINES["HDC9234"] = true
---------------------------------------------------

if (MACHINES["HDC9234"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/hdc92x4.cpp",
		MAME_DIR .. "src/devices/machine/hdc92x4.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/strata.h,MACHINES["STRATA"] = true
---------------------------------------------------

if (MACHINES["STRATA"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/strata.cpp",
		MAME_DIR .. "src/devices/machine/strata.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/steppers.h,MACHINES["STEPPERS"] = true
---------------------------------------------------

if (MACHINES["STEPPERS"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/steppers.cpp",
		MAME_DIR .. "src/devices/machine/steppers.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/em_reel.h,MACHINES["EM_REEL"] = true
---------------------------------------------------

if (MACHINES["EM_REEL"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/em_reel.cpp",
		MAME_DIR .. "src/devices/machine/em_reel.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/corvushd.h,MACHINES["CORVUSHD"] = true
---------------------------------------------------
if (MACHINES["CORVUSHD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/corvushd.cpp",
		MAME_DIR .. "src/devices/machine/corvushd.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/wozfdc.h,MACHINES["WOZFDC"] = true
---------------------------------------------------
if (MACHINES["WOZFDC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/wozfdc.cpp",
		MAME_DIR .. "src/devices/machine/wozfdc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/diablo_hd.h,MACHINES["DIABLO_HD"] = true
---------------------------------------------------
if (MACHINES["DIABLO_HD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/diablo_hd.cpp",
		MAME_DIR .. "src/devices/machine/diablo_hd.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/fdc37c665gt.h,MACHINES["FDC37C665GT"] = true
---------------------------------------------------

if (MACHINES["FDC37C665GT"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/fdc37c665gt.cpp",
		MAME_DIR .. "src/devices/machine/fdc37c665gt.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pci9050.h,MACHINES["PCI9050"] = true
---------------------------------------------------

if (MACHINES["PCI9050"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pci9050.cpp",
		MAME_DIR .. "src/devices/machine/pci9050.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/netlist.h,MACHINES["NETLIST"] = true
---------------------------------------------------

if (MACHINES["NETLIST"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/netlist.cpp",
		MAME_DIR .. "src/devices/machine/netlist.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/nsc810.h,MACHINES["NSC810"] = true
---------------------------------------------------

if (MACHINES["NSC810"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/nsc810.cpp",
		MAME_DIR .. "src/devices/machine/nsc810.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/vt82c496.h,MACHINES["VT82C496"] = true
---------------------------------------------------

if (MACHINES["VT82C496"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/vt82c496.cpp",
		MAME_DIR .. "src/devices/machine/vt82c496.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/fdc37c93x.h,MACHINES["FDC37C93X"] = true
---------------------------------------------------

if (MACHINES["FDC37C93X"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/fdc37c93x.cpp",
		MAME_DIR .. "src/devices/machine/fdc37c93x.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/it8705f.h,MACHINES["IT8705F"] = true
---------------------------------------------------

if (MACHINES["IT8705F"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/it8705f.cpp",
		MAME_DIR .. "src/devices/machine/it8705f.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pc87306.h,MACHINES["PC87306"] = true
---------------------------------------------------

if (MACHINES["PC87306"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pc87306.cpp",
		MAME_DIR .. "src/devices/machine/pc87306.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/pc97338.h,MACHINES["PC97338"] = true
---------------------------------------------------

if (MACHINES["PC97338"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pc97338.cpp",
		MAME_DIR .. "src/devices/machine/pc97338.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/w83787f.h,MACHINES["W83787F"] = true
---------------------------------------------------

if (MACHINES["W83787F"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/w83787f.cpp",
		MAME_DIR .. "src/devices/machine/w83787f.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/w83977tf.h,MACHINES["W83977TF"] = true
---------------------------------------------------

if (MACHINES["W83977TF"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/w83977tf.cpp",
		MAME_DIR .. "src/devices/machine/w83977tf.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/pdc.h,MACHINES["PDC"] = true
---------------------------------------------------

if (MACHINES["PDC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pdc.cpp",
		MAME_DIR .. "src/devices/machine/pdc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/genpc.h,MACHINES["GENPC"] = true
---------------------------------------------------

if (MACHINES["GENPC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/genpc.cpp",
		MAME_DIR .. "src/devices/machine/genpc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/gen_latch.h,MACHINES["GEN_LATCH"] = true
---------------------------------------------------

if (MACHINES["GEN_LATCH"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/gen_latch.cpp",
		MAME_DIR .. "src/devices/machine/gen_latch.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/fdc_pll.h,MACHINES["FDC_PLL"] = true
---------------------------------------------------

if (MACHINES["FDC_PLL"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/fdc_pll.cpp",
		MAME_DIR .. "src/devices/machine/fdc_pll.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/watchdog.h,MACHINES["WATCHDOG"] = true
---------------------------------------------------

if (MACHINES["WATCHDOG"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/watchdog.cpp",
		MAME_DIR .. "src/devices/machine/watchdog.h",
	}
end


---------------------------------------------------
--
--@src/devices/machine/smartmed.h,MACHINES["SMARTMEDIA"] = true
---------------------------------------------------
if (MACHINES["SMARTMEDIA"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/smartmed.cpp",
		MAME_DIR .. "src/devices/machine/smartmed.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/spi_psram.h,MACHINES["SPIPSRAM"] = true
---------------------------------------------------
if (MACHINES["SPIPSRAM"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/spi_psram.cpp",
		MAME_DIR .. "src/devices/machine/spi_psram.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/spi_sdcard.h,MACHINES["SPISDCARD"] = true
---------------------------------------------------
if (MACHINES["SPISDCARD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/spi_sdcard.cpp",
		MAME_DIR .. "src/devices/machine/spi_sdcard.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/scnxx562.h,MACHINES["SCNXX562"] = true
---------------------------------------------------
if (MACHINES["SCNXX562"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/scnxx562.cpp",
		MAME_DIR .. "src/devices/machine/scnxx562.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/input_merger.h,MACHINES["INPUT_MERGER"] = true
---------------------------------------------------
if (MACHINES["INPUT_MERGER"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/input_merger.cpp",
		MAME_DIR .. "src/devices/machine/input_merger.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/k054321.h,MACHINES["K054321"] = true
---------------------------------------------------
if (MACHINES["K054321"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/k054321.cpp",
		MAME_DIR .. "src/devices/machine/k054321.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/smioc.h,MACHINES["SMIOC"] = true
---------------------------------------------------

if (MACHINES["SMIOC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/smioc.cpp",
		MAME_DIR .. "src/devices/machine/smioc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i82586.h,MACHINES["I82586"] = true
---------------------------------------------------

if (MACHINES["I82586"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i82586.cpp",
		MAME_DIR .. "src/devices/machine/i82586.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/adc0844.h,MACHINES["ADC0844"] = true
---------------------------------------------------

if (MACHINES["ADC0844"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/adc0844.cpp",
		MAME_DIR .. "src/devices/machine/adc0844.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/28fxxx.h,MACHINES["28FXXX"] = true
---------------------------------------------------

if (MACHINES["28FXXX"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/28fxxx.cpp",
		MAME_DIR .. "src/devices/machine/28fxxx.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/gen_fifo.h,MACHINES["GEN_FIFO"] = true
---------------------------------------------------

if (MACHINES["GEN_FIFO"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/gen_fifo.cpp",
		MAME_DIR .. "src/devices/machine/gen_fifo.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/output_latch.h,MACHINES["OUTPUT_LATCH"] = true
---------------------------------------------------

if (MACHINES["OUTPUT_LATCH"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/output_latch.cpp",
		MAME_DIR .. "src/devices/machine/output_latch.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/z80daisy.h,MACHINES["Z80DAISY"] = true
---------------------------------------------------

if (MACHINES["Z80DAISY"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/z80daisy.cpp",
		MAME_DIR .. "src/devices/machine/z80daisy.h",
		MAME_DIR .. "src/devices/machine/z80daisy_generic.cpp",
		MAME_DIR .. "src/devices/machine/z80daisy_generic.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i8291a.h,MACHINES["I8291A"] = true
---------------------------------------------------

if (MACHINES["I8291A"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i8291a.cpp",
		MAME_DIR .. "src/devices/machine/i8291a.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ps2dma.h,MACHINES["PS2DMAC"] = true
---------------------------------------------------

if (MACHINES["PS2DMAC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ps2dma.cpp",
		MAME_DIR .. "src/devices/machine/ps2dma.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ps2intc.h,MACHINES["PS2INTC"] = true
---------------------------------------------------

if (MACHINES["PS2INTC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ps2intc.cpp",
		MAME_DIR .. "src/devices/machine/ps2intc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ps2mc.h,MACHINES["PS2MC"] = true
---------------------------------------------------

if (MACHINES["PS2MC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ps2mc.cpp",
		MAME_DIR .. "src/devices/machine/ps2mc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ps2pad.h,MACHINES["PS2PAD"] = true
---------------------------------------------------

if (MACHINES["PS2PAD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ps2pad.cpp",
		MAME_DIR .. "src/devices/machine/ps2pad.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ps2sif.h,MACHINES["PS2SIF"] = true
---------------------------------------------------

if (MACHINES["PS2SIF"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ps2sif.cpp",
		MAME_DIR .. "src/devices/machine/ps2sif.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ps2timer.h,MACHINES["PS2TIMER"] = true
---------------------------------------------------

if (MACHINES["PS2TIMER"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ps2timer.cpp",
		MAME_DIR .. "src/devices/machine/ps2timer.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/iopcdvd.h,MACHINES["IOPCDVD"] = true
---------------------------------------------------

if (MACHINES["IOPCDVD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/iopcdvd.cpp",
		MAME_DIR .. "src/devices/machine/iopcdvd.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/iopdma.h,MACHINES["IOPDMA"] = true
---------------------------------------------------

if (MACHINES["IOPDMA"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/iopdma.cpp",
		MAME_DIR .. "src/devices/machine/iopdma.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/iopintc.h,MACHINES["IOPINTC"] = true
---------------------------------------------------

if (MACHINES["IOPINTC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/iopintc.cpp",
		MAME_DIR .. "src/devices/machine/iopintc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/iopsio2.h,MACHINES["IOPSIO2"] = true
---------------------------------------------------

if (MACHINES["IOPSIO2"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/iopsio2.cpp",
		MAME_DIR .. "src/devices/machine/iopsio2.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ioptimer.h,MACHINES["IOPTIMER"] = true
---------------------------------------------------

if (MACHINES["IOPTIMER"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ioptimer.cpp",
		MAME_DIR .. "src/devices/machine/ioptimer.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/sun4c_mmu.h,MACHINES["SUN4C_MMU"] = true
---------------------------------------------------

if (MACHINES["SUN4C_MMU"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/sun4c_mmu.cpp",
		MAME_DIR .. "src/devices/machine/sun4c_mmu.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/z8038.h,MACHINES["Z8038"] = true
---------------------------------------------------

if (MACHINES["Z8038"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/z8038.cpp",
		MAME_DIR .. "src/devices/machine/z8038.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/scc2698b.h,MACHINES["SCC2698B"] = true
---------------------------------------------------

if (MACHINES["SCC2698B"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/scc2698b.cpp",
		MAME_DIR .. "src/devices/machine/scc2698b.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/aic565.h,MACHINES["AIC565"] = true
---------------------------------------------------

if (MACHINES["AIC565"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/aic565.cpp",
		MAME_DIR .. "src/devices/machine/aic565.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/aic580.h,MACHINES["AIC580"] = true
---------------------------------------------------

if (MACHINES["AIC580"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/aic580.cpp",
		MAME_DIR .. "src/devices/machine/aic580.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/aic6250.h,MACHINES["AIC6250"] = true
---------------------------------------------------

if (MACHINES["AIC6250"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/aic6250.cpp",
		MAME_DIR .. "src/devices/machine/aic6250.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i82357.h,MACHINES["I82357"] = true
---------------------------------------------------

if (MACHINES["I82357"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i82357.cpp",
		MAME_DIR .. "src/devices/machine/i82357.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/xc1700e.h,MACHINES["XC1700E"] = true
---------------------------------------------------

if (MACHINES["XC1700E"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/xc1700e.cpp",
		MAME_DIR .. "src/devices/machine/xc1700e.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/edlc.h,MACHINES["EDLC"] = true
---------------------------------------------------

if (MACHINES["EDLC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/edlc.cpp",
		MAME_DIR .. "src/devices/machine/edlc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/wtl3132.h,MACHINES["WTL3132"] = true
---------------------------------------------------

if (MACHINES["WTL3132"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/wtl3132.cpp",
		MAME_DIR .. "src/devices/machine/wtl3132.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/vrender0.h,MACHINES["VRENDER0"] = true
---------------------------------------------------

if (MACHINES["VRENDER0"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/vrender0.cpp",
		MAME_DIR .. "src/devices/machine/vr0uart.cpp",
		MAME_DIR .. "src/devices/machine/vrender0.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i3001.h,MACHINES["I3001"] = true
---------------------------------------------------

if (MACHINES["I3001"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i3001.cpp",
		MAME_DIR .. "src/devices/machine/i3001.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/i3002.h,MACHINES["I3002"] = true
---------------------------------------------------

if (MACHINES["I3002"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/i3002.cpp",
		MAME_DIR .. "src/devices/machine/i3002.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/s_smp.h,MACHINES["S_SMP"] = true
---------------------------------------------------

if (MACHINES["S_SMP"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/s_smp.cpp",
		MAME_DIR .. "src/devices/machine/s_smp.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/cxd1185.h,MACHINES["CXD1185"] = true
---------------------------------------------------

if (MACHINES["CXD1185"]~=null) then
	MACHINES["NSCSI"] = true
	files {
		MAME_DIR .. "src/devices/machine/cxd1185.cpp",
		MAME_DIR .. "src/devices/machine/cxd1185.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/spifi3.h,MACHINES["SPIFI3"] = true
---------------------------------------------------

if (MACHINES["SPIFI3"]~=null) then
	MACHINES["NSCSI"] = true
	files {
		MAME_DIR .. "src/devices/machine/spifi3.cpp",
		MAME_DIR .. "src/devices/machine/spifi3.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/applefdintf.h,MACHINES["APPLE_FDINTF"] = true
---------------------------------------------------
if (MACHINES["APPLE_FDINTF"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/applefdintf.cpp",
		MAME_DIR .. "src/devices/machine/applefdintf.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/iwm.h,MACHINES["IWM"] = true
---------------------------------------------------
if (MACHINES["IWM"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/iwm.cpp",
		MAME_DIR .. "src/devices/machine/iwm.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/swim1.h,MACHINES["SWIM1"] = true
---------------------------------------------------
if (MACHINES["SWIM1"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/swim1.cpp",
		MAME_DIR .. "src/devices/machine/swim1.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/swim2.h,MACHINES["SWIM2"] = true
---------------------------------------------------
if (MACHINES["SWIM2"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/swim2.cpp",
		MAME_DIR .. "src/devices/machine/swim2.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/swim3.h,MACHINES["SWIM3"] = true
---------------------------------------------------
if (MACHINES["SWIM3"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/swim3.cpp",
		MAME_DIR .. "src/devices/machine/swim3.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mv_sonora.h,MACHINES["MAC_VIDEO_SONORA"] = true
---------------------------------------------------
if (MACHINES["MAC_VIDEO_SONORA"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mv_sonora.cpp",
		MAME_DIR .. "src/devices/machine/mv_sonora.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/alpha_8921.h,MACHINES["ALPHA_8921"] = true
---------------------------------------------------
if (MACHINES["ALPHA_8921"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/alpha_8921.cpp",
		MAME_DIR .. "src/devices/machine/alpha_8921.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/bl_handhelds_menucontrol.h,MACHINES["BL_HANDHELDS_MENUCONTROL"] = true
---------------------------------------------------
if (MACHINES["BL_HANDHELDS_MENUCONTROL"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/bl_handhelds_menucontrol.cpp",
		MAME_DIR .. "src/devices/machine/bl_handhelds_menucontrol.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ns32081.h,MACHINES["NS32081"] = true
---------------------------------------------------
if (MACHINES["NS32081"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ns32081.cpp",
		MAME_DIR .. "src/devices/machine/ns32081.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ns32202.h,MACHINES["NS32202"] = true
---------------------------------------------------
if (MACHINES["NS32202"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ns32202.cpp",
		MAME_DIR .. "src/devices/machine/ns32202.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ns32082.h,MACHINES["NS32082"] = true
---------------------------------------------------
if (MACHINES["NS32082"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ns32082.cpp",
		MAME_DIR .. "src/devices/machine/ns32082.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/bitmap_printer.h,MACHINES["BITMAP_PRINTER"] = true
---------------------------------------------------
if (MACHINES["BITMAP_PRINTER"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/bitmap_printer.cpp",
		MAME_DIR .. "src/devices/machine/bitmap_printer.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ns32382.h,MACHINES["NS32382"] = true
---------------------------------------------------
if (MACHINES["NS32382"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ns32382.cpp",
		MAME_DIR .. "src/devices/machine/ns32382.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/fm_scsi.h,MACHINES["FM_SCSI"] = true
---------------------------------------------------
if (MACHINES["FM_SCSI"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/fm_scsi.cpp",
		MAME_DIR .. "src/devices/machine/fm_scsi.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/archimedes_keyb.h,MACHINES["ARCHIMEDES_KEYB"] = true
---------------------------------------------------
if (MACHINES["ARCHIMEDES_KEYB"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/archimedes_keyb.cpp",
		MAME_DIR .. "src/devices/machine/archimedes_keyb.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/cammu.h,MACHINES["CAMMU"] = true
---------------------------------------------------
if (MACHINES["CAMMU"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/cammu.cpp",
		MAME_DIR .. "src/devices/machine/cammu.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/at.h,MACHINES["AT_MB"] = true
---------------------------------------------------
if (MACHINES["AT_MB"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/at.cpp",
		MAME_DIR .. "src/devices/machine/at.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/bacta_datalogger.h,MACHINES["BACTA_DATALOGGER"] = true
---------------------------------------------------
if (MACHINES["BACTA_DATALOGGER"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/bacta_datalogger.cpp",
		MAME_DIR .. "src/devices/machine/bacta_datalogger.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/nmk112.h,MACHINES["NMK112"] = true
---------------------------------------------------
if (MACHINES["NMK112"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/nmk112.cpp",
		MAME_DIR .. "src/devices/machine/nmk112.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/saa7191.h,MACHINES["SAA7191"] = true
---------------------------------------------------
if (MACHINES["SAA7191"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/saa7191.cpp",
		MAME_DIR .. "src/devices/machine/saa7191.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/segacrpt_device.h,MACHINES["SEGACRPT"] = true
---------------------------------------------------
if (MACHINES["SEGACRPT"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/segacrpt_device.cpp",
		MAME_DIR .. "src/devices/machine/segacrpt_device.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/segacrp2_device.h,MACHINES["SEGACRP2"] = true
---------------------------------------------------
if (MACHINES["SEGACRP2"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/segacrp2_device.cpp",
		MAME_DIR .. "src/devices/machine/segacrp2_device.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/am9516.h,MACHINES["AM9516"] = true
---------------------------------------------------

if (MACHINES["AM9516"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/am9516.cpp",
		MAME_DIR .. "src/devices/machine/am9516.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/micomxe1a.h,MACHINES["MICOMXE1A"] = true
---------------------------------------------------

if (MACHINES["MICOMXE1A"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/micomxe1a.cpp",
		MAME_DIR .. "src/devices/machine/micomxe1a.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/mc88200.h,MACHINES["MC88200"] = true
---------------------------------------------------
if (MACHINES["MC88200"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mc88200.cpp",
		MAME_DIR .. "src/devices/machine/mc88200.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/tc9223.h,MACHINES["TC9223"] = true
---------------------------------------------------

if (MACHINES["TC9223"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tc9223.cpp",
		MAME_DIR .. "src/devices/machine/tc9223.h",
	}
end

---------------------------------------------------
--@src/devices/machine/upd7261.h,MACHINES["UPD7261"] = true
---------------------------------------------------

if (MACHINES["UPD7261"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/upd7261.cpp",
		MAME_DIR .. "src/devices/machine/upd7261.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/cat702.h,MACHINES["CAT702"] = true
---------------------------------------------------
if (MACHINES["CAT702"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/cat702.cpp",
		MAME_DIR .. "src/devices/machine/cat702.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/sci4.h,MACHINES["SCI4"] = true
---------------------------------------------------
if (MACHINES["SCI4"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/sci4.cpp",
		MAME_DIR .. "src/devices/machine/sci4.h",
	}
end
