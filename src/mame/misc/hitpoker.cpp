// license:BSD-3-Clause
// copyright-holders: David Haywood, Angelo Salese

/******************************************************************************

Hit Poker (c) 1997 Accept LTD

preliminary driver by Angelo Salese & David Haywood
Many thanks to Olivier Galibert for the CPU identify effort ;-)

TODO:
- CPU core bugs?
- Protection controls inputs;
- Hangs during attract mode due to unemulated PIC protection;
- complete video HW (unknown bits and hblank);
- 24Khz monitor isn't supported, it changes the resolution to 648 x 480 and
  changes the register 9 (raster lines x character lines) from 7 to 0xf.
- sound (I've heard something reasonable during tests, so it could be OK);
- convert to memory_view;
- convert to tilemap system.

================================================================================

'Hit Poker'?

cpu hd46505SP (HD6845SP) <- ha, ha, ha... --"

other : ZC407615CFN (infralink)

chrystal : no idea

ram km6264BL X3
TMM 2018 X2
DALLAS REAL TIME CLK DS17487-5
SOUND YM2149F
DIP 1X4

============================================================================

Some debug tricks (let's test this CPU as more as possible):
- set a bp at 1185, the "bad crc 000" msg is caused by this routine.
  (kludged to work)
- set a bp at 121f then pc=1223
- set a bp at 3a50 then pc+=2, it should now enter into attract mode

***************************************************************************/


#include "emu.h"

#include "cpu/mc68hc11/mc68hc11.h"
#include "machine/ds17x85.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class hitpoker_state : public driver_device
{
public:
	hitpoker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram", 0x3600, ENDIANNESS_BIG),
		m_paletteram(*this, "paletteram", 0x1000, ENDIANNESS_BIG),
		m_colorram(*this, "colorram", 0x2000, ENDIANNESS_BIG)
	{
	}

	void hitpoker(machine_config &config);
	void reelmtp(machine_config &config);

	void init_hitpoker();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<mc68hc11_cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	memory_share_creator<uint8_t> m_videoram;
	memory_share_creator<uint8_t> m_paletteram;
	memory_share_creator<uint8_t> m_colorram;

	uint8_t m_pic_data = 0;

	[[maybe_unused]] uint8_t test_r();
	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	uint8_t cram_r(offs_t offset);
	void cram_w(offs_t offset, uint8_t data);
	uint8_t paletteram_r(offs_t offset);
	void paletteram_w(offs_t offset, uint8_t data);
	uint8_t hitpoker_pic_r();
	uint8_t reelmtp_pic_r();
	void pic_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void hitpoker_map(address_map &map) ATTR_COLD;
	void reelmtp_map(address_map &map) ATTR_COLD;
};


void hitpoker_state::machine_start()
{
	save_item(NAME(m_pic_data));
}

void hitpoker_state::video_start()
{
	// TODO: convert to tilemaps
}

uint32_t hitpoker_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int count = 0;

	bitmap.fill(rgb_t::black(), cliprect);

	for (int y = 0; y < 31; y++)
	{
		for (int x = 0; x < 81; x++) // it's probably 80 + 1 global line attribute at the start of each line
		{
			int const tile = (((m_videoram[count] << 8)|(m_videoram[count + 1])) & 0x3fff);
			int const gfx_bpp = (m_colorram[count] & 0x80) >> 7; // flag between 4 and 8 bpp
			int const color = gfx_bpp ? ((m_colorram[count] & 0x70) >> 4) : (m_colorram[count] & 0xf);

			m_gfxdecode->gfx(gfx_bpp)->opaque(bitmap, cliprect, tile, color, 0, 0, x * 8, y * 8);

			count += 2;
		}
	}

	return 0;
}

uint8_t hitpoker_state::vram_r(offs_t offset)
{
	uint8_t *ROM = memregion("maincpu")->base();

	if (m_pic_data & 0x10)
		return m_videoram[offset];
	else
		return ROM[offset + 0x8000];
}

void hitpoker_state::vram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
}

uint8_t hitpoker_state::cram_r(offs_t offset)
{
	uint8_t *ROM = memregion("maincpu")->base();

	if (m_pic_data & 0x10)
		return m_colorram[offset];
	else
		return ROM[offset + 0xc000];
}

