// license:BSD-3-Clause
// copyright-holders: Phil Stroffolino

/******************************************************************

Mr F Lea
Pacific Novelty 1982

4 way joystick and jump button

I/O Board

 8910  D780C-1
 8910  8910
               SW2
               SW1              8259    8255

               6116  x  x  IO_C IO_A IO_D  x

CPU Board

 8255  D780C-1                  x  x  6116 6116

                 x  CPU_B5  x  CPU_B3  x  CPU_B1
                 x  CPU_D5  x  CPU_D3  x  CPU_D1

Video Board

                        82S19 82S19 82S19

                        82S19

        20MHz
                    93425 6116 6116 93425

                                        clr ram (7489x2)
                                        clr ram (7489x2)
                                        clr ram (7489x2)
           93422 93422
   x  x  VD_J11 VD_J10  x  x  VD_J7 VD_J6     VD_K4 VD_K3 VD_K2 VD_K1
   x  x  VD-L11 VD_L10  x  x  VD_L7 VD_L6     VD_L4 VD_L3 VD_L2 VD_L1


Stephh's notes (based on the games Z80 code and some tests) :

  - DSW1 bits 0 and 1 determine the "Bonus life" value (1 OFF - 0 ON) :
      * ......11 : 10000 points
      * ......10 : 20000 points
      * ......01 : 30000 points
      * ......00 : 40000 points
  - DSW1 bit 2 determines the "Bonus life" occurrence (1 OFF - 0 ON) :
      * .....1.. : many extra lives can be awarded every "value" points
      * .....0.. : only one extra life can be awarded at "value" points
  - When DSW1 bit 3 is OFF, you can't get any extra lives (code at 0x3368).
  - I've decided to merge these 4 bits in a single choice for the end-user.
  - Credits are coded on 1 byte (0xc6bd) then are divided by 2 for display
    so it can display 1/2 credits when coinage is set to 2C_1C or 2C_3C.
    Surprisingly, due to code at 0x2f77, credits are limited to 15 when
    coinage is set to 1C_1C or 1C_2C and to 14 1/2 is set to 2C_1C or 2C_3C.
  - Level is stored 1 byte (0xc70c), range 0x00-0x07 (code at 0x071d and 0x072c).
    This means that if you complete level 8, you'll restart from level 1
    with its initial difficulty (based on the Dip Switch settings).

******************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/timer.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_GFXBANK     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_GFXBANK)

#include "logmacro.h"

#define LOGGFXBANK(...)     LOGMASKED(LOG_GFXBANK,     __VA_ARGS__)


namespace {

class mrflea_state : public driver_device
{
public:
	mrflea_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_pic(*this, "pic"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void mrflea(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	// video-related
	uint8_t m_gfx_bank = 0;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<pic8259_device> m_pic;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void data1_w(uint8_t data);
	void gfx_bank_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void spriteram_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(slave_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void master_io_map(address_map &map) ATTR_COLD;
	void master_map(address_map &map) ATTR_COLD;
	void slave_io_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
};



void mrflea_state::gfx_bank_w(uint8_t data)
{
	m_gfx_bank = data;

	if (data & ~0x14)
		LOGGFXBANK("unknown gfx bank: 0x%02x\n", data);
}

void mrflea_state::videoram_w(offs_t offset, uint8_t data)
{
	int const bank = offset / 0x400;

	offset &= 0x3ff;
	m_videoram[offset] = data;
	m_videoram[offset + 0x400] = bank;
	/* the address range that tile data is written to sets one bit of
	  the bank select.  The remaining bits are from a video register. */
}

void mrflea_state::spriteram_w(offs_t offset, uint8_t data)
{
	if (offset & 2)
	{
		// tile_number
		m_spriteram[offset | 1] = offset & 1;
		offset &= ~1;
	}

	m_spriteram[offset] = data;
}

void mrflea_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	const uint8_t *source = m_spriteram;
	const uint8_t *finish = source + 0x100;
	rectangle clip = m_screen->visible_area();

	clip.max_x -= 24;
	clip.min_x += 16;

	while (source < finish)
	{
		int const xpos = source[1] - 3;
		int const ypos = source[0] - 16 + 3;
		int const tile_number = source[2] + source[3] * 0x100;

		gfx->transpen(bitmap, clip,
			tile_number,
			0, // color
			0, 0, // no flip
			xpos, ypos, 0);
		gfx->transpen(bitmap, clip,
			tile_number,
			0, // color
			0, 0, // no flip
			xpos, 256 + ypos, 0);
		source += 4;
	}
}

