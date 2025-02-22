// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo
/*************************************************************************

  Fortune I - Baby - Video Poker.
  IGT - Interflip - Recreativos Franco - CODERE Argentina.

  --------------------------------------------------------

  Original driver by Grull Osgo.
  Rewrite and additional work by Roberto Fresca.


  Games running on this hardware:

  * Draw Poker,          1984, IGT - International Game Technology

  * Video Poker,         1984, InterFlip.
  * Black Jack,          1984, InterFlip.
  * Video Dado,          1987, InterFlip.
  * Video Cordoba,       1987, InterFlip.

  * Baby Poker,          1989, Recreativos Franco.
  * Baby Dado,           1989, Recreativos Franco.

  * Video Poker (v1403), 1989, CODERE Argentina.


***************************************************************************

  History:
  ========

  IGT was founded as an A-1 Supply by William "Si" Redd in 1975. It changed its name to Sircoma in 1978,
  and then to International Game Technology (IGT) in 1981. The company went public in 1981, following its
  success in the video poker machine industry.

  Along the way, in 1978, Fortune Coin Company was acquired and contained
  the basis to their Fortune I game machines.

  The Fortune I hardware consisted of the following games:
    Regular Draw Poker
    Progressive Draw Poker
    Joker Wild Poker
    Double Up Draw Poker
    Credit Draw Poker
    Lucky 7 Poker (Seven Card Stud Poker)
    Twenty One
    Live 21
    Count Down 21
    Two Hand 21
    In Between
    Regular Slot
    Credit Slot

  InterFlip (Spain), is a subsidiary of Recreativos Franco.
  This company was created mainly with the purpose of manufacture
  and distribute export-class games.

  These machines were build in upright wood style cabinets, and compliment
  with class "C" (for casinos) spanish gaming regulations.


***************************************************************************

  Hardware Info
  =============

  There are three well known platforms:
  Fortune I, Baby and Baby with I8051 microcontroller.


  Fortune I Platform (Original IGT)
  =================================
  This is a three board system: Main, Interface & Sound board.

  * Main board:

  1x Intel 8039 CPU               Clocked @ 6/8 MHz.
  2x 2716 EPROM                   Program ROM.
  2x 2716 EPROM                   Character/graphics ROM.
  1x 5101 (256 x 4) SRAM          Data Memory (Battery Backed RAM).
  2x 2114 (1024 x 4) SRAM         8-Bit Char. RAM.
  1x 2114 (1024 x 4) SRAM         4-Bit Color RAM.
  1x 74s287 (256 x 4) TTL PROM    Color PROM.

  1x 6 MHz.(or 8 MHz.) Crystal    CPU clock.
  1x 7.8643 MHz. Crystal          Video System.

  1x 3.6 Ni-Cd Battery            Data Memory.

  TTL type Raster video           Configurable HIGH-LOW resolution through hardware jumper.
                                  Configurable 50Hz-60Hz V-Sync through hardware jumper.

  * Interface

  I/O System                      Buffered, latched & optocoupled.


  * Sound board:

  1x Intel 8039 CPU               Clocked @ 8 MHz.
  2x 2716 EPROM                   Program ROM.
  1x 1408 DAC
  1x 8.0000 MHz. Crystal


  Baby Platform (Recreativos Franco)
  ==================================

  This is a single board system: Integrates all Fortune I hardware.
  Replaces the I8039 sound uP with an I8751 uP.
  Adds 1x AY-3-8910 sound processor.


  Baby with I8051 Platform (CODERE Argentina)
  ===========================================

  The same Baby platform which replaces the I8039 main cpu uP daugther board with:

  1x INTEL I8051 CPU            Main CPU.
  1x 27C256                     Program ROM.
  1x NEC D4364 SRAM             Data RAM.
  1x Dallas DS1216              Battery + Timekeeper SRAM Socket.
  1x Dallas DS12321             Watchdog.
  1x AM8255APC                  PPI.
  1x SN74HCT373                 Octal D-type transparent latch with 3-state outputs.
  1x SN74HCT138                 3-line to 8-line Inverting Decoder/demultiplexer.
  1x SN74HCT00                  Quadruple independent 2-input NAND gates.
  1x 8.00000 MHZ. Crystal       System Clock


********************************************************************************

  Main CPU Memory Map
  ===================

  0x0000 - 0x0FFF         Program ROM.

  Data & Video RAM are mapped through I/O hardware implementations due to
  I8039 memory addressing restrictions.


  Main CPU I/0 Map
  ================

  P1.0          ; Used at bit level, Aux_0 signal.
  P1.1          ; Used at bit level, Aux_1 signal.
  P1.2          ; Used at bit level, Aux_2 signal.
  P1.3          ; Used at bit level, Aux_3 signal.
  P1.4          ; Used at bit level, Aux_4 signal & Sound Latch bit 3
  P1.5          ; Used at bit level, Aux_5 signal & Sound Latch bit 0
  P1.6          ; Expands address bus for video and color RAM access (V.A8)
  P1.7          ; Expands address bus for video and color RAM access (V.A9)

  P2.0 - P2.3:  ; I8039 as address bus expansion (Program memory - High address nibble).

  P2.4:         ; Reads 8 bits from data buffer input port (interface for switch encoder).
                      Bit 0: Lamp_1.
                      Bit 1: Lamp_2.
                      Bit 2: Lamp_3.
                      Bit 3: Lamp_4.
                      Bit 4: Coin Acceptor.
                      Bit 5: Hopper 1 & Sound Latch bit 1.
                      Bit 6: Hopper 2 & Sound Latch bit 2.
                      Bit 7: Diverter.

                ; Writes 8 bits to data latch out port (lamps, relays and coils).
                      Bit 0: SW_encoder_0.
                      Bit 1: SW_encoder_1.
                      Bit 2: SW_encoder_2.
                      Bit 3: SW_encoder_3.
                      Bit 4: Coin Out.
                      Bit 5: Undocumented jumper.
                      Bit 6: N/U (pulled up).
                      Bit 7: N/U (pulled up).

  P2.5:         ; Enable access to data RAM.
  P2.6:         ; Enable access to video RAM (write mode) - no read mode.
  P2.7:         ; Enable access to color RAM (write Mode) - no read mode.


********************************************************************************

  Game Info
  =========

  Pay Tables:

  These machines had their pay tables out of screen, in a backlighted upper front panel.


                        Fortune 1 - Draw Poker
  ---------------------------------------------------------------------
  Coins Played       1 Coin    2 Coins    3 Coins    4 Coins    5 Coins
  ---------------------------------------------------------------------
  Royal Flush         300        600        900       1200       4500
  Strt Flush           50        100        150        200        250
  4 of a Kind          25         50         75        100        125
  Full House            9         18         27         36         45
  Flush                 6         12         18         24         30
  Straight              4          8         12         16         20
  3 of a Kind           3          6          9         12         15
  2 Pair                2          4          6          8         10
  ---------------------------------------------------------------------


                      Video Poker (Spanish text only)
  -------------------------------------------------------------------------
  Fichas Jugadas    1 Ficha    2 Fichas    3 Fichas    4 Fichas    5 Fichas
  -------------------------------------------------------------------------
  Escalera Maxima     250        500         750        1000        4000
     de color
  Escalera de Color   100        200         300         400         500
  Poker                50        100         150         200         250
  Full                 11         22          33          44          55
  Color                 7         14          21          28          35
  Escalera              5         10          15          20          25
  Trio                  3          6           9          12          15
  Doble Pareja          2          4           6           8          10
  -------------------------------------------------------------------------


                      Black Jack (Spanish text only)
  ----------------------------------------------------------------------------
  Fichas Jugadas       1 Ficha    2 Fichas    3 Fichas    4 Fichas    5 Fichas
  ----------------------------------------------------------------------------
  Empate                   1          2           3           4            5

  La Banca se pasa         2          4           6           8           10

  Jugador tiene mas        2          4           6           8           10
  que la banca

  Jugador tiene menos      2          4           6           8           10
  de 22 con 6 cartas

  Jugador tiene blackjack  2          5           7          10           12
  y la Banca no
  ----------------------------------------------------------------------------


                              Video Cordoba
  ----------------------------------------------------------------------------
  Fichas Jugadas       1 Ficha    2 Fichas    3 Fichas    4 Fichas    5 Fichas
  ----------------------------------------------------------------------------
  TRIPLE BAR             250         250         250         250        2000
  ............................................................................
  3 x Doble Bar        - 100            |    Olive-Olive-Any Bar        - 18
  3 x Single Bar       -  50            |    3 x Orange                 - 14
  3 x Any Bar          -  20            |    Orange-Orange-Any Bar      - 14
  3 x Bell             -  20            |    3 x Cherry                 - 10
  Bell-Bell-Any Bar    -  20            |    2 x Cherry                 -  5
  3 x Olive            -  10            |    1 x Cherry                 -  2
  ............................................................................
  All combinations are valid only from left to rigth
  ----------------------------------------------------------------------------


                   Video Dado
  ---------------------------------------------
  Twelve      (12)                    33 x 1
  Eleven      (11)                    16 x 1
  Crap        (2,3,12)                 8 x 1
  Seven       (7)                      5 x 1
  Field       (2,12)                   3 x 1
  8 or More   (8,9,10,11,12)           2 x 1
  6 or Less   (2,3,4,5,6)              2 x 1

  Winnings less or equal to 25 can be re-played
  ---------------------------------------------


  All payments with less than 400 coins are done through hopper.
  These payments will be automatic now, due to the new hopper implementation.

  Payments over 400 coins are manual.


**************************************************************************

  [2025-02-14]

  - Changed driver name to fortune1.cpp, being the most significative.
  - Hooked mechanical counters to all games.
  - Added hopper support to all games.
  - Changed the parent/clone relationships.
  - Hooked the new CPU and therefore all the functions.
  - Added NVRAM support to the MSC51 family.
  - Worked the 8155 connections.
  - New set of inputs for bpoker.
  - Added support of DS1215 timekeeper to the new platform.
  - Added watchdog support.
  - Promoted Video Poker (v1403) to working.
  - Fix some wrong connections on the layouts.
  - Fixed some buggy sounds.
  - New realistic button-lamps layout for Video Poker and Fortune1.
  - New realistic button-lamps layout for Baby Poker and Video Poker (v1403).
  - New realistic button-lamps layout for Black Jack (Interflip).
  - New realistic button-lamps layout for Video Dado.
  - New realistic button-lamps layout for Baby Dado.
  - New realistic button-lamps layout for Video Cordoba.
  - Documented the Fortune 1 paytable.
  - Added technical notes about all the three platforms.


  [2008-10-08]

  - Added Baby Poker Game.
  - Added Baby Dado Game.
  - Mapped "Hand Pay" button for Baby Games.
  - Added decoder to Jackpot mechanical counter.
  - Added sound support to Baby Poker Game.
  - Added tower lamps to Baby Games layouts.
  - Reworked layouts for Baby Games.
  - Reworked the color routines.
  - Added new color routines for Baby Games.
  - Redumped the videocba color PROM.
  - Added color switch. (It changes background color in some games).
  - Added "hopper full" switch support (for diverter function).
  - Added diverter function decoder.
  - Added Button-lamps layout.
  - Added full functional mechanical counters decoding.
  - Added 7 Segment decoder and 7 Digit Counter functions.
  - Added button-lamps layout & mechanical counters simulation on layout.
     Mechanical counters to layout: Coin-In, Coin-Out and Coin to Drop.
  - Added NVRAM support to mechanical counters.


  TO DO
  =====

  * Fix some missing pulses on mechanical counters.
  * Fix the bug on bookkeeping mode (videodad & videocba).


**************************************************************************/


