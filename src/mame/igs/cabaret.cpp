// license:BSD-3-Clause
// copyright-holders: Mirko Buffoni, David Haywood

/***************************************************************************

Cabaret (AMT)
Driver by Mirko Buffoni, David Haywood

TODO:
- This game should have an NVRAM.  There is trace of System Reset so need
  to find how to reset its content.
- DSW3 is read, not sure where it's used
- Keyboard is mapped through test mode, but some bits are unknown, and hopper
  is not emulated
- Map LEDs and coin counters
- Remove patches after finding why there are so many pitfalls.  Maybe the
  game expects to read inputs via an external device and expects certain
  timings
- Trojan out internal ROMs for kungfua and double8l
- kungfua and double8l have 5 banks of 8 DIP switches (sheets available for
  double8l)

Press F1+F2 during reset to see 'pork*ish' test mode :P

Interesting thing: this game is copyright AMT 1992, but protection checks
are the same of IGS.  AMT may be previous IGS name.

***************************************************************************/

#include "emu.h"

#include "cpu/z180/z180.h"
#include "machine/i8255.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class cabaret_state : public driver_device
{
public:
	cabaret_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_fg_tile_ram(*this, "fg_tile_ram")
		, m_fg_color_ram(*this, "fg_color_ram")
		, m_bg_scroll(*this, "bg_scroll")
		, m_bg_tile_ram(*this, "bg_tile_ram")
		, m_led(*this, "led6")
	{ }

	void cabaret(machine_config &config);

	void init_cabaret();
	void init_double8l();
	void init_kungfua();

protected:
	virtual void machine_start() override { m_led.resolve(); }
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_fg_tile_ram;
	required_shared_ptr<uint8_t> m_fg_color_ram;
	required_shared_ptr<uint8_t> m_bg_scroll;
	required_shared_ptr<uint8_t> m_bg_tile_ram;

	output_finder<> m_led;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	uint8_t m_nmi_enable = 0;
	uint8_t m_out[3]{};

	void bg_scroll_w(offs_t offset, uint8_t data);
	void bg_tile_w(offs_t offset, uint8_t data);
	void fg_tile_w(offs_t offset, uint8_t data);
	void fg_color_w(offs_t offset, uint8_t data);
	void nmi_and_coins_w(uint8_t data);
	void ppi2_b_w(uint8_t data);
	void ppi2_c_w(uint8_t data);
	void show_out();

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt_cb);

	void program_map(address_map &map) ATTR_COLD;
	void port_map(address_map &map) ATTR_COLD;
};


/***************************************************************************
                                Video Hardware
***************************************************************************/


void cabaret_state::bg_scroll_w(offs_t offset, uint8_t data)
{
	m_bg_scroll[offset] = data;
	m_bg_tilemap->set_scrolly(offset,data);
}

