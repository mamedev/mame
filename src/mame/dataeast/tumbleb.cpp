// license:BSD-3-Clause
// copyright-holders:David Haywood,Bryan McPhail
/***************************************************************************

  the following run on hardware which is physically not the same as
  Tumble Pop but works in a very similar way.

  Tumblepop             (c) 1991 Data East Corporation (Bootleg 1)
  Tumblepop             (c) 1991 Data East Corporation (Bootleg 2)
  Jump Kids             (c) 1993 Comad
  Metal Saver           (c) 1994 First Amusement
  Magicball Fighting    (c) 1994 SemiCom
  Pang Pang             (c) 1994 Dong Gue La Mi Ltd.
  Super Trio[1]         (c) 1994 GameAce
  Fancy World           (c) 1995 Unico
  Hatch Catch           (c) 1995 SemiCom
  Cookie & Bibi[2]      (c) 1995 SemiCom
  Choky! Choky!         (c) 1995 SemiCom
  WonderLeague Star     (c) 1995 Mijin   (Korea Only) - SemiCom used the name Mijin up to 1995
  WonderLeague '96      (c) 1996 SemiCom (Korea Only)
  SD Fighters           (c) 1996 SemiCom (Korea Only)
  Carket Ball           (c) 1996
  Magic Purple          (c) 1996 Unico
  B.C. Story            (c) 1997 SemiCom
  MuHanSeungBu          (c) 1997 SemiCom (Korea Only)
  Date Quiz Go Go       (c) 1998 SemiCom (Korea Only)

  [1] has the same sprites as the bootlegs, not much else is the same tho

  [2] test mode crashes the same way on the real board
      cookbiba set doesn't crash, maybe it's newer? check the rest of the code

  Bootleg sound is not quite correct yet (Nothing on bootleg 2).
  ** at least one of the bootlegs uses a protected PIC to drive the OKI **

  If you reset the game while pressing START1 and START2, "VER 0.00 JAPAN"
  is put into tile ram then MAME crashes !

  Sometimes a garbage sprite gets left after the SemiCom logo in Hatch
  Catch - other SemiCom games on different hw do this too, might just
  be a bug in their code

  Emulation by Bryan McPhail, mish@tendril.co.uk


Stephh's notes (based on the games M68000 code and some tests) :

1) 'tumbleb*' and 'jumpkids'

  - I don't understand the interest of the "Remove Monsters" Dip Switch :
    as I haven't found a way to "end" a level, I guess that it was used to
    test the backgrounds and the "platforms".

  - The "Edit Levels" Dip Switch allows you to add/delete monsters and
    change their position.

    Notes (for 'tumbleb*') :
      * "worlds" and levels are 0-based (00-09 & 00-09) :

          World      Name
            0      America
            1      Brazil
            2      Asia
            3      Soviet
            4      Europe
            5      Egypt
            6      Australia
            7      Antartica
            8      Stratosphere
            9      Space

      * As levels x-9 and 9-x are only constituted of a "big boss", you can't
        edit them !
      * All data is stored within the range 0x02b8c8-0x02d2c9, but it should be
        extended to 0x02ebeb (and perhaps 0x02ffff). TO BE CONFIRMED !
      * Once your levels are ready, turn the Dip Switch OFF and reset the game.
      * Of course, there is no possibility to save the levels when you exit
        MAME, nor the way to reload the default ones 8(

    Additional notes (for 'jumpkids') :
      * As there are only 9 "worlds", editing "world" 9 ("Space") might cause
        unpredictable weird results !
      * The "worlds" names are the same, but the background is different :

          World      Name            Background
            0      America         Stadium
            1      Brazil          Beach
            2      Asia            Planet
            3      Soviet          Prehistoric Ages
            4      Europe          Castle
            5      Egypt           Pyramids
            6      Australia       Lunar base
            7      Antartica       Bridge
            8      Stratosphere    ???
            9      Space           DOES NOT EXIST !

        As I'm not sure of the description of the background, feel free to
        improve the previous list.
      * All data is stored within the range 0x02776e-0x029207, but it should be
        extended to 0x02ab29 (and perhaps 0x02ab49). TO BE CONFIRMED !


2) 'metlsavr'

  - It is unknown what's the effect of DSW2-6 and DSW 2-7.
    All that can be told is that they related somewhere to DSW 2-5 ("Language")
    as they affect in game language (but not the one for explanations in "Test Mode").
    In fact, some values are put to some RAM addresses, but I can't find what they do :

      2-7  2-6  2-5  0x122eec  0x12021a  In game language  "Test Mode" explanations
      OFF  OFF  OFF    0x00      0x00        English                English
      ON   OFF  OFF    0x00      0x01        English                English
      OFF  ON   OFF    0x00      0x01        English                English
      ON   ON   OFF    0x01      0x00        Korean                 English
      OFF  OFF  OFF    0x01      0x00        Korean                 Korean
      ON   OFF  OFF    0x01      0x01        Korean                 Korean
      OFF  ON   OFF    0x01      0x01        Korean                 Korean
      ON   ON   OFF    0x00      0x00        English                Korean

    If you have any clue, please let me know ...


3) 'pangpang'

  - The game needs more investigation to get similar infos to "Tumble Pop" bootlegs/ripoffs.


4) 'htchctch'

  - As I'm too bad at playing such game, I haven't been able to determine
    what's the effect of DSW2-2 (which is called "Stage Skip" in "Test Mode"),
    nor when it is supposed to be tested. Leftover from another game ?

  - "Difficulty" and "Coinage" Dip Switches aren't visible in "Test Mode".


5) 'cookbib'

  - Whatever is written in "Test Mode", Dip Switch bank 1 is unused, so DSW 1-1
    (which is called "Stage Skip") might be a leftover from another game.

  - "Difficulty" and "Coinage" Dip Switches aren't visible in "Test Mode".


6) 'wlstar'

  - DSW 1-3 is read once (check code at 0x000eae), but the address
    where the computed value is stored is NEVER read back !
    I guess this might be a leftover from another game.

  - DSW 2-8 determines the last inning if there is a draw after inning 9.
    After inning 9, scores are checked to see if there us still a draw.
    Once last inning is over, the game ends regardless of the score.

  - The "VS CPU Game Ends" Dip Switch is used when you play a game against the CPU.
    If CPU's score is +10 or +7 than player's score, the game ends.

  - The "VS Game" Dip Switch affects single game against CPU or other player
    (choices 2 and 3). Once the number of innings is achieved, the player(s)
    is (are) proposed to continue, provided they get enough credits.

  - The "Full 2 Players Game" determines how many credits are required
    to enlighten the 4th choice (2 players 9 innings match).


7) 'wonld96'

  - I can't determine what's the effect of DSW 1-7 to DSW 1-5 :(
    All I can tell is that the computed value (from 0x0000 to 0x0007) is stored at 0x12279c.w
    and it is compared with the contents of addresses 0x1207c6.w and 0x12153c.w .

  - DSW 1-4 changes the color of the field, but I don't know if it has
    some other effects. Please check this out and let me know.

  - The "VS CPU Game Ends" Dip Switch is used when you play a game against the CPU.
    If CPU's score is +10 or +7 than player's score, the game ends.

  - The "VS Game" Dip Switch affects single game against CPU or other player
    (choices 2 and 3). Once the number of innings is achieved, the player(s)
    is (are) proposed to continue, provided they get enough credits.

  - The "Full 2 Players Game" determines how many credits are required
    to enlighten the 4th choice (2 players 9 innings match).

  - DSW 2-1 should be "Demo Sounds", but there are NEVER demo sounds
    due to code at 0x000fe6 ('beq     $fea'). So I've marked it as "Unused".


8) 'fncywld'

  - I'm not sure about the release date of this game :
      * on the title screen, it ALWAYS displays 1996
      * when "Language" Dip Switch is set to "English", there is a (c) 1996 "warning"
        screen, but when it is set to "Korean", there is a (c) 1995 "warning" screen !

  - I don't understand the interest of the "Remove Monsters" Dip Switch :
    as I haven't found a way to "end" a level, I guess that it was used to
    test the backgrounds and the "platforms".

  - The "Edit Levels" Dip Switch allows you to add/delete monsters and
    change their position.

    This needs more investigation to get similar infos to "Tumble Pop" bootlegs/ripoffs.


9) 'bcstry*'

  - DSW 2-6 and DSW 2-7 are read once (check code at 0x000628),
    but the address where the computed value is stored is NEVER read back !
    I guess this might be a leftover from another game.

  - The "Event Selection" Dip Switch allows you to select your events by
    pressing BUTTON1 when the moving "cursor" is on the event you want

  - The "Control Type" Dip Switch only has an effect during the events :
    it means that whatever the settings are, you need the joystick and buttons
    to select your character, enter your initials and select your events.

  - The "Debug Mode" Dip Switch (known as "Test Mode" ingame) allows you
    to select your events by using the joystick then pressing BUTTON1.
    This Dip Switch overrides "Event Selection" Dip Switch !


10) 'semibase'

  - I'm completely unsure that there are 4 coin slots for this game,
    but pressing ANY of the 4 COIN* buttons has the same effect :
    credits are incremented depending on "Coinage" settings.

  - "Free Play" Dip Switch is bogus : you can only start a VS computer game
    unless you insert enough credits (see "Full 2 Players Game" Dip Switch).

  - DSW 2-6 and DSW 2-7 are read once (check code at 0x002384),
    but the address where the computed value is stored is NEVER read back !
    I guess this might be a leftover from another game.

  - DSW 2-4 is read once (check code at 0x002366), but the computed value
    is the same (0x0009) whatever the settings are. So I've marked it as "Unused".
    This Dip Switch was supposed to determine the last inning if there is a draw
    after inning 9 un some other Semicom baseball games (eg: 'wlstar').

  - The "VS CPU Game Ends" Dip Switch is used when you play a game against the CPU.
    If CPU's score is +10 or +7 than player's score, the game ends.

  - The "VS Game" Dip Switch affects single game against CPU or other player
    (choices 2 and 3). Once the number of innings is achieved, the player(s)
    is (are) proposed to continue, provided they get enough credits.

  - The "Full 2 Players Game" determines how many credits are required
    to enlighten the 4th choice (2 players 9 innings match).


11) 'dquizgo'

  - "Free Play" Dip Switch is bogus : you can start a game without inserting a coin,
    but you can't use any players joystick controls nor buttons. Ingame bug ?

  - "Lives" Dip Switch isn't visible in "Test Mode".

  - I'm unsure about the number of buttons for this game : there are 3 buttons in
    "Test Mode", but as far as I can see only 1 seems needed when you play the game
    and 2 buttons are required to exit some choices of "Test Mode".




 MuHanSeungBu
 ------------

 Unfinished Test Mode, Hangs with black screen, same as a real PCB.



 Pang Pang
 ---------

 You can't select anything except the first stage on the 'select a stage'
 screen.

 If you get a high score then your entry in the high-score table will be
 corrupt.

 There is a chance that both of these are bugs of the original game, it is
 just a cheap hack of Tumble Pop afterall.  The board doesn't work so it's
 impossible to test.

 The sound is driven by a read-protected PIC, as is the case with the
 tumblepb2 bootleg.  I've simulated the sound in tumbleb2 and am using
 the same simulation code in pangpang, although pangpang has a few extra
 sounds which are not mapped.  As the board does not work the accuracy
 of the sound simulation cannot be verified.

 SD Fighters
 -----------

 "Time" and "Rounds to Win" Dip Switches aren't visible in "Test Mode".


***************************************************************************/

#include "emu.h"
#include "tumbleb.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/h6280/h6280.h"
#include "cpu/mcs51/mcs51.h" // for semicom mcu
#include "cpu/pic16c5x/pic16c5x.h"
#include "decocrpt.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "sound/ymopl.h"
#include "speaker.h"


#define TUMBLEP_HACK    0



/******************************************************************************/

void tumbleb_state::tumblepb_oki_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask == 0xffff)
	{
		m_oki->write(data & 0xff);
		//printf("tumbleb_oki_w %04x %04x\n", data, mem_mask);
	}
	else
	{
		m_oki->write((data >> 8) & 0xff);
		//printf("tumbleb_oki_w %04x %04x\n", data, mem_mask);
	}
	/* STUFF IN OTHER BYTE TOO..*/
}

uint16_t tumbleb_state::tumblepb_prot_r()
{
	return ~0;
}

/******************************************************************************/

uint16_t tumbleb_state::tumblepopb_controls_r(offs_t offset)
{
	switch (offset << 1)
	{
		case 0:
			return ioport("PLAYERS")->read();
		case 2:
			return ioport("DSW")->read();
		case 8:
			return ioport("SYSTEM")->read();
		case 10: /* ? */
		case 12:
			return 0;
	}

	return -0;
}

/******************************************************************************/

/*  Tumble Pop Bootleg Sound Simulation + Notes
  tumblepb2 uses a PIC for the sound cpu, this is read protected, so we have to simulate it

1-11 are instruments
12 - enemy bounce off sides
13 - collect coin/item
14 - ??
15 - suck clown
16 - suck man
17 - power up item
18 - general suck
19 - another suck? or unused?
1a - world 1 clown boss bomb explode (maybe)..
1b - brazil boss, fire from ground
1c - pop?
1d - world 1 clown boss hit
1e - america, turtle spit
1f - man spitting fire
20 - france boss die (maybe) / antartica boss land
21 - france boss being hit
22 - taken too long
23 - used for brazil music?
24 - used for brazil music?
25 - egypt world genie boss sound
26 - final boss
27 - bag explode warning
28 - Let's Clean Up
29 - Tumble Pop! (between levels..)
2a - You Did It!
2b - Death
2c - france world boss arms / snowman fire in antartica
2d - space enemy fire
2e - egypt world genie appear
2f - coin
30 - france cannon fire
31 - giant vacuum? (i got this once on antartica..)
32 - world 1 clown boss bomb bounce
33 - end level
34 - end world?

*/

/* music

command 1 - stop?

        4 - map screen
        5 - america
        6 - asia
        7 - egypt
        8 - antartica
        9 - brazil
        a - japan
        b - australia
        c - france
        d - how to play


        f - stage clear
        10 - boss stage
        12 - between levels

        -- there are more tunes than we have music banks..
           i guess some get repeated
*/


void tumbleb_state::tumbleb2_playmusic(okim6295_device *oki)
{
	int status = oki->read();

	if (m_music_is_playing)
	{
		if (!BIT(status, 3))
		{
			oki->write(0x80 | m_music_command);
			oki->write(0x00 | 0x82);
		}
	}
}


INTERRUPT_GEN_MEMBER(tumbleb_state::tumbleb2_interrupt)
{
	device.execute().set_input_line(6, HOLD_LINE);
	tumbleb2_playmusic(m_oki);
}

static const int tumbleb_sound_lookup[256] = {
	/*0     1     2     3     4     5     6     7     8     9     a     b     c     d     e    f*/
	0x00,  -2,  0x00, 0x00,   -2,   -2,   -2,   -2,   -2,   -2,   -2,   -2,   -2,   -2, 0x00,   -2, /* 0 */
		-2, 0x00,   -2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 1 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, /* 2 */
	0x19, 0x00, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, /* 3 */
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, /* 4 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 5 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 6 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 9 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* a */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* b */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* c */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* d */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* e */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* f */
};

/* we use channels 1,2,3 for sound effects, and channel 4 for music */
void tumbleb_state::tumbleb2_set_music_bank( int bank )
{
	uint8_t *oki = memregion("oki")->base();
	memcpy(&oki[0x38000], &oki[0x80000 + 0x38000 + 0x8000 * bank], 0x8000);
}

void tumbleb_state::tumbleb2_play_sound( okim6295_device *oki, int data )
{
	int status = oki->read();

	if (!BIT(status, 0))
	{
		oki->write(0x80 | data);
		oki->write(0x00 | 0x12);
	}
	else if (!BIT(status, 1))
	{
		oki->write(0x80 | data);
		oki->write(0x00 | 0x22);
	}
	else if (!BIT(status, 2))
	{
		oki->write(0x80 | data);
		oki->write(0x00 | 0x42);
	}
}

/* yay for terrible looped music .. there aren't even enough songs for all the levels */
// bank 0 - tune 1 = end of level
// bank 0 - tune 2 = end of stage?
// bank 1 = map screen
// bank 2 = asia
// bank 3 = antartica?
// bank 4 = south america
// bank 5 = australia
// bank 6 = america?? or europe?
// bank 7 = how to play?
// bank 8 = boss???

void tumbleb_state::process_tumbleb2_music_command( okim6295_device *oki, int data )
{
	int status = oki->read();

	if (data == 1) // stop?
	{
		if (BIT(status, 3))
		{
			oki->write(0x40);       /* Stop playing music */
			m_music_is_playing = 0;
		}
	}
	else
	{
		if (m_music_is_playing != data)
		{
			m_music_is_playing = data;
			oki->write(0x40); // stop the current music
			switch (data)
			{
				case 0x04: // map screen
					m_music_bank = 1;
					m_music_command = 0x38;
					break;

				case 0x05: // america
					m_music_bank = 6;
					m_music_command = 0x38;
					break;

				case 0x06: // asia
					m_music_bank = 2;
					m_music_command = 0x38;
					break;

				case 0x07: // africa/egypt -- don't seem to have a tune for this one
					m_music_bank = 4;
					m_music_command = 0x38;
					break;

				case 0x08: // antartica
					m_music_bank = 3;
					m_music_command = 0x38;
					break;

				case 0x09: // brazil / south america
					m_music_bank = 4;
					m_music_command = 0x38;
					break;

				case 0x0a: // japan -- don't seem to have a tune
					m_music_bank = 2;
					m_music_command = 0x38;
					break;

				case 0x0b: // australia
					m_music_bank = 5;
					m_music_command = 0x38;
					break;

				case 0x0c: // france/europe
					m_music_bank = 6;
					m_music_command = 0x38;
					break;

				case 0x0d: // how to play
					m_music_bank = 7;
					m_music_command = 0x38;
					break;

				case 0x0f: // stage clear
					m_music_bank = 0;
					m_music_command = 0x33;
					break;

				case 0x10: // boss stage
					m_music_bank = 8;
					m_music_command = 0x38;
					break;

				case 0x12: // world clear
					m_music_bank = 0;
					m_music_command = 0x34;
					break;

				default: // anything else..
					m_music_bank = 8;
					m_music_command = 0x38;
					break;
			}

			tumbleb2_set_music_bank(m_music_bank);
			tumbleb2_playmusic(oki);
		}
	}
}


