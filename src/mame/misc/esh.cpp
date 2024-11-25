// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/*
Funai / Gakken Esh's Aurunmilla laserdisc hardware
Driver by Andrew Gardner with help from Daphne Source

Notes:
    (dumper's note) Esh's Aurunmilla can be played in an Interstellar cabinet by swapping the
                    main pcb and the laserdisc. Sound & video pcbs are the same. Control panel is different however.
                    Esh's Aurunmilla can be a 2 player game, while Interstellar is a 1 player only game.
    Hold down TEST while resetting the machine - pops up test mode where...
    The DIPs are software-controlled.
    Two joysticks appear in the IO TEST, but the photos of the control panel I've seen show only 1.
    Eshb has some junk in the IO TEST screen.  Maybe a bad dump?

Todo:
    - LD TROUBLE appears at POST. Sync/timing issue?
    - Performance spike after some time of gameplay, CPU comms gets corrupt?
    - How to init NVRAM? Current defaults definitely aren't.
    - Wrong overlay colors;
    - Convert to tilemaps (see next ToDo for feasibility).
    - Rumor has it there's an analog beep hanging off 0xf5?  Implement it and finish off 0xf5 bits.
    - NVRAM range 0xe000-0xe800 might be too large.  It doesn't seem to write past 0xe600...
    - Maybe some of the IPT_UNKNOWNs do something?
    - Hook up LED's to the MAME lamp system.
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ldv1000.h"
#include "machine/nvram.h"
#include "sound/beep.h"
#include "emupal.h"
#include "speaker.h"


namespace {

class esh_state : public driver_device
{
public:
	esh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_laserdisc(*this, "laserdisc")
		, m_screen(*this, "screen")
		, m_tile_ram(*this, "tile_ram")
		, m_tile_control_ram(*this, "tile_ctrl_ram")
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_beep(*this, "beeper")
		, m_palette(*this, "palette")
	{ }

	void esh(machine_config &config);

	void init_esh();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<pioneer_ldv1000_device> m_laserdisc;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_tile_ram;
	required_shared_ptr<uint8_t> m_tile_control_ram;
	bool m_ld_video_visible = false;
	uint8_t ldp_read();
	void misc_write(uint8_t data);
	void led_writes(offs_t offset, uint8_t data);
	void nmi_line_w(uint8_t data);
	bool m_nmi_enable = false;
	void esh_palette(palette_device &palette) const;
	uint32_t screen_update_esh(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_callback_esh);
	void ld_command_strobe_cb(int state);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<beep_device> m_beep;
	required_device<palette_device> m_palette;

	void z80_0_io(address_map &map) ATTR_COLD;
	void z80_0_mem(address_map &map) ATTR_COLD;
};


/* From daphne */
#define PCB_CLOCK (18432000)


/* VIDEO GOODS */
uint32_t esh_state::screen_update_esh(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const uint8_t pal_bank = m_ld_video_visible == true ? 0x10 : 0x00;
	const uint32_t trans_mask = m_ld_video_visible == true ? 0 : -1;
	gfx_element *gfx;// = m_gfxdecode->gfx(0);

	/* clear */
	bitmap.fill(0, cliprect);


	/* Draw tiles */
	for (int charx = 0; charx < 32; charx++)
	{
		for (int chary = 0; chary < 32; chary++)
		{
			int current_screen_character = (chary*32) + charx;

			int palIndex  = (m_tile_control_ram[current_screen_character] & 0x0f);
			int tileOffs  = (m_tile_control_ram[current_screen_character] & 0x10) >> 4;
			bool blinkLine = bool((m_tile_control_ram[current_screen_character] & 0x40) >> 6);
			bool blinkChar = bool((m_tile_control_ram[current_screen_character] & 0x80) >> 7);


			// TODO: blink timing
			if(blinkChar == true && m_screen->frame_number() & 8)
			{
				if(trans_mask == 0)
					continue;

				for(int yi=0;yi<8;yi++)
					for(int xi=0;xi<8;xi++)
						bitmap.pix(yi+chary*8, xi+charx*8) = m_palette->pen(palIndex * 8 + pal_bank * 0x100);

				continue;
			}

			if(blinkLine == true && m_screen->frame_number() & 8)
				gfx = m_gfxdecode->gfx(1);
			else
				gfx = m_gfxdecode->gfx(0);


			gfx->transpen(bitmap,cliprect,
				m_tile_ram[current_screen_character] + (0x100 * tileOffs),
				palIndex + pal_bank,
				0, 0, charx*8, chary*8, trans_mask);

		}
	}

	/* Draw sprites */
	return 0;
}



/* MEMORY HANDLERS */

uint8_t esh_state::ldp_read()
{
	return m_laserdisc->data_r();
}

void esh_state::misc_write(uint8_t data)
{
	/* Bit 0 unknown */

//  if (data & 0x02)
//      logerror("BEEP!\n");
	m_beep->set_state(BIT(data, 1)); // polarity unknown
	/* Bit 2 unknown */
	m_ld_video_visible = bool(!((data & 0x08) >> 3));

	/* Bits 4-7 unknown */
	/* They cycle through a repeating pattern though */
}

void esh_state::led_writes(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0x00:
		logerror("WRITING 0x%x to P1's START LED\n", data);
		break;
	case 0x01:
		logerror("WRITING 0x%x to P2's START LED\n", data);
		break;
	case 0x02:
		logerror("WRITING 0x%x to P1's BUTTON1 LED\n", data);
		break;
	case 0x03:
		logerror("WRITING 0x%x to P1's BUTTON2 LED\n", data);
		break;
	case 0x04:
		logerror("WRITING 0x%x to P2's BUTTON1 LED\n", data);
		break;
	case 0x05:
		logerror("WRITING 0x%x to P2's BUTTON2 LED\n", data);
		break;
	case 0x06:
		/* Likely coming soon */
		break;
	case 0x07:
		/* Likely coming soon */
		break;
	}
}

void esh_state::nmi_line_w(uint8_t data)
{
	// 0 -> 1 transition enables this, else disabled?
	m_nmi_enable = (data & 1) == 1;
	//if (data == 0x00)
	//  m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	//if (data == 0x01)
	//  m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	if (data & 0xfe)
		logerror("NMI line unknown bit set %02x\n",data);
}


/* PROGRAM MAPS */
void esh_state::z80_0_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xe000, 0xe7ff).ram().share("nvram");
	map(0xf000, 0xf3ff).ram().share("tile_ram");
	map(0xf400, 0xf7ff).ram().share("tile_ctrl_ram");
}


