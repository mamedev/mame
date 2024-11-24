// license:BSD-3-Clause
// copyright-holders: Mike Coates, Pierpaolo Prazzoli

/************************************************************************

Zaccaria Quasar

Driver by Mike Coates and Pierpaolo Prazzoli

TODO:
- Sound (missing invader effect - still not sure all noise in correct places)
- Phase 3 - seems awfully hard - dip settings ?
- Hook up Coin Counter
- Test/Service Mode - not working?
- No attract demo? is this correct?

*************************************************************************

Quasar by Zaccaria (1980)

1K files were 2708
2K files were 2716
512 file was an 82S130 (colour and priority PROM)

2650A CPU

I8085 Sound Board

************************************************************************/


#include "emu.h"

#include "cvs_base.h"

#include "cpu/mcs48/mcs48.h"

#include "speaker.h"


namespace {

class quasar_state : public cvs_base_state
{
public:
	quasar_state(const machine_config &mconfig, device_type type, const char *tag)
		: cvs_base_state(mconfig, type, tag)
		, m_soundlatch(*this, "soundlatch")
		, m_in(*this, "IN%u", 0U)
		, m_dsw(*this, "DSW%u", 0U)
		, m_effectram(*this, "effectram", 0x400, ENDIANNESS_BIG)
	{ }

	void quasar(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<generic_latch_8_device> m_soundlatch;

	required_ioport_array<2> m_in;
	required_ioport_array<3> m_dsw;

	memory_share_creator<uint8_t> m_effectram;

	uint8_t m_effectcontrol = 0U;
	uint8_t m_page = 0U;
	uint8_t m_io_page = 0U;

	void video_page_select_w(offs_t offset, uint8_t data);
	void io_page_select_w(offs_t offset, uint8_t data);
	void video_w(offs_t offset, uint8_t data);
	uint8_t io_r();
	void bullet_w(offs_t offset, uint8_t data);
	void sh_command_w(uint8_t data);
	uint8_t sh_command_r();
	int audio_t1_r();
	void palette(palette_device &palette) const ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program(address_map &map) ATTR_COLD;
	void data(address_map &map) ATTR_COLD;
	void io(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Zaccaria S2650 games share various levels of design with the Century Video
  System (CVS) games, and hence some routines are shared from there.

  Shooting seems to mix custom boards from Zaccaria and sound boards from CVS
  hinting at a strong link between the two companies.

  Zaccaria are an Italian company, Century were based in Manchester UK

***************************************************************************/

void quasar_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	// standard 1 bit per color palette (background and sprites)
	for (int i = 0; i < 8; i++)
		palette.set_indirect_color(i, rgb_t(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2)));

	// effects color map
	for (int i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(i, 0);
		bit1 = BIT(i, 1);
		bit2 = BIT(i, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(i, 3);
		bit1 = BIT(i, 4);
		bit2 = BIT(i, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(i, 6);
		bit1 = BIT(i, 7);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		// intensity 0
		palette.set_indirect_color(0x100 + i, rgb_t::black());

		// intensity 1
		palette.set_indirect_color(0x200 + i, rgb_t(r >> 2, g >> 2, b >> 2));

		// intensity 2
		palette.set_indirect_color(0x300 + i, rgb_t((r >> 2) + (r >> 3), (g >> 2) + (g >> 3), (b >> 2) + (b >> 2)));

		// intensity 3
		palette.set_indirect_color(0x400 + i, rgb_t(r >> 1, g >> 1, b >> 1));
	}

	// Address 0-2 from graphic ROM
	//         3-5 from color RAM
	//         6-8 from sprite chips (Used for priority)
	for (int i = 0; i < 0x200; i++)
		palette.set_pen_indirect(i, color_prom[i] & 0x07);

	// background for collision
	for (int i = 1; i < 8; i++)
		palette.set_pen_indirect(0x200 + i, 7);
	palette.set_pen_indirect(0x200, 0);

	// effects
	for (int i = 0; i < 0x400; i++)
		palette.set_pen_indirect(0x208 + i, 0x100 + i);
}


void quasar_state::video_start()
{
	// create helper bitmap
	m_screen->register_screen_bitmap(m_collision_background);

	// register save
	save_item(NAME(m_collision_background));
}

uint32_t quasar_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// for every character in the video RAM
	for (int offs = 0; offs < 0x0400; offs++)
	{
		uint8_t const code = m_video_ram[offs];
		uint8_t const x = (offs & 0x1f) << 3;
		uint8_t const y = (offs >> 5) << 3;

		// While we have the current character code, draw the effects layer
		// intensity / on and off controlled by latch

		int const forecolor = 0x208 + m_effectram[offs] + (256 * (((m_effectcontrol >> 4) ^ 3) & 3));

		for (int ox = 0; ox < 8; ox++)
			for (int oy = 0; oy < 8; oy++)
				bitmap.pix(y + oy, x + ox) = forecolor;

		// Main Screen
		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				code,
				m_color_ram[offs] & 0x3f,
				0, 0,
				x, y, 0);


		// background for Collision Detection (it can only hit certain items)
		if ((m_color_ram[offs] & 7) == 0)
		{
			m_gfxdecode->gfx(0)->opaque(m_collision_background, cliprect,
					code,
					64,
					0, 0,
					x, y);
		}
	}

	// update the S2636 chips
	bitmap_ind16 const &s2636_0_bitmap = m_s2636[0]->update(cliprect);
	bitmap_ind16 const &s2636_1_bitmap = m_s2636[1]->update(cliprect);
	bitmap_ind16 const &s2636_2_bitmap = m_s2636[2]->update(cliprect);

	// Bullet Hardware
	for (int offs = 8; offs < 256; offs++)
	{
		if (m_bullet_ram[offs] != 0)
		{
			for (int ct = 0; ct < 1; ct++)
			{
				int const bx = 255 - 9 - m_bullet_ram[offs] - ct;

				// bullet/object Collision
				if (s2636_0_bitmap.pix(offs, bx) != 0) m_collision_register |= 0x04;
				if (s2636_2_bitmap.pix(offs, bx) != 0) m_collision_register |= 0x08;

				bitmap.pix(offs, bx) = 7;
			}
		}
	}


	// mix and copy the S2636 images into the main bitmap, also check for collision
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			int const pixel0 = s2636_0_bitmap.pix(y, x);
			int const pixel1 = s2636_1_bitmap.pix(y, x);
			int const pixel2 = s2636_2_bitmap.pix(y, x);

			int const pixel = pixel0 | pixel1 | pixel2;

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				bitmap.pix(y, x) = S2636_PIXEL_COLOR(pixel);

				// S2636 vs. background collision detection
				if (m_palette->pen_indirect(m_collision_background.pix(y, x)))
				{
					if (S2636_IS_PIXEL_DRAWN(pixel0)) m_collision_register |= 0x01;
					if (S2636_IS_PIXEL_DRAWN(pixel2)) m_collision_register |= 0x02;
				}
			}
		}
	}

	return 0;
}