void tumbleb_state::tumbleb2_soundmcu_w(uint16_t data)
{
	int sound = tumbleb_sound_lookup[data & 0xff];

	if (sound == 0x00)
	{
		/* pangpang has more commands than tumbleb2, extra sounds */
		//osd_printf_debug("Command %04x\n", data);
	}
	else if (sound == -2)
	{
		process_tumbleb2_music_command(m_oki, data);
	}
	else
	{
		tumbleb2_play_sound(m_oki, sound);
	}
}

/******************************************************************************/

void tumbleb_state::tumblepopb_main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
#if TUMBLEP_HACK
	map(0x000000, 0x07ffff).writeonly();   /* To write levels modifications */
#endif
	map(0x100000, 0x100001).rw(FUNC(tumbleb_state::tumblepb_prot_r), FUNC(tumbleb_state::tumblepb_oki_w));
	map(0x120000, 0x123fff).ram().share("mainram");
	map(0x140000, 0x1407ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x160000, 0x1607ff).ram().share("spriteram"); /* Bootleg sprite buffer */
	map(0x160800, 0x160807).nopw(); /* writes past the end of spriteram */
	map(0x180000, 0x18000f).r(FUNC(tumbleb_state::tumblepopb_controls_r));
	map(0x18000c, 0x18000d).nopw();
	map(0x1a0000, 0x1a07ff).ram();
	map(0x300000, 0x30000f).w(FUNC(tumbleb_state::tumblepb_control_0_w));
	map(0x320000, 0x320fff).w(FUNC(tumbleb_state::tumblepb_pf1_data_w)).share("pf1_data");
	map(0x322000, 0x322fff).w(FUNC(tumbleb_state::tumblepb_pf2_data_w)).share("pf2_data");
	map(0x340000, 0x3401ff).nopw(); /* Unused row scroll */
	map(0x340400, 0x34047f).nopw(); /* Unused col scroll */
	map(0x342000, 0x3421ff).nopw();
	map(0x342400, 0x34247f).nopw();
}

void tumbleb_state::tumblepopba_main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
#if TUMBLEP_HACK
	map(0x000000, 0x07ffff).writeonly();   /* To write levels modifications */
#endif
	map(0x120000, 0x123fff).ram().share("mainram");
	map(0x140000, 0x1407ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x180000, 0x18000f).r(FUNC(tumbleb_state::tumblepopb_controls_r));
	map(0x180005, 0x180005).lw8(NAME([this] (uint8_t data) { m_oki->set_rom_bank((data >> 4) & 0x03); })); // TODO: verify this
	map(0x180007, 0x180007).w(m_oki, FUNC(okim6295_device::write));
	map(0x1a0000, 0x1a07ff).ram().share("spriteram"); /* Bootleg sprite buffer */
	map(0x1a1000, 0x1a1fff).ram(); // ?
	map(0x200000, 0x200fff).ram(); // ?
	//map(0x300000, 0x30000f).w(FUNC(tumbleb_state::tumblepb_control_0_w)); // 0x180000?
	map(0x320000, 0x320fff).w(FUNC(tumbleb_state::tumblepb_pf1_data_w)).share("pf1_data");
	map(0x322000, 0x322fff).w(FUNC(tumbleb_state::tumblepb_pf2_data_w)).share("pf2_data");
}

void tumbleb_pic_state::funkyjetb_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x120000, 0x1207ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x140000, 0x143fff).ram().share("mainram");
	map(0x160000, 0x1607ff).ram().share("spriteram");
	map(0x1d0382, 0x1d0383).portr("DSW");
	map(0x242102, 0x242103).portr("SYSTEM");
	map(0x200000, 0x2007ff).ram(); // writes 0x180
	map(0x300000, 0x30000f).w(FUNC(tumbleb_pic_state::tumblepb_control_0_w));
	map(0x320000, 0x320fff).ram().w(FUNC(tumbleb_pic_state::tumblepb_pf1_data_w)).share("pf1_data");
	map(0x322000, 0x322fff).ram().w(FUNC(tumbleb_pic_state::tumblepb_pf2_data_w)).share("pf2_data");
	map(0x340000, 0x340bff).ram().share("pf1_rowscroll");
	//map(0x342000, 0x342bff).ram().share("pf2_rowscroll");
}

void tumbleb_pic_state::funkyjetb_oki_map(address_map &map)
{
	map(0x00000, 0x37fff).rom().region("oki", 0);
	map(0x38000, 0x3ffff).bankr("okibank");
}

void tumbleb_state::unico_base_map(address_map &map)
{
	map(0x100000, 0x100003).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0x00ff);
	map(0x100005, 0x100005).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x160000, 0x1607ff).ram().share("spriteram"); /* sprites */
	map(0x160800, 0x16080f).nopw(); /* goes slightly past the end of spriteram? */
	map(0x180000, 0x18000f).r(FUNC(tumbleb_state::tumblepopb_controls_r));
	map(0x18000c, 0x18000d).nopw();
	map(0x1a0000, 0x1a07ff).ram();
	map(0x300000, 0x30000f).w(FUNC(tumbleb_state::tumblepb_control_0_w));
	map(0x320000, 0x321fff).ram().w(FUNC(tumbleb_state::fncywld_pf1_data_w)).share("pf1_data");
	map(0x322000, 0x323fff).ram().w(FUNC(tumbleb_state::fncywld_pf2_data_w)).share("pf2_data");
	map(0x340000, 0x3401ff).nopw(); /* Unused row scroll */
	map(0x340400, 0x34047f).nopw(); /* Unused col scroll */
	map(0x342000, 0x3421ff).nopw();
	map(0x342400, 0x34247f).nopw();
}

void tumbleb_state::fncywld_main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0xff0000, 0xffffff).ram();

	unico_base_map(map);
}


void tumbleb_state::magipur_main_map(address_map &map)
{
	map(0x000000, 0x00ffff).ram().share("mainram");
	map(0xf00000, 0xffffff).rom().region("maincpu", 0);

	map(0x100010, 0x100011).nopw(); // TODO: what is this, fncywld doesn't write here, can't be related to the RAM/ROM arrangement as the writes happen well after boot

	unico_base_map(map);
}


uint16_t tumbleb_state::semibase_unknown_r()
{
	return machine().rand();
}

void tumbleb_state::htchctch_main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10000f).r(FUNC(tumbleb_state::semibase_unknown_r));
	map(0x100000, 0x100001).w(FUNC(tumbleb_state::semicom_soundcmd_w));
	map(0x100002, 0x100003).w(FUNC(tumbleb_state::bcstory_tilebank_w));
	map(0x120000, 0x123fff).ram().share("mainram");
	map(0x140000, 0x1407ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x160000, 0x160fff).ram().share("spriteram"); /* Bootleg sprite buffer */
	map(0x180000, 0x18000f).r(FUNC(tumbleb_state::tumblepopb_controls_r));
	map(0x18000c, 0x18000d).nopw();
	map(0x1a0000, 0x1a0fff).ram();
	map(0x300000, 0x30000f).w(FUNC(tumbleb_state::tumblepb_control_0_w));
	map(0x320000, 0x321fff).w(FUNC(tumbleb_state::tumblepb_pf1_data_w)).share("pf1_data");
	map(0x322000, 0x322fff).w(FUNC(tumbleb_state::tumblepb_pf2_data_w)).share("pf2_data");
	map(0x323000, 0x331fff).noprw(); // metal saver writes there when clearing the above tilemaps, flaw in the program routine
	map(0x341000, 0x342fff).ram();             // Extra ram?
}



void tumbleb_state::jumpkids_sound_w(uint16_t data)
{
	m_soundlatch->write(data & 0xff);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

void tumbleb_state::suprtrio_main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x700000, 0x700fff).ram().share("spriteram");
	map(0xa00000, 0xa0000f).ram().share("control");
	map(0xa20000, 0xa20fff).ram().w(FUNC(tumbleb_state::tumblepb_pf1_data_w)).share("pf1_data");
	map(0xa22000, 0xa22fff).ram().w(FUNC(tumbleb_state::tumblepb_pf2_data_w)).share("pf2_data");
	map(0xcf0000, 0xcf05ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xe00000, 0xe00001).portr("PLAYERS").w(FUNC(tumbleb_state::suprtrio_tilebank_w));
	map(0xe40000, 0xe40001).portr("SYSTEM");
	map(0xe80002, 0xe80003).portr("DSW");
	map(0xec0000, 0xec0001).w(FUNC(tumbleb_state::semicom_soundcmd_w));
	map(0xee0001, 0xee0001).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// writes here after bonus stages,
			// expects bits 7-4 of SYSTEM port to read back same value.
			// cfr. https://mametesters.org/view.php?id=7148
			logerror("Write to prot latch %02x\n", data);
			m_suprtrio_prot_latch = data & 0xf;
		})
	);
	map(0xf00000, 0xf07fff).ram();
}

void tumbleb_state::pangpang_main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x120000, 0x123fff).ram().share("mainram");
	map(0x140000, 0x1407ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x160000, 0x1607ff).ram().share("spriteram"); /* Bootleg sprite buffer */
	map(0x160800, 0x160807).nopw(); // writes past the end of spriteram
	map(0x180000, 0x18000f).r(FUNC(tumbleb_state::tumblepopb_controls_r));
	map(0x1a0000, 0x1a07ff).ram();
	map(0x300000, 0x30000f).w(FUNC(tumbleb_state::tumblepb_control_0_w));
	map(0x320000, 0x321fff).ram().w(FUNC(tumbleb_state::pangpang_pf1_data_w)).share("pf1_data");
	map(0x340000, 0x341fff).ram().w(FUNC(tumbleb_state::pangpang_pf2_data_w)).share("pf2_data");
}


/******************************************************************************/

void tumbleb_pic_state::oki_bank_w(uint8_t data)
{
	m_okibank->set_entry(data & 0x0f);
}

uint8_t tumbleb_pic_state::pic_data_r()
{
	return m_pic_data;
}

void tumbleb_pic_state::pic_data_w(uint8_t data)
{
	m_pic_data = data;
}

void tumbleb_pic_state::pic_ctrl_w(uint8_t data)
{
	if (!BIT(data, 2))
	{
		if (!BIT(data, 0))
			m_pic_data = m_oki->read();
		else if (!BIT(data, 1))
		{
			logerror("OKI write: %02X\n", m_pic_data);
			m_oki->write(m_pic_data);
		}
	}

	if (!BIT(data, 4))
		m_pic_data = m_soundlatch->read();
	if (!BIT(data, 5))
		m_soundlatch->acknowledge_w();
}

void tumbleb_pic_state::driver_start()
{
	m_pic_data = 0xff;
	save_item(NAME(m_pic_data));

	m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x8000);
}

/******************************************************************************/

void tumbleb_state::semicom_soundcmd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_soundlatch->write(data & 0xff);
		// needed for Super Trio which reads the sound with polling
		// m_maincpu->spin_until_time(attotime::from_usec(100));
		machine().scheduler().perfect_quantum(attotime::from_usec(20));

	}
}

void tumbleb_state::oki_sound_bank_w(uint8_t data)
{
	uint8_t *oki = memregion("oki")->base();
	memcpy(&oki[0x30000], &oki[(data * 0x10000) + 0x40000], 0x10000);
}

void tumbleb_state::semicom_sound_map(address_map &map)
{
	map(0x0000, 0xcfff).rom();
	map(0xd000, 0xd7ff).ram();
	map(0xf000, 0xf001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xf002, 0xf002).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	//map(0xf006, 0xf006) ??
	map(0xf008, 0xf008).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xf00e, 0xf00e).w(FUNC(tumbleb_state::oki_sound_bank_w));
}

void tumbleb_state::suprtrio_sound_map(address_map &map)
{
	map(0x0000, 0xcfff).rom();
	map(0xd000, 0xd7ff).ram();
	map(0xf002, 0xf002).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	//map(0xf006, 0xf006) ??
	map(0xf008, 0xf008).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xf00e, 0xf00e).w(FUNC(tumbleb_state::oki_sound_bank_w));
}

/* Jump Kids */

void tumbleb_state::jumpkids_main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x100001).w(FUNC(tumbleb_state::jumpkids_sound_w));
	map(0x120000, 0x123fff).ram().share("mainram");
	map(0x140000, 0x1407ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x160000, 0x1607ff).ram().share("spriteram"); /* Bootleg sprite buffer */
	map(0x160800, 0x160807).nopw(); /* writes past the end of spriteram */
	map(0x180000, 0x18000f).r(FUNC(tumbleb_state::tumblepopb_controls_r));
	map(0x18000c, 0x18000d).nopw();
	map(0x1a0000, 0x1a07ff).ram();
	map(0x300000, 0x30000f).w(FUNC(tumbleb_state::tumblepb_control_0_w));
	map(0x320000, 0x320fff).w(FUNC(tumbleb_state::tumblepb_pf1_data_w)).share("pf1_data");
	map(0x322000, 0x322fff).w(FUNC(tumbleb_state::tumblepb_pf2_data_w)).share("pf2_data");
	map(0x340000, 0x3401ff).nopw(); /* Unused row scroll */
	map(0x340400, 0x34047f).nopw(); /* Unused col scroll */
	map(0x342000, 0x3421ff).nopw();
	map(0x342400, 0x34247f).nopw();
}

void tumbleb_state::jumpkids_oki_bank_w(uint8_t data)
{
	uint8_t* sound1 = memregion("oki")->base();
	uint8_t* sound2 = memregion("oki2")->base();
	int bank = data & 0x03;

	memcpy(sound1 + 0x20000, sound2 + bank * 0x20000, 0x20000);
}

void tumbleb_state::jumpkids_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).w(FUNC(tumbleb_state::jumpkids_oki_bank_w));
	map(0x9800, 0x9800).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


/* Semicom AT89C52 MCU */

// probably not endian safe
void tumbleb_state::prot_p0_w(uint8_t data)
{
	uint16_t word = m_mainram[(m_protbase/2) + m_semicom_prot_offset];
	word = (word & 0xff00) | (data << 0);
	m_mainram[(m_protbase/2) + m_semicom_prot_offset] = word;
}

// probably not endian safe
void tumbleb_state::prot_p1_w(uint8_t data)
{
	uint16_t word = m_mainram[(m_protbase/2) + m_semicom_prot_offset];
	word = (word & 0x00ff) | (data << 8);
	m_mainram[(m_protbase/2) + m_semicom_prot_offset] = word;
}

void tumbleb_state::prot_p2_w(uint8_t data)
{
	m_semicom_prot_offset = data;
}

/******************************************************************************/

static INPUT_PORTS_START( tumblepb )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0001, 0x0001, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
#if TUMBLEP_HACK
	PORT_DIPNAME( 0x0800, 0x0800, "Remove Monsters" )   PORT_DIPLOCATION("SW2:5")
#else
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:5")       // See notes
#endif
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
#if TUMBLEP_HACK
	PORT_DIPNAME( 0x0400, 0x0400, "Edit Levels" )       PORT_DIPLOCATION("SW2:6")
#else
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:6")       // See notes
#endif
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( metlsavr )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")       // to be confirmed
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0008, "4" )
	PORT_DIPSETTING(      0x0004, "5" )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:4,3,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW2:7")     // See notes
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW2:6")     // See notes
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Language ) )         PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0800, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Korean ) )
	PORT_DIPNAME( 0x3000, 0x3000, "Life Meter" )                PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(      0x0000, "66%" )
	PORT_DIPSETTING(      0x3000, "100%" )
	PORT_DIPSETTING(      0x2000, "133%" )
	PORT_DIPSETTING(      0x1000, "166%" )
	PORT_DIPNAME( 0xc000, 0xc000, "Time" )                      PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(      0x4000, "30 Seconds" )
	PORT_DIPSETTING(      0x8000, "40 Seconds" )
	PORT_DIPSETTING(      0xc000, "60 Seconds" )
	PORT_DIPSETTING(      0x0000, "80 Seconds" )
INPUT_PORTS_END

ioport_value tumbleb_state::suprtrio_prot_latch_r()
{
	return m_suprtrio_prot_latch;
}

