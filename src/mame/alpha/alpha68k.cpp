// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail, Stephane Humbert, Angelo Salese
/***************************************************************************

    SNK/Alpha 68000 based games:

    (Game)                  (PCB Number)     (Manufacturer)

    Super Stingray          ? (Early)        Alpha 1986?
    Kyros                   ? (Early)        World Games Inc 1987
    Mahjong Block Jongbou   Alpha 68K-96 N   SNK 1987
    Paddle Mania            Alpha 68K-96 I   SNK 1988
    Time Soldiers (Ver 3)   Alpha 68K-96 II  SNK/Romstar 1987
    Time Soldiers (Ver 1)   Alpha 68K-96 II  SNK/Romstar 1987
    Battlefield (Ver 1)     Alpha 68K-96 II  SNK 1987
    Sky Soldiers            Alpha 68K-96 II  SNK/Romstar 1988
    Gold Medalist           Alpha 68K-96 II  SNK 1988
    Gold Medalist           (Bootleg)        SNK 1988
    Sky Adventure           Alpha 68K-96 V   SNK 1989
    Gang Wars               Alpha 68K-96 V   Alpha 1989
    Gang Wars               (Bootleg)        Alpha 1989
    Super Champion Baseball (V board?)       SNK/Alpha/Romstar/Sega 1989
    The Next Space          A8004-1 PIC      SNK 1989

TODO:
- II & V board: bit 15 of palette RAM isn't hooked up, according to Sky Adventure
  service mode enables "bright", it is actually same as NeoGeo device;
- II & V board: Fix sound CPU crashes properly (nested NMIs);
- Sky Soldiers: According to various references bosses should really time out after some time (reportedly ~80 secs.).
  This is actually handled by the MCU, not unlike Gold Medalist;
- Sky Soldiers: BGM Fade out before boss battle isn't implemented;
- Sky Adventure, probably others: on a real PCB reference BGM stutters when using
  30 Hz autofire (not enough sound resources?);
- Sky Adventure, probably others: sprite drawing is off-sync, cfr. notes in video file;
- Gold Medalist: attract mode has missing finger on button 1, may be btanb;
- Gold Medalist: incorrect blank effect on shooting pistol for dash events (cfr. alpha68k_palette_device);
- Gold Medalist: dash events timers relies on MCU irq timings.
  Previous emulation of 180 Hz was making it way too hard, and the timer was updating at 0.03 secs (-> 30 Hz refresh rate).
  Using a timer of 100 Hz seems a better approximation compared to a stopwatch, of course fine tuning
  this approximation needs HW probing and/or a MCU decap;
- Gold Medalist: MCU irq routine has an event driven path that is never taken into account, what is it for?
- Super Champion Baseball: "ball speed pitch" protection;
- Super Champion Baseball: enables opacity bit on fix layer, those are transparent on SNK Arcade Classics 0
  but actually opaque on a reference shot, sounds like a btanb;
- Fix layer tilemap should be a common device between this, snk68.cpp and other Alpha/SNK-based games;

General notes:

    All II & V games are 68000, z80 plus a custom Alpha microcontroller,
    the microcontroller is able to write to anywhere within main memory.

    Gold Medalist (bootleg) has a 68705 in place of the Alpha controller.
     (Kyros bootleg also? we have decapped MCU dumps of different types for it)

    V boards have more memory and double the amount of colours as II boards.

    Time Soldiers - make the ROM writable and the game will enter a 'debug'
    kind of mode, probably from the development system used.

    Time Soldiers - Title screen is corrupt when set to 'Japanese language',
    the real board does this too!  (Battlefield is corrupt when set to English
    too).

    The Next Space is not an Alpha game, but the video hardware is identical
    to Paddlemania.

    Emulation by Bryan McPhail, mish@tendril.co.uk


Stephh's additional notes (based on the games M68000 code and some tests) :

 1)  'sstingry'

  - You can test the ports (inputs, Dip Switches and sound) by pressing
    player 1 buttons 1 and 2 during the P.O.S.T.

 2)  'kyros'

  - You can enter sort of "test mode" by pressing player 1 buttons 1 and 2
    when you reset the game.

 3)  'paddlema'

  - "Game Time" The effects of "Game Time" and "Match Type"

      Time Setting
                       A            B            C            D            E
       Match Type  1P vs Comp.  2P vs Comp.  1P vs 1P     2P vs 1P     2P vs 2P
            A        1:00         1:10         2:00         2:30         3:00
            B        1:10         1:20         2:10         2:40         3:10
            C        1:20         1:30         2:20         2:50         3:20
            D        1:30         1:40         2:30         3:00         3:30


  - When "Game Mode" Dip Switch is set to "Win Match Against CPU", this has
    an effect on matches types A and B : player is awarded 99 points at the
    round, which is enough to win all matches then see the ending credits.

  - "Button A" and "Button B" do the same thing : they BOTH start a game
    and select difficulty/court. I've mapped them this way because you
    absolutely need the 2 buttons when you are in the "test mode".

 4)  'timesold', 'timesold1' and 'btlfield'

  - The "Unused" Dip Switch is sort of "Debug Mode" Dip Switch and has an
    effect ONLY if 0x008fff.b is writable (as Bryan mentioned it).
    Its role seems only to be limited to display some coordinates.

 5)  'skysoldr'

  - As in "Time Soldiers / Battle Field" there is a something that is sort
    of "Debug Mode" Dip Switch : this is the "Manufacturer" Dip Switch when
    it is set to "Romstar". Again, it has an effect only if 0x000074.w is
    writable and its role seems only to be limited to display some coordinates.

 7)  'skyadvnt', 'skyadvntu' and 'skyadvntj'

  - As in 'skysoldr', you can access to some "hidden features" if 0x000074.w
    is writable :

      * bit 4 (when "Unused" Dip Switch is set to "On") determines invulnerability
      * bit 6 (when "Difficulty" Dip Switch is set to DEF_STR( Hard ) or DEF_STR( Hardest ))
        determines if some coordinates are displayed.

 8)  'gangwars'

  - When "Coin Slots" Dip Switch is set to "1", COIN2 only adds ONE credit
    and this has nothing to do with the microcontroller stuff.
  - There is no Dip Switch to determine if you are allowed to continue a game
    or not, so you ALWAYS have the possibility to continue a game.


Stephh's log (2002.06.19) :

  - Create macros for players inputs and "Coinage" Dip Switch
  - Full check and fix of Dip Switches and Inputs in ALL games (PHEW ! :p)
    Read MY additional notes for unknown issues though ...
  - Improve coin handler for all games (thanks Acho A. Tang for the sample
    for 'sstingry', even if I had to rewrite it !)
  - Fix screen flipping in 'sbasebal' (this was a Dip Switch issue)
  - Add READ16_HANDLER( *_cycle_r ) for the following games :
      * timesold1  (based on the one from 'timesold')
      * btlfield  (based on the one from 'timesold')
      * gangwars  (I split the one from 'gangwarsu')
      * skyadvnt, skyadvntu and skyadvntj
  - Change manufacturer for the following games :
      * timesold
      * timesold1
      * btlfield
      * skysoldr
    I can send you the ending pics where "Alpha Denshi Co." is mentioned.
  - microcontroller_id is no more static to be used in video/alpha68k.c
    (in which I've removed the no more needed 'game_id' and 'strcmp')


Revision History:

Pierpaolo Prazzoli, 25-06-2004

- Added Mahjong Block Jongbou (hold P1 button1 during boot to enter test mode)
- Added cocktail support to Super Stingray and Kyros
- Added coin counters to The Next Space

Acho A. Tang, xx-xx-2002

[Super Stingray]
- improved color, added sound, game timer and coin handler, corrected DIP settings
note: CLUT and color remap PROMs missing

[Kyros]
- fixed color, added sound and coin handler, corrected DIP settings

[Paddle Mania]
- reactivated driver
- improved color, added sound, fixed control and priority, corrected DIP settings

[Time Soldiers]
- fixed priority

[Gold Medalist]
- fixed gameplay, control and priority, corrected DIP settings
- fixed garbled voices using sound ROMs from the bootleg set

[Sky Adventure]
- fixed sprite position and priority

[Gang Wars]
- fixed color in the 2nd last graphics bank

[The Next Space]
- fixed color and sprite glitches, added sound, filled DIP settings


DIP locations verified from manuals for:
- tnextspc
- btlfield
- gangwars
- skyadvnt
- goldmedl
- kyros

***************************************************************************/

#include "emu.h"
#include "alpha68k.h"


/******************************************************************************/

void alpha68k_state::alpha_microcontroller_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%04x:  Alpha write trigger at %04x (%04x)\n", m_maincpu->pc(), offset, data);
	/* 0x44 = coin clear signal to microcontroller? */
	if (offset == 0x2d && ACCESSING_BITS_0_7)
		m_flipscreen = (data & 1);
}

/******************************************************************************/

u16 alpha68k_II_state::control_1_r()
{
	if (m_invert_controls)
		return ~(m_in[0]->read() + (m_in[1]->read() << 8));

	return m_in[0]->read() + (m_in[1]->read() << 8);
}

u16 alpha68k_II_state::control_2_r()
{
	if (m_invert_controls)
		return ~(m_in[3]->read() + ((~(1 << m_in[5]->read())) << 8));

	return m_in[3]->read() + /* Low byte of CN1 */
		((~(1 << m_in[5]->read())) << 8);
}

u16 alpha68k_II_state::control_3_r()
{
	if (m_invert_controls)
		return ~(((~(1 << m_in[6]->read())) << 8) & 0xff00);

	return ((~(1 << m_in[6]->read())) << 8) & 0xff00;
}

/* High 4 bits of CN1 & CN2 */
u16 alpha68k_II_state::control_4_r()
{
	if (m_invert_controls)
		return ~((((~(1 << m_in[6]->read())) << 4) & 0xf000)
			+ (((~(1 << m_in[5]->read()))) & 0x0f00));

	return (((~(1 << m_in[6]->read())) << 4) & 0xf000)
			+ (((~(1 << m_in[5]->read()))) & 0x0f00);
}

void alpha68k_II_state::outlatch_w(offs_t offset, u8 data)
{
	m_outlatch->write_bit((offset >> 3) & 7, BIT(offset, 6));
}

/******************************************************************************/

/* Time Soldiers, Sky Soldiers, Gold Medalist */
u16 alpha68k_II_state::alpha_II_trigger_r(offs_t offset)
{
	/* possible jump codes:
	     - Time Soldiers : 0x21,0x22,0x23,0x24,0x34,0x37,0x3a,0x3d,0x40,0x43,0x46,0x49
	     - Sky Soldiers  : 0x21,0x22,0x23,0x24,0x34,0x37,0x3a,0x3d,0x40,0x43,0x46,0x49
	     - Gold Medalist : 0x21,0x23,0x24,0x5b
	*/
	static const u8 coinage1[8][2] = {{1,1}, {1,2}, {1,3}, {1,4}, {1,5}, {1,6}, {2,3}, {3,2}};
	static const u8 coinage2[8][2] = {{1,1}, {2,1}, {3,1}, {4,1}, {5,1}, {6,1}, {7,1}, {8,1}};
	const u16 source = m_shared_ram[offset];

	switch (offset)
	{
		case 0: /* Dipswitch 2 */
			m_shared_ram[0] = (source & 0xff00) | m_in[4]->read();
			return 0;

		case 0x22: /* Coin value */
			m_shared_ram[0x22] = (source & 0xff00) | (m_credits & 0x00ff);
			return 0;

		case 0x29: /* Query microcontroller for coin insert */
			if ((m_in[2]->read() & 0x3) == 3)
				m_latch = 0;
			if ((m_in[2]->read() & 0x1) == 0 && !m_latch)
			{
				m_shared_ram[0x29] = (source & 0xff00) | (m_coin_id & 0xff);    // coinA
				m_shared_ram[0x22] = (source & 0xff00) | 0x0;
				m_latch = 1;

				if ((m_coin_id & 0xff) == 0x22)
				{
					if (m_game_id == ALPHA68K_BTLFIELDB)
						m_coinvalue = (m_in[4]->read() >> 0) & 7;
					else
						m_coinvalue = (~m_in[4]->read() >> 0) & 7;

					m_deposits1++;
					if (m_deposits1 == coinage1[m_coinvalue][0])
					{
						m_credits = coinage1[m_coinvalue][1];
						m_deposits1 = 0;
					}
					else
						m_credits = 0;
				}
			}
			else if ((m_in[2]->read() & 0x2) == 0 && !m_latch)
			{
				m_shared_ram[0x29] = (source & 0xff00) | (m_coin_id >> 8);  // coinB
				m_shared_ram[0x22] = (source & 0xff00) | 0x0;
				m_latch = 1;

				if ((m_coin_id >> 8) == 0x22)
				{
					if (m_game_id == ALPHA68K_BTLFIELDB)
						m_coinvalue = (m_in[4]->read() >> 0) & 7;
					else
						m_coinvalue = (~m_in[4]->read() >> 0) & 7;

					m_deposits2++;
					if (m_deposits2 == coinage2[m_coinvalue][0])
					{
						m_credits = coinage2[m_coinvalue][1];
						m_deposits2 = 0;
					}
					else
						m_credits = 0;
				}
			}
			else
			{
				if (m_microcontroller_id == 0x8803)     /* Gold Medalist */
				{
					// TODO: dash events increments timer at 0x16c0 in irq routine (event driven?)
					// There's also another unemulated event path if this is 0x5b, unknown purpose
					m_microcontroller_data = 0x21;
				}
				else
				{
					// TODO: Sky Soldiers uses this thread dispatch as well for boss timing out and who knows what else
					// cfr. PC=0x1d52, possible threads at ROM address $1e7e-$1e89 (in bytes)
					// 0x34 should be the step counter handling while 0x37 is what actually times out (or even destroys) the boss on expiration.
					// Notice that there are additional shared RAM checks (i.e. 0x1e6a, 0x1e32, 0x1e3a), which causes the game to soft lock if not satisfied.
					// Apparently bosses should time out in ~80 seconds.
					m_microcontroller_data = 0x00;

					// Notice that a similar system is also used by Time Soldiers but most threads are actually NOP-ed out.
					// (basically anything that is >0x24)
					// Most likely left-overs that eventually were completed with aforementioned Sky Soldiers.
				}
				m_shared_ram[0x29] = (source & 0xff00) | m_microcontroller_data;
			}

			return 0;
		case 0xfe:  /* Custom ID check, same for all games */
			m_shared_ram[0xfe] = (source & 0xff00) | 0x87;
			break;
		case 0xff:  /* Custom ID check, same for all games */
			m_shared_ram[0xff] = (source & 0xff00) | 0x13;
			break;
	}

	logerror("%04x:  Alpha read trigger at %04x\n", m_maincpu->pc(), offset);

	return 0; /* Values returned don't matter */
}