/************************************************************************

  Quasar memory layout

  Paging for screen is controlled by OUT to 0,1,2 or 3

  Paging for IO ports is controlled by OUT to 8,9,A or B

************************************************************************/

void quasar_state::video_page_select_w(offs_t offset, uint8_t data)
{
	m_page = offset & 0x03;
}

void quasar_state::io_page_select_w(offs_t offset, uint8_t data)
{
	m_io_page = offset & 0x03;
}

void quasar_state::video_w(offs_t offset, uint8_t data)
{
	switch (m_page)
	{
	case 0: m_video_ram[offset] = data; break;
	case 1: m_color_ram[offset] = data & 7; break; // 3 bits of ram only - 3 x 2102
	case 2: m_effectram[offset] = data; break;
	case 3: m_effectcontrol = data; break;
	}
}

uint8_t quasar_state::io_r()
{
	uint8_t ans = 0;

	switch (m_io_page)
	{
	case 0: ans = m_in[0]->read(); break;
	case 1: ans = m_in[1]->read(); break;
	case 2: ans = m_dsw[0]->read(); break;
	case 3: ans = m_dsw[1]->read(); break;
	}

	return ans;
}

void quasar_state::bullet_w(offs_t offset, uint8_t data)
{
	m_bullet_ram[offset] = (data ^ 0xff);
}

void quasar_state::sh_command_w(uint8_t data)
{
	// bit 4 = Sound Invader : Linked to an NE555V circuit
	// Not handled yet

	// lower nibble = command to I8035
	// not necessarily like this, but it seems to work better than direct mapping
	// (although schematics has it as direct - but then the schematics are wrong elsewhere to!)
	m_soundlatch->write(bitswap<4>(data,3,0,2,1));
}

uint8_t quasar_state::sh_command_r()
{
	return m_soundlatch->read() + (m_dsw[2]->read() & 0x30);
}

int quasar_state::audio_t1_r()
{
	return m_soundlatch->read() == 0;
}

// memory map taken from the manual

