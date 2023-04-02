// license:BSD-3-Clause
// copyright-holders: Roberto Fresca

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

  Hardware Notes....

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

  TODO:
  - merge with misc/amaticmg.cpp. Hardware is almost identical and software has
    definitely a common origin.
  - supercrd stops with 'ELEKTRONIK DEFEKT 5'. Problem with the decryption or missing
    something in the emulation?
  - fruitstr glitches after boot test and then resets itself. Problem with the decryption
    or missing something in the emulation?

***********************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ymopl.h"
#include "video/mc6845.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_UNKOPCODES     (1U <<  1)

//#define VERBOSE (LOG_GENERAL | LOG_UNKOPCODES)

#include "logmacro.h"

#define LOGUNKOPCODES(...)     LOGMASKED(LOG_UNKOPCODES,     __VA_ARGS__)


namespace {

class supercrd_state : public driver_device
{
public:
	supercrd_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_rombank(*this, "rombank"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void supercrd(machine_config &config);

	void init_fruitstr();
	void init_supercrd();

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_memory_bank m_rombank;
	tilemap_t *m_bg_tilemap = nullptr;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	uint8_t m_decode_table[0x04][0x08][0x08];

	uint8_t decrypted_opcodes_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map);
	void io_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
};


/*************************
*     Video Hardware     *
*************************/

void supercrd_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	static constexpr int resistances_rb[3] = { 1000, 470, 220 };
	static constexpr int resistances_g [2] = { 470, 220 };

	double weights_r[3], weights_b[3], weights_g[2];
	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rb, weights_r,  100,    0,
			3,  resistances_rb, weights_b,  100,    0,
			2,  resistances_g,  weights_g,  100,    0);

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(weights_r, bit0, bit1, bit2);

		// blue component */
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const b = combine_weights(weights_b, bit0, bit1, bit2);

		// green component */
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const g = combine_weights(weights_g, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void supercrd_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void supercrd_state::colorram_w(offs_t offset, uint8_t data)
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
	int const offs = tile_index;
	int const attr = m_videoram[offs] + (m_colorram[offs] << 8);
	int const code = attr & 0xfff;
	int const color = m_colorram[offs] >> 4;  // 4 bits for color.

	tileinfo.set(0, code, color, 0);
}


void supercrd_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supercrd_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 4, 8, 96, 29);
}


uint32_t supercrd_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


uint8_t supercrd_state::decrypted_opcodes_r(offs_t offset)
{
	uint8_t const data = m_maincpu->space(AS_PROGRAM).read_byte(offset);
	uint8_t const row = bitswap<3>(data, 7, 6, 4);
	uint8_t const xor_v = data & 0x07;

	if (((m_decode_table[offset & 0x03][row][xor_v]) == 0x00) && (data != 0xc5) && (data != 0xcd) && (data != 0xe5) && (data != 0xed))
		LOGUNKOPCODES("at %08x check opcode: %02x\n", offset, data);
	return data ^ m_decode_table[offset & 0x03][row][xor_v];
}

void supercrd_state::machine_start()
{
	m_rombank->configure_entries(0, 2, memregion("maincpu")->base() + 0x8000, 0x4000); // TODO: should be more than just 2, at least for supercrd, but for now games don't run enough to reach them
}

/*****************************
*   Memory map information   *
*****************************/

void supercrd_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xafff).ram().w(FUNC(supercrd_state::videoram_w)).share(m_videoram);
	map(0xb000, 0xbfff).ram().w(FUNC(supercrd_state::colorram_w)).share(m_colorram);
	map(0xc000, 0xffff).bankr(m_rombank);
}

void supercrd_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x23).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x41).w("ymsnd", FUNC(ym3812_device::write));
	map(0x60, 0x60).w("crtc", FUNC(mc6845_device::address_w));
	map(0x61, 0x61).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xc0, 0xc0).lw8(NAME([this] (uint8_t data) { m_rombank->set_entry(data); })); // TODO: mask this
}

void supercrd_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(supercrd_state::decrypted_opcodes_r));
}


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

	PORT_START("SW1")
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

