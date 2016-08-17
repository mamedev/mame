// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/**********************************************************************************

  Super Card - Fun World.

  Encrypted CPU (based on a Z80)
  plus 2x 8255 + YM3014 + YM3812 hardware.


  Driver by Roberto Fresca.


***********************************************************************************

  Games running on this hardware:

  * Super Card (encrypted),    Fun World, 1992.
  * Fruit Star (encrypted),    Fun World, 1992.


***********************************************************************************

  Herdware Notes....

  CPU:     Fun World custom CPU based on Z80 family.
  Denom:   SUPCA.
  Number:  417.
  Type:    E92.
  Date:    20-07-92.

  SND:  1x Yamaha YM3812.
        1x Yamaha Y3014B (DAC).

  CRTC: MC68B45P.
  I/O:  2x M5L8255AP-5

  ROMs:  PRG: 2x 27C512 (IC37, IC51)
         GFX: 2x 27C512 (IC10, IC11)
         BP:  1x N82S147N

  1x Xtal 16 MHz.
  1x 8 DIP switches bank.

  1x 2x8  edge connector.
  1x 2x22 edge connector.


  PCB Layout:

  .-------------------------------------------------------------------------------------------------------------------.
  |                                                                                                                   |
  |   .--------.                    .--------.                                                                        |
  |   |        |   .------------.   |        |          .----.      .----.  .-----------.                             |
  |   |        |   | GD74LS157  |   |        |          | A01|      | A02|  |  LM380N   |                             '---.
  |   |        |   '------------'   |SRM2064C|          '----'      '----'  '-----------'                      .---.   ---|
  |   |        |                    |   15   |         .-------------------------------------.  .--------.     |   |   ---|
  |   |        |   .------------.   |        |         |## ooooooooooooooooooooooooooooooo ##|  |        |     |   |   ---|
  |   |        |   | GD74LS157  |   |        |         |## ooooooooooooooooooooooooooooooo ##|  |        |     |A03|   ---|
  |   |        |   '------------'   |        |         '-------------------------------------'  |        |     |   |   ---|
  |   |MC68B45P|                    |        |     .--------. .--------. .--------. .---------. |        |     |   |   ---|
  |   |        |   .------------.   |        |     |Supca417| |   GS   | |Supca417| |         | | YAMAHA |     |   |   ---|
  |   |        |   | GD74LS157  |   |        |     |        | |        | |        | |Lfnd. Nr.| |        |     '---'   ---|
  |   |        |   '------------'   |        |     |  CE2   | |GM76C88-| |  CE1   | |SUPCA 417| | YM3812 |             ---|
  |   |        |                    |        |     |        | |12D     | |        | |         | |        |     .---.   ---|
  |   |        |   .------------.   |        |     |        | |        | |        | |         | |        |     |   |   ---|
  |   |        |   | GD74LS157  |   '--------'     |        | |        | |        | |Type: E92| |        |     |   |  .---'
  |   |        |   '------------'                  |        | |        | |        | |         | |        |     |A03|  |
  |   |        |                                   |        | |    I20 | |        | |         | '--------'     |   |  |
  |   |        |                            .----. |        | |        | |        | |Datum:   |                |   |  |
  |   |        |     .--------------------. |XTAL| |        | |        | |        | |  20.7.92| .--------.     |   |  |
  |   '--------'     |                    | |    | |        | | KOREA  | |        | |         | |        |     '---'  |
  |                  |     GM76C88-12D    | |16  | |  27C512| |        | |  27C512| |         | |        |            |
  | .-------------.  |                    | |MHz | |    IC51| |        | |    IC37| |         | |        |   .---.    |
  | |   74LS245   |  |                    | '----' '--------' '--------' '--------' |FUNWORLD | |        |   |8  |    |
  | '-------------'  '--------------------'                                         |         | |M5L8255A|   |   |    '---.
  |                                                                                 |         | |  P-5   |   |   |     ---|
  | .-------------.      .------------.   .---.   .--------------. .--------------. |         | |        |   |DIP|     ---|
  | |   74LS374   |      | GD74LS374  |   |74L|   |   GD74LS374  | |   GD74LS374  | '---------' |        |   |   |     ---|
  | '-------------'      '------------'   |S36|   '--------------' '--------------'             |        |   |   |     ---|
  |                                       |8AN|                                                 |        |   |1  |     ---|
  |.--------------------.   .---------.   |   |                                      .---.      |        |   '---'     ---|
  ||Supca 410 / ZG1     |   |74LS194A |   |   |        .---------.      .---------.  |GD7|      |        |             ---|
  ||                    |   '---------'   '---'        |GD74LS393|      |GD74LS393|  |4LS|      |        |             ---|
  ||              27C512|                              '---------'      '---------'  |139|      |        |             ---|
  ||                IC10|   .---------.   .---. .---.                                |   |      |        |             ---|
  |'--------------------'   |74LS194A |   |GD7| |GD7|                                |   |      |        |             ---|
  |                         '---------'   |4LS| |4LS|  .---------.      .---------.  |   |      |        |             ---|
  |                                       |393| |32 |  | GD4066B |      |GD74LS08 |  '---'      '--------'             ---|
  |.--------------------.   .---------.   |   | |   |  '---------'      '---------'                                    ---|
  ||Supca 410 / ZG2     |   |74LS194A |   |   | |   |                                           .--------.             ---|
  ||                    |   '---------'   |   | |   |                                .---.      |        |             ---|
  ||              27C512|                 '---' '---'  .----------.    .---------.   |74L|      |        |             ---|
  ||                IC11|   .---------.                |GD74LS174 |    |GD74LS138|   |S36|      |        |             ---|
  |'--------------------'   |74LS194A |                '----------'    '---------'   |8AN|      |        |             ---|
  |                         '---------'   .---.                                      |   |      |M5L8255A|             ---|
  |                                       |GD7|                                      |   |      |  P-5   |             ---|
  |.--------------------.   .---------.   |4LS|                                      |   |      |        |             ---|
  ||                    |   |74LS194A |   |174|                                      '---'      |        |             ---|
  ||    EMPTY SOCKET    |   '---------'   |   |                                                 |        |             ---|
  ||                    |                 |   |                                                 |        |             ---|
  ||                    |   .---------.   |   |                                    .-------.    |        |             ---|
  |'--------------------'   |74LS194A |   '---'                    .---.           |       |    |        |             ---|
  |                         '---------'                            |A00|           |BATTERY|    |        |             ---|
  |                                       .---. .---.              |   |           |       |    |        |             ---|
  |.----------. .----------.              |   | |GD7|        .---. '---'           | 3.6V  |    |        |             ---|
  || 74LS174  | |  74LS174 |              |TBP| |4LS|        |POT|                 |       |    |        |             ---|
  |'----------' '----------'              |24S| |174|        '---'                 | 50mA  |    |        |             ---|
  |                                       |10N| |   |                              |       |    '--------'            .---'
  |  .----------.     .--------------.    |   | |   |                              |       |                          |
  |  |  74LS02  |     |   N82S147N   |    |   | |   |                              '-------'                          |
  |  '----------'     '--------------'    '---' '---'                                                                 |
  '-------------------------------------------------------------------------------------------------------------------'

  A00 = PHILIPS PCF1251P
  A01 = YAMAHA Y3014B
  A02 = GL358/I27
  A03 = ULN2003A

  Marking on the back of the PCB: "CPU-AF Z80 IB300"


  DIP Switches bank
  .---------------.
  | |#| |#| |#| | |
  |---------------|
  |#| |#| |#| |#|#|
  '---------------'
   1 2 3 4 5 6 7 8


  Fun World common color system circuitry
  ---------------------------------------

  74LS174 - Hex D-type flip-flops with reset; positive-edge trigger.
  N82S147 - 4K-bit TTL Bipolar PROM.
  74LS374 - 3-STATE Octal D-Type transparent latches and edge-triggered flip-flops.

                   N82S147         74LS374       RESNET        PULL-DOWN
   74HC174        .-------.       .-------.
  .-------.   (1)-|01   20|--VCC--|20   02|------[(1K)]---+              .-----.
  |       |   (1)-|02   06|-------|03   05|------[(470)]--+--+-----------| RED |
  |16: VCC|   (1)-|03   07|-------|04   06|------[(220)]--+  |           '-----'
  |       |   (1)-|04   08|-------|07     |                  '--[(100)]--GND
  |     02|-------|05   09|-------|08   09|------[(1K)]---+              .------.
  |     05|-------|16   11|-------|13   12|------[(470)]--+--+-----------| BLUE |
  |     07|-------|17   12|-------|14   15|------[(220)]--+  |           '------'
  |     10|-------|18   13|-------|17     |                  '--[(100)]--GND
  |     12|-------|19   14|-------|18   16|------[(470)]--+              .-------.
  |     13|---+---|15   10|---+---|10   19|------[(220)]--+--+-----------| GREEN |
  |15 08  |   |   |       |   |   |   01  |                  |           '-------'
  '-+--+--'   |   '-------'   |   '----+--'                  '--[(100)]--GND
    |  |      |               |        |
    |  '------+------GND------'        |
    '----------------------------------'

  (1): Connected either to:

  NOTE: The 74LS374 could be replaced by a 74HCT373.

