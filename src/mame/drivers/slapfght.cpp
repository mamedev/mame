// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Stephane Humbert
/***************************************************************************

  Toaplan Slap Fight hardware

  Driver by K.Wilkins Jan 1998

  Games supported:
    Alcon / Slap Fight
    Guardian / Get Star
    Performan
    Tiger-Heli

  TODO:
  - proper MCU emulation (mame/machine/slapfght.cpp)
  - alcon cocktail/flipscreen, it doesn't write to the flipscreen reg


****************************************************************************

Main CPU Memory Map
-------------------

$0000-$3fff    ROM (SF_S19)
$4000-$7fff    ROM (SF_S20)
$8000-$bfff    ROM (SF_SH) - This is a 32K ROM - Paged ????? How ????

$c000-$c7ff    2K RAM
$c800-$cfff    READ:Unknown H/W  WRITE:Unknown H/W (Upper PCB)
$d000-$d7ff    Background RAM1
$d800-$dfff    Background RAM2
$e000-$e7ff    Sprite RAM
$e800-$efff    READ:Unknown H/W  WRITE:Unknown H/W
$f000-$f7ff    READ:SF_S16       WRITE:Character RAM
$f800-$ffff    READ:Unknown H/W  WRITE:Attribute RAM

$c800-$cfff    Appears to be RAM BUT 1st 0x10 bytes are swapped with
               the sound CPU and vice versa for READ OPERATIONS


Write I/O MAP
-------------
Addr    Address based write                     Data based write

$00     Reset sound CPU
$01     Clear sound CPU reset
$02
$03
$04
$05
$06     Clear/Disable Hardware interrupt
$07     Enable Hardware interrupt
$08     LOW Bank select for SF_SH               X axis character scroll reg
$09     HIGH Bank select for SF_SH              X axis pixel scroll reg
$0a
$0b
$0c     Select 1st set of sprites colors
$0d     Select 2nd set of sprites colors
$0e
$0f

Read I/O Map
------------

$00     Status register - cycle 0xc7, 0x55, 0x00  (Thanks to Dave Spicer for the info)


Known Info
----------

2K Character RAM at write only address $f000-$f7fff looks to be organised
64x32 chars with the screen rotated through 90 degrees clockwise. There
appears to be some kind of attribute(?) RAM above at $f800-$ffff organised
in the same manner.

From the look of data in the buffer it is arranged thus: 37x32 (HxW) which
would make the overall frame buffer 296x256.

Print function maybe around $09a2 based on info from log file.

$e000 looks like sprite ram, setup routines at $0008.


Sound System CPU Details
------------------------

Memory Map
$0000-$1fff  ROM(SF_S5)
$a080        AY-3-8910(PSG1) Register address
$a081        AY-3-8910(PSG1) Read register
$a082        AY-3-8910(PSG1) Write register
$a090        AY-3-8910(PSG2) Register address
$a091        AY-3-8910(PSG2) Read register
$a092        AY-3-8910(PSG2) Write register
$c800-$cfff  RAM(2K)

Strangely the RAM hardware registers seem to be overlaid at $c800
$00a6 routine here reads I/O ports and stores in, its not a straight
copy, the data is mangled before storage:
PSG1-E -> $c808
PSG1-F -> $c80b
PSG2-E -> $c809
PSG2-F -> $c80a - DIP Switch Bank 2 (Test mode is here)

-------------------------------GET STAR------------------------------------
        following info by Luca Elia (l.elia@tin.it)

                Interesting locations
                ---------------------

c803    credits
c806    used as a watchdog: main cpu reads then writes FF.
    If FF was read, jp 0000h. Sound cpu zeroes it.

c807(1p)    left    7           c809    DSW1(cpl'd)
c808(2p)    down    6           c80a    DSW2(cpl'd)
active_H    right   5           c80b    ip 1(cpl'd)
        up  4
        0   3
        0   2
        but2    1
        but1    0

c21d(main)  1p lives

Main cpu writes to unmapped ports 0e,0f,05,03 at startup.
Before playing, f1 is written to e802 and 00 to port 03.
If flip screen dsw is on, ff is written to e802 an 00 to port 02, instead.

                Interesting routines (main cpu)
                -------------------------------
4a3 wait A irq's
432 init the Ath sprite
569 reads a sequence from e803
607 prints the Ath string (FF terminated). String info is stored at
    65bc in the form of: attribute, dest. address, string address (5 bytes)
b73 checks lives. If zero, writes 0 to port 04 then jp 0000h.
    Before that, sets I to FF as a flag, for the startup ram check
    routine, to not alter the credit counter.
1523    put name in hi-scores?

-------------------------------Performan-----------------------------------
                 Interesting RAM locations (Main CPU).
                 -------------------------------------

$8056            Hero counter
$8057            Level counter
$8006 - $8035    High score table
$8609 - $860f    High score characters to display to screen for highest score


****************************************************************************

Stephh's notes (based on the games Z80 code and some tests) :

0) all games

  - All these games have a specific way of handling inputs :
      * when cabinet is set to "Upright", any player can use inputs
        from player 1 OR from player 2
      * when cabinet is set to "Cocktail", each player has its own inputs
    See Mantis bug ID 01746 attached files with Z80 explanations


1) 'tigerh' and clones

1a) 'tigerh'

  - US version
  - MCU dumped and emulated

1b) 'tigerhj'

  - Japan version
  - MCU dumped and emulated

1c) 'tigerhb1'

  - Bootleg based on 'tigerhj'
  - I can't tell if a specific MCU is required for this game :
      * MCU status doesn't seem to be required
        (see nop instructions at 0x10da, 0x10db, 0x10e5 and 0x10e6)
      * there is patched code at 0x0f9c which is called instead of
        standard call to 0x10cc, and there is no call to 0x0f88
      * there are writes to unknown ports 0x0e and 0x0f
      * hardware test is still computed at 0x109d but there is
        no delay to wait for correct value (0x83) to be returned
        (thus the need of specific read/write handlers)

1d) 'tigerhb2'

  - Bootleg based on 'tigerhj'
  - MCU reads/writes are patched.as well as ROM check

1e) 'tigerhb3'

  - Bootleg based on 'tigerhj'
  - MCU reads/writes are patched.
  - There is extra code at 0x10e2 (where MCU status was checked)
    that prevents player to get any extra lives (even by collecting
    10 diamonds) when Dip Switch is set to "20k then every 80k"
    (see Mantis bug ID 00652 with Z80 explanations).


2) 'grdian' and clones

2a) 'grdian'

  - US version, licensed to Kitkorp - name "Guardian".
  - MCU dumped and emulated.
  - Difficulty determines the number of energy bars you get.
  - Each hit removes 1 energy bar.
  - According to the manual, default difficulty shall be set to "Hard".
  - You can only get ONE extra life.
  - I don't know if it's an ingame bug, but you always have to enter
    your initials after a game ends, even if you score less than 10th place.

2b) 'getstarj'

  - Japan version - name "Get Star".
  - MCU dumped and emulated.
  - You always get 4 energy bars.
  - Difficulty determines how many energy bars you lose when you get hit.
  - I don't know what default difficulty shall be, so I set it to "Easy".
  - You can get multiple extra lives.

2c) 'getstarb1'

  - Bootleg based on 'getstarj'
  - MCU reads/writes are patched, but this hasn't been done in the "test mode".
  - The game seems to have its own protection on startup (check code at 0x6d1a).

2d) 'getstarb2'

  - Bootleg based on 'getstarj'
  - MCU reads/writes are patched, but this hasn't been done in the "test mode".
  - The game seems to have its own protection (check code at 0x0569 and 0x0ac6).
  - Patches are coded differently than in 'getstarb1' and code isn't always perfect
    (lazy coding ?) which causes LOTS of ingames bugs or strange behaviours :
      * patched command 0x20 : NO continue play
      * patched command 0x21 : as soon as a player loses all his lives,
        the game is over (even if other player still has lives)
      * patched command 0x22 : difficulty is ALWAYS set to "Easy" at start
        so "Difficulty" Dip Switches have no effect when you are playing
        (it still has an effect in the "test mode" though)
      * patched command 0x23 : different lives settings
        (thus the need of a specific INPUT_PORTS definition for this game)
      * patched command 0x25 : right and down directions are swapped
        (thus the need of a specific INPUT_PORTS definition for this game)
      * patched command 0x2a : player flag (which determines if player 1 or
        player 2 is playing) is NOT updated, causing the following things :
          . current player score will ALWAYS be displayed under player 1
          . when cabinet is set to "Cocktail", player 2 uses player 1 inputs
      * patched command 0x38 : laser position does NOT change according to
        player position (it ALWATS starts from the middle of the screen)

***************************************************************************/

#include "emu.h"
#include "includes/slapfght.h"

#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "machine/74259.h"
#include "sound/ay8910.h"
#include "speaker.h"


/***************************************************************************

  Main Z80 I/O and Memory Maps

***************************************************************************/

void slapfght_state::perfrman_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8fff).ram().share("share1");
	map(0x9000, 0x97ff).ram().w(FUNC(slapfght_state::videoram_w)).share("videoram");
	map(0x9800, 0x9fff).ram().w(FUNC(slapfght_state::colorram_w)).share("colorram");
	map(0xa000, 0xa7ff).ram().share("spriteram");
}


void slapfght_state::tigerh_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xcfff).ram().share("share1");
	map(0xd000, 0xd7ff).ram().w(FUNC(slapfght_state::videoram_w)).share("videoram");
	map(0xd800, 0xdfff).ram().w(FUNC(slapfght_state::colorram_w)).share("colorram");
	map(0xe000, 0xe7ff).ram().share("spriteram");
	map(0xe800, 0xe800).w(FUNC(slapfght_state::scrollx_lo_w));
	map(0xe801, 0xe801).w(FUNC(slapfght_state::scrollx_hi_w));
	map(0xe802, 0xe802).w(FUNC(slapfght_state::scrolly_w));
	map(0xf000, 0xf7ff).ram().w(FUNC(slapfght_state::fixram_w)).share("fixvideoram");
	map(0xf800, 0xffff).ram().w(FUNC(slapfght_state::fixcol_w)).share("fixcolorram");
}

void slapfght_state::tigerh_map_mcu(address_map &map)
{
	tigerh_map(map);
	map(0xe803, 0xe803).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
}

void slapfght_state::tigerhb1_map(address_map &map)
{
	tigerh_map(map);
	map(0xe803, 0xe803).rw(FUNC(slapfght_state::tigerhb1_prot_r), FUNC(slapfght_state::tigerhb1_prot_w));
}

void slapfght_state::tigerhb2_map(address_map &map)
{
	tigerh_map(map);
	map(0xe803, 0xe803).noprw(); // no MCU
}


void slapfght_state::slapfigh_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xcfff).ram().share("share1");
	map(0xd000, 0xd7ff).ram().w(FUNC(slapfght_state::videoram_w)).share("videoram");
	map(0xd800, 0xdfff).ram().w(FUNC(slapfght_state::colorram_w)).share("colorram");
	map(0xe000, 0xe7ff).ram().share("spriteram");
	map(0xe800, 0xe800).w(FUNC(slapfght_state::scrollx_lo_w));
	map(0xe801, 0xe801).w(FUNC(slapfght_state::scrollx_hi_w));
	map(0xe802, 0xe802).w(FUNC(slapfght_state::scrolly_w));
	map(0xf000, 0xf7ff).ram().w(FUNC(slapfght_state::fixram_w)).share("fixvideoram");
	map(0xf800, 0xffff).ram().w(FUNC(slapfght_state::fixcol_w)).share("fixcolorram");
}

void slapfght_state::slapfigh_map_mcu(address_map &map)
{
	slapfigh_map(map);
	map(0xe803, 0xe803).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
}

void slapfght_state::slapfighb1_map(address_map &map)
{
	slapfigh_map(map);
	map(0xe803, 0xe803).noprw(); // no MCU
}

void slapfght_state::getstar_map(address_map &map)
{
	slapfigh_map(map);
	map(0xe803, 0xe803).rw(FUNC(slapfght_state::getstar_mcusim_r), FUNC(slapfght_state::getstar_mcusim_w));
}

void slapfght_state::slapfighb2_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xcfff).ram().share("share1");
	map(0xd000, 0xd7ff).ram().w(FUNC(slapfght_state::videoram_w)).share("videoram");
	map(0xd800, 0xdfff).ram().w(FUNC(slapfght_state::colorram_w)).share("colorram");
	map(0xe000, 0xe7ff).ram().share("spriteram");
	map(0xe800, 0xe800).w(FUNC(slapfght_state::scrollx_hi_w));
	map(0xe802, 0xe802).w(FUNC(slapfght_state::scrolly_w));
	map(0xe803, 0xe803).w(FUNC(slapfght_state::scrollx_lo_w));
	map(0xec00, 0xefff).rom(); // it reads a copy of the logo from here!
	map(0xf000, 0xf7ff).ram().w(FUNC(slapfght_state::fixram_w)).share("fixvideoram");
	map(0xf800, 0xffff).ram().w(FUNC(slapfght_state::fixcol_w)).share("fixcolorram");
}


/**************************************************************************/

WRITE_LINE_MEMBER(slapfght_state::vblank_irq)
{
	if (state && m_main_irq_enabled)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}

WRITE_LINE_MEMBER(slapfght_state::irq_enable_w)
{
	m_main_irq_enabled = state ? true : false;

	if (!m_main_irq_enabled)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

WRITE_LINE_MEMBER(slapfght_state::sound_reset_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);

	if (state == 0)
		m_sound_nmi_enabled = false;
}

uint8_t slapfght_state::vblank_r()
{
	return m_screen->vblank() ? 1 : 0;
}

void slapfght_state::io_map_nomcu(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(slapfght_state::vblank_r));
	map(0x00, 0x0f).w("mainlatch", FUNC(ls259_device::write_a0));
}

void slapfght_state::io_map_mcu(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(slapfght_state::tigerh_mcu_status_r));
	map(0x00, 0x0f).w("mainlatch", FUNC(ls259_device::write_a0));
}