static INPUT_PORTS_START( suprtrio )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(tumbleb_state, suprtrio_prot_latch_r)
	PORT_BIT( 0xff0e, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")   /* Dip switches */
	PORT_DIPNAME( 0x0007, 0x0000, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW:8,7,6")
	PORT_DIPSETTING(      0x0006, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0018, 0x0010, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW:5,4")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0018, "5" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(      0x0000, "50000" )
	PORT_DIPSETTING(      0x0040, "60000" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SW:1" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( fncywld )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )        // duplicated setting
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Language ) )     // changes the region disclaimer and title screen only, not story text
	PORT_DIPSETTING(      0x0004, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Korean ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0001, 0x0001, "2 Coins to Start, 1 to Continue" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )   // to be confirmed
	PORT_DIPSETTING(      0x3000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )       // See notes (is 'Remove Monsters' if code is patched to enable debug mode)
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )       // See notes (is 'Edit Levels' if code is patched to enable debug mode)
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Freeze" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( magipur )
	PORT_INCLUDE(fncywld)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( htchctch )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")       // to be confirmed
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:1" )

	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x0600, 0x0600, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:7,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW2:2")     // See notes
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cookbib )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")       // to be confirmed
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:1" )

	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x0600, 0x0600, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:7,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Winning Rounds (VS Mode)" )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( chokchok )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")       // to be confirmed
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Winning Rounds" )    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPNAME( 0x0006, 0x0006, "Ball Speed" )        PORT_DIPLOCATION("SW1:7,6")
	PORT_DIPSETTING(      0x0000, "Slow" )
	PORT_DIPSETTING(      0x0006, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, "Fast" )
	PORT_DIPSETTING(      0x0004, "Very Fast" )
	PORT_DIPNAME( 0x0018, 0x0018, "Energy Decrease" )   PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(      0x0000, "Very Small" )
	PORT_DIPSETTING(      0x0010, "Small" )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, "Big" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, "60 Seconds" )
	PORT_DIPSETTING(      0x0000, "90 Seconds" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Starting Balls" )    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x00c0, "4" )
	PORT_DIPSETTING(      0x0040, "5" )
	PORT_DIPSETTING(      0x0080, "6" )

	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x0600, 0x0600, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:7,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x4000, DEF_STR( Korean ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( magicbal )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")       // to be confirmed
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") /* Switch positions based on other games - needs to be verified on real hardware!! */
	PORT_DIPNAME( 0x0003, 0x0003, "Game Time" )     PORT_DIPLOCATION("SW1:8,7") /* Only used if Game is Timed, does this control # of innings if not timed? */
	PORT_DIPSETTING(      0x0003, "5:00" )
	PORT_DIPSETTING(      0x0001, "6:00" )
	PORT_DIPSETTING(      0x0002, "7:00" )
	PORT_DIPSETTING(      0x0000, "8:00" )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:5,4,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Timed Game" )        PORT_DIPLOCATION("SW1:2") /* Game is timed or by innings? */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "2 Players Game" )    PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPSETTING(      0x0800, "2 Credits" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wlstar )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")       // to be confirmed
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPNAME( 0x0010, 0x0010, "2 Players Game" )            PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPSETTING(      0x0010, "2 Credits" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW1:3" )                                 // See notes
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Hardest ) )

	PORT_DIPNAME( 0x0100, 0x0100, "Last Inning" )               PORT_DIPLOCATION("SW2:8")     // See notes
	PORT_DIPSETTING(      0x0000, "9" )
	PORT_DIPSETTING(      0x0100, "12" )
	PORT_DIPNAME( 0x0200, 0x0200, "VS CPU Game Ends" )          PORT_DIPLOCATION("SW2:7")     // See notes
	PORT_DIPSETTING(      0x0200, "+10" )
	PORT_DIPSETTING(      0x0000, "+7" )
	PORT_DIPNAME( 0x0400, 0x0400, "VS Game" )                   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, "1 Credit / 2 Innings" )
	PORT_DIPSETTING(      0x0400, "1 Credit / 3 Innings" )
	PORT_DIPNAME( 0x0800, 0x0800, "Full 2 Players Game" )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, "4 Credits" )
	PORT_DIPSETTING(      0x0800, "6 Credits" )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW2:4,3,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wondl96 )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )      // must be high to avoid endless loops
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")       // to be confirmed
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:7,6,5")  // See notes
	PORT_DIPSETTING(      0x000e, "0" )
	PORT_DIPSETTING(      0x0006, "1" )
	PORT_DIPSETTING(      0x000a, "2" )
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x000c, "4" )
	PORT_DIPSETTING(      0x0004, "5" )
	PORT_DIPSETTING(      0x0008, "6" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x0010, 0x0010, "Field Color" )               PORT_DIPLOCATION("SW1:4")     // See notes
	PORT_DIPSETTING(      0x0010, "Blue" )
	PORT_DIPSETTING(      0x0000, "Green" )
	PORT_DIPNAME( 0x0020, 0x0020, "VS CPU Game Ends" )          PORT_DIPLOCATION("SW1:3")     // See notes
	PORT_DIPSETTING(      0x0020, "+10" )
	PORT_DIPSETTING(      0x0000, "+7" )
	PORT_DIPNAME( 0x0040, 0x0040, "VS Game" )                   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, "1 Credit / 2 Innings" )
	PORT_DIPSETTING(      0x0040, "1 Credit / 3 Innings" )
	PORT_DIPNAME( 0x0080, 0x0000, "Full 2 Players Game" )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0080, "4 Credits" )
	PORT_DIPSETTING(      0x0000, "6 Credits" )

	PORT_DIPUNUSED_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:7,6,5")
	PORT_DIPSETTING(      0x0400, "Level 1" )
	PORT_DIPSETTING(      0x0800, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x0e00, "Level 4" )
	PORT_DIPSETTING(      0x0600, "Level 5" )
	PORT_DIPSETTING(      0x0a00, "Level 6" )
	PORT_DIPSETTING(      0x0200, "Level 7" )
	PORT_DIPSETTING(      0x0c00, "Level 8" )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW2:4,3,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:1" )                                 // See notes
INPUT_PORTS_END

static INPUT_PORTS_START( sdfight )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")       // to be confirmed
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_SERVICE_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:8" ) /* These dips were done from the Test mode screens */
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:7,6,5")
	PORT_DIPSETTING(      0x0004, "Level 1" )
	PORT_DIPSETTING(      0x0008, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x000e, "Level 4" )
	PORT_DIPSETTING(      0x0006, "Level 5" )
	PORT_DIPSETTING(      0x000a, "Level 6" )
	PORT_DIPSETTING(      0x0002, "Level 7" )
	PORT_DIPSETTING(      0x000c, "Level 8" )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:4,3,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:8") /* Only Free Play shows in Test Mode */
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Rounds to Win" )             PORT_DIPLOCATION("SW2:5") /* Does not show up in Test Mode screen */
	PORT_DIPSETTING(      0x0800, "2 Rounds" )
	PORT_DIPSETTING(      0x0000, "3 Rounds" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Time" )                      PORT_DIPLOCATION("SW2:2,1") /* Does not show up in Test Mode screen */
	PORT_DIPSETTING(      0x4000, "30" )
	PORT_DIPSETTING(      0x8000, "50" )
	PORT_DIPSETTING(      0xc000, "70" )
	PORT_DIPSETTING(      0x0000, "90" )
INPUT_PORTS_END

static INPUT_PORTS_START( bcstory )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")       // to be confirmed
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_SERVICE_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:7,6,5")
	PORT_DIPSETTING(      0x0004, "Level 1" )
	PORT_DIPSETTING(      0x0008, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x000e, "Level 4" )
	PORT_DIPSETTING(      0x0006, "Level 5" )
	PORT_DIPSETTING(      0x000a, "Level 6" )
	PORT_DIPSETTING(      0x0002, "Level 7" )
	PORT_DIPSETTING(      0x000c, "Level 8" )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:4,3,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0200, IP_ACTIVE_LOW, "SW2:7" )                                 // See notes
	PORT_DIPUNUSED_DIPLOC( 0x0400, IP_ACTIVE_LOW, "SW2:6" )                                 // See notes
	PORT_DIPUNUSED_DIPLOC( 0x0800, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPNAME( 0x2000, 0x0000, "Event Selection" )           PORT_DIPLOCATION("SW2:3")     // See notes
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Control Type" )              PORT_DIPLOCATION("SW2:2")     // See notes
	PORT_DIPSETTING(      0x4000, "Joystick + Buttons" )
	PORT_DIPSETTING(      0x0000, "Buttons Only" )
	PORT_DIPNAME( 0x8000, 0x8000, "Debug Mode" )                PORT_DIPLOCATION("SW2:1")     // See notes
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( semibase )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )                                            // See notes
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )                                            // See notes
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_SERVICE_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:7,6,5")
	PORT_DIPSETTING(      0x0004, "Level 1" )
	PORT_DIPSETTING(      0x0008, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x000e, "Level 4" )
	PORT_DIPSETTING(      0x0006, "Level 5" )
	PORT_DIPSETTING(      0x000a, "Level 6" )
	PORT_DIPSETTING(      0x0002, "Level 7" )
	PORT_DIPSETTING(      0x000c, "Level 8" )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:4,3,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0200, IP_ACTIVE_LOW, "SW2:7" )                                 // See notes
	PORT_DIPUNUSED_DIPLOC( 0x0400, IP_ACTIVE_LOW, "SW2:6" )                                 // See notes
	PORT_DIPUNUSED_DIPLOC( 0x0800, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:4" )                                 // See notes
	PORT_DIPNAME( 0x2000, 0x2000, "VS CPU Game Ends" )          PORT_DIPLOCATION("SW2:3")     // See notes
	PORT_DIPSETTING(      0x2000, "+10" )
	PORT_DIPSETTING(      0x0000, "+7" )
	PORT_DIPNAME( 0x4000, 0x4000, "VS Game" )                   PORT_DIPLOCATION("SW2:2")     // See notes
	PORT_DIPSETTING(      0x4000, "1 Credit / 2 Innings" )
	PORT_DIPSETTING(      0x0000, "1 Credit / 3 Innings" )
	PORT_DIPNAME( 0x8000, 0x8000, "Full 2 Players Game" )       PORT_DIPLOCATION("SW2:1")     // See notes
	PORT_DIPSETTING(      0x0000, "4 Credits" )
	PORT_DIPSETTING(      0x8000, "6 Credits" )
INPUT_PORTS_END

static INPUT_PORTS_START( dquizgo )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")       // to be confirmed
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_SERVICE_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:7,6,5")
	PORT_DIPSETTING(      0x0004, "Level 1" )
	PORT_DIPSETTING(      0x0008, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x000e, "Level 4" )
	PORT_DIPSETTING(      0x0006, "Level 5" )
	PORT_DIPSETTING(      0x000a, "Level 6" )
	PORT_DIPSETTING(      0x0002, "Level 7" )
	PORT_DIPSETTING(      0x000c, "Level 8" )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:4,3,2")
//  PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
//  PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:8")     // See notes
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0200, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0400, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:2,1")   // See notes
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
	PORT_DIPSETTING(      0x8000, "5" )
INPUT_PORTS_END



static INPUT_PORTS_START( carket )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")       // to be confirmed
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_SERVICE_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:7,6") /* "Rest Chance": 4 for Easy, 3 for Normal & Hard, 2 for Very Hard */
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:5,4,3") /* Tested correct */
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2") /* Test Mode shows Language: Korean / English, but doesn't work?? */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8") /* Shows as "Test Mode" - Unknown Use */
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, "Default Cards" )      PORT_DIPLOCATION("SW2:4,3") /* Tested correct */
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x1000, "4" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPNAME( 0xc000, 0xc000, "Default Balls" )      PORT_DIPLOCATION("SW2:2,1") /* Tested correct */
	PORT_DIPSETTING(      0xc000, "6" )
	PORT_DIPSETTING(      0x4000, "9" )
	PORT_DIPSETTING(      0x8000, "12" )
	PORT_DIPSETTING(      0x0000, "15" )
INPUT_PORTS_END


/******************************************************************************/

static const gfx_layout tcharlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};


static const gfx_layout tlayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ STEP8(16*8*2, 1), STEP8(0, 1) },
	{ STEP16(0, 8*2) },
	64*8
};

static const gfx_layout suprtrio_tlayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(2,4),RGN_FRAC(0,4), RGN_FRAC(3,4), RGN_FRAC(1,4) },

	{ 0, 1, 2, 3, 4, 5, 6, 7,16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6,16*8+7 },
	{ 1*8, 0*8, 2*8, 3*8, 5*8, 4*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};


static GFXDECODE_START( gfx_tumbleb )
	GFXDECODE_ENTRY( "tilegfx", 0, tcharlayout, 256, 16 )  /* Characters 8x8 */
	GFXDECODE_ENTRY( "tilegfx", 0, tlayout,     512, 16 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "tilegfx", 0, tlayout,     256, 16 )  /* Tiles 16x16 */
GFXDECODE_END

static GFXDECODE_START( gfx_tumbleb_spr )
	GFXDECODE_ENTRY( "sprgfx", 0, tlayout, 0, 16 )  /* Sprites 16x16 */
GFXDECODE_END

static GFXDECODE_START( gfx_suprtrio )
	GFXDECODE_ENTRY( "tilegfx", 0, tcharlayout,        256, 16 )   /* Characters 8x8 */
	GFXDECODE_ENTRY( "tilegfx", 0, suprtrio_tlayout,   512, 16 )   /* Tiles 16x16 */
	GFXDECODE_ENTRY( "tilegfx", 0, suprtrio_tlayout,   256, 16 )   /* Tiles 16x16 */
GFXDECODE_END

static GFXDECODE_START( gfx_fncywld )
	GFXDECODE_ENTRY( "tilegfx", 0, tcharlayout, 0x400, 0x40 )  /* Characters 8x8 */
	GFXDECODE_ENTRY( "tilegfx", 0, tlayout,     0x400, 0x40 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "tilegfx", 0, tlayout,     0x200, 0x40 )  /* Tiles 16x16 */
GFXDECODE_END

static GFXDECODE_START( gfx_fncywld_spr )
	GFXDECODE_ENTRY( "sprgfx",  0, tlayout,     0x000, 0x40 )  /* Sprites 16x16 */
GFXDECODE_END

/******************************************************************************/


MACHINE_START_MEMBER(tumbleb_state,tumbleb)
{
	save_item(NAME(m_music_command));
	save_item(NAME(m_music_bank));
	save_item(NAME(m_music_is_playing));

	save_item(NAME(m_control_0));
	save_item(NAME(m_tilebank));
}

MACHINE_RESET_MEMBER(tumbleb_state,tumbleb)
{
	m_music_command = 0;
	m_music_bank = 0;
	m_music_is_playing = 0;
	m_tilebank = 0;
	memset(m_control_0, 0, sizeof(m_control_0));
	m_semicom_prot_offset = 0;
}

void tumbleb_state::tumblepb(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 14000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &tumbleb_state::tumblepopb_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tumbleb_state::irq6_line_hold));

	MCFG_MACHINE_START_OVERRIDE(tumbleb_state,tumbleb)
	MCFG_MACHINE_RESET_OVERRIDE(tumbleb_state,tumbleb)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(58);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(529));
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_tumblepb));
	m_screen->set_palette(m_palette);

	DECO_SPRITE(config, m_sprgen, 0, m_palette, gfx_tumbleb_spr);
	m_sprgen->set_is_bootleg(true);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tumbleb);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 1024);

	MCFG_VIDEO_START_OVERRIDE(tumbleb_state,tumblepb)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 8000000/10, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.70);
}

void tumbleb_state::tumblepba(machine_config &config)
{
	tumblepb(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &tumbleb_state::tumblepopba_main_map);
}

void tumbleb_state::tumbleb2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 14000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &tumbleb_state::tumblepopb_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tumbleb_state::tumbleb2_interrupt));

	MCFG_MACHINE_START_OVERRIDE(tumbleb_state,tumbleb)
	MCFG_MACHINE_RESET_OVERRIDE(tumbleb_state,tumbleb)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(58);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(529));
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_tumblepb));
	m_screen->set_palette(m_palette);

	DECO_SPRITE(config, m_sprgen, 0, m_palette, gfx_tumbleb_spr);
	m_sprgen->set_is_bootleg(true);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tumbleb);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 1024);

	MCFG_VIDEO_START_OVERRIDE(tumbleb_state,tumblepb)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 8000000/10, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.70);
}

void tumbleb_pic_state::funkyjetb(machine_config &config)
{
	tumbleb2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &tumbleb_pic_state::funkyjetb_map);
	m_maincpu->set_vblank_int("screen", FUNC(tumbleb_pic_state::irq6_line_hold));

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->set_separate_acknowledge(true);

	pic16c57_device &pic(PIC16C57(config, "pic", XTAL(8'000'000)/2)); // 8MHz xtal on the PCB, divider unconfirmed
	pic.write_a().set(FUNC(tumbleb_pic_state::oki_bank_w));
	pic.read_b().set(FUNC(tumbleb_pic_state::pic_data_r));
	pic.write_b().set(FUNC(tumbleb_pic_state::pic_data_w));
	pic.read_c().set(m_soundlatch, FUNC(generic_latch_8_device::pending_r)).bit(6);
	pic.write_c().set(FUNC(tumbleb_pic_state::pic_ctrl_w));

	m_oki->set_addrmap(0, &tumbleb_pic_state::funkyjetb_oki_map);
}

void tumbleb_state::jumpkids(machine_config &config) // OSCs: 12MHz, 8MHz & 14.31818MHz
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tumbleb_state::jumpkids_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tumbleb_state::irq6_line_hold));

	Z80(config, m_audiocpu, 8_MHz_XTAL/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &tumbleb_state::jumpkids_sound_map);

	MCFG_MACHINE_START_OVERRIDE(tumbleb_state,tumbleb)
	MCFG_MACHINE_RESET_OVERRIDE(tumbleb_state,tumbleb)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(529));
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_jumpkids));
	m_screen->set_palette(m_palette);

	DECO_SPRITE(config, m_sprgen, 0, m_palette, gfx_tumbleb_spr);
	m_sprgen->set_is_bootleg(true);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tumbleb);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 1024);

	MCFG_VIDEO_START_OVERRIDE(tumbleb_state,tumblepb)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	OKIM6295(config, m_oki, 8_MHz_XTAL/8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.70);
}

void tumbleb_state::fncywld(machine_config &config) // OSCs: 12MHz, 4MHz & 28.63636MHz
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tumbleb_state::fncywld_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tumbleb_state::irq6_line_hold));

	MCFG_MACHINE_START_OVERRIDE(tumbleb_state,tumbleb)
	MCFG_MACHINE_RESET_OVERRIDE(tumbleb_state,tumbleb)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(529));
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_fncywld));
	m_screen->set_palette(m_palette);

	DECO_SPRITE(config, m_sprgen, 0, m_palette, gfx_fncywld_spr);
	m_sprgen->set_is_bootleg(true);
	m_sprgen->set_transpen(15);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fncywld);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 0x800);

	MCFG_VIDEO_START_OVERRIDE(tumbleb_state,fncywld)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 4_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.10);

	OKIM6295(config, m_oki, 4_MHz_XTAL/4, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

void tumbleb_state::magipur(machine_config &config) // OSCs: 12MHz, 4MHz, 28.63636MHz, not the same PCB as fncywld
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &tumbleb_state::magipur_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tumbleb_state::irq6_line_hold));

	MCFG_MACHINE_START_OVERRIDE(tumbleb_state,tumbleb)
	MCFG_MACHINE_RESET_OVERRIDE(tumbleb_state,tumbleb)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60); // refresh rate not verified
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(529));
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_fncywld));
	m_screen->set_palette(m_palette);

	DECO_SPRITE(config, m_sprgen, 0, m_palette, gfx_fncywld_spr);
	m_sprgen->set_is_bootleg(true);
	m_sprgen->set_transpen(15);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fncywld);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 0x800);

	MCFG_VIDEO_START_OVERRIDE(tumbleb_state,fncywld)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", XTAL(4'000'000)).add_route(ALL_OUTPUTS, "mono", 0.10);

	OKIM6295(config, m_oki, XTAL(4'000'000)/4, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}


MACHINE_RESET_MEMBER(tumbleb_state,htchctch)
{
	if (memregion("user1") != nullptr)
	{
		/* copy protection data every reset */
		uint16_t *PROTDATA = (uint16_t*)memregion("user1")->base();
		int i, len = memregion("user1")->bytes();

		for (i = 0; i < len / 2; i++)
			m_mainram[0x000 / 2 + i] = PROTDATA[i];
	}

	MACHINE_RESET_CALL_MEMBER(tumbleb);
}

