// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Rally X      (c) 1980 Namco
New Rally X  (c) 1981 Namco

and also

Jungler      (c) 1981 Konami
Tactician    (c) 1981 Konami
Loco-Motion  (c) 1982 Konami
Commando     (c) 1983 Sega

driver by Nicola Salmoria

There doesn't seem to be much doubt that Konami copied the video hardware of
Rally X.
The boards are surely very different; Rally X has video and sound split on the
two boards, while the Konami version has all video in one board and all sound
in the other.
Rally X uses a single Z80 and Namco sound hardware, while the others use the
standard Konami sound hardware of that era (slave Z80 + 2xAY-3-8910).
Also, the Konami design includes an optional starfield generator, only used
by Tactician. This is identical to the starfield circuit from Scramble.

Rally X has two Namco customs. They are nothing more than simple logic and can
be replaced by daughter boards with TTL parts.
NVC285 (DIP28): Z-80 Sync buss controller. Can be replaced by plug-in board
A082-91383-B000
NVC293 (DIP18): Video shifter. Can be replaced by plug-in board A082-91388-A000.


Rally X Memory map:
------------------
Note: the memory map for the RAM 6x chips derived from the schematics doesn't
seem to be entirely correct. Here it's modified to match program behaviour.
The names might be assigned incorrectly.

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0-000xxxxxxxxxxx R   xxxxxxxx ROM 1B    program ROM \[1]
0-001xxxxxxxxxxx R   xxxxxxxx ROM 1C    program ROM /
0-010xxxxxxxxxxx R   xxxxxxxx ROM 1D    program ROM \[1]
0-011xxxxxxxxxxx R   xxxxxxxx ROM 1E    program ROM /
0-100xxxxxxxxxxx R   xxxxxxxx ROM 1H    program ROM \[1]
0-101xxxxxxxxxxx R   xxxxxxxx ROM 1J    program ROM /
0-110xxxxxxxxxxx R   xxxxxxxx ROM 1K    program ROM \[1]
0-111xxxxxxxxxxx R   xxxxxxxx ROM 1L    program ROM /
1-0000xxxxxxxxxx R/W xxxxxxxx RAM 6A/6C radar tilemap RAM + sprites
1-0001xxxxxxxxxx R/W xxxxxxxx RAM 6B/6D playfield tilemap RAM
1-0010xxxxxxxxxx R/W xxxxxxxx RAM 6J/6K radar tilemap RAM + sprites
1-0011xxxxxxxxxx R/W xxxxxxxx RAM 6H/6L playfield tilemap RAM
1-0110xxxxxxxxxx R/W xxxxxxxx RAM 6F/6M work RAM
1-0111xxxxxxxxxx R/W xxxxxxxx RAM 6E/6N work RAM
1-1----00---xxxx   W ----xxxx SODWR     bullets shape and X pos msb [2]
1-1----01-------   W --------           watchdog reset
1-1----10000xxxx   W ----xxxx RAM 2N    \ sound control registers
1-1----10001xxxx   W ----xxxx RAM 2P    /
1-1----10010----   W          n.c.
1-1----10011----   W xxxxxxxx POSIX     playfield X scroll
1-1----10100----   W xxxxxxxx POSIY     playfield Y scroll
1-1----10101----   W          n.c.
1-1----10110       W          WR2       ?
1-1----10111       W          WR3       ? this is written to A LOT of times every frame
1-1----11----000   W -------x BANG      explosion sound trigger
1-1----11----001   W -------x INT ON    interrupt enable
1-1----11----010   W -------x SOUND ON  sound enable [3]
1-1----11----011   W -------x FLIP      flip screen
1-1----11----100   W -------x           1 player start lamp
1-1----11----101   W -------x           2 players start lamp
1-1----11----110   W -------x           coin lockout
1-1----11----111   W -------x           coin counter
1-1----00------- R   xxxxxxxx           switch inputs
1-1----01------- R   xxxxxxxx           switch inputs
1-1----10------- R   xxxxxxxx           dip switches
1-1----11        R            RDSTB     ?

[1] either 2x2716 or 1x2732
[2] SO = Small Objects? Only locations 4-F are used.
[3] doesn't seem to work in New Rally X.

I/O ports:
OUT on port $0 sets the interrupt vector/instruction (the game uses both
IM 2 and IM 0)


Loco-Motion Memory map:
----------------------
Main CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3D    program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 3E    program ROM
0010xxxxxxxxxxxx R   xxxxxxxx ROM 3F    program ROM
0011xxxxxxxxxxxx R   xxxxxxxx ROM 3G    program ROM
0100xxxxxxxxxxxx R   xxxxxxxx ROM 3J    program ROM
0101xxxxxxxxxxxx R   xxxxxxxx ROM 3K    program ROM
0110xxxxxxxxxxxx R   xxxxxxxx ROM 3L    program ROM
0111xxxxxxxxxxxx R   xxxxxxxx ROM 3M    program ROM
1-000xxxxxxxxxxx R/W xxxxxxxx RAM 3C    tilemap RAM (tile code) [1]
1-001xxxxxxxxxxx R/W xxxxxxxx RAM 3B    tilemap RAM (tile attr) [1]
1-011xxxxxxxxxxx R/W xxxxxxxx RAM 3A    work RAM
1-1----00------- R   xxxxxxxx IN1       switch inputs
1-1----01------- R   xxxxxxxx IN2       switch inputs
1-1----10------- R   xxxxxxxx IN3       switch inputs / dip switches
1-1----11------- R   xxxxxxxx IN4       dip switches
1-1----00---xxxx   W ----xxxx SCARW     bullets shape and X pos msb
1-1----01-------   W --------           watchdog reset
1-1----10-------   W xxxxxxxx SOUNDDATA command for sound CPU
1-1----11----000   W -------x SOUNDON   trigger irq on sound CPU
1-1----11----001   W -------x INTST     irq enable/acknowledge
1-1----11----010   W -------x MUT       sound disable
1-1----11----011   W -------x FLIP      flip screen
1-1----11----100   W -------x OUT1      coin counter #1
1-1----11----101   W -------x OUT2      ?
1-1----11----110   W -------x OUT3      coin counter #2
1-1----11----111   W -------x STARSON   stars enable (optional)

[1] 1st half is "radar" + sprite registers, 2nd half is scrolling playfield


Sound CPU (standard Konami sound board):

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 1B    program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 1C    program ROM
0010--xxxxxxxxxx R/W xxxxxxxx RAM 2B/2C work RAM
0011xxxxxx------   W -------- CKB       RC filters enable for 8910 #1 (data is in A6-A11)
0011------xxxxxx   W -------- CKB       RC filters enable for 8910 #2 (data is in A0-A5)
0100------------ R/W xxxxxxxx SEN1      AY-3-8910 #1 r/w [1]
0101------------   W xxxxxxxx SEN2      AY-3-8910 #1 ctrl
0110------------ R/W xxxxxxxx SEN3      AY-3-8910 #2 r/w
0111------------   W xxxxxxxx SEN4      AY-3-8910 #2 ctrl

[1] port A: command from main CPU; port B: timer


Notes:
- Easter egg (both Rally X and New Rally X):
  - enter service mode
  - keep B1 pressed and enter the following sequence:
    2xU 7xD 1xR 6xL
  (c) NAMCO LTD. 1980 will be added at the bottom of the screen.

- The Test Mode "dip switch" actually comes from the edge connector, but is mapped
  in memory in place of dip switch #8. Dip switch #8 is supposed to freeze the
  game and is entirely handled by hardware.

- PROM 7a controls the video shape. This is used to hide the rightmost 4 char
  columns in Locomotion and Commando, while showing them in Jungler and
  Tactician.

- The playfield scroll registers used in Rally X are present, but not useful
  (always 0) in Jungler and Tactician. They were removed in Locomotn and Commando.

- commsega has more sprites and more "bullets" than the other games.

- it seems that Jungler doesn't support high priority tiles. Maybe they
  disabled that feature because they needed more color combinations.

- there are also 1-pixel sprite and bullet placement differences from game to game.

- commsega:
  Due to a bug at 0x1259, bit 3 of DSW1 also affects the "Bonus Life" value:
     - when bit 3 is OFF, you get an extra life at 30000 points
     - when bit 3 is ON , you get an extra life at 50000 points
   At 0x0050 there is code to give infinite lives for player 1 when bit 3 of DSW0
   is ON. I can't tell however when it is supposed to be called.

- tactician has the score area at the bottom of the screen. While highly unusual,
  this is the correct behaviour and has been verified with screenshots.

TODO:
- commsega: the first time you kill a soldier, the music stops. When you die,
  the music restarts and won't stop a second time.

- rallyx: is the playing field area 221 pixels wide like with bosco? Currently
  kludged with set_scrolldx(3, 3);

- rallyx: Three things in the schematics that I haven't been able to trace:
  WR2, WR3 and RDSTB. Only WR3 is actually used by the game.

- rallyx: emulate the explosion with discrete sound components. The schematics
  are available so it should be possible eventually.

- tactician: the bouncing bomb seems to show incorrect graphics when it's hit.

***************************************************************************/

#include "emu.h"
#include "rallyx.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/samples.h"
#include "speaker.h"

static constexpr XTAL MASTER_CLOCK = 18.432_MHz_XTAL;


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void rallyx_state::rallyx_interrupt_vector_w(uint8_t data)
{
	m_interrupt_vector = data;
}


IRQ_CALLBACK_MEMBER(rallyx_state::interrupt_vector_r)
{
	return m_interrupt_vector;
}


void rallyx_state::bang_w(int state)
{
	if (state == 0 && m_last_bang != 0)
		m_samples->start(0, 0);

	m_last_bang = state;
}


