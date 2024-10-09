// license:BSD-3-Clause
// copyright-holders:
/*************************************************************************************************************
Skeleton driver for IBM ThinkPad 760 series.
IBM ThinkPad 760 was a notebook computer introduced in 1995 by IBM as part of the ThinkPad 700-series.
Eleven models were produced: 760C, 760CD, 760L, 760LD, 760E, 760ED, 760EL, 760ELD, 760XL, 760XD, and 760D/L.

Hardware for the 760XD model.
  CPU PCB:
    -Intel Mobile Pentium MMX 166 (TT80503166).
    -Two Samsung KM732V589T-15 (Cache SRAM, 32KX32).
    -One IDT 71V256 (Lower Power 3.3V CMOS Fast SRAM 256K [32K x 8-Bit]).
  RAM PCB:
    -Four on-board Toshiba TC51V18165CFTS-60 (1M X 16 EDO DRAM).
    -Two DIMM slots.
  Main PCB:
    -Intel PCIset SB82437MX.
    -Intel PCIset SB82371FB.
    -Two Intel PCIset FA82438MX.
    -W48C60-402G.
    -Four M5M4V18165CTTP (1M x 16 EDO DRAM).
    -Dallas DS17485S-5 (real-time clock/calendar).
    -NEC 53G9037.
    -NEC 53G9038.
    -IBM 89G7219.
    -IBM 20H2888.
    -IBM 94G0207.
    -HD6433436A18F (mask ROM, IBM-branded).
    -Maxim MAX3243CAI.
    -C46CM6 (SEEPROM, for BIOS settings).
    -Texas Instruments PCIbus PCI1130PDV.
    -Texas Instruments 90G9510.
    -Intel Flash E28F004 (BIOS).
  Video PCB:
    -NEC 53G9037.
    -IBM 20H2929.
    -IBM 03H9515.
    -Trident Cyber9385T.
    -Two National Semiconductor DS90CF561MTD (LVDS 18-Bit Color Flat Panel Display [FPD] Link).
    -Six KM416V256DT-L6 (256K x 16Bit CMOS Dynamic RAM with Fast Page Mode).
    -AD722JR (RGB to NTSC/PAL Encoder).
    -Philips SAA7110A WP.
  Sound PCB:
    -HM62W4032HFP25.
    -Crystal CS4218-KQ.
    -Texas Instruments TCM320AC36C (Voice-Band Audio Processor).
  Two separate small PCBs with two IR transceivers.

*************************************************************************************************************/

#include "emu.h"
#include "cpu/h8/h83337.h"
#include "cpu/i386/i386.h"
#include "machine/ds17x85.h"
#include "machine/pci.h"
#include "speaker.h"


namespace {

class thinkpad760_state : public driver_device
{
public:
	thinkpad760_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void thinkpad760(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
};


static INPUT_PORTS_START(thinkpad760)
INPUT_PORTS_END

void thinkpad760_state::thinkpad760(machine_config &config)
{
	PENTIUM(config, m_maincpu, 166'000'000); // Intel Mobile Pentium MMX 166 (TT80503166)

	PCI_ROOT(config, "pci", 0);
	// ...

	H83337(config, "mcu", 10_MHz_XTAL); // Actually an Hitachi HD6433436A18F

	DS17485(config, "rtc", 16'000'000); // Dallas DS17485S-5, unknown clock

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

ROM_START(thinkpad760xd)
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "e28f004_89g8164_rev37_h1897m.u17", 0x00000, 0x80000, CRC(6092594f) SHA1(25681e4952a432e1170f69ae75f3260245b6b44b) ) // BIOS

	ROM_REGION( 0x0f780, "mcu", 0)
	ROM_LOAD( "ibm_hd6433436a18f_40h8792.u39",    0x00000, 0x0f780, NO_DUMP ) // Mask ROM, undumped

	ROM_REGION( 0x00080, "seeprom", 0)
	ROM_LOAD( "st93c46c.u30",                     0x00000, 0x00080, CRC(22cac7b5) SHA1(ee48ecf5d59e243e9afb0ca7e41ed8437eec8097) ) // BIOS settings
ROM_END


} // anonymous namespace


//    YEAR, NAME,          PARENT, COMPAT, MACHINE,      INPUT,       CLASS,             INIT,       COMPANY, FULLNAME,         FLAGS
COMP( 1995, thinkpad760xd, 0,      0,      thinkpad760,  thinkpad760, thinkpad760_state, empty_init, "IBM",   "ThinkPad 760XD", MACHINE_IS_SKELETON )