#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/ds1215.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "babydad.lh"
#include "babypkr.lh"
#include "blckjack.lh"
#include "bpoker.lh"
#include "videocba.lh"
#include "videodad.lh"
#include "videopkr.lh"


namespace {

#define CPU_CLOCK       (XTAL(6'000'000))         // main cpu clock
#define CPU_CLOCK_ALT   (XTAL(8'000'000))         // alternative main cpu clock for newer games
#define SOUND_CLOCK     (XTAL(8'000'000))         // sound cpu clock
#define VIDEO_CLOCK     (XTAL_7.8643MHz)


class videopkr_state : public driver_device
{
public:
	videopkr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_data_ram(*this, "data_ram", 0x100, ENDIANNESS_LITTLE)
		, m_video_ram(*this, "video_ram", 0x400, ENDIANNESS_LITTLE)
		, m_color_ram(*this, "color_ram", 0x400, ENDIANNESS_LITTLE)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_digits(*this, "digit%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
		, m_hopper(*this, "hopper")
	{ }

	void videopkr(machine_config &config);
	void blckjack(machine_config &config);
	void videodad(machine_config &config);
	void fortune1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint8_t videopkr_io_r(offs_t offset);
	void videopkr_io_w(offs_t offset, uint8_t data);
	uint8_t videopkr_p1_data_r();
	uint8_t videopkr_p2_data_r();
	void videopkr_p1_data_w(uint8_t data);
	void videopkr_p2_data_w(uint8_t data);
	int videopkr_t0_latch();
	void prog_w(int state);
	uint8_t sound_io_r();
	void sound_io_w(uint8_t data);
	uint8_t sound_p2_r();
	void sound_p2_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void videopkr_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(vidadcba);
	void fortune1_palette(palette_device &palette) const;
	uint32_t screen_update_videopkr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(sound_t1_callback);
	void count_7dig(unsigned long data, uint8_t index);

	void i8039_io_port(address_map &map) ATTR_COLD;
	void i8039_map(address_map &map) ATTR_COLD;
	void i8039_sound_mem(address_map &map) ATTR_COLD;
	void i8039_sound_port(address_map &map) ATTR_COLD;

	memory_share_creator<uint8_t> m_data_ram;
	memory_share_creator<uint8_t> m_video_ram;
	memory_share_creator<uint8_t> m_color_ram;
	uint16_t m_p1;
	uint16_t m_p2;
	uint8_t m_t0_latch;
	uint16_t m_n_offs;
	uint8_t m_vp_sound_p2;
	uint8_t m_p24_data;
	uint8_t m_sound_latch;
	uint8_t m_sound_ant;
	uint8_t m_dc_4020;
	uint8_t m_dc_40103;
	uint8_t m_te_40103;
	uint8_t m_ld_40103;
	uint8_t m_ant_jckp;
	uint8_t m_jckp;
	uint8_t m_ant_cio;
	uint8_t m_c_io;
	uint8_t m_hp_1;
	uint8_t m_hp_2;
	uint8_t m_bell;
	uint8_t m_aux3;
	uint8_t m_dvrt;
	unsigned long m_count0;
	unsigned long m_count1;
	unsigned long m_count2;
	unsigned long m_count3;
	unsigned long m_count4;
	tilemap_t *m_bg_tilemap;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<28> m_digits;
	output_finder<14> m_lamps;
	optional_device<ticket_dispenser_device> m_hopper;
};


class babypkr_state : public videopkr_state
{
public:
	babypkr_state(const machine_config &mconfig, device_type type, const char *tag)
		: videopkr_state(mconfig, type, tag)
		, m_aysnd(*this, "aysnd")
		, m_watchdog(*this, "watchdog")
		, m_rtc(*this, "rtc")
		, m_top_lamps(*this, "TOP_%u", 1U)
	{
	}

