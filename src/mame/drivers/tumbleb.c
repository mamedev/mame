/***************************************************************************

  the following run on hardware which is physically not the same as
  Tumble Pop but works in a very similar way.

  Tumblepop             (c) 1991 Data East Corporation (Bootleg 1)
  Tumblepop             (c) 1991 Data East Corporation (Bootleg 2)
  Jump Kids             (c) 1993 Comad
  Metal Saver           (c) 1994 First Amusement
  Pang Pang             (c) 1994 Dong Gue La Mi Ltd.
  Super Trio[1]         (c) 1994 GameAce
  Fancy World           (c) 1995 Unico
  Hatch Catch           (c) 1995 SemiCom
  Cookie & Bibi[2]      (c) 1995 SemiCom
  Choky! Choky!         (c) 1995 SemiCom
  WonderLeague Star     (c) 1995 Mijin   (Korea Only) - SemiCom used the name Mijin up to 1995
  WonderLeague '96      (c) 1996 SemiCom (Korea Only)
  SD Fighters           (c) 1996 SemiCom (Korea Only)
  B.C. Story            (c) 1997 SemiCom
  MuHanSeungBu          (c) 1997 SemiCom (Korea Only)
  Date Quiz Go Go       (c) 1998 SemiCom (Korea Only)
  Jumping Pop           (c) 2001 ESD


  [1] has the same sprites as the bootlegs, not much else is the same tho

  [2] test mode crashes the same way on the real board

  Bootleg sound is not quite correct yet (Nothing on bootleg 2).
  ** at least one of the bootlegs uses a protected PIC to drive the OKI **

  If you reset the game while pressing START1 and START2, "VER 0.00 JAPAN"
  is put into tile ram then MAME crashes !

  Sometimes a garbage sprite gets left after the SemiCom logo in Hatch
  Catch - other SemiCom games on different hw do this too, might just
  be a bug in their code

  Emulation by Bryan McPhail, mish@tendril.co.uk
  Jumping Pop sound emulation by R. Belmont


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

      * As levels x-9 and 9-x are only constitued of a "big boss", you can't
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


8) 'fncywrld'

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


12) 'jumppop'

  - It's difficult to find a name for DSW B-3. All I can tell is that,
    when it is set to "2", the girls aren't nude, but less dressed ;)

  - The game needs more investigation to get similar infos to "Tumble Pop" bootlegs/ripoffs.



 MuHanSeungBu
 ------------

 Unfinished Test Mode, Hangs with black screen, same as a real PCB.


 Jumping Pop
 -----------

 Jumping Pop is a complete rip-off of Tumble Pop, not even the levels have
 been changed, it simply has different hardware and new 8bpp backgrounds!


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
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/h6280/h6280.h"
#include "cpu/mcs51/mcs51.h" // for semicom mcu
#include "includes/decocrpt.h"
#include "sound/2151intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "includes/tumbleb.h"
#include "video/decospr.h"

#define TUMBLEP_HACK	0
#define FNCYWLD_HACK	0



/******************************************************************************/

WRITE16_MEMBER(tumbleb_state::tumblepb_oki_w)
{
	okim6295_device *oki = machine().device<okim6295_device>("oki");
	if (mem_mask == 0xffff)
	{
		oki->write(space, 0, data & 0xff);
		//printf("tumbleb_oki_w %04x %04x\n", data, mem_mask);
	}
	else
	{
		oki->write(space, 0, (data >> 8) & 0xff);
		//printf("tumbleb_oki_w %04x %04x\n", data, mem_mask);
	}
    /* STUFF IN OTHER BYTE TOO..*/
}

READ16_MEMBER(tumbleb_state::tumblepb_prot_r)
{
	return ~0;
}

WRITE16_MEMBER(tumbleb_state::jumppop_sound_w)
{
	soundlatch_w(space, 0, data & 0xff);
	device_set_input_line(m_audiocpu, 0, ASSERT_LINE);
}

/******************************************************************************/

