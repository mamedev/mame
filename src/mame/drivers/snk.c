// license:???
// copyright-holders:Marco Cassili,Ernesto Corvi,Tim Lindquist,Carlos A. Lozano,Bryan McPhail,Jarek Parchanski,Nicola Salmoria,Tomasz Slanina,Phil Stroffolino,Acho A. Tang,Victor Trucco
/*

snk.c

various SNK triple Z80 games

ay8910x2 + "SNK wave"
Marvin's Maze, Vanguard II, Mad Crasher

ay8910x2
Jumping Cross, Gladiator 1984, HAL21

ym3526
ASO, Tank

ym3526x2
Athena, Ikari

ym3526 + y8950
Victory Road, Psycho Soldier, Bermuda Triangle, Touchdown Fever, Guerrilla War

ym3812 + y8950
Chopper1

ym3812
Fighting Golf

y8950
Fighting Soccer


Credits (in alphabetical order)
    Marco Cassili
    Ernesto Corvi
    Tim Lindquist
    Carlos A. Lozano
    Bryan McPhail
    Jarek Parchanski
    Nicola Salmoria
    Tomasz Slanina
    Phil Stroffolino
    Acho A. Tang
    Victor Trucco


Notes:
------

- How to enter test mode:
  1983 marvins: n/a
  1984 madcrash: keep F2 pressed during boot
  1984 vangrd2: n/a
  1984 jcross: n/a
  1984 sgladiat: keep F2 pressed during boot
  1985 hal21: n/a
  1985 aso: keep 1 pressed during boot
  1985 tnk3: keep 1 pressed during boot
  1986 athena: keep 1 pressed during boot
  1986 ikari: keep 1 pressed during boot
  1986 victroad: keep 1 pressed during boot
  1987 bermudat: keep 1 pressed during boot
  1987 worldwar: keep F2 pressed during boot
  1987 gwar: keep F2 pressed during boot
  1987 psychos: use the service mode dip switch
  1987 tdfever: keep F2 pressed during boot
  1988 tdfever2: keep F2 pressed during boot
  1988 chopper: keep F2 pressed during boot
  1988 fsoccer: use the service mode dip switch
  1988 fitegolf: use the service mode dip switch

- in all games except jcross, credits are edded on the 0->1 transition of the
  coin inputs. However declaring the inputs as ACTIVE_HIGH makes ikarijp
  enter test mode on boot, therefore I have to assume that ACTIVE_LOW is the
  correct setting and the games just wait for the pulse to finish before
  adding a credit.

- the I/O area (C000-CFFF) is probably mirrored in large part on the two main
  CPUs, however I mapped only the addresses actually used by the games.

- marvins: the fg and bg tilemaps are set to be 512x256 like in madcrash and
  vangrd2, but they could be 256x256.

- the Flip Screen dip switch in madcrash doesn't work correctly, the screen is
  inverted but the bg and sprite positions are wrong. This is a bug in the game;
  the Cocktail dip switch works correctly and the screen is properly flipped
  when player 2 plays.

- madcrash fails the ROM test, but ROMs are verified to be good so it looks like
  a bug. The service mode functionality is very limited anyway. To get past the
  failed ROM test and see the service mode, use this ROM patch:

    UINT8 *mem = machine.root_device().memregion("maincpu")->base();
    mem[0x3a5d] = mem[0x3a5e] = mem[0x3a5f] = 0;

- The "SNK Wave" custom sound circuitry is only actually used by marvins and
  vangrd2. The madcrash pcb probably still has it, while later boards like jcross
  might not have it at all.

- sgladiat runs on a modified jcross pcb (same pcb ID with flying wires).

- the original sgladiat pcb is verified to have huge sprite lag.

- RAM test fails in sgladiat when "Debug" Dip Switch is ON. Correct behaviour?

- the only difference between hal21 and hal21j is the audio ROM.

- there are no "Bonus Lives" settings in alphamis and arian (always 50k 100k)
  and only an "Occurrence" Dip Switch.

- when test mode displays 80k 160k in athena, it is in fact 60k 120k.

- the Fighting Golf flyer shows a different gameplay, where button A is pressed
  multiple times to select the spin. The flyer also says that the game "includes
  the unique feature of 2 distinct styles of game play, selectable by dipswitch
  setting. For the avid golfer, a more challenging style of swing.".
  This feature only exists when "Language" Dip Swicth is set to "English"

- fitegolfu:  An "SNK Fighting Golf Program Update" notice published in a trade
  journal outlines 3 improvements which is supposed to allow game players a bit
  more time at crucial points in the game.

  1) Shot time: The 12/10 seconds dip has been changed to 15/12 seconds.
  2) Power/Swing gauge moves slower when the ball is on the green.
  3) Hit Check area around the cup is enlarged for easier putting.

- there are two versions of the Ikari Warriors board, one has the standard JAMMA
  connector while the other has the custom SNK connector. The video and audio
  PCBs are the same, only the CPU PCB changes.

- gwara seemingly has a different video board. Fix chars and scroll registers are
  in different locations, while gwar (new) matches the bootleg and original
  japanese versions.

- there has been great confusion in the past about the four Bermuda Triangle /
  World Wars sets. World Wars was thought to be an earlier version, since it
  appears to have simpler gameplay and graphics. In reality, there is strong
  evidence that proves that World Wars is a follow-up to Bermuda Triangle.
  The gameplay is significantly different, and the only ROMs in common are the
  audio samples, so it cannot be considered a simple clone, even if bermudaa
  has the worldwar gameplay and the bermudat title screen!! What might have
  happened is that bermudat performed poorly in Japan/world arcades so it was
  redesigned before releasing it in the US/different parts of the world.
  Possibly the Bermuda Triangle title screen was kept for the US since it
  hand't been relased there yet, while it was changed to World Wars for places
  where it had already been released.

  The most compelling pieces of evidence to indicate which sets are newer are
  the following:
  - to enter test mode, bermudat/bermudaj use P1 start (old style: older games
    from tnk3 to victroad do it). worldwar/bermudaa use a dedicated service
    button (new style: newer games like gwar do it).
  - some of the tiles in the fg ROM (letters saying "standard", "power",
    "hyper", "safety", "danger") are present in all sets, but are actually used
    only in bermudat/bermudaj.
  - the worldwar test mode shows wrong descriptions for the dip switches, which
    are actually correct for bermudaj.

  So in the end the chronology of the four sets is:
  1) bermudat
  - Old gameplay
  - World version? (English speech)
  - 5 letters when entering initials, and "TOKYO" as default names
  - Test mode accessed using P1 start
  - There is bug in test mode: when the "Bonus Life" dip switches are set to
    "60k 120k", it shows "80000P 160000P".
  2) bermudaj
  - Old gameplay
  - Definitely Japan version (Japanese speech)
  - 5 letters when entering initials, and "TOKYO" as default names
  - Test mode accessed using P1 start
  - The test mode bug is fixed.
  3) worldwar
  - New gameplay
  - World version?
  - 5 letters when entering initials, and "WORLD" as default names
  - Test mode accessed using dedicated service button
  - Dip switches description in test mode are wrong: they are the same as
    bermudaj, even if the actual behaviour has changed.
  4) bermudaa
  - New gameplay
  - US version
  - 3 letters when entering initials, and "SNK  " as default names
  - Test mode accessed using dedicated service button
  - Dip switches descriptions updated.

- ikari/victroad do a "front hard check" during boot. This appears to be
  hardware bounds checking test which is also used during the game.
  However the game also does software collision detection, because making
  the test always return true is enough to make the game work.

- bermudat/bermudaj do a "turbo front check" during boot, which is missing in
  bermudaa/worldwar. Perhaps they realised that as a protection device it was
  useless since the game doesn't use the hardware after the check.

- tdfever2: though the gfx appear strange on the vs. screens / choose number of
  players screens eg. the numbers aren't in the boxes, they're in the lower
  right corners, and there is no background color like there is in tdfever, this
  is not a bug, it is confirmed to be correct against the pcb.

- the gwarb bootleg has two Y8950 instead of YM3526+Y8950. However, the sound
  program ROM is identical to gwar, so the replacement Y8950 is definitely used
  only in its YM3526 compatible part, without using its additional ADPCM
  capabilities. It should be noted that the gwara sound program has a small
  patch that apparently would set two extra bits in register 04 which should
  only exist in the Y8950, not in the YM3526.

- while tdfever2 does write to the addresses where the YM3526 is mapped in
  tdfever, those seem to be bogus writes since it causes sound to malfunction
  (a continuous scale is played by the Y8950). Taking the YM3526 out appears to
  fix the problem since all game sounds are apparently played by the Y8950 alone.

- fsoccerb is based on fsoccerj : the only differences are joysticks hacks
  (code patched at 0x00f1 + extra code at 0xb700 and after).


TODO:
-----

- madcrash shadows change color, however this should be the correct behaviour
  as far as I can tell. Should be verified on the pcb.

- one unknown dip switch in madcrash. Also the Difficulty dip switch is not
  verified (though it's listed in the manual).

- jcross might be a bad dump. The current ROM set is made by mixing two sets
  which were marked as 'bad'.

- jcross: only the first bank of the fg charset is used. The second bank is
  almost identical apart from a few characters. Should it be used? When?

- sgladiat: unknown writes to D200/DA00, probably video related. Also some bits
  of A600 are unknown.

- one unknown dip switch in sgladiat.

- hal21: unknown sound writes to E002.

- hal21: when the flip screen dip switch is on, the game screen is correctly
  flipped, but the title screen remains upside down (and sprites are displayed
  in the wrong position). This looks like a bug in the game.

- ASO: unknown writes to CE00, probably video related. Always 05?
  Also unknown writes to F002 by the sound CPU, during reset.

- Fighting Golf: unknown writes to CF00, probably video related.

- ikari/victroad: unknown writes to C980. This is probably (also) related to the
  color used to draw the FG layer, and supporting it is needed to fix the color
  test in ikaria/ikarijp.

- gwar: unknown writes to CA00/CA40. Always 0?

- tdfever/fsoccer: the dots in the radar flicker. In fsoccer, this is greatly
  improved by forcing partial screen updates when the sprite RAM is changed (see
  snk68.c for another game that needs this). tdfever dots still flicker a lot,
  however I'm not sure if this is an emulation bug or the real game behaviour.

- psychos: the pcb has glitches (colored lines of length up to 16 pixels) during
  scrolling on the left side of the screen. This doesn't happen in the emulation
  so it might be an unemulated  hardware bug.

- worldwar: at the beginning of the game, the bottom 16 pixels of the bg are
  blank. I think that this is related to the psychos glitch above, i.e. the pcb
  wouldn't display that area correctly anyway so operators were supposed to
  adjust the screen size to make them invisible.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/snk.h"
#include "sound/snkwave.h"
#include "sound/ay8910.h"
#include "sound/3526intf.h"
#include "sound/3812intf.h"
#include "sound/8950intf.h"


/*********************************************************************/
// Interrupt handlers common to all SNK triple Z80 games

READ8_MEMBER(snk_state::snk_cpuA_nmi_trigger_r)
{
	if(!space.debugger_access())
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
	return 0xff;
}

WRITE8_MEMBER(snk_state::snk_cpuA_nmi_ack_w)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

READ8_MEMBER(snk_state::snk_cpuB_nmi_trigger_r)
{
	if(!space.debugger_access())
	{
		m_subcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
	return 0xff;
}

WRITE8_MEMBER(snk_state::snk_cpuB_nmi_ack_w)
{
	m_subcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

/*********************************************************************/

enum
{
	YM1IRQ_ASSERT,
	YM1IRQ_CLEAR,
	YM2IRQ_ASSERT,
	YM2IRQ_CLEAR,
	CMDIRQ_BUSY_ASSERT,
	BUSY_CLEAR,
	CMDIRQ_CLEAR
};

/*********************************************************************/

WRITE8_MEMBER(snk_state::marvins_soundlatch_w)
{
	m_marvins_sound_busy_flag = 1;
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

READ8_MEMBER(snk_state::marvins_soundlatch_r)
{
	m_marvins_sound_busy_flag = 0;
	return soundlatch_byte_r(space, 0);
}

CUSTOM_INPUT_MEMBER(snk_state::marvins_sound_busy)
{
	return m_marvins_sound_busy_flag;
}

READ8_MEMBER(snk_state::marvins_sound_nmi_ack_r)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return 0xff;
}

/*********************************************************************/

TIMER_CALLBACK_MEMBER(snk_state::sgladiat_sndirq_update_callback)
{
	switch(param)
	{
		case CMDIRQ_BUSY_ASSERT:
			m_sound_status |= 8|4;
			break;

		case BUSY_CLEAR:
			m_sound_status &= ~4;
			break;

		case CMDIRQ_CLEAR:
			m_sound_status &= ~8;
			break;
	}

	m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_sound_status & 0x8) ? ASSERT_LINE : CLEAR_LINE);
}


WRITE8_MEMBER(snk_state::sgladiat_soundlatch_w)
{
	soundlatch_byte_w(space, offset, data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sgladiat_sndirq_update_callback),this), CMDIRQ_BUSY_ASSERT);
}

READ8_MEMBER(snk_state::sgladiat_soundlatch_r)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sgladiat_sndirq_update_callback),this), BUSY_CLEAR);
	return soundlatch_byte_r(space,0);
}

READ8_MEMBER(snk_state::sgladiat_sound_nmi_ack_r)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sgladiat_sndirq_update_callback),this), CMDIRQ_CLEAR);
	return 0xff;
}

READ8_MEMBER(snk_state::sgladiat_sound_irq_ack_r)
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	return 0xff;
}


/*********************************************************************

    All the later games (from athena onwards) have the same sound status flag handling.

    This 4 bit register is mapped at 0xf800.

    Writes to this register always contain 0x0f in the lower nibble.
    The upper nibble contains a mask, which clears bits

    bit 0:  irq request from first YM chip
    bit 1:  irq request from second YM chip
    bit 2:  sound cpu busy
    bit 3:  sound command pending

    The main CPU can read the busy flag using an input port.

    The earlier games are different. E.g. in tnk3 there are only three status
    bits since there is only one YM chip, and the bits are cleared using
    separate memory addresses. Additionally, clearing the cmd irq also
    clears the sound latch.

*********************************************************************/

TIMER_CALLBACK_MEMBER(snk_state::sndirq_update_callback)
{
	switch(param)
	{
		case YM1IRQ_ASSERT:
			m_sound_status |= 1;
			break;

		case YM1IRQ_CLEAR:
			m_sound_status &= ~1;
			break;

		case YM2IRQ_ASSERT:
			m_sound_status |= 2;
			break;

		case YM2IRQ_CLEAR:
			m_sound_status &= ~2;
			break;

		case CMDIRQ_BUSY_ASSERT:
			m_sound_status |= 8|4;
			break;

		case BUSY_CLEAR:
			m_sound_status &= ~4;
			break;

		case CMDIRQ_CLEAR:
			m_sound_status &= ~8;
			break;
	}

	m_audiocpu->set_input_line(0, (m_sound_status & 0xb) ? ASSERT_LINE : CLEAR_LINE);
}



WRITE_LINE_MEMBER(snk_state::ymirq_callback_1 )
{
	if (state)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sndirq_update_callback),this), YM1IRQ_ASSERT);
}

WRITE_LINE_MEMBER(snk_state::ymirq_callback_2)
{
	if (state)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sndirq_update_callback),this), YM2IRQ_ASSERT);
}



WRITE8_MEMBER(snk_state::snk_soundlatch_w)
{
	soundlatch_byte_w(space, offset, data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sndirq_update_callback),this), CMDIRQ_BUSY_ASSERT);
}

CUSTOM_INPUT_MEMBER(snk_state::snk_sound_busy)
{
	return (m_sound_status & 4) ? 1 : 0;
}



READ8_MEMBER(snk_state::snk_sound_status_r)
{
	return m_sound_status;
}

WRITE8_MEMBER(snk_state::snk_sound_status_w)
{
	if (~data & 0x10)   // ack YM1 irq
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sndirq_update_callback),this), YM1IRQ_CLEAR);

	if (~data & 0x20)   // ack YM2 irq
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sndirq_update_callback),this), YM2IRQ_CLEAR);

	if (~data & 0x40)   // clear busy flag
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sndirq_update_callback),this), BUSY_CLEAR);

	if (~data & 0x80)   // ack command from main cpu
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sndirq_update_callback),this), CMDIRQ_CLEAR);
}



READ8_MEMBER(snk_state::tnk3_cmdirq_ack_r)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sndirq_update_callback),this), CMDIRQ_CLEAR);
	return 0xff;
}

READ8_MEMBER(snk_state::tnk3_ymirq_ack_r)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sndirq_update_callback),this), YM1IRQ_CLEAR);
	return 0xff;
}

READ8_MEMBER(snk_state::tnk3_busy_clear_r)
{
	// it's uncertain whether the latch should be cleared here or when it's read
	soundlatch_clear_byte_w(space, 0, 0);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(snk_state::sndirq_update_callback),this), BUSY_CLEAR);
	return 0xff;
}



/*****************************************************************************

"Hard Flags" simulation

Apparently this is just a hardware bounds checking test.

The details might be slightly wrong, however this implementation passes the
startup checks, and is good enough to make collision detection work in the
game, so it cannot be far from correct (e.g. inverting the bit order in
hardflags_check8 will make collision detection not work).

In particular, the hf_size handling might be wrong.

A trojan could be used on the board to verify the exact behaviour.

*****************************************************************************/