	void babypkr(machine_config &config);
	void bpoker(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	void prog_w(uint8_t data);
	uint8_t bp_io_port_r(offs_t offset);
	void bp_io_port_w(offs_t offset, uint8_t data);
	uint8_t bp_video_io_r(offs_t offset);
	void bp_video_io_w(offs_t offset, uint8_t data);
	uint8_t bp_color_io_r(offs_t offset);
	void bp_color_io_w(offs_t offset, uint8_t data);
	void bpoker_p1_data_w(uint8_t data);
	void bpoker_wd_reset_w(offs_t offset, uint8_t data);
	uint8_t bp_timekeep_r(offs_t offset);
	void bp_timekeep_w(offs_t offset, uint8_t data);

private:
	uint8_t baby_sound_p0_r();
	void baby_sound_p0_w(uint8_t data);
	uint8_t baby_sound_p1_r();
	void baby_sound_p3_w(uint8_t data);
	void babypkr_palette(palette_device &palette) const;

	void i8751_map(address_map &map) ATTR_COLD;
	void i8751_io_port(address_map &map) ATTR_COLD;
	void i8051_sound_mem(address_map &map) ATTR_COLD;
	void i8051_sound_port(address_map &map) ATTR_COLD;

	required_device<ay8910_device> m_aysnd;
	optional_device<watchdog_timer_device> m_watchdog;
	optional_device<ds1215_device> m_rtc;
	output_finder<3> m_top_lamps;

	uint8_t m_sbp0 = 0U;
};


/***********************************
*          Video Hardware          *
***********************************/

// BCD to 7-seg decoder
static uint8_t dec_7seg(int data)
{
	uint8_t segment;
	switch (data)
	{
		case 0: segment = 0x3f; break;
		case 1: segment = 0x06; break;
		case 2: segment = 0x5b; break;
		case 3: segment = 0x4f; break;
		case 4: segment = 0x66; break;
		case 5: segment = 0x6d; break;
		case 6: segment = 0x7d; break;
		case 7: segment = 0x07; break;
		case 8: segment = 0x7f; break;
		case 9: segment = 0x6f; break;
		default: segment = 0x79;
	}

	return segment;
}

// Display a seven digit counter on layout - Index points to less significant digit
void videopkr_state::count_7dig(unsigned long data, uint8_t index)
{
	for (auto i = 0; i < 7; i++)
	{
		m_digits[index+i] = dec_7seg(data % 10);
		data /= 10;
	}
}

void videopkr_state::videopkr_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int j = 0; j < palette.entries(); j++)
	{
		int const i = BIT(color_prom[j], 3);

		// red component
		int const tr = 0xf0 - (0xf0 * BIT(color_prom[j], 0));
		int const r = tr - (i * (tr / 5));

		// green component
		int const tg = 0xf0 - (0xf0 * BIT(color_prom[j], 1));
		int const g = tg - (i * (tg / 5));

		// blue component
		int const tb = 0xf0 - (0xf0 * BIT(color_prom[j], 2));
		int const b = tb - (i * (tb / 5));

		palette.set_pen_color(j, rgb_t(r, g, b));
	}
}

void babypkr_state::babypkr_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int j = 0; j < palette.entries(); j++)
	{
		// intensity component
		int const i = 0x2f * BIT(color_prom[j], 3);
		int const top = 0xff - i;

		// red component
		int const tr = 0xdf * BIT(color_prom[j], 0);
		int const r = top - ((tr * top) / 0x100 );

		// green component
		int const tg = 0xdf * BIT(color_prom[j], 1);
		int const g = top - ((tg * top) / 0x100 );

		// blue component
		int const tb = 0xdf * BIT(color_prom[j], 2);
		int const b = top - ((tb * top) / 0x100);

		palette.set_pen_color(j, rgb_t(r, g, b));
	}
}

void videopkr_state::fortune1_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int j = 0; j < palette.entries(); j++)
	{
		int const i = BIT(color_prom[j], 3);

		// red component
		int const tr = 0xf0 - (0xf0 * BIT(color_prom[j], 0));
		int const r = tr - (i * (tr / 5));

		// green component
		int const tg = 0xf0 - (0xf0 * BIT(color_prom[j], 1));
		int const g = tg - (i * (tg / 5));

		// blue component
		int const tb = 0xf0 - (0xf0 * BIT(color_prom[j], 2));
		int const b = tb - (i * (tb / 5));

		// Swap position of Inner-most colors on each 4 color palette
		int c = j;
		if ((c % 4) == 1 || (c % 4) == 2)
			c = (int(c / 4) * 4) + (3 - (c % 4));

		palette.set_pen_color(c, rgb_t(r, g, b));
	}
}

TILE_GET_INFO_MEMBER(videopkr_state::get_bg_tile_info)
{
	int offs = tile_index;
	int attr = m_color_ram[offs] + ioport("IN2")->read();  // Color switch action
	int code = m_video_ram[offs];
	int color = attr;
	tileinfo.set(0, code, color, 0);
}


void videopkr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(videopkr_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

VIDEO_START_MEMBER(videopkr_state,vidadcba)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(videopkr_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);
}


uint32_t videopkr_state::screen_update_videopkr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->mark_all_dirty();
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/***********************************
*           R/W Handlers           *
***********************************/

uint8_t videopkr_state::videopkr_io_r(offs_t offset)
{
	uint8_t valor = 0, hf, co;

	uint16_t kbdin;

	switch (m_p2)
	{
		case 0xef:  // inputs are multiplexed through a diode matrix
		{
			hf = ((ioport("IN1")->read() & 0x10 ) >> 4) & 1;        // Hopper full detection
			co = 0x10 * ((ioport("IN1")->read() & 0x20 ) >> 5);     // Coin out detection
			kbdin = ((ioport("IN1")->read() & 0xaf ) << 8) + ioport("IN0")->read();

			switch (kbdin)
			{
				case 0x0000: valor = 0x00; break;
				case 0x0001: valor = 0x01; break;   // Door
				case 0x4000: valor = 0x02; break;
				case 0x8000: valor = 0x03; break;   // Hand Pay
				case 0x0002: valor = 0x04; break;   // Books
				case 0x0004: valor = 0x05; break;   // Coin In
				case 0x0008: valor = 0x07; break;   // Start
				case 0x0010: valor = 0x08; break;   // Discard
				case 0x0020: valor = 0x09; break;   // Cancel
				case 0x0040: valor = 0x0a; break;   // Hold 1
				case 0x0080: valor = 0x0b; break;   // Hold 2
				case 0x0100: valor = 0x0c; break;   // Hold 3
				case 0x0200: valor = 0x0d; break;   // Hold 4
				case 0x0400: valor = 0x0e; break;   // Hold 5
				case 0x0800: valor = 0x06; break;   // Bet
			}

			if ((valor == 0x00) & hf )
			{
				valor = 0x0f;
			}

			valor += co;
			break;
		}

		case 0xdf:
		{
			m_n_offs = ((m_p1 & 0xc0) << 2 ) + offset;
			valor = m_data_ram[offset];
			break;
		}

		case 0x5f:
		{
			m_n_offs = ((m_p1 & 0xc0) << 2 ) + offset;
			valor = m_data_ram[offset];
			break;
		}

		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
		{
			m_n_offs = ((m_p1 & 0xc0) << 2 ) + offset;
			valor = m_color_ram[m_n_offs];
			break;
		}

		case 0xbc:
		case 0xbd:
		case 0xbe:
		case 0xbf:
		{
			m_n_offs = ((m_p1 & 0xc0) << 2 ) + offset;
			valor = m_video_ram[m_n_offs];
			break;
		}
	}

	return valor;
}

void videopkr_state::videopkr_io_w(offs_t offset, uint8_t data)
{
	switch (m_p2)
	{
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
		{
			m_n_offs = ((m_p1 & 0xc0) << 2 ) + offset;
			m_color_ram[m_n_offs] = data & 0x0f;
			m_video_ram[m_n_offs] = data;
			m_bg_tilemap->mark_tile_dirty(m_n_offs);
			break;
		}

		case 0xdf:
		{
			m_data_ram[offset] = (data & 0x0f) + 0xf0;
			break;
		}

		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
		{
			m_n_offs = ((m_p1 & 0xc0) << 2 ) + offset;
			m_color_ram[m_n_offs] = data & 0x0f;
			m_bg_tilemap->mark_tile_dirty(m_n_offs);
			break;
		}

		case 0xbc:
		case 0xbd:
		case 0xbe:
		case 0xbf:
		{
			m_n_offs = ((m_p1 & 0xc0) << 2 ) + offset;
			m_video_ram[m_n_offs] = data;
			m_bg_tilemap->mark_tile_dirty(m_n_offs);
			break;
		}

		case 0xef:  // Port 2.4
		{
			m_lamps[0] = BIT(data, 0);    // L_1
			m_lamps[1] = BIT(data, 1);    // L_2
			m_lamps[2] = BIT(data, 2);    // L_3
			m_lamps[3] = BIT(data, 3);    // L_4
			m_lamps[4] = BIT(data, 4);    // Coin
			m_lamps[5] = BIT(data, 5);    // Hopper_1
			m_lamps[6] = BIT(data, 6);    // Hopper_2
			m_lamps[7] = BIT(data, 7);    // Diverter
			m_p24_data = data;
			m_hp_1 = (~m_p24_data >> 6) & 1;
			m_hp_2 = (~m_p24_data >> 5) & 1;
			m_dvrt = (~m_p24_data >> 7) & 1;

			if((m_p24_data & 0x60)==0x60)
				m_hopper->motor_w(true);
			else
				m_hopper->motor_w(false);

			//popmessage("hopper %02x", m_p24_data & 0x60);

			break;
		}

		case 0xff:
		{
			m_t0_latch = m_t0_latch ^ 0x01;     // fix the bookkeeping mode
			break;
		}
	}
}