/* Sky Adventure, Gang Wars, Super Champion Baseball */
u16 alpha68k_III_state::alpha_V_trigger_r(offs_t offset)
{
	/* possible jump codes:
	     - Sky Adventure           : 0x21,0x22,0x23,0x24,0x34,0x37,0x3a,0x3d,0x40,0x43,0x46,0x49
	     - Gang Wars               : 0x21,0x23,0x24,0x54
	     - Super Champion Baseball : 0x21,0x23,0x24
	*/
	static const u8 coinage1[8][2] = {{1,1}, {1,5}, {1,3}, {2,3}, {1,2}, {1,6}, {1,4}, {3,2}};
	static const u8 coinage2[8][2] = {{1,1}, {5,1}, {3,1}, {7,1}, {2,1}, {6,1}, {4,1}, {8,1}};
	u16 source = m_shared_ram[offset];

	switch (offset)
	{
		case 0: /* Dipswitch 1 */
			m_shared_ram[0] = (source & 0xff00) | m_in[4]->read();
			return 0;
		case 0x22: /* Coin value */
			m_shared_ram[0x22] = (source & 0xff00) | (m_credits & 0x00ff);
			return 0;
		case 0x29: /* Query microcontroller for coin insert */
			if ((m_in[2]->read() & 0x3) == 3)
				m_latch = 0;
			if ((m_in[2]->read() & 0x1) == 0 && !m_latch)
			{
				m_shared_ram[0x29] = (source & 0xff00) | (m_coin_id & 0xff);    // coinA
				m_shared_ram[0x22] = (source & 0xff00) | 0x0;
				m_latch = 1;

				if ((m_coin_id & 0xff) == 0x22)
				{
					m_coinvalue = (~m_in[4]->read() >> 1) & 7;
					m_deposits1++;
					if (m_deposits1 == coinage1[m_coinvalue][0])
					{
						m_credits = coinage1[m_coinvalue][1];
						m_deposits1 = 0;
					}
					else
						m_credits = 0;
				}
			}
			else if ((m_in[2]->read() & 0x2) == 0 && !m_latch)
			{
				m_shared_ram[0x29] = (source & 0xff00) | (m_coin_id>>8);    // coinB
				m_shared_ram[0x22] = (source & 0xff00) | 0x0;
				m_latch = 1;

				if ((m_coin_id >> 8) == 0x22)
				{
					m_coinvalue = (~m_in[4]->read() >> 1) & 7;
					m_deposits2++;
					if (m_deposits2 == coinage2[m_coinvalue][0])
					{
						m_credits = coinage2[m_coinvalue][1];
						m_deposits2 = 0;
					}
					else
						m_credits = 0;
				}
			}
			else
			{
				m_microcontroller_data = 0x00;
				m_shared_ram[0x29] = (source & 0xff00) | m_microcontroller_data;
			}

			return 0;
		case 0xfe:  /* Custom ID check */
			m_shared_ram[0xfe] = (source & 0xff00) | (m_microcontroller_id >> 8);
			break;
		case 0xff:  /* Custom ID check */
			m_shared_ram[0xff] = (source & 0xff00) | (m_microcontroller_id & 0xff);
			break;

		case 0x1f00: /* Dipswitch 1 */
			m_shared_ram[0x1f00] = (source & 0xff00) | m_in[4]->read();
			return 0;
		case 0x1f29: /* Query microcontroller for coin insert */
			if ((m_in[2]->read() & 0x3) == 3)
				m_latch = 0;
			if ((m_in[2]->read() & 0x1) == 0 && !m_latch)
			{
				m_shared_ram[0x1f29] = (source & 0xff00) | (m_coin_id & 0xff);  // coinA
				m_shared_ram[0x1f22] = (source & 0xff00) | 0x0;
				m_latch = 1;

				if ((m_coin_id & 0xff) == 0x22)
				{
					m_coinvalue = (~m_in[4]->read() >> 1) & 7;
					m_deposits1++;
					if (m_deposits1 == coinage1[m_coinvalue][0])
					{
						m_credits = coinage1[m_coinvalue][1];
						m_deposits1 = 0;
					}
					else
						m_credits = 0;
				}
			}
			else if ((m_in[2]->read() & 0x2) == 0 && !m_latch)
			{
				m_shared_ram[0x1f29] = (source & 0xff00) | (m_coin_id >> 8);    // coinB
				m_shared_ram[0x1f22] = (source & 0xff00) | 0x0;
				m_latch = 1;

				if ((m_coin_id >> 8) == 0x22)
				{
					m_coinvalue = (~m_in[4]->read() >> 1) & 7;
					m_deposits2++;
					if (m_deposits2 == coinage2[m_coinvalue][0])
					{
						m_credits = coinage2[m_coinvalue][1];
						m_deposits2 = 0;
					}
					else
						m_credits = 0;
				}
			}
			else
			{
				m_microcontroller_data = 0x00;
				m_shared_ram[0x1f29] = (source & 0xff00) | m_microcontroller_data;
			}

			/* Gang Wars expects the first dip to appear in RAM at 0x02c6,
			   the microcontroller supplies it (it does for all the other games,
			   but usually to 0x0 in RAM) when 0x21 is read (code at 0x009332) */
			source = m_shared_ram[0x0163];
			m_shared_ram[0x0163] = (source & 0x00ff) | (m_in[4]->read() << 8);

			return 0;
		case 0x1ffe:  /* Custom ID check */
			m_shared_ram[0x1ffe] = (source & 0xff00) | (m_microcontroller_id >> 8);
			break;
		case 0x1fff:  /* Custom ID check */
			m_shared_ram[0x1fff] = (source & 0xff00) | (m_microcontroller_id & 0xff);
			break;
	}

	logerror("%04x:  Alpha read trigger at %04x\n", m_maincpu->pc(), offset);

	return 0; /* Values returned don't matter */
}

/******************************************************************************/

void alpha68k_II_state::alpha68k_II_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x008ffe, 0x008fff).nopw();
	map(0x040000, 0x040fff).ram().share("shared_ram");
	map(0x080000, 0x080001).r(FUNC(alpha68k_II_state::control_1_r)); /* Joysticks */
	map(0x080001, 0x080001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0c0000, 0x0c0001).r(FUNC(alpha68k_II_state::control_2_r)); /* CN1 & Dip 1 */
	map(0x0c0001, 0x0c0001).select(0x78).w(FUNC(alpha68k_II_state::outlatch_w));
	map(0x0c8000, 0x0c8001).r(FUNC(alpha68k_II_state::control_3_r)); /* Bottom of CN2 */
	map(0x0d0000, 0x0d0001).r(FUNC(alpha68k_II_state::control_4_r)); /* Top of CN1 & CN2 */
	map(0x0d8000, 0x0d8001).nopr(); /* IRQ ack? */
	map(0x0e0000, 0x0e0001).nopr(); /* IRQ ack? */
	map(0x0e8000, 0x0e8001).nopr(); /* watchdog? */
	map(0x100000, 0x100fff).ram().w(FUNC(alpha68k_II_state::videoram_w)).share("videoram");
	map(0x200000, 0x207fff).rw(m_sprites, FUNC(snk68_spr_device::spriteram_r), FUNC(snk68_spr_device::spriteram_w)).share("spriteram");
	map(0x300000, 0x3001ff).rw(FUNC(alpha68k_II_state::alpha_II_trigger_r), FUNC(alpha68k_II_state::alpha_microcontroller_w));
	map(0x400000, 0x400fff).rw(m_palette, FUNC(alpha68k_palette_device::read), FUNC(alpha68k_palette_device::write));
	map(0x800000, 0x83ffff).rom().region("maincpu", 0x40000);
}

void alpha68k_III_state::alpha68k_V_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x043fff).ram().share("shared_ram");
	map(0x080000, 0x080001).r(FUNC(alpha68k_III_state::control_1_r)); /* Joysticks */
	map(0x080000, 0x080000).w(FUNC(alpha68k_III_state::video_bank_w));
	map(0x080001, 0x080001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0c0000, 0x0c0001).lr16(NAME([this] () -> u16 { return m_in[3]->read(); })); /* Dip 2 */
	map(0x0c0001, 0x0c0001).select(0x78).w(FUNC(alpha68k_III_state::outlatch_w));
	map(0x0d8000, 0x0d8001).nopr(); /* IRQ ack? */
	map(0x0e0000, 0x0e0001).nopr(); /* IRQ ack? */
	map(0x0e8000, 0x0e8001).nopr(); /* watchdog? */
	map(0x100000, 0x100fff).ram().w(FUNC(alpha68k_III_state::videoram_w)).share("videoram");
	map(0x200000, 0x207fff).rw(m_sprites, FUNC(snk68_spr_device::spriteram_r), FUNC(snk68_spr_device::spriteram_w)).share("spriteram");
	map(0x300000, 0x303fff).r(FUNC(alpha68k_III_state::alpha_V_trigger_r));
	map(0x300000, 0x3001ff).w(FUNC(alpha68k_III_state::alpha_microcontroller_w));
	map(0x303e00, 0x303fff).w(FUNC(alpha68k_III_state::alpha_microcontroller_w)); /* Gang Wars mirror */
	map(0x400000, 0x401fff).rw(m_palette, FUNC(alpha68k_palette_device::read), FUNC(alpha68k_palette_device::write));
	map(0x800000, 0x83ffff).rom().region("maincpu", 0x40000);
}

void alpha68k_III_state::alpha68k_III_map(address_map &map)
{
	alpha68k_III_state::alpha68k_V_map(map);
	map(0x300000, 0x3001ff).rw(FUNC(alpha68k_III_state::alpha_II_trigger_r), FUNC(alpha68k_III_state::alpha_microcontroller_w));
	map(0x300200, 0x303fff).unmaprw();
}

/******************************************************************************/

void alpha68k_II_state::sound_bank_w(u8 data)
{
	m_audiobank->set_entry(data & 0x1f);
}

void alpha68k_II_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xc000, 0xffff).bankr("audiobank");
}

void alpha68k_II_state::sound_portmap(address_map &map)
{
	map.global_mask(0x0f);
	map(0x00, 0x00).mirror(0x0f).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x00, 0x00).mirror(0x01).w(m_soundlatch, FUNC(generic_latch_8_device::clear_w));
	map(0x08, 0x08).mirror(0x01).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x0a, 0x0b).w("ym2", FUNC(ym2413_device::write));
	map(0x0c, 0x0d).w("ym1", FUNC(ym2203_device::write));
	map(0x0e, 0x0e).mirror(0x01).w(FUNC(alpha68k_II_state::sound_bank_w));
}

/******************************************************************************/

#define ALPHA68K_COINAGE_BITS_0TO2 \
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:4,5,6") \
	PORT_DIPSETTING(    0x07, "A 1C/1C B 1C/1C" )   \
	PORT_DIPSETTING(    0x06, "A 1C/2C B 2C/1C" )   \
	PORT_DIPSETTING(    0x05, "A 1C/3C B 3C/1C" )   \
	PORT_DIPSETTING(    0x04, "A 1C/4C B 4C/1C" )   \
	PORT_DIPSETTING(    0x03, "A 1C/5C B 5C/1C" )   \
	PORT_DIPSETTING(    0x02, "A 1C/6C B 6C/1C" )   \
	PORT_DIPSETTING(    0x01, "A 2C/3C B 7C/1C" )   \
	PORT_DIPSETTING(    0x00, "A 3C/2C B 8C/1C" )

#define ALPHA68K_COINAGE_BITS_1TO3 \
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:4,5,6") \
	PORT_DIPSETTING(    0x0e, "A 1C/1C B 1C/1C" )   \
	PORT_DIPSETTING(    0x06, "A 1C/2C B 2C/1C" )   \
	PORT_DIPSETTING(    0x0a, "A 1C/3C B 3C/1C" )   \
	PORT_DIPSETTING(    0x02, "A 1C/4C B 4C/1C" )   \
	PORT_DIPSETTING(    0x0c, "A 1C/5C B 5C/1C" )   \
	PORT_DIPSETTING(    0x04, "A 1C/6C B 6C/1C" )   \
	PORT_DIPSETTING(    0x08, "A 2C/3C B 7C/1C" )   \
	PORT_DIPSETTING(    0x00, "A 3C/2C B 8C/1C" )

#define ALPHA68K_COINAGE_BITS_2TO4 \
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:2,3,4") \
	PORT_DIPSETTING(    0x1c, "A 1C/1C B 1C/1C" )   \
	PORT_DIPSETTING(    0x18, "A 1C/2C B 2C/1C" )   \
	PORT_DIPSETTING(    0x14, "A 1C/3C B 3C/1C" )   \
	PORT_DIPSETTING(    0x10, "A 1C/4C B 4C/1C" )   \
	PORT_DIPSETTING(    0x0c, "A 1C/5C B 5C/1C" )   \
	PORT_DIPSETTING(    0x08, "A 1C/6C B 6C/1C" )   \
	PORT_DIPSETTING(    0x04, "A 2C/3C B 7C/1C" )   \
	PORT_DIPSETTING(    0x00, "A 3C/2C B 8C/1C" )

static INPUT_PORTS_START( timesold )
	PORT_START("IN0")
	ALPHA68K_PLAYER_INPUT_LSB( 1, IPT_UNKNOWN, IPT_START1, IP_ACTIVE_LOW )

	PORT_START("IN1")
	ALPHA68K_PLAYER_INPUT_LSB( 2, IPT_UNKNOWN, IPT_START2, IP_ACTIVE_LOW )

	ALPHA68K_MCU

	PORT_START("IN3")  /* Service + dip */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)

	/* 2 physical sets of _6_ dip switches */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Japanese ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:5" )        /* Listed as "Unused" - See notes */
	PORT_DIPNAME( 0x80, 0x80, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW1:6") /* Listed in the manual as "Free Play" */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4") /* A 6 way dip switch */
	ALPHA68K_COINAGE_BITS_0TO2
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")  /* player 1 12-way rotary control - converted in controls_r() */
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("IN6")  /* player 2 12-way rotary control - converted in controls_r() */
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)
INPUT_PORTS_END

/* Same as 'timesold' but different default settings for the "Language" Dip Switch */
static INPUT_PORTS_START( btlfield )
	PORT_START("IN0")
	ALPHA68K_PLAYER_INPUT_LSB( 1, IPT_UNKNOWN, IPT_START1, IP_ACTIVE_LOW )

	PORT_START("IN1")
	ALPHA68K_PLAYER_INPUT_LSB( 2, IPT_UNKNOWN, IPT_START2, IP_ACTIVE_LOW )

	ALPHA68K_MCU

	PORT_START("IN3")  /* Service + dip */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)

	/* 2 physical sets of _6_ dip switches */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Language ) )         PORT_DIPLOCATION("SW1:4") /* Listed as "Unused". */
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Japanese ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:5" )            /* Listed as "Unused", see notes. */
	PORT_DIPNAME( 0x80, 0x80, "Invulnerability (Cheat)")        PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4") /* A 6 way dip switch */
	ALPHA68K_COINAGE_BITS_0TO2
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")  /* player 1 12-way rotary control - converted in controls_r() */
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("IN6")  /* player 2 12-way rotary control - converted in controls_r() */
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)
INPUT_PORTS_END

static INPUT_PORTS_START( btlfieldb )
	PORT_INCLUDE( btlfield )

	PORT_MODIFY("IN4") /* A 6 way dip switch */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x00, "A 1C/1C B 1C/1C" )
	PORT_DIPSETTING(    0x01, "A 1C/2C B 2C/1C" )
	PORT_DIPSETTING(    0x02, "A 1C/3C B 3C/1C" )
	PORT_DIPSETTING(    0x03, "A 1C/4C B 4C/1C" )
	PORT_DIPSETTING(    0x04, "A 1C/5C B 5C/1C" )
	PORT_DIPSETTING(    0x05, "A 1C/6C B 6C/1C" )
	PORT_DIPSETTING(    0x06, "A 2C/3C B 7C/1C" )
	PORT_DIPSETTING(    0x07, "A 3C/2C B 8C/1C" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:1" )

	/* Bootleg does not appear to have rotary gun direction movements */
	PORT_MODIFY("IN5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static INPUT_PORTS_START( skysoldr )
	PORT_START("IN0")
	ALPHA68K_PLAYER_INPUT_LSB( 1, IPT_UNKNOWN, IPT_START1, IP_ACTIVE_LOW )

	PORT_START("IN1")
	ALPHA68K_PLAYER_INPUT_LSB( 2, IPT_UNKNOWN, IPT_START2, IP_ACTIVE_LOW )

	ALPHA68K_MCU

	PORT_START("IN3")  /* Service + dip */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)

	/* 2 physical sets of _6_ dip switches */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:4") /* Manual states "Always On" */
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x40, 0x40, "Manufacturer" )      PORT_DIPLOCATION("SW1:5") /* Manual states "Always Off"  See notes */
	PORT_DIPSETTING(    0x40, "SNK" )
	PORT_DIPSETTING(    0x00, "Romstar" )
	PORT_DIPNAME( 0x80, 0x80, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4") /* A 6 way dip switch */
	ALPHA68K_COINAGE_BITS_0TO2
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")  /* player 1 12-way rotary control */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN6")  /* player 2 12-way rotary control */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( goldmedl )
	// 3 buttons per player, no joystick
	// arrangement confirmed by attract mode
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // START3 is mapped on dip
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")  /* 3 buttons per player, no joystick */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	ALPHA68K_MCU

	PORT_START("IN3")  /* Service + dip */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)

	/* 2 physical sets of _6_ dip switches */
	// defaults are retrieved from Gold Medalist Romstar manual
	PORT_DIPNAME( 0x04, 0x04, "Event Select" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x88, 0x80, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:2,6")
	PORT_DIPSETTING(    0x00, "Upright 2 Players" )
	PORT_DIPSETTING(    0x80, "Upright 4 Players" )
	PORT_DIPSETTING(    0x88, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x08, "Cocktail (duplicate)" ) // not documented in manual
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START3 ) PORT_DIPLOCATION("SW1:3") // dip listed as "Always OFF" in manual, is start3 really routed here?
	PORT_DIPNAME( 0x20, 0x00, "Speed For 100M Dash" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "10 Beats For Max Speed" )
	PORT_DIPSETTING(    0x20, "14 Beats For Max Speed" )
	PORT_DIPNAME( 0x40, 0x00, "Computer Demonstration" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )

	PORT_START("IN4") /* A 6 way dip switch */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	ALPHA68K_COINAGE_BITS_2TO4
	// TODO: default is actually demo sounds OFF
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")  /* player 1 12-way rotary control */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN6")  /* player 2 12-way rotary control */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( skyadvnt )
	PORT_START("IN0")
	ALPHA68K_PLAYER_INPUT_LSB( 1, IPT_BUTTON3, IPT_START1, IP_ACTIVE_LOW )

	PORT_START("IN1")
	ALPHA68K_PLAYER_INPUT_LSB( 2, IPT_BUTTON3, IPT_START2, IP_ACTIVE_LOW )

	ALPHA68K_MCU

	PORT_START("IN3")  /* Service + dip */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)

	/* 2 physical sets of _6_ dip switches */
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:3" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:4,5") // See notes
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN4") /* A 6 way dip switch */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:6" )        /* Listed as "Unused" */
	ALPHA68K_COINAGE_BITS_1TO3
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* Same as 'skyadvnt' but bits 0-3 of 2nd set of Dip Switches are different */
static INPUT_PORTS_START( skyadvntu )
	PORT_INCLUDE( skyadvnt )

	PORT_MODIFY("IN4") /* A 6 way dip switch */
	PORT_DIPNAME( 0x01, 0x00, "Price to Continue" )     PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "Same as Start" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END