/* The palette system is addressable through a PLD.
   The game could have 2 different palettes, located
   in the first and second half of the bipolar PROM.
*/

static GFXDECODE_START( gfx_supercrd )  // Addressing the first half of the palette
	GFXDECODE_ENTRY( "tiles", 0, charlayout, 0, 16 )
GFXDECODE_END


/**************************
*     Machine Drivers     *
**************************/

void supercrd_state::supercrd(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = XTAL(16'000'000);

	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK / 4);    // 4MHz, guess
	m_maincpu->set_addrmap(AS_PROGRAM, &supercrd_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &supercrd_state::io_map);
	m_maincpu->set_addrmap(AS_OPCODES, &supercrd_state::decrypted_opcodes_map);

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set_ioport("IN0");
	ppi0.in_pb_callback().set_ioport("IN1");
	ppi0.in_pc_callback().set_ioport("IN2");

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	//ppi1.out_pa_callback().set(FUNC(supercrd_state::out_a_w));
	ppi1.in_pb_callback().set_ioport("SW1");
	//ppi1.out_pc_callback().set(FUNC(supercrd_state::out_c_w));

	// video hardware

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size((124+1)*4, (30+1)*8);               // Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1)
	screen.set_visarea(0*4, 96*4-1, 0*8, 29*8-1);  // Taken from MC6845 init, registers 01 & 06
	screen.set_screen_update(FUNC(supercrd_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_supercrd);
	PALETTE(config, "palette", FUNC(supercrd_state::palette), 0x200);

	mc6845_device &crtc(MC6845(config, "crtc", MASTER_CLOCK / 8));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(4);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI); // no NMI mask?

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", MASTER_CLOCK / 4).add_route(ALL_OUTPUTS, "mono", 0.5); // Y3014B DAC
}


/*************************
*        Rom Load        *
*************************/

ROM_START( supercrd )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "supca_417_ce1.ic37", 0x00000, 0x08000, CRC(b67f7d38) SHA1(eaf8f24d476185d4744858afcbf0005362f49cab) )
	ROM_CONTINUE(                   0x00000, 0x08000 )
	ROM_LOAD( "supca_417_ce2.ic51", 0x08000, 0x08000, CRC(36415f73) SHA1(9881b88991f034d79260502289432a7318aa1647) )    // wrong
	ROM_IGNORE(                     0x8000)

	ROM_REGION( 0x20000, "gfxtemp", 0 )
	ROM_LOAD( "supca_410_zg2.ic11", 0x00000, 0x10000, CRC(a4646dc6) SHA1(638ad334bb4f1430381474ddfaa1029cb4d13916) )
	ROM_LOAD( "supca_410_zg1.ic10", 0x10000, 0x10000, CRC(d3d9ae13) SHA1(4825677bbab2b77ce5aa6500c55a61874932b319) )

	ROM_REGION( 0x10000, "tiles", 0 )
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
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fruitstar_t10s-i-1.ic37", 0x0000, 0x8000, CRC(cd458e9f) SHA1(3fdf59360704ae1550c108c59907067fc7c8424c) ) // 1st half: empty; 2nd half: program (1st half)
	ROM_CONTINUE(                        0x0000, 0x8000)
	ROM_LOAD( "fruitstar_t10s-i-2.ic51", 0x8000, 0x8000, CRC(4536976b) SHA1(9a0ef6245e5aedfdb690df4c6d7a32ebf1b22590) ) // 1st half: program (2nd half); 2nd half: empty
	ROM_IGNORE(                                  0x8000)

	ROM_REGION( 0x20000, "gfxtemp", 0 )
	ROM_LOAD( "fruitstar_zg2.ic11", 0x00000, 0x10000, CRC(4feddc60) SHA1(27a724e4d3273800bbf2f23628737a9be29fe5db) )
	ROM_LOAD( "fruitstar_zg1.ic10", 0x10000, 0x10000, CRC(c69deb11) SHA1(00988c81a11ad96e4d789c53cfdced9ba8ee9ce0) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_COPY( "gfxtemp",            0x08000, 0x0000, 0x8000 )   // ok
	ROM_COPY( "gfxtemp",            0x18000, 0x8000, 0x8000 )   // ok

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "n82s147an.ic9",  0x0000, 0x0200, CRC(eed0aa96) SHA1(4e96e3b44430ebede1bf1affc60d43751266743e) )
	ROM_LOAD( "tbp24s10n.ic6",  0x0200, 0x0200, NO_DUMP )  // missing bipolar PROM...