READ16_MEMBER(tumbleb_state::tumblepopb_controls_r)
{
	switch (offset << 1)
	{
		case 0:
			return input_port_read(machine(), "PLAYERS");
		case 2:
			return input_port_read(machine(), "DSW");
		case 8:
			return input_port_read(machine(), "SYSTEM");
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


static void tumbleb2_playmusic( device_t *device )
{
	tumbleb_state *state = device->machine().driver_data<tumbleb_state>();
	okim6295_device *oki = downcast<okim6295_device *>(device);
	int status = oki->read_status();

	if (state->m_music_is_playing)
	{
		if (!BIT(status, 3))
		{
			oki->write_command(0x80 | state->m_music_command);
			oki->write_command(0x00 | 0x82);
		}
	}
}


static INTERRUPT_GEN( tumbleb2_interrupt )
{
	tumbleb_state *state = device->machine().driver_data<tumbleb_state>();
	device_set_input_line(device, 6, HOLD_LINE);
	tumbleb2_playmusic(state->m_oki);
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
static void tumbleb2_set_music_bank( running_machine &machine, int bank )
{
	UINT8 *oki = machine.region("oki")->base();
	memcpy(&oki[0x38000], &oki[0x80000 + 0x38000 + 0x8000 * bank], 0x8000);
}

static void tumbleb2_play_sound( okim6295_device *oki, int data )
{
	int status = oki->read_status();

	if (!BIT(status, 0))
	{
		oki->write_command(0x80 | data);
		oki->write_command(0x00 | 0x12);
	}
	else if (!BIT(status, 1))
	{
		oki->write_command(0x80 | data);
		oki->write_command(0x00 | 0x22);
	}
	else if (!BIT(status, 2))
	{
		oki->write_command(0x80 | data);
		oki->write_command(0x00 | 0x42);
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

static void process_tumbleb2_music_command( okim6295_device *oki, int data )
{
	tumbleb_state *state = oki->machine().driver_data<tumbleb_state>();
	int status = oki->read_status();

	if (data == 1) // stop?
	{
		if (BIT(status, 3))
		{
			oki->write_command(0x40);		/* Stop playing music */
			state->m_music_is_playing = 0;
		}
	}
	else
	{
		if (state->m_music_is_playing != data)
		{
			state->m_music_is_playing = data;
			oki->write_command(0x40); // stop the current music
			switch (data)
			{
				case 0x04: // map screen
					state->m_music_bank = 1;
					state->m_music_command = 0x38;
					break;

				case 0x05: // america
					state->m_music_bank = 6;
					state->m_music_command = 0x38;
					break;

				case 0x06: // asia
					state->m_music_bank = 2;
					state->m_music_command = 0x38;
					break;

				case 0x07: // africa/egypt -- don't seem to have a tune for this one
					state->m_music_bank = 4;
					state->m_music_command = 0x38;
					break;

				case 0x08: // antartica
					state->m_music_bank = 3;
					state->m_music_command = 0x38;
					break;

				case 0x09: // brazil / south america
					state->m_music_bank = 4;
					state->m_music_command = 0x38;
					break;

				case 0x0a: // japan -- don't seem to have a tune
					state->m_music_bank = 2;
					state->m_music_command = 0x38;
					break;

				case 0x0b: // australia
					state->m_music_bank = 5;
					state->m_music_command = 0x38;
					break;

				case 0x0c: // france/europe
					state->m_music_bank = 6;
					state->m_music_command = 0x38;
					break;

				case 0x0d: // how to play
					state->m_music_bank = 7;
					state->m_music_command = 0x38;
					break;

				case 0x0f: // stage clear
					state->m_music_bank = 0;
					state->m_music_command = 0x33;
					break;

				case 0x10: // boss stage
					state->m_music_bank = 8;
					state->m_music_command = 0x38;
					break;

				case 0x12: // world clear
					state->m_music_bank = 0;
					state->m_music_command = 0x34;
					break;

				default: // anything else..
					state->m_music_bank = 8;
					state->m_music_command = 0x38;
					break;
			}

			tumbleb2_set_music_bank(oki->machine(), state->m_music_bank);
			tumbleb2_playmusic(oki);
		}
	}
}


static WRITE16_DEVICE_HANDLER( tumbleb2_soundmcu_w )
{
	int sound = tumbleb_sound_lookup[data & 0xff];

	if (sound == 0x00)
	{
		/* pangpang has more commands than tumbleb2, extra sounds */
		//mame_printf_debug("Command %04x\n", data);
	}
	else if (sound == -2)
	{
		process_tumbleb2_music_command(downcast<okim6295_device *>(device), data);
	}
	else
	{
		tumbleb2_play_sound(downcast<okim6295_device *>(device), sound);
	}
}

/******************************************************************************/

static ADDRESS_MAP_START( tumblepopb_main_map, AS_PROGRAM, 16, tumbleb_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
#if TUMBLEP_HACK
	AM_RANGE(0x000000, 0x07ffff) AM_WRITEONLY	/* To write levels modifications */
#endif
	AM_RANGE(0x100000, 0x100001) AM_READWRITE(tumblepb_prot_r, tumblepb_oki_w)
	AM_RANGE(0x120000, 0x123fff) AM_RAM AM_BASE(m_mainram)
	AM_RANGE(0x140000, 0x1407ff) AM_RAM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x160000, 0x1607ff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size) /* Bootleg sprite buffer */
	AM_RANGE(0x160800, 0x160807) AM_WRITEONLY /* writes past the end of spriteram */
	AM_RANGE(0x180000, 0x18000f) AM_READ(tumblepopb_controls_r)
	AM_RANGE(0x18000c, 0x18000d) AM_WRITENOP
	AM_RANGE(0x1a0000, 0x1a07ff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_WRITE(tumblepb_control_0_w)
	AM_RANGE(0x320000, 0x320fff) AM_WRITE(tumblepb_pf1_data_w) AM_BASE(m_pf1_data)
	AM_RANGE(0x322000, 0x322fff) AM_WRITE(tumblepb_pf2_data_w) AM_BASE(m_pf2_data)
	AM_RANGE(0x340000, 0x3401ff) AM_WRITENOP /* Unused row scroll */
	AM_RANGE(0x340400, 0x34047f) AM_WRITENOP /* Unused col scroll */
	AM_RANGE(0x342000, 0x3421ff) AM_WRITENOP
	AM_RANGE(0x342400, 0x34247f) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( fncywld_main_map, AS_PROGRAM, 16, tumbleb_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
#if FNCYWLD_HACK
	AM_RANGE(0x000000, 0x0fffff) AM_WRITEONLY	/* To write levels modifications */
#endif
	AM_RANGE(0x100000, 0x100003) AM_DEVREADWRITE8_LEGACY("ymsnd", ym2151_r, ym2151_w, 0x00ff)
	AM_RANGE(0x100004, 0x100005) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x140000, 0x140fff) AM_RAM_WRITE(paletteram16_xxxxRRRRGGGGBBBB_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x160000, 0x1607ff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size) /* sprites */
	AM_RANGE(0x160800, 0x16080f) AM_WRITEONLY /* goes slightly past the end of spriteram? */
	AM_RANGE(0x180000, 0x18000f) AM_READ(tumblepopb_controls_r)
	AM_RANGE(0x18000c, 0x18000d) AM_WRITENOP
	AM_RANGE(0x1a0000, 0x1a07ff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_WRITE(tumblepb_control_0_w)
	AM_RANGE(0x320000, 0x321fff) AM_RAM_WRITE(fncywld_pf1_data_w) AM_BASE(m_pf1_data)
	AM_RANGE(0x322000, 0x323fff) AM_RAM_WRITE(fncywld_pf2_data_w) AM_BASE(m_pf2_data)
	AM_RANGE(0x340000, 0x3401ff) AM_WRITENOP /* Unused row scroll */
	AM_RANGE(0x340400, 0x34047f) AM_WRITENOP /* Unused col scroll */
	AM_RANGE(0x342000, 0x3421ff) AM_WRITENOP
	AM_RANGE(0x342400, 0x34247f) AM_WRITENOP
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END


READ16_MEMBER(tumbleb_state::semibase_unknown_r)
{
	return machine().rand();
}

static ADDRESS_MAP_START( htchctch_main_map, AS_PROGRAM, 16, tumbleb_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10000f) AM_READ(semibase_unknown_r)
	AM_RANGE(0x100000, 0x100001) AM_WRITE(semicom_soundcmd_w)
	AM_RANGE(0x100002, 0x100003) AM_WRITE(bcstory_tilebank_w)
	AM_RANGE(0x120000, 0x123fff) AM_RAM AM_BASE(m_mainram)
	AM_RANGE(0x140000, 0x1407ff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x160000, 0x160fff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size) /* Bootleg sprite buffer */
	AM_RANGE(0x180000, 0x18000f) AM_READ(tumblepopb_controls_r)
	AM_RANGE(0x18000c, 0x18000d) AM_WRITENOP
	AM_RANGE(0x1a0000, 0x1a0fff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_WRITE(tumblepb_control_0_w)
	AM_RANGE(0x320000, 0x321fff) AM_WRITE(tumblepb_pf1_data_w) AM_BASE(m_pf1_data)
	AM_RANGE(0x322000, 0x322fff) AM_WRITE(tumblepb_pf2_data_w) AM_BASE(m_pf2_data)
	AM_RANGE(0x323000, 0x331fff) AM_NOP // metal saver writes there when clearing the above tilemaps, flaw in the program routine
	AM_RANGE(0x341000, 0x342fff) AM_RAM				// Extra ram?
ADDRESS_MAP_END


static ADDRESS_MAP_START( jumppop_main_map, AS_PROGRAM, 16, tumbleb_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x120000, 0x123fff) AM_RAM AM_BASE(m_mainram)
	AM_RANGE(0x140000, 0x1407ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x160000, 0x160fff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size) /* Bootleg sprite buffer */
	AM_RANGE(0x180000, 0x180001) AM_NOP	/* IRQ ack? */
	AM_RANGE(0x180002, 0x180003) AM_READ_PORT("PLAYERS")
	AM_RANGE(0x180004, 0x180005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x180006, 0x180007) AM_READ_PORT("DSW")
	AM_RANGE(0x18000c, 0x18000d) AM_WRITE(jumppop_sound_w)
	AM_RANGE(0x1a0000, 0x1a7fff) AM_RAM
	AM_RANGE(0x300000, 0x303fff) AM_RAM_WRITE(tumblepb_pf2_data_w) AM_BASE(m_pf2_data)
	AM_RANGE(0x320000, 0x323fff) AM_RAM_WRITE(tumblepb_pf1_data_w) AM_BASE(m_pf1_data)
	AM_RANGE(0x380000, 0x38000f) AM_WRITEONLY AM_BASE(m_control)
ADDRESS_MAP_END

WRITE16_MEMBER(tumbleb_state::jumpkids_sound_w)
{
	soundlatch_w(space, 0, data & 0xff);
	device_set_input_line(m_audiocpu, 0, HOLD_LINE);
}

static ADDRESS_MAP_START( suprtrio_main_map, AS_PROGRAM, 16, tumbleb_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x700000, 0x700fff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0xa00000, 0xa0000f) AM_RAM AM_BASE(m_control)
	AM_RANGE(0xa20000, 0xa20fff) AM_RAM_WRITE(tumblepb_pf1_data_w) AM_BASE(m_pf1_data)
	AM_RANGE(0xa22000, 0xa22fff) AM_RAM_WRITE(tumblepb_pf2_data_w) AM_BASE(m_pf2_data)
	AM_RANGE(0xcf0000, 0xcf05ff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0xe00000, 0xe00001) AM_READ_PORT("PLAYERS") AM_WRITE(suprtrio_tilebank_w)
	AM_RANGE(0xe40000, 0xe40001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xe80002, 0xe80003) AM_READ_PORT("DSW")
	AM_RANGE(0xec0000, 0xec0001) AM_WRITE(semicom_soundcmd_w)
	AM_RANGE(0xf00000, 0xf07fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pangpang_main_map, AS_PROGRAM, 16, tumbleb_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x120000, 0x123fff) AM_RAM AM_BASE(m_mainram)
	AM_RANGE(0x140000, 0x1407ff) AM_RAM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x160000, 0x1607ff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size) /* Bootleg sprite buffer */
	AM_RANGE(0x160800, 0x160807) AM_WRITEONLY // writes past the end of spriteram
	AM_RANGE(0x180000, 0x18000f) AM_READ(tumblepopb_controls_r)
	AM_RANGE(0x1a0000, 0x1a07ff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_WRITE(tumblepb_control_0_w)
	AM_RANGE(0x320000, 0x321fff) AM_RAM_WRITE(pangpang_pf1_data_w) AM_BASE(m_pf1_data)
	AM_RANGE(0x340000, 0x341fff) AM_RAM_WRITE(pangpang_pf2_data_w) AM_BASE(m_pf2_data)
ADDRESS_MAP_END


/******************************************************************************/

WRITE16_MEMBER(tumbleb_state::semicom_soundcmd_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w(space, 0, data & 0xff);
		// needed for Super Trio which reads the sound with polling
		// device_spin_until_time(&space.device(), attotime::from_usec(100));
		machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(20));

	}
}

WRITE8_MEMBER(tumbleb_state::oki_sound_bank_w)
{
	UINT8 *oki = machine().region("oki")->base();
	memcpy(&oki[0x30000], &oki[(data * 0x10000) + 0x40000], 0x10000);
}

static ADDRESS_MAP_START( semicom_sound_map, AS_PROGRAM, 8, tumbleb_state )
	AM_RANGE(0x0000, 0xcfff) AM_ROM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM
	AM_RANGE(0xf000, 0xf001) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xf002, 0xf002) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	//AM_RANGE(0xf006, 0xf006) ??
	AM_RANGE(0xf008, 0xf008) AM_READ(soundlatch_r)
	AM_RANGE(0xf00e, 0xf00e) AM_WRITE(oki_sound_bank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( suprtrio_sound_map, AS_PROGRAM, 8, tumbleb_state )
	AM_RANGE(0x0000, 0xcfff) AM_ROM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM
	AM_RANGE(0xf002, 0xf002) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	//AM_RANGE(0xf006, 0xf006) ??
	AM_RANGE(0xf008, 0xf008) AM_READ(soundlatch_r)
	AM_RANGE(0xf00e, 0xf00e) AM_WRITE(oki_sound_bank_w)
ADDRESS_MAP_END

WRITE8_MEMBER(tumbleb_state::jumppop_z80_bank_w)
{
	memory_set_bankptr(machine(), "bank1", machine().region("audiocpu")->base() + 0x10000 + (0x4000 * data));
}

static ADDRESS_MAP_START( jumppop_sound_map, AS_PROGRAM, 8, tumbleb_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

READ8_MEMBER(tumbleb_state::jumppop_z80latch_r)
{
	device_set_input_line(m_audiocpu, 0, CLEAR_LINE);
	return soundlatch_r(space, 0);
}

static ADDRESS_MAP_START( jumppop_sound_io_map, AS_IO, 8, tumbleb_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE_LEGACY("ymsnd", ym3812_w)
	AM_RANGE(0x02, 0x02) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x03, 0x03) AM_READ(jumppop_z80latch_r)
	AM_RANGE(0x04, 0x04) AM_NOP
	AM_RANGE(0x05, 0x05) AM_WRITE(jumppop_z80_bank_w)
	AM_RANGE(0x06, 0x06) AM_NOP
ADDRESS_MAP_END

/* Jump Kids */

static ADDRESS_MAP_START( jumpkids_main_map, AS_PROGRAM, 16, tumbleb_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_WRITE(jumpkids_sound_w)
	AM_RANGE(0x120000, 0x123fff) AM_RAM AM_BASE(m_mainram)
	AM_RANGE(0x140000, 0x1407ff) AM_RAM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x160000, 0x1607ff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size) /* Bootleg sprite buffer */
	AM_RANGE(0x160800, 0x160807) AM_WRITEONLY /* writes past the end of spriteram */
	AM_RANGE(0x180000, 0x18000f) AM_READ(tumblepopb_controls_r)
	AM_RANGE(0x18000c, 0x18000d) AM_WRITENOP
	AM_RANGE(0x1a0000, 0x1a07ff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_WRITE(tumblepb_control_0_w)
	AM_RANGE(0x320000, 0x320fff) AM_WRITE(tumblepb_pf1_data_w) AM_BASE(m_pf1_data)
	AM_RANGE(0x322000, 0x322fff) AM_WRITE(tumblepb_pf2_data_w) AM_BASE(m_pf2_data)
	AM_RANGE(0x340000, 0x3401ff) AM_WRITENOP /* Unused row scroll */
	AM_RANGE(0x340400, 0x34047f) AM_WRITENOP /* Unused col scroll */
	AM_RANGE(0x342000, 0x3421ff) AM_WRITENOP
	AM_RANGE(0x342400, 0x34247f) AM_WRITENOP
ADDRESS_MAP_END

WRITE8_MEMBER(tumbleb_state::jumpkids_oki_bank_w)
{
	UINT8* sound1 = machine().region("oki")->base();
	UINT8* sound2 = machine().region("oki2")->base();
	int bank = data & 0x03;

	memcpy(sound1 + 0x20000, sound2 + bank * 0x20000, 0x20000);
}

static ADDRESS_MAP_START( jumpkids_sound_map, AS_PROGRAM, 8, tumbleb_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_WRITE(jumpkids_oki_bank_w)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)
ADDRESS_MAP_END


READ8_MEMBER(tumbleb_state::prot_io_r)
{
	// never read?
	return 0x00;
}


// probably not endian safe
WRITE8_MEMBER(tumbleb_state::prot_io_w)
{

	switch (offset)
	{
		case 0x00:
		{
			UINT16 word = m_mainram[(m_protbase/2) + m_semicom_prot_offset];
			word = (word & 0xff00) | (data << 0);
			m_mainram[(m_protbase/2) + m_semicom_prot_offset] = word;

			break;
		}

		case 0x01:
		{
			UINT16 word = m_mainram[(m_protbase/2) + m_semicom_prot_offset];
			word = (word & 0x00ff) | (data << 8);
			m_mainram[(m_protbase/2) + m_semicom_prot_offset] = word;

			break;
		}

		case 0x02: // offset
		{
			m_semicom_prot_offset = data;
			break;
		}

		case 0x03: // ??
		{
			//logerror("offset %02x data %02x\n",offset,data);
			break;
		}
	}
}


/* Semicom AT89C52 MCU */
static ADDRESS_MAP_START( protection_map, AS_PROGRAM, 8, tumbleb_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( protection_iomap, AS_IO, 8, tumbleb_state )
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READWRITE(prot_io_r,prot_io_w)
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( tumblepb )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0001, 0x0001, "2 Coins to Start, 1 to Continue" )	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
#if TUMBLEP_HACK
	PORT_DIPNAME( 0x0800, 0x0800, "Remove Monsters" )	PORT_DIPLOCATION("SW2:5")
#else
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )	PORT_DIPLOCATION("SW2:5")		// See notes
#endif
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
#if TUMBLEP_HACK
	PORT_DIPNAME( 0x0400, 0x0400, "Edit Levels" )		PORT_DIPLOCATION("SW2:6")
#else
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )	PORT_DIPLOCATION("SW2:6")		// See notes
#endif
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:8")
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
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )       // to be confirmed
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
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")	/* Dip switches */
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
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )
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
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Language ) )		// only seems to the title screen
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
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )	// to be confirmed
	PORT_DIPSETTING(      0x3000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )

#if FNCYWLD_HACK
	PORT_DIPNAME( 0x0800, 0x0800, "Remove Monsters" )
#else
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )		// See notes
#endif
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
#if FNCYWLD_HACK
	PORT_DIPNAME( 0x0400, 0x0400, "Edit Levels" )