void cabaret_state::bg_tile_w(offs_t offset, uint8_t data)
{
	m_bg_tile_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(cabaret_state::get_bg_tile_info)
{
	int const code = m_bg_tile_ram[tile_index];
	tileinfo.set(1, code & 0xff, 0, 0);
}

TILE_GET_INFO_MEMBER(cabaret_state::get_fg_tile_info)
{
	int const code = m_fg_tile_ram[tile_index] | (m_fg_color_ram[tile_index] << 8);
	int const tile = code & 0x1fff;
	tileinfo.set(0, code, tile != 0x1fff ? ((code >> 12) & 0xe) + 1 : 0, 0);
}

void cabaret_state::fg_tile_w(offs_t offset, uint8_t data)
{
	m_fg_tile_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void cabaret_state::fg_color_w(offs_t offset, uint8_t data)
{
	m_fg_color_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void cabaret_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cabaret_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cabaret_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(64);
}


uint32_t cabaret_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/***************************************************************************
                                Memory Maps
***************************************************************************/



void cabaret_state::show_out()
{
#ifdef MAME_DEBUG
	popmessage("%02x %02x %02x",m_out[0], m_out[1], m_out[2]);
#endif
}

void cabaret_state::nmi_and_coins_w(uint8_t data)
{
	if ((m_nmi_enable ^ data) & (~0xdd))
	{
		logerror("%s: nmi_and_coins = %02x\n", machine().describe_context(), data);
		//popmessage("%02x", data);
	}

	machine().bookkeeping().coin_counter_w(0, data & 0x01);   // coin_a
	machine().bookkeeping().coin_counter_w(1, data & 0x04);   // coin_c
	machine().bookkeeping().coin_counter_w(2, data & 0x08);   // key in
	machine().bookkeeping().coin_counter_w(3, data & 0x10);   // coin m_out mech

	m_led = BIT(data, 6);   // LED for coin m_out / hopper active

	m_nmi_enable = data;    // data & 0x80     // NMI enable?

	m_out[0] = data;
	show_out();
}

void cabaret_state::ppi2_b_w(uint8_t data)
{
	m_out[1] = data;
	show_out();
}

void cabaret_state::ppi2_c_w(uint8_t data)
{
	m_out[2] = data;
	show_out();
}



void cabaret_state::program_map(address_map &map)
{
	map(0x00000, 0x0efff).rom();
	map(0x0f000, 0x0ffff).ram();
}

void cabaret_state::port_map(address_map &map)
{
	map(0x0000, 0x003f).ram(); // Z180 internal regs

	map(0x0080, 0x0083).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0090, 0x0093).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x00a0, 0x00a3).rw("ppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));

	map(0x00b0, 0x00b0).portr("DSW3");

	map(0x00e0, 0x00e1).w("ymsnd", FUNC(ym2413_device::write));

	map(0x2000, 0x27ff).ram().w(FUNC(cabaret_state::fg_tile_w)).share(m_fg_tile_ram);
	map(0x2800, 0x2fff).ram().w(FUNC(cabaret_state::fg_color_w)).share(m_fg_color_ram);

	map(0x3000, 0x37ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x3800, 0x3fff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");

	map(0x1000, 0x103f).ram().w(FUNC(cabaret_state::bg_scroll_w)).share(m_bg_scroll);

	map(0x1800, 0x19ff).ram().w(FUNC(cabaret_state::bg_tile_w)).share(m_bg_tile_ram);
	map(0x8000, 0xffff).rom().region("bgmaps", 0);
}


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( cabaret )
	PORT_START("DSW1")      // OK
	PORT_DIPNAME( 0x07, 0x00, "Poke %" ) PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "65%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "82%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x01, "88%" )
	PORT_DIPSETTING(    0x00, "92%" )
	PORT_DIPNAME( 0x08, 0x00, "Double %" ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, "Coin Credit" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x10, "5/1" )
	PORT_DIPSETTING(    0x00, "10/1" )
	PORT_DIPNAME( 0x20, 0x00, "Held Method" ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, "Discard" )
	PORT_DIPSETTING(    0x00, "Held" )
	PORT_DIPNAME( 0x40, 0x00, "Speed" ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, "Slow" )
	PORT_DIPSETTING(    0x00, "Quick" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")      // OK
	PORT_DIPNAME( 0x03, 0x00, "Limit" ) PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x03, "5000" )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x01, "15000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPNAME( 0x0c, 0x00, "Max Bet" ) PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "20" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x10, 0x00, "Withdraw" ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_DIPLOCATION("SWB:6,7,8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0xff, 0xff )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Memory Clear") // stats, memory
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Decrement")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Collect")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("W-Up")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x8x6 =
{
	8, 8,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
		RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
		RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,2*8) },
	8*8*2
};

static const gfx_layout layout_8x32x6i =
{
	8, 32,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
		RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
		RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP32(0,2*8) },
	8*32*2
};

static GFXDECODE_START( gfx_cabaret )
	GFXDECODE_ENTRY( "fgtiles", 0x00000, layout_8x8x6,  0, 16 )
	GFXDECODE_ENTRY( "bgtiles", 0x00000, layout_8x32x6i, 0, 16 )
