// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Mirko Buffoni
/********************************************************************

  Super Motor (prototype)
  19??, Duintronic.

  Driver by Roberto Fresca and Mirko Buffoni.


*********************************************************************

  Seems to be a prototype, since there are some issues.
  You can choose motorcycle <-> car game through a DIP switch.

  Also there are four difficult modes:

  - Easy: You only have slow trucks in the game, and gas cans on
          the road that extend 10 seconds the game time.

  - Normal: You have trucks and cars on the street. Also gas cans.

  - Medium: You have trucks and cars on the street, but no gas cans.
          Trucks and cars can stop.

  - Hard: You have trucks and cars on the street, but no gas cans.
          Trucks and cars can stop. Seems a bit harder.

  Maybe the easy option was meant for kids cabs.


  Duintronic, the manufacturer, is a Spanish company from Barcelona,
  created by someone from Ampetronic, and finally acquired by one of
  the Tecfri CEOs. The only arcade game known released by them was
  Buccaneers, running on IREM M75 Vigilante hardware.


*********************************************************************

  Hardware specs...

  It's a dedicated PCB that has etched the Duintronic logo.


  1x Zilog Z0840006PSC (Z80) @ 5MHz.  (stickered: S.P.V. -BR  BRI  NC)
  1x Yamaha YM2413 @ 2.5 MHz.

  1x NEC D446C-2, 2K x 8 SRAM.        (work RAM)
  1x NEC D4016C–2 SRAM 32K x 8 SRAM.  (VRAM)

  4x 27C512 ROM.
  3x N82S129N bipolar PROMs.

  1x LM324N low-power quad op amp.  (audio amp)

  3x GAL16V8-20.  (marked 1, 2, 3)

  1x 5.000 MHz. crystal.
  1x 20.00 MHz. crystal.

  2x 8 DIP switches banks.

  1x (2x28) JAMMA edge connector.


  PCB layout...

  .---------------------------------------------------------------------------------------------------------------------------.
  |         1                  2                     3               4                    5               6                   |
  |                  .--------------------.                                          .---------------------------.            |
  |   .------------. | AM27C512-255DC     | .---------------.  .------------.        | Zilog Z0840006PSC (Z80)   |            |
  | A |  74LS377N  | |                    | | 3  GAL16V8-20 |  |  74LS245N  |        |                           |            |
  |   '------------' |       - 1 -        | '---------------'  '------------'        |    S.P.V. -BR  BRI  NC    |            |
  |                  '--------------------'                                          '---------------------------'            |
  |   .------------.                           .------------.  .------------.         .-----------. .---------.          .----'
  | B |  74LS377N  | .----------------.        |  74LS273N  |  |  74HC245P  |         | 74LS367AN | | 74LS04N |          |
  |   '------------' | NEC            |        '------------'  '------------'         '-----------' '---------'          |
  |                  |    D4016C-2    |                                                                                  |
  |   .------------. |                |          .----------.  .--------------------.                  .-------.         |
  | C |  74LS377N  | '----------------'          | 74LS157N |  | TMS 27C512JL       |  .-----------.   | XTAL  |         |
  |   '------------'                             '----------'  |                    |  |  74HC04N  |   | 5 MHZ.|         '----.
  |                                                            |       - 2 -        |  '-----------'   '-------'          ----|
  |   .------------. .----------. .----------.   .----------.  '--------------------'                                     ----|
  | D | 74HC245N   | | 74LS04N  | | 74LS00N  |   | 74LS157N |                        .-----------.  .------------.        ----|
  |   '------------' '----------' '----------'   '----------'  .----------------.    | 74LS74AN  |  |  74LS244N  |        ----|
  |                                                            | NEC            |    '-----------'  '------------'        ----|
  |   .------------. .-----------.               .----------.  |    D446C-2     |                                         ----|
  | E | 74LS273N   | | 74LS283B1 |               | 74LS157N |  |                |   .------------.  .------------.        ----|
  |   '------------' '-----------'               '----------'  '----------------'   | DSW #1     |  |  74LS244N  |        ----|
  |                                                                                 |  ||||||||  |  '------------'        ----|
  |   .------------. .-----------. .----------.  .----------.                       '------------'                      J ----|
  | F | 74LS273N   | | 74LS283B1 | | 74LS139N |  | 74LS157N |                       .------------.                        ----|
  |   '------------' '-----------' '----------'  '----------'  .------------.       | DSW #2     |  .------------.      A ----|
  |                                                            |  74LS245N  |       |  ||||||||  |  |  74LS244N  |        ----|
  |   .-----------.  .----------.      .--------------------.  '------------'       '------------'  '------------'      M ----|
  | G | 74LS283B1 |  | 74LS157N |      | AM27C512-200DC     |                                                             ----|
  |   '-----------'  '----------'      |                    |  .------------.       .------------.           ++++       M ----|
  |                                    |       - 3 -        |  |  74LS244N  |       |  74LS273N  |   RESNET  ||||         ----|
  |   .-----------.  .----------.      '--------------------'  '------------'       '------------'           ++++       A ----|
  | H | 74LS283B1 |  | 74HC273N |                                                                                         ----|
  |   '-----------'  '----------'              .------------.  .------------.       .------------.                        ----|
  |                           .------------.   | 74LS273N   |  |  74LS244N  |       |  74HC273N  |                        ----|
  | I .--------------------.  | 74LS377N   |   '------------'  '------------'       '------------'                        ----|
  |   | TMS 27C512-20JL    |  '------------'                                                                              ----|
  |   |                    |  .------------.   .------------.  .------------.       .------------.  .------------.        ----|
  | J |       - 4 -        |  | 74HC273N   |   | 74LS273N   |  | 1 N82S129N |       |   YM2413   |  |  75LS38    |        ----|
  |   '--------------------'  '------------'   '------------'  '------------'       '------------'  '------------'        ----|
  |                                                                                                                       ----|
  |   .-----------.  .-----------.          .---------------.  .------------.       ++++++++++++++                       .----'
  | K | 74LS163AN |  | 74LS163AN |   LOGO   | 2  GAL16V8-20 |  | 2 N82S129N |       ||| RESNET |||                       |
  |   '-----------'  '-----------'          '---------------'  '------------'       ++++++++++++++                       |
  |                                                                                                                      |
  |   .-----------.  .---------------.         .------------.  .------------.        .----------.                        |
  | L |  74LS08N  |  | 3  GAL16V8-20 |         | 74LS163AN  |  | 3 N82S129N |        |  LM324N  |                        '----.
  |   '-----------'  '---------------'         '------------'  '------------'        '----------'                             |
  |                                   .----.                                                                                  |
  |   .----------.   .-----------.    |XTAL|   .------------.  .------------.                               ---               |
  | M | 74LS74AN |   |  74LS04N  |    | 20 |   | 74LS163AN  |  |  74S112AN  |                             / POT \             |
  |   '----------'   '-----------'    |MHZ.|   '------------'  '------------'                             \     /             |
  |                                   '----'                                                                ---               |
  '---------------------------------------------------------------------------------------------------------------------------'


*********************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/resnet.h"
#include "sound/ymopl.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

#define FIRST_CLOCK     XTAL(5'000'000)
#define SECOND_CLOCK    XTAL(20'000'000)
#define CPU_CLOCK       (FIRST_CLOCK)       // verified 5 MHz.
#define SND_CLOCK       (SECOND_CLOCK / 8)  // verified 2.5 MHz.


class smotor_state : public driver_device
{
public:
	smotor_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ym2413(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoreg(*this, "videoreg"),
		m_videoram(*this, "videoram")
	{ }

	void smotor(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ym2413_device> m_ym2413;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoreg;
	required_shared_ptr<uint8_t> m_videoram;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	void videoram_w(offs_t offset, uint8_t data);
	void cpu_io_videoreg_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void smotor_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void smotor_cpu_map(address_map &map) ATTR_COLD;
	void smotor_cpu_io(address_map &map) ATTR_COLD;
};


/*********************************************
*               Video Hardware               *
*********************************************/