WRITE8_MEMBER(snk_state::hardflags_scrollx_w)
{
	m_hf_posx = (m_hf_posx & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::hardflags_scrolly_w)
{
	m_hf_posy = (m_hf_posy & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::hardflags_scroll_msb_w)
{
	m_hf_posx = (m_hf_posx & 0xff) | ((data & 0x80) << 1);
	m_hf_posy = (m_hf_posy & 0xff) | ((data & 0x40) << 2);

	// low 6 bits might indicate radius, but it's not clear
}

int snk_state::hardflags_check(int num)
{
	const UINT8 *sr = &m_spriteram[0x800 + 4*num];
	int x = sr[2] + ((sr[3] & 0x80) << 1);
	int y = sr[0] + ((sr[3] & 0x10) << 4);

	int dx = (x - m_hf_posx) & 0x1ff;
	int dy = (y - m_hf_posy) & 0x1ff;

	if (dx > 0x20 && dx <= 0x1e0 && dy > 0x20 && dy <= 0x1e0)
		return 0;
	else
		return 1;
}

int snk_state::hardflags_check8(int num)
{
	return
		(hardflags_check(num + 0) << 0) |
		(hardflags_check(num + 1) << 1) |
		(hardflags_check(num + 2) << 2) |
		(hardflags_check(num + 3) << 3) |
		(hardflags_check(num + 4) << 4) |
		(hardflags_check(num + 5) << 5) |
		(hardflags_check(num + 6) << 6) |
		(hardflags_check(num + 7) << 7);
}

READ8_MEMBER(snk_state::hardflags1_r){ return hardflags_check8(0*8); }
READ8_MEMBER(snk_state::hardflags2_r){ return hardflags_check8(1*8); }
READ8_MEMBER(snk_state::hardflags3_r){ return hardflags_check8(2*8); }
READ8_MEMBER(snk_state::hardflags4_r){ return hardflags_check8(3*8); }
READ8_MEMBER(snk_state::hardflags5_r){ return hardflags_check8(4*8); }
READ8_MEMBER(snk_state::hardflags6_r){ return hardflags_check8(5*8); }
READ8_MEMBER(snk_state::hardflags7_r)
{
	// apparently the startup tests use bits 0&1 while the game uses bits 4&5
	return
		(hardflags_check(6*8 + 0) << 0) |
		(hardflags_check(6*8 + 1) << 1) |
		(hardflags_check(6*8 + 0) << 4) |
		(hardflags_check(6*8 + 1) << 5);
}



/*****************************************************************************

"Turbo Front" simulation

This is used by Bermuda Triangle, and appears to be similar to Ikari Warriors.
In this case, however, the test is only done during boot and not used at all
in the game.

The simulation is modeled on the ikari one, however all we can do is that it
passes the startup checks. There is not enough evidence to infer more.

A trojan could be used on the board to verify the exact behaviour.

*****************************************************************************/


WRITE8_MEMBER(snk_state::turbocheck16_1_w)
{
	m_tc16_posy = (m_tc16_posy & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::turbocheck16_2_w)
{
	m_tc16_posx = (m_tc16_posx & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::turbocheck32_1_w)
{
	m_tc32_posy = (m_tc32_posy & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::turbocheck32_2_w)
{
	m_tc32_posx = (m_tc32_posx & ~0xff) | data;
}

WRITE8_MEMBER(snk_state::turbocheck_msb_w)
{
	m_tc16_posx = (m_tc16_posx & 0xff) | ((data & 0x80) << 1);
	m_tc16_posy = (m_tc16_posy & 0xff) | ((data & 0x40) << 2);
	m_tc32_posx = (m_tc32_posx & 0xff) | ((data & 0x80) << 1);
	m_tc32_posy = (m_tc32_posy & 0xff) | ((data & 0x40) << 2);

	// low 6 bits might indicate radius, but it's not clear
}

int snk_state::turbofront_check(int small, int num)
{
	const UINT8 *sr = &m_spriteram[0x800*small + 4*num];
	int x = sr[2] + ((sr[3] & 0x80) << 1);
	int y = sr[0] + ((sr[3] & 0x10) << 4);

	int dx = (x - (small ? m_tc16_posx : m_tc32_posx)) & 0x1ff;
	int dy = (y - (small ? m_tc16_posy : m_tc32_posy)) & 0x1ff;

	if (dx > 0x20 && dx <= 0x1e0 && dy > 0x20 && dy <= 0x1e0)
		return 0;
	else
		return 1;
}

int snk_state::turbofront_check8(int small, int num)
{
	return
		(turbofront_check(small, num + 0) << 0) |
		(turbofront_check(small, num + 1) << 1) |
		(turbofront_check(small, num + 2) << 2) |
		(turbofront_check(small, num + 3) << 3) |
		(turbofront_check(small, num + 4) << 4) |
		(turbofront_check(small, num + 5) << 5) |
		(turbofront_check(small, num + 6) << 6) |
		(turbofront_check(small, num + 7) << 7);
}

READ8_MEMBER(snk_state::turbocheck16_1_r){ return turbofront_check8(1, 0*8); }
READ8_MEMBER(snk_state::turbocheck16_2_r){ return turbofront_check8(1, 1*8); }
READ8_MEMBER(snk_state::turbocheck16_3_r){ return turbofront_check8(1, 2*8); }
READ8_MEMBER(snk_state::turbocheck16_4_r){ return turbofront_check8(1, 3*8); }
READ8_MEMBER(snk_state::turbocheck16_5_r){ return turbofront_check8(1, 4*8); }
READ8_MEMBER(snk_state::turbocheck16_6_r){ return turbofront_check8(1, 5*8); }
READ8_MEMBER(snk_state::turbocheck16_7_r){ return turbofront_check8(1, 6*8); }
READ8_MEMBER(snk_state::turbocheck16_8_r){ return turbofront_check8(1, 7*8); }
READ8_MEMBER(snk_state::turbocheck32_1_r){ return turbofront_check8(0, 0*8); }
READ8_MEMBER(snk_state::turbocheck32_2_r){ return turbofront_check8(0, 1*8); }
READ8_MEMBER(snk_state::turbocheck32_3_r){ return turbofront_check8(0, 2*8); }
READ8_MEMBER(snk_state::turbocheck32_4_r){ return turbofront_check8(0, 3*8); }



/*****************************************************************************

Guerrilla War protection

We add a 0xf value for 1 input read once every 8 rotations.
0xf isn't a valid direction, but avoids the "joystick error" protection
which happens when direction changes directly from 0x5<->0x6 8 times.
The rotary joystick is a mechanical 12-way positional switch, so what happens
is that occasionally while rotating the stick none of the switches will be
closed. The protection test verifies that this happens, to prevent replacement
of the rotary stick with a simple TTL counter.
Note that returning 0xf just once is enough to disable the test. On the other
hand, always returning 0xf inbetween valid values confuses the game.

*****************************************************************************/

CUSTOM_INPUT_MEMBER(snk_state::gwar_rotary)
{
	static const char *const ports[] = { "P1ROT", "P2ROT" };
	int which = (int)(FPTR)param;
	int value = ioport(ports[which])->read();

	if ((m_last_value[which] == 0x5 && value == 0x6) || (m_last_value[which] == 0x6 && value == 0x5))
	{
		if (!m_cp_count[which])
			value = 0xf;
		m_cp_count[which] = (m_cp_count[which] + 1) & 0x07;
	}
	m_last_value[which] = value;

	return value;
}

CUSTOM_INPUT_MEMBER(snk_state::gwarb_rotary)
{
	if (ioport("JOYSTICK_MODE")->read() == 1)
	{
		return gwar_rotary(field, param);
	}
	else
	{
		return 0x0f;
	}
}

/************************************************************************/


WRITE8_MEMBER(snk_state::athena_coin_counter_w)
{
	coin_counter_w(machine(), 0, ~data & 2);
	coin_counter_w(machine(), 1, ~data & 1);
}

WRITE8_MEMBER(snk_state::ikari_coin_counter_w)
{
	if (~data & 0x80)
	{
		coin_counter_w(machine(), 0, 1);
		coin_counter_w(machine(), 0, 0);
	}

	if (~data & 0x40)
	{
		coin_counter_w(machine(), 1, 1);
		coin_counter_w(machine(), 1, 0);
	}
}

WRITE8_MEMBER(snk_state::tdfever_coin_counter_w)
{
	coin_counter_w(machine(), 0, data & 1);
	coin_counter_w(machine(), 1, data & 2);
}

WRITE8_MEMBER(snk_state::countryc_trackball_w)
{
	m_countryc_trackball = data & 1;
}

CUSTOM_INPUT_MEMBER(snk_state::countryc_trackball_x)
{
	return ioport(m_countryc_trackball ? "TRACKBALLX2" : "TRACKBALLX1")->read();
}

CUSTOM_INPUT_MEMBER(snk_state::countryc_trackball_y)
{
	return ioport(m_countryc_trackball ? "TRACKBALLY2" : "TRACKBALLY1")->read();
}


/************************************************************************/

CUSTOM_INPUT_MEMBER(snk_state::snk_bonus_r)
{
	int bit_mask = (FPTR)param;

	switch (bit_mask)
	{
		case 0x01:  /* older games : "Occurrence" Dip Switch (DSW2:1) */
			return ((ioport("BONUS")->read() & bit_mask) >> 0);
		case 0xc0:  /* older games : "Bonus Life" Dip Switches (DSW1:7,8) */
			return ((ioport("BONUS")->read() & bit_mask) >> 6);

		case 0x04:  /* later games : "Occurrence" Dip Switch (DSW1:3) */
			return ((ioport("BONUS")->read() & bit_mask) >> 2);
		case 0x30:  /* later games : "Bonus Life" Dip Switches (DSW2:5,6) */
			return ((ioport("BONUS")->read() & bit_mask) >> 4);

		default:
			logerror("snk_bonus_r : invalid %02X bit_mask\n",bit_mask);
			return 0;
	}
}

/************************************************************************/

static ADDRESS_MAP_START( marvins_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6000) AM_WRITE(marvins_palette_bank_w)
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("IN0")
	AM_RANGE(0x8100, 0x8100) AM_READ_PORT("IN1")
	AM_RANGE(0x8200, 0x8200) AM_READ_PORT("IN2")
	AM_RANGE(0x8300, 0x8300) AM_WRITE(marvins_soundlatch_w)
	AM_RANGE(0x8400, 0x8400) AM_READ_PORT("DSW1")
	AM_RANGE(0x8500, 0x8500) AM_READ_PORT("DSW2")
	AM_RANGE(0x8600, 0x8600) AM_WRITE(marvins_flipscreen_w)
	AM_RANGE(0x8700, 0x8700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xc000, 0xcfff) AM_RAM AM_SHARE("spriteram")   // + work ram
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(marvins_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("share5")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")    // + work RAM
	AM_RANGE(0xf800, 0xf800) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xf900, 0xf900) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(snk_fg_scrolly_w)
	AM_RANGE(0xfb00, 0xfb00) AM_WRITE(snk_fg_scrollx_w)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xfd00, 0xfd00) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xfe00, 0xfe00) AM_WRITE(snk_sprite_split_point_w)
	AM_RANGE(0xff00, 0xff00) AM_WRITE(marvins_scroll_msb_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( marvins_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8700, 0x8700) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)
	AM_RANGE(0xc000, 0xcfff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(marvins_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("share5")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
	AM_RANGE(0xf800, 0xf800) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xf900, 0xf900) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(snk_fg_scrolly_w)
	AM_RANGE(0xfb00, 0xfb00) AM_WRITE(snk_fg_scrollx_w)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xfd00, 0xfd00) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xfe00, 0xfe00) AM_WRITE(snk_sprite_split_point_w)
	AM_RANGE(0xff00, 0xff00) AM_WRITE(marvins_scroll_msb_w)
ADDRESS_MAP_END


// vangrd2 accesses video registers at xxF1 instead of xx00
static ADDRESS_MAP_START( madcrash_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("IN0")
	AM_RANGE(0x8100, 0x8100) AM_READ_PORT("IN1")
	AM_RANGE(0x8200, 0x8200) AM_READ_PORT("IN2")
	AM_RANGE(0x8300, 0x8300) AM_WRITE(marvins_soundlatch_w)
	AM_RANGE(0x8400, 0x8400) AM_READ_PORT("DSW1")
	AM_RANGE(0x8500, 0x8500) AM_READ_PORT("DSW2")
	AM_RANGE(0x8600, 0x8600) AM_MIRROR(0xff) AM_WRITE(marvins_flipscreen_w)
	AM_RANGE(0x8700, 0x8700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("spriteram") // + work ram
	AM_RANGE(0xc800, 0xc800) AM_MIRROR(0xff) AM_WRITE(marvins_palette_bank_w)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(marvins_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("share5")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")    // + work RAM
	AM_RANGE(0xf800, 0xf800) AM_MIRROR(0xff) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xf900, 0xf900) AM_MIRROR(0xff) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xfa00, 0xfa00) AM_MIRROR(0xff) AM_WRITE(snk_sprite_split_point_w)
	AM_RANGE(0xfb00, 0xfb00) AM_MIRROR(0xff) AM_WRITE(marvins_scroll_msb_w)
	AM_RANGE(0xfc00, 0xfc00) AM_MIRROR(0xff) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xfd00, 0xfd00) AM_MIRROR(0xff) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xfe00, 0xfe00) AM_MIRROR(0xff) AM_WRITE(snk_fg_scrolly_w)
	AM_RANGE(0xff00, 0xff00) AM_MIRROR(0xff) AM_WRITE(snk_fg_scrollx_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( madcrash_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x8700, 0x8700) AM_WRITE(snk_cpuB_nmi_ack_w)   // vangrd2
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_WRITE(snk_cpuB_nmi_ack_w)   // madcrash
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(marvins_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_SHARE("share5")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
	AM_RANGE(0xd800, 0xd800) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xd900, 0xd900) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xda00, 0xda00) AM_WRITE(snk_sprite_split_point_w)
	AM_RANGE(0xdb00, 0xdb00) AM_WRITE(marvins_scroll_msb_w)
	AM_RANGE(0xdc00, 0xdc00) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xdd00, 0xdd00) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xde00, 0xde00) AM_WRITE(snk_fg_scrolly_w)
	AM_RANGE(0xdf00, 0xdf00) AM_WRITE(snk_fg_scrollx_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("share3")
ADDRESS_MAP_END

static ADDRESS_MAP_START( madcrush_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("IN0")
	AM_RANGE(0x8100, 0x8100) AM_READ_PORT("IN1")
	AM_RANGE(0x8200, 0x8200) AM_READ_PORT("IN2")
	AM_RANGE(0x8300, 0x8300) AM_WRITE(marvins_soundlatch_w)
	AM_RANGE(0x8400, 0x8400) AM_READ_PORT("DSW1")
	AM_RANGE(0x8500, 0x8500) AM_READ_PORT("DSW2")
	AM_RANGE(0x8600, 0x8600) AM_MIRROR(0xff) AM_WRITE(marvins_flipscreen_w)
	AM_RANGE(0x8700, 0x8700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("spriteram") // + work ram
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(marvins_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xc800, 0xc800) AM_MIRROR(0xff) AM_WRITE(marvins_palette_bank_w)
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share5")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
	AM_RANGE(0xf800, 0xf800) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xf900, 0xf900) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(snk_fg_scrolly_w)
	AM_RANGE(0xfb00, 0xfb00) AM_WRITE(snk_fg_scrollx_w)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xfd00, 0xfd00) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xfe00, 0xfe00) AM_WRITE(snk_sprite_split_point_w)
	AM_RANGE(0xff00, 0xff00) AM_WRITE(marvins_scroll_msb_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( madcrush_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_WRITE(snk_cpuB_nmi_ack_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("spriteram")   // + work ram
	AM_RANGE(0xc800, 0xc800) AM_MIRROR(0xff) AM_WRITE(marvins_palette_bank_w)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(marvins_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_SHARE("share5")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram") // ??
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
	AM_RANGE(0xf800, 0xf800) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xf900, 0xf900) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(snk_fg_scrolly_w)
	AM_RANGE(0xfb00, 0xfb00) AM_WRITE(snk_fg_scrollx_w)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xfd00, 0xfd00) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xfe00, 0xfe00) AM_WRITE(snk_sprite_split_point_w)
	AM_RANGE(0xff00, 0xff00) AM_WRITE(marvins_scroll_msb_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( jcross_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa100, 0xa100) AM_READ_PORT("IN1")
	AM_RANGE(0xa200, 0xa200) AM_READ_PORT("IN2")
	AM_RANGE(0xa300, 0xa300) AM_WRITE(sgladiat_soundlatch_w)
	AM_RANGE(0xa400, 0xa400) AM_READ_PORT("DSW1")
	AM_RANGE(0xa500, 0xa500) AM_READ_PORT("DSW2")
	AM_RANGE(0xa600, 0xa600) AM_WRITE(sgladiat_flipscreen_w)    // flip screen, bg palette bank
	AM_RANGE(0xa700, 0xa700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xd300, 0xd300) AM_WRITE(jcross_scroll_msb_w)
	AM_RANGE(0xd400, 0xd400) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xd500, 0xd500) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xd600, 0xd600) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xd700, 0xd700) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("spriteram") // + work ram
	AM_RANGE(0xe000, 0xefff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")    // + work RAM
	AM_RANGE(0xffff, 0xffff) AM_WRITENOP    // simply a program patch to not write to two not existing video registers?
ADDRESS_MAP_END

static ADDRESS_MAP_START( jcross_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa700, 0xa700) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc800, 0xd7ff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( sgladiat_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa100, 0xa100) AM_READ_PORT("IN1")
	AM_RANGE(0xa200, 0xa200) AM_READ_PORT("IN2")
	AM_RANGE(0xa300, 0xa300) AM_WRITE(sgladiat_soundlatch_w)
	AM_RANGE(0xa400, 0xa400) AM_READ_PORT("DSW1")
	AM_RANGE(0xa500, 0xa500) AM_READ_PORT("DSW2")
	AM_RANGE(0xa600, 0xa600) AM_WRITE(sgladiat_flipscreen_w)    // flip screen, bg palette bank
	AM_RANGE(0xa700, 0xa700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xd200, 0xd200) AM_WRITENOP    // unknown
	AM_RANGE(0xd300, 0xd300) AM_WRITE(sgladiat_scroll_msb_w)
	AM_RANGE(0xd400, 0xd400) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xd500, 0xd500) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xd600, 0xd600) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xd700, 0xd700) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("spriteram") // + work ram
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xe800, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")    // + work RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sgladiat_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)
	AM_RANGE(0xa600, 0xa600) AM_WRITE(sgladiat_flipscreen_w)    // flip screen, bg palette bank
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xda00, 0xda00) AM_WRITENOP    // unknown
	AM_RANGE(0xdb00, 0xdb00) AM_WRITE(sgladiat_scroll_msb_w)
	AM_RANGE(0xdc00, 0xdc00) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xdd00, 0xdd00) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xde00, 0xde00) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xdf00, 0xdf00) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( hal21_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0")
	AM_RANGE(0xc100, 0xc100) AM_READ_PORT("IN1")
	AM_RANGE(0xc200, 0xc200) AM_READ_PORT("IN2")
	AM_RANGE(0xc300, 0xc300) AM_WRITE(sgladiat_soundlatch_w)
	AM_RANGE(0xc400, 0xc400) AM_READ_PORT("DSW1")
	AM_RANGE(0xc500, 0xc500) AM_READ_PORT("DSW2")
	AM_RANGE(0xc600, 0xc600) AM_WRITE(hal21_flipscreen_w)   // flip screen, bg tile and palette bank
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xd300, 0xd300) AM_WRITE(jcross_scroll_msb_w)
	AM_RANGE(0xd400, 0xd400) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xd500, 0xd500) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xd600, 0xd600) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xd700, 0xd700) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("spriteram") // + work ram
	AM_RANGE(0xe800, 0xf7ff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")    // + work RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hal21_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_WRITE(snk_cpuB_nmi_ack_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( aso_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0")
	AM_RANGE(0xc100, 0xc100) AM_READ_PORT("IN1")
	AM_RANGE(0xc200, 0xc200) AM_READ_PORT("IN2")
	AM_RANGE(0xc400, 0xc400) AM_WRITE(snk_soundlatch_w)
	AM_RANGE(0xc500, 0xc500) AM_READ_PORT("DSW1")
	AM_RANGE(0xc600, 0xc600) AM_READ_PORT("DSW2")
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(aso_videoattrs_w) // flip screen, scroll msb
	AM_RANGE(0xc900, 0xc900) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xca00, 0xca00) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xcb00, 0xcb00) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xcc00, 0xcc00) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xce00, 0xce00) AM_WRITENOP    // always 05?
	AM_RANGE(0xcf00, 0xcf00) AM_WRITE(aso_bg_bank_w)    // tile and palette bank
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("spriteram")   // + work ram
	AM_RANGE(0xe800, 0xf7ff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")    // + work RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( aso_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd800, 0xe7ff) AM_RAM_WRITE(marvins_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( tnk3_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0")
	AM_RANGE(0xc100, 0xc100) AM_READ_PORT("IN1")
	AM_RANGE(0xc200, 0xc200) AM_READ_PORT("IN2")
	// c300 is an input in tnk3, output in athena/fitegolf (coin counter)
	// and in countryc (trackball select) (see DRIVER_INIT).
	AM_RANGE(0xc300, 0xc300) AM_READ_PORT("IN3") AM_WRITE(athena_coin_counter_w)
	AM_RANGE(0xc400, 0xc400) AM_WRITE(snk_soundlatch_w)
	AM_RANGE(0xc500, 0xc500) AM_READ_PORT("DSW1")
	AM_RANGE(0xc600, 0xc600) AM_READ_PORT("DSW2")
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(tnk3_videoattrs_w)    // flip screen, char bank, scroll msb
	AM_RANGE(0xc900, 0xc900) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xca00, 0xca00) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xcb00, 0xcb00) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xcc00, 0xcc00) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xcf00, 0xcf00) AM_WRITENOP    // fitegolf/countryc only. Either 0 or 1. Video related?
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_SHARE("spriteram") // + work ram
	AM_RANGE(0xd800, 0xf7ff) AM_RAM_WRITE(snk_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")    // + work RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tnk3_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)   // tnk3, athena
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)   // fitegolf
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xefff) AM_RAM_WRITE(snk_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( ikari_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0")
	AM_RANGE(0xc100, 0xc100) AM_READ_PORT("IN1")
	AM_RANGE(0xc200, 0xc200) AM_READ_PORT("IN2")
	AM_RANGE(0xc300, 0xc300) AM_READ_PORT("IN3") AM_WRITE(ikari_coin_counter_w) // ikarijp doesn't use the coin counter
	AM_RANGE(0xc400, 0xc400) AM_WRITE(snk_soundlatch_w)
	AM_RANGE(0xc500, 0xc500) AM_READ_PORT("DSW1")
	AM_RANGE(0xc600, 0xc600) AM_READ_PORT("DSW2")
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xc880, 0xc880) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xc900, 0xc900) AM_WRITE(ikari_bg_scroll_msb_w)
	AM_RANGE(0xc980, 0xc980) AM_WRITE(ikari_unknown_video_w)
	AM_RANGE(0xca00, 0xca00) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xca80, 0xca80) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xcb00, 0xcb00) AM_WRITE(snk_sp32_scrolly_w)
	AM_RANGE(0xcb80, 0xcb80) AM_WRITE(snk_sp32_scrollx_w)
	AM_RANGE(0xcc00, 0xcc00) AM_WRITE(hardflags_scrolly_w)
	AM_RANGE(0xcc80, 0xcc80) AM_WRITE(hardflags_scrollx_w)
	AM_RANGE(0xcd00, 0xcd00) AM_WRITE(ikari_sp_scroll_msb_w)
	AM_RANGE(0xcd80, 0xcd80) AM_WRITE(hardflags_scroll_msb_w)
	AM_RANGE(0xce00, 0xce00) AM_READ(hardflags1_r)
	AM_RANGE(0xce20, 0xce20) AM_READ(hardflags2_r)
	AM_RANGE(0xce40, 0xce40) AM_READ(hardflags3_r)
	AM_RANGE(0xce60, 0xce60) AM_READ(hardflags4_r)
	AM_RANGE(0xce80, 0xce80) AM_READ(hardflags5_r)
	AM_RANGE(0xcea0, 0xcea0) AM_READ(hardflags6_r)
	AM_RANGE(0xcee0, 0xcee0) AM_READ(hardflags7_r)
	// note the mirror. ikari and victroad use d800, ikarijp uses d000
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(snk_bg_videoram_w) AM_MIRROR(0x0800) AM_SHARE("bg_videoram")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("spriteram")   // + work ram
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")    // + work RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ikari_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)
	AM_RANGE(0xc980, 0xc980) AM_WRITE(ikari_unknown_video_w)
	AM_RANGE(0xcc00, 0xcc00) AM_WRITE(hardflags_scrolly_w)
	AM_RANGE(0xcc80, 0xcc80) AM_WRITE(hardflags_scrollx_w)
	AM_RANGE(0xcd80, 0xcd80) AM_WRITE(hardflags_scroll_msb_w)
	AM_RANGE(0xce00, 0xce00) AM_READ(hardflags1_r)
	AM_RANGE(0xce20, 0xce20) AM_READ(hardflags2_r)
	AM_RANGE(0xce40, 0xce40) AM_READ(hardflags3_r)
	AM_RANGE(0xce60, 0xce60) AM_READ(hardflags4_r)
	AM_RANGE(0xce80, 0xce80) AM_READ(hardflags5_r)
	AM_RANGE(0xcea0, 0xcea0) AM_READ(hardflags6_r)
	AM_RANGE(0xcee0, 0xcee0) AM_READ(hardflags7_r)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(snk_bg_videoram_w) AM_MIRROR(0x0800) AM_SHARE("bg_videoram")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( bermudat_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0")
	AM_RANGE(0xc100, 0xc100) AM_READ_PORT("IN1")
	AM_RANGE(0xc200, 0xc200) AM_READ_PORT("IN2")
	AM_RANGE(0xc300, 0xc300) AM_READ_PORT("IN3") AM_WRITE(ikari_coin_counter_w)
	AM_RANGE(0xc400, 0xc400) AM_WRITE(snk_soundlatch_w)
	AM_RANGE(0xc500, 0xc500) AM_READ_PORT("DSW1")
	AM_RANGE(0xc600, 0xc600) AM_READ_PORT("DSW2")
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xc840, 0xc840) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xc880, 0xc880) AM_WRITE(gwara_videoattrs_w)   // flip screen, scroll msb
	AM_RANGE(0xc8c0, 0xc8c0) AM_WRITE(gwar_tx_bank_w)   // char and palette bank
	AM_RANGE(0xc900, 0xc900) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xc940, 0xc940) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xc980, 0xc980) AM_WRITE(snk_sp32_scrolly_w)
	AM_RANGE(0xc9c0, 0xc9c0) AM_WRITE(snk_sp32_scrollx_w)
	// the "turbo check" addresses are only used by bermudat/bermudaj, not bermudaa/worldwar or the other games
	AM_RANGE(0xca00, 0xca00) AM_WRITE(turbocheck16_1_w)
	AM_RANGE(0xca40, 0xca40) AM_WRITE(turbocheck16_2_w)
	AM_RANGE(0xca80, 0xca80) AM_WRITE(gwara_sp_scroll_msb_w)
	AM_RANGE(0xcac0, 0xcac0) AM_WRITE(snk_sprite_split_point_w)
	AM_RANGE(0xcb00, 0xcb00) AM_READ(turbocheck16_1_r)
	AM_RANGE(0xcb10, 0xcb10) AM_READ(turbocheck16_2_r)
	AM_RANGE(0xcb20, 0xcb20) AM_READ(turbocheck16_3_r)
	AM_RANGE(0xcb30, 0xcb30) AM_READ(turbocheck16_4_r)
	AM_RANGE(0xcb40, 0xcb40) AM_READ(turbocheck16_5_r)
	AM_RANGE(0xcb50, 0xcb50) AM_READ(turbocheck16_6_r)
	AM_RANGE(0xcb60, 0xcb60) AM_READ(turbocheck16_7_r)
	AM_RANGE(0xcb70, 0xcb70) AM_READ(turbocheck16_8_r)
	AM_RANGE(0xcc00, 0xcc00) AM_WRITE(turbocheck32_1_w)
	AM_RANGE(0xcc40, 0xcc40) AM_WRITE(turbocheck32_2_w)
	AM_RANGE(0xcc80, 0xcc80) AM_WRITE(turbocheck_msb_w)
	AM_RANGE(0xccc0, 0xccc0) AM_READ(turbocheck32_1_r)
	AM_RANGE(0xccd0, 0xccd0) AM_READ(turbocheck32_2_r)
	AM_RANGE(0xcce0, 0xcce0) AM_READ(turbocheck32_3_r)
	AM_RANGE(0xccf0, 0xccf0) AM_READ(turbocheck32_4_r)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(snk_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("spriteram")   // + work ram
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")    // + work RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( bermudat_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xc840, 0xc840) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xc880, 0xc880) AM_WRITE(gwara_videoattrs_w)   // flip screen, scroll msb
	AM_RANGE(0xc8c0, 0xc8c0) AM_WRITE(gwar_tx_bank_w)   // char and palette bank
	AM_RANGE(0xc900, 0xc900) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xc940, 0xc940) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xc980, 0xc980) AM_WRITE(snk_sp32_scrolly_w)
	AM_RANGE(0xc9c0, 0xc9c0) AM_WRITE(snk_sp32_scrollx_w)
	AM_RANGE(0xca80, 0xca80) AM_WRITE(gwara_sp_scroll_msb_w)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(snk_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( gwar_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0")
	AM_RANGE(0xc100, 0xc100) AM_READ_PORT("IN1")
	AM_RANGE(0xc200, 0xc200) AM_READ_PORT("IN2")
	AM_RANGE(0xc300, 0xc300) AM_READ_PORT("IN3") AM_WRITE(ikari_coin_counter_w)
	AM_RANGE(0xc400, 0xc400) AM_WRITE(snk_soundlatch_w)
	AM_RANGE(0xc500, 0xc500) AM_READ_PORT("DSW1")
	AM_RANGE(0xc600, 0xc600) AM_READ_PORT("DSW2")
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xc840, 0xc840) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xc880, 0xc880) AM_WRITE(gwar_videoattrs_w)    // flip screen, scroll msb
	AM_RANGE(0xc8c0, 0xc8c0) AM_WRITE(gwar_tx_bank_w)   // char and palette bank
	AM_RANGE(0xc900, 0xc900) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xc940, 0xc940) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xc980, 0xc980) AM_WRITE(snk_sp32_scrolly_w)
	AM_RANGE(0xc9c0, 0xc9c0) AM_WRITE(snk_sp32_scrollx_w)
	AM_RANGE(0xca00, 0xca00) AM_WRITENOP    // always 0?
	AM_RANGE(0xca40, 0xca40) AM_WRITENOP    // always 0?
	AM_RANGE(0xcac0, 0xcac0) AM_WRITE(snk_sprite_split_point_w)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(snk_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("spriteram")   // + work ram
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")    // + work RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( gwar_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)
	AM_RANGE(0xc8c0, 0xc8c0) AM_WRITE(gwar_tx_bank_w)   // char and palette bank
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(snk_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( gwara_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0")
	AM_RANGE(0xc100, 0xc100) AM_READ_PORT("IN1")
	AM_RANGE(0xc200, 0xc200) AM_READ_PORT("IN2")
	AM_RANGE(0xc300, 0xc300) AM_READ_PORT("IN3") AM_WRITE(ikari_coin_counter_w)
	AM_RANGE(0xc400, 0xc400) AM_WRITE(snk_soundlatch_w)
	AM_RANGE(0xc500, 0xc500) AM_READ_PORT("DSW1")
	AM_RANGE(0xc600, 0xc600) AM_READ_PORT("DSW2")
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")    // + work RAM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(snk_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("spriteram")   // + work ram
	AM_RANGE(0xf800, 0xf800) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xf840, 0xf840) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xf880, 0xf880) AM_WRITE(gwara_videoattrs_w)   // flip screen, scroll msb
	AM_RANGE(0xf8c0, 0xf8c0) AM_WRITE(gwar_tx_bank_w)   // char and palette bank
	AM_RANGE(0xf900, 0xf900) AM_WRITE(snk_sp16_scrolly_w)
	AM_RANGE(0xf940, 0xf940) AM_WRITE(snk_sp16_scrollx_w)
	AM_RANGE(0xf980, 0xf980) AM_WRITE(snk_sp32_scrolly_w)
	AM_RANGE(0xf9c0, 0xf9c0) AM_WRITE(snk_sp32_scrollx_w)
	AM_RANGE(0xfa80, 0xfa80) AM_WRITE(gwara_sp_scroll_msb_w)
	AM_RANGE(0xfac0, 0xfac0) AM_WRITE(snk_sprite_split_point_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gwara_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(snk_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("spriteram")   // + work ram
	AM_RANGE(0xf8c0, 0xf8c0) AM_WRITE(gwar_tx_bank_w)   // char and palette bank
ADDRESS_MAP_END


static ADDRESS_MAP_START( tdfever_cpuA_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0")
	AM_RANGE(0xc080, 0xc080) AM_READ_PORT("IN1")
	AM_RANGE(0xc100, 0xc100) AM_READ_PORT("IN2")
	AM_RANGE(0xc180, 0xc180) AM_READ_PORT("IN3")
	AM_RANGE(0xc200, 0xc200) AM_READ_PORT("IN4")
	AM_RANGE(0xc280, 0xc280) AM_READ_PORT("IN5")
	AM_RANGE(0xc300, 0xc300) AM_READ_PORT("IN6")
	AM_RANGE(0xc380, 0xc380) AM_READ_PORT("IN7")
	AM_RANGE(0xc400, 0xc400) AM_READ_PORT("IN8")
	AM_RANGE(0xc480, 0xc480) AM_READ_PORT("IN9")
	AM_RANGE(0xc500, 0xc500) AM_WRITE(snk_soundlatch_w)
	AM_RANGE(0xc580, 0xc580) AM_READ_PORT("DSW1")
	AM_RANGE(0xc600, 0xc600) AM_READ_PORT("DSW2")
	AM_RANGE(0xc680, 0xc680) AM_WRITE(tdfever_coin_counter_w)
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(snk_bg_scrolly_w)
	AM_RANGE(0xc840, 0xc840) AM_WRITE(snk_bg_scrollx_w)
	AM_RANGE(0xc880, 0xc880) AM_WRITE(gwara_videoattrs_w)   // flip screen, scroll msb
	AM_RANGE(0xc8c0, 0xc8c0) AM_WRITE(gwar_tx_bank_w)   // char and palette bank
	AM_RANGE(0xc900, 0xc900) AM_WRITE(tdfever_sp_scroll_msb_w)
	AM_RANGE(0xc980, 0xc980) AM_WRITE(snk_sp32_scrolly_w)
	AM_RANGE(0xc9c0, 0xc9c0) AM_WRITE(snk_sp32_scrollx_w)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(snk_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM_WRITE(tdfever_spriteram_w) AM_SHARE("spriteram")    // + work ram
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")    // + work RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tdfever_cpuB_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)   // tdfever, tdfever2
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)   // fsoccer
	AM_RANGE(0xc8c0, 0xc8c0) AM_WRITE(gwar_tx_bank_w)   // char and palette bank
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(snk_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM_WRITE(tdfever_spriteram_w) AM_SHARE("spriteram")
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(snk_tx_videoram_w) AM_SHARE("tx_videoram")
ADDRESS_MAP_END

/***********************************************************************/

static ADDRESS_MAP_START( marvins_sound_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4000) AM_READ(marvins_soundlatch_r)
	AM_RANGE(0x8000, 0x8001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x8002, 0x8007) AM_DEVWRITE("wave", snkwave_device, snkwave_w)
	AM_RANGE(0x8008, 0x8009) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(marvins_sound_nmi_ack_r)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( marvins_sound_portmap, AS_IO, 8, snk_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READNOP // read on startup, then the Z80 automatically pulls down the IORQ pin to ack irq
ADDRESS_MAP_END


static ADDRESS_MAP_START( jcross_sound_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ(sgladiat_soundlatch_r)
	AM_RANGE(0xc000, 0xc000) AM_READ(sgladiat_sound_nmi_ack_r)
	AM_RANGE(0xe000, 0xe001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0xe002, 0xe003) AM_WRITENOP    // ? always FFFF, snkwave leftover?
	AM_RANGE(0xe004, 0xe005) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( jcross_sound_portmap, AS_IO, 8, snk_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(sgladiat_sound_irq_ack_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( hal21_sound_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ(sgladiat_soundlatch_r)
	AM_RANGE(0xc000, 0xc000) AM_READ(sgladiat_sound_nmi_ack_r)
	AM_RANGE(0xe000, 0xe001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
//  AM_RANGE(0xe002, 0xe002) AM_WRITENOP    // bitfielded(0-5) details unknown. Filter enable?
	AM_RANGE(0xe008, 0xe009) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hal21_sound_portmap, AS_IO, 8, snk_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READNOP // read on startup, then the Z80 automatically pulls down the IORQ pin to ack irq
ADDRESS_MAP_END


static ADDRESS_MAP_START( tnk3_YM3526_sound_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xc000, 0xc000) AM_READ(tnk3_busy_clear_r)
	AM_RANGE(0xe000, 0xe001) AM_DEVREADWRITE("ym1", ym3526_device, read, write)
	AM_RANGE(0xe004, 0xe004) AM_READ(tnk3_cmdirq_ack_r)
	AM_RANGE(0xe006, 0xe006) AM_READ(tnk3_ymirq_ack_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( aso_YM3526_sound_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd000, 0xd000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe000, 0xe000) AM_READ(tnk3_busy_clear_r)
	AM_RANGE(0xf000, 0xf001) AM_DEVREADWRITE("ym1", ym3526_device, read, write)
//  AM_RANGE(0xf002, 0xf002) AM_READNOP unknown
	AM_RANGE(0xf004, 0xf004) AM_READ(tnk3_cmdirq_ack_r)
	AM_RANGE(0xf006, 0xf006) AM_READ(tnk3_ymirq_ack_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( YM3526_YM3526_sound_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe800, 0xe800) AM_DEVREADWRITE("ym1", ym3526_device, status_port_r, control_port_w)
	AM_RANGE(0xec00, 0xec00) AM_DEVWRITE("ym1", ym3526_device, write_port_w)
	AM_RANGE(0xf000, 0xf000) AM_DEVREADWRITE("ym2", ym3526_device, status_port_r, control_port_w)
	AM_RANGE(0xf400, 0xf400) AM_DEVWRITE("ym2", ym3526_device, write_port_w)
	AM_RANGE(0xf800, 0xf800) AM_READWRITE(snk_sound_status_r, snk_sound_status_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( YM3812_sound_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe800, 0xe800) AM_DEVREADWRITE("ym1", ym3812_device, status_port_r, control_port_w)
	AM_RANGE(0xec00, 0xec00) AM_DEVWRITE("ym1", ym3812_device, write_port_w)
	AM_RANGE(0xf800, 0xf800) AM_READWRITE(snk_sound_status_r, snk_sound_status_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( YM3526_Y8950_sound_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe800, 0xe800) AM_DEVREADWRITE("ym1", ym3526_device, status_port_r, control_port_w)
	AM_RANGE(0xec00, 0xec00) AM_DEVWRITE("ym1", ym3526_device, write_port_w)
	AM_RANGE(0xf000, 0xf000) AM_DEVREADWRITE("ym2", y8950_device, status_port_r, control_port_w)
	AM_RANGE(0xf400, 0xf400) AM_DEVWRITE("ym2", y8950_device, write_port_w)
	AM_RANGE(0xf800, 0xf800) AM_READWRITE(snk_sound_status_r, snk_sound_status_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( YM3812_Y8950_sound_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe800, 0xe800) AM_DEVREADWRITE("ym1", ym3812_device, status_port_r, control_port_w)
	AM_RANGE(0xec00, 0xec00) AM_DEVWRITE("ym1", ym3812_device, write_port_w)
	AM_RANGE(0xf000, 0xf000) AM_DEVREADWRITE("ym2", y8950_device, status_port_r, control_port_w)
	AM_RANGE(0xf400, 0xf400) AM_DEVWRITE("ym2", y8950_device, write_port_w)
	AM_RANGE(0xf800, 0xf800) AM_READWRITE(snk_sound_status_r, snk_sound_status_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( Y8950_sound_map, AS_PROGRAM, 8, snk_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xf000, 0xf000) AM_DEVREADWRITE("ym2", y8950_device, status_port_r, control_port_w)
	AM_RANGE(0xf400, 0xf400) AM_DEVWRITE("ym2", y8950_device, write_port_w)
	AM_RANGE(0xf800, 0xf800) AM_READWRITE(snk_sound_status_r, snk_sound_status_w)
ADDRESS_MAP_END

/*********************************************************************/

static INPUT_PORTS_START( marvins )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,marvins_sound_busy, NULL) /* sound CPU status */
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )   // service switch according to schematics, see code at 0x0453. Goes to garbage.

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME(0x04,  0x04, "Infinite Lives (Cheat)") PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "1st Bonus Life" )        PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x03, "40000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x05, "60000" )
	PORT_DIPSETTING(    0x06, "70000" )
	PORT_DIPSETTING(    0x07, "80000" )
	PORT_DIPNAME( 0x18, 0x08, "2nd Bonus Life" )        PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x08, "1st bonus*2" )
	PORT_DIPSETTING(    0x10, "1st bonus*3" )
	PORT_DIPSETTING(    0x18, "1st bonus*4" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( vangrd2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,marvins_sound_busy, NULL) /* sound CPU status */
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x38, "30000" )
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x28, "50000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x18, "70000" )
	PORT_DIPSETTING(    0x10, "80000" )
	PORT_DIPSETTING(    0x08, "90000" )
	PORT_DIPSETTING(    0x00, "100000" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Freeze" )                    PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Language ) )         PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x10, 0x00, "Bonus Life Occurrence" )     PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x00, "Every bonus" )
	PORT_DIPSETTING(    0x10, "Bonus only" )
	PORT_DIPNAME( 0x20, 0x20, "Infinite Lives (Cheat)")     PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( madcrash )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,marvins_sound_busy, NULL) /* sound CPU status */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,marvins_sound_busy, NULL) /* sound CPU status */
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SERVICE )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC(0x01, IP_ACTIVE_LOW, "DSW1:1")    /* Listed as Unused */
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW1:4,5,6")
//  PORT_DIPSETTING(    0x08, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, "20000 60000" )
	PORT_DIPSETTING(    0x80, "40000 90000" )
	PORT_DIPSETTING(    0x40, "50000 120000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Bonus Life Occurrence" )     PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, "1st, 2nd, then every 2nd" )  /* Check the "Non Bugs" page */
	PORT_DIPSETTING(    0x00, "1st and 2nd only" )
	PORT_DIPNAME( 0x06, 0x04, "Scroll Speed" )              PORT_DIPLOCATION("DSW2:2,3")
	PORT_DIPSETTING(    0x06, "Slow" )//DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, "Fast" )//DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, "Faster" )
	PORT_DIPNAME( 0x18, 0x10, "Game mode" )                 PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x10, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x08, "Infinite Lives (Cheat)")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )               /* Check the "Non Bugs" page */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNKNOWN_DIPLOC(0x80, IP_ACTIVE_LOW, "DSW2:8")   /* Listed as Unused, it is actually tested in many places */
INPUT_PORTS_END


static INPUT_PORTS_START( jcross )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Upright Controls" )          PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
//  PORT_DIPSETTING(    0x10, "INVALID !" )                 /* settings table at 0x0378 is only 5 bytes wide */
//  PORT_DIPSETTING(    0x08, "INVALID !" )                 /* settings table at 0x0378 is only 5 bytes wide */
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0xc0)

	PORT_START("DSW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x01)
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x10, "Game mode" )                 PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x10, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x08, "Infinite Lives (Cheat)" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "DSW2:7" )
	PORT_DIPNAME( 0x80, 0x80, "No BG Collision (Cheat)" )   PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0xc1, 0xc1, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW2:1,DSW1:7,8")
	PORT_DIPSETTING(    0xc1, "20k 60k 40k+" )
	PORT_DIPSETTING(    0x81, "40k 120k 80k+" )
	PORT_DIPSETTING(    0x41, "60k 160k 100k+" )
	PORT_DIPSETTING(    0xc0, "20k" )
	PORT_DIPSETTING(    0x80, "40k" )
	PORT_DIPSETTING(    0x40, "60k" )
//  PORT_DIPSETTING(    0x01, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sgladiat )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SERVICE )            /* code at 0x054e */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "DSW1:1" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )          /* Single Controls */
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( 5C_1C ) )            /* duplicated setting */
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0xc0)

	PORT_START("DSW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x01)
	PORT_DIPNAME( 0x02, 0x02, "Time" )                      PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, "More" )                      /* Hazard race 2:30 / Chariot race 3:30 */
	PORT_DIPSETTING(    0x00, "Less" )                      /* Hazard race 2:00 / Chariot race 3:00 */
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "DSW2:3" )
	PORT_DIPNAME( 0x18, 0x10, "Game Mode" )                 PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x10, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x08, "Infinite Lives (Cheat)")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW2:7")    /* code at 0x4169 */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "No Opponents (Cheat)" )      PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0xc1, 0xc1, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW2:1,DSW1:7,8")
	PORT_DIPSETTING(    0xc1, "20k 60k 60k+" )
	PORT_DIPSETTING(    0x81, "40k 90k 90k+" )
	PORT_DIPSETTING(    0x41, "50k 120k 120k+" )
	PORT_DIPSETTING(    0xc0, "20k 60k" )
	PORT_DIPSETTING(    0x80, "40k 90k" )
	PORT_DIPSETTING(    0x40, "50k 120k" )
//  PORT_DIPSETTING(    0x01, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( hal21 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "DSW1:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )          /* Dual Controls, simultaneous play */
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )         /* Alternative play */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )            /* duplicated setting */
//  PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )            /* duplicated setting */
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0xc0)

	PORT_START("DSW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x01)
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x10, "Game mode" )                 PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x10, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x08, "Infinite Lives (Cheat)" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "DSW2:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0xc1, 0xc1, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW2:1,DSW1:7,8")
	PORT_DIPSETTING(    0xc1, "20k 60k 60k+" )
	PORT_DIPSETTING(    0x81, "40k 90k 90k+" )
	PORT_DIPSETTING(    0x41, "50k 120k 120k+" )
	PORT_DIPSETTING(    0xc0, "20k 60k" )
	PORT_DIPSETTING(    0x80, "40k 90k" )
	PORT_DIPSETTING(    0x40, "50k 120k" )
//  PORT_DIPSETTING(    0x01, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( aso )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )           /* uses "Coinage" settings - code at 0x2e04 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, "3 Times" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )          /* Single Controls */
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0xc0)

	PORT_START("DSW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x01)
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "All Ships at Start (Cheat)") PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Start Area" )                PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0xc1, 0xc1, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW2:1,DSW1:7,8")
	PORT_DIPSETTING(    0xc1, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x81, "60k 120k 120k+" )
	PORT_DIPSETTING(    0x41, "100k 200k 200k+" )
	PORT_DIPSETTING(    0xc0, "50k 100k" )
	PORT_DIPSETTING(    0x80, "60k 120k" )
	PORT_DIPSETTING(    0x40, "100k 200k" )
//  PORT_DIPSETTING(    0x01, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( alphamis )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )           /* uses "Coin A" settings - code at 0x2e17 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, "3 Times" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )          /* Single Controls */
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_6C ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "DSW1:8" )

	PORT_START("DSW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x01)
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "All Ships at Start (Cheat)") PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Start Area" )                PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x00, "50k 100k" )
INPUT_PORTS_END


static INPUT_PORTS_START( tnk3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0xf0, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)
	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0xf0, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "No BG Collision (Cheat)")    PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )          /* Single Controls */
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )            /* duplicated setting */
//  PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )            /* duplicated setting */
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0xc0)

	PORT_START("DSW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x01)
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x10, "Game Mode" )                 PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x10, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x08, "Infinite Lives (Cheat)")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "DSW2:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, "5 Times" )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0xc1, 0xc1, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW2:1,DSW1:7,8")
	PORT_DIPSETTING(    0xc1, "20k 60k 60k+" )
	PORT_DIPSETTING(    0x81, "40k 90k 90k+" )
	PORT_DIPSETTING(    0x41, "50k 120k 120k+" )
	PORT_DIPSETTING(    0xc0, "20k 60k" )
	PORT_DIPSETTING(    0x80, "40k 90k" )
	PORT_DIPSETTING(    0x40, "50k 120k" )
//  PORT_DIPSETTING(    0x01, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( athena )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )           /* uses "Coin A" settings - code at 0x09d4 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "DSW1:1" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )          /* Single Controls */
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x04)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                    PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x30)
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "DSW2:7" )
	PORT_DIPNAME( 0x80, 0x80, "Energy" )                    PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, "12" )
	PORT_DIPSETTING(    0x00, "14" )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0x34, 0x34, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:3,DSW2:5,6")
	PORT_DIPSETTING(    0x34, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x24, "60k 120k 120k+" )
	PORT_DIPSETTING(    0x14, "100k 200k 200k+" )
	PORT_DIPSETTING(    0x30, "50k 100k" )
	PORT_DIPSETTING(    0x20, "60k 120k" )
	PORT_DIPSETTING(    0x10, "100k 200k" )
//  PORT_DIPSETTING(    0x04, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( fitegolf )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )          /* uses "Coin A" settings - code at 0x045b */
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_TILT )              /* reset */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE )           /* same as the dip switch */
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Language ) )         PORT_DIPLOCATION("DSW1:1")    /* Version */
	PORT_DIPSETTING(    0x01, DEF_STR( English ) )          /* Over Sea */
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )         /* Domestic */
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )          /* Dual Controls */
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, "Gameplay" )        PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01) PORT_DIPLOCATION("DSW1:4")    /* code at 0x011e */
	PORT_DIPSETTING(    0x00, "Basic Player" )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x08, "Avid Golfer" )     PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )    PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )     PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Shot Time" )                 PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x00, "Short (10 sec)" )
	PORT_DIPSETTING(    0x01, "Long (12 sec)" )
	PORT_DIPNAME( 0x02, 0x02, "Bonus Holes" )               PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, "More (Par 1,Birdie 2,Eagle 3)" )
	PORT_DIPSETTING(    0x00, "Less (Par 0,Birdie 1,Eagle 2)" )
	PORT_DIPNAME( 0x0c, 0x0c, "Game Mode" )                 PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x04, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x0c, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x08, "Infinite Holes (Cheat)")
	PORT_DIPNAME( 0x30, 0x30, "Play Holes" )                PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )                     PORT_DIPLOCATION("DSW2:8")
INPUT_PORTS_END


static INPUT_PORTS_START( fitegolfu )
		PORT_INCLUDE( fitegolf )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Shot Time" )                 PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x00, "Short (12 sec)" )
	PORT_DIPSETTING(    0x01, "Long (15 sec)" )
INPUT_PORTS_END



static INPUT_PORTS_START( countryc )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )          /* uses "Coin A" settings - code at 0x0450 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_TILT )              /* reset */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE )           /* same as the dip switch */
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,countryc_trackball_x, 0)

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,countryc_trackball_y, 0)

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Language ) )         PORT_DIPLOCATION("DSW1:1")    /* Version */
	PORT_DIPSETTING(    0x01, DEF_STR( English ) )          /* Over Sea */
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )         /* Domestic */
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Controls ) )         PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Dual ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "DSW1:4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Shot Time" )                 PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x00, "Short (15 sec)" )
	PORT_DIPSETTING(    0x01, "Long (20 sec)" )
	PORT_DIPNAME( 0x02, 0x02, "Bonus Holes" )               PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, "More (Par 1,Birdie 2,Eagle 3)" )
	PORT_DIPSETTING(    0x00, "Less (Par 0,Birdie 1,Eagle 2)" )
	PORT_DIPNAME( 0x0c, 0x0c, "Game Mode" )                 PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x04, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x0c, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x08, "Infinite Holes (Cheat)")
	PORT_DIPNAME( 0x30, 0x30, "Play Holes" )                PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )                     PORT_DIPLOCATION("DSW2:8")

	PORT_START("TRACKBALLX1")
	PORT_BIT( 0x7f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("TRACKBALLY1")
	PORT_BIT( 0x7f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("TRACKBALLX2")
	PORT_BIT( 0x7f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("TRACKBALLY2")
	PORT_BIT( 0x7f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( ikari )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )          /* adds 1 credit - code at 0x0a15 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0xf0, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0xf0, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Allow killing each other" )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "P1 & P2 Fire Buttons" )      PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Separate" )
	PORT_DIPSETTING(    0x00, "Common" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x04)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x08, "Game Mode" )                 PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x0c, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x08, "Demo Sounds On" )
	PORT_DIPSETTING(    0x04, "Freeze" )
	PORT_DIPSETTING(    0x00, "Infinite Lives (Cheat)")
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x30)
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "DSW2:7" )           /* read at 0x07c4, but strange test at 0x07cc */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0x34, 0x34, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:3,DSW2:5,6")
	PORT_DIPSETTING(    0x34, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x24, "60k 120k 120k+" )
	PORT_DIPSETTING(    0x14, "100k 200k 200k+" )
	PORT_DIPSETTING(    0x30, "50k 100k" )
	PORT_DIPSETTING(    0x20, "60k 120k" )
	PORT_DIPSETTING(    0x10, "100k 200k" )
//  PORT_DIPSETTING(    0x04, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ikaria )
	PORT_INCLUDE( ikari )

	// non-JAMMA system inputs

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )          /* adds 1 credit - code at 0x0a00 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_TILT )              /* reset */
INPUT_PORTS_END

static INPUT_PORTS_START( ikarinc )
	PORT_INCLUDE( ikaria )

	// no continues in this version

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "DSW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( ikarijpb )
	PORT_INCLUDE( ikarinc )

	// no rotary joystick in this version. Player fires in the direction he's facing.
	// this is accomplished by hooking the joystick input to the rotary input, plus
	// of course the code is patched to handle that.

	PORT_MODIFY("IN1")
	PORT_BIT( 0x21, 0x01, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x42, 0x02, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x84, 0x04, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x18, 0x08, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)

	PORT_MODIFY("IN2")
	PORT_BIT( 0x21, 0x01, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x42, 0x02, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x84, 0x04, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x18, 0x08, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( victroad )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )          /* adds 1 credit - code at 0x0a19 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_TILT )              /* reset */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0xf0, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0xf0, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Kill friend & walk everywhere (Cheat)") PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "P1 & P2 Fire Buttons" )      PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Separate" )
	PORT_DIPSETTING(    0x00, "Common" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x04)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x08, "Game Mode" )                 PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x0c, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x08, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x04, "Infinite Lives (Cheat)")
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x30)
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Credits Buy Lives During Play" ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0x34, 0x34, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:3,DSW2:5,6")
	PORT_DIPSETTING(    0x34, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x24, "60k 120k 120k+" )
	PORT_DIPSETTING(    0x14, "100k 200k 200k+" )
	PORT_DIPSETTING(    0x30, "50k 100k" )
	PORT_DIPSETTING(    0x20, "60k 120k" )
	PORT_DIPSETTING(    0x10, "100k 200k" )
//  PORT_DIPSETTING(    0x04, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dogosokb )
	PORT_INCLUDE( victroad )

	// no rotary joystick in this version. Player fires in the direction he's facing.

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "DSW2:8" )           /* patched code at 0x04ee */
INPUT_PORTS_END


static INPUT_PORTS_START( bermudat )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )          /* uses "Coin A" settings - code at 0x0a0a */
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_TILT )              /* reset */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0xf0, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0xf0, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "DSW1:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x04)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x08, "Game Mode" )                 PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x0c, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x08, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x04, "Infinite Lives (Cheat)")
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x30)
	PORT_DIPNAME( 0xc0, 0x80, "Game Style" )                PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(    0xc0, "Normal without continue" )
	PORT_DIPSETTING(    0x80, "Normal with continue" )
	PORT_DIPSETTING(    0x40, "Time attack 3 minutes" )
	PORT_DIPSETTING(    0x00, "Time attack 5 minutes" )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0x34, 0x34, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:3,DSW2:5,6")
	PORT_DIPSETTING(    0x34, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x24, "60k 120k 120k+" )
	PORT_DIPSETTING(    0x14, "100k 200k 200k+" )
	PORT_DIPSETTING(    0x30, "50k 100k" )
	PORT_DIPSETTING(    0x20, "60k 120k" )
	PORT_DIPSETTING(    0x10, "100k 200k" )
//  PORT_DIPSETTING(    0x04, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( worldwar )
	PORT_INCLUDE( bermudat )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "DSW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "DSW2:8" )

	PORT_MODIFY("BONUS") /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0x34, 0x34, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:3,DSW2:5,6")
	PORT_DIPSETTING(    0x34, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x24, "80k 160k 160k+" )
	PORT_DIPSETTING(    0x14, "100k 200k 200k+" )
	PORT_DIPSETTING(    0x30, "50k 100k" )
	PORT_DIPSETTING(    0x20, "80k 160k" )
	PORT_DIPSETTING(    0x10, "100k 200k" )
//  PORT_DIPSETTING(    0x04, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bermudaa )
	PORT_INCLUDE( worldwar )

	PORT_MODIFY("BONUS") /* fake port to handle bonus lives settings via multiple input ports */
PORT_DIPNAME( 0x34, 0x34, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:3,DSW2:5,6")
	PORT_DIPSETTING(    0x34, "25k 50k 50k+" )
	PORT_DIPSETTING(    0x24, "35k 70k 70k+" )
	PORT_DIPSETTING(    0x14, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x30, "25k 50k" )
	PORT_DIPSETTING(    0x20, "35k 70k" )
	PORT_DIPSETTING(    0x10, "50k 100k" )
//  PORT_DIPSETTING(    0x04, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( psychos )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_TILT )              /* reset */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )                     PORT_DIPLOCATION("DSW1:1")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x04)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                    PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x30)
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "DSW2:8" )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0x34, 0x30, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:3,DSW2:5,6")
	PORT_DIPSETTING(    0x30, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x20, "60k 120k 120k+" )
	PORT_DIPSETTING(    0x10, "100k 200k 200k+" )
	PORT_DIPSETTING(    0x34, "50k" )
	PORT_DIPSETTING(    0x24, "60k" )
	PORT_DIPSETTING(    0x14, "100k" )