GFXDECODE_END




/***************************************************************************
                                Machine Drivers
***************************************************************************/

void cabaret_state::machine_reset()
{
	m_nmi_enable =   0;
}

INTERRUPT_GEN_MEMBER(cabaret_state::interrupt_cb)
{
	if (m_nmi_enable & 0x80)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void cabaret_state::cabaret(machine_config &config)
{
	// basic machine hardware
	Z80180(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &cabaret_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &cabaret_state::port_map);
	m_maincpu->set_vblank_int("screen", FUNC(cabaret_state::interrupt_cb));

	i8255_device &ppi1(I8255(config, "ppi1"));
	ppi1.in_pa_callback().set_ioport("BUTTONS2");
	ppi1.in_pb_callback().set_ioport("SERVICE");
	ppi1.in_pc_callback().set_ioport("COINS");

	i8255_device &ppi2(I8255(config, "ppi2"));
	ppi2.in_pa_callback().set_ioport("BUTTONS1");
	ppi2.out_pb_callback().set(FUNC(cabaret_state::ppi2_b_w));
	ppi2.out_pc_callback().set(FUNC(cabaret_state::ppi2_c_w));

	i8255_device &ppi3(I8255(config, "ppi3"));
	ppi3.out_pa_callback().set(FUNC(cabaret_state::nmi_and_coins_w));
	ppi3.tri_pa_callback().set_constant(0xf0);
	ppi3.in_pb_callback().set_ioport("DSW1");
	ppi3.in_pc_callback().set_ioport("DSW2");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(cabaret_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cabaret);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x800);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	YM2413(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.0);
}


void cabaret_state::init_cabaret()
{
	uint8_t *rom = memregion("maincpu")->base();

	// decrypt the program ROM
	for (int i = 0; i < 0xf000; i++)
	{
		if ((i & 0x2206) == 0x2002) rom[i] ^= 0x01;
	}

	// Patch pitfalls
	rom[0x1012] =
	rom[0x1013] = 0;
	rom[0x13b8] = 0x18;
	rom[0x53a6] = 0x18;
	rom[0x73c6] = 0x18;
	rom[0xc46a] = 0x18;
	rom[0xc583] = 0x18;
	rom[0xc5fa] = 0x18;
	rom[0xc6c4] = 0x18;
}

void cabaret_state::init_double8l()
{
	uint8_t *rom = memregion("maincpu")->base();

	// TODO: verify if/when the internal ROM gets dumped
	for (int i = 0x4000; i < 0x10000; i++)
		if ((i & 0x3206) == 0x2002)
			rom[i] = rom[i] ^ 0x01;
}

void cabaret_state::init_kungfua()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0x4000; i < 0x10000; i++)
	{
		rom[i] = rom[i] ^ 0x01;
	}
}


ROM_START( cabaret )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cg-8v204.u97",  0x00000, 0x10000, CRC(44cebf77) SHA1(e3f4e4abf41388f0eed50cf9a0fd0b14aa2f8b93) )

	ROM_REGION( 0x60000, "fgtiles", 0 )
	ROM_LOAD( "cg-4.u43",  0x40000, 0x20000, CRC(e509f50a) SHA1(7e68ca54642c92cdb348d5cf9466065938d0e027) )
	ROM_LOAD( "cg-5.u44",  0x20000, 0x20000, CRC(e2cbf489) SHA1(3a15ed7efd5696656e6d55b54ec0ff779bdb0d98) )
	ROM_LOAD( "cg-6.u45",  0x00000, 0x20000, CRC(4f2fced7) SHA1(b954856ffdc97fbc99fd3ec087376fbf466d2d5a) )

	ROM_REGION( 0xc000, "bgtiles", 0 )
	ROM_LOAD( "cg-1.u40",  0x8000, 0x4000, CRC(7dee8b1f) SHA1(80dbdf6aab9b02cc000956b7894023552428e6a1) )
	ROM_LOAD( "cg-2.u41",  0x0000, 0x4000, CRC(ce8dea39) SHA1(b30d1678a7b98cd821d2ce7383a83cb7c9f31b5f) )
	ROM_LOAD( "cg-3.u42",  0x4000, 0x4000, CRC(7e1f821f) SHA1(b709d49f9d1890fe3b8ca7f90affc0017a0ad95e) )

	ROM_REGION( 0x8000, "bgmaps", 0 )
	ROM_LOAD( "cg-7.u98",  0x0000, 0x8000, CRC(b93ae6f8) SHA1(accb87045c278d5d79fff65bb763aa6e8025a945) )   // background maps, read by the CPU
