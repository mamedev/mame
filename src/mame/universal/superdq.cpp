// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    Universal System 1

    Games Supported:
    - Universal Super Don Quix-ote

**************************************************************************

    ROM Revisions
    -------------


    Laserdisc Players Used
    ----------------------
    Pioneer LD-V1000

*************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ldv1000.h"
#include "sound/sn76496.h"
#include "video/resnet.h"
#include "emupal.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

#define MASTER_CLOCK    20000000


class superdq_state : public driver_device
{
public:
	superdq_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_laserdisc(*this, "laserdisc"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void superdq(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<pioneer_ldv1000_device> m_laserdisc;
	uint8_t m_ld_in_latch = 0;
	uint8_t m_ld_out_latch = 0;

	required_shared_ptr<uint8_t> m_videoram;
	tilemap_t *m_tilemap = nullptr;
	int m_color_bank = 0;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void superdq_videoram_w(offs_t offset, uint8_t data);
	void superdq_io_w(uint8_t data);
	uint8_t superdq_ld_r();
	void superdq_ld_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void superdq_palette(palette_device &palette) const;
	uint32_t screen_update_superdq(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(superdq_vblank);
	void superdq_io(address_map &map) ATTR_COLD;
	void superdq_map(address_map &map) ATTR_COLD;
};

TILE_GET_INFO_MEMBER(superdq_state::get_tile_info)
{
	int tile = m_videoram[tile_index];

	tileinfo.set(0, tile, m_color_bank, 0);
}

void superdq_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(superdq_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t superdq_state::screen_update_superdq(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



/*************************************
 *
 *  Palette conversion
 *
 *************************************/

void superdq_state::superdq_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static const int resistances[3] = { 820, 390, 200 };
	double rweights[3], gweights[3], bweights[2];

	// compute the color output resistor weights
	compute_resistor_weights(0, 255, -1.0,
			3,  &resistances[0], rweights, 220, 0,
			3,  &resistances[0], gweights, 220, 0,
			2,  &resistances[1], bweights, 220, 0);

	// initialize the palette with these colors
	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 7);
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 5);
		int const r = combine_weights(rweights, bit2, bit1, bit0);

		// green component
		bit0 = BIT(color_prom[i], 4);
		bit1 = BIT(color_prom[i], 3);
		bit2 = BIT(color_prom[i], 2);
		int const g = combine_weights(gweights, bit2, bit1, bit0);

		// blue component
		bit0 = BIT(color_prom[i], 1);
		bit1 = BIT(color_prom[i], 0);
		int const b = combine_weights(bweights, bit1, bit0);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void superdq_state::machine_reset()
{
	m_ld_in_latch = 0;
	m_ld_out_latch = 0xff;
	m_color_bank = 0;
}

INTERRUPT_GEN_MEMBER(superdq_state::superdq_vblank)
{
	/* status is read when the STATUS line from the laserdisc
	   toggles (600usec after the vblank). We could set up a
	   timer to do that, but this works as well */
	m_ld_in_latch = m_laserdisc->data_r();

	/* command is written when the COMMAND line from the laserdisc
	   toggles (680usec after the vblank). We could set up a
	   timer to do that, but this works as well */
	m_laserdisc->data_w(m_ld_out_latch);
	device.execute().set_input_line(0, ASSERT_LINE);
}

void superdq_state::superdq_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void superdq_state::superdq_io_w(uint8_t data)
{
	static const uint8_t black_color_entries[] = {7,15,16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

	if ( data & 0x40 ) /* bit 6 = irqack */
		m_maincpu->set_input_line(0, CLEAR_LINE);

	machine().bookkeeping().coin_counter_w(0, data & 0x08 );
	machine().bookkeeping().coin_counter_w(1, data & 0x04 );

	m_color_bank = ( data & 2 ) ? 1 : 0;

	for (int i = 0; i < std::size(black_color_entries); i++)
	{
		int index = black_color_entries[i];
		if (data & 0x80)
			m_palette->set_pen_color(index, m_palette->pen_color(index) & rgb_t(0,255,255,255));
		else
			m_palette->set_pen_color(index, m_palette->pen_color(index) | rgb_t(255,0,0,0));
	}

	/*
	    bit 5 = DISP1?
	    bit 4 = DISP2?
	    bit 0 = unused
	*/
}

uint8_t superdq_state::superdq_ld_r()
{
	return m_ld_in_latch;
}

void superdq_state::superdq_ld_w(uint8_t data)
{
	m_ld_out_latch = data;
}



/*************************************
 *
 *  Memory map
 *
 *************************************/

void superdq_state::superdq_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x5c00, 0x5fff).ram().w(FUNC(superdq_state::superdq_videoram_w)).share("videoram");
}

void superdq_state::superdq_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0").w(FUNC(superdq_state::superdq_ld_w));
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("DSW1");
	map(0x03, 0x03).portr("DSW2");
	map(0x04, 0x04).r(FUNC(superdq_state::superdq_ld_r)).w("snsnd", FUNC(sn76496_device::write));
	map(0x08, 0x08).w(FUNC(superdq_state::superdq_io_w));
	map(0x0c, 0x0d).noprw(); /* HD46505S */
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( superdq )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )   /* Service button */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )       /* TEST button */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "100000" )
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, "Infinite Lives" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every sprite takes 32 consecutive bytes */
};


