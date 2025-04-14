// license:BSD-3-Clause
// copyright-holders: Aaron Giles

/***************************************************************************

    Atari Blasteroids hardware

    driver by Aaron Giles

    Games supported:
        * Blasteroids (1987) [5 sets]

    Known bugs:
        * none at this time

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"

#include "atarigen.h"
#include "atarijsa.h"
#include "atarimo.h"

#include "cpu/m68000/m68000.h"
#include "machine/eeprompar.h"
#include "machine/timer.h"
#include "machine/watchdog.h"

#include "emupal.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class blstroid_state : public atarigen_state
{
public:
	blstroid_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
		m_playfield_tilemap(*this, "playfield"),
		m_jsa(*this, "jsa"),
		m_mob(*this, "mob"),
		m_priorityram(*this, "priorityram")
	{ }

	void blstroid(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_CALLBACK_MEMBER(irq_on);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);
	void scanline_int_ack_w(uint16_t data = 0);
	void video_int_ack_w(uint16_t data = 0);
	void halt_until_hblank_0_w(uint16_t data = 0);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<atari_jsa_i_device> m_jsa;
	required_device<atari_motion_objects_device> m_mob;
	required_shared_ptr<uint16_t> m_priorityram;

	emu_timer *m_irq_off_timer = nullptr;
	emu_timer *m_irq_on_timer = nullptr;

	static const atari_motion_objects_config s_mob_config;

	bool m_scanline_int_state = false;
};


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(blstroid_state::get_playfield_tile_info)
{
	uint16_t const data = m_playfield_tilemap->basemem_read(tile_index);
	int const code = data & 0x1fff;
	int const color = (data >> 13) & 0x07;
	tileinfo.set(0, code, color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config blstroid_state::s_mob_config =
{
	1,                  // index to which gfx system
	1,                  // number of motion object banks
	1,                  // are the entries linked?
	0,                  // are the entries split?
	0,                  // render in reverse order?
	0,                  // render in swapped X/Y order?
	0,                  // does the neighbor bit affect the next object?
	0,                  // pixels per SLIP entry (0 for no-slip)
	0,                  // pixel offset for SLIPs
	0,                  // maximum number of links to visit/scanline (0=all)

	0x000,              // base palette entry
	0,                  // transparent pen index

	{{ 0,0,0x0ff8,0 }}, // mask for the link
	{{ 0,0x3fff,0,0 }}, // mask for the code index
	{{ 0,0,0,0x000f }}, // mask for the color
	{{ 0,0,0,0xffc0 }}, // mask for the X position
	{{ 0xff80,0,0,0 }}, // mask for the Y position
	{{ 0 }},            // mask for the width, in tiles
	{{ 0x000f,0,0,0 }}, // mask for the height, in tiles
	{{ 0,0x8000,0,0 }}, // mask for the horizontal flip
	{{ 0,0x4000,0,0 }}, // mask for the vertical flip
	{{ 0 }},            // mask for the priority
	{{ 0 }},            // mask for the neighbor
	{{ 0 }},            // mask for absolute coordinates

	{{ 0 }},            // mask for the special value
	0                  // resulting value to indicate "special"
};

void blstroid_state::video_start()
{
	m_irq_off_timer = timer_alloc(FUNC(blstroid_state::irq_off), this);
	m_irq_on_timer = timer_alloc(FUNC(blstroid_state::irq_on), this);

	m_scanline_int_state = false;

	save_item(NAME(m_scanline_int_state));
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

TIMER_CALLBACK_MEMBER(blstroid_state::irq_off)
{
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(blstroid_state::irq_on)
{
	m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(blstroid_state::scanline_update)
{
	int const scanline = param;
	int const offset = (scanline / 8) * 64 + 40;

	// check for interrupts
	if (offset < 0x800)
		if (BIT(m_playfield_tilemap->basemem_read(offset), 15))
		{
			// FIXME: - the only thing this IRQ does it tweak the starting MO link
			// unfortunately, it does it too early for the given MOs!
			// perhaps it is not actually hooked up on the real PCB...
			return;

			// set a timer to turn the interrupt on at HBLANK of the 7th scanline
			// and another to turn it off one scanline later
			int const width = m_screen->width();
			int const vpos  = m_screen->vpos();
			attotime const period_on  = m_screen->time_until_pos(vpos + 7, width * 0.9);
			attotime const period_off = m_screen->time_until_pos(vpos + 8, width * 0.9);

			m_irq_on_timer->adjust(period_on);
			m_irq_off_timer->adjust(period_off);
		}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t blstroid_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	// draw the playfield
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	m_mob->iterate_dirty_rects(
			cliprect,
			[this, &bitmap, &mobitmap] (rectangle const &rect)
			{
				for (int y = rect.top(); y <= rect.bottom(); y++)
				{
					uint16_t const *const mo = &mobitmap.pix(y);
					uint16_t *const pf = &bitmap.pix(y);
					for (int x = rect.left(); x <= rect.right(); x++)
					{
						if (mo[x] != 0xffff)
						{
							/* verified via schematics

							    priority address = HPPPMMMM
							*/
							int const priaddr = ((pf[x] & 8) << 4) | (pf[x] & 0x70) | ((mo[x] & 0xf0) >> 4);
							if (m_priorityram[priaddr] & 1)
								pf[x] = mo[x];
						}
					}
				}
			});

	return 0;
}