uint8_t videopkr_state::videopkr_p1_data_r()
{
	return m_p1;
}

uint8_t videopkr_state::videopkr_p2_data_r()
{
	return m_p2;
}

void videopkr_state::videopkr_p1_data_w(uint8_t data)
{
	m_p1 = data;

	m_lamps[8] = BIT(data, 0);    // Aux_0 - Jackpot mech. counter (Baby Games)
	m_lamps[9] = BIT(data, 1);    // Aux_1 -
	m_lamps[10] = BIT(data, 2);   // Aux_2 -
	m_lamps[11] = BIT(data, 3);   // Aux_3 -
	m_lamps[12] = BIT(data, 4);   // Aux_4 - Bell
	m_lamps[13] = BIT(data, 5);   // Aux_5 - /CIO

	m_jckp = m_p1 & 1;

	if ((~m_c_io & 1) & m_ant_cio & m_hp_1 & m_hp_2)
	{
		++m_count1;  // Decoded Coin In mechanical counter
		machine().bookkeeping().coin_counter_w(0, 1);
		machine().bookkeeping().coin_counter_w(0, 0);
	}

	if ((~m_c_io & 1) & m_ant_cio & (~m_hp_1 & 1) & (~m_hp_2 & 1))
	{
		++m_count2;  // Decoded Coin Out mechanical counter
		machine().bookkeeping().coin_counter_w(1, 1);
		machine().bookkeeping().coin_counter_w(1, 0);
	}

	if (~m_c_io & m_ant_cio & m_hp_1 & m_hp_2 & ~m_dvrt)
	{
		++m_count3;  // Decoded Coin to Drop mechanical counter
		machine().bookkeeping().coin_counter_w(2, 1);
		machine().bookkeeping().coin_counter_w(2, 0);
	}

	if (~m_jckp & m_ant_jckp)
	{
		++m_count4;  // Decoded Jackpot mechanical counter
		machine().bookkeeping().coin_counter_w(3, 1);
		machine().bookkeeping().coin_counter_w(3, 0);
	}

	count_7dig(m_count1, 0);
	count_7dig(m_count2, 7);
	count_7dig(m_count3, 14);
	count_7dig(m_count4, 21);

	m_ant_cio = m_c_io;
	m_ant_jckp = m_jckp;
}

void videopkr_state::videopkr_p2_data_w(uint8_t data)
{
	m_p2 = data;
}

int videopkr_state::videopkr_t0_latch()
{
	return m_t0_latch;
}

void videopkr_state::prog_w(int state)
{
	if (!state)
		m_maincpu->set_input_line(0, CLEAR_LINE);   // clear interrupt FF
}

TIMER_DEVICE_CALLBACK_MEMBER(videopkr_state::sound_t1_callback)
{
	if (m_te_40103 == 1)
	{
		m_dc_40103++;

		if (m_dc_40103 == 0)
		{
			m_soundcpu->set_input_line(0, ASSERT_LINE);
		}
	}
}


/*************************************
*     Fortune I Sound Handlers       *
*************************************/
/*

  Sound Data ( Sound Board latch )

    Data Bit     Comes from
    ---------------------------------------------
    bit 0        Coin I/O     Port 1.5
    bit 1        Hopper2      Port 2.4 Data Bit 6
    bit 2        Hopper1      Port 2.4 Data Bit 5
    bit 3        Bell         Port 1.4
    bit 4        Aux_3        Port 1.3
    bit 5        N/U          Pulled Up
    bit 6        N/U          Pulled Up
    bit 7        N/U          Pulled Up


  Sound Codes

    Hex     Bin         Sound                       Game
    --------------------------------------------------------------------------
    0xFF    11111111    No Sound (default state)    All Games.
    0xFE    11111110    Coin In sound               All Games.
    0xEF    11101111    Cards draw sound            Video Poker & Black Jack.
    0xF9    11111001    Hopper run                  Video Poker & Black Jack.
    0xF8    11111000    Coin Out sound              Video Poker & Black Jack.
    0xF6    11110110    Coin Out sound              Video Dado  & Video Cordoba.
    0xFA    11111010    Dice rolling sound          Video Dado.
    0xFA    11111010    Spinning reels sound        Video Cordoba.
    0xFB    11111011    Dice rolling sound          Video Dado.
    0xFB    11111011    Stopping reels sound        Video Cordoba.

*/

uint8_t videopkr_state::sound_io_r()
{
	switch (m_vp_sound_p2)
	{
		case 0xbf:
		{
			m_c_io = (m_p1 >> 5) & 1;
			m_hp_1 = (~m_p24_data >> 6) & 1;
			m_hp_2 = (~m_p24_data >> 5) & 1;
			m_bell = (m_p1 >> 4) & 1;
			m_aux3 = (m_p1 >> 3) & 1;
			m_dvrt = (~m_p24_data >> 7) & 1;
			m_sound_ant = m_sound_latch;
			m_sound_latch = m_c_io + (m_hp_1 << 1) + (m_hp_2 << 2) + (m_bell << 3) + 0xf0;

			break;
		}
	}

	return m_sound_latch;
}

void videopkr_state::sound_io_w(uint8_t data)
{
	if (m_vp_sound_p2 == 0x5f || m_vp_sound_p2 == 0xdf)
	{
		m_dc_40103 = data;
		m_dc_4020 = 0;
	}
}

uint8_t videopkr_state::sound_p2_r()
{
	return m_vp_sound_p2;
}

void videopkr_state::sound_p2_w(uint8_t data)
{
	m_vp_sound_p2 = data;

	switch (data)
	{
		case 0x5f:
		{
			m_te_40103 = 0;  // p2.7 low
			m_ld_40103 = 0;  // p2.5 low
			break;
		}

		case 0x7f:
		{
			m_te_40103 = 0;
			m_ld_40103 = 1;
			break;
		}

		case 0xff:
		{
			m_te_40103 = 1;
			m_ld_40103 = 1;
			break;
		}
	}
}


/********************************************
*     Baby Platform with I8051 Handlers     *
*********************************************/

void  babypkr_state::prog_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);   // clear interrupt FF
}

void babypkr_state::bpoker_p1_data_w(uint8_t data)
{
	m_p1 = data;

	m_lamps[8] = BIT(data, 0);    // Aux_0 - Jackpot mechanical counter (Baby Games)
	m_lamps[9] = BIT(data, 1);    // Aux_1 -
	m_lamps[10] = BIT(data, 2);   // Aux_2 -
	m_lamps[11] = BIT(data, 3);   // Aux_3 -
	m_lamps[12] = BIT(data, 4);   // Aux_4 - Bell
	m_lamps[13] = BIT(data, 5);   // Aux_5 - /CIO

	m_jckp = m_p1 & 1;

	if ((~m_c_io & 1) & m_ant_cio & m_hp_1 & m_hp_2)
	{
		++m_count1;  // Decoded Coin In mechanical counter
		machine().bookkeeping().coin_counter_w(0, 1);
		machine().bookkeeping().coin_counter_w(0, 0);
	}

	if ((~m_c_io & 1) & m_ant_cio & (~m_hp_1 & 1) & (~m_hp_2 & 1))
	{
		++m_count2;  // Decoded Coin Out mechanical counter
		machine().bookkeeping().coin_counter_w(1, 1);
		machine().bookkeeping().coin_counter_w(1, 0);
	}

	if (~m_c_io & m_ant_cio & m_hp_1 & m_hp_2 & ~m_dvrt)
	{
		++m_count3;  // Decoded Coin to Drop mechanical counter
		machine().bookkeeping().coin_counter_w(2, 1);
		machine().bookkeeping().coin_counter_w(2, 0);
	}

	if (~m_jckp & m_ant_jckp)
	{
		++m_count4;  // Decoded Jackpot mechanical counter
		machine().bookkeeping().coin_counter_w(3, 1);
		machine().bookkeeping().coin_counter_w(3, 0);
	}

	count_7dig(m_count1, 0);
	count_7dig(m_count2, 7);
	count_7dig(m_count3, 14);
	count_7dig(m_count4, 21);

	m_ant_cio = m_c_io;
	m_ant_jckp = m_jckp;

}

