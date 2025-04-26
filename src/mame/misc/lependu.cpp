// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo
/**********************************************************************************

  Le Pendu.
  Avenir Amusement Inc.

  Very rare French/English bilingual hangman style game.
  Sequel of "Le Super Pendu", with better graphics among other things...
  Runs in a heavily modified Bonanza Enterprises hardware.

  Driver by Roberto Fresca & Grull Osgo.


  Games supported:

  * Le Pendu (Bilingue, Version 04),                198?, Avenir Amusement Inc.
  * Code Magik (Ver 5.5) / Super 7 (stealth game),  198?, Voyageur de L'Espace Inc.


***********************************************************************************

  The "mini-boy" poker games made by Bonanza Enterprises were very popular
  in Quebec back in the early '80s. They were everywhere in small restaurants,
  bars and convenience stores.

  Popularity faded away when the slot machines (lucky 7's) were introduced.
  Many "miniboy" machines were converted to play lucky 7 slots. (A kit was
  available from a local company).

  All independent gambling devices were made illegal in 1993, when the government
  lottery company (lotto-Quebec) took over the video-lottery business. Thousands
  of machines were stored, destroyed or seized by the police.

  To keep them alive, some of these machines were converted to Le Pendu / Hangman.
  A conversion kit was proposed by a local company, developed in Quebec.
  The kit includes a in-house modification of the "Bonanza" poker board, plus a new
  5 buttons plate to replace the traditional 15 poker buttons, since the game has
  nothing to do with gambling (Hangman game, you need to guess a word before the
  character man is hanged).

  It was not a great success, so the game is very rare. On top of that, the designer
  added copy protection to his work, and the game cannot start when the battery
  drains. (part of the modification was to remove the battery memory on/off switch)

  There are some critical registers and rerouted subroutines in the battery backed
  RAM, hence the game dies once the battery drains.

  Here, you can see a video of the game working for refference:
  https://youtu.be/e3d8KyUVL_g


***********************************************************************************

  Le Pendu Bilingue (Version 04)
  ------------------------------

  Bonanza Enterpises PCB
  BB-03

  1x Rockwell R6502AP (R6502-15) CPU.

  2x Rockwell R6520P (R6520-11) PIAs.
  1x Hitachi HD46505SP CRTC.

  2x 2764 program ROMs

  1x 2764 GFX ROM.
  2x 2732 GFX ROMs.

  2x 27C256 French words / data ROMs.

  1x 8 DIP switches bank.
  1x 10 MHz Xtal.

  Discrete circuitry for sound.


***********************************************************************************

  Code Magik (Ver 5.5) / Super 7

  This is a very rare game that has a gambling game hidden inside in stealth mode.

  The front game is a very good Mastermind type game, where you must to guess
  a 4-numbers code to deactivate the bomb. The game will return how many numbers
  are correct, and how many in the right position. You have 20 attempts to beat
  the machine and win the game.

  Instructions (French/English/Spanish):

  - Entrez votre code en choisissant 4 chiffres de 0 a 9.
  - Vous avez 20 essaiz.
  - Reussissez a l'interieur de 5 essais et vous aurez una partie gratuite.
  - Abandonnez la partie en appuyant d'abord sur CARTE et ensuite sur MISE.

  - Enter your code by choosing 4 digits from 0 to 9.
  - You have 20 attempts.
  - Succeed within 5 tries and you will have a free game.
  - Give up the game by first pressing CARTE and then MISE.

  - Ingrese su código eligiendo 4 dígitos del 0 al 9.
  - Tienes 20 intentos.
  - Si lo logras en 5 intentos, tendrás un juego gratis.
  - Abandona el juego presionando primero CARTE y luego MISE.


  This is the very first arcade game of this type.

  The game has complex routines that allow a serial comm with an unknown external
  device that can transfer a whole NVRAM to the game, so I strongly think that
  the game could be configurable and switched remotely.

  Part of the code was injected in the NVRAM, so once the battery drains, the game
  will be inoperable, showing only the attract (title + explanation screen).

  After a big effort and days of reverse-engineering, we were capable to reconstruct
  the NVRAM structure and the hidden game switch.


**********************************************************************************/


#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "machine/nvram.h"
#include "sound/discrete.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "lependu.lh"
#include "codemagik.lh"