void quasar_state::program(address_map &map)
{
	map(0x0000, 0x13ff).rom();
	map(0x1400, 0x14ff).mirror(0x6000).ram().w(FUNC(quasar_state::bullet_w)).share(m_bullet_ram);
	map(0x1500, 0x15ff).mirror(0x6000).rw(m_s2636[0], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1600, 0x16ff).mirror(0x6000).rw(m_s2636[1], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1700, 0x17ff).mirror(0x6000).rw(m_s2636[2], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1800, 0x1bff).mirror(0x6000).ram().w(FUNC(quasar_state::video_w)).share(m_color_ram);
	map(0x1c00, 0x1fff).mirror(0x6000).ram();
	map(0x2000, 0x33ff).rom();
	map(0x4000, 0x53ff).rom();
	map(0x6000, 0x73ff).rom();
}

void quasar_state::io(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(quasar_state::io_r), FUNC(quasar_state::video_page_select_w));
	map(0x08, 0x0b).w(FUNC(quasar_state::io_page_select_w));
}

void quasar_state::data(address_map &map)
{
	map(S2650_CTRL_PORT, S2650_CTRL_PORT).r(FUNC(quasar_state::collision_r)).nopw();
	map(S2650_DATA_PORT, S2650_DATA_PORT).rw(FUNC(quasar_state::collision_clear), FUNC(quasar_state::sh_command_w));
}

/*************************************
 *
 *  Sound board memory handlers
 *
 *************************************/

void quasar_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
}

void quasar_state::sound_portmap(address_map &map)
{
	map(0x00, 0x7f).ram();
	map(0x80, 0x80).r(FUNC(quasar_state::sh_command_r));
}

/************************************************************************

  Inputs

************************************************************************/

static INPUT_PORTS_START( quasar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )            // test switch

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )            // table (from manual)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )            // count enable (from manual)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Coin_A ) )           // confirmed
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) )           // confirmed
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x30, 0x00, "Number of Rockets" )         // confirmed
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Free_Play ) ) // requires initial coin, but doesn't decrease coins on game over
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode" )                 // confirmed
	PORT_DIPSETTING(    0x00, "Collisions excluded" )
	PORT_DIPSETTING(    0x80, "Collisions included" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x01, "High Score" )
	PORT_DIPSETTING(    0x00, "No H.S." ) // this option only wants bit 0 OFF
	PORT_DIPSETTING(    0x01, "Normal H.S." )
	PORT_DIPSETTING(    0x03, "Low H.S." )
	PORT_DIPSETTING(    0x05, "Medium H.S." )
	PORT_DIPSETTING(    0x07, "High H.S." )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x60, 0x20, "Extended Play" )
	PORT_DIPSETTING(    0x20, "5500" )                      // confirmed
	PORT_DIPSETTING(    0x40, "7500" )
	PORT_DIPSETTING(    0x60, "9500" )
	PORT_DIPSETTING(    0x00, "17500" )
	PORT_DIPNAME( 0x80, 0x80, "Full Screen Rocket" )        // confirmed
	PORT_DIPSETTING(    0x80, "Stop at edge" )
	PORT_DIPSETTING(    0x00, "Wrap Around" )

	PORT_START("DSW2")
#if 0
	PORT_DIPNAME( 0x0f, 0x00, "Noise to play" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x01, "01" )
	PORT_DIPSETTING(    0x02, "02" )
	PORT_DIPSETTING(    0x03, "03" )
	PORT_DIPSETTING(    0x04, "04" )
	PORT_DIPSETTING(    0x05, "05" )
	PORT_DIPSETTING(    0x06, "06" )
	PORT_DIPSETTING(    0x07, "07" )
	PORT_DIPSETTING(    0x08, "08" )
	PORT_DIPSETTING(    0x09, "09" )
	PORT_DIPSETTING(    0x0a, "0A" )
	PORT_DIPSETTING(    0x0b, "0B" )
	PORT_DIPSETTING(    0x0c, "0C" )
	PORT_DIPSETTING(    0x0d, "0D" )
	PORT_DIPSETTING(    0x0e, "0E" )
	PORT_DIPSETTING(    0x0f, "0F" )
