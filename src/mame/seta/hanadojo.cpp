// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

花道場 (Hana Doujou) - Alba 1984
AAJ-11 PCB
Test Mode tests a "N-Board I/O"

TODO:
- Uses a form of NVRAM thrash protection, not completely understood:
\- bp b6e sub-routine: loops with ld hl,$8f14 de,$0005 expecting all values to be 0x55 or 0xaa;
\- writes a 0 to $8f28 the 3rd time around, expecting one of the IOX to prevent that;
\- some code uploaded in RAM at $8c00, including an IOX data_w 0x60;
\- wpset 0x8f00,0x100,r,PC!=0xb91
\- program and I/O accesses to $b8xx / $bcxx areas;
- Resets itself at the end of first attract hand & coin-in sequences
\- bp 1a3,1,{a=4;g} to bypass;
- Hookup NMI for coin chutes;
- Hopper plus other bits and bobs ("noise detection" antenna?);
- CRTC processes params as 1088x224 with char width 8;
- hanadojoa: slightly more protected, keeps resetting at IOX tests;

===================================================================================================

Main components are:

NEC D780C CPU
12 MHz XTAL (near CPU)
NEC D449C RAM (near CPU ROMs)
unknown 40-pin chip (near CPU, stickered AN-001 on one PCB)
2x Toshiba TMM2009P-B RAM (near GFX ROMs)
HD46505SP CRTC
2x bank of 8 DIP switches
bank of 4 DIP switches
AY-3-8910
unknown 40-pin chip (stickered AN-002)
unknown 40-pin chip (stickered AN-003)
2x AX-014 epoxy covered chips
AX-013 epoxy covered chip

===================================================================================================

CN1 connects to hopper drive NS-01
CN2/CN3 connects to 1P/2P control panels

**************************************************************************************************/

#include "emu.h"

#include "iox_hle.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class hanadojo_state : public driver_device
{
public:
	hanadojo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_iox(*this, "iox_%u", 1U)
		, m_nvram(*this, "nvram")
		, m_dswb(*this, "DSWB")
		, m_dswc(*this, "DSWC")
		, m_patsw(*this, "PATSW")
		, m_gfxdecode(*this, "gfxdecode")
		, m_videoram(*this, "videoram")

	{ }

	void hanadojo(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<iox_hle_device, 2> m_iox;
	required_device<nvram_device> m_nvram;
	required_ioport m_dswb;
	required_ioport m_dswc;
	required_ioport m_patsw;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_videoram;

	void palette_init(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	bool m_nvram_lock;
	std::unique_ptr<uint8_t[]> m_nvram_ptr;
};

void hanadojo_state::palette_init(palette_device &palette) const
{
	const u8 *color_prom = memregion("color_prom")->base();
	const u8 *clut_prom = memregion("clut_prom")->base();

	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		bit0 = 0;
		bit1 = (color_prom[i] >> 0) & 0x01;
		bit2 = (color_prom[i] >> 1) & 0x01;
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 2) & 0x01;
		bit1 = (color_prom[i] >> 3) & 0x01;
		bit2 = (color_prom[i] >> 4) & 0x01;
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 5) & 0x01;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// assume bit 4 coming from external bank
	for (int i = 0; i < 0x200; i++)
	{
		palette.set_pen_indirect(i, clut_prom[i & 0xff] | BIT(i, 8) << 4);
	}
}


void hanadojo_state::video_start()
{
}

uint32_t hanadojo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// throwaway sample code
	gfx_element *gfx = m_gfxdecode->gfx(0);

	const u16 pitch = 34;
	const u32 cclk_x = 0x20000;
	const int dx = 16;

	for (int y = 0; y < 28; y++)
	{
		u16 base_address = (y * pitch) << 1;
		for (int x = 0; x < pitch; x++)
		{
			const u16 tile_address = base_address + (x << 1);
			const u16 value = m_videoram[tile_address] | (m_videoram[tile_address + 1] << 8);
			u16 tile = value & 0xff;
			tile |= (value & 0xe000) >> 5;
			u8 color = (value >> 8) & 0x1f;

			gfx->zoom_opaque(bitmap, cliprect, tile, color, 0, 0, x * dx, y * 8, cclk_x, 0x10000);
		}
	}

	return 0;
}


