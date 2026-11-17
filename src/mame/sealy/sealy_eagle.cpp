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
- seems close to misc/crospuzl.cpp (EAGLE and Amazon-LF are two EISC based SOCs by ADC)
- the AE32000 core isn't compatible with the SE3208 (16 registers instead of 8, different encodings)

The program flash goes through the Actel (see decrypt()). The decrypted program starts with
the ADC USB bootloader v1.21, which checks the NAND, SRAM and texture RAM and then loads the
game.

TODO:
- the decryption was worked out from the program (vector table, call targets, strings), the
  key of the areas holding only data can't be verified this way:
  - qczl: 0xa603e-0xa643b, zduoxing: 0xda88-0xde87 and 0x2c8ee-0x2cceb are tables of 256
    words that also give sensible values with another key (0x981a, 0xa4d3 and 0x0449);
  - abaozang: 0x50000 onwards is assumed to be unencrypted, 0x6b7ae-0x6c173 also gives
    sensible values with 0x1044;
  - zduoxing: the key of 0x2d236 onwards (random looking data) is a guess;
- emulate the EAGLE SoC (AE32000C core).
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

	void init_qczl() ATTR_COLD;
	void init_abaozang() ATTR_COLD;
	void init_zduoxing() ATTR_COLD;

private:
	struct key_range { uint32_t end; uint16_t key; };

	required_device<cpu_device> m_maincpu;

	template <typename A, typename D> void decrypt(A &&addr_swap, D &&data_swap, std::initializer_list<key_range> keys) ATTR_COLD;

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
	screen_device &screen(SCREEN(config, "screen"));
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
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "prog.u40", 0x000000, 0x200000, CRC(f5a39de0) SHA1(994251f4cf011f8465d690865d47ea4532661ff6) )

	ROM_REGION( 0x2100000, "nand", 0 )
	ROM_LOAD( "nand.u43", 0x0000000, 0x2100000, CRC(6accbd37) SHA1(6ab206ab23dda23a30a4b8161497c4846ae3ea14) )

	ROM_REGION( 0x4280, "pic", 0 ) // HY04
	ROM_LOAD( "hy04", 0x000000, 0x4280, NO_DUMP )
ROM_END

// 埃及宝藏 / Āijí Bǎozàng / Egyptian Treasure
ROM_START( abaozang )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "treasure_egypt.u54", 0x000000, 0x200000, CRC(3a67685c) SHA1(1935bf85662a76b66ce76f6a8f98af6c633d922c) )

	ROM_REGION( 0x2100000, "nand", 0 )
	ROM_LOAD( "utl0g4q1c.u58", 0x0000000, 0x2100000, CRC(03cad573) SHA1(bee19526aac2c8157bc4d547f3d98ea6537a29b9) )

	ROM_REGION( 0x4280, "pic", 0 ) // HY04
	ROM_LOAD( "hy04", 0x000000, 0x4280, NO_DUMP )
ROM_END

// 指多星 / Zhǐ Duō Xīng / Finger Star
ROM_START( zduoxing )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "zhiduoxing.u40", 0x000000, 0x200000, CRC(837da815) SHA1(a2d1b079bd4e0ff564048b2badcaefcc72d373cb) )

	ROM_REGION( 0x2100000, "nand", 0 )
	ROM_LOAD( "utf022pac.u43", 0x0000000, 0x2100000, CRC(b81ab264) SHA1(f9930a466bc8828800a62e292f6eab6f9300e24b) )

	ROM_REGION( 0x4280, "pic", 0 ) // HY04
	ROM_LOAD( "hy04", 0x000000, 0x4280, NO_DUMP )
ROM_END


// The Actel between the flash and the SoC scrambles the address lines, XORs the data with a key
// chosen by comparing the CPU address with a list of boundaries (different for each game, not
// aligned to any power of two) and scrambles the data lines. The parts of the flash the program
// doesn't use are filled with random 15-bit values.
template <typename A, typename D>
void sealy_eagle_state::decrypt(A &&addr_swap, D &&data_swap, std::initializer_list<key_range> keys)
{
	memory_region *const region = memregion("maincpu");
	uint8_t *const rom = region->base();
	std::vector<uint8_t> const src(rom, rom + region->bytes());

	for (uint32_t i = 0; i < region->bytes(); i += 2)
	{
		uint16_t key = 0;
		for (auto const &k : keys)
		{
			if (i < k.end)
			{
				key = k.key;
				break;
			}
		}

		uint32_t const a = addr_swap(i);
		uint16_t const data = data_swap((src[a] | (src[a + 1] << 8)) ^ key);
		rom[i] = data & 0xff;
		rom[i + 1] = data >> 8;
	}
}

void sealy_eagle_state::init_qczl()
{
	decrypt(
			[] (uint32_t a) { return bitswap<21>(a, 9, 6, 16, 14, 5, 20, 15, 13, 2, 11, 17, 12, 8, 19, 3, 10, 4, 7, 18, 1, 0); },
			[] (uint16_t x) { return bitswap<16>(x, 2, 4, 3, 9, 1, 11, 5, 13, 14, 12, 0, 6, 10, 15, 8, 7); },
			{ { 0x01e346, 0xdb91 }, { 0x03bd78, 0xc507 }, { 0x0567ee, 0x4b47 }, { 0x081776, 0x3aad }, { 0x200000, 0x9c8e } });
}

void sealy_eagle_state::init_abaozang()
{
	decrypt(
			[] (uint32_t a) { return bitswap<21>(a, 10, 5, 16, 20, 7, 14, 2, 13, 9, 11, 19, 15, 8, 17, 3, 18, 4, 12, 6, 1, 0); },
			[] (uint16_t x) { return bitswap<16>(x, 6, 2, 10, 15, 4, 1, 5, 13, 8, 0, 11, 3, 7, 12, 9, 14); },
			{ { 0x00e420, 0x1ebc }, { 0x015874, 0xc6a7 }, { 0x019bf0, 0x5ed2 }, { 0x027c9a, 0x81e2 }, { 0x03a738, 0xf449 },
			  { 0x04215a, 0x6c29 }, { 0x050000, 0x9135 }, { 0x200000, 0x0000 } });
}

void sealy_eagle_state::init_zduoxing()
{
	decrypt(
			[] (uint32_t a) { return bitswap<21>(a, 6, 17, 20, 7, 15, 5, 2, 13, 10, 11, 19, 3, 8, 9, 14, 18, 4, 12, 16, 1, 0); },
			[] (uint16_t x) { return bitswap<16>(x, 2, 11, 6, 4, 12, 3, 13, 0, 5, 9, 1, 15, 14, 8, 7, 10); },
			{ { 0x00937e, 0x3590 }, { 0x0164da, 0xb887 }, { 0x01eb00, 0xa4ad }, { 0x024666, 0x0c49 }, { 0x02a8d2, 0x1b45 },
			  { 0x200000, 0xa4d3 } });
}

} // anonymous namespace


GAME( 2011, qczl,     0, qczl, qczl, sealy_eagle_state, init_qczl,     ROT0, "Sealy", "Qing Cheng Zhi Lian", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 201?, abaozang, 0, qczl, qczl, sealy_eagle_state, init_abaozang, ROT0, "Sealy", "Aiji Baozang",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 201?, zduoxing, 0, qczl, qczl, sealy_eagle_state, init_zduoxing, ROT0, "Sealy", "Zhi Duo Xing",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