***********************************************************************************/


#define MASTER_CLOCK    XTAL_16MHz

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "video/mc6845.h"
#include "machine/nvram.h"
#include "video/resnet.h"


class supercrd_state : public driver_device
{
public:
	supercrd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(supercrd_videoram_w);
	DECLARE_WRITE8_MEMBER(supercrd_colorram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_PALETTE_INIT(supercrd);
	DECLARE_VIDEO_START(supercrd);
	UINT32 screen_update_supercrd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


/*************************
*     Video Hardware     *
*************************/

PALETTE_INIT_MEMBER(supercrd_state, supercrd)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	static const int resistances_rb[3] = { 1000, 470, 220 };
	static const int resistances_g [2] = { 470, 220 };
	double weights_r[3], weights_b[3], weights_g[2];

	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rb, weights_r,  100,    0,
			3,  resistances_rb, weights_b,  100,    0,
			2,  resistances_g,  weights_g,  100,    0);


	for (i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);
		/* blue component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		b = combine_3_weights(weights_b, bit0, bit1, bit2);
		/* green component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		g = combine_2_weights(weights_g, bit0, bit1);

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}


WRITE8_MEMBER(supercrd_state::supercrd_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(supercrd_state::supercrd_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(supercrd_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    xxxx ----   tiles color.
    ---- xxxx   unused.
*/
	int offs = tile_index;
	int attr = m_videoram[offs] + (m_colorram[offs] << 8);
	int code = attr & 0xfff;
	int color = m_colorram[offs] >> 4;  // 4 bits for color.

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}


