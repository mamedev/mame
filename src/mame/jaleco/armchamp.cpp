// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Jaleco/Yuvo Arm Champs hardware
    Nasco X100 boardset

    driver by Phil Bennett

    TODO:
        * Improve arm control. To start the game, wiggle left and right
          rapidly
        * Accurate video timing

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class armchamp_state : public driver_device
{
public:
	armchamp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rombank(*this, "rombank"),
		m_nvram(*this, "nvram"),
		m_ppi(*this, "ppi%u", 0U),
		m_screen(*this, "screen"),
		m_vram(*this, "vram"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_msm(*this, "msm"),
		m_ym(*this, "ym")
	{ }

	void armchamp(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	static constexpr XTAL MASTER_CLOCK = 12_MHz_XTAL;

	uint32_t screen_update_armchamp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_prg(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_tile_info);
	void vram_w(offs_t offset, uint8_t data);
	void io1_w(uint8_t data);
	void io2_w(uint8_t data);
	void io4_w(uint8_t data);
	void adpcm_w(uint8_t data);
	uint8_t arm_r();
	void msm5205_vck(int state);

	required_device<cpu_device> m_maincpu;
	required_memory_bank m_rombank;
	required_device<nvram_device> m_nvram;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_vram;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<msm5205_device> m_msm;
	required_device<ym2149_device> m_ym;

	tilemap_t *m_tilemap;
	uint8_t m_io1 = 0;
	uint8_t m_io2 = 0;
	uint8_t m_io4 = 0;
	uint8_t m_tile_latch = 0;
	uint8_t m_adpcm_byte = 0;
	uint8_t m_msm5205_vclk_toggle = 0;
	uint16_t m_paletteram[0x100] = { };
};



/*************************************
 *
 *  Machine start and reset
 *
 *************************************/

void armchamp_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	m_rombank->configure_entries(0, 8, &ROM[0x10000], 0x8000);

	m_palette->basemem().set(m_paletteram, 0x100, 16, ENDIANNESS_LITTLE, 2);

	save_item(NAME(m_io1));
	save_item(NAME(m_io2));
	save_item(NAME(m_io4));
	save_item(NAME(m_tile_latch));
	save_item(NAME(m_adpcm_byte));
	save_item(NAME(m_msm5205_vclk_toggle));
	save_item(NAME(m_paletteram));
}

void armchamp_state::machine_reset()
{
	m_msm5205_vclk_toggle = 0;
}



/*************************************
 *
 *  Video system start and update
 *
 *************************************/

void armchamp_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(armchamp_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap->set_scrolly(0, -16);
}

uint32_t armchamp_state::screen_update_armchamp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}



/*************************************
 *
 *  VRAM access
 *
 *************************************/

TILE_GET_INFO_MEMBER(armchamp_state::get_tile_info)
{
	// .....xxx xxxxxxxx - Code
	// ....x... ........ - Banked tile
	// xxxx.... ........ - Color

	uint8_t upper = m_vram[tile_index * 2 + 1];
	int code = ((upper & 0x7) << 8) | m_vram[tile_index * 2];
	int color = (upper >> 4) & 0xf;

	if (upper & 0x8)
		code += ((m_io1 >> 3) & 0xf) * 0x800;

	tileinfo.set(0, code, color, 0);
}

void armchamp_state::vram_w(offs_t offset, uint8_t data)
{
	if (BIT(m_io1, 2))
	{
		// Palette RAM access
		m_palette->basemem().write8(offset & 0x1ff, data);
		m_palette->write8(offset & 0x1ff, data);
	}
	else
	{
		// VRAM access
		if (offset & 1)
		{
			m_tile_latch = data;
		}
		else
		{
			m_vram[offset] = data;
			m_vram[offset + 1] = m_tile_latch;
			m_tilemap->mark_tile_dirty(offset / 2);
		}
	}
}