ROM_END

// custom CPU package DB8LN CPUV1.0 1991 (almost surely Z180 based).
ROM_START( double8l )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// contains leftover x86 code at 0-3fff
	ROM_LOAD( "db8ln_cpuv1.0_1991.u80", 0x00000, 0x04000, NO_DUMP )
	ROM_LOAD( "u97",                    0x00000, 0x10000, CRC(84686a43) SHA1(9e41c725a4fe9c674979c8437715e5c3197e4aa2) ) // 27C512, didn't have an original sticker

	ROM_REGION( 0x60000, "fgtiles", 0 ) // all Intel 27C010A
	ROM_LOAD( "d8ln_cgf4.u43",  0x40000, 0x20000, CRC(d16b724d) SHA1(37ad925b07ef64d28670a8937ef6edfb00708d59) )
	ROM_LOAD( "d8ln_cgf5.u44",  0x20000, 0x20000, CRC(7c4aa36f) SHA1(0d1fe127a300b3341b0f15c6bdaeffde1564b204) )
	ROM_LOAD( "d8ln_cgf6.u45",  0x00000, 0x20000, CRC(759a8b81) SHA1(9b885d0eab72c7257005f1da21269ce490888640) )

	ROM_REGION( 0xc000, "bgtiles", 0 )
	ROM_LOAD( "d8ln_cgf1.u40",  0x8000, 0x4000, CRC(969bb2e9) SHA1(5aecce965f558a11ec117ab5ccb82b841de6051f) ) // Microchip 27C128
	ROM_LOAD( "d8ln_cgf2.u41",  0x0000, 0x4000, CRC(bf6ef240) SHA1(a79e16ba259f03c0479e87a647a3a6a2ce0a1aac) ) // Microchip 27C128
	ROM_LOAD( "d8ln_cgf3.u42",  0x4000, 0x4000, CRC(f432f709) SHA1(a203c01358b42f6b8639ce45afa70a86afdc79cf) ) // Intel 27128A

	ROM_REGION( 0x8000, "bgmaps", 0 )
	ROM_LOAD( "d8ln_cgf7.u98",  0x0000, 0x8000, CRC(5268ae60) SHA1(9e5d819fc5a5623cc3e81384ba542391c59ab7f1) )   // 27C256, background maps, read by the CPU

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "dm74s287.u38", 0x000, 0x100, CRC(935d8c14) SHA1(04e90b01ef1e3485d3bfcf9473a18b7afea25d1d) )

	ROM_REGION( 0x600, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "ami18cv8.u46",  0x000, 0x155, NO_DUMP )
	ROM_LOAD( "pal16l8an.u47", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "ami18cv8.u48",  0x400, 0x155, NO_DUMP )
ROM_END

/*

Cherry master looking board

Big chip with no markings at U80 stickered  KUNG FU
                                            V1.0
                                            1992

Board silkscreend on top                    PCB NO.0013-B

.45 27010   stickered   6
.44 27010   stickered   5
.43 27010   stickered   4
.42 27128   stickered   3
.41 27128   stickered   2
.40 27128   stickered   1
.98 27256   stickered   7   couldn't read chip, but board was silkscreened 27c256
.97 27512   stickered   ?   looked like Japanese writing
.38 74s287
.46 18cv8               <--- same checksum as .48
.47 pal16l8a            <--- checksum was 0
.48 18cv8               <--- same checksum as .46

unknown 24 pin chip @ u29
open 24 pin socket @ u54
12 MHz crystal

5 x DSW8
3 x NEC D8255AC

*/