static INPUT_PORTS_START( gangwarsu )
	PORT_START("IN0")
	ALPHA68K_PLAYER_INPUT_LSB( 1, IPT_BUTTON3, IPT_START1, IP_ACTIVE_LOW )

	PORT_START("IN1")
	ALPHA68K_PLAYER_INPUT_LSB( 2, IPT_BUTTON3, IPT_START2, IP_ACTIVE_LOW )

	ALPHA68K_MCU

	PORT_START("IN3")  /* Service + dip */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)

	/* 2 physical sets of _6_ dip switches */
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x00, "Timer Speed" )       PORT_DIPLOCATION("SW2:3") // Check code at 0x01923a
	PORT_DIPSETTING(    0x00, "Slow" )          // 1 second = 0x01ff
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )       // 1 second = 0x013f
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN4") /* A 6 way dip switch */
	PORT_DIPNAME( 0x01, 0x00, "Price to Continue" )     PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "Same as Start" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* Same as 'gangwarsu' but bits 0-3 of 2nd set of Dip Switches are different */
static INPUT_PORTS_START( gangwars )
	PORT_INCLUDE( gangwarsu )   /* See notes about "IN2" (microcontroller) */

	PORT_MODIFY("IN3")
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:1" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_MODIFY("IN4") /* A 6 way dip switch */
	PORT_DIPNAME( 0x01, 0x00, "Coin Slots" )        PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sbasebal )
	PORT_START("IN0")
	ALPHA68K_PLAYER_INPUT_LSB( 1, IPT_BUTTON3, IPT_START1, IP_ACTIVE_LOW )

	PORT_START("IN1")
	ALPHA68K_PLAYER_INPUT_LSB( 2, IPT_BUTTON3, IPT_START2, IP_ACTIVE_LOW )

	PORT_START("IN2")  /* Coin input to microcontroller */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // COIN2 - unused due to code at 0x0002b4


	PORT_START("IN3")  /* Service + dip */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)

	/* 2 physical sets of _6_ dip switches */
	PORT_DIPNAME( 0x04, 0x04, "Freeze" )                PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")  // Check code at 0x0089e6
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "3:30" )
	PORT_DIPSETTING(    0x80, "3:00" )
	PORT_DIPSETTING(    0x40, "2:30" )
	PORT_DIPSETTING(    0xc0, "2:00" )

	PORT_START("IN4") /* A 6 way dip switch */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")  // Check code at 0x009d3a
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, "Price to Continue" )     PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "Same as Start" )

	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( sbasebalj )
	PORT_START("IN0")
	ALPHA68K_PLAYER_INPUT_LSB( 1, IPT_BUTTON3, IPT_START1, IP_ACTIVE_LOW )

	PORT_START("IN1")
	ALPHA68K_PLAYER_INPUT_LSB( 2, IPT_BUTTON3, IPT_START2, IP_ACTIVE_LOW )

	PORT_START("IN2")  /* Coin input to microcontroller */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )


	PORT_START("IN3")  /* Service + dip */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)

	/* 2 physical sets of _6_ dip switches */
	PORT_DIPNAME( 0x04, 0x04, "Freeze" )                PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")  // Check code at 0x0089e6
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "4:30" )
	PORT_DIPSETTING(    0x80, "4:00" )
	PORT_DIPSETTING(    0x40, "3:30" )
	PORT_DIPSETTING(    0xc0, "3:00" )


	PORT_START("IN4") /* A 6 way dip switch */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")  // Check code at 0x009d3a
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_2C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:6") // Demo sounds seem to be always off in this version
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8x8 */
	RGN_FRAC(1,1),
	4,      /* 4 bits per pixel */
	{ STEP4(0,4) },
	{ STEP4(8*4*4+3,-1), STEP4(3,-1) },
	{ STEP8(0,4*4) },
	32*8 /* every char takes 32 consecutive bytes */
};

/* Same format as neogeo fix layer tiles */
static const gfx_layout charlayout_V =
{
	8,8,
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ STEP4(0,1) },
	{ 16*8+4, 16*8+0, 24*8+4, 24*8+0, 4, 0, 8*8+4, 8*8+0 },
	{ STEP8(0,8) },
	32*8    /* every char takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,      /* 4 bits per pixel */
	{ STEP4(0,8) },
	{ STEP8(16*8*4+7,-1), STEP8(7,-1) },
	{ STEP16(0,8*4) },
	16*16*4    /* every sprite takes 32 consecutive bytes */
};

/******************************************************************************/

static GFXDECODE_START( gfx_alpha68k_II )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0,  16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 128 )
GFXDECODE_END

static GFXDECODE_START( gfx_alpha68k_V )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_V,  0,  16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,  0, 256 )
GFXDECODE_END

/******************************************************************************/

void alpha68k_II_state::porta_w(u8 data)
{
	if (data == 0xff)
		return; // skip

	/* guess */
	if (data == 0 && m_sound_pa_latch) // 1 -> 0 transition = enables NMI
		m_sound_nmi_mask = 1;

	if (data && m_sound_pa_latch == 0) // 0 -> 1 transition = disables NMI
		m_sound_nmi_mask = 0;

	m_sound_pa_latch = data & 1;
}


/******************************************************************************/


MACHINE_START_MEMBER(alpha68k_state,common)
{
	save_item(NAME(m_trigstate));
	save_item(NAME(m_deposits1));
	save_item(NAME(m_deposits2));
	save_item(NAME(m_credits));
	save_item(NAME(m_coinvalue));
	save_item(NAME(m_microcontroller_data));
	save_item(NAME(m_latch));
	save_item(NAME(m_flipscreen));
}

MACHINE_RESET_MEMBER(alpha68k_state,common)
{
	m_trigstate = 0;
	m_deposits1 = 0;
	m_deposits2 = 0;
	m_credits = 0;
	m_coinvalue = 0;
	m_microcontroller_data = 0;
	m_latch = 0;
	m_flipscreen = 0;
}

MACHINE_START_MEMBER(alpha68k_III_state,alpha68k_V)
{
	u8 *ROM = memregion("audiocpu")->base();

	m_audiobank->configure_entries(0, 32, &ROM[0], 0x4000);

	MACHINE_START_CALL_MEMBER(common);

	save_item(NAME(m_bank_base));
	save_item(NAME(m_sound_nmi_mask));
	save_item(NAME(m_sound_pa_latch));
}

MACHINE_RESET_MEMBER(alpha68k_III_state,alpha68k_V)
{
	MACHINE_RESET_CALL_MEMBER(common);

	m_bank_base = 0;
}

MACHINE_RESET_MEMBER(alpha68k_II_state,alpha68k_II)
{
	MACHINE_RESET_CALL_MEMBER(common);
}

MACHINE_START_MEMBER(alpha68k_II_state,alpha68k_II)
{
	u8 *ROM = memregion("audiocpu")->base();

	m_audiobank->configure_entries(0, 32, &ROM[0], 0x4000);

	MACHINE_START_CALL_MEMBER(common);

	save_item(NAME(m_bank_base));
	save_item(NAME(m_sound_nmi_mask));
	save_item(NAME(m_sound_pa_latch));
}

#define ALPHA68K_PIXEL_CLOCK (24_MHz_XTAL / 4)
#define ALPHA68K_HTOTAL 384

void alpha68k_state::set_screen_raw_params(machine_config &config)
{
//  TODO: Same as snk68.cpp, which in turn is awfully similar to NeoGeo CRTC parameters
	m_screen->set_raw(ALPHA68K_PIXEL_CLOCK,ALPHA68K_HTOTAL,0,256,264,16,240);
}

INTERRUPT_GEN_MEMBER(alpha68k_II_state::sound_nmi)
{
	if (m_sound_nmi_mask)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void alpha68k_II_state::base_config(machine_config &config)
{
	LS259(config, m_outlatch); // 14A
	m_outlatch->q_out_cb<2>().set(FUNC(alpha68k_II_state::video_control2_w));
	m_outlatch->q_out_cb<3>().set(FUNC(alpha68k_II_state::video_control3_w));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ym1(YM2203(config, "ym1", ALPHA68K_PIXEL_CLOCK / 2)); // TODO: verify me
	ym1.port_a_write_callback().set(FUNC(alpha68k_II_state::porta_w));
	ym1.add_route(ALL_OUTPUTS, "speaker", 0.65);

	ym2413_device &ym2(YM2413(config, "ym2", 3.579545_MHz_XTAL)); // TODO: verify me
	ym2.add_route(ALL_OUTPUTS, "speaker", 1.0);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.75); // ALPHA-VOICE88 custom DAC
}

void alpha68k_II_state::video_config(machine_config &config, u16 num_pens)
{
	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(alpha68k_II_state::screen_update));
	m_screen->set_palette(m_palette);

	// TODO: should really be same as snk68.cpp
	MCFG_VIDEO_START_OVERRIDE(alpha68k_II_state,alpha68k)

	SNK68_SPR(config, m_sprites, 0);
	m_sprites->set_gfxdecode_tag(m_gfxdecode);
	m_sprites->set_tile_indirect_cb(FUNC(alpha68k_II_state::tile_callback));
	m_sprites->set_xpos_shift(15);
	m_sprites->set_color_entry_mask((num_pens / 16) - 1);

	// TODO: change into NeoGeo palette format ...
	ALPHA68K_PALETTE(config, m_palette, 0);
	m_palette->set_entries(num_pens);
}

void alpha68k_II_state::alpha68k_II(machine_config &config)
{
	base_config(config);
	m_outlatch->parallel_out_cb().set(FUNC(alpha68k_II_state::video_bank_w)).rshift(4).mask(0x07);

	/* basic machine hardware */
	M68000(config, m_maincpu, 8000000); // TODO: verify me
	m_maincpu->set_addrmap(AS_PROGRAM, &alpha68k_II_state::alpha68k_II_map);
	m_maincpu->set_vblank_int("screen", FUNC(alpha68k_state::irq3_line_hold)); /* VBL */

	Z80(config, m_audiocpu, 6000000); // TODO: verify me
	m_audiocpu->set_addrmap(AS_PROGRAM, &alpha68k_II_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &alpha68k_II_state::sound_portmap);
	m_audiocpu->set_periodic_int(FUNC(alpha68k_II_state::sound_nmi), attotime::from_hz(7614));

	MCFG_MACHINE_START_OVERRIDE(alpha68k_II_state,alpha68k_II)
	MCFG_MACHINE_RESET_OVERRIDE(alpha68k_II_state,alpha68k_II)

	video_config(config, 2048);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_alpha68k_II);
}

void alpha68k_II_state::btlfieldb(machine_config &config)
{
	alpha68k_II(config);
	m_maincpu->set_vblank_int("screen", FUNC(alpha68k_II_state::irq1_line_hold));
	// TODO: timing
	m_maincpu->set_periodic_int(FUNC(alpha68k_II_state::irq2_line_hold), attotime::from_hz(60*4)); // MCU irq
}

void goldmedal_II_state::goldmedal(machine_config &config)
{
	alpha68k_II(config);
	m_maincpu->set_vblank_int("screen", FUNC(goldmedal_II_state::irq1_line_hold));
	// TODO: dash events relies on MCU irq timings
	m_maincpu->set_periodic_int(FUNC(goldmedal_II_state::irq2_line_hold), attotime::from_hz(100)); // MCU irq
}

void alpha68k_III_state::alpha68k_III(machine_config &config)
{
	base_config(config);
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &alpha68k_III_state::alpha68k_III_map);
	m_maincpu->set_vblank_int("screen", FUNC(alpha68k_III_state::irq1_line_hold)); /* VBL */

	Z80(config, m_audiocpu, 24_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &alpha68k_III_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &alpha68k_III_state::sound_portmap);
	m_audiocpu->set_periodic_int(FUNC(alpha68k_III_state::sound_nmi), attotime::from_hz(ALPHA68K_PIXEL_CLOCK / ALPHA68K_HTOTAL / 2));

	MCFG_MACHINE_START_OVERRIDE(alpha68k_III_state,alpha68k_V)
	MCFG_MACHINE_RESET_OVERRIDE(alpha68k_III_state,alpha68k_V)

	/* video hardware */
	video_config(config, 4096);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_alpha68k_V);
}

// goldmedla
void goldmedal_III_state::goldmedal(machine_config &config)
{
	alpha68k_III_state::alpha68k_III(config);
	// TODO: dash events relies on MCU irq timings
	m_maincpu->set_periodic_int(FUNC(goldmedal_III_state::irq2_line_hold), attotime::from_hz(100)); // MCU irq
}

void alpha68k_V_state::alpha68k_V(machine_config &config)
{
	alpha68k_III_state::alpha68k_III(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &alpha68k_V_state::alpha68k_V_map);
	m_maincpu->set_vblank_int("screen", FUNC(alpha68k_V_state::irq3_line_hold)); /* VBL */
}

void skyadventure_state::skyadventure(machine_config &config)
{
	alpha68k_V_state::alpha68k_V(config);
	m_sprites->set_tile_indirect_cb(FUNC(skyadventure_state::tile_callback_noflipx));
}

void gangwars_state::gangwars(machine_config &config)
{
	alpha68k_V_state::alpha68k_V(config);
	m_sprites->set_tile_indirect_cb(FUNC(gangwars_state::tile_callback_noflipy));
}

/******************************************************************************/