void hanadojo_state::program_map(address_map &map)
{
	// ldir 0-0x1ff at POST
	map(0x0000, 0x7fff).rom().nopw();
	map(0x8800, 0x8eff).ram();
	map(0x8f00, 0x8fff).lrw8(
		NAME([this] (offs_t offset) { return m_nvram_ptr[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			// NOTE: not all of it, would otherwise never decrease coin counter in-game
			if (!m_nvram_lock || (offset & 0x28) != 0x28)
				m_nvram_ptr[offset] = data;
		})
	);
	map(0x9000, 0x97ff).ram().share(m_videoram);
}

void hanadojo_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).rw("ay", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0x0001, 0x0001).w("ay", FUNC(ay8910_device::data_w));

	map(0x0020, 0x0020).w("crtc", FUNC(hd6845s_device::address_w));
	map(0x0021, 0x0021).w("crtc", FUNC(hd6845s_device::register_w));
	// TODO: 2x IOX!
	// player inputs, PATSW 2
	map(0x0040, 0x0040).rw(m_iox[0], FUNC(iox_hle_device::data_r), FUNC(iox_hle_device::data_w));
	map(0x0041, 0x0041).rw(m_iox[0], FUNC(iox_hle_device::status_r), FUNC(iox_hle_device::command_w));

	// coinage (cfr. init at $61), service buttons, dip bank B
	map(0x0060, 0x0060).rw(m_iox[1], FUNC(iox_hle_device::data_r), FUNC(iox_hle_device::data_w));
	map(0x0061, 0x0061).rw(m_iox[1], FUNC(iox_hle_device::status_r), FUNC(iox_hle_device::command_w));

	map(0x0080, 0x0080).nopw(); // very noisy, just watchdog?

	map(0x00a0, 0x00a3).lr8(
		NAME([this] (offs_t offset) {
			const u8 bankb_select = BIT(offset, 1) ?
				BIT(m_patsw->read(), offset & 1) :
				BIT(m_dswb->read(), offset + 6);
			return (bankb_select) | (BIT(m_dswc->read(), offset) << 1);
		})
	);
}


// TODO: may make more sense to just use IPT_POKER_HOLD* instead.
static INPUT_PORTS_START( hanadojo )
	PORT_START("P1_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_PLAYER(1) PORT_NAME("P1 Cancel") // CAN
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(1)

	PORT_START("P1_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_PLAYER(1) PORT_NAME("P1 Play / Bet") // PLY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_PLAYER(1) PORT_NAME("P1 Deal") // DEA
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(1)

	PORT_START("P1_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_PLAYER(2) PORT_NAME("P2 Cancel")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(2)

	PORT_START("P2_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_PLAYER(2) PORT_NAME("P2 Play / Bet")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_PLAYER(2) PORT_NAME("P2 Deal")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(2)

	PORT_START("P2_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(2)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PATSW")
	PORT_DIPNAME( 0x01, 0x01, "PATSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "PATSW1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWA")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SWA:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SWA:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SWA:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SWA:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SWA:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SWA:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SWA:7")
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_HIGH, "SWA:8")

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SWB:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SWB:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SWB:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SWB:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SWB:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SWB:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SWB:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SWB:8")

	PORT_START("DSWC")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SWC:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SWC:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SWC:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SWC:4")

	// NOTE: DSWD is 4 bit banks, near AY on board.
	// Game has no clear bank display compared to the others, assume one is SW2 nearby.
	PORT_START("DSWD")
	PORT_DIPNAME( 0x01, 0x00, "ARS" ) // Credit Clear
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "RES" ) // RST?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) // "M M" -> Memory Mode
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) // "CLR"
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYIN )
	PORT_DIPNAME( 0x20, 0x00, "MDL" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	// vestigial, just to have one for now
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
INPUT_PORTS_END


static GFXDECODE_START( gfx_hanadojo )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x3_planar, 0, 32 * 2 )
GFXDECODE_END