uint8_t babypkr_state::bp_io_port_r(offs_t offset)
{
	uint8_t valor = 0;
	uint8_t hf, co;
	uint16_t kbdin;

	hf = ((ioport("IN1")->read() & 0x10 ) >> 4) & 1;        // Hopper full detection
	co = 0x10 * ((ioport("IN1")->read() & 0x20 ) >> 5);     // Coin Out detection
	kbdin = ((ioport("IN1")->read() & 0xaf ) << 8) + ioport("IN0")->read();

	switch (kbdin)
	{
		case 0x0000: valor = 0x00; break;
		case 0x0001: valor = 0x01; break;   // Door
		case 0x4000: valor = 0x02; break;
		case 0x8000: valor = 0x03; break;   // Hand Pay
		case 0x0002: valor = 0x04; break;   // Books
		case 0x0004: valor = 0x05; break;   // Coin In
		case 0x0008: valor = 0x07; break;   // Start
		case 0x0010: valor = 0x08; break;   // Discard
		case 0x0020: valor = 0x09; break;   // Cancel
		case 0x0040: valor = 0x0a; break;   // Hold 1
		case 0x0080: valor = 0x0b; break;   // Hold 2
		case 0x0100: valor = 0x0c; break;   // Hold 3
		case 0x0200: valor = 0x0d; break;   // Hold 4
		case 0x0400: valor = 0x0e; break;   // Hold 5
		case 0x0800: valor = 0x06; break;   // Bet
	}

	if ((valor == 0x00) & hf )
	{
		valor = 0x0f;
	}

	valor += co;

	return valor;
}

void babypkr_state::bp_io_port_w(offs_t offset, uint8_t data)
{
	m_lamps[0] = BIT(data, 0);    // L_1
	m_lamps[1] = BIT(data, 1);    // L_2
	m_lamps[2] = BIT(data, 2);    // L_3
	m_lamps[3] = BIT(data, 3);    // L_4
	m_lamps[4] = BIT(data, 4);    // Coin
	m_lamps[5] = BIT(data, 5);    // Hopper_1
	m_lamps[6] = BIT(data, 6);    // Hopper_2
	m_lamps[7] = BIT(data, 7);    // Diverter
	m_p24_data = data;
	m_hp_1 = (m_p24_data >> 6) & 1;
	m_hp_2 = (m_p24_data >> 5) & 1;
	m_dvrt = (~m_p24_data >> 7) & 1;

	// bit 5 - bit 6 -> Hopper
	if((data&0x60)==0x60)
		m_hopper->motor_w(true);
	else
		m_hopper->motor_w(false);
}

uint8_t babypkr_state::bp_video_io_r(offs_t offset)
{
	uint8_t ret;
	uint16_t v_addr = ((m_p1 & 0xc0) << 2 ) + offset;
		ret = m_video_ram[v_addr];
	return ret;
}

void babypkr_state::bp_video_io_w(offs_t offset, uint8_t data)
{
	uint16_t v_addr = ((m_p1 & 0xc0) << 2 ) + offset;
	m_video_ram[v_addr] = data;
	m_bg_tilemap->mark_tile_dirty(v_addr);
}

uint8_t babypkr_state::bp_color_io_r(offs_t offset)
{
	uint8_t ret;
	uint16_t v_addr = ((m_p1 & 0xc0) << 2 ) + offset;
	ret = m_color_ram[v_addr];
	return ret;
}

void babypkr_state::bp_color_io_w(offs_t offset, uint8_t data)
{
	uint16_t v_addr = ((m_p1 & 0xc0) << 2 ) + offset;
	m_color_ram[v_addr] = data;
	m_bg_tilemap->mark_tile_dirty(v_addr);
}

void babypkr_state::bpoker_wd_reset_w(offs_t offset, uint8_t data)
{
	m_watchdog->watchdog_reset();
}


uint8_t babypkr_state::bp_timekeep_r(offs_t offset)
{
	if (m_rtc->ceo_r())
		return m_rtc->read();
	else
		m_rtc->read();
	return 0;
}

void babypkr_state::bp_timekeep_w(offs_t offset, uint8_t data)
{
	if (m_rtc->ceo_r())
	{
		m_rtc->write(data & 0x01);
		return;
	}
	m_rtc->write(data & 0x01);
}


/*****************************************
*          Baby Sound Handlers           *
*****************************************/

uint8_t babypkr_state::baby_sound_p0_r()
{
	return m_sbp0;
}

void babypkr_state::baby_sound_p0_w(uint8_t data)
{
	m_sbp0 = data;
}

uint8_t babypkr_state::baby_sound_p1_r()
{
	m_c_io = (m_p1 >> 5) & 1;
	m_hp_1 = (~m_p24_data >> 6) & 1;
	m_hp_2 = (~m_p24_data >> 5) & 1;
	m_bell = (m_p1 >> 4) & 1;
	m_aux3 = (m_p1 >> 3) & 1;
	return m_c_io | (m_hp_1 << 1) | (m_hp_2 << 2) | 0xf8;
}

void babypkr_state::baby_sound_p3_w(uint8_t data)
{
	m_top_lamps[0] = BIT(data, 1);
	m_top_lamps[1] = BIT(data, 2);
	m_top_lamps[2] = BIT(data, 3);

	if (!(data & 0x10))
	{
		m_aysnd->reset();
		logerror("AY3-8910: Reset\n");
	}

	uint8_t ay_intf = (data >> 5) & 0x07;
	switch (ay_intf)
	{
		case 0x00:  break;
		case 0x01:  break;
		case 0x02:  break;
		case 0x03:  m_aysnd->data_w(m_sbp0); break;
		case 0x04:  break;
		case 0x05:  m_sbp0 = m_aysnd->data_r(); break;
		case 0x06:  break;
		case 0x07:  m_aysnd->address_w(m_sbp0); break;
	}
}


/*****************************************
*         Memory Map Information         *
*****************************************/

void videopkr_state::i8039_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void videopkr_state::i8039_io_port(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(videopkr_state::videopkr_io_r), FUNC(videopkr_state::videopkr_io_w));
}

void videopkr_state::i8039_sound_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void videopkr_state::i8039_sound_port(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(videopkr_state::sound_io_r), FUNC(videopkr_state::sound_io_w));
}

void babypkr_state::i8051_sound_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void babypkr_state::i8051_sound_port(address_map &map)
{
	map(0x0000, 0x1ff).ram();
}

void babypkr_state::i8751_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void babypkr_state::i8751_io_port(address_map &map)
{
	map(0x0000, 0x3fff).ram().share("nvram");
	map(0x4900, 0x49ff).rw(FUNC(babypkr_state::bp_timekeep_r), FUNC(babypkr_state::bp_timekeep_w));
	map(0x8000, 0x80ff).rw(FUNC(babypkr_state::bp_io_port_r), FUNC(babypkr_state::bp_io_port_w));
	map(0x9000, 0x9000).w(FUNC(babypkr_state::prog_w));  // replaces PROG line in i8039 used to clear interrupt flip flop
	map(0xa000, 0xa0ff).rw(FUNC(babypkr_state::bp_video_io_r), FUNC(babypkr_state::bp_video_io_w));  // partial video RAM address
	map(0xb000, 0xb0ff).rw(FUNC(babypkr_state::bp_color_io_r), FUNC(babypkr_state::bp_color_io_w));  // Idem to color RAM
	map(0xc000, 0xc003).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf000, 0xf000).w(FUNC(babypkr_state::bpoker_wd_reset_w));
}


/*****************************************
*              Input Ports               *
*****************************************/