void tumbleb_state::htchctch(machine_config &config) // OSCs: 15MHz, 4.096MHz
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 15_MHz_XTAL); /* verified */
	m_maincpu->set_addrmap(AS_PROGRAM, &tumbleb_state::htchctch_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tumbleb_state::irq6_line_hold));

	Z80(config, m_audiocpu, 15_MHz_XTAL/4); /* 3.75MHz verified on dquizgo */
	m_audiocpu->set_addrmap(AS_PROGRAM, &tumbleb_state::semicom_sound_map);

	MCFG_MACHINE_START_OVERRIDE(tumbleb_state,tumbleb)
	MCFG_MACHINE_RESET_OVERRIDE(tumbleb_state,htchctch)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2400)); // ?? cookbib needs it above ~2400 or the Joystick on the How to Play screen is the wrong colour?!
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_semicom));
	m_screen->set_palette(m_palette);

	DECO_SPRITE(config, m_sprgen, 0, m_palette, gfx_tumbleb_spr);
	m_sprgen->set_is_bootleg(true);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tumbleb);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	MCFG_VIDEO_START_OVERRIDE(tumbleb_state,tumblepb)

	/* sound hardware - same as hyperpac */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	/* on at least hatch catch, cookie & bibi and choky choky the YM2151 clock is connected directly to the Z80 clock so the speed should match */
	ym2151_device &ymsnd(YM2151(config, "ymsnd", 15_MHz_XTAL/4)); /* 3.75MHz verified */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.10);

	/* correct for cookie & bibi and hatch catch, (4096000/4) */
	OKIM6295(config, m_oki, 4.096_MHz_XTAL/4, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void tumbleb_state::htchctch_mcu(machine_config &config) // OSCs: 15MHz, 4.096MHz
{
	htchctch(config);

	i87c52_device &prot(I87C52(config, "protection", 15_MHz_XTAL)); // decapped as 87C51FA (chip was marked as P87C52)
	prot.port_out_cb<0>().set(FUNC(tumbleb_state::prot_p0_w));
	prot.port_out_cb<1>().set(FUNC(tumbleb_state::prot_p1_w));
	prot.port_out_cb<2>().set(FUNC(tumbleb_state::prot_p2_w));
}

void tumbleb_state::cookbib(machine_config &config)
{
	htchctch(config);
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_semicom_altoffsets));
}

// some Choky! Choky! PCBs have left factory with a 3.57mhz while some have a 4.096 which matches other games, assuming the former are factory errors
void tumbleb_state::chokchok(machine_config &config) // OSCs: 15MHz, 4.096MHz
{
	htchctch_mcu(config);

	m_palette->set_format(palette_device::xBGR_444, 1024);
}

void tumbleb_state::cookbib_mcu(machine_config &config) // OSCs: 15MHz, 4.096MHz
{
	htchctch(config);

	/* basic machine hardware */
	at89c52_device &prot(AT89C52(config, "protection", 15_MHz_XTAL));
	prot.port_out_cb<0>().set(FUNC(tumbleb_state::prot_p0_w));
	prot.port_out_cb<1>().set(FUNC(tumbleb_state::prot_p1_w));
	prot.port_out_cb<2>().set(FUNC(tumbleb_state::prot_p2_w));

	/* video hardware */
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_semicom_altoffsets));
}

void tumbleb_state::bcstory(machine_config &config) // OSCs: 15MHz, 4.096MHz
{
	htchctch(config);
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_bcstory));
}

void tumbleb_state::semibase(machine_config &config) // OSCs: 15MHz, 4.096MHz
{
	htchctch(config);
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_semibase));
}

void tumbleb_state::sdfight(machine_config &config) // OSCs: 15MHz, 4.096MHz
{
	htchctch(config);
	MCFG_VIDEO_START_OVERRIDE(tumbleb_state,sdfight)
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_sdfight));
}

void tumbleb_state::metlsavr(machine_config &config)// OSCs: 14MHz, 3.579545MHz
{
	cookbib(config);
	m_palette->set_format(palette_device::xBGR_444, 1024);

	subdevice<ym2151_device>("ymsnd")->set_clock(3.579545_MHz_XTAL);
}


void tumbleb_state::suprtrio(machine_config &config) // OSCs: 14MHz, 12MHz & 8MHz
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 14_MHz_XTAL); /* 14mhz should be correct, but lots of sprite flicker later in game */
	m_maincpu->set_addrmap(AS_PROGRAM, &tumbleb_state::suprtrio_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tumbleb_state::irq6_line_hold));

	Z80(config, m_audiocpu, 8_MHz_XTAL/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &tumbleb_state::suprtrio_sound_map);

	MCFG_MACHINE_START_OVERRIDE(tumbleb_state,tumbleb)
	MCFG_MACHINE_RESET_OVERRIDE(tumbleb_state,tumbleb)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
//  m_screen->set_refresh_hz(60);
//  m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(529));
//  m_screen->set_size(40*8, 32*8);
//  m_screen->set_visarea(0*8, 40*8-1, 1*8-1, 31*8-2);
	// not measured, assume same as tumblep for now.
	// Game has a very dull irq routine to stay at the mercy of set_vblank_time,
	// reportedly happens to randomly crash at stage 3 boss + be laggy on later levels otherwise.
	m_screen->set_raw(XTAL(14'000'000) / 2, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_suprtrio));
	m_screen->set_palette("palette");

	DECO_SPRITE(config, m_sprgen, 0, m_palette, gfx_tumbleb_spr);
	m_sprgen->set_is_bootleg(true);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_suprtrio);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	MCFG_VIDEO_START_OVERRIDE(tumbleb_state,suprtrio)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	OKIM6295(config, m_oki, 8_MHz_XTAL/8, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void tumbleb_state::pangpang(machine_config &config) // OSCs: 14MHz, 12MHz & 8MHz
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 14_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tumbleb_state::pangpang_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tumbleb_state::tumbleb2_interrupt));

	MCFG_MACHINE_START_OVERRIDE(tumbleb_state,tumbleb)
	MCFG_MACHINE_RESET_OVERRIDE(tumbleb_state,tumbleb)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(58);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1529));
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(tumbleb_state::screen_update_pangpang));
	m_screen->set_palette(m_palette);

	DECO_SPRITE(config, m_sprgen, 0, m_palette, gfx_tumbleb_spr);
	m_sprgen->set_is_bootleg(true);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_tumbleb);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 1024);

	MCFG_VIDEO_START_OVERRIDE(tumbleb_state,pangpang)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 8_MHz_XTAL/8, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.70);
}

/******************************************************************************/

ROM_START( tumbleb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE ("thumbpop.12", 0x00000, 0x40000, CRC(0c984703) SHA1(588d2b2464e0027c8d0703a2b62ebda225ba4276) )
	ROM_LOAD16_BYTE( "thumbpop.13", 0x00001, 0x40000, CRC(864c4053) SHA1(013eb35e79aa7a7cd1a8061c4b75b37a8bfb10c6) )

	ROM_REGION( 0x080000, "tilegfx", 0 )
	ROM_LOAD16_BYTE( "thumbpop.19",  0x00000, 0x40000, CRC(0795aab4) SHA1(85b38804446f6b0b4d8c3a59a8958d520c567a4e) )
	ROM_LOAD16_BYTE( "thumbpop.18",  0x00001, 0x40000, CRC(ad58df43) SHA1(2e562bfffb42543af767dd9e82a1d2465dfcd8b8) )

	ROM_REGION( 0x100000, "sprgfx", 0 )
	ROM_LOAD( "map-01.rom",   0x00000, 0x80000, CRC(e81ffa09) SHA1(01ada9557ead91eb76cf00db118d6c432104a398) )
	ROM_LOAD( "map-00.rom",   0x80000, 0x80000, CRC(8c879cfe) SHA1(a53ef7811f14a8b105749b1cf29fe8a3a33bab5e) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "thumbpop.snd", 0x00000, 0x80000, CRC(fabbf15d) SHA1(de60be43a5cd1d4b93c142bde6cbfc48a25545a3) )
ROM_END

ROM_START( tumbleb2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 27c208, 68000 code */
	ROM_LOAD16_BYTE ("wj-2", 0x00000, 0x40000, CRC(34b016e1) SHA1(b4c496358d48469d170a69e8bba58e0ea919b418) )
	ROM_LOAD16_BYTE( "wj-3", 0x00001, 0x40000, CRC(89501c71) SHA1(2c202218934b845fdf7c99eaf280dccad90767f2) )

	ROM_REGION( 0x2d4c, "cpu1", 0 ) /* PIC16c57 */
	ROM_LOAD( "pic_16c57", 0x00000, 0x2d4c, NO_DUMP ) // protected

	ROM_REGION( 0x080000, "tilegfx", 0 ) // 27c208
	ROM_LOAD16_BYTE( "wj-9",  0x00000, 0x40000, CRC(0795aab4) SHA1(85b38804446f6b0b4d8c3a59a8958d520c567a4e) )
	ROM_LOAD16_BYTE( "wj-8",  0x00001, 0x40000, CRC(ad58df43) SHA1(2e562bfffb42543af767dd9e82a1d2465dfcd8b8) )

	ROM_REGION( 0x100000, "sprgfx", 0 ) // in the 0.35 beta cycle this bootleg was added with the same sprite ROMs as the original, but a PCB was found with the following 27c208 ROMs
	ROM_LOAD16_BYTE( "wj-6", 0x00000, 0x40000, CRC(ee91db18) SHA1(06a2f15228a8233b685506077ed1248cd5fc3bb3) ) // map-01.rom   [even]     IDENTICAL
	ROM_LOAD16_BYTE( "wj-7", 0x00001, 0x40000, CRC(87cffb06) SHA1(db3adbbf33cdbff72b6c5ee1228c760cc4897ad0) ) // map-01.rom   [odd]      IDENTICAL
	ROM_LOAD16_BYTE( "wj-4", 0x80000, 0x40000, CRC(79a29725) SHA1(c47366dedaf821f452d8e5394d426f18a79d615e) ) // map-00.rom   [even]     IDENTICAL
	ROM_LOAD16_BYTE( "wj-5", 0x80001, 0x40000, CRC(dda8932e) SHA1(bd20806916cc5774a5cc70907d88c7ab4eb7ac14) ) // map-00.rom   [odd]      IDENTICAL
	//ROM_LOAD( "map-01.rom",   0x00000, 0x80000, CRC(e81ffa09) SHA1(01ada9557ead91eb76cf00db118d6c432104a398) )
	//ROM_LOAD( "map-00.rom",   0x80000, 0x80000, CRC(8c879cfe) SHA1(a53ef7811f14a8b105749b1cf29fe8a3a33bab5e) )

	ROM_REGION( 0x100000, "oki", 0 ) /* 27c408, Oki samples */
	ROM_LOAD( "wj-1", 0x00000, 0x80000, CRC(fabbf15d) SHA1(de60be43a5cd1d4b93c142bde6cbfc48a25545a3) )
	ROM_RELOAD(0x80000,0x80000)

	ROM_REGION(0x1500, "plds", 0 ) // not dumped ones are soldered
	ROM_LOAD( "palce16v8.1",   0x0000, 0x104, NO_DUMP )
	ROM_LOAD( "palce20v8.2",   0x0000, 0x157, NO_DUMP )
	ROM_LOAD( "palce16v8.3",   0x0200, 0x104, NO_DUMP )
	ROM_LOAD( "palce16v8.4",   0x0400, 0x104, NO_DUMP )
	ROM_LOAD( "palce16v8.5",   0x0600, 0x104, NO_DUMP )
	ROM_LOAD( "palce16v8.6",   0x0800, 0x104, NO_DUMP )
	ROM_LOAD( "palce16v8.7",   0x0a00, 0x104, NO_DUMP )
	ROM_LOAD( "palce22v10.8",  0x0c00, 0x2dd, NO_DUMP )
	ROM_LOAD( "palce22v10.9",  0x0f00, 0x2dd, NO_DUMP )
	ROM_LOAD( "palce16v8h.10", 0x1100, 0x117, CRC(eef433f9) SHA1(996bfb88d114b661265edc975eef4c24d0f07b55) )
	ROM_LOAD( "palce16v8.11",  0x1300, 0x104, NO_DUMP )
ROM_END

// different sprite / tilemap handling, might be Playmark style, it had Playmark stickers on the ROMs
ROM_START( tumblepba )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "2.ic83", 0x00000, 0x40000, CRC(b6b50b17) SHA1(e82886efc29f6f67d06e23df42eb970262445d6d) )
	ROM_LOAD16_BYTE ("1.ic82", 0x00001, 0x40000, CRC(f1b514a7) SHA1(046bfc40f8bfe85bcd6e9700d5759a1cf959d421) )

	ROM_REGION( 0x080000, "tilegfx", 0 )
	ROM_LOAD16_BYTE( "11.ic116",  0x00000, 0x20000, CRC(76cb97bb) SHA1(9e42f003774f70fa19cdb1799135c25aed13383c) )
	ROM_LOAD16_BYTE( "10.ic118",  0x00001, 0x20000, CRC(0b38b5ba) SHA1(2ee834a65fba098e9c4f633cfb049b33c9b90d2c) )
	ROM_LOAD16_BYTE( "9.ic120",  0x40000, 0x20000, CRC(56037b6d) SHA1(58e39f2c3525ba3ec3a2d5fd9b2c4a3e5071b7e6) )
	ROM_LOAD16_BYTE( "8.ic122",  0x40001, 0x20000, CRC(9ef861b6) SHA1(555f6fe08137807b4c7f3b1beeed6d853e3ed649) )

	ROM_REGION( 0x100000, "sprgfx", 0 )
	ROM_LOAD16_BYTE( "5.ic119",  0x00000, 0x40000, CRC(59ba9cdb) SHA1(1800a904ce7c651c859551c933c17682708ab303) )
	ROM_LOAD16_BYTE( "4.ic121",  0x00001, 0x40000, CRC(358cb2a8) SHA1(1faf75753fec3d4dfae208dc7eca2fb63ce52eb6) )
	ROM_LOAD16_BYTE( "7.ic115",  0x80000, 0x40000, CRC(0273eec0) SHA1(305d1a111f04650b7e3616fb6ecac1c579312acc) )
	ROM_LOAD16_BYTE( "6.ic117",  0x80001, 0x40000, CRC(7f8daf52) SHA1(aa5f111a9c75c260bb77878bb95c1e34d70ea7b6) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "3.ic24", 0x00000, 0x80000, CRC(63e45de7) SHA1(417f945ee8cf820b1733c4dee26ef05e91e80457) )
ROM_END

ROM_START( funkyjetb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "4-27c020.bin", 0x00000, 0x40000, CRC(4e5bfda3) SHA1(b1bcc4ad1343d379de9a143a72a84db9830c8fa0) )
	ROM_LOAD16_BYTE( "3-27c020.bin", 0x00001, 0x40000, CRC(e253e20c) SHA1(6fe1704872bf807ed24f500c997f62c880f6c5c1) )

	ROM_REGION( 0x1000, "pic", 0 )    /* Sound CPU */
	ROM_LOAD( "1-pic16c57-xt.bin",   0x00000,  0x1000, CRC(653ed006) SHA1(38b7c7b3f40d73e2e7c4361c918eaf4bba2bdc3c) ) // From decap

	ROM_REGION( 0x080000, "tilegfx", 0 )
	ROM_LOAD16_BYTE( "5-27c020.bin", 0x000000, 0x40000, CRC(f75ff923) SHA1(4177e94ba1e3a82861d0277ba5d0abc24482ffe2) )
	ROM_LOAD16_BYTE( "6-27c020.bin", 0x000001, 0x40000, CRC(91a2bcb5) SHA1(c659b5deff2a4b2b69bddb956a3df1ed79a50ab0) )

	ROM_REGION( 0x100000, "sprgfx", 0 )
	ROM_LOAD16_BYTE( "8-27c020.bin",  0x000000, 0x40000, CRC(49aab1d6) SHA1(2c215df271af1d49eeff472298e7aa00879c1799) ) /* sprites */
	ROM_LOAD16_BYTE( "7-27c020.bin",  0x000001, 0x40000, CRC(f6e362bf) SHA1(8e5c82eb19b8948f064360215107311efa9ea12f) )
	ROM_LOAD16_BYTE( "10-27c020.bin", 0x080000, 0x40000, CRC(ffe70cc2) SHA1(e81720c86ea0df7554af060dae66901bf612b4db) )
	ROM_LOAD16_BYTE( "9-27c020.bin",  0x080001, 0x40000, CRC(d13437a6) SHA1(cc848e5e8cc91f6fb5c1de219cc1db57b84337a9) )

	ROM_REGION( 0x80000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "2-27c4001.bin",    0x00000, 0x80000, CRC(7dbc988b) SHA1(0bf89e651a992672ee4041addc9c4614b475c48a) )

	ROM_REGION( 0x1600, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "p1-palce16v8h-25pc.bin",  0x0000, 0x0117, CRC(bb8f5dd6) SHA1(b09f36293f7b4ae30b8bebebcca6457a64b87d0f) )
	ROM_LOAD( "p2-palce22v10h-25pc.bin", 0x0200, 0x02dd, CRC(d53c01da) SHA1(e90eaa95a8b429643cfb0bce7dff86e850ec01d8) )
	ROM_LOAD( "p3-palce22v10h-25pc.bin", 0x0500, 0x02dd, CRC(5c0b43af) SHA1(ef5cfabde32063707c97a9aed63f8bf1bde2cb6d) )
	ROM_LOAD( "p4-palce16v8h-25pc.bin",  0x0800, 0x0117, CRC(36749fe3) SHA1(d2a88d5c2d3d32aeade59d5c4e4474982d19447e) )
	ROM_LOAD( "p5-palce20v8h-25pc.bin",  0x0a00, 0x0157, CRC(1c9b6557) SHA1(ceb78bf78e511251143984626f6b8421e5d627d5) )
	ROM_LOAD( "p6-palce16v8h-25pc.bin",  0x0c00, 0x0117, CRC(76b06a9a) SHA1(32a5d5d6ce4819ae5cceb6499f5f95b77e6b4f33) )
	ROM_LOAD( "p7-palce16v8h-25pc.bin",  0x0e00, 0x0117, CRC(070f48ec) SHA1(51fed4c1072762f4a4d2c0706d6a6cab4c769376) )
	ROM_LOAD( "p8-palce16v8h-25pc.bin",  0x1000, 0x0117, CRC(747270a6) SHA1(7ec39a172400e536bd4136250f8ec391c6b1320f) )
	ROM_LOAD( "p9-palce16v8h-25pc.bin",  0x1000, 0x0117, CRC(954b7413) SHA1(207972f0021f26b5b99f6b96eb650cc3213ae490) )
	ROM_LOAD( "p10-hy18cv8s-25.bin",     0x1400, 0x0155, CRC(6dc83459) SHA1(1121aaedf1f913cccae81dcc1f95e20c24d263fb) )
ROM_END

/*

CPU
1x MC68000P10 (main)
1x Z8400APS (sound)
1x OKI M6295 (sound)
1x oscillator 12.000MHz (close to main)
1x oscillator 8.000MHz (close to Z80 and Oki)
1x oscillator 14.31818 (far from everything)

ROMs
1x 27C040 (21)
1x 27C1000 (ic18)
1x 27C256 (22)
8x 27C020 (23-30)

Note
1x JAMMA edge connector
1x trimmer (volume)
2x 8 switches dip

*/

ROM_START( jumpkids )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "23-ic29.15c", 0x00000, 0x40000, CRC(6ba11e91) SHA1(9f83ef79beb97af1625e7b46858d6f0681dafb23) )
	ROM_LOAD16_BYTE( "24-ic30.17c", 0x00001, 0x40000, CRC(5795d98b) SHA1(d1435f0b79a4fa45770c56b91f078c1885fbd048) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "22-ic19.3c", 0x00000, 0x08000, CRC(bd619530) SHA1(b4c050012b0f1c31877b3d489a68389be93cc82c) )

	ROM_REGION( 0x80000, "tilegfx", 0 ) /* GFX */
	ROM_LOAD16_BYTE( "30-ic125.15j", 0x00000, 0x40000, CRC(44b9a089) SHA1(b6f99b0b597d540b375616dad4354fc9dbb75a21) )
	ROM_LOAD16_BYTE( "29-ic124.13j", 0x00001, 0x40000, CRC(3f98ec69) SHA1(f09a62d9bd7ab7681436a1f2f450565573927165) )

	ROM_REGION( 0x100000, "sprgfx", 0 ) /* GFX */
	ROM_LOAD16_BYTE( "25-ic69.1g",  0x00000, 0x40000, CRC(176ae857) SHA1(e3178d2a15452a36eb94caf5e5ff3a561783a5f4) )
	ROM_LOAD16_BYTE( "28-ic131.1l", 0x00001, 0x40000, CRC(ed837757) SHA1(27a35e47e1b627270f4b0e4319ec330a6cad5ed1)  )
	ROM_LOAD16_BYTE( "26-ic70.2g",  0x80000, 0x40000, CRC(e8b34980) SHA1(edbf5517c6c9c9c3344d11eabb4a58da87386725) )
	ROM_LOAD16_BYTE( "27-ic100.1j", 0x80001, 0x40000, CRC(3918dda3) SHA1(9409b5a5dc4c44c1ddcb77278541d012b5d8e052) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* More Samples */
	ROM_LOAD( "21-ic17.1c", 0x00000, 0x80000, CRC(e5094f75) SHA1(578f32d4e4212c6cfdef186c2a6dc1d9408e8dfc) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ic18.2c", 0x00000, 0x20000, CRC(a63736c3) SHA1(fca413c04026ecb60a6025a117fea2b5404ac058) )
ROM_END

ROM_START( fncywld )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "01_fw02.bin", 0x000000, 0x080000, CRC(ecb978c1) SHA1(68fbf93a81875f744c6f9820dc4c7d88e912e0a0) )
	ROM_LOAD16_BYTE( "02_fw03.bin", 0x000001, 0x080000, CRC(2d233b42) SHA1(aebeb5d3e06e73d14f713f201b25466bcac97a68) )

	ROM_REGION( 0x100000, "sprgfx", 0  )
	ROM_LOAD16_BYTE( "05_fw06.bin",  0x00000, 0x40000, CRC(e141ecdc) SHA1(fd656ceb2baccefadfa1e9f6932b1e0f0ec0a189) )
	ROM_LOAD16_BYTE( "06_fw07.bin",  0x00001, 0x40000, CRC(0058a812) SHA1(fc6101a11af63536d0a345c820bcd234bb4ce91a) )
	ROM_LOAD16_BYTE( "03_fw04.bin",  0x80000, 0x40000, CRC(6ad38c14) SHA1(a9951432c2ec5e07ed2ee5faac3f2558242438f2) )
	ROM_LOAD16_BYTE( "04_fw05.bin",  0x80001, 0x40000, CRC(b8d079a6) SHA1(8ad63fba26f7588a9764a0585c159fb57cb8c7ed) )

	ROM_REGION( 0x100000, "tilegfx", 0 )
	ROM_LOAD16_BYTE( "08_fw09.bin", 0x00000, 0x40000, CRC(a4a00de9) SHA1(65f03a65569f70fb6f3a0fc7caf038bb44a7f503) )
	ROM_LOAD16_BYTE( "07_fw08.bin", 0x00001, 0x40000, CRC(b48cd1d4) SHA1(a95eeba38ae1ce0a2086edb767f636a9cdbd0176) )
	ROM_LOAD16_BYTE( "10_fw11.bin", 0x80000, 0x40000, CRC(f21bab48) SHA1(84371b31487ca5abcbf57152a64f384959d19209) )
	ROM_LOAD16_BYTE( "09_fw10.bin", 0x80001, 0x40000, CRC(6aea8e0f) SHA1(91e2eeef001351c73b1bfbc1a7840e37d3f89900) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "00_fw01.bin", 0x000000, 0x040000, CRC(b395fe01) SHA1(ac7f2e21413658f8d2a1abf3a76b7817a4e050c9) )
ROM_END

/*

Magic Purple, (c) 1996 Unico

+---------------------------------------------+
|        unico_1     6116          unico_4    |
|             M6295  6116                     |
| YM3012     YM2151  6116          unico_5    |
|         4.000MHz   6116                     |
| 28.6363MHz         +--------+ GAL-D         |
|                    | Actel  |               |
|J                   | A1020B | GAL-E   GAL-G |
|A                   +--------+         GAL-H |
|M                        2018          GAL-I |
|M                        2018                |
|A          76C28         +--------+          |
|           76C28         | Actel  | GAL-F    |
|        62256 62256      | A1020B |          |
| DSWA unico_2 unico_3    +--------+          |
| DSWB   MC68000P12          6264    unico_6  |
|    GAL-A GAL-B             6264    unico_7  |
|          GAL-C 12.00MHz                     |
+---------------------------------------------+

  CPU: MC68000P12
Sound: YM2151+YM3012 & OKI M6295 (bagded as KA51+BS902 & AD-65)
  OSC: 28.6363MHz, 12.000MHz, 4.000MHz
Other: Actel A1020B PL84C
       8-position dipswitch x 2
RAM: HY62256A LP-70 x 2 - 32K x 8 SRAM
     HY6264ALP-10 x 2 - 8K x 8 SRAM
     SYC6116L-45P x 4 - 2K x 8 RAM
     MCM2018N45 x 2 - 2K x 8 SRAM
     GM76C28K-10 x 2 - 2K x 8 SRAM
GAL-A through GAL-I misc undumped GALs

Although not currently measured, clocks "should" be:
 MC68000P12 - 12.00MHz
     YM2151 - 4.00MHz
  OKI M6295 - 1.000MHz  (4.000MHz OSC / 4)

*/
ROM_START( magipur )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "unico_2-27c040.bin", 0x000000, 0x080000, CRC(135c5de7) SHA1(95c75e9e69793f67df9378391ae45915ef9bbb89) )
	ROM_LOAD16_BYTE( "unico_3-27c040.bin", 0x000001, 0x080000, CRC(ee4b16da) SHA1(82391ed4d21d3944ca482be00ab7c0838cf190ff) )

	ROM_REGION( 0x100000, "sprgfx", 0  )
	ROM_LOAD16_BYTE( "unico_4-27c040.bin",  0x80000, 0x40000, CRC(e460a77d) SHA1(bde15705750e002bd576098700161b0944984401) )
	ROM_CONTINUE(0x80001, 0x40000)
	ROM_LOAD16_BYTE( "unico_5-27c040.bin",  0x00000, 0x40000, CRC(79c53627) SHA1(9e2673b3becf0508f630f3bd8ff5fc30520b120b) )
	ROM_CONTINUE(0x00001, 0x40000)

	ROM_REGION( 0x100000, "tilegfx", 0 )
	ROM_LOAD16_BYTE( "unico_6-27c040.bin", 0x00001, 0x40000, CRC(b25b5872) SHA1(88a6a110073060c3b7b2987cc41d23c4ca412b43) )
	ROM_CONTINUE(0x00000, 0x40000)
	ROM_LOAD16_BYTE( "unico_7-27c040.bin", 0x80001, 0x40000, CRC(d3c3a672) SHA1(5bbd67a953e1d47d05006a4ef4aa7a23e807f11b) )
	ROM_CONTINUE(0x80000, 0x40000)

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "unico_1-27c020.bin", 0x000000, 0x040000, CRC(84dcf771) SHA1(f8a693a11b14608a582a90b7fd7d3be92e46a0e1) )
ROM_END

