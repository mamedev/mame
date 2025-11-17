// license:BSD-3-Clause
// copyright-holders:

/*******************************************************************
Tong Zi Maque
Ster, 2003 (might be incorrect?)
Hardware info By Guru
---------------------

No PCB number
"MADE IN SPIDER" on some ROMs and spider web logo on PCB
MADE IN TAIWAN 2003
|--------------------------------------|
|BATT    62256     Z80-PROG.U2         |
|           62256    15MHz  Z80        |-|
|                           DS12B887.U12 |
|GFX1.U6                       13.560MHz |
|                                        |
|     GFX2.U5                           M|
|           |--------|         TLP620-4 A|
|           |ALTERA  |         TLP620-4 H|
|           |EPM7256 |   |--------|     J|
|           |SQC208  |   |ALTERA  |     O|
|           |--------|   |EPM7192 |     N|
|                        |SQC160  |     G|
|                        |--------|      |
|        U6295                93C46.U11  |
|78L05  29F002.U9   BUTTON    TLP620-4   |
|uPC1241H      T518A   TLP620-4          |
| VOL                                  |-|
|-|  10WAY  |-----|       22WAY        |
  |---------|     |--------------------|
Notes:
         Z80 - Zilog Z84C0008 Z80 CPU. Clock 13.560MHz
       U6295 - OKI M6295. Clock 1.13MHz [13.560/12]. Pin 7 HIGH.
       62256 - 32kB x8-bit SRAM (the one nearest to the Z80 is battery-backed)
      BUTTON - Reset? Test?
    TLP620-4 - Toshiba TLP620-4 Photocoupler. Package contains 4 driver/receiver pairs.
     EMP7256 - Altera Max 7000-Series EPM7256SQC208 Programmable Logic Device
     EMP7192 - Altera Max 7000-Series EPM7192SQC160 Programmable Logic Device
    uPC1241H - NEC uPC1241H Mono Audio Power Amp
       T518A - PST518A System Reset IC
       78L05 - 5V Regulator in TO92 package
       U5/U6 - 27C160 EPROM (graphics)
   93C46.U11 - Atmel AT93C46 EEPROM (128 bytes) (for game settings or high scores?)
 Z80-PROG.U2 - Winbond W29EE011 EEPROM (Z80 program). The databus on this ROM connects to the EPM7192 and the Z80 databus is also connected
               to the EPM7192 on different pins. Additionally there's no valid Z80 code and no plain text in the ROM. This all leads to it
               being highly likely that the program is encrypted.
      29F002 - AMD AM29F002 PLCC32 (surface-mounted) EEPROM. This is connected to the OKI chip and holds the audio samples.
DS12B887.U12 - Dallas DS12B887 Real Time Clock Module. This looks suspiciously like it's used for protection data? The PCB contains a battery
               (for game settings or high scores?) and a 93C46 EEPROM (for game settings or high scores?). No other data needs to be stored
               anywhere. The DS12B887 is connected to the EPM7192 chip. It's possible it might just be for time/bookkeeping. The chip
               (dated 9614) is dumped but I'd be surprised if the battery inside was still alive and the data is good, although the data isn't
               just garbage and reads the same each time so by some miracle it might be ok.
               The first 14 bytes are clock and control registers. The remaining space is general-purpose battery-backed RAM. These same chips
               were used often in PCs around the 286/386/486 era to store the CMOS data. The RAM can technically hold anything, maybe even
               a decryption key....

*******************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/ds128x.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class sterz80_state : public driver_device
{
public:
	sterz80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void tongzi(machine_config &config) ATTR_COLD;

	void init_tongzi() ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


uint32_t sterz80_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void sterz80_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void sterz80_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
}


static INPUT_PORTS_START( tongzi )
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


static const gfx_layout gfx_8x8x16 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0, 2) },
	{ STEP8(0, 8*2) },
	{ STEP8(0, 8*8*2) },
	8*8*16
};

// TODO: enough to see something, probably needs adjustments
static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x16, 0, 1 )
GFXDECODE_END


void sterz80_state::tongzi(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 13.560_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sterz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &sterz80_state::io_map);
	// m_maincpu->set_vblank_int("screen", FUNC(sterz80_state::irq0_line_hold));

	EEPROM_93C46_8BIT(config, "eeprom");

	DS12885(config, "rtc", 32.768_kHz_XTAL); // TODO: should be DS12B887

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: verify everything once emulation works
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(sterz80_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 13.560_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( tongzi )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.u2", 0x00000, 0x20000, CRC(8f394f91) SHA1(e49c682cb819885334c1f25d5221e3f59c21a7e2) ) // encrypted

	ROM_REGION( 0x400000, "tiles", 0 )
	ROM_LOAD( "1.u6", 0x000000, 0x200000, CRC(e885dcc9) SHA1(ea4a72eec7b65cb668c5abb64347426192bd0f86) )
	ROM_LOAD( "2.u5", 0x200000, 0x200000, CRC(a7c11185) SHA1(96c9479802db0594845b2b282553c73406b84bb0) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u11", 0x00, 0x80, CRC(bc385e5a) SHA1(b6a2c02e3b558aff2fb2a3b84ea307ad0996a70c) )

	ROM_REGION( 0x80, "rtc", 0 )
	ROM_LOAD( "ds12887.u12", 0x00, 0x80, CRC(520e6efb) SHA1(f4e59ac6a519d310e8362fd088f50c7ee7f970e1) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "am29f002.u9", 0x00000, 0x40000, CRC(4bbd8cfa) SHA1(690d7a98764162b0771629c02fd3c488761d8ec0) ) // encrypted
ROM_END


void sterz80_state::init_tongzi()
{
	// decrypt main CPU ROM
	// TODO
	// the encryption seems to be based on address-determined XORs and data bitswaps. At a first glance the XORs seem to be chosen
	// by bits 0, 1, 4, 5, 6 and 7 of the address, the data bitswaps by bits 0 and 1 of the address.

	// decrypt M6295 ROM
	uint8_t *okirom = memregion("oki")->base();
	std::vector<uint8_t> buffer(0x40000);

	memcpy(&buffer[0], okirom, 0x40000);

	for (int i = 0; i < 0x40000; i++)
	{
		okirom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 4, 3, 1, 2, 5, 0, 10, 13, 8, 6, 15, 17, 7, 9, 12, 14, 11, 16)];
		okirom[i] = bitswap<8>(okirom[i], 0, 1, 2, 3, 4, 5, 6, 7);
	}
}

} // anonymous namespace


GAME( 2003, tongzi, 0, tongzi, tongzi, sterz80_state, init_tongzi, ROT0, "Ster", "Tong Zi Maque", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