ROM_START( timesold )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "bf.3",       0x00000,  0x10000, CRC(a491e533) SHA1(e7a2e866e574ea4eb23c1c4cbd312a87c9f81b5e) )
	ROM_LOAD16_BYTE( "bf.4",       0x00001,  0x10000, CRC(34ebaccc) SHA1(dda5350d01cffee51d070eb518beecbaec7e4b21) )
	ROM_LOAD16_BYTE( "bf.1",       0x20000,  0x10000, CRC(158f4cb3) SHA1(48335a1e68afda24e1cca8cce5f869f30c6bda9c) )
	ROM_LOAD16_BYTE( "bf.2",       0x20001,  0x10000, CRC(af01a718) SHA1(588fda345b5ebd75d03d78c431227f220932ee46) )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "bf.7",            0x00000,  0x10000, CRC(f8b293b5) SHA1(d326763628d7cbe864abc15d6db7fa7fe4381f31) )
	ROM_LOAD( "bf.8",            0x20000,  0x10000, CRC(8a43497b) SHA1(c64519b2aced8b072efdd1a6286f082094a50e61) )
	ROM_LOAD( "bf.9",            0x40000,  0x10000, CRC(1408416f) SHA1(d7a32de156791f923635d7fdddc8db97f66bfb2a) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x010000, "gfx1", 0 )  /* chars */
	ROM_LOAD16_BYTE( "bf.6",     0x00001,  0x08000, CRC(086a364d) SHA1(b008d4b351ada4240dd6c82c45405a2489e36019) )
	ROM_LOAD16_BYTE( "bf.5",     0x00000,  0x08000, CRC(3cec2f55) SHA1(e4fca0c8193680385c7dd4d6c599492c9e0dd4af) )

	ROM_REGION( 0x200000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "bf.10",           0x000000, 0x20000, CRC(613313ba) SHA1(4940ddc5f7f4e3165a830dbfa6a65ddb23a33e12) )
	ROM_LOAD32_BYTE( "bf.11",           0x000001, 0x20000, CRC(92b42eba) SHA1(0f76d9fedaced65829a19105bb5cdfbf31c48427) )
	ROM_LOAD32_BYTE( "bf.12",           0x000002, 0x20000, CRC(7ca8bb32) SHA1(bb7747319bebb04965e536b729d76d4c7c5304e1) )
	ROM_LOAD32_BYTE( "bf.13",           0x000003, 0x20000, CRC(56a3a26a) SHA1(d8485f629df98155c706ab0f725dd5fe475f1272) )
	ROM_LOAD32_BYTE( "bf.14",           0x080000, 0x20000, CRC(efda5c45) SHA1(482d855bd0aa8cf28bb2e5ae096a7fa9491a26c8) )
	ROM_LOAD32_BYTE( "bf.15",           0x080001, 0x20000, CRC(ba3b9f5a) SHA1(2461f3a862889d31eee6c1572b1f47b987ac99bd) )
	ROM_LOAD32_BYTE( "bf.16",           0x080002, 0x20000, CRC(2aa74125) SHA1(8323669101ccd2c1b785e27c6a7ea43d6d61a622) )
	ROM_LOAD32_BYTE( "bf.17",           0x080003, 0x20000, CRC(6b37d048) SHA1(bc7b7abd971313e50ac5f69d7dbec9de8a354537) )
	ROM_LOAD32_BYTE( "bf.18",           0x100000, 0x20000, CRC(e886146a) SHA1(cb4b1a002fe0c139d698fb9fd633cc9ff5daf017) )
	ROM_LOAD32_BYTE( "bf.19",           0x100001, 0x20000, CRC(8994bf10) SHA1(424ed2c4009250bdf5cb1ad5015d0b62a2f2a990) )
	ROM_LOAD32_BYTE( "bf.20",           0x100002, 0x20000, CRC(bab6a7c5) SHA1(983f18d58cbb6852adad8723a815168c17a8c82a) )
	ROM_LOAD32_BYTE( "bf.21",           0x100003, 0x20000, CRC(bc3b3944) SHA1(6c99d2b093e5cb04dc3422c2f0f81a20f5a504b5) )
ROM_END

ROM_START( timesold1 )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "3",          0x00000,  0x10000, CRC(bc069a29) SHA1(891a6809931871a1da0a5a4d313623a8b92326e3) )
	ROM_LOAD16_BYTE( "4",          0x00001,  0x10000, CRC(ac7dca56) SHA1(4322d601ea5abe222f2d707fbfbfb3b207509760) )
	ROM_LOAD16_BYTE( "bf.1",       0x20000,  0x10000, CRC(158f4cb3) SHA1(48335a1e68afda24e1cca8cce5f869f30c6bda9c) )
	ROM_LOAD16_BYTE( "bf.2",       0x20001,  0x10000, CRC(af01a718) SHA1(588fda345b5ebd75d03d78c431227f220932ee46) )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "bf.7",            0x00000,  0x10000, CRC(f8b293b5) SHA1(d326763628d7cbe864abc15d6db7fa7fe4381f31) )
	ROM_LOAD( "bf.8",            0x20000,  0x10000, CRC(8a43497b) SHA1(c64519b2aced8b072efdd1a6286f082094a50e61) )
	ROM_LOAD( "bf.9",            0x40000,  0x10000, CRC(1408416f) SHA1(d7a32de156791f923635d7fdddc8db97f66bfb2a) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x010000, "gfx1", 0 )  /* chars */
	ROM_LOAD16_BYTE( "bf.6",     0x00001,  0x08000, CRC(086a364d) SHA1(b008d4b351ada4240dd6c82c45405a2489e36019) )
	ROM_LOAD16_BYTE( "bf.5",     0x00000,  0x08000, CRC(3cec2f55) SHA1(e4fca0c8193680385c7dd4d6c599492c9e0dd4af) )

	ROM_REGION( 0x200000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "bf.10",           0x000000, 0x20000, CRC(613313ba) SHA1(4940ddc5f7f4e3165a830dbfa6a65ddb23a33e12) )
	ROM_LOAD32_BYTE( "bf.11",           0x000001, 0x20000, CRC(92b42eba) SHA1(0f76d9fedaced65829a19105bb5cdfbf31c48427) )
	ROM_LOAD32_BYTE( "bf.12",           0x000002, 0x20000, CRC(7ca8bb32) SHA1(bb7747319bebb04965e536b729d76d4c7c5304e1) )
	ROM_LOAD32_BYTE( "bf.13",           0x000003, 0x20000, CRC(56a3a26a) SHA1(d8485f629df98155c706ab0f725dd5fe475f1272) )
	ROM_LOAD32_BYTE( "bf.14",           0x080000, 0x20000, CRC(efda5c45) SHA1(482d855bd0aa8cf28bb2e5ae096a7fa9491a26c8) )
	ROM_LOAD32_BYTE( "bf.15",           0x080001, 0x20000, CRC(ba3b9f5a) SHA1(2461f3a862889d31eee6c1572b1f47b987ac99bd) )
	ROM_LOAD32_BYTE( "bf.16",           0x080002, 0x20000, CRC(2aa74125) SHA1(8323669101ccd2c1b785e27c6a7ea43d6d61a622) )
	ROM_LOAD32_BYTE( "bf.17",           0x080003, 0x20000, CRC(6b37d048) SHA1(bc7b7abd971313e50ac5f69d7dbec9de8a354537) )
	ROM_LOAD32_BYTE( "bf.18",           0x100000, 0x20000, CRC(e886146a) SHA1(cb4b1a002fe0c139d698fb9fd633cc9ff5daf017) )
	ROM_LOAD32_BYTE( "bf.19",           0x100001, 0x20000, CRC(8994bf10) SHA1(424ed2c4009250bdf5cb1ad5015d0b62a2f2a990) )
	ROM_LOAD32_BYTE( "bf.20",           0x100002, 0x20000, CRC(bab6a7c5) SHA1(983f18d58cbb6852adad8723a815168c17a8c82a) )
	ROM_LOAD32_BYTE( "bf.21",           0x100003, 0x20000, CRC(bc3b3944) SHA1(6c99d2b093e5cb04dc3422c2f0f81a20f5a504b5) )
ROM_END

ROM_START( btlfield )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "bfv1_03.bin", 0x00000, 0x10000, CRC(8720af0d) SHA1(3a26dc06d98c16600b9fa0b1a12f703feac48c9d) )
	ROM_LOAD16_BYTE( "bfv1_04.bin", 0x00001, 0x10000, CRC(7dcccbe6) SHA1(33b69c139c94a9d292c93b4f148441e1bda5aba5) )
	ROM_LOAD16_BYTE( "bf.1",        0x20000, 0x10000, CRC(158f4cb3) SHA1(48335a1e68afda24e1cca8cce5f869f30c6bda9c) )
	ROM_LOAD16_BYTE( "bf.2",        0x20001, 0x10000, CRC(af01a718) SHA1(588fda345b5ebd75d03d78c431227f220932ee46) )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "bf.7",            0x00000,  0x10000, CRC(f8b293b5) SHA1(d326763628d7cbe864abc15d6db7fa7fe4381f31) )
	ROM_LOAD( "bf.8",            0x20000,  0x10000, CRC(8a43497b) SHA1(c64519b2aced8b072efdd1a6286f082094a50e61) )
	ROM_LOAD( "bf.9",            0x40000,  0x10000, CRC(1408416f) SHA1(d7a32de156791f923635d7fdddc8db97f66bfb2a) )

	ROM_REGION( 0x010000, "gfx1", 0 )  /* chars */
	ROM_LOAD16_BYTE( "bfv1_06.bin", 0x00001, 0x08000, CRC(022b9de9) SHA1(5a736a4cfe05e7681c78ab816dfe04074fe0293d) )
	ROM_LOAD16_BYTE( "bfv1_05.bin", 0x00000, 0x08000, CRC(be269dbf) SHA1(3240badbf65e076cc1f7caaec1081df9a4371d47) )

	ROM_REGION( 0x200000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "bf.10",           0x000000, 0x20000, CRC(613313ba) SHA1(4940ddc5f7f4e3165a830dbfa6a65ddb23a33e12) )
	ROM_LOAD32_BYTE( "bf.11",           0x000001, 0x20000, CRC(92b42eba) SHA1(0f76d9fedaced65829a19105bb5cdfbf31c48427) )
	ROM_LOAD32_BYTE( "bf.12",           0x000002, 0x20000, CRC(7ca8bb32) SHA1(bb7747319bebb04965e536b729d76d4c7c5304e1) )
	ROM_LOAD32_BYTE( "bf.13",           0x000003, 0x20000, CRC(56a3a26a) SHA1(d8485f629df98155c706ab0f725dd5fe475f1272) )
	ROM_LOAD32_BYTE( "bf.14",           0x080000, 0x20000, CRC(efda5c45) SHA1(482d855bd0aa8cf28bb2e5ae096a7fa9491a26c8) )
	ROM_LOAD32_BYTE( "bf.15",           0x080001, 0x20000, CRC(ba3b9f5a) SHA1(2461f3a862889d31eee6c1572b1f47b987ac99bd) )
	ROM_LOAD32_BYTE( "bf.16",           0x080002, 0x20000, CRC(2aa74125) SHA1(8323669101ccd2c1b785e27c6a7ea43d6d61a622) )
	ROM_LOAD32_BYTE( "bf.17",           0x080003, 0x20000, CRC(6b37d048) SHA1(bc7b7abd971313e50ac5f69d7dbec9de8a354537) )
	ROM_LOAD32_BYTE( "bf.18",           0x100000, 0x20000, CRC(e886146a) SHA1(cb4b1a002fe0c139d698fb9fd633cc9ff5daf017) )
	ROM_LOAD32_BYTE( "bf.19",           0x100001, 0x20000, CRC(8994bf10) SHA1(424ed2c4009250bdf5cb1ad5015d0b62a2f2a990) )
	ROM_LOAD32_BYTE( "bf.20",           0x100002, 0x20000, CRC(bab6a7c5) SHA1(983f18d58cbb6852adad8723a815168c17a8c82a) )
	ROM_LOAD32_BYTE( "bf.21",           0x100003, 0x20000, CRC(bc3b3944) SHA1(6c99d2b093e5cb04dc3422c2f0f81a20f5a504b5) )
ROM_END

ROM_START( btlfieldb )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "3.bin",      0x00000, 0x10000, CRC(141f10ca) SHA1(4f6a59975964c92693476576533aba80c089b5ef) )
	ROM_LOAD16_BYTE( "1.bin",      0x00001, 0x10000, CRC(caa09adf) SHA1(5df0775119b3e957bbe620142a5454e337bdf4b8) )
	ROM_LOAD16_BYTE( "bf.1",       0x20000, 0x10000, CRC(158f4cb3) SHA1(48335a1e68afda24e1cca8cce5f869f30c6bda9c) )
	ROM_LOAD16_BYTE( "bf.2",       0x20001, 0x10000, CRC(af01a718) SHA1(588fda345b5ebd75d03d78c431227f220932ee46) )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "bf.7",            0x00000,  0x10000, CRC(f8b293b5) SHA1(d326763628d7cbe864abc15d6db7fa7fe4381f31) )
	ROM_LOAD( "bf.8",            0x20000,  0x10000, CRC(8a43497b) SHA1(c64519b2aced8b072efdd1a6286f082094a50e61) )
	ROM_LOAD( "bf.9",            0x40000,  0x10000, CRC(1408416f) SHA1(d7a32de156791f923635d7fdddc8db97f66bfb2a) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x010000, "gfx1", 0 )  /* chars */
	ROM_LOAD16_BYTE( "bfv1_06.bin", 0x00001, 0x08000, CRC(022b9de9) SHA1(5a736a4cfe05e7681c78ab816dfe04074fe0293d) )
	ROM_LOAD16_BYTE( "bfv1_05.bin", 0x00000, 0x08000, CRC(be269dbf) SHA1(3240badbf65e076cc1f7caaec1081df9a4371d47) )

	ROM_REGION( 0x200000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "12.bin",          0x000000, 0x10000, CRC(8cab60f2) SHA1(92410d430cab112e87888a9cf50c304957f43be0) )
	ROM_LOAD32_BYTE( "16.bin",          0x000001, 0x10000, CRC(6166553a) SHA1(c2279589646ebd8ea77c477e5f2641048ab6f0d4) )
	ROM_LOAD32_BYTE( "21.bin",          0x000002, 0x10000, CRC(81b75cdc) SHA1(8f529f293899c34d5be31bb675dda6706c2d29a9) )
	ROM_LOAD32_BYTE( "28.bin",          0x000003, 0x10000, CRC(3399e86e) SHA1(6e9a6a623ce2a2a76edfd328192ba85cd820dbc3) )
	ROM_LOAD32_BYTE( "11.bin",          0x040000, 0x10000, CRC(e296581e) SHA1(5d889e9365ecff7c518e39130c55a7b836ff2861) )
	ROM_LOAD32_BYTE( "15.bin",          0x040001, 0x10000, CRC(805439d7) SHA1(385d05341aa71cee96a92eaeec13f4b45d420726) )
	ROM_LOAD32_BYTE( "22.bin",          0x040002, 0x10000, CRC(5231e4df) SHA1(b3a75eed0496a7437af5f970b23e495ecfe17422) )
	ROM_LOAD32_BYTE( "27.bin",          0x040003, 0x10000, CRC(86529c8a) SHA1(500d1651b0a86db4c0dd8c357a6363ef2a07b5ba) )
	ROM_LOAD32_BYTE( "8.bin",           0x080000, 0x10000, CRC(54462294) SHA1(b772729bbc54b26b09ccbf953c3b054ed4e22273) )
	ROM_LOAD32_BYTE( "13.bin",          0x080001, 0x10000, CRC(5622cb93) SHA1(e8683998714430836313b72d6be894129b65c772) )
	ROM_LOAD32_BYTE( "20.bin",          0x080002, 0x10000, CRC(5a944f3e) SHA1(dad0322800348465393fdce9c379636b3829b76c) )
	ROM_LOAD32_BYTE( "25.bin",          0x080003, 0x10000, CRC(6764d81b) SHA1(331e455fb228f2b6e97124d620688ec66da50603) )
	ROM_LOAD32_BYTE( "7.bin",           0x0c0000, 0x10000, CRC(03b18a1d) SHA1(d4d288ccbe193accc3fd5c04b7152197305a69b7) )
	ROM_LOAD32_BYTE( "14.bin",          0x0c0001, 0x10000, CRC(67860390) SHA1(1eb25a7e84a10f1385222ab992412db68b4e8266) )
	ROM_LOAD32_BYTE( "19.bin",          0x0c0002, 0x10000, CRC(db1dcd5e) SHA1(cc1d442b7a20ad923f88365f834ce608a56d6f6e) )
	ROM_LOAD32_BYTE( "26.bin",          0x0c0003, 0x10000, CRC(335b7b50) SHA1(8649a710e07835c7ed88efbf8c963a0b5c81d170) )
	ROM_LOAD32_BYTE( "5.bin",           0x100000, 0x10000, CRC(d6f3d746) SHA1(3f4401d1ab024f8d1a6ab9dd4116d24cad76c3b1) )
	ROM_LOAD32_BYTE( "9.bin",           0x100001, 0x10000, CRC(02f73dc5) SHA1(369851080f4716ea9a7b9fa40aa40b4a55bfe0ba) )
	ROM_LOAD32_BYTE( "17.bin",          0x100002, 0x10000, CRC(69210ee6) SHA1(b627d8694821a66ca9825f05ce7f4997b4bfe60c) )
	ROM_LOAD32_BYTE( "24.bin",          0x100003, 0x10000, CRC(2e78684a) SHA1(b4a64fef05c11f06640b461b264ec11a86f4119d) )
	ROM_LOAD32_BYTE( "6.bin",           0x140000, 0x10000, CRC(07714f39) SHA1(8a9b62947b29955598218f924b9f76c027177295) )
	ROM_LOAD32_BYTE( "10.bin",          0x140001, 0x10000, CRC(e5de7eb8) SHA1(534000a03548f6f7cf466511caf203c487aeaa79) )
	ROM_LOAD32_BYTE( "18.bin",          0x140002, 0x10000, CRC(40bf0b3d) SHA1(db084a0879845b265ca3f9359e19d907dc2ac00c) )
	ROM_LOAD32_BYTE( "23.bin",          0x140003, 0x10000, CRC(500e0dbc) SHA1(88caea968340b3932469f8149bc7a08a7ca7d285) )