static INPUT_PORTS_START( videopkr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )         PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )        PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL )   PORT_NAME("Discard")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_NAME("Hopper Weight") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT ) // over 400 Coins/Credits

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( blckjack )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )         PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )        PORT_NAME("Deal")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )       PORT_NAME("Hit")           PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 )       PORT_NAME("Stand")         PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 )       PORT_NAME("Double")        PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_NAME("Hopper Weight") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( videodad )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )         PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_NAME("Craps")         PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 )       PORT_NAME("6 or Less")     PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 )       PORT_NAME("Seven")         PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON5 )       PORT_NAME("8 or More")     PORT_CODE(KEYCODE_C)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 )       PORT_NAME("Field")         PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 )       PORT_NAME("Eleven")        PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON8 )       PORT_NAME("Twelve")        PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_NAME("Hopper Weight") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT ) // over 400 Coins/Credits

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( videocba )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )         PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )        PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_NAME("Hopper Weight") PORT_TOGGLE PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT ) // over 400 Coins/Credits

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( babypkr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )         PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )        PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL )   PORT_NAME("Double / Discard")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )  PORT_NAME("Cancel / Take / Payout")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_NAME("Hopper Weight") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT ) // over 400 Coins/Credits

	PORT_START("IN2")
	PORT_DIPNAME( 0x20, 0x00, "Color Sw." )             PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( babydad )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )         PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_NAME("Craps")         PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 )       PORT_NAME("6 or Less")     PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 )       PORT_NAME("Seven")         PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON5 )       PORT_NAME("8 or More")     PORT_CODE(KEYCODE_X)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 )       PORT_NAME("Field")         PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 )       PORT_NAME("Eleven")        PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON8 )       PORT_NAME("Twelve")        PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_NAME("Hopper Weight") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT ) // over 400 Coins/Credits

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( bpoker )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )         PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )        PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL )   PORT_NAME("Double / Discard")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )  PORT_NAME("Cancel / Take / Payout")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )   PORT_NAME("Hold 2 / Red")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )   PORT_NAME("Hold 4 / Black")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_NAME("Hopper Weight") PORT_CODE(KEYCODE_H) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT ) // over 400 Coins/Credits

	PORT_START("IN2")
	PORT_DIPNAME( 0x20, 0x00, "Color Sw." )             PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )

	PORT_START("PPI_PA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("SET")            PORT_CODE(KEYCODE_A)  // Change field value / Last Operations
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("NEXT")           PORT_CODE(KEYCODE_S)  // Select next field / Paper Cut (Ticket printer)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("INIT")           PORT_CODE(KEYCODE_D)  // Set NVRAM data / Set Time / Set Machine Number & Series
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("BOOK")           PORT_CODE(KEYCODE_F)  // Bookeeping / Enter / Exit
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("PERIOD")         PORT_CODE(KEYCODE_G)  // Operator Key / NVRAM Clear / Close and Report period
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("pa-5")           PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("pa-6")           PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("pa-7")           PORT_CODE(KEYCODE_L)

	PORT_START("PPI_PC")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("pc-4")           PORT_CODE(KEYCODE_E)  // /BUSY (Ticket printer)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("pc-5")           PORT_CODE(KEYCODE_T)  // /NO_PAPER (Ticket printer)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("pc-6")           PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("pc-7")           PORT_CODE(KEYCODE_U)
INPUT_PORTS_END


/*****************************************
*            Graphics Layouts            *
*****************************************/

static const gfx_layout tilelayout_16 =
{
	16, 8,
	RGN_FRAC(1,4),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		RGN_FRAC(1,4), RGN_FRAC(1,4) + 1, RGN_FRAC(1,4) + 2, RGN_FRAC(1,4) + 3,
		RGN_FRAC(1,4) + 4, RGN_FRAC(1,4) + 5, RGN_FRAC(1,4) + 6, RGN_FRAC(1,4) + 7
	},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tilelayout_8 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/************************************
*    Graphics Decode Information    *
************************************/

static GFXDECODE_START( gfx_videopkr )
	GFXDECODE_ENTRY( "tiles", 0, tilelayout_8, 0, 64 )
GFXDECODE_END


static GFXDECODE_START( gfx_videodad )
	GFXDECODE_ENTRY( "tiles", 0, tilelayout_16, 0, 64 )
GFXDECODE_END


/*****************************************
*         Machine Start / Reset          *
*****************************************/

void videopkr_state::machine_start()
{
	m_digits.resolve();
	m_lamps.resolve();

	m_vp_sound_p2 = 0xff;   // default P2 latch value
	m_sound_latch = 0xff;   // default sound data latch value
	m_p24_data = 0xff;
	m_p1 = 0xff;
	m_ant_cio = 0;
	m_count0 = 0;
	m_count1 = 0;
	m_count2 = 0;
	m_count3 = 0;
	m_count4 = 0;
	m_ant_jckp = 0;
}

void babypkr_state::machine_start()
{
	videopkr_state::machine_start();
	m_p24_data = 0;
	m_top_lamps.resolve();
}


/*****************************************
*            Machine Drivers             *
*****************************************/

void videopkr_state::videopkr(machine_config &config)
{
	// basic machine hardware
	i8039_device &maincpu(I8039(config, m_maincpu, CPU_CLOCK));
	maincpu.set_addrmap(AS_PROGRAM, &videopkr_state::i8039_map);
	maincpu.set_addrmap(AS_IO, &videopkr_state::i8039_io_port);
	maincpu.p1_in_cb().set(FUNC(videopkr_state::videopkr_p1_data_r));
	maincpu.p1_out_cb().set(FUNC(videopkr_state::videopkr_p1_data_w));
	maincpu.p2_in_cb().set(FUNC(videopkr_state::videopkr_p2_data_r));
	maincpu.p2_out_cb().set(FUNC(videopkr_state::videopkr_p2_data_w));
	maincpu.prog_out_cb().set(FUNC(videopkr_state::prog_w));
	maincpu.t0_in_cb().set(FUNC(videopkr_state::videopkr_t0_latch));
	maincpu.set_vblank_int("screen", FUNC(videopkr_state::irq0_line_assert));

	i8039_device &soundcpu(I8039(config, m_soundcpu, SOUND_CLOCK));
	soundcpu.set_addrmap(AS_PROGRAM, &videopkr_state::i8039_sound_mem);
	soundcpu.set_addrmap(AS_IO, &videopkr_state::i8039_sound_port);
	soundcpu.p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	soundcpu.p2_in_cb().set(FUNC(videopkr_state::sound_p2_r));
	soundcpu.p2_out_cb().set(FUNC(videopkr_state::sound_p2_w));

	NVRAM(config, "data_ram", nvram_device::DEFAULT_ALL_0);

	TIMER(config, "t1_timer").configure_periodic(FUNC(videopkr_state::sound_t1_callback), attotime::from_hz(50));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(5*8, 31*8-1, 3*8, 29*8-1);
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(videopkr_state::screen_update_videopkr));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_videopkr);
	PALETTE(config, "palette", FUNC(videopkr_state::videopkr_palette), 256);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.275);

	HOPPER(config, m_hopper, attotime::from_msec(150));
}

void videopkr_state::blckjack(machine_config &config)
{
	videopkr(config);

	// basic machine hardware

	// video hardware
	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(4*8, 31*8-1, 2*8, 30*8-1);
}

void videopkr_state::videodad(machine_config &config)
{
	videopkr(config);

	// basic machine hardware
	m_maincpu->set_clock(CPU_CLOCK_ALT);

	// video hardware
	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_size(32*16, 32*8);
	screen.set_visarea(4*16, 31*16-1, 2*8, 30*8-1);

	m_gfxdecode->set_info(gfx_videodad);
	MCFG_VIDEO_START_OVERRIDE(videopkr_state,vidadcba)
}

void videopkr_state::fortune1(machine_config &config)
{
	videopkr(config);

	// basic machine hardware
	m_maincpu->set_clock(CPU_CLOCK_ALT);

	// video hardware
	subdevice<palette_device>("palette")->set_init(FUNC(videopkr_state::fortune1_palette));
}

void babypkr_state::babypkr(machine_config &config)
{
	videopkr(config);

	// basic machine hardware
	m_maincpu->set_clock(CPU_CLOCK_ALT);

	// most likely romless or eprom
	i8031_device &soundcpu(I8031(config.replace(), m_soundcpu, CPU_CLOCK));
	soundcpu.set_addrmap(AS_PROGRAM, &babypkr_state::i8051_sound_mem);
	soundcpu.set_addrmap(AS_IO, &babypkr_state::i8051_sound_port);
	soundcpu.port_in_cb<0>().set(FUNC(babypkr_state::baby_sound_p0_r));
	soundcpu.port_out_cb<0>().set(FUNC(babypkr_state::baby_sound_p0_w));
	soundcpu.port_in_cb<1>().set(FUNC(babypkr_state::baby_sound_p1_r));
	soundcpu.port_out_cb<2>().set("dac", FUNC(dac_byte_interface::data_w));
	soundcpu.port_out_cb<3>().set(FUNC(babypkr_state::baby_sound_p3_w));

	// video hardware
	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_size(32*16, 32*8);
	screen.set_visarea(5*16, 31*16-1, 3*8, 29*8-1);

	subdevice<palette_device>("palette")->set_init(FUNC(babypkr_state::babypkr_palette));
	m_gfxdecode->set_info(gfx_videodad);
	MCFG_VIDEO_START_OVERRIDE(babypkr_state,vidadcba)

	AY8910(config, m_aysnd, CPU_CLOCK / 6);  // no ports used
	m_aysnd->add_route(ALL_OUTPUTS, "speaker", 0.3);
}