void mrflea_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint8_t *source = m_videoram;
	gfx_element *gfx = m_gfxdecode->gfx(1);
	int base = 0;

	if (BIT(m_gfx_bank, 2))
		base |= 0x400;

	if (BIT(m_gfx_bank, 4))
		base |= 0x200;

	for (int sy = 0; sy < 256; sy += 8)
	{
		for (int sx = 0; sx < 256; sx += 8)
		{
			int const tile_number = base + source[0] + source[0x400] * 0x100;
			source++;

				gfx->opaque(bitmap, cliprect,
				tile_number,
				0, // color
				0, 0, // no flip
				sx, sy);
		}
	}
}

uint32_t mrflea_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(mrflea_state::slave_interrupt)
{
	int const scanline = param;

	if (scanline == 248)
		m_pic->ir1_w(ASSERT_LINE);
	if (scanline == 0)
		m_pic->ir1_w(CLEAR_LINE);
}

void mrflea_state::data1_w(uint8_t data)
{
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void mrflea_state::master_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xe000, 0xe7ff).ram().w(FUNC(mrflea_state::videoram_w)).share(m_videoram);
	map(0xe800, 0xe83f).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xec00, 0xecff).ram().w(FUNC(mrflea_state::spriteram_w)).share(m_spriteram);
}

void mrflea_state::master_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).nopw(); // watchdog?
	map(0x40, 0x43).rw("mainppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x60, 0x60).w(FUNC(mrflea_state::gfx_bank_w));
}


void mrflea_state::slave_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x2000, 0x3fff).rom();
	map(0x8000, 0x80ff).ram();
	map(0x9000, 0x905a).ram(); // ?
}

void mrflea_state::slave_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).nopw(); // watchdog
	map(0x10, 0x11).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x20, 0x23).rw("subppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x40).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x41, 0x41).w("ay1", FUNC(ay8910_device::address_w));
	map(0x42, 0x42).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x43, 0x43).w("ay2", FUNC(ay8910_device::address_w));
	map(0x44, 0x44).rw("ay3", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x45, 0x45).w("ay3", FUNC(ay8910_device::address_w));
	map(0x46, 0x46).rw("ay4", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x47, 0x47).w("ay4", FUNC(ay8910_device::address_w));
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