ROM_END

ROM_START( skysoldr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ss.3",     0x00000, 0x10000, CRC(7b88aa2e) SHA1(17ed682fb67e8fa05a1309e87ac29c09adcd7474) )
	ROM_CONTINUE ( 0x40000,      0x10000 )
	ROM_LOAD16_BYTE( "ss.4",     0x00001, 0x10000, CRC(f0283d43) SHA1(bfbc7235c9ff52b9ab269247e9c4a9d574ba25e2) )
	ROM_CONTINUE ( 0x40001,      0x10000 )
	ROM_LOAD16_BYTE( "ss.1",     0x20000, 0x10000, CRC(20e9dbc7) SHA1(632e5c7348a88620b85f968501a33609cc993972) )
	ROM_CONTINUE ( 0x60000,      0x10000 )
	ROM_LOAD16_BYTE( "ss.2",     0x20001, 0x10000, CRC(486f3432) SHA1(56b6c74031001bccb98e73f228e697556e8111d4) )
	ROM_CONTINUE ( 0x60001,      0x10000 )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "ss.7",            0x00000, 0x10000, CRC(b711fad4) SHA1(0a9515cb36b8d03ee5f7e0669a9948571b4ec34e) )
	ROM_LOAD( "ss.8",            0x20000, 0x10000, CRC(e5cf7b37) SHA1(770ee80a1cc0f877486c6b47812db2b1118651d9) )
	ROM_LOAD( "ss.9",            0x40000, 0x10000, CRC(76124ca2) SHA1(5b87178ab663cd8aa67670f0c14c9cbb8616b04d) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x010000, "gfx1", 0 )  /* chars */
	ROM_LOAD16_BYTE( "ss.6",     0x00001, 0x08000, CRC(93b30b55) SHA1(51cacc48f4a298131852d41da80126bda5988920) )
	ROM_LOAD16_BYTE( "ss.5",     0x00000, 0x08000, CRC(928ba287) SHA1(c415c5b84b83ee0e5e0aa60eb33132145fcd7487) )

	ROM_REGION( 0x200000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "ss.10",          0x000000, 0x20000, CRC(e48c1623) SHA1(1181e16d0d36d246ce4401b5cdacb0780acf835c) )
	ROM_LOAD32_BYTE( "ss.11",          0x000001, 0x20000, CRC(6c63e9c5) SHA1(cb349a8e0a2a1bcd098c59a879d8e4cbb51adee6) )
	ROM_LOAD32_BYTE( "ss.12",          0x000002, 0x20000, CRC(63bb4e89) SHA1(98b5d61469c18830459ccf8aaa51d101e4ae0a4f) )
	ROM_LOAD32_BYTE( "ss.13",          0x000003, 0x20000, CRC(3506c06b) SHA1(434f12523973de0d08ae30320d833be445c36e3f) )
	ROM_LOAD32_BYTE( "ss.14",          0x080000, 0x20000, CRC(190c8704) SHA1(88b12720d0aa01f9df5792f8f3229e582995e4a1) )
	ROM_LOAD32_BYTE( "ss.15",          0x080001, 0x20000, CRC(55f71ab1) SHA1(2cb69d5532b35915b1fe5653a351508eccabd945) )
	ROM_LOAD32_BYTE( "ss.16",          0x080002, 0x20000, CRC(138179f7) SHA1(2eb02077ffa4a87feed2c0503b294730f67ac484) )
	ROM_LOAD32_BYTE( "ss.17",          0x080003, 0x20000, CRC(a7f524e0) SHA1(97e3e637d754a8c8c25c9b8bf8f67b1f8776e460) )
	ROM_LOAD32_BYTE( "ss.18",          0x100000, 0x20000, CRC(cb6ff33a) SHA1(b1e24b11e0c5da12d21282120e3aab8773854e11) )
	ROM_LOAD32_BYTE( "ss.19",          0x100001, 0x20000, CRC(312a21f5) SHA1(aef32d805cd338c3980cb282e86157590fbfe791) )
	ROM_LOAD32_BYTE( "ss.20",          0x100002, 0x20000, CRC(268cc7b4) SHA1(a9fe6b3302bafd52214a7eb0ad8a8a238eb29542) )
	ROM_LOAD32_BYTE( "ss.21",          0x100003, 0x20000, CRC(cb7bf5fe) SHA1(8ded0be6b7d3ac4478234df9e72e6cad95f36d21) )
	ROM_LOAD32_BYTE( "ss.22",          0x180000, 0x20000, CRC(e69b4485) SHA1(041fcadb84d92bca4650768ad954fe47cfa00e62) )
	ROM_LOAD32_BYTE( "ss.23",          0x180001, 0x20000, CRC(923c19c2) SHA1(8ca88e49dc246798fd0c59cff2669a15aaa933a6) )
	ROM_LOAD32_BYTE( "ss.24",          0x180002, 0x20000, CRC(f63b8417) SHA1(6a629ae6dd2b15c840f778a0f2ce9476ef578d5e) )
	ROM_LOAD32_BYTE( "ss.25",          0x180003, 0x20000, CRC(65138016) SHA1(871b0ba39710b1094519cd808339e80ea366a016) )
ROM_END

ROM_START( skysoldrbl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "g.bin",     0x00000, 0x10000, CRC(4d3273e9) SHA1(7ddaba59114180fe371d2326fc49d6274e58f5c9) )  //different from other set
	ROM_LOAD16_BYTE( "c.bin",     0x00001, 0x10000, CRC(86c7af62) SHA1(4092558f3c11130e917d06b8d43f8f00815e4148) )  //different from other set
	ROM_LOAD16_BYTE( "e.bin",     0x20000, 0x10000, CRC(03115b75) SHA1(e36f2eab0198bf6b5b419aacc593b3790b479e81) )
	ROM_LOAD16_BYTE( "a.bin",     0x20001, 0x10000, CRC(7aa103c7) SHA1(1907b92a3769089e01af36f74e0ff30e7a8f178c) )
	ROM_LOAD16_BYTE( "h-gtop.bin",     0x40000, 0x10000, CRC(f41dfeab) SHA1(1b4f68c0f55e89a9dcd0fae8fb26074b97b5303a) )
	ROM_LOAD16_BYTE( "d-ctop.bin",     0x40001, 0x10000, CRC(56560a3c) SHA1(c57c33d3935c23d56ae256981e4c3dcd80fb86a2) )
	ROM_LOAD16_BYTE( "f-etop.bin",     0x60000, 0x10000, CRC(60a52583) SHA1(975d309ba55730c87cb5ea786c4d2d82358a1b73) )
	ROM_LOAD16_BYTE( "b-atop.bin",     0x60001, 0x10000, CRC(028fd31b) SHA1(feb18a7217c107bb5f8e5c5ec5bc4173e977286b) )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "33.ic11",            0x00000, 0x10000, CRC(b711fad4) SHA1(0a9515cb36b8d03ee5f7e0669a9948571b4ec34e) )
	ROM_LOAD( "34.ic12",            0x20000, 0x10000, CRC(e5cf7b37) SHA1(770ee80a1cc0f877486c6b47812db2b1118651d9) )
	ROM_LOAD( "35.ic13",            0x40000, 0x10000, CRC(76124ca2) SHA1(5b87178ab663cd8aa67670f0c14c9cbb8616b04d) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "68705r3p.mcu", 0x000, 0x1000, NO_DUMP ) // the BOOTLEGs use a 68705, I think it's programmed to act the same way as the original MCU tho.

	ROM_REGION( 0x010000, "gfx1", 0 )  /* chars */
	ROM_LOAD16_BYTE( "xx.ic1",   0x00001, 0x08000, CRC(93b30b55) SHA1(51cacc48f4a298131852d41da80126bda5988920) )
	ROM_LOAD16_BYTE( "xx.ic2",   0x00000, 0x08000, CRC(928ba287) SHA1(c415c5b84b83ee0e5e0aa60eb33132145fcd7487) )

	ROM_REGION( 0x200000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "26.ica9",         0x000000, 0x10000, CRC(2aad8c4d) SHA1(ff407b43cede6a0f2c199be82bad0c491975c2ad) )
	ROM_LOAD32_BYTE( "18.ica1",         0x000001, 0x10000, CRC(08419273) SHA1(0ded4b60b0ce17a922fb7170f992c4f6c75be895) )
	ROM_LOAD32_BYTE( "10.ic23",         0x000002, 0x10000, CRC(8e67a39e) SHA1(9a26e8119604cd965cc6afb1474f6db8dcdcc12c) )
	ROM_LOAD32_BYTE( "2.ic15",          0x000003, 0x10000, CRC(022bcdc1) SHA1(30cc680e1947713eb6f87684e45d286da711e443) )
	ROM_LOAD32_BYTE( "25.ica8",         0x040000, 0x10000, CRC(7bca633e) SHA1(fe8610608c2bb457669dbf6a19d7681f145a93e7) )
	ROM_LOAD32_BYTE( "17.ic30",         0x040001, 0x10000, CRC(6258a61b) SHA1(d56a9f1dfa02dc59935f03b86a134076e3039bf4) )
	ROM_LOAD32_BYTE( "9.ic22",          0x040002, 0x10000, CRC(6f6d2593) SHA1(7ceb54fa685be7a860e96acaba6983dea2b63b87) )
	ROM_LOAD32_BYTE( "1.ic14",          0x040003, 0x10000, CRC(129a58b5) SHA1(cfcb1e475651cf59e81b045b166708eeac0ba458) )
	ROM_LOAD32_BYTE( "28.ic41",         0x080000, 0x10000, CRC(da94809d) SHA1(35c99e98cdfa444d7af689894b078519b5eb2ed4) )
	ROM_LOAD32_BYTE( "20.ica3",         0x080001, 0x10000, CRC(5e716c62) SHA1(9427cd1578221ee48f4a8d8a24a232cb9e9b2206) )
	ROM_LOAD32_BYTE( "12.ic25",         0x080002, 0x10000, CRC(549182ba) SHA1(0068348340893e589196a43d7dbfb80ee8019a17) )
	ROM_LOAD32_BYTE( "4.ic17",          0x080003, 0x10000, CRC(ccaf1968) SHA1(7d302ec0b2fe9b440c2ee7503924e8b97d2fead0) )
	ROM_LOAD32_BYTE( "27.ica10",        0x0c0000, 0x10000, CRC(dd1e56c0) SHA1(e2bfc85518ee4de1fb9cd58a358c450c6acf652b) )
	ROM_LOAD32_BYTE( "19.ica2",         0x0c0001, 0x10000, CRC(f3922f1e) SHA1(e6ec6d1ea4cb23a78c61b6e5ab794a44b451b8b7) )
	ROM_LOAD32_BYTE( "11.ic24",         0x0c0002, 0x10000, CRC(b5b06e28) SHA1(535a18c955c338fe9e1140a63e19402e81aeb5a2) )
	ROM_LOAD32_BYTE( "3.ic16",          0x0c0003, 0x10000, CRC(796999ba) SHA1(1ffd862b99d3452160e047af9445da0c1a709d55) )
	ROM_LOAD32_BYTE( "30.ic43",         0x100000, 0x10000, CRC(9eb10d3d) SHA1(ba1445e2c166f72a67295d595990efbdd3460736) )
	ROM_LOAD32_BYTE( "22.ica5",         0x100001, 0x10000, CRC(1a7c2f20) SHA1(9951185635c02822fd337ed1ddf91a5e334180a0) )
	ROM_LOAD32_BYTE( "14.ic27",         0x100002, 0x10000, CRC(1498a515) SHA1(c67f4a767823bf1e9cec33b332ebd68befe27ca2) )
	ROM_LOAD32_BYTE( "6.ic19",          0x100003, 0x10000, CRC(45b1ab8a) SHA1(c106f17890eb5be77f1eaf4eea0b07b59174b197) )
	ROM_LOAD32_BYTE( "31.ic44",         0x140000, 0x10000, CRC(6b6c4e56) SHA1(1af79c5931be2eb1421172e6e7877a97681fdb84) )
	ROM_LOAD32_BYTE( "23.ica6",         0x140001, 0x10000, CRC(3155aca2) SHA1(204250660cfaaea5674aa56c888a55c8e2e894a1) )
	ROM_LOAD32_BYTE( "15.ic28",         0x140002, 0x10000, CRC(34545c01) SHA1(9abeff9723f38537bf01fd928d01018a37da6669) )
	ROM_LOAD32_BYTE( "7.ic20",          0x140003, 0x10000, CRC(052247d1) SHA1(386d24704702845108273262a4802afa35be1850) )
	ROM_LOAD32_BYTE( "32.ic45",         0x180000, 0x10000, CRC(fdf55eca) SHA1(0c61ab8fc60c69c4d3fa976976f42bda63c06549) )
	ROM_LOAD32_BYTE( "24.ica7",         0x180001, 0x10000, CRC(8fc95590) SHA1(05b4675c3dd957bff5c9d106cdcc7595e240c781) )
	ROM_LOAD32_BYTE( "16.ic29",         0x180002, 0x10000, CRC(ea5c20a1) SHA1(5693f76b084159a1443d6fb2c587b68a38a6ed9d) )
	ROM_LOAD32_BYTE( "8.ic21",          0x180003, 0x10000, CRC(f670ce4b) SHA1(2844b927e20612ae71b11f543dcfc926d70923a6) )
	ROM_LOAD32_BYTE( "29.ic42",         0x1c0000, 0x10000, CRC(cf888369) SHA1(d1ef5b2c81bbf4e039cc6cfee8339700a1dbb4ca) )  //different from other set (is one of them bad?)
	ROM_LOAD32_BYTE( "21.ica4",         0x1c0001, 0x10000, CRC(f7ea25b0) SHA1(9d92120e3636fc3306e3d5e994ae9171a3be4d42) )
	ROM_LOAD32_BYTE( "13.ic26",         0x1c0002, 0x10000, CRC(962a3e28) SHA1(1c78a099ec282bace2c22cb4484b3e3d525db3a8) )
	ROM_LOAD32_BYTE( "5.ic18",          0x1c0003, 0x10000, CRC(fe34cd89) SHA1(ea86405da4d83e2f438fe28cdbc4e460d680e5e8) )
ROM_END

ROM_START( goldmedl )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gm.3",      0x00000,  0x10000, CRC(ddf0113c) SHA1(1efe39da1e25e7a556c48243a15d95388bc67e69) )
	ROM_LOAD16_BYTE( "gm.4",      0x00001,  0x10000, CRC(16db4326) SHA1(7c82afcdabbb9ce082025b444ad967817ba36879) )
	ROM_LOAD16_BYTE( "gm.1",      0x20000,  0x10000, CRC(54a11e28) SHA1(5e36c86b4d30b07539d9d00c682cbc3d88b6ba01) )
	ROM_LOAD16_BYTE( "gm.2",      0x20001,  0x10000, CRC(4b6a13e4) SHA1(fb6bd4690f4f7aa7ae082c31c366c09e1eda801d) )

