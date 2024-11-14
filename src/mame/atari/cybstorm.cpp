// license:BSD-3-Clause
// copyright-holders: Aaron Giles, Phil Bennett

/***************************************************************************

    Atari Cyberstorm hardware

    driver by Aaron Giles and Phil Bennett

    Games supported:
        * Cyberstorm (prototype) (1993)

    Known bugs:
        * STAIN effect not 100% correct

    Possible bugs:
        * Tilemap offsets shifted by 1 pixel?

***************************************************************************/


#include "emu.h"

#include "atarijsa.h"
#include "atarivad.h"

#include "cpu/m68000/m68020.h"
#include "machine/bankdev.h"
#include "machine/eeprompar.h"
#include "machine/watchdog.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class cybstorm_state : public driver_device
{
public:
	cybstorm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_jsa(*this, "jsa")
		, m_vad(*this, "vad")
		, m_vadbank(*this, "vadbank")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
	{ }

	void cybstorm(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<atari_jsa_iiis_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_device<address_map_bank_device> m_vadbank;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t m_latch_data = 0U;
	uint8_t m_alpha_tile_bank = 0U;

	static const atari_motion_objects_config s_mob_config;

	void latch_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void round2(machine_config &config);
	void main_map(address_map &map) ATTR_COLD;
	void vadbank_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(cybstorm_state::get_alpha_tile_info)
{
	uint16_t const data = m_vad->alpha().basemem_read(tile_index);
	int const code = ((data & 0x400) ? (m_alpha_tile_bank * 0x400) : 0) + (data & 0x3ff);
	int const color = (data >> 11) & 0x0f;
	int const opaque = data & 0x8000;
	tileinfo.set(2, code, color, opaque ? TILEMAP_PIXEL_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(cybstorm_state::get_playfield_tile_info)
{
	uint16_t const data1 = m_vad->playfield().basemem_read(tile_index);
	uint16_t const data2 = m_vad->playfield().extmem_read(tile_index) & 0xff;
	int const code = data1;
	int const color = 8 + (data2 & 0x07);
	tileinfo.set(0, code, color, data2 & 0x80 ? TILE_FLIPX : 0);
	tileinfo.category = (data2 >> 4) & 3;
}


TILE_GET_INFO_MEMBER(cybstorm_state::get_playfield2_tile_info)
{
	uint16_t const data1 = m_vad->playfield2().basemem_read(tile_index);
	uint16_t const data2 = m_vad->playfield2().extmem_read(tile_index) >> 8;
	int const code = data1;
	int const color = data2 & 0x07;
	tileinfo.set(0, code, color, data2 & 0x80 ? TILE_FLIPX : 0);
	tileinfo.category = (data2 >> 4) & 3;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config cybstorm_state::s_mob_config =
{
	1,                  // index to which gfx system
	1,                  // number of motion object banks
	1,                  // are the entries linked?
	0,                  // are the entries split?
	1,                  // render in reverse order?
	0,                  // render in swapped X/Y order?
	0,                  // does the neighbor bit affect the next object?
	8,                  // pixels per SLIP entry (0 for no-slip)
	0,                  // pixel offset for SLIPs
	0,                  // maximum number of links to visit/scanline (0=all)

	0x1000,             // base palette entry
	0x1000,             // maximum number of colors
	0,                  // transparent pen index

	{{ 0x03ff,0,0,0 }}, // mask for the link
	{{ 0,0x7fff,0,0 }, { 0x3c00,0,0,0 }},   // mask for the code index
	{{ 0,0,0x000f,0 }}, // mask for the color
	{{ 0,0,0xff80,0 }}, // mask for the X position
	{{ 0,0,0,0xff80 }}, // mask for the Y position
	{{ 0,0,0,0x0070 }}, // mask for the width, in tiles*/
	{{ 0,0,0,0x0007 }}, // mask for the height, in tiles
	{{ 0,0x8000,0,0 }}, // mask for the horizontal flip
	{{ 0 }},            // mask for the vertical flip
	{{ 0,0,0x0070,0 }}, // mask for the priority
	{{ 0 }},            // mask for the neighbor
	{{ 0 }},            // mask for absolute coordinates

	{{ 0 }},            // mask for the special value
	0,                  // resulting value to indicate "special"
};

void cybstorm_state::video_start()
{
	// motion objects have 256 color granularity
	m_gfxdecode->gfx(1)->set_granularity(256);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t cybstorm_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_vad->mob().draw_async(cliprect);

	// draw the playfield
	bitmap_ind8 &priority_bitmap = screen.priority();
	priority_bitmap.fill(0, cliprect);
	m_vad->playfield().draw(screen, bitmap, cliprect, 0, 0x00);
	m_vad->playfield().draw(screen, bitmap, cliprect, 1, 0x01);
	m_vad->playfield().draw(screen, bitmap, cliprect, 2, 0x02);
	m_vad->playfield().draw(screen, bitmap, cliprect, 3, 0x03);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 0, 0x80);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 1, 0x84);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 2, 0x88);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 3, 0x8c);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_vad->mob().bitmap();
	for (const sparse_dirty_rect *rect = m_vad->mob().first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			uint8_t const *const pri = &priority_bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x])
				{
					int const mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					// upper bit of MO priority signals special rendering and doesn't draw anything
					if (mopriority & 4)
					{
						if ((mopriority & 0x3) != 0)
							continue;
					}

					// foreground playfield case
					if (pri[x] & 0x80)
					{
						int const pfpriority = (pri[x] >> 2) & 3;

						if (mopriority > pfpriority)
							pf[x] = (mo[x] & atari_motion_objects_device::DATA_MASK) | 0x1000;
					}

					// background playfield case
					else
					{
						int const pfpriority = pri[x] & 3;

						// playfield priority 3 always wins
						if (pfpriority == 3)
							;

						// otherwise, MOs get shown
						else
							pf[x] = (mo[x] & atari_motion_objects_device::DATA_MASK) | 0x1000;
					}

					// don't erase yet -- we need to make another pass later
				}
		}

	// now go back and process the upper bit of MO priority
	for (const sparse_dirty_rect *rect = m_vad->mob().first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			int count = 0;
			for (int x = rect->left(); x <= rect->right() || (count && x < bitmap.width()); x++)
			{
				const uint16_t START_MARKER = ((4 << atari_motion_objects_device::PRIORITY_SHIFT) | 3);
				const uint16_t END_MARKER =   ((4 << atari_motion_objects_device::PRIORITY_SHIFT) | 7);
				const uint16_t MASK = ((4 << atari_motion_objects_device::PRIORITY_SHIFT) | 0x3f);

				// TODO: Stain pixels should not overlap sprites!
				if ((mo[x] & MASK) == START_MARKER)
				{
					count++;
				}

				if (count)
				{
					// Only applies to PF pixels
					if ((pf[x] & 0x1000) == 0)
					{
						pf[x] |= 0x2000;
					}
				}

				if ((mo[x] & MASK) == END_MARKER)
				{
					count--;
				}

				// erase behind ourselves
				mo[x] = 0;
			}
		}

	// add the alpha on top
	m_vad->alpha().draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