void smotor_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset/2);
}


TILE_GET_INFO_MEMBER(smotor_state::get_fg_tile_info)
{
	tile_index *= 2;

	int attr = m_videoram[tile_index + 1];
	int code = m_videoram[tile_index] | ((attr & 0x0f) << 8);
	int color = (attr) >> 4;
	tileinfo.set(0, code, color, 0);
}


TILE_GET_INFO_MEMBER(smotor_state::get_bg_tile_info)
{
	uint8_t *tilerom = memregion("bgmap")->base();

	tile_index *= 2;

	int data = tilerom[tile_index];
	int attr = tilerom[tile_index + 1];
	int code = (data | ((attr & 0x07) << 8)) + 0x800;
	int color = attr >> 4;

	tileinfo.set(0, code, color, 0);
}

void smotor_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(smotor_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(smotor_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 1024);
	m_bg_tilemap->set_scroll_rows(32);
	m_fg_tilemap->set_transparent_pen(0);
}

uint32_t smotor_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_videoreg[0],0)) {
		m_bg_tilemap->set_scrolly((m_videoreg[3] + (m_videoreg[5] << 8)));
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	} else {
		bitmap.fill(0, cliprect);
	}

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/***************************************************************************

  Convert the color PROMs into a more useable format.

  There are three 256x4 palette PROMs (one per gun).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 680 ohm resistor  -- RED/GREEN/BLUE
        -- 1.0kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void smotor_state::smotor_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	static constexpr int resistances[4] = { 2200, 1000, 680, 220 };

	// compute the color output resistor weights
	double rweights[4], gweights[4], bweights[4];
	compute_resistor_weights(0, 255, -1.0,
			4,  &resistances[0], rweights, 330, 0,
			4,  &resistances[0], gweights, 330, 0,
			4,  &resistances[0], bweights, 330, 0);

	// initialize the palette with these colors
	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		bit3 = BIT(color_prom[i], 3);
		int const r = combine_weights(rweights, bit0, bit1, bit2, bit3);

		// green component
		bit0 = BIT(color_prom[i+0x100], 0);
		bit1 = BIT(color_prom[i+0x100], 1);
		bit2 = BIT(color_prom[i+0x100], 2);
		bit3 = BIT(color_prom[i+0x100], 3);
		int const g = combine_weights(gweights, bit0, bit1, bit2, bit3);

		// blue component
		bit0 = BIT(color_prom[i+0x200], 0);
		bit1 = BIT(color_prom[i+0x200], 1);
		bit2 = BIT(color_prom[i+0x200], 2);
		bit3 = BIT(color_prom[i+0x200], 3);
		int const b = combine_weights(bweights, bit0, bit1, bit2, bit3);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/*********************************************
*           Memory Map Information           *
*********************************************/