#if 0 // old ROM map
	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "goldsnd0.c47",   0x00000,  0x08000, BAD_DUMP CRC(031d27dc) ) // bad dump
	ROM_CONTINUE(               0x10000,  0x78000 )
#endif

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, "audiocpu", 0 ) // banking is slightly different from other Alpha68kII games
	ROM_LOAD( "38.bin",          0x00000,  0x10000, BAD_DUMP CRC(4bf251b8) SHA1(d69a6607e92dbe8081c7c66b6853f02d578ef73f) ) // we use the bootleg set instead
	ROM_LOAD( "39.bin",          0x10000,  0x10000, BAD_DUMP CRC(1d92be86) SHA1(9b6e7141653ee7b7b1915a545d381419aec4e483) )
	ROM_LOAD( "40.bin",          0x20000,  0x10000, BAD_DUMP CRC(8dafc4e8) SHA1(7d4898557ad638ab8461060bc7ae406d7d24c5a4) )
	ROM_LOAD( "1.bin",           0x30000,  0x10000, BAD_DUMP CRC(1e78062c) SHA1(821c037edf32eb8b03e5c487d3bab0622337e80b) )

	ROM_REGION( 0x010000, "gfx1", 0 )  /* chars */
	ROM_LOAD16_BYTE( "gm.6",     0x00001, 0x08000, CRC(56020b13) SHA1(17e176a9c82ed0d6cb5c4014034ce4e16b8ef4fb) )
	ROM_LOAD16_BYTE( "gm.5",     0x00000, 0x08000, CRC(667f33f1) SHA1(6d05603b49927f09c9bb34e787b003eceaaf7062) )

	ROM_REGION( 0x200000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "goldchr3.c46",   0x000000, 0x80000, CRC(6faaa07a) SHA1(8c81ac35220835691d7620b334e83f1fb4f79a52) )
	ROM_LOAD32_BYTE( "goldchr2.c45",   0x000001, 0x80000, CRC(e6b0aa2c) SHA1(88d852803d92147d75853f0e7efa0f2a71820ac6) )
	ROM_LOAD32_BYTE( "goldchr1.c44",   0x000002, 0x80000, CRC(55db41cd) SHA1(15fa192ea2b829dc6dc0cb88fc2c5e5a30af6c91) )
	ROM_LOAD32_BYTE( "goldchr0.c43",   0x000003, 0x80000, CRC(76572c3f) SHA1(e7a1abf4240510810a0f9663295c0fbab9e55a63) )
ROM_END

// it runs in an Alpha-68K96III system board
ROM_START( goldmedla )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gm3-7.bin", 0x00000, 0x10000, CRC(11a63f4c) SHA1(840a8f1f6d80d0395c65f8ad30cc6bfe5a9693f4) )
	ROM_LOAD16_BYTE( "gm4-7.bin", 0x00001, 0x10000, CRC(e19966af) SHA1(a2523627fcc9f5e4a82b4ebec937880fc0e0e9f3) )
	ROM_LOAD16_BYTE( "gm1-7.bin", 0x40000, 0x10000, CRC(6d87b8a6) SHA1(6f47b42d6577691334784e961a991de2ad67f677) )
	ROM_LOAD16_BYTE( "gm2-7.bin", 0x40001, 0x10000, CRC(8d579505) SHA1(81f225edbba1cac65275e2929336d076afbbd2bf) )
	ROM_COPY( "maincpu", 0x40000, 0x60000, 0x20000 )

	ROM_REGION( 0x80000, "audiocpu", 0 ) // banking is slightly different from other Alpha68kII games
	ROM_LOAD( "38.bin",          0x00000,  0x10000, BAD_DUMP CRC(4bf251b8) SHA1(d69a6607e92dbe8081c7c66b6853f02d578ef73f) ) // we use the bootleg set instead
	ROM_LOAD( "39.bin",          0x10000,  0x10000, BAD_DUMP CRC(1d92be86) SHA1(9b6e7141653ee7b7b1915a545d381419aec4e483) )
	ROM_LOAD( "40.bin",          0x20000,  0x10000, BAD_DUMP CRC(8dafc4e8) SHA1(7d4898557ad638ab8461060bc7ae406d7d24c5a4) )
	ROM_LOAD( "1.bin",           0x30000,  0x10000, BAD_DUMP CRC(1e78062c) SHA1(821c037edf32eb8b03e5c487d3bab0622337e80b) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x010000, "gfx1", 0 )  /* chars */
	ROM_LOAD( "gm5-1.bin", 0x000000, 0x10000, CRC(77c601a3) SHA1(5db88b0000fa5e460aa431ca7b75e8fcf629e31e) )

	ROM_REGION( 0x200000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "goldchr3.c46",   0x000000, 0x80000, CRC(6faaa07a) SHA1(8c81ac35220835691d7620b334e83f1fb4f79a52) )
	ROM_LOAD32_BYTE( "goldchr2.c45",   0x000001, 0x80000, CRC(e6b0aa2c) SHA1(88d852803d92147d75853f0e7efa0f2a71820ac6) )
	ROM_LOAD32_BYTE( "goldchr1.c44",   0x000002, 0x80000, CRC(55db41cd) SHA1(15fa192ea2b829dc6dc0cb88fc2c5e5a30af6c91) )
	ROM_LOAD32_BYTE( "goldchr0.c43",   0x000003, 0x80000, CRC(76572c3f) SHA1(e7a1abf4240510810a0f9663295c0fbab9e55a63) )

	ROM_REGION( 0x10000, "user1", 0 ) // TODO: legacy gfx roms, are these even on this specific board?
	ROM_LOAD16_BYTE( "gm.6",     0x00001, 0x08000, BAD_DUMP CRC(56020b13) SHA1(17e176a9c82ed0d6cb5c4014034ce4e16b8ef4fb) )
	ROM_LOAD16_BYTE( "gm.5",     0x00000, 0x08000, BAD_DUMP CRC(667f33f1) SHA1(6d05603b49927f09c9bb34e787b003eceaaf7062) )
ROM_END

//AT: the bootleg set has strong resemblance of "goldmed7" on an Alpha-68K96III system board
ROM_START( goldmedlb )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "l_3.bin",   0x00000,  0x10000, CRC(5e106bcf) SHA1(421ddfdd5ef1e9b5b7c45617fd690df982d63c4b) )
	ROM_LOAD16_BYTE( "l_4.bin",   0x00001,  0x10000, CRC(e19966af) SHA1(a2523627fcc9f5e4a82b4ebec937880fc0e0e9f3) )
	ROM_LOAD16_BYTE( "l_1.bin",   0x40000,  0x08000, CRC(7eec7ee5) SHA1(4fbb0832f50a83e5060c6891aacccc8f28a84086) )
	ROM_LOAD16_BYTE( "l_2.bin",   0x40001,  0x08000, CRC(bf59e4f9) SHA1(76c276c54f0f1cc08db7f0169fb7a1357278a1fd) )
	ROM_COPY( "maincpu", 0x40000, 0x60000, 0x20000 )

	ROM_REGION( 0x80000, "audiocpu", 0 ) //AT: looks identical to goldsnd0.c47
	ROM_LOAD( "38.bin",          0x00000,  0x10000, CRC(4bf251b8) SHA1(d69a6607e92dbe8081c7c66b6853f02d578ef73f) )
	ROM_LOAD( "39.bin",          0x10000,  0x10000, CRC(1d92be86) SHA1(9b6e7141653ee7b7b1915a545d381419aec4e483) )
	ROM_LOAD( "40.bin",          0x20000,  0x10000, CRC(8dafc4e8) SHA1(7d4898557ad638ab8461060bc7ae406d7d24c5a4) )
	ROM_LOAD( "1.bin",           0x30000,  0x10000, CRC(1e78062c) SHA1(821c037edf32eb8b03e5c487d3bab0622337e80b) )

	ROM_REGION( 0x010000, "gfx1", 0 )  /* chars */
	ROM_LOAD( "l_5.bin",   0x00000,  0x10000, CRC(77c601a3) SHA1(5db88b0000fa5e460aa431ca7b75e8fcf629e31e) ) // identical to gm5-1.bin in "goldmed7"

	/* I haven't yet verified if these are the same as the bootleg */

	ROM_REGION( 0x200000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "goldchr3.c46",   0x000000, 0x80000, CRC(6faaa07a) SHA1(8c81ac35220835691d7620b334e83f1fb4f79a52) )
	ROM_LOAD32_BYTE( "goldchr2.c45",   0x000001, 0x80000, CRC(e6b0aa2c) SHA1(88d852803d92147d75853f0e7efa0f2a71820ac6) )
	ROM_LOAD32_BYTE( "goldchr1.c44",   0x000002, 0x80000, CRC(55db41cd) SHA1(15fa192ea2b829dc6dc0cb88fc2c5e5a30af6c91) )
	ROM_LOAD32_BYTE( "goldchr0.c43",   0x000003, 0x80000, CRC(76572c3f) SHA1(e7a1abf4240510810a0f9663295c0fbab9e55a63) )

	ROM_REGION( 0x10000, "user1", 0 ) // TODO: legacy gfx roms, are these even on this specific board?
	ROM_LOAD16_BYTE( "gm.6",     0x00001, 0x08000, BAD_DUMP CRC(56020b13) SHA1(17e176a9c82ed0d6cb5c4014034ce4e16b8ef4fb) )
	ROM_LOAD16_BYTE( "gm.5",     0x00000, 0x08000, BAD_DUMP CRC(667f33f1) SHA1(6d05603b49927f09c9bb34e787b003eceaaf7062) )
	// TODO: recover this!
	//  ROM_LOAD( "33.bin",          0x00000, 0x10000, CRC(05600b13) )
ROM_END

ROM_START( skyadvnt )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sa1.bin",   0x00000,  0x20000, CRC(c2b23080) SHA1(d72430ae43137e3ecbaa327f37e4f3f028690a83) )
	ROM_LOAD16_BYTE( "sa2.bin",   0x00001,  0x20000, CRC(06074e72) SHA1(363b468fad5de0637baf8bb7b92798bfb81a07c5) )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "sa.3",           0x00000,  0x10000, CRC(3d0b32e0) SHA1(b845dc7b887f16dd010b9d43860b6af334995199) )
	ROM_LOAD( "sa.4",           0x20000,  0x10000, CRC(c2e3c30c) SHA1(43e1b489d879950bce3568433a4781377c3eebe7) )
	ROM_LOAD( "sa.5",           0x40000,  0x10000, CRC(11cdb868) SHA1(6cd9c7952b4789e819272cbe0623f3e6f607b7eb) )
	ROM_LOAD( "sa.6",           0x60000,  0x08000, CRC(237d93fd) SHA1(4c65169e4ce6a9be229410bbfd9b823060a829d7) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )  /* chars */
	ROM_LOAD( "sa.7",           0x000000, 0x08000, CRC(ea26e9c5) SHA1(13cb5a5955c813cd48f98f62f045a4cbc61806a1) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_ERASEFF )  /* sprites */
	ROM_LOAD32_BYTE( "sachr3",         0x000000, 0x80000, CRC(a986b8d5) SHA1(e8e2f3e0f85b9565243eab7dc8606168811f41e4) )
	ROM_LOAD32_BYTE( "sachr2",         0x000001, 0x80000, CRC(504b07ae) SHA1(ba74f74c1cb04dd1ab4acf518099605ec9c71f94) )
	ROM_LOAD32_BYTE( "sachr1",         0x000002, 0x80000, CRC(e734dccd) SHA1(24258dd5994f1b14600fc354b0ab36f870967afc) )
	ROM_LOAD32_BYTE( "sachr0",         0x000003, 0x80000, CRC(e281b204) SHA1(50a041c701970013b84826d67c8002ccd291bfdd) )
ROM_END

ROM_START( skyadvntu )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sa_v3.1",   0x00000,  0x20000, CRC(862393b5) SHA1(6c9176a6ae286854f2fa7512c293984a3b952f10) )
	ROM_LOAD16_BYTE( "sa_v3.2",   0x00001,  0x20000, CRC(fa7a14d1) SHA1(d941042cff726f02e1e645a158b6a2484869464b) )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "sa.3",           0x00000,  0x10000, CRC(3d0b32e0) SHA1(b845dc7b887f16dd010b9d43860b6af334995199) )
	ROM_LOAD( "sa.4",           0x20000,  0x10000, CRC(c2e3c30c) SHA1(43e1b489d879950bce3568433a4781377c3eebe7) )
	ROM_LOAD( "sa.5",           0x40000,  0x10000, CRC(11cdb868) SHA1(6cd9c7952b4789e819272cbe0623f3e6f607b7eb) )
	ROM_LOAD( "sa.6",           0x60000,  0x08000, CRC(237d93fd) SHA1(4c65169e4ce6a9be229410bbfd9b823060a829d7) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )  /* chars */
	ROM_LOAD( "sa.7",           0x000000, 0x08000, CRC(ea26e9c5) SHA1(13cb5a5955c813cd48f98f62f045a4cbc61806a1) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_ERASEFF )  /* sprites */
	ROM_LOAD32_BYTE( "sachr3",         0x000000, 0x80000, CRC(a986b8d5) SHA1(e8e2f3e0f85b9565243eab7dc8606168811f41e4) )
	ROM_LOAD32_BYTE( "sachr2",         0x000001, 0x80000, CRC(504b07ae) SHA1(ba74f74c1cb04dd1ab4acf518099605ec9c71f94) )
	ROM_LOAD32_BYTE( "sachr1",         0x000002, 0x80000, CRC(e734dccd) SHA1(24258dd5994f1b14600fc354b0ab36f870967afc) )
	ROM_LOAD32_BYTE( "sachr0",         0x000003, 0x80000, CRC(e281b204) SHA1(50a041c701970013b84826d67c8002ccd291bfdd) )
ROM_END

ROM_START( skyadvntj )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "saj1.c19",  0x00000,  0x20000, CRC(662cb4b8) SHA1(853ad557ee7942cef542253f0e643955e27f0ed2) )
	ROM_LOAD16_BYTE( "saj2.e19",  0x00001,  0x20000, CRC(06d6130a) SHA1(3411ac90e3039e46887451fc97ec2a22ad0f18fe) )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "sa.3",           0x00000,  0x10000, CRC(3d0b32e0) SHA1(b845dc7b887f16dd010b9d43860b6af334995199) )
	ROM_LOAD( "sa.4",           0x20000,  0x10000, CRC(c2e3c30c) SHA1(43e1b489d879950bce3568433a4781377c3eebe7) )
	ROM_LOAD( "sa.5",           0x40000,  0x10000, CRC(11cdb868) SHA1(6cd9c7952b4789e819272cbe0623f3e6f607b7eb) )
	ROM_LOAD( "sa.6",           0x60000,  0x08000, CRC(237d93fd) SHA1(4c65169e4ce6a9be229410bbfd9b823060a829d7) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )  /* chars */
	ROM_LOAD( "sa.7",           0x000000, 0x08000, CRC(ea26e9c5) SHA1(13cb5a5955c813cd48f98f62f045a4cbc61806a1) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_ERASEFF )  /* sprites */
	ROM_LOAD32_BYTE( "sachr3",         0x000000, 0x80000, CRC(a986b8d5) SHA1(e8e2f3e0f85b9565243eab7dc8606168811f41e4) )
	ROM_LOAD32_BYTE( "sachr2",         0x000001, 0x80000, CRC(504b07ae) SHA1(ba74f74c1cb04dd1ab4acf518099605ec9c71f94) )
	ROM_LOAD32_BYTE( "sachr1",         0x000002, 0x80000, CRC(e734dccd) SHA1(24258dd5994f1b14600fc354b0ab36f870967afc) )
	ROM_LOAD32_BYTE( "sachr0",         0x000003, 0x80000, CRC(e281b204) SHA1(50a041c701970013b84826d67c8002ccd291bfdd) )
ROM_END