/*************************************
 *
 *  Initialization
 *
 *************************************/

void cybstorm_state::machine_start()
{
	save_item(NAME(m_latch_data));
	save_item(NAME(m_alpha_tile_bank));
}



/*************************************
 *
 *  I/O handling
 *
 *************************************/

void cybstorm_state::latch_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t oldword = m_latch_data;
	COMBINE_DATA(&m_latch_data);

	// bit 4 is connected to the /RESET pin on the 6502
	m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, m_latch_data & 0x00100000 ? CLEAR_LINE : ASSERT_LINE);

	// alpha bank is selected by the upper 4 bits
	if ((oldword ^ m_latch_data) & 0x00e00000)
	{
		m_screen->update_partial(m_screen->vpos());
		m_vad->alpha().mark_all_dirty();
		m_alpha_tile_bank = (m_latch_data >> 21) & 7;
	}
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void cybstorm_state::main_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x20ffff).ram().w("palette", FUNC(palette_device::write32)).share("palette");
	map(0x3effc0, 0x3effff).rw(m_vad, FUNC(atari_vad_device::control_read), FUNC(atari_vad_device::control_write));
	map(0x3f0000, 0x3fffff).m(m_vadbank, FUNC(address_map_bank_device::amap16));
	map(0x9f0000, 0x9f0003).portr("9F0000");
	map(0x9f0010, 0x9f0013).portr("9F0010");
	map(0x9f0031, 0x9f0031).r(m_jsa, FUNC(atari_jsa_iii_device::main_response_r));
	map(0x9f0041, 0x9f0041).w(m_jsa, FUNC(atari_jsa_iii_device::main_command_w));
	map(0x9f0050, 0x9f0053).w(FUNC(cybstorm_state::latch_w));
	map(0xfb0000, 0xfb0003).w("watchdog", FUNC(watchdog_timer_device::reset32_w));
	map(0xfc0000, 0xfc0003).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write32));
	map(0xfd0000, 0xfd0fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask32(0xff00ff00);
	map(0xfe0000, 0xffffff).ram();
}