#endif
	PORT_DIPNAME( 0x10, 0x10, "Sound Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

// S2636 mappings

static GFXDECODE_START( gfx_quasar )
	GFXDECODE_ENTRY( "tiles", 0x0000, charlayout, 0, 64+1 )
GFXDECODE_END

// ****************************************
// Quasar S2650 Main CPU, I8035 sound board
// ****************************************

void quasar_state::machine_start()
{
	cvs_base_state::machine_start();

	// register state save
	save_item(NAME(m_effectcontrol));
	save_item(NAME(m_page));
	save_item(NAME(m_io_page));
}

void quasar_state::machine_reset()
{
	cvs_base_state::machine_reset();

	m_effectcontrol = 0;
	m_page = 0;
	m_io_page = 8;
}

void quasar_state::quasar(machine_config &config)
{
	// basic machine hardware
	S2650(config, m_maincpu, 14'318'000 / 4);  // 14 mhz crystal divide by 4 on board
	m_maincpu->set_addrmap(AS_PROGRAM, &quasar_state::program);
	m_maincpu->set_addrmap(AS_IO, &quasar_state::io);
	m_maincpu->set_addrmap(AS_DATA, &quasar_state::data);
	m_maincpu->set_vblank_int("screen", FUNC(quasar_state::irq0_line_assert));
	m_maincpu->sense_handler().set("screen", FUNC(screen_device::vblank));
	m_maincpu->intack_handler().set([this]() { m_maincpu->set_input_line(0, CLEAR_LINE); return 0x03; });

	i8035_device &soundcpu(I8035(config, "soundcpu", 6'000'000)); // 6MHz crystal divide by 15 in CPU
	soundcpu.set_addrmap(AS_PROGRAM, &quasar_state::sound_map);
	soundcpu.set_addrmap(AS_IO, &quasar_state::sound_portmap);
	soundcpu.t1_in_cb().set(FUNC(quasar_state::audio_t1_r));
	soundcpu.p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));

	config.set_maximum_quantum(attotime::from_hz(6000));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_refresh_hz(50); // From dot clock
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(256, 256);
	m_screen->set_visarea(1*8+1, 29*8-1, 2*8, 32*8-1);
	m_screen->set_screen_update(FUNC(quasar_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_quasar);
	PALETTE(config, m_palette, FUNC(quasar_state::palette), (64 + 1) * 8 + (4 * 256), 0x500);

	S2636(config, m_s2636[0], 0);
	m_s2636[0]->set_offsets(CVS_S2636_Y_OFFSET - 8, CVS_S2636_X_OFFSET - 9);

	S2636(config, m_s2636[1], 0);
	m_s2636[1]->set_offsets(CVS_S2636_Y_OFFSET - 8, CVS_S2636_X_OFFSET - 9);

	S2636(config, m_s2636[2], 0);
	m_s2636[2]->set_offsets(CVS_S2636_Y_OFFSET - 8, CVS_S2636_X_OFFSET - 9);

	// sound hardware
	GENERIC_LATCH_8(config, m_soundlatch);

	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 1.0); // unknown DAC
}

ROM_START( quasar )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "7b_01.bin",    0x0000, 0x0400, CRC(20a7feaf) SHA1(ab89087efca2fcb9568f49ba117755ae2c1bd3a3) )
	ROM_CONTINUE(             0x4000, 0x0400 )
	ROM_LOAD( "6b_02.bin",    0x0400, 0x0400, CRC(c14af4a1) SHA1(ca2d3aff94db43aa7c25d33b345a53f484f679cd) )
	ROM_CONTINUE(             0x4400, 0x0400 )
	ROM_LOAD( "5b_03.bin",    0x0800, 0x0400, CRC(3f051d8b) SHA1(1dd7a5eddfb0d7871705ac9ec1b9c16c2b80ddf0) )
	ROM_CONTINUE(             0x4800, 0x0400 )
	ROM_LOAD( "3b_04.bin",    0x0c00, 0x0400, CRC(e14d04ed) SHA1(3176902e3efd72946468c7e7a221d88fcbf63c97) )
	ROM_CONTINUE(             0x4c00, 0x0400 )
	ROM_LOAD( "2b_05.bin",    0x1000, 0x0400, CRC(f2113222) SHA1(576e0ac92ba076e00eeeae73892246f92fff252a) )
	ROM_CONTINUE(             0x5000, 0x0400 )
	ROM_LOAD( "7c_06.bin",    0x2000, 0x0400, CRC(f7f1267d) SHA1(29c99191b0b6186af6772d04543a5fd235f5eafd) )
	ROM_LOAD( "6c_07.bin",    0x2400, 0x0400, CRC(772004eb) SHA1(bfafb6005a1a0cff39b76ec0ad4ea1f438a2f174) )
	ROM_LOAD( "5c_08.bin",    0x2800, 0x0400, CRC(7a87b6f3) SHA1(213b8ccd7bdd650e19d2746b2d617c1950ba3d2b) )
	ROM_LOAD( "3c_09.bin",    0x2c00, 0x0400, CRC(ef87c2cb) SHA1(1ba10dd3996c047e595c54a37c1abb44df3b63c6) )
	ROM_LOAD( "2c_10.bin",    0x3000, 0x0400, CRC(be6c4f84) SHA1(b3a779457bd0d33ccb23c21a7e7cd4a6fc78bb7f) )

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "quasar.snd",   0x0000, 0x0800, CRC(9e489768) SHA1(a9f01ef0a6512543bbdfec56037f37a0440b2b94) )

	ROM_REGION( 0x1800, "tiles", 0 )
	ROM_LOAD( "6g.bin",       0x0000, 0x0800, CRC(deb2f4a9) SHA1(9a15d07a9b35bef34ce923973fc59fbd911f8111) )
	ROM_LOAD( "7g.bin",       0x0800, 0x0800, CRC(76222f30) SHA1(937286fcb50bd0a61db9e71e04b5eb1a0746e1c0) )
	ROM_LOAD( "9g.bin",       0x1000, 0x0800, CRC(fd0a36e9) SHA1(93f1207a36418b9ab15a25163a092308b9916528) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "12m_q.bin",    0x0000, 0x0200, CRC(1ab8633d) SHA1(3aed29f2326676a8d8a5de6f6bb923b6510896d8) )