VIDEO_START_MEMBER(supercrd_state, supercrd)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supercrd_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 4, 8, 96, 29);
}


UINT32 supercrd_state::screen_update_supercrd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/*****************************
*   Memory map information   *
*****************************/

static ADDRESS_MAP_START( supercrd_map, AS_PROGRAM, 8, supercrd_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM_WRITE(supercrd_videoram_w) AM_SHARE("videoram") // wrong
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(supercrd_colorram_w) AM_SHARE("colorram") // wrong
//  AM_RANGE(0x0000, 0x0000) AM_RAM AM_SHARE("nvram")
//  AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_device, address_w)
//  AM_RANGE(0xe001, 0xe001) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
ADDRESS_MAP_END


/*****************************
*   Input port definitions   *
*****************************/

static INPUT_PORTS_START( supercrd )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	4,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(0,2), RGN_FRAC(0,2) + 4, RGN_FRAC(1,2), RGN_FRAC(1,2) + 4 },
	{ 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2
};


/******************************
* Graphics Decode Information *
******************************/

/* The palette system is adressable through a PLD.
   The game could have 2 different palettes, located
   in the first and second half of the bipolar PROM.
*/

static GFXDECODE_START( supercrd )  /* Adressing the first half of the palette */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END


/**************************
*     Machine Drivers     *
**************************/

static MACHINE_CONFIG_START( supercrd, supercrd_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/8)    /* 2MHz, guess */
	MCFG_CPU_PROGRAM_MAP(supercrd_map)

//  MCFG_NVRAM_ADD_0FILL("nvram")

//  MCFG_DEVICE_ADD("ppi8255_0", I8255, 0)
//  MCFG_DEVICE_ADD("ppi8255_1", I8255, 0)

	/* video hardware */

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE((124+1)*4, (30+1)*8)               /* Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1) */
	MCFG_SCREEN_VISIBLE_AREA(0*4, 96*4-1, 0*8, 29*8-1)  /* Taken from MC6845 init, registers 01 & 06 */
	MCFG_SCREEN_UPDATE_DRIVER(supercrd_state, screen_update_supercrd)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", supercrd)

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_INIT_OWNER(supercrd_state, supercrd)
	MCFG_VIDEO_START_OVERRIDE(supercrd_state, supercrd)