void hanadojo_state::machine_start()
{
	save_item(NAME(m_nvram_lock));

	const u32 nvram_size = 0x100;
	m_nvram_ptr = std::make_unique<uint8_t[]>(nvram_size);
	m_nvram->set_base(m_nvram_ptr.get(), nvram_size);

	save_pointer(NAME(m_nvram_ptr), nvram_size);
}

void hanadojo_state::machine_reset()
{
	m_nvram_lock = false;
}

void hanadojo_state::hanadojo(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 2); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &hanadojo_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &hanadojo_state::io_map);

	IOX_HLE(config, m_iox[0], 0);
	m_iox[0]->p1_key_input_cb<0>().set_ioport("P1_KEY0");
	m_iox[0]->p1_key_input_cb<1>().set_ioport("P1_KEY1");
	m_iox[0]->p1_key_input_cb<2>().set_ioport("P1_KEY2");
	m_iox[0]->p2_key_input_cb<0>().set_ioport("P2_KEY0");
	m_iox[0]->p2_key_input_cb<1>().set_ioport("P2_KEY1");
	m_iox[0]->p2_key_input_cb<2>().set_ioport("P2_KEY2");

	IOX_HLE(config, m_iox[1], 0);
	m_iox[1]->p1_direct_input_cb().set_ioport("DSWD");
	// TODO: looks latch inverted compared to the $a0-$a1 version
	m_iox[1]->p2_direct_input_cb().set([this] () { return (m_dswb->read() & 0x3f) ^ 0x3f; });

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	hd6845s_device &crtc(HD6845S(config, "crtc", 12_MHz_XTAL / 4)); // divider guessed
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(4);
	crtc.out_vsync_callback().set_inputline(m_maincpu, 0);

	PALETTE(config, "palette", FUNC(hanadojo_state::palette_init), 0x100 * 2, 0x20);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_hanadojo);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// TODO: default values
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(hanadojo_state::screen_update));
	screen.set_palette("palette");

	SPEAKER(config, "speaker").front_center();

	ay8910_device &ay(AY8910(config, "ay", 12_MHz_XTAL / 16)); // divider guessed
	ay.port_a_write_callback().set([this] (u8 data) {
		// avoid locking the NVRAM on device startup
		if (data == 0xff)
			return;
		m_nvram_lock = !!BIT(data, 7);
		// bit 5 is flip screen, others unknown
		logerror("AY Port A: %02x\n", data);
	});
	ay.port_b_read_callback().set_ioport("DSWA");
	ay.add_route(ALL_OUTPUTS, "speaker", 0.33);
}

ROM_START( hanadojo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "n0_1.2l",  0x0000, 0x2000, CRC(fedc7a24) SHA1(75baed5c4f86185a24c8a9995a246246fda480d2) )
	ROM_LOAD( "n10_2.2k", 0x2000, 0x2000, CRC(e9fd71d5) SHA1(dbd5ed3bf81e507ca9a08c06fe22060a03bf3eed) )
	ROM_LOAD( "n10_3.2h", 0x4000, 0x2000, CRC(8dc494f5) SHA1(23df7c564f9b33c6c3cffb7be5dbf3c025468e3e) )
	ROM_LOAD( "n10_4.2f", 0x6000, 0x2000, CRC(ecbbfe7f) SHA1(0425e5b71a07f93d1562be158a7c33041f025fdc) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "no6.4f",  0x0000, 0x2000, CRC(f049bc57) SHA1(cb6baaa4eaf9306a54ec39689e3b16ab7acc82c7) )
	ROM_LOAD( "no7.4e",  0x4000, 0x2000, CRC(2ff0c6c7) SHA1(a92926d497189fadb11d0c8fd12d8e4a1506c4fd) )
	ROM_LOAD( "no8.4d",  0x8000, 0x2000, CRC(32ed9c86) SHA1(6614df331ab8e57327b27393d189cc538f1b9567) )
	ROM_LOAD( "no9.6f",  0x2000, 0x2000, CRC(1e5f720e) SHA1(e3d55fae723625fcdd78ee02fb10e47d8e0628f0) )
	ROM_LOAD( "no10.6e", 0x6000, 0x2000, CRC(bfb79118) SHA1(191c441b60fdec714733e326e7ad984b551e2cce) )
	ROM_LOAD( "no11.6d", 0xa000, 0x2000, CRC(a601d401) SHA1(ac10da18c6ef46d9c9da10e292dcb49554676885) )

	ROM_REGION( 0x020, "color_prom", 0 )
	ROM_LOAD( "n1.11d", 0x000, 0x020, CRC(1e6f668a) SHA1(6006ee30920e51581862b0e7f56ac724831b0034) )

	ROM_REGION( 0x100, "clut_prom", 0 )
	ROM_LOAD( "n2.9d",  0x000, 0x100, CRC(e6812f63) SHA1(2286b43970e51d6cfbceaaf74bcb6d2f35620d3a) )