void armchamp_state::io1_w(uint8_t data)
{
	// .... ..xx - ROM bank
	// .... .x.. - Palette/VRAM bank
	// .xxx x... - Tile ROM bank
	// x... .... - Speech ROM banking enable

	m_rombank->set_entry(bitswap<3>(data, 7, 1, 0));

	if ((m_io1 ^ data) & 0x78)
	{
		m_screen->update_partial(m_screen->vpos());
		m_tilemap->mark_all_dirty();
	}

	m_io1 = data;
}

void armchamp_state::io2_w(uint8_t data)
{
	// .... ...x - ? Active as soon as coin is inserted
	// .... ..x. - ?
	// .... .x.. - ?
	// ...x .... - Coin counter
	// ..x. .... - ?
	// x... .... - MSM5205 reset

	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
	m_msm->reset_w(BIT(data, 7) ? 0 : 1);
	m_io2 = data;
}

void armchamp_state::io4_w(uint8_t data)
{
	// .... ....x - ? Flipped every 4 frames

	m_io4 = data;
}

void armchamp_state::adpcm_w(uint8_t data)
{
	m_adpcm_byte = data;
}

uint8_t armchamp_state::arm_r()
{
	// .xxx xxxx - Encoder position
	// x... .... - 'Apeal' sensor (active low)

	// TODO
	return (ioport("ARM")->read() & 0xff);
}

void armchamp_state::msm5205_vck(int state)
{
	if (m_io2 & 0x80)
	{
		if (m_msm5205_vclk_toggle == 0)
		{
			m_msm->data_w(m_adpcm_byte >> 4);
		}
		else
		{
			m_msm->data_w(m_adpcm_byte & 0xf);
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		}

		m_msm5205_vclk_toggle ^= 1;
	}
}



/*************************************
 *
 *  CPU memory map
 *
 *************************************/

void armchamp_state::main_prg(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6800, 0x6fff).ram().share("nvram");
	map(0x7000, 0x7fff).writeonly().w(FUNC(armchamp_state::vram_w)).share(m_vram);
	map(0x8000, 0xffff).bankr("rombank");
}