/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

void blstroid_state::scanline_int_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}


void blstroid_state::video_int_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
}


void blstroid_state::halt_until_hblank_0_w(uint16_t data)
{
	halt_until_hblank_0(*m_maincpu, *m_screen);
}


void blstroid_state::machine_reset()
{
	atarigen_state::machine_reset();
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

// full map verified from schematics
void blstroid_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x83ffff);
	map(0x000000, 0x03ffff).mirror(0x000000).rom();
	map(0x800000, 0x800001).mirror(0x0381fe).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x800200, 0x800201).mirror(0x0381fe).w(FUNC(blstroid_state::scanline_int_ack_w));
	map(0x800400, 0x800401).mirror(0x0381fe).w(FUNC(blstroid_state::video_int_ack_w));
	map(0x800600, 0x800601).mirror(0x0381fe).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x800800, 0x8009ff).mirror(0x038000).writeonly().share(m_priorityram);
	map(0x800a01, 0x800a01).mirror(0x0381fe).w(m_jsa, FUNC(atari_jsa_i_device::main_command_w));
	map(0x800c00, 0x800c01).mirror(0x0381fe).w(m_jsa, FUNC(atari_jsa_i_device::sound_reset_w));
	map(0x800e00, 0x800e01).mirror(0x0381fe).w(FUNC(blstroid_state::halt_until_hblank_0_w));
	map(0x801401, 0x801401).mirror(0x0383fe).r(m_jsa, FUNC(atari_jsa_i_device::main_response_r));
	map(0x801800, 0x801801).mirror(0x0383f8).portr("DIAL0");
	map(0x801804, 0x801805).mirror(0x0383f8).portr("DIAL1");
	map(0x801c00, 0x801c01).mirror(0x0383fc).portr("IN0");
	map(0x801c02, 0x801c03).mirror(0x0383fc).portr("IN1");
	map(0x802000, 0x8023ff).mirror(0x038c00).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x803000, 0x8033ff).mirror(0x038c00).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x804000, 0x804fff).mirror(0x038000).ram().w(m_playfield_tilemap, FUNC(tilemap_device::write16)).share("playfield");
	map(0x805000, 0x805fff).mirror(0x038000).ram().share("mob");
	map(0x806000, 0x807fff).mirror(0x038000).ram();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( blstroid )
	PORT_START("DIAL0")     // ff9800
	PORT_BIT( 0x00ff, 0, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIAL1")     // ff9804
	PORT_BIT( 0x00ff, 0, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")       // ff9c00
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::hblank))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")       // ff9c02
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::hblank))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout molayout =
{
	16,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4, RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+12, 8, 12,
			RGN_FRAC(1,2)+16, RGN_FRAC(1,2)+20, 16, 20, RGN_FRAC(1,2)+24, RGN_FRAC(1,2)+28, 24, 28 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8
};


