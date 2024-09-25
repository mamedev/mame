// license:BSD-3-Clause
// copyright-holders: Roberto Fresca

/**********************************************************************************

  Super Card - Fun World.

  Encrypted CPU (based on a Z80)
  plus 2x 8255 + YM3014 + YM3812 hardware.

  Close to Amatic 8000-1 hardware.


  Driver by Roberto Fresca.


***********************************************************************************

  Games running on this hardware:

  * Super Card (v417, encrypted),     Fun World, 1992.
  * Fruit Star (T10S, encrypted),     Fun World, 1992.
  * Fruit Star (v810, encrypted),     Fun World, 1992.
  * Gamble Poker (v816, encrypted),   Fun World, 1990.
  * Gamble Poker (v812, encrypted),   Fun World, 1990.
  * Super Stars (v839, encrypted),    Fun World, 1996.
  * Super Stars (v834, encrypted),    Fun World, 1990.
  * Red Line (v808, encrypted),       Fun World, 1989.


***********************************************************************************

  To boot into the game...

  1) Let the initial test ends.
  2) Turn ON Service Key (key 9). The screen will show "Elektronik Defekt 5".
  3) Turn ON Personal A key (key 0).
  4) Turn OFF Personal A key.


  For Super Star and Red Line sets...

  1) Let the initial test ends.
  2) Turn ON Service Key (key F2).
  3) Press once Start key (key 1).


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


***********************************************************************************

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
#define LOG_UNKOPCODES     (1U << 1)

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
	void init_gampo();
	void init_supercrd();
	void init_supst();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

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
	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
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

		// blue component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const b = combine_weights(weights_b, bit0, bit1, bit2);

		// green component
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
//  int const color = m_colorram[offs] >> 4;  // 4 bits for color.
	int const color = m_colorram[offs] >> 3;  // 4 bits for color.

	tileinfo.set(0, code, color, 0);
}


void supercrd_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supercrd_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 4, 8, 96, 31);
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
		LOGUNKOPCODES("at %08x check opcode: %02x !\n", offset, data);
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_F) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_U) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_O) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_0) PORT_NAME("Service A (Personal A)") PORT_TOGGLE  // Service/Personal A key - Bookkeeping
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )                 PORT_NAME("Remote Credits")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9) PORT_NAME("Service Key") PORT_TOGGLE  // Sw Elektronik Defekt 5
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Personal A Settings" )
	PORT_DIPSETTING(    0x01, "Brief" )                 // Remote A, Abgeschrieben A.
	PORT_DIPSETTING(    0x00, "Complete" )              // Remote A, Gewechselt A, Abgeschrieben A, Nachgefuellt A.
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x04, "10 credits" )
	PORT_DIPSETTING(    0x00, "5 credits" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, "10 credits" )
	PORT_DIPSETTING(    0x00, "5 credits" )
	PORT_DIPNAME( 0x10, 0x10, "Remote Value" )
	PORT_DIPSETTING(    0x10, "100 Credits / Pulse" )
	PORT_DIPSETTING(    0x00, "50 Credits / Pulse" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )  PORT_CODE(KEYCODE_8) PORT_NAME("Remote Credits (Service)")
INPUT_PORTS_END

static INPUT_PORTS_START( suprstar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_NAME("Coin 1 (Muenze 1)") PORT_IMPULSE(3)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Service B (Dienst B") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )          PORT_NAME("Coin 2 (Muenze 2)")   PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Hopper Payout pulse") PORT_IMPULSE(3)      PORT_CODE(KEYCODE_Q)  // Hopper paying pulse
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // 'Ausgegeben 0 - Hopper Leer' (spent 0 - hopper empty)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 (Halten 3)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 (Halten 2)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Take (Loeschen)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 (Halten 1)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Service A (Dienst A") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_NAME("Bet (Setzen) / Half Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )       PORT_NAME("Service C (Dienst C") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Service (Master)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 (Halten 4)")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1")                   PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x01, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x02, 0x02, "DSW2")                   PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x02, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x04, 0x04, "Coin 1 (Muenzen 1)" )    PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, "Coin 2 (Muenzen 2)" )    PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Jackpot")                PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, "Jackpot KZB")
	PORT_DIPSETTING(    0x00, "Jackpot LZB")
	PORT_DIPNAME( 0x20, 0x20, "Fruechtebonus" )         PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, "Fruechtebonus Bleibt" )
	PORT_DIPSETTING(    0x00, "Fruechtebonus Clear" )
	PORT_DIPNAME( 0x40, 0x40, "DSW7")                   PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x80, 0x80, "BH")                     PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, "Dreifachschluessel")
	PORT_DIPSETTING(    0x00, "Normalgeraet")
INPUT_PORTS_END

static INPUT_PORTS_START( supst834 )
	PORT_INCLUDE( suprstar )
	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x20, 0x20, "DSW2")                   PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
INPUT_PORTS_END

static INPUT_PORTS_START( redline )
	PORT_INCLUDE( suprstar )
	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x04, 0x04, "DSW3")                   PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x08, 0x08, "DSW4")                   PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x10, 0x10, "Remote" )                PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, "100" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x20, 0x20, "DSW6")                   PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
INPUT_PORTS_END

static INPUT_PORTS_START( fruitstr )
	PORT_INCLUDE( supercrd )
	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Hopper")                 PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x01, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x02, 0x02, "Max. Win")               PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x00, "300" )

	PORT_DIPNAME( 0x04, 0x04, "Coin 1 (Muenzen 1)" )    PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x08, 0x08, "Coin 2 (Muenzen 2)" )    PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_DIPNAME( 0x10, 0x10, "Remote")                 PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, "100" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x20, 0x20, "DSW6")                   PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x40, 0x40, "DSW7")                   PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x80, 0x80, "BH")                     PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, "Dreifach")
	PORT_DIPSETTING(    0x00, "Normalg")
INPUT_PORTS_END

static INPUT_PORTS_START( fruitstra )
	PORT_INCLUDE( supercrd )
	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Hopper")                 PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x01, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x02, 0x02, "DSW2")                   PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x02, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )

	PORT_DIPNAME( 0x04, 0x04, "Coin 1 (Muenzen 1)" )    PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, "Coin 2 (Muenzen 2)" )    PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_DIPNAME( 0x10, 0x10, "DSW5")                   PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x20, 0x20, "Wechsler" )              PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, "DSW7")                   PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x80, 0x80, "BH")                     PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, "Dreifach")
	PORT_DIPSETTING(    0x00, "Normal")
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
	GFXDECODE_ENTRY( "tiles", 0, charlayout, 0, 32 )  // formerly 16
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
	screen.set_size((124+1)*4, (30+1)*8);          // Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1)
	screen.set_visarea(0*4, 96*4-1, 0*8, 29*8-1);  // Taken from MC6845 init, registers 01 & 06
	screen.set_screen_update(FUNC(supercrd_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_supercrd);
	PALETTE(config, "palette", FUNC(supercrd_state::palette), 0x200);

	mc6845_device &crtc(MC6845(config, "crtc", MASTER_CLOCK / 8));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(4);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);  // no NMI mask?

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", MASTER_CLOCK / 4).add_route(ALL_OUTPUTS, "mono", 0.5);  // Y3014B DAC
}


/*************************
*        Rom Load        *
*************************/