void armchamp_state::main_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x09, 0x09).r("ym", FUNC(ym2149_device::data_r));
	map(0x0a, 0x0b).w("ym", FUNC(ym2149_device::data_address_w));
	map(0x0c, 0x0c).w(FUNC(armchamp_state::adpcm_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( armchamp )
	PORT_START("DSW-IN")
	PORT_DIPNAME( 0x01, 0x01, "Game Type" ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, "Fixed" )
	PORT_DIPSETTING(    0x00, "Tournament" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Game Rate" ) PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(    0x00, "5 time")
	PORT_DIPSETTING(    0x04, "4 time")
	PORT_DIPSETTING(    0x08, "3 time")
	PORT_DIPSETTING(    0x0c, "2 time")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW-OUT")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Step-4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Step-3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Step-2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Step-1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Lady")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("List")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Maintenance")

	PORT_START("ARM")
	PORT_BIT(0xff, 0x40, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_CENTERDELTA(0) PORT_MINMAX(0x00, 0xff)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static GFXDECODE_START( gfx_armchamp )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_msb, 0, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void armchamp_state::armchamp(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &armchamp_state::main_prg);
	m_maincpu->set_addrmap(AS_IO, &armchamp_state::main_io);
	m_maincpu->set_vblank_int("screen", FUNC(armchamp_state::irq0_line_hold));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	I8255(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->out_pb_callback().set(FUNC(armchamp_state::io1_w));
	m_ppi[0]->out_pc_callback().set(FUNC(armchamp_state::io2_w));
	m_ppi[0]->tri_pc_callback().set_constant(0);

	I8255(config, m_ppi[1]);
	m_ppi[1]->out_pa_callback().set(FUNC(armchamp_state::io4_w));
	m_ppi[1]->in_pb_callback().set(FUNC(armchamp_state::arm_r));
	m_ppi[1]->in_pc_callback().set_ioport("IN1");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 26*8-1);

	screen.set_screen_update(FUNC(armchamp_state::screen_update_armchamp));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_armchamp);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x100);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	YM2149(config, m_ym, MASTER_CLOCK / 8);
	m_ym->add_route(ALL_OUTPUTS, "speaker", 0.1);
	m_ym->port_a_read_callback().set_ioport("DSW-IN");
	m_ym->port_b_read_callback().set_ioport("DSW-OUT");

	MSM5205(config, m_msm, MASTER_CLOCK / 2 / 16);
	m_msm->vck_legacy_callback().set(FUNC(armchamp_state::msm5205_vck));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);
	m_msm->add_route(ALL_OUTPUTS, "speaker", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( armchamp )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x08000, CRC(df9f6fb4) SHA1(a1688f2e097ca2fc3dbaa4964af7f46fdfe5e916) )
	ROM_LOAD( "2.bin", 0x10000, 0x10000, CRC(b064ee62) SHA1(991d5105dd801ba97fa5303a1c4b901e9477a6c6) )
	ROM_LOAD( "3.bin", 0x20000, 0x10000, CRC(f4c95927) SHA1(c5a90cc3799a6a991057431af6556c7d501bad31) )
	ROM_LOAD( "4.bin", 0x30000, 0x10000, CRC(9e5b9e64) SHA1(646bb33d0f2d8c646f6948021f1276e6797b9ab4) )
	ROM_LOAD( "5.bin", 0x40000, 0x10000, CRC(13ab0c54) SHA1(05d0dc1c5bcd9bece09a3e52931391d46d3c0fda) )

	ROM_REGION( 0xb0000, "gfx1", 0 )
	ROM_LOAD( "chrj00.bin", 0x00000, 0x10000, CRC(57ea8a5c) SHA1(94393ef0e26c92f0e1f3813442fcc1cf843610e8) )
	ROM_LOAD( "6.bin",  0x10000, 0x10000, CRC(c117c0ec) SHA1(7cc5c880d3752367f6dab8a54873215b6bb8bcf3) )
	ROM_LOAD( "7.bin",  0x20000, 0x10000, CRC(0b0f4b2d) SHA1(68855f0fa1b392c8396d3a2c64fed01d51102cf4) )
	ROM_LOAD( "8.bin",  0x30000, 0x10000, CRC(f3c7f974) SHA1(c715c36cd382a6604450495b9b18afe68f126a7b) )
	ROM_LOAD( "9.bin",  0x40000, 0x10000, CRC(836932c4) SHA1(66e4c549e5e2ac55faf744a96a3e144f37ada55c) )
	ROM_LOAD( "10.bin", 0x50000, 0x10000, CRC(cc84e61c) SHA1(1799f3f10a05c5752d38fac8e6a2b43199add244) )
	ROM_LOAD( "11.bin", 0x60000, 0x10000, CRC(f22333a3) SHA1(46680b8152b242deaa6ed1a242e4217f1542aea5) )
	ROM_LOAD( "12.bin", 0x70000, 0x10000, CRC(b49e3425) SHA1(b6084e32707cdab71d1d96cc93473579a7e16ab1) )
	ROM_LOAD( "13.bin", 0x80000, 0x10000, CRC(2cc6dc09) SHA1(1c30bf5fab9ee7ecabdb3274748920a058563bad) )
	ROM_LOAD( "14.bin", 0x90000, 0x10000, CRC(7b82bb4a) SHA1(d4dbc5023164762963bf0bdc5d3fa1e0fbf98fab) )
	ROM_LOAD( "15.bin", 0xa0000, 0x10000, CRC(4bf226f1) SHA1(b9b2c2a5822705f18c820942054295f48a27a504) )
ROM_END

} // anonymous namespace



/***************************************************************************

  Game driver(s)

***************************************************************************/

GAME( 1988, armchamp, 0, armchamp, armchamp, armchamp_state, empty_init, ROT90, "Jaleco / Yuvo", "Arm Champs (Japan)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
// An export version also exists