//  PORT_DIPSETTING(    0x04, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( gwar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )          /* uses "Coin A" settings - code at 0x08c8 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_TILT )              /* reset */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,gwar_rotary, (void*)0)

	PORT_START("P1ROT")
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,gwar_rotary, (void*)1)

	PORT_START("P2ROT")
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x04)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x08, "Game Mode" )                 PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x0c, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x08, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x04, "Infinite Lives (Cheat)")
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x30)
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "DSW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "DSW2:8" )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0x34, 0x30, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:3,DSW2:5,6")
	PORT_DIPSETTING(    0x30, "30k 60k 60k+" )
	PORT_DIPSETTING(    0x20, "40k 80k 80k+" )
	PORT_DIPSETTING(    0x10, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x34, "30k 60k" )
	PORT_DIPSETTING(    0x24, "40k 80k" )
	PORT_DIPSETTING(    0x14, "50k 100k" )
//  PORT_DIPSETTING(    0x04, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gwarb )
	PORT_INCLUDE( gwar )

	// This version is modified to work either with or without a rotary joystick
	// connected. If rotary is not connected, player fires in the direction he's facing.

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,gwarb_rotary, (void*)0)

	PORT_MODIFY("IN2")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,gwarb_rotary, (void*)1)

	PORT_START("JOYSTICK_MODE")
	PORT_CONFNAME( 0x01, 0x00, "Joystick mode" )
	PORT_CONFSETTING( 0x00, "Normal Joystick" )
	PORT_CONFSETTING( 0x01, "Rotary Joystick" )
INPUT_PORTS_END


static INPUT_PORTS_START( chopper )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )          /* uses "Coin A" settings - code at 0x0849 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_TILT )              /* reset */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )          /* Single Controls */
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x04)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Game Mode" )                 PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x08, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x0c, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x04, "Infinite Lives (Cheat)")
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_bonus_r, (void *)0x30)
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Invulnerability (Cheat)")    PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BONUS")  /* fake port to handle bonus lives settings via multiple input ports */
	PORT_DIPNAME( 0x34, 0x30, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:3,DSW2:5,6")
	PORT_DIPSETTING(    0x30, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x20, "75k 150k 150k+" )
	PORT_DIPSETTING(    0x10, "100k 200k 200k+" )
	PORT_DIPSETTING(    0x34, "50k 100k" )
	PORT_DIPSETTING(    0x24, "75k 150k" )
	PORT_DIPSETTING(    0x14, "100k 200k" )
//  PORT_DIPSETTING(    0x04, DEF_STR( None ) )             /* duplicated setting */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( choppera )
	PORT_INCLUDE( chopper )

	// P1 and P2 inputs are split in two ports

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( tdfever )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE )           /* also reset - code at 0x074a */
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )          /* adds 1 credit - code at 0x1065 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 ) PORT_NAME("Start Game A")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 ) PORT_NAME("Start Game B")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("Start Game C")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 ) PORT_NAME("Start Game D")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START5 ) PORT_NAME("Start Game E")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(3)

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(4)

	PORT_START("IN6")
	PORT_BIT( 0x7f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN7")
	PORT_BIT( 0x7f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN8")
	PORT_BIT( 0x7f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN9")
	PORT_BIT( 0x7f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "2 Player Upright" )
	PORT_DIPSETTING(    0x00, "4 Player Cocktail" )
	PORT_DIPNAME( 0x0c, 0x00, "1st Down Bonus Time" )       PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(    0x00, "Every 1st Down" )
	PORT_DIPSETTING(    0x04, "Every 4 1st Downs" )
	PORT_DIPSETTING(    0x08, "Every 6 1st Downs" )
	PORT_DIPSETTING(    0x0c, "Every 8 1st Downs" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:7,8")  /* Manual shows these two as blank */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x08, "Game Mode" )                 PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x0c, "Demo Sound Off" )
	PORT_DIPSETTING(    0x08, "Demo Sound On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x04, "Never Finish (Cheat)")
/*
Actual Play Times listed in manual based on Players & cabinet type:

                Upright/Cocktail     Cocktail Only
 Dip Switch       A       B         C      D      E    <-- Start button "MODE"
 SW6 SW7 SW8    1PvsCPU 2PvsCPU   1Pvs2P 2Pvs1P 2Pvs2P
 OFF OFF OFF     1:00    1:10      2:00   2:10   3:00
 ON  OFF OFF     1:10    1:20      2:10   2:20   3:10
 OFF ON  OFF     1:20    1:30      2:20   2:30   3:20
 ON  ON  OFF     1:30    1:40      2:30   2:40   3:30
 OFF OFF ON      1:40    1:50      2:40   2:50   3:40
 ON  OFF ON      1:50    2:00      2:50   3:00   3:50
 OFF ON  ON      2:00    2:10      3:00   3:10   4:00
 ON  ON  ON      2:10    2:20      3:10   3:20   4:10
*/
	PORT_DIPNAME( 0x70, 0x70, "Play Time (Type A)" )        PORT_DIPLOCATION("DSW2:5,6,7")
	PORT_DIPSETTING(    0x70, "1:00" )
	PORT_DIPSETTING(    0x60, "1:10" )
	PORT_DIPSETTING(    0x50, "1:20" )
	PORT_DIPSETTING(    0x40, "1:30" )
	PORT_DIPSETTING(    0x30, "1:40" )
	PORT_DIPSETTING(    0x20, "1:50" )
	PORT_DIPSETTING(    0x10, "2:00" )
	PORT_DIPSETTING(    0x00, "2:10" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( fsoccer )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE )           /* same as the dip switch / also reset - code at 0x00cc */
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )          /* uses "Coin A" settings - code at 0x677f */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snk_state,snk_sound_busy, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 ) PORT_NAME("Start Game A")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 ) PORT_NAME("Start Game B")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("Start Game C")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 ) PORT_NAME("Start Game D")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START5 ) PORT_NAME("Start Game E")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(3)

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(4)

	PORT_START("IN6")
	PORT_BIT( 0x7f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN7")
	PORT_BIT( 0x7f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN8")
	PORT_BIT( 0x7f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN9")
	PORT_BIT( 0x7f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Cabinet ) )         PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "Upright (With VS)" )
	PORT_DIPSETTING(    0x02, "Upright (Without VS)" )
	PORT_DIPSETTING(    0x00, "Cocktail (2 Players)" )
	PORT_DIPSETTING(    0x01, "Cocktail (4 Players)" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Version ) )         PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x00, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0c, DEF_STR( Europe ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )          PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )          PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Game Mode" )                 PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x08, "Demo Sound Off" )
	PORT_DIPSETTING(    0x0c, "Demo Sound On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x04, "Win Match Against CPU (Cheat)")
/*
Dip Switch settings are the time for match type A. Here is what you have to add for games B to E :

Match Type       B        C        D        E
Time to add    00:30    01:00    01:30    02:00
*/
	PORT_DIPNAME( 0x70, 0x70, "Play Time (Type A)" )        PORT_DIPLOCATION("DSW2:5,6,7")
	PORT_DIPSETTING(    0x10, "1:00" )
	PORT_DIPSETTING(    0x60, "1:10" )
	PORT_DIPSETTING(    0x50, "1:20" )
	PORT_DIPSETTING(    0x40, "1:30" )
	PORT_DIPSETTING(    0x30, "1:40" )
	PORT_DIPSETTING(    0x20, "1:50" )
	PORT_DIPSETTING(    0x70, "2:00" )
	PORT_DIPSETTING(    0x00, "2:10" )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )                     PORT_DIPLOCATION("DSW2:8")
INPUT_PORTS_END


static INPUT_PORTS_START( fsoccerb )

	PORT_INCLUDE( fsoccer )

	// no spinners in this version. Player shoots in the direction he's facing
	// (see the 'jmp' instruction at 0x00f1)

	PORT_MODIFY("IN6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN8")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN9")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*********************************************************************/

static const gfx_layout charlayout_4bpp =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4*1, 4*0, 4*3, 4*2, 4*5, 4*4, 4*7, 4*6 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tilelayout_4bpp =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4*1, 4*0, 4*3, 4*2, 4*5, 4*4, 4*7, 4*6,
		32+4*1, 32+4*0, 32+4*3, 32+4*2, 32+4*5, 32+4*4, 32+4*7, 32+4*6 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	64*16
};

static const gfx_layout spritelayout_3bpp =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 7,6,5,4,3,2,1,0, 15,14,13,12,11,10,9,8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

static const gfx_layout spritelayout_4bpp =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{
		8,9,10,11,12,13,14,15,
		0,1,2,3,4,5,6,7
	},
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

static const gfx_layout bigspritelayout_3bpp =
{
	32,32,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{
		7,6,5,4,3,2,1,0,
		15,14,13,12,11,10,9,8,
		23,22,21,20,19,18,17,16,
		31,30,29,28,27,26,25,24
	},
	{
		0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
		16*32+0*32, 16*32+1*32, 16*32+2*32, 16*32+3*32,
		16*32+4*32, 16*32+5*32, 16*32+6*32, 16*32+7*32,
		16*32+8*32, 16*32+9*32, 16*32+10*32, 16*32+11*32,
		16*32+12*32, 16*32+13*32, 16*32+14*32, 16*32+15*32,
	},
	16*32*2
};

static const gfx_layout bigspritelayout_4bpp =
{
	32,32,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{
		24,25,26,27,28,29,30,31,
		16,17,18,19,20,21,22,23,
		8,9,10,11,12,13,14,15,
		0,1,2,3,4,5,6,7
	},
	{
		0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
		16*32+0*32, 16*32+1*32, 16*32+2*32, 16*32+3*32,
		16*32+4*32, 16*32+5*32, 16*32+6*32, 16*32+7*32,
		16*32+8*32, 16*32+9*32, 16*32+10*32, 16*32+11*32,
		16*32+12*32, 16*32+13*32, 16*32+14*32, 16*32+15*32,
	},
	16*32*2
};

/*********************************************************************/

static GFXDECODE_START( marvins )
	GFXDECODE_ENTRY( "tx_tiles",   0, charlayout_4bpp,   0x180, 0x080>>4 )
	GFXDECODE_ENTRY( "fg_tiles",   0, charlayout_4bpp,   0x080, 0x080>>4 )
	GFXDECODE_ENTRY( "bg_tiles",   0, charlayout_4bpp,   0x100, 0x080>>4 )
	GFXDECODE_ENTRY( "sp16_tiles", 0, spritelayout_3bpp, 0x000, 0x080>>3 )
	/* colors 0x200-0x3ff contain shadows */
GFXDECODE_END

static GFXDECODE_START( tnk3 )
	GFXDECODE_ENTRY( "tx_tiles",   0, charlayout_4bpp,   0x180, 0x080>>4 )
	GFXDECODE_ENTRY( "bg_tiles",   0, charlayout_4bpp,   0x080, 0x100>>4 )
	GFXDECODE_ENTRY( "sp16_tiles", 0, spritelayout_3bpp, 0x000, 0x080>>3 )
	/* colors 0x200-0x3ff contain shadows */
GFXDECODE_END

static GFXDECODE_START( ikari )
	GFXDECODE_ENTRY( "tx_tiles",   0, charlayout_4bpp,      0x180, 0x080>>4 )
	GFXDECODE_ENTRY( "bg_tiles",   0, tilelayout_4bpp,      0x100, 0x080>>4 )
	GFXDECODE_ENTRY( "sp16_tiles", 0, spritelayout_3bpp,    0x000, 0x080>>3 )
	GFXDECODE_ENTRY( "sp32_tiles", 0, bigspritelayout_3bpp, 0x080, 0x080>>3 )
	/* colors 0x200-0x3ff contain shadows */
GFXDECODE_END

static GFXDECODE_START( gwar )
	GFXDECODE_ENTRY( "tx_tiles",   0, charlayout_4bpp,      0x000, 0x100>>4 )
	GFXDECODE_ENTRY( "bg_tiles",   0, tilelayout_4bpp,      0x300, 0x100>>4 )
	GFXDECODE_ENTRY( "sp16_tiles", 0, spritelayout_4bpp,    0x100, 0x100>>4 )
	GFXDECODE_ENTRY( "sp32_tiles", 0, bigspritelayout_4bpp, 0x200, 0x100>>4 )
GFXDECODE_END

static GFXDECODE_START( tdfever )
	GFXDECODE_ENTRY( "tx_tiles",   0, charlayout_4bpp,      0x000, 0x100>>4 )
	GFXDECODE_ENTRY( "bg_tiles",   0, tilelayout_4bpp,      0x200, 0x100>>4 )
	GFXDECODE_ENTRY( "sp32_tiles", 0, bigspritelayout_4bpp, 0x100, 0x100>>4 )
	/* colors 0x300-0x3ff contain shadows */
GFXDECODE_END

/**********************************************************************/