void cybstorm_state::vadbank_map(address_map &map)
{
	map(0x000000, 0x001fff).ram().w(m_vad, FUNC(atari_vad_device::playfield2_latched_msb_w)).share("vad:playfield2");
	map(0x002000, 0x003fff).ram().w(m_vad, FUNC(atari_vad_device::playfield_latched_lsb_w)).share("vad:playfield");
	map(0x004000, 0x005fff).ram().w(m_vad, FUNC(atari_vad_device::playfield_upper_w)).share("vad:playfield_ext");
	map(0x006000, 0x007fff).ram().share("vad:mob");
	map(0x008000, 0x008eff).w(m_vad, FUNC(atari_vad_device::alpha_w)).share("vad:alpha");
	map(0x008f00, 0x008f7f).ram().share("vad:eof");
	map(0x008f80, 0x008fff).ram().share("vad:mob:slip");
	map(0x009000, 0x00ffff).ram();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( cybstorm )
	PORT_START("9F0000")
	// are these 8 inputs old debugging inputs no longer used by the game, only tested in test mode?
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED ) // White D0 in test mode
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED ) // Red D1 in test mode
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED ) // Freeze D2 in test mode
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED ) // Step D3 in test mode
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED ) // Right
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED ) // Left
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED ) // Down
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) // Up

	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Jab Punch")
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Strong Punch")
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Fierce Punch")
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_UNUSED ) // Blue Button in test mode
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Quick Kick")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Strong Kick")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Fierce Kick")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Quick Kick")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Strong Kick")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Fierce Kick")
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED ) // Green Button in test mode

	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Jab Punch")
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Strong Punch")
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Fierce Punch")
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("9F0010")
	// are these old debug Coin / Service inputs? (coins are read through sound board instead?) still tested in test mode
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED ) // LCOIN
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED ) // CLCOIN
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED ) // CRCOIN
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED ) // RCOIN
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED ) // Service
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED ) // Service
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED ) // Service
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) // Service

	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNUSED ) // not tested

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_HBLANK("screen")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_CUSTOM )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_SERVICE( 0x00400000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED ) // not tested
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout anlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};