// verified from Z80 code
static INPUT_PORTS_START( mrflea )
	// AY1 port 1 -> 0x807d (CPU1) -> 0xcabe (CPU0) with bits in reverse order
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	// AY1 port 0 -> 0x807e (CPU1) -> 0xcabf (CPU0) with bits in reverse order
	PORT_START("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfb, IP_ACTIVE_LOW, IPT_UNUSED )

	// AY2 port 1, cpl'ed -> 0x807f (CPU1) -> 0xcac1 (CPU0)
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x07, DEF_STR( Bonus_Life ) )       // see notes - table of tables at 0x3475 (4 * 1 word, LSB first)
	PORT_DIPSETTING(    0x07, "Every 10k" )
	PORT_DIPSETTING(    0x06, "Every 20k" )
	PORT_DIPSETTING(    0x05, "Every 30k" )
	PORT_DIPSETTING(    0x04, "Every 40k" )
	PORT_DIPSETTING(    0x03, "10k only" )
	PORT_DIPSETTING(    0x02, "20k only" )
	PORT_DIPSETTING(    0x01, "30k only" )
	PORT_DIPSETTING(    0x00, "40K only" )
	PORT_DIPSETTING(    0x0f, DEF_STR( None ) )
	PORT_DIPUNUSED( 0x10, 0x10 )
	PORT_DIPUNUSED( 0x20, 0x20 )
	PORT_DIPUNUSED( 0x40, 0x40 )
	PORT_DIPUNUSED( 0x80, 0x80 )

	// AY2 port 0, cpl'ed -> 0x8080 (CPU1) -> 0xcac0 (CPU0)
	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )          // see notes
	PORT_DIPSETTING( 0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING( 0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING( 0x0c, "3" )
	PORT_DIPSETTING( 0x08, "4" )
	PORT_DIPSETTING( 0x04, "5" )
	PORT_DIPSETTING( 0x00, "7" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )       // see notes
	PORT_DIPSETTING( 0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING( 0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED( 0x40, 0x40 )
	PORT_DIPUNUSED( 0x80, 0x80 )

	PORT_START("UNKNOWN")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout sprite_layout = {
	16,16,
	0x200, // number of sprites
	4,
	{ 0*0x4000*8,1*0x4000*8,2*0x4000*8,3*0x4000*8 },
	{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 },
	{
		0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,
		0x80,0x90,0xa0,0xb0,0xc0,0xd0,0xe0,0xf0
	},
	16*16
};

static GFXDECODE_START( gfx_mrflea )
	GFXDECODE_ENTRY( "sprites", 0, sprite_layout,        0x10, 1 )
	GFXDECODE_ENTRY( "chars",   0, gfx_8x8x4_packed_msb, 0x00, 1 )
GFXDECODE_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mrflea_state::machine_start()
{
	save_item(NAME(m_gfx_bank));
}

void mrflea_state::machine_reset()
{
	m_gfx_bank = 0;
}

void mrflea_state::mrflea(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4'000'000); // 4 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &mrflea_state::master_map);
	m_maincpu->set_addrmap(AS_IO, &mrflea_state::master_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(mrflea_state::irq0_line_hold)); // NMI resets the game

	Z80(config, m_subcpu, 6'000'000); // runs in IM 1, so doesn't use 8259 INTA
	m_subcpu->set_addrmap(AS_PROGRAM, &mrflea_state::slave_map);
	m_subcpu->set_addrmap(AS_IO, &mrflea_state::slave_io_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(mrflea_state::slave_interrupt), "screen", 0, 1);

	config.set_maximum_quantum(attotime::from_hz(6000));

	i8255_device &mainppi(I8255(config, "mainppi", 0));
	mainppi.in_pb_callback().set("subppi", FUNC(i8255_device::pb_r));
	mainppi.out_pc_callback().set("subppi", FUNC(i8255_device::pc4_w)).bit(7); // OBFA -> STBA
	mainppi.out_pc_callback().append("subppi", FUNC(i8255_device::pc2_w)).bit(1); // IBFB -> ACKB

	i8255_device &subppi(I8255(config, "subppi", 0));
	subppi.in_pa_callback().set("mainppi", FUNC(i8255_device::pa_r));
	subppi.out_pc_callback().set("mainppi", FUNC(i8255_device::pc6_w)).bit(5); // IBFA -> ACKA
	subppi.out_pc_callback().append(m_pic, FUNC(pic8259_device::ir0_w)).bit(3); // INTRA
	subppi.out_pc_callback().append("mainppi", FUNC(i8255_device::pc2_w)).bit(1); // OBFB -> STBB

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_subcpu, 0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 31*8-1);
	m_screen->set_screen_update(FUNC(mrflea_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mrflea);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", 2'000'000));
	ay1.port_a_read_callback().set_ioport("IN1");
	ay1.port_b_read_callback().set_ioport("IN0");
	ay1.add_route(ALL_OUTPUTS, "mono", 0.25);

	AY8910(config, "ay2", 2'000'000).add_route(ALL_OUTPUTS, "mono", 0.25); // not used for sound?

	ay8910_device &ay3(AY8910(config, "ay3", 2'000'000));
	ay3.port_a_read_callback().set_ioport("DSW2");
	ay3.port_b_read_callback().set_ioport("DSW1");
	ay3.add_route(ALL_OUTPUTS, "mono", 0.25);

	ay8910_device &ay4(AY8910(config, "ay4", 2'000'000));
	ay4.port_a_read_callback().set_ioport("UNKNOWN");
	ay4.port_b_write_callback().set(FUNC(mrflea_state::data1_w));
	ay4.add_route(ALL_OUTPUTS, "mono", 0.25);
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mrflea )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "cpu_d1", 0x0000, 0x2000, CRC(d286217c) SHA1(d750d64bb70f735a38b737881abb9a5fbde1c98c) )
	ROM_LOAD( "cpu_d3", 0x2000, 0x2000, CRC(95cf94bc) SHA1(dd0a51d79b0b28952e6177f36af93f296b3cd954) )
	ROM_LOAD( "cpu_d5", 0x4000, 0x2000, CRC(466ca77e) SHA1(513f41a888166a057d28bdc572571a713d77ae5f) )
	ROM_LOAD( "cpu_b1", 0x6000, 0x2000, CRC(721477d6) SHA1(a8a491fcd17a392ca40abfef892dfbc236fd6e0c) )
	ROM_LOAD( "cpu_b3", 0x8000, 0x2000, CRC(f55b01e4) SHA1(93689fa02aab9d1f1acd55b305eafe542ee447b8) )
	ROM_LOAD( "cpu_b5", 0xa000, 0x2000, CRC(79f560aa) SHA1(7326693d7369682f5770bf80df0181d603212900) )

	ROM_REGION( 0x10000, "subcpu", 0 ) // Z80 code; IO CPU
	ROM_LOAD( "io_a11", 0x0000, 0x1000, CRC(7a20c3ee) SHA1(8e0d5770881e6d3d1df17a2ede5a8823ca9d78e3) )
	ROM_LOAD( "io_c11", 0x2000, 0x1000, CRC(8d26e0c8) SHA1(e90e37bd64e991dc47ab80394337073c69b450da) )
	ROM_LOAD( "io_d11", 0x3000, 0x1000, CRC(abd9afc0) SHA1(873314164707ee84739ec76c6119a65a17001620) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "vd_l10", 0x0000, 0x2000, CRC(48b2adf9) SHA1(91390cdbd8df610edec87c1681db1576e2f3c58d) )
	ROM_LOAD( "vd_l11", 0x2000, 0x2000, CRC(2ff168c0) SHA1(e24b6a33e9ce50771983db8b8de7e79a1e87929c) )
	ROM_LOAD( "vd_l6",  0x4000, 0x2000, CRC(100158ca) SHA1(83a619e5897a2b379eb7a72fde3e1bc08b7a34c4) )
	ROM_LOAD( "vd_l7",  0x6000, 0x2000, CRC(34501577) SHA1(4b41fbc3d9ebf562aadfb1a96a5b3e177cac34c7) )
	ROM_LOAD( "vd_j10", 0x8000, 0x2000, CRC(3f29b8c3) SHA1(99f306f9c0ec20e690d5a87911cd48ae2b336560) )
	ROM_LOAD( "vd_j11", 0xa000, 0x2000, CRC(39380bea) SHA1(68e4213ef2a1502f74b1dc7af73ef5b355ed5f66) )
	ROM_LOAD( "vd_j6",  0xc000, 0x2000, CRC(2b4b110e) SHA1(37644113b2ce7bd525697ebb2fc8cb295c228a60) )
	ROM_LOAD( "vd_j7",  0xe000, 0x2000, CRC(3a3c8b1e) SHA1(5991d80990212ffe92c546b0e4b4e01c68fdd0cd) )

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "vd_k1",  0x0000, 0x2000, CRC(7540e3a7) SHA1(e292e7ec47eaefee8bec1585ec33ea4e6cb64e81) )
	ROM_LOAD( "vd_k2",  0x2000, 0x2000, CRC(6c688219) SHA1(323640b99d9e39b327f500ff2ae6a7f8d0da3ada) )
	ROM_LOAD( "vd_k3",  0x4000, 0x2000, CRC(15e96f3c) SHA1(e57a219666dd440909d3fb75d9a5708cbb904389) )
	ROM_LOAD( "vd_k4",  0x6000, 0x2000, CRC(fe5100df) SHA1(17833f26527f570a3d7365e977492a81ab4e8669) )
	ROM_LOAD( "vd_l1",  0x8000, 0x2000, CRC(d1e3d056) SHA1(5277fdcea9c00f90396bd3120b3221c52f2e3f98) )
	ROM_LOAD( "vd_l2",  0xa000, 0x2000, CRC(4d7fb925) SHA1(dc5224318451a59b020996a513269698a6d19972) )
	ROM_LOAD( "vd_l3",  0xc000, 0x2000, CRC(6d81588a) SHA1(8dbc53d7034a661f9d9afd99f3a3cb5dff3ff137) )
	ROM_LOAD( "vd_l4",  0xe000, 0x2000, CRC(423735a5) SHA1(4ee93f93cd2b08560e148525e08880d64c64fcd2) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1982, mrflea, 0, mrflea, mrflea, mrflea_state, empty_init, ROT270, "Pacific Novelty", "The Amazing Adventures of Mr. F. Lea" , MACHINE_SUPPORTS_SAVE )