void hitpoker_state::cram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
}

uint8_t hitpoker_state::paletteram_r(offs_t offset)
{
	uint8_t *ROM = memregion("maincpu")->base();

	if (m_pic_data & 0x10)
		return m_paletteram[offset];
	else
		return ROM[offset + 0xe000];
}

void hitpoker_state::paletteram_w(offs_t offset, uint8_t data)
{
	m_paletteram[offset] = data;
	offset >>= 1;
	int const datax = 256 * m_paletteram[offset * 2] + m_paletteram[offset * 2 + 1];

	// RGB565
	int const b = (datax & 0xf800) >> 11;
	int const g = (datax & 0x07e0) >> 5;
	int const r = (datax & 0x001f) >> 0;

	m_palette->set_pen_color(offset, pal5bit(r), pal6bit(g), pal5bit(b));
}


uint8_t hitpoker_state::hitpoker_pic_r()
{
	// logerror("%s PIC R\n", machine().describe_context());

	if (m_maincpu->pc() == 0x3143 ||
		m_maincpu->pc() == 0x314e ||
		m_maincpu->pc() == 0x3164 ||
		m_maincpu->pc() == 0x3179)
		return m_pic_data;

	return (m_pic_data & 0x7f) | (m_pic_data & 0x40 ? 0x80 : 0x00);
}

uint8_t hitpoker_state::reelmtp_pic_r() // TODO: is this really a PIC or is it internal to the custom MCU?
{
	// logerror("%s PIC R\n", machine().describe_context());

	// TODO:

	if (m_maincpu->pc() == 0x471d ||
		m_maincpu->pc() == 0x4728 ||
		m_maincpu->pc() == 0x473e ||
		m_maincpu->pc() == 0x4753)
		return m_pic_data;

	return (m_pic_data & 0x7f) | (m_pic_data & 0x40 ? 0x80 : 0x00);
}

void hitpoker_state::pic_w(uint8_t data)
{
	m_pic_data = data; // | (data & 0x40) ? 0x80 : 0x00;
	// logerror("%02x PIC W\n", data);
}

uint8_t hitpoker_state::test_r()
{
	return machine().rand();
}

// overlap empty ROM addresses
void hitpoker_state::hitpoker_map(address_map &map)
{
	map(0x0000, 0xb5ff).rom();
	map(0xbf00, 0xffff).rom();

	map(0x8000, 0xb5ff).rw(FUNC(hitpoker_state::vram_r), FUNC(hitpoker_state::vram_w));
	map(0xb800, 0xbdff).ram();
	map(0xbe00, 0xbe7f).rw("rtc", FUNC(ds17x85_device::read_direct), FUNC(ds17x85_device::write_direct));
	map(0xbe80, 0xbe80).w("crtc", FUNC(mc6845_device::address_w));
	map(0xbe81, 0xbe81).w("crtc", FUNC(mc6845_device::register_w));
	map(0xbe90, 0xbe91).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xbea0, 0xbea0).portr("VBLANK"); //probably other bits as well
//  map(0xbe00, 0xbeff).r(FUNC(hitpoker_state::test_r));
	map(0xc000, 0xdfff).rw(FUNC(hitpoker_state::cram_r), FUNC(hitpoker_state::cram_w));
	map(0xe000, 0xefff).rw(FUNC(hitpoker_state::paletteram_r), FUNC(hitpoker_state::paletteram_w));
}

void hitpoker_state::reelmtp_map(address_map &map)
{
	map(0x0100, 0x07ff).ram();
	map(0x0800, 0xb7ff).rom();
	map(0xb900, 0xffff).rom();

	map(0x8000, 0xb5ff).rw(FUNC(hitpoker_state::vram_r), FUNC(hitpoker_state::vram_w));
	map(0xb800, 0xb87f).rw("rtc", FUNC(ds17x85_device::read_direct), FUNC(ds17x85_device::write_direct));
	map(0xb880, 0xb880).w("crtc", FUNC(mc6845_device::address_w));
	map(0xb881, 0xb881).w("crtc", FUNC(mc6845_device::register_w));
	map(0xb890, 0xb891).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xb8a0, 0xb8a0).portr("VBLANK");
	//map(0xb8c1, 0xb8c1).w // TODO
	//map(0xb8e1, 0xb8e1).w // TODO
	map(0xc000, 0xdfff).rw(FUNC(hitpoker_state::cram_r), FUNC(hitpoker_state::cram_w));
	//map(0xe000, 0xefff).rw(FUNC(hitpoker_state::paletteram_r), FUNC(hitpoker_state::paletteram_w));
}