/* IO MAPS */
void esh_state::z80_0_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xf0, 0xf0).portr("IN0");
	map(0xf1, 0xf1).portr("IN1");
	map(0xf2, 0xf2).portr("IN2");
	map(0xf3, 0xf3).portr("IN3");
	map(0xf4, 0xf4).r(FUNC(esh_state::ldp_read)).w(m_laserdisc, FUNC(pioneer_ldv1000_device::data_w));
	map(0xf5, 0xf5).w(FUNC(esh_state::misc_write));    /* Continuously writes repeating patterns */
	map(0xf8, 0xfd).w(FUNC(esh_state::led_writes));
	map(0xfe, 0xfe).w(FUNC(esh_state::nmi_line_w));    /* Both 0xfe and 0xff flip quickly between 0 and 1 */
	map(0xff, 0xff).noprw();                  /*   (they're probably not NMI enables - likely LED's like their neighbors :) */
}                                 /*   (someday 0xf8-0xff will probably be a single handler) */


/* PORTS */
static INPUT_PORTS_START( esh )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void esh_state::esh_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	// Oddly enough, the top 4 bits of each byte is 0 <- ???
	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// Presumably resistor values would help here

		// red component
		bit0 = BIT(color_prom[i+0x100], 0);
		bit1 = BIT(color_prom[i+0x100], 1);
		bit2 = BIT(color_prom[i+0x100], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = 0; //BIT(color_prom[i+0x100], 0);
		bit1 = BIT(color_prom[i+0x100], 3);
		bit2 = BIT(color_prom[i+0x100], 4);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		// TODO: actually opaque flag
		int b;
		if((color_prom[i+0x100] >> 7) & 1)
			b = 0xff;
		else
		{
			bit0 = 0; //BIT(color_prom[i+0x100], 5);
			bit1 = BIT(color_prom[i+0x100], 5);
			bit2 = BIT(color_prom[i+0x100], 6);
			b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		}

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

static GFXDECODE_START( gfx_esh )
	GFXDECODE_ENTRY("gfx1", 0, gfx_8x8x3_planar, 0x0, 0x20)
	GFXDECODE_ENTRY("gfx2", 0, gfx_8x8x3_planar, 0x0, 0x20)
GFXDECODE_END

INTERRUPT_GEN_MEMBER(esh_state::vblank_callback_esh)
{
	// IRQ
	device.execute().set_input_line(0, HOLD_LINE);
}

// TODO: 0xfe NMI enabled after writing to LD command port, NMI reads LD port.
void esh_state::ld_command_strobe_cb(int state)
{
	if(m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

void esh_state::machine_start()
{
}


/* DRIVER */
void esh_state::esh(machine_config &config)
{
	/* main cpu */
	Z80(config, m_maincpu, PCB_CLOCK/6);                       /* The denominator is a Daphne guess based on PacMan's hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &esh_state::z80_0_mem);
	m_maincpu->set_addrmap(AS_IO, &esh_state::z80_0_io);
	m_maincpu->set_vblank_int("screen", FUNC(esh_state::vblank_callback_esh));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	PIONEER_LDV1000(config, m_laserdisc, 0);
	m_laserdisc->command_strobe_callback().set(FUNC(esh_state::ld_command_strobe_cb));
	m_laserdisc->set_overlay(256, 256, FUNC(esh_state::screen_update_esh));
	m_laserdisc->add_route(0, "lspeaker", 1.0);
	m_laserdisc->add_route(1, "rspeaker", 1.0);

	/* video hardware */
	m_laserdisc->add_ntsc_screen(config, "screen");

	PALETTE(config, m_palette, FUNC(esh_state::esh_palette), 256);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_esh);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 2000).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// we just disable even lines so we can simulate line blinking
#define ROM_INTERLACED_GFX \
	ROM_REGION( 0x3000, "gfx2", 0 ) \
	ROM_COPY( "gfx1", 0, 0, 0x3000 ) \
	ROMX_FILL( 0, 0x3000, 0x00, ROM_SKIP(1) )
ROM_START( esh )
	/* Main program CPU */
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "is1.h8", 0x0000, 0x2000, CRC(114c912b) SHA1(7c033a102d046199f3e2c6787579dac5b5295d50) )
	ROM_LOAD( "is2.f8", 0x2000, 0x2000, CRC(0e3b6e62) SHA1(5e8160180e20705e727329f9d70305fcde176a25) )

	/* Tiles */
	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "a.m3", 0x0000, 0x1000, CRC(a04736d8) SHA1(3b642b5d7168cf4a09328eee54c532be815d2bcf) )
	ROM_LOAD( "b.l3", 0x1000, 0x1000, CRC(9366dde7) SHA1(891db65384d47d13355b2eea37f57c34bc775c8f) )
	ROM_LOAD( "c.k3", 0x2000, 0x1000, CRC(a936ef01) SHA1(bcacb281ccb72ceb57fb6a79380cc3a9688743c4) )

	ROM_INTERLACED_GFX

	/* Color (+other) PROMs */
	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "rgb.j1", 0x000, 0x200, CRC(1e9f795f) SHA1(61a58694929fa39b2412bc9244e5681d65a0eacb) )
	ROM_LOAD( "h.c5",   0x200, 0x100, CRC(abde5e4b) SHA1(9dd3a7fd523b519ac613b9f08ae9cc962992cf5d) )    /* Video timing? */
	ROM_LOAD( "v.c6",   0x300, 0x100, CRC(7157ba22) SHA1(07355f30efe46196d216356eda48a59fc622e43f) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "esh_ver2_en", 0, SHA1(c04709d95fd92259f013ec1cd28e3e36a163abe1) )