ROM_START( suprtrio )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "rom2",  0x00000, 0x40000, CRC(4102e59d) SHA1(f06f1273dbbb91fa61d84541aa124d9c88ee94c1) )
	ROM_LOAD16_BYTE( "rom1",  0x00001, 0x40000, CRC(cc3a83c3) SHA1(6f8b1b6b666ce11c02e9defcba751d88621e572d) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 */
	ROM_LOAD( "rom4l", 0x000000, 0x10000, CRC(466aa96d) SHA1(37f1ba148dbad27ed8e71a0b3434ff970fcb519f) )

	ROM_REGION( 0x100000, "tilegfx", 0 ) /* bg tiles */
	ROM_LOAD( "rom4",  0x00000, 0x20000, CRC(cd2dfae4) SHA1(1d872b5abaf72d34bd4a45f6be69aa6474887b4b) )
	ROM_CONTINUE(      0x40000, 0x20000 )
	ROM_CONTINUE(      0x20000, 0x20000 )
	ROM_CONTINUE(      0x60000, 0x20000 )
	ROM_LOAD( "rom5",  0x80000, 0x20000, CRC(4e64da64) SHA1(f2518b3d83d7fd46000ca982b2d91ce75034b411) )
	ROM_CONTINUE(      0xc0000, 0x20000 )
	ROM_CONTINUE(      0xa0000, 0x20000 )
	ROM_CONTINUE(      0xe0000, 0x20000 )

	ROM_REGION( 0x100000, "sprgfx", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "rom9l", 0x00000, 0x40000, CRC(cc45f437) SHA1(fa735c3b3f96266ddfb611af6908abe72d5ae9d9) )
	ROM_LOAD16_BYTE( "rom8l", 0x00001, 0x40000, CRC(9bc90169) SHA1(3bc0d34911f063ff79c529346f41695376428f75) )
	ROM_LOAD16_BYTE( "rom7l", 0x80000, 0x40000, CRC(bfc7c756) SHA1(e533f633dec63c27ac78f170e222e590e815a022) )
	ROM_LOAD16_BYTE( "rom6l", 0x80001, 0x40000, CRC(bb3499af) SHA1(1a0a6a63227e8ad28aa23afc6d076037518b4802) )

	ROM_REGION( 0xb0000, "oki", 0 ) /* samples */
	ROM_LOAD( "rom3h", 0x00000, 0x30000, CRC(34ea7ec9) SHA1(1f80a2c7ed4fb13610731732b11268d1d7be5bb2) )
	ROM_CONTINUE(      0x40000, 0x50000 )
	ROM_LOAD( "rom3l", 0x90000, 0x20000, CRC(1b73233b) SHA1(5d82bbdc31d99f8d77bdb5c2f6e5e23037b4bca0) )
ROM_END

/*

1x MC68000P10 (main)
1x PIC16C57
1x OKI M6295
1x oscillator 8.000MHz (close to OKI)
1x oscillator 14.000MHz
1x oscillator 12.000000MHz

*/

ROM_START( pangpang )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE ("2.bin", 0x00000, 0x40000, CRC(45436666) SHA1(a319d27320d74266b5e5af7eb1452ecc0b158318) )
	ROM_LOAD16_BYTE( "3.bin", 0x00001, 0x40000, CRC(2725cbe7) SHA1(3ce2d8b1460a26ac0d982103d8796cdc296a64e1) )

	ROM_REGION( 0x2d4c, "cpu1", 0 ) /* PIC16c57 */
	ROM_LOAD( "pic_16c57", 0x00000, 0x2d4c, BAD_DUMP CRC(1ca515b4) SHA1(b2d302a7e45ac5b783d408584b93b534eaee6523) ) // protected :-(

	ROM_REGION( 0x100000, "tilegfx", 0 ) // PF1 tilemap
	ROM_LOAD16_BYTE( "11.bin", 0x00000, 0x20000, CRC(a2b9fec8) SHA1(121771466c288e132cdcf6abdc3bbe2578de9260) )
	ROM_CONTINUE(0x80000,0x20000)
	ROM_LOAD16_BYTE( "10.bin", 0x00001, 0x20000, CRC(4f59d7b9) SHA1(a0eabb44ecb6922f656a5032c0ab757813b9cc13) )
	ROM_CONTINUE(0x80001,0x20000)
	ROM_LOAD16_BYTE( "6.bin", 0x40000, 0x20000, CRC(1ebbc4f1) SHA1(6fb745ebe7ee8ecf5036ac0c4a5dda71cbb40063) )
	ROM_CONTINUE(0xc0000,0x20000)
	ROM_LOAD16_BYTE( "7.bin", 0x40001, 0x20000, CRC(cd544173) SHA1(b929d771040a48356b449458d3125142b9bfc365) )
	ROM_CONTINUE(0xc0001,0x20000)

	ROM_REGION( 0x100000, "sprgfx", 0 )
	ROM_LOAD16_BYTE( "8.bin",   0x00000, 0x40000, CRC(ea0fa1e0) SHA1(1f2f6264097d15339782c2e399d125c3835fd852) )
	ROM_LOAD16_BYTE( "9.bin",   0x00001, 0x40000, CRC(1da5fe49) SHA1(338be1a9f8c42e685e1cefb12b2d169b7560e5f7) )
	ROM_LOAD16_BYTE( "4.bin",   0x80000, 0x40000, CRC(4f282eb1) SHA1(3731045a500082d37588edf7cbb0c0ebae566aab) )
	ROM_LOAD16_BYTE( "5.bin",   0x80001, 0x40000, CRC(00694df9) SHA1(f07373c7ef379daa4e788c169579e23a1133d884) )

	ROM_REGION( 0x100000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(e722bb02) SHA1(ebb8c87d32dccbebf6d8a47703ac12be984f4a3d) )
	ROM_RELOAD(0x80000,0x80000)
ROM_END


/**************************************

            SemiCom Games

 Uses similar hardware with 87c52 MCU for protection

 SemiCom used the name "Mijin" in 1995


**************************************/


/*

Metal Saver By First Amusement (1994)

The pcb is identical to Final Tetris pcb
 (strange, final tetris is based on snowbros logic)

1x 68k
1x z80
1x CA101 (compatible to YM2151)
1x Oki 6295
1x OSC 14mhz (near 68k)
1x OSC 3.579545 (near z80)
1x FPGA Actel 1020A PL84C
1x 87c52 based MCU (very similar to tyical SemiCom mcu)
2x banks of Dipswitch

First Amusement logo is almost identical to SemiCom logo.

There is a PC version of Metal Saver, so did SemiCom do
the arcade version on SemiCom type hardware?

*/

ROM_START( metlsavr )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "first-3.ub18", 0x00001, 0x40000, CRC(87bf4ed2) SHA1(ee1a23232bc37d95dca6d612b4e22ed2b723bd01) )
	ROM_LOAD16_BYTE( "first-4.ub17", 0x00000, 0x40000, CRC(667a494d) SHA1(282391ed7fa994ec51d39c6b086a808ee43e8af1) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "first-2.ua7", 0x00000, 0x10000, CRC(49505edf) SHA1(ea3007f1adbe8e2597ee6201bbd5d07fa9f7c733) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(17aa17a9) SHA1(5b83159c62473f79e7fced0d86acfaf697ad5537) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "first-1.uc1", 0x00000, 0x40000, CRC(e943dacb) SHA1(65a786467fc9efe503aad4e183df352e52143fc2) )

	ROM_REGION( 0x80000, "tilegfx", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "first-5.rom5", 0x00001, 0x40000, CRC(dd4af746) SHA1(185a8080173b3c05fcc5f5ee2f71606987826e79) )
	ROM_LOAD16_BYTE( "first-6.rom6", 0x00000, 0x40000, CRC(808b0e0b) SHA1(f4913e135986b28b4e56bdcc4fd7dd5aad9aa467) )

	ROM_REGION( 0x200000, "sprgfx", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "first-7.uor1",  0x000000, 0x80000, CRC(a6816747) SHA1(0ec288a1e23bb78de0e284b759a5e83304744960) )
	ROM_LOAD16_BYTE( "first-8.uor2",  0x000001, 0x80000, CRC(377020e5) SHA1(490dd2383a49554f2c5d65df798a3933f5c5a62e) )
	ROM_LOAD16_BYTE( "first-9.uor3",  0x100000, 0x80000, CRC(fccf1bb7) SHA1(12cb397fd6438068558ec4d64298cfbe4f9e0e7e) )
	ROM_LOAD16_BYTE( "first-10.uor4", 0x100001, 0x80000, CRC(a22b587b) SHA1(e2f6785eb17f66a8b4fc102524b5013e72f1a0f3) )
ROM_END

/* BC Story */