void babypkr_state::bpoker(machine_config &config)
{
	babypkr(config);

	i8751_device &maincpu(I8751(config.replace(), m_maincpu, XTAL(6'000'000)));
	maincpu.set_addrmap(AS_PROGRAM, &babypkr_state::i8751_map);
	maincpu.set_addrmap(AS_IO, &babypkr_state::i8751_io_port);
	maincpu.set_vblank_int("screen", FUNC(babypkr_state::irq0_line_assert));
	maincpu.port_in_cb<0>().set_constant(0);
	maincpu.port_out_cb<1>().set(FUNC(babypkr_state::bpoker_p1_data_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(600));  // Dallas DS1232: TD(2) -> float = 600ms) Verified
	DS1215(config, m_rtc);

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_size(32*16, 32*8);
	screen.set_visarea(3*16, 31*16-1, 0*8, 29*8-1);

	i8255_device &ppi(I8255A(config, "ppi"));
	//ppi.out_pa_callback().set_constant(0);
	ppi.in_pb_callback().set_ioport("PPI_PA");
	//ppi.out_pc_callback().set_constant(0);
	ppi.in_pc_callback().set_ioport("PPI_PC");
}


/*****************************************
*                Rom Load                *
*****************************************/

ROM_START( fortune1 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "pk485-s-000-7ff.c5",   0x0000, 0x0800, CRC(d74c4860) SHA1(9d151e2be5c1e9fc2e7ce5e533eb08e4b849f2c1) )
	ROM_LOAD( "pk485-s-800-fff.c7",   0x0800, 0x0800, CRC(490da6b0) SHA1(4b7afd058aeda929821d62c58e234769d64339e1) )

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "vpsona3.pbj",    0x0000, 0x0800, CRC(a4f7bf7f) SHA1(a08287821f3471cb3e1ae0528811da930fd57387) )
	ROM_LOAD( "vpsona2.pbj",    0x0800, 0x0800, CRC(583a9b95) SHA1(a10e85452e285b2a63f885f4e39b7f76ee8b2407) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "cg073-cg0-a.b12",     0x0000, 0x0800, CRC(fff2d7aa) SHA1(935b8623fda5b4b25ba1aaea869ebb2baded515c) )
	ROM_LOAD( "cg073-cg1-a.b15",     0x0800, 0x0800, CRC(a7cb05c4) SHA1(7cd76ade7cf9c50421b054ee525108829c31307c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "3140-cap8.b8", 0x0000, 0x0100, CRC(09abf5f1) SHA1(f2d6b4f2f08b47b93728dafb50576d5ca859255f) )
ROM_END

ROM_START( videopkr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "vpoker.c5",      0x0000, 0x0800, CRC(200d21e4) SHA1(d991c9f10a36a02491bb0aba32129675fed77a10) )
	ROM_LOAD( "vpoker.c7",      0x0800, 0x0800, CRC(f72c2a90) SHA1(e9c54d1f895cde0aaca4121a252da40594195a25) )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound cpu program */
	ROM_LOAD( "vpsona3.pbj",    0x0000, 0x0800, CRC(a4f7bf7f) SHA1(a08287821f3471cb3e1ae0528811da930fd57387) )
	ROM_LOAD( "vpsona2.pbj",    0x0800, 0x0800, CRC(583a9b95) SHA1(a10e85452e285b2a63f885f4e39b7f76ee8b2407) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "vpbj_b15.org",   0x0000, 0x0800, CRC(67468e3a) SHA1(761766f0fb92693d32179a914e11da517cc5747d) )
	ROM_LOAD( "vpbj_b12.org",   0x0800, 0x0800, CRC(4aba166e) SHA1(930cea2216a39b5d72021d1b449db018a121adce) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "vpbjorg.col",    0x0000, 0x0100, CRC(09abf5f1) SHA1(f2d6b4f2f08b47b93728dafb50576d5ca859255f) )
ROM_END

ROM_START( blckjack )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bjc5org.old",    0x0000, 0x0800, CRC(e266a28a) SHA1(1f90c85a2a817f1927c9ab2cbf79cfa2dd116dc8) )
	ROM_LOAD( "bjc7org.old",    0x0800, 0x0800, CRC(c60c565f) SHA1(c9ed232301750288bd000ac4e2dcf2253745ff0a) )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound cpu program */
	ROM_LOAD( "vpsona3.pbj",    0x0000, 0x0800, CRC(a4f7bf7f) SHA1(a08287821f3471cb3e1ae0528811da930fd57387) )
	ROM_LOAD( "vpsona2.pbj",    0x0800, 0x0800, CRC(583a9b95) SHA1(a10e85452e285b2a63f885f4e39b7f76ee8b2407) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "vpbj_b15.org",   0x0000, 0x0800, CRC(67468e3a) SHA1(761766f0fb92693d32179a914e11da517cc5747d) )
	ROM_LOAD( "vpbj_b12.org",   0x0800, 0x0800, CRC(4aba166e) SHA1(930cea2216a39b5d72021d1b449db018a121adce) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "vpbjorg.col",    0x0000, 0x0100, CRC(09abf5f1) SHA1(f2d6b4f2f08b47b93728dafb50576d5ca859255f) )
ROM_END

ROM_START( videodad )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dac5org.old",    0x0000, 0x0800, CRC(b373c8e9) SHA1(7a99d6aa152f8e6adeddbfdfd13278edeaa529bc) )
	ROM_LOAD( "dac7org.old",    0x0800, 0x0800, CRC(afabae30) SHA1(c4198ba8de6811e3367b0154ff479f6738721bfa) )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound cpu program */
	ROM_LOAD( "vdsona3.dad",    0x0000, 0x0800, CRC(13f7a462) SHA1(2e2e904637ca7873a2ed67d7ab1524e51b324660) )
	ROM_LOAD( "vdsona2.dad",    0x0800, 0x0800, CRC(120e4512) SHA1(207748d4f5793180305bb115af877042517d901f) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "vdadob15.bin",   0x0000, 0x0800, CRC(caa6a4b0) SHA1(af99da30b8ee63d54ac1f1e6737ed707501a5a25) )
	ROM_LOAD( "vdadob14.bin",   0x0800, 0x0800, CRC(eabfae6b) SHA1(189b38da5e9c99f99c5425cdfefccc6991e3f85e) )
	ROM_LOAD( "vdadob12.bin",   0x1000, 0x0800, CRC(176f7b31) SHA1(613521ed9caf904db22860686e0424d0c0e0cba6) )
	ROM_LOAD( "vdadob11.bin",   0x1800, 0x0800, CRC(259492c7) SHA1(003cc40a88f2b9fad0089574963e7e654211bb16) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "vdvcorg.col",    0x0000, 0x0100, CRC(741b1a22) SHA1(50983ea37f0479793ba38a112a0266c2edc4b5ef) )
ROM_END

ROM_START( videocba )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "vcc5org.old",    0x0000, 0x0800, CRC(96d72283) SHA1(056197a9e2ad40d1d6610bbe8a1855b81c0a6715) )
	ROM_LOAD( "vcc7org.old",    0x0800, 0x0800, CRC(fdec55c1) SHA1(19b740f3b7f2acaa0fc09f4c0a2fe69721ebbcaf) )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound cpu program */
	ROM_LOAD( "vcsona3.rod",    0x0000, 0x0800, CRC(b0948d6c) SHA1(6c45d350288f69b4b2b5ac16ab2b418f14c6eded) )
	ROM_LOAD( "vcsona2.rod",    0x0800, 0x0800, CRC(44ff9e85) SHA1(5d7988d2d3bca932b77e014dc61f7a2347b01603) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "vcbab15.bin",    0x0000, 0x0800, CRC(fce8c772) SHA1(f9736b724b620d60a17d77f6b773f39b99b47190) )
	ROM_LOAD( "vcbab14.bin",    0x0800, 0x0800, CRC(6fd66330) SHA1(0ee3b3329b94ded81f028ebb687e580787c74ded) )
	ROM_LOAD( "vcbab12.bin",    0x1000, 0x0800, CRC(e534d6c3) SHA1(7a93c6c07b5a28558ee005fed2098dc2933c3252) )
	ROM_LOAD( "vcbab11.bin",    0x1800, 0x0800, CRC(e2069a6d) SHA1(2d4e71f2838451215e6f9629e2d1a35808510353) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "vdcbaorg.col",   0x0000, 0x0100, CRC(6cdca5ae) SHA1(f7430af1adfa24fdd68a026ee431ead7d47ba269) )