ROM_END

ROM_START( esha )
	/* Main program CPU */
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "is1.h8", 0x0000, 0x2000, CRC(114c912b) SHA1(7c033a102d046199f3e2c6787579dac5b5295d50) )
	ROM_LOAD( "is2.f8", 0x2000, 0x2000, CRC(7a562f49) SHA1(acfa49b3b3d96b001a5dbdee39cbb0ca80be1763) ) // sldh

	/* Tiles */
	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "a.m3", 0x0000, 0x1000, CRC(a04736d8) SHA1(3b642b5d7168cf4a09328eee54c532be815d2bcf) )
	ROM_LOAD( "b.l3", 0x1000, 0x1000, CRC(9366dde7) SHA1(891db65384d47d13355b2eea37f57c34bc775c8f) )
	ROM_LOAD( "c.k3", 0x2000, 0x1000, CRC(a936ef01) SHA1(bcacb281ccb72ceb57fb6a79380cc3a9688743c4) )

	ROM_INTERLACED_GFX

	/* Color (+other) PROMs */
	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "rgb.j1", 0x000, 0x200, CRC(1e9f795f) SHA1(61a58694929fa39b2412bc9244e5681d65a0eacb) )
	ROM_LOAD( "h.c5",   0x200, 0x100, CRC(abde5e4b) SHA1(9dd3a7fd523b519ac613b9f08ae9cc962992cf5d) )    /* Video timing? */
	ROM_LOAD( "v.c6",   0x300, 0x100, CRC(7157ba22) SHA1(07355f30efe46196d216356eda48a59fc622e43f) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "esh_ver2_en", 0, SHA1(c04709d95fd92259f013ec1cd28e3e36a163abe1) )