ROM_START( bcstry )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "bcstry_u.35",  0x40001, 0x20000, CRC(d25b80a4) SHA1(6ea1c28cf508b856e93a06063e634a09291cb32c) )
	ROM_CONTINUE ( 0x00001, 0x20000)
	ROM_LOAD16_BYTE( "bcstry_u.62",  0x40000, 0x20000, CRC(7f7aa244) SHA1(ee9bb2bf22d16f06d7935168e2bd09296fba3abc) )
	ROM_CONTINUE ( 0x00000, 0x20000)

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "bcstry_u.21", 0x04000, 0x4000 , CRC(3ba072d4) SHA1(8b64d3ab4c63132f2f77b2cf38a88eea1a8f11e0) )
	ROM_CONTINUE( 0x0000, 0x4000 )
	ROM_CONTINUE( 0xc000, 0x4000 )
	ROM_CONTINUE( 0x8000, 0x4000 )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from shared ram, the MCU puts it there */
	/* taken from other set, check... */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(e84e328c) SHA1(ce21988980654acb573bfb7396fd2f536204ecf0) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "bcstry_u.64", 0x00000, 0x40000, CRC(23f0e0fe) SHA1(a8c3cbb6378797db353ca2873e73ff157a6f8a3c) )

	ROM_REGION( 0x200000, "tilegfx", 0 ) /* Tiles */
	ROM_LOAD16_BYTE( "bcstry_u.109", 0x000000, 0x20000, CRC(eb04d37a) SHA1(818dc7aafac577920d94c65e47d965dc0474d92c) ) // tiles a plane 0
	ROM_CONTINUE ( 0x100000,0x20000) // tiles a plane 1
	ROM_CONTINUE ( 0x040000,0x20000) // tiles b plane 0
	ROM_CONTINUE ( 0x140000,0x20000) // tiles b plane 1
	ROM_LOAD16_BYTE( "bcstry_u.113", 0x000001, 0x20000, CRC(746ecdd7) SHA1(afb6dbc0fb94e7ce96a9b219f5f7cd3721d1c1c4) ) // tiles a plane 2
	ROM_CONTINUE ( 0x100001,0x20000) // tiles a plane 3
	ROM_CONTINUE ( 0x040001,0x20000) // tiles b plane 2
	ROM_CONTINUE ( 0x140001,0x20000) // tiles b plane 3
	ROM_LOAD16_BYTE( "bcstry_u.110", 0x080000, 0x20000, CRC(1bfe65c3) SHA1(27dec16b271866ff336d8b25d352977ca80c35bf) ) // tiles c plane 0
	ROM_CONTINUE ( 0x180000,0x20000) // tiles c plane 1
	ROM_CONTINUE ( 0x0c0000,0x20000) // tiles d plane 0
	ROM_CONTINUE ( 0x1c0000,0x20000) // tiles d plane 1
	ROM_LOAD16_BYTE( "bcstry_u.111", 0x080001, 0x20000, CRC(c8bf3a3c) SHA1(604fc57c4d3a581016aa2516236c568488d23c77) ) // tiles c plane 2
	ROM_CONTINUE ( 0x180001,0x20000) // tiles c plane 3
	ROM_CONTINUE ( 0x0c0001,0x20000) // tiles d plane 2
	ROM_CONTINUE ( 0x1c0001,0x20000) // tiles d plane 3

	ROM_REGION( 0x400000, "sprgfx", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "bcstry_u.100", 0x000000, 0x80000, CRC(8c11cbed) SHA1(e04e53af4fe732bf9d20a9ae5c2a90b576ee0b83) ) // b
	ROM_LOAD16_BYTE( "bcstry_u.106", 0x000001, 0x80000, CRC(5219bcbf) SHA1(4b88eab7ffc2dc1de451ae4ee52f1536e179ea13) ) // b
	ROM_LOAD16_BYTE( "bcstry_u.99",  0x100000, 0x80000, CRC(cdb1af87) SHA1(df1fbda5c7ce4fbd64d6db9eb80946e06119f096) ) // a
	ROM_LOAD16_BYTE( "bcstry_u.105", 0x100001, 0x80000, CRC(8166b596) SHA1(cbf6f5cec5f6991bb1d4ec0ea03cd617ff38fc3b) ) // a
	ROM_LOAD16_BYTE( "bcstry_u.104", 0x200000, 0x80000, CRC(377c0c71) SHA1(77efa9530b1c311d93c84dd8452701414f740269) ) // b
	ROM_LOAD16_BYTE( "bcstry_u.108", 0x200001, 0x80000, CRC(442307ed) SHA1(71b7f19af64d9961f0f9205b86b4b0ebc13fddda) ) // b
	ROM_LOAD16_BYTE( "bcstry_u.102", 0x300000, 0x80000, CRC(71b40ece) SHA1(1a13dfd7615a6f61851897ebcb10fa69bc8ae525) ) // a
	ROM_LOAD16_BYTE( "bcstry_u.107", 0x300001, 0x80000, CRC(ab3c923a) SHA1(aaca1d2ed7b53e0933e0bd94a19458dd1598f204) ) // a
ROM_END

ROM_START( bcstrya )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "prg1.ic35",  0x40001, 0x20000, CRC(2c55100a) SHA1(bc98a0015c99ef84ebd3fc3f7b7a3bdfd700e1da) )
	ROM_CONTINUE ( 0x00001, 0x20000)
	ROM_LOAD16_BYTE( "prg2.ic62",  0x40000, 0x20000, CRC(f54c0a96) SHA1(79a3635792a23f47fc914d1d5e118b5a643ca100) )
	ROM_CONTINUE ( 0x00000, 0x20000)

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "bcstry_u.21", 0x04000, 0x4000 , CRC(3ba072d4) SHA1(8b64d3ab4c63132f2f77b2cf38a88eea1a8f11e0) )
	ROM_CONTINUE( 0x0000, 0x4000 )
	ROM_CONTINUE( 0xc000, 0x4000 )
	ROM_CONTINUE( 0x8000, 0x4000 )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(e84e328c) SHA1(ce21988980654acb573bfb7396fd2f536204ecf0) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "bcstry_u.64", 0x00000, 0x40000, CRC(23f0e0fe) SHA1(a8c3cbb6378797db353ca2873e73ff157a6f8a3c) )

	ROM_REGION( 0x200000, "tilegfx", 0 ) /* Tiles */
	ROM_LOAD16_BYTE( "bcstry_u.109", 0x000000, 0x20000, CRC(eb04d37a) SHA1(818dc7aafac577920d94c65e47d965dc0474d92c) ) // tiles a plane 0
	ROM_CONTINUE ( 0x100000,0x20000) // tiles a plane 1
	ROM_CONTINUE ( 0x040000,0x20000) // tiles b plane 0
	ROM_CONTINUE ( 0x140000,0x20000) // tiles b plane 1
	ROM_LOAD16_BYTE( "bcstry_u.113", 0x000001, 0x20000, CRC(746ecdd7) SHA1(afb6dbc0fb94e7ce96a9b219f5f7cd3721d1c1c4) ) // tiles a plane 2
	ROM_CONTINUE ( 0x100001,0x20000) // tiles a plane 3
	ROM_CONTINUE ( 0x040001,0x20000) // tiles b plane 2
	ROM_CONTINUE ( 0x140001,0x20000) // tiles b plane 3
	ROM_LOAD16_BYTE( "bcstry_u.110", 0x080000, 0x20000, CRC(1bfe65c3) SHA1(27dec16b271866ff336d8b25d352977ca80c35bf) ) // tiles c plane 0
	ROM_CONTINUE ( 0x180000,0x20000) // tiles c plane 1
	ROM_CONTINUE ( 0x0c0000,0x20000) // tiles d plane 0
	ROM_CONTINUE ( 0x1c0000,0x20000) // tiles d plane 1
	ROM_LOAD16_BYTE( "bcstry_u.111", 0x080001, 0x20000, CRC(c8bf3a3c) SHA1(604fc57c4d3a581016aa2516236c568488d23c77) ) // tiles c plane 2
	ROM_CONTINUE ( 0x180001,0x20000) // tiles c plane 3
	ROM_CONTINUE ( 0x0c0001,0x20000) // tiles d plane 2
	ROM_CONTINUE ( 0x1c0001,0x20000) // tiles d plane 3

	ROM_REGION( 0x400000, "sprgfx", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "bcstry_u.100", 0x000000, 0x80000, CRC(8c11cbed) SHA1(e04e53af4fe732bf9d20a9ae5c2a90b576ee0b83) ) // b
	ROM_LOAD16_BYTE( "bcstry_u.106", 0x000001, 0x80000, CRC(5219bcbf) SHA1(4b88eab7ffc2dc1de451ae4ee52f1536e179ea13) ) // b
	ROM_LOAD16_BYTE( "bcstry_u.99",  0x100000, 0x80000, CRC(cdb1af87) SHA1(df1fbda5c7ce4fbd64d6db9eb80946e06119f096) ) // a
	ROM_LOAD16_BYTE( "bcstry_u.105", 0x100001, 0x80000, CRC(8166b596) SHA1(cbf6f5cec5f6991bb1d4ec0ea03cd617ff38fc3b) ) // a
	ROM_LOAD16_BYTE( "bcstry_u.104", 0x200000, 0x80000, CRC(377c0c71) SHA1(77efa9530b1c311d93c84dd8452701414f740269) ) // b
	ROM_LOAD16_BYTE( "bcstry_u.108", 0x200001, 0x80000, CRC(442307ed) SHA1(71b7f19af64d9961f0f9205b86b4b0ebc13fddda) ) // b
	ROM_LOAD16_BYTE( "bcstry_u.102", 0x300000, 0x80000, CRC(71b40ece) SHA1(1a13dfd7615a6f61851897ebcb10fa69bc8ae525) ) // a
	ROM_LOAD16_BYTE( "bcstry_u.107", 0x300001, 0x80000, CRC(ab3c923a) SHA1(aaca1d2ed7b53e0933e0bd94a19458dd1598f204) ) // a
ROM_END

/*

Hatch Catch
SemiCom, 1995

PCB Layout
----------

|---------------------------------------------|
|       M6295  0.UC1  4.096MHz      PAL  6.OR1|
|YM3016 YM2151 6116                      7.OR2|
|uPC1241H  PAL 1.UA7                     8.OR3|
|         Z80B        6116               9.OR4|
|                     6116             6116   |
|                    PAL               6116   |
|J                   PAL               PAL    |
|A  PAL                                       |
|M  6116                                   PAL|
|M  6116                                      |
|A  PAL PAL                                   |
|   PAL                        4.M5           |
|DSW1                          5.M6           |
|             15MHz                           |
|DSW2           62256          6264           |
|               62256          6264    ACTEL  |
|        68000  2.B16          PAL     A1020B |
|87C52          3.B17                 (PLCC84)|
|---------------------------------------------|

Notes:
        68k clock: 15MHz
        Z80 clock: 3.42719MHz  <-- should be 3.75
      M6295 clock: 1.024MHz, sample rate = /132
      87C52 clock: 15MHz
     YM2151 clock: 3.42719MHz  <-- should be 3.75
            VSync: 60Hz

Interrupts:

Lev 1 0x64 0000 00c0 <- just reset .. not used
Lev 2 0x68 0000 00c0  ""
Lev 3 0x6c 0000 00c0  ""
Lev 4 0x70 0000 00c0  ""
Lev 5 0x74 0000 00c0  ""
Lev 6 0x78 0012 0000 <- RAM shared with protection device (first 0x200 bytes?)

*/

ROM_START( htchctch )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "p03.b16",  0x00001, 0x20000, CRC(eff14c40) SHA1(8fdda1fb859546c16f940e51f7e126768205154c) )
	ROM_LOAD16_BYTE( "p04.b17",  0x00000, 0x20000, CRC(6991483a) SHA1(c8d868ef1f87655c37f0b1efdbb71cd26918f270) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "p02.b5", 0x00000, 0x10000 , CRC(c5a03186) SHA1(42561ab36e6d7a43828d3094e64bd1229ab893ba) )

	ROM_REGION( 0x2000, "protection", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c51fa.bin", 0x00000, 0x2000, CRC(a30312f3) SHA1(e61a89b4ed9555252fc1a64c50d345085f7674c7) ) // decapped

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "p01.c1", 0x00000, 0x20000, CRC(18c06829) SHA1(46b180319ed33abeaba70d2cc61f17639e59bfdb) )

	ROM_REGION( 0x80000, "tilegfx", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "p06srom5.bin", 0x00001, 0x40000, CRC(3d2cbb0d) SHA1(bc80be594a40989e3c23539fc2021de65a2444c5) )
	ROM_LOAD16_BYTE( "p07srom6.bin", 0x00000, 0x40000, CRC(0207949c) SHA1(84b4dcd27fe89a5350b6642ef99719bb85514174) )

	ROM_REGION( 0x80000, "sprgfx", 0 ) /* GFX */
	ROM_LOAD16_BYTE( "p08uor1.bin",  0x00000, 0x20000, CRC(6811e7b6) SHA1(8157f92a3168ffbac86cd8c6294b9c0f3ee0835d) )
	ROM_LOAD16_BYTE( "p09uor2.bin",  0x00001, 0x20000, CRC(1c6549cf) SHA1(c05aba9b744144db4537e472842b0d53325aa78f) )
	ROM_LOAD16_BYTE( "p10uor3.bin",  0x40000, 0x20000, CRC(6462e6e0) SHA1(0d107214dfb257e15931701bad6b42c6aadd8a18) )
	ROM_LOAD16_BYTE( "p11uor4.bin",  0x40001, 0x20000, CRC(9c511d98) SHA1(6615cbb125bd1e1b4da400ec4c4a0f4df8f6fa75) )
ROM_END

/* Carket Ball */

/*
carket ball


68000P10 Xtal 15Mhz
2*62256
27020.ub17
27020.ub18  prg roms


gfx ram (???) 2x6264
near
27040.srom5
27040.srom6

gfx
27020.uor1
27020.uor2
27020.uor3
27020.uor4

sound Z80 (Xtal 4.096Mhz)
27512.ub5 sound prg
6116 sound ram


27010.uc1 audio data


sound hardware
Z80+6295+YM2151+YM3012

protection??
Intel P8752BH (protected)
*/

ROM_START( carket )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "27020.ub18",  0x00001, 0x20000, CRC(3bedee05) SHA1(0d9ddbe0b34307ac02740f813f89937c3af24c30) )
	ROM_LOAD16_BYTE( "27020.ub17",  0x00000, 0x20000, CRC(b43fb7b6) SHA1(609e2626522bff49c13e253337072fcc6c0b8eae) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "27512.ub5", 0x00000, 0x10000 , CRC(750516fe) SHA1(5025ffc0fa2461047db0d847de863f8b633a0c7c) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "carket_protdata.bin", 0x00000, 0x200 , CRC(5778470a) SHA1(3bf0c90e32a1ecf9b4f0ab90f61f29d59616ffbc) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "27010.uc1", 0x00000, 0x20000, CRC(b825bb9c) SHA1(9e444306e7ac1282871f0132f0137bf7aa87b7e0) )

	ROM_REGION( 0x200000, "tilegfx", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "27040.srom5", 0x00001, 0x20000, CRC(d3e2c243) SHA1(279905c56f7f8eada076c15de67a6f0c571cb317) )
	ROM_CONTINUE ( 0x100001,0x20000)
	ROM_CONTINUE ( 0x040001,0x20000)
	ROM_CONTINUE ( 0x140001,0x20000)
	ROM_LOAD16_BYTE( "27040.srom6", 0x00000, 0x20000, CRC(0291be4c) SHA1(3106e4d2eb3256ce9914e562ac335beb351f79e6) )
	ROM_CONTINUE ( 0x100000,0x20000)
	ROM_CONTINUE ( 0x040000,0x20000)
	ROM_CONTINUE ( 0x140000,0x20000)

	ROM_REGION( 0x100000, "sprgfx", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "27020.uor1",  0x000000, 0x40000, CRC(e6a94756) SHA1(114808ab18dc15dc47126673d0ccc40c7d8b8c20) )
	ROM_LOAD16_BYTE( "27020.uor2",  0x000001, 0x40000, CRC(f7158b76) SHA1(e1e35a88aa18376593389fff0bbe3784b17dccab) )
	ROM_LOAD16_BYTE( "27020.uor3",  0x080000, 0x40000, CRC(9295c315) SHA1(97e6c7550abb0e20ff21d7307a3850a4cfc0985f) )
	ROM_LOAD16_BYTE( "27020.uor4",  0x080001, 0x40000, CRC(333f1ed1) SHA1(0eaa3e11cbbb181106639298dc1a551c9ccb1208) )
ROM_END

/* Cookie & Bibi */