static INPUT_PORTS_START( hitpoker )
	PORT_START("VBLANK")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) // resets the game?
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) // "permanent ram initialized" if combined with the reset switch
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "H-Blank" ) // scanline counter probably
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Monitor" ) // a JP probably
	PORT_DIPSETTING(    0x40, "15KHz" )
	PORT_DIPSETTING(    0x00, "24KHz" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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

// TODO: complete, currently just the bare minimum to make it show something
static INPUT_PORTS_START( reelmtp )
	PORT_INCLUDE( hitpoker )

	PORT_MODIFY("VBLANK") // these 2 seem inverted wrt hitpoker
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_DIPNAME( 0x80, 0x80, "H-Blank" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout hitpoker_layout_4bpp =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,4,8,12 },
	{ 0,1,2,3,16,17,18,19 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout hitpoker_layout_8bpp =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+4,RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+12,0,4,8,12 },
	{ 0,1,2,3,16,17,18,19 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static GFXDECODE_START( gfx_hitpoker )
	GFXDECODE_ENTRY( "tiles", 0, hitpoker_layout_4bpp,   0, 0x100  )
	GFXDECODE_ENTRY( "tiles", 0, hitpoker_layout_8bpp,   0, 8  )
GFXDECODE_END

void hitpoker_state::hitpoker(machine_config &config)
{
	MC68HC11A1(config, m_maincpu, 8'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hitpoker_state::hitpoker_map);
	m_maincpu->in_pa_callback().set(FUNC(hitpoker_state::hitpoker_pic_r));
	m_maincpu->out_pa_callback().set(FUNC(hitpoker_state::pic_w));
	m_maincpu->in_pe_callback().set_constant(0);

	ds17x85_device &rtc(DS17487(config, "rtc", 32768));
	rtc.irq().set_inputline(m_maincpu, MC68HC11_IRQ_LINE);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(648, 480); // set by the CRTC
	screen.set_visarea(0, 648-1, 0, 240-1);
	screen.set_screen_update(FUNC(hitpoker_state::screen_update));

	hd6845s_device &crtc(HD6845S(config, "crtc", XTAL(3'579'545) / 2));  // hand tuned to get ~60 FPS
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	//crtc.out_vsync_callback().set(FUNC(hitpoker_state::irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_hitpoker);
	PALETTE(config, m_palette).set_entries(0x800);

	SPEAKER(config, "mono").front_center();

	ym2149_device &aysnd(YM2149(config, "aysnd", 1'500'000));
	aysnd.port_a_read_callback().set_ioport("DSW1");
	aysnd.port_b_read_callback().set_ioport("DSW2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void hitpoker_state::reelmtp(machine_config &config)
{
	hitpoker(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &hitpoker_state::reelmtp_map);
	m_maincpu->in_pa_callback().set(FUNC(hitpoker_state::reelmtp_pic_r));
	// TODO: there's only one 25.175 MHz XTAL. Adjust clocks

	// TODO: RAMDAC
}

void hitpoker_state::init_hitpoker()
{
	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0x1220] = 0x01; // patch EEPROM write?
	ROM[0x1221] = 0x01;
	ROM[0x1222] = 0x01;

	ROM[0x10c6] = 0x01;
	ROM[0x10c7] = 0x01; // patch the checksum routine

	// must match RTC serial number
}

ROM_START( hitpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u4.bin",         0x00000, 0x10000, CRC(0016497a) SHA1(017320bfe05fea8a48e26a66c0412415846cee7c) )

	ROM_REGION( 0x200, "maincpu:eeprom", ROMREGION_ERASEFF )
	// must match RTC serial number
	ROM_FILL( 0x003, 1, 0x4d )
	ROM_FILL( 0x002, 1, 0x41 )
	ROM_FILL( 0x001, 1, 0x4d )
	ROM_FILL( 0x000, 1, 0x45 )

	ROM_REGION( 0x10000, "pic", 0 )
	ROM_LOAD( "pic",            0x00000, 0x1000, NO_DUMP ) // unknown type

	ROM_REGION( 0x100000, "tiles", 0 ) // tile 0x4c8 seems to contain something non-gfx related, could be tilemap / colour data, check!
	ROM_LOAD16_BYTE( "u42.bin",         0x00001, 0x40000, CRC(cbe56fec) SHA1(129bfd10243eaa7fb6a087f96de90228e6030353) )
	ROM_LOAD16_BYTE( "u43.bin",         0x00000, 0x40000, CRC(6c0d4283) SHA1(04a4fd82f5cc0ed9f548e490ac67d287227073c3) )
	ROM_LOAD16_BYTE( "u44.bin",         0x80001, 0x40000, CRC(e23d5f30) SHA1(ca8855301528aa4eeff40cb820943b4268f8596e) ) // the 'adult images' are 8bpp
	ROM_LOAD16_BYTE( "u45.bin",         0x80000, 0x40000, CRC(e65b3e52) SHA1(c0c1a360a4a1823bf71c0a4105ff41f4102862e8) ) //  the first part of these 2 is almost empty as the standard gfx are 4bpp
ROM_END

// PCB features Motorola SC80566FN (custom MC68HC11 based MCU), 25.175 MHz XTAL, YM2149F, DS17487-5 RTC, MC6845P CRTC, ADV471KP50E RAMDAC, 1 4-DIP bank
// Can't spot any PIC.
ROM_START( reelmtp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u12.bin", 0x00000, 0x10000, CRC(b613a0e1) SHA1(071f36b88261a44d82ef0322ddf12502ffacf998) ) // 27C512

	ROM_REGION( 0x200, "maincpu:eeprom", ROMREGION_ERASEFF )
	// must match RTC serial number. TODO!
	ROM_FILL( 0x003, 1, 0x4d )
	ROM_FILL( 0x002, 1, 0x41 )
	ROM_FILL( 0x001, 1, 0x4d )
	ROM_FILL( 0x000, 1, 0x45 )

	// TODO: bigger than what MAME expects for this RTC model?
	ROM_REGION( 0x1100, "rtc_dump", 0 )
	ROM_LOAD( "ds17487-5.u14", 0x0000, 0x1100, CRC(bffb8967) SHA1(28276f01c114c477a0c022b17d250c4952922625) )

	ROM_REGION( 0x180000, "tiles", 0 ) // TODO: wrong loading / decode
	ROM_LOAD( "u40.bin", 0x000000, 0x40000, CRC(cbac4a12) SHA1(01133620ba5b3bb02712d30356d003b748315258) ) // M27C2001
	// 0x40000 - 07ffff?
	ROM_LOAD( "u41.bin", 0x080000, 0x80000, CRC(17c57221) SHA1(1a3cd5a5a3d7736fc50d605da98667a3efa7cd3c) ) // 27C040
	ROM_LOAD( "u42.bin", 0x100000, 0x80000, CRC(30f31d9e) SHA1(4637e04e647ae3fc6b93093ae7468fefdcc355cc) ) // 27C040

	ROM_REGION( 0x600, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "palce16v8h.u10", 0x000, 0x117, CRC(75ab49ba) SHA1(9060d59e92df0c6ed7b5041a98bb4ab5759e7d62) )
	ROM_LOAD( "palce16v8h.2",   0x200, 0x117, NO_DUMP ) // U location no readable, but just over the above one
	ROM_LOAD( "pal.u26",        0x400, 0x117, NO_DUMP ) // scratched, can't read type
ROM_END

} // anonymous namespace


GAME( 1997, hitpoker, 0, hitpoker, hitpoker, hitpoker_state, init_hitpoker, ROT0, "Accept Ltd.", "Hit Poker (Bulgaria)",  MACHINE_NOT_WORKING )
GAME( 1999, reelmtp,  0, reelmtp,  reelmtp,  hitpoker_state, empty_init,    ROT0, "Accept Ltd.", "Reel Magic Turbo Play", MACHINE_NOT_WORKING )