ROM_END

ROM_START( quasara )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "7b_01.bin",    0x0000, 0x0400, CRC(20a7feaf) SHA1(ab89087efca2fcb9568f49ba117755ae2c1bd3a3) )
	ROM_CONTINUE(             0x4000, 0x0400 )
	ROM_LOAD( "6b_02.bin",    0x0400, 0x0400, CRC(c14af4a1) SHA1(ca2d3aff94db43aa7c25d33b345a53f484f679cd) )
	ROM_CONTINUE(             0x4400, 0x0400 )
	ROM_LOAD( "5b_03.bin",    0x0800, 0x0400, CRC(3f051d8b) SHA1(1dd7a5eddfb0d7871705ac9ec1b9c16c2b80ddf0) )
	ROM_CONTINUE(             0x4800, 0x0400 )
	ROM_LOAD( "3b_04.bin",    0x0c00, 0x0400, CRC(e14d04ed) SHA1(3176902e3efd72946468c7e7a221d88fcbf63c97) )
	ROM_CONTINUE(             0x4c00, 0x0400 )
	ROM_LOAD( "2b_05.bin",    0x1000, 0x0400, CRC(f2113222) SHA1(576e0ac92ba076e00eeeae73892246f92fff252a) )
	ROM_CONTINUE(             0x5000, 0x0400 )
	ROM_LOAD( "7c_06.bin",    0x2000, 0x0400, CRC(f7f1267d) SHA1(29c99191b0b6186af6772d04543a5fd235f5eafd) )
	ROM_LOAD( "6c_07.bin",    0x2400, 0x0400, CRC(772004eb) SHA1(bfafb6005a1a0cff39b76ec0ad4ea1f438a2f174) )
	ROM_LOAD( "5c_08.bin",    0x2800, 0x0400, CRC(7a87b6f3) SHA1(213b8ccd7bdd650e19d2746b2d617c1950ba3d2b) )
	ROM_LOAD( "3c_09.bin",    0x2c00, 0x0400, CRC(ef87c2cb) SHA1(1ba10dd3996c047e595c54a37c1abb44df3b63c6) )
	ROM_LOAD( "2c_10a.bin",   0x3000, 0x0400, CRC(a31c0435) SHA1(48e1c5da455610145310dfe4c6b6e4302b531876) ) // different from quasar set

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "quasar.snd",   0x0000, 0x0800, CRC(9e489768) SHA1(a9f01ef0a6512543bbdfec56037f37a0440b2b94) )

	ROM_REGION( 0x1800, "tiles", 0 )
	ROM_LOAD( "6g.bin",       0x0000, 0x0800, CRC(deb2f4a9) SHA1(9a15d07a9b35bef34ce923973fc59fbd911f8111) )
	ROM_LOAD( "7g.bin",       0x0800, 0x0800, CRC(76222f30) SHA1(937286fcb50bd0a61db9e71e04b5eb1a0746e1c0) )
	ROM_LOAD( "9g.bin",       0x1000, 0x0800, CRC(fd0a36e9) SHA1(93f1207a36418b9ab15a25163a092308b9916528) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "12m_q.bin",    0x0000, 0x0200, CRC(1ab8633d) SHA1(3aed29f2326676a8d8a5de6f6bb923b6510896d8) )
ROM_END

} // anonymous namespace


GAME( 1980, quasar,   0,      quasar,   quasar, quasar_state, empty_init, ROT90, "Zaccaria / Zelco", "Quasar (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, quasara,  quasar, quasar,   quasar, quasar_state, empty_init, ROT90, "Zaccaria / Zelco", "Quasar (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