ROM_START( cookbib )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "prg1.ub16",  0x00001, 0x20000, CRC(cda6335f) SHA1(34a57785a458d3e9a66c91734b4511fbc9f3455c) )
	ROM_LOAD16_BYTE( "prg2.ub17",  0x00000, 0x20000, CRC(2664a335) SHA1(8d1c4825720a09db6156599ab905292640b04cba) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "prg-s.ub5", 0x00000, 0x10000 , CRC(547d6ea3) SHA1(42929e453c4f1c90c29197a9bed953139cfe2873) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(a77d13f4) SHA1(13db72f5b171b0c1226e97ea98d9edd7144d56d9) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sound.uc1", 0x00000, 0x20000, CRC(545e19b6) SHA1(ef518bbe44b22e7ef77ee6af337ebcad9b2674e0) )

	ROM_REGION( 0x80000, "tilegfx", 0 ) /* */
	ROM_LOAD16_BYTE( "srom5.bin", 0x00001, 0x40000, CRC(73a46e43) SHA1(054fac2dc5dffcbb9d81600689c07774d2e200b6) )
	ROM_LOAD16_BYTE( "srom6.bin", 0x00000, 0x40000, CRC(ade2dbec) SHA1(12d385d22307d8251e711788dff2e503c8f8ca7c) )

	ROM_REGION( 0x80000, "sprgfx", 0 ) /* GFX */
	ROM_LOAD16_BYTE( "uor1.bin",  0x00000, 0x20000, CRC(a7d91f23) SHA1(eb9694e05b8a04ed1cdbb834e1bf745a2b0260be) )
	ROM_LOAD16_BYTE( "uor2.bin",  0x00001, 0x20000, CRC(9aacbec2) SHA1(c1cfe243a7d51c950785073f235d72cc01724cdb) )
	ROM_LOAD16_BYTE( "uor3.bin",  0x40000, 0x20000, CRC(3fee0c3c) SHA1(c71439ba8033c549e40522db5270caf4a297fb99) )
	ROM_LOAD16_BYTE( "uor4.bin",  0x40001, 0x20000, CRC(bed9ed2d) SHA1(7103b99cd0d54df864ea4a0f269011e30ad29ed7) )
ROM_END

ROM_START( cookbiba )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "d13.u818",  0x00001, 0x20000, CRC(19c75b1f) SHA1(c6c920d7cb1616b828cc8245cd8f7beb72753b7c) )
	ROM_LOAD16_BYTE( "d14.u817",  0x00000, 0x20000, CRC(0021349f) SHA1(ae3dbda2fd0e99ce037e35b87a554fbe28b5fe14) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "d12.ub5", 0x00000, 0x10000 , CRC(0a16e0b4) SHA1(c20e8cf2b4fc6cf22d9588ff4b3e15cac8ebf505) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */ // note, different to cookbib
	/* this is not a real rom but instead the data extracted from shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "cookbiba_protdata.bin", 0x00000, 0x200 , CRC(7f05b832) SHA1(c6141b15ee9e31f7a28748f330405b90c3b9508a) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sound.uc1", 0x00000, 0x20000, CRC(545e19b6) SHA1(ef518bbe44b22e7ef77ee6af337ebcad9b2674e0) )

	ROM_REGION( 0x80000, "tilegfx", 0 ) /* */
	ROM_LOAD16_BYTE( "srom5.bin", 0x00001, 0x40000, CRC(73a46e43) SHA1(054fac2dc5dffcbb9d81600689c07774d2e200b6) )
	ROM_LOAD16_BYTE( "srom6.bin", 0x00000, 0x40000, CRC(ade2dbec) SHA1(12d385d22307d8251e711788dff2e503c8f8ca7c) )

	ROM_REGION( 0x80000, "sprgfx", 0 ) /* GFX */
	ROM_LOAD16_BYTE( "d17.uor1",  0x00000, 0x20000, CRC(2fab7c2d) SHA1(6024d3428cc09905c791a3f394487b8bb484a131) )
	ROM_LOAD16_BYTE( "d18.uor2",  0x00001, 0x20000, CRC(341750a0) SHA1(a218ec0e61591b02bf1a8ba69f2af8d8d29fda8a) )
	ROM_LOAD16_BYTE( "d19.uor3",  0x40000, 0x20000, CRC(343d2e41) SHA1(d1b123bb4c8749a1cda2c4e25eef09f46ecd68aa) )
	ROM_LOAD16_BYTE( "d20.uor4",  0x40001, 0x20000, CRC(c35cc03d) SHA1(cbf51cee2a39e3ea3d421a9e0b2422b1ad306031))
ROM_END


/* Choky Choky */

ROM_START( chokchok )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ub18.bin",  0x00001, 0x40000, CRC(b183852a) SHA1(fd50c6d91dba64b936ac367e5e5235d09ed60fdd) )
	ROM_LOAD16_BYTE( "ub17.bin",  0x00000, 0x40000, CRC(ecdb45ca) SHA1(03eb2d27ae4de25aa15477135d3b4de8b3b7f7f0) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "ub5.bin", 0x00000, 0x10000 , CRC(30c2171d) SHA1(3954e286d57b955af6ba9b1a0b49c442d7f295ae) )

	ROM_REGION( 0x2000, "protection", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "p87c52ebpn.bin", 0x00000, 0x2000, CRC(0d6b4918) SHA1(cfa2c035ef214a4097df12f868b13d4d10f00d0b) ) // decapped

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uc1.bin", 0x00000, 0x40000, CRC(f3f57abd) SHA1(601dc669020ef9156fa221e768be9b88454e3f55) )

	ROM_REGION( 0x200000, "tilegfx", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "srom5.bin", 0x00001, 0x20000, CRC(836608b8) SHA1(7aa624274efee0a7affb6a1a417752b5ce116c04) )
	ROM_CONTINUE ( 0x100001,0x20000)
	ROM_CONTINUE ( 0x040001,0x20000)
	ROM_CONTINUE ( 0x140001,0x20000)
	ROM_LOAD16_BYTE( "srom6.bin", 0x00000, 0x20000, CRC(31d5715d) SHA1(32612464124290b273c4b1a8b571291f9aeff01c) )
	ROM_CONTINUE ( 0x100000,0x20000)
	ROM_CONTINUE ( 0x040000,0x20000)
	ROM_CONTINUE ( 0x140000,0x20000)

	ROM_REGION( 0x200000, "sprgfx", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "uor1.bin",  0x000000, 0x80000, CRC(ded6642a) SHA1(357c836ebe62e0f7f9e7afdf7428f42d827ede06) )
	ROM_LOAD16_BYTE( "uor2.bin",  0x000001, 0x80000, CRC(493f9516) SHA1(2e1d38493558dc79cd4d232ac421cd5649f4119a) )
	ROM_LOAD16_BYTE( "uor3.bin",  0x100000, 0x80000, CRC(e2dc3e12) SHA1(9e2571f93d27b9048fe8e42d3f13a8e509b3adca) )
	ROM_LOAD16_BYTE( "uor4.bin",  0x100001, 0x80000, CRC(6f377530) SHA1(1367987e3af0baa8e22f09d1b40ad838f33371bc) )
ROM_END


/* Date Quiz Go Go */

ROM_START( dquizgo )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ub18",     0x00001, 0x80000, CRC(07f869f2) SHA1(1e69f8a6ce3bcf0feaeb43cc7c0fc3fa324466b2) )
	ROM_LOAD16_BYTE( "ub17",     0x00000, 0x80000, CRC(0b96ab14) SHA1(fc132118eaa938c85d210713320b0a04425f48de) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "ub5",    0x00000, 0x10000, CRC(e40481da) SHA1(1c1fabcb67693235eaa6ff59ae12a35854b5564a) )

	ROM_REGION( 0x2000, "protection", 0 ) // P87C52EBPN MCU, after decapping the die was 87C51RA+
	ROM_LOAD( "87c51rap.bin", 0x00000, 0x2000, CRC(03bc1f83) SHA1(9ccd49edd8e47a04c5dfe4ac163fbd98ea90aea3) ) // decapped

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uc1",    0x00000, 0x40000, CRC(d0f4c4ba) SHA1(669a04a977e98d8a594cc1621cbb9526c9081ec0) )

	ROM_REGION( 0x80000, "tilegfx", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "srom5",     0x00001, 0x40000, CRC(f1cdd21d) SHA1(0bfc09abce40712c4e95f13ad0d4b78684e44630) )
	ROM_IGNORE(0x40000)
	ROM_LOAD16_BYTE( "srom6",     0x00000, 0x40000, CRC(f848939e) SHA1(bf5e62300dd13a37f4715c67a2eec88034a94311) )
	ROM_IGNORE(0x40000)

	ROM_REGION( 0x200000, "sprgfx", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "uor1",     0x000000, 0x80000, CRC(b4912bf6) SHA1(ef827adba58470a201f3c1ecc3286d728a753eff) )
	ROM_LOAD16_BYTE( "uor2",     0x000001, 0x80000, CRC(b011cf93) SHA1(b993df91511ac17d5bf8e688333f2953b87d5be4) )
	ROM_LOAD16_BYTE( "uor3",     0x100000, 0x80000, CRC(d96c3582) SHA1(6b313462fd8985fae60bc59cd9c99c97ab70fdcc) )
	ROM_LOAD16_BYTE( "uor4",     0x100001, 0x80000, CRC(77ff23eb) SHA1(7fc891810591458fd62f3d4b9b4c966662e90403) )
ROM_END

/*

SD Fighters (c) 1996 SemiCom

Game released 1996-09-13

PCB Layout
----------

YMET 961023

+------------------------------------------+
| M6295  UC1                               |
|                              5.uj2 11.uk2|
|       YM3012 YM2151          6.uj3 12.uk3|
|4.096MHZ  Z80  UA7 pLSI1032   7.uj4 13.uk4|
|J                             8.uj5 14.uk5|
|A                                         |
|M                  pLSI1032   9.uj2 15.uk2|
|M                            10.uj2 16.uk2|
|A     68000  UM61256         UM6164       |
|DSW1          U818                        |
|    87C52    UM61256         UM6164       |
|DSW2          U817                        |
| 15.00MHz                                 |
+------------------------------------------+

Motorola MC68000P10
ZiLOG Z0840006PSC
Intel P87C52 MCU

Lattice pLSI 1032 (x2)
Yamaha YM2151 & YM3012 (badged as BS901 & KA12)
OKI M6295 (badged as AD-65)

OSC: 4.096MHz, 15.000MHz

*/

ROM_START( sdfight )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u818", 0xc0001, 0x20000, CRC(a60e5b22) SHA1(eda1a5de881718f78a45720c3ca43a6288a0e65d) )
	ROM_CONTINUE( 0x80001, 0x20000)
	ROM_CONTINUE( 0x40001, 0x20000)
	ROM_CONTINUE( 0x00001, 0x20000)
	ROM_LOAD16_BYTE( "u817", 0xc0000, 0x20000, CRC(9f284f4d) SHA1(f4a471fb09c2fd73692ddaa03083644493256aae) )
	ROM_CONTINUE( 0x80000, 0x20000)
	ROM_CONTINUE( 0x40000, 0x20000)
	ROM_CONTINUE( 0x00000, 0x20000)

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "ua7", 0x00000, 0x10000 , CRC(c3d36da4) SHA1(7290a977bfa9a3d5e0c98a0f589d877e38aa10a1) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", ROMREGION_ERASE00 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(efb8b822) SHA1(139c39771c057ae322d3601f7e0a58b43fa8860a) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uc1", 0x00000, 0x40000, CRC(535cae2c) SHA1(e9d59ab23cbbc0375987ea68e170ddb1cc75cff8) )

	ROM_REGION( 0x200000, "tilegfx", 0 ) /* Tiles */
	ROM_LOAD16_BYTE( "9.ug11", 0x000001, 0x20000, CRC(bf809ccd) SHA1(4d648d7cdeb5ce4a918b8372dbd33c2fbf307dc0) ) // tiles a plane 0
	ROM_CONTINUE ( 0x100001,0x20000) // tiles a plane 1
	ROM_CONTINUE ( 0x040001,0x20000) // tiles b plane 0
	ROM_CONTINUE ( 0x140001,0x20000) // tiles b plane 1
	ROM_LOAD16_BYTE( "10.ug12", 0x000000, 0x20000, CRC(a5a3bfa2) SHA1(9b0d791f80f4cba14b7fab1aa7550784d6c4c4f7) ) // tiles a plane 2
	ROM_CONTINUE ( 0x100000,0x20000) // tiles a plane 3
	ROM_CONTINUE ( 0x040000,0x20000) // tiles b plane 2
	ROM_CONTINUE ( 0x140000,0x20000) // tiles b plane 3
	ROM_LOAD16_BYTE( "15.ui11", 0x080001, 0x20000, CRC(3bc8aa6d) SHA1(a8983957da5e286ec437f2fc83dfabf81fe56ca2) ) // tiles c plane 0
	ROM_CONTINUE ( 0x180001,0x20000) // tiles c plane 1
	ROM_CONTINUE ( 0x0c0001,0x20000) // tiles d plane 0
	ROM_CONTINUE ( 0x1c0001,0x20000) // tiles d plane 1
	ROM_LOAD16_BYTE( "16.ui12", 0x080000, 0x20000, CRC(71e6b78d) SHA1(a676395b2357093c4800d8520df10f7ef17cb3ee) ) // tiles c plane 2
	ROM_CONTINUE ( 0x180000,0x20000) // tiles c plane 3
	ROM_CONTINUE ( 0x0c0000,0x20000) // tiles d plane 2
	ROM_CONTINUE ( 0x1c0000,0x20000) // tiles d plane 3

	ROM_REGION( 0x400000, "sprgfx", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "11.uk2", 0x000000, 0x80000, CRC(d006fadc) SHA1(79014bc0c7909763829ba02d5434d4543b4b80e5) ) // b
	ROM_LOAD16_BYTE( "12.uk3", 0x000001, 0x80000, CRC(2a2f4153) SHA1(d86692ee17ad052fdd8fccded57e3e30012026f6) ) // b
	ROM_LOAD16_BYTE( "5.uj2",  0x100000, 0x80000, CRC(f1246cbf) SHA1(de80a8f0d29ee76e11f38d9982ffcb4fd228153a) ) // b
	ROM_LOAD16_BYTE( "6.uj3",  0x100001, 0x80000, CRC(d346878c) SHA1(93174f6f6cc797323c5e429bf324d4ffe081f072) ) // b
	ROM_LOAD16_BYTE( "13.uk4", 0x200000, 0x80000, CRC(9bc40774) SHA1(b56c57258ec9c07c7efff9c0c632390d2d5ce4e2) ) // a
	ROM_LOAD16_BYTE( "14.uk5", 0x200001, 0x80000, CRC(a1e61674) SHA1(a5a50f479a019b39082429fa3425a95480838f84) ) // a
	ROM_LOAD16_BYTE( "7.uj4",  0x300000, 0x80000, CRC(dbdece8a) SHA1(20199cc915a1f8088372682c054cac69bc3b4918) ) // a
	ROM_LOAD16_BYTE( "8.uj5",  0x300001, 0x80000, CRC(60be7dd1) SHA1(d212dee3acf696cac0843e968a71ec1fb9b16dc9) ) // a
ROM_END



