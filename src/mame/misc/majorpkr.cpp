// license:BSD-3-Clause
// copyright-holders: Roberto Fresca
// thanks-to: Tomasz Slanina, Rob Ragon

/**********************************************************************************

  Major Poker / Major Joker
  1994 PAL System.

  Driver by Roberto Fresca.


  Special thanks to Tomasz Slanina & Rob Ragon for their invaluable help.

***********************************************************************************

  Hardware Notes:
  --------------

  1x Z80 @ 6 MHz. (main CPU)

  1x OKI M6295 @ 1.5 MHz. (4 channel mixing ADPCM voice synthesis). Pin7 high.

  1x Hitachi HD6845 (CRT Controller) @ 750 kHz.
     Hs 15625 Hz.
     Vs 52.786 Hz.

  1x Logic box (like CPU boxes) with 5 PLDs, maybe for protection.

  2x MB8464 (Video?)8Kx8
  1x MB8416 (NVRAM) 2Kx8
  1x MB8464 (?) 8Kx8

  2x 27C040 (Roms 1 & 2) (GFX ROMs).
  2x 27C010 (Roms 3 & 4) (GFX ROMs).
  1x 27C020 (Rom 5) (4-bit ADPCM samples)
  1x 27C512 (Rom 6) (main program)

  1x Xtal @ 12 MHz.

  4x 8 DIP switches banks.
  2x switches (SW1, SW2).

  1x 2x6-pin male connector. (jumper1)

  1x 2x10-pin edge connector.
  1x 2x22-pin edge connector.


***********************************************************************************

  PORTS:
  ------

  00  W ---> ROM bank.
  01  W ---> RAM bank.
  02  W ---> RAM bank.

  10 R  ---> Regular inputs (holds) multiplexed with credits out.
  10  W ---> Writes a kind of watchdog (bit4) and mech counters pulses (bits 0-1-2-3).
  11 R  ---> Read input port (remaining controls).
  11  W ---> Writes mux selector.
  12 R  ---> Read coin/credits port.
  12  W ---> Video registers: Normal or Up Down screen (bit6 off/on). Also writes 0xFC & 0x11 at CRTC offsets 0x0C & 0x0D (0's if normal).
  13 R  ---> Multiplexed port for DIP switches banks.
  13  W ---> Lamps out (array a)
  14 R  ---> Freeze. Switch in bit0 that pause the game when active. Just once active, the code loops till the bit resets.
  14  W ---> Lamps out (array b).

  50 RW ---> OKI6295 (R/W)

  60  W ---> PSG SN76489/96 initialization routines. (Maybe a leftover for different hardware)
             The offset is initialized with the following sequence: 0x9f, 0xbf, 0xdf, 0xff.


  Pulses - Port 10h.
  ------------------
    - bits -
    7654 3210
    ---- ---x   Credits Out mech counter.
    ---- --x-   Credits 3 mech counter.
    ---- -x--   Credits 1 mech counter.
    ---- x---   Credits 2 mech counter.
    ---x ----   Watchdog? (constant writes). (*)
    xxx- ----   Unknown.

  (*) Tied to a ULN Opto Triac (pin3 solder side edge connector - Lock Out (100V))


  Lamps - Array A - Port 13h.
  ---------------------------
    - bits -
    7654 3210
    ---- ---x   Hold 1 lamp.
    ---- --x-   Hold 2 lamp.
    ---- -x--   Hold 3 lamp.
    ---- x---   Hold 4 lamp.
    ---x ----   Hold 5 lamp.
    --x- ----   Big lamp.
    -x-- ----   Small lamp.
    x--- ----   Unknown.


  Lamps - Array B - Port 14h.
  ---------------------------
    - bits -
    7654 3210
    ---- ---x   Bet lamp.
    ---- --x-   Draw lamp.
    ---- -x--   Cancel lamp.
    ---- x---   Take lamp.
    ---x ----   Double-Up lamp.
    --x- ----   Fever lamp.
    xx-- ----   Unknown.

***********************************************************************************

  Instructions...
  (translated from a Japanese flyer)

  Bonus stage:
  - 10 fever awarded by three of a kind of 1, 3, or 7.
  - 15 fever awarded by four of a kind.

  Slot stage:
  - 10 bonus stage awarded by three consecutive 1, 3, or 7.
  -  5 bonus stage awarded by four consecutive numbers other than 1, 3, or 7.

  If bonus is awarded while you are in bonus stage, odds will be x4.
  Bonus stage & slot stage: You can get triple & fourth fever points, and you
  can challenge 3 or 4 times of double-up game by this point.

  ....

  Service Mode:

  F2 to enter the Book/Settings Mode.
  HOLD1 to move UP.
  HOLD2 to move DOWN.
  HOLD3 to move LEFT.
  HOLD4 to move RIGHT.
  HOLD5 to move next screen / exit.

  You can shortcut the DIP settings pressing "0" in Book Mode.
  HOLD5 to exit.

***********************************************************************************

   Major Poker: Things that you should know!!! (from "engrish" manual)
  -------------------------------------------------------------------

  The game not stabilise until 10,000 games have been played. As a result, the
  actual payout % may exceed the set payout % during this period. After the 10,000
  games the machine will follow the set payout %.

  Original game for your location.

  Analyzer screen #1: Provides you with data regarding games played so far such as
  numbers of games, number of fevers, etc...

  Analyzer screen #2: Allows you to adjust various aspects of the game to suit your
  location. For example you can increase the number of Fever (this does not affect
  the payout %). Also you can make the dealing speed quicker or slower.

  Even if you do not adjust the game it is ready to be played.


  DIP Switches....

  .-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----.
  | DIP Switches #1               |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | OFF Fixed                     | OFF |                                         |
  +-------------------------------+-----+-----+-----------------------------------+
  | Screen Direction   Normal     |     | OFF |                                   |
  |                    Up Down    |     | ON  |                                   |
  +-------------------------------+-----+-----+-----+-----------------------------+
  | Hopper             No         |           | OFF |                             |
  |                    Yes        |           | ON  |                             |
  +-------------------------------+-----------+-----+-----+-----------------------+
  | Hopper SW Active   High       |                 | OFF |                       |
  |                    Low        |                 | ON  |                       |
  +-------------------------------+-----------------+-----+-----+-----------------+
  | Coin Payout        Payout SW  |                       | OFF |                 |
  |                    Automatic  |                       | ON  |                 |
  +-------------------------------+-----------------------+-----+-----+-----------+
  | Hold Cancel        No         |                             | OFF |           |
  |                    Yes        |                             | ON  |           |
  +-------------------------------+-----------------------------+-----+-----+-----+
  | Auto Hold          No         |                                   | OFF |     |
  |                    Yes        |                                   | ON  |     |
  +-------------------------------+-----------------------------------+-----+-----+
  | Fever Mode         Yes        |                                         | OFF |
  |                    No         |                                         | ON  |
  '-------------------------------+-----------------------------------------+-----'

  .-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----.
  | DIP Switches #2               |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | Payout %           95%        | OFF | OFF | OFF |                             |
  |                    90%        | ON  | OFF | OFF |                             |
  |                    85%        | OFF | ON  | OFF |                             |
  |                    80%        | ON  | ON  | OFF |                             |
  |                    75%        | OFF | OFF | ON  |                             |
  |                    70%        | ON  | OFF | ON  |                             |
  |                    65%        | OFF | ON  | ON  |                             |
  |                    60%        | ON  | ON  | ON  |                             |
  +-------------------------------+-----+-----+-----+-----+-----+-----------------+
  | Min Bet size        1         |                 | OFF | OFF |                 |
  | for Fever           3         |                 | ON  | OFF |                 |
  |                     5         |                 | OFF | ON  |                 |
  |                    10         |                 | ON  | ON  |                 |
  +-------------------------------+-----------------+-----+-----+-----+-----+-----+
  | Double-Up          Weak       |                             | OFF | OFF |     |
  | Game Difficulty    ...        |                             | ON  | OFF |     |
  |                    ...        |                             | OFF | ON  |     |
  |                    Strong     |                             | ON  | ON  |     |
  +-------------------------------+-----------------------------+-----+-----+-----+
  | OFF Fixed                     |                                         | OFF |
  '-------------------------------+-----------------------------------------+-----'

  .-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----.
  | DIP Switches #3               |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | Key In   1 Coin - 50 Credits  | OFF | OFF | OFF |                             |
  |          1 Coin - 5 Credits   | ON  | OFF | OFF |                             |
  |          1 Coin - 10 Credits  | OFF | ON  | OFF |                             |
  |          1 Coin - 20 Credits  | ON  | ON  | OFF |                             |
  |          1 Coin - 25 Credits  | OFF | OFF | ON  |                             |
  |          1 Coin - 40 Credits  | ON  | OFF | ON  |                             |
  |          1 Coin - 60 Credits  | OFF | ON  | ON  |                             |
  |          1 Coin - 100 Credits | ON  | ON  | ON  |                             |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----------+
  | Coin-A   1 Coin - 5 Credits   |                 | OFF | OFF | OFF |           |
  |          1 Coin - 1 Credit    |                 | ON  | OFF | OFF |           |
  |          1 Coin - 2 Credits   |                 | OFF | ON  | OFF |           |
  |          1 Coin - 10 Credits  |                 | ON  | ON  | OFF |           |
  |          1 Coin - 20 Credits  |                 | OFF | OFF | ON  |           |
  |          1 Coin - 25 Credits  |                 | ON  | OFF | ON  |           |
  |          1 Coin - 40 Credits  |                 | OFF | ON  | ON  |           |
  |          1 Coin - 50 Credits  |                 | ON  | ON  | ON  |           |
  +-------------------------------+-----------------+-----+-----+-----+-----+-----+
  | Credit Limit        5000      |                                   | OFF | OFF |
  |                    10000      |                                   | ON  | OFF |
  |                    20000      |                                   | OFF | ON  |
  |                    30000      |                                   | ON  | ON  |
  '-------------------------------+-----------------------------------+-----+-----'

  .-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----.
  | DIP Switches #4               |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | Coin-B   1 Coin - 50 Credits  | OFF | OFF | OFF | OFF |                       |
  |          1 Coin - 1 Credit    | ON  | OFF | OFF | OFF |                       |
  |          1 Coin - 2 Credits   | OFF | ON  | OFF | OFF |                       |
  |          1 Coin - 4 Credits   | ON  | ON  | OFF | OFF |                       |
  |          1 Coin - 5 Credits   | OFF | OFF | ON  | OFF |                       |
  |          1 Coin - 10 Credits  | ON  | OFF | ON  | OFF |                       |
  |          1 Coin - 20 Credits  | OFF | ON  | ON  | OFF |                       |
  |          1 Coin - 25 Credits  | ON  | ON  | ON  | OFF |                       |
  |          1 Coin - 100 Credits | OFF | OFF | OFF | ON  |                       |
  |          2 Coins - 1 Credit   | ON  | OFF | OFF | ON  |                       |
  |          2 Coins - 5 Credits  | OFF | ON  | OFF | ON  |                       |
  |          4 Coins - 1 Credit   | ON  | ON  | OFF | ON  |                       |
  |          5 Coins - 1 Credit   | OFF | OFF | ON  | ON  |                       |
  |          5 Coins - 2 Credits  | ON  | OFF | ON  | ON  |                       |
  |          10 Coins - 1 Credit  | OFF | ON  | ON  | ON  |                       |
  |          20 Coins - 1 Credit  | ON  | ON  | ON  | ON  |                       |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----------+
  | Max Bet            20         |                       | OFF | OFF |           |
  |                    10         |                       | ON  | OFF |           |
  |                    30         |                       | OFF | ON  |           |
  |                    50         |                       | ON  | ON  |           |
  +-------------------------------+-----------------------+-----+-----+-----+-----+
  | Credit-In Limit    No Limit   |                                   | OFF | OFF |
  |                    1000       |                                   | ON  | OFF |
  |                    2000       |                                   | OFF | ON  |
  |                    5000       |                                   | ON  | ON  |
  '-------------------------------+-----------------------------------+-----+-----'


***********************************************************************************

  Power Supply edge connector Layout...

  .-----------------------+-+--+---------------------------.
  |       Components side |L|PN| Solder side               |
  +-----------------------+-+--+---------------------------+
  |                   GND |A|01| GND                       |
  |                   GND |B|02| GND                       |
  |                   +5V |C|03| +5V                       |
  |                   +5V |D|04| +5V                       |
  |              Lamp BET |E|05| Lamp HOLD 1               |
  |                  +12V |F|06| +12V                      |
  |             Lamp DEAL |H|07| Lamp HOLD 2               |
  |           Lamp CANCEL |J|08| Lamp HOLD 3               |
  |       Lamp TAKE SCORE |K|09| Lamp HOLD 4               |
  |        Lamp DOUBLE UP |L|10| Lamp HOLD 5               |
  '-----------------------+-+--+---------------------------'

  Edge connector Layout...

  .-----------------------+-+--+---------------------------.
  |       Components side |L|PN| Solder side               |
  +-----------------------+-+--+---------------------------+
  |           Out Counter |A|01| AC 100V In                |
  |        Key In Counter |B|02| AC Out to Hopper          |
  |     Coin In Counter A |C|03| Lock Out (100V)           |
  |             Sound GND |D|04| Sound Out                 |
  |        HOLD 5 / SMALL |E|05| Payout                    |
  |                Key In |F|06| HOLD 2                    |
  |                HOLD 3 |H|07| Coin B In                 |
  |                HOLD 4 |J|08| HOLD 1 / BIG              |
  |                       |K|09| RED (TV)                  |
  |                       |L|10| GREEN (TV)                |
  |            TAKE SCORE |M|11| Sync (TV)                 |
  |             DOUBLE UP |N|12| BLUE (TV)                 |
  |             Coin A In |P|13| DEAL                      |
  |                CANCEL |R|14| Analyzer (Books/Settings) |
  |  * SW2 (Clear Meters) |S|15| BET                       |
  |       Coin In Counter |T|16| Out of Fever Lamp         |
  |                ** GND |U|17| GND **                    |
  |                ** GND |V|18| GND **                    |
  |                       |W|19| GND                       |
  |                       |X|20|                           |
  |     Hopper Limited SW |Y|21| Hopper Payout             |
  |                       |Z|22|                           |
  '-----------------------+-+--+---------------------------'

  *   Soft SW2 Extension. Traced (undocumented).
  **  Edge connector pins U, V, 17 and 18 (GND), are tied
      to Power Supply edge connector pins A, B, 01 and 02.

***********************************************************************************

  Book / Settings Mode:
  ---------------------

  First Screen (Books & Meters):

  5C     0         F5C     0
  RF     0         FRF     0
  SF     0         FSF     0
  4C     0         F4C     0
  FH     0         FFH     0
  FL     0         FFL     0
  ST     0         FST     0
  3C     0         F3C     0
  2P     0         F2P     0

  MAIN GAMES               0
  FEVER GAMES              0
  FEVER OUT                0
  S-FEVER GAMES            0
  S-FEVER OUT              0
  TOTAL BET                0
  TOTAL SCORE              0
  SCORE%BET                0%
  W-UP IN                  0
  W-UP OUT                 0
  W-UP OUT%IN              0%
  CREDIT IN                0
  CREDIT OUT               0
  CREDIT OUT%IN            0%
  DOWN COUNT               0
  TOTAL-CREDIT IN          0
  TOTAL-CREDIT OUT         0
  TOTAL-CREDIT IN/OUT      0

  DATA RESET: In this first screen, press the SW2 to clear all data,
  except for the Total Credits meters.


  Second Screen (Settings):

                  MIN                NTL                MAX
  FEVER           --------------------*--------------------
  5 CARD          --------------------*--------------------
  ROYAL           --------------------*--------------------
  STR-FLUSH       --------------------*--------------------
  4 CARD          --------------------*--------------------
  FULL HOUSE      --------------------*--------------------
  FLUSH           --------------------*--------------------
  STRAIGHT        --------------------*--------------------
  3 CARD          --------------------*--------------------
  2 PAIR          --------------------*--------------------
  NORMAL JKR      --------------------*--------------------
  FEVER JKR       --------------------*--------------------
  DEAL SPEED      --------------------*--------------------
                  difficult appear              easy appear

  HOLD 1 (Up)
  HOLD 2 (Down)
  HOLD 3 (Left)
  HOLD 4 (Right)
  HOLD 5 (Next)


  Third Screen (Settings):

  Memory Switches...

  DEALER                OFF  ON         <--- Girl Dealer.
  FVR-ANIMATION         OFF  ON         <--- Fever Screen animation.
  DUP-NUDE              OFF  ON         <--- Double-Up Nude.
  SLOW-ACTION           OFF  ON         <--- Speed of Cards.
  KEY-LOCK              OFF  ON         <--- Lock settings leaving only Books. Other revisions may have "JQKA-1PAIR" (Jacks or Better).
  FVR-SLOT              OFF  ON         <--- Fever's number slot.
  AMUSEMENT-MODE        OFF  ON         <--- Self explanatory....

  1ST BET           1  5  10  20  30    <--- First Bet.
  CNT BET           1  5  10  20  30    <--- Second Bet.

  BACK-RGB R        0 1 2 3 4 5 6 7     <--- Background R component (0 = light, 7 = strong)
  BACK-RGB G        0 1 2 3 4 5 6 7     <--- Background G component (0 = light, 7 = strong)
  BACK-RGB B        0 1 2 3 4 5 6 7     <--- Background B component (0 = light, 7 = strong)

  BACK-PATTERN      0 1 2 3 4 5 6       <--- Background Pattern (0 = None, 1-6 available when dealer is off)

  HOLD 1 (Up)
  HOLD 2 (Down)
  HOLD 3 (Off)
  HOLD 4 (On)
  HOLD 5 (Exit)


  Fourth Screen (DIP switches test):

  DIP Switch test...

  DIPSW-1    00000000    00000000
  DIPSW-2    00000000    00000000
  DIPSW-3    00000000    00000000
  DIPSW-4    00000000    00000000

  HOLD 5 (Exit)

  Created of DIP Switch (Green)       <--- ?????
  Test Mode of DIP Switch (White)     <--- Show on the fly each DIP switch state.

***********************************************************************************

  To Do:

  - Find the input that unlocks the "KEY-LOCK" mode in the settings.
  - Resistors Network.

**********************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "majorpkr.lh"


// configurable logging
#define LOG_ROMBANK     (1U << 1)
#define LOG_PALETTEBANK (1U << 2)
#define LOG_VRAMBANK    (1U << 3)
#define LOG_LAMPSA      (1U << 4)
#define LOG_LAMPSB      (1U << 5)
#define LOG_PULSE       (1U << 6)

//#define VERBOSE (LOG_GENERAL | LOG_ROMBANK | LOG_PALETTEBANK | LOG_VRAMBANK | LOG_LAMPS | LOGLAMPSB | LOGPULSE)

#include "logmacro.h"

#define LOGROMBANK(...)     LOGMASKED(LOG_ROMBANK,     __VA_ARGS__)
#define LOGPALETTEBANK(...) LOGMASKED(LOG_PALETTEBANK, __VA_ARGS__)
#define LOGVRAMBANK(...)    LOGMASKED(LOG_VRAMBANK,    __VA_ARGS__)
#define LOGLAMPSA(...)      LOGMASKED(LOG_LAMPSA,      __VA_ARGS__)
#define LOGLAMPSB(...)      LOGMASKED(LOG_LAMPSB,      __VA_ARGS__)
#define LOGPULSE(...)       LOGMASKED(LOG_PULSE,       __VA_ARGS__)


namespace {

class majorpkr_state : public driver_device
{
public:
	majorpkr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_palette_bank(*this, "palette_bank"),
		m_vram_view(*this, "vram_view"),
		m_rom_bank(*this, "rom_bank"),
		m_fg_vram(*this, "fg_vram"),
		m_bg_vram(*this, "bg_vram"),
		m_paletteram(*this, "paletteram", 0x2000, ENDIANNESS_LITTLE),
		m_dsw(*this, "DSW%u", 1U),
		m_input(*this, "IN0-%u", 0U),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void majorpkr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_memory_bank m_palette_bank;
	memory_view m_vram_view;
	required_memory_bank m_rom_bank;
	required_shared_ptr<uint8_t> m_fg_vram;
	required_shared_ptr<uint8_t> m_bg_vram;
	memory_share_creator<uint8_t> m_paletteram;

	required_ioport_array<4> m_dsw;
	required_ioport_array<2> m_input;
	output_finder<13> m_lamps;

	tilemap_t *m_bg_tilemap, *m_fg_tilemap;

	uint8_t m_mux_data = 0;

	void rom_bank_w(uint8_t data);
	void palette_bank_w(uint8_t data);
	void vram_bank_w(uint8_t data);
	void fg_vram_w(offs_t offset, uint8_t data);
	void bg_vram_w(offs_t offset, uint8_t data);
	void vidreg_w(uint8_t data);
	uint8_t mux_port_r();
	uint8_t mux_port2_r();
	void mux_sel_w(uint8_t data);
	void lamps_a_w(uint8_t data);
	void lamps_b_w(uint8_t data);
	void pulses_w(uint8_t data);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void map(address_map &map) ATTR_COLD;
	void portmap(address_map &map) ATTR_COLD;
};


/*************************
*     Video Hardware     *
*************************/