ROM_START( gangwars )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gw-ver1-e1.19c", 0x00000, 0x20000, CRC(7752478e) SHA1(7266dd0d2c57433191ae4d1d4e17b32c8c3c8c73) )
	ROM_LOAD16_BYTE( "gw-ver1-e2.19d", 0x00001, 0x20000, CRC(c2f3b85e) SHA1(79c215d8b43ec7728e3745b359e64f6bb8240881) )

	/* Extra code bank */
	ROM_LOAD16_BYTE( "gw-ver1-e3.18c", 0x40000, 0x20000, CRC(2a5fe86e) SHA1(0e668f51430983a17e1965143a0bf3aa4d3156ee) )
	ROM_LOAD16_BYTE( "gw-ver1-e4.18d", 0x40001, 0x20000, CRC(c8b60c53) SHA1(352c3bcc18cf63bcb757d774c2c2247ce0c4e736) )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "gw-12.10f",      0x00000, 0x10000, CRC(e6d6c9cf) SHA1(c35a7a385592e55bdfe232d042f2228f4f7e9ffa) )
	ROM_LOAD( "gw-11.11f",      0x20000, 0x10000, CRC(7b9f2608) SHA1(8d61dfa32369450e396cc8a5d67c58eedb2167e6) )
	ROM_LOAD( "gw-10.13f",      0x40000, 0x10000, CRC(eb305d42) SHA1(93910cf60c1b8a87969888d8693c7d6782f1e799) )
	ROM_LOAD( "gw-9.15f",       0x60000, 0x10000, CRC(84e5c946) SHA1(0b071d15b664a9c529713b1b896bdb5ebfa16c25) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "gfx1", 0 )  /* chars */
	ROM_LOAD( "gw-13.4l",     0x000000, 0x10000, CRC(b75bf1d0) SHA1(c22c0049274c45701be0a7be2afc0517620a3a10) )

	ROM_REGION( 0x400000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "guernica-c3.17h",     0x000000, 0x80000, CRC(281a4138) SHA1(47fc0d91873996e05db87323c3b08a85863f90d9) )
	ROM_LOAD32_BYTE( "guernica-c2.18h",     0x000001, 0x80000, CRC(2fcbea97) SHA1(eb60bf374ef771e379030d2b660a813be76bed5e) )
	ROM_LOAD32_BYTE( "guernica-c1.20h",     0x000002, 0x80000, CRC(b0fd1c23) SHA1(a6dbed81b751c1f662f63a7426d8333aca866d79) ) // rom with CRC 5d384c3b has a fixed bits problem on bit 0x80 in the first 0x200 bytes
	ROM_LOAD32_BYTE( "guernica-c0.21h",     0x000003, 0x80000, CRC(e60c9882) SHA1(8cf1d9cf0db72977b303fd6b469611600631ab9a) )
	ROM_LOAD32_BYTE( "gw-5.21f",            0x200000, 0x20000, CRC(9ef36031) SHA1(2faeb6a769991ab11403c6c37507b706a61bad69) )
	ROM_LOAD32_BYTE( "gw-6.20f",            0x200001, 0x20000, CRC(ddbbcda7) SHA1(1c368ad2a4ed31748c94545fc7c808aa53d76f64) )
	ROM_LOAD32_BYTE( "gw-7.18f",            0x200002, 0x20000, CRC(4656d377) SHA1(67d6f714cca3891be0173c543ece5e8ab699f645) )
	ROM_LOAD32_BYTE( "gw-8.17f",            0x200003, 0x20000, CRC(798ed82a) SHA1(1932131e05aae0a77ba8d8ef947c1a3b0b5e3d43) )
ROM_END




ROM_START( gangwarsb ) // this is a common bootleg, main code etc. matches the original, has a 68705 MCU (undumped) duplicating the functionality of the Alpha one, GFX roms are split (content identical)
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gwb_ic.m15", 0x00000, 0x20000, CRC(7752478e) SHA1(7266dd0d2c57433191ae4d1d4e17b32c8c3c8c73) )
	ROM_LOAD16_BYTE( "gwb_ic.m16", 0x00001, 0x20000, CRC(c2f3b85e) SHA1(79c215d8b43ec7728e3745b359e64f6bb8240881) )

	/* Extra code bank */
	ROM_LOAD16_BYTE( "gwb_ic.m17", 0x40000, 0x20000, CRC(2a5fe86e) SHA1(0e668f51430983a17e1965143a0bf3aa4d3156ee) )
	ROM_LOAD16_BYTE( "gwb_ic.m18", 0x40001, 0x20000, CRC(c8b60c53) SHA1(352c3bcc18cf63bcb757d774c2c2247ce0c4e736) )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "gwb_ic.380",     0x00000, 0x10000, CRC(e6d6c9cf) SHA1(c35a7a385592e55bdfe232d042f2228f4f7e9ffa) )
	ROM_LOAD( "gwb_ic.421",     0x20000, 0x10000, CRC(7b9f2608) SHA1(8d61dfa32369450e396cc8a5d67c58eedb2167e6) )
	ROM_LOAD( "gwb_ic.420",     0x40000, 0x10000, CRC(eb305d42) SHA1(93910cf60c1b8a87969888d8693c7d6782f1e799) )
	ROM_LOAD( "gwb_ic.419",     0x60000, 0x10000, CRC(84e5c946) SHA1(0b071d15b664a9c529713b1b896bdb5ebfa16c25) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "68705.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "gfx1", 0 )  /* chars */
	ROM_LOAD( "gwb_ic.m19",     0x000000, 0x10000, CRC(b75bf1d0) SHA1(c22c0049274c45701be0a7be2afc0517620a3a10) )

	ROM_REGION( 0x400000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "gwb_ic.308",     0x000000, 0x10000, CRC(321a2fdd) SHA1(b2f37f14a13bc2c2f78b2b0e27fde18a23146e22) )
	ROM_LOAD32_BYTE( "gwb_ic.309",     0x040000, 0x10000, CRC(4d908f65) SHA1(6095a34ef4a6905d57c47af4a507dff3a04e5c07) )
	ROM_LOAD32_BYTE( "gwb_ic.310",     0x080000, 0x10000, CRC(fc888541) SHA1(e732a03209a88fc7a23b4e4ff69a437d6fbfc2d1) )
	ROM_LOAD32_BYTE( "gwb_ic.311",     0x0c0000, 0x10000, CRC(181b128b) SHA1(2646c9f9cca6277ddd764c07478798c8af3eb297) )
	ROM_LOAD32_BYTE( "gwb_ic.312",     0x100000, 0x10000, CRC(930665f3) SHA1(03af85c45acb9600b27dcdd6ec96d147046030e5) )
	ROM_LOAD32_BYTE( "gwb_ic.313",     0x140000, 0x10000, CRC(c18f4ca8) SHA1(f5cb666d5aa53f201b6664d1c18b89a211230e78) )
	ROM_LOAD32_BYTE( "gwb_ic.314",     0x180000, 0x10000, CRC(dfc44b60) SHA1(311422d4ea77118c0058e9f1a824f74cfa79cb87) )
	ROM_LOAD32_BYTE( "gwb_ic.307",     0x1c0000, 0x10000, CRC(28082a7f) SHA1(e30bade13e03bca49c1f7001c9440ce251ece15d) )
	ROM_LOAD32_BYTE( "gwb_ic.280",     0x200000, 0x10000, CRC(222b3dcd) SHA1(f9afe24c01daefe61939672efa2cb68bcc7235f0) )
	ROM_LOAD32_BYTE( "gwb_ic.321",     0x240000, 0x10000, CRC(6b421c7b) SHA1(d96f91dc7e5f46990b05701483edf43a828a8879) )

	ROM_LOAD32_BYTE( "gwb_ic.300",     0x000001, 0x10000, CRC(f3fa0877) SHA1(7950ef86ee66d19693f0b7071a3a34d9200f5a19) )
	ROM_LOAD32_BYTE( "gwb_ic.301",     0x040001, 0x10000, CRC(f8c866de) SHA1(c6baa41bab35d4d9e80c5c52db74e9eb6b9605f5) )
	ROM_LOAD32_BYTE( "gwb_ic.302",     0x080001, 0x10000, CRC(5b0d587d) SHA1(852bec7d37d8cee33e5bc30080bf8a6a8d2472e5) )
	ROM_LOAD32_BYTE( "gwb_ic.303",     0x0c0001, 0x10000, CRC(d8c0e102) SHA1(f660876ab3457230b1c37835f5ad1ccd1e8e821a) )
	ROM_LOAD32_BYTE( "gwb_ic.304",     0x100001, 0x10000, CRC(b02bc9d8) SHA1(e0466b93c08363cceaba27c8d85e2609d12a10e7) )
	ROM_LOAD32_BYTE( "gwb_ic.305",     0x140001, 0x10000, CRC(5e04a9aa) SHA1(663330b467eb6406719d4d6cf7b05835b1600a37) )
	ROM_LOAD32_BYTE( "gwb_ic.306",     0x180001, 0x10000, CRC(e2172955) SHA1(af13776e6537e736815a1180a1f6bad385724b0c) )
	ROM_LOAD32_BYTE( "gwb_ic.299",     0x1c0001, 0x10000, CRC(e39f5599) SHA1(3c08a8163b528ebbcb627c511ccc2edacf0653c2) )
	ROM_LOAD32_BYTE( "gwb_ic.320",     0x200001, 0x10000, CRC(9a7b51d8) SHA1(0ab01972d838c938bfd07d7b4661a0ecd009b2cb) )
	ROM_LOAD32_BYTE( "gwb_ic.319",     0x240001, 0x10000, CRC(c5b862b7) SHA1(a48be3e32ae5a656d8d239796e6e7bddd4a0805b) )

	ROM_LOAD32_BYTE( "gwb_ic.292",     0x000002, 0x10000, CRC(c125f7be) SHA1(5d68abd91fa4fa18275c0597c51ce6d3e743d84d) )
	ROM_LOAD32_BYTE( "gwb_ic.293",     0x040002, 0x10000, CRC(c04fce8e) SHA1(499edd3b16770d20368f49e5c66c299740831ff0) )
	ROM_LOAD32_BYTE( "gwb_ic.294",     0x080002, 0x10000, CRC(4eda3df5) SHA1(574fef723ebd8fa116b4a379036ee5ec3eb10c90) )
	ROM_LOAD32_BYTE( "gwb_ic.295",     0x0c0002, 0x10000, CRC(6e60c475) SHA1(928494400bbdc3571cb5b1ccb51d39537c5fd904) )
	ROM_LOAD32_BYTE( "gwb_ic.296",     0x100002, 0x10000, CRC(99b2a557) SHA1(7a4053909cb5f4b2a32f3caac4e2ccdb64c2ce84) )
	ROM_LOAD32_BYTE( "gwb_ic.297",     0x140002, 0x10000, CRC(10373f63) SHA1(98ee65c68823530ad2eefd6e570db2f38b59c48e) )
	ROM_LOAD32_BYTE( "gwb_ic.298",     0x180002, 0x10000, CRC(df37ec4d) SHA1(d2670dde87970a6f33ca3cd81bdc9991d663bac6) )
	ROM_LOAD32_BYTE( "gwb_ic.291",     0x1c0002, 0x10000, CRC(beb07a2e) SHA1(f2751bef1850db7173f119fc0cfeefdf47ed7a86) )
	ROM_LOAD32_BYTE( "gwb_ic.318",     0x200002, 0x10000, CRC(9aeaddf9) SHA1(d609314015376672be8147b9eabbfe4c5611ab73) )
	ROM_LOAD32_BYTE( "gwb_ic.317",     0x240002, 0x10000, CRC(1622fadd) SHA1(240eaf117145773e388220513c2906ad2ac5d68b) )

	ROM_LOAD32_BYTE( "gwb_ic.284",     0x000003, 0x10000, CRC(4aa95d66) SHA1(e5bb51fd32a7e9dc23aa13de35b8757dc11f7908) )
	ROM_LOAD32_BYTE( "gwb_ic.285",     0x040003, 0x10000, CRC(3a1f3ce0) SHA1(edd8820111a3ef9558286280dc819c6d9f21212f) )
	ROM_LOAD32_BYTE( "gwb_ic.286",     0x080003, 0x10000, CRC(886e298b) SHA1(8e8b35a0b24c9c3a1d00079b60bdbaa6c8ce597b) )
	ROM_LOAD32_BYTE( "gwb_ic.287",     0x0c0003, 0x10000, CRC(b9542e6a) SHA1(e5762db1a44a966d2c2b7e8a92abab577e24172f) )
	ROM_LOAD32_BYTE( "gwb_ic.288",     0x100003, 0x10000, CRC(8e620056) SHA1(59ea29d681c4b001b656b4b8014b14ac78a69625) )
	ROM_LOAD32_BYTE( "gwb_ic.289",     0x140003, 0x10000, CRC(c754d69f) SHA1(e64b8e1f719f5a95b7bcab2d25a40c8b819f7d4f) )
	ROM_LOAD32_BYTE( "gwb_ic.290",     0x180003, 0x10000, CRC(306d1963) SHA1(2f19ba97b9bd1744b656095ae0244df2db03b09b) )
	ROM_LOAD32_BYTE( "gwb_ic.283",     0x1c0003, 0x10000, CRC(b46e5761) SHA1(3c4c13c5896186fe36ace8704afeef84b0a0cb78) )
	ROM_LOAD32_BYTE( "gwb_ic.316",     0x200003, 0x10000, CRC(655b1518) SHA1(d97fd911901f92786bc22dac8e085cf4fa0cb1e9) )
	ROM_LOAD32_BYTE( "gwb_ic.315",     0x240003, 0x10000, CRC(e7c9b103) SHA1(6f70ca9b6a7439f9250145477f682f7487e11710) )
ROM_END


ROM_START( gangwarsj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gw-ver1-j1.19c", 0x00000, 0x20000, CRC(98bf9668) SHA1(5572b866c591b117e606b30554f12e15dd8a1c76) )
	ROM_LOAD16_BYTE( "gw-ver1-j2.19d", 0x00001, 0x20000, CRC(41868606) SHA1(a7239a0e740ce3b9d091eb864f699d72d8031618) )

	/* Extra code bank */
	ROM_LOAD16_BYTE( "gw-ver1-j3.18c", 0x40000, 0x20000, CRC(6e6b7e1f) SHA1(300c9c87ec471c8f9c6f19ad43e1ce0bef63b67b) )
	ROM_LOAD16_BYTE( "gw-ver1-j4.18d", 0x40001, 0x20000, CRC(1f13eb20) SHA1(f8c76ac9271ce158d9cb79d7b3be92750806e7d6) )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "gw-12.10f",      0x00000, 0x10000, CRC(e6d6c9cf) SHA1(c35a7a385592e55bdfe232d042f2228f4f7e9ffa) )
	ROM_LOAD( "gw-11.11f",      0x20000, 0x10000, CRC(7b9f2608) SHA1(8d61dfa32369450e396cc8a5d67c58eedb2167e6) )
	ROM_LOAD( "gw-10.13f",      0x40000, 0x10000, CRC(eb305d42) SHA1(93910cf60c1b8a87969888d8693c7d6782f1e799) )
	ROM_LOAD( "gw-9.15f",       0x60000, 0x10000, CRC(84e5c946) SHA1(0b071d15b664a9c529713b1b896bdb5ebfa16c25) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "gfx1", 0 )  /* chars */
	ROM_LOAD( "gw-13.4l",     0x000000, 0x10000, CRC(b75bf1d0) SHA1(c22c0049274c45701be0a7be2afc0517620a3a10) )

	ROM_REGION( 0x400000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "guernica-c3.17h",     0x000000, 0x80000, CRC(281a4138) SHA1(47fc0d91873996e05db87323c3b08a85863f90d9) )
	ROM_LOAD32_BYTE( "guernica-c2.18h",     0x000001, 0x80000, CRC(2fcbea97) SHA1(eb60bf374ef771e379030d2b660a813be76bed5e) )
	ROM_LOAD32_BYTE( "guernica-c1.20h",     0x000002, 0x80000, CRC(b0fd1c23) SHA1(a6dbed81b751c1f662f63a7426d8333aca866d79) )
	ROM_LOAD32_BYTE( "guernica-c0.21h",     0x000003, 0x80000, CRC(e60c9882) SHA1(8cf1d9cf0db72977b303fd6b469611600631ab9a) )
	ROM_LOAD32_BYTE( "gw-5.21f",            0x200000, 0x20000, CRC(9ef36031) SHA1(2faeb6a769991ab11403c6c37507b706a61bad69) )
	ROM_LOAD32_BYTE( "gw-6.20f",            0x200001, 0x20000, CRC(ddbbcda7) SHA1(1c368ad2a4ed31748c94545fc7c808aa53d76f64) )
	ROM_LOAD32_BYTE( "gw-7.18f",            0x200002, 0x20000, CRC(4656d377) SHA1(67d6f714cca3891be0173c543ece5e8ab699f645) )
	ROM_LOAD32_BYTE( "gw-8.17f",            0x200003, 0x20000, CRC(798ed82a) SHA1(1932131e05aae0a77ba8d8ef947c1a3b0b5e3d43) )
ROM_END


ROM_START( gangwarsu )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "u1",        0x00000, 0x20000, CRC(11433507) SHA1(df32c14d4105d3ad899dfa8e9dbc2a1fe51dfa6a) )
	ROM_LOAD16_BYTE( "u2",        0x00001, 0x20000, CRC(44cc375f) SHA1(38fc402014a816d9b1f7680407175adecfa39efe) )

	/* Extra code bank */
	ROM_LOAD16_BYTE( "u3",        0x40000,  0x20000, CRC(de6fd3c0) SHA1(d957e8de3cb0eda1837376f687b8c272e97e1d11) )
	ROM_LOAD16_BYTE( "u4",        0x40001,  0x20000, CRC(43f7f5d3) SHA1(13ea03cfae97d0067dcfdc6febb53dbe268a91eb) )

	ROM_REGION( 0x80000, "audiocpu", 0 )   /* Sound CPU */
	ROM_LOAD( "u12",            0x00000, 0x10000, CRC(2620caa1) SHA1(bd464abce0bedab68cb913321e76d83eb36ca374) )
	ROM_LOAD( "u11",            0x20000, 0x10000, CRC(2218ceb9) SHA1(69a843308cb0628ad856a09a33cd148f36ce0d24) )
	ROM_LOAD( "u10",            0x40000, 0x10000, CRC(636978ae) SHA1(5d8093bc43192c89e230af318609222a69866b6e) )
	ROM_LOAD( "u9",             0x60000, 0x10000, CRC(9136745e) SHA1(d7a2bfeac69ab2dbd4565a5bd1abb1f3f1199b42) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "gfx1", 0 )  /* chars */
	ROM_LOAD( "gw-13.4l",     0x000000, 0x10000, CRC(b75bf1d0) SHA1(c22c0049274c45701be0a7be2afc0517620a3a10) )

	ROM_REGION( 0x400000, "gfx2", 0 )  /* sprites */
	ROM_LOAD32_BYTE( "guernica-c3.17h",     0x000000, 0x80000, CRC(281a4138) SHA1(47fc0d91873996e05db87323c3b08a85863f90d9) )
	ROM_LOAD32_BYTE( "guernica-c2.18h",     0x000001, 0x80000, CRC(2fcbea97) SHA1(eb60bf374ef771e379030d2b660a813be76bed5e) )
	ROM_LOAD32_BYTE( "guernica-c1.20h",     0x000002, 0x80000, CRC(b0fd1c23) SHA1(a6dbed81b751c1f662f63a7426d8333aca866d79) )
	ROM_LOAD32_BYTE( "guernica-c0.21h",     0x000003, 0x80000, CRC(e60c9882) SHA1(8cf1d9cf0db72977b303fd6b469611600631ab9a) )
	ROM_LOAD32_BYTE( "u5",                  0x200000, 0x20000, CRC(94612190) SHA1(dd7818744b1b6738d268044f13e0647e7a3b2d1e) ) // this set of u5,u6,u7,u8 have a one-way sign, used on stage 3
	ROM_LOAD32_BYTE( "u6",                  0x200001, 0x20000, CRC(5a4ea0f0) SHA1(7ea8b3f66f32ab9b33a522edca6d5c6416fd7a9b) )
	ROM_LOAD32_BYTE( "u7",                  0x200002, 0x20000, CRC(33f324cb) SHA1(c00f55ce85749cbbb9569a22cc6c9c886ed9ab78) )
	ROM_LOAD32_BYTE( "u8",                  0x200003, 0x20000, CRC(c1995c2c) SHA1(909e1070b4ec28a1f4a2cd9fbc3bde781ffbdda8) )