static MACHINE_CONFIG_START( marvins, snk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3360000)   /* 3.36 MHz */
	MCFG_CPU_PROGRAM_MAP(marvins_cpuA_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, 3360000)   /* 3.36 MHz */
	MCFG_CPU_PROGRAM_MAP(marvins_cpuB_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)  /* verified on schematics */
	MCFG_CPU_PROGRAM_MAP(marvins_sound_map)
	MCFG_CPU_IO_MAP(marvins_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(snk_state, nmi_line_assert,  244)  // schematics show a separate 244Hz timer

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(36*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 1*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(snk_state, screen_update_marvins)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", marvins)

	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_ENABLE_SHADOWS()

	MCFG_PALETTE_INIT_OWNER(snk_state,tnk3)
	MCFG_VIDEO_START_OVERRIDE(snk_state,marvins)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 2000000)  /* verified on schematics */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)

	MCFG_SOUND_ADD("ay2", AY8910, 2000000)  /* verified on schematics */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)

	MCFG_SNKWAVE_ADD("wave", 8000000)   /* verified on schematics */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( vangrd2, marvins )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(madcrash_cpuA_map)

	MCFG_CPU_MODIFY("sub")
	MCFG_CPU_PROGRAM_MAP(madcrash_cpuB_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( madcrush, marvins )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(madcrush_cpuA_map)

	MCFG_CPU_MODIFY("sub")
	MCFG_CPU_PROGRAM_MAP(madcrush_cpuB_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( jcross, snk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3350000) /* NOT verified */
	MCFG_CPU_PROGRAM_MAP(jcross_cpuA_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, 3350000) /* NOT verified */
	MCFG_CPU_PROGRAM_MAP(jcross_cpuB_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000) /* NOT verified */
	MCFG_CPU_PROGRAM_MAP(jcross_sound_map)
	MCFG_CPU_IO_MAP(jcross_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(snk_state, irq0_line_assert,  244) // Marvin's frequency, sounds ok

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(36*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 1*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(snk_state, screen_update_tnk3)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tnk3)

	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_ENABLE_SHADOWS()

	MCFG_PALETTE_INIT_OWNER(snk_state,tnk3)
	MCFG_VIDEO_START_OVERRIDE(snk_state,jcross)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 2000000)  /* NOT verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)

	MCFG_SOUND_ADD("ay2", AY8910, 2000000)  /* NOT verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sgladiat, jcross )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sgladiat_cpuA_map)

	MCFG_CPU_MODIFY("sub")
	MCFG_CPU_PROGRAM_MAP(sgladiat_cpuB_map)

	/* video hardware */
	/* visible area is correct. Debug info is shown in the black bars at the sides
	   of the screen when the Debug dip switch is on */

	MCFG_VIDEO_START_OVERRIDE(snk_state,sgladiat)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( hal21, jcross )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hal21_cpuA_map)

	MCFG_CPU_MODIFY("sub")
	MCFG_CPU_PROGRAM_MAP(hal21_cpuB_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(hal21_sound_map)
	MCFG_CPU_IO_MAP(hal21_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(snk_state, irq0_line_hold,  220) // music tempo, hand tuned

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(snk_state,hal21)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( tnk3, snk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_13_4MHz/4) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(tnk3_cpuA_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, XTAL_13_4MHz/4) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(tnk3_cpuB_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(tnk3_YM3526_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(36*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 1*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(snk_state, screen_update_tnk3)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tnk3)

	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_ENABLE_SHADOWS()

	MCFG_PALETTE_INIT_OWNER(snk_state,tnk3)
	MCFG_VIDEO_START_OVERRIDE(snk_state,tnk3)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM3526, XTAL_8MHz/2) /* verified on pcb */
	MCFG_YM3526_IRQ_HANDLER(WRITELINE(snk_state, ymirq_callback_1))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( aso, tnk3 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(aso_cpuA_map)

	MCFG_CPU_MODIFY("sub")
	MCFG_CPU_PROGRAM_MAP(aso_cpuB_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(aso_YM3526_sound_map)

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(snk_state,aso)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( athena, tnk3 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(YM3526_YM3526_sound_map)

	/* sound hardware */
	MCFG_SOUND_ADD("ym2", YM3526, XTAL_8MHz/2) /* verified on pcb */
	MCFG_YM3526_IRQ_HANDLER(WRITELINE(snk_state, ymirq_callback_2))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( fitegolf, tnk3 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("audiocpu")
	// xtal is 4MHz instead of 8MHz/2 but the end result is the same
	MCFG_CPU_PROGRAM_MAP(YM3812_sound_map)

	/* sound hardware */
	MCFG_SOUND_REPLACE("ym1", YM3812, XTAL_4MHz) /* verified on pcb */
	MCFG_YM3812_IRQ_HANDLER(WRITELINE(snk_state, ymirq_callback_1))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( ikari, snk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_13_4MHz/4) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(ikari_cpuA_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, XTAL_13_4MHz/4) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(ikari_cpuB_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(YM3526_YM3526_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(36*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 1*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(snk_state, screen_update_ikari)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ikari)

	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 0x400)
	MCFG_PALETTE_ENABLE_SHADOWS()

	MCFG_VIDEO_START_OVERRIDE(snk_state,ikari)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM3526, XTAL_8MHz/2) /* verified on pcb */
	MCFG_YM3526_IRQ_HANDLER(WRITELINE(snk_state, ymirq_callback_1))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)

	MCFG_SOUND_ADD("ym2", YM3526, XTAL_8MHz/2) /* verified on pcb */
	MCFG_YM3526_IRQ_HANDLER(WRITELINE(snk_state, ymirq_callback_2))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( victroad, ikari )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(YM3526_Y8950_sound_map)

	/* sound hardware */
	MCFG_SOUND_REPLACE("ym2", Y8950, XTAL_8MHz/2) /* verified on pcb */
	MCFG_Y8950_IRQ_HANDLER(WRITELINE(snk_state, ymirq_callback_2))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( bermudat, snk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_8MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(bermudat_cpuA_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, XTAL_8MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(bermudat_cpuB_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(YM3526_Y8950_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(24000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	// this visible area matches the psychos pcb
	MCFG_SCREEN_SIZE(50*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 50*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(snk_state, screen_update_gwar)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gwar)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 0x400)
	MCFG_VIDEO_START_OVERRIDE(snk_state,gwar)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM3526, XTAL_8MHz/2) /* verified on pcb */
	MCFG_YM3526_IRQ_HANDLER(WRITELINE(snk_state, ymirq_callback_1))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)

	MCFG_SOUND_ADD("ym2", Y8950, XTAL_8MHz/2) /* verified on pcb */
	MCFG_Y8950_IRQ_HANDLER(WRITELINE(snk_state, ymirq_callback_2))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( psychos, bermudat )

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(snk_state,psychos)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( gwar, bermudat )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(gwar_cpuA_map)

	MCFG_CPU_MODIFY("sub")
	MCFG_CPU_PROGRAM_MAP(gwar_cpuB_map)

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( gwara, bermudat )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(gwara_cpuA_map)

	MCFG_CPU_MODIFY("sub")
	MCFG_CPU_PROGRAM_MAP(gwara_cpuB_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( chopper1, bermudat )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("sub")
	MCFG_CPU_PROGRAM_MAP(gwar_cpuB_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(YM3812_Y8950_sound_map)

	/* sound hardware */
	MCFG_SOUND_REPLACE("ym1", YM3812, 4000000)
	MCFG_YM3812_IRQ_HANDLER(WRITELINE(snk_state, ymirq_callback_1))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( choppera, chopper1 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(gwar_cpuA_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( tdfever, snk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(tdfever_cpuA_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(tdfever_cpuB_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(YM3526_Y8950_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(50*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 50*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(snk_state, screen_update_tdfever)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tdfever)

	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 0x400)
	MCFG_PALETTE_ENABLE_SHADOWS()

	MCFG_VIDEO_START_OVERRIDE(snk_state,tdfever)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM3526, 4000000)
	MCFG_YM3526_IRQ_HANDLER(WRITELINE(snk_state, ymirq_callback_1))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("ym2", Y8950, 4000000)
	MCFG_Y8950_IRQ_HANDLER(WRITELINE(snk_state, ymirq_callback_2))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( tdfever2, tdfever )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(Y8950_sound_map)

	/* sound hardware */

	// apparently, no "ym1" in tdfever2
	// (registers are written to but they cause sound not to work)
	MCFG_DEVICE_REMOVE("ym1")
MACHINE_CONFIG_END


/***********************************************************************/

ROM_START( marvins )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for CPUA code */
	ROM_LOAD( "pa1",   0x0000, 0x2000, CRC(0008d791) SHA1(6ffb174b2d680314f74efeef83da9f3ee3e0c753) )
	ROM_LOAD( "pa2",   0x2000, 0x2000, CRC(9457003c) SHA1(05ecd5c638a12163e2a65bdfcc09875618f792e1) )
	ROM_LOAD( "pa3",   0x4000, 0x2000, CRC(54c33ecb) SHA1(cfbf9ffc125fbc51f2abef180f36781f9e748bbd) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for CPUB code */
	ROM_LOAD( "pb1",   0x0000, 0x2000, CRC(3b6941a5) SHA1(9c29870196eaed87f34456fdb06bf7b69c8f489d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "m1",    0x0000, 0x2000, CRC(2314c696) SHA1(1b84a0c82a4dcff648752f53aa1f0abf5357c5d1) )
	ROM_LOAD( "m2",    0x2000, 0x2000, CRC(74ba5799) SHA1(c278b0e5c4134f6077d4ae7b51e3c5cba28af1a8) )

	ROM_REGION( 0x2000, "tx_tiles", 0 )
	ROM_LOAD( "s1",    0x0000, 0x2000, CRC(327f70f3) SHA1(078dcc6b4697617d4d833ccd59c6a543b2a88d9e) )

	ROM_REGION( 0x2000, "fg_tiles", 0 )
	ROM_LOAD( "b1",    0x0000, 0x2000, CRC(e528bc60) SHA1(3365ac7cbc57739054bc11e68831be87c0c1a97a) )

	ROM_REGION( 0x2000, "bg_tiles", 0 )
	ROM_LOAD( "b2",    0x0000, 0x2000, CRC(e528bc60) SHA1(3365ac7cbc57739054bc11e68831be87c0c1a97a) )

	ROM_REGION( 0x6000, "sp16_tiles", 0 )
	ROM_LOAD( "f1",    0x0000, 0x2000, CRC(0bd6b4e5) SHA1(c56747ff2135db734f1b5f6c2906de5ac8f53bbc) )
	ROM_LOAD( "f2",    0x2000, 0x2000, CRC(8fc2b081) SHA1(fb345965375cb62ec1b947d6c6d071380dc0f395) )
	ROM_LOAD( "f3",    0x4000, 0x2000, CRC(e55c9b83) SHA1(04b0d99955e4b11820015b7721ac6399a3d5a829) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "marvmaze.j1",  0x000, 0x400, CRC(92f5b06d) SHA1(97979ffb6fb065d9c99da43173180fefb2de1886) )
	ROM_LOAD( "marvmaze.j2",  0x400, 0x400, CRC(d2b25665) SHA1(b913b8b9c5ee0a29b5a115b2432c5706979059cf) )
	ROM_LOAD( "marvmaze.j3",  0x800, 0x400, CRC(df9e6005) SHA1(8f633f664c3f8e4f6ca94bee74a68c8fda8873e3) )
ROM_END

/***********************************************************************/

ROM_START( madcrash )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for CPUA code */
	ROM_LOAD( "p8",    0x0000, 0x2000, CRC(ecb2fdc9) SHA1(7dd79fbbe286a9f18ed2cae45b1bfab765e549a1) )
	ROM_LOAD( "p9",    0x2000, 0x2000, CRC(0a87df26) SHA1(327710452bdc5dbb931abc853957225814f224c5) )
	ROM_LOAD( "p10",   0x4000, 0x2000, CRC(6eb8a87c) SHA1(375377df22b331175aaf1f9eb8d8ad83e8e146f6) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for CPUB code */
	ROM_LOAD( "p4",   0x0000, 0x2000, CRC(5664d699) SHA1(5bfa57a0f8d718d522003da6513a70d7ca3a87a3) )
	ROM_LOAD( "p5",   0x2000, 0x2000, CRC(dea2865a) SHA1(0807281e35159ee29fbe2d1aa087b57804f1a14f) )
	ROM_LOAD( "p6",   0x4000, 0x2000, CRC(e25a9b9c) SHA1(26853611e3898907239e15f1a00f62290889f89b) )
	ROM_LOAD( "p7",   0x6000, 0x2000, CRC(55b14a36) SHA1(7d5566a6ba285af92ddf560efda60a79f1da84c2) )
	ROM_LOAD( "p3",   0x8000, 0x2000, CRC(e3c8c2cb) SHA1(b3e39eacd2609ff0fa0f511bff0fc83e6b3970d4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "p1",   0x0000, 0x2000, CRC(2dcd036d) SHA1(4da42ab1e502fff57f5d5787df406289538fa484) )
	ROM_LOAD( "p2",   0x2000, 0x2000, CRC(cc30ae8b) SHA1(ffedc747b9e0b616a163ff8bb1def318e522585b) )

	ROM_REGION( 0x2000, "tx_tiles", 0 )
	ROM_LOAD( "p13",    0x0000, 0x2000, CRC(48c4ade0) SHA1(3628abb4f425b8c9d8659c8e4082735168b0f3e9) )

	ROM_REGION( 0x2000, "fg_tiles", 0 )
	ROM_LOAD( "p11",    0x0000, 0x2000, CRC(67174956) SHA1(65a921176294212971c748932a9010f45e1fb499) )

	ROM_REGION( 0x2000, "bg_tiles", 0 )
	ROM_LOAD( "p12",    0x0000, 0x2000, CRC(085094c1) SHA1(5c5599d1ed7f8a717ada54bbd28383a22e09a8fe) )

	ROM_REGION( 0x6000, "sp16_tiles", 0 )
	ROM_LOAD( "p16",    0x0000, 0x2000, CRC(6153611a) SHA1(b352f92b233761122f74830e46913cc4df800259) )
	ROM_LOAD( "p15",    0x2000, 0x2000, CRC(a74149d4) SHA1(e8011a8d4d1a98a0ffe67fc28ea9fa192ca80321) )
	ROM_LOAD( "p14",    0x4000, 0x2000, CRC(07e807bc) SHA1(f651d3a5394ced8e0a1b2be3aa52b3e5a5d84c37) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "m3-prom.j3",  0x000, 0x400, CRC(d19e8a91) SHA1(b21fbdb8ed8d0b27c3ec78cf2e115624f69c67e0) )
	ROM_LOAD( "m2-prom.j4",  0x400, 0x400, CRC(9fc325af) SHA1(a180662f168ba001376f25f5d9205cb119c1ffee) )
	ROM_LOAD( "m1-prom.j5",  0x800, 0x400, CRC(07678443) SHA1(267951886d8b031dd633dc4823d9bd862a585437) )
ROM_END

ROM_START( madcrush )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for CPUA code */
	ROM_LOAD( "p3.a8",   0x0000, 0x2000, CRC(fbd3eda1) SHA1(23fb06978fe51ec409f1ebdbcc70d1b3b73f08ca) ) /* These 3 roms are located on the A2003 UP02-03 PCB */
	ROM_LOAD( "p4.a9",   0x2000, 0x2000, CRC(1bc67cab) SHA1(7d667c234d9eac34c0e90df7f68e9f5aa2726e8c) )
	ROM_LOAD( "p5.a10",  0x4000, 0x2000, CRC(d905ff79) SHA1(5b45e63d10191544ff6ca8c3ecb517484d70d5e3) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for CPUB code */
	ROM_LOAD( "p6.a11",   0x0000, 0x2000, CRC(432b5743) SHA1(d3c86c9983ee2174c58becc1e250d94426e6fc70) ) /* These 3 roms are located on the A2003 UP02-03 PCB */
	ROM_LOAD( "p7.a13",   0x2000, 0x2000, CRC(dea2865a) SHA1(0807281e35159ee29fbe2d1aa087b57804f1a14f) ) /* Same as Mad Crasher, but different label */
	ROM_LOAD( "p8.a14",   0x4000, 0x2000, CRC(e25a9b9c) SHA1(26853611e3898907239e15f1a00f62290889f89b) ) /* Same as Mad Crasher, but different label */
	/* Roms P9 & P10 are located on the A3006SUB plug-in module also containing a Z80A CPU plugged into the A2003 UP02-03 PCB */
	ROM_LOAD( "p10.bin",  0x6000, 0x2000, CRC(55b14a36) SHA1(7d5566a6ba285af92ddf560efda60a79f1da84c2) ) /* Same as Mad Crasher, but different label */
	ROM_LOAD( "p9.bin",   0x8000, 0x2000, CRC(e3c8c2cb) SHA1(b3e39eacd2609ff0fa0f511bff0fc83e6b3970d4) ) /* Same as Mad Crasher, but different label */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "p1.a6",   0x0000, 0x2000, CRC(2dcd036d) SHA1(4da42ab1e502fff57f5d5787df406289538fa484) ) /* Located on the A2003UP03-01 duaghtercard PCB */
	ROM_LOAD( "p2.a8",   0x2000, 0x2000, CRC(cc30ae8b) SHA1(ffedc747b9e0b616a163ff8bb1def318e522585b) ) /* Located on the A2003UP03-01 duaghtercard PCB */

	ROM_REGION( 0x2000, "tx_tiles", 0 )
	ROM_LOAD( "p13.e2",    0x0000, 0x2000, CRC(fcdd36ca) SHA1(bb9408e1feaa15949f11d797e3eb91d37c3e0add) ) /* Located on the A2003 UP01-04 PCB */

	ROM_REGION( 0x2000, "fg_tiles", 0 )
	ROM_LOAD( "p11.a2",    0x0000, 0x2000, CRC(67174956) SHA1(65a921176294212971c748932a9010f45e1fb499) ) /* Located on the A2003 UP01-04 PCB */

	ROM_REGION( 0x2000, "bg_tiles", 0 )
	ROM_LOAD( "p12.c2",    0x0000, 0x2000, CRC(085094c1) SHA1(5c5599d1ed7f8a717ada54bbd28383a22e09a8fe) ) /* Located on the A2003 UP01-04 PCB */

	ROM_REGION( 0x6000, "sp16_tiles", 0 )
	ROM_LOAD( "p16.k4",    0x0000, 0x2000, CRC(6153611a) SHA1(b352f92b233761122f74830e46913cc4df800259) ) /* These 3 roms are located on the A2003 UP01-04 PCB */
	ROM_LOAD( "p15.k2",    0x2000, 0x2000, CRC(a74149d4) SHA1(e8011a8d4d1a98a0ffe67fc28ea9fa192ca80321) )
	ROM_LOAD( "p14.k1",    0x4000, 0x2000, CRC(07e807bc) SHA1(f651d3a5394ced8e0a1b2be3aa52b3e5a5d84c37) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "m3-prom.j3",  0x000, 0x400, CRC(d19e8a91) SHA1(b21fbdb8ed8d0b27c3ec78cf2e115624f69c67e0) ) /* These 3 bproms are located on the A2003 UP02-03 PCB */
	ROM_LOAD( "m2-prom.j4",  0x400, 0x400, CRC(9fc325af) SHA1(a180662f168ba001376f25f5d9205cb119c1ffee) )
	ROM_LOAD( "m1-prom.j5",  0x800, 0x400, CRC(07678443) SHA1(267951886d8b031dd633dc4823d9bd862a585437) )
ROM_END

/***********************************************************************/

ROM_START( vangrd2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.9a",  0x0000, 0x2000, CRC(bc9eeca5) SHA1(5a737e0f0aa1a3a5296d1e1fec13b34aee970609) )
	ROM_LOAD( "p3.11a", 0x2000, 0x2000, CRC(3970f69d) SHA1(b0ef7494888804ab5b4002730fb0232a7fd6797b) )
	ROM_LOAD( "p2.12a", 0x4000, 0x2000, CRC(58b08b58) SHA1(eccc85191d678a0115a113002a43203afd857a5b) )
	ROM_LOAD( "p4.14a", 0x6000, 0x2000, CRC(a95f11ea) SHA1(8007efb4ad948c8768e474fc77134f3ce52da1d2) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "p5.4a", 0x0000, 0x2000, CRC(e4dfd0ba) SHA1(12d45ff147f3ea9c9e898c3831874cd7c1a071b7) )
	ROM_LOAD( "p6.6a", 0x2000, 0x2000, CRC(894ff00d) SHA1(1c66f327d8e94dc6ac386e11fcc5eb17c9081434) )
	ROM_LOAD( "p7.7a", 0x4000, 0x2000, CRC(40b4d069) SHA1(56c464bd055125ffc2da02d70137aa5efe5cd8f6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "p8.6a", 0x0000, 0x2000, CRC(a3daa438) SHA1(4e659ac7e3ebaf85bc3ce5c9946fcf0af23083b4) )
	ROM_LOAD( "p9.8a", 0x2000, 0x2000, CRC(9345101a) SHA1(b99ad1c2a79df50b0a60fdd43ca466f6cb38445b) )

	ROM_REGION( 0x2000, "tx_tiles", 0 )
	ROM_LOAD( "p15.1e", 0x0000, 0x2000, CRC(85718a41) SHA1(4c9aa1f8b229410414cd67bac8cb10a14bea12f4) )

	ROM_REGION( 0x2000, "fg_tiles", 0 )
	ROM_LOAD( "p13.1a", 0x0000, 0x2000, CRC(912f22c6) SHA1(5042edc80b58f77b3576b5e6eb8c6460c8a35494) )

	ROM_REGION( 0x2000, "bg_tiles", 0 )
	ROM_LOAD( "p9",     0x0000, 0x2000, CRC(7aa0b684) SHA1(d52670ec50b1a07d6c2c537f67922063deacdeea) )

	ROM_REGION( 0x6000, "sp16_tiles", 0 )
	ROM_LOAD( "p10.4kl", 0x0000, 0x2000, CRC(5bfc04c0) SHA1(4eb152fdf39cb0024f71d5bdf1bfc79c2b8c2329) )
	ROM_LOAD( "p11.3kl", 0x2000, 0x2000, CRC(620cd4ec) SHA1(a2fcc3d24d0d3c7cc601620ae7a709f46b613c0f) )
	ROM_LOAD( "p12.1kl", 0x4000, 0x2000, CRC(8658ea6c) SHA1(d5ea9be2c1776b11abc77c944a653eeb73b27fc8) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "mb7054.3j", 0x000, 0x400, CRC(506f659a) SHA1(766f1a0dd462eba64546c514004e6542e200d7c3) )
	ROM_LOAD( "mb7054.4j", 0x400, 0x400, CRC(222133ce) SHA1(109a63c8c44608a8ad9183e7b5d269765cc5f067) )
	ROM_LOAD( "mb7054.5j", 0x800, 0x400, CRC(2e21a79b) SHA1(1956377c799e0bbd127bf4fae016adc148efe007) )
ROM_END

/***********************************************************************/

ROM_START( jcross )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jcrossa0.10b",  0x0000, 0x2000, CRC(0e79bbcd) SHA1(7088a8effd30080529b797991e24e9807bf90475) )
	ROM_LOAD( "jcrossa1.12b",  0x2000, 0x2000, CRC(999b2bcc) SHA1(e5d13c9c11a82cedee15777341e6424639ecf2f5) )
	ROM_LOAD( "jcrossa2.13b",  0x4000, 0x2000, CRC(ac89e49c) SHA1(9b9a0eec8ad341ce7af58bffe55f10bec696af62) )
	ROM_LOAD( "jcrossa3.14b",  0x6000, 0x2000, CRC(4fd7848d) SHA1(870aea0b8e027616814df87afd24418fd140f736) )
	ROM_LOAD( "jcrossa4.15b",  0x8000, 0x2000, CRC(8500575d) SHA1(b8751b86508de484f2eb8a6702c63a47ec882036) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "jcrossb0.15a",  0x0000, 0x2000, CRC(77ed51e7) SHA1(56b457846f71f442da6f99889231d4b71d5fcb6c) )
	ROM_LOAD( "jcrossb1.14a",  0x2000, 0x2000, CRC(23cf0f70) SHA1(f258e899f332a026eeb0db92330fd60c478218af) )
	ROM_LOAD( "jcrossb2.13a",  0x4000, 0x2000, CRC(5bed3118) SHA1(f105ca55223a4bfbc8e2d61c365c76cf2153254c) )
	ROM_LOAD( "jcrossb3.12a",  0x6000, 0x2000, CRC(cd75dc95) SHA1(ef03d2b0f66f30fad5132e7b6aee9ec978650b53) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jcrosss0.f1",   0x0000, 0x2000, CRC(9ae8ea93) SHA1(1d824302305a41bf5c354c36e2e11981d1aa5ea4) )
	ROM_LOAD( "jcrosss1.h2",   0x2000, 0x2000, CRC(83785601) SHA1(cd3d484ef5464090c4b543b1edbbedcc52b15071) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "jcrossb4.10a",  0x0000, 0x2000, CRC(08ad93fe) SHA1(04baf2d9735b0d794b114abeced5a6b899958ce7) )
	ROM_LOAD( "jcrosss.d2",    0x2000, 0x2000, CRC(3ebb5beb) SHA1(de0a1f0fdb5b08b76dab9fa64d9ae3047c4ff84b) )

	ROM_REGION( 0x2000, "bg_tiles", 0 )
	ROM_LOAD( "jcrossb1.a2",   0x0000, 0x2000, CRC(ea3dfbc9) SHA1(eee56acd1c9dbc6c3ecdee4ffe860273e65cc09b) )

	ROM_REGION( 0x6000, "sp16_tiles", 0 )
	ROM_LOAD( "jcrossf0.l2",   0x0000, 0x2000, CRC(4532509b) SHA1(c99f87e2b06b94d815e6099bccb2aee0edf8c98d) )
	ROM_LOAD( "jcrossf1.k2",   0x2000, 0x2000, CRC(70d219bf) SHA1(9ff9f88221edd141e8204ac810434b4290db7cff) )
	ROM_LOAD( "jcrossf2.j2",   0x4000, 0x2000, CRC(42a12b9d) SHA1(9f2bdb1f84f444442282cf0fc1f7b3c7f9a9bf48) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "jcrossp2.j7",  0x000, 0x400, CRC(b72a96a5) SHA1(20d40e4b6a2652e61dc3ad0c4afaec04e3c7cf74) )
	ROM_LOAD( "jcrossp1.j8",  0x400, 0x400, CRC(35650448) SHA1(17e4a661ff304c093bb0253efceaf4e9b2498924) )
	ROM_LOAD( "jcrossp0.j9",  0x800, 0x400, CRC(99f54d48) SHA1(9bd20eaa9706d28eaca9f5e195204d89e302272f) )
ROM_END

/***********************************************************************/

ROM_START( sgladiat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "glad.005",   0x0000, 0x4000, CRC(4bc60f0b) SHA1(19baf7533b8fc6bab372f1d35603068a6b93627c) )
	ROM_LOAD( "glad.004",   0x4000, 0x4000, CRC(db557f46) SHA1(dc3565096c95a20d2c64dd4f5d0b465fbd85e041) )
	ROM_LOAD( "glad.003",   0x8000, 0x2000, CRC(55ce82b4) SHA1(703f017a8501e4dd5166b5717b244aa6b1e7dc0a) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "glad.002",   0x0000, 0x4000, CRC(8350261c) SHA1(046099128d5d941b3a37dce559ff5f0aa7f61683) )
	ROM_LOAD( "glad.001",   0x4000, 0x4000, CRC(5ee9d3fb) SHA1(6c8d8db7a966d3d3a2e8c46fd779e12e1f1e3716) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "glad.007",  0x0000, 0x2000, CRC(c25b6731) SHA1(519c6844bfec958b9bb65f148b3527b41fe38b99) )
	ROM_LOAD( "glad.006",  0x2000, 0x2000, CRC(2024d716) SHA1(6ff069fc53524d13c386e8e714ba3056509adc4d) )

	ROM_REGION( 0x2000, "tx_tiles", 0 )
	ROM_LOAD( "glad.011",   0x0000, 0x2000, CRC(305bb294) SHA1(e148571a581b12ff9502a65ec428e4d19bc757cb) )

	ROM_REGION( 0x2000, "bg_tiles", 0 )
	ROM_LOAD( "glad.012",   0x0000, 0x2000, CRC(b7dd519f) SHA1(7bd760e54712648105d1049e678fa6b9effa600b) )

	ROM_REGION( 0x6000, "sp16_tiles", 0 )
	ROM_LOAD( "glad.008", 0x0000, 0x2000, CRC(bcf42587) SHA1(1546fe903fbc6dc0b410c83ab51887c33c91ec2d) )
	ROM_LOAD( "glad.009", 0x2000, 0x2000, CRC(912a20e0) SHA1(9621b955bc00b7c52ed8363bb441b568efb55863) )
	ROM_LOAD( "glad.010", 0x4000, 0x2000, CRC(8b1db3a5) SHA1(5ca403d40071ab13deb7fdb04cb0e055e6b30b05) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137.001",  0x000, 0x400, CRC(d9184823) SHA1(455c6a437d54c29673dddb8248ca78d000c7f354) )
	ROM_LOAD( "82s137.002",  0x400, 0x400, CRC(1a6b0953) SHA1(552ac2897abe507f2fd9ca11c8128a0314af215c) )
	ROM_LOAD( "82s137.003",  0x800, 0x400, CRC(c0e70308) SHA1(d7dbc500bc9991c2d1b95850f3723a2a224fbfbb) )
ROM_END

/***********************************************************************/

ROM_START( hal21 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hal21p1.bin",    0x0000, 0x2000, CRC(9d193830) SHA1(8e4e9c8bc774d7c7c0b68a5fa5cabdc6b5cfa41b) )
	ROM_LOAD( "hal21p2.bin",    0x2000, 0x2000, CRC(c1f00350) SHA1(8709455a980931565ccca60162a04c6c3133099b) )
	ROM_LOAD( "hal21p3.bin",    0x4000, 0x2000, CRC(881d22a6) SHA1(4b2a65dc18620f7f77532f791212fccfe1f0b245) )
	ROM_LOAD( "hal21p4.bin",    0x6000, 0x2000, CRC(ce692534) SHA1(e1d8e6948578ec9d0b6dc2aff17ad23b8ce46d6a) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "hal21p5.bin",    0x0000, 0x2000, CRC(3ce0684a) SHA1(5e76770a3252d5565a8f11a79ac3a9a6c31a43e2) )
	ROM_LOAD( "hal21p6.bin",    0x2000, 0x2000, CRC(878ef798) SHA1(0aae152947c9c6733b77dd1ac14f2f6d6bfabeaa) )
	ROM_LOAD( "hal21p7.bin",    0x4000, 0x2000, CRC(72ebbe95) SHA1(b1f7dc535e7670647500391d21dfa971d5e342a2) )
	ROM_LOAD( "hal21p8.bin",    0x6000, 0x2000, CRC(17e22ad3) SHA1(0e10a3c0f2e2ec284f4e0f1055397a8ccd1ff0f7) )
	ROM_LOAD( "hal21p9.bin",    0x8000, 0x2000, CRC(b146f891) SHA1(0b2db3e14b0401a7914002c6f7c26933a1cba162) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "hal21p10.bin",   0x0000, 0x4000, CRC(916f7ba0) SHA1(7b8bcd59d768c4cd226de96895d3b9755bb3ba79) )

	ROM_REGION( 0x2000, "tx_tiles", 0 )
	ROM_LOAD( "hal21p12.bin", 0x0000, 0x2000, CRC(9839a7cd) SHA1(d3f9d964263a64aa3648faf5eb2e4fa532ae7852) )

	ROM_REGION( 0x4000, "bg_tiles", 0 )
	ROM_LOAD( "hal21p11.bin", 0x0000, 0x4000, CRC(24abc57e) SHA1(1d7557a62adc059fb3fe20a09be18c2f40441581) )

	ROM_REGION( 0xc000, "sp16_tiles", 0 )
	ROM_LOAD( "hal21p13.bin", 0x00000, 0x4000, CRC(052b4f4f) SHA1(032eb5771d33defce86e222f3e7aa22bc37db6db) )
	ROM_LOAD( "hal21p14.bin", 0x04000, 0x4000, CRC(da0cb670) SHA1(1083bdd3488dfaa5094a2ef52cfc4206f35c9612) )
	ROM_LOAD( "hal21p15.bin", 0x08000, 0x4000, CRC(5c5ea945) SHA1(f9ce206cab4fad1f6478d731d4b096ec33e7b99f) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "hal21_3.prm",  0x000, 0x400, CRC(605afff8) SHA1(94e80ebd574b1580dac4a2aebd57e3e767890c0d) )
	ROM_LOAD( "hal21_2.prm",  0x400, 0x400, CRC(c5d84225) SHA1(cc2cd32f81ed7c1bcdd68e91d00f8081cb706ce7) )
	ROM_LOAD( "hal21_1.prm",  0x800, 0x400, CRC(195768fc) SHA1(c88bc9552d57d52fb4b030d118f48fedccf563f4) )
ROM_END

ROM_START( hal21j )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hal21p1.bin",    0x0000, 0x2000, CRC(9d193830) SHA1(8e4e9c8bc774d7c7c0b68a5fa5cabdc6b5cfa41b) )
	ROM_LOAD( "hal21p2.bin",    0x2000, 0x2000, CRC(c1f00350) SHA1(8709455a980931565ccca60162a04c6c3133099b) )
	ROM_LOAD( "hal21p3.bin",    0x4000, 0x2000, CRC(881d22a6) SHA1(4b2a65dc18620f7f77532f791212fccfe1f0b245) )
	ROM_LOAD( "hal21p4.bin",    0x6000, 0x2000, CRC(ce692534) SHA1(e1d8e6948578ec9d0b6dc2aff17ad23b8ce46d6a) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "hal21p5.bin",    0x0000, 0x2000, CRC(3ce0684a) SHA1(5e76770a3252d5565a8f11a79ac3a9a6c31a43e2) )
	ROM_LOAD( "hal21p6.bin",    0x2000, 0x2000, CRC(878ef798) SHA1(0aae152947c9c6733b77dd1ac14f2f6d6bfabeaa) )
	ROM_LOAD( "hal21p7.bin",    0x4000, 0x2000, CRC(72ebbe95) SHA1(b1f7dc535e7670647500391d21dfa971d5e342a2) )
	ROM_LOAD( "hal21p8.bin",    0x6000, 0x2000, CRC(17e22ad3) SHA1(0e10a3c0f2e2ec284f4e0f1055397a8ccd1ff0f7) )
	ROM_LOAD( "hal21p9.bin",    0x8000, 0x2000, CRC(b146f891) SHA1(0b2db3e14b0401a7914002c6f7c26933a1cba162) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "hal21-10.bin",   0x0000, 0x4000, CRC(a182b3f0) SHA1(b76eff97a58a96467e9f3a74125a0a770e7678f8) )

	ROM_REGION( 0x2000, "tx_tiles", 0 )
	ROM_LOAD( "hal21p12.bin", 0x0000, 0x2000, CRC(9839a7cd) SHA1(d3f9d964263a64aa3648faf5eb2e4fa532ae7852) )

	ROM_REGION( 0x4000, "bg_tiles", 0 )
	ROM_LOAD( "hal21p11.bin", 0x0000, 0x4000, CRC(24abc57e) SHA1(1d7557a62adc059fb3fe20a09be18c2f40441581) )

	ROM_REGION( 0xc000, "sp16_tiles", 0 )
	ROM_LOAD( "hal21p13.bin", 0x00000, 0x4000, CRC(052b4f4f) SHA1(032eb5771d33defce86e222f3e7aa22bc37db6db) )
	ROM_LOAD( "hal21p14.bin", 0x04000, 0x4000, CRC(da0cb670) SHA1(1083bdd3488dfaa5094a2ef52cfc4206f35c9612) )
	ROM_LOAD( "hal21p15.bin", 0x08000, 0x4000, CRC(5c5ea945) SHA1(f9ce206cab4fad1f6478d731d4b096ec33e7b99f) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "hal21_3.prm",  0x000, 0x400, CRC(605afff8) SHA1(94e80ebd574b1580dac4a2aebd57e3e767890c0d) )
	ROM_LOAD( "hal21_2.prm",  0x400, 0x400, CRC(c5d84225) SHA1(cc2cd32f81ed7c1bcdd68e91d00f8081cb706ce7) )
	ROM_LOAD( "hal21_1.prm",  0x800, 0x400, CRC(195768fc) SHA1(c88bc9552d57d52fb4b030d118f48fedccf563f4) )
ROM_END

/***********************************************************************/

ROM_START( aso )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "aso_p1.d8", 0x0000, 0x4000, CRC(84981f3c) SHA1(5b6af7cf47f5f664df7ddc3615b4da0dea257a05) )
	ROM_LOAD( "aso_p2.d7", 0x4000, 0x4000, CRC(cfe912a6) SHA1(60138a7dcff8a65a33209619fcb58be313a77511) )
	ROM_LOAD( "aso_p3.d5", 0x8000, 0x4000, CRC(39a666d2) SHA1(b5426520eb600d44bc5566d742d7b88194076494) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "aso_p4.d3", 0x0000, 0x4000, CRC(a4122355) SHA1(bca2b7f5abec502bde12ce591ee7af1329ce8ce5) )
	ROM_LOAD( "p5.d2",     0x4000, 0x4000, CRC(9879e506) SHA1(0bce5fcb9d05ce77cd8e9ad1cac04ef617928db0) )
	ROM_LOAD( "p6.d1",     0x8000, 0x4000, CRC(c0bfdf1f) SHA1(65b15ce9c2e78df79cb603c58639421d29701633) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p7.f4", 0x0000, 0x4000, CRC(dbc19736) SHA1(fe365d70ead8243374979d2162c395fed9870405) )
	ROM_LOAD( "p8.f3", 0x4000, 0x4000, CRC(537726a9) SHA1(ddf66946be71d2e6ab2cc53150e3b36d45dde2eb) )
	ROM_LOAD( "p9.f2", 0x8000, 0x4000, CRC(aef5a4f4) SHA1(e908e79e27ff892fe75d1ba5cb0bc9dc6b7b4268) )

	ROM_REGION( 0x2000, "tx_tiles", 0 )
	ROM_LOAD( "aso_p14.h1", 0x0000, 0x2000, CRC(8baa2253) SHA1(e6e4a5aa005e89744c4e2a19a080cf322edc6b52) )

	ROM_REGION( 0x8000, "bg_tiles", 0 )
	ROM_LOAD( "p10.h14", 0x0000, 0x8000, CRC(00dff996) SHA1(4f6ce4c0f2da0d2a711bcbf9aa998b4e31d0d9bf) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "p11.h11", 0x04000, 0x4000, CRC(7feac86c) SHA1(13b81f006ec587583416c1e7432da4c3f0375924) )
	ROM_CONTINUE(        0x00000, 0x4000)
	ROM_LOAD( "p12.h9",  0x0c000, 0x4000, CRC(6895990b) SHA1(e84554cae9a768021c3dc7183bc3d28e2dd768ee) )
	ROM_CONTINUE(        0x08000, 0x4000)
	ROM_LOAD( "p13.h8",  0x14000, 0x4000, CRC(87a81ce1) SHA1(28c1069e6c08ecd579f99620c1cb6df01ad1aa74) )
	ROM_CONTINUE(        0x10000, 0x4000)

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "mb7122h.f12",  0x000, 0x00400, CRC(5b0a0059) SHA1(f61e17c8959f1cd6cc12b38f2fb7c6190ebd0e0c) )
	ROM_LOAD( "mb7122h.f13",  0x400, 0x00400, CRC(37e28dd8) SHA1(681726e490872a574dd0295823a44d64ef3a7b45) )
	ROM_LOAD( "mb7122h.f14",  0x800, 0x00400, CRC(c3fd1dd3) SHA1(c48030cc458f0bebea0ffccf3d3c43260da6a7fb) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8a-1.bin", 0x0000, 0x0104, CRC(4e3f9e0d) SHA1(de448d50c0d1cdef159a8c4028846142210eba0b) )
	ROM_LOAD( "pal16l8a-2.bin", 0x0200, 0x0104, CRC(2a681f9e) SHA1(b26eb631d3e4fa6850a109a9a63d377cf86923bc) )
	ROM_LOAD( "pal16r6a.bin",   0x0400, 0x0104, CRC(59c03681) SHA1(d21090b35596c28d44862782386e84dfc1feff0c) )
ROM_END

ROM_START( alphamis )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.rom",  0x0000, 0x4000, CRC(69af874b) SHA1(11a13574614e7e3b9e33c2b2827571946a805376) )
	ROM_LOAD( "p2.rom",  0x4000, 0x4000, CRC(7707bfe3) SHA1(fb1f4ef862f6762d2479e537fc67a819d11ace76) )
	ROM_LOAD( "p3.rom",  0x8000, 0x4000, CRC(b970d642) SHA1(d3a8045f05f001e5e2fae8ef7900cf87ab17fc74) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "p4.rom",    0x0000, 0x4000, CRC(91a89d3c) SHA1(46ef8718c81aac2f09dd1884538750edf9662760) )
	ROM_LOAD( "p5.d2",     0x4000, 0x4000, CRC(9879e506) SHA1(0bce5fcb9d05ce77cd8e9ad1cac04ef617928db0) )
	ROM_LOAD( "p6.d1",     0x8000, 0x4000, CRC(c0bfdf1f) SHA1(65b15ce9c2e78df79cb603c58639421d29701633) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p7.f4", 0x0000, 0x4000, CRC(dbc19736) SHA1(fe365d70ead8243374979d2162c395fed9870405) )
	ROM_LOAD( "p8.f3", 0x4000, 0x4000, CRC(537726a9) SHA1(ddf66946be71d2e6ab2cc53150e3b36d45dde2eb) )
	ROM_LOAD( "p9.f2", 0x8000, 0x4000, CRC(aef5a4f4) SHA1(e908e79e27ff892fe75d1ba5cb0bc9dc6b7b4268) )

	ROM_REGION( 0x2000, "tx_tiles", 0 )
	ROM_LOAD( "p14.rom", 0x0000, 0x2000, CRC(acbe29b2) SHA1(e304c6d30888fa7549d25e6329ba94d5088bd8b7) )

	ROM_REGION( 0x8000, "bg_tiles", 0 )
	ROM_LOAD( "p10.h14", 0x0000, 0x8000, CRC(00dff996) SHA1(4f6ce4c0f2da0d2a711bcbf9aa998b4e31d0d9bf) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "p11.h11", 0x04000, 0x4000, CRC(7feac86c) SHA1(13b81f006ec587583416c1e7432da4c3f0375924) )
	ROM_CONTINUE(        0x00000, 0x4000)
	ROM_LOAD( "p12.h9",  0x0c000, 0x4000, CRC(6895990b) SHA1(e84554cae9a768021c3dc7183bc3d28e2dd768ee) )
	ROM_CONTINUE(        0x08000, 0x4000)
	ROM_LOAD( "p13.h8",  0x14000, 0x4000, CRC(87a81ce1) SHA1(28c1069e6c08ecd579f99620c1cb6df01ad1aa74) )
	ROM_CONTINUE(        0x10000, 0x4000)

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "mb7122h.f12",  0x000, 0x00400, CRC(5b0a0059) SHA1(f61e17c8959f1cd6cc12b38f2fb7c6190ebd0e0c) )
	ROM_LOAD( "mb7122h.f13",  0x400, 0x00400, CRC(37e28dd8) SHA1(681726e490872a574dd0295823a44d64ef3a7b45) )
	ROM_LOAD( "mb7122h.f14",  0x800, 0x00400, CRC(c3fd1dd3) SHA1(c48030cc458f0bebea0ffccf3d3c43260da6a7fb) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8a-1.bin", 0x0000, 0x0104, CRC(4e3f9e0d) SHA1(de448d50c0d1cdef159a8c4028846142210eba0b) )
	ROM_LOAD( "pal16l8a-2.bin", 0x0200, 0x0104, CRC(2a681f9e) SHA1(b26eb631d3e4fa6850a109a9a63d377cf86923bc) )
	ROM_LOAD( "pal16r6a.bin",   0x0400, 0x0104, CRC(59c03681) SHA1(d21090b35596c28d44862782386e84dfc1feff0c) )
ROM_END

ROM_START( arian )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.d8",   0x0000, 0x4000, CRC(0ca89307) SHA1(d0ecb97b1e147a4001a4383fd5709394e2358a45) ) /* roms that differ from above sets all had a red stripe on the label */
	ROM_LOAD( "p2.d7",   0x4000, 0x4000, CRC(724518c3) SHA1(debbfe2a485af5f452d208a04705dbd48d47d90f) ) /* IE: P1 through P4 and P14 */
	ROM_LOAD( "p3.d6",   0x8000, 0x4000, CRC(4d8db650) SHA1(184141847d38077737ee7140861d94832018e2e2) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "p4.d3",     0x0000, 0x4000, CRC(47baf1db) SHA1(3947a679745811e5499d690f2b73b4f28b1d47f9) )
	ROM_LOAD( "p5.d2",     0x4000, 0x4000, CRC(9879e506) SHA1(0bce5fcb9d05ce77cd8e9ad1cac04ef617928db0) )
	ROM_LOAD( "p6.d1",     0x8000, 0x4000, CRC(c0bfdf1f) SHA1(65b15ce9c2e78df79cb603c58639421d29701633) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p7.f4", 0x0000, 0x4000, CRC(dbc19736) SHA1(fe365d70ead8243374979d2162c395fed9870405) )
	ROM_LOAD( "p8.f3", 0x4000, 0x4000, CRC(537726a9) SHA1(ddf66946be71d2e6ab2cc53150e3b36d45dde2eb) )
	ROM_LOAD( "p9.f2", 0x8000, 0x4000, CRC(aef5a4f4) SHA1(e908e79e27ff892fe75d1ba5cb0bc9dc6b7b4268) )

	ROM_REGION( 0x2000, "tx_tiles", 0 )
	ROM_LOAD( "p14.h1",  0x0000, 0x2000, CRC(e599bd30) SHA1(bf70aae9a15d548bb532ca1fc8d7220dfa150d6e) )

	ROM_REGION( 0x8000, "bg_tiles", 0 )
	ROM_LOAD( "p10.h14", 0x0000, 0x8000, CRC(00dff996) SHA1(4f6ce4c0f2da0d2a711bcbf9aa998b4e31d0d9bf) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "p11.h11", 0x04000, 0x4000, CRC(7feac86c) SHA1(13b81f006ec587583416c1e7432da4c3f0375924) )
	ROM_CONTINUE(        0x00000, 0x4000)
	ROM_LOAD( "p12.h9",  0x0c000, 0x4000, CRC(6895990b) SHA1(e84554cae9a768021c3dc7183bc3d28e2dd768ee) )
	ROM_CONTINUE(        0x08000, 0x4000)
	ROM_LOAD( "p13.h8",  0x14000, 0x4000, CRC(87a81ce1) SHA1(28c1069e6c08ecd579f99620c1cb6df01ad1aa74) )
	ROM_CONTINUE(        0x10000, 0x4000)

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "mb7122h.f12",  0x000, 0x00400, CRC(5b0a0059) SHA1(f61e17c8959f1cd6cc12b38f2fb7c6190ebd0e0c) )
	ROM_LOAD( "mb7122h.f13",  0x400, 0x00400, CRC(37e28dd8) SHA1(681726e490872a574dd0295823a44d64ef3a7b45) )
	ROM_LOAD( "mb7122h.f14",  0x800, 0x00400, CRC(c3fd1dd3) SHA1(c48030cc458f0bebea0ffccf3d3c43260da6a7fb) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8a-1.bin", 0x0000, 0x0104, CRC(4e3f9e0d) SHA1(de448d50c0d1cdef159a8c4028846142210eba0b) )
	ROM_LOAD( "pal16l8a-2.bin", 0x0200, 0x0104, CRC(2a681f9e) SHA1(b26eb631d3e4fa6850a109a9a63d377cf86923bc) )
	ROM_LOAD( "pal16r6a.bin",   0x0400, 0x0104, CRC(59c03681) SHA1(d21090b35596c28d44862782386e84dfc1feff0c) )
ROM_END

/***********************************************************************/

ROM_START( tnk3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tnk3-p1.bin",  0x0000, 0x4000, CRC(0d2a8ca9) SHA1(eba950dab044496e8c1c02af20a9d380996ea20a) )
	ROM_LOAD( "tnk3-p2.bin",  0x4000, 0x4000, CRC(0ae0a483) SHA1(6a1ba86da4fd75bfb00855db04eac2727ec4159e) )
	ROM_LOAD( "tnk3-p3.bin",  0x8000, 0x4000, CRC(d16dd4db) SHA1(dcbc61251c13e11ce3cdd7a5ad200cd2d2758cab) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "tnk3-p4.bin",  0x0000, 0x4000, CRC(01b45a90) SHA1(85ba3b157cd6463c92ed831bb48d38f3a16f9537) )
	ROM_LOAD( "tnk3-p5.bin",  0x4000, 0x4000, CRC(60db6667) SHA1(9c4bb99473c6d9b8ac9086b7364b6278b70757f6) )
	ROM_LOAD( "tnk3-p6.bin",  0x8000, 0x4000, CRC(4761fde7) SHA1(dadf60e33f5dd8108478ca480bcef6b2624cfca8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "tnk3-p10.bin",  0x0000, 0x4000, CRC(7bf0a517) SHA1(0197feeaf511ac59f3df8195ec57e947fb08e995) )
	ROM_LOAD( "tnk3-p11.bin",  0x4000, 0x4000, CRC(0569ce27) SHA1(7aa73f57ad97445ce5729f05cd8d24973886dbf5) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "7122.2",  0x000, 0x400, CRC(34c06bc6) SHA1(bb68e96a8fcc754840420952dab961e03bf6acdd) )
	ROM_LOAD( "7122.1",  0x400, 0x400, CRC(6d0ac66a) SHA1(e792218ec43dd10473dc020afed8527cf43ea0d0) )
	ROM_LOAD( "7122.0",  0x800, 0x400, CRC(4662b4c8) SHA1(391c2b8a17ce2e092b46a17fc4170dc1e3bde426) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "tnk3-p14.bin", 0x0000, 0x2000, CRC(1fd18c43) SHA1(611b5aa97df84c0117681772deb006f32a899ad3) )
	ROM_RELOAD(               0x2000, 0x2000 )

	ROM_REGION( 0x8000, "bg_tiles", 0 )
	ROM_LOAD( "tnk3-p12.bin", 0x0000, 0x4000, CRC(ff495a16) SHA1(e6b97a63efe58018260ff34f0ea4edc81718cb14) )
	ROM_LOAD( "tnk3-p13.bin", 0x4000, 0x4000, CRC(f8344843) SHA1(c741dc84b48f830f6d4eaa4476f5c2a391153acc) )

	ROM_REGION( 0x0c000, "sp16_tiles", 0 )
	ROM_LOAD( "tnk3-p7.bin", 0x00000, 0x4000, CRC(06b92c88) SHA1(b39c2cc4a58937d89f9b0c9093b9742509db64a3) )
	ROM_LOAD( "tnk3-p8.bin", 0x04000, 0x4000, CRC(63d0e2eb) SHA1(96182639bb620d9692a4c8266130769c44dd29f8) )
	ROM_LOAD( "tnk3-p9.bin", 0x08000, 0x4000, CRC(872e3fac) SHA1(98e7e9315fe7ccc51151c67dc60a362a1c2d8372) )
ROM_END

ROM_START( tnk3j )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.4e",  0x0000, 0x4000, CRC(03aca147) SHA1(9ce4cfdfbd22f10e13c8e474dc2e5aa3bfd57e0b) )
	ROM_LOAD( "tnk3-p2.bin",  0x4000, 0x4000, CRC(0ae0a483) SHA1(6a1ba86da4fd75bfb00855db04eac2727ec4159e) )
	ROM_LOAD( "tnk3-p3.bin",  0x8000, 0x4000, CRC(d16dd4db) SHA1(dcbc61251c13e11ce3cdd7a5ad200cd2d2758cab) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "tnk3-p4.bin",  0x0000, 0x4000, CRC(01b45a90) SHA1(85ba3b157cd6463c92ed831bb48d38f3a16f9537) )
	ROM_LOAD( "tnk3-p5.bin",  0x4000, 0x4000, CRC(60db6667) SHA1(9c4bb99473c6d9b8ac9086b7364b6278b70757f6) )
	ROM_LOAD( "tnk3-p6.bin",  0x8000, 0x4000, CRC(4761fde7) SHA1(dadf60e33f5dd8108478ca480bcef6b2624cfca8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "tnk3-p10.bin",  0x0000, 0x4000, CRC(7bf0a517) SHA1(0197feeaf511ac59f3df8195ec57e947fb08e995) )
	ROM_LOAD( "tnk3-p11.bin",  0x4000, 0x4000, CRC(0569ce27) SHA1(7aa73f57ad97445ce5729f05cd8d24973886dbf5) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "7122.2",  0x000, 0x400, CRC(34c06bc6) SHA1(bb68e96a8fcc754840420952dab961e03bf6acdd) )
	ROM_LOAD( "7122.1",  0x400, 0x400, CRC(6d0ac66a) SHA1(e792218ec43dd10473dc020afed8527cf43ea0d0) )
	ROM_LOAD( "7122.0",  0x800, 0x400, CRC(4662b4c8) SHA1(391c2b8a17ce2e092b46a17fc4170dc1e3bde426) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "p14.1e", 0x0000, 0x2000, CRC(6bd575ca) SHA1(446bb929fa19a7ff8b92731f71ab3e3252899f07) )
	ROM_RELOAD(         0x2000, 0x2000 )

	ROM_REGION( 0x8000, "bg_tiles", 0 )
	ROM_LOAD( "tnk3-p12.bin", 0x0000, 0x4000, CRC(ff495a16) SHA1(e6b97a63efe58018260ff34f0ea4edc81718cb14) )
	ROM_LOAD( "tnk3-p13.bin", 0x4000, 0x4000, CRC(f8344843) SHA1(c741dc84b48f830f6d4eaa4476f5c2a391153acc) )

	ROM_REGION( 0x0c000, "sp16_tiles", 0 )
	ROM_LOAD( "tnk3-p7.bin", 0x00000, 0x4000, CRC(06b92c88) SHA1(b39c2cc4a58937d89f9b0c9093b9742509db64a3) )
	ROM_LOAD( "tnk3-p8.bin", 0x04000, 0x4000, CRC(63d0e2eb) SHA1(96182639bb620d9692a4c8266130769c44dd29f8) )
	ROM_LOAD( "tnk3-p9.bin", 0x08000, 0x4000, CRC(872e3fac) SHA1(98e7e9315fe7ccc51151c67dc60a362a1c2d8372) )
ROM_END

/***********************************************************************/

ROM_START( athena )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "up02_p4.rom",  0x0000, 0x4000,  CRC(900a113c) SHA1(3a85f87cbf79d60f58858df4852d6d97300c9280) )
	ROM_LOAD( "up02_m4.rom",  0x4000, 0x8000,  CRC(61c69474) SHA1(93f1222a3908c84fe6679e2deb90afbe4a22e675) )

	ROM_REGION(  0x10000 , "sub", 0 )
	ROM_LOAD( "up02_p8.rom",  0x0000, 0x4000, CRC(df50af7e) SHA1(2a69089aecf598cb11f4f1c9b42d81670f9bd68e) )
	ROM_LOAD( "up02_m8.rom",  0x4000, 0x8000, CRC(f3c933df) SHA1(70a0bf63230be53da9196fae4c3e604205275ddd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "up02_g6.rom",  0x0000, 0x4000, CRC(42dbe029) SHA1(9aa311860693bd3e73f2b72ca4b171cb95f069ee) )
	ROM_LOAD( "up02_k6.rom",  0x4000, 0x8000, CRC(596f1c8a) SHA1(8f1400c77473c845e57a14fa479cf4f7ac66a909) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "up02_c2.rom",  0x000, 0x400, CRC(294279ae) SHA1(b3db5617b83845a6c1abca8f71fa4598758a2a56) )
	ROM_LOAD( "up02_b1.rom",  0x400, 0x400, CRC(d25c9099) SHA1(f3933075cce1255affc61dfefd9559b6e15ed29c) )
	ROM_LOAD( "up02_c1.rom",  0x800, 0x400, CRC(a4a4e7dc) SHA1(aa694c2d44dcabc6cfd46307c55c3759eff57236) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "up01_d2.rom",  0x0000, 0x4000,  CRC(18b4bcca) SHA1(2476aa6c8d55e117d840202a97fe2a65e252ad7f) )

	ROM_REGION( 0x8000, "bg_tiles", 0 )
	ROM_LOAD( "up01_b2.rom",  0x0000, 0x8000,  CRC(f269c0eb) SHA1(a947c6e4d82e0aafa616d25395ef63c33d9beb06) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "up01_p2.rom",  0x00000, 0x8000, CRC(c63a871f) SHA1(0ab8ebebd750fdcad283eed427179f2124b300ae) )
	ROM_LOAD( "up01_s2.rom",  0x08000, 0x8000, CRC(760568d8) SHA1(9dc447c446791c79322e21e3caef6ceae347e2fb) )
	ROM_LOAD( "up01_t2.rom",  0x10000, 0x8000, CRC(57b35c73) SHA1(6d15b94b50c3734f7d60bd9bd1c5e6c76591d829) )
ROM_END

/***********************************************************************/

ROM_START( fitegolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gu2",    0x0000, 0x4000, CRC(19be7ad6) SHA1(6f0faf606e44a3f8cc027699cc816aa3414a1b98) )
	ROM_LOAD( "gu1",    0x4000, 0x8000, CRC(bc32568f) SHA1(35fec3dbdd773ec7f427ecdd81066fb8f1b74e05) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gu6",    0x0000, 0x4000, CRC(2b9978c5) SHA1(5490e9f796697318650fc5f70c0e64d6785ad7fc) )
	ROM_LOAD( "gu5",    0x4000, 0x8000, CRC(ea3d138c) SHA1(af0a0bfe2d266179946948cf42fe697505798a4f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gu3",    0x0000, 0x4000, CRC(811b87d7) SHA1(fb387f42085d6e0e5a88729ca0e50656411ce037) )
	ROM_LOAD( "gu4",    0x4000, 0x8000, CRC(2d998e2b) SHA1(a471cfbb4dabc90fcc29c562620b9965eaff6861) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137.2c",  0x00000, 0x00400, CRC(6e4c7836) SHA1(3ab3c498939fac992e2bf1c33983ee821a9b6a18) )
	ROM_LOAD( "82s137.1b",  0x00400, 0x00400, CRC(29e7986f) SHA1(85ba8d3443458c27728f633745857a1315dd183f) )
	ROM_LOAD( "82s137.1c",  0x00800, 0x00400, CRC(27ba9ff9) SHA1(f021d10460f40de4447560df5ac47fa53bb57ff9) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "gu8",   0x0000, 0x4000, CRC(f1628dcf) SHA1(efea343d3a9dd45ef74947c297e166e34afbb680) )

	ROM_REGION( 0x8000, "bg_tiles", 0 )
	ROM_LOAD( "gu7",  0x0000, 0x8000, CRC(4655f94e) SHA1(08526206d8e929bb01d61fff8de2ee99fd287c17) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "gu9",   0x00000, 0x8000, CRC(d4957ec5) SHA1(8ead7866ba5ac66ead6b707aa868bcae30c486e1) )
	ROM_LOAD( "gu10",  0x08000, 0x8000, CRC(b3acdac2) SHA1(7377480d5e1b5ab2c49f5fee2927623ce8240e19) )
	ROM_LOAD( "gu11",  0x10000, 0x8000, CRC(b99cf73b) SHA1(23989fc3914e77d364807a9eb96a4ddf75ad7cf1) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16r6a.6c", 0x0000, 0x0104, CRC(de291f4e) SHA1(b50294d30cb8eacc7a9bb8b46695a7463ef45ff1) )
	ROM_LOAD( "pal16l8a.3f", 0x0200, 0x0104, CRC(c5f1c1da) SHA1(e17293be0f77d302c59c1095fe1ec65e45557627) )
	ROM_LOAD( "pal20l8a.6r", 0x0400, 0x0144, CRC(0f011673) SHA1(383e6f6e78daec9c874d5b48378111ca60f5ed64) )
ROM_END

ROM_START( fitegolfu )  /*  Later US version containing enhancements to make the game a little easier */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "np45.128", 0x0000, 0x4000, CRC(16e8e763) SHA1(0b5296f2a91a7f3176b7461ca4958865ce998241) )
	ROM_LOAD( "mn45.256", 0x4000, 0x8000, CRC(a4fa09d5) SHA1(ae7f0cb47de06006ae71252c4201a93a01a26887) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gu6",    0x0000, 0x4000, CRC(2b9978c5) SHA1(5490e9f796697318650fc5f70c0e64d6785ad7fc) )  // NP8.256
	ROM_LOAD( "gu5",    0x4000, 0x8000, CRC(ea3d138c) SHA1(af0a0bfe2d266179946948cf42fe697505798a4f) )  // MN8.256

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gu3",    0x0000, 0x4000, CRC(811b87d7) SHA1(fb387f42085d6e0e5a88729ca0e50656411ce037) )  // FG67.256
	ROM_LOAD( "gu4",    0x4000, 0x8000, CRC(2d998e2b) SHA1(a471cfbb4dabc90fcc29c562620b9965eaff6861) )  // K67.256

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137.2c",  0x00000, 0x00400, CRC(6e4c7836) SHA1(3ab3c498939fac992e2bf1c33983ee821a9b6a18) )
	ROM_LOAD( "82s137.1b",  0x00400, 0x00400, CRC(29e7986f) SHA1(85ba8d3443458c27728f633745857a1315dd183f) )
	ROM_LOAD( "82s137.1c",  0x00800, 0x00400, CRC(27ba9ff9) SHA1(f021d10460f40de4447560df5ac47fa53bb57ff9) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "gu8",   0x0000, 0x4000, CRC(f1628dcf) SHA1(efea343d3a9dd45ef74947c297e166e34afbb680) )       // D2.128

	ROM_REGION( 0x8000, "bg_tiles", 0 )
	ROM_LOAD( "gu7",  0x0000, 0x8000, CRC(4655f94e) SHA1(08526206d8e929bb01d61fff8de2ee99fd287c17) )        // BC2.256

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "gu9",   0x00000, 0x8000, CRC(d4957ec5) SHA1(8ead7866ba5ac66ead6b707aa868bcae30c486e1) )  // P2.256
	ROM_LOAD( "gu10",  0x08000, 0x8000, CRC(b3acdac2) SHA1(7377480d5e1b5ab2c49f5fee2927623ce8240e19) )  // R2.256
	ROM_LOAD( "gu11",  0x10000, 0x8000, CRC(b99cf73b) SHA1(23989fc3914e77d364807a9eb96a4ddf75ad7cf1) )  // S2.256

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16r6a.6c", 0x0000, 0x0104, CRC(de291f4e) SHA1(b50294d30cb8eacc7a9bb8b46695a7463ef45ff1) )
	ROM_LOAD( "pal16l8a.3f", 0x0200, 0x0104, CRC(c5f1c1da) SHA1(e17293be0f77d302c59c1095fe1ec65e45557627) )
	ROM_LOAD( "pal20l8a.6r", 0x0400, 0x0144, CRC(0f011673) SHA1(383e6f6e78daec9c874d5b48378111ca60f5ed64) )
ROM_END

/***********************************************************************/

/*
Country Club
SNK, 1988

TOP PCB (sound board)
-------
PCB No: A7004
CPU   : Z80
SOUND : YM3812, Y3014B
XTAL  : 4.000MHz
RAM   : 6116 (x1)
DIPSW : 8 position (x2)
ROMs  :
        cc1.1f      27c512             sound program


2nd PCB (CPU board)
-------
PCB No: A5001UP02-01
CPU   : Z80 (x2)
RAM   : 6116 (x4), 2148 (x4)
OTHER : PALs (x2), TC4584 (used for trackball/spinner control)
ROMs  :
        cc2.2e      27c128      \
        cc3.2g        ''         | for Z80 #1
        cc4.2h        ''        /
        cc5.4e        ''        \
        cc6.4g        ''         | for Z80 #2
        cc7.4h        ''        /
        cc8.7e      27c256      \
        cc9.7g        ''         | gfx
        cc10.7h       ''        /
        cc1pr.5f    63s441      \
        cc2pr.5g      ''         | proms
        cc3pr.5h      ''        /


3rd PCB (Video board)
-------
PCB No: A5001UP01-01
XTAL  : 13.400MHz
RAM   : 6116 (x1), 6264 (x2)
OTHER : PAL (x1)
ROMs  :
        cc11.1e     27c128      \
        cc13.2d       ''         | gfx
        cc12.2c       ''        /
*/

ROM_START( countryc )
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "cc2.2e",  0x0000, 0x04000,  CRC(43d205e0) SHA1(d32f597bf2b70e326e68583cb95e0beeea34d5d0) )
	ROM_LOAD( "cc3.2g",  0x4000, 0x04000,  CRC(7290770f) SHA1(41184047e3e21f6ff4f724d59f4c6f34b19bcfc1) )
	ROM_LOAD( "cc4.2h",  0x8000, 0x04000,  CRC(61990582) SHA1(b12e6da3b8d7690bf6848a624b42dcb93f69ead7) )

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cc5.4e",  0x00000, 0x4000, CRC(07666af8) SHA1(4b4c51bd1bc5ee49bb516e6851b2e6b5a7780576) )
	ROM_LOAD( "cc6.4g",  0x04000, 0x4000, CRC(ab18fd9f) SHA1(30a30998191cb81a6bfcd672e54f8a155639ccd7) )
	ROM_LOAD( "cc7.4h",  0x08000, 0x4000, CRC(58a1ec0c) SHA1(877935463121a992851e9b76074e1a4d033a0b2e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cc1.1f",  0x00000, 0x10000, CRC(863f1624) SHA1(11c0aeefaddf16cc9e1c259e97b90fe418d70c89) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "cc1pr.5f",  0x000, 0x00400, CRC(7da9ce33) SHA1(42b272473986819e96633684b6dd9630ca2c37d6) )
	ROM_LOAD( "cc2pr.5g",  0x400, 0x00400, CRC(982e4f46) SHA1(c4703a35201bc4c6b43f629a9a6a4c66354c6305) )
	ROM_LOAD( "cc3pr.5h",  0x800, 0x00400, CRC(47f2b83d) SHA1(6335be47f09ad33d7e05fda26a2f3fb9048dbbc2) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "cc11.1e",  0x0000, 0x4000, CRC(ce927ac7) SHA1(a0dd281912aa9ae7e408c2132fae30bffbc83750) )

	ROM_REGION( 0x8000, "bg_tiles", 0 )
	ROM_LOAD( "cc13.2d",  0x0000, 0x4000, CRC(ef86c388) SHA1(19e443f6a4901a3c9db868964c08b0f58be1983d) )
	ROM_LOAD( "cc12.2c",  0x4000, 0x4000, CRC(d7d55a36) SHA1(1956097c2633f603cc1557f6e686b3c06b199dd8) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "cc10.7h",  0x00000, 0x8000, CRC(90091667) SHA1(d0d3813a0c3ac7e9e9ab824292dccb27c2087ea7) )
	ROM_LOAD( "cc9.7g",   0x08000, 0x8000, CRC(56249142) SHA1(10b703f15977ba21757aee3d212790372b35cc66) )
	ROM_LOAD( "cc8.7e",   0x10000, 0x8000, CRC(55943065) SHA1(ea545c6e8666c915994836d2f2cfc02db35e37c1) )
ROM_END

/***********************************************************************/

ROM_START( ikari )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.rom",  0x0000, 0x10000, CRC(52a8b2dd) SHA1(a896387d68ed9a55c313bdb81acdf8d68b7a1264) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "2.rom",  0x0000, 0x10000, CRC(45364d55) SHA1(323b998f782a4681ceb18016c5fb0fa1d6361aac) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "3.rom",  0x0000, 0x10000, CRC(56a26699) SHA1(e9ccb27f1e711e4648fdfe3c7ff956038d3e101c) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "7122er.prm",  0x000, 0x400, CRC(b9bf2c2c) SHA1(8eb62152dcb04f463baf6ec2a66148eb947403ef) )
	ROM_LOAD( "7122eg.prm",  0x400, 0x400, CRC(0703a770) SHA1(62861ef4987003d4965ef5018ccdf7157981d939) )
	ROM_LOAD( "7122eb.prm",  0x800, 0x400, CRC(0a11cdde) SHA1(faae17398341317e7afbd06b903b8e9e65967bf1) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "7.rom",    0x00000, 0x4000, CRC(a7eb4917) SHA1(6c07323cc243df4c5c30bc0daedbff3887309f65) )

	ROM_REGION( 0x20000, "bg_tiles", 0 )
	ROM_LOAD( "17.rom", 0x00000, 0x8000, CRC(e0dba976) SHA1(5a8f14f7a199b5fb1862debda0bceee42cddac59) )
	ROM_LOAD( "18.rom", 0x08000, 0x8000, CRC(24947d5f) SHA1(ffd18074ced8171c9da56c839e8289afc29af2c9) )
	ROM_LOAD( "19.rom", 0x10000, 0x8000, CRC(9ee59e91) SHA1(fe51d13ab73cb596a233669e304b2be66f9becae) )
	ROM_LOAD( "20.rom", 0x18000, 0x8000, CRC(5da7ec1a) SHA1(4b212c1dfe4c18eced90ee3a783e7edf8d23c906) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "8.rom",  0x00000, 0x8000, CRC(9827c14a) SHA1(b54dcee95c6f6e46c187a117b4e7aaf1c0ece6c6) )
	ROM_LOAD( "9.rom",  0x08000, 0x8000, CRC(545c790c) SHA1(7738738f4a1343b04efd029ecaefac74010451f0) )
	ROM_LOAD( "10.rom", 0x10000, 0x8000, CRC(ec9ba07e) SHA1(6b492b2cd7b8cca948ce39c3450f1cc153f41d90) )

	ROM_REGION( 0x30000, "sp32_tiles", 0 )
	ROM_LOAD( "11.rom", 0x00000, 0x8000, CRC(5c75ea8f) SHA1(4e8ee56a2dbeb9ac2dd74bc584dba29433d91ae0) )
	ROM_LOAD( "14.rom", 0x08000, 0x8000, CRC(3293fde4) SHA1(3e2f0fa00c22f1c0c1427d8d3de57dd9ec7682a9) )
	ROM_LOAD( "12.rom", 0x10000, 0x8000, CRC(95138498) SHA1(8ac3d2cd793312434b9ffb8c47c30473f713e0e8) )
	ROM_LOAD( "15.rom", 0x18000, 0x8000, CRC(65a61c99) SHA1(767694c919180de208b6211b593db68fc5a66ff1) )
	ROM_LOAD( "13.rom", 0x20000, 0x8000, CRC(315383d7) SHA1(1c1c5931e3447c4dcbd54fc8ae383b03cb5fbf5b) )
	ROM_LOAD( "16.rom", 0x28000, 0x8000, CRC(e9b03e07) SHA1(124e5328a965ea2af28c4d74934a82394a2ffd72) )
ROM_END

ROM_START( ikaria )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.bin",  0x0000, 0x4000, CRC(ad0e440e) SHA1(a942063e4d12e78c198b091596a82a56a6788e8d) )
	ROM_LOAD( "p2.bin",  0x4000, 0x8000, CRC(b585e931) SHA1(6eaf7592b2c42c5992c9fbece62640ad647f86ef) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "p3",  0x0000, 0x4000, CRC(8a9bd1f0) SHA1(dbf855e328daeddd38c64b7af2d303426d13bf3b) )
	ROM_LOAD( "p4",  0x4000, 0x8000, CRC(f4101cb4) SHA1(cee0eb1cae9f584fb5a866d3a8725f6a3feba912) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ik5",  0x0000, 0x4000, CRC(863448fa) SHA1(19cad05dc9c4495f36e0d8627927ea6d0a971824) )    // p5
	ROM_LOAD( "ik6",  0x4000, 0x8000, CRC(9b16aa57) SHA1(69866ce41c587721702c92ac2e9ba3f6645004cf) )    // p6

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "7122er.prm",  0x000, 0x400, CRC(b9bf2c2c) SHA1(8eb62152dcb04f463baf6ec2a66148eb947403ef) )
	ROM_LOAD( "7122eg.prm",  0x400, 0x400, CRC(0703a770) SHA1(62861ef4987003d4965ef5018ccdf7157981d939) )
	ROM_LOAD( "7122eb.prm",  0x800, 0x400, CRC(0a11cdde) SHA1(faae17398341317e7afbd06b903b8e9e65967bf1) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "7.rom",    0x00000, 0x4000, CRC(a7eb4917) SHA1(6c07323cc243df4c5c30bc0daedbff3887309f65) )   // p7

	ROM_REGION( 0x20000, "bg_tiles", 0 )
	ROM_LOAD( "17.rom", 0x00000, 0x8000, CRC(e0dba976) SHA1(5a8f14f7a199b5fb1862debda0bceee42cddac59) )
	ROM_LOAD( "18.rom", 0x08000, 0x8000, CRC(24947d5f) SHA1(ffd18074ced8171c9da56c839e8289afc29af2c9) )
	ROM_LOAD( "19.rom", 0x10000, 0x8000, CRC(9ee59e91) SHA1(fe51d13ab73cb596a233669e304b2be66f9becae) )
	ROM_LOAD( "20.rom", 0x18000, 0x8000, CRC(5da7ec1a) SHA1(4b212c1dfe4c18eced90ee3a783e7edf8d23c906) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "8.rom",  0x00000, 0x8000, CRC(9827c14a) SHA1(b54dcee95c6f6e46c187a117b4e7aaf1c0ece6c6) ) // p8
	ROM_LOAD( "9.rom",  0x08000, 0x8000, CRC(545c790c) SHA1(7738738f4a1343b04efd029ecaefac74010451f0) ) // p9
	ROM_LOAD( "10.rom", 0x10000, 0x8000, CRC(ec9ba07e) SHA1(6b492b2cd7b8cca948ce39c3450f1cc153f41d90) ) // p10

	ROM_REGION( 0x30000, "sp32_tiles", 0 )
	ROM_LOAD( "11.rom", 0x00000, 0x8000, CRC(5c75ea8f) SHA1(4e8ee56a2dbeb9ac2dd74bc584dba29433d91ae0) )
	ROM_LOAD( "14.rom", 0x08000, 0x8000, CRC(3293fde4) SHA1(3e2f0fa00c22f1c0c1427d8d3de57dd9ec7682a9) )
	ROM_LOAD( "12.rom", 0x10000, 0x8000, CRC(95138498) SHA1(8ac3d2cd793312434b9ffb8c47c30473f713e0e8) )
	ROM_LOAD( "15.rom", 0x18000, 0x8000, CRC(65a61c99) SHA1(767694c919180de208b6211b593db68fc5a66ff1) )
	ROM_LOAD( "13.rom", 0x20000, 0x8000, CRC(315383d7) SHA1(1c1c5931e3447c4dcbd54fc8ae383b03cb5fbf5b) )
	ROM_LOAD( "16.rom", 0x28000, 0x8000, CRC(e9b03e07) SHA1(124e5328a965ea2af28c4d74934a82394a2ffd72) )
ROM_END

ROM_START( ikarinc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1",  0x0000, 0x4000, CRC(738fcec4) SHA1(24a29f9487064b745262638350a332996b986e5d) )
	ROM_LOAD( "p2",  0x4000, 0x8000, CRC(89f7945a) SHA1(39f7f40b2028a77d6e7c79f27c2420b8422b5dab) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "p3",  0x0000, 0x4000, CRC(8a9bd1f0) SHA1(dbf855e328daeddd38c64b7af2d303426d13bf3b) )
	ROM_LOAD( "p4",  0x4000, 0x8000, CRC(f4101cb4) SHA1(cee0eb1cae9f584fb5a866d3a8725f6a3feba912) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ik5",  0x0000, 0x4000, CRC(863448fa) SHA1(19cad05dc9c4495f36e0d8627927ea6d0a971824) )    // p5
	ROM_LOAD( "ik6",  0x4000, 0x8000, CRC(9b16aa57) SHA1(69866ce41c587721702c92ac2e9ba3f6645004cf) )    // p6

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "7122er.prm",  0x000, 0x400, CRC(b9bf2c2c) SHA1(8eb62152dcb04f463baf6ec2a66148eb947403ef) )
	ROM_LOAD( "7122eg.prm",  0x400, 0x400, CRC(0703a770) SHA1(62861ef4987003d4965ef5018ccdf7157981d939) )
	ROM_LOAD( "7122eb.prm",  0x800, 0x400, CRC(0a11cdde) SHA1(faae17398341317e7afbd06b903b8e9e65967bf1) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "7.rom",    0x00000, 0x4000, CRC(a7eb4917) SHA1(6c07323cc243df4c5c30bc0daedbff3887309f65) )   // p7

	ROM_REGION( 0x20000, "bg_tiles", 0 )
	ROM_LOAD( "17.rom", 0x00000, 0x8000, CRC(e0dba976) SHA1(5a8f14f7a199b5fb1862debda0bceee42cddac59) )
	ROM_LOAD( "18.rom", 0x08000, 0x8000, CRC(24947d5f) SHA1(ffd18074ced8171c9da56c839e8289afc29af2c9) )
	ROM_LOAD( "19.rom", 0x10000, 0x8000, CRC(9ee59e91) SHA1(fe51d13ab73cb596a233669e304b2be66f9becae) )
	ROM_LOAD( "20.rom", 0x18000, 0x8000, CRC(5da7ec1a) SHA1(4b212c1dfe4c18eced90ee3a783e7edf8d23c906) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "8.rom",  0x00000, 0x8000, CRC(9827c14a) SHA1(b54dcee95c6f6e46c187a117b4e7aaf1c0ece6c6) ) // p8
	ROM_LOAD( "9.rom",  0x08000, 0x8000, CRC(545c790c) SHA1(7738738f4a1343b04efd029ecaefac74010451f0) ) // p9
	ROM_LOAD( "10.rom", 0x10000, 0x8000, CRC(ec9ba07e) SHA1(6b492b2cd7b8cca948ce39c3450f1cc153f41d90) ) // p10

	ROM_REGION( 0x30000, "sp32_tiles", 0 )
	ROM_LOAD( "11.rom", 0x00000, 0x8000, CRC(5c75ea8f) SHA1(4e8ee56a2dbeb9ac2dd74bc584dba29433d91ae0) )
	ROM_LOAD( "14.rom", 0x08000, 0x8000, CRC(3293fde4) SHA1(3e2f0fa00c22f1c0c1427d8d3de57dd9ec7682a9) )
	ROM_LOAD( "12.rom", 0x10000, 0x8000, CRC(95138498) SHA1(8ac3d2cd793312434b9ffb8c47c30473f713e0e8) )
	ROM_LOAD( "15.rom", 0x18000, 0x8000, CRC(65a61c99) SHA1(767694c919180de208b6211b593db68fc5a66ff1) )
	ROM_LOAD( "13.rom", 0x20000, 0x8000, CRC(315383d7) SHA1(1c1c5931e3447c4dcbd54fc8ae383b03cb5fbf5b) )
	ROM_LOAD( "16.rom", 0x28000, 0x8000, CRC(e9b03e07) SHA1(124e5328a965ea2af28c4d74934a82394a2ffd72) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "ampal16r6a-a5004.1", 0x0000, 0x0104, CRC(a2e9a162) SHA1(35abf667725abea74d36c76552387e7a1debe75a) )
	ROM_LOAD( "pal20l8a-a5004.2",   0x0200, 0x0144, CRC(28f2c404) SHA1(d0832ef9e6be6449018f9b224d5f7203820a5135) )
	ROM_LOAD( "ampal16l8a-a5004.3", 0x0400, 0x0104, CRC(540351f2) SHA1(d9c3aebb839935c8b49678693b87cc2bca2a674d) )
	ROM_LOAD( "ampal16l8a-a5004.4", 0x0600, 0x0104, CRC(540351f2) SHA1(d9c3aebb839935c8b49678693b87cc2bca2a674d) )
ROM_END

ROM_START( ikarijp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "up03_l4.rom",  0x0000, 0x4000, CRC(cde006be) SHA1(a42e23659cf0ea5194f8a7a9a1679ebcaed75ead) )
	ROM_LOAD( "up03_k4.rom",  0x4000, 0x8000, CRC(26948850) SHA1(bfeba5f7019f6eaacf2a5464756d9cb283c5f5a2) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "ik3",  0x0000, 0x4000, CRC(9bb385f8) SHA1(70cc30bece54c28205017e755dc32a1c088f9f80) )
	ROM_LOAD( "ik4",  0x4000, 0x8000, CRC(3a144bca) SHA1(c1b09bffb8d89e607332304b1d8845794f25273f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ik5",  0x0000, 0x4000, CRC(863448fa) SHA1(19cad05dc9c4495f36e0d8627927ea6d0a971824) )
	ROM_LOAD( "ik6",  0x4000, 0x8000, CRC(9b16aa57) SHA1(69866ce41c587721702c92ac2e9ba3f6645004cf) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "7122er.prm",  0x000, 0x400, CRC(b9bf2c2c) SHA1(8eb62152dcb04f463baf6ec2a66148eb947403ef) )
	ROM_LOAD( "7122eg.prm",  0x400, 0x400, CRC(0703a770) SHA1(62861ef4987003d4965ef5018ccdf7157981d939) )
	ROM_LOAD( "7122eb.prm",  0x800, 0x400, CRC(0a11cdde) SHA1(faae17398341317e7afbd06b903b8e9e65967bf1) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "ik7",    0x00000, 0x4000, CRC(9e88f536) SHA1(80e9aadeb626e60318a2139fd1b3875f6256c492) )

	ROM_REGION( 0x20000, "bg_tiles", 0 )
	ROM_LOAD( "17.rom", 0x00000, 0x8000, CRC(e0dba976) SHA1(5a8f14f7a199b5fb1862debda0bceee42cddac59) )
	ROM_LOAD( "18.rom", 0x08000, 0x8000, CRC(24947d5f) SHA1(ffd18074ced8171c9da56c839e8289afc29af2c9) )
	ROM_LOAD( "ik19",   0x10000, 0x8000, CRC(566242ec) SHA1(ca25587460491597d462d2526d59afbc9b92fb75) )
	ROM_LOAD( "20.rom", 0x18000, 0x8000, CRC(5da7ec1a) SHA1(4b212c1dfe4c18eced90ee3a783e7edf8d23c906) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "ik8",  0x00000, 0x8000, CRC(75d796d0) SHA1(395c1d22b935c92c50a326edc8b6cd9aab235f7c) )
	ROM_LOAD( "ik9",  0x08000, 0x8000, CRC(2c34903b) SHA1(1949fc0cef4b30665ad288fa8e506a05741face0) )
	ROM_LOAD( "ik10", 0x10000, 0x8000, CRC(da9ccc94) SHA1(be3c9d44a887ac823039153b832dfae18fe69965) )

	ROM_REGION( 0x30000, "sp32_tiles", 0 )
	ROM_LOAD( "11.rom", 0x00000, 0x8000, CRC(5c75ea8f) SHA1(4e8ee56a2dbeb9ac2dd74bc584dba29433d91ae0) )
	ROM_LOAD( "14.rom", 0x08000, 0x8000, CRC(3293fde4) SHA1(3e2f0fa00c22f1c0c1427d8d3de57dd9ec7682a9) )
	ROM_LOAD( "12.rom", 0x10000, 0x8000, CRC(95138498) SHA1(8ac3d2cd793312434b9ffb8c47c30473f713e0e8) )
	ROM_LOAD( "15.rom", 0x18000, 0x8000, CRC(65a61c99) SHA1(767694c919180de208b6211b593db68fc5a66ff1) )
	ROM_LOAD( "13.rom", 0x20000, 0x8000, CRC(315383d7) SHA1(1c1c5931e3447c4dcbd54fc8ae383b03cb5fbf5b) )
	ROM_LOAD( "16.rom", 0x28000, 0x8000, CRC(e9b03e07) SHA1(124e5328a965ea2af28c4d74934a82394a2ffd72) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "ampal16r6a-a5004.1", 0x0000, 0x0104, CRC(a2e9a162) SHA1(35abf667725abea74d36c76552387e7a1debe75a) )
	ROM_LOAD( "pal20l8a-a5004.2",   0x0200, 0x0144, CRC(28f2c404) SHA1(d0832ef9e6be6449018f9b224d5f7203820a5135) )
	ROM_LOAD( "ampal16l8a-a5004.3", 0x0400, 0x0104, CRC(540351f2) SHA1(d9c3aebb839935c8b49678693b87cc2bca2a674d) )
	ROM_LOAD( "ampal16l8a-a5004.4", 0x0600, 0x0104, CRC(540351f2) SHA1(d9c3aebb839935c8b49678693b87cc2bca2a674d) )
ROM_END

ROM_START( ikarijpb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ik1",      0x00000, 0x4000, CRC(2ef87dce) SHA1(4b52567fee81018f7a4b33bac79ea521c7d19d52) )
	ROM_LOAD( "up03_k4.rom",  0x04000, 0x8000, CRC(26948850) SHA1(bfeba5f7019f6eaacf2a5464756d9cb283c5f5a2) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "ik3",    0x0000, 0x4000, CRC(9bb385f8) SHA1(70cc30bece54c28205017e755dc32a1c088f9f80) )
	ROM_LOAD( "ik4",    0x4000, 0x8000, CRC(3a144bca) SHA1(c1b09bffb8d89e607332304b1d8845794f25273f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ik5",    0x0000, 0x4000, CRC(863448fa) SHA1(19cad05dc9c4495f36e0d8627927ea6d0a971824) )
	ROM_LOAD( "ik6",    0x4000, 0x8000, CRC(9b16aa57) SHA1(69866ce41c587721702c92ac2e9ba3f6645004cf) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "7122er.prm", 0x000, 0x400, CRC(b9bf2c2c) SHA1(8eb62152dcb04f463baf6ec2a66148eb947403ef) )
	ROM_LOAD( "7122eg.prm", 0x400, 0x400, CRC(0703a770) SHA1(62861ef4987003d4965ef5018ccdf7157981d939) )
	ROM_LOAD( "7122eb.prm", 0x800, 0x400, CRC(0a11cdde) SHA1(faae17398341317e7afbd06b903b8e9e65967bf1) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "ik7", 0x0000, 0x4000, CRC(9e88f536) SHA1(80e9aadeb626e60318a2139fd1b3875f6256c492) )

	ROM_REGION( 0x20000, "bg_tiles", 0 )
	ROM_LOAD( "17.rom", 0x00000, 0x8000, CRC(e0dba976) SHA1(5a8f14f7a199b5fb1862debda0bceee42cddac59) )
	ROM_LOAD( "18.rom", 0x08000, 0x8000, CRC(24947d5f) SHA1(ffd18074ced8171c9da56c839e8289afc29af2c9) )
	ROM_LOAD( "ik19",   0x10000, 0x8000, CRC(566242ec) SHA1(ca25587460491597d462d2526d59afbc9b92fb75) )
	ROM_LOAD( "20.rom", 0x18000, 0x8000, CRC(5da7ec1a) SHA1(4b212c1dfe4c18eced90ee3a783e7edf8d23c906) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "ik8",    0x00000, 0x8000, CRC(75d796d0) SHA1(395c1d22b935c92c50a326edc8b6cd9aab235f7c) )
	ROM_LOAD( "ik9",    0x08000, 0x8000, CRC(2c34903b) SHA1(1949fc0cef4b30665ad288fa8e506a05741face0) )
	ROM_LOAD( "ik10",   0x10000, 0x8000, CRC(da9ccc94) SHA1(be3c9d44a887ac823039153b832dfae18fe69965) )

	ROM_REGION( 0x30000, "sp32_tiles", 0 )
	ROM_LOAD( "11.rom", 0x00000, 0x8000, CRC(5c75ea8f) SHA1(4e8ee56a2dbeb9ac2dd74bc584dba29433d91ae0) )
	ROM_LOAD( "14.rom", 0x08000, 0x8000, CRC(3293fde4) SHA1(3e2f0fa00c22f1c0c1427d8d3de57dd9ec7682a9) )
	ROM_LOAD( "12.rom", 0x10000, 0x8000, CRC(95138498) SHA1(8ac3d2cd793312434b9ffb8c47c30473f713e0e8) )
	ROM_LOAD( "15.rom", 0x18000, 0x8000, CRC(65a61c99) SHA1(767694c919180de208b6211b593db68fc5a66ff1) )
	ROM_LOAD( "13.rom", 0x20000, 0x8000, CRC(315383d7) SHA1(1c1c5931e3447c4dcbd54fc8ae383b03cb5fbf5b) )
	ROM_LOAD( "16.rom", 0x28000, 0x8000, CRC(e9b03e07) SHA1(124e5328a965ea2af28c4d74934a82394a2ffd72) )
ROM_END

/***********************************************************************/

ROM_START( victroad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.p4", 0x0000, 0x10000,  CRC(e334acef) SHA1(f6d8da554276abbe5579c92eea46591a92623f6e) )

	ROM_REGION(  0x10000 , "sub", 0 )
	ROM_LOAD( "p2.p8", 0x00000, 0x10000, CRC(907fac83) SHA1(691d95f95ef7a308c7f5e7defb20971b54423745) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p3.k7", 0x00000, 0x10000, CRC(bac745f6) SHA1(c118d94aff16cbf1b85615ff5a93292f6e98c149) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "mb7122e.1k", 0x000, 0x400, CRC(491ab831) SHA1(2801d68d8a7fddaca5c48f09d421fc48ff53d244) )
	ROM_LOAD( "mb7122e.2l", 0x400, 0x400, CRC(8feca424) SHA1(c3d666f4b4b914199b24ded02f9a1b643bf90d26) )
	ROM_LOAD( "mb7122e.1l", 0x800, 0x400, CRC(220076ca) SHA1(a353c770c0ffb1105fb93c97977597ad2fda8ac8) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "p7.b3", 0x0000, 0x4000,  CRC(2b6ed95b) SHA1(dddf3aa21776778153572a20d29d47928a7116d8) )

	ROM_REGION( 0x20000, "bg_tiles", 0 )
	ROM_LOAD( "p17.cd4", 0x00000, 0x8000, CRC(19d4518c) SHA1(133ac6e3d75af6cfc9aa9d1d467f16696c7f3794) )
	ROM_LOAD( "p18.cd2", 0x08000, 0x8000, CRC(d818be43) SHA1(274827b13e8572f68302b7e0b5964d3e32544303) )
	ROM_LOAD( "p19.b4",  0x10000, 0x8000, CRC(d64e0f89) SHA1(41204d5b0bc9d2f2599c3e881f10b73bddae3c5c) )
	ROM_LOAD( "p20.b2",  0x18000, 0x8000, CRC(edba0f31) SHA1(b3fc886d3cf7a34b470dd72cc0268a193f9a64d7) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "p8.d3",   0x00000, 0x8000, CRC(df7f252a) SHA1(435aade99144c9be51f65d76583256aa089cce78) )
	ROM_LOAD( "p9.ef3",  0x08000, 0x8000, CRC(9897bc05) SHA1(ec181dc64dd78ff2fab193509743376ab192b99e) )
	ROM_LOAD( "p10.gh3", 0x10000, 0x8000, CRC(ecd3c0ea) SHA1(f398b6a64706fcaa727ff1c150e05888091cb77c) )

	ROM_REGION( 0x30000, "sp32_tiles", 0 )
	ROM_LOAD( "p11.m4",  0x00000, 0x8000, CRC(668b25a4) SHA1(235423e3b442271581dde0195fdff2a37596a9bc) )
	ROM_LOAD( "p14.m2",  0x08000, 0x8000, CRC(a7031d4a) SHA1(9ea184990372909de7d8fe0891bb3e0441b13f90) )
	ROM_LOAD( "p12.np4", 0x10000, 0x8000, CRC(f44e95fa) SHA1(6633bd1e9e947cae5ba696f6fd393bf0cd7969b0) )
	ROM_LOAD( "p15.np2", 0x18000, 0x8000, CRC(120d2450) SHA1(8699db76f598e7719fa5f9a3dcc07d24c53e5da4) )
	ROM_LOAD( "p13.r4",  0x20000, 0x8000, CRC(980ca3d8) SHA1(bda6f19edf43c61c0c8d2235bb60def76c801b87) )
	ROM_LOAD( "p16.r2",  0x28000, 0x8000, CRC(9f820e8a) SHA1(2be0128d6861241f6a9c5a7032368dbc6d57b44e) )

	ROM_REGION( 0x20000, "ym2", 0 )
	ROM_LOAD( "p4.ef5", 0x00000, 0x10000, CRC(e10fb8cc) SHA1(591aa1f947216795252dc4d9ec2600ef63dada7d) )
	ROM_LOAD( "p5.g5",  0x10000, 0x10000, CRC(93e5f110) SHA1(065a78805e50ce6a48cb7930f264bada236feb13) )
ROM_END

ROM_START( dogosoke )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "up03_p4.rom",  0x0000, 0x10000,  CRC(37867ad2) SHA1(4444e428eb7126451f34351b1a2bc193484ca641) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "p2",  0x00000, 0x10000, CRC(907fac83) SHA1(691d95f95ef7a308c7f5e7defb20971b54423745) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "up03_k7.rom",  0x00000, 0x10000, CRC(173fa571) SHA1(fb9c783e5377fa86f70afee6804c8ee9061b27fd) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "up03_k1.rom",  0x000, 0x400, CRC(10a2ce2b) SHA1(8de93250b81fbabb012c96454ef3a888b2783ab5) )
	ROM_LOAD( "up03_l2.rom",  0x400, 0x400, CRC(99dc9792) SHA1(dcdcea2bad524776e17eaeb70dd4882283f1b125) )
	ROM_LOAD( "up03_l1.rom",  0x800, 0x400, CRC(e7213160) SHA1(bc762a346e1639c8a9636fe85c18d68a08c1b586) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "up02_b3.rom",  0x0000, 0x4000,  CRC(51a4ec83) SHA1(8cb743c68a51b71ef3d78127b2cf6ab0877b13f6) )

	ROM_REGION( 0x20000, "bg_tiles", 0 )
	ROM_LOAD( "p17",  0x00000, 0x8000, CRC(19d4518c) SHA1(133ac6e3d75af6cfc9aa9d1d467f16696c7f3794) )
	ROM_LOAD( "p18",  0x08000, 0x8000, CRC(d818be43) SHA1(274827b13e8572f68302b7e0b5964d3e32544303) )
	ROM_LOAD( "p19",  0x10000, 0x8000, CRC(d64e0f89) SHA1(41204d5b0bc9d2f2599c3e881f10b73bddae3c5c) )
	ROM_LOAD( "p20",  0x18000, 0x8000, CRC(edba0f31) SHA1(b3fc886d3cf7a34b470dd72cc0268a193f9a64d7) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "up02_d3.rom",  0x00000, 0x8000, CRC(d43044f8) SHA1(4d5bc3730ea1bb1978ae246745416b71979cb100) )
	ROM_LOAD( "up02_e3.rom",  0x08000, 0x8000, CRC(365ed2d8) SHA1(e0f600c936483e3d0d03709ae709321d072145bd) )
	ROM_LOAD( "up02_g3.rom",  0x10000, 0x8000, CRC(92579bf3) SHA1(eb2084bf5c62cbbf08dc25997702f8e8eb3dcc5d) )

	ROM_REGION( 0x30000, "sp32_tiles", 0 )
	ROM_LOAD( "p11", 0x00000, 0x8000, CRC(668b25a4) SHA1(235423e3b442271581dde0195fdff2a37596a9bc) )
	ROM_LOAD( "p14", 0x08000, 0x8000, CRC(a7031d4a) SHA1(9ea184990372909de7d8fe0891bb3e0441b13f90) )
	ROM_LOAD( "p12", 0x10000, 0x8000, CRC(f44e95fa) SHA1(6633bd1e9e947cae5ba696f6fd393bf0cd7969b0) )
	ROM_LOAD( "p15", 0x18000, 0x8000, CRC(120d2450) SHA1(8699db76f598e7719fa5f9a3dcc07d24c53e5da4) )
	ROM_LOAD( "p13", 0x20000, 0x8000, CRC(980ca3d8) SHA1(bda6f19edf43c61c0c8d2235bb60def76c801b87) )
	ROM_LOAD( "p16", 0x28000, 0x8000, CRC(9f820e8a) SHA1(2be0128d6861241f6a9c5a7032368dbc6d57b44e) )

	ROM_REGION( 0x20000, "ym2", 0 )
	ROM_LOAD( "up03_f5.rom", 0x00000, 0x10000, CRC(5b43fe9f) SHA1(28f803f633b83b17f9b10516d38c862f90d55ff3) )
	ROM_LOAD( "up03_g5.rom", 0x10000, 0x10000, CRC(aae30cd6) SHA1(9d0d2c0f947387a0924bf0ed73de9305c1625054) )
ROM_END

ROM_START( dogosokb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "01",  0x00000, 0x10000, CRC(53b0ad90) SHA1(7581365d6c82b35189852d96437b0f19abe2cf74) )

	ROM_REGION(  0x10000 , "sub", 0 )
	ROM_LOAD( "p2",  0x00000, 0x10000, CRC(907fac83) SHA1(691d95f95ef7a308c7f5e7defb20971b54423745) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "up03_k7.rom",  0x00000, 0x10000, CRC(173fa571) SHA1(fb9c783e5377fa86f70afee6804c8ee9061b27fd) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "up03_k1.rom",  0x000, 0x400, CRC(10a2ce2b) SHA1(8de93250b81fbabb012c96454ef3a888b2783ab5) )
	ROM_LOAD( "up03_l2.rom",  0x400, 0x400, CRC(99dc9792) SHA1(dcdcea2bad524776e17eaeb70dd4882283f1b125) )
	ROM_LOAD( "up03_l1.rom",  0x800, 0x400, CRC(e7213160) SHA1(bc762a346e1639c8a9636fe85c18d68a08c1b586) )

	ROM_REGION( 0x4000, "tx_tiles", 0 )
	ROM_LOAD( "up02_b3.rom",  0x0000, 0x4000,  CRC(51a4ec83) SHA1(8cb743c68a51b71ef3d78127b2cf6ab0877b13f6) )

	ROM_REGION( 0x20000, "bg_tiles", 0 )
	ROM_LOAD( "p17",  0x00000, 0x8000, CRC(19d4518c) SHA1(133ac6e3d75af6cfc9aa9d1d467f16696c7f3794) )
	ROM_LOAD( "p18",  0x08000, 0x8000, CRC(d818be43) SHA1(274827b13e8572f68302b7e0b5964d3e32544303) )
	ROM_LOAD( "p19",  0x10000, 0x8000, CRC(d64e0f89) SHA1(41204d5b0bc9d2f2599c3e881f10b73bddae3c5c) )
	ROM_LOAD( "p20",  0x18000, 0x8000, CRC(edba0f31) SHA1(b3fc886d3cf7a34b470dd72cc0268a193f9a64d7) )

	ROM_REGION( 0x18000, "sp16_tiles", 0 )
	ROM_LOAD( "up02_d3.rom",  0x00000, 0x8000, CRC(d43044f8) SHA1(4d5bc3730ea1bb1978ae246745416b71979cb100) )
	ROM_LOAD( "up02_e3.rom",  0x08000, 0x8000, CRC(365ed2d8) SHA1(e0f600c936483e3d0d03709ae709321d072145bd) )
	ROM_LOAD( "up02_g3.rom",  0x10000, 0x8000, CRC(92579bf3) SHA1(eb2084bf5c62cbbf08dc25997702f8e8eb3dcc5d) )

	ROM_REGION( 0x30000, "sp32_tiles", 0 )
	ROM_LOAD( "p11", 0x00000, 0x8000, CRC(668b25a4) SHA1(235423e3b442271581dde0195fdff2a37596a9bc) )
	ROM_LOAD( "p14", 0x08000, 0x8000, CRC(a7031d4a) SHA1(9ea184990372909de7d8fe0891bb3e0441b13f90) )
	ROM_LOAD( "p12", 0x10000, 0x8000, CRC(f44e95fa) SHA1(6633bd1e9e947cae5ba696f6fd393bf0cd7969b0) )
	ROM_LOAD( "p15", 0x18000, 0x8000, CRC(120d2450) SHA1(8699db76f598e7719fa5f9a3dcc07d24c53e5da4) )
	ROM_LOAD( "p13", 0x20000, 0x8000, CRC(980ca3d8) SHA1(bda6f19edf43c61c0c8d2235bb60def76c801b87) )
	ROM_LOAD( "p16", 0x28000, 0x8000, CRC(9f820e8a) SHA1(2be0128d6861241f6a9c5a7032368dbc6d57b44e) )

	ROM_REGION( 0x20000, "ym2", 0 )
	ROM_LOAD( "up03_f5.rom", 0x00000, 0x10000, CRC(5b43fe9f) SHA1(28f803f633b83b17f9b10516d38c862f90d55ff3) )
	ROM_LOAD( "up03_g5.rom", 0x10000, 0x10000, CRC(aae30cd6) SHA1(9d0d2c0f947387a0924bf0ed73de9305c1625054) )
ROM_END

/***********************************************************************/

/*
pals
----
Fuse Plot - a6003-3
?

PAL16L8/A/A-2/A-4
*
DD PAL16L8/A/A-2/A-4*
QP20*
QF2048*
G0*
F0*
L0     11111111111111111111111111111111*
L32    11111110111111111111111111111111*
L64    11111111111111101111111111111011*
L96    11111111110111101111111111111111*
L128   00000000000000000000000000000000*
L160   00000000000000000000000000000000*
L192   00000000000000000000000000000000*
L224   00000000000000000000000000000000*
L256   11111111111111111111111111111111*
L288   11101111111111111111111111111111*
L320   10111111111111111111111111111111*
L352   11111011111111111111111111111111*
L384   11111111101111111111111111111111*
L416   00000000000000000000000000000000*
L448   00000000000000000000000000000000*
L480   00000000000000000000000000000000*
L512   11111111111111111111111111111111*
L544   11111111111111111111111111111110*
L576   11111111111111111111111111101111*
L608   11111111111111111111111011111111*
L640   11111111111111111110111111111111*
L672   00000000000000000000000000000000*
L704   00000000000000000000000000000000*
L736   00000000000000000000000000000000*
L768   11111111111111111111111111111111*
L800   11111111111110111111111111111111*
L832   11111111111111111011111111111111*
L864   11111111111111111111101111111111*
L896   11111111111111111111111110111111*
L928   00000000000000000000000000000000*
L960   00000000000000000000000000000000*
L992   00000000000000000000000000000000*
L1024  00000000000000000000000000000000*
L1056  00000000000000000000000000000000*
L1088  00000000000000000000000000000000*
L1120  00000000000000000000000000000000*
L1152  00000000000000000000000000000000*
L1184  00000000000000000000000000000000*
L1216  00000000000000000000000000000000*
L1248  00000000000000000000000000000000*
L1280  00000000000000000000000000000000*
L1312  00000000000000000000000000000000*
L1344  00000000000000000000000000000000*
L1376  00000000000000000000000000000000*
L1408  00000000000000000000000000000000*
L1440  00000000000000000000000000000000*
L1472  00000000000000000000000000000000*
L1504  00000000000000000000000000000000*
L1536  00000000000000000000000000000000*
L1568  00000000000000000000000000000000*
L1600  00000000000000000000000000000000*
L1632  00000000000000000000000000000000*
L1664  00000000000000000000000000000000*
L1696  00000000000000000000000000000000*
L1728  00000000000000000000000000000000*
L1760  00000000000000000000000000000000*
L1792  11111111111111111111111111111111*
L1824  11111110111111111111111111111111*
L1856  11111111111011111111111111110111*
L1888  11111111111001110111011101111111*
L1920  00000000000000000000000000000000*
L1952  00000000000000000000000000000000*
L1984  00000000000000000000000000000000*
L2016  00000000000000000000000000000000*
C57BE*
?


Fuse Plot - a6004-1
?

PAL16R6B-2/B-4
*
DD PAL16R6B-2/B-4*
QP20*
QF2048*
G0*
F0*
L0     11111111111111111111111111111111*
L32    11111111111111111111011111111111*
L64    11111110111111111111111111111111*
L96    00000000000000000000000000000000*
L128   00000000000000000000000000000000*
L160   00000000000000000000000000000000*
L192   00000000000000000000000000000000*
L224   00000000000000000000000000000000*
L256   01111111011111111111111111111111*
L288   00000000000000000000000000000000*
L320   00000000000000000000000000000000*
L352   00000000000000000000000000000000*
L384   00000000000000000000000000000000*
L416   00000000000000000000000000000000*
L448   00000000000000000000000000000000*
L480   00000000000000000000000000000000*
L512   11111111101111111111111111111111*
L544   00000000000000000000000000000000*
L576   00000000000000000000000000000000*
L608   00000000000000000000000000000000*
L640   00000000000000000000000000000000*
L672   00000000000000000000000000000000*
L704   00000000000000000000000000000000*
L736   00000000000000000000000000000000*
L768   11111110111101110111101111111111*
L800   00000000000000000000000000000000*
L832   00000000000000000000000000000000*
L864   00000000000000000000000000000000*
L896   00000000000000000000000000000000*
L928   00000000000000000000000000000000*
L960   00000000000000000000000000000000*
L992   00000000000000000000000000000000*
L1024  11111101111111110111101111111111*
L1056  00000000000000000000000000000000*
L1088  00000000000000000000000000000000*
L1120  00000000000000000000000000000000*
L1152  00000000000000000000000000000000*
L1184  00000000000000000000000000000000*
L1216  00000000000000000000000000000000*
L1248  00000000000000000000000000000000*
L1280  11111101111111111111101111111111*
L1312  00000000000000000000000000000000*
L1344  00000000000000000000000000000000*
L1376  00000000000000000000000000000000*
L1408  00000000000000000000000000000000*
L1440  00000000000000000000000000000000*
L1472  00000000000000000000000000000000*
L1504  00000000000000000000000000000000*
L1536  00000000000000000000000000000000*
L1568  00000000000000000000000000000000*
L1600  00000000000000000000000000000000*
L1632  00000000000000000000000000000000*
L1664  00000000000000000000000000000000*
L1696  00000000000000000000000000000000*
L1728  00000000000000000000000000000000*
L1760  00000000000000000000000000000000*
L1792  11111111111111111111111111111111*
L1824  11111101111111111111111110111111*
L1856  01110111011111111111111111111111*
L1888  01111111011111111111111110111111*
L1920  01110101011111111111111111111111*
L1952  00000000000000000000000000000000*
L1984  00000000000000000000000000000000*
L2016  00000000000000000000000000000000*
C311C*
?


Fuse Plot - a6004-2
?

PAL20L8A/A-2
*
DD PAL20L8A/A-2*
QP24*
QF2560*
G0*
F0*
L0     0000000000000000000000000000000000000000*
L40    0000000000000000000000000000000000000000*
L80    0000000000000000000000000000000000000000*
L120   0000000000000000000000000000000000000000*
L160   0000000000000000000000000000000000000000*
L200   0000000000000000000000000000000000000000*
L240   0000000000000000000000000000000000000000*
L280   0000000000000000000000000000000000000000*
L320   1111111111111111111111111111111111111111*
L360   1010011001111011111111111111111111111111*
L400   1010011010110111111111111111111111111111*
L440   1111110111111111101110110111011110111111*
L480   1111110111111111101110110111101101111111*
L520   0000000000000000000000000000000000000000*
L560   0000000000000000000000000000000000000000*
L600   0000000000000000000000000000000000000000*
L640   1111111111111111111111111111111111111111*
L680   1010011001110111111111111111111111111111*
L720   1111110111111111101110110111011101111111*
L760   0000000000000000000000000000000000000000*
L800   0000000000000000000000000000000000000000*
L840   0000000000000000000000000000000000000000*
L880   0000000000000000000000000000000000000000*
L920   0000000000000000000000000000000000000000*
L960   1111111111111111111111111111111111111111*
L1000  1010101010110111111111111111111111111111*
L1040  1111110111111111101110111011101101111111*
L1080  0000000000000000000000000000000000000000*
L1120  0000000000000000000000000000000000000000*
L1160  0000000000000000000000000000000000000000*
L1200  0000000000000000000000000000000000000000*
L1240  0000000000000000000000000000000000000000*
L1280  1111111111111111111111111111111111111111*
L1320  1111111011111111111111111111111111111011*
L1360  1111110111111111111111111111111111111110*
L1400  0000000000000000000000000000000000000000*
L1440  0000000000000000000000000000000000000000*
L1480  0000000000000000000000000000000000000000*
L1520  0000000000000000000000000000000000000000*
L1560  0000000000000000000000000000000000000000*
L1600  1111111111111111111111111111111111111111*
L1640  1010101001111011111111111111111111111111*
L1680  1010101001110111111111111111111111111111*
L1720  1111110111111111101110111011011110111111*
L1760  1111110111111111101110111011011101111111*
L1800  0000000000000000000000000000000000000000*
L1840  0000000000000000000000000000000000000000*
L1880  0000000000000000000000000000000000000000*
L1920  1111111111111111111111111111111111111111*
L1960  1010011010111011111111111111111111111111*
L2000  1111110111111111101110110111101110111111*
L2040  0000000000000000000000000000000000000000*
L2080  0000000000000000000000000000000000000000*
L2120  0000000000000000000000000000000000000000*
L2160  0000000000000000000000000000000000000000*
L2200  0000000000000000000000000000000000000000*
L2240  0000000000000000000000000000000000000000*
L2280  0000000000000000000000000000000000000000*
L2320  0000000000000000000000000000000000000000*
L2360  0000000000000000000000000000000000000000*
L2400  0000000000000000000000000000000000000000*
L2440  0000000000000000000000000000000000000000*
L2480  0000000000000000000000000000000000000000*
L2520  0000000000000000000000000000000000000000*
C63C0*
?
*/
/***********************************************************************/

ROM_START( bermudat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bt_p1.rom",  0x0000, 0x10000,  CRC(43dec5e9) SHA1(2b29016d4af2a0a6be87f440f235a6a76f8a52a0) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "bt_p2.rom",  0x00000, 0x10000, CRC(0e193265) SHA1(765ad63d1f752920d3d7829747e8f2808670ee84) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bt_p3.rom",  0x00000, 0x10000, CRC(53a82e50) SHA1(ce1e72f0ddc5e19c2d8a6a545ce205c7c39da2dd) )

	ROM_REGION( 0x1400, "proms", 0 )
	ROM_LOAD( "btj_01r.prm", 0x0000, 0x0400, CRC(f4b54d06) SHA1(620ea513dbf3219844cdb36ea5d7e2a1b13e3198) ) /* red */
	ROM_LOAD( "btj_02g.prm", 0x0400, 0x0400, CRC(baac139e) SHA1(c951c9a2d8bb1af178de63c6e2cb716dcb2ac57c) ) /* green */
	ROM_LOAD( "btj_03b.prm", 0x0800, 0x0400, CRC(2edf2e0b) SHA1(b430ec934399909e6e1f27c7bf47bbacf01f266f) ) /* blue */
	ROM_LOAD( "btj_h.prm",   0x0c00, 0x0400, CRC(c20b197b) SHA1(504cb28d652029fe87a5411d6239e78d93c83e91) ) /* ? */
	ROM_LOAD( "btj_v.prm",   0x1000, 0x0400, CRC(5d0c617f) SHA1(845e52173c33500227cabe1e21b34919d2856215) ) /* ? */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "bt_p10.rom", 0x0000, 0x8000,  CRC(d3650211) SHA1(cc7cfe05c5903caf33f8f02c416f68e6d2f6baa7) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "bt_p22.rom", 0x00000, 0x10000, CRC(8daf7df4) SHA1(c6b5157821f3751bc70411ba0e1ea43d223ad0f6) )
	ROM_LOAD( "bt_p21.rom", 0x10000, 0x10000, CRC(b7689599) SHA1(ffa35b480efbc55948e5d0202e7a7ab6446db905) )
	ROM_LOAD( "bt_p20.rom", 0x20000, 0x10000, CRC(ab6217b7) SHA1(fb4b0fcd9ff1f04cf772a46b6727d3de531beb0e) )
	ROM_LOAD( "bt_p19.rom", 0x30000, 0x10000, CRC(8ed759a0) SHA1(cd039ed9cb4127729bd29c6232dcbb77b85a4159) )

	ROM_REGION( 0x20000, "sp16_tiles", 0 )
	ROM_LOAD( "bt_p6.rom",  0x00000, 0x8000, CRC(8ffdf969) SHA1(68672dc74156ebbf59316dfeae25b155d699d0eb) )
	ROM_LOAD( "bt_p7.rom",  0x08000, 0x8000, CRC(268d10df) SHA1(6a297bbd7b4248306d8756a80f4403c45d833eb3) )
	ROM_LOAD( "bt_p8.rom",  0x10000, 0x8000, CRC(3e39e9dd) SHA1(394c85841113a1b2bdd744445e3e4e3acc7099c6) )
	ROM_LOAD( "bt_p9.rom",  0x18000, 0x8000, CRC(bf56da61) SHA1(855687b6a0a4cef3b8294ca359abe14b11ad5749) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "bt_p11.rom", 0x00000, 0x10000, CRC(aae7410e) SHA1(19dcd13fc53c05bac05d2242965129ab1e3a4a88) )
	ROM_LOAD( "bt_p12.rom", 0x10000, 0x10000, CRC(18914f70) SHA1(2c4e7db8b6e70dffb27d10032f750932c7379a66) )
	ROM_LOAD( "bt_p13.rom", 0x20000, 0x10000, CRC(cd79ce81) SHA1(00f205f8a97f839e2592bdfb624fe6b902ce5a93) )
	ROM_LOAD( "bt_p14.rom", 0x30000, 0x10000, CRC(edc57117) SHA1(899a524973f407c3be1de9dac50f3d373bccb2e5) )
	ROM_LOAD( "bt_p15.rom", 0x40000, 0x10000, CRC(448bf9f4) SHA1(0f880ba3e97a57c937afdce29a1461bc310196eb) )
	ROM_LOAD( "bt_p16.rom", 0x50000, 0x10000, CRC(119999eb) SHA1(0030121239c3ef07c093a7e2146c4027e1b544ac) )
	ROM_LOAD( "bt_p17.rom", 0x60000, 0x10000, CRC(b5462139) SHA1(9af190cf5fabcc017d707be43bd141dc6db12827) )
	ROM_LOAD( "bt_p18.rom", 0x70000, 0x10000, CRC(cb416227) SHA1(aba0b5a0c93713c676a59e8d3c36d780a4e01894) )

	ROM_REGION( 0x20000, "ym2", 0 )
	ROM_LOAD( "bt_p4.rom",  0x00000, 0x10000, CRC(4bc83229) SHA1(b58d08ebed0b02279385a7ac2f385e62443e3de6) )
	ROM_LOAD( "bt_p5.rom",  0x10000, 0x10000, CRC(817bd62c) SHA1(d3ee2ff01a4da8b928728b2fd4948fabd2b04420) )
ROM_END

ROM_START( bermudatj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "btj_p01.bin", 0x0000, 0x10000,  CRC(eda75f36) SHA1(d6fcb46dc45007a77bf6a8ca7aa53aefedcecf92) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "bt_p2.rom",   0x00000, 0x10000, CRC(0e193265) SHA1(765ad63d1f752920d3d7829747e8f2808670ee84) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "btj_p03.bin", 0x00000, 0x10000, CRC(fea8a096) SHA1(593e34a20ab6f5bae9d74415af5a834646d2444e) )

	ROM_REGION( 0x1400, "proms", 0 )
	ROM_LOAD( "btj_01r.prm", 0x0000, 0x0400, CRC(f4b54d06) SHA1(620ea513dbf3219844cdb36ea5d7e2a1b13e3198) ) /* red */
	ROM_LOAD( "btj_02g.prm", 0x0400, 0x0400, CRC(baac139e) SHA1(c951c9a2d8bb1af178de63c6e2cb716dcb2ac57c) ) /* green */
	ROM_LOAD( "btj_03b.prm", 0x0800, 0x0400, CRC(2edf2e0b) SHA1(b430ec934399909e6e1f27c7bf47bbacf01f266f) ) /* blue */
	ROM_LOAD( "btj_h.prm",   0x0c00, 0x0400, CRC(c20b197b) SHA1(504cb28d652029fe87a5411d6239e78d93c83e91) ) /* ? */
	ROM_LOAD( "btj_v.prm",   0x1000, 0x0400, CRC(5d0c617f) SHA1(845e52173c33500227cabe1e21b34919d2856215) ) /* ? */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "bt_p10.rom",  0x0000, 0x8000,  CRC(d3650211) SHA1(cc7cfe05c5903caf33f8f02c416f68e6d2f6baa7) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "bt_p22.rom",  0x00000, 0x10000, CRC(8daf7df4) SHA1(c6b5157821f3751bc70411ba0e1ea43d223ad0f6) )
	ROM_LOAD( "bt_p21.rom",  0x10000, 0x10000, CRC(b7689599) SHA1(ffa35b480efbc55948e5d0202e7a7ab6446db905) )
	ROM_LOAD( "bt_p20.rom",  0x20000, 0x10000, CRC(ab6217b7) SHA1(fb4b0fcd9ff1f04cf772a46b6727d3de531beb0e) )
	ROM_LOAD( "bt_p19.rom",  0x30000, 0x10000, CRC(8ed759a0) SHA1(cd039ed9cb4127729bd29c6232dcbb77b85a4159) )

	ROM_REGION( 0x20000, "sp16_tiles", 0 )
	ROM_LOAD( "bt_p6.rom",   0x00000, 0x8000, CRC(8ffdf969) SHA1(68672dc74156ebbf59316dfeae25b155d699d0eb) )
	ROM_LOAD( "bt_p7.rom",   0x08000, 0x8000, CRC(268d10df) SHA1(6a297bbd7b4248306d8756a80f4403c45d833eb3) )
	ROM_LOAD( "bt_p8.rom",   0x10000, 0x8000, CRC(3e39e9dd) SHA1(394c85841113a1b2bdd744445e3e4e3acc7099c6) )
	ROM_LOAD( "bt_p9.rom",   0x18000, 0x8000, CRC(bf56da61) SHA1(855687b6a0a4cef3b8294ca359abe14b11ad5749) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "bt_p11.rom",  0x00000, 0x10000, CRC(aae7410e) SHA1(19dcd13fc53c05bac05d2242965129ab1e3a4a88) )
	ROM_LOAD( "bt_p12.rom",  0x10000, 0x10000, CRC(18914f70) SHA1(2c4e7db8b6e70dffb27d10032f750932c7379a66) )
	ROM_LOAD( "bt_p13.rom",  0x20000, 0x10000, CRC(cd79ce81) SHA1(00f205f8a97f839e2592bdfb624fe6b902ce5a93) )
	ROM_LOAD( "bt_p14.rom",  0x30000, 0x10000, CRC(edc57117) SHA1(899a524973f407c3be1de9dac50f3d373bccb2e5) )
	ROM_LOAD( "bt_p15.rom",  0x40000, 0x10000, CRC(448bf9f4) SHA1(0f880ba3e97a57c937afdce29a1461bc310196eb) )
	ROM_LOAD( "bt_p16.rom",  0x50000, 0x10000, CRC(119999eb) SHA1(0030121239c3ef07c093a7e2146c4027e1b544ac) )
	ROM_LOAD( "bt_p17.rom",  0x60000, 0x10000, CRC(b5462139) SHA1(9af190cf5fabcc017d707be43bd141dc6db12827) )
	ROM_LOAD( "bt_p18.rom",  0x70000, 0x10000, CRC(cb416227) SHA1(aba0b5a0c93713c676a59e8d3c36d780a4e01894) )

	ROM_REGION( 0x20000, "ym2", 0 )
	ROM_LOAD( "btj_p04.bin", 0x00000, 0x10000, CRC(b2e01129) SHA1(b47ffbcbd9a70f74dfd6906d4f9386db24a7294f) )
	ROM_LOAD( "btj_p05.bin", 0x10000, 0x10000, CRC(924c24f7) SHA1(7a2dafbdaa748121fc6279677f6bffd9e10b1a54) )
ROM_END

ROM_START( worldwar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ww4.bin",  0x0000, 0x10000,  CRC(bc29d09f) SHA1(9bd5a47565934590347b7152457869331ae94375) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "ww5.bin",  0x00000, 0x10000, CRC(8dc15909) SHA1(dc0f0e969c36469cc91ecfb1a98cfdb1020972eb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ww3.bin",  0x00000, 0x10000, CRC(8b74c951) SHA1(f4560380f16bcd396d08f48541c65f7be5b290d0) )

	ROM_REGION( 0x1400, "proms", 0 )
	ROM_LOAD( "ww_r.bin",    0x0000, 0x0400, CRC(b88e95f0) SHA1(621c4bf716134d758dae2b3bc519f0a890a11fdb) ) /* red */
	ROM_LOAD( "ww_g.bin",    0x0400, 0x0400, CRC(5e1616b2) SHA1(f2df8f06e717f16c689a941a3a1762dfeb377c83) ) /* green */
	ROM_LOAD( "ww_b.bin",    0x0800, 0x0400, CRC(e9770796) SHA1(2d3001650e781ba7c92a1b3ad0cb9d8c59166e5e) ) /* blue */
	ROM_LOAD( "btj_h.prm",   0x0c00, 0x0400, CRC(c20b197b) SHA1(504cb28d652029fe87a5411d6239e78d93c83e91) ) /* ? */
	ROM_LOAD( "btj_v.prm",   0x1000, 0x0400, CRC(5d0c617f) SHA1(845e52173c33500227cabe1e21b34919d2856215) ) /* ? */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "ww6.bin", 0x0000, 0x8000,  CRC(d57570ab) SHA1(98997de12225d177be4916c7f2e6a7a2df24b8f2) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "ww11.bin", 0x00000, 0x10000, CRC(603ddcb5) SHA1(766d477672f7936a2b12d3aef435b59aaa77886d) )
	ROM_LOAD( "ww12.bin", 0x10000, 0x10000, CRC(388093ff) SHA1(b449031c8225b10d7e27e3a2a0636cfd8cb4e03d) )
	ROM_LOAD( "ww13.bin", 0x20000, 0x10000, CRC(83a7ef62) SHA1(692be1db8b0b0ff518ffe6e000fa8eb0ca7d8b06) )
	ROM_LOAD( "ww14.bin", 0x30000, 0x10000, CRC(04c784be) SHA1(1a485eeb65dee295c791006d58e4e7305bdcf490) )

	ROM_REGION( 0x20000, "sp16_tiles", 0 )
	ROM_LOAD( "ww10.bin", 0x00000, 0x08000, CRC(f68a2d51) SHA1(bf3bfcb7fcb77f4605472775025dc69e979155c8) )
	ROM_LOAD( "ww9.bin",  0x08000, 0x08000, CRC(d9d35911) SHA1(74c23f2967be76ced82522a67291de233528b099) )
	ROM_LOAD( "ww8.bin",  0x10000, 0x08000, CRC(0ec15086) SHA1(6f5fb4a0f96b3ab745f402c04c2cdc2bacaf4844) )
	ROM_LOAD( "ww7.bin",  0x18000, 0x08000, CRC(53c4b24e) SHA1(5f72848f585dcee857715d6ca0020237dd23abc3) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "ww21.bin", 0x00000, 0x10000, CRC(be974fbe) SHA1(bcfafb85ad858fc0a3dceb2d5fe319d812df50fc) )
	ROM_LOAD( "ww22.bin", 0x10000, 0x10000, CRC(9914972a) SHA1(57a27173bc525b18f42699eab9300d4c8652a7c6) )
	ROM_LOAD( "ww19.bin", 0x20000, 0x10000, CRC(c39ac1a7) SHA1(9f8048250306ee23c6c66c751b64f19168123ff3) )
	ROM_LOAD( "ww20.bin", 0x30000, 0x10000, CRC(8504170f) SHA1(e9970d006dbc63640234bb4baa76a10d84f22bcd) )
	ROM_LOAD( "ww15.bin", 0x40000, 0x10000, CRC(d55ce063) SHA1(c0845db7e928e735746822ab94e5f148f38e73cc) )
	ROM_LOAD( "ww16.bin", 0x50000, 0x10000, CRC(a2d19ce5) SHA1(ec1e22c8aa1d24b24fa97015c43e651aebb5a3bb) )
	ROM_LOAD( "ww17.bin", 0x60000, 0x10000, CRC(a9a6b128) SHA1(bd09fcf91211739a304771f633f04235d32b057d) )
	ROM_LOAD( "ww18.bin", 0x70000, 0x10000, CRC(c712d24c) SHA1(59858d446491e63f8bd0fd1f8aa20262fa0522ef) )

	ROM_REGION( 0x20000, "ym2", 0 )
	ROM_LOAD( "bt_p4.rom",  0x00000, 0x10000, CRC(4bc83229) SHA1(b58d08ebed0b02279385a7ac2f385e62443e3de6) )
	ROM_LOAD( "bt_p5.rom",  0x10000, 0x10000, CRC(817bd62c) SHA1(d3ee2ff01a4da8b928728b2fd4948fabd2b04420) )
ROM_END

ROM_START( bermudata )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4",  0x0000, 0x10000,  CRC(4de39d01) SHA1(4312660c6658079c2d148c07d24f741804f3e45c) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "5",  0x00000, 0x10000, CRC(76158e94) SHA1(221e59b3fd87c6193755753d6ac6a96807e23120) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "3",  0x00000, 0x10000, CRC(c79134a8) SHA1(247459d31022f1491978ba7fcc62dd71983c9057) )

	ROM_REGION( 0x1400, "proms", 0 )
	ROM_LOAD( "mb7122e.1k",  0x0000, 0x0400, CRC(1e8fc4c3) SHA1(21b26e6a046c10bab57d2fa986082b7e45a6c4de) ) /* red */
	ROM_LOAD( "mb7122e.2l",  0x0400, 0x0400, CRC(23ce9707) SHA1(c83ef6c3324770c756f1daf01c22214e5dde161e) ) /* green */
	ROM_LOAD( "mb7122e.1l",  0x0800, 0x0400, CRC(26caf985) SHA1(113629bf2e2309dea23a39bc9206e228639d16f3) ) /* blue */
	ROM_LOAD( "btj_h.prm",   0x0c00, 0x0400, CRC(c20b197b) SHA1(504cb28d652029fe87a5411d6239e78d93c83e91) ) /* ? */
	ROM_LOAD( "btj_v.prm",   0x1000, 0x0400, CRC(5d0c617f) SHA1(845e52173c33500227cabe1e21b34919d2856215) ) /* ? */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "6", 0x0000, 0x8000,  CRC(a0e6710c) SHA1(28010eaed046681295661b6fa3e76090ba86592b) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "ww11.bin", 0x00000, 0x10000, CRC(603ddcb5) SHA1(766d477672f7936a2b12d3aef435b59aaa77886d) )
	ROM_LOAD( "ww12.bin", 0x10000, 0x10000, CRC(388093ff) SHA1(b449031c8225b10d7e27e3a2a0636cfd8cb4e03d) )
	ROM_LOAD( "ww13.bin", 0x20000, 0x10000, CRC(83a7ef62) SHA1(692be1db8b0b0ff518ffe6e000fa8eb0ca7d8b06) )
	ROM_LOAD( "ww14.bin", 0x30000, 0x10000, CRC(04c784be) SHA1(1a485eeb65dee295c791006d58e4e7305bdcf490) )

	ROM_REGION( 0x20000, "sp16_tiles", 0 )
	ROM_LOAD( "ww10.bin", 0x00000, 0x08000, CRC(f68a2d51) SHA1(bf3bfcb7fcb77f4605472775025dc69e979155c8) )
	ROM_LOAD( "ww9.bin",  0x08000, 0x08000, CRC(d9d35911) SHA1(74c23f2967be76ced82522a67291de233528b099) )
	ROM_LOAD( "ww8.bin",  0x10000, 0x08000, CRC(0ec15086) SHA1(6f5fb4a0f96b3ab745f402c04c2cdc2bacaf4844) )
	ROM_LOAD( "ww7.bin",  0x18000, 0x08000, CRC(53c4b24e) SHA1(5f72848f585dcee857715d6ca0020237dd23abc3) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "ww21.bin", 0x00000, 0x10000, CRC(be974fbe) SHA1(bcfafb85ad858fc0a3dceb2d5fe319d812df50fc) )
	ROM_LOAD( "ww22.bin", 0x10000, 0x10000, CRC(9914972a) SHA1(57a27173bc525b18f42699eab9300d4c8652a7c6) )
	ROM_LOAD( "ww19.bin", 0x20000, 0x10000, CRC(c39ac1a7) SHA1(9f8048250306ee23c6c66c751b64f19168123ff3) )
	ROM_LOAD( "ww20.bin", 0x30000, 0x10000, CRC(8504170f) SHA1(e9970d006dbc63640234bb4baa76a10d84f22bcd) )
	ROM_LOAD( "ww15.bin", 0x40000, 0x10000, CRC(d55ce063) SHA1(c0845db7e928e735746822ab94e5f148f38e73cc) )
	ROM_LOAD( "ww16.bin", 0x50000, 0x10000, CRC(a2d19ce5) SHA1(ec1e22c8aa1d24b24fa97015c43e651aebb5a3bb) )
	ROM_LOAD( "ww17.bin", 0x60000, 0x10000, CRC(a9a6b128) SHA1(bd09fcf91211739a304771f633f04235d32b057d) )
	ROM_LOAD( "ww18.bin", 0x70000, 0x10000, CRC(c712d24c) SHA1(59858d446491e63f8bd0fd1f8aa20262fa0522ef) )

	ROM_REGION( 0x20000, "ym2", 0 )
	ROM_LOAD( "bt_p4.rom",  0x00000, 0x10000, CRC(4bc83229) SHA1(b58d08ebed0b02279385a7ac2f385e62443e3de6) )
	ROM_LOAD( "bt_p5.rom",  0x10000, 0x10000, CRC(817bd62c) SHA1(d3ee2ff01a4da8b928728b2fd4948fabd2b04420) )
ROM_END

/***********************************************************************/

ROM_START( psychos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p7",  0x00000, 0x10000, CRC(562809f4) SHA1(71d2a0fbfbe953e2bc4169d3c0a4f259911f04c3) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "up03_m8.rom",  0x00000, 0x10000, CRC(5f426ddb) SHA1(d4b2215122b23066ba2b231992f0f27057259ded) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p5",  0x0000, 0x10000,  CRC(64503283) SHA1(e380164ac4268eda1d9ca2404b3dddc5fd3f9dcc) )

	ROM_REGION( 0x1400, "proms", 0 )
	ROM_LOAD( "up03_k1.rom",  0x0000, 0x400, CRC(27b8ca8c) SHA1(a2dbc22ca10c2c2c874bf766fe64981f9be75aba) ) /* red */
	ROM_LOAD( "up03_l1.rom",  0x0400, 0x400, CRC(40e78c9e) SHA1(779c84e5a40365d36088a018d9d1a3524f53844a) ) /* green */
	ROM_LOAD( "up03_k2.rom",  0x0800, 0x400, CRC(d845d5ac) SHA1(e1e0954c44264456a02aebe5e3b0bba6031b837b) ) /* blue */
	ROM_LOAD( "mb7122e.8j",   0x0c00, 0x400, CRC(c20b197b) SHA1(504cb28d652029fe87a5411d6239e78d93c83e91) ) /* ? */
	ROM_LOAD( "mb7122e.8k",   0x1000, 0x400, CRC(5d0c617f) SHA1(845e52173c33500227cabe1e21b34919d2856215) ) /* ? */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "up02_a3.rom",  0x0000, 0x8000,  CRC(11a71919) SHA1(ffb8c54ad5162ea5040508ccb9244b7cd087c047) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "up01_f1.rom",  0x00000, 0x10000, CRC(167e5765) SHA1(5deb66255278e1891c344e0e9665c6f0fda59391) )
	ROM_LOAD( "up01_d1.rom",  0x10000, 0x10000, CRC(8b0fe8d0) SHA1(30b24878e0e333a635dae475b6527b03b9e0302c) )
	ROM_LOAD( "up01_c1.rom",  0x20000, 0x10000, CRC(f4361c50) SHA1(59d0915c4c4d07e26d205ffee95d7628f8eefb6d) )
	ROM_LOAD( "up01_a1.rom",  0x30000, 0x10000, CRC(e4b0b95e) SHA1(8e35138f9d1fc6c1d787cf09ec17a900710db375) )

	ROM_REGION( 0x20000, "sp16_tiles", 0 )
	ROM_LOAD( "up02_f3.rom",  0x00000, 0x8000, CRC(f96f82db) SHA1(8062721431762dfcf7cc499a1f050e4cbe0fc793) )
	ROM_LOAD( "up02_e3.rom",  0x08000, 0x8000, CRC(2b007733) SHA1(7b808a134a9aa70aef1cf2a503b7ea786fd05275) )
	ROM_LOAD( "up02_c3.rom",  0x10000, 0x8000, CRC(efa830e1) SHA1(0a41a764a751a6566b9bb58086a417cfb7925d50) )
	ROM_LOAD( "up02_b3.rom",  0x18000, 0x8000, CRC(24559ee1) SHA1(ca2166558a8dffba9042349db2f85f9111bd8d93) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "up01_f10.rom",  0x00000, 0x10000, CRC(2bac250e) SHA1(aaf424fb9663e14c19d4063a402fb3c4c5c5b059) )
	ROM_LOAD( "up01_h10.rom",  0x10000, 0x10000, CRC(5e1ba353) SHA1(1219cd11d5837c6680f6cbbf91cfece6564cacaa) )
	ROM_LOAD( "up01_j10.rom",  0x20000, 0x10000, CRC(9ff91a97) SHA1(064047800f3d7cb6eaf35988e0db0dc4dfa7e10f) )
	ROM_LOAD( "up01_l10.rom",  0x30000, 0x10000, CRC(ae1965ef) SHA1(7da6f14fa46f0443da8502f61e9f7d4aa603a19b) )
	ROM_LOAD( "up01_m10.rom",  0x40000, 0x10000, CRC(df283b67) SHA1(92650d3517efdef1358f5c9b9ee30d48a3bcc45a) )
	ROM_LOAD( "up01_n10.rom",  0x50000, 0x10000, CRC(914f051f) SHA1(743aa05ce1b4a9a49e9515e6c56c721bebd2bd2c) )
	ROM_LOAD( "up01_r10.rom",  0x60000, 0x10000, CRC(c4488472) SHA1(98540ca924cc20e82859b7bb88e521ff3f9f3b37) )
	ROM_LOAD( "up01_s10.rom",  0x70000, 0x10000, CRC(8ec7fe18) SHA1(65697058fe557066921072df691f3aa19f54968c) )

	ROM_REGION( 0x40000, "ym2", 0 )
	ROM_LOAD( "p1",  0x00000, 0x10000, CRC(58f1683f) SHA1(8b713b2806d1a56794c990ed221ce016bb881082) )
	ROM_LOAD( "p2",  0x10000, 0x10000, CRC(da3abda1) SHA1(aeafe8f41c0ea2f93791abce01a53d8e417d1216) )
	ROM_LOAD( "p3",  0x20000, 0x10000, CRC(f3683ae8) SHA1(a2e77995f835eaa211ea7d384382cf6a5a121490) )
	ROM_LOAD( "p4",  0x30000, 0x10000, CRC(437d775a) SHA1(355c227b22ae34f47e2bb27d4b5440ccaedf2eea) )
ROM_END

ROM_START( psychosj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "up03_m4.rom",  0x0000, 0x10000,  CRC(05dfb409) SHA1(e6c378c86689c7ab9190908c8e4aa2d4563c3774) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "up03_m8.rom",  0x00000, 0x10000, CRC(5f426ddb) SHA1(d4b2215122b23066ba2b231992f0f27057259ded) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "up03_j6.rom",  0x00000, 0x10000, CRC(bbd0a8e3) SHA1(ea8ca9de8f6042cf14ebfc83bc956751358f9521) )

	ROM_REGION( 0x1400, "proms", 0 )
	ROM_LOAD( "up03_k1.rom",  0x0000, 0x400, CRC(27b8ca8c) SHA1(a2dbc22ca10c2c2c874bf766fe64981f9be75aba) ) /* red */
	ROM_LOAD( "up03_l1.rom",  0x0400, 0x400, CRC(40e78c9e) SHA1(779c84e5a40365d36088a018d9d1a3524f53844a) ) /* green */
	ROM_LOAD( "up03_k2.rom",  0x0800, 0x400, CRC(d845d5ac) SHA1(e1e0954c44264456a02aebe5e3b0bba6031b837b) ) /* blue */
	ROM_LOAD( "mb7122e.8j",   0x0c00, 0x400, CRC(c20b197b) SHA1(504cb28d652029fe87a5411d6239e78d93c83e91) ) /* ? */
	ROM_LOAD( "mb7122e.8k",   0x1000, 0x400, CRC(5d0c617f) SHA1(845e52173c33500227cabe1e21b34919d2856215) ) /* ? */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "up02_a3.rom",  0x0000, 0x8000,  CRC(11a71919) SHA1(ffb8c54ad5162ea5040508ccb9244b7cd087c047) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "up01_f1.rom",  0x00000, 0x10000, CRC(167e5765) SHA1(5deb66255278e1891c344e0e9665c6f0fda59391) )
	ROM_LOAD( "up01_d1.rom",  0x10000, 0x10000, CRC(8b0fe8d0) SHA1(30b24878e0e333a635dae475b6527b03b9e0302c) )
	ROM_LOAD( "up01_c1.rom",  0x20000, 0x10000, CRC(f4361c50) SHA1(59d0915c4c4d07e26d205ffee95d7628f8eefb6d) )
	ROM_LOAD( "up01_a1.rom",  0x30000, 0x10000, CRC(e4b0b95e) SHA1(8e35138f9d1fc6c1d787cf09ec17a900710db375) )

	ROM_REGION( 0x20000, "sp16_tiles", 0 )
	ROM_LOAD( "up02_f3.rom",  0x00000, 0x8000, CRC(f96f82db) SHA1(8062721431762dfcf7cc499a1f050e4cbe0fc793) )
	ROM_LOAD( "up02_e3.rom",  0x08000, 0x8000, CRC(2b007733) SHA1(7b808a134a9aa70aef1cf2a503b7ea786fd05275) )
	ROM_LOAD( "up02_c3.rom",  0x10000, 0x8000, CRC(efa830e1) SHA1(0a41a764a751a6566b9bb58086a417cfb7925d50) )
	ROM_LOAD( "up02_b3.rom",  0x18000, 0x8000, CRC(24559ee1) SHA1(ca2166558a8dffba9042349db2f85f9111bd8d93) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "up01_f10.rom",  0x00000, 0x10000, CRC(2bac250e) SHA1(aaf424fb9663e14c19d4063a402fb3c4c5c5b059) )
	ROM_LOAD( "up01_h10.rom",  0x10000, 0x10000, CRC(5e1ba353) SHA1(1219cd11d5837c6680f6cbbf91cfece6564cacaa) )
	ROM_LOAD( "up01_j10.rom",  0x20000, 0x10000, CRC(9ff91a97) SHA1(064047800f3d7cb6eaf35988e0db0dc4dfa7e10f) )
	ROM_LOAD( "up01_l10.rom",  0x30000, 0x10000, CRC(ae1965ef) SHA1(7da6f14fa46f0443da8502f61e9f7d4aa603a19b) )
	ROM_LOAD( "up01_m10.rom",  0x40000, 0x10000, CRC(df283b67) SHA1(92650d3517efdef1358f5c9b9ee30d48a3bcc45a) )
	ROM_LOAD( "up01_n10.rom",  0x50000, 0x10000, CRC(914f051f) SHA1(743aa05ce1b4a9a49e9515e6c56c721bebd2bd2c) )
	ROM_LOAD( "up01_r10.rom",  0x60000, 0x10000, CRC(c4488472) SHA1(98540ca924cc20e82859b7bb88e521ff3f9f3b37) )
	ROM_LOAD( "up01_s10.rom",  0x70000, 0x10000, CRC(8ec7fe18) SHA1(65697058fe557066921072df691f3aa19f54968c) )

	ROM_REGION( 0x40000, "ym2", 0 )
	ROM_LOAD( "up03_b5.rom",  0x00000, 0x10000, CRC(0f8e8276) SHA1(8894ccccaf67ae3cfea926725c114f8e5607e4b2) )
	ROM_LOAD( "up03_c5.rom",  0x10000, 0x10000, CRC(34e41dfb) SHA1(cdc4cb47a31c4f6eee8bc804389ee62af5173c15) )
	ROM_LOAD( "up03_d5.rom",  0x20000, 0x10000, CRC(aa583c5e) SHA1(8433517d789c6b30938bfef366b44a0412dd5e7e) )
	ROM_LOAD( "up03_f5.rom",  0x30000, 0x10000, CRC(7e8bce7a) SHA1(dd482045332719c76e598110d7285997b337352a) )
ROM_END

/***********************************************************************/

ROM_START( gwar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7g",  0x00000, 0x10000, CRC(5bcfa7dc) SHA1(1af2c36df287c9c84be8e7fc173b66f3dde5375e) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "g02",  0x00000, 0x10000, CRC(86d931bf) SHA1(8bf7c7a7c01561568973d01956e5398bbc9c3463) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "g03",  0x00000, 0x10000, CRC(eb544ab9) SHA1(433af63feb4c4ef0e3bd383f2f9bc19e436fb103) )

	ROM_REGION( 0x2400, "proms", 0 )
	ROM_LOAD( "guprom.3",  0x0000, 0x0400, CRC(090236a3) SHA1(40d066e34291492c6baf8c120657e3d547274b59) ) /* red */
	ROM_LOAD( "guprom.2",  0x0400, 0x0400, CRC(9147de69) SHA1(e4b3b546e429c195e82f97322e2a295882e38a58) ) /* green */
	ROM_LOAD( "guprom.1",  0x0800, 0x0400, CRC(7f9c839e) SHA1(2fa60fa335f76891d961c9bd0066fa7f82f76779) ) /* blue */
	ROM_LOAD( "btj_h.prm", 0x0c00, 0x0400, CRC(c20b197b) SHA1(504cb28d652029fe87a5411d6239e78d93c83e91) ) /* h-decode */
	ROM_LOAD( "btj_v.prm", 0x1000, 0x0400, CRC(5d0c617f) SHA1(845e52173c33500227cabe1e21b34919d2856215) ) /* v-decode */
	ROM_LOAD( "ls.bin",    0x1400, 0x1000, CRC(73df921d) SHA1(c0f765da3e0e80d104b0baaa7a83bdcc399254b3) ) /* ls-joystick encoder */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "g05",  0x0000, 0x08000, CRC(80f73e2e) SHA1(820824fb10f7dfec6247b46dde8ff7124bde3734) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "g06",  0x00000, 0x10000, CRC(f1dcdaef) SHA1(d9b65e7f4025787037628528d3bef699be2eb874) )
	ROM_LOAD( "g07",  0x10000, 0x10000, CRC(326e4e5e) SHA1(6935429925d748bb43072429db0d3b08ffdbc95d) )
	ROM_LOAD( "g08",  0x20000, 0x10000, CRC(0aa70967) SHA1(a6cbadbb960280b5e79660c0bbd43089ced39a44) )
	ROM_LOAD( "g09",  0x30000, 0x10000, CRC(b7686336) SHA1(d654d282862ff00488be38fb9c1302c8bb6f7e7c) )

	ROM_REGION( 0x40000, "sp16_tiles", 0 )
	ROM_LOAD( "g10",  0x00000, 0x10000, CRC(58600f7d) SHA1(3dcd25d1ed07e6f74f3316ebe41768eb155f4c45) )
	ROM_LOAD( "g11",  0x10000, 0x10000, CRC(a3f9b463) SHA1(ee83d18cf08972c792b05c277b1ca25d732e294d) )
	ROM_LOAD( "g12",  0x20000, 0x10000, CRC(092501be) SHA1(85d9a8922dde6824805a4b8e6c52b2a9ad092df9) )
	ROM_LOAD( "g13",  0x30000, 0x10000, CRC(25801ea6) SHA1(1aa61716d6be399a1eee2ee5079f13da0f1bd4e8) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "g20",  0x00000, 0x10000, CRC(2b46edff) SHA1(db97e042621dcbedfeed71937ead6d715899d4f7) )
	ROM_LOAD( "g21",  0x10000, 0x10000, CRC(be19888d) SHA1(bc7b1b6236d41685faacc2008d51ae2da9a82909) )
	ROM_LOAD( "g18",  0x20000, 0x10000, CRC(2d653f0c) SHA1(99eb7883822b10f61b6e922c0d0519aacac83732) )
	ROM_LOAD( "g19",  0x30000, 0x10000, CRC(ebbf3ba2) SHA1(bc3631c43058faf1ec6b21ed8017b744afee6f5d) )
	ROM_LOAD( "g16",  0x40000, 0x10000, CRC(aeb3707f) SHA1(58d1a71cf83ab0f5f0dd67d441edbc8ece8c2ba5) )
	ROM_LOAD( "g17",  0x50000, 0x10000, CRC(0808f95f) SHA1(f67763cceb287a02e3b946ade52105a72161e540) )
	ROM_LOAD( "g14",  0x60000, 0x10000, CRC(8dfc7b87) SHA1(e3d75020aa1b90f12633f6515a0386f87441b225) )
	ROM_LOAD( "g15",  0x70000, 0x10000, CRC(06822aac) SHA1(630d438cbebe0b5af571948d0d3f4996f52aae1d) )

	ROM_REGION( 0x10000, "ym2", 0 )
	ROM_LOAD( "g04",  0x00000, 0x10000, CRC(2255f8dd) SHA1(fac31b617762d0fa39cf82a658be250b91ab73ce) )
ROM_END

ROM_START( gwara )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gv ver1 3.p4", 0x00000, 0x10000, CRC(24936d83) SHA1(33842322ead66e426946c6cfaa04e56afea90d78) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gv4.p8", 0x00000, 0x10000, CRC(26335a55) SHA1(de3e7d9e204a969745367aa37326d7b3e28c7424) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gv2.k7", 0x00000, 0x10000, CRC(896682dd) SHA1(dc2125c2378a01291197b2798a5eef6459cf5b99) )

	ROM_REGION( 0x2400, "proms", 0 )
	ROM_LOAD( "1.k1",        0x0000, 0x0400, CRC(090236a3) SHA1(40d066e34291492c6baf8c120657e3d547274b59) ) /* MB7122, red */
	ROM_LOAD( "3.l2",        0x0400, 0x0400, CRC(9147de69) SHA1(e4b3b546e429c195e82f97322e2a295882e38a58) ) /* MB7122, green */
	ROM_LOAD( "2.l1",        0x0800, 0x0400, CRC(7f9c839e) SHA1(2fa60fa335f76891d961c9bd0066fa7f82f76779) ) /* MB7122, blue */
	ROM_LOAD( "horizon.j8",  0x0c00, 0x0400, CRC(c20b197b) SHA1(504cb28d652029fe87a5411d6239e78d93c83e91) ) /* MB7122E, h-decode */
	ROM_LOAD( "vertical.k8", 0x1000, 0x0400, CRC(5d0c617f) SHA1(845e52173c33500227cabe1e21b34919d2856215) ) /* MB7122E, v-decode */
	ROM_LOAD( "ls.bin",      0x1400, 0x1000, CRC(73df921d) SHA1(c0f765da3e0e80d104b0baaa7a83bdcc399254b3) ) /* ls-joystick encoder */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "gv5.a3", 0x0000, 0x08000, CRC(80f73e2e) SHA1(820824fb10f7dfec6247b46dde8ff7124bde3734) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "gv13.ef1", 0x00000, 0x10000, CRC(f1dcdaef) SHA1(d9b65e7f4025787037628528d3bef699be2eb874) )
	ROM_LOAD( "gv12.d1",  0x10000, 0x10000, CRC(326e4e5e) SHA1(6935429925d748bb43072429db0d3b08ffdbc95d) )
	ROM_LOAD( "gv11.c1",  0x20000, 0x10000, CRC(0aa70967) SHA1(a6cbadbb960280b5e79660c0bbd43089ced39a44) )
	ROM_LOAD( "gv10.a1",  0x30000, 0x10000, CRC(b7686336) SHA1(d654d282862ff00488be38fb9c1302c8bb6f7e7c) )

	ROM_REGION( 0x40000, "sp16_tiles", 0 )
	ROM_LOAD( "gv9.g3",  0x00000, 0x10000, CRC(58600f7d) SHA1(3dcd25d1ed07e6f74f3316ebe41768eb155f4c45) )
	ROM_LOAD( "gv8.e3",  0x10000, 0x10000, CRC(a3f9b463) SHA1(ee83d18cf08972c792b05c277b1ca25d732e294d) )
	ROM_LOAD( "gv7.cd3", 0x20000, 0x10000, CRC(092501be) SHA1(85d9a8922dde6824805a4b8e6c52b2a9ad092df9) )
	ROM_LOAD( "gv6.b3",  0x30000, 0x10000, CRC(25801ea6) SHA1(1aa61716d6be399a1eee2ee5079f13da0f1bd4e8) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "gv14.f10",  0x00000, 0x10000, CRC(2b46edff) SHA1(db97e042621dcbedfeed71937ead6d715899d4f7) )
	ROM_LOAD( "gv15.h10",  0x10000, 0x10000, CRC(be19888d) SHA1(bc7b1b6236d41685faacc2008d51ae2da9a82909) )
	ROM_LOAD( "gv16.j10",  0x20000, 0x10000, CRC(2d653f0c) SHA1(99eb7883822b10f61b6e922c0d0519aacac83732) )
	ROM_LOAD( "gv17.l10",  0x30000, 0x10000, CRC(ebbf3ba2) SHA1(bc3631c43058faf1ec6b21ed8017b744afee6f5d) )
	ROM_LOAD( "gv18.m10",  0x40000, 0x10000, CRC(aeb3707f) SHA1(58d1a71cf83ab0f5f0dd67d441edbc8ece8c2ba5) )
	ROM_LOAD( "gv19.pn10", 0x50000, 0x10000, CRC(0808f95f) SHA1(f67763cceb287a02e3b946ade52105a72161e540) )
	ROM_LOAD( "gv20.r10",  0x60000, 0x10000, CRC(8dfc7b87) SHA1(e3d75020aa1b90f12633f6515a0386f87441b225) )
	ROM_LOAD( "gv21.s10",  0x70000, 0x10000, CRC(06822aac) SHA1(630d438cbebe0b5af571948d0d3f4996f52aae1d) )

	ROM_REGION( 0x10000, "ym2", 0 )
	ROM_LOAD( "gv1.g5", 0x00000, 0x10000, CRC(2255f8dd) SHA1(fac31b617762d0fa39cf82a658be250b91ab73ce) )
ROM_END

ROM_START( gwarj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7y3047",  0x00000, 0x10000, CRC(7f8a880c) SHA1(1eb1c3eb45aa933118e5bd116eb3f81f39063ae3) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "g02",  0x00000, 0x10000, CRC(86d931bf) SHA1(8bf7c7a7c01561568973d01956e5398bbc9c3463) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "g03",  0x00000, 0x10000, CRC(eb544ab9) SHA1(433af63feb4c4ef0e3bd383f2f9bc19e436fb103) )

	ROM_REGION( 0x2400, "proms", 0 )
	ROM_LOAD( "guprom.3",  0x0000, 0x0400, CRC(090236a3) SHA1(40d066e34291492c6baf8c120657e3d547274b59) ) /* red */
	ROM_LOAD( "guprom.2",  0x0400, 0x0400, CRC(9147de69) SHA1(e4b3b546e429c195e82f97322e2a295882e38a58) ) /* green */
	ROM_LOAD( "guprom.1",  0x0800, 0x0400, CRC(7f9c839e) SHA1(2fa60fa335f76891d961c9bd0066fa7f82f76779) ) /* blue */
	ROM_LOAD( "btj_h.prm", 0x0c00, 0x0400, CRC(c20b197b) SHA1(504cb28d652029fe87a5411d6239e78d93c83e91) ) /* h-decode */
	ROM_LOAD( "btj_v.prm", 0x1000, 0x0400, CRC(5d0c617f) SHA1(845e52173c33500227cabe1e21b34919d2856215) ) /* v-decode */
	ROM_LOAD( "ls.bin",    0x1400, 0x1000, CRC(73df921d) SHA1(c0f765da3e0e80d104b0baaa7a83bdcc399254b3) ) /* ls-joystick encoder */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "792001",  0x0000, 0x08000, CRC(99d7ddf3) SHA1(4e4bc400d184e1fb9d0af3a33cc6f6d099bb3bee) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "g06",  0x00000, 0x10000, CRC(f1dcdaef) SHA1(d9b65e7f4025787037628528d3bef699be2eb874) )
	ROM_LOAD( "g07",  0x10000, 0x10000, CRC(326e4e5e) SHA1(6935429925d748bb43072429db0d3b08ffdbc95d) )
	ROM_LOAD( "g08",  0x20000, 0x10000, CRC(0aa70967) SHA1(a6cbadbb960280b5e79660c0bbd43089ced39a44) )
	ROM_LOAD( "g09",  0x30000, 0x10000, CRC(b7686336) SHA1(d654d282862ff00488be38fb9c1302c8bb6f7e7c) )

	ROM_REGION( 0x40000, "sp16_tiles", 0 )
	ROM_LOAD( "g10",  0x00000, 0x10000, CRC(58600f7d) SHA1(3dcd25d1ed07e6f74f3316ebe41768eb155f4c45) )
	ROM_LOAD( "g11",  0x10000, 0x10000, CRC(a3f9b463) SHA1(ee83d18cf08972c792b05c277b1ca25d732e294d) )
	ROM_LOAD( "g12",  0x20000, 0x10000, CRC(092501be) SHA1(85d9a8922dde6824805a4b8e6c52b2a9ad092df9) )
	ROM_LOAD( "g13",  0x30000, 0x10000, CRC(25801ea6) SHA1(1aa61716d6be399a1eee2ee5079f13da0f1bd4e8) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "g20",  0x00000, 0x10000, CRC(2b46edff) SHA1(db97e042621dcbedfeed71937ead6d715899d4f7) )
	ROM_LOAD( "g21",  0x10000, 0x10000, CRC(be19888d) SHA1(bc7b1b6236d41685faacc2008d51ae2da9a82909) )
	ROM_LOAD( "g18",  0x20000, 0x10000, CRC(2d653f0c) SHA1(99eb7883822b10f61b6e922c0d0519aacac83732) )
	ROM_LOAD( "g19",  0x30000, 0x10000, CRC(ebbf3ba2) SHA1(bc3631c43058faf1ec6b21ed8017b744afee6f5d) )
	ROM_LOAD( "g16",  0x40000, 0x10000, CRC(aeb3707f) SHA1(58d1a71cf83ab0f5f0dd67d441edbc8ece8c2ba5) )
	ROM_LOAD( "g17",  0x50000, 0x10000, CRC(0808f95f) SHA1(f67763cceb287a02e3b946ade52105a72161e540) )
	ROM_LOAD( "g14",  0x60000, 0x10000, CRC(8dfc7b87) SHA1(e3d75020aa1b90f12633f6515a0386f87441b225) )
	ROM_LOAD( "g15",  0x70000, 0x10000, CRC(06822aac) SHA1(630d438cbebe0b5af571948d0d3f4996f52aae1d) )

	ROM_REGION( 0x10000, "ym2", 0 )
	ROM_LOAD( "g04",  0x00000, 0x10000, CRC(2255f8dd) SHA1(fac31b617762d0fa39cf82a658be250b91ab73ce) )
ROM_END

ROM_START( gwarb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g01",  0x00000, 0x10000, CRC(ce1d3c80) SHA1(605ada3529d0b26425e6c573c31117249bb7a7db) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "g02",  0x00000, 0x10000, CRC(86d931bf) SHA1(8bf7c7a7c01561568973d01956e5398bbc9c3463) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "g03",  0x00000, 0x10000, CRC(eb544ab9) SHA1(433af63feb4c4ef0e3bd383f2f9bc19e436fb103) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "guprom.3", 0x000, 0x400, CRC(090236a3) SHA1(40d066e34291492c6baf8c120657e3d547274b59) ) /* red */ // up03_k1.rom
	ROM_LOAD( "guprom.2", 0x400, 0x400, CRC(9147de69) SHA1(e4b3b546e429c195e82f97322e2a295882e38a58) ) /* green */ // up03_l1.rom
	ROM_LOAD( "guprom.1", 0x800, 0x400, CRC(7f9c839e) SHA1(2fa60fa335f76891d961c9bd0066fa7f82f76779) ) /* blue */ // up03_k2.rom

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "g05",  0x0000, 0x08000, CRC(80f73e2e) SHA1(820824fb10f7dfec6247b46dde8ff7124bde3734) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "g06",  0x00000, 0x10000, CRC(f1dcdaef) SHA1(d9b65e7f4025787037628528d3bef699be2eb874) )
	ROM_LOAD( "g07",  0x10000, 0x10000, CRC(326e4e5e) SHA1(6935429925d748bb43072429db0d3b08ffdbc95d) )
	ROM_LOAD( "g08",  0x20000, 0x10000, CRC(0aa70967) SHA1(a6cbadbb960280b5e79660c0bbd43089ced39a44) )
	ROM_LOAD( "g09",  0x30000, 0x10000, CRC(b7686336) SHA1(d654d282862ff00488be38fb9c1302c8bb6f7e7c) )

	ROM_REGION( 0x40000, "sp16_tiles", 0 )
	ROM_LOAD( "g10",  0x00000, 0x10000, CRC(58600f7d) SHA1(3dcd25d1ed07e6f74f3316ebe41768eb155f4c45) )
	ROM_LOAD( "g11",  0x10000, 0x10000, CRC(a3f9b463) SHA1(ee83d18cf08972c792b05c277b1ca25d732e294d) )
	ROM_LOAD( "g12",  0x20000, 0x10000, CRC(092501be) SHA1(85d9a8922dde6824805a4b8e6c52b2a9ad092df9) )
	ROM_LOAD( "g13",  0x30000, 0x10000, CRC(25801ea6) SHA1(1aa61716d6be399a1eee2ee5079f13da0f1bd4e8) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "g20",  0x00000, 0x10000, CRC(2b46edff) SHA1(db97e042621dcbedfeed71937ead6d715899d4f7) )
	ROM_LOAD( "g21",  0x10000, 0x10000, CRC(be19888d) SHA1(bc7b1b6236d41685faacc2008d51ae2da9a82909) )
	ROM_LOAD( "g18",  0x20000, 0x10000, CRC(2d653f0c) SHA1(99eb7883822b10f61b6e922c0d0519aacac83732) )
	ROM_LOAD( "g19",  0x30000, 0x10000, CRC(ebbf3ba2) SHA1(bc3631c43058faf1ec6b21ed8017b744afee6f5d) )
	ROM_LOAD( "g16",  0x40000, 0x10000, CRC(aeb3707f) SHA1(58d1a71cf83ab0f5f0dd67d441edbc8ece8c2ba5) )
	ROM_LOAD( "g17",  0x50000, 0x10000, CRC(0808f95f) SHA1(f67763cceb287a02e3b946ade52105a72161e540) )
	ROM_LOAD( "g14",  0x60000, 0x10000, CRC(8dfc7b87) SHA1(e3d75020aa1b90f12633f6515a0386f87441b225) )
	ROM_LOAD( "g15",  0x70000, 0x10000, CRC(06822aac) SHA1(630d438cbebe0b5af571948d0d3f4996f52aae1d) )

	ROM_REGION( 0x10000, "ym2", 0 )
	ROM_LOAD( "g04",  0x00000, 0x10000, CRC(2255f8dd) SHA1(fac31b617762d0fa39cf82a658be250b91ab73ce) )
ROM_END

/***********************************************************************/

ROM_START( chopper )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kk_01.rom",  0x0000, 0x10000,  CRC(8fa2f839) SHA1(13cfdbeb433aa3e1dc7e7927c00690e02ed08274) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "kk_04.rom",  0x00000, 0x10000, CRC(004f7d9a) SHA1(4d1c830f69dbf2f1523f9ad7da9b3275fd6b5dfb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "kk_03.rom",  0x00000, 0x10000, CRC(dbaafb87) SHA1(e7d7f68250bda217230560481ba51da335fc05d7) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "up03_k1.rom",  0x0000, 0x0400, CRC(7f07a45c) SHA1(f751d01d9c25609195b19643395290dab8b8bc5c) ) /* red */
	ROM_LOAD( "up03_l1.rom",  0x0400, 0x0400, CRC(15359fc3) SHA1(4ced674fb18b80ebe5fd6459e0fb9542461dbc74) ) /* green */
	ROM_LOAD( "up03_k2.rom",  0x0800, 0x0400, CRC(79b50f7d) SHA1(41579e498046570a6a74309310b5341fcde9c7de) ) /* blue */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "kk_05.rom",  0x0000, 0x8000, CRC(defc0987) SHA1(ea8eca852aadce90857eb8e65d78631409faac01) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "kk_10.rom",  0x00000, 0x10000, CRC(5cf4d22b) SHA1(b66864740898478becb188d7dd34d61187926e4d) )
	ROM_LOAD( "kk_11.rom",  0x10000, 0x10000, CRC(9af4cad0) SHA1(dd8c1a76e6a90661c5442c0a096cb9ffe496d12a) )
	ROM_LOAD( "kk_12.rom",  0x20000, 0x10000, CRC(02fec778) SHA1(477a3e22f913cc7783d6cbfce86f98fea9eaf3ec) )
	ROM_LOAD( "kk_13.rom",  0x30000, 0x10000, CRC(2756817d) SHA1(acde21454ddf843425deff3357c9e3a7e7a2baec) )

	ROM_REGION( 0x20000, "sp16_tiles", 0 )
	ROM_LOAD( "kk_09.rom",  0x00000, 0x08000, CRC(653c4342) SHA1(aacb3a7772dcea4c88f0010b3654f4159cfb6a8b) )
	ROM_LOAD( "kk_08.rom",  0x08000, 0x08000, CRC(2da45894) SHA1(09f1ac544a119c9d3a9eeb0606f35585d35c2d1d) )
	ROM_LOAD( "kk_07.rom",  0x10000, 0x08000, CRC(a0ebebdf) SHA1(83d8a9ba7b7ffd42e50afb017e4d0d40fe3e2739) )
	ROM_LOAD( "kk_06.rom",  0x18000, 0x08000, CRC(284fad9e) SHA1(7bb572d7d5983a514e8381954ac89a720b86e9ba) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "kk_18.rom",  0x00000, 0x10000, CRC(6abbff36) SHA1(8dde3163e454876a4b666b25c81c09b0740143b5) )
	ROM_LOAD( "kk_19.rom",  0x10000, 0x10000, CRC(5283b4d3) SHA1(980f74d3f468203cf9c1a5f3bc256139975035f3) )
	ROM_LOAD( "kk_20.rom",  0x20000, 0x10000, CRC(6403ddf2) SHA1(54a044d9a1ba89fec3bea0e771f75fcc532e7aad) )
	ROM_LOAD( "kk_21.rom",  0x30000, 0x10000, CRC(9f411940) SHA1(73b0bd360a76ab183f8c7b41f78e930e49e2600c) )
	ROM_LOAD( "kk_14.rom",  0x40000, 0x10000, CRC(9bad9e25) SHA1(0eb1e23dc7084172dd19927a1b084101d10b5137) )
	ROM_LOAD( "kk_15.rom",  0x50000, 0x10000, CRC(89faf590) SHA1(876fc6dac48fef396670522470c41fc9d9b6a0b2) )
	ROM_LOAD( "kk_16.rom",  0x60000, 0x10000, CRC(efb1fb6c) SHA1(12edd64e29472f3c6822f957b23547c64dab65d2) )
	ROM_LOAD( "kk_17.rom",  0x70000, 0x10000, CRC(6b7fb0a5) SHA1(805ee6f439d9e921e1ece27438ba9c00b870e305) )

	ROM_REGION( 0x10000, "ym2", 0 )
	ROM_LOAD( "kk_02.rom",  0x00000, 0x10000, CRC(06169ae0) SHA1(2690ce7cb28cf5c6d37886ce5fbe444067c08403) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16r6b.2c", 0x0000, 0x0104, CRC(311e5ae6) SHA1(8a3799e1611ac4086dda2755c5ad44c0dc16ff5b) )
ROM_END

ROM_START( choppera )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1a.rom",  0x0000, 0x10000,  CRC(dc325860) SHA1(89391897e6f31d9c1d3b7f27618f63fe8018d42a) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "4a.rom",  0x00000, 0x10000, CRC(56d10ba3) SHA1(345a80239fd425c7fe1dfec9385c99a307511e00) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "kk_03.rom",  0x00000, 0x10000, CRC(dbaafb87) SHA1(e7d7f68250bda217230560481ba51da335fc05d7) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "up03_k1.rom",  0x0000, 0x0400, CRC(7f07a45c) SHA1(f751d01d9c25609195b19643395290dab8b8bc5c) ) /* red */
	ROM_LOAD( "up03_l1.rom",  0x0400, 0x0400, CRC(15359fc3) SHA1(4ced674fb18b80ebe5fd6459e0fb9542461dbc74) ) /* green */
	ROM_LOAD( "up03_k2.rom",  0x0800, 0x0400, CRC(79b50f7d) SHA1(41579e498046570a6a74309310b5341fcde9c7de) ) /* blue */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "kk_05.rom",  0x0000, 0x8000, CRC(defc0987) SHA1(ea8eca852aadce90857eb8e65d78631409faac01) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "kk_10.rom",  0x00000, 0x10000, CRC(5cf4d22b) SHA1(b66864740898478becb188d7dd34d61187926e4d) )
	ROM_LOAD( "11a.rom",    0x10000, 0x10000, CRC(881ac259) SHA1(6cce41878c9d9712996d4987a9a578f1301b8feb) )
	ROM_LOAD( "12a.rom",    0x20000, 0x10000, CRC(de96b331) SHA1(725cfe739f7ed0f37eb620d9566bfda1369f4d50) )
	ROM_LOAD( "kk_13.rom",  0x30000, 0x10000, CRC(2756817d) SHA1(acde21454ddf843425deff3357c9e3a7e7a2baec) )

	ROM_REGION( 0x20000, "sp16_tiles", 0 )
	ROM_LOAD( "9a.rom",  0x00000, 0x08000, CRC(106c2dcc) SHA1(919497757664c92e9955db50f5096ac81cec33c3) )
	ROM_LOAD( "8a.rom",  0x08000, 0x08000, CRC(d4f88f62) SHA1(ac89ffa83e0e207acce39711b93d94affc61c1cc) )
	ROM_LOAD( "7a.rom",  0x10000, 0x08000, CRC(28ae39f9) SHA1(7d51489b824b76710f6d4434a77f5f2833fcc532) )
	ROM_LOAD( "6a.rom",  0x18000, 0x08000, CRC(16774a36) SHA1(d1207513f790a30eef8802e63cfeeb10321d6ff7) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "kk_18.rom",  0x00000, 0x10000, CRC(6abbff36) SHA1(8dde3163e454876a4b666b25c81c09b0740143b5) )
	ROM_LOAD( "kk_19.rom",  0x10000, 0x10000, CRC(5283b4d3) SHA1(980f74d3f468203cf9c1a5f3bc256139975035f3) )
	ROM_LOAD( "kk_20.rom",  0x20000, 0x10000, CRC(6403ddf2) SHA1(54a044d9a1ba89fec3bea0e771f75fcc532e7aad) )
	ROM_LOAD( "kk_21.rom",  0x30000, 0x10000, CRC(9f411940) SHA1(73b0bd360a76ab183f8c7b41f78e930e49e2600c) )
	ROM_LOAD( "kk_14.rom",  0x40000, 0x10000, CRC(9bad9e25) SHA1(0eb1e23dc7084172dd19927a1b084101d10b5137) )
	ROM_LOAD( "kk_15.rom",  0x50000, 0x10000, CRC(89faf590) SHA1(876fc6dac48fef396670522470c41fc9d9b6a0b2) )
	ROM_LOAD( "kk_16.rom",  0x60000, 0x10000, CRC(efb1fb6c) SHA1(12edd64e29472f3c6822f957b23547c64dab65d2) )
	ROM_LOAD( "kk_17.rom",  0x70000, 0x10000, CRC(6b7fb0a5) SHA1(805ee6f439d9e921e1ece27438ba9c00b870e305) )

	ROM_REGION( 0x10000, "ym2", 0 )
	ROM_LOAD( "kk_02.rom",  0x00000, 0x10000, CRC(06169ae0) SHA1(2690ce7cb28cf5c6d37886ce5fbe444067c08403) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16r6b.2c", 0x0000, 0x0104, CRC(311e5ae6) SHA1(8a3799e1611ac4086dda2755c5ad44c0dc16ff5b) )
ROM_END

ROM_START( chopperb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "chpri-1.bin",  0x0000, 0x10000,  CRC(a4e6e978) SHA1(dafc2a3da3725344023a09f5bdaedd0e8e1dbbe2) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "chpri-4.bin",  0x00000, 0x10000, CRC(56d10ba3) SHA1(345a80239fd425c7fe1dfec9385c99a307511e00) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "kk_03.rom",  0x00000, 0x10000, CRC(dbaafb87) SHA1(e7d7f68250bda217230560481ba51da335fc05d7) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "up03_k1.rom",  0x0000, 0x0400, CRC(7f07a45c) SHA1(f751d01d9c25609195b19643395290dab8b8bc5c) ) /* red */
	ROM_LOAD( "up03_l1.rom",  0x0400, 0x0400, CRC(15359fc3) SHA1(4ced674fb18b80ebe5fd6459e0fb9542461dbc74) ) /* green */
	ROM_LOAD( "up03_k2.rom",  0x0800, 0x0400, CRC(79b50f7d) SHA1(41579e498046570a6a74309310b5341fcde9c7de) ) /* blue */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "kk_05.rom",  0x0000, 0x8000, CRC(defc0987) SHA1(ea8eca852aadce90857eb8e65d78631409faac01) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "kk_10.rom",    0x00000, 0x10000, CRC(5cf4d22b) SHA1(b66864740898478becb188d7dd34d61187926e4d) )
	ROM_LOAD( "chpri-11.bin", 0x10000, 0x10000, CRC(881ac259) SHA1(6cce41878c9d9712996d4987a9a578f1301b8feb) )
	ROM_LOAD( "chpri-12.bin", 0x20000, 0x10000, CRC(de96b331) SHA1(725cfe739f7ed0f37eb620d9566bfda1369f4d50) )
	ROM_LOAD( "kk_13.rom",    0x30000, 0x10000, CRC(2756817d) SHA1(acde21454ddf843425deff3357c9e3a7e7a2baec) )

	ROM_REGION( 0x20000, "sp16_tiles", 0 )
	ROM_LOAD( "chpri-9.bin",  0x00000, 0x08000, CRC(106c2dcc) SHA1(919497757664c92e9955db50f5096ac81cec33c3) )
	ROM_LOAD( "chpri-8.bin",  0x08000, 0x08000, CRC(d4f88f62) SHA1(ac89ffa83e0e207acce39711b93d94affc61c1cc) )
	ROM_LOAD( "chpri-7.bin",  0x10000, 0x08000, CRC(28ae39f9) SHA1(7d51489b824b76710f6d4434a77f5f2833fcc532) )
	ROM_LOAD( "chpri-6.bin",  0x18000, 0x08000, CRC(16774a36) SHA1(d1207513f790a30eef8802e63cfeeb10321d6ff7) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "kk_18.rom",  0x00000, 0x10000, CRC(6abbff36) SHA1(8dde3163e454876a4b666b25c81c09b0740143b5) )
	ROM_LOAD( "kk_19.rom",  0x10000, 0x10000, CRC(5283b4d3) SHA1(980f74d3f468203cf9c1a5f3bc256139975035f3) )
	ROM_LOAD( "kk_20.rom",  0x20000, 0x10000, CRC(6403ddf2) SHA1(54a044d9a1ba89fec3bea0e771f75fcc532e7aad) )
	ROM_LOAD( "kk_21.rom",  0x30000, 0x10000, CRC(9f411940) SHA1(73b0bd360a76ab183f8c7b41f78e930e49e2600c) )
	ROM_LOAD( "kk_14.rom",  0x40000, 0x10000, CRC(9bad9e25) SHA1(0eb1e23dc7084172dd19927a1b084101d10b5137) )
	ROM_LOAD( "kk_15.rom",  0x50000, 0x10000, CRC(89faf590) SHA1(876fc6dac48fef396670522470c41fc9d9b6a0b2) )
	ROM_LOAD( "kk_16.rom",  0x60000, 0x10000, CRC(efb1fb6c) SHA1(12edd64e29472f3c6822f957b23547c64dab65d2) )
	ROM_LOAD( "kk_17.rom",  0x70000, 0x10000, CRC(6b7fb0a5) SHA1(805ee6f439d9e921e1ece27438ba9c00b870e305) )

	ROM_REGION( 0x10000, "ym2", 0 )
	ROM_LOAD( "kk_02.rom",  0x00000, 0x10000, CRC(06169ae0) SHA1(2690ce7cb28cf5c6d37886ce5fbe444067c08403) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16r6b.2c", 0x0000, 0x0104, CRC(311e5ae6) SHA1(8a3799e1611ac4086dda2755c5ad44c0dc16ff5b) )
ROM_END

ROM_START( legofair )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "up03_m4.rom",  0x0000, 0x10000,  CRC(79a485c0) SHA1(bbf51e7321656b6a04223909d4958ceb4892193a) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "up03_m8.rom",  0x00000, 0x10000, CRC(96d3a4d9) SHA1(e23a06e6117eca14b24de2d6fd48f5aa2a26d3bb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "kk_03.rom",  0x00000, 0x10000, CRC(dbaafb87) SHA1(e7d7f68250bda217230560481ba51da335fc05d7) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "up03_k1.rom",  0x0000, 0x0400, CRC(7f07a45c) SHA1(f751d01d9c25609195b19643395290dab8b8bc5c) ) /* red */
	ROM_LOAD( "up03_l1.rom",  0x0400, 0x0400, CRC(15359fc3) SHA1(4ced674fb18b80ebe5fd6459e0fb9542461dbc74) ) /* green */
	ROM_LOAD( "up03_k2.rom",  0x0800, 0x0400, CRC(79b50f7d) SHA1(41579e498046570a6a74309310b5341fcde9c7de) ) /* blue */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "kk_05.rom",  0x0000, 0x8000, CRC(defc0987) SHA1(ea8eca852aadce90857eb8e65d78631409faac01) )

	ROM_REGION( 0x40000, "bg_tiles", 0 )
	ROM_LOAD( "kk_10.rom",  0x00000, 0x10000, CRC(5cf4d22b) SHA1(b66864740898478becb188d7dd34d61187926e4d) )
	ROM_LOAD( "kk_11.rom",  0x10000, 0x10000, CRC(9af4cad0) SHA1(dd8c1a76e6a90661c5442c0a096cb9ffe496d12a) )
	ROM_LOAD( "kk_12.rom",  0x20000, 0x10000, CRC(02fec778) SHA1(477a3e22f913cc7783d6cbfce86f98fea9eaf3ec) )
	ROM_LOAD( "kk_13.rom",  0x30000, 0x10000, CRC(2756817d) SHA1(acde21454ddf843425deff3357c9e3a7e7a2baec) )

	ROM_REGION( 0x20000, "sp16_tiles", 0 )
	ROM_LOAD( "kk_09.rom",  0x00000, 0x08000, CRC(653c4342) SHA1(aacb3a7772dcea4c88f0010b3654f4159cfb6a8b) )
	ROM_LOAD( "kk_08.rom",  0x08000, 0x08000, CRC(2da45894) SHA1(09f1ac544a119c9d3a9eeb0606f35585d35c2d1d) )
	ROM_LOAD( "kk_07.rom",  0x10000, 0x08000, CRC(a0ebebdf) SHA1(83d8a9ba7b7ffd42e50afb017e4d0d40fe3e2739) )
	ROM_LOAD( "kk_06.rom",  0x18000, 0x08000, CRC(284fad9e) SHA1(7bb572d7d5983a514e8381954ac89a720b86e9ba) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "kk_18.rom",  0x00000, 0x10000, CRC(6abbff36) SHA1(8dde3163e454876a4b666b25c81c09b0740143b5) )
	ROM_LOAD( "kk_19.rom",  0x10000, 0x10000, CRC(5283b4d3) SHA1(980f74d3f468203cf9c1a5f3bc256139975035f3) )
	ROM_LOAD( "kk_20.rom",  0x20000, 0x10000, CRC(6403ddf2) SHA1(54a044d9a1ba89fec3bea0e771f75fcc532e7aad) )
	ROM_LOAD( "kk_21.rom",  0x30000, 0x10000, CRC(9f411940) SHA1(73b0bd360a76ab183f8c7b41f78e930e49e2600c) )
	ROM_LOAD( "kk_14.rom",  0x40000, 0x10000, CRC(9bad9e25) SHA1(0eb1e23dc7084172dd19927a1b084101d10b5137) )
	ROM_LOAD( "kk_15.rom",  0x50000, 0x10000, CRC(89faf590) SHA1(876fc6dac48fef396670522470c41fc9d9b6a0b2) )
	ROM_LOAD( "kk_16.rom",  0x60000, 0x10000, CRC(efb1fb6c) SHA1(12edd64e29472f3c6822f957b23547c64dab65d2) )
	ROM_LOAD( "kk_17.rom",  0x70000, 0x10000, CRC(6b7fb0a5) SHA1(805ee6f439d9e921e1ece27438ba9c00b870e305) )

	ROM_REGION( 0x10000, "ym2", 0 )
	ROM_LOAD( "kk_02.rom",  0x00000, 0x10000, CRC(06169ae0) SHA1(2690ce7cb28cf5c6d37886ce5fbe444067c08403) )
ROM_END

/***********************************************************************/

ROM_START( fsoccer )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fs3_ver4.bin",  0x00000, 0x10000, CRC(94c3f918) SHA1(7c8343556d6c3897e72f8b41c6fbdc5c58e78b8c) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "fs1_ver4.bin",  0x00000, 0x10000, CRC(97830108) SHA1(dab241baf8d889c768e1fbe25f1e5059b3cbbab6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fs2.3j",  0x00000, 0x10000, CRC(9ee54ea1) SHA1(4e3bbacaa0e247eb8c4043f394e763817a4f9a28) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "2.8e", 0x000, 0x400, CRC(bf4ac706) SHA1(b5015563d88dbd93ba2838f01b189812958f142b) ) /* red */
	ROM_LOAD( "1.8d", 0x400, 0x400, CRC(1bac8010) SHA1(16854b1b6f3d1be48a247796d65aeb90547099b6) ) /* green */
	ROM_LOAD( "3.9e", 0x800, 0x400, CRC(dbeddb14) SHA1(6053b587a3c8272aefe728a7198a15aa7fb9b2fa) ) /* blue */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "fs13.4n",  0x0000, 0x08000, CRC(0de7b7ad) SHA1(4fa54b2acf83f03d09d16fc054ad6623cafe0f4a) )

	ROM_REGION( 0x50000, "bg_tiles", 0 )
	ROM_LOAD( "fs14.8d",  0x00000, 0x10000, CRC(38c38b40) SHA1(c4580add0946720441f5ef751d0d4a944cd92ad5) )
	ROM_LOAD( "fs15.8e",  0x10000, 0x10000, CRC(a614834f) SHA1(d73930e4bd780915e1b0d7f3fe7cbeaad19c233f) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "fs12.2t", 0x00000, 0x10000, CRC(b2442c30) SHA1(ba9331810659726389494ddc7c94c5a02ba80747) )
	ROM_LOAD( "fs11.2s", 0x10000, 0x10000, CRC(022f3e96) SHA1(57aa423b8f62015566bc3021300ac7e9682ed500) )
	ROM_LOAD( "fs10.2r", 0x20000, 0x10000, CRC(e42864d8) SHA1(fe18f58e5507676780fe181e2fb0e0e9d72e276e) )
	ROM_LOAD( "fs9.2p",  0x30000, 0x10000, CRC(d8112aa6) SHA1(575dd6dff2f00901603768f2c121eb0ea5afa444) )
	ROM_LOAD( "fs8.2n",  0x40000, 0x10000, CRC(11156a7d) SHA1(f298a54fa4c118bf8e7c7cccb6c95a4b97daf4d4) )
	ROM_LOAD( "fs7.2l",  0x50000, 0x10000, CRC(d584964b) SHA1(7c806fc40dcce700ed0c268abbd2704938b65ff2) )
	ROM_LOAD( "fs6.2k",  0x60000, 0x10000, CRC(588d14b3) SHA1(c0489b061503677a38e4c5800ea8be17aabf4039) )
	ROM_LOAD( "fs5.2j",  0x70000, 0x10000, CRC(def2f1d8) SHA1(b72e4dec3306d8afe461ac812b2de67ee85f9dd9) )

	ROM_REGION( 0x10000, "ym2", 0 )
	ROM_LOAD( "fs4.bin",  0x00000, 0x10000, CRC(435c3716) SHA1(42053741f60594e7ae8516b3ba600f5badb3620f) )
ROM_END

ROM_START( fsoccerj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fs3.6c",  0x00000, 0x10000, CRC(c5f505fa) SHA1(bc54a6482029735c7ec1d6dd819cad6bac32ac20) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "fs1.2c",  0x00000, 0x10000, CRC(2f68e38b) SHA1(0cbf2de24a5a5ae2134eb6f1e1404691554192bc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fs2.3j",  0x00000, 0x10000, CRC(9ee54ea1) SHA1(4e3bbacaa0e247eb8c4043f394e763817a4f9a28) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "2.8e", 0x000, 0x400, CRC(bf4ac706) SHA1(b5015563d88dbd93ba2838f01b189812958f142b) ) /* red */
	ROM_LOAD( "1.8d", 0x400, 0x400, CRC(1bac8010) SHA1(16854b1b6f3d1be48a247796d65aeb90547099b6) ) /* green */
	ROM_LOAD( "3.9e", 0x800, 0x400, CRC(dbeddb14) SHA1(6053b587a3c8272aefe728a7198a15aa7fb9b2fa) ) /* blue */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "fs13.4n",  0x0000, 0x08000, CRC(0de7b7ad) SHA1(4fa54b2acf83f03d09d16fc054ad6623cafe0f4a) )

	ROM_REGION( 0x50000, "bg_tiles", 0 )
	ROM_LOAD( "fs14.8d",  0x00000, 0x10000, CRC(38c38b40) SHA1(c4580add0946720441f5ef751d0d4a944cd92ad5) )
	ROM_LOAD( "fs15.8e",  0x10000, 0x10000, CRC(a614834f) SHA1(d73930e4bd780915e1b0d7f3fe7cbeaad19c233f) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "fs12.2t", 0x00000, 0x10000, CRC(b2442c30) SHA1(ba9331810659726389494ddc7c94c5a02ba80747) )
	ROM_LOAD( "fs11.2s", 0x10000, 0x10000, CRC(022f3e96) SHA1(57aa423b8f62015566bc3021300ac7e9682ed500) )
	ROM_LOAD( "fs10.2r", 0x20000, 0x10000, CRC(e42864d8) SHA1(fe18f58e5507676780fe181e2fb0e0e9d72e276e) )
	ROM_LOAD( "fs9.2p",  0x30000, 0x10000, CRC(d8112aa6) SHA1(575dd6dff2f00901603768f2c121eb0ea5afa444) )
	ROM_LOAD( "fs8.2n",  0x40000, 0x10000, CRC(11156a7d) SHA1(f298a54fa4c118bf8e7c7cccb6c95a4b97daf4d4) )
	ROM_LOAD( "fs7.2l",  0x50000, 0x10000, CRC(d584964b) SHA1(7c806fc40dcce700ed0c268abbd2704938b65ff2) )
	ROM_LOAD( "fs6.2k",  0x60000, 0x10000, CRC(588d14b3) SHA1(c0489b061503677a38e4c5800ea8be17aabf4039) )
	ROM_LOAD( "fs5.2j",  0x70000, 0x10000, CRC(def2f1d8) SHA1(b72e4dec3306d8afe461ac812b2de67ee85f9dd9) )

	ROM_REGION( 0x10000, "ym2", 0 )
	ROM_LOAD( "fs4.7p",  0x00000, 0x10000, CRC(435c3716) SHA1(42053741f60594e7ae8516b3ba600f5badb3620f) )
ROM_END

ROM_START( fsoccerb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ft-003.bin",  0x00000, 0x10000, CRC(649d4448) SHA1(876a4cf3ce3211ee19390deb17a661ec52b419d2) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "ft-001.bin",  0x00000, 0x10000, CRC(2f68e38b) SHA1(0cbf2de24a5a5ae2134eb6f1e1404691554192bc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fs2.3j",  0x00000, 0x10000, CRC(9ee54ea1) SHA1(4e3bbacaa0e247eb8c4043f394e763817a4f9a28) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "2.8e", 0x000, 0x400, CRC(bf4ac706) SHA1(b5015563d88dbd93ba2838f01b189812958f142b) ) /* red */
	ROM_LOAD( "1.8d", 0x400, 0x400, CRC(1bac8010) SHA1(16854b1b6f3d1be48a247796d65aeb90547099b6) ) /* green */
	ROM_LOAD( "3.9e", 0x800, 0x400, CRC(dbeddb14) SHA1(6053b587a3c8272aefe728a7198a15aa7fb9b2fa) ) /* blue */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "fs13.4n",  0x0000, 0x08000, CRC(0de7b7ad) SHA1(4fa54b2acf83f03d09d16fc054ad6623cafe0f4a) )

	ROM_REGION( 0x50000, "bg_tiles", 0 )
	ROM_LOAD( "fs14.8d",  0x00000, 0x10000, CRC(38c38b40) SHA1(c4580add0946720441f5ef751d0d4a944cd92ad5) )
	ROM_LOAD( "fs15.8e",  0x10000, 0x10000, CRC(a614834f) SHA1(d73930e4bd780915e1b0d7f3fe7cbeaad19c233f) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "fs12.2t", 0x00000, 0x10000, CRC(b2442c30) SHA1(ba9331810659726389494ddc7c94c5a02ba80747) )
	ROM_LOAD( "fs11.2s", 0x10000, 0x10000, CRC(022f3e96) SHA1(57aa423b8f62015566bc3021300ac7e9682ed500) )
	ROM_LOAD( "fs10.2r", 0x20000, 0x10000, CRC(e42864d8) SHA1(fe18f58e5507676780fe181e2fb0e0e9d72e276e) )
	ROM_LOAD( "fs9.2p",  0x30000, 0x10000, CRC(d8112aa6) SHA1(575dd6dff2f00901603768f2c121eb0ea5afa444) )
	ROM_LOAD( "fs8.2n",  0x40000, 0x10000, CRC(11156a7d) SHA1(f298a54fa4c118bf8e7c7cccb6c95a4b97daf4d4) )
	ROM_LOAD( "fs7.2l",  0x50000, 0x10000, CRC(d584964b) SHA1(7c806fc40dcce700ed0c268abbd2704938b65ff2) )
	ROM_LOAD( "fs6.2k",  0x60000, 0x10000, CRC(588d14b3) SHA1(c0489b061503677a38e4c5800ea8be17aabf4039) )
	ROM_LOAD( "fs5.2j",  0x70000, 0x10000, CRC(def2f1d8) SHA1(b72e4dec3306d8afe461ac812b2de67ee85f9dd9) )

	ROM_REGION( 0x10000, "ym2", 0 )
	ROM_LOAD( "fs4.bin",  0x00000, 0x10000, CRC(435c3716) SHA1(42053741f60594e7ae8516b3ba600f5badb3620f) )
ROM_END

ROM_START( fsoccerba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fs3.c6", 0x00000, 0x10000, CRC(e644d207) SHA1(efd5a6cf99461a0dc6cec6c8e2c16d82c6630132) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "fs1_ver4.bin",  0x00000, 0x10000, CRC(97830108) SHA1(dab241baf8d889c768e1fbe25f1e5059b3cbbab6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fs2.3j",  0x00000, 0x10000, CRC(9ee54ea1) SHA1(4e3bbacaa0e247eb8c4043f394e763817a4f9a28) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "2.8e", 0x000, 0x400, CRC(bf4ac706) SHA1(b5015563d88dbd93ba2838f01b189812958f142b) ) /* red */
	ROM_LOAD( "1.8d", 0x400, 0x400, CRC(1bac8010) SHA1(16854b1b6f3d1be48a247796d65aeb90547099b6) ) /* green */
	ROM_LOAD( "3.9e", 0x800, 0x400, CRC(dbeddb14) SHA1(6053b587a3c8272aefe728a7198a15aa7fb9b2fa) ) /* blue */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "fs13.4n",  0x0000, 0x08000, CRC(0de7b7ad) SHA1(4fa54b2acf83f03d09d16fc054ad6623cafe0f4a) )

	ROM_REGION( 0x50000, "bg_tiles", 0 )
	ROM_LOAD( "fs14.8d",  0x00000, 0x10000, CRC(38c38b40) SHA1(c4580add0946720441f5ef751d0d4a944cd92ad5) )
	ROM_LOAD( "fs15.8e",  0x10000, 0x10000, CRC(a614834f) SHA1(d73930e4bd780915e1b0d7f3fe7cbeaad19c233f) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "fs12.2t", 0x00000, 0x10000, CRC(b2442c30) SHA1(ba9331810659726389494ddc7c94c5a02ba80747) )
	ROM_LOAD( "fs11.2s", 0x10000, 0x10000, CRC(022f3e96) SHA1(57aa423b8f62015566bc3021300ac7e9682ed500) )
	ROM_LOAD( "fs10.2r", 0x20000, 0x10000, CRC(e42864d8) SHA1(fe18f58e5507676780fe181e2fb0e0e9d72e276e) )
	ROM_LOAD( "fs9.2p",  0x30000, 0x10000, CRC(d8112aa6) SHA1(575dd6dff2f00901603768f2c121eb0ea5afa444) )
	ROM_LOAD( "fs8.2n",  0x40000, 0x10000, CRC(11156a7d) SHA1(f298a54fa4c118bf8e7c7cccb6c95a4b97daf4d4) )
	ROM_LOAD( "fs7.2l",  0x50000, 0x10000, CRC(d584964b) SHA1(7c806fc40dcce700ed0c268abbd2704938b65ff2) )
	ROM_LOAD( "fs6.2k",  0x60000, 0x10000, CRC(588d14b3) SHA1(c0489b061503677a38e4c5800ea8be17aabf4039) )
	ROM_LOAD( "fs5.2j",  0x70000, 0x10000, CRC(def2f1d8) SHA1(b72e4dec3306d8afe461ac812b2de67ee85f9dd9) )

	ROM_REGION( 0x10000, "ym2", 0 )
	ROM_LOAD( "fs4.7p",  0x00000, 0x10000, CRC(435c3716) SHA1(42053741f60594e7ae8516b3ba600f5badb3620f) )
ROM_END

/***********************************************************************/

ROM_START( tdfever )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "td2-ver3u.6c",  0x0000, 0x10000,  CRC(92138fe4) SHA1(17a2bc12f516cdbea3cc5e283b0a8a2d101dfa47) ) /* Red "U" stamped on label */

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "td1-ver3u.2c",  0x00000, 0x10000, CRC(798711f5) SHA1(a67d6b71c08df00592cf1a18806ed1c2ee757066) ) /* Red "U" stamped on label */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "td3-ver2u.3j",  0x00000, 0x10000, CRC(5d13e0b1) SHA1(a8d8d7cbc4f5be1c0bf10bceff54104d421758c2) ) /* Red "U" stamped on label */

	ROM_REGION( 0x0c00, "proms", 0 ) /* Red "T" stamped on label for each prom */
	ROM_LOAD( "2t.8e",  0x000, 0x00400, CRC(67bdf8a0) SHA1(7a0dc9bf56d607516638d38761aa99211d536d9f) ) /* Silkscreened as MB7122E (R) */
	ROM_LOAD( "1t.8d",  0x400, 0x00400, CRC(9c4a9198) SHA1(2d9be23c6a622eba5d3fb0d9912bad03658e563b) ) /* Silkscreened as MB7122E (G) */
	ROM_LOAD( "3t.9e",  0x800, 0x00400, CRC(c93c18e8) SHA1(9d4ca20c44bd35aabccab5f94cb45057361ccd99) ) /* Silkscreened as MB7122E (B) */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "td14-u.4n",  0x0000, 0x8000,  CRC(e841bf1a) SHA1(ba93163b00e973eb5da9ddc64becce2bbe9ede05) ) /* Red "U" stamped on label */

	ROM_REGION( 0x50000, "bg_tiles", 0 )
	ROM_LOAD( "td15.8d",       0x00000, 0x10000, CRC(ad6e0927) SHA1(dd1c346fbf908af7b3e314f416937f48ade6af4c) )
	ROM_LOAD( "td16.8e",       0x10000, 0x10000, CRC(181db036) SHA1(2c5ed172950fce1467517490a8ab3b7ac6594121) )
	ROM_LOAD( "td17.8f",       0x20000, 0x10000, CRC(c5decca3) SHA1(12aff8adc0ad2bf903122ad065d182692d32fb7a) )
	ROM_LOAD( "td18-ver2u.8g", 0x30000, 0x10000, CRC(3924da37) SHA1(6100eb438fb090f74639739ddcc2844f5daa7180) ) /* Red "U" stamped on label */
	ROM_LOAD( "td19.8j",       0x40000, 0x10000, CRC(bc17ea7f) SHA1(5c3fe43c7fc204d33b5b2a71f22da00e2ba7fbdf) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "td13.2t",   0x00000, 0x10000, CRC(88e2e819) SHA1(6d5529792dbd2ba63a1bc470e9d3ea63b876cfd8) )
	ROM_LOAD( "td12-1.2s", 0x10000, 0x10000, CRC(f6f83d63) SHA1(15780a2c1fc7c8456fe073c372f2f4828125e800) ) /* Blue "1" stamped on label */
	ROM_LOAD( "td11.2r",   0x20000, 0x10000, CRC(a0d53fbd) SHA1(a49f29b3f07ec833651aa0e37b0e87f3f72e0e3a) )
	ROM_LOAD( "td10-1.20", 0x30000, 0x10000, CRC(c8c71c7b) SHA1(7988e9e86c2dfebb0f1b5a8c42c97993a530e780) ) /* Blue "1" stamped on label */
	ROM_LOAD( "td9.2n",    0x40000, 0x10000, CRC(a8979657) SHA1(ec2f61a24b04437a9abd0a306923ae2aeee3eba9) )
	ROM_LOAD( "td8-1.2l",  0x50000, 0x10000, CRC(28f49182) SHA1(3ee06d7d1bac8719d2b05613a7ffc1bc82ddcdae) ) /* Blue "1" stamped on label */
	ROM_LOAD( "td7.2k",    0x60000, 0x10000, CRC(72a5590d) SHA1(d8bd664702af9c66a2bda756d8417d1b69b0cab8) )
	ROM_LOAD( "td6-1.2j",  0x70000, 0x10000, CRC(9b6d4053) SHA1(3d91358b08ed648f48369147441d77a7528d3356) ) /* Blue "1" stamped on label */

	ROM_REGION( 0x20000, "ym2", 0 )
	ROM_LOAD( "td5.7p",  0x00000, 0x10000, CRC(04794557) SHA1(94f476e88b089ad98a133e7356fd271601119fdf) )
	ROM_LOAD( "td4.7n",  0x10000, 0x10000, CRC(155e472e) SHA1(722b4625e6ab796e129daf903386b5b6b1a945cd) )
ROM_END

ROM_START( tdfeverj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "up02_c6.rom",  0x0000, 0x10000,  CRC(88d88ec4) SHA1(774de920290b5c787b0f3d0076883dda106364be) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "up02_c2.rom",  0x00000, 0x10000, CRC(191e6442) SHA1(6a4d0d7efea734443eef538e99562ce4e2949a84) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "up02_j3.rom",  0x00000, 0x10000, CRC(4e4d71c7) SHA1(93744c7d4822ab1750a50ab895a83f77dfcb4bb3) )

	ROM_REGION( 0x0c00, "proms", 0 ) /* Red "T" stamped on label for each prom */
	ROM_LOAD( "2t.8e",  0x000, 0x00400, CRC(67bdf8a0) SHA1(7a0dc9bf56d607516638d38761aa99211d536d9f) ) /* Silkscreened as MB7122E (R) */
	ROM_LOAD( "1t.8d",  0x400, 0x00400, CRC(9c4a9198) SHA1(2d9be23c6a622eba5d3fb0d9912bad03658e563b) ) /* Silkscreened as MB7122E (G) */
	ROM_LOAD( "3t.9e",  0x800, 0x00400, CRC(c93c18e8) SHA1(9d4ca20c44bd35aabccab5f94cb45057361ccd99) ) /* Silkscreened as MB7122E (B) */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "td14.4n",  0x0000, 0x8000,  CRC(af9bced5) SHA1(ec8b9c0649d33e4b0ed4f7d84530016581370278) )

	ROM_REGION( 0x50000, "bg_tiles", 0 )
	ROM_LOAD( "td15.8d", 0x00000, 0x10000, CRC(ad6e0927) SHA1(dd1c346fbf908af7b3e314f416937f48ade6af4c) )
	ROM_LOAD( "td16.8e", 0x10000, 0x10000, CRC(181db036) SHA1(2c5ed172950fce1467517490a8ab3b7ac6594121) )
	ROM_LOAD( "td17.8f", 0x20000, 0x10000, CRC(c5decca3) SHA1(12aff8adc0ad2bf903122ad065d182692d32fb7a) )
	ROM_LOAD( "up01.8g", 0x30000, 0x10000, CRC(4512cdfb) SHA1(f9e57804801962e85fdd3412e6e3774e75160535) )
	ROM_LOAD( "td19.8j", 0x40000, 0x10000, CRC(bc17ea7f) SHA1(5c3fe43c7fc204d33b5b2a71f22da00e2ba7fbdf) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "td13.2t",   0x00000, 0x10000, CRC(88e2e819) SHA1(6d5529792dbd2ba63a1bc470e9d3ea63b876cfd8) )
	ROM_LOAD( "td12-1.2s", 0x10000, 0x10000, CRC(f6f83d63) SHA1(15780a2c1fc7c8456fe073c372f2f4828125e800) ) /* Blue "1" stamped on label */
	ROM_LOAD( "td11.2r",   0x20000, 0x10000, CRC(a0d53fbd) SHA1(a49f29b3f07ec833651aa0e37b0e87f3f72e0e3a) )
	ROM_LOAD( "td10-1.20", 0x30000, 0x10000, CRC(c8c71c7b) SHA1(7988e9e86c2dfebb0f1b5a8c42c97993a530e780) ) /* Blue "1" stamped on label */
	ROM_LOAD( "td9.2n",    0x40000, 0x10000, CRC(a8979657) SHA1(ec2f61a24b04437a9abd0a306923ae2aeee3eba9) )
	ROM_LOAD( "td8-1.2l",  0x50000, 0x10000, CRC(28f49182) SHA1(3ee06d7d1bac8719d2b05613a7ffc1bc82ddcdae) ) /* Blue "1" stamped on label */
	ROM_LOAD( "td7.2k",    0x60000, 0x10000, CRC(72a5590d) SHA1(d8bd664702af9c66a2bda756d8417d1b69b0cab8) )
	ROM_LOAD( "td6-1.2j",  0x70000, 0x10000, CRC(9b6d4053) SHA1(3d91358b08ed648f48369147441d77a7528d3356) ) /* Blue "1" stamped on label */

	ROM_REGION( 0x20000, "ym2", 0 )
	ROM_LOAD( "td5.7p",  0x00000, 0x10000, CRC(04794557) SHA1(94f476e88b089ad98a133e7356fd271601119fdf) )
	ROM_LOAD( "td4.7n",  0x10000, 0x10000, CRC(155e472e) SHA1(722b4625e6ab796e129daf903386b5b6b1a945cd) )
ROM_END

ROM_START( tdfever2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "td02.6c", 0x0000,  0x10000, CRC(9e3eaed8) SHA1(4a591767b22a46605747740a1e1de9aada2893fe) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "td01.1c", 0x00000, 0x10000, CRC(0ec294c0) SHA1(b16656e5fef1c78310f20633d25cda6d6018bf52) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "td03.2j", 0x00000, 0x10000, CRC(4092f16c) SHA1(0821a8afc91862e95e742546367724a862fc6c9f) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "up03-2.8e", 0x000, 0x00400, CRC(1593c302) SHA1(46008b03c76547d57e3c8658f5f00321c2463cd5) ) /* red */
	ROM_LOAD( "up03-2.8d", 0x400, 0x00400, CRC(ac9df947) SHA1(214855e1015f7b519e336159c6ea62ab1f576353) ) /* green */
	ROM_LOAD( "up03-2.9e", 0x800, 0x00400, CRC(73cdf192) SHA1(63d1aa1b00035bbfe5bebd9bc9992a5d6f5abd10) ) /* blue */

	ROM_REGION( 0x8000, "tx_tiles", 0 )
	ROM_LOAD( "td06.3n", 0x0000,  0x8000,  CRC(d6521b0d) SHA1(79aba420b2f039d580892fa34de5d63be1a4f222) )

	ROM_REGION( 0x60000, "bg_tiles", 0 )
	ROM_LOAD( "td15.8d",   0x00000, 0x10000, CRC(ad6e0927) SHA1(dd1c346fbf908af7b3e314f416937f48ade6af4c) )
	ROM_LOAD( "td16.8e",   0x10000, 0x10000, CRC(181db036) SHA1(2c5ed172950fce1467517490a8ab3b7ac6594121) )
	ROM_LOAD( "td17.8f",   0x20000, 0x10000, CRC(c5decca3) SHA1(12aff8adc0ad2bf903122ad065d182692d32fb7a) )
	ROM_LOAD( "td18.8g",   0x30000, 0x10000, CRC(1a5a2200) SHA1(178f3850fd23d888a3e7d14f44cba3426a16bc94) )
	ROM_LOAD( "td19-2.8j", 0x40000, 0x10000, CRC(f1081329) SHA1(efcc210d50923a8c9125227c741ba4b71cd9f688) )
	ROM_LOAD( "td20.8k",   0x50000, 0x10000, CRC(86cbb2e6) SHA1(77ecd6eefc7bb1933374ecd21a5b46798bdbb94d) )

	ROM_REGION( 0x80000, "sp32_tiles", 0 )
	ROM_LOAD( "td14.2t", 0x00000, 0x10000, CRC(88e2e819) SHA1(6d5529792dbd2ba63a1bc470e9d3ea63b876cfd8) )
	ROM_LOAD( "td13.2s", 0x10000, 0x10000, CRC(c9bb9138) SHA1(955101e343e643320b29a29116bea556a25d696f) )
	ROM_LOAD( "td12.2r", 0x20000, 0x10000, CRC(a0d53fbd) SHA1(a49f29b3f07ec833651aa0e37b0e87f3f72e0e3a) )
	ROM_LOAD( "td11.2p", 0x30000, 0x10000, CRC(d43abc81) SHA1(8d635dfaa7a99863f133cf599b99f2a6afcfc8a6) )
	ROM_LOAD( "td10.2n", 0x40000, 0x10000, CRC(a8979657) SHA1(ec2f61a24b04437a9abd0a306923ae2aeee3eba9) )
	ROM_LOAD( "td09.2l", 0x50000, 0x10000, CRC(c93b6cd3) SHA1(e528d62e998f5682b497e864818c1b50ba314944) )
	ROM_LOAD( "td08.2k", 0x60000, 0x10000, CRC(72a5590d) SHA1(d8bd664702af9c66a2bda756d8417d1b69b0cab8) )
	ROM_LOAD( "td07.2j", 0x70000, 0x10000, CRC(4845e78b) SHA1(360df759a761f28df93250f3a2e4e9366d627240) )

	ROM_REGION( 0x40000, "ym2", 0 )
	ROM_LOAD( "td05.7p", 0x00000, 0x10000, CRC(e332e41f) SHA1(3fe41e35c5abbd8f8b9cff91bf85815275c62776) )
	ROM_LOAD( "td04.7n", 0x10000, 0x10000, CRC(98af6d2d) SHA1(0f41f53d4143ec54b8e84cd480e3ab34c3e7ea20) )
	ROM_LOAD( "td22.7l", 0x20000, 0x10000, CRC(34b4bce9) SHA1(bf9b000995dcbb27450c0ad1a8ef1bcc4feee080) )
	ROM_LOAD( "td21.7k", 0x30000, 0x10000, CRC(f5a96d8e) SHA1(33bb2c41426449179fc27ee88b2c8db27b4ed1da) )
ROM_END

/***********************************************************************/

DRIVER_INIT_MEMBER(snk_state,countryc)
{
	// replace coin counter with trackball select
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xc300, 0xc300, write8_delegate(FUNC(snk_state::countryc_trackball_w),this));
}



GAME( 1983, marvins,  0,        marvins,  marvins, driver_device,  0,        ROT270, "SNK", "Marvin's Maze", 0 )
GAME( 1984, vangrd2,  0,        vangrd2,  vangrd2, driver_device,  0,        ROT270, "SNK", "Vanguard II", 0 )
GAME( 1984, madcrash, 0,        vangrd2,  madcrash, driver_device, 0,        ROT0,   "SNK", "Mad Crasher", 0 )
GAME( 1984, madcrush, madcrash, madcrush, madcrash, driver_device, 0,        ROT0,   "SNK", "Mad Crusher (Japan)", 0 )

GAME( 1984, jcross,   0,        jcross,   jcross, driver_device,   0,        ROT270, "SNK", "Jumping Cross", 0 )
GAME( 1984, sgladiat, 0,        sgladiat, sgladiat, driver_device, 0,        ROT0,   "SNK", "Gladiator 1984", 0 )
GAME( 1985, hal21,    0,        hal21,    hal21, driver_device,    0,        ROT270, "SNK", "HAL21", 0 )
GAME( 1985, hal21j,   hal21,    hal21,    hal21, driver_device,    0,        ROT270, "SNK", "HAL21 (Japan)", 0 )

GAME( 1985, aso,      0,        aso,      aso, driver_device,      0,        ROT270, "SNK", "ASO - Armored Scrum Object", 0 )
GAME( 1985, alphamis, aso,      aso,      alphamis, driver_device, 0,        ROT270, "SNK", "Alpha Mission", 0 )
GAME( 1985, arian,    aso,      aso,      alphamis, driver_device, 0,        ROT270, "SNK", "Arian Mission", 0 )
GAME( 1985, tnk3,     0,        tnk3,     tnk3, driver_device,     0,        ROT270, "SNK", "T.N.K III (US)", 0 )
GAME( 1985, tnk3j,    tnk3,     tnk3,     tnk3, driver_device,     0,        ROT270, "SNK", "T.A.N.K (Japan)", 0 )
GAME( 1986, athena,   0,        athena,   athena, driver_device,   0,        ROT0,   "SNK", "Athena", 0 )
GAME( 1988, fitegolf, 0,        fitegolf, fitegolf, driver_device, 0,        ROT0,   "SNK", "Fighting Golf (World?)", 0 )
GAME( 1988, fitegolfu,fitegolf, fitegolf, fitegolfu, driver_device,0,        ROT0,   "SNK", "Fighting Golf (US)", 0 )
GAME( 1988, countryc, 0,        fitegolf, countryc, snk_state, countryc,     ROT0,   "SNK", "Country Club", 0 )

GAME( 1986, ikari,    0,        ikari,    ikari, driver_device,    0,        ROT270, "SNK", "Ikari Warriors (US JAMMA)", 0 ) // distributed by Tradewest(?)
GAME( 1986, ikaria,   ikari,    ikari,    ikaria, driver_device,   0,        ROT270, "SNK", "Ikari Warriors (US)", 0 ) // distributed by Tradewest(?)
GAME( 1986, ikarinc,  ikari,    ikari,    ikarinc, driver_device,  0,        ROT270, "SNK", "Ikari Warriors (US No Continues)", 0 ) // distributed by Tradewest(?)
GAME( 1986, ikarijp,  ikari,    ikari,    ikarinc, driver_device,  0,        ROT270, "SNK", "Ikari (Japan No Continues)", 0 )
GAME( 1986, ikarijpb, ikari,    ikari,    ikarijpb, driver_device, 0,        ROT270, "bootleg", "Ikari (Joystick hack bootleg)", 0 )
GAME( 1986, victroad, 0,        victroad, victroad, driver_device, 0,        ROT270, "SNK", "Victory Road", 0 )
GAME( 1986, dogosoke, victroad, victroad, victroad, driver_device, 0,        ROT270, "SNK", "Dogou Souken", 0 )
GAME( 1986, dogosokb, victroad, victroad, dogosokb, driver_device, 0,        ROT270, "bootleg", "Dogou Souken (Joystick hack bootleg)", 0 )

GAME( 1987, bermudat, 0,        bermudat, bermudat, driver_device, 0,        ROT270, "SNK", "Bermuda Triangle (World?)", 0 )
GAME( 1987, bermudatj,bermudat, bermudat, bermudat, driver_device, 0,        ROT270, "SNK", "Bermuda Triangle (Japan)", 0 )
GAME( 1987, worldwar, 0,        bermudat, worldwar, driver_device, 0,        ROT270, "SNK", "World Wars (World?)", 0 )
GAME( 1987, bermudata,worldwar, bermudat, bermudaa, driver_device, 0,        ROT270, "SNK", "Bermuda Triangle (World Wars) (US)", 0 )
GAME( 1987, psychos,  0,        psychos,  psychos, driver_device,  0,        ROT0,   "SNK", "Psycho Soldier (US)", 0 )
GAME( 1987, psychosj, psychos,  psychos,  psychos, driver_device,  0,        ROT0,   "SNK", "Psycho Soldier (Japan)", 0 )
GAME( 1987, gwar,     0,        gwar,     gwar, driver_device,     0,        ROT270, "SNK", "Guerrilla War (US)", 0 )
GAME( 1987, gwarj,    gwar,     gwar,     gwar, driver_device,     0,        ROT270, "SNK", "Guevara (Japan)", 0 )
GAME( 1987, gwara,    gwar,     gwara,    gwar, driver_device,     0,        ROT270, "SNK", "Guerrilla War (Version 1)", 0 )
GAME( 1987, gwarb,    gwar,     gwar,     gwarb, driver_device,    0,        ROT270, "bootleg", "Guerrilla War (Joystick hack bootleg)", 0 )
GAME( 1988, chopper,  0,        chopper1, chopper, driver_device,  0,        ROT270, "SNK", "Chopper I (US set 1)", 0 )
GAME( 1988, choppera, chopper,  choppera, choppera, driver_device, 0,        ROT270, "SNK", "Chopper I (US set 2)", 0 )
GAME( 1988, chopperb, chopper,  chopper1, chopper, driver_device,  0,        ROT270, "SNK", "Chopper I (US set 3)", 0 )
GAME( 1988, legofair, chopper,  chopper1, chopper, driver_device,  0,        ROT270, "SNK", "Koukuu Kihei Monogatari - The Legend of Air Cavalry (Japan)", 0 )

GAME( 1987, tdfever,  0,        tdfever,  tdfever, driver_device,  0,        ROT90,  "SNK", "TouchDown Fever (US)", 0 )
GAME( 1987, tdfeverj, tdfever,  tdfever,  tdfever, driver_device,  0,        ROT90,  "SNK", "TouchDown Fever (Japan)", 0 )
GAME( 1988, tdfever2, tdfever,  tdfever2, tdfever, driver_device,  0,        ROT90,  "SNK", "TouchDown Fever 2", 0 ) /* upgrade kit for Touchdown Fever */
GAME( 1988, fsoccer,  0,        tdfever2, fsoccer, driver_device,  0,        ROT0,   "SNK", "Fighting Soccer (version 4)", 0 )
GAME( 1988, fsoccerj, fsoccer,  tdfever2, fsoccer, driver_device,  0,        ROT0,   "SNK", "Fighting Soccer (Japan)", 0 )
GAME( 1988, fsoccerb, fsoccer,  tdfever2, fsoccerb, driver_device, 0,        ROT0,   "bootleg", "Fighting Soccer (Joystick hack bootleg)", 0 )
GAME( 1988, fsoccerba,fsoccer,  tdfever2, fsoccerb, driver_device, 0,        ROT0,   "bootleg", "Fighting Soccer (Joystick hack bootleg, alt)", 0 )