ROM_START( kungfua )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	// u97 contains leftover x86 code at 0-3fff (compiled with Borland Turbo-C).
	// You can rename the rom to kungfu.exe and run it (DOS MZ executable)!
	// The rest is Z80 code, so the CPU at u80 is probably a variant with internal ROM.
	ROM_LOAD( "kungfu-internal.u80", 0x00000, 0x04000, NO_DUMP )
	ROM_LOAD( "kungfu.u97",          0x00000, 0x10000, CRC(5c8e16de) SHA1(4af3795753d6e08f528b861d3a771c782e173556) )

	ROM_REGION( 0x60000, "fgtiles", 0 )
	ROM_LOAD( "kungfu-4.u43", 0x40000, 0x20000, CRC(df4afedb) SHA1(56ab18c46a199653c284417a8e9edc9f32374318) )
	ROM_LOAD( "kungfu-5.u44", 0x20000, 0x20000, CRC(25c9c98e) SHA1(2d3a399d8d53ee5cb8106d2b35d1ab1778439f81) )
	ROM_LOAD( "kungfu-6.u45", 0x00000, 0x20000, CRC(f1ec5f0d) SHA1(0aa888e13312ed5d98953c81f03a61c6175c7fec) )

	ROM_REGION( 0xc000, "bgtiles", ROMREGION_ERASE00 )
	ROM_LOAD( "kungfu-1.u40", 0x8000, 0x4000, CRC(abaada6b) SHA1(a6b910db7451e8ca737f43f32dfc8fc5ecf865f4) )
	ROM_LOAD( "kungfu-2.u41", 0x0000, 0x4000, CRC(927b3060) SHA1(a780ea5aaee04287cc9533c2d258dc18f8426530) )
	ROM_LOAD( "kungfu-3.u42", 0x4000, 0x4000, CRC(bbf78e03) SHA1(06fee093e75e2611d00c076c2e0a681938fa8b74) )

	ROM_REGION( 0x8000, "bgmaps", 0 )
	ROM_LOAD( "kungfu-7.u98", 0x0000, 0x8000, CRC(1d3f0c79) SHA1(0a33798b69fbdc0fb7c47c51f5759e42acd2c608) ) // background maps, read by the CPU

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "kungfu.u38", 0x000, 0x100, CRC(2074f729) SHA1(eb9a60dec57a029ae6d3fc53aa7bc78e8ac34392) )

	ROM_REGION( 0x1000, "plds", 0 ) // TODO: convert with jedutil
	ROM_LOAD( "kungfu.u46", 0x000, 0xde1, CRC(5d4aacaf) SHA1(733546ce0585c40833e1c34504c33219a2bea0a9) )
	ROM_LOAD( "kungfu.u47", 0x000, 0xaee, CRC(5c7e25b5) SHA1(7d37e4abfe1256bd9cb168e0f02e651118dfb304) )
	ROM_LOAD( "kungfu.u48", 0x000, 0xde1, CRC(5d4aacaf) SHA1(733546ce0585c40833e1c34504c33219a2bea0a9) )
ROM_END

} // anonymous namespace


GAME( 1992, cabaret,  0,      cabaret, cabaret, cabaret_state, init_cabaret,  ROT0, "AMT Co. Ltd.", "Cabaret Show",                 MACHINE_NOT_WORKING )
GAME( 1992, double8l, 0,      cabaret, cabaret, cabaret_state, init_double8l, ROT0, "AMT Co. Ltd.", "Double 8 Line",                MACHINE_NOT_WORKING ) // missing internal ROM dump
GAME( 1992, kungfua,  kungfu, cabaret, cabaret, cabaret_state, init_kungfua,  ROT0, "IGS",          "Kung Fu Fighters (IGS, v100)", MACHINE_NOT_WORKING ) // missing internal ROM dump