ROM_END

ROM_START( eshb )
	/* Main program CPU */
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "1.h8",   0x0000, 0x2000, CRC(8d27d363) SHA1(529d8e4283e736edb5a9193df1ed8d0164471864) )  /* Hand-written ROM label */
	ROM_LOAD( "is2.f8", 0x2000, 0x2000, CRC(0e3b6e62) SHA1(5e8160180e20705e727329f9d70305fcde176a25) )

	/* Tiles */
	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "a.m3", 0x0000, 0x1000, CRC(a04736d8) SHA1(3b642b5d7168cf4a09328eee54c532be815d2bcf) )
	ROM_LOAD( "b.l3", 0x1000, 0x1000, CRC(9366dde7) SHA1(891db65384d47d13355b2eea37f57c34bc775c8f) )
	ROM_LOAD( "c.k3", 0x2000, 0x1000, CRC(a936ef01) SHA1(bcacb281ccb72ceb57fb6a79380cc3a9688743c4) )

	ROM_INTERLACED_GFX

	/* Color (+other) PROMs */
	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "rgb.j1", 0x000, 0x200, CRC(1e9f795f) SHA1(61a58694929fa39b2412bc9244e5681d65a0eacb) )
	ROM_LOAD( "h.c5",   0x200, 0x100, CRC(abde5e4b) SHA1(9dd3a7fd523b519ac613b9f08ae9cc962992cf5d) )    /* Video timing? */
	ROM_LOAD( "v.c6",   0x300, 0x100, CRC(7157ba22) SHA1(07355f30efe46196d216356eda48a59fc622e43f) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "esh_ver2_en", 0, SHA1(c04709d95fd92259f013ec1cd28e3e36a163abe1) )
ROM_END


void esh_state::init_esh()
{
}

} // anonymous namespace


//    YEAR  NAME   PARENT   MACHINE  INPUT  STATE      INIT      MONITOR  COMPANY          FULLNAME                     FLAGS
GAME( 1983, esh,   0,       esh,     esh,   esh_state, init_esh, ROT0,    "Funai/Gakken",  "Esh's Aurunmilla (set 1)",  MACHINE_NOT_WORKING|MACHINE_IMPERFECT_COLORS)
GAME( 1983, esha,  esh,     esh,     esh,   esh_state, init_esh, ROT0,    "Funai/Gakken",  "Esh's Aurunmilla (set 2)",  MACHINE_NOT_WORKING|MACHINE_IMPERFECT_COLORS)
GAME( 1983, eshb,  esh,     esh,     esh,   esh_state, init_esh, ROT0,    "Funai/Gakken",  "Esh's Aurunmilla (set 3)",  MACHINE_NOT_WORKING|MACHINE_IMPERFECT_COLORS)