ROM_END

ROM_START( sbasebal )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "snksb1.c19", 0x00000, 0x20000, CRC(304fef2d) SHA1(03154e590807f7fd009068b403e1ea039272029d) )
	ROM_LOAD16_BYTE( "snksb2.e19", 0x00001, 0x20000, CRC(35821339) SHA1(2c4303bf799de7cb364cadac44ff28306088e2f4) )

	ROM_REGION( 0x80000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "sb-3.g9",  0x00000, 0x10000, CRC(89e12f25) SHA1(1c569958a7f5a91b54f1316c1d5ee027be8618d6) )
	ROM_LOAD( "sb-4.g11", 0x20000, 0x10000, CRC(cca2555d) SHA1(13c672331e8e5e5dd8fc3aa7829d46de6b8271f3) )
	ROM_LOAD( "sb-5.g13", 0x40000, 0x10000, CRC(f45ee36f) SHA1(cdfdf696e9fcd2827ab1dd6adc2a45085911333d) )
	ROM_LOAD( "sb-6.g15", 0x60000, 0x10000, CRC(651c9472) SHA1(bcff6679e22143cd6816c441c5a67b4956ee7ee0) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "sb-7.l3", 0x000000, 0x10000, CRC(8f3c2e25) SHA1(a5b3880f3079cce607678fd4ea5971560ce9ed8d) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_ERASEFF ) /* sprites */
	ROM_LOAD32_BYTE( "kcb-chr3.h21", 0x000000, 0x80000, CRC(719071c7) SHA1(47eded73eae25af04cf369f1a8ee657fd06b6480) )
	ROM_LOAD32_BYTE( "kcb-chr2.h19", 0x000001, 0x80000, CRC(014f0f90) SHA1(e80594f06faf303c4034a711fe55dad046ebf9aa) )
	ROM_LOAD32_BYTE( "kcb-chr1.h18", 0x000002, 0x80000, CRC(a5ce1e10) SHA1(c91cad45a918166155be3f93f4ed299389579f4a) )
	ROM_LOAD32_BYTE( "kcb-chr0.h16", 0x000003, 0x80000, CRC(b8a1a088) SHA1(cb21a04387431b1810130abd86a2ebf78cf09a3b) )
ROM_END

ROM_START( sbasebalj )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sb-j-1.c19", 0x00000, 0x20000, CRC(c46a3c03) SHA1(51d22f2873e45ae64453f3003940b3871d065c5b) )
	ROM_LOAD16_BYTE( "sb-j-2.e19", 0x00001, 0x20000, CRC(a8ec2287) SHA1(9c873f3388a00babc1cd38188ef1fe6e2741fd67) )

	ROM_REGION( 0x80000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "sb-3.g9",  0x00000, 0x10000, CRC(89e12f25) SHA1(1c569958a7f5a91b54f1316c1d5ee027be8618d6) )
	ROM_LOAD( "sb-4.g11", 0x20000, 0x10000, CRC(cca2555d) SHA1(13c672331e8e5e5dd8fc3aa7829d46de6b8271f3) )
	ROM_LOAD( "sb-5.g13", 0x40000, 0x10000, CRC(f45ee36f) SHA1(cdfdf696e9fcd2827ab1dd6adc2a45085911333d) )
	ROM_LOAD( "sb-6.g15", 0x60000, 0x10000, CRC(651c9472) SHA1(bcff6679e22143cd6816c441c5a67b4956ee7ee0) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "alpha.mcu", 0x000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "sb-7.l3", 0x000000, 0x10000, CRC(8f3c2e25) SHA1(a5b3880f3079cce607678fd4ea5971560ce9ed8d) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_ERASEFF ) /* sprites */
	ROM_LOAD32_BYTE( "kcb-chr3.h21", 0x000000, 0x80000, CRC(719071c7) SHA1(47eded73eae25af04cf369f1a8ee657fd06b6480) )
	ROM_LOAD32_BYTE( "kcb-chr2.h19", 0x000001, 0x80000, CRC(014f0f90) SHA1(e80594f06faf303c4034a711fe55dad046ebf9aa) )
	ROM_LOAD32_BYTE( "kcb-chr1.h18", 0x000002, 0x80000, CRC(a5ce1e10) SHA1(c91cad45a918166155be3f93f4ed299389579f4a) )
	ROM_LOAD32_BYTE( "kcb-chr0.h16", 0x000003, 0x80000, CRC(b8a1a088) SHA1(cb21a04387431b1810130abd86a2ebf78cf09a3b) )
ROM_END

/******************************************************************************/

void alpha68k_II_state::init_timesold()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0;
	m_coin_id = 0x22 | (0x22 << 8);
	m_game_id = 0;
}

void alpha68k_II_state::init_timesold1()
{
	m_invert_controls = 1;
	m_microcontroller_id = 0;
	m_coin_id = 0x22 | (0x22 << 8);
	m_game_id = 0;
}

void alpha68k_II_state::init_btlfield()
{
	m_invert_controls = 1;
	m_microcontroller_id = 0;
	m_coin_id = 0x22 | (0x22 << 8);
	m_game_id = 0;
}

void alpha68k_II_state::init_btlfieldb()
{
	m_invert_controls = 1;
	m_microcontroller_id = 0;
	m_coin_id = 0x22 | (0x22 << 8); //not checked
	m_game_id = ALPHA68K_BTLFIELDB;
}

void alpha68k_II_state::init_skysoldr()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0;
	m_coin_id = 0x22 | (0x22 << 8);
	m_game_id = 0;
}

void goldmedal_II_state::init_goldmedl()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0x8803;
	m_coin_id = 0x23 | (0x24 << 8);
	m_game_id = 0;
}

void goldmedal_III_state::init_goldmedla()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0x8803; //Guess - routine to handle coinage is the same as in 'goldmedl'
	m_coin_id = 0x23 | (0x24 << 8);
	m_game_id = 0;
}

void alpha68k_V_state::init_skyadvnt()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0x8814;
	m_coin_id = 0x22 | (0x22 << 8);
	m_game_id = 0;
}

void alpha68k_V_state::init_skyadvntu()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0x8814;
	m_coin_id = 0x23 | (0x24 << 8);
	m_game_id = 0;
}

void alpha68k_V_state::init_gangwarsu()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0x8512;
	m_coin_id = 0x23 | (0x24 << 8);
	m_game_id = 0;
}

void alpha68k_V_state::init_gangwars()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0x8512;
	m_coin_id = 0x23 | (0x24 << 8);
	m_game_id = 0;
}

void alpha68k_V_state::init_sbasebal()
{
	u16 *rom = (u16 *)memregion("maincpu")->base();

	/* Patch protection check, it does a divide by zero because the MCU is trying to
	   calculate the ball speed when a strike is scored, notice that current emulation
	   just returns 49 mi/h every time that this event happens.
	   68k reads at [0x4023e], then subtracts this value with [0x41838], presumably it's raw speed minus angle.
	   main CPU then writes the result to RAM location [0x41866], probably just to signal the result to the MCU.
	   Update: Game also have max pitch speeds, how this is taken into account?
	   */
	rom[0xb672/2] = 0x4e71;

	/* And patch the ROM checksums */
	rom[0x44e/2] = 0x4e71;
	rom[0x450/2] = 0x4e71;
	rom[0x458/2] = 0x4e71;
	rom[0x45a/2] = 0x4e71;

	m_invert_controls = 0;
	m_microcontroller_id = 0x8512;  // Same as 'gangwars' ?
	m_coin_id = 0x23 | (0x24 << 8);
	m_game_id = 0;
}

void alpha68k_V_state::init_sbasebalj()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0x8512;  // Same as 'gangwars' ?
	m_coin_id = 0x23 | (0x24 << 8);
	m_game_id = 0;
}

/******************************************************************************/

// Alpha II HW
GAME( 1987, timesold,  0,        alpha68k_II,    timesold,  alpha68k_II_state, init_timesold,  ROT90, "Alpha Denshi Co. (SNK/Romstar license)",            "Time Soldiers (US Rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, timesold1, timesold, alpha68k_II,    timesold,  alpha68k_II_state, init_timesold1, ROT90, "Alpha Denshi Co. (SNK/Romstar license)",            "Time Soldiers (US Rev 1)", MACHINE_SUPPORTS_SAVE )

GAME( 1987, btlfield,  timesold, alpha68k_II,    btlfield,  alpha68k_II_state, init_btlfield,  ROT90, "Alpha Denshi Co. (SNK license)",                    "Battle Field (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, btlfieldb, timesold, btlfieldb,      btlfieldb, alpha68k_II_state, init_btlfieldb, ROT90, "bootleg",                                           "Battle Field (bootleg)", MACHINE_SUPPORTS_SAVE )

GAME( 1988, skysoldr,  0,        alpha68k_II,    skysoldr,  alpha68k_II_state, init_skysoldr,  ROT90, "Alpha Denshi Co. (SNK of America/Romstar license)", "Sky Soldiers (US)", MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION ) // unemulated boss time out behaviour
GAME( 1988, skysoldrbl,skysoldr, alpha68k_II,    skysoldr,  alpha68k_II_state, init_skysoldr,  ROT90, "bootleg",                                           "Sky Soldiers (bootleg)", MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION ) // unknown if/how bootleggers handled boss time out, seems likely

GAME( 1988, goldmedl,  0,        goldmedal,   goldmedl,  goldmedal_II_state, init_goldmedl,  ROT0,  "SNK",                                               "Gold Medalist (set 1, Alpha68k II PCB)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING )

// Alpha III HW
GAME( 1988, goldmedla, goldmedl, goldmedal,   goldmedl,  goldmedal_III_state, init_goldmedla, ROT0,  "SNK",                                               "Gold Medalist (set 2, Alpha68k III PCB)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING )
GAME( 1988, goldmedlb, goldmedl, goldmedal,   goldmedl,  goldmedal_III_state, init_goldmedla, ROT0,  "bootleg",                                               "Gold Medalist (bootleg, Alpha68k III PCB)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING )

// Alpha V HW
GAME( 1989, skyadvnt,  0,        skyadventure,     skyadvnt,  skyadventure_state, init_skyadvnt,  ROT90, "Alpha Denshi Co.",                                  "Sky Adventure (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, skyadvntu, skyadvnt, skyadventure,     skyadvntu, skyadventure_state, init_skyadvntu, ROT90, "Alpha Denshi Co. (SNK of America license)",         "Sky Adventure (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, skyadvntj, skyadvnt, skyadventure,     skyadvnt,  skyadventure_state, init_skyadvnt,  ROT90, "Alpha Denshi Co.",                                  "Sky Adventure (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, gangwars,  0,        gangwars,     gangwars,  gangwars_state, init_gangwars,  ROT0,  "Alpha Denshi Co.",                                  "Gang Wars", MACHINE_SUPPORTS_SAVE )
GAME( 1989, gangwarsj, gangwars, gangwars,     gangwars,  gangwars_state, init_gangwars,  ROT0,  "Alpha Denshi Co.",                                  "Gang Wars (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, gangwarsu, gangwars, gangwars,     gangwarsu, gangwars_state, init_gangwarsu, ROT0,  "Alpha Denshi Co.",                                  "Gang Wars (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, gangwarsb, gangwars, gangwars,     gangwars,  gangwars_state, init_gangwars,  ROT0,  "bootleg",                                           "Gang Wars (bootleg)", MACHINE_SUPPORTS_SAVE ) // has (undumped) 68705 MCU in place of Alpha MCU, otherwise the same as 'gangwars'

GAME( 1989, sbasebal,  0,        alpha68k_V,   sbasebal,  alpha68k_V_state, init_sbasebal,  ROT0,  "Alpha Denshi Co. (SNK of America license)",         "Super Champion Baseball (US)", MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION ) // calculated pitcher launching speed
GAME( 1989, sbasebalj, sbasebal, alpha68k_V,   sbasebalj, alpha68k_V_state, init_sbasebalj, ROT0,  "Alpha Denshi Co.",                                  "Super Champion Baseball (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION ) // same as above