ROM_START( magicbal )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "jb17", 0x00000, 0x40000, CRC(501e64dd) SHA1(938c1b5364d02fe982d490118a86d4ca4b1297f2) )
	ROM_LOAD16_BYTE( "jb18", 0x00001, 0x40000, CRC(84dcdf68) SHA1(83907705c1c45685e1888c187c8136865c43ee0b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "ub5", 0x00000, 0x10000, CRC(48a9e99d) SHA1(d438f86b6cc9f8e145c89bac355a9bd2d634801e) )

	ROM_REGION( 0x10000, "protection", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, NO_DUMP )

	ROM_REGION16_BE( 0x200, "user1", ROMREGION_ERASE00 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200, CRC(fb67d20d) SHA1(63f2862a7ded075d501e21919f211d156bef4fb4) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uc1", 0x00000, 0x40000, CRC(6e4cec27) SHA1(9dd07684502300589e957d1bcde0239880eaada2) )

	ROM_REGION( 0x80000, "tilegfx", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "rom5", 0x00001, 0x40000, CRC(b9561ae0) SHA1(e2fb11df167a984f98eb6d3a1b77e749646da403) )
	ROM_LOAD16_BYTE( "rom6", 0x00000, 0x40000, CRC(b03a19ea) SHA1(66ab219111c53f79104aa9db250e4b2133a29924) )

	ROM_REGION( 0x200000, "sprgfx", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "uor1",  0x000000, 0x80000, CRC(1835ac6f) SHA1(3c0b171c248a98e1facb5f4fe1c94f98a07b7149) )
	ROM_LOAD16_BYTE( "uor2",  0x000001, 0x80000, CRC(c9db161e) SHA1(3b7b45db005a7144e4c6386d917e89096172385e) )
	ROM_LOAD16_BYTE( "uor3",  0x100000, 0x80000, CRC(69f54d5a) SHA1(10685a14304a0966027e729fc55433c05943391c) )
	ROM_LOAD16_BYTE( "uor4",  0x100001, 0x80000, CRC(3736eef4) SHA1(c801fbf743a9cea6a605f716fb22cee1322fa340) )
ROM_END

/*

Wonderleague Star (c) 1995 Mijin  (SemiCom traded under the Mijin name until 1995)

Game released 1995-07-07

PCB Layout
----------

950522

+------------------------------------------+
| M6295  UC1  4.096MHz                     |
|                                     UDR1 |
| YM3012 YM2151  UA7                  UDR2 |
|               Z80                   UDR3 |
|J                                    UDR4 |
|A                                         |
|M                                         |
|M            15.000MHz       SROM5        |
|A DSW1                       SROM6        |
|       68000 HY62256         HY6284A      |
|  DSW2       HY62256         HY6284A      |
|              U818                        |
|    87C52     U817              TCP1020AFN|
+------------------------------------------+

Motorola MC68000P10
ZiLOG Z0840006PSC
Philips P87C52IBPN MCU

TI TPC1020AFN-084C
Yamaha YM2151 & YM3012 (badged as BS901 & BS902)
OKI M6295 (badged AD-65)

OSC: 4.096MHz, 15.000MHz

Second in a series of Baseball games:

1. Magicball Fighting (1994)
2. Wonder League Star (1995)
3. Wonder League 96 (1996)
4. MuHanSeungBu (1997)

*/

ROM_START( wlstar )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "n-3.u818", 0x00001, 0x40000, CRC(f01bc623) SHA1(ef9d32071bd259fad8243ff4622a82062e67c196) )
	ROM_LOAD16_BYTE( "n-4.u817", 0x00000, 0x40000, CRC(fc3e829b) SHA1(736835766f2b534c4ea3081f7a715a09a068a9f6) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "ua7", 0x00000, 0x10000, CRC(90cafa5f) SHA1(2d2ba8e395544e49899cac662d87585592b12040) )

	ROM_REGION( 0x10000, "protection", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, CRC(ab5e2a7e) SHA1(9d3dbbbf0fac12ed82184222a077d81243abb39d) ) /* decapped */

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ua1", 0x00000, 0x40000,  CRC(de217d30) SHA1(5d7a6f82b106dd1185c7dcde193177cc46c4782f) )

	ROM_REGION( 0x200000, "tilegfx", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "5.srom5", 0x00001, 0x20000, CRC(f7f8c859) SHA1(28b21abfaff2b0502459d1e219c4397ca78a1495) )
	ROM_CONTINUE ( 0x100001,0x20000)
	ROM_CONTINUE ( 0x040001,0x20000)
	ROM_CONTINUE ( 0x140001,0x20000)
	ROM_LOAD16_BYTE( "6.srom6", 0x00000, 0x20000, CRC(34ace2a8) SHA1(f4ab3eceaacf65b5417edbb74b719f2993fa53ef) )
	ROM_CONTINUE ( 0x100000,0x20000)
	ROM_CONTINUE ( 0x040000,0x20000)
	ROM_CONTINUE ( 0x140000,0x20000)

	ROM_REGION( 0x200000, "sprgfx", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "7.udr1",  0x000000, 0x80000, CRC(6e47c31d) SHA1(c9c2d798197e6fc16d7750391c13506a87f8a49b) )
	ROM_LOAD16_BYTE( "8.udr2",  0x000001, 0x80000, CRC(09c5d57c) SHA1(0c53b90be28636008fa3f590b6a851022316f2e8) )
	ROM_LOAD16_BYTE( "9.udr3",  0x100000, 0x80000, CRC(3ec064f0) SHA1(642c49acfe8388717666d423ae94789eb61105a6) )
	ROM_LOAD16_BYTE( "10.udr4", 0x100001, 0x80000, CRC(b4693cdd) SHA1(031290586321b95436b30a3339a10dd8df12b245) )
ROM_END

/* Wonderleague '96 */

ROM_START( wondl96 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ub17.bin", 0x00000, 0x40000, CRC(41d8e03c) SHA1(a3ff92ac2ef9b829cf89ceff606f8dd913025bef) )
	ROM_LOAD16_BYTE( "ub18.bin", 0x00001, 0x40000, CRC(0e4963af) SHA1(6c0607541ca0e5dd8aa4f8138d71150b1ab066cd) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "ub5.bin", 0x00000, 0x10000, CRC(d99d19c4) SHA1(a7fae11275bb156cdbf2805fcc3aec44892d0817) )

	ROM_REGION( 0x10000, "protection", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, CRC(6f4c659a) SHA1(7a1453531d9ceb37af21b96becfa9b06bff0a528) ) /* decapped */

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uc1.bin", 0x00000, 0x40000,  CRC(0e7913e6) SHA1(9a44bd7ca4030627a26010583216ce1c8032ee1b) )

	ROM_REGION( 0x200000, "tilegfx", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "srom5.bin", 0x00001, 0x20000, CRC(db8010c3) SHA1(db43d894d545a72e8da16555c54dcdbd89d87e3d) )
	ROM_CONTINUE ( 0x100001,0x20000)
	ROM_CONTINUE ( 0x040001,0x20000)
	ROM_CONTINUE ( 0x140001,0x20000)
	ROM_LOAD16_BYTE( "srom6.bin", 0x00000, 0x20000, CRC(2f364e54) SHA1(b0d3581567bd46565e9422f6bfb11273fc760fda) )
	ROM_CONTINUE ( 0x100000,0x20000)
	ROM_CONTINUE ( 0x040000,0x20000)
	ROM_CONTINUE ( 0x140000,0x20000)

	ROM_REGION( 0x200000, "sprgfx", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "uor1.bin",  0x000000, 0x80000, CRC(e1e9eebb) SHA1(c92ca988a988c6c5f574654f263e239c2aea9f32) )
	ROM_LOAD16_BYTE( "uor2.bin",  0x000001, 0x80000, CRC(ddebfe83) SHA1(94e7ed19b9fb87fd7733b832d668449ab0442587) )
	ROM_LOAD16_BYTE( "uor3.bin",  0x100000, 0x80000, CRC(7efe4d67) SHA1(b96d42cbb9c62502aac6aad9122b44c165149707) )
	ROM_LOAD16_BYTE( "uor4.bin",  0x100001, 0x80000, CRC(7b1596d1) SHA1(2fab7d6c82318aa335a82af50835139f61e04a3d) )
ROM_END


/*

SemiCom Baseball (MuHanSeungBu)

PCB Layout
----------

+----------------------------------+
|       65728 4.096MHz M6295  ic99 |
|         ic2                 ic100|
|  YM3012 Z80          ic64   ic102|
|  YM2151        5814         ic104|
|J               5814         ic105|
|A               6264  QL2007 ic106|
|M      65728    6264         ic107|
|M      65728                 ic108|
|A DSW1                       ic109|
|        87c52        65728   ic110|
|  DSW2               65728   ic111|
|          ic35  ic62 68000   ic113|
|         62256 62256 15MHz        |
+----------------------------------+

Motorola MC68HC000P10
Goldstar Z8400B PS
Philips P87C52EBPN MCU

QuickLogic QL2007-XPQ208C
Yamaha YM2151 & YM3012 (badged as BS901 & BS902)
OKI M6295 (badged AD-65)

OSC: 4.096MHz, 15.000MHz

Ram:
 UM62256E-70LL x 2 (by 68000)
 HY6264A LP-70 x 2 (by QuickLogic QL2007)
 Temic HM3-65728BK-5 (by Z80)
 Temic HM3-65728BK-5 x 2
 Sony CXK5814P-35L x 2

No roms had any labels, the IC position and
logical use (IE:z80 for the Z80 program rom)

Fourth in a series of Baseball games:

1. Magicball Fighting (1994)
2. Wonder League Star (1995)
3. Wonder League 96 (1996)
4. MuHanSeungBu (1997)

*/

ROM_START( semibase )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ic35.68k",  0x40001, 0x20000, CRC(d2249605) SHA1(ab3faa832f14f799e4a975673495d30160c6eae5) )
	ROM_CONTINUE ( 0x00001, 0x20000)
	ROM_LOAD16_BYTE( "ic62.68k",  0x40000, 0x20000, CRC(85ea81c3) SHA1(7e97316f5f373b98fa4063acd74f784b312a1cc4) )
	ROM_CONTINUE ( 0x00000, 0x20000)

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "ic21.z80", 0x04000, 0x4000 , CRC(d95c64d0) SHA1(1b239e8b23b820610dbf67cbd525d4a6c956ba35) )
	ROM_CONTINUE( 0x0000, 0x4000 )
	ROM_CONTINUE( 0xc000, 0x4000 )
	ROM_CONTINUE( 0x8000, 0x4000 )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from shared ram, the MCU puts it there */
	/* once the game has decrypted this it's almost identical to bcstory with several ram addresses being 0x4 higher than in bcstory
	 and 1200FE: andi.b  #$f, D1  instead of #$3  (unless bcstory data is wrong?) */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(ecbf2163) SHA1(634b366a8c4ba8699851861bf935b55850f93a7f) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ic64.snd", 0x00000, 0x40000, CRC(8a60649c) SHA1(aeb266436f6af4173b84dbb19362563b6c5db507) )

	ROM_REGION( 0x200000, "tilegfx", 0 ) /* Tiles */
	ROM_LOAD16_BYTE( "ic109.gfx", 0x000000, 0x20000, CRC(2b86e983) SHA1(f625da05d68c78173e346f9c60ab4b0672b9f357) ) // tiles a plane 0
	ROM_CONTINUE ( 0x100000,0x20000) // tiles a plane 1
	ROM_CONTINUE ( 0x040000,0x20000) // tiles b plane 0
	ROM_CONTINUE ( 0x140000,0x20000) // tiles b plane 1
	ROM_LOAD16_BYTE( "ic113.gfx", 0x000001, 0x20000, CRC(e39b6610) SHA1(604f876f0bf9ed70f627944397e78ee16869d0ba) ) // tiles a plane 2
	ROM_CONTINUE ( 0x100001,0x20000) // tiles a plane 3
	ROM_CONTINUE ( 0x040001,0x20000) // tiles b plane 2
	ROM_CONTINUE ( 0x140001,0x20000) // tiles b plane 3
	ROM_LOAD16_BYTE( "ic110.gfx", 0x080000, 0x20000, CRC(bba4a015) SHA1(4e03585ff493148b9eeaaabb8d37630962ab6c74) ) // tiles c plane 0
	ROM_CONTINUE ( 0x180000,0x20000) // tiles c plane 1
	ROM_CONTINUE ( 0x0c0000,0x20000) // tiles d plane 0
	ROM_CONTINUE ( 0x1c0000,0x20000) // tiles d plane 1
	ROM_LOAD16_BYTE( "ic111.gfx", 0x080001, 0x20000, CRC(61133b63) SHA1(8820c88297fbcf5e1102c01245391f49a9c63186) ) // tiles c plane 2
	ROM_CONTINUE ( 0x180001,0x20000) // tiles c plane 3
	ROM_CONTINUE ( 0x0c0001,0x20000) // tiles d plane 2
	ROM_CONTINUE ( 0x1c0001,0x20000) // tiles d plane 3

	ROM_REGION( 0x400000, "sprgfx", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "ic100.gfx", 0x000000, 0x80000, CRC(01c3d12a) SHA1(128c21b18f73445a8e77fe5dd3072c1b1e20c47a) ) // b
	ROM_LOAD16_BYTE( "ic106.gfx", 0x000001, 0x80000, CRC(db282ac2) SHA1(127637967e7620cd7e81aff268fb776d0211e58a) ) // b
	ROM_LOAD16_BYTE( "ic99.gfx",  0x100000, 0x80000, CRC(349df821) SHA1(34af8b748aad5807300f8e76eb8a99366878004b) ) // a
	ROM_LOAD16_BYTE( "ic105.gfx", 0x100001, 0x80000, CRC(f7caa81c) SHA1(2270d133c7b116d66581fc688086dd331b811478) ) // a
	ROM_LOAD16_BYTE( "ic104.gfx", 0x200000, 0x80000, CRC(51a5d38a) SHA1(0258ae29779f7f1246845622a579d37dca64fb2f) ) // b
	ROM_LOAD16_BYTE( "ic108.gfx", 0x200001, 0x80000, CRC(b253d60e) SHA1(aca2f6c2233372841908377407068c5d45f5f9c4) ) // b
	ROM_LOAD16_BYTE( "ic102.gfx", 0x300000, 0x80000, CRC(3caefe97) SHA1(e60c6ef9e1dd6abdd763648dbcebefa4f19364c4) ) // a
	ROM_LOAD16_BYTE( "ic107.gfx", 0x300001, 0x80000, CRC(68109898) SHA1(dbc0d431da33e22b8d0f918b9c8a3c1667bc4f8e) ) // a
ROM_END


/******************************************************************************/

#if TUMBLEP_HACK
void tumbleb_state::tumblepb_patch_code(uint16_t offset)
{
	/* A hack which enables all Dip Switches effects */
	uint16_t *RAM = (uint16_t *)memregion("maincpu")->base();
	RAM[(offset + 0)/2] = 0x0240;
	RAM[(offset + 2)/2] = 0xffff;   // andi.w  #$f3ff, D0
}
#endif


void tumbleb_state::tumblepb_gfx_rearrange(int rgn)
{
	uint8_t* rom;
	int len;

	if (rgn == 1)
	{
		rom = memregion("tilegfx")->base();
		len = memregion("tilegfx")->bytes();
	}
	else
	{
		rom = memregion("sprgfx")->base();
		len = memregion("sprgfx")->bytes();
	}

	int i;

	/* gfx data is in the wrong order */
	for (i = 0; i < len; i++)
	{
		if ((i & 0x20) == 0)
		{
			int t = rom[i]; rom[i] = rom[i + 0x20]; rom[i + 0x20] = t;
		}
	}
	/* low/high half are also swapped */
	for (i = 0; i < len/2; i++)
	{
		int t = rom[i]; rom[i] = rom[i + len/2]; rom[i + len/2] = t;
	}
}

void tumbleb_state::init_tumblepb()
{
	tumblepb_gfx_rearrange(1);

	#if TUMBLEP_HACK
	tumblepb_patch_code(0x000132);
	#endif
}

void tumbleb_state::init_tumblepba()
{
	// rearrange the bg data instead of the sprite data on this one!
	tumblepb_gfx_rearrange(2);
}

void tumbleb_state::init_tumbleb2()
{
	tumblepb_gfx_rearrange(1);

	#if TUMBLEP_HACK
	tumblepb_patch_code(0x000132);
	#endif
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x100000, 0x100001, write16smo_delegate(*this, FUNC(tumbleb_state::tumbleb2_soundmcu_w)));

}

void tumbleb_state::init_jumpkids()
{
	tumblepb_gfx_rearrange(1);

	#if TUMBLEP_HACK
	tumblepb_patch_code(0x00013a);
	#endif
}

void tumbleb_state::init_fncywld()
{
	tumblepb_gfx_rearrange(1);

	/* This is a hack to allow you to use the extra features
	   of the 2 first "Unused" Dip Switch (see notes above). */
	//uint16_t *RAM = (uint16_t *)memregion("maincpu")->base();
	//RAM[0x0005fa/2] = 0x4e71;
	//RAM[0x00060a/2] = 0x4e71;
}

void tumbleb_state::init_magipur()
{
	tumblepb_gfx_rearrange(1);

	uint16_t *src = (uint16_t*)memregion( "maincpu" )->base();
	// copy vector table? game expects RAM at 0, and ROM at f00000?!
	memcpy(m_mainram, src, 0x80);
}


uint16_t tumbleb_state::bcstory_1a0_read()
{
	//osd_printf_debug("bcstory_io %06x\n",m_maincpu->pc());

	if (m_maincpu->pc()==0x0560) return 0x1a0;
	else return ioport("SYSTEM")->read();
}

void tumbleb_state::init_bcstory()
{
	tumblepb_gfx_rearrange(1);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x180008, 0x180009, read16smo_delegate(*this, FUNC(tumbleb_state::bcstory_1a0_read))); // io should be here??
}


void tumbleb_state::init_htchctch()
{
	if (memregion("user1") != nullptr)
	{
		uint16_t *PROTDATA = (uint16_t*)memregion("user1")->base();
		int len = memregion("user1")->bytes();
		/* simulate RAM initialization done by the protection MCU */

		for (int i = 0; i < len / 2; i++)
			m_mainram[0x000/2 + i] = PROTDATA[i];
	}

	tumblepb_gfx_rearrange(1);

}


void tumbleb_state::suprtrio_decrypt_code()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();
	std::vector<uint16_t> buf(0x80000/2);
	int i;

	/* decrypt main ROMs */
	memcpy(&buf[0], rom, 0x80000);
	for (i = 0; i < 0x40000; i++)
	{
		int j = i ^ 0x06;
		if ((i & 1) == 0) j ^= 0x02;
		if ((i & 3) == 0) j ^= 0x08;
		rom[i] = buf[j];
	}
}

void tumbleb_state::suprtrio_decrypt_gfx()
{
	uint16_t *rom = (uint16_t *)memregion("tilegfx")->base();
	std::vector<uint16_t> buf(0x100000/2);
	int i;

	/* decrypt tiles */
	memcpy(&buf[0], rom, 0x100000);
	for (i = 0; i < 0x80000; i++)
	{
		int j = i ^ 0x02;
		if (i & 1) j ^= 0x04;
		rom[i] = buf[j];
	}
}

void tumbleb_state::init_suprtrio()
{
	suprtrio_decrypt_code();
	suprtrio_decrypt_gfx();
}

void tumbleb_state::init_chokchok()
{
	init_htchctch();

	// different palette format, closer to tumblep -- is this controlled by a register? the palette was right with the hatch catch trojan
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x140000, 0x140fff, write16s_delegate(*m_palette, FUNC(palette_device::write16)));

	// slightly different banking
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x100002, 0x100003, write16s_delegate(*this, FUNC(tumbleb_state::chokchok_tilebank_w)));
}

void tumbleb_state::init_carket()
{
	init_htchctch();

	// slightly different banking
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x100002, 0x100003, write16s_delegate(*this, FUNC(tumbleb_state::chokchok_tilebank_w)));
}

void tumbleb_state::init_wlstar()
{
	tumblepb_gfx_rearrange(1);

	// slightly different banking
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x100002, 0x100003, write16smo_delegate(*this, FUNC(tumbleb_state::wlstar_tilebank_w)));

	m_protbase = 0x0000;
}

void tumbleb_state::init_wondl96()
{
	init_wlstar();
	m_protbase = 0x0200;
}

void tumbleb_state::init_dquizgo()
{
	tumblepb_gfx_rearrange(1);

	m_protbase = 0x0200;
}



/******************************************************************************/

/* Misc 'bootleg' hardware - close to base Tumble Pop */
GAME( 1991, tumbleb,  tumblep, tumblepb,     tumblepb, tumbleb_state, init_tumblepb, ROT0, "bootleg", "Tumble Pop (bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1991, tumbleb2, tumblep, tumbleb2,     tumblepb, tumbleb_state, init_tumbleb2, ROT0, "bootleg", "Tumble Pop (bootleg with PIC)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // PIC is protected, sound simulation not 100%
GAME( 1991, tumblepba,tumblep, tumblepba,    tumblepb, tumbleb_state, init_tumblepba,ROT0, "bootleg (Playmark)", "Tumble Pop (Playmark bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // Playmark stickers on ROMs, offset pf1_alt tilemap, OKI banking not confirmed + volume issues?

GAME( 1992, funkyjetb,funkyjet,funkyjetb,    tumblepb, tumbleb_pic_state, init_tumblepb, ROT0, "bootleg", "Funky Jet (bootleg)", MACHINE_NO_SOUND | MACHINE_WRONG_COLORS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // wrong palette, inputs not working, PIC driving an OKI

GAME( 1993, jumpkids, 0,       jumpkids,     tumblepb, tumbleb_state, init_jumpkids, ROT0, "Comad",    "Jump Kids", MACHINE_SUPPORTS_SAVE )

GAME( 1994, pangpang, 0,       pangpang,     tumblepb, tumbleb_state, init_tumbleb2, ROT0, "Dong Gue La Mi Ltd.", "Pang Pang", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // PIC is protected, sound simulation not 100%

/* Misc 'bootleg' hardware - more changes from base hardware */
GAME( 1994, suprtrio, 0,       suprtrio,     suprtrio, tumbleb_state, init_suprtrio, ROT0, "Gameace", "Super Trio", MACHINE_SUPPORTS_SAVE )

GAME( 1996, fncywld,  0,       fncywld,      fncywld, tumbleb_state,  init_fncywld,  ROT0, "Unico",   "Fancy World - Earth of Crisis" , MACHINE_SUPPORTS_SAVE ) // game says 1996, testmode 1995?

GAME( 1996, magipur,  0,       magipur,      magipur, tumbleb_state,  init_magipur,  ROT0, "Unico",   "Magic Purple" , MACHINE_SUPPORTS_SAVE )

/* First Amusement / Mijin / SemiCom hardware (MCU protected) */
GAME( 1994, metlsavr, 0,       metlsavr,     metlsavr, tumbleb_state, init_chokchok, ROT0, "First Amusement", "Metal Saver", MACHINE_SUPPORTS_SAVE )

GAME( 1994, magicbal, 0,       metlsavr,     magicbal, tumbleb_state, init_chokchok, ROT0, "SemiCom", "Magicball Fighting (Korea)", MACHINE_SUPPORTS_SAVE ) // also still has the Metal Saver (c)1994 First Amusement tiles in the GFX

GAME( 1995, chokchok, 0,       chokchok,     chokchok, tumbleb_state, init_chokchok, ROT0, "SemiCom", "Choky! Choky!", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1995, wlstar,   0,       cookbib_mcu,  wlstar, tumbleb_state,   init_wlstar,   ROT0, "Mijin",   "Wonder League Star - Sok-Magicball Fighting (Korea)", MACHINE_SUPPORTS_SAVE ) // translates to 'Wonder League Star - Return of Magicball Fighting'

GAME( 1995, htchctch, 0,       htchctch_mcu, htchctch, tumbleb_state, init_htchctch, ROT0, "SemiCom", "Hatch Catch" , MACHINE_SUPPORTS_SAVE ) // not 100% sure about gfx offsets

GAME( 1995, cookbib,  0,       cookbib,      cookbib, tumbleb_state,  init_htchctch, ROT0, "SemiCom", "Cookie & Bibi (set 1)" , MACHINE_SUPPORTS_SAVE ) // not 100% sure about gfx offsets
GAME( 1995, cookbiba, cookbib, cookbib,      cookbib, tumbleb_state,  init_htchctch, ROT0, "SemiCom", "Cookie & Bibi (set 2)" , MACHINE_SUPPORTS_SAVE )

GAME( 1996, carket,   0,       htchctch,     carket,  tumbleb_state,  init_carket,   ROT0, "SemiCom", "Carket Ball", MACHINE_SUPPORTS_SAVE )

GAME( 1996, wondl96,  0,       cookbib_mcu,  wondl96, tumbleb_state,  init_wondl96,  ROT0, "SemiCom", "Wonder League '96 (Korea)", MACHINE_SUPPORTS_SAVE )

GAME( 1996, sdfight,  0,       sdfight,      sdfight, tumbleb_state,  init_bcstory,  ROT0, "SemiCom / Tirano", "SD Fighters (Korea)", MACHINE_SUPPORTS_SAVE )

GAME( 1997, bcstry,   0,       bcstory,      bcstory, tumbleb_state,  init_bcstory,  ROT0, "SemiCom / Tirano", "B.C. Story (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // gfx offsets?
GAME( 1997, bcstrya,  bcstry,  bcstory,      bcstory, tumbleb_state,  init_bcstory,  ROT0, "SemiCom / Tirano", "B.C. Story (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // gfx offsets?

GAME( 1997, semibase, 0,       semibase,     semibase, tumbleb_state, init_bcstory,  ROT0, "SemiCom / DMD", "MuHanSeungBu (SemiCom Baseball) (Korea)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )// sprite offsets..

GAME( 1998, dquizgo,  0,       cookbib_mcu,  dquizgo, tumbleb_state,  init_dquizgo,  ROT0, "SemiCom / AceVer", "Date Quiz Go Go (Korea)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // check layer offsets