ROM_END

ROM_START( hanadojoa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "n1_1.2l",  0x0000, 0x2000, CRC(25c6360f) SHA1(d38a01471264f00f5ed6b4bb831620b13e8be411) )
	ROM_LOAD( "n20_2.2k", 0x2000, 0x2000, CRC(e9fd71d5) SHA1(dbd5ed3bf81e507ca9a08c06fe22060a03bf3eed) )
	ROM_LOAD( "n20_3.2h", 0x4000, 0x2000, CRC(094b55c9) SHA1(a1735ed788af778a3da358069af8567a3724aa0d) )
	ROM_LOAD( "n20_4.2f", 0x6000, 0x2000, CRC(ca47a101) SHA1(b31488e5102cd7f576bfc3ee4253e0fb752e72c9) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "no6.4f",  0x0000, 0x2000, CRC(f049bc57) SHA1(cb6baaa4eaf9306a54ec39689e3b16ab7acc82c7) )
	ROM_LOAD( "no7.4e",  0x4000, 0x2000, CRC(2ff0c6c7) SHA1(a92926d497189fadb11d0c8fd12d8e4a1506c4fd) )
	ROM_LOAD( "no8.4d",  0x8000, 0x2000, CRC(32ed9c86) SHA1(6614df331ab8e57327b27393d189cc538f1b9567) )
	ROM_LOAD( "no9.6f",  0x2000, 0x2000, CRC(1e5f720e) SHA1(e3d55fae723625fcdd78ee02fb10e47d8e0628f0) )
	ROM_LOAD( "no10.6e", 0x6000, 0x2000, CRC(bfb79118) SHA1(191c441b60fdec714733e326e7ad984b551e2cce) )
	ROM_LOAD( "no11.6d", 0xa000, 0x2000, CRC(a601d401) SHA1(ac10da18c6ef46d9c9da10e292dcb49554676885) )

	ROM_REGION( 0x020, "color_prom", 0 )
	ROM_LOAD( "n1.11d", 0x000, 0x020, CRC(1e6f668a) SHA1(6006ee30920e51581862b0e7f56ac724831b0034) )

	ROM_REGION( 0x100, "clut_prom", 0 )
	ROM_LOAD( "n2.9d",  0x000, 0x100, CRC(e6812f63) SHA1(2286b43970e51d6cfbceaaf74bcb6d2f35620d3a) )
ROM_END

} // anonymous namespace


GAME( 1984, hanadojo,  0,        hanadojo, hanadojo, hanadojo_state, empty_init, ROT0, "Alba", "Hana Doujou (set 1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_UNEMULATED_PROTECTION )
GAME( 1984, hanadojoa, hanadojo, hanadojo, hanadojo, hanadojo_state, empty_init, ROT0, "Alba", "Hana Doujou (set 2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_UNEMULATED_PROTECTION )