ROM_END


/*
Encryption observations:
- only opcodes are encrypted;
- there are 4 XOR tables selected by bits 0 and 1 of the address;
- within a table the XOR is chosen depending on bits 0, 1, 2, 4, 6, 7 of the data. Only bit 3 and 5 aren't considered;
- XOR values only affect bits 0, 1, 4 and 6;
- the games use different XOR tables;
- code is mostly the same for both games up to 0x96e, then they start differing significantly;
- the encryption concept is the same as ladylinrb,c,d,e in igs/goldstar.cpp;
- compare to suprstar in misc/amaticmg.cpp for confirmed correctly decrypted code.
*/

void supercrd_state::init_supercrd() // TODO: check unknown opcodes
{
	uint8_t unkn = 0x00;

	uint8_t xor_table[0x04][0x08][0x08] =
	{
		{
			{ 0x13, 0x11, 0x01, 0x11, 0x02, 0x50, 0x51, 0x43 }, // 0x0x and 0x2x
			{ 0x11, 0x52, 0x50, 0x43, 0x10, 0x43, unkn, 0x02 }, // 0x1x and 0x3x
			{ 0x51, 0x13, 0x02, 0x12, 0x43, 0x00, unkn, 0x51 }, // 0x4x and 0x6x
			{ 0x50, 0x53, 0x13, 0x00, 0x51, 0x12, 0x02, 0x11 }, // 0x5x and 0x7x
			{ 0x12, unkn, 0x40, 0x51, 0x03, 0x50, unkn, 0x12 }, // 0x8x and 0xax
			{ 0x50, 0x01, 0x53, 0x50, 0x43, 0x43, unkn, 0x00 }, // 0x9x and 0xbx
			{ unkn, 0x41, 0x43, 0x52, 0x42, 0x00, unkn, unkn }, // 0xcx and 0xex
			{ 0x43, 0x02, unkn, 0x02, unkn, 0x43, 0x10, 0x43 }  // 0xdx and 0xfx
		},
		{
			{ 0x13, 0x10, 0x02, 0x40, 0x50, 0x43, 0x11, 0x51 }, // 0x0x and 0x2x
			{ 0x43, 0x41, 0x43, 0x53, 0x50, 0x00, 0x51, 0x01 }, // 0x1x and 0x3x
			{ 0x52, 0x51, 0x00, 0x40, 0x40, 0x00, 0x52, 0x40 }, // 0x4x and 0x6x
			{ 0x52, 0x50, 0x13, 0x01, 0x52, 0x02, 0x03, 0x52 }, // 0x5x and 0x7x
			{ 0x43, unkn, 0x50, 0x41, 0x12, 0x51, 0x11, unkn }, // 0x8x and 0xax
			{ 0x43, 0x00, 0x01, 0x42, unkn, unkn, 0x40, unkn }, // 0x9x and 0xbx
			{ unkn, unkn, unkn, 0x52, unkn, 0x00, 0x53, 0x01 }, // 0xcx and 0xex
			{ 0x51, 0x10, 0x12, 0x41, 0x50, 0x00, 0x50, 0x50 }  // 0xdx and 0xfx
		},
		{
			{ 0x43, 0x51, 0x01, 0x13, 0x51, 0x42, unkn, 0x01 }, // 0x0x and 0x2x
			{ 0x50, 0x43, 0x13, 0x11, unkn, 0x42, 0x12, 0x01 }, // 0x1x and 0x3x
			{ 0x53, 0x10, 0x11, 0x52, 0x51, 0x00, 0x10, 0x13 }, // 0x4x and 0x6x
			{ 0x42, 0x13, 0x13, 0x53, 0x40, 0x52, 0x10, 0x52 }, // 0x5x and 0x7x
			{ 0x40, 0x43, 0x51, 0x51, 0x51, 0x01, unkn, 0x00 }, // 0x8x and 0xax
			{ unkn, 0x01, 0x43, 0x02, unkn, 0x53, unkn, unkn }, // 0x9x and 0xbx
			{ 0x01, 0x02, unkn, 0x50, 0x51, 0x00, 0x51, 0x10 }, // 0xcx and 0xex
			{ 0x42, unkn, unkn, unkn, 0x00, 0x41, unkn, 0x01 }  // 0xdx and 0xfx
		},
		{
			{ 0x42, 0x00, 0x43, 0x53, 0x03, 0x53, 0x00, 0x11 }, // 0x0x and 0x2x
			{ 0x13, 0x02, 0x12, 0x11, 0x41, 0x02, 0x50, 0x53 }, // 0x1x and 0x3x
			{ 0x00, 0x12, 0x52, 0x12, 0x03, 0x00, 0x43, 0x43 }, // 0x4x and 0x6x
			{ 0x42, 0x40, 0x11, 0x01, 0x41, 0x02, 0x02, 0x43 }, // 0x5x and 0x7x
			{ unkn, 0x12, 0x50, 0x43, 0x13, unkn, unkn, 0x02 }, // 0x8x and 0xax
			{ 0x51, 0x01, 0x10, 0x02, 0x52, unkn, 0x43, unkn }, // 0x9x and 0xbx
			{ 0x02, 0x02, unkn, 0x10, unkn, 0x00, 0x10, 0x40 }, // 0xcx and 0xex
			{ 0x01, 0x01, 0x53, 0x50, 0x41, unkn, 0x12, 0x03 }  // 0xdx and 0xfx
		}
	};

	for (int j = 0; j < 0x04; j++)
	{
		for (int i = 0; i < 0x100; i++)
		{
			uint8_t const row = bitswap<3>(i, 7, 6, 4);
			uint8_t const xor_v = i & 0x07;
			if ((i & 0x28) == 0)
				LOGUNKOPCODES("table: %01x encop: %02x; decop: %02x\n", j, i, i ^ xor_table[j][row][xor_v]);
		}
	}

	std::copy(&xor_table[0][0][0], &xor_table[0][0][0] + 0x04 * 0x08 * 0x08, &m_decode_table[0][0][0]);
}