//  MCFG_MC6845_ADD("crtc", MC6845, "screen", MASTER_CLOCK/8)
//  MCFG_MC6845_SHOW_BORDER_AREA(false)
//  MCFG_MC6845_CHAR_WIDTH(4)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( supercrd )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "supca_417_ce1.ic37", 0x00000, 0x10000, CRC(b67f7d38) SHA1(eaf8f24d476185d4744858afcbf0005362f49cab) )    // wrong... 1st and 3rd quarter: program
	ROM_LOAD( "supca_417_ce2.ic51", 0x10000, 0x08000, CRC(36415f73) SHA1(9881b88991f034d79260502289432a7318aa1647) )    // wrong
	ROM_IGNORE(                     0x8000)

	ROM_REGION( 0x20000, "gfxtemp", 0 )
	ROM_LOAD( "supca_410_zg2.ic11", 0x00000, 0x10000, CRC(a4646dc6) SHA1(638ad334bb4f1430381474ddfaa1029cb4d13916) )
	ROM_LOAD( "supca_410_zg1.ic10", 0x10000, 0x10000, CRC(d3d9ae13) SHA1(4825677bbab2b77ce5aa6500c55a61874932b319) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_COPY( "gfxtemp",            0x08000, 0x0000, 0x8000 )   // ok
	ROM_COPY( "gfxtemp",            0x18000, 0x8000, 0x8000 )   // ok

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "82s147_bad",     0x0000, 0x0200, CRC(8a220b4b) SHA1(4ce4f1e13526e0598a87eee731295e925c2e7d3f) )  // dump attempt #1
	ROM_LOAD( "82s147_bad_1",   0x0200, 0x0200, CRC(306be196) SHA1(531d876cd48984b5b167ebafd6e61ea66e3e60b2) )  // dump attempt #2
	ROM_LOAD( "82s147_bad_2",   0x0400, 0x0200, CRC(39cb5cf4) SHA1(2eb5d6de673bafdfcb8811d0355197864c2f7ee8) )  // dump attempt #3
	ROM_LOAD( "tbp24s10n.ic6",  0x0600, 0x0200, CRC(6f9c6934) SHA1(1f424f8cf5755a0e5feb4724f3282308c0774f1f) )  // other bp...
ROM_END

/*

  Fruit Star.
  Fun World.

  Encrypted Fun World CPU
  based on Z80

  Lfnd.Nr. T10S
  Type:    I92
  Datum:   17-02-92

  PCB: "CPU-AF Z80 IB300"

*/
ROM_START( fruitstr )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "fruitstar_t10s-i-1.ic37", 0x0000, 0x8000, CRC(cd458e9f) SHA1(3fdf59360704ae1550c108c59907067fc7c8424c) ) // 1st half: empty; 2nd half: program (1st half)
	ROM_CONTINUE(                        0x0000, 0x8000)
	ROM_LOAD( "fruitstar_t10s-i-2.ic51", 0x8000, 0x8000, CRC(4536976b) SHA1(9a0ef6245e5aedfdb690df4c6d7a32ebf1b22590) ) // 1st half: program (2nd half); 2nd half: empty
	ROM_IGNORE(                                  0x8000)

	ROM_REGION( 0x20000, "gfxtemp", 0 )
	ROM_LOAD( "fruitstar_zg2.ic11", 0x00000, 0x10000, CRC(4feddc60) SHA1(27a724e4d3273800bbf2f23628737a9be29fe5db) )
	ROM_LOAD( "fruitstar_zg1.ic10", 0x10000, 0x10000, CRC(c69deb11) SHA1(00988c81a11ad96e4d789c53cfdced9ba8ee9ce0) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_COPY( "gfxtemp",            0x08000, 0x0000, 0x8000 )   // ok
	ROM_COPY( "gfxtemp",            0x18000, 0x8000, 0x8000 )   // ok

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "n82s147an.ic9",  0x0000, 0x0200, CRC(eed0aa96) SHA1(4e96e3b44430ebede1bf1affc60d43751266743e) )
	ROM_LOAD( "tbp24s10n.ic6",  0x0200, 0x0200, NO_DUMP )  // missing bipolar PROM...
ROM_END


/*    YEAR  NAME      PARENT  MACHINE   INPUT     STATE          INIT    ROT    COMPANY      FULLNAME                 FLAGS   */
GAME( 1992, supercrd, 0,      supercrd, supercrd, driver_device, 0,      ROT0, "Fun World", "Super Card (encrypted)", MACHINE_NOT_WORKING )
GAME( 1992, fruitstr, 0,      supercrd, supercrd, driver_device, 0,      ROT0, "Fun World", "Fruit Star (encrypted)", MACHINE_NOT_WORKING )