static const gfx_layout pflayout =
{
	8,8,
	RGN_FRAC(1,8),
	8,
	{ RGN_FRAC(7,8), RGN_FRAC(6,8), RGN_FRAC(5,8), RGN_FRAC(4,8), RGN_FRAC(3,8), RGN_FRAC(2,8), RGN_FRAC(1,8), RGN_FRAC(0,8) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( gfx_cybstorm )
	GFXDECODE_ENTRY( "spr_pf1",  0, pflayout,             0, 16 )       // sprites & playfield
	GFXDECODE_ENTRY( "spr_pf2",  0, gfx_8x8x6_planar,  4096, 64 )       // sprites & playfield
	GFXDECODE_ENTRY( "chars",    0, anlayout,         16384, 64 )       // characters 8x8
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void cybstorm_state::round2(machine_config &config)
{
	// basic machine hardware
	M68EC020(config, m_maincpu, 14.318181_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cybstorm_state::main_map);

	EEPROM_2816(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	ATARI_VAD(config, m_vad, 0, m_screen);
	m_vad->scanline_int_cb().set_inputline(m_maincpu, M68K_IRQ_4);
	TILEMAP(config, "vad:playfield", m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64).set_info_callback(FUNC(cybstorm_state::get_playfield_tile_info));
	TILEMAP(config, "vad:playfield2", m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64, 0).set_info_callback(FUNC(cybstorm_state::get_playfield2_tile_info));
	TILEMAP(config, "vad:alpha", m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_ROWS, 64, 32, 0).set_info_callback(FUNC(cybstorm_state::get_alpha_tile_info));
	ATARI_MOTION_OBJECTS(config, "vad:mob", 0, m_screen, cybstorm_state::s_mob_config).set_gfxdecode(m_gfxdecode);

	ADDRESS_MAP_BANK(config, "vadbank").set_map(&cybstorm_state::vadbank_map).set_options(ENDIANNESS_BIG, 16, 32, 0x90000);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_cybstorm);
	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 32768);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_palette("palette");
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	// note: these parameters are from published specs, not derived
	// the board uses an SOS-2 chip to generate video signals
	m_screen->set_raw(14.318181_MHz_XTAL / 2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(cybstorm_state::screen_update));
}


void cybstorm_state::cybstorm(machine_config &config)
{
	round2(config);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ATARI_JSA_IIIS(config, m_jsa, 0);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	m_jsa->test_read_cb().set_ioport("9F0010").bit(22);
	m_jsa->add_route(0, "lspeaker", 0.9);
	m_jsa->add_route(1, "rspeaker", 0.9);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( cybstorm )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68020 code
	ROM_LOAD32_BYTE( "st_11.22.prog.6a",   0x000000, 0x080000, CRC(8b112ee9) SHA1(cd8367c47c653b8a1ba236c354f009f4297d521d) )
	ROM_LOAD32_BYTE( "st_11.22.prog.8a",   0x000001, 0x080000, CRC(36b7cec9) SHA1(c9c2ba6df1fc849200e0c66a7cbc292e8b0b22f3) )
	ROM_LOAD32_BYTE( "st_11.22.prog2.13a", 0x000002, 0x080000, CRC(1318f2c5) SHA1(929fbe96621852a10b7072490e1e554cdb2f20d8) )
	ROM_LOAD32_BYTE( "st_11.22.prog.16a",  0x000003, 0x080000, CRC(4ae586a8) SHA1(daa803ed38f6582677b397e744dd8f5f60cfb508) )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "st_11.22.6502", 0x010000, 0x004000, CRC(947421b2) SHA1(72b2b66122e779135f1f5af794e4d8513ccbbef6) )
	ROM_CONTINUE(              0x004000, 0x00c000 )

	ROM_REGION( 0x20000, "chars", 0 )
	ROM_LOAD( "st_11.22.csalph.6c", 0x000000, 0x020000, CRC(bafa4bbe) SHA1(c033a952fab6eb3a06c44ba7f48e58b20fe144f0) )

	ROM_REGION( 0x400000, "spr_pf1", ROMREGION_INVERT )
	ROM_LOAD( "st_11.22.pf0", 0x000000, 0x080000, CRC(0cf5874c) SHA1(1d739a3fc42baa5556aa22e051c873db9357396f) )
	ROM_LOAD( "st_11.22.pf1", 0x080000, 0x080000, CRC(ee0a6a81) SHA1(7c36ccbcd51497ea8a872ddf7dabe2ceb0895408) )
	ROM_LOAD( "st_11.22.pf2", 0x100000, 0x080000, CRC(03791514) SHA1(0688b55015f8d86ee92497cb7fcdfbdbfbc492a2) )
	ROM_LOAD( "st_11.22.pf3", 0x180000, 0x080000, CRC(8daf8d8f) SHA1(bfe90c789df5952f2e55c6cceebbf1285ea8d18e) )
	ROM_LOAD( "st_11.22.pf4", 0x200000, 0x080000, CRC(c0f759ab) SHA1(6acbc7c12669c89efeec9218eba03523748e4bf2) )
	ROM_LOAD( "st_11.22.pf5", 0x280000, 0x080000, CRC(921a080e) SHA1(7d3ef110569bacacfa269fab63777bf7ffb4c68e) )
	ROM_LOAD( "st_11.22.pf6", 0x300000, 0x080000, CRC(58b3c0d9) SHA1(226ff2e948c5bb0ca150700a2f3426492fce79f7) )
	ROM_LOAD( "st_11.22.pf7", 0x380000, 0x080000, CRC(f84b27ca) SHA1(a7812e18e15fad9992a59b0ebd177cb848a743bb) )

	ROM_REGION( 0xc00000, "spr_pf2", ROMREGION_INVERT )
	ROM_LOAD( "st_11.22.mo00", 0x000000, 0x080000, CRC(216ffdb9) SHA1(7e6418da1419d82e67bef9ae314781708ed62a76) )
	ROM_LOAD( "st_11.22.mo01", 0x200000, 0x080000, CRC(af15908b) SHA1(9dc8dbf0288a084891bdd646cfb7b8c97b89cf2e) )
	ROM_LOAD( "st_11.22.mo02", 0x400000, 0x080000, CRC(fc066982) SHA1(bbf258ff23619234cb31b4afab4eac1681cdeae0) )
	ROM_LOAD( "st_11.22.mo03", 0x600000, 0x080000, CRC(95c85715) SHA1(585cdb3cadfcd205e7dfcf846230b404093cd018) )
	ROM_LOAD( "st_11.22.mo04", 0x800000, 0x080000, CRC(f53cebc8) SHA1(91280f8bf9f2fbb977f3971324134ffd8eec11b9) )
	ROM_LOAD( "st_11.22.mo05", 0xa00000, 0x080000, CRC(6c696989) SHA1(1a688252faa85a380fd639950068b28de0b50cdf) )
	ROM_LOAD( "st_11.22.mo10", 0x080000, 0x080000, CRC(a65b00da) SHA1(12a482e58207ac6fc2f7a47da1162a38ea902a96) )
	ROM_LOAD( "st_11.22.mo11", 0x280000, 0x080000, CRC(11da3f44) SHA1(18f228524e2a00655f4e965208f99d892741d7cb) )
	ROM_LOAD( "st_11.22.mo12", 0x480000, 0x080000, CRC(44257e7d) SHA1(307f350908b5f8d53495368e19a02e1042d9cb03) )
	ROM_LOAD( "st_11.22.mo13", 0x680000, 0x080000, CRC(8ec4cc3e) SHA1(20e74ce2aa60cd2eb67a39b1bd8cf37454db4776) )
	ROM_LOAD( "st_11.22.mo14", 0x880000, 0x080000, CRC(8f144f42) SHA1(6a35cf399775aaae50eeb35812fcf1343d9f9e1a) )
	ROM_LOAD( "st_11.22.mo15", 0xa80000, 0x080000, CRC(3de4035a) SHA1(ecba9b464eb8b37a85a1c81ee4d7935a11646ed9) )
	ROM_LOAD( "st_11.22.mo20", 0x100000, 0x080000, CRC(6f79ef90) SHA1(e323a7c35dac021c6b32870938dd54e865014078) )
	ROM_LOAD( "st_11.22.mo21", 0x300000, 0x080000, CRC(69726b74) SHA1(31ba5ac1584ba6b63ec4a2189d531319db716690) )
	ROM_LOAD( "st_11.22.mo22", 0x500000, 0x080000, CRC(5323d1f4) SHA1(65e81316467a3e5413ef480ba278a0a70e5c2f51) )
	ROM_LOAD( "st_11.22.mo23", 0x700000, 0x080000, CRC(2387c947) SHA1(26d0f1ee83c5e84df4d143d6d3fd422f34d0d9af) )
	ROM_LOAD( "st_11.22.mo24", 0x900000, 0x080000, CRC(2f133ccf) SHA1(a280879de200eb10e495e8ca804515c0e7d02eb9) )
	ROM_LOAD( "st_11.22.mo25", 0xb00000, 0x080000, CRC(0746b2c5) SHA1(5414c3e600352cfb88d521ed3252941893afe438) )
	ROM_LOAD( "st_11.22.mo30", 0x180000, 0x080000, CRC(7c0f2a9b) SHA1(82a405203356643529017201797b37efb7d1b227) )
	ROM_LOAD( "st_11.22.mo31", 0x380000, 0x080000, CRC(8c21a84c) SHA1(7e84afb2b72af32af731df25fa1dd4d5f4dde7ba) )
	ROM_LOAD( "st_11.22.mo32", 0x580000, 0x080000, CRC(a7382479) SHA1(c694e4ca4111252c3b23e274695ed235dd8a92e2) )
	ROM_LOAD( "st_11.22.mo33", 0x780000, 0x080000, CRC(eac63276) SHA1(d72e85c46ce609b6275280c8cd73ed93aa6eddc3) )
	ROM_LOAD( "st_11.22.mo34", 0x980000, 0x080000, CRC(b8b0c8b6) SHA1(f47218a4d94aa151964687a6e4c02f2b3065fdd3) )
	ROM_LOAD( "st_11.22.mo35", 0xb80000, 0x080000, CRC(f0b9cf9d) SHA1(7ce30b05c1ee02346e8f568f36274b46d1ed99c4) )

	ROM_REGION( 0x100000, "jsa:oki1", 0 ) // ADPCM
	ROM_LOAD( "st_11.22.5a", 0x000000, 0x080000, CRC(d469692c) SHA1(b7d94c042cf9f28ea65d44f5305d56459562d209) )

	ROM_REGION( 0x100000, "jsa:oki2", 0 ) // ADPCM
	ROM_LOAD( "st_11.22.5a", 0x000000, 0x080000, CRC(d469692c) SHA1(b7d94c042cf9f28ea65d44f5305d56459562d209) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1993, cybstorm, 0, cybstorm, cybstorm, cybstorm_state, empty_init, ROT0, "Atari Games", "Cyberstorm (prototype)", MACHINE_SUPPORTS_SAVE )