void smotor_state::smotor_cpu_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xf000, 0xf7ff).ram().w(FUNC(smotor_state::videoram_w)).share("videoram");
	map(0xf800, 0xffff).ram();
}

void smotor_state::smotor_cpu_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("DSW1");

	map(0x00, 0x01).w("ymsnd", FUNC(ym2413_device::write));

	map(0x02, 0x02).portr("DSW2");

	map(0x02, 0x07).w(FUNC(smotor_state::cpu_io_videoreg_w)).share("videoreg");

/*
  00:  RW  ; port_r / ym2413_w #1
  01:  RW  ; dsw1_r / ym2413_w #2
  02:  RW  ; dsw2_r / bg_enable_w, coin counters  (values written: 0x30 [boot], 0x20 [attract], 0x11 [ingame])
  04:   W  ;        / ???         (values written: 0x00)
  05:   W  ;        / LSB Scrolly
  06:   W  ;        / BG offset reset  (values written: 0xff)
  07:   W  ;        / MSB Scrolly
*/

}

void smotor_state::cpu_io_videoreg_w(offs_t offset, uint8_t data)
{
	switch (offset) {

		case 0x00:
			m_videoreg[offset] = data;
			m_ym2413->set_output_gain(ALL_OUTPUTS, BIT(data, 4) ? 1.0f : 0.0f);
			if ((BIT(data, 0)) & (BIT(data, 2)))    // coin lock + coin a, used for init.
				break;
			machine().bookkeeping().coin_lockout_global_w(BIT(data, 0));  // coin lock.
			machine().bookkeeping().coin_counter_w(1, BIT(data, 1));  // coin b counter.
			machine().bookkeeping().coin_counter_w(0, BIT(data, 2));  // coin a counter.
			break;

		case 0x02:
		case 0x03:
			m_videoreg[3] = data;
			break;
		case 0x04:
		case 0x05:
			m_videoreg[5] = data;
			break;
		default:
			logerror("OUT[%02x] = %02X\n", offset + 2, data);
	}
}


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START( smotor )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_5C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_2C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_5C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_2C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Difficult ) )    PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "Easy (with gas cans, only trucks)" )
	PORT_DIPSETTING(    0x04, "Normal (with gas cans, trucks and cars)" )
	PORT_DIPSETTING(    0x02, "Medium (no gas cans, both trucks and cars)" )
	PORT_DIPSETTING(    0x00, "Hard (no gas cans, both trucks and cars)" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "1:00" )
	PORT_DIPSETTING(    0x10, "1:20" )
	PORT_DIPSETTING(    0x08, "1:40" )
	PORT_DIPSETTING(    0x00, "2:00" )
	PORT_DIPNAME( 0x20, 0x20, "Cycle / Car" )           PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Cycle" )
	PORT_DIPSETTING(    0x00, "Car" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*********************************************
*              Graphics Layouts              *
*********************************************/

static const gfx_layout tiles8x8_layout =
{
	8, 8,
	0x1000,
	4,
	{ 0, 2, 4, 6 },
	{ 1, 0, 9, 8, 17, 16, 25, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32 * 8
};


/**************************************************
*           Graphics Decode Information           *
**************************************************/

static GFXDECODE_START( gfx_smotor )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END



/*********************************************
*           Machine Start & Reset            *
*********************************************/

void smotor_state::machine_start()
{
}


/*********************************************
*              Machine Config                *
*********************************************/

void smotor_state::smotor(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &smotor_state::smotor_cpu_map);
	m_maincpu->set_addrmap(AS_IO, &smotor_state::smotor_cpu_io);
	m_maincpu->set_vblank_int("screen", FUNC(smotor_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(smotor_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_smotor);
	PALETTE(config, m_palette, FUNC(smotor_state::smotor_palette), 256);  // 256 static colors

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2413(config, "ymsnd", SND_CLOCK).add_route(ALL_OUTPUTS, "speaker", 2.0);
}


/*********************************************
*                  ROM Load                  *
*********************************************/

ROM_START( smotor )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2_27c512.d4", 0x00000, 0x10000, CRC(82ca877d) SHA1(fc11c793c25865ebd3d5199c61ce14c535e0d2a8) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "1_27c512.a2", 0x00000, 0x10000, CRC(f12cb8bb) SHA1(0aaa7040b3bdba01e823212550cc0f990eee02e8) )
	ROM_LOAD( "4_27c512.i1", 0x10000, 0x10000, CRC(4c82a10b) SHA1(215354f37f6c13378aca85c96c209610f774711a) )

	ROM_REGION( 0x10000, "bgmap", 0 )
	ROM_LOAD( "3_27c512.g3", 0x00000, 0x10000, CRC(0f67d74b) SHA1(90bb8884f95c883fa3d95e97fba2c0420668dab3) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "82s129.l4", 0x000, 0x100, CRC(cd06515a) SHA1(589961f0438b889d20223d8139992dc8bf35b935) )  // R
	ROM_LOAD( "82s129.j4", 0x100, 0x100, CRC(26472e34) SHA1(c08f61911135cf1fe847a109f25de7f32646f5ec) )  // G
	ROM_LOAD( "82s129.k4", 0x200, 0x100, CRC(b4cffc3b) SHA1(1536ea938f1dc8e506ad98ae6810279e83fe9192) )  // B

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "1_gal16v8.a3", 0x0000, 0x0117, NO_DUMP )  // device is dead
	ROM_LOAD( "2_gal16v8.k3", 0x0000, 0x0117, NO_DUMP )  // device is dead
	ROM_LOAD( "3_gal16v8.l2", 0x0000, 0x0117, NO_DUMP )  // device is dead
ROM_END

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME     PARENT  MACHINE  INPUT    CLASS         INIT        ROT    COMPANY       FULLNAME                  FLAGS
GAME( 19??, smotor,  0,      smotor,  smotor,  smotor_state, empty_init, ROT0, "Duintronic", "Super Motor (prototype)", MACHINE_SUPPORTS_SAVE )