TILE_GET_INFO_MEMBER(majorpkr_state::bg_get_tile_info)
{
	int const code = m_bg_vram[2 * tile_index] + (m_bg_vram[2 * tile_index + 1] << 8);

	tileinfo.set(0,
			(code & 0x1fff),
			code >> 13,
			0);
}

TILE_GET_INFO_MEMBER(majorpkr_state::fg_get_tile_info)
{
	int const code = m_fg_vram[2 * tile_index] + (m_fg_vram[2 * tile_index + 1] << 8);

	tileinfo.set(1,
			(code & 0x07ff),
			code >> 13,
			(code & (1 << 12)) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}


void majorpkr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(majorpkr_state::bg_get_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 36, 28);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(majorpkr_state::fg_get_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 36, 28);
	m_fg_tilemap->set_transparent_pen(0);

	m_palette->basemem().set(m_paletteram, 0x2000, 16, ENDIANNESS_LITTLE, 2);
	m_palette_bank->configure_entries(0, 4, m_paletteram, 0x800);
}


uint32_t majorpkr_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0);

	return 0;
}


/******************************
*         R/W Handlers        *
******************************/

void majorpkr_state::rom_bank_w(uint8_t data)
{
	m_rom_bank->set_entry(data & 0x3);
	if (data & (0x3 ^ 0xff))
		LOGROMBANK("%s: accessing rom bank %02X\n", machine().describe_context(), data);
}

