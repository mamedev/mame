-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   machine.lua
--
--   Rules for building machine cores
--
----------------------------------------------------------------------------


---------------------------------------------------
--
--@src/devices/machine/akiko.h,MACHINES["AKIKO"] = true
---------------------------------------------------

if (MACHINES["AKIKO"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/akiko.cpp",
		MAME_DIR .. "src/devices/machine/akiko.h",
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
--@src/devices/machine/gayle.h,MACHINES["GAYLE"] = true
---------------------------------------------------

if (MACHINES["GAYLE"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/gayle.cpp",
		MAME_DIR .. "src/devices/machine/gayle.h",
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
--@src/devices/machine/6532riot.h,MACHINES["RIOT6532"] = true
---------------------------------------------------

if (MACHINES["RIOT6532"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/6532riot.cpp",
		MAME_DIR .. "src/devices/machine/6532riot.h",
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
--@src/devices/machine/68561mpcc.h,MACHINES += 68561MPCC
---------------------------------------------------

if (MACHINES["68561MPCC"]~=null) then
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
--@src/devices/machine/8530scc.h,MACHINES["8530SCC"] = true
---------------------------------------------------

if (MACHINES["8530SCC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/8530scc.cpp",
		MAME_DIR .. "src/devices/machine/8530scc.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/aakart.h,MACHINES["AAKARTDEV"] = true
---------------------------------------------------

if (MACHINES["AAKARTDEV"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/aakart.cpp",
		MAME_DIR .. "src/devices/machine/aakart.h",
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
--@src/devices/machine/am53cf96.h,MACHINES["AM53CF96"] = true
---------------------------------------------------

if (MACHINES["AM53CF96"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/am53cf96.cpp",
		MAME_DIR .. "src/devices/machine/am53cf96.h",
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
--@src/devices/machine/amigafdc.h,MACHINES["AMIGAFDC"] = true
---------------------------------------------------

if (MACHINES["AMIGAFDC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/amigafdc.cpp",
		MAME_DIR .. "src/devices/machine/amigafdc.h",
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
--@src/devices/machine/ataflash.h,MACHINES["ATAFLASH"] = true
---------------------------------------------------

if (MACHINES["ATAFLASH"]~=null) then
	MACHINES["IDE"] = true
	MACHINES["PCCARD"] = true
	files {
		MAME_DIR .. "src/devices/machine/ataflash.cpp",
		MAME_DIR .. "src/devices/machine/ataflash.h",
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
--@src/devices/machine/cr589.h,MACHINES["CR589"] = true
---------------------------------------------------

if (MACHINES["CR589"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/cr589.cpp",
		MAME_DIR .. "src/devices/machine/cr589.h",
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
--@src/devices/machine/ds1315.h,MACHINES["DS1315"] = true
---------------------------------------------------

if (MACHINES["DS1315"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ds1315.cpp",
		MAME_DIR .. "src/devices/machine/ds1315.h",
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
--@src/devices/machine/atadev.h,MACHINES["IDE"] = true
--@src/devices/machine/ataintf.h,MACHINES["IDE"] = true
---------------------------------------------------

if (MACHINES["IDE"]~=null) then
	MACHINES["T10"] = true
	files {
		MAME_DIR .. "src/devices/machine/atadev.cpp",
		MAME_DIR .. "src/devices/machine/atadev.h",
		MAME_DIR .. "src/devices/machine/atahle.cpp",
		MAME_DIR .. "src/devices/machine/atahle.h",
		MAME_DIR .. "src/devices/machine/ataintf.cpp",
		MAME_DIR .. "src/devices/machine/ataintf.h",
		MAME_DIR .. "src/devices/machine/atapicdr.cpp",
		MAME_DIR .. "src/devices/machine/atapicdr.h",
		MAME_DIR .. "src/devices/machine/atapihle.cpp",
		MAME_DIR .. "src/devices/machine/atapihle.h",
		MAME_DIR .. "src/devices/machine/idectrl.cpp",
		MAME_DIR .. "src/devices/machine/idectrl.h",
		MAME_DIR .. "src/devices/machine/idehd.cpp",
		MAME_DIR .. "src/devices/machine/idehd.h",
		MAME_DIR .. "src/devices/machine/vt83c461.cpp",
		MAME_DIR .. "src/devices/machine/vt83c461.h",
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
--@src/devices/machine/linflash.h,MACHINES["LINFLASH"] = true
---------------------------------------------------

if (MACHINES["LINFLASH"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/linflash.cpp",
		MAME_DIR .. "src/devices/machine/linflash.h",
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
--@src/devices/machine/matsucd.h,MACHINES["MATSUCD"] = true
---------------------------------------------------

if (MACHINES["MATSUCD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/matsucd.cpp",
		MAME_DIR .. "src/devices/machine/matsucd.h",
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
--@src/devices/machine/mb89352.h,MACHINES["MB89352"] = true
---------------------------------------------------

if (MACHINES["MB89352"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mb89352.cpp",
		MAME_DIR .. "src/devices/machine/mb89352.h",
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
--@src/devices/machine/mc2661.h,MACHINES["MC2661"] = true
---------------------------------------------------

if (MACHINES["MC2661"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mc2661.cpp",
		MAME_DIR .. "src/devices/machine/mc2661.h",
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
--@src/devices/machine/mos6530.h,MACHINES["MIOT6530"] = true
---------------------------------------------------

if (MACHINES["MIOT6530"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/mos6530.cpp",
		MAME_DIR .. "src/devices/machine/mos6530.h",
		MAME_DIR .. "src/devices/machine/mos6530n.cpp",
		MAME_DIR .. "src/devices/machine/mos6530n.h",
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
--@src/devices/machine/ncr539x.h,MACHINES["NCR539x"] = true
---------------------------------------------------

if (MACHINES["NCR539x"]~=null) then
	MACHINES["SCSI"] = true
	files {
		MAME_DIR .. "src/devices/machine/ncr539x.cpp",
		MAME_DIR .. "src/devices/machine/ncr539x.h",
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
--@src/devices/machine/nscsi_cd.h,MACHINES["NSCSI"] = true
--@src/devices/machine/nscsi_hd.h,MACHINES["NSCSI"] = true
--@src/devices/machine/nscsi_s1410.h,MACHINES["NSCSI"] = true
---------------------------------------------------

if (MACHINES["NSCSI"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/nscsi_bus.cpp",
		MAME_DIR .. "src/devices/machine/nscsi_bus.h",
		MAME_DIR .. "src/devices/machine/nscsi_cb.cpp",
		MAME_DIR .. "src/devices/machine/nscsi_cb.h",
		MAME_DIR .. "src/devices/machine/nscsi_cd.cpp",
		MAME_DIR .. "src/devices/machine/nscsi_cd.h",
		MAME_DIR .. "src/devices/machine/nscsi_hd.cpp",
		MAME_DIR .. "src/devices/machine/nscsi_hd.h",
		MAME_DIR .. "src/devices/machine/nscsi_s1410.cpp",
		MAME_DIR .. "src/devices/machine/nscsi_s1410.h",
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
		MAME_DIR .. "src/devices/machine/lpc.cpp",
		MAME_DIR .. "src/devices/machine/lpc.h",
		MAME_DIR .. "src/devices/machine/lpc-acpi.cpp",
		MAME_DIR .. "src/devices/machine/lpc-acpi.h",
		MAME_DIR .. "src/devices/machine/lpc-rtc.cpp",
		MAME_DIR .. "src/devices/machine/lpc-rtc.h",
		MAME_DIR .. "src/devices/machine/lpc-pit.cpp",
		MAME_DIR .. "src/devices/machine/lpc-pit.h",
		MAME_DIR .. "src/devices/machine/vrc4373.cpp",
		MAME_DIR .. "src/devices/machine/vrc4373.h",
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
	MACHINES["PCCARD"] = true
	files {
		MAME_DIR .. "src/devices/machine/rf5c296.cpp",
		MAME_DIR .. "src/devices/machine/rf5c296.h",
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
--@src/devices/machine/s3c2400.h,MACHINES["S3C2400"] = true
---------------------------------------------------

if (MACHINES["S3C2400"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/s3c2400.cpp",
		MAME_DIR .. "src/devices/machine/s3c2400.h",
		MAME_DIR .. "src/devices/machine/s3c24xx.inc",
	}
end

---------------------------------------------------
--
--@src/devices/machine/s3c2410.h,MACHINES["S3C2410"] = true
---------------------------------------------------

if (MACHINES["S3C2410"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/s3c2410.cpp",
		MAME_DIR .. "src/devices/machine/s3c2410.h",
		MAME_DIR .. "src/devices/machine/s3c24xx.inc",
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
--@src/devices/machine/s3c2440.h,MACHINES["S3C2440"] = true
---------------------------------------------------

if (MACHINES["S3C2440"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/s3c2440.cpp",
		MAME_DIR .. "src/devices/machine/s3c2440.h",
		MAME_DIR .. "src/devices/machine/s3c24xx.inc",
	}
end

---------------------------------------------------
--
--@src/devices/machine/saturn.h,MACHINES["SATURN"] = true
---------------------------------------------------

if (MACHINES["SATURN"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/saturn.cpp",
	}
end

---------------------------------------------------
--
--@src/devices/machine/serflash.h,MACHINES["SERFLASH"] = true
---------------------------------------------------

if (MACHINES["SERFLASH"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/serflash.cpp",
		MAME_DIR .. "src/devices/machine/serflash.h",
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
--@src/devices/machine/stvcd.h,MACHINES["STVCD"] = true
---------------------------------------------------

if (MACHINES["STVCD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/stvcd.cpp",
	}
end

---------------------------------------------------
--
--
---------------------------------------------------

if (BUSES["SCSI"]~=null) then
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
--@src/devices/machine/tmp68301.h,MACHINES["TMP68301"] = true
---------------------------------------------------

if (MACHINES["TMP68301"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/tmp68301.cpp",
		MAME_DIR .. "src/devices/machine/tmp68301.h",
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
		MAME_DIR .. "src/devices/machine/fdc_pll.cpp",
		MAME_DIR .. "src/devices/machine/fdc_pll.h",
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
--@src/devices/machine/wd_fdc.h,MACHINES["WD_FDC"] = true
---------------------------------------------------

if (MACHINES["WD_FDC"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/wd_fdc.cpp",
		MAME_DIR .. "src/devices/machine/wd_fdc.h",
		MAME_DIR .. "src/devices/machine/fdc_pll.cpp",
		MAME_DIR .. "src/devices/machine/fdc_pll.h",
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
--@src/devices/machine/wd33c93.h,MACHINES["WD33C93"] = true
---------------------------------------------------

if (MACHINES["WD33C93"]~=null) then
	MACHINES["SCSI"] = true
	files {
		MAME_DIR .. "src/devices/machine/wd33c93.cpp",
		MAME_DIR .. "src/devices/machine/wd33c93.h",
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
--@src/devices/machine/z80dart.h,MACHINES["Z80DART"] = true
---------------------------------------------------

if (MACHINES["Z80DART"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/z80dart.cpp",
		MAME_DIR .. "src/devices/machine/z80dart.h",
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
--@src/devices/machine/pccard.h,MACHINES["PCCARD"] = true
---------------------------------------------------

if (MACHINES["PCCARD"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/pccard.cpp",
		MAME_DIR .. "src/devices/machine/pccard.h",
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
--@src/devices/machine/ncr5380n.h,MACHINES["NCR5380N"] = true
---------------------------------------------------

if (MACHINES["NCR5380N"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ncr5380n.cpp",
		MAME_DIR .. "src/devices/machine/ncr5380n.h",
	}
end

---------------------------------------------------
--
--@src/devices/machine/ncr5389.h,MACHINES["NCR5390"] = true
---------------------------------------------------

if (MACHINES["NCR5390"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/ncr5390.cpp",
		MAME_DIR .. "src/devices/machine/ncr5390.h",
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

