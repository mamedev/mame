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
2x 34-pin connector
Master / Slave switch (PCB silkscreened SW1)
Off / On switch (PCB silkscreened SW2)
bank of 4 DIP switches (PCB silkscreened DIP1)
BU9480 stereo D/A converter
NJM2904 dual operational amplifier
TA8213K audio power amplifier


TODO:
- sound;
- understand vregs area;
- EEPROM;
- hopper / counters / lamps;
- progressive feature.
*/


#include "emu.h"

#include "cpu/m68000/m68020.h"
#include "machine/eepromser.h"
#include "machine/ins8250.h"
#include "machine/nvram.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class clownmgc_state : public driver_device
{
public:
	clownmgc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_spriteram(*this, "spriteram")
	{ }

	void clownmgc(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint32_t> m_spriteram;

	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scantimer_cb);

	void program_map(address_map &map) ATTR_COLD;
};


void clownmgc_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs <= m_spriteram.bytes() / 2 - 2; offs += 2)
	{
		uint16_t const sprite = (m_spriteram[offs + 1] & 0xffff);
		int16_t const x = (m_spriteram[offs] >> 16);
		int16_t const y = (m_spriteram[offs] & 0xffff);
		uint16_t const attr = m_spriteram[offs + 1] >> 16;
		uint16_t const color = attr & 0xff;

		// TODO: attr & 0xff00

		if (BIT(sprite, 15))
			return;

		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, sprite, color, 0, 0, x, y, 0);
	}
}

uint32_t clownmgc_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	draw_sprites(bitmap, cliprect);

	return 0;
}


void clownmgc_state::program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x000000, 0x07ffff).rom();
	map(0x400000, 0x41ffff).ram().share("nvram");
	map(0x500000, 0x50000f).rw("uart", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w)).umask32(0x00ff00ff);
	map(0x600000, 0x600003).portr("IN0");
	map(0x600004, 0x600007).portr("IN1").nopw(); // w lamps / counters?
	map(0x700000, 0x700003).nopw(); // watchdog?
	map(0x800000, 0x801fff).ram().share(m_spriteram);
	map(0x802000, 0x807fff).ram().share("unkram");
	map(0x808000, 0x80ffff).ram().w("palette", FUNC(palette_device::write32)).share("palette");
	map(0x810000, 0x81004f).ram().share("vregs");
	map(0x810022, 0x810023).lr16(NAME([] () -> uint16_t { return 0x0080; })); // TODO: some kind of ready bit? freezes after coin up without this
	map(0x810026, 0x810027).lr16(NAME([] () -> uint16_t { return 0x0080; })); // TODO: some kind of ready bit? freezes after start without this
}


static INPUT_PORTS_START( clownmgc )
	PORT_START("IN0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Max Bet")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Re-Bet")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Test")
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT(     0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // battery low if on
	PORT_BIT(     0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN ) // battery empty if on
	PORT_BIT(     0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x00000100, 0x00000100, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1") // maybe
	PORT_DIPSETTING(          0x00000100, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000200, 0x00000200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(          0x00000200, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000400, 0x00000400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(          0x00000400, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000800, 0x00000800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(          0x00000800, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT(     0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(     0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(     0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00100000, IP_ACTIVE_LOW, IPT_OTHER ) // hopper
	PORT_BIT(     0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x01000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(     0x20000000, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT(     0x40000000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Reset")
	PORT_BIT(     0x80000000, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Configuration")
INPUT_PORTS_END


static GFXDECODE_START( gfx_clownmgc )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x8_raw, 0, 0x100 )
GFXDECODE_END


TIMER_DEVICE_CALLBACK_MEMBER(clownmgc_state::scantimer_cb)
{
	int const scanline = param;

	if (scanline == 120)
		m_maincpu->set_input_line(M68K_IRQ_3, HOLD_LINE); // coins

	if (scanline == 0)
		m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE); // main
}


void clownmgc_state::clownmgc(machine_config &config)
{
	M68EC020(config, m_maincpu, 42.954545_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &clownmgc_state::program_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(clownmgc_state::scantimer_cb), m_screen, 0, 1);

	EEPROM_93C46_16BIT(config, "eeprom");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	NS16550(config, "uart", 0); // TODO: clock

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(40*8, 30*8);
	m_screen->set_visarea(0, 40*8-1, 0, 30*8-1);
	m_screen->set_screen_update(FUNC(clownmgc_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_clownmgc);

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 0x4000);

	SPEAKER(config, "mono").front_center();
}


ROM_START( clownmgc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "clownmagic_ms_103c.u72", 0x00000, 0x80000, CRC(5cdae921) SHA1(8d01aca34c089b704e1c47bcced2e634ab6a2f61) )

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD( "mx29f1610.u22", 0x000000, 0x200000, CRC(8bce8956) SHA1(31529730091394994492ed91a82661c0ef6bc25b) )
	ROM_LOAD( "mx29f1610.u4",  0x200000, 0x200000, CRC(523f431e) SHA1(b0c231e27571d9d612d72830cbe5b3e873668c9b) )
	ROM_LOAD( "mx29f1610.u24", 0x400000, 0x200000, CRC(10388b18) SHA1(51629743acda1a82b52970759963ac629e59051f) )
ROM_END


} // anonymous namespace


// possibly Progressive isn't part of the title
GAME( 2002, clownmgc, 0, clownmgc, clownmgc, clownmgc_state, empty_init, ROT0, "Visco", "Progressive Clown Magic (ver. 1.03c)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