ROM_START( supercrd )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "supca_417_ce1.ic37", 0x00000, 0x08000, CRC(b67f7d38) SHA1(eaf8f24d476185d4744858afcbf0005362f49cab) )
	ROM_CONTINUE(                   0x00000, 0x08000 )
	ROM_LOAD( "supca_417_ce2.ic51", 0x08000, 0x08000, CRC(36415f73) SHA1(9881b88991f034d79260502289432a7318aa1647) )
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
	ROM_LOAD( "fruitstar_t10s-i-1.ic37", 0x0000, 0x8000, CRC(cd458e9f) SHA1(3fdf59360704ae1550c108c59907067fc7c8424c) )  // 1st half: empty; 2nd half: program (1st half)
	ROM_CONTINUE(                        0x0000, 0x8000)
	ROM_LOAD( "fruitstar_t10s-i-2.ic51", 0x8000, 0x8000, CRC(4536976b) SHA1(9a0ef6245e5aedfdb690df4c6d7a32ebf1b22590) )  // 1st half: program (2nd half); 2nd half: empty
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

  Fruit Star.
  Fun World.
  v810

  Encrypted Fun World CPU
  based on Z80

  Lfnd. Nr. 1010 H
  Type: C
  Datum: März 97

  Also other identical sets have:

  Lfnd. Nr. Fu. St.
  Type: "I"
  Datum: 17.9.96

  Lfnd. Nr. 208
  Type: I92
  Datum: 20.7.92

  Lfnd. Nr. 1208
  Type: I
  Datum:

*/
ROM_START( fruitstra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fstar_810_ci1.ic37", 0x0000, 0x8000, CRC(35f7af22) SHA1(c33eeed954cd3c0b9a775fdc02f816b8f2a9281e) )  // 1st half: empty; 2nd half: program (1st half)
	ROM_CONTINUE(                   0x0000, 0x8000)
	ROM_LOAD( "fstar_810_ci2.ic51", 0x8000, 0x8000, CRC(f931e7cc) SHA1(135d09c4ed0ba9d5fba4c7b5113253188aab7070) )  // 1st half: program (2nd half); 2nd half: empty
	ROM_IGNORE(                             0x8000)

	ROM_REGION( 0x20000, "gfxtemp", 0 )
	ROM_LOAD( "frstar_801_zg2.ic11", 0x00000, 0x10000, CRC(4c1d85a7) SHA1(dfbdf2aaea557a74fdc96976145485a73c956b1a) )
	ROM_LOAD( "frstar_801_zg1.ic10", 0x10000, 0x10000, CRC(81911ead) SHA1(8e67c52ec345aa8472afb189f2243e1377ee5b90) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_COPY( "gfxtemp",            0x08000, 0x0000, 0x8000 )   // ok
	ROM_COPY( "gfxtemp",            0x18000, 0x8000, 0x8000 )   // ok

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147.ic9",  0x0000, 0x0200, CRC(eed0aa96) SHA1(4e96e3b44430ebede1bf1affc60d43751266743e) )
ROM_END


/*
  Super Stars
  v839

  CPU:

  Lfnd. Nr. 839
  Type: "C"
  Datum: 6.9.94

*/
ROM_START( supst839 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "supst_839_cc1.ic37", 0x00000, 0x08000, CRC(48257f23) SHA1(cd46cea9b42ccfceb2841792856bc46363c47295) )
	ROM_CONTINUE(                   0x00000, 0x08000 )
	ROM_LOAD( "supst_839_cc2.ic51", 0x08000, 0x08000, CRC(06ab5bd5) SHA1(1e8a4a5b83cea8516bc981b7a4a6f8cfedaa9d8b) )
	ROM_IGNORE(                     0x8000)

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "zg2.ic11", 0x00000, 0x8000, CRC(5cafeec2) SHA1(1a527d25009e2cd53d30d660539d0e2615f597ed) )
	ROM_LOAD( "zg1.ic10", 0x08000, 0x8000, CRC(884f32dc) SHA1(ce0ab1a7b7d70cd18016118d98afefec6d25b937) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "n82s147.ic9", 0x0000, 0x0200, CRC(dfeabd11) SHA1(21e8bbcf4aba5e4d672e5585890baf8c5bc77c98) )
ROM_END

/*
  Super Stars
  v834

  CPU:

  Lfnd. Nr. 4153
  Type: C90
  Datum: 2.5.90

*/
ROM_START( supst834 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "supst_834_cc1.ic37", 0x00000, 0x08000, CRC(875a09ec) SHA1(b6f5718f3268613f32d41e74d15d17f1dabe8515) )
	ROM_CONTINUE(                   0x00000, 0x08000 )
	ROM_LOAD( "supst_834_cc2.ic51", 0x08000, 0x08000, CRC(4c132d75) SHA1(40dc3bd219714a6b1c7187075b5d17852073051d) )
	ROM_IGNORE(                     0x8000)

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "super_050_zg2.ic11", 0x00000, 0x08000, CRC(5cafeec2) SHA1(1a527d25009e2cd53d30d660539d0e2615f597ed) )
	ROM_LOAD( "super_050_zg1.ic10", 0x08000, 0x08000, CRC(884f32dc) SHA1(ce0ab1a7b7d70cd18016118d98afefec6d25b937) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "fp30a.ic9", 0x0000, 0x0200, CRC(dfeabd11) SHA1(21e8bbcf4aba5e4d672e5585890baf8c5bc77c98) )
ROM_END


/*
  Red Line
  v808

  CPU:

  Lfnd. Nr. 4076
  Type: C89
  Datum: 19.09.89

  Also there are other identical sets labelled:

  Lfnd. Nr. 4011
  Type: C89
  Datum: 22.5.89

  Lfnd. Nr. 4028
  Type: C89
  Datum: 6.6.89

*/
ROM_START( redline )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "redl_808_cc1-g.ic37", 0x00000, 0x08000, CRC(67aeda4f) SHA1(b2c2498833fad45acbfedfd8e9a4bd61904abbf5) )
	ROM_CONTINUE(                    0x00000, 0x08000 )
	ROM_LOAD( "redl_808_cc2-g.ic51", 0x08000, 0x08000, CRC(15940276) SHA1(359cdfb8f9c05ffd5fa1d184ab81d9ecc79c2ea5) )
	ROM_IGNORE(                      0x8000)

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "redl_808_zg2-g.ic11", 0x00000, 0x08000, CRC(56b78d6d) SHA1(5f4613ff9112659daae6b698cf95e0c87293a6a7) )
	ROM_LOAD( "redl_808_zg1-g.ic10", 0x08000, 0x08000, CRC(fa413560) SHA1(16ef0d323c742e5fa78969da6ed25286ab0c6bf3) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "am27s29.ic9", 0x0000, 0x0200, CRC(dfeabd11) SHA1(21e8bbcf4aba5e4d672e5585890baf8c5bc77c98) )
ROM_END