#else
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )		// See notes
#endif
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Freeze" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
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
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )       // to be confirmed
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
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )       // to be confirmed
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
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )       // to be confirmed
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Winning Rounds" )	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPNAME( 0x0006, 0x0006, "Ball Speed" )		PORT_DIPLOCATION("SW1:7,6")
	PORT_DIPSETTING(      0x0000, "Slow" )
	PORT_DIPSETTING(      0x0006, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, "Fast" )
	PORT_DIPSETTING(      0x0004, "Very Fast" )
	PORT_DIPNAME( 0x0018, 0x0018, "Energy Decrease" )	PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(      0x0000, "Very Small" )
	PORT_DIPSETTING(      0x0010, "Small" )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, "Big" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Game_Time ) )	PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, "60 Seconds" )
	PORT_DIPSETTING(      0x0000, "90 Seconds" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Starting Balls" )	PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x00c0, "4" )
	PORT_DIPSETTING(      0x0040, "5" )
	PORT_DIPSETTING(      0x0080, "6" )

	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x0600, 0x0600, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:7,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coinage ) )	PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Language ) )	PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x4000, DEF_STR( Korean ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:1")
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
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )       // to be confirmed
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
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )       // to be confirmed
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
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )       // to be confirmed
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
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )       // to be confirmed
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

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) )	    PORT_DIPLOCATION("SW2:8")
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

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) )	    PORT_DIPLOCATION("SW2:8")
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
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )       // to be confirmed
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

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) )	    PORT_DIPLOCATION("SW2:8")     // See notes
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

static INPUT_PORTS_START( jumppop )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_SERVICE_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SWA:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:6,7,8")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Picture Viewer" )        PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "BG Type" )               PORT_DIPLOCATION("SWB:4")     // See notes
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
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

static const gfx_layout jumppop_tcharlayout =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+8,0,8,RGN_FRAC(1,2)+16,RGN_FRAC(1,2)+24,16,24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tlayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
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


static const gfx_layout jumpop_tlayout =
{
	16,16,
	RGN_FRAC(1,2),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+8,0,8,RGN_FRAC(1,2)+16,RGN_FRAC(1,2)+24,16,24,
	256+RGN_FRAC(1,2)+0,256+RGN_FRAC(1,2)+8,256+0,256+8,256+RGN_FRAC(1,2)+16,256+RGN_FRAC(1,2)+24,256+16,256+24
	},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	512+0*32, 512+1*32, 512+2*32, 512+3*32, 512+4*32, 512+5*32, 512+6*32, 512+7*32
	},
	128*8
};



