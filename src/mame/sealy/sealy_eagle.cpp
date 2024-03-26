// license:BSD-3-Clause
// copyright-holders:

/*******************************************************************
Qing Cheng Zhi Lian, Sealy, 2011
Hardware Info by Guru
---------------------

This is a high-res Mahjong game that outputs VGA at 31.5kHz running on emulated PC-like hardware.
The game takes approximately 1 minute to boot while something is copied somewhere
(either NAND or the main ROM). Perhaps the main ROM is uploaded to the FPGA or files
in the NAND (MSDOS File System) are loaded into RAM and unpacked.
While this happens the display shows LOADING while the text slowly fills with blue
color from left to right as it loads.
The MSDOS file system in the NAND suggest this board might be emulating some kind of basic x86 PC.


Sealy 110415-EGMJ-V1.2
  |-----------------------------------------------|
  |  CR2032          1117-1.5        VOL  TDA1519 |
  |1117-5.0      |---------|                    J3|
|-|              | ACTEL   |  U40     DA1311    S2|  J4
|     62LV256    | FUS10N  |              MAX1487E|-|DB9
|                | AFS250  |      HY04        SW3 | |RS485
|                |         | 3.579545MHz          |-|SERIAL
|                |---------|        32.768kHz     |
|J                    33MHz             DS1302    |
|A             14.31818MHz          VR3           |
|M                   |----------|               J2|
|M      42S16160     | EAGLE    |    42S16160     |
|A                   | PKM32AG-Q|                 |
|                    | PEAK     |                 |
|       42S16160     | MICROTECH|    42S16160     |-|J10
|                    |----------|                 | |HD15
|-|                                               |-|VGA
  |                         U43                   |
  |         EL817(x20)             1086-33      J8|
  |SW1                              LM1085  J7    |
  |-----|    18WAY       |----| 10WAY   |---------|
        |----------------|    |---------|
Notes:
      EAGLE - "PEAK MICROTECH EISC EAGLE PKM32AG-Q" SoC with 32bit EISC(AE32000C) Processor Core
     FUS10N - Actel FUS10N AFS250 FPGA
    62LV256 - 32kB x8-bit SRAM (battery-backed)
   42S16160 - ISSI IC42S16160H-7TL 4M x 16Bit x 4 Banks (256-Mbit) SDRAM
     DA1311 - Philips TDA1311 Stereo DAC
   MAX1487E - Maxim MAX1487E RS-485/RS-422 Transceiver
       HY04 - Protection DIP8 IC, possibly a PIC or other microcontroller. Clock input 3.579545MHz
     DS1302 - Dallas DS1302 Real Time Clock
        SW1 - Test Switch
        SW3 - 2-position DIP Switch (both ON)
    TDA1519 - TDA1519 6W Stereo Audio Power Amplifier
   1117-5.0 - AMS1117 5.0 5.0V LDO Regulator
   1117-1.5 - AMS1117 1.5 1.5V LDO Regulator
      EL817 - Everlight EL817 Photocoupler (same as Sharp PC817)
    1086-33 - LM1086-33 3.3V 1.5A LDO Regulator
     LM1085 - LM1085 IS-1.8 1.8V 3A LDO Regulator
     CR2032 - 3V Coin Battery
         S2 - Not-populated slide switch marked 'CGA VGA'
         J2 - Not-populated PS/2 port
         J3 - Not-populated stereo 3.5mm jack
         J7 - Not-populated DB9 serial connector
         J8 - USB-B connector
        VOL - Volume Pot
        VR3 - Potentiometer for screen brightness
        U40 - Sharp LH28F160BJD-TTL80 1M x 16-bit Flash ROM (DIP42)
        U43 - Samsung Electronics K9F5608U0D 256Mbit (32MB x8-bit) NAND Flash ROM (TSOP48)
              This is being used like a hard drive. At 0x00108000h (block 64) there's an MSDOS 5.0 FAT16 File System

Note:
- same protection chip as misc/menghong.cpp
- seems close to misc/crospuzl.cpp(EAGLE and Amazon-LF are two EISC based SOCs by ADC)
*******************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "machine/ds1302.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class sealy_eagle_state : public driver_device
{
public:
	sealy_eagle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{}

	void qczl(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t sealy_eagle_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void sealy_eagle_state::program_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
}


static INPUT_PORTS_START( qczl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void sealy_eagle_state::qczl(machine_config &config)
{
	ARM9(config, m_maincpu, 14.318181_MHz_XTAL); // placeholder for unemulated SoC
	m_maincpu->set_addrmap(AS_PROGRAM, &sealy_eagle_state::program_map);

	DS1302(config, "rtc", 32.768_kHz_XTAL);

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(sealy_eagle_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x100); // wrong

	SPEAKER(config, "mono").front_center();
}


ROM_START( qczl )
	ROM_REGION( 0x2000000, "maincpu", 0 )
	ROM_LOAD( "prog.u40", 0x000000, 0x200000, CRC(f5a39de0) SHA1(994251f4cf011f8465d690865d47ea4532661ff6) )

	ROM_REGION( 0x21000000, "nand", 0 )
	ROM_LOAD( "nand.u43", 0x0000000, 0x2100000, CRC(6accbd37) SHA1(6ab206ab23dda23a30a4b8161497c4846ae3ea14) )

	ROM_REGION( 0x4280, "pic", 0 ) // HY04
	ROM_LOAD( "hy04", 0x000000, 0x4280, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 2011, qczl, 0, qczl, qczl, sealy_eagle_state, empty_init, ROT0, "Sealy", "Qing Cheng Zhi Lian", MACHINE_IS_SKELETON )