static GFXDECODE_START( gfx_blstroid )
	GFXDECODE_SCALE( "tiles",   0, gfx_8x8x4_packed_msb, 256, 16, 2, 1 )
	GFXDECODE_ENTRY( "sprites", 0, molayout,               0, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void blstroid_state::blstroid(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14.318181_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &blstroid_state::main_map);

	EEPROM_2804(config, "eeprom").lock_after_write(true);

	TIMER(config, "scantimer").configure_scanline(FUNC(blstroid_state::scanline_update), m_screen, 0, 8);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	GFXDECODE(config, "gfxdecode", "palette", gfx_blstroid);

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 512);

	TILEMAP(config, m_playfield_tilemap, m_gfxdecode, 2, 16, 8, TILEMAP_SCAN_ROWS, 64, 32).set_info_callback(FUNC(blstroid_state::get_playfield_tile_info));

	ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, blstroid_state::s_mob_config);
	m_mob->set_gfxdecode(m_gfxdecode);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	// note: these parameters are from published specs, not derived
	// the board uses an SOS-2 chip to generate video signals
	m_screen->set_raw(14.318181_MHz_XTAL, 456*2, 0, 320*2, 262, 0, 240);
	m_screen->set_screen_update(FUNC(blstroid_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set_inputline(m_maincpu, M68K_IRQ_2, ASSERT_LINE);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	ATARI_JSA_I(config, m_jsa, 0);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_4);
	m_jsa->test_read_cb().set_ioport("IN0").bit(7);
	m_jsa->add_route(0, "speaker", 1.0, 0);
	m_jsa->add_route(1, "speaker", 1.0, 1);
	config.device_remove("jsa:pokey");
	config.device_remove("jsa:tms");
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( blstroid )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136057-4123.6c",  0x000000, 0x010000, CRC(d14badc4) SHA1(ccba30e1eb6b3351cbc7ea18951debb7f7aa4520) )
	ROM_LOAD16_BYTE( "136057-4121.6b",  0x000001, 0x010000, CRC(ae3e93e8) SHA1(66ccff68e9b0f7e97abf126f977775e29ce4eee5) )
	ROM_LOAD16_BYTE( "136057-4124.4c",  0x020000, 0x010000, CRC(fd2365df) SHA1(63ed3f9a92fed985f9ddb93687f11a24c8309f56) )
	ROM_LOAD16_BYTE( "136057-4122.4b",  0x020001, 0x010000, CRC(c364706e) SHA1(e03cd60d139000607d83240b0b48865eafb1188b) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136057-1135.2k",  0x00000, 0x10000, CRC(baa8b5fe) SHA1(4af1f9bec3ffa856016a89bc20041d572305ba3a) )

	ROM_REGION( 0x040000, "tiles", 0 )
	ROM_LOAD( "136057-1101.1l",  0x000000, 0x010000, CRC(3c2daa5b) SHA1(2710a05e95afd8452104c4f4a9250a3b7d728a42) )
	ROM_LOAD( "136057-1102.1m",  0x010000, 0x010000, CRC(f84f0b97) SHA1(00cb5f1e0f92742683ee71854085b1e4db4bd6bb) )
	ROM_LOAD( "136057-1103.3l",  0x020000, 0x010000, CRC(ae5274f0) SHA1(87070e6e51d557c1b10ef32ac0ed670856d5aaf1) )
	ROM_LOAD( "136057-1104.3m",  0x030000, 0x010000, CRC(4bb72060) SHA1(94cd1a6900f47a5178cec041fa6dc9cfee1f9c3f) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "136057-1105.5m",  0x000000, 0x010000, CRC(50e0823f) SHA1(f638becad83307ed43d138d452199e4c6725512f) )
	ROM_LOAD( "136057-1107.67m", 0x010000, 0x010000, CRC(729de7a9) SHA1(526b08e6d54cd0b991c4207c23119d2940a34009) )
	ROM_LOAD( "136057-1109.8m",  0x020000, 0x010000, CRC(090e42ab) SHA1(903aa99e6e39407319f6e90102b24604884ee047) )
	ROM_LOAD( "136057-1111.10m", 0x030000, 0x010000, CRC(1ff79e67) SHA1(12d408184f814bab411f567e8b29914a289e3fb8) )
	ROM_LOAD( "136057-1113.11m", 0x040000, 0x010000, CRC(4be1d504) SHA1(f41ff2d31e2e0e5b6d89fbbf014ba767c7b9f299) )
	ROM_LOAD( "136057-1115.13m", 0x050000, 0x010000, CRC(e4409310) SHA1(09180f1ab2ac8465b6641e94271c72bf566b2597) )
	ROM_LOAD( "136057-1117.14m", 0x060000, 0x010000, CRC(7aaca15e) SHA1(4014d60f2b6590c96796dbb2a538f1976194f3e7) )
	ROM_LOAD( "136057-1119.16m", 0x070000, 0x010000, CRC(33690379) SHA1(09ddfd18ccab1c639837171a763a981c867af0b1) )
	ROM_LOAD( "136057-1106.5n",  0x080000, 0x010000, CRC(2720ee71) SHA1(ebfd58effebadab361dfb4bd77d626911da4409a) )
	ROM_LOAD( "136057-1108.67n", 0x090000, 0x010000, CRC(2faecd15) SHA1(7fe9535b9bc72fd5527dbd1079f559ac16f2a31e) )
	ROM_LOAD( "136057-1110.8n",  0x0a0000, 0x010000, CRC(a15e79e1) SHA1(3fc8c33f438fd304b566a62bbe0f6e17a696edbc) )
	ROM_LOAD( "136057-1112.10n", 0x0b0000, 0x010000, CRC(4d5fc284) SHA1(c66f95af700828225a62f46437ca83453900f7fc) )
	ROM_LOAD( "136057-1114.11n", 0x0c0000, 0x010000, CRC(a70fc6e6) SHA1(fbf469b8f5c6e69540743748ad994a6490ad7745) )
	ROM_LOAD( "136057-1116.13n", 0x0d0000, 0x010000, CRC(f423b4f8) SHA1(a431686233b104074728a81cf41604deea0fbb56) )
	ROM_LOAD( "136057-1118.14n", 0x0e0000, 0x010000, CRC(56fa3d16) SHA1(9d9c1fb7912774954224d8f0220047324122ab23) )
	ROM_LOAD( "136057-1120.16n", 0x0f0000, 0x010000, CRC(f257f738) SHA1(a5904ec25d2190f11708c2e1e41832fd66332428) )
ROM_END


ROM_START( blstroid3 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136057-3123.6c",  0x000000, 0x010000, CRC(8fb050f5) SHA1(4944ffb0843262afe41fc6b876ab6858dcefc95f) )
	ROM_LOAD16_BYTE( "136057-3121.6b",  0x000001, 0x010000, CRC(21fae262) SHA1(2516a75d76bcfdea5ab41a4898d47ed166bd1996) )
	ROM_LOAD16_BYTE( "136057-3124.4c",  0x020000, 0x010000, CRC(a9140c31) SHA1(02518bf998c0c74dff66f3192dcb1f91b1812cf8) )
	ROM_LOAD16_BYTE( "136057-3122.4b",  0x020001, 0x010000, CRC(137fbb17) SHA1(3dda03ecdb2dc9a9cd78aeaa502497662496a26d) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136057-1135.2k",  0x00000, 0x10000, CRC(baa8b5fe) SHA1(4af1f9bec3ffa856016a89bc20041d572305ba3a) )

	ROM_REGION( 0x040000, "tiles", 0 )
	ROM_LOAD( "136057-1101.1l",  0x000000, 0x010000, CRC(3c2daa5b) SHA1(2710a05e95afd8452104c4f4a9250a3b7d728a42) )
	ROM_LOAD( "136057-1102.1m",  0x010000, 0x010000, CRC(f84f0b97) SHA1(00cb5f1e0f92742683ee71854085b1e4db4bd6bb) )
	ROM_LOAD( "136057-1103.3l",  0x020000, 0x010000, CRC(ae5274f0) SHA1(87070e6e51d557c1b10ef32ac0ed670856d5aaf1) )
	ROM_LOAD( "136057-1104.3m",  0x030000, 0x010000, CRC(4bb72060) SHA1(94cd1a6900f47a5178cec041fa6dc9cfee1f9c3f) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "136057-1105.5m",  0x000000, 0x010000, CRC(50e0823f) SHA1(f638becad83307ed43d138d452199e4c6725512f) )
	ROM_LOAD( "136057-1107.67m", 0x010000, 0x010000, CRC(729de7a9) SHA1(526b08e6d54cd0b991c4207c23119d2940a34009) )
	ROM_LOAD( "136057-1109.8m",  0x020000, 0x010000, CRC(090e42ab) SHA1(903aa99e6e39407319f6e90102b24604884ee047) )
	ROM_LOAD( "136057-1111.10m", 0x030000, 0x010000, CRC(1ff79e67) SHA1(12d408184f814bab411f567e8b29914a289e3fb8) )
	ROM_LOAD( "136057-1113.11m", 0x040000, 0x010000, CRC(4be1d504) SHA1(f41ff2d31e2e0e5b6d89fbbf014ba767c7b9f299) )
	ROM_LOAD( "136057-1115.13m", 0x050000, 0x010000, CRC(e4409310) SHA1(09180f1ab2ac8465b6641e94271c72bf566b2597) )
	ROM_LOAD( "136057-1117.14m", 0x060000, 0x010000, CRC(7aaca15e) SHA1(4014d60f2b6590c96796dbb2a538f1976194f3e7) )
	ROM_LOAD( "136057-1119.16m", 0x070000, 0x010000, CRC(33690379) SHA1(09ddfd18ccab1c639837171a763a981c867af0b1) )
	ROM_LOAD( "136057-1106.5n",  0x080000, 0x010000, CRC(2720ee71) SHA1(ebfd58effebadab361dfb4bd77d626911da4409a) )
	ROM_LOAD( "136057-1108.67n", 0x090000, 0x010000, CRC(2faecd15) SHA1(7fe9535b9bc72fd5527dbd1079f559ac16f2a31e) )
	ROM_LOAD( "136057-1110.8n",  0x0a0000, 0x010000, CRC(a15e79e1) SHA1(3fc8c33f438fd304b566a62bbe0f6e17a696edbc) )
	ROM_LOAD( "136057-1112.10n", 0x0b0000, 0x010000, CRC(4d5fc284) SHA1(c66f95af700828225a62f46437ca83453900f7fc) )
	ROM_LOAD( "136057-1114.11n", 0x0c0000, 0x010000, CRC(a70fc6e6) SHA1(fbf469b8f5c6e69540743748ad994a6490ad7745) )
	ROM_LOAD( "136057-1116.13n", 0x0d0000, 0x010000, CRC(f423b4f8) SHA1(a431686233b104074728a81cf41604deea0fbb56) )
	ROM_LOAD( "136057-1118.14n", 0x0e0000, 0x010000, CRC(56fa3d16) SHA1(9d9c1fb7912774954224d8f0220047324122ab23) )
	ROM_LOAD( "136057-1120.16n", 0x0f0000, 0x010000, CRC(f257f738) SHA1(a5904ec25d2190f11708c2e1e41832fd66332428) )
ROM_END


ROM_START( blstroid2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136057-2123.6c",  0x000000, 0x010000, CRC(5a092513) SHA1(11396125842ea3a43d61b4ce266bb8053fdefd73) )
	ROM_LOAD16_BYTE( "136057-2121.6b",  0x000001, 0x010000, CRC(486aac51) SHA1(5e7fe7eb225d1c2701c21658ba2bad14ef7b64b1) )
	ROM_LOAD16_BYTE( "136057-2124.4c",  0x020000, 0x010000, CRC(d0fa38fe) SHA1(8aeae50dff6bcd14ac5faf10f15724b7f7430f5c) )
	ROM_LOAD16_BYTE( "136057-2122.4b",  0x020001, 0x010000, CRC(744bf921) SHA1(bb9118bfc04745df2eb78e1d1e70f7fc2e0509d4) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136057-1135.2k",  0x00000, 0x10000, CRC(baa8b5fe) SHA1(4af1f9bec3ffa856016a89bc20041d572305ba3a) )

	ROM_REGION( 0x040000, "tiles", 0 )
	ROM_LOAD( "136057-1101.1l",  0x000000, 0x010000, CRC(3c2daa5b) SHA1(2710a05e95afd8452104c4f4a9250a3b7d728a42) )
	ROM_LOAD( "136057-1102.1m",  0x010000, 0x010000, CRC(f84f0b97) SHA1(00cb5f1e0f92742683ee71854085b1e4db4bd6bb) )
	ROM_LOAD( "136057-1103.3l",  0x020000, 0x010000, CRC(ae5274f0) SHA1(87070e6e51d557c1b10ef32ac0ed670856d5aaf1) )
	ROM_LOAD( "136057-1104.3m",  0x030000, 0x010000, CRC(4bb72060) SHA1(94cd1a6900f47a5178cec041fa6dc9cfee1f9c3f) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "136057-1105.5m",  0x000000, 0x010000, CRC(50e0823f) SHA1(f638becad83307ed43d138d452199e4c6725512f) )
	ROM_LOAD( "136057-1107.67m", 0x010000, 0x010000, CRC(729de7a9) SHA1(526b08e6d54cd0b991c4207c23119d2940a34009) )
	ROM_LOAD( "136057-1109.8m",  0x020000, 0x010000, CRC(090e42ab) SHA1(903aa99e6e39407319f6e90102b24604884ee047) )
	ROM_LOAD( "136057-1111.10m", 0x030000, 0x010000, CRC(1ff79e67) SHA1(12d408184f814bab411f567e8b29914a289e3fb8) )
	ROM_LOAD( "136057-1113.11m", 0x040000, 0x010000, CRC(4be1d504) SHA1(f41ff2d31e2e0e5b6d89fbbf014ba767c7b9f299) )
	ROM_LOAD( "136057-1115.13m", 0x050000, 0x010000, CRC(e4409310) SHA1(09180f1ab2ac8465b6641e94271c72bf566b2597) )
	ROM_LOAD( "136057-1117.14m", 0x060000, 0x010000, CRC(7aaca15e) SHA1(4014d60f2b6590c96796dbb2a538f1976194f3e7) )
	ROM_LOAD( "136057-1119.16m", 0x070000, 0x010000, CRC(33690379) SHA1(09ddfd18ccab1c639837171a763a981c867af0b1) )
	ROM_LOAD( "136057-1106.5n",  0x080000, 0x010000, CRC(2720ee71) SHA1(ebfd58effebadab361dfb4bd77d626911da4409a) )
	ROM_LOAD( "136057-1108.67n", 0x090000, 0x010000, CRC(2faecd15) SHA1(7fe9535b9bc72fd5527dbd1079f559ac16f2a31e) )
	ROM_LOAD( "136057-1110.8n",  0x0a0000, 0x010000, CRC(a15e79e1) SHA1(3fc8c33f438fd304b566a62bbe0f6e17a696edbc) )
	ROM_LOAD( "136057-1112.10n", 0x0b0000, 0x010000, CRC(4d5fc284) SHA1(c66f95af700828225a62f46437ca83453900f7fc) )
	ROM_LOAD( "136057-1114.11n", 0x0c0000, 0x010000, CRC(a70fc6e6) SHA1(fbf469b8f5c6e69540743748ad994a6490ad7745) )
	ROM_LOAD( "136057-1116.13n", 0x0d0000, 0x010000, CRC(f423b4f8) SHA1(a431686233b104074728a81cf41604deea0fbb56) )
	ROM_LOAD( "136057-1118.14n", 0x0e0000, 0x010000, CRC(56fa3d16) SHA1(9d9c1fb7912774954224d8f0220047324122ab23) )
	ROM_LOAD( "136057-1120.16n", 0x0f0000, 0x010000, CRC(f257f738) SHA1(a5904ec25d2190f11708c2e1e41832fd66332428) )
ROM_END


ROM_START( blstroidg )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "136057-2223.6c",  0x000000, 0x010000, CRC(cc82108b) SHA1(487a80cac2a196e9b17c64c5d0b884d1ed8da401) )
	ROM_LOAD16_BYTE( "136057-2221.6b",  0x000001, 0x010000, CRC(84822e68) SHA1(763edc9b3605e583506ca1d9befab66411fc720a) )
	ROM_LOAD16_BYTE( "136057-2224.4c",  0x020000, 0x010000, CRC(849249d4) SHA1(61d6eaff7df54f0353639e192eb6074a80916e29) )
	ROM_LOAD16_BYTE( "136057-2222.4b",  0x020001, 0x010000, CRC(bdeaba0d) SHA1(f479514b5d9543f9e12aa1ac48e20bf054cb18d0) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136057-1135.2k",  0x00000, 0x10000, CRC(baa8b5fe) SHA1(4af1f9bec3ffa856016a89bc20041d572305ba3a) )

	ROM_REGION( 0x040000, "tiles", 0 )
	ROM_LOAD( "136057-1101.1l",  0x000000, 0x010000, CRC(3c2daa5b) SHA1(2710a05e95afd8452104c4f4a9250a3b7d728a42) )
	ROM_LOAD( "136057-1102.1m",  0x010000, 0x010000, CRC(f84f0b97) SHA1(00cb5f1e0f92742683ee71854085b1e4db4bd6bb) )
	ROM_LOAD( "136057-1103.3l",  0x020000, 0x010000, CRC(ae5274f0) SHA1(87070e6e51d557c1b10ef32ac0ed670856d5aaf1) )
	ROM_LOAD( "136057-1104.3m",  0x030000, 0x010000, CRC(4bb72060) SHA1(94cd1a6900f47a5178cec041fa6dc9cfee1f9c3f) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "136057-1105.5m",  0x000000, 0x010000, CRC(50e0823f) SHA1(f638becad83307ed43d138d452199e4c6725512f) )
	ROM_LOAD( "136057-1107.67m", 0x010000, 0x010000, CRC(729de7a9) SHA1(526b08e6d54cd0b991c4207c23119d2940a34009) )
	ROM_LOAD( "136057-1109.8m",  0x020000, 0x010000, CRC(090e42ab) SHA1(903aa99e6e39407319f6e90102b24604884ee047) )
	ROM_LOAD( "136057-1111.10m", 0x030000, 0x010000, CRC(1ff79e67) SHA1(12d408184f814bab411f567e8b29914a289e3fb8) )
	ROM_LOAD( "136057-1113.11m", 0x040000, 0x010000, CRC(4be1d504) SHA1(f41ff2d31e2e0e5b6d89fbbf014ba767c7b9f299) )
	ROM_LOAD( "136057-1115.13m", 0x050000, 0x010000, CRC(e4409310) SHA1(09180f1ab2ac8465b6641e94271c72bf566b2597) )
	ROM_LOAD( "136057-1117.14m", 0x060000, 0x010000, CRC(7aaca15e) SHA1(4014d60f2b6590c96796dbb2a538f1976194f3e7) )
	ROM_LOAD( "136057-1119.16m", 0x070000, 0x010000, CRC(33690379) SHA1(09ddfd18ccab1c639837171a763a981c867af0b1) )
	ROM_LOAD( "136057-1106.5n",  0x080000, 0x010000, CRC(2720ee71) SHA1(ebfd58effebadab361dfb4bd77d626911da4409a) )
	ROM_LOAD( "136057-1108.67n", 0x090000, 0x010000, CRC(2faecd15) SHA1(7fe9535b9bc72fd5527dbd1079f559ac16f2a31e) )
	ROM_LOAD( "136057-1110.8n",  0x0a0000, 0x010000, CRC(a15e79e1) SHA1(3fc8c33f438fd304b566a62bbe0f6e17a696edbc) )
	ROM_LOAD( "136057-1112.10n", 0x0b0000, 0x010000, CRC(4d5fc284) SHA1(c66f95af700828225a62f46437ca83453900f7fc) )
	ROM_LOAD( "136057-1114.11n", 0x0c0000, 0x010000, CRC(a70fc6e6) SHA1(fbf469b8f5c6e69540743748ad994a6490ad7745) )
	ROM_LOAD( "136057-1116.13n", 0x0d0000, 0x010000, CRC(f423b4f8) SHA1(a431686233b104074728a81cf41604deea0fbb56) )
	ROM_LOAD( "136057-1118.14n", 0x0e0000, 0x010000, CRC(56fa3d16) SHA1(9d9c1fb7912774954224d8f0220047324122ab23) )
	ROM_LOAD( "136057-1120.16n", 0x0f0000, 0x010000, CRC(f257f738) SHA1(a5904ec25d2190f11708c2e1e41832fd66332428) )
ROM_END


ROM_START( blstroidh )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "eheadh0.c6",  0x00000, 0x10000, CRC(061f0898) SHA1(a277399aa8af665b1fb40c2bb4cf5d36d333db8d) )
	ROM_LOAD16_BYTE( "eheadl0.b6",  0x00001, 0x10000, CRC(ae8df7cb) SHA1(9eaf377bbfa09e2d3ae77764dbf09ff79b65b34f) )
	ROM_LOAD16_BYTE( "eheadh1.c5",  0x20000, 0x10000, CRC(0b7a3cb6) SHA1(7dc585ff536055e85b0849aa075f2fdab34a8e1c) )
	ROM_LOAD16_BYTE( "eheadl1.b5",  0x20001, 0x10000, CRC(43971694) SHA1(a39a8da244645bb56081fd71609a33d8b7d78478) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) // 6502 code
	ROM_LOAD( "136057-1135.2k",  0x00000, 0x10000, CRC(baa8b5fe) SHA1(4af1f9bec3ffa856016a89bc20041d572305ba3a) )

	ROM_REGION( 0x040000, "tiles", 0 )
	ROM_LOAD( "136057-1101.1l",  0x000000, 0x010000, CRC(3c2daa5b) SHA1(2710a05e95afd8452104c4f4a9250a3b7d728a42) )
	ROM_LOAD( "136057-1102.1m",  0x010000, 0x010000, CRC(f84f0b97) SHA1(00cb5f1e0f92742683ee71854085b1e4db4bd6bb) )
	ROM_LOAD( "136057-1103.3l",  0x020000, 0x010000, CRC(ae5274f0) SHA1(87070e6e51d557c1b10ef32ac0ed670856d5aaf1) )
	ROM_LOAD( "136057-1104.3m",  0x030000, 0x010000, CRC(4bb72060) SHA1(94cd1a6900f47a5178cec041fa6dc9cfee1f9c3f) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "136057-1105.5m",  0x000000, 0x010000, CRC(50e0823f) SHA1(f638becad83307ed43d138d452199e4c6725512f) )
	ROM_LOAD( "136057-1107.67m", 0x010000, 0x010000, CRC(729de7a9) SHA1(526b08e6d54cd0b991c4207c23119d2940a34009) )
	ROM_LOAD( "136057-1109.8m",  0x020000, 0x010000, CRC(090e42ab) SHA1(903aa99e6e39407319f6e90102b24604884ee047) )
	ROM_LOAD( "136057-1111.10m", 0x030000, 0x010000, CRC(1ff79e67) SHA1(12d408184f814bab411f567e8b29914a289e3fb8) )
	ROM_LOAD( "mol4.m12",        0x040000, 0x010000, CRC(571139ea) SHA1(646ad4d98f2125aa14ff5e39493cbbbd2f7bf3f8) )
	ROM_LOAD( "136057-1115.13m", 0x050000, 0x010000, CRC(e4409310) SHA1(09180f1ab2ac8465b6641e94271c72bf566b2597) )
	ROM_LOAD( "136057-1117.14m", 0x060000, 0x010000, CRC(7aaca15e) SHA1(4014d60f2b6590c96796dbb2a538f1976194f3e7) )
	ROM_LOAD( "mol7.m16",        0x070000, 0x010000, CRC(d27b2d91) SHA1(5268936a99927c5d31a5f23129e2169abe29d23c) )
	ROM_LOAD( "136057-1106.5n",  0x080000, 0x010000, CRC(2720ee71) SHA1(ebfd58effebadab361dfb4bd77d626911da4409a) )
	ROM_LOAD( "136057-1108.67n", 0x090000, 0x010000, CRC(2faecd15) SHA1(7fe9535b9bc72fd5527dbd1079f559ac16f2a31e) )
	ROM_LOAD( "moh2.n8",         0x0a0000, 0x010000, CRC(a15e79e1) SHA1(3fc8c33f438fd304b566a62bbe0f6e17a696edbc) )
	ROM_LOAD( "136057-1112.10n", 0x0b0000, 0x010000, CRC(4d5fc284) SHA1(c66f95af700828225a62f46437ca83453900f7fc) )
	ROM_LOAD( "moh4.n12",        0x0c0000, 0x010000, CRC(1a74e960) SHA1(fb5a631254fd770fa9542ca4419d4d16bae9591b) )
	ROM_LOAD( "136057-1116.13n", 0x0d0000, 0x010000, CRC(f423b4f8) SHA1(a431686233b104074728a81cf41604deea0fbb56) )
	ROM_LOAD( "136057-1118.14n", 0x0e0000, 0x010000, CRC(56fa3d16) SHA1(9d9c1fb7912774954224d8f0220047324122ab23) )
	ROM_LOAD( "moh7.n16",        0x0f0000, 0x010000, CRC(a93cbbe7) SHA1(5583e2421ae25181039c6145319453fb73e7bbf5) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1987, blstroid,  0,        blstroid, blstroid, blstroid_state, empty_init, ROT0, "Atari Games", "Blasteroids (rev 4)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, blstroid3, blstroid, blstroid, blstroid, blstroid_state, empty_init, ROT0, "Atari Games", "Blasteroids (rev 3)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, blstroid2, blstroid, blstroid, blstroid, blstroid_state, empty_init, ROT0, "Atari Games", "Blasteroids (rev 2)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, blstroidg, blstroid, blstroid, blstroid, blstroid_state, empty_init, ROT0, "Atari Games", "Blasteroids (German, rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, blstroidh, blstroid, blstroid, blstroid, blstroid_state, empty_init, ROT0, "Atari Games", "Blasteroids (with heads)",    MACHINE_SUPPORTS_SAVE )