static GFXDECODE_START( tumbleb )
	GFXDECODE_ENTRY( "gfx1", 0, tcharlayout, 256, 16 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tlayout,     512, 16 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx1", 0, tlayout,     256, 16 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tlayout,       0, 16 )	/* Sprites 16x16 */
GFXDECODE_END

static GFXDECODE_START( suprtrio )
	GFXDECODE_ENTRY( "gfx1", 0, tcharlayout,		256, 16 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, suprtrio_tlayout,	512, 16 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx1", 0, suprtrio_tlayout,	256, 16 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tlayout,    		  0, 16 )	/* Sprites 16x16 */
GFXDECODE_END

static GFXDECODE_START( fncywld )
	GFXDECODE_ENTRY( "gfx1", 0, tcharlayout, 0x400, 0x40 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tlayout,     0x400, 0x40 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx1", 0, tlayout,     0x200, 0x40 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tlayout,     0x000, 0x40 )	/* Sprites 16x16 */
GFXDECODE_END

static GFXDECODE_START( jumppop )
	GFXDECODE_ENTRY( "gfx1", 0, jumppop_tcharlayout,0x100, 0x40 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, jumpop_tlayout,		0x100, 0x40 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx1", 0, jumpop_tlayout,		0x100, 0x40 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tlayout,			0x000, 0x40 )	/* Sprites 16x16 */
GFXDECODE_END


/******************************************************************************/


static MACHINE_START( tumbleb )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("audiocpu");
	state->m_oki = machine.device("oki");

	state->save_item(NAME(state->m_music_command));
	state->save_item(NAME(state->m_music_bank));
	state->save_item(NAME(state->m_music_is_playing));

	state->save_item(NAME(state->m_control_0));
	state->save_item(NAME(state->m_flipscreen));
	state->save_item(NAME(state->m_tilebank));
}

static MACHINE_RESET( tumbleb )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_music_command = 0;
	state->m_music_bank = 0;
	state->m_music_is_playing = 0;
	state->m_flipscreen = 0;
	state->m_tilebank = 0;
	memset(state->m_control_0, 0, ARRAY_LENGTH(state->m_control_0));
}

static MACHINE_CONFIG_START( tumblepb, tumbleb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 14000000)
	MCFG_CPU_PROGRAM_MAP(tumblepopb_main_map)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

	MCFG_MACHINE_START(tumbleb)
	MCFG_MACHINE_RESET(tumbleb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_STATIC(tumblepb)

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 3);
	decospr_device::set_is_bootleg(*device, true);

	MCFG_GFXDECODE(tumbleb)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(tumblepb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 8000000/10, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( tumbleb2, tumbleb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 14000000)
	MCFG_CPU_PROGRAM_MAP(tumblepopb_main_map)
	MCFG_CPU_VBLANK_INT("screen", tumbleb2_interrupt)

	MCFG_MACHINE_START(tumbleb)
	MCFG_MACHINE_RESET(tumbleb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_STATIC(tumblepb)

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 3);
	decospr_device::set_is_bootleg(*device, true);

	MCFG_GFXDECODE(tumbleb)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(tumblepb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 8000000/10, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( jumpkids, tumbleb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(jumpkids_main_map)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

	/* z80? */
	MCFG_CPU_ADD("audiocpu", Z80, 8000000/2)
	MCFG_CPU_PROGRAM_MAP(jumpkids_sound_map)

	MCFG_MACHINE_START(tumbleb)
	MCFG_MACHINE_RESET(tumbleb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_STATIC(jumpkids)

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 3);
	decospr_device::set_is_bootleg(*device, true);

	MCFG_GFXDECODE(tumbleb)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(tumblepb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 8000000/8, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( fncywld, tumbleb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(fncywld_main_map)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

	MCFG_MACHINE_START(tumbleb)
	MCFG_MACHINE_RESET(tumbleb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_STATIC(fncywld)

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 3);
	decospr_device::set_is_bootleg(*device, true);
	decospr_device::set_transpen(*device, 15);

	MCFG_GFXDECODE(fncywld)
	MCFG_PALETTE_LENGTH(0x800)

	MCFG_VIDEO_START(fncywld)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2151, 32220000/9)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.20)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.20)

	MCFG_OKIM6295_ADD("oki", 1023924, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END



static void semicom_irqhandler( device_t *device, int irq )
{
	tumbleb_state *state = device->machine().driver_data<tumbleb_state>();
	device_set_input_line(state->m_audiocpu, 0, irq ? ASSERT_LINE : CLEAR_LINE);
}


static const ym2151_interface semicom_ym2151_interface =
{
	semicom_irqhandler
};

static MACHINE_RESET (htchctch)
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	/* copy protection data every reset */
	UINT16 *PROTDATA = (UINT16*)machine.region("user1")->base();
	int i, len = machine.region("user1")->bytes();

	for (i = 0; i < len / 2; i++)
		state->m_mainram[0x000/2 + i] = PROTDATA[i];

	MACHINE_RESET_CALL(tumbleb);
}

static MACHINE_CONFIG_START( htchctch, tumbleb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 15000000) /* verified */
	MCFG_CPU_PROGRAM_MAP(htchctch_main_map)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 15000000/4) /* verified on dquizgo */
	MCFG_CPU_PROGRAM_MAP(semicom_sound_map)

	MCFG_MACHINE_START(tumbleb)
	MCFG_MACHINE_RESET(htchctch)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2400)) // ?? cookbib needs it above ~2400 or the Joystick on the How to Play screen is the wrong colour?!
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_STATIC(semicom)

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 3);
	decospr_device::set_is_bootleg(*device, true);

	MCFG_GFXDECODE(tumbleb)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(tumblepb)

	/* sound hardware - same as hyperpac */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2151, 3427190)
	MCFG_SOUND_CONFIG(semicom_ym2151_interface)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.10)

	MCFG_OKIM6295_ADD("oki", 1024000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cookbib, htchctch )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_STATIC( semicom_altoffsets )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cookbib_mcu, htchctch )

	/* basic machine hardware */
	MCFG_CPU_ADD("protection", I8052, 16000000)  // AT89C52
	MCFG_CPU_PROGRAM_MAP(protection_map)
	MCFG_CPU_IO_MAP(protection_iomap)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_STATIC( semicom_altoffsets )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bcstory, htchctch )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_STATIC(bcstory)

	MCFG_SOUND_REPLACE("ymsnd", YM2151, 3427190)
	MCFG_SOUND_CONFIG(semicom_ym2151_interface)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( semibase, bcstory )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_STATIC(semibase )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sdfight, bcstory )
	MCFG_VIDEO_START(sdfight)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_STATIC(sdfight)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( metlsavr, cookbib )

	MCFG_SOUND_REPLACE("ymsnd", YM2151, 3427190)
	MCFG_SOUND_CONFIG(semicom_ym2151_interface)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( jumppop, tumbleb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(jumppop_main_map)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 3500000) /* verified */
	MCFG_CPU_PROGRAM_MAP(jumppop_sound_map)
	MCFG_CPU_IO_MAP(jumppop_sound_io_map)
	MCFG_CPU_PERIODIC_INT(nmi_line_pulse, 1953)	/* measured */

	MCFG_MACHINE_START(tumbleb)
	MCFG_MACHINE_RESET(tumbleb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_STATIC(jumppop)

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 3);
	decospr_device::set_is_bootleg(*device, true);

	MCFG_GFXDECODE(jumppop)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(jumppop)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM3812, 3500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.70)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.70)

	MCFG_OKIM6295_ADD("oki", 875000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( suprtrio, tumbleb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 14000000) /* 14mhz should be correct, but lots of sprite flicker later in game */
	MCFG_CPU_PROGRAM_MAP(suprtrio_main_map)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 8000000)
	MCFG_CPU_PROGRAM_MAP(suprtrio_sound_map)

	MCFG_MACHINE_START(tumbleb)
	MCFG_MACHINE_RESET(tumbleb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8-1, 31*8-2)
	MCFG_SCREEN_UPDATE_STATIC(suprtrio)

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 3);
	decospr_device::set_is_bootleg(*device, true);

	MCFG_GFXDECODE(suprtrio)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(suprtrio)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 875000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pangpang, tumbleb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 14000000)
	MCFG_CPU_PROGRAM_MAP(pangpang_main_map)
	MCFG_CPU_VBLANK_INT("screen", tumbleb2_interrupt)

	MCFG_MACHINE_START(tumbleb)
	MCFG_MACHINE_RESET(tumbleb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_STATIC(pangpang)

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 3);
	decospr_device::set_is_bootleg(*device, true);

	MCFG_GFXDECODE(tumbleb)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(pangpang)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 8000000/10, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_CONFIG_END

/******************************************************************************/