/*
  Gamble Poker
  v816

  CPU:

  Lfnd. Nr. 9018
  Type: F90
  Datum: 22.10.90

  Also another identical set has:

  Lfnd. Nr. 9060
  Type: F90
  Datum: 12.12.90

*/
ROM_START( gampo816 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "gampo_816_cf1.ic37", 0x00000, 0x08000, CRC(f82308d3) SHA1(cb50011033e4797646de04201ca34a0380402c15) )
	ROM_CONTINUE(                   0x00000, 0x08000 )
	ROM_LOAD( "gampo_816_cf2.ic51", 0x08000, 0x08000, CRC(ffa3f6be) SHA1(adda3fc0ca8a490ac68e6c285bfa4d2285fc5d0f) )
	ROM_IGNORE(                     0x8000)

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "gampo_807_zg2.ic11", 0x00000, 0x08000, CRC(dbc83c74) SHA1(1cf27c4946fdde1af5ae0f4f1803957a575461f3) )
	ROM_LOAD( "gampo_807_zg1.ic10", 0x08000, 0x08000, CRC(52cf8211) SHA1(8d871d658a33a98fc6408373dd60db0b7d3ad3b4) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "n82s147.ic9", 0x0000, 0x0200, CRC(a28ccbf1) SHA1(fd46d39bb4031148b7a13481fafc909b76e7bdde) )
ROM_END