static GFXDECODE_START( gfx_superdq )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 2 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void superdq_state::machine_start()
{
}


void superdq_state::superdq(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &superdq_state::superdq_map);
	m_maincpu->set_addrmap(AS_IO, &superdq_state::superdq_io);
	m_maincpu->set_vblank_int("screen", FUNC(superdq_state::superdq_vblank));

	PIONEER_LDV1000(config, m_laserdisc, 0);
	m_laserdisc->set_overlay(256, 256, FUNC(superdq_state::screen_update_superdq));
	m_laserdisc->add_route(0, "lspeaker", 1.0);
	m_laserdisc->add_route(1, "rspeaker", 1.0);

	/* video hardware */
	m_laserdisc->add_ntsc_screen(config, "screen");

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_superdq);
	PALETTE(config, m_palette, FUNC(superdq_state::superdq_palette), 32);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SN76496(config, "snsnd", MASTER_CLOCK/8).add_route(ALL_OUTPUTS, "lspeaker", 0.8);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( superdq )        /* long scenes */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sdq-prog.bin", 0x0000, 0x4000, CRC(96b931e2) SHA1(a2408272e19b02755368a6d7e526eec15896e586) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sdq-char.bin", 0x0000, 0x2000, CRC(5fb0e440) SHA1(267413aeb36b661458b7229d65d7b1d03562a1d3) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sdq-cprm.bin", 0x0000, 0x0020, CRC(96701569) SHA1(b0f40373735d1af0c62e5ab06045a064b4eb1794) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "superdq", 0, NO_DUMP )
ROM_END

ROM_START( superdqs )       /* short scenes */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sdq_c45.rom", 0x0000, 0x4000, CRC(0f4d4832) SHA1(c6db63721f0c73151eb9a678ceafd0e7d6121fd3) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sdq_a8.rom", 0x0000, 0x2000, CRC(7d981a14) SHA1(0a0949113b80c30adbb5bdb108d396993225be5b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sdq-cprm.bin", 0x0000, 0x0020, CRC(96701569) SHA1(b0f40373735d1af0c62e5ab06045a064b4eb1794) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "superdq", 0, NO_DUMP )
ROM_END

ROM_START( superdqa )       /* short scenes, alternate */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sdq_c45a.rom", 0x0000, 0x4000, CRC(b12ce1f8) SHA1(3f0238ea73a6d3e1fe62f83ed3343ca4c268bdd6) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sdq_a8.rom", 0x0000, 0x2000, CRC(7d981a14) SHA1(0a0949113b80c30adbb5bdb108d396993225be5b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sdq-cprm.bin", 0x0000, 0x0020, CRC(96701569) SHA1(b0f40373735d1af0c62e5ab06045a064b4eb1794) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "superdq", 0, NO_DUMP )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1984, superdq,  0,        superdq, superdq, superdq_state, empty_init, ROT0, "Universal", "Super Don Quix-ote (Long Scenes)",       MACHINE_NOT_WORKING )
GAME( 1984, superdqs, superdq,  superdq, superdq, superdq_state, empty_init, ROT0, "Universal", "Super Don Quix-ote (Short Scenes)",      MACHINE_NOT_WORKING )
GAME( 1984, superdqa, superdq,  superdq, superdq, superdq_state, empty_init, ROT0, "Universal", "Super Don Quix-ote (Short Scenes, Alt)", MACHINE_NOT_WORKING )