void majorpkr_state::palette_bank_w(uint8_t data)
{
	m_palette_bank->set_entry(data & 0x3);
	if (data & (0x3 ^ 0xff))
		LOGPALETTEBANK("%s: accessing palette bank %02X\n", machine().describe_context(), data);
}

void majorpkr_state::vram_bank_w(uint8_t data)
{
	m_vram_view.select(data & 0x3);
	if (data & (0x3 ^ 0xff))
		LOGVRAMBANK("%s: accessing vram bank %02X\n", machine().describe_context(), data);
}

void majorpkr_state::fg_vram_w(offs_t offset, uint8_t data)
{
	m_fg_vram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

void majorpkr_state::bg_vram_w(offs_t offset, uint8_t data)
{
	m_bg_vram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}

void majorpkr_state::vidreg_w(uint8_t data)
{
/*  If bit6 is active, the screen is drawn upside down.
    (also 0xfc and 0x11 are written to the CRTC registers 0x0c and 0x0d)
    So, the CRTC display start address = 0xfc11
*/
	if (data & 0x40)
	{
		// upside down screen
		m_bg_tilemap->set_flip(TILEMAP_FLIPX | TILEMAP_FLIPY);
		m_fg_tilemap->set_flip(TILEMAP_FLIPX | TILEMAP_FLIPY);
	}

/*  If bit6 is not active, the screen is drawn normally.
    (also 0x00 is written to the CRTC registers 0xc0 and 0xd0)
    So, the CRTC display start address = 0x0000
*/
	else
	{
		// normal screen
		m_bg_tilemap->set_flip(0);
		m_fg_tilemap->set_flip(0);
	}
}


/***** Multiplexed Ports *****/

uint8_t majorpkr_state::mux_port_r()
{
	switch (m_mux_data & 0xf0)       // 00-10-20-30-0F-1F-2F-3F.
	{
		case 0x00: return m_dsw[0]->read();   // confirmed.
		case 0x10: return m_dsw[1]->read();   // confirmed.
		case 0x20: return m_dsw[2]->read();   // confirmed.
		case 0x30: return m_dsw[3]->read();   // confirmed.
	}

	return 0xff;
}

uint8_t majorpkr_state::mux_port2_r()
{
	if ((m_mux_data & 0x0f) == 4)
	{
		return m_input[1]->read();
	}
	else
	{
		return m_input[0]->read();
	}
}

void majorpkr_state::mux_sel_w(uint8_t data)
{
	m_mux_data = data;  // 00-10-20-30-0F-1F-2F-3F.
}


/*************************
*    Lamps and Pulses    *
*************************/

void majorpkr_state::lamps_a_w(uint8_t data)
{
/*  Lamps - Array A.

    - bits -
    7654 3210
    ---- ---x   Hold 1 lamp.
    ---- --x-   Hold 2 lamp.
    ---- -x--   Hold 3 lamp.
    ---- x---   Hold 4 lamp.
    ---x ----   Hold 5 lamp.
    --x- ----   Big lamp.
    -x-- ----   Small lamp.
    x--- ----   Unknown.
*/
	m_lamps[0] = BIT(data, 0);  // Lamp 0: Hold 1.
	m_lamps[1] = BIT(data, 1);  // Lamp 1: Hold 2.
	m_lamps[2] = BIT(data, 2);  // Lamp 2: Hold 3.
	m_lamps[3] = BIT(data, 3);  // Lamp 3: Hold 4.
	m_lamps[4] = BIT(data, 4);  // Lamp 4: Hold 5.
	m_lamps[5] = BIT(data, 5);  // Lamp 5: Big or Small (need identification).
	m_lamps[6] = BIT(data, 6);  // Lamp 6: Big or Small (need identification).

	if (data & 0x80)
		LOGLAMPSA("Lamps A: Write to 13h: %02x\n", data);
}

void majorpkr_state::lamps_b_w(uint8_t data)
{
/*  Lamps - Array B.

    - bits -
    7654 3210
    ---- ---x   Bet lamp.
    ---- --x-   Draw lamp.
    ---- -x--   Cancel lamp.
    ---- x---   Take lamp.
    ---x ----   Double-Up lamp.
    --x- ----   Fever lamp.
    xx-- ----   Unknown.
*/
	m_lamps[7] = BIT(data, 0);   // Lamp 7: Bet.
	m_lamps[8] = BIT(data, 1);   // Lamp 8: Draw.
	m_lamps[9] = BIT(data, 2);   // Lamp 9: Cancel.
	m_lamps[10] = BIT(data, 3);  // Lamp 10: Take.
	m_lamps[11] = BIT(data, 4);  // Lamp 11: D-UP.
	m_lamps[12] = BIT(data, 5);  // Lamp 12: Fever.

	if (data & 0xc0)
		LOGLAMPSB("Lamps B: Write to 14h: %02x\n", data);
}

void majorpkr_state::pulses_w(uint8_t data)
{
/*  Pulses...

    - bits -
    7654 3210
    ---- ---x   Credits Out mech counter.
    ---- --x-   Credits 3 mech counter.
    ---- -x--   Credits 1 mech counter.
    ---- x---   Credits 2 mech counter.
    ---x ----   Watchdog? (constant writes).
    xxx- ----   Unknown.
*/
	machine().bookkeeping().coin_counter_w(3, data & 0x01);  // Credits Out (all).
	machine().bookkeeping().coin_counter_w(2, data & 0x02);  // Credits 3.
	machine().bookkeeping().coin_counter_w(0, data & 0x04);  // Credits 1.
	machine().bookkeeping().coin_counter_w(1, data & 0x08);  // Credits 2.

	if (data & 0xe0)
		LOGPULSE("Pulse: Write to 10h: %02x\n", data);
}

void majorpkr_state::machine_start()
{
	m_lamps.resolve();

	uint8_t *rom = memregion("maincpu")->base();
	m_rom_bank->configure_entries(0, 4, &rom[0xe000], 0x800);

	save_item(NAME(m_mux_data));
}

/*************************
* Memory map information *
*************************/

void majorpkr_state::map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xe7ff).bankr(m_rom_bank);
	map(0xe800, 0xefff).ram().share("nvram");
	map(0xf000, 0xf7ff).bankr(m_palette_bank).lw8(NAME([this] (offs_t offset, uint8_t data) { m_palette->write8(offset | (m_palette_bank->entry() << 11), data); }));
	map(0xf800, 0xffff).view(m_vram_view);
	m_vram_view[0](0xf800, 0xffff).ram().w(FUNC(majorpkr_state::fg_vram_w)).share(m_fg_vram);
	m_vram_view[1](0xf800, 0xffff).ram().w(FUNC(majorpkr_state::bg_vram_w)).share(m_bg_vram);
	m_vram_view[2](0xf800, 0xffff).ram().share("spare_ram0"); // spare vram? cleared during boot along with fg and bg
	m_vram_view[3](0xf800, 0xffff).ram().share("spare_ram1"); // "
}