ROM_END

ROM_START( babypkr )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "pok8039.old",    0x0000, 0x4000, CRC(c5400ef1) SHA1(1f27c92d2979319070a695f71ed494f6d47fe88f) )

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "dadvpbj.son",    0x0000, 0x1000, CRC(7b71cd30) SHA1(d782c50689a5aea632b6d274a1a7435a092ad20c) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "vpbjep15.mme",   0x00000, 0x8000,CRC(cad0f7cf) SHA1(0721b8b30dbf2a5da2967b0cfce24b4cd62d3f9d) )
	ROM_LOAD( "vpbjep14.mme",   0x08000, 0x8000,CRC(96f512fa) SHA1(f5344aeb57f53c43156e923fb7f0d8d37c73dbe9) )
	ROM_LOAD( "vpbjep12.mme",   0x10000, 0x8000,CRC(cfdca530) SHA1(609a5ad6f34e6b5c1c35584ddc62d4ff87546415) )
	ROM_LOAD( "vpbjep11.mme",   0x18000, 0x8000,CRC(44e6c489) SHA1(ca211cb3807c476cd8c5ac98b0d18b4b2724df45) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "babypok.col",    0x0000, 0x0100, CRC(2b98e88a) SHA1(bb22ef090e9e5dddc5c160d41a5f52df0db6feb6) )
ROM_END

ROM_START( babydad )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "da400org.old",   0x0000, 0x4000, CRC(cbca3a0c) SHA1(5d9428f26edf2c5531398a6ae36b4e9169b2c1c1) )

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "dadvpbj.son",    0x0000, 0x1000, CRC(7b71cd30) SHA1(d782c50689a5aea632b6d274a1a7435a092ad20c) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "ep15dad.dad",    0x00000, 0x8000,CRC(21bd102d) SHA1(52788d09dbe38fa29b8ff044a1c5249cad3d45b4) )
	ROM_LOAD( "ep14dad.dad",    0x08000, 0x8000,CRC(b6e2c8a2) SHA1(352d88e1d764da5133de2be9987d4875f0c9237f) )
	ROM_LOAD( "ep12dad.dad",    0x10000, 0x8000,CRC(98702beb) SHA1(6d42ea48df7546932570da1e9b0be7a1f01f930c) )
	ROM_LOAD( "ep11dad.dad",    0x18000, 0x8000,CRC(90aac63b) SHA1(8b312f2313334b4b5b0344b786aa1a7a4979ea92) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "babydad.col",    0x0000, 0x0100, CRC(b3358b3f) SHA1(d499a08fefaa3566de2e6fcddd237d6dfa840d8a) )
ROM_END

/*
  Video Poker PCB
  -----------------

  Main + daughterboard...


  MAINBOARD:
  ----------

  1x AMD D8751H.
  1x AY-3-8910A.
  1x LM380N.
  1x 6.0 MHz. Xtal.

  5x pots:
  - 3 together on the connectors opposit side (maybe RGB).
  - 1 near the AY-3-8910.
  - 1 opposit to the sound circuitry.

  1x 8.0000 MHz. Xtal, near the 3 pots (RGB).
  NOTE: silkscreened 7.8643 MHz. on the PCB.

  ROMs:

  conf_11_poker_ver_1.00_9055.bin : AMD Am27C256.
  checksum : 00779055h
  CRC-32 : B8ABC965h

  conf_12_poker_ver_1.00_3909.bin : AMD Am27C256.
  checksum : 00753909h
  CRC-32 : 3E72D96Ch

  conf_14_poker_ver_1.00_813a.bin : AMD Am27C256.
  checksum : 0074813Ah
  CRC-32 : F3D6A741h

  conf_15_poker_ver_1.00_ea91.bin : AMD Am27C256.
  checksum : 0074EA91h
  CRC-32 : 4EFEA023h

  sonido_dados_poker_y_b.jack_3d2f_(d8751h).bin : AMD D8751H.
  checksum : 000C3D2Fh
  CRC-32 : 7B71CD30h


  DAUGHTERBOARD:
  --------------

  1x AMD P80C31BH (8031 CPU)
  1x AMD P8255A (PPI)
  1x 40-pin empty socket (silkscreened 'zocalo' = socket)

  1x NEC D4464C-15 SRAM
  1x DALLAS DS1232

  1x 27256 ROM:

  b_poker_ver_1403.bin

  FUJITSU MBM27256.
  -Buffer checksum : 003D1403h
  -CRC-32 : 61ECA2F6h

  1x 8.0000 MHz. Xtal.

  1x 3-pin connector (JP2).
  1x 7-pin connector (JP4).
  1x 8-pin connector (JP3).
  1x 14-pin connector (impresora).

*/
ROM_START( bpoker )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "b_poker_ver_1403.bin", 0x0000, 0x8000, CRC(61eca2f6) SHA1(62a671e86b94005a9ffc4b6545a90c43880e0a11) )

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "sonido_dados_poker_y_b.jack_3d2f_=d8751h=.bin", 0x0000, 0x1000, CRC(7b71cd30) SHA1(d782c50689a5aea632b6d274a1a7435a092ad20c) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "conf_15_poker_ver_1.00_ea91.bin", 0x00000, 0x8000, CRC(4efea023) SHA1(c10a30353d793a54eab14bd5e9687668743b66de) )
	ROM_LOAD( "conf_14_poker_ver_1.00_813a.bin", 0x08000, 0x8000, CRC(f3d6a741) SHA1(5fbfcf4b8fdd1ef9f3d0f9acc735d5c23f45b607) )
	ROM_LOAD( "conf_12_poker_ver_1.00_3909.bin", 0x10000, 0x8000, CRC(3e72d96c) SHA1(3d921b9a79b8116984f58954141800d1856d8311) )
	ROM_LOAD( "conf_11_poker_ver_1.00_9055.bin", 0x18000, 0x8000, CRC(b8abc965) SHA1(61a6cdcfd4cd65d4a7ce02c6a2c4216ab6da095c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "bpkr_col.bin", 0x0000, 0x0100, CRC(1c34f02c) SHA1(7160f2216caf4854c892601fb6977fa9ada12187) )
ROM_END

} // Anonymous namespace


/*****************************************
*              Game Drivers              *
*****************************************/

//     YEAR  NAME      PARENT    MACHINE   INPUT     CLASS           INIT        ROT   COMPANY                                FULLNAME                          FLAGS                LAYOUT
GAMEL( 1984, fortune1, 0,        fortune1, videopkr, videopkr_state, empty_init, ROT0, "IGT - International Game Technology", "Fortune I (PK485-S) Draw Poker", 0,                   layout_videopkr )
GAMEL( 1984, videopkr, fortune1, videopkr, videopkr, videopkr_state, empty_init, ROT0, "InterFlip",                           "Video Poker",                    0,                   layout_videopkr )
GAMEL( 1984, blckjack, fortune1, blckjack, blckjack, videopkr_state, empty_init, ROT0, "InterFlip",                           "Black Jack (InterFlip)",         0,                   layout_blckjack )
GAMEL( 1987, videodad, fortune1, videodad, videodad, videopkr_state, empty_init, ROT0, "InterFlip",                           "Video Dado",                     0,                   layout_videodad )
GAMEL( 1987, videocba, fortune1, videodad, videocba, videopkr_state, empty_init, ROT0, "InterFlip",                           "Video Cordoba",                  0,                   layout_videocba )
GAMEL( 1987, babypkr,  fortune1, babypkr,  babypkr,  babypkr_state,  empty_init, ROT0, "Recreativos Franco",                  "Baby Poker",                     0,                   layout_babypkr  )
GAMEL( 1987, babydad,  fortune1, babypkr,  babydad,  babypkr_state,  empty_init, ROT0, "Recreativos Franco",                  "Baby Dado",                      0,                   layout_babydad  )
GAMEL( 1989, bpoker,   fortune1, bpoker,   bpoker,   babypkr_state,  empty_init, ROT0, "CODERE Argentina",                    "Video Poker (v1403)",            0,                   layout_bpoker   )