namespace {

#define MASTER_CLOCK    XTAL(10'000'000)
#define CPU_CLOCK       (MASTER_CLOCK/16)
#define PIXEL_CLOCK     (MASTER_CLOCK/2)


class lependu_state : public driver_device
{
public:
	lependu_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram"),
		m_pia(*this, "pia%u", 0U),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_discrete(*this, "discrete"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_bank(*this, "bank%u", 0U),
		m_input(*this, "IN.%u", 0U),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void lependu(machine_config &config);
	void codemagik(machine_config &config);

	void init_lependu();

	void lamps_w(uint8_t data);
	void lamps_cm_w(uint8_t data);

	void sound_w(uint8_t data);
	void mux_w(uint8_t data);

	uint32_t screen_update_lependu(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

	void lependu_videoram_w(offs_t offset, uint8_t data);
	void lependu_colorram_w(offs_t offset, uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_nvram;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<discrete_device> m_discrete;

private:
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void lependu_palette(palette_device &palette) const;
	void lependu_map(address_map &map) ATTR_COLD;

	uint8_t lependu_mux_port_r();
	void pia0_ca2_w(int state);

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_memory_bank_array<2> m_bank;
	required_ioport_array<4> m_input;
	output_finder<6> m_lamps;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_mux_data = 0xff;
};


/*********************************************
*               Video Hardware               *
*********************************************/

void lependu_state::lependu_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void lependu_state::lependu_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(lependu_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    --xx xx--   tiles color.
    -x-- --x-   tiles bank.
    x--- ---x   unused.
*/

	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index];
	int bank = (attr & 0x02) >> 1 | (attr & 0x40) >> 5;  // bits 6 and 1 switch the gfx banks
	int color = (attr & 0x3c) >> 2;                      // bits 2-3-4-5 for color

	tileinfo.set(bank, code, color, 0);
}


void lependu_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lependu_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t lependu_state::screen_update_lependu(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void lependu_state::lependu_palette(palette_device &palette) const
{
/*  prom bits
    7654 3210
    ---- ---x   red component.
    ---- --x-   green component.
    ---- -x--   blue component.
    ---- x---   intensity.
    xxxx ----   unused.
*/

	// 0000IBGR
	uint8_t const *const color_prom = memregion("proms")->base();
	if (!color_prom)
		return;

	for (int i = 0; i < palette.entries(); i++)
	{
		constexpr int intenmin = 0xe0;
		constexpr int intenmax = 0xff;    // 3.3 Volts (the whole range)

		// intensity component
		int const inten = BIT(color_prom[i], 3);

		// red component
		int const r = BIT(color_prom[i], 0) * (inten ? intenmax : intenmin);

		// green component
		int const g = BIT(color_prom[i], 1) * (inten ? intenmax : intenmin);

		// blue component
		int const b = BIT(color_prom[i], 2) * (inten ? intenmax : intenmin);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/*******************************************
*               R/W Handlers               *
*******************************************/

void lependu_state::mux_w(uint8_t data)
{
	m_bank[0]->set_entry(data & 0x03);
	m_bank[1]->set_entry(data & 0x03);
	m_mux_data = data;
}

uint8_t lependu_state::lependu_mux_port_r()
{
	uint8_t data = 0xff;

	for (int i = 0; i < 4 ; i++)
		if (BIT(~m_mux_data, i + 4))
			data &= m_input[i]->read();

	return data;
}


/*******************************************
*            Lamps and Outputs             *
*******************************************/

void lependu_state::lamps_w(uint8_t data)
{
/********** General Lamps wiring ***********

    7654 3210
    ---- ---x  Unused.
    ---- --x-  Unused.
    ---- -x--  Unused.
    ---- x---  Button 5.
    ---x ----  Button 2.
    --x- ----  Button 3.
    -x-- ----  Button 1.
    x--- ----  Button 4.

*/
	for (int i = 0; i < 5 ; i++)
		m_lamps[i] = BIT(~data, i + 3);
}

void lependu_state::lamps_cm_w(uint8_t data)
{
/********** General Lamps wiring ***********

    7654 3210
    ---- ---x  Unused.
    ---- --x-  Unused.
    ---- -x--  Unused.
    ---- x---  Button 5.
    ---x ----  Button 2.
    --x- ----  Button 3.
    -x-- ----  Button 1.
    x--- ----  Button 4.

*/
	for (int i = 0; i < 6 ; i++)
		m_lamps[i] = BIT(~data, i);

	// Select stealth game (Keyboard "L" toggle)
	m_nvram[0x0004] = m_input[3]->read() >> 7;
}


void lependu_state::sound_w(uint8_t data)
{
	// 555 voltage controlled
	logerror("Sound Data: %2x\n",data & 0x0f);

	// discrete sound is connected to PIA1, portA: bits 0-3
	m_discrete->write(NODE_01, data >> 3 & 0x01);
	m_discrete->write(NODE_10, data & 0x07);
}

void lependu_state::pia0_ca2_w(int state)
{
}

/*********************************************
*           Memory Map Information           *
*********************************************/

void lependu_state::lependu_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0bff).ram().w(FUNC(lependu_state::lependu_videoram_w)).share("videoram");
	map(0x0c00, 0x0fff).ram().w(FUNC(lependu_state::lependu_colorram_w)).share("colorram");
	map(0x10b0, 0x10b0).w("crtc", FUNC(mc6845_device::address_w));
	map(0x10b1, 0x10b1).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x10f4, 0x10f7).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x10f8, 0x10fb).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0x9fff).bankr("bank0");
	map(0xa000, 0xbfff).bankr("bank1");
	map(0xc000, 0xffff).rom();
}


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START(lependu)
	// Multiplexed - 4x5bits
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )     PORT_NAME("Stats / Meters")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Button 4 / Stats Input Test")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Button 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Button 5 / Stats Exit")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Button 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Button 3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN.2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )    PORT_NAME("Audit") // audit? (inside the game) to check...
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )       // 25c coin
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

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

	PORT_START("SW2")
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