ROM_START( tumbleb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE ("thumbpop.12", 0x00000, 0x40000, CRC(0c984703) SHA1(588d2b2464e0027c8d0703a2b62ebda225ba4276) )
	ROM_LOAD16_BYTE( "thumbpop.13", 0x00001, 0x40000, CRC(864c4053) SHA1(013eb35e79aa7a7cd1a8061c4b75b37a8bfb10c6) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "thumbpop.19",  0x00000, 0x40000, CRC(0795aab4) SHA1(85b38804446f6b0b4d8c3a59a8958d520c567a4e) )
	ROM_LOAD16_BYTE( "thumbpop.18",  0x00001, 0x40000, CRC(ad58df43) SHA1(2e562bfffb42543af767dd9e82a1d2465dfcd8b8) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "map-01.rom",   0x00000, 0x80000, CRC(e81ffa09) SHA1(01ada9557ead91eb76cf00db118d6c432104a398) )
	ROM_LOAD( "map-00.rom",   0x80000, 0x80000, CRC(8c879cfe) SHA1(a53ef7811f14a8b105749b1cf29fe8a3a33bab5e) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "thumbpop.snd", 0x00000, 0x80000, CRC(fabbf15d) SHA1(de60be43a5cd1d4b93c142bde6cbfc48a25545a3) )
ROM_END

ROM_START( tumbleb2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE ("thumbpop.2", 0x00000, 0x40000, CRC(34b016e1) SHA1(b4c496358d48469d170a69e8bba58e0ea919b418) )
	ROM_LOAD16_BYTE( "thumbpop.3", 0x00001, 0x40000, CRC(89501c71) SHA1(2c202218934b845fdf7c99eaf280dccad90767f2) )

	ROM_REGION( 0x2d4c, "cpu1", 0 ) /* PIC16c57 */
	ROM_LOAD( "pic_16c57", 0x00000, 0x2d4c, NO_DUMP ) // protected

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "thumbpop.19",  0x00000, 0x40000, CRC(0795aab4) SHA1(85b38804446f6b0b4d8c3a59a8958d520c567a4e) )
	ROM_LOAD16_BYTE( "thumbpop.18",  0x00001, 0x40000, CRC(ad58df43) SHA1(2e562bfffb42543af767dd9e82a1d2465dfcd8b8) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "map-01.rom",   0x00000, 0x80000, CRC(e81ffa09) SHA1(01ada9557ead91eb76cf00db118d6c432104a398) )
	ROM_LOAD( "map-00.rom",   0x80000, 0x80000, CRC(8c879cfe) SHA1(a53ef7811f14a8b105749b1cf29fe8a3a33bab5e) )

	ROM_REGION( 0x100000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "thumbpop.snd", 0x00000, 0x80000, CRC(fabbf15d) SHA1(de60be43a5cd1d4b93c142bde6cbfc48a25545a3) )
	ROM_RELOAD(0x80000,0x80000)
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

	ROM_REGION( 0x80000, "gfx1", 0 ) /* GFX */
	ROM_LOAD16_BYTE( "30-ic125.15j", 0x00000, 0x40000, CRC(44b9a089) SHA1(b6f99b0b597d540b375616dad4354fc9dbb75a21) )
	ROM_LOAD16_BYTE( "29-ic124.13j", 0x00001, 0x40000, CRC(3f98ec69) SHA1(f09a62d9bd7ab7681436a1f2f450565573927165) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* GFX */
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
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "01_fw02.bin", 0x000000, 0x080000, CRC(ecb978c1) SHA1(68fbf93a81875f744c6f9820dc4c7d88e912e0a0) )
	ROM_LOAD16_BYTE( "02_fw03.bin", 0x000001, 0x080000, CRC(2d233b42) SHA1(aebeb5d3e06e73d14f713f201b25466bcac97a68) )

	ROM_REGION( 0x100000, "gfx2", 0  )
	ROM_LOAD16_BYTE( "05_fw06.bin",  0x00000, 0x40000, CRC(e141ecdc) SHA1(fd656ceb2baccefadfa1e9f6932b1e0f0ec0a189) )
	ROM_LOAD16_BYTE( "06_fw07.bin",  0x00001, 0x40000, CRC(0058a812) SHA1(fc6101a11af63536d0a345c820bcd234bb4ce91a) )
	ROM_LOAD16_BYTE( "03_fw04.bin",  0x80000, 0x40000, CRC(6ad38c14) SHA1(a9951432c2ec5e07ed2ee5faac3f2558242438f2) )
	ROM_LOAD16_BYTE( "04_fw05.bin",  0x80001, 0x40000, CRC(b8d079a6) SHA1(8ad63fba26f7588a9764a0585c159fb57cb8c7ed) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "08_fw09.bin", 0x00000, 0x40000, CRC(a4a00de9) SHA1(65f03a65569f70fb6f3a0fc7caf038bb44a7f503) )
	ROM_LOAD16_BYTE( "07_fw08.bin", 0x00001, 0x40000, CRC(b48cd1d4) SHA1(a95eeba38ae1ce0a2086edb767f636a9cdbd0176) )
	ROM_LOAD16_BYTE( "10_fw11.bin", 0x80000, 0x40000, CRC(f21bab48) SHA1(84371b31487ca5abcbf57152a64f384959d19209) )
	ROM_LOAD16_BYTE( "09_fw10.bin", 0x80001, 0x40000, CRC(6aea8e0f) SHA1(91e2eeef001351c73b1bfbc1a7840e37d3f89900) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "00_fw01.bin", 0x000000, 0x040000, CRC(b395fe01) SHA1(ac7f2e21413658f8d2a1abf3a76b7817a4e050c9) )
ROM_END

ROM_START( suprtrio )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "rom2",  0x00000, 0x40000, CRC(4102e59d) SHA1(f06f1273dbbb91fa61d84541aa124d9c88ee94c1) )
	ROM_LOAD16_BYTE( "rom1",  0x00001, 0x40000, CRC(cc3a83c3) SHA1(6f8b1b6b666ce11c02e9defcba751d88621e572d) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 */
	ROM_LOAD( "rom4l", 0x000000, 0x10000, CRC(466aa96d) SHA1(37f1ba148dbad27ed8e71a0b3434ff970fcb519f) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* bg tiles */
	ROM_LOAD( "rom4",  0x00000, 0x20000, CRC(cd2dfae4) SHA1(1d872b5abaf72d34bd4a45f6be69aa6474887b4b) )
	ROM_CONTINUE(      0x40000, 0x20000 )
	ROM_CONTINUE(      0x20000, 0x20000 )
	ROM_CONTINUE(      0x60000, 0x20000 )
	ROM_LOAD( "rom5",  0x80000, 0x20000, CRC(4e64da64) SHA1(f2518b3d83d7fd46000ca982b2d91ce75034b411) )
	ROM_CONTINUE(      0xc0000, 0x20000 )
	ROM_CONTINUE(      0xa0000, 0x20000 )
	ROM_CONTINUE(      0xe0000, 0x20000 )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* sprites */
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

	ROM_REGION( 0x100000, "gfx1", 0 ) // PF1 tilemap
	ROM_LOAD16_BYTE( "11.bin", 0x00000, 0x20000, CRC(a2b9fec8) SHA1(121771466c288e132cdcf6abdc3bbe2578de9260) )
	ROM_CONTINUE(0x80000,0x20000)
	ROM_LOAD16_BYTE( "10.bin", 0x00001, 0x20000, CRC(4f59d7b9) SHA1(a0eabb44ecb6922f656a5032c0ab757813b9cc13) )
	ROM_CONTINUE(0x80001,0x20000)
	ROM_LOAD16_BYTE( "6.bin", 0x40000, 0x20000, CRC(1ebbc4f1) SHA1(6fb745ebe7ee8ecf5036ac0c4a5dda71cbb40063) )
	ROM_CONTINUE(0xc0000,0x20000)
	ROM_LOAD16_BYTE( "7.bin", 0x40001, 0x20000, CRC(cd544173) SHA1(b929d771040a48356b449458d3125142b9bfc365) )
	ROM_CONTINUE(0xc0001,0x20000)

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "8.bin",   0x00000, 0x40000, CRC(ea0fa1e0) SHA1(1f2f6264097d15339782c2e399d125c3835fd852) )
	ROM_LOAD16_BYTE( "9.bin",   0x00001, 0x40000, CRC(1da5fe49) SHA1(338be1a9f8c42e685e1cefb12b2d169b7560e5f7) )
	ROM_LOAD16_BYTE( "4.bin",   0x80000, 0x40000, CRC(4f282eb1) SHA1(3731045a500082d37588edf7cbb0c0ebae566aab) )
	ROM_LOAD16_BYTE( "5.bin",   0x80001, 0x40000, CRC(00694df9) SHA1(f07373c7ef379daa4e788c169579e23a1133d884) )

	ROM_REGION( 0x100000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(e722bb02) SHA1(ebb8c87d32dccbebf6d8a47703ac12be984f4a3d) )
	ROM_RELOAD(0x80000,0x80000)
ROM_END

/*

Jumping Pop
ESD, 2001

PCB Layout
----------

|------------------------------------------------------|
| TDA1519A                62256         PAL            |
| SAMPLES.BIN YM3014      62256         BG0.BIN        |
|             YM3812    |---------|     BG1.BIN        |
|6295   Z80   6116      |         |                    |
|          Z80_PRG.BIN  |A40MX04  |PAL                 |
|                       |         |                    |
|J                      |         |                    |
|A PAL                  |---------|                    |
|M                           6116                      |
|M                           6116                      |
|A     14MHz                 6116                      |
|      16MHz                 6116|---------|           |
|      68K_PRG.BIN        PAL    |         |           |
|                         PAL    |A40MX04  |           |
|              |-----|    PAL    |         |  SP0.BIN  |
|      62256   |68000|           |         |  SP1.BIN  |
|DIP1  62256   |     |           |---------|           |
|      PAL     |-----|           6116  6116            |
|DIP2  PAL                       6116  6116            |
|------------------------------------------------------|
Notes:
      68000   - Motorola MC68EC000FU10, running at 16.000MHz (QFP64)
      YM3812  - Yamaha YM3812, running at 3.500MHz [14 / 4] (DIP24)
      YM3012  - Yamaha YM3012 16bit Serial DAC (DIP8)
      Z80     - Zilog Z84C0006FEC, running at 3.500MHz [14 / 4] (QFP44)
      6295    - Oki M6295, running at 875kHz [14 / 16], samples rate 6.628787879kHz [875000 /132] (QFP44)
      A40MX04 - Actel A40MX04-F FPGA (x2, PLCC84)
      TDA1519A- Philips TDA1519A Dual 6W Power Amplifier
      DIP1/2  - 8 Position Dip Switch
      62256   - 8K x8 SRAM (x4, DIP28)
      6116    - 2K x8 SRAM (x9, DIP24)
      VSync   - 60Hz

      ROMs -
              Filename      Type                                      Use
              ---------------------------------------------------------------------------
              68K_PRG.BIN   Hitachi HN27C4096 256K x16 EPROM          68000 Program
              Z80_PRG.BIN   Atmel AT27C020 256K x8 OTP MASKROM        Z80 Program
              SAMPLES.BIN   Atmel AT27C020 256K x8 OTP MASKROM        Oki M6295 Samples
              BG0/1.BIN     Macronix 29F8100MC 1M x8 SOP44 FlashROM   Background Graphics
              SP0/1.BIN     Macronix 29F8100MC 1M x8 SOP44 FlashROM   Sprite Graphics

              Note there are no IC locations on the PCB, so the extension of the ROMs is just 'BIN'

*/

ROM_START( jumppop )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_WORD_SWAP ("68k_prg.bin", 0x00000, 0x80000, CRC(123536b9) SHA1(3597dec81e98d7bdf4ea9053983e62f127defcb7) )

	ROM_REGION( 0x80000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "z80_prg.bin", 0x00000, 0x40000, CRC(a88d4424) SHA1(eefb5ac79632931a36f360713c482cd079891f91) )
	ROM_RELOAD( 0x10000, 0x40000)

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "bg0.bin", 0x000000, 0x100000, CRC(35a1363d) SHA1(66c550b0bdea7c8b079f186f5e044f731d31bc58) )
	ROM_LOAD( "bg1.bin", 0x100000, 0x100000, CRC(5b37f943) SHA1(fe73b839f29d4c32823418711b22f85a5f583ec2) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "sp0.bin", 0x000000, 0x100000, CRC(7c5d0633) SHA1(1fba60073d1d5d4dbd217fde181fa73a9d92bdc6) )
	ROM_LOAD( "sp1.bin", 0x100000, 0x100000, CRC(7eae782e) SHA1(a33c544ad9516ec409c209968e72f63e7cdb934b) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "samples.bin", 0x00000, 0x40000, CRC(066f30a7) SHA1(6bdd0210001c597819f7132ffa1dc1b1d55b4e0a) )
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

	ROM_REGION( 0x80000, "gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "first-5.rom5", 0x00001, 0x40000, CRC(dd4af746) SHA1(185a8080173b3c05fcc5f5ee2f71606987826e79) )
	ROM_LOAD16_BYTE( "first-6.rom6", 0x00000, 0x40000, CRC(808b0e0b) SHA1(f4913e135986b28b4e56bdcc4fd7dd5aad9aa467) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* sprites */
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

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
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

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
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

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
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

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
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
        Z80 clock: 3.42719MHz  <-- strange clock, but verified correct.
      M6295 clock: 1.024MHz, sample rate = /132
      87C52 clock: 15MHz
     YM2151 clock: 3.42719MHz
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

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(5b27adb6) SHA1(a0821093d8c73765ff15767bdfc0afa95aa1371d) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "p01.c1", 0x00000, 0x20000, CRC(18c06829) SHA1(46b180319ed33abeaba70d2cc61f17639e59bfdb) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "p06srom5.bin", 0x00001, 0x40000, CRC(3d2cbb0d) SHA1(bc80be594a40989e3c23539fc2021de65a2444c5) )
	ROM_LOAD16_BYTE( "p07srom6.bin", 0x00000, 0x40000, CRC(0207949c) SHA1(84b4dcd27fe89a5350b6642ef99719bb85514174) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* GFX */
	ROM_LOAD16_BYTE( "p08uor1.bin",  0x00000, 0x20000, CRC(6811e7b6) SHA1(8157f92a3168ffbac86cd8c6294b9c0f3ee0835d) )
	ROM_LOAD16_BYTE( "p09uor2.bin",  0x00001, 0x20000, CRC(1c6549cf) SHA1(c05aba9b744144db4537e472842b0d53325aa78f) )
	ROM_LOAD16_BYTE( "p10uor3.bin",  0x40000, 0x20000, CRC(6462e6e0) SHA1(0d107214dfb257e15931701bad6b42c6aadd8a18) )
	ROM_LOAD16_BYTE( "p11uor4.bin",  0x40001, 0x20000, CRC(9c511d98) SHA1(6615cbb125bd1e1b4da400ec4c4a0f4df8f6fa75) )
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

	ROM_REGION( 0x80000, "gfx1", 0 ) /* */
	ROM_LOAD16_BYTE( "srom5.bin", 0x00001, 0x40000, CRC(73a46e43) SHA1(054fac2dc5dffcbb9d81600689c07774d2e200b6) )
	ROM_LOAD16_BYTE( "srom6.bin", 0x00000, 0x40000, CRC(ade2dbec) SHA1(12d385d22307d8251e711788dff2e503c8f8ca7c) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* GFX */
	ROM_LOAD16_BYTE( "uor1.bin",  0x00000, 0x20000, CRC(a7d91f23) SHA1(eb9694e05b8a04ed1cdbb834e1bf745a2b0260be) )
	ROM_LOAD16_BYTE( "uor2.bin",  0x00001, 0x20000, CRC(9aacbec2) SHA1(c1cfe243a7d51c950785073f235d72cc01724cdb) )
	ROM_LOAD16_BYTE( "uor3.bin",  0x40000, 0x20000, CRC(3fee0c3c) SHA1(c71439ba8033c549e40522db5270caf4a297fb99) )
	ROM_LOAD16_BYTE( "uor4.bin",  0x40001, 0x20000, CRC(bed9ed2d) SHA1(7103b99cd0d54df864ea4a0f269011e30ad29ed7) )
ROM_END

/* Choky Choky */

ROM_START( chokchok )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ub18.bin",  0x00001, 0x40000, CRC(b183852a) SHA1(fd50c6d91dba64b936ac367e5e5235d09ed60fdd) )
	ROM_LOAD16_BYTE( "ub17.bin",  0x00000, 0x40000, CRC(ecdb45ca) SHA1(03eb2d27ae4de25aa15477135d3b4de8b3b7f7f0) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 Code */
	ROM_LOAD( "ub5.bin", 0x00000, 0x10000 , CRC(30c2171d) SHA1(3954e286d57b955af6ba9b1a0b49c442d7f295ae) )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x200, "user1", 0 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from shared ram, the MCU puts it there */
	ROM_LOAD16_WORD( "protdata.bin", 0x00000, 0x200 , CRC(0bd39834) SHA1(2860c2b7fcb74546afde11a59d4b359612ab6e68) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uc1.bin", 0x00000, 0x40000, CRC(f3f57abd) SHA1(601dc669020ef9156fa221e768be9b88454e3f55) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "srom5.bin", 0x00001, 0x20000, CRC(836608b8) SHA1(7aa624274efee0a7affb6a1a417752b5ce116c04) )
	ROM_CONTINUE ( 0x100001,0x20000)
	ROM_CONTINUE ( 0x040001,0x20000)
	ROM_CONTINUE ( 0x140001,0x20000)
	ROM_LOAD16_BYTE( "srom6.bin", 0x00000, 0x20000, CRC(31d5715d) SHA1(32612464124290b273c4b1a8b571291f9aeff01c) )
	ROM_CONTINUE ( 0x100000,0x20000)
	ROM_CONTINUE ( 0x040000,0x20000)
	ROM_CONTINUE ( 0x140000,0x20000)

	ROM_REGION( 0x200000, "gfx2", 0 ) /* sprites */
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

	ROM_REGION( 0x10000, "cpu2", 0 ) /* Intel 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x2000, NO_DUMP ) /* can't be dumped */

	ROM_REGION16_BE( 0x400, "user1", ROMREGION_ERASE00 ) /* Data from Shared RAM */
	/* this is not a real rom but instead the data extracted from shared ram, the MCU puts it there */
	/* It seems to put some data at 0x000-0x1ff too, but it looks like garbage, and doesn't appear to be used */
	ROM_LOAD16_WORD( "protdata.bin", 0x200, 0x200 , CRC(6064b9e0) SHA1(546bd8f9201427118940de2d1916b258b60710c6) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "uc1",    0x00000, 0x40000, CRC(d0f4c4ba) SHA1(669a04a977e98d8a594cc1621cbb9526c9081ec0) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "srom5",     0x00001, 0x40000, CRC(f1cdd21d) SHA1(0bfc09abce40712c4e95f13ad0d4b78684e44630) )
	ROM_IGNORE(0x40000)
	ROM_LOAD16_BYTE( "srom6",     0x00000, 0x40000, CRC(f848939e) SHA1(bf5e62300dd13a37f4715c67a2eec88034a94311) )
	ROM_IGNORE(0x40000)

	ROM_REGION( 0x200000, "gfx2", 0 ) /* sprites */
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

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
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

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "11.uk2", 0x000000, 0x80000, CRC(d006fadc) SHA1(79014bc0c7909763829ba02d5434d4543b4b80e5) ) // b
	ROM_LOAD16_BYTE( "12.uk3", 0x000001, 0x80000, CRC(2a2f4153) SHA1(d86692ee17ad052fdd8fccded57e3e30012026f6) ) // b
	ROM_LOAD16_BYTE( "5.uj2",  0x100000, 0x80000, CRC(f1246cbf) SHA1(de80a8f0d29ee76e11f38d9982ffcb4fd228153a) ) // b
	ROM_LOAD16_BYTE( "6.uj3",  0x100001, 0x80000, CRC(d346878c) SHA1(93174f6f6cc797323c5e429bf324d4ffe081f072) ) // b
	ROM_LOAD16_BYTE( "13.uk4", 0x200000, 0x80000, CRC(9bc40774) SHA1(b56c57258ec9c07c7efff9c0c632390d2d5ce4e2) ) // a
	ROM_LOAD16_BYTE( "14.uk5", 0x200001, 0x80000, CRC(a1e61674) SHA1(a5a50f479a019b39082429fa3425a95480838f84) ) // a
	ROM_LOAD16_BYTE( "7.uj4",  0x300000, 0x80000, CRC(dbdece8a) SHA1(20199cc915a1f8088372682c054cac69bc3b4918) ) // a
	ROM_LOAD16_BYTE( "8.uj5",  0x300001, 0x80000, CRC(60be7dd1) SHA1(d212dee3acf696cac0843e968a71ec1fb9b16dc9) ) // a
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

	ROM_REGION( 0x200000, "gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "5.srom5", 0x00001, 0x20000, CRC(f7f8c859) SHA1(28b21abfaff2b0502459d1e219c4397ca78a1495) )
	ROM_CONTINUE ( 0x100001,0x20000)
	ROM_CONTINUE ( 0x040001,0x20000)
	ROM_CONTINUE ( 0x140001,0x20000)
	ROM_LOAD16_BYTE( "6.srom6", 0x00000, 0x20000, CRC(34ace2a8) SHA1(f4ab3eceaacf65b5417edbb74b719f2993fa53ef) )
	ROM_CONTINUE ( 0x100000,0x20000)
	ROM_CONTINUE ( 0x040000,0x20000)
	ROM_CONTINUE ( 0x140000,0x20000)

	ROM_REGION( 0x200000, "gfx2", 0 ) /* sprites */
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

	ROM_REGION( 0x200000, "gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "srom5.bin", 0x00001, 0x20000, CRC(db8010c3) SHA1(db43d894d545a72e8da16555c54dcdbd89d87e3d) )
	ROM_CONTINUE ( 0x100001,0x20000)
	ROM_CONTINUE ( 0x040001,0x20000)
	ROM_CONTINUE ( 0x140001,0x20000)
	ROM_LOAD16_BYTE( "srom6.bin", 0x00000, 0x20000, CRC(2f364e54) SHA1(b0d3581567bd46565e9422f6bfb11273fc760fda) )
	ROM_CONTINUE ( 0x100000,0x20000)
	ROM_CONTINUE ( 0x040000,0x20000)
	ROM_CONTINUE ( 0x140000,0x20000)

	ROM_REGION( 0x200000, "gfx2", 0 ) /* sprites */
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

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
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

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
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
void tumblepb_patch_code(running_machine &machine, UINT16 offset)
{
	/* A hack which enables all Dip Switches effects */
	UINT16 *RAM = (UINT16 *)machine.region("maincpu")->base();
	RAM[(offset + 0)/2] = 0x0240;
	RAM[(offset + 2)/2] = 0xffff;	// andi.w  #$f3ff, D0
}
#endif


static void tumblepb_gfx1_rearrange(running_machine &machine)
{
	UINT8 *rom = machine.region("gfx1")->base();
	int len = machine.region("gfx1")->bytes();
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

static DRIVER_INIT( tumblepb )
{
	tumblepb_gfx1_rearrange(machine);

	#if TUMBLEP_HACK
	tumblepb_patch_code(machine, 0x000132);
	#endif
}

static DRIVER_INIT( tumbleb2 )
{
	device_t *oki = machine.device("oki");

	tumblepb_gfx1_rearrange(machine);

	#if TUMBLEP_HACK
	tumblepb_patch_code(machine, 0x000132);
	#endif

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(*oki, 0x100000, 0x100001, FUNC(tumbleb2_soundmcu_w) );

}

static DRIVER_INIT( jumpkids )
{
	tumblepb_gfx1_rearrange(machine);

	#if TUMBLEP_HACK
	tumblepb_patch_code(machine, 0x00013a);
	#endif
}

static DRIVER_INIT( fncywld )
{
	tumblepb_gfx1_rearrange(machine);

	#if FNCYWLD_HACK
	/* This is a hack to allow you to use the extra features
       of the 2 first "Unused" Dip Switch (see notes above). */
	UINT16 *RAM = (UINT16 *)machine.region("maincpu")->base();
	RAM[0x0005fa/2] = 0x4e71;
	RAM[0x00060a/2] = 0x4e71;
	#endif
}


READ16_MEMBER(tumbleb_state::bcstory_1a0_read)
{
	//mame_printf_debug("bcstory_io %06x\n",cpu_get_pc(&space.device()));

	if (cpu_get_pc(&space.device())==0x0560) return 0x1a0;
	else return input_port_read(machine(), "SYSTEM");
}

static DRIVER_INIT ( bcstory )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	tumblepb_gfx1_rearrange(machine);
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0x180008, 0x180009, read16_delegate(FUNC(tumbleb_state::bcstory_1a0_read),state)); // io should be here??
}


static DRIVER_INIT( htchctch )
{

	tumbleb_state *state = machine.driver_data<tumbleb_state>();
//  UINT16 *HCROM = (UINT16*)machine.region("maincpu")->base();
	UINT16 *PROTDATA = (UINT16*)machine.region("user1")->base();
	int i, len = machine.region("user1")->bytes();
	/* simulate RAM initialization done by the protection MCU */
	/* verified on real hardware */
//  static const UINT16 htchctch_mcu68k[] =
//  {
//      /* moved to protdata.bin file .. */
//  };


//  for (i = 0; i < sizeof(htchctch_mcu68k) / sizeof(htchctch_mcu68k[0]); i++)
//      state->m_mainram[0x000/2 + i] = htchctch_mcu68k[i];

	for (i = 0; i < len / 2; i++)
		state->m_mainram[0x000/2 + i] = PROTDATA[i];



	tumblepb_gfx1_rearrange(machine);

/* trojan.. */
#if 0
	/* patch the irq 6 vector */
	HCROM[0x00078/2] = 0x0001;
	HCROM[0x0007a/2] = 0xe000;

	/* our new interrupt code */

	/* put registers on stack */
	HCROM[0x1e000/2] = 0x48e7;
	HCROM[0x1e002/2] = 0xfffe;

	/* put the address we want to copy FROM in A0 */
	HCROM[0x1e004/2] = 0x41f9;
	HCROM[0x1e006/2] = 0x0012;
	HCROM[0x1e008/2] = 0x0000;

	/* put the address we want to copy TO in A1 */
	HCROM[0x1e00a/2] = 0x43f9;
	HCROM[0x1e00c/2] = 0x0012;
	HCROM[0x1e00e/2] = 0x2000;

	/* put the number of words we want to copy into D0 */
	HCROM[0x1e010/2] = 0x203c;
	HCROM[0x1e012/2] = 0x0000;
	HCROM[0x1e014/2] = 0x0100;

	/* copy a word */
	HCROM[0x1e016/2] = 0x32d8;

	/* decrease counter d0 */
	HCROM[0x1e018/2] = 0x5380;

	/* compare d0 to 0 */
	HCROM[0x1e01a/2] = 0x0c80;
	HCROM[0x1e01c/2] = 0x0000;
	HCROM[0x1e01e/2] = 0x0000;

	/* if its not 0 then branch back */
	HCROM[0x1e020/2] = 0x66f4;




	/* jump to drawing subroutine */
	HCROM[0x1e022/2] = 0x4eb9;
	HCROM[0x1e024/2] = 0x0001;
	HCROM[0x1e026/2] = 0xe100;

	/* get back registers from stack*/
	HCROM[0x1e028/2] = 0x4cdf;
	HCROM[0x1e02a/2] = 0x7fff;

	/* jump to where the interrupt vector was copied to */
	HCROM[0x1e02c/2] = 0x4ef9;
	HCROM[0x1e02e/2] = 0x0012;
	HCROM[0x1e030/2] = 0x2000;
	/* we're back in the game code */


	/* these subroutines are called from the new interrupt code above, i use them to draw */

	/* DRAWING SUBROUTINE */

	/* put the address we want to write to in A0 */
	HCROM[0x1e100/2] = 0x41f9;
	HCROM[0x1e102/2] = 0x0032;
	HCROM[0x1e104/2] = 0x0104;

	/* put the character we want to draw into D0 */
	/* this bit isn't needed .. we end up using d4 then copying it over */
	HCROM[0x1e106/2] = 0x203c;
	HCROM[0x1e108/2] = 0x0000;
	HCROM[0x1e10a/2] = 0x0007;

	/* put the address we to read to in A2 */
	HCROM[0x1e10c/2] = 0x45f9;
	HCROM[0x1e10e/2] = 0x0012;
//  HCROM[0x1e110/2] = 0x2000;
	HCROM[0x1e110/2] = 0x2000+0x60+0x60+0x60+0x60+0x60;

	/* put the number of rows into D3 */
	HCROM[0x1e112/2] = 0x263c;
	HCROM[0x1e114/2] = 0x0000;
	HCROM[0x1e116/2] = 0x000c;

	/* put the number of bytes per row into D2 */
	HCROM[0x1e118/2] = 0x243c;
	HCROM[0x1e11a/2] = 0x0000;
	HCROM[0x1e11c/2] = 0x0008;


	// move content of a2 to d4 (byte)
	HCROM[0x1e11e/2] = 0x1812;

	HCROM[0x1e120/2] = 0xe84c; // shift d4 right by 4

	HCROM[0x1e122/2] = 0x0244; // mask with 0x000f
	HCROM[0x1e124/2] = 0x000f; //

	HCROM[0x1e126/2] = 0x3004; // d4 -> d0

	/* jump to character draw to draw first bit */
	HCROM[0x1e128/2] = 0x4eb9;
	HCROM[0x1e12a/2] = 0x0001;
	HCROM[0x1e12c/2] = 0xe200;

	/* add 2 to draw address a0 */
	HCROM[0x1e12e/2] = 0xd1fc;
	HCROM[0x1e130/2] = 0x0000;
	HCROM[0x1e132/2] = 0x0002;


	// move content of a2 to d4 (byte)
	HCROM[0x1e134/2] = 0x1812;

	HCROM[0x1e136/2] = 0x0244; // mask with 0x000f
	HCROM[0x1e138/2] = 0x000f; //

	HCROM[0x1e13a/2] = 0x3004; // d4 -> d0

	/* jump to character draw to draw second bit */
	HCROM[0x1e13c/2] = 0x4eb9;
	HCROM[0x1e13e/2] = 0x0001;
	HCROM[0x1e140/2] = 0xe200;

	/* add 2 to draw address a0 */
	HCROM[0x1e142/2] = 0xd1fc;
	HCROM[0x1e144/2] = 0x0000;
	HCROM[0x1e146/2] = 0x0002;

	/* add 1 to read address a2 */
	HCROM[0x1e148/2] = 0xd5fc;
	HCROM[0x1e14a/2] = 0x0000;
	HCROM[0x1e14c/2] = 0x0001;

// brr
	/* decrease counter d2 */
	HCROM[0x1e14e/2] = 0x5382;

	/* compare d2 to 0 */
	HCROM[0x1e150/2] = 0x0c82;
	HCROM[0x1e152/2] = 0x0000;
	HCROM[0x1e154/2] = 0x0000;

	/* if its not 0 then branch back */
	HCROM[0x1e156/2] = 0x66c6;

	/* add 0xe0 to draw address a0 (0x100-0x20) */
	HCROM[0x1e158/2] = 0xd1fc;
	HCROM[0x1e15a/2] = 0x0000;
	HCROM[0x1e15c/2] = 0x00e0;

	/* decrease counter d2 */
	HCROM[0x1e15e/2] = 0x5383;

	/* compare d2 to 0 */
	HCROM[0x1e160/2] = 0x0c83;
	HCROM[0x1e162/2] = 0x0000;
	HCROM[0x1e164/2] = 0x0000;

	/* if its not 0 then branch back */
	HCROM[0x1e166/2] = 0x66b0;

	HCROM[0x1e168/2] = 0x4e75; // rts

	/* DRAW CHARACTER SUBROUTINE, note, this won't restore a1,d1, don't other places! */

	/* move address into A0->A1 for use by this subroutine */
	HCROM[0x1e200/2] = 0x2248;

	/* move address into D0->D1 for top half of character */
	HCROM[0x1e202/2] = 0x2200;

	/* add 0x30 to d1 to get the REAL tile code */
	HCROM[0x1e204/2] = 0x0681;
	HCROM[0x1e206/2] = 0x0000;
	HCROM[0x1e208/2] = 0x0030;

	/* or with 0xf000 to add the tile attribute */
	HCROM[0x1e20a/2] = 0x0081;
	HCROM[0x1e20c/2] = 0x0000;
	HCROM[0x1e20e/2] = 0xf000;

	/* write d1 -> a1 for TOP half */
	HCROM[0x1e210/2] = 0x32c1; // not ideal .. we don't need to increase a1

	/* move address into A0->A1 for use by this subroutine */
	HCROM[0x1e212/2] = 0x2248;

	/* add 0x80 to the address so we have the bottom location */
	HCROM[0x1e214/2] = 0xd2fc;
	HCROM[0x1e216/2] = 0x0080;

	/* move address into D0->D1 for bottom  half of character */
	HCROM[0x1e218/2] = 0x2200;

	/* add 0x54 to d1 to get the REAL tile code for bottom half */
	HCROM[0x1e21a/2] = 0x0681;
	HCROM[0x1e21c/2] = 0x0000;
	HCROM[0x1e21e/2] = 0x0054;

	/* or with 0xf000 to add the tile attribute */
	HCROM[0x1e220/2] = 0x0081;
	HCROM[0x1e222/2] = 0x0000;
	HCROM[0x1e224/2] = 0xf000;

	/* write d1 -> a1 for BOTTOM half */
	HCROM[0x1e226/2] = 0x32c1; // not ideal .. we don't need to increase a1


	HCROM[0x1e228/2] = 0x4e75;

	machine.device("maincpu")->memory().space(AS_PROGRAM)->nop_write(0x140000, 0x1407ff); // kill palette writes as the interrupt code we don't have controls them


	{
		FILE *fp;

		fp=fopen("hcatch", "w+b");
		if (fp)
		{
			fwrite(HCROM, 0x40000, 1, fp);
			fclose(fp);
		}
	}
#endif

}


static void suprtrio_decrypt_code(running_machine &machine)
{
	UINT16 *rom = (UINT16 *)machine.region("maincpu")->base();
	UINT16 *buf = auto_alloc_array(machine, UINT16, 0x80000/2);
	int i;

	/* decrypt main ROMs */
	memcpy(buf, rom, 0x80000);
	for (i = 0; i < 0x40000; i++)
	{
		int j = i ^ 0x06;
		if ((i & 1) == 0) j ^= 0x02;
		if ((i & 3) == 0) j ^= 0x08;
		rom[i] = buf[j];
	}
	auto_free(machine, buf);
}

static void suprtrio_decrypt_gfx(running_machine &machine)
{
	UINT16 *rom = (UINT16 *)machine.region("gfx1")->base();
	UINT16 *buf = auto_alloc_array(machine, UINT16, 0x100000/2);
	int i;

	/* decrypt tiles */
	memcpy(buf, rom, 0x100000);
	for (i = 0; i < 0x80000; i++)
	{
		int j = i ^ 0x02;
		if (i & 1) j ^= 0x04;
		rom[i] = buf[j];
	}
	auto_free(machine, buf);
}

static DRIVER_INIT( suprtrio )
{
	suprtrio_decrypt_code(machine);
	suprtrio_decrypt_gfx(machine);
}

static DRIVER_INIT( chokchok )
{
	DRIVER_INIT_CALL(htchctch);

	/* different palette format, closer to tumblep -- is this controlled by a register? the palette was right with the hatch catch trojan */
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x140000, 0x140fff, write16_delegate(FUNC(tumbleb_state::paletteram16_xxxxBBBBGGGGRRRR_word_w), state));

	/* slightly different banking */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x100002, 0x100003, write16_delegate(FUNC(tumbleb_state::chokchok_tilebank_w),state));
}

static DRIVER_INIT( wlstar )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	tumblepb_gfx1_rearrange(machine);

	/* slightly different banking */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x100002, 0x100003, write16_delegate(FUNC(tumbleb_state::wlstar_tilebank_w),state));

	state->m_protbase = 0x0000;
}

static DRIVER_INIT( wondl96 )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	DRIVER_INIT_CALL( wlstar );
	state->m_protbase = 0x0200;
}