void slapfght_state::getstarb1_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(slapfght_state::getstarb1_prot_r));
	map(0x00, 0x0f).w("mainlatch", FUNC(ls259_device::write_a0));
}

void slapfght_state::getstarb2_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(slapfght_state::getstar_mcusim_status_r));
	map(0x00, 0x0f).w("mainlatch", FUNC(ls259_device::write_a0));
}



/***************************************************************************

  Sound Z80 I/O Memory Maps

***************************************************************************/

INTERRUPT_GEN_MEMBER(slapfght_state::sound_nmi)
{
	if (m_sound_nmi_enabled)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void slapfght_state::sound_nmi_enable_w(offs_t offset, uint8_t data)
{
	m_sound_nmi_enabled = offset ? false : true;
}

void slapfght_state::perfrman_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x8800, 0x8fff).ram().share("share1");
	map(0xa080, 0xa080).w("ay1", FUNC(ay8910_device::address_w));
	map(0xa081, 0xa081).r("ay1", FUNC(ay8910_device::data_r));
	map(0xa082, 0xa082).w("ay1", FUNC(ay8910_device::data_w));
	map(0xa090, 0xa090).w("ay2", FUNC(ay8910_device::address_w));
	map(0xa091, 0xa091).r("ay2", FUNC(ay8910_device::data_r));
	map(0xa092, 0xa092).w("ay2", FUNC(ay8910_device::data_w));
	map(0xa0e0, 0xa0e0).select(0x0010).w(FUNC(slapfght_state::sound_nmi_enable_w));
}

void slapfght_state::tigerh_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0xa080, 0xa080).w("ay1", FUNC(ay8910_device::address_w));
	map(0xa081, 0xa081).r("ay1", FUNC(ay8910_device::data_r));
	map(0xa082, 0xa082).w("ay1", FUNC(ay8910_device::data_w));
	map(0xa090, 0xa090).w("ay2", FUNC(ay8910_device::address_w));
	map(0xa091, 0xa091).r("ay2", FUNC(ay8910_device::data_r));
	map(0xa092, 0xa092).w("ay2", FUNC(ay8910_device::data_w));
	map(0xa0e0, 0xa0e0).select(0x0010).w(FUNC(slapfght_state::sound_nmi_enable_w));
	map(0xc800, 0xcfff).ram().share("share1");
	map(0xd000, 0xffff).ram();
}




/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( slapfght_generic )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:8,7,6")
//  PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x40, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:1" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	/* Filled by "Bonus Lives" settings */
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:1" )
INPUT_PORTS_END


static INPUT_PORTS_START( perfrman )
	PORT_INCLUDE(slapfght_generic)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x20, "Intermissions" )             PORT_DIPLOCATION("SW1:3")     /* "Screen Test" in the "test mode" */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xf0, 0x70, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4,3,2,1")
	PORT_DIPSETTING(    0xb0, "20k, 120k, then every 100k" )
	PORT_DIPSETTING(    0xa0, "40k, 120k, then every 100k" )
	PORT_DIPSETTING(    0x90, "60k, 160k, then every 100k" )
	PORT_DIPSETTING(    0x80, "Every 100k" )
	PORT_DIPSETTING(    0x70, "20k, 220k, then every 200k" )
	PORT_DIPSETTING(    0x60, "40k, 240k, then every 200k" )
	PORT_DIPSETTING(    0x50, "60k, 260k, then every 200k" )
	PORT_DIPSETTING(    0x40, "Every 200k" )
	PORT_DIPSETTING(    0x30, "20k, 320k, then every 300k" )
	PORT_DIPSETTING(    0x20, "40k, 340k, then every 300k" )
	PORT_DIPSETTING(    0x10, "60k, 360k, then every 300k" )
	PORT_DIPSETTING(    0x00, "Every 300k" )
	PORT_DIPSETTING(    0xf0, "20k only" )
	PORT_DIPSETTING(    0xe0, "40k only" )
	PORT_DIPSETTING(    0xd0, "60k only" )
	PORT_DIPSETTING(    0xc0, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( tigerh )
	PORT_INCLUDE(slapfght_generic)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Player Speed" )              PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Fast" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4") // see notes for 'tigerhb3'
	PORT_DIPSETTING(    0x10, "20k and every 80k" )
	PORT_DIPSETTING(    0x00, "50k and every 120k" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:1" )
INPUT_PORTS_END


static INPUT_PORTS_START( slapfigh )
	PORT_INCLUDE(slapfght_generic)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Screen Test" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE     PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )     PORT_DIPLOCATION("SW1:1")

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_SERVICE_DIPLOC(0x02, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x30, "30000 100000" )
	PORT_DIPSETTING(    0x10, "50000 200000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x00, "100000" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( getstar )
	PORT_INCLUDE(slapfght_generic)

	PORT_MODIFY("DSW2")
	/* harder default difficulty (confirmed by manual) */
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x30, "50k only" )
	PORT_DIPSETTING(    0x20, "100k only" )
	PORT_DIPSETTING(    0x10, "150k only" )
	PORT_DIPSETTING(    0x00, "200k only" )
INPUT_PORTS_END

static INPUT_PORTS_START( getstarj )
	PORT_INCLUDE(getstar)

	/* easier default difficulty (to be confirmed) and different bonus lives settings */
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, "30k, 130k, then every 100k" )
	PORT_DIPSETTING(    0x00, "50k, 200k, then every 150k" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:3" )
INPUT_PORTS_END

static INPUT_PORTS_START( getstarb2 )
	PORT_INCLUDE(getstarj)

	/* RIGHT and DOWN are swapped due to code at 0x0551 */
	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL

	/* different lives settings due to code at 0x0e36 */
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x03, "240 (Cheat)")
	/* difficulty settings have no effect due to code at 0x0e7c */
	/* I don't mark the Dips as "unused" because they are still read in the "test mode" */
INPUT_PORTS_END



/***************************************************************************

  Driver/Machine Inits

***************************************************************************/

void slapfght_state::machine_start()
{
	// zerofill
	m_palette_bank = 0;
	m_scrollx_lo = 0;
	m_scrollx_hi = 0;
	m_scrolly = 0;
	m_main_irq_enabled = false;
	m_sound_nmi_enabled = false;



	m_getstar_status = 0;
	m_getstar_sequence_index = 0;
	m_getstar_status_state = 0;
	m_getstar_cmd = 0;
	m_gs_a = 0;
	m_gs_d = 0;
	m_gs_e = 0;
	m_tigerhb_cmd = 0;

	// savestates
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_scrollx_lo));
	save_item(NAME(m_scrollx_hi));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_main_irq_enabled));
	save_item(NAME(m_sound_nmi_enabled));


	save_item(NAME(m_getstar_status));
	save_item(NAME(m_getstar_sequence_index));
	save_item(NAME(m_getstar_status_state));
	save_item(NAME(m_getstar_cmd));
	save_item(NAME(m_gs_a));
	save_item(NAME(m_gs_d));
	save_item(NAME(m_gs_e));
	save_item(NAME(m_tigerhb_cmd));
}

void slapfght_state::machine_reset()
{
	m_getstar_status = 0xc7;
	m_getstar_status_state = 0;
	m_getstar_sequence_index = 0;

	// reset sound
	m_sound_nmi_enabled = false;
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

/**************************************************************************/

void slapfght_state::init_banks()
{
	uint8_t *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &ROM[0x10000], 0x4000);

	membank("bank1")->set_entry(0);
}

void slapfght_state::init_slapfigh()
{
	init_banks();
}

void slapfght_state::init_getstarb1()
{
	m_getstar_id = GETSTARB1;
	init_banks();

	/* requires this or it gets stuck with 'rom test' on screen */
	/* it is possible the program roms are slightly corrupt like the gfx roms, or
	   that the bootleg simply shouldn't execute the code due to the modified roms */
	/* TODO: find & fix the cause of the following happening. */
	uint8_t *ROM = memregion("maincpu")->base();
	ROM[0x6d56] = 0xc3; // jp instead of jp z
}

void slapfght_state::init_getstarb2()
{
	m_getstar_id = GETSTARB2;
	init_banks();
}



/***************************************************************************

  GFX Decode, Chip Interfaces

***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,            /* 8*8 characters */
	RGN_FRAC(1,2),  /* 1024 characters */
	2,              /* 2 bits per pixel */
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	8,8,            /* 8*8 tiles */
	RGN_FRAC(1,4),  /* 2048/4096 tiles */
	4,              /* 4 bits per pixel */
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8    /* every tile takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,          /* 16*16 sprites */
	RGN_FRAC(1,4),  /* 512/1024 sprites */
	4,              /* 4 bits per pixel */
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8,
			9, 10 ,11, 12, 13, 14, 15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*8    /* every sprite takes 64 consecutive bytes */
};

static const gfx_layout perfrman_charlayout =
{
	8,8,            /* 8*8 characters */
	RGN_FRAC(1,3),  /* 1024 characters */
	3,              /* 3 bits per pixel */
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout perfrman_spritelayout =
{
	16,16,          /* 16*16 sprites */
	RGN_FRAC(1,3),  /* 256 sprites */
	3,              /* 3 bits per pixel */
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8,
			9, 10 ,11, 12, 13, 14, 15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*8
};


static GFXDECODE_START( gfx_perfrman )
	GFXDECODE_ENTRY( "gfx1", 0, perfrman_charlayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, perfrman_spritelayout, 128, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_slapfght )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0,  64 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,   0,  16 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0,  16 )
GFXDECODE_END



/***************************************************************************

  Machine Drivers

***************************************************************************/

void slapfght_state::perfrman(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(16'000'000)/4); // 4MHz? XTAL is known, divider is guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &slapfght_state::perfrman_map);
	m_maincpu->set_addrmap(AS_IO, &slapfght_state::io_map_nomcu);

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set(FUNC(slapfght_state::sound_reset_w));
	mainlatch.q_out_cb<1>().set(FUNC(slapfght_state::flipscreen_w));
	mainlatch.q_out_cb<3>().set(FUNC(slapfght_state::irq_enable_w));
	mainlatch.q_out_cb<6>().set(FUNC(slapfght_state::palette_bank_w));

	Z80(config, m_audiocpu, XTAL(16'000'000)/8); // 2MHz? XTAL is known, divider is guessed
	m_audiocpu->set_addrmap(AS_PROGRAM, &slapfght_state::perfrman_sound_map);
	m_audiocpu->set_periodic_int(FUNC(slapfght_state::sound_nmi), attotime::from_hz(240)); // music speed, verified

	config.set_perfect_quantum(m_maincpu);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(36_MHz_XTAL/6, 388, 0, 296, 270, 0, 240);
	m_screen->set_screen_update(FUNC(slapfght_state::screen_update_perfrman));
	m_screen->screen_vblank().append(FUNC(slapfght_state::vblank_irq));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_perfrman);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);
	MCFG_VIDEO_START_OVERRIDE(slapfght_state, perfrman)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", XTAL(16'000'000)/8));
	ay1.port_a_read_callback().set_ioport("IN0");
	ay1.port_b_read_callback().set_ioport("IN1");
	ay1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ay8910_device &ay2(AY8910(config, "ay2", XTAL(16'000'000)/8));
	ay2.port_a_read_callback().set_ioport("DSW1");
	ay2.port_b_read_callback().set_ioport("DSW2");
	ay2.add_route(ALL_OUTPUTS, "mono", 0.25);
}


void slapfght_state::tigerh(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 36_MHz_XTAL/6); // 6MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &slapfght_state::tigerh_map_mcu);
	m_maincpu->set_addrmap(AS_IO, &slapfght_state::io_map_mcu);

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set(FUNC(slapfght_state::sound_reset_w));
	mainlatch.q_out_cb<1>().set(FUNC(slapfght_state::flipscreen_w));
	mainlatch.q_out_cb<3>().set(FUNC(slapfght_state::irq_enable_w));

	Z80(config, m_audiocpu, 36_MHz_XTAL/12); // 3MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &slapfght_state::tigerh_sound_map);
	m_audiocpu->set_periodic_int(FUNC(slapfght_state::sound_nmi), attotime::from_hz(360)); // music speed, verified with pcb recording

	TAITO68705_MCU_TIGER(config, m_bmcu, 36_MHz_XTAL/12); // 3MHz

	config.set_perfect_quantum(m_maincpu);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(36_MHz_XTAL/6, 388, 0, 296, 270, 0, 240);
	m_screen->set_screen_update(FUNC(slapfght_state::screen_update_slapfight));
	m_screen->screen_vblank().append(FUNC(slapfght_state::vblank_irq));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_slapfght);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);
	MCFG_VIDEO_START_OVERRIDE(slapfght_state, slapfight)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", 36_MHz_XTAL/24)); // 1.5MHz
	ay1.port_a_read_callback().set_ioport("IN0");
	ay1.port_b_read_callback().set_ioport("IN1");
	ay1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ay8910_device &ay2(AY8910(config, "ay2", 36_MHz_XTAL/24)); // 1.5MHz
	ay2.port_a_read_callback().set_ioport("DSW1");
	ay2.port_b_read_callback().set_ioport("DSW2");
	ay2.add_route(ALL_OUTPUTS, "mono", 0.25);
}

void slapfght_state::tigerhb1(machine_config &config)
{
	tigerh(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &slapfght_state::tigerhb1_map);
	m_maincpu->set_addrmap(AS_IO, &slapfght_state::io_map_nomcu);

	config.device_remove("bmcu");
}

void slapfght_state::tigerhb2(machine_config &config)
{
	tigerhb1(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &slapfght_state::tigerhb2_map);
}

void slapfght_state::tigerhb4(machine_config &config)
{
	tigerhb2(config);

	// TODO: hook up
	m68705p_device &mcu(M68705P5(config, "mcu", 6000000)); // unverified clock
	mcu.porta_r().set_log("port A read");
	mcu.portb_r().set_log("port B read");
	mcu.portc_r().set_log("port C read");
	mcu.porta_w().set([this](uint8_t data) { logerror("port a write: %02x\n", data); });
	mcu.portb_w().set([this](uint8_t data) { logerror("port b write: %02x\n", data); });
	mcu.portc_w().set([this](uint8_t data) { logerror("port c write: %02x\n", data); });
}