void rallyx_state::irq_mask_w(int state)
{
	m_main_irq_mask = state;
	if (!state)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void rallyx_state::nmi_mask_w(int state)
{
	m_main_irq_mask = state;
	if (!state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


void rallyx_state::sound_on_w(int state)
{
	// this doesn't work in New Rally X so I'm not supporting it
	//m_namco_sound->pacman_sound_enable_w(state);
}


void rallyx_state::coin_lockout_w(int state)
{
	machine().bookkeeping().coin_lockout_w(0, !state);
}


void rallyx_state::coin_counter_1_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}


void rallyx_state::coin_counter_2_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void rallyx_state::rallyx_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x8fff).ram().w(FUNC(rallyx_state::videoram_w)).share(m_videoram);
	map(0x9800, 0x9fff).ram();
	map(0xa000, 0xa000).portr("P1");
	map(0xa080, 0xa080).portr("P2");
	map(0xa100, 0xa100).portr("DSW");
	map(0xa000, 0xa00f).writeonly().share(m_radarattr);
	map(0xa080, 0xa080).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xa100, 0xa11f).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0xa130, 0xa130).w(FUNC(rallyx_state::scrollx_w));
	map(0xa140, 0xa140).w(FUNC(rallyx_state::scrolly_w));
	map(0xa170, 0xa170).nopw();            // ?
	map(0xa180, 0xa187).w("mainlatch", FUNC(ls259_device::write_d0));
}

void rallyx_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0, 0).w(FUNC(rallyx_state::rallyx_interrupt_vector_w));
}


