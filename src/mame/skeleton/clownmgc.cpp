// license:BSD-3-Clause
// copyright-holders:

/*
Clown Magic by Visco
Distributed by Able
String in ROM says '2002.11.20'
Video reference: https://www.youtube.com/watch?v=h88O2nWpjbo

MS-02B Main PCB

MC68EC020FG16C CPU (PCB silkscreened MC68EC020FG)
MX27C4100DC-12 (program ROM, silkscreened MX27C4100)
33.33 MHz XTAL (near program ROM)
AT93C46F EEPROM
240-pin, scratched off chip (PCB silkscreened VDC)
42.95454 MHz XTAL (between CPU and VDC)
2x T224160B-35J RAM (on the left of the VDC)
208-pin, scratched off chip (PCB silkscreened GUL)
3x MX29F1610AMC-90 (GFX ROMs, PCB silkscreened MX29F1610)
CXD1178Q RGB 3-channel D/A Converter
4x TC55257DFTL SRAM (near CPU, PCB silkscreened BS62LV256)
2x T14M256A SRAM (between VCU and GUL)
3x TD62083AF Darlington driver (PCB silkscreened TD62083F)
TA550C ACE (PCB silkscreened TL16C550)
MM1027 system reset (battery backed-up)
COM IN port
COM OUT port
2x  34-pin connector
Master / Slave switch (PCB silkscreened SW1)
Off / On switch (PCB silkscreened SW2)
bank of 4 DIP switches (PCB silkscreened DIP1)
BU9480 stereo D/A converter
NJM2904 dual operational amplifier
TA8213K audio power amplifier
*/


#include "emu.h"

#include "cpu/m68000/m68020.h"
#include "machine/eepromser.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class clownmgc_state : public driver_device
{
public:
	clownmgc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void clownmgc(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t clownmgc_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void clownmgc_state::video_start()
{
}


void clownmgc_state::program_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x400000, 0x41ffff).ram();
	map(0x800000, 0x80ffff).ram();
}


static INPUT_PORTS_START( clownmgc )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_clownmgc ) // TODO
	GFXDECODE_ENTRY( "tiles", 0, gfx_16x16x8_raw, 0, 16 )
	GFXDECODE_ENTRY( "reels", 0, gfx_16x16x8_raw, 0, 16 ) // wrong
GFXDECODE_END


void clownmgc_state::clownmgc(machine_config &config)
{
	M68EC020(config, m_maincpu, 42.954545_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &clownmgc_state::program_map);
	//m_maincpu->set_vblank_int("screen", FUNC(clownmgc_state::irq2_line_hold));

	EEPROM_93C46_16BIT(config, "eeprom");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 64*8-1);
	screen.set_screen_update(FUNC(clownmgc_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_clownmgc);

	PALETTE(config, "palette", palette_device::BGR_555); // TODO

	SPEAKER(config, "mono").front_center();
}


ROM_START( clownmgc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "clownmagic_ms_103c.u72", 0x00000, 0x80000, CRC(5cdae921) SHA1(8d01aca34c089b704e1c47bcced2e634ab6a2f61) )

	ROM_REGION( 0x400000, "tiles", 0 )
	ROM_LOAD( "mx29f1610.u22", 0x000000, 0x200000, CRC(8bce8956) SHA1(31529730091394994492ed91a82661c0ef6bc25b) )
	ROM_LOAD( "mx29f1610.u24", 0x200000, 0x200000, CRC(10388b18) SHA1(51629743acda1a82b52970759963ac629e59051f) )

	ROM_REGION( 0x200000, "reels", 0 )
	ROM_LOAD( "mx29f1610.u4", 0x000000, 0x200000, CRC(523f431e) SHA1(b0c231e27571d9d612d72830cbe5b3e873668c9b) )
ROM_END


} // anonymous namespace

GAME( 2002, clownmgc, 0, clownmgc, clownmgc, clownmgc_state, empty_init, ROT0, "Visco", "Progressive Clown Magic", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