void slapfght_state::slapfigh(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 36_MHz_XTAL/6); // 6MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &slapfght_state::slapfigh_map_mcu);
	m_maincpu->set_addrmap(AS_IO, &slapfght_state::io_map_mcu);

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set(FUNC(slapfght_state::sound_reset_w));
	mainlatch.q_out_cb<1>().set(FUNC(slapfght_state::flipscreen_w));
	mainlatch.q_out_cb<3>().set(FUNC(slapfght_state::irq_enable_w));
	mainlatch.q_out_cb<4>().set_membank("bank1");

	Z80(config, m_audiocpu, 36_MHz_XTAL/12); // 3MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &slapfght_state::tigerh_sound_map);
	m_audiocpu->set_periodic_int(FUNC(slapfght_state::sound_nmi), attotime::from_hz(180));

	TAITO68705_MCU(config, m_bmcu, 36_MHz_XTAL/12); // 3MHz
	m_bmcu->aux_strobe_cb().set(FUNC(slapfght_state::scroll_from_mcu_w));

	config.set_perfect_quantum(m_maincpu);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(36_MHz_XTAL/6, 388, 0, 296, 270, 0, 240);
	m_screen->set_screen_update(FUNC(slapfght_state::screen_update_slapfight));
	m_screen->screen_vblank().append(FUNC(slapfght_state::vblank_irq));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_slapfght);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);
	MCFG_VIDEO_START_OVERRIDE(slapfght_state, slapfight)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", 36_MHz_XTAL/24)); // 1.5MHz
	ay1.port_a_read_callback().set_ioport("IN0");
	ay1.port_b_read_callback().set_ioport("IN1");
	ay1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ay8910_device &ay2(AY8910(config, "ay2", 36_MHz_XTAL/24)); // 1.5MHz
	ay2.port_a_read_callback().set_ioport("DSW1");
	ay2.port_b_read_callback().set_ioport("DSW2");
	ay2.add_route(ALL_OUTPUTS, "mono", 0.25);
}

void slapfght_state::slapfighb1(machine_config &config)
{
	slapfigh(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &slapfght_state::slapfighb1_map);
	m_maincpu->set_addrmap(AS_IO, &slapfght_state::io_map_nomcu);

	config.device_remove("bmcu");
}

void slapfght_state::slapfighb2(machine_config &config)
{
	slapfighb1(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &slapfght_state::slapfighb2_map);
}

void slapfght_state::getstarb1(machine_config &config)
{
	slapfighb1(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &slapfght_state::getstarb1_io_map);
}

void slapfght_state::getstarb2(machine_config &config)
{
	slapfighb1(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &slapfght_state::getstar_map);
	m_maincpu->set_addrmap(AS_IO, &slapfght_state::getstarb2_io_map);
}



/***************************************************************************

  Game Drivers

***************************************************************************/