void rallyx_state::jungler_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram().w(FUNC(rallyx_state::videoram_w)).share(m_videoram);
	map(0x9800, 0x9fff).ram();
	map(0xa000, 0xa000).portr("P1");
	map(0xa080, 0xa080).portr("P2");
	map(0xa100, 0xa100).portr("DSW1");
	map(0xa180, 0xa180).portr("DSW2");
	map(0xa000, 0xa00f).mirror(0x00f0).writeonly().share(m_radarattr);   // jungler writes to a03x
	map(0xa080, 0xa080).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xa100, 0xa100).w(m_timeplt_audio, FUNC(timeplt_audio_device::sound_data_w));
	map(0xa130, 0xa130).w(FUNC(rallyx_state::scrollx_w)); // only jungler and tactcian
	map(0xa140, 0xa140).w(FUNC(rallyx_state::scrolly_w)); // only jungler and tactcian
	map(0xa180, 0xa187).w("mainlatch", FUNC(ls259_device::write_d0));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( rallyx )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("P2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x08, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("DSW:4,5,6")
	PORT_DIPSETTING(    0x10, "1 Car, Medium" )
	PORT_DIPSETTING(    0x28, "1 Car, Hard" )
	PORT_DIPSETTING(    0x00, "2 Cars, Easy" )
	PORT_DIPSETTING(    0x18, "2 Cars, Medium" )
	PORT_DIPSETTING(    0x30, "2 Cars, Hard" )
	PORT_DIPSETTING(    0x08, "3 Cars, Easy" )
	PORT_DIPSETTING(    0x20, "3 Cars, Medium" )
	PORT_DIPSETTING(    0x38, "3 Cars, Hard" )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW:2,3")
	PORT_DIPSETTING(    0x02, "15000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x00)
	PORT_DIPSETTING(    0x04, "30000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x00)
	PORT_DIPSETTING(    0x06, "40000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x00)

	PORT_DIPSETTING(    0x02, "20000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x08)
	PORT_DIPSETTING(    0x04, "40000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x08)
	PORT_DIPSETTING(    0x06, "60000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x08)

	PORT_DIPSETTING(    0x02, "10000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x10)
	PORT_DIPSETTING(    0x04, "20000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x10)
	PORT_DIPSETTING(    0x06, "30000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x10)

	PORT_DIPSETTING(    0x02, "15000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x18)
	PORT_DIPSETTING(    0x04, "30000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x18)
	PORT_DIPSETTING(    0x06, "40000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x18)

	PORT_DIPSETTING(    0x02, "20000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x20)
	PORT_DIPSETTING(    0x04, "40000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x20)
	PORT_DIPSETTING(    0x06, "60000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x20)

	PORT_DIPSETTING(    0x02, "10000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x28)
	PORT_DIPSETTING(    0x04, "20000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x28)
	PORT_DIPSETTING(    0x06, "30000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x28)

	PORT_DIPSETTING(    0x02, "15000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x30)
	PORT_DIPSETTING(    0x04, "30000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x30)
	PORT_DIPSETTING(    0x06, "40000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x30)

	PORT_DIPSETTING(    0x02, "20000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x38)
	PORT_DIPSETTING(    0x04, "40000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x38)
	PORT_DIPSETTING(    0x06, "60000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x38)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_SERVICE_DIPLOC( 0x01, 0x01, "DSW:1")
INPUT_PORTS_END

static INPUT_PORTS_START( rallyxeg )
	PORT_INCLUDE( rallyx )

	PORT_MODIFY("P1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
INPUT_PORTS_END

static INPUT_PORTS_START( dngrtrck )
	PORT_INCLUDE( rallyx )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, "1stC/1C 2ndC/2C" )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, "1stC/2C 2ndC/3C" )
INPUT_PORTS_END

static INPUT_PORTS_START( nrallyx )
	PORT_INCLUDE( rallyx )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("DSW:4,5,6")
	PORT_DIPSETTING(    0x10, "1 Car, Medium" )
	PORT_DIPSETTING(    0x28, "1 Car, Hard" )
	PORT_DIPSETTING(    0x18, "2 Cars, Medium" )
	PORT_DIPSETTING(    0x30, "2 Cars, Hard" )
	PORT_DIPSETTING(    0x00, "3 Cars, Easy" )
	PORT_DIPSETTING(    0x20, "3 Cars, Medium" )
	PORT_DIPSETTING(    0x38, "3 Cars, Hard" )
	PORT_DIPSETTING(    0x08, "4 Cars, Easy" )

	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW:2,3")
	PORT_DIPSETTING(    0x02, "20000/80000" )   PORT_CONDITION("DSW", 0x38, EQUALS, 0x00)
	PORT_DIPSETTING(    0x04, "20000/100000" )  PORT_CONDITION("DSW", 0x38, EQUALS, 0x00)
	PORT_DIPSETTING(    0x06, "20000/120000" )  PORT_CONDITION("DSW", 0x38, EQUALS, 0x00)

	PORT_DIPSETTING(    0x02, "20000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x08)
	PORT_DIPSETTING(    0x04, "40000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x08)
	PORT_DIPSETTING(    0x06, "60000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x08)

	PORT_DIPSETTING(    0x02, "20000/80000" )   PORT_CONDITION("DSW", 0x38, EQUALS, 0x10)
	PORT_DIPSETTING(    0x04, "20000/100000" )  PORT_CONDITION("DSW", 0x38, EQUALS, 0x10)
	PORT_DIPSETTING(    0x06, "20000/120000" )  PORT_CONDITION("DSW", 0x38, EQUALS, 0x10)

	PORT_DIPSETTING(    0x02, "20000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x18)
	PORT_DIPSETTING(    0x04, "40000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x18)
	PORT_DIPSETTING(    0x06, "60000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x18)

	PORT_DIPSETTING(    0x02, "20000/80000" )   PORT_CONDITION("DSW", 0x38, EQUALS, 0x20)
	PORT_DIPSETTING(    0x04, "20000/100000" )  PORT_CONDITION("DSW", 0x38, EQUALS, 0x20)
	PORT_DIPSETTING(    0x06, "20000/120000" )  PORT_CONDITION("DSW", 0x38, EQUALS, 0x20)

	PORT_DIPSETTING(    0x02, "20000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x28)
	PORT_DIPSETTING(    0x04, "40000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x28)
	PORT_DIPSETTING(    0x06, "60000" )     PORT_CONDITION("DSW", 0x38, EQUALS, 0x28)

	PORT_DIPSETTING(    0x02, "20000/80000" )   PORT_CONDITION("DSW", 0x38, EQUALS, 0x30)
	PORT_DIPSETTING(    0x04, "20000/100000" )  PORT_CONDITION("DSW", 0x38, EQUALS, 0x30)
	PORT_DIPSETTING(    0x06, "20000/120000" )  PORT_CONDITION("DSW", 0x38, EQUALS, 0x30)

	PORT_DIPSETTING(    0x02, "20000/80000" )   PORT_CONDITION("DSW", 0x38, EQUALS, 0x38)
	PORT_DIPSETTING(    0x04, "20000/100000" )  PORT_CONDITION("DSW", 0x38, EQUALS, 0x38)
	PORT_DIPSETTING(    0x06, "20000/120000" )  PORT_CONDITION("DSW", 0x38, EQUALS, 0x38)
INPUT_PORTS_END


static INPUT_PORTS_START( jungler )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW1")      // Sound board
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")      // CPU board
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DSW2:4,5,6")
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Test (255 lives)" )      PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( locomotn )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW1")      // Sound board
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "255" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "DSW1:3")
	PORT_DIPNAME( 0x02, 0x02, "Intermissions" )     PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")      // CPU board
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DSW2:1,2,3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DSW2:5,6,7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "No Coin B" )
	// "No Coin B" = coins produce sound, but no effect on coin counter
INPUT_PORTS_END


static INPUT_PORTS_START( tactcian )
	PORT_INCLUDE( locomotn )

		PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_MODIFY("DSW1")      // Sound board
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x30, "255" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW1:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )        PORT_CONDITION("DSW2", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW2", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW2", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW2", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW2", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x04, "A 2C/1C  B 1C/3C" )      PORT_CONDITION("DSW2", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW2", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x06, "A 1C/1C  B 1C/6C" )      PORT_CONDITION("DSW2", 0x01, EQUALS, 0x01)
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x00, "10k, 80k then every 100k" )
	PORT_DIPSETTING(    0x01, "20k, 80k then every 100k" )

	PORT_MODIFY("DSW2")      // CPU board
	PORT_DIPNAME( 0x01, 0x00, "Coin Mode" )         PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x00, "Mode 1" )
	PORT_DIPSETTING(    0x01, "Mode 2" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "DSW2:2")
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "DSW2:3")
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "DSW2:4")
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "DSW2:5")
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "DSW2:6")
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "DSW2:7")
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "DSW2:8")
INPUT_PORTS_END


static INPUT_PORTS_START( commsega )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW1")      // (sound board)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_DIPLOCATION("DSW1:7")
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "DSW1:6")
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "DSW1:5")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:4") // "Infinite Lives" - See notes
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("DSW2")      // (CPU board)
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DSW2:3,4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )            // Bonus Life : 50000 points
	PORT_DIPSETTING(    0x14, DEF_STR( 3C_1C ) )            // Bonus Life : 50000 points
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )            // Bonus Life : 30000 points
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )            // Bonus Life : 30000 points
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )            // Bonus Life : 50000 points
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )            // Bonus Life : 30000 points
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )            // Bonus Life : 30000 points
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )        // Bonus Life : 50000 points
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "DSW2:6")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:7") // Check code at 0x1fc5
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )             // 16 flying enemies to kill
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )             // 24 flying enemies to kill
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout rallyx_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout jungler_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout rallyx_spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
				24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static const gfx_layout jungler_spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static const gfx_layout dotlayout =
{
	4,4,
	8,
	2,
	{ 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8 },
	{ 0*32, 1*32, 2*32, 3*32 },
	16*8
};

static GFXDECODE_START( gfx_rallyx )
	GFXDECODE_ENTRY( "gfx1", 0, rallyx_charlayout,     0, 64 )
	GFXDECODE_ENTRY( "gfx1", 0, rallyx_spritelayout,   0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, dotlayout,         64*4,  1 )
GFXDECODE_END

static GFXDECODE_START( gfx_jungler )
	GFXDECODE_ENTRY( "gfx1", 0, jungler_charlayout,    0, 64 )
	GFXDECODE_ENTRY( "gfx1", 0, jungler_spritelayout,  0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, dotlayout,          64*4,  1 )
GFXDECODE_END


/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const char *const rallyx_sample_names[] =
{
	"*rallyx",
	"bang",
	nullptr   // end of array
};

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void rallyx_state::machine_start()
{
	m_interrupt_vector = 0;

	save_item(NAME(m_last_bang));
	save_item(NAME(m_stars_enable));
	save_item(NAME(m_main_irq_mask));
	save_item(NAME(m_interrupt_vector));
}

void rallyx_state::rallyx_vblank_irq(int state)
{
	if (state && m_main_irq_mask)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}

void rallyx_state::jungler_vblank_irq(int state)
{
	if (state && m_main_irq_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void rallyx_state::rallyx(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK/6);    // 3.072 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &rallyx_state::rallyx_map);
	m_maincpu->set_addrmap(AS_IO, &rallyx_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(rallyx_state::interrupt_vector_r));

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 259 at 12M or 4099 at 11M on Logic Board I
	mainlatch.q_out_cb<0>().set(FUNC(rallyx_state::bang_w)); // BANG
	mainlatch.q_out_cb<1>().set(FUNC(rallyx_state::irq_mask_w)); // INT ON
	mainlatch.q_out_cb<2>().set(FUNC(rallyx_state::sound_on_w)); // SOUND ON
	mainlatch.q_out_cb<3>().set(FUNC(rallyx_state::flip_screen_set)); // FLIP
	mainlatch.q_out_cb<4>().set_output("led0");
	mainlatch.q_out_cb<5>().set_output("led1");
	mainlatch.q_out_cb<6>().set(FUNC(rallyx_state::coin_lockout_w));
	mainlatch.q_out_cb<7>().set(FUNC(rallyx_state::coin_counter_1_w));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK/3, 48*8, 0*8, 36*8, 33*8, 2*8, 30*8);
	m_screen->set_screen_update(FUNC(rallyx_state::screen_update_rallyx));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(rallyx_state::rallyx_vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rallyx);

	PALETTE(config, m_palette, FUNC(rallyx_state::rallyx_palette), 64*4 + 4, 32);
	m_palette->enable_shadows();

	MCFG_VIDEO_START_OVERRIDE(rallyx_state,rallyx)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	NAMCO(config, m_namco_sound, MASTER_CLOCK/6/32); // 96 KHz
	m_namco_sound->set_voices(3);
	m_namco_sound->add_route(ALL_OUTPUTS, "mono", 1.0);

	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(rallyx_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.80);
}

void rallyx_state::jungler(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK/6);    // 3.072 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &rallyx_state::jungler_map);

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 1C on Loco-Motion
	mainlatch.q_out_cb<0>().set("timeplt_audio", FUNC(timeplt_audio_device::sh_irqtrigger_w)); // SOUNDON
	mainlatch.q_out_cb<1>().set(FUNC(rallyx_state::nmi_mask_w)); // INTST
	mainlatch.q_out_cb<2>().set("timeplt_audio", FUNC(timeplt_audio_device::mute_w)); // MUT
	mainlatch.q_out_cb<3>().set(FUNC(rallyx_state::flip_screen_set)); // FLIP
	mainlatch.q_out_cb<4>().set(FUNC(rallyx_state::coin_counter_1_w)); // OUT1
	mainlatch.q_out_cb<5>().set_nop(); // OUT2
	mainlatch.q_out_cb<6>().set(FUNC(rallyx_state::coin_counter_2_w)); // OUT3
	mainlatch.q_out_cb<7>().set(FUNC(rallyx_state::stars_enable_w)); // STARSON

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK/3, 48*8, 0*8, 36*8, 33*8, 2*8, 30*8);
	m_screen->set_screen_update(FUNC(rallyx_state::screen_update_jungler));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(rallyx_state::jungler_vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_jungler);

	PALETTE(config, m_palette, FUNC(rallyx_state::jungler_palette), 64*4 + 4 + 64, 32 + 64);
	m_palette->enable_shadows();

	MCFG_VIDEO_START_OVERRIDE(rallyx_state,jungler)

	// sound hardware
	LOCOMOTN_AUDIO(config, "timeplt_audio");
}

void rallyx_state::tactcian(machine_config &config)
{
	jungler(config);

	m_screen->set_screen_update(FUNC(rallyx_state::screen_update_locomotn));

	MCFG_VIDEO_START_OVERRIDE(rallyx_state,locomotn)
}

void rallyx_state::locomotn(machine_config &config)
{
	jungler(config);

	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(rallyx_state::screen_update_locomotn));

	MCFG_VIDEO_START_OVERRIDE(rallyx_state,locomotn)
}

void rallyx_state::commsega(machine_config &config)
{
	jungler(config);

	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(rallyx_state::screen_update_locomotn));

	MCFG_VIDEO_START_OVERRIDE(rallyx_state,commsega)
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( rallyx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1b",           0x0000, 0x1000, CRC(5882700d) SHA1(b6029e9730f1694894fe8b729ac0ba8d6712dea9) )
	ROM_LOAD( "rallyxn.1e",   0x1000, 0x1000, CRC(ed1eba2b) SHA1(82d3a4b34b0ff5cfdb8ca7c18ad5c63d943b8484) )
	ROM_LOAD( "rallyxn.1h",   0x2000, 0x1000, CRC(4f98dd1c) SHA1(8a20fadcea76802d1c412ba62086abb846ad54a8) )
	ROM_LOAD( "rallyxn.1k",   0x3000, 0x1000, CRC(9aacccf0) SHA1(9b22079972c0f9970d62d62751db4783a87796d5) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "8e",           0x0000, 0x1000, CRC(277c1de5) SHA1(30bc57263e8dad870c501c76bce6f42d69ab9e00) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "rx1-6.8m",     0x0000, 0x0100, CRC(3c16f62c) SHA1(7a3800be410e306cf85753b9953ffc5575afbcd6) )  // PROM type: IM5623    - dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "rx1-1.11n",    0x0000, 0x0020, CRC(c7865434) SHA1(70c1c9610ba6f1ead77f347e7132958958bccb31) )  // PROM type: M3-7603-5 - palette
	ROM_LOAD( "rx1-7.8p",     0x0020, 0x0100, CRC(834d4fda) SHA1(617864d3df0917a513e8255ad8d96ae7a04da5a1) )  // PROM type: IM5623    - lookup table
	ROM_LOAD( "rx1-2.4n",     0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) )  // PROM type: N82S123N  - video layout (not used)
	ROM_LOAD( "rx1-3.7k",     0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) )  // PROM type: M3-7603-5 - video timing (not used)

	ROM_REGION( 0x0200, "namco", 0 ) // sound PROMs
	ROM_LOAD( "rx1-5.3p",     0x0000, 0x0100, CRC(4bad7017) SHA1(3e6da9d798f5e07fa18d6ce7d0b148be98c766d5) )  // PROM type: IM5623
	ROM_LOAD( "rx1-4.2m",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // PROM type: IM5623 - not used
ROM_END

ROM_START( rallyxa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rx1_prg_1.1b", 0x0000, 0x0800, CRC(ef9238db) SHA1(44313381652916a59a8d9959fb170184578472af) )
	ROM_LOAD( "rx1_prg_2.1c", 0x0800, 0x0800, CRC(7cbeb656) SHA1(ff6e669f7d3e91c1cc835106cccefcd81aa28bb8) )
	ROM_LOAD( "rx1_prg_3.1d", 0x1000, 0x0800, CRC(334b1042) SHA1(348a303eb8f03f19e5060d81f733d3145113abd5) )
	ROM_LOAD( "rx1_prg_4.1e", 0x1800, 0x0800, CRC(d6618add) SHA1(4c66160996f3195a83628f486789721935d2cf5b) )
	ROM_LOAD( "rx1_prg_5.bin",0x2000, 0x0800, CRC(3d69f24e) SHA1(fe5a43b7144f62d28aaf0dd92e1d02ef9199b132) )
	ROM_LOAD( "rx1_prg_6.bin",0x2800, 0x0800, CRC(e9740f16) SHA1(02a134ccd3d6557d46492747b04da02e933aa6b4) )
	ROM_LOAD( "rx1_prg_7.1k", 0x3000, 0x0800, CRC(843109f2) SHA1(7241d1025f249d23a0d15b5e31fdb2f5297ffbf4) )
	ROM_LOAD( "rx1_prg_8.1l", 0x3800, 0x0800, CRC(9b846ec9) SHA1(1fd8cce517f31a15e06cf250bc50b5a663424877) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "rx1_chg_1.8e", 0x0000, 0x0800, CRC(1fff38a4) SHA1(5f6ccce2e0daad5915d017e8d067f187eb2ed41d) )
	ROM_LOAD( "rx1_chg_2.8d", 0x0800, 0x0800, CRC(68dff552) SHA1(5dad38db45afbd79b5627a75b295fc920ad68856) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "rx1-6.8m",     0x0000, 0x0100, CRC(3c16f62c) SHA1(7a3800be410e306cf85753b9953ffc5575afbcd6) )  // PROM type: IM5623    - dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "rx1-1.11n",    0x0000, 0x0020, CRC(c7865434) SHA1(70c1c9610ba6f1ead77f347e7132958958bccb31) )  // PROM type: M3-7603-5 - palette
	ROM_LOAD( "rx1-7.8p",     0x0020, 0x0100, CRC(834d4fda) SHA1(617864d3df0917a513e8255ad8d96ae7a04da5a1) )  // PROM type: IM5623    - lookup table
	ROM_LOAD( "rx1-2.4n",     0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) )  // PROM type: N82S123N  - video layout (not used)
	ROM_LOAD( "rx1-3.7k",     0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) )  // PROM type: M3-7603-5 - video timing (not used)

	ROM_REGION( 0x0200, "namco", 0 ) // sound PROMs
	ROM_LOAD( "rx1-5.3p",     0x0000, 0x0100, CRC(4bad7017) SHA1(3e6da9d798f5e07fa18d6ce7d0b148be98c766d5) )  // PROM type: IM5623
	ROM_LOAD( "rx1-4.2m",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // PROM type: IM5623 - not used
ROM_END

ROM_START( rallyxm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1b",           0x0000, 0x1000, CRC(5882700d) SHA1(b6029e9730f1694894fe8b729ac0ba8d6712dea9) )
	ROM_LOAD( "1e",           0x1000, 0x1000, CRC(786585ec) SHA1(8aa75f10d695f4b3483c4bf7030b733318fd3bf3) )
	ROM_LOAD( "1h",           0x2000, 0x1000, CRC(110d7dcd) SHA1(23e0855c2c9300f2068711d160fcdfaedd07832f) )
	ROM_LOAD( "1k",           0x3000, 0x1000, CRC(473ab447) SHA1(f0a37ccc48c97c53672f754ca2ac37dc0dc91a9f) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "8e",           0x0000, 0x1000, CRC(277c1de5) SHA1(30bc57263e8dad870c501c76bce6f42d69ab9e00) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "rx1-6.8m",     0x0000, 0x0100, CRC(3c16f62c) SHA1(7a3800be410e306cf85753b9953ffc5575afbcd6) )  // PROM type: IM5623    - dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "rx1-1.11n",    0x0000, 0x0020, CRC(c7865434) SHA1(70c1c9610ba6f1ead77f347e7132958958bccb31) )  // PROM type: M3-7603-5 - palette
	ROM_LOAD( "rx1-7.8p",     0x0020, 0x0100, CRC(834d4fda) SHA1(617864d3df0917a513e8255ad8d96ae7a04da5a1) )  // PROM type: IM5623    - lookup table
	ROM_LOAD( "rx1-2.4n",     0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) )  // PROM type: N82S123N  - video layout (not used)
	ROM_LOAD( "rx1-3.7k",     0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) )  // PROM type: M3-7603-5 - video timing (not used)

	ROM_REGION( 0x0200, "namco", 0 ) // sound PROMs
	ROM_LOAD( "rx1-5.3p",     0x0000, 0x0100, CRC(4bad7017) SHA1(3e6da9d798f5e07fa18d6ce7d0b148be98c766d5) )  // PROM type: IM5623
	ROM_LOAD( "rx1-4.2m",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // PROM type: IM5623 - not used
ROM_END

ROM_START( rallyxmr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "166.bin",      0x0000, 0x0800, CRC(ef9238db) SHA1(44313381652916a59a8d9959fb170184578472af) )
	ROM_LOAD( "167.bin",      0x0800, 0x0800, CRC(7cbeb656) SHA1(ff6e669f7d3e91c1cc835106cccefcd81aa28bb8) )
	ROM_LOAD( "168.bin",      0x1000, 0x0800, CRC(334b1042) SHA1(348a303eb8f03f19e5060d81f733d3145113abd5) )
	ROM_LOAD( "169.bin",      0x1800, 0x0800, CRC(b4852b52) SHA1(bf82ab1db49811114d16cf2cb5a318b98c07603c) )
	ROM_LOAD( "170.bin",      0x2000, 0x0800, CRC(3d69f24e) SHA1(fe5a43b7144f62d28aaf0dd92e1d02ef9199b132) )
	ROM_LOAD( "171.bin",      0x2800, 0x0800, CRC(e9740f16) SHA1(02a134ccd3d6557d46492747b04da02e933aa6b4) )
	ROM_LOAD( "172.bin",      0x3000, 0x0800, CRC(843109f2) SHA1(7241d1025f249d23a0d15b5e31fdb2f5297ffbf4) )
	ROM_LOAD( "173.bin",      0x3800, 0x0800, CRC(3b5b1a81) SHA1(5aa4dd850283062113181674849d531a2908340c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "175.bin",      0x0000, 0x0800, CRC(50a224e2) SHA1(33da1bdc33f085d19ae2c482747c509cf9441674) )
	ROM_LOAD( "174.bin",      0x0800, 0x0800, CRC(68dff552) SHA1(5dad38db45afbd79b5627a75b295fc920ad68856) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "rx1-6.8m",     0x0000, 0x0100, CRC(3c16f62c) SHA1(7a3800be410e306cf85753b9953ffc5575afbcd6) )  // PROM type: IM5623    - dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "rx1-1.11n",    0x0000, 0x0020, CRC(c7865434) SHA1(70c1c9610ba6f1ead77f347e7132958958bccb31) )  // PROM type: M3-7603-5 - palette
	ROM_LOAD( "rx1-7.8p",     0x0020, 0x0100, CRC(834d4fda) SHA1(617864d3df0917a513e8255ad8d96ae7a04da5a1) )  // PROM type: IM5623    - lookup table
	ROM_LOAD( "rx1-2.4n",     0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) )  // PROM type: N82S123N  - video layout (not used)
	ROM_LOAD( "rx1-3.7k",     0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) )  // PROM type: M3-7603-5 - video timing (not used)

	ROM_REGION( 0x0200, "namco", 0 ) // sound PROMs
	ROM_LOAD( "rx1-5.3p",     0x0000, 0x0100, CRC(4bad7017) SHA1(3e6da9d798f5e07fa18d6ce7d0b148be98c766d5) )  // PROM type: IM5623
	ROM_LOAD( "rx1-4.2m",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // PROM type: IM5623 - not used
ROM_END

ROM_START( rallyxeg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6101_2716.1b", 0x0000, 0x0800, CRC(921699dc) SHA1(5d242c0f0b2a041dc7e7c2aaa07fcf1edbac02af) )
	ROM_LOAD( "6102_2716.1c", 0x0800, 0x0800, CRC(7cbeb656) SHA1(ff6e669f7d3e91c1cc835106cccefcd81aa28bb8) )
	ROM_LOAD( "6103_2716.1e", 0x1000, 0x0800, CRC(36e9918f) SHA1(2fe6c9f672d3dcaefa13f2035a2cd2f838e609f7) )
	ROM_LOAD( "6104_2716.1f", 0x1800, 0x0800, CRC(b986fcf8) SHA1(bcdf6d0627c9c8310cde2a827006d776c99aa1fc) )
	ROM_LOAD( "6105_2716.1h", 0x2000, 0x0800, CRC(1fb0ced8) SHA1(38916ece476283f3f61309005771b7a833ba9053) )
	ROM_LOAD( "6106_2716.1j", 0x2800, 0x0800, CRC(d72ee519) SHA1(81f4743b6a72e59900851462d643ffdc7dd2dd64) )
	ROM_LOAD( "6107_2716.1k", 0x3000, 0x0800, CRC(843109f2) SHA1(7241d1025f249d23a0d15b5e31fdb2f5297ffbf4) )
	ROM_LOAD( "6108_2716.1m", 0x3800, 0x0800, CRC(8ab078ef) SHA1(3aab0f898d76724362a793ef2d130f2b8d4ed936) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "10_2716.8e",   0x0000, 0x0800, CRC(836fb24f) SHA1(bf09d6c428541e2098f3ec1ca72882f8ad8391bb) )
	ROM_LOAD( "9_2716.8d",    0x0800, 0x0800, CRC(22d7113b) SHA1(a9e0bc9761613645160f4cbcc7b72252641f4f57) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "r6_82s129.8m", 0x0000, 0x0100, CRC(3c16f62c) SHA1(7a3800be410e306cf85753b9953ffc5575afbcd6) )

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "r1_82s123.1n", 0x0000, 0x0020, CRC(c7865434) SHA1(70c1c9610ba6f1ead77f347e7132958958bccb31) )
	ROM_LOAD( "r7_82s129.8p", 0x0020, 0x0100, CRC(834d4fda) SHA1(617864d3df0917a513e8255ad8d96ae7a04da5a1) )
	ROM_LOAD( "r2_82s123.4n", 0x0120, 0x0020, CRC(659c3f5d) SHA1(1d668fb00148ce526c978fefb344262cf2a7271c) )
	ROM_LOAD( "r3_82s123.7k", 0x0140, 0x0020, CRC(bdef006f) SHA1(9bcac4637a9c1638c18c11fd93c697a0fc514c97) )

	ROM_REGION( 0x0200, "namco", 0 ) // sound PROMs
	ROM_LOAD( "r5_82s129.3p", 0x0000, 0x0100, CRC(4bad7017) SHA1(3e6da9d798f5e07fa18d6ce7d0b148be98c766d5) )
	ROM_LOAD( "r4_82s129.2m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
ROM_END

ROM_START( dngrtrck ) // PROMs weren't dumped for this PCB, supposed to match
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1b-2716.bin",  0x0000, 0x0800, CRC(b6180a12) SHA1(f442fe81f7fac6e915944640c763d7016a6577f6) )
	ROM_LOAD( "1c-2716.bin",  0x0800, 0x0800, CRC(7cbeb656) SHA1(ff6e669f7d3e91c1cc835106cccefcd81aa28bb8) )
	ROM_LOAD( "1d-2716.bin",  0x1000, 0x0800, CRC(594207b1) SHA1(addea746e2378c44c06d6d18d466138946a339cf) )
	ROM_LOAD( "1e-2716.bin",  0x1800, 0x0800, CRC(ae447251) SHA1(7c86193c9418929662b35143c66a5dff44663dd7) )
	ROM_LOAD( "1h-2716.bin",  0x2000, 0x0800, CRC(e0d4b534) SHA1(fb64b21c418a2633d592f4476aed909ea6678fb2) )
	ROM_LOAD( "1j-2716.bin",  0x2800, 0x0800, CRC(e9740f16) SHA1(02a134ccd3d6557d46492747b04da02e933aa6b4) )
	ROM_LOAD( "1k-2716.bin",  0x3000, 0x0800, CRC(843109f2) SHA1(7241d1025f249d23a0d15b5e31fdb2f5297ffbf4) )
	ROM_LOAD( "1l-2716.bin",  0x3800, 0x0800, CRC(17759749) SHA1(8169eebcb02615b99f786f6c5294eb31a8d1911b) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "8e-2716.bin",  0x0000, 0x0800, CRC(50a224e2) SHA1(33da1bdc33f085d19ae2c482747c509cf9441674) )
	ROM_LOAD( "8d-2716.bin",  0x0800, 0x0800, CRC(68dff552) SHA1(5dad38db45afbd79b5627a75b295fc920ad68856) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "rx1-6.8m",     0x0000, 0x0100, CRC(3c16f62c) SHA1(7a3800be410e306cf85753b9953ffc5575afbcd6) )  // PROM type: IM5623    - dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "rx1-1.11n",    0x0000, 0x0020, CRC(c7865434) SHA1(70c1c9610ba6f1ead77f347e7132958958bccb31) )  // PROM type: M3-7603-5 - palette
	ROM_LOAD( "rx1-7.8p",     0x0020, 0x0100, CRC(834d4fda) SHA1(617864d3df0917a513e8255ad8d96ae7a04da5a1) )  // PROM type: IM5623    - lookup table
	ROM_LOAD( "rx1-2.4n",     0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) )  // PROM type: N82S123N  - video layout (not used)
	ROM_LOAD( "rx1-3.7k",     0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) )  // PROM type: M3-7603-5 - video timing (not used)

	ROM_REGION( 0x0200, "namco", 0 ) // sound PROMs
	ROM_LOAD( "rx1-5.3p",     0x0000, 0x0100, CRC(4bad7017) SHA1(3e6da9d798f5e07fa18d6ce7d0b148be98c766d5) )  // PROM type: IM5623
	ROM_LOAD( "rx1-4.2m",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // PROM type: IM5623 - not used
ROM_END

ROM_START( nrallyx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nrx_prg1.1d",  0x0000, 0x0800, CRC(ba7de9fc) SHA1(2133ca327589600bcbd796c213f034daa0457f72) )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_LOAD( "nrx_prg2.1e",  0x0800, 0x0800, CRC(eedfccae) SHA1(9fca8500f724864a2b73e38bd40cbaeef41617d7) )
	ROM_CONTINUE(             0x1800, 0x0800 )
	ROM_LOAD( "nrx_prg3.1k",  0x2000, 0x0800, CRC(b4d5d34a) SHA1(c533470aac040b3471d79fd6d35beb4fd4b5bb19) )
	ROM_CONTINUE(             0x3000, 0x0800 )
	ROM_LOAD( "nrx_prg4.1l",  0x2800, 0x0800, CRC(7da5496d) SHA1(ffac2c07dda57285673073266712fa2987e3b34f) )
	ROM_CONTINUE(             0x3800, 0x0800 )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "nrx_chg1.8e",  0x0000, 0x0800, CRC(1fff38a4) SHA1(5f6ccce2e0daad5915d017e8d067f187eb2ed41d) )
	ROM_LOAD( "nrx_chg2.8d",  0x0800, 0x0800, CRC(85d9fffd) SHA1(12dff66d98a808b9dc952b2d87a56308b46a973e) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "rx1-6.8m",     0x0000, 0x0100, CRC(3c16f62c) SHA1(7a3800be410e306cf85753b9953ffc5575afbcd6) )  // PROM type: IM5623    - dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "nrx1-1.11n",   0x0000, 0x0020, CRC(a0a49017) SHA1(494c920a157e9f876d533c1b0146275a366c4989) )  // PROM type: M3-7603-5 - palette
	ROM_LOAD( "nrx1-7.8p",    0x0020, 0x0100, CRC(4e46f485) SHA1(3f013aafba96a76d410f2db16d1d24d2fb257aaf) )  // PROM type: IM5623    - lookup table
	ROM_LOAD( "rx1-2.4n",     0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) )  // PROM type: N82S123N  - video layout (not used)
	ROM_LOAD( "rx1-3.7k",     0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) )  // PROM type: M3-7603-5 - video timing (not used)

	ROM_REGION( 0x0200, "namco", 0 ) // sound PROMs
	ROM_LOAD( "rx1-5.3p",     0x0000, 0x0100, CRC(4bad7017) SHA1(3e6da9d798f5e07fa18d6ce7d0b148be98c766d5) )  // PROM type: IM5623
	ROM_LOAD( "rx1-4.2m",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // PROM type: IM5623 - not used
ROM_END

ROM_START( nrallyxb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nrallyx.1b",   0x0000, 0x1000, CRC(9404c8d6) SHA1(ee7e45c22a2fbf72d3ac5ac26ab1111a22623fc5) )
	ROM_LOAD( "nrallyx.1e",   0x1000, 0x1000, CRC(ac01bf3f) SHA1(8e1a7cce92ef709d18727db6ee7f89936f4b8df8) )
	ROM_LOAD( "nrallyx.1h",   0x2000, 0x1000, CRC(aeba29b5) SHA1(2a6e4568729b83c430bf70e43c4146ad6a556b1b) )
	ROM_LOAD( "nrallyx.1k",   0x3000, 0x1000, CRC(78f17da7) SHA1(1e035746a10f91e898166a58093d45bdb158ae47) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "nrallyx.8e",   0x0000, 0x1000, CRC(ca7a174a) SHA1(dc553df18c45ba399661122be75b71d6cb54d6a2) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "rx1-6.8m",     0x0000, 0x0100, CRC(3c16f62c) SHA1(7a3800be410e306cf85753b9953ffc5575afbcd6) )  // PROM type: IM5623    - dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "nrx1-1.11n",   0x0000, 0x0020, CRC(a0a49017) SHA1(494c920a157e9f876d533c1b0146275a366c4989) )  // PROM type: M3-7603-5 - palette
	ROM_LOAD( "nrx1-7.8p",    0x0020, 0x0100, CRC(4e46f485) SHA1(3f013aafba96a76d410f2db16d1d24d2fb257aaf) )  // PROM type: IM5623    - lookup table
	ROM_LOAD( "rx1-2.4n",     0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) )  // PROM type: N82S123N  - video layout (not used)
	ROM_LOAD( "rx1-3.7k",     0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) )  // PROM type: M3-7603-5 - video timing (not used)

	ROM_REGION( 0x0200, "namco", 0 ) // sound PROMs
	ROM_LOAD( "rx1-5.3p",     0x0000, 0x0100, CRC(4bad7017) SHA1(3e6da9d798f5e07fa18d6ce7d0b148be98c766d5) )  // PROM type: IM5623
	ROM_LOAD( "rx1-4.2m",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // PROM type: IM5623 - not used
ROM_END

ROM_START( jungler )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jungr1",       0x0000, 0x1000, CRC(5bd6ad15) SHA1(608de86e19c6726bb7d21e7dc0e936f00121a3f4) )
	ROM_LOAD( "jungr2",       0x1000, 0x1000, CRC(dc99f1e3) SHA1(942405f6c7d816139e36289126fe883a6a9a0a08) )
	ROM_LOAD( "jungr3",       0x2000, 0x1000, CRC(3dcc03da) SHA1(2c328a46511c4c9eec6515b9316a586de6503152) )
	ROM_LOAD( "jungr4",       0x3000, 0x1000, CRC(f92e9940) SHA1(d72a4d0a0ab7c9a1dcbb7925eb8530052640a234) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "1b",           0x0000, 0x1000, CRC(f86999c3) SHA1(4660bd7826219b1bad7d9178918823196d4fd8d6) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5k",           0x0000, 0x0800, CRC(924262bf) SHA1(593f59630b3bd369aef0819992106b4e6e6a241f) )
	ROM_LOAD( "5m",           0x0800, 0x0800, CRC(131a08ac) SHA1(167a0710a2a153f7f7c6839d2340e5aa725ef039) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "82s129.10g",   0x0000, 0x0100, CRC(c59c51b7) SHA1(e8ac60fed9ba16c61a4c3c09e27f8c3f4e254014) ) // dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "18s030.8b",    0x0000, 0x0020, CRC(55a7e6d1) SHA1(f9e4ff3b165235db2fd8dab94c43bc686c3ad29b) ) // palette
	ROM_LOAD( "tbp24s10.9d",  0x0020, 0x0100, CRC(d223f7b8) SHA1(87b62f09d4eda09c16d99d1554017d18e52b5886) ) // lookup table
	ROM_LOAD( "18s030.7a",    0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) ) // video layout (not used)
	ROM_LOAD( "6331-1.10a",   0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END

ROM_START( junglers )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5c",           0x0000, 0x1000, CRC(edd71b28) SHA1(6bdd85bc1c24ca57573252fd636e05759164de8a) )
	ROM_LOAD( "5a",           0x1000, 0x1000, CRC(61ea4d46) SHA1(575ffe9fc7d5777c8f2d2b449623c353f42a4249) )
	ROM_LOAD( "4d",           0x2000, 0x1000, CRC(557c7925) SHA1(84d8eb2fdb7ee9098805be9f457a37f51e4bc3b8) )
	ROM_LOAD( "4c",           0x3000, 0x1000, CRC(51aac9a5) SHA1(2c8a24b4ce8cec96c6e09332f3f63bd7d25ae4c6) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "1b",           0x0000, 0x1000, CRC(f86999c3) SHA1(4660bd7826219b1bad7d9178918823196d4fd8d6) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5k",           0x0000, 0x0800, CRC(924262bf) SHA1(593f59630b3bd369aef0819992106b4e6e6a241f) )
	ROM_LOAD( "5m",           0x0800, 0x0800, CRC(131a08ac) SHA1(167a0710a2a153f7f7c6839d2340e5aa725ef039) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "82s129.10g",   0x0000, 0x0100, CRC(c59c51b7) SHA1(e8ac60fed9ba16c61a4c3c09e27f8c3f4e254014) ) // dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "18s030.8b",    0x0000, 0x0020, CRC(55a7e6d1) SHA1(f9e4ff3b165235db2fd8dab94c43bc686c3ad29b) ) // palette
	ROM_LOAD( "tbp24s10.9d",  0x0020, 0x0100, CRC(d223f7b8) SHA1(87b62f09d4eda09c16d99d1554017d18e52b5886) ) // lookup table
	ROM_LOAD( "18s030.7a",    0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) ) // video layout (not used)
	ROM_LOAD( "6331-1.10a",   0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END

// main PCB is marked: "OLYMPIA", "[C] 13-53", "Konami LICENSED", "CE" and "22 A" on solder side
// sound PCB is marked: "OLYMPIA", "[C] 15-82", "Konami LICENSED", "CE" and "22 B" on component side
ROM_START( junglero )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "j1.bin",       0x0000, 0x1000, CRC(5bd6ad15) SHA1(608de86e19c6726bb7d21e7dc0e936f00121a3f4) )
	ROM_LOAD( "j2.bin",       0x1000, 0x1000, CRC(dc99f1e3) SHA1(942405f6c7d816139e36289126fe883a6a9a0a08) )
	ROM_LOAD( "j3.bin",       0x2000, 0x1000, CRC(3dcc03da) SHA1(2c328a46511c4c9eec6515b9316a586de6503152) )
	ROM_LOAD( "j4.bin",       0x3000, 0x1000, CRC(f92e9940) SHA1(d72a4d0a0ab7c9a1dcbb7925eb8530052640a234) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "js1.bin",      0x0000, 0x1000, CRC(f86999c3) SHA1(4660bd7826219b1bad7d9178918823196d4fd8d6) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "j5.bin",       0x0000, 0x0800, CRC(4190c6c0) SHA1(ebd3b5b0e6660045f1ee84006536fa31cb3d5f8e) )
	ROM_IGNORE(0x800)
	ROM_LOAD( "j6.bin",       0x0800, 0x0800, CRC(5c001c66) SHA1(aab8342131f831cb9bab4258488a0f666c35ee4d) )
	ROM_IGNORE(0x800)

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "am27s21dc.jn4",0x0000, 0x0100, CRC(c59c51b7) SHA1(e8ac60fed9ba16c61a4c3c09e27f8c3f4e254014) ) // dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "dm74s288n.jn2",0x0000, 0x0020, CRC(55a7e6d1) SHA1(f9e4ff3b165235db2fd8dab94c43bc686c3ad29b) ) // palette
	ROM_LOAD( "tbp24s10.jn5", 0x0020, 0x0100, CRC(d223f7b8) SHA1(87b62f09d4eda09c16d99d1554017d18e52b5886) ) // lookup table
	ROM_LOAD( "dm74s288n.jn1",0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) ) // video layout (not used)
	ROM_LOAD( "mb7051.jn3",   0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END

ROM_START( jackler ) // Board ID SL-HA-2061-21-B
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jackler_j1.r1",0x0000, 0x1000, CRC(3fc0d149) SHA1(d2d8273d57e26ebc97158549d5c7dada78bf2ae4) )
	ROM_LOAD( "jackler_j2.r2",0x1000, 0x1000, CRC(5f482c7d) SHA1(5111f114c8427271d4641a55c88e54853e82aa50) )
	ROM_LOAD( "jungr3",       0x2000, 0x1000, CRC(3dcc03da) SHA1(2c328a46511c4c9eec6515b9316a586de6503152) ) // jackler_j2.r2
	ROM_LOAD( "jungr4",       0x3000, 0x1000, CRC(f92e9940) SHA1(d72a4d0a0ab7c9a1dcbb7925eb8530052640a234) ) // jackler_j3.r3

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "1b",           0x0000, 0x1000, CRC(f86999c3) SHA1(4660bd7826219b1bad7d9178918823196d4fd8d6) ) // jackler_j7_sound.1b

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "jackler_j5.r9",0x0000, 0x1000, CRC(4190c6c0) SHA1(ebd3b5b0e6660045f1ee84006536fa31cb3d5f8e) ) // Both are 2x original
	ROM_LOAD( "jackler_j6.r10",0x0800, 0x1000, CRC(5c001c66) SHA1(aab8342131f831cb9bab4258488a0f666c35ee4d) ) // so mapped to overlap

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "82s129.10g",   0x0000, 0x0100, CRC(c59c51b7) SHA1(e8ac60fed9ba16c61a4c3c09e27f8c3f4e254014) ) // dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "18s030.8b",    0x0000, 0x0020, CRC(55a7e6d1) SHA1(f9e4ff3b165235db2fd8dab94c43bc686c3ad29b) ) // palette
	ROM_LOAD( "tbp24s10.9d",  0x0020, 0x0100, CRC(d223f7b8) SHA1(87b62f09d4eda09c16d99d1554017d18e52b5886) ) // lookup table
	ROM_LOAD( "18s030.7a",    0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) ) // video layout (not used)
	ROM_LOAD( "6331-1.10a",   0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END

ROM_START( savanna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sav1.bin",     0x0000, 0x1000, CRC(958c57eb) SHA1(b804ef99bb2f5658de508d3f9f83ca491012a51f) )
	ROM_LOAD( "sav2.bin",     0x1000, 0x1000, CRC(61ea4d46) SHA1(575ffe9fc7d5777c8f2d2b449623c353f42a4249) )
	ROM_LOAD( "sav3.bin",     0x2000, 0x1000, CRC(557c7925) SHA1(84d8eb2fdb7ee9098805be9f457a37f51e4bc3b8) )
	ROM_LOAD( "sav4.bin",     0x3000, 0x1000, CRC(b38b6cbd) SHA1(76ab41097bceb3d73c95ab8a89df702e554ba403) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "1b",           0x0000, 0x1000, CRC(f86999c3) SHA1(4660bd7826219b1bad7d9178918823196d4fd8d6) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5k",           0x0000, 0x0800, CRC(924262bf) SHA1(593f59630b3bd369aef0819992106b4e6e6a241f) )
	ROM_LOAD( "5m",           0x0800, 0x0800, CRC(131a08ac) SHA1(167a0710a2a153f7f7c6839d2340e5aa725ef039) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "82s129.10g",   0x0000, 0x0100, CRC(c59c51b7) SHA1(e8ac60fed9ba16c61a4c3c09e27f8c3f4e254014) ) // dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "18s030.8b",    0x0000, 0x0020, CRC(55a7e6d1) SHA1(f9e4ff3b165235db2fd8dab94c43bc686c3ad29b) ) // palette
	ROM_LOAD( "tbp24s10.9d",  0x0020, 0x0100, CRC(d223f7b8) SHA1(87b62f09d4eda09c16d99d1554017d18e52b5886) ) // lookup table
	ROM_LOAD( "18s030.7a",    0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) ) // video layout (not used)
	ROM_LOAD( "6331-1.10a",   0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END

ROM_START( tactcian )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tacticia.001", 0x0000, 0x1000, CRC(99163e39) SHA1(0a863f358a0bb065a9e2c41fcf4c20d370001dfe) )
	ROM_LOAD( "tacticia.002", 0x1000, 0x1000, CRC(6d3e8a69) SHA1(2b4b3f2b7401064540f59070ef6742d1f44ca839) )
	ROM_LOAD( "tacticia.003", 0x2000, 0x1000, CRC(0f71d0fa) SHA1(cb55243853b8b33034af7a6438f9a7c85a774d71) )
	ROM_LOAD( "tacticia.004", 0x3000, 0x1000, CRC(5e15f3b3) SHA1(01979f64b281a958f0a4effe2be21bf0e0a812bf) )
	ROM_LOAD( "tacticia.005", 0x4000, 0x1000, CRC(76456106) SHA1(580428f3c8cf442ee5c0f56db973644229aa8093) )
	ROM_LOAD( "tacticia.006", 0x5000, 0x1000, CRC(b33ca9ea) SHA1(0299c1cb9a3c6368bbbacb60c6f5c6854035a7bf) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "tacticia.s2",  0x0000, 0x1000, CRC(97d145a7) SHA1(7aee9004287590a25e153d45b95dfaac89fbe996) )
	ROM_LOAD( "tacticia.s1",  0x1000, 0x1000, CRC(067f781b) SHA1(640bc7813c239e497644e53a080d81366fcd04df) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tacticia.c1",  0x0000, 0x1000, CRC(5d3ee965) SHA1(654c033291f3d139fb94f7aacbc2d1917856deb6) )
	ROM_LOAD( "tacticia.c2",  0x1000, 0x1000, CRC(e8c59c4f) SHA1(e4881f2e2e08bb8af37cc679c4e2367528ac4804) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "tact6301.004", 0x0000, 0x0100, CRC(88b0b511) SHA1(785eded1ba761cdb59db579eb8a786516ff58152) ) // dots - tac.a7

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "tact6331.002", 0x0000, 0x0020, CRC(b7ef83b7) SHA1(5ffab25c2dc5be0856a43a93711d39c4aec6660b) ) // palette
	ROM_LOAD( "tact6301.003", 0x0020, 0x0100, CRC(a92796f2) SHA1(0faab2dc0f868f4023a34ecfcf972d1c86a224a0) ) // lookup table - tac.b4
	ROM_LOAD( "tact6331.001", 0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) ) // video layout (not used)
//  ROM_LOAD( "10a.bpr",      0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END

ROM_START( tactcian2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tan1",         0x0000, 0x1000, CRC(ddf38b75) SHA1(bad66fd6ae0ab3b91989fca14a8696ed855dc852) )
	ROM_LOAD( "tan2",         0x1000, 0x1000, CRC(f065ee2e) SHA1(f2362c471981af3348465f3c8a5ffb38058432a5) )
	ROM_LOAD( "tan3",         0x2000, 0x1000, CRC(2dba64fe) SHA1(8d312a6db99d2248fef2bbc590ceba333b0fde8b) )
	ROM_LOAD( "tan4",         0x3000, 0x1000, CRC(2ba07847) SHA1(3cd7cd0621ed930cb5955fc2ffe3239f6e176321) )
	ROM_LOAD( "tan5",         0x4000, 0x1000, CRC(1dae4c61) SHA1(70283b8412b0725f1c2acc281625c582a4fae39d) )
	ROM_LOAD( "tan6",         0x5000, 0x1000, CRC(2b36a18d) SHA1(bea8f36ec98975438ab267509bd9d1d1eb605945) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	// sound ROMs were missing - using the ones from the other set
	ROM_LOAD( "tacticia.s2",  0x0000, 0x1000, CRC(97d145a7) SHA1(7aee9004287590a25e153d45b95dfaac89fbe996) )
	ROM_LOAD( "tacticia.s1",  0x1000, 0x1000, CRC(067f781b) SHA1(640bc7813c239e497644e53a080d81366fcd04df) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "c1",           0x0000, 0x1000, CRC(5399471f) SHA1(66aea0df982ccbd6caaa24c258b2ba364bc1ecfd) )
	ROM_LOAD( "c2",           0x1000, 0x1000, CRC(8e8861e8) SHA1(38728418b09df06356c1e45a26cf438b93517ce5) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "tact6301.004", 0x0000, 0x0100, CRC(88b0b511) SHA1(785eded1ba761cdb59db579eb8a786516ff58152) ) // dots - tac.a7

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "tact6331.002", 0x0000, 0x0020, CRC(b7ef83b7) SHA1(5ffab25c2dc5be0856a43a93711d39c4aec6660b) ) // palette
	ROM_LOAD( "tact6301.003", 0x0020, 0x0100, CRC(a92796f2) SHA1(0faab2dc0f868f4023a34ecfcf972d1c86a224a0) ) // lookup table - tac.b4
	ROM_LOAD( "tact6331.001", 0x0120, 0x0020, CRC(8f574815) SHA1(4f84162db9d58b64742c67dc689eb665b9862fb3) ) // video layout (not used)
//  ROM_LOAD( "10a.bpr",      0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END

ROM_START( locomotn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1a.cpu",       0x0000, 0x1000, CRC(b43e689a) SHA1(7f1a0fa1ea9ff95a9d51b23ea00792ba22024282) )
	ROM_LOAD( "2a.cpu",       0x1000, 0x1000, CRC(529c823d) SHA1(714ae0af254646eb6ebc5f47422246832e89ccfb) )
	ROM_LOAD( "3.cpu",        0x2000, 0x1000, CRC(c9dbfbd1) SHA1(10ec7403053ef52d0ce4aa6eab3e82a3ea5e57ff) )
	ROM_LOAD( "4.cpu",        0x3000, 0x1000, CRC(caf6431c) SHA1(f013d8846fad9f64367b69febeb7512029a639c0) )
	ROM_LOAD( "5.cpu",        0x4000, 0x1000, CRC(64cf8dd6) SHA1(8fa1b5c4a7f136cb74833425a565fa558eeee083) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "1b_s1.bin",    0x0000, 0x1000, CRC(a1105714) SHA1(6e2e264748ab90bc5e8e8167f17ff91677ef6ae7) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5l_c1.bin",    0x0000, 0x1000, CRC(5732eda9) SHA1(451de30946a9c8198c5ec83cc5c50e3ac2f9f56b) )
	ROM_LOAD( "c2.cpu",       0x1000, 0x1000, CRC(c3035300) SHA1(ddb1d658a28b973b60e2ce72fd7662537e147860) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "10g.bpr",      0x0000, 0x0100, CRC(2ef89356) SHA1(5ed33386bab5d583358709c92f21ad9ad1a1bce9) ) // dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "8b.bpr",       0x0000, 0x0020, CRC(75b05da0) SHA1(aee98f5389e42332f30a6882ee22ff23f37e0573) ) // palette
	ROM_LOAD( "9d.bpr",       0x0020, 0x0100, CRC(aa6cf063) SHA1(08c1c9ab03eb168954b0170d40e95eed81022acd) ) // lookup table
	ROM_LOAD( "7a.bpr",       0x0120, 0x0020, CRC(48c8f094) SHA1(61592209720fddc8991751edf08b6950388af42e) ) // video layout (not used)
	ROM_LOAD( "10a.bpr",      0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END

ROM_START( gutangtn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bimm_001.r1",  0x0000, 0x1000, CRC(dfa2089c) SHA1(ef2280578c33589d2831b21e22b0786f23eea881) )
	ROM_LOAD( "bimm_002.r2",  0x1000, 0x1000, CRC(1de5e6a0) SHA1(8bb3408a510662ff3b9b7201d2d06fe70685bf7f) )
	ROM_LOAD( "bimm_003.r3",  0x2000, 0x1000, CRC(01f909fe) SHA1(c80295e9f91ce25bfd28e72823b20ee6f6524a5c) )
	ROM_LOAD( "bimm_004.r4",  0x3000, 0x1000, CRC(a89eb3e3) SHA1(058928ade909faba06f177750f914cf1dabaefc3) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "bimm_007.b1",  0x0000, 0x1000, CRC(3d83f6d3) SHA1(e10ed6b6ce7280697c1bc9dbe6c6e6018e1d8be4) )
	ROM_LOAD( "bimm_008.c1",  0x1000, 0x1000, CRC(323e1937) SHA1(75499d6c8a9032fac090a13cd4f36bd350f52dab) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "bimm_005.r9",  0x0000, 0x1000, CRC(992d079c) SHA1(b5acd30f2e8700cc4cd852b190bd1f4163b137e8) )
	ROM_LOAD( "bimm_006.r10", 0x1000, 0x1000, CRC(f0414f1d) SHA1(c7795184e2c1701eddb914d7134bf2830c4a3781) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "5.bpr",        0x0000, 0x0100, CRC(21fb583f) SHA1(b8c65fbdd5d8b70bf51341cd60fc2efeaab8bb82) ) // dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "2.bpr",        0x0000, 0x0020, CRC(26f42e6f) SHA1(f51578216a5d588c4d0143ce7a23d695a15a3914) ) // palette
	ROM_LOAD( "3.bpr",        0x0020, 0x0100, CRC(4aecc0c8) SHA1(3c1086a598d84b4bcb277556b716fd18c76c4364) ) // lookup table
	ROM_LOAD( "7a.bpr",       0x0120, 0x0020, CRC(48c8f094) SHA1(61592209720fddc8991751edf08b6950388af42e) ) // video layout (not used)
	ROM_LOAD( "10a.bpr",      0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END

ROM_START( gutangtns )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3d_1.bin",     0x0000, 0x1000, CRC(e9757395) SHA1(78e2f8988ed39d2ecfe1f874be370f603d5eecc1) )
	ROM_LOAD( "3e_2.bin",     0x1000, 0x1000, CRC(11d21d2e) SHA1(fd17dd481bb7bb39234fa7e9946b1cb4fa18109e) )
	ROM_LOAD( "3f_3.bin",     0x2000, 0x1000, CRC(4d80f895) SHA1(7d83f4ee34226636012a84f46af01991a28b96f6) )
	ROM_LOAD( "3h_4.bin",     0x3000, 0x1000, CRC(aa258ddf) SHA1(0f01ac0d72d8bb5a55c91a6fba3e55ed1c038b86) )
	ROM_LOAD( "3j_5.bin",     0x4000, 0x1000, CRC(52aec87e) SHA1(6516724c4e570972f070f6dab5b066ea92f56be0) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "1b_s1.bin",    0x0000, 0x1000, CRC(a1105714) SHA1(6e2e264748ab90bc5e8e8167f17ff91677ef6ae7) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5l_c1.bin",    0x0000, 0x1000, CRC(5732eda9) SHA1(451de30946a9c8198c5ec83cc5c50e3ac2f9f56b) )
	ROM_LOAD( "5m_c2.bin",    0x1000, 0x1000, CRC(51c542fd) SHA1(1437f8cba15811361b2c5b46085587ea3598fc88) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "10g.bpr",      0x0000, 0x0100, CRC(2ef89356) SHA1(5ed33386bab5d583358709c92f21ad9ad1a1bce9) ) // dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "8b.bpr",       0x0000, 0x0020, CRC(75b05da0) SHA1(aee98f5389e42332f30a6882ee22ff23f37e0573) ) // palette
	ROM_LOAD( "9d.bpr",       0x0020, 0x0100, CRC(aa6cf063) SHA1(08c1c9ab03eb168954b0170d40e95eed81022acd) ) // lookup table
	ROM_LOAD( "7a.bpr",       0x0120, 0x0020, CRC(48c8f094) SHA1(61592209720fddc8991751edf08b6950388af42e) ) // video layout (not used)
	ROM_LOAD( "10a.bpr",      0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END

ROM_START( cottong ) // bootleg based on gutangtn set
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c1",           0x0000, 0x1000, CRC(2c256fe6) SHA1(115594c616497eec998e4e3255ec6ab6299346fa) )
	ROM_LOAD( "c2",           0x1000, 0x1000, CRC(1de5e6a0) SHA1(8bb3408a510662ff3b9b7201d2d06fe70685bf7f) ) // == bimm_002.r2
	ROM_LOAD( "c3",           0x2000, 0x1000, CRC(01f909fe) SHA1(c80295e9f91ce25bfd28e72823b20ee6f6524a5c) ) // == bimm_003.r3
	ROM_LOAD( "c4",           0x3000, 0x1000, CRC(a89eb3e3) SHA1(058928ade909faba06f177750f914cf1dabaefc3) ) // == bimm_004.r4

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "c7",           0x0000, 0x1000, CRC(3d83f6d3) SHA1(e10ed6b6ce7280697c1bc9dbe6c6e6018e1d8be4) ) // == bimm_007.b1
	ROM_LOAD( "c8",           0x1000, 0x1000, CRC(323e1937) SHA1(75499d6c8a9032fac090a13cd4f36bd350f52dab) ) // == bimm_008.c1

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "c5",           0x0000, 0x1000, CRC(992d079c) SHA1(b5acd30f2e8700cc4cd852b190bd1f4163b137e8) ) // == bimm_005.r9
	ROM_LOAD( "c6",           0x1000, 0x1000, CRC(0149ef46) SHA1(58f684a9b7b9410236b3c54ea6c0fa9853a078c5) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "5.bpr",        0x0000, 0x0100, CRC(21fb583f) SHA1(b8c65fbdd5d8b70bf51341cd60fc2efeaab8bb82) ) // dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "2.bpr",        0x0000, 0x0020, CRC(26f42e6f) SHA1(f51578216a5d588c4d0143ce7a23d695a15a3914) ) // palette
	ROM_LOAD( "3.bpr",        0x0020, 0x0100, CRC(4aecc0c8) SHA1(3c1086a598d84b4bcb277556b716fd18c76c4364) ) // lookup table
	ROM_LOAD( "7a.bpr",       0x0120, 0x0020, CRC(48c8f094) SHA1(61592209720fddc8991751edf08b6950388af42e) ) // video layout (not used)
	ROM_LOAD( "10a.bpr",      0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END

ROM_START( locoboot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g.116",        0x0000, 0x1000, CRC(1248799c) SHA1(b0e513bb7ca6266f9182a91c2a30adc4b414a7ad) )
	ROM_LOAD( "g.117",        0x1000, 0x1000, CRC(5b5b5753) SHA1(22f7fa0968843b52aa6eac743e5447502c86b10f) )
	ROM_LOAD( "g.118",        0x2000, 0x1000, CRC(6bc269e1) SHA1(22d2c97e597fb7e6ae9074c8f921c902b879efe8) )
	ROM_LOAD( "g.119",        0x3000, 0x1000, CRC(3feb762e) SHA1(94ee68549752fac3c67582d968d3f5e3f1380eef) )

	/* no other roms were present in this set,
	   but it appears to work best with the cottong roms,
	   and the program roms appear to be a hack of that
	*/

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "c7",           0x0000, 0x1000, CRC(3d83f6d3) SHA1(e10ed6b6ce7280697c1bc9dbe6c6e6018e1d8be4) )
	ROM_LOAD( "c8",           0x1000, 0x1000, CRC(323e1937) SHA1(75499d6c8a9032fac090a13cd4f36bd350f52dab) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "c5",           0x0000, 0x1000, CRC(992d079c) SHA1(b5acd30f2e8700cc4cd852b190bd1f4163b137e8) )
	ROM_LOAD( "c6",           0x1000, 0x1000, CRC(0149ef46) SHA1(58f684a9b7b9410236b3c54ea6c0fa9853a078c5) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "5.bpr",        0x0000, 0x0100, CRC(21fb583f) SHA1(b8c65fbdd5d8b70bf51341cd60fc2efeaab8bb82) ) // dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "2.bpr",        0x0000, 0x0020, CRC(26f42e6f) SHA1(f51578216a5d588c4d0143ce7a23d695a15a3914) ) // palette
	ROM_LOAD( "3.bpr",        0x0020, 0x0100, CRC(4aecc0c8) SHA1(3c1086a598d84b4bcb277556b716fd18c76c4364) ) // lookup table
	ROM_LOAD( "7a.bpr",       0x0120, 0x0020, CRC(48c8f094) SHA1(61592209720fddc8991751edf08b6950388af42e) ) // video layout (not used)
	ROM_LOAD( "10a.bpr",      0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END

ROM_START( commsega )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "csega1",       0x0000, 0x1000, CRC(92de3405) SHA1(81ef4274b13f92d6274a0a037d7dc77ba0f67a1b) )
	ROM_LOAD( "csega2",       0x1000, 0x1000, CRC(f14e2f9a) SHA1(c1a7ec1c306e07bac0bbf19b60f756650f63ae29) )
	ROM_LOAD( "csega3",       0x2000, 0x1000, CRC(941dbf48) SHA1(01d2d64fb662af423aa04507ba97997772130c54) )
	ROM_LOAD( "csega4",       0x3000, 0x1000, CRC(e0ac69b4) SHA1(3a52b2a6204b7310cfe321c582352b437de16660) )
	ROM_LOAD( "csega5",       0x4000, 0x1000, CRC(bc56ebd0) SHA1(a178cd5ba381b107e720e18f3549247477037998) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "csega8",       0x0000, 0x1000, CRC(588b4210) SHA1(43bac1bdac721567e4b5d56e9e4488165872bd6a) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "csega7",       0x0000, 0x1000, CRC(e8e374f9) SHA1(442cc6b7e7d5b9472a5c16d6f78db0c03e651e98) )
	ROM_LOAD( "csega6",       0x1000, 0x1000, CRC(cf07fd5e) SHA1(4fe351c3d093f8f5fcf95e3e921a06e44d14d2a7) )

	ROM_REGION( 0x0100, "gfx2", 0 )
	ROM_LOAD( "gg3.bpr",      0x0000, 0x0100, CRC(ae7fd962) SHA1(118359cffb2ad3fdf09456a484aa730cb1b85a5d) ) // dots

	ROM_REGION( 0x0160, "proms", 0 )
	ROM_LOAD( "gg1.bpr",      0x0000, 0x0020, CRC(f69e585a) SHA1(248740b154732b6bc6f772d4bb19d654798c3739) ) // palette
	ROM_LOAD( "gg2.bpr",      0x0020, 0x0100, CRC(0b756e30) SHA1(8890e305547df8df108af0f89074fb1c8bed8e6c) ) // lookup table
	ROM_LOAD( "gg0.bpr",      0x0120, 0x0020, CRC(48c8f094) SHA1(61592209720fddc8991751edf08b6950388af42e) ) // video layout (not used)
	ROM_LOAD( "tt3.bpr",      0x0140, 0x0020, CRC(b8861096) SHA1(26fad384ed7a1a1e0ba719b5578e2dbb09334a25) ) // video timing (not used)
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

//    YEAR  NAME       PARENT    MACHINE   INPUT     CLASS         INIT        ROT    COMPANY                               FULLNAME                             FLAGS
GAME( 1980, rallyx,    0,        rallyx,   rallyx,   rallyx_state, empty_init, ROT0,  "Namco",                              "Rally X (32k Ver.?)",               MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, rallyxa,   rallyx,   rallyx,   rallyx,   rallyx_state, empty_init, ROT0,  "Namco",                              "Rally X",                           MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, rallyxm,   rallyx,   rallyx,   rallyx,   rallyx_state, empty_init, ROT0,  "Namco (Midway license)",             "Rally X (Midway)",                  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, rallyxmr,  rallyx,   rallyx,   rallyx,   rallyx_state, empty_init, ROT0,  "bootleg (Model Racing)",             "Rally X (Model Racing bootleg)",    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, rallyxeg,  rallyx,   rallyx,   rallyxeg, rallyx_state, empty_init, ROT90, "bootleg (Video Game / Electrogame)", "Rally X (Video Game bootleg)",      MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, dngrtrck,  rallyx,   rallyx,   dngrtrck, rallyx_state, empty_init, ROT0,  "bootleg (Petaco)",                   "Danger Track (bootleg of Rally X)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, nrallyx,   0,        rallyx,   nrallyx,  rallyx_state, empty_init, ROT0,  "Namco",                              "New Rally X",                       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, nrallyxb,  nrallyx,  rallyx,   nrallyx,  rallyx_state, empty_init, ROT0,  "Namco",                              "New Rally X (bootleg?)",            MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, jungler,   0,        jungler,  jungler,  rallyx_state, empty_init, ROT90, "Konami",                             "Jungler",                           MACHINE_SUPPORTS_SAVE )
GAME( 1981, junglers,  jungler,  jungler,  jungler,  rallyx_state, empty_init, ROT90, "Konami (Stern Electronics license)", "Jungler (Stern Electronics)",       MACHINE_SUPPORTS_SAVE )
GAME( 1981, junglero,  jungler,  jungler,  jungler,  rallyx_state, empty_init, ROT90, "Konami (Olympia license)",           "Jungler (Olympia)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1982, jackler,   jungler,  jungler,  jungler,  rallyx_state, empty_init, ROT90, "bootleg",                            "Jackler (bootleg of Jungler)",      MACHINE_SUPPORTS_SAVE )
GAME( 1982, savanna,   jungler,  jungler,  jungler,  rallyx_state, empty_init, ROT90, "bootleg (Olympia)",                  "Savanna (bootleg of Jungler)",      MACHINE_SUPPORTS_SAVE ) // or licensed from Konami?
GAME( 1982, tactcian,  0,        tactcian, tactcian, rallyx_state, empty_init, ROT90, "Konami (Sega license)",              "Tactician (set 1)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1981, tactcian2, tactcian, tactcian, tactcian, rallyx_state, empty_init, ROT90, "Konami (Sega license)",              "Tactician (set 2)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1982, locomotn,  0,        locomotn, locomotn, rallyx_state, empty_init, ROT90, "Konami (Centuri license)",           "Loco-Motion",                       MACHINE_SUPPORTS_SAVE )
GAME( 1982, gutangtn,  locomotn, locomotn, locomotn, rallyx_state, empty_init, ROT90, "Konami",                             "Guttang Gottong",                   MACHINE_SUPPORTS_SAVE )
GAME( 1982, gutangtns, locomotn, locomotn, locomotn, rallyx_state, empty_init, ROT90, "Konami (Sega license)",              "Guttang Gottong (Sega license)",    MACHINE_SUPPORTS_SAVE )
GAME( 1982, cottong,   locomotn, locomotn, locomotn, rallyx_state, empty_init, ROT90, "bootleg",                            "Cotocoto Cottong",                  MACHINE_SUPPORTS_SAVE )
GAME( 1982, locoboot,  locomotn, locomotn, locomotn, rallyx_state, empty_init, ROT90, "bootleg",                            "Loco-Motion (bootleg)",             MACHINE_SUPPORTS_SAVE )
GAME( 1983, commsega,  0,        commsega, commsega, rallyx_state, empty_init, ROT90, "Sega",                               "Commando (Sega)",                   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