/*
  00  W ---> ROM bank.
  01  W ---> RAM bank.
  02  W ---> RAM bank.

  10 R  ---> Regular inputs (holds) multiplexed with credits out.
  10  W ---> Writes a kind of watchdog (bit4) and mech counters pulses (bits 0-1-2-3).
  11 R  ---> Read input port (remaining controls).
  11  W ---> Writes mux selector.
  12 R  ---> Read coin/credits port.
  12  W ---> Video registers: Normal or Up Down screen (bit6 off/on). Also writes 0xFC & 0x11 at CRTC offsets 0x0C & 0x0D (0's if normal).
  13 R  ---> Multiplexed port for DIP switches banks.
  13  W ---> Lamps out (array a)
  14 R  ---> Freeze. Switch in bit0 that pause the game when active. Just once active, the code loops till the bit resets.
  14  W ---> Lamps out (array b).

  50 RW ---> OKI6295 (RW)

  60  W ---> PSG SN76489/96 initialization routines.
             (Maybe a leftover for different hardware).
*/
void majorpkr_state::portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(majorpkr_state::rom_bank_w));
	map(0x01, 0x01).w(FUNC(majorpkr_state::palette_bank_w));
	map(0x02, 0x02).w(FUNC(majorpkr_state::vram_bank_w));

	map(0x10, 0x10).r(FUNC(majorpkr_state::mux_port2_r));   // muxed set of controls.
	map(0x10, 0x10).w(FUNC(majorpkr_state::pulses_w));     // kind of watchdog on bit4... mech counters on bits 0-1-2-3.
	map(0x11, 0x11).portr("IN1");
	map(0x11, 0x11).w(FUNC(majorpkr_state::mux_sel_w));    // multiplexer selector.
	map(0x12, 0x12).portr("IN2");
	map(0x12, 0x12).w(FUNC(majorpkr_state::vidreg_w));     // video registers: normal or up down screen.
	map(0x13, 0x13).r(FUNC(majorpkr_state::mux_port_r));    // all 4 DIP switches banks multiplexed.
	map(0x13, 0x13).w(FUNC(majorpkr_state::lamps_a_w));    // lamps a out.
	map(0x14, 0x14).portr("TEST");   // "freeze" switch.
	map(0x14, 0x14).w(FUNC(majorpkr_state::lamps_b_w));    // lamps b out.

	map(0x30, 0x30).w("crtc", FUNC(mc6845_device::address_w));
	map(0x31, 0x31).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	map(0x50, 0x50).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x60, 0x60).nopw();    // leftover from a PSG SN76489/96?...
}


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( majorpkr )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )   // muxed with Key Out
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH )  PORT_NAME("Big")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW )   PORT_NAME("Small / DIP Test (In Book Mode)")

	PORT_START("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT ) // muxed with HOLD 2
	PORT_BIT( 0xfd, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE )       PORT_NAME("Book/Settings Mode")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_CODE(KEYCODE_1_PAD)  PORT_NAME("UNK 1-PAD")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_CODE(KEYCODE_2_PAD)  PORT_NAME("UNK 2-PAD")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYIN)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE )       PORT_CODE(KEYCODE_0)      PORT_NAME("SW2 (Clear Short-Term Meters)")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_CODE(KEYCODE_4_PAD)  PORT_NAME("UNK 4-PAD")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE )       PORT_CODE(KEYCODE_E)      PORT_NAME("Manual Payout")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_CODE(KEYCODE_5_PAD)  PORT_NAME("UNK 5-PAD")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_CODE(KEYCODE_6_PAD)  PORT_NAME("UNK 6-PAD")

	PORT_START("DSW1")  // multiplexed x4 & inverted
	PORT_DIPNAME( 0x01, 0x00, "OFF Fixed" )         PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Screen Direction" )  PORT_DIPLOCATION("DSW1:2")  // Activates bit6 at port 0x10 & change CRTC registers
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, "Up Down" )
	PORT_DIPNAME( 0x04, 0x00, "Payout" )            PORT_DIPLOCATION("DSW1:3")  // Hopper: No / Yes (in the manual)
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPSETTING(    0x04, "Manual" )
	PORT_DIPNAME( 0x08, 0x00, "Hopper SW Active" )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Low ) )
	PORT_DIPNAME( 0x10, 0x00, "Auto Max Bet" )      PORT_DIPLOCATION("DSW1:5")  // Coin Payout: Payout SW / Automatic (in the manual)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Hold Cancel" )       PORT_DIPLOCATION("DSW1:6")  // Inverse from the manual
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Auto Hold" )         PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Fever Mode" )        PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW2")  // multiplexed x4 & inverted
	PORT_DIPNAME( 0x07, 0x00, "Payout %" )                  PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "65%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x18, 0x00, "Min Bet Size for Fever" )    PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPNAME( 0x60, 0x00, "D-UP Game Difficulty" )      PORT_DIPLOCATION("DSW2:6,7")
	PORT_DIPSETTING(    0x00, "0 (Weak)" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x60, "3 (Strong)" )
	PORT_DIPNAME( 0x80, 0x00, "OFF Fixed" )                 PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")  // multiplexed x4 & inverted
	PORT_DIPNAME( 0x07, 0x00, "Key In" )                PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(    0x01, "5 Credits / Pulse" )
	PORT_DIPSETTING(    0x02, "10 Credits / Pulse" )
	PORT_DIPSETTING(    0x03, "20 Credits / Pulse" )
	PORT_DIPSETTING(    0x04, "25 Credits / Pulse" )
	PORT_DIPSETTING(    0x05, "40 Credits / Pulse" )
	PORT_DIPSETTING(    0x00, "50 Credits / Pulse" )
	PORT_DIPSETTING(    0x06, "60 Credits / Pulse" )
	PORT_DIPSETTING(    0x07, "100 Credits / Pulse" )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DSW3:4,5,6")
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x30, "1 Coin/40 Credits" )
	PORT_DIPSETTING(    0x38, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0xc0, 0x00, "Credit Limit" )          PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x40, "10000" )
	PORT_DIPSETTING(    0x80, "20000" )
	PORT_DIPSETTING(    0xc0, "30000" )

	PORT_START("DSW4")  // multiplexed x4 & inverted
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DSW4:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "20 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0e, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0d, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x30, 0x00, "Max Bet" )               PORT_DIPLOCATION("DSW4:5,6")
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPSETTING(    0x20, "30" )
	PORT_DIPSETTING(    0x30, "50" )
	PORT_DIPNAME( 0xc0, 0x00, "Credit-In Limit" )       PORT_DIPLOCATION("DSW4:7,8")
	PORT_DIPSETTING(    0x00, "No Limit" )
	PORT_DIPSETTING(    0x40, "1000" )
	PORT_DIPSETTING(    0x80, "2000" )
	PORT_DIPSETTING(    0xc0, "5000" )

	PORT_START("TEST")
	PORT_DIPNAME( 0x01, 0x00, "Freeze" )        // Freeze the execution
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout tilelayout =
{
	16, 8,
	RGN_FRAC(1,2),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{
		0*8, 0*8+RGN_FRAC(1,2), 1*8, 1*8+RGN_FRAC(1,2), 2*8, 2*8+RGN_FRAC(1,2), 3*8, 3*8+RGN_FRAC(1,2),
		4*8, 4*8+RGN_FRAC(1,2), 5*8, 5*8+RGN_FRAC(1,2), 6*8, 6*8+RGN_FRAC(1,2), 7*8, 7*8+RGN_FRAC(1,2)
	},
	{ 0*8*8, 1*8*8, 2*8*8, 3*8*8, 4*8*8, 5*8*8, 6*8*8, 7*8*8 },
	8*8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( gfx_majorpkr )
	GFXDECODE_ENTRY( "bg_gfx", 0, tilelayout, 8*256, 8 )
	GFXDECODE_ENTRY( "fg_gfx", 0, tilelayout, 0, 8 )
GFXDECODE_END


/*************************
*    Machine Drivers     *
*************************/

void majorpkr_state::majorpkr(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = 12_MHz_XTAL;
	static constexpr XTAL CPU_CLOCK = (MASTER_CLOCK / 2); // 6 MHz, measured.
	static constexpr XTAL OKI_CLOCK = (MASTER_CLOCK / 8); // 1.5 MHz, measured.
	static constexpr XTAL CRTC_CLOCK = (MASTER_CLOCK / 16); // 750 kHz, measured.

	// basic machine hardware
	z80_device &maincpu(Z80(config, "maincpu", CPU_CLOCK));  // 6 MHz.
	maincpu.set_addrmap(AS_PROGRAM, &majorpkr_state::map);
	maincpu.set_addrmap(AS_IO, &majorpkr_state::portmap);
	maincpu.set_vblank_int("screen", FUNC(majorpkr_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(CRTC_CLOCK*16, (47+1)*16, 0, (36*16)-16, (36+1)*8, 0, (28*8));  // from CRTC registers.
	screen.set_screen_update(FUNC(majorpkr_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_majorpkr);

	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 0x100 * 16);

	mc6845_device &crtc(MC6845(config, "crtc", CRTC_CLOCK));  // verified.
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_visarea_adjust(0, -16, 0, 0);
	crtc.set_char_width(16);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, "oki", OKI_CLOCK, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);  // clock frequency & pin 7 verified.
}


/*************************
*        Rom Load        *
*************************/

/*
  Major Poker.
  Original PAL System game.
*/
ROM_START( majorpkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6_pp_27c512_823b.bin", 0x00000, 0x10000, CRC(a3d5475e) SHA1(cb41508b55da8b8c658a2f2ccc6ebda09db29040) )

	ROM_REGION( 0x100000, "bg_gfx", 0 )
	ROM_LOAD( "1_27c040_7d3b.bin", 0x00000, 0x80000, CRC(67299eff) SHA1(34d3d8baf08dea495b699dd63272b445e2acb42d) )
	ROM_LOAD( "2_27c040_6039.bin", 0x80000, 0x80000, CRC(2d68b177) SHA1(01c934e0383991f2208b915cc5015463a8b6a8fd) )

	ROM_REGION( 0x40000, "fg_gfx", 0 )
	ROM_LOAD( "3_27c010_af18.bin", 0x00000, 0x20000, CRC(54452bb8) SHA1(9d13c17b85dd0185ba64fc6f90425e0c75363960) )
	ROM_LOAD( "4_27c010_92d6.bin", 0x20000, 0x20000, CRC(2e1e0972) SHA1(729dba2ef6ae8a7299c7ceb38835bebb0c42d28e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "5_27c020_8630.bin", 0x00000, 0x40000, CRC(4843858e) SHA1(27629829cf7753d7801a6eb42bb77ca2a467bebd) )

	ROM_REGION( 0x1000, "plds1", 0 )  // from protection box.
	ROM_LOAD( "u1_box_palce16v8h.bin",  0x0000, 0x0117, NO_DUMP )  // need to be extracted from the box and cracked...
	ROM_LOAD( "u2_box_palce16v8h.bin",  0x0200, 0x0117, NO_DUMP )  // need to be extracted from the box and cracked...
	ROM_LOAD( "u3_box_palce16v8h.bin",  0x0400, 0x0117, NO_DUMP )  // need to be extracted from the box and cracked...
	ROM_LOAD( "u4_box_palce16v8h.bin",  0x0600, 0x0117, NO_DUMP )  // need to be extracted from the box and cracked...
	ROM_LOAD( "u5_box_palce20v8h.bin",  0x0800, 0x0157, NO_DUMP )  // need to be extracted from the box and cracked...

	ROM_REGION( 0x2000, "plds2", 0 )  // from PCB
	ROM_LOAD( "g1_gal16v8d.bin",  0x0000, 0x0117, CRC(5ec2527a) SHA1(af9832a75efc25578ca79a08fae4bb169d4eb5ec) )
	ROM_LOAD( "g2_gal16v8d.bin",  0x0200, 0x0117, CRC(f6a04079) SHA1(fd9e7fac2867de9746138e5aa22fdac10c370d65) )  // protected, but cracked...
	ROM_LOAD( "g3_gal16v8d.bin",  0x0400, 0x0117, CRC(8b36df82) SHA1(b629557a8ebc88edd9e13372906f393f9fbc0669) )
	ROM_LOAD( "g4_gal16v8d.bin",  0x0600, 0x0117, CRC(8b36df82) SHA1(b629557a8ebc88edd9e13372906f393f9fbc0669) )
	ROM_LOAD( "g5_gal16v8d.bin",  0x0800, 0x0117, CRC(8b36df82) SHA1(b629557a8ebc88edd9e13372906f393f9fbc0669) )
	ROM_LOAD( "g6_gal16v8d.bin",  0x0a00, 0x0117, CRC(5ec2527a) SHA1(af9832a75efc25578ca79a08fae4bb169d4eb5ec) )
	ROM_LOAD( "g7_gal20v8b.bin",  0x0c00, 0x0157, CRC(9f45d431) SHA1(c3c9e6ed25a7cd7536974b906c993f5d7a58e65d) )
	ROM_LOAD( "g8_gal20v8b.bin",  0x0e00, 0x0157, CRC(9f45d431) SHA1(c3c9e6ed25a7cd7536974b906c993f5d7a58e65d) )
	ROM_LOAD( "g9_gal16v8d.bin",  0x1000, 0x0117, CRC(5ec2527a) SHA1(af9832a75efc25578ca79a08fae4bb169d4eb5ec) )
	ROM_LOAD( "g10_gal16v8d.bin", 0x1200, 0x0117, CRC(5bdfd9f3) SHA1(5bca47c1fa4b1a6b7d1041a12f98153fc1b23065) )
ROM_END


/*
  Major Poker.
  Micro Manufacturing intro.
  Program is totally different.
  Graphics ROMs are identical to the parent set.
*/
ROM_START( majorpkra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512__a.rom6", 0x00000, 0x10000, CRC(0213a933) SHA1(0c3238f037bcbe096c85b5c57ac735d707361f87) )

	ROM_REGION( 0x100000, "bg_gfx", 0 )
	ROM_LOAD( "27c040.rom1", 0x00000, 0x80000, CRC(67299eff) SHA1(34d3d8baf08dea495b699dd63272b445e2acb42d) )
	ROM_LOAD( "27c040.rom2", 0x80000, 0x80000, CRC(2d68b177) SHA1(01c934e0383991f2208b915cc5015463a8b6a8fd) )

	ROM_REGION( 0x40000, "fg_gfx", 0 )
	ROM_LOAD( "27c1001.rom3", 0x00000, 0x20000, CRC(54452bb8) SHA1(9d13c17b85dd0185ba64fc6f90425e0c75363960) )
	ROM_LOAD( "27c1001.rom4", 0x20000, 0x20000, CRC(2e1e0972) SHA1(729dba2ef6ae8a7299c7ceb38835bebb0c42d28e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "27c2001.rom5", 0x00000, 0x40000, CRC(4843858e) SHA1(27629829cf7753d7801a6eb42bb77ca2a467bebd) )
ROM_END

/*
  Major Poker.
  Micro Manufacturing intro.
  Graphics ROMs are identical to the parent set.

  Only one byte of difference against set C.
  Offset 0x38a7 = 0x08 (instead of 0x10).
*/
ROM_START( majorpkrb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512__b.rom6", 0x00000, 0x10000, CRC(3ab1e2c2) SHA1(11339fe32bb372f01b0983d2d571440530353b2a) )

	ROM_REGION( 0x100000, "bg_gfx", 0 )
	ROM_LOAD( "27c040.rom1", 0x00000, 0x80000, CRC(67299eff) SHA1(34d3d8baf08dea495b699dd63272b445e2acb42d) )
	ROM_LOAD( "27c040.rom2", 0x80000, 0x80000, CRC(2d68b177) SHA1(01c934e0383991f2208b915cc5015463a8b6a8fd) )

	ROM_REGION( 0x40000, "fg_gfx", 0 )
	ROM_LOAD( "27c1001.rom3", 0x00000, 0x20000, CRC(54452bb8) SHA1(9d13c17b85dd0185ba64fc6f90425e0c75363960) )
	ROM_LOAD( "27c1001.rom4", 0x20000, 0x20000, CRC(2e1e0972) SHA1(729dba2ef6ae8a7299c7ceb38835bebb0c42d28e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "27c2001.rom5", 0x00000, 0x40000, CRC(4843858e) SHA1(27629829cf7753d7801a6eb42bb77ca2a467bebd) )
ROM_END

/*
  Major Poker.
  Micro Manufacturing intro.
  Graphics ROMs are identical to the parent set.

  Only one byte of difference against set B.
  Offset 0x38a7 = 0x10 (instead of 0x08).
*/
ROM_START( majorpkrc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512__c.rom6", 0x00000, 0x10000, CRC(7379026b) SHA1(49e4f935ba3d27c70df351e3e61fb94e00f1536e) )

	ROM_REGION( 0x100000, "bg_gfx", 0 )
	ROM_LOAD( "27c040.rom1", 0x00000, 0x80000, CRC(67299eff) SHA1(34d3d8baf08dea495b699dd63272b445e2acb42d) )
	ROM_LOAD( "27c040.rom2", 0x80000, 0x80000, CRC(2d68b177) SHA1(01c934e0383991f2208b915cc5015463a8b6a8fd) )

	ROM_REGION( 0x40000, "fg_gfx", 0 )
	ROM_LOAD( "27c1001.rom3", 0x00000, 0x20000, CRC(54452bb8) SHA1(9d13c17b85dd0185ba64fc6f90425e0c75363960) )
	ROM_LOAD( "27c1001.rom4", 0x20000, 0x20000, CRC(2e1e0972) SHA1(729dba2ef6ae8a7299c7ceb38835bebb0c42d28e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "27c2001.rom5", 0x00000, 0x40000, CRC(4843858e) SHA1(27629829cf7753d7801a6eb42bb77ca2a467bebd) )
ROM_END

/*
  Lucky Poker.
  Looks like a bootleg/hack of Major Joker.
  Graphics ROMs are identical to the parent set.
*/
ROM_START( luckypkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512__luckypkr.rom6", 0x00000, 0x10000, CRC(e5e6d79d) SHA1(2c4b54d8cc9083cfa1508a73269baff923504778) )

	ROM_REGION( 0x100000, "bg_gfx", 0 )
	ROM_LOAD( "27c040.rom1", 0x00000, 0x80000, CRC(67299eff) SHA1(34d3d8baf08dea495b699dd63272b445e2acb42d) )
	ROM_LOAD( "27c040.rom2", 0x80000, 0x80000, CRC(2d68b177) SHA1(01c934e0383991f2208b915cc5015463a8b6a8fd) )

	ROM_REGION( 0x40000, "fg_gfx", 0 )
	ROM_LOAD( "27c1001.rom3", 0x00000, 0x20000, CRC(54452bb8) SHA1(9d13c17b85dd0185ba64fc6f90425e0c75363960) )
	ROM_LOAD( "27c1001.rom4", 0x20000, 0x20000, CRC(2e1e0972) SHA1(729dba2ef6ae8a7299c7ceb38835bebb0c42d28e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "27c2001.rom5", 0x00000, 0x40000, CRC(4843858e) SHA1(27629829cf7753d7801a6eb42bb77ca2a467bebd) )
ROM_END

/*
  슈퍼 윷놀이 (Playing Super Yut / Super Yutnori).
  Bootleg of Major Poker.
  Four locations for DIP switches banks on the PCB, but only three were populated (one was missing).
  Also, none of the switches on the PCB (SW1 and SW2) were present on their PCB locations.
  Hardware:
   -GoldStar Z8400B.
   -Actel PL84C.
   -AD65 (OKI6295).
   -12.000 MHz xtal.
*/
ROM_START( syutnori )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6.bin", 0x00000, 0x10000, CRC(a8e35b31) SHA1(887ded6bf2c9ce1b95336c870889f94eb380c57e) )

	ROM_REGION( 0x100000, "bg_gfx", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(82eabf1b) SHA1(6faae3c5a989ade5f5245a8475831dd5fb814147) )
	ROM_LOAD( "2.bin", 0x80000, 0x80000, CRC(0ccd3ad7) SHA1(1b338d621c86fa931a776e3a3293d3034cf65959) )

	ROM_REGION( 0x40000, "fg_gfx", 0 )
	ROM_LOAD( "3.bin", 0x00000, 0x20000, CRC(03619b00) SHA1(f13732ff0022d611ea46f2a23ceac99ca88c581b) )
	ROM_LOAD( "4.bin", 0x20000, 0x20000, CRC(c429786c) SHA1(39de9c6ac282fc3f19e469af216530e7e8998a4f) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "5.bin", 0x00000, 0x40000, CRC(4843858e) SHA1(27629829cf7753d7801a6eb42bb77ca2a467bebd) )

	ROM_REGION( 0x2000, "plds", 0 )
	ROM_LOAD( "1_atf16v8b.bin",    0x0000, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "2_atf16v8b.bin",    0x0200, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "3_atf16v8b.bin",    0x0400, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "4_atf16v8b.bin",    0x0600, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "5_atf16v8b.bin",    0x0800, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "6_atf16v8b.bin",    0x0a00, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "7_atf16v8b.bin",    0x0c00, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "8_atf16v8b.bin",    0x0e00, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "9_palce20v8h.bin",  0x1000, 0x0157, NO_DUMP ) // protected
	ROM_LOAD( "10_palce20v8h.bin", 0x1200, 0x0157, NO_DUMP ) // protected
ROM_END

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT     CLASS           INIT        ROT   COMPANY                             FULLNAME                                          FLAGS                  LAYOUT
GAMEL( 1994, majorpkr,  0,        majorpkr, majorpkr, majorpkr_state, empty_init, ROT0, "PAL System",                       "Major Poker (set 1, v2.0)",                      MACHINE_SUPPORTS_SAVE, layout_majorpkr )
GAMEL( 1994, majorpkra, majorpkr, majorpkr, majorpkr, majorpkr_state, empty_init, ROT0, "PAL System / Micro Manufacturing", "Major Poker (set 2, Micro Manufacturing intro)", MACHINE_SUPPORTS_SAVE, layout_majorpkr )
GAMEL( 1994, majorpkrb, majorpkr, majorpkr, majorpkr, majorpkr_state, empty_init, ROT0, "PAL System / Micro Manufacturing", "Major Poker (set 3, Micro Manufacturing intro)", MACHINE_SUPPORTS_SAVE, layout_majorpkr )
GAMEL( 1994, majorpkrc, majorpkr, majorpkr, majorpkr, majorpkr_state, empty_init, ROT0, "PAL System / Micro Manufacturing", "Major Poker (set 4, Micro Manufacturing intro)", MACHINE_SUPPORTS_SAVE, layout_majorpkr )
GAMEL( 1994, luckypkr,  majorpkr, majorpkr, majorpkr, majorpkr_state, empty_init, ROT0, "bootleg",                          "Lucky Poker (bootleg/hack of Major Poker)",      MACHINE_SUPPORTS_SAVE, layout_majorpkr )
GAMEL( 1994, syutnori,  majorpkr, majorpkr, majorpkr, majorpkr_state, empty_init, ROT0, "bootleg",                          "Super Yutnori (bootleg of Major Poker)",         MACHINE_SUPPORTS_SAVE, layout_majorpkr ) // 슈퍼 윷놀이