ROM_START( perfrman )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Main CPU code */
	ROM_LOAD( "ci07.0", 0x0000, 0x4000, CRC(7ad32eea) SHA1(e5b29793e9c8c5c9322ca2af468a9810a598c0ae) )
	ROM_LOAD( "ci08.1", 0x4000, 0x4000, CRC(90a02d5f) SHA1(9f2d2ce70a5bc96fc9d268e2b24533f73361225c) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU code */
	ROM_LOAD( "ci06.4", 0x0000, 0x2000, CRC(df891ad0) SHA1(0d33e7d0562831382f48d1588ef20a1bc73be71a) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "ci02.7", 0x0000, 0x2000, CRC(8efa960a) SHA1(d547ea23f2dd622500bf3f38cd9aca4e80aa27ca) )
	ROM_LOAD( "ci01.6", 0x2000, 0x2000, CRC(2e8e69df) SHA1(183c1868f0c94a2a82709f9c38020ee81c283051) )
	ROM_LOAD( "ci00.5", 0x4000, 0x2000, CRC(79e191f8) SHA1(3a755857dab147b73761aebfcf931dc3c87286a4) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "ci05.10", 0x0000, 0x2000, CRC(809a4ccc) SHA1(bca5a27abe205a65e1160d0c0c61e9831a949acc) )
	ROM_LOAD( "ci04.9",  0x2000, 0x2000, CRC(026f27b3) SHA1(a222d31368fa5117824f5a14a1e52f01326e1f63) )
	ROM_LOAD( "ci03.8",  0x4000, 0x2000, CRC(6410d9eb) SHA1(7e57de9255cbcacb4610cabb1364e2a4933ec12b) )

	ROM_REGION( 0x300, "proms", 0 ) /* Color BPROMs */
	ROM_LOAD( "ci14.16", 0x000, 0x0100, CRC(515f8a3b) SHA1(a99d4c119f6c4c6cd1b3fd208eadfb69ef7e8e2d) )
	ROM_LOAD( "ci13.15", 0x100, 0x0100, CRC(a9a397eb) SHA1(a84cf23efa0cf3e97b8dd1fff868c85d7eda1253) )
	ROM_LOAD( "ci12.14", 0x200, 0x0100, CRC(67f86e3d) SHA1(b1240212ea91cf451dbd7c6e2bfccbac76568cf6) )

	ROM_REGION( 0x220, "user1", 0 )
	ROM_LOAD( "ci11.11", 0x000, 0x0100, CRC(d492e6c2) SHA1(5789adda3a63ef8656ebd012416fcf3f991241fe) )
	ROM_LOAD( "ci10.12", 0x100, 0x0100, CRC(59490887) SHA1(c894edecbcfc67972ad893cd7c8197d07862a20a) )
	ROM_LOAD( "ci09.13", 0x200, 0x0020, CRC(aa0ca5a5) SHA1(4c45be71658f40ebb05634febba5822f1a8a7f79) )
ROM_END

ROM_START( perfrmanu )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Main CPU code */
	ROM_LOAD( "ci07.0",    0x0000, 0x4000, CRC(7ad32eea) SHA1(e5b29793e9c8c5c9322ca2af468a9810a598c0ae) )
	ROM_LOAD( "ci108r5.1", 0x4000, 0x4000, CRC(9d373efa) SHA1(b1d87e033ee3c50cfc56db05891b00b7bc236733) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU code */
	ROM_LOAD( "ci06.4", 0x0000, 0x2000, CRC(df891ad0) SHA1(0d33e7d0562831382f48d1588ef20a1bc73be71a) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "ci02.7", 0x0000, 0x2000, CRC(8efa960a) SHA1(d547ea23f2dd622500bf3f38cd9aca4e80aa27ca) )
	ROM_LOAD( "ci01.6", 0x2000, 0x2000, CRC(2e8e69df) SHA1(183c1868f0c94a2a82709f9c38020ee81c283051) )
	ROM_LOAD( "ci00.5", 0x4000, 0x2000, CRC(79e191f8) SHA1(3a755857dab147b73761aebfcf931dc3c87286a4) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "ci05.10", 0x0000, 0x2000, CRC(809a4ccc) SHA1(bca5a27abe205a65e1160d0c0c61e9831a949acc) )
	ROM_LOAD( "ci04.9",  0x2000, 0x2000, CRC(026f27b3) SHA1(a222d31368fa5117824f5a14a1e52f01326e1f63) )
	ROM_LOAD( "ci03.8",  0x4000, 0x2000, CRC(6410d9eb) SHA1(7e57de9255cbcacb4610cabb1364e2a4933ec12b) )

	ROM_REGION( 0x300, "proms", 0 ) /* Color BPROMs */
	ROM_LOAD( "ci14.16", 0x0000, 0x0100, CRC(515f8a3b) SHA1(a99d4c119f6c4c6cd1b3fd208eadfb69ef7e8e2d) )
	ROM_LOAD( "ci13.15", 0x0100, 0x0100, CRC(a9a397eb) SHA1(a84cf23efa0cf3e97b8dd1fff868c85d7eda1253) )
	ROM_LOAD( "ci12.14", 0x0200, 0x0100, CRC(67f86e3d) SHA1(b1240212ea91cf451dbd7c6e2bfccbac76568cf6) )

	ROM_REGION( 0x220, "user1", 0 )
	ROM_LOAD( "ci11.11",    0x0000, 0x0100, CRC(d492e6c2) SHA1(5789adda3a63ef8656ebd012416fcf3f991241fe) )
	ROM_LOAD( "ci10.12",    0x0100, 0x0100, CRC(59490887) SHA1(c894edecbcfc67972ad893cd7c8197d07862a20a) )
	ROM_LOAD( "ci09r1.13",  0x0200, 0x0020, CRC(d9e92f6f) SHA1(7dc2939267b7d2b1eeeca906cc6151fab2cf1cc4) )
ROM_END


/*
Tiger-Heli
1985 Taito Corporation

The Tiger-Heli PCB is almost exactly the same PCB as Slap Fight/Alcon but a few chips
have different placings and there's more EPROM locations on one of the boards
(although they're not populated).
Also, Tiger-Heli has a 22-way edge connector, while Alcon/Slap Fight is JAMMA.


PCB Layouts - Top Board
-----------------------

Tiger-Heli -   M6100111A
               850011401

GX-511-A MADE IN JAPAN
|--------------------------------------------------|
|VOL ROM21.12Q                 2148                |
|MB3712                        2148                |
|    ROM19.12P                 2148                |
|                              2148                |
|    ROM20.12M                         6264        |
|                                                  |
|                                      6264        |
|                                                  |
|2                 A47_13.8J A47_12.6J 6264        |
|2    AY3-8910                                     |
|W                 A47_11.8H A47_10.6H 6264        |
|A    DSW1  DSW2                                   |
|Y                                       2148      |
|     AY3-8910                                     |
|                                   PAL    ROM16.1E|
|                                                  |
|  A47_03.12D                              ROM17.1C|
|                                                  |
|                                     ROM18.2B     |
|  Z80A   2016                                     |
|                                                  |
|-----|----------|--------------|----------|-------|
      |----------|              |----------|
Notes:
      AY3-8910 clock - 1.500MHz (36/24)
      Z80A clock - 3.000MHz (36/12)
      VSync - 57Hz
      HSync - 15.02kHz
      A47_03 - 2764 EPROM
      A47_10 to A47_13 - 27128 EPROM
      ROM18 - 18S030 PROM
      ROM16, ROM17, ROM19, ROM20, ROM21 - 82S129 PROM


Bottom Board
------------

GX-511-B MADE IN JAPAN
|--------------------------------------------------|
|                        A47_14.6A                 |
|                                                  |
|    ROM14.2C                          ROM15.8B    |
|                                                  |
|                                                  |
|              2016      A47_05.6F                 |
|                                           36MHz  |
|              2016      A47_04.6G                 |
|                                                  |
|                                                  |
|                                                  |
|                                                  |
|                                                  |
|                                        A47_02.8K |
|                                                  |
|              A47_09.4M A47_08.6M       X         |
|     2016                                         |
|              2016      A47_07.6N       A47_01.8N |
|                                                  |
|              2016      A47_06.6P       A47_00.8P |
|                                           Z80B   |
|-----|----------|--------------|----------|-------|
      |----------|              |----------|
Notes:
      Z80B clock - 6.000MHz (36/6)
      ROM14, ROM15 - 82S129 PROM
      A47_00 to A47_02 - 27128 EPROM
      A47_06 to A47_09 - 27128 EPROM
      A47_04, A47_05 - 2764 EPROM
      A47_14 - Motorola DA68237 15-00011-001 (MC68705P3-compatible) Micro-Controller (Not protected). Clock 3.000MHz
      X - Unpopulated socket
*/

ROM_START( tigerh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a47_00-1.8p", 0x0000, 0x4000, CRC(4be73246) SHA1(a6f6a36fa7e3d269b87b777c0975b210d8b53483) ) /* NOTE: The use of the "-1" here does NOT refer to revision, but to region */
	ROM_LOAD( "a47_01-1.8n", 0x4000, 0x4000, CRC(aad04867) SHA1(5e9ff3c982afe104428e936ef417de2d238dc033) )
	ROM_LOAD( "a47_02-1.8k", 0x8000, 0x4000, CRC(4843f15c) SHA1(c0c145c9df9d6273171ac64fb7396e65a786f67c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a47_03.12d", 0x0000, 0x2000, CRC(d105260f) SHA1(f6a0e393e29354bb37fb723828f3267d030a45ea) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )
	ROM_LOAD( "a47_14.6a", 0x0000, 0x0800, CRC(4042489f) SHA1(b977e0821b6b1aa5a0a0f349cd78150af1a231df) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a47_05.6f", 0x0000, 0x2000, CRC(c5325b49) SHA1(6df9051e7545dcac4995340f80957510457aaf64) ) /* Chars */
	ROM_LOAD( "a47_04.6g", 0x2000, 0x2000, CRC(cd59628e) SHA1(7be6479f20eb51b79b93e6fd65ab219096d54984) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "a47_09.4m", 0x0000, 0x4000, CRC(31fae8a8) SHA1(ef8c23776431f00a74b25c5800755b6fa8d585ec) ) /* Tiles */
	ROM_LOAD( "a47_08.6m", 0x4000, 0x4000, CRC(e539af2b) SHA1(0c8369a0fac1cbe40c07b51e16e8f8a9b8ed03b8) )
	ROM_LOAD( "a47_07.6n", 0x8000, 0x4000, CRC(02fdd429) SHA1(fa392f2e57cfb6af4c124e0c151a4652f83e5577) )
	ROM_LOAD( "a47_06.6p", 0xc000, 0x4000, CRC(11fbcc8c) SHA1(b4fdb9ee00b749e1a54cfc0cdf55cc5e9bee3662) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "a47_13.8j", 0x0000, 0x4000, CRC(739a7e7e) SHA1(5fee71d9e1540903a6cf7bcaab30acaa088d35ed) ) /* Sprites */
	ROM_LOAD( "a47_12.6j", 0x4000, 0x4000, CRC(c064ecdb) SHA1(fa8d712e2b2bda78b9375d96c93a4d7549c94075) )
	ROM_LOAD( "a47_11.8h", 0x8000, 0x4000, CRC(744fae9b) SHA1(b324350469c51043e1d90ce58808d966467435b9) )
	ROM_LOAD( "a47_10.6h", 0xc000, 0x4000, CRC(e1cf844e) SHA1(eeb8eff09f96c693e147d155a8c0a87416d64603) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.12q", 0x0000, 0x0100, CRC(2c69350d) SHA1(658bf63c6d1e718f99494cd1c9346c3622913beb) )
	ROM_LOAD( "82s129.12m", 0x0100, 0x0100, CRC(7142e972) SHA1(4a854c2fdd006077aecb695832110ae6bf5819c1) )
	ROM_LOAD( "82s129.12n", 0x0200, 0x0100, CRC(25f273f2) SHA1(2c696745f42fa09b64295a39536aeba08ab58d67) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16r4a.2e", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( tigerhj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a47_00.8p", 0x0000, 0x4000, CRC(cbdbe3cc) SHA1(5badf76cdf4a7f0ae9e85ee602420ba5c128efef) )
	ROM_LOAD( "a47_01.8n", 0x4000, 0x4000, CRC(65df2152) SHA1(8e1516905a4af379cb0d0b9d42ff1cc3179c3589) )
	ROM_LOAD( "a47_02.8k", 0x8000, 0x4000, CRC(633d324b) SHA1(70a17d17ebe003bfb2246e92e925a343a92553e5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a47_03.12d", 0x0000, 0x2000, CRC(d105260f) SHA1(f6a0e393e29354bb37fb723828f3267d030a45ea) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )
	ROM_LOAD( "a47_14.6a", 0x0000, 0x0800, CRC(4042489f) SHA1(b977e0821b6b1aa5a0a0f349cd78150af1a231df) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a47_05.6f", 0x0000, 0x2000, CRC(c5325b49) SHA1(6df9051e7545dcac4995340f80957510457aaf64) ) /* Chars */
	ROM_LOAD( "a47_04.6g", 0x2000, 0x2000, CRC(cd59628e) SHA1(7be6479f20eb51b79b93e6fd65ab219096d54984) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "a47_09.4m", 0x0000, 0x4000, CRC(31fae8a8) SHA1(ef8c23776431f00a74b25c5800755b6fa8d585ec) ) /* Tiles */
	ROM_LOAD( "a47_08.6m", 0x4000, 0x4000, CRC(e539af2b) SHA1(0c8369a0fac1cbe40c07b51e16e8f8a9b8ed03b8) )
	ROM_LOAD( "a47_07.6n", 0x8000, 0x4000, CRC(02fdd429) SHA1(fa392f2e57cfb6af4c124e0c151a4652f83e5577) )
	ROM_LOAD( "a47_06.6p", 0xc000, 0x4000, CRC(11fbcc8c) SHA1(b4fdb9ee00b749e1a54cfc0cdf55cc5e9bee3662) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "a47_13.8j", 0x0000, 0x4000, CRC(739a7e7e) SHA1(5fee71d9e1540903a6cf7bcaab30acaa088d35ed) ) /* Sprites */
	ROM_LOAD( "a47_12.6j", 0x4000, 0x4000, CRC(c064ecdb) SHA1(fa8d712e2b2bda78b9375d96c93a4d7549c94075) )
	ROM_LOAD( "a47_11.8h", 0x8000, 0x4000, CRC(744fae9b) SHA1(b324350469c51043e1d90ce58808d966467435b9) )
	ROM_LOAD( "a47_10.6h", 0xc000, 0x4000, CRC(e1cf844e) SHA1(eeb8eff09f96c693e147d155a8c0a87416d64603) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.12q", 0x0000, 0x0100, CRC(2c69350d) SHA1(658bf63c6d1e718f99494cd1c9346c3622913beb) )
	ROM_LOAD( "82s129.12m", 0x0100, 0x0100, CRC(7142e972) SHA1(4a854c2fdd006077aecb695832110ae6bf5819c1) )
	ROM_LOAD( "82s129.12n", 0x0200, 0x0100, CRC(25f273f2) SHA1(2c696745f42fa09b64295a39536aeba08ab58d67) )
ROM_END

ROM_START( tigerhb1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b0.5",      0x00000, 0x4000, CRC(6ae7e13c) SHA1(47ef34635f8648e883a850293d92a46e95976a50) )
	ROM_LOAD( "a47_01.8n", 0x04000, 0x4000, CRC(65df2152) SHA1(8e1516905a4af379cb0d0b9d42ff1cc3179c3589) )
	ROM_LOAD( "a47_02.8k", 0x08000, 0x4000, CRC(633d324b) SHA1(70a17d17ebe003bfb2246e92e925a343a92553e5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a47_03.12d", 0x0000, 0x2000, CRC(d105260f) SHA1(f6a0e393e29354bb37fb723828f3267d030a45ea) )

	ROM_REGION( 0x0800, "cpu2", ROMREGION_ERASEFF ) // probably doesn't have an MCU
	/* is this the right mcu for this set? the mcu handling code in the roms seems patched and it doesn't work correctly */
//  ROM_LOAD( "a47_14.6a",   0x0000, 0x0800, CRC(4042489f) SHA1(b977e0821b6b1aa5a0a0f349cd78150af1a231df) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a47_05.6f", 0x00000, 0x2000, CRC(c5325b49) SHA1(6df9051e7545dcac4995340f80957510457aaf64) ) /* Chars */
	ROM_LOAD( "a47_04.6g", 0x02000, 0x2000, CRC(cd59628e) SHA1(7be6479f20eb51b79b93e6fd65ab219096d54984) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "a47_09.4m", 0x00000, 0x4000, CRC(31fae8a8) SHA1(ef8c23776431f00a74b25c5800755b6fa8d585ec) ) /* Tiles */
	ROM_LOAD( "a47_08.6m", 0x04000, 0x4000, CRC(e539af2b) SHA1(0c8369a0fac1cbe40c07b51e16e8f8a9b8ed03b8) )
	ROM_LOAD( "a47_07.6n", 0x08000, 0x4000, CRC(02fdd429) SHA1(fa392f2e57cfb6af4c124e0c151a4652f83e5577) )
	ROM_LOAD( "a47_06.6p", 0x0c000, 0x4000, CRC(11fbcc8c) SHA1(b4fdb9ee00b749e1a54cfc0cdf55cc5e9bee3662) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "a47_13.8j", 0x00000, 0x4000, CRC(739a7e7e) SHA1(5fee71d9e1540903a6cf7bcaab30acaa088d35ed) ) /* Sprites */
	ROM_LOAD( "a47_12.6j", 0x04000, 0x4000, CRC(c064ecdb) SHA1(fa8d712e2b2bda78b9375d96c93a4d7549c94075) )
	ROM_LOAD( "a47_11.8h", 0x08000, 0x4000, CRC(744fae9b) SHA1(b324350469c51043e1d90ce58808d966467435b9) )
	ROM_LOAD( "a47_10.6h", 0x0c000, 0x4000, CRC(e1cf844e) SHA1(eeb8eff09f96c693e147d155a8c0a87416d64603) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.12q", 0x0000, 0x0100, CRC(2c69350d) SHA1(658bf63c6d1e718f99494cd1c9346c3622913beb) )
	ROM_LOAD( "82s129.12m", 0x0100, 0x0100, CRC(7142e972) SHA1(4a854c2fdd006077aecb695832110ae6bf5819c1) )
	ROM_LOAD( "82s129.12n", 0x0200, 0x0100, CRC(25f273f2) SHA1(2c696745f42fa09b64295a39536aeba08ab58d67) )
ROM_END

ROM_START( tigerhb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom00_09", 0x00000, 0x4000, CRC(ef738c68) SHA1(c78c802d885b7f7c5e312ec079d52b8817590735) )
	ROM_LOAD( "rom01_08", 0x04000, 0x4000, CRC(65df2152) SHA1(8e1516905a4af379cb0d0b9d42ff1cc3179c3589) ) /* == a47_01.8n */
	ROM_LOAD( "rom02_07", 0x08000, 0x4000, CRC(36e250b9) SHA1(79bd86bde81981e4d0dbee420bc0a10c80b5241e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a47_03.12d", 0x0000, 0x2000, CRC(d105260f) SHA1(f6a0e393e29354bb37fb723828f3267d030a45ea) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a47_05.6f", 0x00000, 0x2000, CRC(c5325b49) SHA1(6df9051e7545dcac4995340f80957510457aaf64) ) /* Chars */
	ROM_LOAD( "a47_04.6g", 0x02000, 0x2000, CRC(cd59628e) SHA1(7be6479f20eb51b79b93e6fd65ab219096d54984) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "a47_09.4m", 0x00000, 0x4000, CRC(31fae8a8) SHA1(ef8c23776431f00a74b25c5800755b6fa8d585ec) ) /* Tiles */
	ROM_LOAD( "a47_08.6m", 0x04000, 0x4000, CRC(e539af2b) SHA1(0c8369a0fac1cbe40c07b51e16e8f8a9b8ed03b8) )
	ROM_LOAD( "a47_07.6n", 0x08000, 0x4000, CRC(02fdd429) SHA1(fa392f2e57cfb6af4c124e0c151a4652f83e5577) )
	ROM_LOAD( "a47_06.6p", 0x0c000, 0x4000, CRC(11fbcc8c) SHA1(b4fdb9ee00b749e1a54cfc0cdf55cc5e9bee3662) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "a47_13.8j", 0x00000, 0x4000, CRC(739a7e7e) SHA1(5fee71d9e1540903a6cf7bcaab30acaa088d35ed) ) /* Sprites */
	ROM_LOAD( "a47_12.6j", 0x04000, 0x4000, CRC(c064ecdb) SHA1(fa8d712e2b2bda78b9375d96c93a4d7549c94075) )
	ROM_LOAD( "a47_11.8h", 0x08000, 0x4000, CRC(744fae9b) SHA1(b324350469c51043e1d90ce58808d966467435b9) )
	ROM_LOAD( "a47_10.6h", 0x0c000, 0x4000, CRC(e1cf844e) SHA1(eeb8eff09f96c693e147d155a8c0a87416d64603) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.12q", 0x0000,  0x0100, CRC(2c69350d) SHA1(658bf63c6d1e718f99494cd1c9346c3622913beb) )
	ROM_LOAD( "82s129.12m", 0x0100,  0x0100, CRC(7142e972) SHA1(4a854c2fdd006077aecb695832110ae6bf5819c1) )
	ROM_LOAD( "82s129.12n", 0x0200,  0x0100, CRC(25f273f2) SHA1(2c696745f42fa09b64295a39536aeba08ab58d67) )
ROM_END

ROM_START( tigerhb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "14", 0x00000, 0x4000, CRC(ca59dd73) SHA1(c07961fcc209ec10ace3830d79c8ccc1cfda9765) )
	ROM_LOAD( "13", 0x04000, 0x4000, CRC(38bd54db) SHA1(75e999f606c410d7481bc4d29c4b523d45847649) )
	ROM_LOAD( "12", 0x08000, 0x4000, CRC(633d324b) SHA1(70a17d17ebe003bfb2246e92e925a343a92553e5) ) /* == a47_02.8k */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a47_03.12d", 0x0000, 0x2000, CRC(d105260f) SHA1(f6a0e393e29354bb37fb723828f3267d030a45ea) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a47_05.6f", 0x00000, 0x2000, CRC(c5325b49) SHA1(6df9051e7545dcac4995340f80957510457aaf64) ) /* Chars */
	ROM_LOAD( "a47_04.6g", 0x02000, 0x2000, CRC(cd59628e) SHA1(7be6479f20eb51b79b93e6fd65ab219096d54984) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "a47_09.4m", 0x00000, 0x4000, CRC(31fae8a8) SHA1(ef8c23776431f00a74b25c5800755b6fa8d585ec) ) /* Tiles */
	ROM_LOAD( "a47_08.6m", 0x04000, 0x4000, CRC(e539af2b) SHA1(0c8369a0fac1cbe40c07b51e16e8f8a9b8ed03b8) )
	ROM_LOAD( "a47_07.6n", 0x08000, 0x4000, CRC(02fdd429) SHA1(fa392f2e57cfb6af4c124e0c151a4652f83e5577) )
	ROM_LOAD( "a47_06.6p", 0x0c000, 0x4000, CRC(11fbcc8c) SHA1(b4fdb9ee00b749e1a54cfc0cdf55cc5e9bee3662) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "a47_13.8j", 0x00000, 0x4000, CRC(739a7e7e) SHA1(5fee71d9e1540903a6cf7bcaab30acaa088d35ed) ) /* Sprites */
	ROM_LOAD( "a47_12.6j", 0x04000, 0x4000, CRC(c064ecdb) SHA1(fa8d712e2b2bda78b9375d96c93a4d7549c94075) )
	ROM_LOAD( "a47_11.8h", 0x08000, 0x4000, CRC(744fae9b) SHA1(b324350469c51043e1d90ce58808d966467435b9) )
	ROM_LOAD( "a47_10.6h", 0x0c000, 0x4000, CRC(e1cf844e) SHA1(eeb8eff09f96c693e147d155a8c0a87416d64603) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.12q", 0x0000, 0x0100, CRC(2c69350d) SHA1(658bf63c6d1e718f99494cd1c9346c3622913beb) )
	ROM_LOAD( "82s129.12m", 0x0100, 0x0100, CRC(7142e972) SHA1(4a854c2fdd006077aecb695832110ae6bf5819c1) )
	ROM_LOAD( "82s129.12n", 0x0200, 0x0100, CRC(25f273f2) SHA1(2c696745f42fa09b64295a39536aeba08ab58d67) )
ROM_END

// this bootleg has a 68705P5S MCU
ROM_START( tigerhb4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0", 0x00000, 0x4000, CRC(ba5a31e0) SHA1(88c2cf07b36d8a2439fa04b5cc68fb5fe7683a30) )
	ROM_LOAD( "1", 0x04000, 0x4000, CRC(65df2152) SHA1(8e1516905a4af379cb0d0b9d42ff1cc3179c3589) )
	ROM_LOAD( "2", 0x08000, 0x4000, CRC(36e250b9) SHA1(79bd86bde81981e4d0dbee420bc0a10c80b5241e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a47_03.12d", 0x0000, 0x2000, CRC(d105260f) SHA1(f6a0e393e29354bb37fb723828f3267d030a45ea) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "68705p5s.bin", 0x0000, 0x0800, CRC(062eb9d7) SHA1(8337c24a63564c3698d04c723fcb6c067f7f5060) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a47_05.6f", 0x00000, 0x2000, CRC(c5325b49) SHA1(6df9051e7545dcac4995340f80957510457aaf64) ) /* Chars */
	ROM_LOAD( "a47_04.6g", 0x02000, 0x2000, CRC(cd59628e) SHA1(7be6479f20eb51b79b93e6fd65ab219096d54984) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "a47_09.4m", 0x00000, 0x4000, CRC(31fae8a8) SHA1(ef8c23776431f00a74b25c5800755b6fa8d585ec) ) /* Tiles */
	ROM_LOAD( "a47_08.6m", 0x04000, 0x4000, CRC(e539af2b) SHA1(0c8369a0fac1cbe40c07b51e16e8f8a9b8ed03b8) )
	ROM_LOAD( "a47_07.6n", 0x08000, 0x4000, CRC(02fdd429) SHA1(fa392f2e57cfb6af4c124e0c151a4652f83e5577) )
	ROM_LOAD( "a47_06.6p", 0x0c000, 0x4000, CRC(11fbcc8c) SHA1(b4fdb9ee00b749e1a54cfc0cdf55cc5e9bee3662) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "a47_13.8j", 0x00000, 0x4000, CRC(739a7e7e) SHA1(5fee71d9e1540903a6cf7bcaab30acaa088d35ed) ) /* Sprites */
	ROM_LOAD( "a47_12.6j", 0x04000, 0x4000, CRC(c064ecdb) SHA1(fa8d712e2b2bda78b9375d96c93a4d7549c94075) )
	ROM_LOAD( "a47_11.8h", 0x08000, 0x4000, CRC(744fae9b) SHA1(b324350469c51043e1d90ce58808d966467435b9) )
	ROM_LOAD( "a47_10.6h", 0x0c000, 0x4000, CRC(e1cf844e) SHA1(eeb8eff09f96c693e147d155a8c0a87416d64603) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.12q", 0x0000,  0x0100, CRC(2c69350d) SHA1(658bf63c6d1e718f99494cd1c9346c3622913beb) )
	ROM_LOAD( "82s129.12m", 0x0100,  0x0100, CRC(7142e972) SHA1(4a854c2fdd006077aecb695832110ae6bf5819c1) )
	ROM_LOAD( "82s129.12n", 0x0200,  0x0100, CRC(25f273f2) SHA1(2c696745f42fa09b64295a39536aeba08ab58d67) )
ROM_END

/*

Slap Fight/Alcon
1986 Taito Corporation

Slap Fight and Alcon are the same PCBs exactly, with just 4 ROMs changed. The same MCU is
common to Slap Fight and Alcon. There is an alternate "A76" version of Slap Fight with its
own unique MCU code.

PCB Layouts - Top Board
-----------------------

Alcon -        M6100186A
               860100714

Slap Fight -   M6100179A
               860090333


8606S MADE IN JAPAN
|--------------------------------------------------|
|VOL ROM21.12Q                 2148                |
|MB3730                        2148                |
|    ROM19.12P                 2148                |
|                              2148                |
|    ROM20.12M                         6264        |
|                                                  |
|                                      6264        |
|                                                  |
|J                 A77_12.8J A77_11.6J 6264        |
|A    AY3-8910                                     |
|M                 A77_10.8H A77_09.6H 6264        |
|M    DSW1  DSW2                                   |
|A                                       2148      |
|     AY3-8910                                     |
|                                          ROM16.1E|
|                                                  |
|  A77_02.12D                              ROM17.1C|
|                                                  |
|                                     ROM18.2B     |
|  Z80A   2016                                     |
|                                                  |
|-----|----------|--------------|----------|-------|
      |----------|              |----------|
Notes:
      AY-3-8910 clock - 1.500MHz (36/24)
      Z80A clock - 3.000MHz (36/12)
      VSync - 57Hz
      HSync - 14.97kHz
      A77_02 - 2764 EPROM
      A77_09 to A77_12 - 27256 EPROM
      ROM18 - 18S030 PROM
      ROM16, ROM17, ROM19, ROM20, ROM21 - 82S129 PROM


Bottom Board
------------

8606M MADE IN JAPAN
|--------------------------------------------------|
|                        A77_13.6A                 |
|                                                  |
|    ROM14.2C                          ROM15.8B    |
|                                                  |
|                                                  |
|              2016      A77_04.6F                 |
|                                           36MHz  |
|              2016      A77_03.6G                 |
|                                                  |
|                                                  |
|                                                  |
|                                                  |
|                                                  |
|                        A77_08.6K                 |
|                                                  |
|                        A77_07.6M         2016    |
|     2016                                         |
|              2016      A77_06.6N       A77_01.8N |
|                                                  |
|              2016      A77_05.6P       A77_00.8P |
|                                           Z80B   |
|-----|----------|--------------|----------|-------|
      |----------|              |----------|
Notes:
      Z80B clock - 6.000MHz (36/6)
      ROM14, ROM15 - 82S129 PROM
      A77_00, A77_01 - 27256 EPROM (replace with A77_00-1, A77_01-1 on Alcon)
      A77_05 to A77_08 - 27256 EPROM
      A77_03, A77_04 - 2764 EPROM (replace with A77_03-1, A77_04-1 on Alcon)
      A77_13 - Motorola MC68705P5S Micro-Controller (protected). Clock 3.000MHz

Non-color PROM usage (slap fight board names, seem common for all games):
  ROM14.2C:
    Horizontal signals.  In normal mode hpos goes from 0 to 387, in
    flipped mode hpos goes from 291 to -96.  The prom address is:
      ((hpos >> 2) & 0x7f) | (flip ? 0x80 : 0x00)

    Bits are:
      0: 1 = sync active
      1: 1 = blank active
      2: unused
      3: 0 = reset hpos

  ROM15.8B:
    Vertical signals.  Vpos goes from 0 to 269.  The prom address is
      vpos >> 1.

    Bits are:
      0: 0 = sync active
      1: 1 = blank active
      2: unused
      3: 0 = reset vpos

  ROM16.1E, ROM17.1C:
    Two nibbles providing a byte (1E high, 1C low).  Seems linked with
    reading the sprite ram, addressing is a little weird and usage not
    obvious.  Low 3 bits seem to choose a target (0=none,
    1,3=position, 4=code, 2=misc, 5+ unknown).

  ROM18.2B:
    Something to do with sprites, even less clear.
*/

ROM_START( alcon )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "a77_00-1.8p", 0x00000, 0x8000, CRC(2ba82d60) SHA1(b37659aa18a3f96a3cc7fa93db2439f36487b8c8) ) /* NOTE: The use of the "-1" here does NOT refer to revision, but to region */
	ROM_LOAD( "a77_01-1.8n", 0x10000, 0x8000, CRC(18bb2f12) SHA1(7c16d4bbb8b5e22f227aff170e5e6326c5968968) ) /* banked at 8000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a77_02.12d", 0x0000, 0x2000, CRC(87f4705a) SHA1(a90d5644ce268f3321047a4f96df96ac294d2f1b) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )
	ROM_LOAD( "a77_13.6a", 0x0000, 0x0800, CRC(a70c81d9) SHA1(f155ffd25a946b0459216a8f80ded16e6e2f9258) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a77_04-1.6f", 0x00000, 0x2000, CRC(31003483) SHA1(7014ceb6313ac5a3d2dcb735643dfd8bfabaa185) ) /* Chars */
	ROM_LOAD( "a77_03-1.6g", 0x02000, 0x2000, CRC(404152c0) SHA1(d05bc9baa1f336475fffc2f19f1018e9f0547f10) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "a77_08.6k", 0x00000, 0x8000, CRC(b6358305) SHA1(c7bb4236a75ec6b88f011bc30f8fb9a718e2ca3e) ) /* Tiles */
	ROM_LOAD( "a77_07.6m", 0x08000, 0x8000, CRC(e92d9d60) SHA1(2554617e0e6615ca8c85a49299a4a0e762478339) )
	ROM_LOAD( "a77_06.6n", 0x10000, 0x8000, CRC(5faeeea3) SHA1(696fba24bcf1f3a7e914a4403854da5eededaf7f) )
	ROM_LOAD( "a77_05.6p", 0x18000, 0x8000, CRC(974e2ea9) SHA1(3840550fc3a833828dad8f3e300d2ea583d69ce7) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "a77_12.8j", 0x00000, 0x8000, CRC(8545d397) SHA1(9a1fd5bfd8fb830b8e46643c08eef32ba968fc23) ) /* Sprites */
	ROM_LOAD( "a77_11.7j", 0x08000, 0x8000, CRC(b1b7b925) SHA1(199b0b52bbeb384211171eca5c50a1c0ebf6826f) )
	ROM_LOAD( "a77_10.8h", 0x10000, 0x8000, CRC(422d946b) SHA1(c251ef9597a11ec8de39be4fcbddaba84e649ef2) )
	ROM_LOAD( "a77_09.7h", 0x18000, 0x8000, CRC(587113ae) SHA1(90abe961494a1af7c87693a419fbabf7a58a5dee) )


	ROM_REGION( 0x0720, "proms", 0 )
	ROM_LOAD( "21_82s129.12q", 0x0000, 0x0100, CRC(a0efaf99) SHA1(5df01663480acad1f89abab8662d437617a66d1c) ) /* Silkscreened as ROM21 */
	ROM_LOAD( "20_82s129.12m", 0x0100, 0x0100, CRC(a56d57e5) SHA1(bfbd0db52b23fe1b4994e05103be3d412c1c013e) ) /* Silkscreened as ROM20 */
	ROM_LOAD( "19_82s129.12n", 0x0200, 0x0100, CRC(5cbf9fbf) SHA1(abfa58fa4e44ebc56f2e0fac9bcc36164c845fa3) ) /* Silkscreened as ROM19 */
	ROM_LOAD( "14_82s129.2c",  0x0300, 0x0100, CRC(4dc04453) SHA1(524e5a212ab496c52adc0def995b75e89aba48e7) ) /* Silkscreened as ROM14 */
	ROM_LOAD( "15_82s129.8b",  0x0400, 0x0100, CRC(48ac3db4) SHA1(6a73fe21529bc760b8df1893aee9e89fb60976eb) ) /* Silkscreened as ROM15 */
	ROM_LOAD( "16_82s129.1e",  0x0500, 0x0100, CRC(59490887) SHA1(c894edecbcfc67972ad893cd7c8197d07862a20a) ) /* Silkscreened as ROM16 */
	ROM_LOAD( "17_82s129.1c",  0x0600, 0x0100, CRC(d492e6c2) SHA1(5789adda3a63ef8656ebd012416fcf3f991241fe) ) /* Silkscreened as ROM17 */
	ROM_LOAD( "18_82s123.2b",  0x0700, 0x0020, CRC(aa0ca5a5) SHA1(4c45be71658f40ebb05634febba5822f1a8a7f79) ) /* Silkscreened as ROM18 */
ROM_END

ROM_START( slapfigh )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "a77_00.8p", 0x00000, 0x8000, CRC(674c0e0f) SHA1(69fc17881c89cc5e82b0fefec49c4116054f9e3b) )
	ROM_LOAD( "a77_01.8n", 0x10000, 0x8000, CRC(3c42e4a7) SHA1(8e4da1e6e73603e484ba4f5609ac9ea92999a526) ) /* banked at 8000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a77_02.12d", 0x0000, 0x2000, CRC(87f4705a) SHA1(a90d5644ce268f3321047a4f96df96ac294d2f1b) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a77_13.6a", 0x0000, 0x0800, CRC(a70c81d9) SHA1(f155ffd25a946b0459216a8f80ded16e6e2f9258) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a77_04.6f", 0x00000, 0x2000, CRC(2ac7b943) SHA1(d0c3560bb1f0c2647aeff807cb4b09450237b955) ) /* Chars */
	ROM_LOAD( "a77_03.6g", 0x02000, 0x2000, CRC(33cadc93) SHA1(59ffc206c62a651d2ac0ef52f519dd56edf2c021) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "a77_08.6k", 0x00000, 0x8000, CRC(b6358305) SHA1(c7bb4236a75ec6b88f011bc30f8fb9a718e2ca3e) ) /* Tiles */
	ROM_LOAD( "a77_07.6m", 0x08000, 0x8000, CRC(e92d9d60) SHA1(2554617e0e6615ca8c85a49299a4a0e762478339) )
	ROM_LOAD( "a77_06.6n", 0x10000, 0x8000, CRC(5faeeea3) SHA1(696fba24bcf1f3a7e914a4403854da5eededaf7f) )
	ROM_LOAD( "a77_05.6p", 0x18000, 0x8000, CRC(974e2ea9) SHA1(3840550fc3a833828dad8f3e300d2ea583d69ce7) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "a77_12.8j", 0x00000, 0x8000, CRC(8545d397) SHA1(9a1fd5bfd8fb830b8e46643c08eef32ba968fc23) ) /* Sprites */
	ROM_LOAD( "a77_11.7j", 0x08000, 0x8000, CRC(b1b7b925) SHA1(199b0b52bbeb384211171eca5c50a1c0ebf6826f) )
	ROM_LOAD( "a77_10.8h", 0x10000, 0x8000, CRC(422d946b) SHA1(c251ef9597a11ec8de39be4fcbddaba84e649ef2) )
	ROM_LOAD( "a77_09.7h", 0x18000, 0x8000, CRC(587113ae) SHA1(90abe961494a1af7c87693a419fbabf7a58a5dee) )

	ROM_REGION( 0x0720, "proms", 0 )
	ROM_LOAD( "21_82s129.12q", 0x0000, 0x0100, CRC(a0efaf99) SHA1(5df01663480acad1f89abab8662d437617a66d1c) ) /* Silkscreened as ROM21 */
	ROM_LOAD( "20_82s129.12m", 0x0100, 0x0100, CRC(a56d57e5) SHA1(bfbd0db52b23fe1b4994e05103be3d412c1c013e) ) /* Silkscreened as ROM20 */
	ROM_LOAD( "19_82s129.12n", 0x0200, 0x0100, CRC(5cbf9fbf) SHA1(abfa58fa4e44ebc56f2e0fac9bcc36164c845fa3) ) /* Silkscreened as ROM19 */
	ROM_LOAD( "14_82s129.2c",  0x0300, 0x0100, CRC(4dc04453) SHA1(524e5a212ab496c52adc0def995b75e89aba48e7) ) /* Silkscreened as ROM14 */
	ROM_LOAD( "15_82s129.8b",  0x0400, 0x0100, CRC(48ac3db4) SHA1(6a73fe21529bc760b8df1893aee9e89fb60976eb) ) /* Silkscreened as ROM15 */
	ROM_LOAD( "16_82s129.1e",  0x0500, 0x0100, CRC(59490887) SHA1(c894edecbcfc67972ad893cd7c8197d07862a20a) ) /* Silkscreened as ROM16 */
	ROM_LOAD( "17_82s129.1c",  0x0600, 0x0100, CRC(d492e6c2) SHA1(5789adda3a63ef8656ebd012416fcf3f991241fe) ) /* Silkscreened as ROM17 */
	ROM_LOAD( "18_82s123.2b",  0x0700, 0x0020, CRC(aa0ca5a5) SHA1(4c45be71658f40ebb05634febba5822f1a8a7f79) ) /* Silkscreened as ROM18 */
ROM_END

/*

This set comes from a different type of board with unique Taito ID code of "A76"
PCBs are labeled GX-006-A & GX-006-B  It has a 22pin edge connector and closely
resembles the Tiger-Heli PCB (shown above) with rom placement and components.

*/

ROM_START( slapfigha )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "a76_00.8p", 0x00000, 0x4000, CRC(ac22bb86) SHA1(3ecff006fc487d494f21adb7bff6f8c56eb5d707) )
	ROM_LOAD( "a76_01.8n", 0x04000, 0x4000, CRC(d6b4f02e) SHA1(37f840c444ba7dcc75810580c9da83289670d5cc) )
	ROM_LOAD( "a76_02.8k", 0x10000, 0x8000, CRC(9dd0971f) SHA1(92bd0b54635bf5c4118a53e0f897c65f5eb2984a) ) /* banked at 8000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a76_03.12d", 0x0000, 0x2000, CRC(87f4705a) SHA1(a90d5644ce268f3321047a4f96df96ac294d2f1b) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )
	ROM_LOAD( "a76_14.6a", 0x0000, 0x0800, CRC(3f4980bb) SHA1(bab19515b83b5a4e38ea7557c99ad44b9c8dff14) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a76_05.6f", 0x00000, 0x2000, CRC(be9a1bc5) SHA1(2fabfd42cd49db67654eac824c9852ed368a6e50) ) /* Chars */
	ROM_LOAD( "a76_04.6g", 0x02000, 0x2000, CRC(3519daa4) SHA1(ab77cc1bfe7c394d1a90a4c50d5d4a98158eb86d) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "a76_09.4m", 0x00000, 0x8000, CRC(b6358305) SHA1(c7bb4236a75ec6b88f011bc30f8fb9a718e2ca3e) ) /* Tiles */
	ROM_LOAD( "a76_08.6m", 0x08000, 0x8000, CRC(e92d9d60) SHA1(2554617e0e6615ca8c85a49299a4a0e762478339) )
	ROM_LOAD( "a76_07.6n", 0x10000, 0x8000, CRC(5faeeea3) SHA1(696fba24bcf1f3a7e914a4403854da5eededaf7f) )
	ROM_LOAD( "a76_06.6p", 0x18000, 0x8000, CRC(974e2ea9) SHA1(3840550fc3a833828dad8f3e300d2ea583d69ce7) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "a76_13.8j", 0x00000, 0x8000, CRC(8545d397) SHA1(9a1fd5bfd8fb830b8e46643c08eef32ba968fc23) ) /* Sprites */
	ROM_LOAD( "a76_12.6j", 0x08000, 0x8000, CRC(b1b7b925) SHA1(199b0b52bbeb384211171eca5c50a1c0ebf6826f) )
	ROM_LOAD( "a76_11.8h", 0x10000, 0x8000, CRC(422d946b) SHA1(c251ef9597a11ec8de39be4fcbddaba84e649ef2) )
	ROM_LOAD( "a76_10.6h", 0x18000, 0x8000, CRC(587113ae) SHA1(90abe961494a1af7c87693a419fbabf7a58a5dee) )

	ROM_REGION( 0x0720, "proms", 0 )
	ROM_LOAD( "a76-17.12q", 0x0000, 0x0100, CRC(a0efaf99) SHA1(5df01663480acad1f89abab8662d437617a66d1c) ) /* 82S129 Biploar PROM, Silkscreened as ROM21 */
	ROM_LOAD( "a76-15.12m", 0x0100, 0x0100, CRC(a56d57e5) SHA1(bfbd0db52b23fe1b4994e05103be3d412c1c013e) ) /* 82S129 Biploar PROM, Silkscreened as ROM20 */
	ROM_LOAD( "a76-16.12n", 0x0200, 0x0100, CRC(5cbf9fbf) SHA1(abfa58fa4e44ebc56f2e0fac9bcc36164c845fa3) ) /* 82S129 Biploar PROM, Silkscreened as ROM19 */
	ROM_LOAD( "14_82s129.2c",  0x0300, 0x0100, CRC(4dc04453) SHA1(524e5a212ab496c52adc0def995b75e89aba48e7) ) /* Silkscreened as ROM14 */
	ROM_LOAD( "15_82s129.8b",  0x0400, 0x0100, CRC(48ac3db4) SHA1(6a73fe21529bc760b8df1893aee9e89fb60976eb) ) /* Silkscreened as ROM15 */
	ROM_LOAD( "16_82s129.1e",  0x0500, 0x0100, CRC(59490887) SHA1(c894edecbcfc67972ad893cd7c8197d07862a20a) ) /* Silkscreened as ROM16 */
	ROM_LOAD( "17_82s129.1c",  0x0600, 0x0100, CRC(d492e6c2) SHA1(5789adda3a63ef8656ebd012416fcf3f991241fe) ) /* Silkscreened as ROM17 */
	ROM_LOAD( "18_82s123.2b",  0x0700, 0x0020, CRC(aa0ca5a5) SHA1(4c45be71658f40ebb05634febba5822f1a8a7f79) ) /* Silkscreened as ROM18 */
ROM_END

/*
PCB Details from slapfighb1 boardset:

Upper PCB (Sound board)
---------
Z80A CPU
Toshiba TMM2016BP-10 (2KB SRAM)
sf_s05 (Fujitsu MBM2764-25 8KB EPROM) - Sound CPU Code

Yamaha YM2149F (Qty 2 - Pin compatible with AY-3-8190)
Hitachi SRAM - HM6464 (8KB - Qty 4)

sf_s01 (OKI M27256-N 32KB PROM)              Sprite Data (16x16 4bpp)
sf_s02 (OKI M27256-N 32KB PROM)              Sprite Data
sf_s03 (OKI M27256-N 32KB PROM)              Sprite Data
sf_s04 (OKI M27256-N 32KB PROM)              Sprite Data


Lower PCB
---------
Z80B CPU
12MHz Xtal
Toshiba TMM2016BP-10 (2KB SRAM - Total Qty 6 = 2+2+1+1)

sf_s10 (Fujitsu MBM2764-25 8KB EPROM)        Font/Character Data (8x8 2bpp)
sf_s11 (Fujitsu MBM2764-25 8KB EPROM)

sf_s06 (OKI M27256-N 32KB PROM)              Tile Data (8x8 4bpp)
sf_s07 (OKI M27256-N 32KB PROM)              Tile Data
sf_s08 (OKI M27256-N 32KB PROM)              Tile Data
sf_s09 (OKI M27256-N 32KB PROM)              Tile Data

sf_s16 (Fujitsu MBM2764-25 8KB EPROM)        Colour Tables (512B used?)

sf_sH  (OKI M27256-N 32KB PROM)              Level Maps ???

sf_s19 (NEC S27128 16KB EPROM)               CPU Code $0000-$3fff
sf_s20 (Mitsubishi M5L27128K 16KB EPROM)     CPU Code $4000-$7fff
*/

ROM_START( slapfighb1 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "sf_r19jb.bin", 0x00000, 0x8000, CRC(9a7ac8b3) SHA1(01fbad9b4fc80f2406eff18db20e196e212d0c17) )
	ROM_LOAD( "sf_rh.bin",    0x10000, 0x8000, CRC(3c42e4a7) SHA1(8e4da1e6e73603e484ba4f5609ac9ea92999a526) ) /* banked at 8000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sf_r05.bin", 0x0000, 0x2000, CRC(87f4705a) SHA1(a90d5644ce268f3321047a4f96df96ac294d2f1b) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "sf_r11.bin", 0x00000, 0x2000, CRC(2ac7b943) SHA1(d0c3560bb1f0c2647aeff807cb4b09450237b955) ) /* Chars */
	ROM_LOAD( "sf_r10.bin", 0x02000, 0x2000, CRC(33cadc93) SHA1(59ffc206c62a651d2ac0ef52f519dd56edf2c021) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "sf_r06.bin", 0x00000, 0x8000, CRC(b6358305) SHA1(c7bb4236a75ec6b88f011bc30f8fb9a718e2ca3e) ) /* Tiles */
	ROM_LOAD( "sf_r09.bin", 0x08000, 0x8000, CRC(e92d9d60) SHA1(2554617e0e6615ca8c85a49299a4a0e762478339) )
	ROM_LOAD( "sf_r08.bin", 0x10000, 0x8000, CRC(5faeeea3) SHA1(696fba24bcf1f3a7e914a4403854da5eededaf7f) )
	ROM_LOAD( "sf_r07.bin", 0x18000, 0x8000, CRC(974e2ea9) SHA1(3840550fc3a833828dad8f3e300d2ea583d69ce7) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "sf_r03.bin", 0x00000, 0x8000, CRC(8545d397) SHA1(9a1fd5bfd8fb830b8e46643c08eef32ba968fc23) ) /* Sprites */
	ROM_LOAD( "sf_r01.bin", 0x08000, 0x8000, CRC(b1b7b925) SHA1(199b0b52bbeb384211171eca5c50a1c0ebf6826f) )
	ROM_LOAD( "sf_r04.bin", 0x10000, 0x8000, CRC(422d946b) SHA1(c251ef9597a11ec8de39be4fcbddaba84e649ef2) )
	ROM_LOAD( "sf_r02.bin", 0x18000, 0x8000, CRC(587113ae) SHA1(90abe961494a1af7c87693a419fbabf7a58a5dee) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "sf_col21.bin", 0x0000, 0x0100, CRC(a0efaf99) SHA1(5df01663480acad1f89abab8662d437617a66d1c) )
	ROM_LOAD( "sf_col20.bin", 0x0100, 0x0100, CRC(a56d57e5) SHA1(bfbd0db52b23fe1b4994e05103be3d412c1c013e) )
	ROM_LOAD( "sf_col19.bin", 0x0200, 0x0100, CRC(5cbf9fbf) SHA1(abfa58fa4e44ebc56f2e0fac9bcc36164c845fa3) )
ROM_END

ROM_START( slapfighb2 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "sf_r19eb.bin", 0x00000, 0x4000, CRC(2efe47af) SHA1(69ce3e83a0d8fa5ee4737c741d31cf32db6b9919) )
	ROM_LOAD( "sf_r20eb.bin", 0x04000, 0x4000, CRC(f42c7951) SHA1(d76e7a72f6ced67b550ba68cd42987f7111f5468) )
	ROM_LOAD( "sf_rh.bin",    0x10000, 0x8000, CRC(3c42e4a7) SHA1(8e4da1e6e73603e484ba4f5609ac9ea92999a526) ) /* banked at 8000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sf_r05.bin", 0x0000, 0x2000, CRC(87f4705a) SHA1(a90d5644ce268f3321047a4f96df96ac294d2f1b) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "sf_r11.bin", 0x00000, 0x2000, CRC(2ac7b943) SHA1(d0c3560bb1f0c2647aeff807cb4b09450237b955) ) /* Chars */
	ROM_LOAD( "sf_r10.bin", 0x02000, 0x2000, CRC(33cadc93) SHA1(59ffc206c62a651d2ac0ef52f519dd56edf2c021) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "sf_r06.bin", 0x00000, 0x8000, CRC(b6358305) SHA1(c7bb4236a75ec6b88f011bc30f8fb9a718e2ca3e) ) /* Tiles */
	ROM_LOAD( "sf_r09.bin", 0x08000, 0x8000, CRC(e92d9d60) SHA1(2554617e0e6615ca8c85a49299a4a0e762478339) )
	ROM_LOAD( "sf_r08.bin", 0x10000, 0x8000, CRC(5faeeea3) SHA1(696fba24bcf1f3a7e914a4403854da5eededaf7f) )
	ROM_LOAD( "sf_r07.bin", 0x18000, 0x8000, CRC(974e2ea9) SHA1(3840550fc3a833828dad8f3e300d2ea583d69ce7) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "sf_r03.bin", 0x00000, 0x8000, CRC(8545d397) SHA1(9a1fd5bfd8fb830b8e46643c08eef32ba968fc23) ) /* Sprites */
	ROM_LOAD( "sf_r01.bin", 0x08000, 0x8000, CRC(b1b7b925) SHA1(199b0b52bbeb384211171eca5c50a1c0ebf6826f) )
	ROM_LOAD( "sf_r04.bin", 0x10000, 0x8000, CRC(422d946b) SHA1(c251ef9597a11ec8de39be4fcbddaba84e649ef2) )
	ROM_LOAD( "sf_r02.bin", 0x18000, 0x8000, CRC(587113ae) SHA1(90abe961494a1af7c87693a419fbabf7a58a5dee) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "sf_col21.bin", 0x0000, 0x0100, CRC(a0efaf99) SHA1(5df01663480acad1f89abab8662d437617a66d1c) )
	ROM_LOAD( "sf_col20.bin", 0x0100, 0x0100, CRC(a56d57e5) SHA1(bfbd0db52b23fe1b4994e05103be3d412c1c013e) )
	ROM_LOAD( "sf_col19.bin", 0x0200, 0x0100, CRC(5cbf9fbf) SHA1(abfa58fa4e44ebc56f2e0fac9bcc36164c845fa3) )
ROM_END

/* very similar to slapfighb2, is slapfighb2 missing the logo rom? */
ROM_START( slapfighb3 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "k1-10.u90", 0x00000, 0x4000, CRC(2efe47af) SHA1(69ce3e83a0d8fa5ee4737c741d31cf32db6b9919) )
	ROM_LOAD( "k1-09.u89", 0x04000, 0x4000, CRC(17c187c5) SHA1(6e1fd651f56036d1c6c830de8479df25fc182c10) )
	ROM_LOAD( "k1-08.u88", 0x0e000, 0x2000, CRC(945af97f) SHA1(4d901be477b6101338eb1d86497e1bdc57f9c1b4) ) // contains another copy of the logo tilemap read at ec00!
	ROM_LOAD( "k1-07.u87", 0x10000, 0x8000, CRC(3c42e4a7) SHA1(8e4da1e6e73603e484ba4f5609ac9ea92999a526) ) /* banked at 8000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "k1-11.u89", 0x0000, 0x2000, CRC(87f4705a) SHA1(a90d5644ce268f3321047a4f96df96ac294d2f1b) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "k1-02.u57", 0x00000, 0x2000, CRC(2ac7b943) SHA1(d0c3560bb1f0c2647aeff807cb4b09450237b955) ) /* Chars */
	ROM_LOAD( "k1-03.u58", 0x02000, 0x2000, CRC(33cadc93) SHA1(59ffc206c62a651d2ac0ef52f519dd56edf2c021) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "k1-01.u49", 0x00000, 0x8000, CRC(b6358305) SHA1(c7bb4236a75ec6b88f011bc30f8fb9a718e2ca3e) ) /* Tiles */
	ROM_LOAD( "k1-04.u62", 0x08000, 0x8000, CRC(e92d9d60) SHA1(2554617e0e6615ca8c85a49299a4a0e762478339) )
	ROM_LOAD( "k1-05.u63", 0x10000, 0x8000, CRC(5faeeea3) SHA1(696fba24bcf1f3a7e914a4403854da5eededaf7f) )
	ROM_LOAD( "k1-06.u64", 0x18000, 0x8000, CRC(974e2ea9) SHA1(3840550fc3a833828dad8f3e300d2ea583d69ce7) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "k1-15.u60", 0x00000, 0x8000, CRC(8545d397) SHA1(9a1fd5bfd8fb830b8e46643c08eef32ba968fc23) ) /* Sprites */
	ROM_LOAD( "k1-13.u50", 0x08000, 0x8000, CRC(b1b7b925) SHA1(199b0b52bbeb384211171eca5c50a1c0ebf6826f) )
	ROM_LOAD( "k1-14.u59", 0x10000, 0x8000, CRC(422d946b) SHA1(c251ef9597a11ec8de39be4fcbddaba84e649ef2) )
	ROM_LOAD( "k1-12.u49", 0x18000, 0x8000, CRC(587113ae) SHA1(90abe961494a1af7c87693a419fbabf7a58a5dee) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "sf_col21.bin", 0x0000, 0x0100, CRC(a0efaf99) SHA1(5df01663480acad1f89abab8662d437617a66d1c) )
	ROM_LOAD( "sf_col20.bin", 0x0100, 0x0100, CRC(a56d57e5) SHA1(bfbd0db52b23fe1b4994e05103be3d412c1c013e) )
	ROM_LOAD( "sf_col19.bin", 0x0200, 0x0100, CRC(5cbf9fbf) SHA1(abfa58fa4e44ebc56f2e0fac9bcc36164c845fa3) )
ROM_END

/*

Guardian
1986 Taito Corporation

The Guardian PCB is exactly the same PCB as Tiger-Heli, including the
edge connector pinout.

PCB Layout
----------

Top Board

Sticker - M6100151B  860010898
GX-006-A MADE IN JAPAN
|--------------------------------------------------|
|VOL ROM21.12Q                 2148                |
|MB3712                        2148                |
|    ROM19.12P                 2148                |
|                              2148                |
|    ROM20.12M                         6264        |
|                                                  |
|                                      6264        |
|                                                  |
|2                 A68_13.8J A68_12.6J 6264        |
|2    YM2149F                                      |
|W                 A68_11.8H A68_10.6H 6264        |
|A    DSW1  DSW2                                   |
|Y                                       2148      |
|     YM2149F                                      |
|                                          ROM16.1E|
|                                                  |
|  A68_03.12D                              ROM17.1C|
|                                                  |
|                                     ROM18.2B     |
|  Z80A   6116                                     |
|                                                  |
|-----|----------|--------------|----------|-------|
      |----------|              |----------|
Notes:
      YM2149F - Clock 1.500MHz (36/24)
         Z80A - Clock 3.000MHz (36/12)
         6116 - 2kx8 SRAM
         6264 - 8kx8 SRAM
       MB3712 - Audio Power AMP
         2148 - 1kx4 SRAM
       DSW1/2 - 8-position DIP-switch
        VSync - 56.91313Hz
        HSync - 14.97090kHz
       A68_03 - 2764 EPROM
 A68_10 to 13 - 27256 EPROM
        ROM18 - 82S123 PROM
ROM16/17/19/20/21 - 82S129 PROM


Bottom Board

GX-006-B MADE IN JAPAN
|--------------------------------------------------|
|                        A68_14.6A                 |
|                                                  |
|    ROM14.2C                          ROM15.8B    |
|                                                  |
|                                                  |
|              6116      A68_05.6F                 |
|                                           36MHz  |
|              6116      A68_04.6G                 |
|                                                  |
|                                                  |
|                                                  |
|                                                  |
|                                        6116      |
|                                        A68_02.8K |
|                                                  |
|             A68_09.4M  A47_08.6M       X         |
|     6116                                         |
|             6116       A68_07.6N       A68_01.8N |
|                                                  |
|             6116       A68_06.6P       A68_00.8P |
|                                           Z80B   |
|-----|----------|--------------|----------|-------|
      |----------|              |----------|
Notes:
      Z80B - Clock 6.000MHz (36/6)
      6116 - 2kx8 SRAM
  ROM14/15 - 82S129 PROM
 A68_00/01 - 27128 EPROM
A68_02 & 06 to 09 - 27256 EPROM
 A68_04/05 - 2764 EPROM
    A68_14 - Motorola MC68705P5 Micro-Controller (protected). Clock 3.000MHz [36/12]
         X - Unpopulated socket
*/

ROM_START( grdian )
	ROM_REGION( 0x18000, "maincpu", 0 )     /* Region 0 - main cpu code */
	ROM_LOAD( "a68_00-1.8p", 0x00000, 0x4000, CRC(6a8bdc6c) SHA1(c923bca539bd2eb9a34cb9c7a67a199e28bc081a) )
	ROM_LOAD( "a68_01-1.8n", 0x04000, 0x4000, CRC(ebe8db3c) SHA1(9046d6e63c33fc9cbd48b90dcbcc0badf1d3b9ba) )
	ROM_LOAD( "a68_02-1.8k", 0x10000, 0x8000, CRC(343e8415) SHA1(00b98055277a0ddfb7d0bda6537df10a4049533e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Region 3 - sound cpu code */
	ROM_LOAD( "a68-03.12d", 0x0000, 0x2000, CRC(18daa44c) SHA1(1a3d22a186c591321d1b836ee30d89fba4771122) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a68_14.6a", 0x0000, 0x0800, CRC(87c4ca48) SHA1(ae15dab7adde7ad2381ca3d70331891c8f3bcc42) )

	ROM_REGION( 0x04000, "gfx1", 0 )    /* Region 1 - temporary for gfx */
	ROM_LOAD( "a68_05-1.6f", 0x00000, 0x2000, CRC(06f60107) SHA1(c5dcf0c7a5863ea960ee747d2d7ec7ac8bb7d3af) ) /* Chars */
	ROM_LOAD( "a68_04-1.6g", 0x02000, 0x2000, CRC(1fc8f277) SHA1(59dc1a0fad23b1e98abca3d0b1685b9d2939b059) )

	ROM_REGION( 0x20000, "gfx2", 0 )    /* Region 1 - temporary for gfx */
	ROM_LOAD( "a68_09.4m", 0x00000, 0x8000, CRC(a293cc2e) SHA1(a2c2598e92982d13b51cbb6efb4b963142233433) ) /* Tiles */
	ROM_LOAD( "a68_08.6m", 0x08000, 0x8000, CRC(37662375) SHA1(46ba8a3f0b553d476ecf431d0d20556896b4ca43) )
	ROM_LOAD( "a68_07.6n", 0x10000, 0x8000, CRC(cf1a964c) SHA1(e9223c8d4f3bdafed193a1ded63e377f16f45e17) )
	ROM_LOAD( "a68_06.6p", 0x18000, 0x8000, CRC(05f9eb9a) SHA1(a71640a63b259799086d361ef293aa26cec46a0c) )

	ROM_REGION( 0x20000, "gfx3", 0 )    /* Region 1 - temporary for gfx */
	ROM_LOAD( "a68-13.8j", 0x00000, 0x8000, CRC(643fb282) SHA1(d904d3c27c2b56341929c5eed4ea97e948c53c34) ) /* Sprites */
	ROM_LOAD( "a68-12.6j", 0x08000, 0x8000, CRC(11f74e32) SHA1(02d8b4cc679f45a02c4989f2b62cde91b7418235) )
	ROM_LOAD( "a68-11.8h", 0x10000, 0x8000, CRC(f24158cf) SHA1(db4c6b68a488b0798ea5f793ac8ced283a8ecab2) )
	ROM_LOAD( "a68-10.6h", 0x18000, 0x8000, CRC(83161ed0) SHA1(a6aa28f22f487dc3a2ec07935e6d42bcdd1eff81) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "rom21.12q", 0x0000, 0x0100, CRC(d6360b4d) SHA1(3e64548c82a3378fc091e104cdc2b0c7e592fc44) )
	ROM_LOAD( "rom20.12m", 0x0100, 0x0100, CRC(4ca01887) SHA1(2892c89d5e60f1d10593adffff55c1a9654e8209) )
	ROM_LOAD( "rom19.12p", 0x0200, 0x0100, CRC(513224f0) SHA1(15b34612206138f6fc5f7478925b1fff2ed56aa8) )
ROM_END

ROM_START( getstarj )
	ROM_REGION( 0x18000, "maincpu", 0 )     /* Region 0 - main cpu code */
	ROM_LOAD( "a68_00.8p", 0x00000, 0x4000, CRC(ad1a0143) SHA1(0d9adeb12bd4d5ad11e5bada0cd7498bc565c1db) )
	ROM_LOAD( "a68_01.8n", 0x04000, 0x4000, CRC(3426eb7c) SHA1(e91db45a650a1bfefd7c12c7553b647bc916c7c8) )
	ROM_LOAD( "a68_02.8k", 0x10000, 0x8000, CRC(3567da17) SHA1(29d698606d0bd30abfc3171d79bfad95b0de89fc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Region 3 - sound cpu code */
	ROM_LOAD( "a68-03.12d", 0x00000, 0x2000, CRC(18daa44c) SHA1(1a3d22a186c591321d1b836ee30d89fba4771122) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a68_14.6a", 0x0000, 0x0800, CRC(87c4ca48) SHA1(ae15dab7adde7ad2381ca3d70331891c8f3bcc42) )

	ROM_REGION( 0x04000, "gfx1", 0 )    /* Region 1 - temporary for gfx */
	ROM_LOAD( "a68_05.6f", 0x00000, 0x2000, CRC(e3d409e7) SHA1(0b6be4767f110729f4dd1a472ef8d9a0c718b684) ) /* Chars */
	ROM_LOAD( "a68_04.6g", 0x02000, 0x2000, CRC(6e5ac9d4) SHA1(74f90b7a1ceb3b1c2fd92dff100d92dea0155530) )

	ROM_REGION( 0x20000, "gfx2", 0 )    /* Region 1 - temporary for gfx */
	ROM_LOAD( "a68_09.4m", 0x00000, 0x8000, CRC(a293cc2e) SHA1(a2c2598e92982d13b51cbb6efb4b963142233433) ) /* Tiles */
	ROM_LOAD( "a68_08.6m", 0x08000, 0x8000, CRC(37662375) SHA1(46ba8a3f0b553d476ecf431d0d20556896b4ca43) )
	ROM_LOAD( "a68_07.6n", 0x10000, 0x8000, CRC(cf1a964c) SHA1(e9223c8d4f3bdafed193a1ded63e377f16f45e17) )
	ROM_LOAD( "a68_06.6p", 0x18000, 0x8000, CRC(05f9eb9a) SHA1(a71640a63b259799086d361ef293aa26cec46a0c) )

	ROM_REGION( 0x20000, "gfx3", 0 )    /* Region 1 - temporary for gfx */
	ROM_LOAD( "a68-13.8j", 0x00000, 0x8000, CRC(643fb282) SHA1(d904d3c27c2b56341929c5eed4ea97e948c53c34) ) /* Sprites */
	ROM_LOAD( "a68-12.6j", 0x08000, 0x8000, CRC(11f74e32) SHA1(02d8b4cc679f45a02c4989f2b62cde91b7418235) )
	ROM_LOAD( "a68-11.8h", 0x10000, 0x8000, CRC(f24158cf) SHA1(db4c6b68a488b0798ea5f793ac8ced283a8ecab2) )
	ROM_LOAD( "a68-10.6h", 0x18000, 0x8000, CRC(83161ed0) SHA1(a6aa28f22f487dc3a2ec07935e6d42bcdd1eff81) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "rom21.12q", 0x0000, 0x0100, CRC(d6360b4d) SHA1(3e64548c82a3378fc091e104cdc2b0c7e592fc44) )
	ROM_LOAD( "rom20.12m", 0x0100, 0x0100, CRC(4ca01887) SHA1(2892c89d5e60f1d10593adffff55c1a9654e8209) )
	ROM_LOAD( "rom19.12p", 0x0200, 0x0100, CRC(513224f0) SHA1(15b34612206138f6fc5f7478925b1fff2ed56aa8) )
ROM_END

ROM_START( getstarb1 )
	ROM_REGION( 0x18000, "maincpu", 0 )  /* Region 0 - main cpu code */
	ROM_LOAD( "getstar_14_b.8p",  0x00000, 0x4000, CRC(9afad7e0) SHA1(6b2e82a6b7fcbfed5f4d250959ecc571fdf0cbc2) ) // gs_rb_1 - 27128 (matches the socket label)
	ROM_LOAD( "getstar_13_b.8n",  0x04000, 0x4000, CRC(5feb0a60) SHA1(b1300055180ddf6ca96475eb3a27a17722273fc6) ) // gs_rb_2 - 27128 (matches the socket label)
	ROM_LOAD( "getstar_12_b.8k",  0x10000, 0x8000, CRC(e3cfb1ba) SHA1(bd21655c82a14e18ff9df4539c4d0bb2484c73f1) ) // gs_rb_3 - 27256 (The socket is labeled as 27128)

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Region 3 - sound cpu code */
	ROM_LOAD( "getstar_5_a.12e", 0x00000, 0x2000, CRC(18daa44c) SHA1(1a3d22a186c591321d1b836ee30d89fba4771122) ) // == a68-03 - 2764 (matches the socket label)

	ROM_REGION( 0x04000, "gfx1", 0 )     /* Region 1 - temporary for gfx */  /* Chars */  /* Verified on new bootleg PCB found */
	ROM_LOAD( "getstar_7_b.6f", 0x00000, 0x2000, CRC(e3d409e7) SHA1(0b6be4767f110729f4dd1a472ef8d9a0c718b684) ) // 2764 (matches the socket label)
	ROM_LOAD( "getstar_8_b.6g", 0x02000, 0x2000, CRC(6e5ac9d4) SHA1(74f90b7a1ceb3b1c2fd92dff100d92dea0155530) ) // 2764 (matches the socket label)

	ROM_REGION( 0x20000, "gfx2", 0 )     /* Region 1 - temporary for gfx */  /* Tiles */
	ROM_LOAD( "getstar_6_b.4m",  0x00000, 0x8000, CRC(a293cc2e) SHA1(a2c2598e92982d13b51cbb6efb4b963142233433) ) // == a68_09 - 27256 (the socket is labeled as 27128)
	ROM_LOAD( "getstar_9_b.6m",  0x08000, 0x8000, CRC(37662375) SHA1(46ba8a3f0b553d476ecf431d0d20556896b4ca43) ) // == a68_08 - 27256 (The socket is labeled as 27128)
	ROM_LOAD( "getstar_10_b.6n", 0x10000, 0x8000, CRC(cf1a964c) SHA1(e9223c8d4f3bdafed193a1ded63e377f16f45e17) ) // == a68_07 - 27256 (The socket is labeled as 27128)
	ROM_LOAD( "getstar_11_b.6p", 0x18000, 0x8000, CRC(05f9eb9a) SHA1(a71640a63b259799086d361ef293aa26cec46a0c) ) // == a68_06 - 27256 (The socket is labeled as 27128)

	ROM_REGION( 0x20000, "gfx3", 0 )     /* Region 1 - temporary for gfx */  /* Sprites */
	ROM_LOAD( "getstar_4_a.8j", 0x00000, 0x8000, CRC(643fb282) SHA1(d904d3c27c2b56341929c5eed4ea97e948c53c34) ) // == a68-13 - 27256 (the socket is labeled as 27128)
	ROM_LOAD( "getstar_2_a.7j", 0x08000, 0x8000, CRC(11f74e32) SHA1(02d8b4cc679f45a02c4989f2b62cde91b7418235) ) // == a68-12 - 27256 (the socket is labeled as 27128)
	ROM_LOAD( "getstar_3_a.8h", 0x10000, 0x8000, CRC(f24158cf) SHA1(db4c6b68a488b0798ea5f793ac8ced283a8ecab2) ) // == a68-11 - 27256 (the socket is labeled as 27128)
	ROM_LOAD( "getstar_1_a.7h", 0x18000, 0x8000, CRC(83161ed0) SHA1(a6aa28f22f487dc3a2ec07935e6d42bcdd1eff81) ) // == a68-10 - 27256 (the socket is labeled as 27128)

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "getstar_p7_a.12q", 0x0000, 0x0100, CRC(d6360b4d) SHA1(3e64548c82a3378fc091e104cdc2b0c7e592fc44) ) // rom21 - 82S129N (matches the socket label)
	ROM_LOAD( "getstar_p5_a.12m", 0x0100, 0x0100, CRC(4ca01887) SHA1(2892c89d5e60f1d10593adffff55c1a9654e8209) ) // rom20 - 82S129N (matches the socket label)
	ROM_LOAD( "getstar_p6_a.12n", 0x0200, 0x0100, CRC(35eefbbb) SHA1(b885938cc7ef322b354a0d60f9c7614686bb33ea) ) // rom19 - 82S129N (matches the socket label)

	/* Unused. Weren't on the first dump, but verified to be on the PCB */
	ROM_REGION( 0x0500, "proms2", 0 )
	ROM_LOAD( "getstar_p1_a.1c", 0x0000, 0x0100, CRC(d492e6c2) SHA1(5789adda3a63ef8656ebd012416fcf3f991241fe) ) // 82S129N (matches the socket label)
	ROM_LOAD( "getstar_p2_a.1e", 0x0100, 0x0100, CRC(59490887) SHA1(c894edecbcfc67972ad893cd7c8197d07862a20a) ) // 82S129N (matches the socket label)
	ROM_LOAD( "getstar_p3_a.2b", 0x0200, 0x0020, CRC(aa0ca5a5) SHA1(4c45be71658f40ebb05634febba5822f1a8a7f79) ) // 82S123N (matches the socket label)
	ROM_LOAD( "getstar_p8_b.2c", 0x0300, 0x0100, CRC(4dc04453) SHA1(524e5a212ab496c52adc0def995b75e89aba48e7) ) // 82S129N (matches the socket label)
	ROM_LOAD( "getstar_p9_b.8b", 0x0400, 0x0100, CRC(48ac3db4) SHA1(6a73fe21529bc760b8df1893aee9e89fb60976eb) ) // 82S129N (matches the socket label)

	/* Unused, but verified to be on the PCB */
	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "getstar_p4_a.2e",  0x0000,  0x0104, NO_DUMP ) // PAL16R4ACN - Protected
ROM_END

ROM_START( getstarb2 )
	ROM_REGION( 0x18000, "maincpu", 0 )     /* Region 0 - main cpu code */
	ROM_LOAD( "gs_14.rom",  0x00000, 0x4000, CRC(1a57a920) SHA1(b1e9d5b29c0e3632eec3ad1ee51bf3392e4b816d) )
	ROM_LOAD( "gs_13.rom",  0x04000, 0x4000, CRC(805f8e77) SHA1(c3ad6eae842d2d10f716998d5a803038fa7b338f) )
	ROM_LOAD( "a68_02.bin", 0x10000, 0x8000, CRC(3567da17) SHA1(29d698606d0bd30abfc3171d79bfad95b0de89fc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Region 3 - sound cpu code */
	ROM_LOAD( "a68-03", 0x0000,  0x2000, CRC(18daa44c) SHA1(1a3d22a186c591321d1b836ee30d89fba4771122) )

	ROM_REGION( 0x04000, "gfx1", 0 )    /* Region 1 - temporary for gfx */
	ROM_LOAD( "a68_05.bin", 0x00000, 0x2000, CRC(e3d409e7) SHA1(0b6be4767f110729f4dd1a472ef8d9a0c718b684) ) /* Chars */
	ROM_LOAD( "a68_04.bin", 0x02000, 0x2000, CRC(6e5ac9d4) SHA1(74f90b7a1ceb3b1c2fd92dff100d92dea0155530) )

	ROM_REGION( 0x20000, "gfx2", 0 )    /* Region 1 - temporary for gfx */
	ROM_LOAD( "a68_09", 0x00000, 0x8000, CRC(a293cc2e) SHA1(a2c2598e92982d13b51cbb6efb4b963142233433) ) /* Tiles */
	ROM_LOAD( "a68_08", 0x08000, 0x8000, CRC(37662375) SHA1(46ba8a3f0b553d476ecf431d0d20556896b4ca43) )
	ROM_LOAD( "a68_07", 0x10000, 0x8000, CRC(cf1a964c) SHA1(e9223c8d4f3bdafed193a1ded63e377f16f45e17) )
	ROM_LOAD( "a68_06", 0x18000, 0x8000, CRC(05f9eb9a) SHA1(a71640a63b259799086d361ef293aa26cec46a0c) )

	ROM_REGION( 0x20000, "gfx3", 0 )    /* Region 1 - temporary for gfx */
	ROM_LOAD( "a68-13", 0x00000, 0x8000, CRC(643fb282) SHA1(d904d3c27c2b56341929c5eed4ea97e948c53c34) ) /* Sprites */
	ROM_LOAD( "a68-12", 0x08000, 0x8000, CRC(11f74e32) SHA1(02d8b4cc679f45a02c4989f2b62cde91b7418235) )
	ROM_LOAD( "a68-11", 0x10000, 0x8000, CRC(f24158cf) SHA1(db4c6b68a488b0798ea5f793ac8ced283a8ecab2) )
	ROM_LOAD( "a68-10", 0x18000, 0x8000, CRC(83161ed0) SHA1(a6aa28f22f487dc3a2ec07935e6d42bcdd1eff81) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "rom21", 0x0000, 0x0100, CRC(d6360b4d) SHA1(3e64548c82a3378fc091e104cdc2b0c7e592fc44) )
	ROM_LOAD( "rom20", 0x0100, 0x0100, CRC(4ca01887) SHA1(2892c89d5e60f1d10593adffff55c1a9654e8209) )
	ROM_LOAD( "rom19", 0x0200, 0x0100, CRC(513224f0) SHA1(15b34612206138f6fc5f7478925b1fff2ed56aa8) )
ROM_END


/*  ( YEAR  NAME        PARENT    MACHINE     INPUT      INIT                       MONITOR, COMPANY, FULLNAME, FLAGS ) */
GAME( 1985, perfrman,   0,        perfrman,   perfrman,  slapfght_state, empty_init,     ROT270, "Toaplan / Data East Corporation", "Performan (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, perfrmanu,  perfrman, perfrman,   perfrman,  slapfght_state, empty_init,     ROT270, "Toaplan / Data East USA",         "Performan (US)", MACHINE_SUPPORTS_SAVE )

GAME( 1985, tigerh,     0,        tigerh,     tigerh,    slapfght_state, empty_init,     ROT270, "Toaplan / Taito America Corp.",   "Tiger-Heli (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, tigerhj,    tigerh,   tigerh,     tigerh,    slapfght_state, empty_init,     ROT270, "Toaplan / Taito",                 "Tiger-Heli (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, tigerhb1,   tigerh,   tigerhb1,   tigerh,    slapfght_state, empty_init,     ROT270, "bootleg",                         "Tiger-Heli (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, tigerhb2,   tigerh,   tigerhb2,   tigerh,    slapfght_state, empty_init,     ROT270, "bootleg",                         "Tiger-Heli (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, tigerhb3,   tigerh,   tigerhb2,   tigerh,    slapfght_state, empty_init,     ROT270, "bootleg",                         "Tiger-Heli (bootleg set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, tigerhb4,   tigerh,   tigerhb4,   tigerh,    slapfght_state, empty_init,     ROT270, "bootleg",                         "Tiger-Heli (bootleg set 4)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // MCU not hooked up

GAME( 1986, alcon,      0,        slapfigh,   slapfigh,  slapfght_state, init_slapfigh,  ROT270, "Toaplan / Taito America Corp.",   "Alcon (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1986, slapfigh,   alcon,    slapfigh,   slapfigh,  slapfght_state, init_slapfigh,  ROT270, "Toaplan / Taito",                 "Slap Fight (A77 set, 8606M PCB)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1986, slapfigha,  alcon,    slapfigh,   slapfigh,  slapfght_state, init_slapfigh,  ROT270, "Toaplan / Taito",                 "Slap Fight (A76 set, GX-006-A PCB)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1986, slapfighb1, alcon,    slapfighb1, slapfigh,  slapfght_state, init_slapfigh,  ROT270, "bootleg",                         "Slap Fight (bootleg set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1986, slapfighb2, alcon,    slapfighb2, slapfigh,  slapfght_state, init_slapfigh,  ROT270, "bootleg",                         "Slap Fight (bootleg set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL ) // England?
GAME( 1986, slapfighb3, alcon,    slapfighb2, slapfigh,  slapfght_state, init_slapfigh,  ROT270, "bootleg",                         "Slap Fight (bootleg set 3)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL ) // PCB labeled 'slap fighter'

GAME( 1986, grdian,     0,        slapfigh,   getstar,   slapfght_state, init_slapfigh,  ROT0,   "Toaplan / Taito America Corporation (Kitkorp license)", "Guardian (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, getstarj,   grdian,   slapfigh,   getstarj,  slapfght_state, init_slapfigh,  ROT0,   "Toaplan / Taito",                                       "Get Star (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, getstarb1,  grdian,   getstarb1,  getstarj,  slapfght_state, init_getstarb1, ROT0,   "bootleg",                                               "Get Star (bootleg set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1986, getstarb2,  grdian,   getstarb2,  getstarb2, slapfght_state, init_getstarb2, ROT0,   "bootleg",                                               "Get Star (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