static INPUT_PORTS_START(codemagik)
	// Multiplexed - 4x5bits
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Mise")            // mise/bet
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )  PORT_NAME("Service / Test")  // test
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Carte / Exit")    // carte/deal/exit
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("BUTTON 4")        // cancel
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("BUTTON 1")        // <--
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("BUTTON 2")  // -->
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("BUTTON 5")  // fin
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("BUTTON 3")  // choice
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN.2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )       // 25c coin
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Stealth mode / Switch Games") PORT_CODE(KEYCODE_L) PORT_TOGGLE
	PORT_BIT( 0x7b, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*********************************************
*              Graphics Layouts              *
*********************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	0x100,
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

/**************************************************
*           Graphics Decode Information           *
**************************************************/

static GFXDECODE_START( gfx_lependu )
//  banks ok
	GFXDECODE_ENTRY( "gfx1", 0,      tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0,      tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0800, tilelayout, 0, 16 )
GFXDECODE_END


/**********************************************************
*                 Discrete Sound Routines                 *
***********************************************************

    Pendu discrete sound circuitry.
    -------------------------------

           +12V                            .--------+---------------.  +5V
            |                              |        |               |   |
         .--+                              |        Z             +-------+
         |  |                             8|        Z 1K          | PC617 |
         Z  Z                         +----+----+   Z             +-------+
    330K Z  Z 47K                     |   VCC   |3  |   20K   1uF  |     |
         Z  Z             1.7uF       |        Q|---|--ZZZZZ--||---+     Z   1uF         6|\
         |  |            .-||-- GND   |         |7  |              |  1K Z<--||--+--------| \
         |  |   30K      |           4|      DIS|---+              Z     Z       |   LM380|  \  220uF
  PA0 ---|--|--ZZZZZ--.  |      .-----|R        |   |         2.2K Z     |       Z        |  8>--||----> Audio Out.
         |  |         |  |      |     |   555   |   Z              Z    -+-  10K Z     7+-|  /
         |  |   15K   |  |      |    5|         |   Z 10K          |    GND      Z     3+-| /-.1   .---> Audio Out.
  PA1 ---+--|--ZZZZZ--+--+------|-----|CV       |   Z              |             |     2+-|/  |    |
            |         |         |    2|         |6  |              |             |      |     |    |
            |  7.5K   |         |  .--|TR    THR|---+---||---------+-------------+      +-||--'    |
  PA2 ------+--ZZZZZ--'  |\     |  |  |   GND   |   |                            |      |          |
                         | \    |  |  +----+----+   |  .1uF                      |      | 10uF     |
                        9|  \   |  |      1|        |                           -+-     |          |
  PA3 -------------------|  8>--'  |      -+-       |                           GND     +----------'
                    4069 |  /      |      GND       |                                   |
                         | /       |                |                                  -+-
                         |/        '----------------'                                  GND
*/

static const discrete_555_desc lependu_555_vco_desc =
	{DISC_555_OUT_DC | DISC_555_OUT_ENERGY, 5, DEFAULT_555_VALUES};

static const discrete_dac_r1_ladder dac_lependu_ladder =
{
	3,                                  // size of ladder
	{RES_K(30), RES_K(15), RES_K(7.5)}, // elements

//  external vBias doesn't seems to be accurate.
//  using the 555 internal values sound better.

	5,                                  // voltage Bias resistor is tied to
	RES_K(5),                           // additional resistor tied to vBias
	RES_K(10),                          // resistor tied to ground

	CAP_U(4.7)                          // filtering cap tied to ground
};

static DISCRETE_SOUND_START( lependu_discrete )
/*
    - bits -
    76543210
    --------
    .....xxx --> sound data.
    ....x... --> enable/disable.

*/
	DISCRETE_INPUT_NOT   (NODE_01)      // bit 3: enable/disable
	DISCRETE_INPUT_DATA  (NODE_10)      // bits 0-2: sound data

	DISCRETE_DAC_R1(NODE_20, NODE_10, 5, &dac_lependu_ladder)

	DISCRETE_555_ASTABLE_CV(NODE_30, NODE_01, RES_K(1), RES_K(10), CAP_U(.1), NODE_20, &lependu_555_vco_desc)
	DISCRETE_OUTPUT(NODE_30, 3000)

DISCRETE_SOUND_END


/******************************************
*          Machine Start & Reset          *
******************************************/

void lependu_state::machine_start()
{
	m_lamps.resolve();

	uint8_t *ROM1 = memregion("data1")->base();
	uint8_t *ROM2 = memregion("data2")->base();
	m_bank[0]->configure_entries(0, 4, &ROM1[0], 0x2000);
	m_bank[1]->configure_entries(0, 4, &ROM2[0], 0x2000);
}

void lependu_state::machine_reset()
{
	m_bank[0]->set_entry(0);
	m_bank[1]->set_entry(0);
}


/*********************************************
*              Machine Drivers               *
*********************************************/

void lependu_state::lependu(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &lependu_state::lependu_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	PIA6821(config, m_pia[0]);
	m_pia[0]->readpa_handler().set(FUNC(lependu_state::lependu_mux_port_r));
	m_pia[0]->writepb_handler().set(FUNC(lependu_state::lamps_w));
	m_pia[0]->ca2_handler().set(FUNC(lependu_state::pia0_ca2_w));

	PIA6821(config, m_pia[1]);
	m_pia[1]->readpa_handler().set_ioport("SW1");
	m_pia[1]->readpb_handler().set_ioport("SW2");
	m_pia[1]->writepa_handler().set(FUNC(lependu_state::sound_w));
	m_pia[1]->writepb_handler().set(FUNC(lependu_state::mux_w)); // mux + bankswitch

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, (39 + 1) * 8, 0, 32 * 8, ((31 + 1) * 8) + 4, 0, 29 * 8); // from MC6845 parameters
	m_screen->set_screen_update(FUNC(lependu_state::screen_update_lependu));

	mc6845_device &crtc(HD6845S(config, "crtc", CPU_CLOCK)); // Hitachi HD46505SP
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lependu);
	PALETTE(config, m_palette, FUNC(lependu_state::lependu_palette), 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, lependu_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void lependu_state::codemagik(machine_config &config)
{
	lependu(config);

	// basic machine hardware
	PIA6821(config.replace(), m_pia[0]);
	m_pia[0]->readpa_handler().set(FUNC(lependu_state::lependu_mux_port_r));
	m_pia[0]->writepb_handler().set(FUNC(lependu_state::lamps_cm_w));
	m_pia[0]->ca2_handler().set(FUNC(lependu_state::pia0_ca2_w));
}


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( lependu )
	ROM_REGION( 0x10000, "maincpu", 0 ) // program ROMs
	ROM_LOAD( "000401.11a",  0xc000, 0x2000, CRC(8f6a5d0c) SHA1(a139796dce307bb50994edc20ea4ea3534fe79ff) )
	ROM_LOAD( "000402.10a",  0xe000, 0x2000, CRC(79fb6a44) SHA1(1d95c77c4169925530c4313cf37c3b6633911737) )

	ROM_REGION( 0x08000, "data1", 0 ) // banked data
	ROM_LOAD( "fra_04.12a",  0x0000, 0x8000, CRC(60b9a387) SHA1(b455f7e955869308dcc5709db34a478f08dc69bb) )

	ROM_REGION( 0x08000, "data2", 0 ) // banked data
	ROM_LOAD( "fra_05.14a",  0x0000, 0x8000, CRC(5834b8e3) SHA1(f4377b54761ae934ff2c09c2808424e2b4a44898) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(                0x0000, 0x4000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "1y.3a",       0x4000, 0x2000, CRC(ae0e37f8) SHA1(2e3404c55b92a7f9ec72d7b96bbea95ee028026c) )    // chars / multicolor tiles, bitplane 3

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "3y.1a",    0x0000, 0x1000, CRC(ea868221) SHA1(fcf9a840537feb28c9fb65b58b9a41b2412aa4ef) )    // cards deck and alt gfx, bitplane1
	ROM_LOAD( "2y.3a",    0x1000, 0x1000, CRC(6d1da4bb) SHA1(dc8c70faa301e2f7e9089d38e0ef618e8352e569) )    // cards deck gfx, bitplane2
	ROM_COPY( "gfx1",     0x4800, 0x2000, 0x0800 )    // cards deck gfx, bitplane3.
	ROM_COPY( "gfx1",     0x5800, 0x2800, 0x0800 )    // cards deck alt gfx, bitplane3.

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mini.5d",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*
  Code Magik / Super 7
  Voyageur de L'Espace Inc.

  Obscure stealth vintage game, using IRQ instead of NMI.

  Code Magik is a sort of Mastermind game that can be switched
  to a poker bonus game called Super 7, through a complex use of the hardware.

*/
ROM_START( codemagik )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2764.11a", 0xc000, 0x2000, CRC(61a6ffef) SHA1(d7f0f1b415b14d9919f9ac3b8a1d74f3e14ccc2d) )
	ROM_LOAD( "2764.10a", 0xe000, 0x2000, CRC(4aac24d5) SHA1(06c89052d0f6f9d435ff88be62706595fe716791) )

	ROM_REGION( 0x08000, "data1", 0 ) // banked data
	ROM_FILL(             0x0000, 0x8000, 0x0000 ) // filling the bank

	ROM_REGION( 0x08000, "data2", 0 ) // banked data
	ROM_FILL(             0x0000, 0x8000, 0x0000 ) // filling the bank

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(                0x0000, 0x4000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "1y_2764.4a",  0x4000, 0x2000, CRC(41b83c2d) SHA1(b46a43636c577c4a3620c69ef874f11aa9d48473) )    // chars / multicolor tiles, bitplane 3

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "3y_2732.1a", 0x0000, 0x1000, CRC(ea868221) SHA1(fcf9a840537feb28c9fb65b58b9a41b2412aa4ef) )    // cards deck and alt gfx, bitplane1
	ROM_LOAD( "2y_2732.3a", 0x1000, 0x1000, CRC(6d1da4bb) SHA1(dc8c70faa301e2f7e9089d38e0ef618e8352e569) )    // cards deck and alt gfx, bitplane2
	ROM_COPY( "gfx1",       0x4800, 0x2000, 0x0800 )    // cards deck gfx, bitplane3.
	ROM_COPY( "gfx1",       0x5800, 0x2800, 0x0800 )    // cards deck alt gfx, bitplane3.

	ROM_REGION( 0x0800, "nvram", 0 )  // default NVRAM, otherwise the game can't be played.
	ROM_LOAD( "codemagik_nvram.bin", 0x0000, 0x0800, CRC(55f269ca) SHA1(c55b9f1eff66811d122718bef2bb067d4f4b817a) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dex.5d",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END


/*********************************************
*                Driver Init                 *
*********************************************/

void lependu_state::init_lependu()
{
	uint8_t *ROM = memregion("maincpu")->base();

/*  break the loop at $cc53 that prevents to start a game

    $cc53 lda $01c4
    $cc56 bne $cc53
*/
	ROM[0xcc56] = 0xea;
	ROM[0xcc57] = 0xea;

	// patch to allow a second game...
	ROM[0xcc18] = 0x00;

	// fix checksum to avoid RAM clear
	ROM[0xdd79] = 0xb7;
}

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//     YEAR  NAME       PARENT    MACHINE    INPUT      STATE          INIT          ROT    COMPANY                      FULLNAME                                         FLAGS   LAYOUT
GAMEL( 198?, lependu,   0,        lependu,   lependu,   lependu_state, init_lependu, ROT0, "Avenir Amusement Inc.",     "Le Pendu (Bilingue, Version 04)",                0,      layout_lependu )
GAMEL( 198?, codemagik, 0,        codemagik, codemagik, lependu_state, empty_init,   ROT0, "Voyageur de L'Espace Inc.", "Code Magik (Ver 5.5) / Super 7 (stealth game)",  0,      layout_codemagik )