/*
  Gamble Poker
  v812

  CPU:

  Lfnd. Nr. 9041
  Type: F90
  Datum: 21.11.90

*/
ROM_START( gampo812 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "gampo_812_cf1.ic37", 0x00000, 0x08000, CRC(bcd2429c) SHA1(876349a83db78fce52e05ec458da7636c12100fb) )
	ROM_CONTINUE(                   0x00000, 0x08000 )
	ROM_LOAD( "gampo_812_cf2.ic51", 0x08000, 0x08000, CRC(1e6f2a30) SHA1(bc2adce26cf926d01acad4e30bd5999568f2384a) )
	ROM_IGNORE(                     0x8000)

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "gampo_807_zg2.ic11", 0x00000, 0x08000, CRC(dbc83c74) SHA1(1cf27c4946fdde1af5ae0f4f1803957a575461f3) )
	ROM_LOAD( "gampo_807_zg1.ic10", 0x08000, 0x08000, CRC(52cf8211) SHA1(8d871d658a33a98fc6408373dd60db0b7d3ad3b4) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "n82s147.ic9", 0x0000, 0x0200, CRC(a28ccbf1) SHA1(fd46d39bb4031148b7a13481fafc909b76e7bdde) )
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
		{ // quadrant 1 should be ok, missing decrypted 0x14, 0x17, 0x82, 0x84, 0x85, 0xd0
			{ 0x13, 0x11, 0x01, 0x11, 0x02, 0x50, 0x51, 0x43 }, // 0x0x and 0x2x
			{ 0x11, 0x52, 0x50, 0x43, 0x10, 0x43, unkn, 0x02 }, // 0x1x and 0x3x
			{ 0x51, 0x13, 0x02, 0x12, 0x43, 0x00, unkn, 0x51 }, // 0x4x and 0x6x
			{ 0x50, 0x53, 0x13, 0x00, 0x51, 0x12, 0x02, 0x11 }, // 0x5x and 0x7x
			{ 0x12, 0x02, 0x40, 0x51, 0x03, 0x50, 0x42, 0x12 }, // 0x8x and 0xax
			{ 0x50, 0x01, 0x53, 0x50, 0x43, 0x43, unkn, 0x00 }, // 0x9x and 0xbx
			{ unkn, 0x41, 0x43, 0x52, 0x42, 0x00, unkn, unkn }, // 0xcx and 0xex
			{ 0x43, 0x02, unkn, 0x02, unkn, 0x43, 0x10, 0x43 }  // 0xdx and 0xfx
		},
		{ // quadrants 1,2 should be ok, missing decrypted 0x80, 0x82, 0x83, 0x85, 0x87, 0xd0, 0xd7
			{ 0x13, 0x10, 0x02, 0x40, 0x50, 0x43, 0x11, 0x51 }, // 0x0x and 0x2x
			{ 0x43, 0x41, 0x43, 0x53, 0x50, 0x00, 0x51, 0x01 }, // 0x1x and 0x3x
			{ 0x52, 0x51, 0x00, 0x40, 0x40, 0x00, 0x52, 0x40 }, // 0x4x and 0x6x
			{ 0x52, 0x50, 0x13, 0x01, 0x52, 0x02, 0x03, 0x52 }, // 0x5x and 0x7x
			{ 0x43, unkn, 0x50, 0x41, 0x12, 0x51, 0x11, unkn }, // 0x8x and 0xax
			{ 0x43, 0x01, 0x01, 0x42, unkn, unkn, 0x40, unkn }, // 0x9x and 0xbx
			{ unkn, unkn, unkn, 0x52, 0x00, 0x00, 0x53, 0x01 }, // 0xcx and 0xex
			{ 0x51, 0x10, 0x12, 0x41, 0x50, 0x00, 0x50, 0x50 }  // 0xdx and 0xfx
		},
		{ // quadrant 1 should be ok, missing decrypted 0x17, 0x44 0x80, 0x81, 0x82, 0x83, 0x85, 0xc7, 0xd0
			{ 0x43, 0x51, 0x01, 0x13, 0x51, 0x42, unkn, 0x01 }, // 0x0x and 0x2x
			{ 0x50, 0x43, 0x13, 0x11, unkn, 0x42, 0x12, 0x01 }, // 0x1x and 0x3x
			{ 0x53, 0x10, 0x11, 0x52, 0x51, 0x00, 0x10, 0x13 }, // 0x4x and 0x6x
			{ 0x42, 0x13, 0x13, 0x53, 0x40, 0x52, 0x10, 0x52 }, // 0x5x and 0x7x
			{ 0x40, 0x43, 0x51, 0x51, 0x51, 0x01, 0x42, 0x00 }, // 0x8x and 0xax
			{ unkn, 0x01, 0x43, 0x02, unkn, 0x53, 0x00, unkn }, // 0x9x and 0xbx
			{ 0x01, 0x02, unkn, 0x50, 0x51, 0x00, 0x51, 0x10 }, // 0xcx and 0xex
			{ 0x42, unkn, unkn, unkn, 0x00, 0x41, unkn, 0x01 }  // 0xdx and 0xfx
		},
		{ // quadrants 1,2,3 should be ok, missing decrypted 0x84, 0x94, 0x96, 0xc7, 0xd7
			{ 0x42, 0x00, 0x43, 0x53, 0x03, 0x53, 0x00, 0x11 }, // 0x0x and 0x2x
			{ 0x13, 0x02, 0x12, 0x11, 0x41, 0x02, 0x50, 0x53 }, // 0x1x and 0x3x
			{ 0x00, 0x12, 0x52, 0x12, 0x03, 0x00, 0x43, 0x43 }, // 0x4x and 0x6x
			{ 0x42, 0x40, 0x11, 0x01, 0x41, 0x02, 0x02, 0x43 }, // 0x5x and 0x7x
			{ 0x00, 0x12, 0x50, 0x43, 0x13, unkn, unkn, 0x02 }, // 0x8x and 0xax
			{ 0x51, 0x01, 0x10, 0x02, 0x52, unkn, 0x43, unkn }, // 0x9x and 0xbx
			{ 0x02, 0x02, 0x50, 0x10, unkn, 0x00, 0x10, 0x40 }, // 0xcx and 0xex
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
		{ // quadrants 1, 2, 3 should be ok, missing decrypted 0x84, 0x85
			{ 0x40, 0x52, 0x53, 0x11, 0x40, 0x02, 0x10, 0x40 }, // 0x0x and 0x2x
			{ 0x11, 0x00, 0x02, 0x43, 0x40, 0x10, 0x02, 0x40 }, // 0x1x and 0x3x
			{ 0x40, 0x03, 0x10, 0x02, 0x02, 0x00, 0x40, 0x43 }, // 0x4x and 0x6x
			{ 0x52, 0x12, 0x41, 0x50, 0x02, 0x00, 0x43, 0x40 }, // 0x5x and 0x7x
			{ 0x03, 0x51, 0x12, 0x42, 0x51, 0x53, 0x51, 0x01 }, // 0x8x and 0xax
			{ 0x43, 0x52, 0x50, 0x01, 0x50, unkn, 0x00, unkn }, // 0x9x and 0xbx
			{ 0x11, 0x13, 0x53, 0x50, 0x02, 0x00, 0x41, unkn }, // 0xcx and 0xex
			{ 0x50, 0x50, 0x12, 0x51, unkn, 0x41, 0x43, 0x40 }  // 0xdx and 0xfx
		},
		{ // quadrants 1, 2 should be ok, missing decrypted 0x80, 0x82, 0x83, 0xc4, 0xc7, 0xd0
			{ 0x42, 0x11, 0x41, 0x51, 0x51, 0x12, 0x10, 0x03 }, // 0x0x and 0x2x
			{ 0x40, 0x12, 0x13, 0x01, 0x42, 0x10, 0x51, 0x03 }, // 0x1x and 0x3x
			{ 0x01, 0x41, 0x11, 0x41, 0x42, 0x00, 0x41, 0x01 }, // 0x4x and 0x6x
			{ 0x10, 0x40, 0x41, 0x02, 0x41, 0x11, 0x02, 0x00 }, // 0x5x and 0x7x
			{ 0x12, 0x41, 0x50, 0x42, 0x01, unkn, unkn, 0x03 }, // 0x8x and 0xax
			{ 0x11, 0x40, 0x02, unkn, 0x52, 0x43, 0x00, 0x40 }, // 0x9x and 0xbx
			{ 0x51, 0x52, unkn, unkn, 0x51, 0x00, 0x40, 0x50 }, // 0xcx and 0xex
			{ 0x13, unkn, 0x10, 0x00, 0x40, 0x01, 0x51, 0x02 }  // 0xdx and 0xfx
		},
		{ // quadrants 1, 2, 3 should be ok, missing decrypted 0x85, 0xc7
			{ 0x43, 0x12, 0x50, 0x41, 0x53, 0x11, 0x03, 0x51 }, // 0x0x and 0x2x
			{ 0x11, 0x40, 0x10, 0x01, 0x01, 0x11, 0x42, 0x01 }, // 0x1x and 0x3x
			{ 0x00, 0x51, 0x03, 0x40, 0x03, 0x00, 0x02, 0x50 }, // 0x4x and 0x6x
			{ 0x03, 0x51, 0x43, 0x03, 0x01, 0x53, 0x10, 0x50 }, // 0x5x and 0x7x
			{ 0x51, 0x40, 0x51, 0x02, 0x02, 0x52, 0x40, 0x13 }, // 0x8x and 0xax
			{ 0x40, 0x11, 0x02, 0x41, 0x42, 0x51, unkn, 0x13 }, // 0x9x and 0xbx
			{ 0x51, 0x52, 0x02, 0x00, unkn, 0x00, 0x53, 0x13 }, // 0xcx and 0xex
			{ 0x53, 0x13, 0x50, 0x41, 0x53, 0x42, 0x40, 0x02 }  // 0xdx and 0xfx
		},
		{ // quadrants 1, 2 should be ok, missing decrypted 0x84, 0xc7
			{ 0x41, 0x13, 0x13, 0x13, 0x42, 0x42, 0x10, 0x01 }, // 0x0x and 0x2x
			{ 0x52, 0x12, 0x13, 0x53, 0x41, 0x10, 0x02, 0x41 }, // 0x1x and 0x3x
			{ 0x11, 0x13, 0x11, 0x50, 0x40, 0x00, 0x53, 0x10 }, // 0x4x and 0x6x
			{ 0x52, 0x01, 0x11, 0x53, 0x10, 0x01, 0x41, 0x50 }, // 0x5x and 0x7x
			{ 0x03, 0x01, 0x52, 0x02, 0x42, 0x10, 0x52, unkn }, // 0x8x and 0xax
			{ 0x01, 0x01, 0x52, 0x40, 0x11, 0x01, unkn, unkn }, // 0x9x and 0xbx
			{ 0x53, 0x43, 0x13, 0x51, unkn, 0x00, 0x51, 0x12 }, // 0xcx and 0xex
			{ 0x13, 0x03, 0x10, 0x12, 0x52, 0x03, 0x51, 0x00 }  // 0xdx and 0xfx
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

void supercrd_state::init_gampo() // TODO: check unknown opcodes
{
	uint8_t unkn = 0x00;

	uint8_t xor_table[0x04][0x08][0x08] =
	{
		{ // quadrants 1, 2 should be ok, missing decrypted 0x82, 0x84, 0xc7, 0xd0
			{ 0x51, 0x12, 0x41, 0x02, 0x02, 0x13, 0x11, 0x43 }, // 0x0x and 0x2x
			{ 0x43, 0x12, 0x53, 0x43, 0x53, 0x00, 0x02, 0x51 }, // 0x1x and 0x3x
			{ 0x51, 0x01, 0x52, 0x43, 0x43, 0x00, 0x42, 0x12 }, // 0x4x and 0x6x
			{ 0x42, 0x53, 0x10, 0x01, 0x00, 0x03, 0x01, 0x52 }, // 0x5x and 0x7x
			{ 0x42, 0x00, 0x50, unkn, 0x13, unkn, 0x01, 0x41 }, // 0x8x and 0xax
			{ 0x50, 0x00, 0x51, 0x01, 0x12, unkn, unkn, 0x42 }, // 0x9x and 0xbx
			{ 0x50, 0x41, 0x13, 0x40, unkn, 0x00, unkn, 0x52 }, // 0xcx and 0xex
			{ 0x11, 0x42, 0x01, unkn, unkn, 0x41, 0x12, 0x01 }  // 0xdx and 0xfx
		},
		{ // quadrants 1, 2 should be ok, missing decrypted 0x80, 0x82, 0x83, 0x94, 0xc7, 0xd0
			{ 0x10, 0x51, 0x50, 0x02, 0x11, 0x02, 0x41, 0x01 }, // 0x0x and 0x2x
			{ 0x50, 0x13, 0x51, 0x01, 0x43, 0x02, 0x02, 0x42 }, // 0x1x and 0x3x
			{ 0x02, 0x41, 0x11, 0x02, 0x40, 0x00, 0x12, 0x51 }, // 0x4x and 0x6x
			{ 0x53, 0x40, 0x41, 0x02, 0x02, 0x13, 0x12, 0x52 }, // 0x5x and 0x7x
			{ 0x12, 0x12, 0x40, 0x50, 0x53, 0x12, 0x40, 0x51 }, // 0x8x and 0xax
			{ unkn, 0x01, unkn, 0x02, 0x01, 0x10, unkn, 0x10 }, // 0x9x and 0xbx
			{ unkn, 0x02, 0x02, 0x02, 0x42, 0x00, 0x50, 0x03 }, // 0xcx and 0xex
			{ 0x01, 0x03, 0x53, unkn, 0x00, 0x51, unkn, 0x02 }  // 0xdx and 0xfx
		},
		{ // quadrant 1 should be ok, missing decrypted 0x17, 0x44, 0x80, 0x96, 0xc7, 0xd0, 0xd7
			{ 0x52, 0x02, 0x00, 0x13, 0x43, 0x01, 0x03, unkn }, // 0x0x and 0x2x
			{ 0x02, 0x00, 0x12, 0x51, 0x01, 0x40, 0x42, 0x03 }, // 0x1x and 0x3x
			{ 0x03, 0x10, 0x43, 0x13, 0x43, 0x00, 0x00, 0x41 }, // 0x4x and 0x6x
			{ 0x03, 0x10, 0x41, 0x13, 0x02, 0x43, 0x01, unkn }, // 0x5x and 0x7x
			{ 0x52, 0x50, unkn, 0x41, unkn, unkn, 0x02, 0x52 }, // 0x8x and 0xax
			{ 0x00, 0x00, unkn, 0x10, 0x01, unkn, 0x01, 0x10 }, // 0x9x and 0xbx
			{ 0x41, 0x12, unkn, 0x50, 0x10, 0x00, 0x02, 0x53 }, // 0xcx and 0xex
			{ 0x10, 0x10, 0x40, 0x10, 0x12, unkn, 0x50, 0x01 }  // 0xdx and 0xfx
		},
		{ // quadrants 1, 2, 3 should be ok, missing decrypted 0x94, 0xc7
			{ 0x02, 0x43, 0x52, 0x51, 0x42, 0x41, 0x50, 0x13 }, // 0x0x and 0x2x
			{ 0x43, 0x11, 0x00, 0x03, 0x10, 0x10, 0x43, 0x40 }, // 0x1x and 0x3x
			{ 0x51, 0x42, 0x13, 0x42, 0x43, 0x00, 0x51, 0x13 }, // 0x4x and 0x6x
			{ 0x13, 0x11, 0x41, 0x12, 0x41, 0x53, 0x11, 0x41 }, // 0x5x and 0x7x
			{ 0x12, 0x42, 0x51, 0x43, 0x03, 0x03, 0x51, 0x43 }, // 0x8x and 0xax
			{ 0x52, 0x02, 0x12, 0x03, 0x11, 0x03, unkn, 0x00 }, // 0x9x and 0xbx
			{ 0x41, 0x43, 0x10, 0x02, 0x51, 0x00, 0x13, 0x13 }, // 0xcx and 0xex
			{ 0x00, 0x52, 0x03, 0x42, 0x02, 0x13, 0x52, unkn }  // 0xdx and 0xfx
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

void supercrd_state::init_supst() // TODO: check unknown opcodes
{
	uint8_t unkn = 0x00;

	uint8_t xor_table[0x04][0x08][0x08] =
	{
		{ // quadrants 1, 2 should be ok, missing decrypted 0x82, 0xc4, 0xc7, 0xd0, 0xd4, 0xd7
			{ 0x50, 0x12, 0x02, 0x11, 0x40, 0x11, 0x50, 0x52 }, // 0x0x and 0x2x
			{ 0x53, 0x01, 0x43, 0x12, 0x01, 0x10, 0x41, 0x13 }, // 0x1x and 0x3x
			{ 0x51, 0x12, 0x03, 0x40, 0x02, 0x00, 0x01, 0x51 }, // 0x4x and 0x6x
			{ 0x12, 0x03, 0x12, 0x51, 0x00, 0x52, 0x50, 0x40 }, // 0x5x and 0x7x
			{ 0x12, 0x53, 0x41, 0x02, unkn, unkn, 0x00, 0x41 }, // 0x8x and 0xax
			{ 0x51, unkn, 0x03, 0x40, 0x42, unkn, 0x01, 0x03 }, // 0x9x and 0xbx
			{ 0x00, 0x10, 0x52, 0x01, 0x41, 0x00, 0x41, 0x12 }, // 0xcx and 0xex
			{ 0x50, 0x52, 0x41, unkn, 0x41, unkn, 0x40, 0x53 }  // 0xdx and 0xfx
		},
		{ // quadrants 1, 2, 3 should be ok, missing decrypted 0xc7, 0xd7
			{ 0x52, 0x12, 0x43, 0x00, 0x01, 0x50, 0x11, 0x40 }, // 0x0x and 0x2x
			{ 0x52, 0x52, 0x00, 0x53, 0x12, 0x01, 0x00, 0x10 }, // 0x1x and 0x3x
			{ 0x41, 0x12, 0x12, 0x41, 0x00, 0x00, 0x12, 0x52 }, // 0x4x and 0x6x
			{ 0x01, 0x51, 0x42, 0x42, 0x02, 0x51, 0x01, 0x11 }, // 0x5x and 0x7x
			{ 0x53, 0x03, 0x10, 0x53, 0x51, 0x43, unkn, unkn }, // 0x8x and 0xax
			{ 0x10, 0x53, 0x03, 0x10, 0x42, 0x01, 0x42, 0x11 }, // 0x9x and 0xbx
			{ 0x50, 0x52, 0x43, 0x03, 0x43, 0x00, 0x51, 0x52 }, // 0xcx and 0xex
			{ 0x01, 0x12, 0x00, 0x12, 0x10, 0x50, 0x40, 0x53 }  // 0xdx and 0xfx
		},
		{ // quadrants 1, 2, 4 should be ok, missing decrypted 0x82, 0xd0
			{ 0x50, 0x12, 0x53, 0x03, 0x02, 0x02, 0x50, 0x40 }, // 0x0x and 0x2x
			{ 0x11, 0x51, 0x00, 0x03, 0x11, 0x40, 0x12, 0x40 }, // 0x1x and 0x3x
			{ 0x01, 0x42, 0x01, 0x01, 0x10, 0x00, 0x52, 0x03 }, // 0x4x and 0x6x
			{ 0x52, 0x03, 0x43, 0x00, 0x42, 0x40, 0x41, 0x11 }, // 0x5x and 0x7x
			{ unkn, 0x01, 0x53, 0x41, 0x13, 0x02, 0x40, 0x11 }, // 0x8x and 0xax
			{ 0x02, 0x02, 0x40, 0x10, 0x50, 0x43, 0x13, 0x40 }, // 0x9x and 0xbx
			{ 0x41, 0x51, 0x02, unkn, 0x11, 0x00, 0x12, 0x43 }, // 0xcx and 0xex
			{ 0x11, 0x02, 0x11, 0x42, 0x40, 0x40, 0x50, 0x10 }  // 0xdx and 0xfx
		},
		{ // quadrants 1, 2, 3 should be ok, missing decrypted 0xc4, 0xc7
			{ 0x50, 0x11, 0x01, 0x02, 0x42, 0x41, 0x51, 0x11 }, // 0x0x and 0x2x
			{ 0x53, 0x02, 0x43, 0x11, 0x53, 0x12, 0x03, 0x11 }, // 0x1x and 0x3x
			{ 0x13, 0x41, 0x03, 0x03, 0x41, 0x00, 0x52, 0x50 }, // 0x4x and 0x6x
			{ 0x41, 0x13, 0x00, 0x41, 0x00, 0x51, 0x03, 0x01 }, // 0x5x and 0x7x
			{ 0x13, 0x43, 0x03, 0x50, 0x52, unkn, 0x51, 0x11 }, // 0x8x and 0xax
			{ 0x12, 0x43, 0x02, 0x53, 0x11, unkn, 0x50, 0x02 }, // 0x9x and 0xbx
			{ 0x01, 0x42, 0x12, 0x51, 0x50, 0x00, 0x42, 0x12 }, // 0xcx and 0xex
			{ 0x41, 0x00, 0x52, 0x10, 0x52, 0x42, 0x51, 0x03 }  // 0xdx and 0xfx
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


//    YEAR  NAME       PARENT    MACHINE   INPUT      STATE           INIT           ROT    COMPANY      FULLNAME                          FLAGS
GAME( 1992, supercrd,  0,        supercrd, supercrd,  supercrd_state, init_supercrd, ROT0,  "Fun World", "Super Card (v417, encrypted)",   MACHINE_NOT_WORKING )
GAME( 1992, fruitstr,  0,        supercrd, fruitstr,  supercrd_state, init_fruitstr, ROT0,  "Fun World", "Fruit Star (T10S, encrypted)",   MACHINE_NOT_WORKING )
GAME( 1992, fruitstra, fruitstr, supercrd, fruitstra, supercrd_state, init_fruitstr, ROT0,  "Fun World", "Fruit Star (v810, encrypted)",   MACHINE_NOT_WORKING )
GAME( 1990, gampo816,  0,        supercrd, supercrd,  supercrd_state, init_gampo,    ROT0,  "Fun World", "Gamble Poker (v816, encrypted)", MACHINE_NOT_WORKING )
GAME( 1990, gampo812,  gampo816, supercrd, supercrd,  supercrd_state, init_gampo,    ROT0,  "Fun World", "Gamble Poker (v812, encrypted)", MACHINE_NOT_WORKING )
GAME( 1994, supst839,  0,        supercrd, suprstar,  supercrd_state, init_supst,    ROT90, "Fun World", "Super Stars (v839, encrypted)",  MACHINE_NOT_WORKING )
GAME( 1990, supst834,  supst839, supercrd, supst834,  supercrd_state, init_supst,    ROT90, "Fun World", "Super Stars (v834, encrypted)",  MACHINE_NOT_WORKING )
GAME( 1989, redline,   0,        supercrd, redline,   supercrd_state, init_supst,    ROT90, "Fun World", "Red Line (v808, encrypted)",     MACHINE_NOT_WORKING )