void supercrd_state::init_fruitstr() // TODO: check unknown opcodes
{
	uint8_t unkn = 0x00;

	uint8_t xor_table[0x04][0x08][0x08] =
	{
		{
			{ 0x40, 0x52, 0x53, 0x11, 0x40, 0x02, 0x10, 0x40 }, // 0x0x and 0x2x
			{ 0x11, 0x00, 0x02, 0x43, 0x40, 0x10, 0x02, 0x40 }, // 0x1x and 0x3x
			{ 0x40, 0x03, 0x10, 0x02, 0x02, 0x00, 0x40, 0x43 }, // 0x4x and 0x6x
			{ 0x52, 0x12, 0x41, 0x50, 0x02, 0x00, 0x43, 0x40 }, // 0x5x and 0x7x
			{ 0x03, unkn, 0x12, 0x42, 0x51, 0x53, unkn, 0x01 }, // 0x8x and 0xax
			{ 0x43, 0x52, 0x50, 0x01, unkn, unkn, 0x00, unkn }, // 0x9x and 0xbx
			{ 0x11, 0x13, 0x53, 0x50, 0x02, 0x00, 0x41, unkn }, // 0xcx and 0xex
			{ 0x50, 0x50, 0x12, unkn, unkn, 0x41, 0x43, 0x40 }  // 0xdx and 0xfx
		},
		{
			{ 0x42, 0x11, 0x51, 0x51, 0x51, 0x12, 0x10, 0x03 }, // 0x0x and 0x2x
			{ 0x40, 0x12, 0x13, 0x01, 0x42, 0x10, 0x51, 0x03 }, // 0x1x and 0x3x
			{ 0x01, 0x41, 0x11, 0x41, 0x42, 0x00, 0x41, 0x01 }, // 0x4x and 0x6x
			{ unkn, 0x40, 0x41, 0x02, 0x41, 0x11, 0x02, 0x00 }, // 0x5x and 0x7x
			{ 0x12, 0x41, 0x50, 0x42, 0x00, unkn, unkn, 0x03 }, // 0x8x and 0xax
			{ 0x11, 0x40, 0x02, unkn, 0x52, 0x43, 0x00, 0x40 }, // 0x9x and 0xbx
			{ 0x51, 0x52, unkn, unkn, 0x51, 0x00, 0x40, 0x50 }, // 0xcx and 0xex
			{ 0x13, unkn, 0x10, 0x00, 0x40, 0x01, 0x51, 0x02 }  // 0xdx and 0xfx
		},
		{
			{ unkn, 0x12, 0x50, 0x41, 0x53, 0x11, 0x03, 0x51 }, // 0x0x and 0x2x
			{ 0x11, 0x40, 0x10, 0x01, 0x01, 0x11, 0x42, 0x01 }, // 0x1x and 0x3x
			{ 0x00, 0x51, unkn, 0x40, 0x03, 0x00, 0x02, 0x50 }, // 0x4x and 0x6x
			{ 0x03, 0x51, 0x43, 0x03, 0x01, 0x53, 0x10, 0x50 }, // 0x5x and 0x7x
			{ 0x51, 0x40, 0x51, 0x02, 0x02, 0x52, 0x40, 0x13 }, // 0x8x and 0xax
			{ 0xff, unkn, 0x02, 0x41, 0x42, 0x51, unkn, 0x13 }, // 0x9x and 0xbx
			{ 0x51, 0x52, 0x02, 0x00, unkn, 0x00, 0x53, 0x13 }, // 0xcx and 0xex
			{ 0x53, 0x13, 0x50, 0x41, 0x53, 0x42, 0x40, 0x02 }  // 0xdx and 0xfx
		},
		{
			{ 0x41, 0x13, 0x13, 0x13, 0x42, 0x42, 0x10, 0x01 }, // 0x0x and 0x2x
			{ 0x52, 0x12, 0x13, 0x53, 0x41, 0x10, 0x02, 0x41 }, // 0x1x and 0x3x
			{ 0x11, 0x13, 0x11, 0x50, 0x40, 0x00, 0x53, 0x10 }, // 0x4x and 0x6x
			{ 0x52, 0x01, 0x11, 0x53, 0x10, 0x01, 0x41, 0x50 }, // 0x5x and 0x7x
			{ 0x03, 0x01, 0x52, 0x02, 0x42, 0x10, 0x52, unkn }, // 0x8x and 0xax
			{ 0x01, 0x01, 0x52, 0x40, 0x11, 0x01, unkn, unkn }, // 0x9x and 0xbx
			{ 0x53, 0x43, 0x13, 0x51, unkn, 0x00, 0x51, 0x12 }, // 0xcx and 0xex
			{ 0x13, 0x03, 0x10, 0x12, 0x52, 0x03, 0x51, unkn }  // 0xdx and 0xfx
		}
	};

	for (int j = 0; j < 0x04; j++)
	{
		for (int i = 0; i < 0x100; i++)
		{
			uint8_t const row = bitswap<3>(i, 7, 6, 4);
			uint8_t const xor_v = i & 0x07;
			if ((i & 0x28) == 0)
				LOGUNKOPCODES("table: %01x encop: %02x; decop: %02x\n", j, i, i ^ xor_table[j][row][xor_v]);
		}
	}

	std::copy(&xor_table[0][0][0], &xor_table[0][0][0] + 0x04 * 0x08 * 0x08, &m_decode_table[0][0][0]);
}

} // anonymous namespace


//    YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT           ROT   COMPANY      FULLNAME                  FLAGS
GAME( 1992, supercrd, 0,      supercrd, supercrd, supercrd_state, init_supercrd, ROT0, "Fun World", "Super Card (encrypted)", MACHINE_IS_SKELETON )
GAME( 1992, fruitstr, 0,      supercrd, supercrd, supercrd_state, init_fruitstr, ROT0, "Fun World", "Fruit Star (encrypted)", MACHINE_IS_SKELETON )