static DRIVER_INIT ( dquizgo )
{
	tumblepb_gfx1_rearrange(machine);
}


/******************************************************************************/

GAME( 1991, tumbleb,  tumblep, tumblepb,    tumblepb, tumblepb, ROT0, "bootleg", "Tumble Pop (bootleg set 1)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE  )
GAME( 1991, tumbleb2, tumblep, tumbleb2,    tumblepb, tumbleb2, ROT0, "bootleg", "Tumble Pop (bootleg set 2)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE  ) // PIC is protected, sound simulation not 100%
GAME( 1993, jumpkids, 0,       jumpkids,    tumblepb, jumpkids, ROT0, "Comad",    "Jump Kids", GAME_SUPPORTS_SAVE )
GAME( 1994, metlsavr, 0,       metlsavr,    metlsavr, chokchok, ROT0, "First Amusement", "Metal Saver", GAME_SUPPORTS_SAVE )
GAME( 1994, pangpang, 0,       pangpang,    tumblepb, tumbleb2, ROT0, "Dong Gue La Mi Ltd.", "Pang Pang", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE  ) // PIC is protected, sound simulation not 100%
GAME( 1994, suprtrio, 0,       suprtrio,    suprtrio, suprtrio, ROT0, "Gameace", "Super Trio", GAME_SUPPORTS_SAVE )
GAME( 1996, fncywld,  0,       fncywld,     fncywld,  fncywld,  ROT0, "Unico",   "Fancy World - Earth of Crisis" , GAME_SUPPORTS_SAVE ) // game says 1996, testmode 1995?
GAME( 2001, jumppop,  0,       jumppop,     jumppop,  0,        ORIENTATION_FLIP_X, "ESD", "Jumping Pop", GAME_SUPPORTS_SAVE )

// Should also be 'Magicball Fighting' (c)1994
GAME( 1995, wlstar,   0,       cookbib_mcu, wlstar,   wlstar,   ROT0, "Mijin",   "Wonder League Star - Sok-Magicball Fighting (Korea)", GAME_SUPPORTS_SAVE ) // translates to 'Wonder League Star - Return of Magicball Fighting'
GAME( 1995, htchctch, 0,       htchctch,    htchctch, htchctch, ROT0, "SemiCom", "Hatch Catch" , GAME_SUPPORTS_SAVE ) // not 100% sure about gfx offsets
GAME( 1995, cookbib,  0,       cookbib,     cookbib,  htchctch, ROT0, "SemiCom", "Cookie & Bibi" , GAME_SUPPORTS_SAVE ) // not 100% sure about gfx offsets
GAME( 1995, chokchok, 0,       cookbib,     chokchok, chokchok, ROT0, "SemiCom", "Choky! Choky!", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE  ) // corruption during attract mode (tmap disable?)
GAME( 1996, wondl96,  0,       cookbib_mcu, wondl96,  wondl96,  ROT0, "SemiCom", "Wonder League '96 (Korea)", GAME_SUPPORTS_SAVE )
GAME( 1996, sdfight,  0,       sdfight,     sdfight,  bcstory,  ROT0, "SemiCom", "SD Fighters (Korea)", GAME_SUPPORTS_SAVE )
GAME( 1997, bcstry,   0,       bcstory,     bcstory,  bcstory,  ROT0, "SemiCom", "B.C. Story (set 1)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE ) // gfx offsets?
GAME( 1997, bcstrya,  bcstry,  bcstory,     bcstory,  bcstory,  ROT0, "SemiCom", "B.C. Story (set 2)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE ) // gfx offsets?
GAME( 1997, semibase, 0,       semibase,    semibase, bcstory,  ROT0, "SemiCom", "MuHanSeungBu (SemiCom Baseball) (Korea)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )// sprite offsets..
GAME( 1998, dquizgo,  0,       cookbib,     dquizgo,  dquizgo,  ROT0, "SemiCom", "Date Quiz Go Go (Korea)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE ) // check layer offsets
