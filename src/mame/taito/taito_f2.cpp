// license:BSD-3-Clause
// copyright-holders:David Graves, Bryan McPhail, Brad Oliver, Andrew Prime, Brian Troha, Nicola Salmoria
// thanks-to:Richard Bush
/***************************************************************************

Taito F2 System

driver by David Graves, Bryan McPhail, Brad Oliver, Andrew Prime, Brian
Troha, Nicola Salmoria with some initial help from Richard Bush

The Taito F2 system is a fairly flexible hardware platform. The main board
supports three 64x64 tiled scrolling background planes of 8x8 tiles, and a
powerful sprite engine capable of handling all the video chores by itself
(used in e.g. Super Space Invaders). The front tilemap has characters which
are generated in RAM for maximum versatility (fading effects etc.).
The expansion board can have additional gfx chip e.g. for a zooming/rotating
tilemap, or additional tilemap planes.

Sound is handled by a Z80 with a YM2610 connected to it.

The memory map for each of the games is similar but shuffled around.

Notes:
- Metal Black has secret command to select stage.
  Start the machine with holding service switch.
  Then push 1p start, 1p start, 1p start, service SW, 1p start
  while error message is displayed.

(This also works in many of the other games. Use in Don Doko Don to play
 an extra set of fifty levels.)


Custom chips
------------
The old version of the F2 main board (larger) has
TC0100SCN (tilemaps)
TC0200OBJ+TC0210FBC (sprites)
TC0140SYT (sound communication & other stuff)

The new version has
TC0100SCN (tilemaps)
TC0540OBN+TC0520TBC (sprites)
TC0530SYC (sound communication & other stuff)

            I/O    Priority / Palette      Additional gfx                 Other
         --------- ------------------- ----------------------- ----------------------------
finalb   TC0220IOC TC0110PCR TC0070RGB
dondokod TC0220IOC TC0360PRI TC0260DAR TC0280GRD(x2)(zoom/rot)
megab    TC0220IOC TC0360PRI TC0260DAR                         TC0030CMD(C-Chip protection)
thundfox TC0220IOC TC0360PRI TC0260DAR TC0100SCN (so it has two)
cameltry TC0220IOC TC0360PRI TC0260DAR TC0280GRD(x2)(zoom/rot)
qtorimon TC0220IOC TC0110PCR TC0070RGB
liquidk  TC0220IOC TC0360PRI TC0260DAR
quizhq   TMP82C265 TC0110PCR TC0070RGB
ssi      TC0510NIO           TC0260DAR
gunfront TC0510NIO TC0360PRI TC0260DAR
growl    TMP82C265 TC0360PRI TC0260DAR                         TC0190FMC(sprite banking?)
mjnquest           TC0110PCR TC0070RGB
footchmp TE7750    TC0360PRI TC0260DAR TC0480SCP(tilemaps)     TC0190FMC(sprite banking?)
koshien  TC0510NIO TC0360PRI TC0260DAR
yuyugogo TC0510NIO           TC0260DAR
ninjak   TE7750    TC0360PRI TC0260DAR                         TC0190FMC(sprite banking?)
solfigtr TMP82C265 TC0360PRI TC0260DAR                         TC0190FMC(sprite banking?)
qzquest  TC0510NIO           TC0260DAR
pulirula TC0510NIO TC0360PRI TC0260DAR TC0430GRW(zoom/rot)
metalb   TC0510NIO TC0360PRI TC0260DAR TC0480SCP(tilemaps)
qzchikyu TC0510NIO           TC0260DAR
yesnoj   TMP82C265           TC0260DAR                         TC8521AP(RTC)
deadconx TE7750    TC0360PRI TC0260DAR TC0480SCP(tilemaps)     TC0190FMC(sprite banking?)
dinorex  TC0510NIO TC0360PRI TC0260DAR
qjinsei  TC0510NIO TC0360PRI TC0260DAR
qcrayon  TC0510NIO TC0360PRI TC0260DAR
qcrayon2 TC0510NIO TC0360PRI TC0260DAR
driftout TC0510NIO TC0360PRI TC0260DAR TC0430GRW(zoom/rot)

A complete list of Taito Games can be still accessed with archive.org:
http://web.archive.org/web/20010502131808/www.taito.co.jp/game-history/

F2 Motherboard ( Big ) K1100432A, J1100183A
               (Small) K1100608A, J1100242A

Apr.1989 Final Blow (B82, M4300123A, K1100433A)
Jul.1989 Don Doko Don (B95, M4300131A, K1100454A, J1100195A)
Oct.1989 Mega Blast (C11)
Feb.1990 Quiz Torimonochou (C41, K1100554A)
Apr.1990 Cameltry (C38, M4300167A, K1100556A)
Jul.1990 Quiz H.Q. (C53, K1100594A)
Aug.1990 Thunder Fox (C28, M4300181A, K1100580A) (exists in F1 version too)
Sep.1990 Liquid Kids/Mizubaku Daibouken (C49, K1100593A)
Nov.1990 MJ-12/Super Space Invaders (C64, M4300195A, K1100616A, J1100248A)
Jan.1991 Gun Frontier (C71, M4300199A, K1100625A, K1100629A(overseas))
Feb.1991 Growl/Runark (C74, M4300210A, K1100639A)
Mar.1991 Hat Trick Hero/Euro Football Championship (C80, K11J0646A)
Mar.1991 Yuu-yu no Quiz de Go!Go! (C83, K11J0652A)
Apr.1991 Ah Eikou no Koshien (C81, M43J0214A, K11J654A)
Apr.1991 Ninja Kids (C85, M43J0217A, K11J0659A)
May.1991 Mahjong Quest (C77, K1100637A, K1100637B)
Jul.1991 Quiz Quest (C92, K11J0678A)
Sep.1991 Metal Black (D12)
Oct.1991 Drift Out (Visco) (M43X0241A, K11X0695A)
Nov.1991 PuLiRuLa (C98, M43J0225A, K11J0672A)
Feb.1992 Quiz Chikyu Boueigun (D19, K11J0705A)
Jul.1992 Dead Connection (D28, K11J0715A)
Nov.1992 Dinorex (D39, K11J0254A)
Mar.1993 Quiz Jinsei Gekijou (D48, M43J0262A, K11J0742A)
Aug.1993 Quiz Crayon Shinchan (D55, K11J0758A)
Dec.1993 Crayon Shinchan Orato Asobo (D63, M43J0276A, K11J0779A)

Mar.1992 Yes.No. Shinri Tokimeki Chart (Fortune teller machine) (D20, K11J0706B)

Thunder Fox, Drift Out, "Quiz Crayon Shinchan", and "Crayon Shinchan
Orato Asobo" has "Not F2" version PCB.
Foreign version of Cameltry uses different hardware (B89's PLD,
K1100573A, K1100574A).




Sprite extension area types
===========================

These games need a special value for f2_spriteext:

Yuyugogo = 1
Pulirula = 2
Dinorex = 3
Quiz Crayon 1&2 = 3
Quiz Jinsei = 3
(all other games need it to be zero)

TODO Lists
==========

- The sprite system is still partly a mystery, and not an accurate emulation.
  A lot of sprite glitches are caused by data in sprite ram not being correct,
  part from one frame and part from the previous one. There has to be some
  buffering inside the chip but it's not clear how. See below the irq section
  for a long list of observations on sprite glitches.

  Other limitations include: misplaced tile of the zooming title in Qcrayon
  (the one on the yellow background in attract); sprites when you get a home
  run in Koshien are often out on x axis by 1 pixel.

- TC0480SCP emulation (footchmp, metalb, deadconx) has slight inaccuracies.
  Zoomed layers and zoomed pixel rows are not precisely positioned.

- DIPS, still many unknown

- Restored save states on some games tend to hang.

- All non-quiz games except Solfigtr have 2 country sets dumped: if 1 byte diff
  then create the third set.


Don Doko Don
------------

Roz layer is one pixel out vertically when screen flipped.


Cameltry (cameltrya)
--------

Alt version with YM2203 and M6295
Sound frequencies may be incorrect


Gun Frontier
------------

There are mask sprites used on the waterfall in the first round
of attract demo, however it's not clear what they should mask since
there don't seem to be sprites below them. Shadow maybe?
(BM161104 - Fixed)


Pulirula
--------

In level 3, the mask sprites used for the door are misaligned by one pixel to
the left.
(BM100705 - Not a bug - various alignment problems seem to be confirmed from a real pcb).
Shadows appear to have some kind of flicker effect on real pcb - not emulated/understood.

Metal Black
-----------

Tilemap screenflip support has an issue: blue planet early in attract
should be 1 pixel left.

Sprite emulation issues may be responsible for minor glitches on the
"bolts" on round 4 boss ship: some sprite/tilemap lag creeps in.

Missing two blend effects: there's a sun sprite underneath tilemaps
in round 1; and the boss sprite crosses under the tilemaps at start
of round 5 finale.
(BM161104 - Fixed)


Yesnoj
------

Input mapping incomplete. There's a 0x01 one which only seems to be
used in printer [printer test?] mode. It seems to be a printer status
input. With the value currently returned, it sounds an alarm and says
[Japanese trans.] "Error detected on the printer. Call machine operator."
Update (2008.10.09) Printer shall now be correct but it's still not clear
how the inputs are physically connected and what the are supposed to do.

[Coin lockout/ctr?]

Calendar / Time in "test mode" always reset to default settings.


Quiz Crayon 2
-------------

There should be a highlight circle around the player while it moves on the
map. This is done by a sprite which doesn't have priority over the
background. This is probably the same thing as the waterfall in Gun Frontier.
(BM161104 - Fixed)

Driftout
--------

Sprites are 1 pixel too far right in screenflip.
Roz layer is around 4 pixels too far down in screenflip.


PCB Layout
----------

K11T0658A
NINJA KIDS
|----------------------------------------------|
|PAL PAL  C85-12*.49  C85-15*.50   DSWA  DSWB  |
|68000P12 C85-06.49   C85-07.50  OSC1  TE7750  |
|                                              |
|           84256      84256     C85-03.65     |
|      MB3771                                  |
| TC0190FMC                 TC0100SCN TC51832  |
|C85-02.19  84256      84256          TC51832 J|
|C85-01.17                        HM3-65764KS A|
|                                             M|
|TC51832 TC51832              TC0360PRI       M|
|TC51832 TC51832   TC0540OBN       TC0260DAR  A|
|TC51832 TC51832                               |
|TC51832 TC51832   TC0520TBC                   |
|TC51832 TC51832                OSC2  Z80A     |
|TC51832 TC51832       TC0530SYC               |
|TC51832 TC51832 C85-05.33    C85-14.54        |
|                C85-04.31 YM2610 YM3016       |
|----------------------------------------------|

Notes:
      OSC1 = 26.686MHz
      OSC2 = 24.000MHz
      68000 clock: 12.000MHz (24 / 2)
        Z80 clock: 4.000MHz  (24 / 6)
     YM2610 clock: 8.000MHz  (24 / 3)
            Vsync: 60Hz

***************************************************************************/

#include "emu.h"
#include "taito_f2.h"
#include "taitoipt.h"
#include "taitosnd.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/rp5c01.h"
#include "machine/te7750.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"
#include "speaker.h"


/**********************************************************
                        GAME INPUTS
**********************************************************/

void taitof2_state::coin_nibble_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0,  data & 0x04);
	machine().bookkeeping().coin_counter_w(1,  data & 0x08);
}

void taitof2_state::growl_coin_word_w(u8 data)/* what about coins 3&4 ?? */
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0,  data & 0x04);
	machine().bookkeeping().coin_counter_w(1,  data & 0x08);
}

void taitof2_state::_4p_coin_word_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_lockout_w(2, ~data & 0x04);
	machine().bookkeeping().coin_lockout_w(3, ~data & 0x08);
	machine().bookkeeping().coin_counter_w(0,  data & 0x10);
	machine().bookkeeping().coin_counter_w(1,  data & 0x20);
	machine().bookkeeping().coin_counter_w(2,  data & 0x40);
	machine().bookkeeping().coin_counter_w(3,  data & 0x80);
}

u16 taitof2_state::cameltry_paddle_r(offs_t offset)
{
	int curr, res = 0xff;

	switch (offset)
	{
		case 0x00:
			curr = m_io_paddle[0]->read();
			res = curr - m_last[0];
			m_last[0] = curr;
			return res;

		case 0x02:
			curr = m_io_paddle[1]->read();
			res = curr - m_last[1];
			m_last[1] = curr;
			return res;
	}

	logerror("CPU #0 PC %06x: warning - read unmapped paddle offset %06x\n", m_maincpu->pc(), offset);

	return 0;
}

u16 taitof2_state::mjnquest_dsw_r(offs_t offset)
{
	switch (offset)
	{
		case 0x00:
		{
			return (m_io_in[5]->read() << 8) + m_io_dswa->read();   /* DSW A + coin */
		}

		case 0x01:
		{
			return (m_io_in[6]->read() << 8) + m_io_dswb->read();   /* DSW B + coin */
		}
	}

	logerror("CPU #0 PC %06x: warning - read unmapped dsw_r offset %06x\n", m_maincpu->pc(), offset);

	return 0xff;
}

u16 taitof2_state::mjnquest_input_r()
{
	switch (m_mjnquest_input)
	{
		case 0x01:
			return m_io_in[0]->read();

		case 0x02:
			return m_io_in[1]->read();

		case 0x04:
			return m_io_in[2]->read();

		case 0x08:
			return m_io_in[3]->read();

		case 0x10:
			return m_io_in[4]->read();

	}

	logerror("CPU #0 mjnquest_input %06x: warning - read unknown input %06x\n", m_maincpu->pc(), m_mjnquest_input);

	return 0xff;
}

void taitof2_state::mjnquest_inputselect_w(u16 data)
{
	m_mjnquest_input = (data >> 6);
}

/******************************************************************
                       INTERRUPTS (still a WIP)

The are two interrupt request signals: VBL and DMA. DMA comes
from the sprite generator (maybe when it has copied the data to
a private buffer, or rendered the current frame, or who knows what
else).
The requests are mapped through a PAL so no hardwiring, but the PAL
could be the same across all the games. All the games have just two
valid vectors, IRQ5 and IRQ6.

It seems that usually VBL maps to IRQ5 and DMA to IRQ6. However
there are jumpers on the board allowing to swap the two interrupt
request signals, so this could explain a need for certain games to
have them in the opposite order.

There are lots of sprite glitches in many games because the sprite ram
is often updated in two out-of-sync chunks. I am almost sure there is
some partial buffering going on in the sprite chip, and DMA has to
play a part in it.


             sprite ctrl regs         interrupts & sprites
          0006 000a    8006 800a
          ----------------------    -----------------------------------------------
finalb    8000 0300    0000 0000    Needs partial buffering like dondokod to avoid glitches
dondokod  8000 0000/8  0000 0000    IRQ6 just sets a flag. IRQ5 waits for that flag,
                                    toggles ctrl register 0000<->0008, and copies bytes
                                    0 and 8 *ONLY* of sprite data (code, color, flip,
                                    ctrl). The other bytes of sprite data (coordinates
                                    and zoom) are updated by the main program.
                                    Caching sprite data and using bytes 0 and 8 from
                                    previous frame and the others from *TWO* frames
                                    before is enough to get glitch-free sprites that seem
                                    to be perfectly in sync with scrolling (check the tree
                                    mouths during level change).
thundfox  8000 0000    0000 0000    IRQ6 copies bytes 0 and 8 *ONLY* of sprite data (code,
                                    color, flip, ctrl). The other bytes of sprite data
                                    (coordinates and zoom) are updated (I think) by the
                                    main program.
                                    The same sprite data caching that works for dondokod
                                    improves sprites, but there are still glitches related
                                    to zoom (check third round of attract mode). Those
                                    glitches can be fixed by buffering also the zoom ctrl
                                    byte.
                                    Moreover, sprites are not in perfect sync with the
                                    background (sometimes they are one frame behind, but
                                    not always).
qtorimon  8000 0000    0000 0000    IRQ6 does some stuff but doesn't seem to deal with
                                    sprites. IRQ5 copies bytes 0, 8 *AND ALSO 2* of sprite
                                    data in one routine, and immediately after that the
                                    remaining bytes 4 and 6 in another routine, without
                                    doing, it seems, any waiting inbetween.
                                    Nevertheless, separated sprite data caching like in
                                    dondokod is still required to avoid glitches.
liquidk   8000 0000/8  0000 0000    Same as dondokod. An important difference is that
                                    the sprite ctrl register doesn't toggle every frame
                                    (because the handler can't complete the frame in
                                    time?). This can be seen easily in the attract mode,
                                    where sprite glitches appear.
                                    Correctly handling the ctrl register and sprite data
                                    caching seems to be vital to avoid sprite glitches.
quizhq    8000 0000    0000 0000    Both IRQ5 and IRQ6 do stuff, I haven't investigated.
                                    There is a very subtle sprite glitch if sprite data
                                    buffering is not used: the blinking INSERT COIN in
                                    the right window will get moved as garbage chars on
                                    the left window score and STOCK for one frame when
                                    INSERT COINS disappears from the right. This happens
                                    because bytes 0 and 8 of the sprite data are one
                                    frame behind and haven't been cleared yet.
ssi       8000 0000    0000 0000    IRQ6 does nothing. IRQ5 copies bytes 0 and 8 *ONLY*
                                    of sprite data (code, color, flip, ctrl). The other
                                    bytes of sprite data (coordinates and zoom) are
                                    updated by the main program.
                                    The same sprite data caching that works for dondokod
                                    avoids major glitches, but I'm not sure it's working
                                    right when the big butterfly (time bonus) is on
                                    screen (it flickers on and off every frame).
gunfront  8000 1000/1  8001 1000/1  The toggling bit in the control register selects the
                                    sprite bank used. It normally toggles every frame but
                                    sticks for two frame when lots of action is going on
                                    (see smart bombs in attract mode) and glitches will
                                    appear if it is not respected.
                                    IRQ6 writes the sprite ctrl registers, and also writes
                                    related data to the sprites at 9033e0/90b3e0. The
                                    active one gets 8000/8001 in byte 6 and 1001/1000 in
                                    byte 10, while the other gets 0. Note that the value
                                    in byte 10 is inverted from the active bank, as if it
                                    were a way to tell the sprite hardware "after this, go
                                    to the other bank".
                                    Note also that IRQ6 isn't the only one writing to the
                                    sprite ctrl registers, this is done also in the parts
                                    that actually change the sprite data (I think it's
                                    main program, not interrupt), so it's not clear who's
                                    "in charge". Actually it seems that what IRQ6 writes
                                    is soon overwritten so that what I outlined above
                                    regarding 9033e0/90b3e0 is no longer true, and they
                                    are no longer in sync with the ctrl registers, messing
                                    up smart bombs.
                                    There don't seem to be other glitches even without
                                    sprite data buffering.
growl     8000 0000    8001 0001    IRQ6 just sets a flag. I haven't investigated who
                                    updates the sprite ram.
                                    This game uses sprite banks like gunfront, but unlike
                                    gunfront it doesn't change the ctrl registers. What it
                                    does is change the sprites at 903210/90b210; 8000/8001
                                    is always written in byte 6, while byte 10 receives
                                    the active bank (1000 or 1001). There are also end of
                                    list markers placed before that though, and those seem
                                    to always match what's stored in the ctrl registers
                                    (8000 1000 for the first bank and 8001 1001 for the
                                    second).
                                    There don't seem to be sprite glitches even without
                                    sprite data buffering, but sprites are not in sync with
                                    the background.
mjnquest  8000 0800/8  0000 0000
footchmp  8000 0000    8001 0001    IRQ6 just sets a flag (and writes to an unknown memory
                                    location).
                                    This games uses sprite banks as well, this time it
                                    writes markers at 2033e0/20b3e0, it always writes
                                    1000/1001 to byte 10, while it writes 8000 or 8001 to
                                    byte 6 depending on the active bank. This is the exact
                                    opposite of growl...
hthero
koshien   8000 0000    8001 0001    Another game using banks.The markers are again at
                                    9033e0/90b3e0 but this time byte 6 receives 9000/9001.
                                    Byte 10 is 1000 or 1001 depending on the active bank.
yuyugogo  8000 0800/8  0000 0000
ninjak    8000 0000    8001 0001    uses banks
solfigtr  8000 0000    8001 0001    uses banks
qzquest   8000 0000    0000 0000    Separated sprite data caching like in dondokod is
                                    required to avoid glitches.
pulirula  8000 0000    8001 0001    uses banks
qzchikyu  8000 0000    0000 0000    With this game there are glitches and the sprite data
                                    caching done in dondokod does NOT fix them.
deadconx 8/9000 0000/1 8/9001 0000/1 I guess it's not a surprise that this game uses banks
                                    in yet another different way.
dinorex   8000 0000    8001 0001    uses banks
driftout  8000 0000/8  0000 0000    The first control changes from 8000 to 0000 at the end
                                    of the attract demo and seems to stay that way.


******************************************************************/

TIMER_CALLBACK_MEMBER(taitof2_state::trigger_int6)
{
	m_maincpu->set_input_line(6, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(taitof2_state::interrupt)
{
	m_int6_timer->adjust(m_maincpu->cycles_to_attotime(500));
	device.execute().set_input_line(5, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(taitof2_state::megab_interrupt)
{
	interrupt(device);
	m_cchip->ext_interrupt(ASSERT_LINE);
	m_cchip_irq_clear->adjust(attotime::zero);
}

TIMER_DEVICE_CALLBACK_MEMBER(taitof2_state::cchip_irq_clear_cb)
{
	m_cchip->ext_interrupt(CLEAR_LINE);
}


/****************************************************************
                            SOUND
****************************************************************/

void taitof2_state::sound_bankswitch_w(u8 data)
{
	m_audiobank->set_entry(data & 7);

#ifdef MAME_DEBUG
	if ((data & 7) > 2)
		logerror("CPU #1 switch to ROM bank %06x: should only happen if Z80 prg rom is 128K!\n",data & 7);
#endif
}


u8 taitof2_state::driveout_sound_command_r()
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
//  logerror("sound IRQ OFF (sound command=%02x)\n", m_driveout_sound_latch);
	return m_driveout_sound_latch;
}


void taitof2_state::oki_bank_w(u8 data)
{
	if (data & 4)
	{
		m_okibank->set_entry((data & 3));
	}
}

void taitof2_state::driveout_sound_command_w(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		m_nibble = data & 1;
	}
	else
	{
		if (m_nibble == 0)
		{
			m_driveout_sound_latch = (data & 0x0f) | (m_driveout_sound_latch & 0xf0);
		}
		else
		{
			m_driveout_sound_latch = ((data << 4) & 0xf0) | (m_driveout_sound_latch & 0x0f);
			m_audiocpu->set_input_line(0, ASSERT_LINE);
		}
	}
}


/***********************************************************
                     MEMORY STRUCTURES
***********************************************************/

void taitof2_state::finalb_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x200007).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::word_w));    /* palette */
	map(0x300000, 0x30000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x320001, 0x320001).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x320003, 0x320003).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x810000, 0x81ffff).nopw();   /* error in game init code ? */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xb00002, 0xb00003).nopw();   /* ?? */
}

void taitof2_state::dondokod_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x201fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x320000, 0x320000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x320002, 0x320002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xa00000, 0xa01fff).rw(m_tc0280grd, FUNC(tc0280grd_device::tc0280grd_word_r), FUNC(tc0280grd_device::tc0280grd_word_w));    /* ROZ tilemap */
	map(0xa02000, 0xa0200f).w(m_tc0280grd, FUNC(tc0280grd_device::tc0280grd_ctrl_word_w));
	map(0xb00000, 0xb0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* ?? */
}

void taitof2_state::megab_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x100000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x100002, 0x100002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x120000, 0x12000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x180000, 0x1807ff).rw(m_cchip, FUNC(taito_cchip_device::mem68_r), FUNC(taito_cchip_device::mem68_w)).umask16(0x00ff);
	map(0x180800, 0x180fff).rw(m_cchip, FUNC(taito_cchip_device::asic_r), FUNC(taito_cchip_device::asic68_w)).umask16(0x00ff);
	map(0x200000, 0x20ffff).ram();
	map(0x300000, 0x301fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x40001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* ?? */
	map(0x600000, 0x60ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x610000, 0x61ffff).ram();   /* unused? */
	map(0x620000, 0x62000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x800000, 0x80ffff).ram().share("spriteram");
}

void taitof2_state::thundfox_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x101fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x200000, 0x20000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x220000, 0x220000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x220002, 0x220002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x300000, 0x30ffff).ram();
	map(0x400000, 0x40ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));  /* tilemaps */
	map(0x420000, 0x42000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x500000, 0x50ffff).rw(m_tc0100scn[1], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));  /* tilemaps */
	map(0x520000, 0x52000f).rw(m_tc0100scn[1], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x600000, 0x60ffff).ram().share("spriteram");
	map(0x800000, 0x80001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0xff00);
}

void taitof2_state::cameltry_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x201fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x300018, 0x30001f).r(FUNC(taitof2_state::cameltry_paddle_r));
	map(0x320000, 0x320000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x320002, 0x320002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x813fff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xa00000, 0xa01fff).rw(m_tc0280grd, FUNC(tc0280grd_device::tc0280grd_word_r), FUNC(tc0280grd_device::tc0280grd_word_w));    /* ROZ tilemap */
	map(0xa02000, 0xa0200f).w(m_tc0280grd, FUNC(tc0280grd_device::tc0280grd_ctrl_word_w));
	map(0xd00000, 0xd0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* ?? */
}

void taitof2_state::driftoutct_map(address_map &map)
{
	cameltry_map(map);

	map(0x040000, 0x07ffff).rom();
	map(0x300018, 0x30001f).unmapr();
	map(0x300018, 0x300019).portr("PADDLE1");
	map(0x30001a, 0x30001b).portr("PADDLE2");
}

void taitof2_state::cameltrya_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x201fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x300018, 0x30001f).r(FUNC(taitof2_state::cameltry_paddle_r));
	map(0x320000, 0x320000).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0x320002, 0x320002).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0x800000, 0x813fff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xa00000, 0xa01fff).rw(m_tc0280grd, FUNC(tc0280grd_device::tc0280grd_word_r), FUNC(tc0280grd_device::tc0280grd_word_w));    /* ROZ tilemap */
	map(0xa02000, 0xa0200f).w(m_tc0280grd, FUNC(tc0280grd_device::tc0280grd_ctrl_word_w));
	map(0xd00000, 0xd0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* ?? */
}

void taitof2_state::qtorimon_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x200007).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::word_w));    /* palette */
	map(0x500000, 0x50000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x600000, 0x600000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x600002, 0x600002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0x910000, 0x9120ff).nopw();   /* error in init code ? */
}

void taitof2_state::liquidk_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x201fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x320001, 0x320001).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x320003, 0x320003).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xb00000, 0xb0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* ?? */
}

void taitof2_state::quizhq_map(address_map &map)
{
	map(0x000000, 0x0bffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x200007).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::word_w));    /* palette */
	map(0x500005, 0x500005).w(FUNC(taitof2_state::growl_coin_word_w));
	map(0x500000, 0x500001).portr("DSWB");
	map(0x500002, 0x500003).portr("IN0");
	map(0x580000, 0x580001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* ??? */
	map(0x580006, 0x580007).nopw();   /* ??? */
	map(0x580000, 0x580001).portr("DSWA");
	map(0x580002, 0x580003).portr("IN1");
	map(0x580004, 0x580005).portr("IN2");
	map(0x600001, 0x600001).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x600003, 0x600003).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x680000, 0x680001).nopw();   /* ??? */
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x810000, 0x81ffff).nopw();   /* error in init code ? */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
}

void taitof2_state::ssi_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_r), FUNC(tc0510nio_device::halfword_w));
	map(0x200000, 0x20ffff).ram();
	map(0x300000, 0x301fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x400000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x400002, 0x400002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
//  map(0x500000, 0x500001).nopw();   /* ?? */
	map(0x600000, 0x60ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps (not used) */
	map(0x620000, 0x62000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x800000, 0x80ffff).ram().share("spriteram");   /* sprite ram */
}

void taitof2_state::gunfront_map(address_map &map)
{
	map(0x000000, 0x0bffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x201fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_wordswap_r), FUNC(tc0510nio_device::halfword_wordswap_w));
	map(0x320000, 0x320000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x320002, 0x320002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
//  map(0xa00000, 0xa00001).nopw();   /* ?? */
	map(0xb00000, 0xb0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* ?? */
}

void taitof2_state::growl_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x201fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300005, 0x300005).w(FUNC(taitof2_state::growl_coin_word_w));
	map(0x300000, 0x300001).portr("DSWA");
	map(0x300002, 0x300003).portr("DSWB");
	map(0x320000, 0x320001).portr("IN0");
	map(0x320002, 0x320003).portr("IN1");
	map(0x320004, 0x320005).portr("IN2");
	map(0x340000, 0x340001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x400000, 0x400000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x400002, 0x400002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x500000, 0x50000f).w(FUNC(taitof2_state::spritebank_w));
	map(0x504000, 0x504001).nopw();    /* unknown... various values */
	map(0x508000, 0x50800f).portr("IN3");
	map(0x50c000, 0x50c00f).portr("IN4");
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xb00000, 0xb0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* ?? */
}

void taitof2_state::mjnquest_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x110000, 0x11ffff).ram();   /* "sram" */
	map(0x120000, 0x12ffff).ram();
	map(0x200000, 0x200007).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::word_w));    /* palette */
	map(0x300000, 0x30000f).r(FUNC(taitof2_state::mjnquest_dsw_r));
	map(0x310000, 0x310001).r(FUNC(taitof2_state::mjnquest_input_r));
	map(0x320000, 0x320001).w(FUNC(taitof2_state::mjnquest_inputselect_w));
	map(0x330000, 0x330001).nopw();   /* watchdog ? */
	map(0x350000, 0x350001).nopw();   /* watchdog ? */
	map(0x360000, 0x360000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x360002, 0x360002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x380001, 0x380001).w(FUNC(taitof2_state::mjnquest_gfxbank_w));   /* scr gfx bank select */
	map(0x400000, 0x40ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x420000, 0x42000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x500000, 0x50ffff).ram().share("spriteram");
}

void taitof2_state::footchmp_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x20ffff).ram().share("spriteram");
	map(0x300000, 0x30000f).w(FUNC(taitof2_state::spritebank_w)); /* updated at $a6e, off irq5 */
	map(0x400000, 0x40ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));     /* tilemaps */
	map(0x430000, 0x43002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0x500000, 0x50001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* 500002 written like a watchdog?! */
	map(0x600000, 0x601fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x70001f).rw("te7750", FUNC(te7750_device::read), FUNC(te7750_device::write)).umask16(0x00ff);
	map(0x800000, 0x800001).rw("watchdog", FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));   /* ??? */
	map(0xa00001, 0xa00001).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0xa00003, 0xa00003).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
}

void taitof2_state::koshien_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x201fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_r), FUNC(tc0510nio_device::halfword_w));
	map(0x320000, 0x320000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x320002, 0x320002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xa20000, 0xa20001).w(FUNC(taitof2_state::koshien_spritebank_w));
	map(0xb00000, 0xb0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0xff00);
}

void taitof2_state::yuyugogo_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x200000, 0x20000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_r), FUNC(tc0510nio_device::halfword_w));
	map(0x400000, 0x400000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x400002, 0x400002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xa00000, 0xa01fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xb00000, 0xb10fff).ram();   /* deliberate writes to $b10xxx, I think */
	map(0xc00000, 0xc01fff).w(FUNC(taitof2_state::sprite_extension_w)).share("sprite_ext");
	map(0xd00000, 0xdfffff).rom().region("extra", 0);
}

void taitof2_state::ninjak_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x201fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30001f).rw("te7750", FUNC(te7750_device::read), FUNC(te7750_device::write)).umask16(0xff00);
	map(0x380000, 0x380001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* ??? */
	map(0x400000, 0x400000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x400002, 0x400002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x600000, 0x60000f).w(FUNC(taitof2_state::spritebank_w));
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xb00000, 0xb0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* b00002 written like a watchdog?! */
}

void taitof2_state::solfigtr_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x201fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300005, 0x300005).w(FUNC(taitof2_state::growl_coin_word_w));    /* NOT VERIFIED */
	map(0x300000, 0x300001).portr("DSWA");
	map(0x300002, 0x300003).portr("DSWB");
	map(0x320000, 0x320001).portr("IN0");
	map(0x320002, 0x320003).portr("IN1");
	map(0x320004, 0x320005).portr("IN2");
	map(0x340000, 0x340001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* NOT VERIFIED */
	map(0x400000, 0x400000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x400002, 0x400002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x500000, 0x50000f).w(FUNC(taitof2_state::spritebank_w));
	map(0x504000, 0x504001).nopw();    /* unknown... various values */
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xb00000, 0xb0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* ?? */
}

void taitof2_state::qzquest_map(address_map &map)
{
	map(0x000000, 0x17ffff).rom();
	map(0x200000, 0x20000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_r), FUNC(tc0510nio_device::halfword_w));
	map(0x300001, 0x300001).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x300003, 0x300003).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x400000, 0x401fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x50ffff).ram();
	map(0x600000, 0x60ffff).ram().share("spriteram");
	map(0x700000, 0x70ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x720000, 0x72000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
}

void taitof2_state::pulirula_map(address_map &map)
{
	map(0x000000, 0x0bffff).rom();
	map(0x200000, 0x200000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x200002, 0x200002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x300000, 0x30ffff).ram();
	map(0x400000, 0x401fff).rw(m_tc0430grw, FUNC(tc0280grd_device::tc0430grw_word_r), FUNC(tc0280grd_device::tc0430grw_word_w));    /* ROZ tilemap */
	map(0x402000, 0x40200f).w(m_tc0430grw, FUNC(tc0280grd_device::tc0430grw_ctrl_word_w));
//  map(0x500000, 0x500001).nopw();   /* ??? */
	map(0x600000, 0x603fff).w(FUNC(taitof2_state::sprite_extension_w)).share("sprite_ext");
	map(0x700000, 0x701fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xa00000, 0xa0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0xff00);
	map(0xb00000, 0xb0000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_r), FUNC(tc0510nio_device::halfword_w));
}

void taitof2_state::metalb_map(address_map &map)
{
	map(0x000000, 0x0bffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x300000, 0x30ffff).ram().share("spriteram");
//  map(0x42000c, 0x42000f).nopw();   /* zeroed */
	map(0x500000, 0x50ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));     /* tilemaps */
	map(0x530000, 0x53002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0x600000, 0x60001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);
	map(0x700000, 0x703fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x800000, 0x80000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_wordswap_r), FUNC(tc0510nio_device::halfword_wordswap_w));
	map(0x900000, 0x900000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x900002, 0x900002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
//  map(0xa00000, 0xa00001).nopw();   /* ??? */
}

void taitof2_state::qzchikyu_map(address_map &map)
{
	map(0x000000, 0x17ffff).rom();
	map(0x200000, 0x20000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_r), FUNC(tc0510nio_device::halfword_w));
	map(0x300001, 0x300001).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x300003, 0x300003).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x400000, 0x401fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x50ffff).ram();
	map(0x600000, 0x60ffff).ram().share("spriteram");
	map(0x700000, 0x70ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x720000, 0x72000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
}

void taitof2_state::yesnoj_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20ffff).ram();
	map(0x400000, 0x40ffff).ram().share("spriteram");
	map(0x500000, 0x50ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x520000, 0x52000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x600000, 0x601fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x70001f).rw("rtc", FUNC(tc8521_device::read), FUNC(tc8521_device::write)).umask16(0x00ff);
	map(0x800000, 0x800000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x800002, 0x800002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x900002, 0x900003).nopw();   /* lots of similar writes */
	map(0xa00000, 0xa00001).portr("IN0");
	map(0xa00002, 0xa00003).portr("IN1");
	map(0xa00004, 0xa00005).portr("IN2");
	map(0xb00000, 0xb00001).portr("DSWA");
	map(0xc00000, 0xc00001).nopw();   /* watchdog ?? */
	map(0xd00000, 0xd00001).nopw();   /* lots of similar writes */
}

void taitof2_state::deadconx_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x20ffff).ram().share("spriteram");
	map(0x300000, 0x30000f).w(FUNC(taitof2_state::spritebank_w));
	map(0x400000, 0x40ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));     /* tilemaps */
//    map(0x42000c, 0x42000f).nopw();   /* zeroed */
	map(0x430000, 0x43002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0x500000, 0x50001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* uses 500002 like a watchdog !? */
	map(0x600000, 0x601fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x70001f).rw("te7750", FUNC(te7750_device::read), FUNC(te7750_device::write)).umask16(0x00ff);
	map(0x800000, 0x800001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* ??? */
	map(0xa00000, 0xa00000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0xa00002, 0xa00002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
}

void taitof2_state::dinorex_map(address_map &map)
{
	map(0x000000, 0x2fffff).rom();
	map(0x300000, 0x30000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_r), FUNC(tc0510nio_device::halfword_w));
	map(0x400000, 0x400fff).w(FUNC(taitof2_state::sprite_extension_w)).share("sprite_ext");
	map(0x500000, 0x501fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x600000, 0x60ffff).ram();
	map(0x700000, 0x70001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* ?? */
	map(0x800000, 0x80ffff).ram().share("spriteram");
	map(0x900000, 0x90ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x920000, 0x92000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0xa00000, 0xa00000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0xa00002, 0xa00002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0xb00000, 0xb00001).nopw();   /* watchdog? */
}

void taitof2_state::qjinsei_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x200000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x200002, 0x200002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x300000, 0x30ffff).ram();
	map(0x500000, 0x500001).nopw();   /* watchdog ? */
	map(0x600000, 0x603fff).w(FUNC(taitof2_state::sprite_extension_w)).share("sprite_ext");
	map(0x700000, 0x701fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xa00000, 0xa0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* ?? */
	map(0xb00000, 0xb0000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_r), FUNC(tc0510nio_device::halfword_w));
}

void taitof2_state::qcrayon_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
//  map(0x200000, 0x200001).nopw();   /* unknown */
	map(0x300000, 0x3fffff).rom().region("extra", 0);   /* extra data rom */
	map(0x500000, 0x500000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x500002, 0x500002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x600000, 0x603fff).w(FUNC(taitof2_state::sprite_extension_w)).share("sprite_ext");
	map(0x700000, 0x701fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x800000, 0x80ffff).ram().share("spriteram");
	map(0x900000, 0x90ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x920000, 0x92000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0xa00000, 0xa0000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_r), FUNC(tc0510nio_device::halfword_w));
	map(0xb00000, 0xb0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* ?? */
}

void taitof2_state::qcrayon2_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20ffff).ram();
	map(0x300000, 0x301fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x40ffff).ram().share("spriteram");
	map(0x500000, 0x50ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x520000, 0x52000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x600000, 0x67ffff).rom().region("extra", 0);   /* extra data rom */
	map(0x700000, 0x70000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_r), FUNC(tc0510nio_device::halfword_w));
	map(0x900000, 0x90001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0x00ff);  /* ?? */
	map(0xa00000, 0xa00000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0xa00002, 0xa00002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0xb00000, 0xb017ff).w(FUNC(taitof2_state::sprite_extension_w)).share("sprite_ext");
}

void taitof2_state::driftout_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x200000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x200002, 0x200002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x300000, 0x30ffff).ram();
	map(0x400000, 0x401fff).rw(m_tc0430grw, FUNC(tc0280grd_device::tc0430grw_word_r), FUNC(tc0280grd_device::tc0430grw_word_w));    /* ROZ tilemap */
	map(0x402000, 0x40200f).w(m_tc0430grw, FUNC(tc0280grd_device::tc0430grw_ctrl_word_w));
	map(0x700000, 0x701fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xa00000, 0xa0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0xff00);
	map(0xb00000, 0xb0000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_r), FUNC(tc0510nio_device::halfword_w));
	map(0xb00018, 0xb00019).portr("PADDLE1");
	map(0xb0001a, 0xb0001b).portr("PADDLE2");
}

/* same as driftout, except for sound address 0x200000 */
void taitof2_state::driveout_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x200003).nopr().w(FUNC(taitof2_state::driveout_sound_command_w)).umask16(0xff00);
	map(0x300000, 0x30ffff).ram();
	map(0x400000, 0x401fff).rw(m_tc0430grw, FUNC(tc0280grd_device::tc0430grw_word_r), FUNC(tc0280grd_device::tc0430grw_word_w));    /* ROZ tilemap */
	map(0x402000, 0x40200f).w(m_tc0430grw, FUNC(tc0280grd_device::tc0430grw_ctrl_word_w));
	map(0x700000, 0x701fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x800000, 0x80ffff).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x820000, 0x82000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x900000, 0x90ffff).ram().share("spriteram");
	map(0xa00000, 0xa0001f).w(m_tc0360pri, FUNC(tc0360pri_device::write)).umask16(0xff00);
	map(0xb00000, 0xb0000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_r), FUNC(tc0510nio_device::halfword_w));
	map(0xb00018, 0xb00019).portr("PADDLE1");
	map(0xb0001a, 0xb0001b).portr("PADDLE2");
}


/***************************************************************************/

void taitof2_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("audiobank");
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0xe200, 0xe200).nopr().w("tc0140syt", FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw("tc0140syt", FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xe400, 0xe403).nopw(); /* pan */
	map(0xea00, 0xea00).nopr();
	map(0xee00, 0xee00).nopw(); /* ? */
	map(0xf000, 0xf000).nopw(); /* ? */
	map(0xf200, 0xf200).w(FUNC(taitof2_state::sound_bankswitch_w));   /* ?? */
}


/* Alt version of Cameltry, YM2203 + M6925 sound */

void taitof2_state::cameltrya_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();     // I can't see a bank control, but there ARE some bytes past 0x8000
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x9001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xa000, 0xa000).w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xa001, 0xa001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
//  map(0xb000, 0xb000).w(FUNC(taitof2_state::unknown_w));    // probably controlling sample player?
	map(0xb000, 0xb001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


void taitof2_state::driveout_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).w(FUNC(taitof2_state::oki_bank_w));
	map(0x9800, 0x9800).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(FUNC(taitof2_state::driveout_sound_command_r));
}


void taitof2_state::driveout_oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).bankr("okibank");
	map(0x20000, 0x3ffff).rom().region("oki", 0x80000);
}

/***********************************************************
                     INPUT PORTS, DIPs
***********************************************************/

#define TAITO_F2_SYSTEM_INPUT \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
/* The other bits vary from one game to another, so they are not included here */


static INPUT_PORTS_START( finalb )
	PORT_START("DSWA")
	/* Not sure how to handle alternate controls */
	PORT_DIPNAME( 0x01, 0x01, "Alternate Controls" )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	TAITO_DSWA_BITS_1_TO_3_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	TAITO_JOY_UDLR_3_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_3_BUTTONS_START( 2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT   /* controls below are DIP selectable */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) /* 1P sen.sw.? */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) /* 1P ducking? */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) /* 2P sen.sw.? */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) /* 2P ducking? */
INPUT_PORTS_END

static INPUT_PORTS_START( finalbj )
	PORT_INCLUDE(finalb)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( finalbu )
	PORT_INCLUDE(finalb)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( dondokod )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "10k and 100k" )
	PORT_DIPSETTING(    0x08, "10k and 150k" )
	PORT_DIPSETTING(    0x04, "10k and 250k" )
	PORT_DIPSETTING(    0x00, "10k and 350k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS_START( 2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( dondokodu )
	PORT_INCLUDE(dondokod)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( dondokodj )
	PORT_INCLUDE(dondokod)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( megab )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "100k only" )
	PORT_DIPSETTING(    0x04, "150k only" )
	PORT_DIPSETTING(    0x08, "200k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPNAME( 0x40, 0x40, "Upright Controls" )      PORT_DIPLOCATION("SW2:7") /* ie single or two players at once */
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Dual ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( megabu )
	PORT_INCLUDE(megab)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( megabj )
	PORT_INCLUDE(megab)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( thundfox )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x04, 0x04, "Timer" )         PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Upright Controls" )      PORT_DIPLOCATION("SW2:8") /* ie single or two players at once */
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Dual ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_3_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_3_BUTTONS_START( 2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( thundfoxu )
	PORT_INCLUDE(thundfox)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( thundfoxj )
	PORT_INCLUDE(thundfox)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( cameltry )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_US_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Start remain time" )     PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "35" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x0c, "50" )
	PORT_DIPSETTING(    0x08, "60" )
	PORT_DIPNAME( 0x30, 0x30, "Continue play time" )    PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "+20" )
	PORT_DIPSETTING(    0x10, "+25" )
	PORT_DIPSETTING(    0x30, "+30" )
	PORT_DIPSETTING(    0x20, "+40" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Upright Controls" )      PORT_DIPLOCATION("SW2:8") /* ie single or two players at once */
	PORT_DIPSETTING(    0x80, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PADDLE1")   /* Paddle A */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("PADDLE2")   /* Paddle B */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( cameltryj )
	PORT_INCLUDE(cameltry)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( qtorimon )
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x40, 0x40, "Show Correct Answer" )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( liquidk )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "30k and 100k" )
	PORT_DIPSETTING(    0x08, "30k and 150k" )
	PORT_DIPSETTING(    0x04, "50k and 250k" )
	PORT_DIPSETTING(    0x00, "50k and 350k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Upright Controls" )      PORT_DIPLOCATION("SW2:8") /* ie single or two players at once */
	PORT_DIPSETTING(    0x80, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS_START( 2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( liquidku )
	PORT_INCLUDE(liquidk)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( mizubaku )
	PORT_INCLUDE(liquidk)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( ssi )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Shields" )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x0c, "1")
	PORT_DIPSETTING(    0x04, "2")
	PORT_DIPSETTING(    0x08, "3")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "2")
	PORT_DIPSETTING(    0x10, "3")
	PORT_DIPNAME( 0xa0, 0xa0, "2 Players Mode" )        PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0xa0, "Simultaneous")
	PORT_DIPSETTING(    0x80, "Alternate, Single")
	PORT_DIPSETTING(    0x00, "Alternate, Dual")
	PORT_DIPSETTING(    0x20, "Not Allowed")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS_START( 2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( majest12u )
	PORT_INCLUDE(ssi)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( majest12j )
	PORT_INCLUDE(ssi)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( growl )
	PORT_START("IN0")
	TAITO_JOY_UDLR_3_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_3_BUTTONS_START( 2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPNAME( 0x30, 0x30, "Cabinet Type" )          PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "2 Players" )
	PORT_DIPSETTING(    0x20, "4 Players / 4 Coin Slots" )  // Push Player button A to start
	PORT_DIPSETTING(    0x10, "4 Players / 2 cabinets combined" )
	PORT_DIPSETTING(    0x00, "4 Players / 2 Coin Slots" )
	PORT_DIPNAME( 0x40, 0x40, "Final Boss Continue" )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( growlu )
	PORT_INCLUDE(growl)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( runark )
	PORT_INCLUDE(growl)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( pulirula )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Magic" )     PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
//  PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x80, "Upright Controls" )  PORT_DIPLOCATION("SW2:8") /* ie single or two players at once */
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Dual ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_3_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_3_BUTTONS_START( 2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( pulirulaj )
	PORT_INCLUDE(pulirula)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( qzquest )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)  //??
	PORT_DIPNAME( 0x0c, 0x0c, "Countdown Timer" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "Very Slow" )
	PORT_DIPSETTING(    0x04, "Slow" )
	PORT_DIPSETTING(    0x0c, "Medium" )
	PORT_DIPSETTING(    0x08, "Fast" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( qzchikyu )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)  //??
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( footchmp )
	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS_START( 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, "Game Over Type" )        PORT_DIPLOCATION("SW1:1") // 2p simultaneous play
	PORT_DIPSETTING(    0x01, "Both Teams' Games Over" )
	PORT_DIPSETTING(    0x00, "Losing Team's Game is Over" )
	TAITO_DSWA_BITS_1_TO_3_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Game_Time ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "1.5 Minutes" )
	PORT_DIPSETTING(    0x0c, " 2  Minutes" )
	PORT_DIPSETTING(    0x04, "2.5 Minutes" )
	PORT_DIPSETTING(    0x08, " 3  Minutes" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "2 Players" )
	PORT_DIPSETTING(    0x20, "4 Players / 4 Coin Slots" )  // Push Player button A to start
	PORT_DIPSETTING(    0x10, "4 Players / 2 cabinets combined" )
	PORT_DIPSETTING(    0x00, "4 Players / 2 Coin Slots" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Game Version" )          PORT_DIPLOCATION("SW2:8")   // Not used for Hat Trick Hero / Euro Champ '92
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, "European" )

	PORT_START("IN3")
	TAITO_JOY_UDLR_2_BUTTONS_START( 3 )

	PORT_START("IN4")
	TAITO_JOY_UDLR_2_BUTTONS_START( 4 )
INPUT_PORTS_END

static INPUT_PORTS_START( htherou )
	PORT_INCLUDE(footchmp)

	PORT_MODIFY("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( htheroj )
	PORT_INCLUDE(footchmp)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x80, 0x00, "Game Over Type" )        PORT_DIPLOCATION("SW1:1")   // 2p simultaneous play
	PORT_DIPSETTING(    0x80, "Both Teams' Games Over" )
	PORT_DIPSETTING(    0x00, "Losing Team's Game is Over" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x20, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x00, "1.5 Minutes" )
	PORT_DIPSETTING(    0x30, " 2  Minutes" )
	PORT_DIPSETTING(    0x20, "2.5 Minutes" )
	PORT_DIPSETTING(    0x10, " 3  Minutes" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x0c, "2 Players" )
	PORT_DIPSETTING(    0x04, "4 Players / 4 Coin Slots" )  // Push Player button A to start
	PORT_DIPSETTING(    0x08, "4 Players / 2 cabinets combined" )
	PORT_DIPSETTING(    0x00, "4 Players / 2 Coin Slots" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( footchmpbl )
	PORT_INCLUDE(footchmp)

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // doesn't boot otherwise?
INPUT_PORTS_END


static INPUT_PORTS_START( ninjak )
	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS_START( 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Cabinet Type" )          PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "2 players" )
	PORT_DIPSETTING(    0x08, "TROG (4 players / 2 coin slots)" )
	PORT_DIPSETTING(    0x04, "MTX2 (4 players / 2 cabinets combined)" )
	PORT_DIPSETTING(    0x00, "TMNT (4 players / 4 coin slots)" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Type" )             PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, "1 Player only" )
	PORT_DIPSETTING(    0x80, "Multiplayer" )

	PORT_START("IN3")
	TAITO_JOY_UDLR_2_BUTTONS_START( 3 )

	PORT_START("IN4")
	TAITO_JOY_UDLR_2_BUTTONS_START( 4 )
INPUT_PORTS_END

static INPUT_PORTS_START( ninjaku )
	PORT_INCLUDE(ninjak)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( ninjakj )
	PORT_INCLUDE(ninjak)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( driftout )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Control" )           PORT_DIPLOCATION("SW2:3,4") /* correct acc. to service mode */
	PORT_DIPSETTING(    0x0c, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x08, "Paddle" )
	PORT_DIPSETTING(    0x04, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, "Steering wheel" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* 2P not used? */

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PADDLE1")   /* Paddle A */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("PADDLE2")   /* Paddle B */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( driftoutct )
	PORT_INCLUDE(driftout)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( gunfront )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "10k and every 80k" )
	PORT_DIPSETTING(    0x0c, "20k and every 80k" )
	PORT_DIPSETTING(    0x04, "30k and every 80k" )
	PORT_DIPSETTING(    0x00, "60k and every 80k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Upright Controls" )      PORT_DIPLOCATION("SW2:8") /* ie single or two players at once */
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Dual ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS_START( 2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( gunfrontj )
	PORT_INCLUDE(gunfront)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( metalb )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, "80k and every 160k" )
	PORT_DIPSETTING(    0x0c, "70k and every 150k" )
	PORT_DIPSETTING(    0x00, "100k and every 200k" )
	PORT_DIPSETTING(    0x08, "50k and every 120k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Upright Controls" )      PORT_DIPLOCATION("SW2:8") /* ie single or two players at once */
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Dual ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_3_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_3_BUTTONS_START( 2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( metalbj )
	PORT_INCLUDE(metalb)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( deadconx )
	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS_START( 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_OPTIONAL // input test only
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_OPTIONAL // input test only
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service A") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service B") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service C") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("IN3") // all input test only
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3) PORT_OPTIONAL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3) PORT_OPTIONAL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3) PORT_OPTIONAL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3) PORT_OPTIONAL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_OPTIONAL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_OPTIONAL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 ) PORT_OPTIONAL

	PORT_START("IN4") // all input test only
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4) PORT_OPTIONAL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4) PORT_OPTIONAL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4) PORT_OPTIONAL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4) PORT_OPTIONAL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_OPTIONAL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_OPTIONAL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 ) PORT_OPTIONAL

	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB") /* DSW B, missing a timer speed maybe? */
	PORT_DIPNAME( 0x03, 0x03, "Number of Enemies 1/2 Player" )  PORT_DIPLOCATION("SW2:1,2") // see MT08059
	PORT_DIPSETTING(    0x01, "25/45" )
	PORT_DIPSETTING(    0x03, "30/50" )
	PORT_DIPSETTING(    0x02, "40/60" )
	PORT_DIPSETTING(    0x00, "50/70" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "NOT USE" */
	PORT_DIPNAME( 0x18, 0x18, "Life Meter") PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x08, "12" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "NOT USE" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "NOT USE" */
	PORT_DIPNAME( 0x80, 0x80, "Upright Controls" )  PORT_DIPLOCATION("SW2:8")   // single or two players at once
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Dual ) )
INPUT_PORTS_END

static INPUT_PORTS_START( deadconxj ) /* Matches PDF of Dip Sheet but not matching current taito coin macros */
	PORT_INCLUDE(deadconx)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )        /* Listed as "NOT USE" */
INPUT_PORTS_END


static INPUT_PORTS_START( dinorex )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Damage" )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "Small" )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, "Big" )
	PORT_DIPSETTING(    0x00, "Biggest" )
	PORT_DIPNAME( 0x10, 0x10, "Timer Speed" )   PORT_DIPLOCATION("SW2:5")   // Appears to make little difference
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x20, 0x20, "Match Type" )    PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Best of 3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x80, "Upright Controls" )  PORT_DIPLOCATION("SW2:8")   /* ie single or two players at once */
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Dual ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_3_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_3_BUTTONS_START( 2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( dinorexj )
	PORT_INCLUDE(dinorex)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( dinorexu )
	PORT_INCLUDE(dinorex)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( solfigtr )
	PORT_START("IN0")
	TAITO_JOY_UDLR_3_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_3_BUTTONS_START( 2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */
INPUT_PORTS_END

static INPUT_PORTS_START( koshien )
	PORT_START("DSWA")  /* DSW A, one lets you control fielders ? */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	TAITO_DSWA_BITS_1_TO_3_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x04, 0x04, "Timer" )         PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Upright Controls" )      PORT_DIPLOCATION("SW2:8") /* ie single or two players at once */
	PORT_DIPSETTING(    0x80, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( quizhq )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) // ??
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) // ??
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Time" )          PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "5 seconds" )
	PORT_DIPSETTING(    0x08, "10 seconds" )
	PORT_DIPSETTING(    0x04, "15 seconds" )
	PORT_DIPSETTING(    0x00, "20 seconds" )
	PORT_DIPNAME( 0x30, 0x30, "Stock" )         PORT_DIPLOCATION("SW2:5,6") /* AKA "Lives" */
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Upright Controls" )      PORT_DIPLOCATION("SW2:8") /* ie single or two players at once */
	PORT_DIPSETTING(    0x80, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
INPUT_PORTS_END

static INPUT_PORTS_START( qjinsei )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) // ??
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) // ??
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( qcrayon )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Default Time" )      PORT_DIPLOCATION("SW2:3,4") /* Can be affected ingame by some items and/or player location */
	PORT_DIPSETTING(    0x00, "6 seconds" )
	PORT_DIPSETTING(    0x04, "7 seconds" )
	PORT_DIPSETTING(    0x08, "8 seconds" )
	PORT_DIPSETTING(    0x0c, "10 seconds" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) // ??
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) // ??
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( qcrayon2 )
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )    /* These 2 Dip Switches were designed to change the default timer */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )    /* but the 10 seconds setting is duplicated 4 times in the tables */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x80, "Game Control" )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, "4 Buttons" )

	PORT_START("IN0")
	/* Joystick Control */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)                  PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )                                 PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )                                 PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )                                  PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	/* 4 Buttons Control */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)                  PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)                  PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)                  PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)                  PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )                                  PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )                                 PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )                                 PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )                                 PORT_CONDITION("DSWB",0x80,EQUALS,0x00)

	PORT_START("IN1")
	/* Joystick Control */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)                  PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )                                 PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )                                 PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )                                  PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	/* 4 Buttons Control */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)                  PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)                  PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)                  PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)                  PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )                                  PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )                                 PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )                                 PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )                                 PORT_CONDITION("DSWB",0x80,EQUALS,0x00)

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( yuyugogo )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) // ??
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) // ??
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	TAITO_F2_SYSTEM_INPUT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( mjnquest )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )      // ?
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( yesnoj )
	/* 0xb00000 -> 0x20c0e0.b ($40e0,A5) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, "Print Results" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Printer" )           PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "2 Players Game" )        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
	TAITO_COINAGE_US_COIN_START_LOC(SW1)
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSWB")                                           /* does it physically exist ? */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "P1 Yes" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "P1 No" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME( "P2 Yes" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME( "P2 No" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM )                 /* printer : paper time-out ? */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM )                /* printer : unknown */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM )                /* printer : paper */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM )                 /* printer : unknown */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )                    /* not mapped in "test mode" */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )                   /* not mapped in "test mode" */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***********************************************************
                        GFX DECODING
***********************************************************/

static const gfx_layout layout_6bpp_hi =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ STEP2(0,1) },
	{ STEP4(3*2,-2), STEP4(7*2,-2), STEP4(11*2,-2), STEP4(15*2,-2) },
	{ STEP16(0,16*2) },
	16*16*2
};

static GFXDECODE_START( gfx_taitof2 )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x4_packed_lsb, 0, 256 )   /* sprites */
GFXDECODE_END

static GFXDECODE_START( gfx_finalb )
	GFXDECODE_ENTRY( "sprites",    0, gfx_16x16x4_packed_lsb, 0, 256 )   // low 4bpp of 6bpp sprites
	GFXDECODE_ENTRY( "sprites_hi", 0, layout_6bpp_hi,         0, 256 )   // hi 2bpp of 6bpp sprites
GFXDECODE_END

static const gfx_layout footchmpbl_tilelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,4),
	4,  /* 4 bits per pixel */
	{ RGN_FRAC(0,4), RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( gfx_footchmpbl )
	GFXDECODE_ENTRY( "sprites", 0, footchmpbl_tilelayout, 0, 256 )    /* sprites */
GFXDECODE_END


void taitof2_state::cameltrya_porta_w(u8 data)
{
	// Implement //
}

/***********************************************************
                      MACHINE DRIVERS
***********************************************************/

void taitof2_state::machine_start()
{
	if (m_audiobank)
	{
		const int banks = memregion("audiocpu")->bytes() / 0x4000;
		int i = 0;
		while (i < 8)
		{
			m_audiobank->configure_entry(i, memregion("audiocpu")->base() + (0x4000 * (i % banks))); // TODO : mirrored?
			i++;
		}
	}

	m_int6_timer = timer_alloc(FUNC(taitof2_state::trigger_int6), this);
}

void taitof2_state::taito_f2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24000000/2); /* 12 MHz */
	m_maincpu->set_vblank_int("screen", FUNC(taitof2_state::interrupt));

	Z80(config, m_audiocpu, 24000000/6);   /* 4 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitof2_state::sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));  /* frames per second, vblank duration */
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update));
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_no_buffer));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_taitof2);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 4096);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 24000000/3)); /* Was 16000000/2, but only a 24Mhz OSC */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}

void taitof2_state::taito_f2_tc0220ioc(machine_config &config)
{
	taito_f2(config);

	/* basic machine hardware */
	config.device_remove("watchdog");
	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(taitof2_state::coin_nibble_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");
}

void taitof2_state::taito_f2_tc0510nio(machine_config &config)
{
	taito_f2(config);

	/* basic machine hardware */
	config.device_remove("watchdog");
	TC0510NIO(config, m_tc0510nio, 0);
	m_tc0510nio->read_0_callback().set_ioport("DSWA");
	m_tc0510nio->read_1_callback().set_ioport("DSWB");
	m_tc0510nio->read_2_callback().set_ioport("IN0");
	m_tc0510nio->read_3_callback().set_ioport("IN1");
	m_tc0510nio->write_4_callback().set(FUNC(taitof2_state::coin_nibble_w));
	m_tc0510nio->read_7_callback().set_ioport("IN2");

	m_palette->set_format(palette_device::xRGB_555, 4096);
}

void taitof2_state::taito_f2_te7750(machine_config &config)
{
	taito_f2(config);

	te7750_device &te7750(TE7750(config, "te7750"));
	te7750.in_port1_cb().set_ioport("DSWA");
	te7750.in_port2_cb().set_ioport("DSWB");
	te7750.in_port3_cb().set_ioport("IN2");
	te7750.out_port4_cb().set(FUNC(taitof2_state::_4p_coin_word_w));
	te7750.in_port6_cb().set_ioport("IN0");
	te7750.in_port7_cb().set_ioport("IN1");
	te7750.in_port8_cb().set_ioport("IN3");
	te7750.in_port9_cb().set_ioport("IN4");
}

void taitof2_state::taito_f2_tc0110pcr(machine_config &config)
{
	TC0110PCR(config, m_tc0110pcr, 0);
	m_gfxdecode->set_palette(m_tc0110pcr);
	m_screen->set_palette(m_tc0110pcr);
}

void taitof2_state::finalb(machine_config &config)
{
	taito_f2_tc0220ioc(config);
	taito_f2_tc0110pcr(config);
	config.device_remove("palette");

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::finalb_map);

	/* video hardware */
	m_gfxdecode->set_info(gfx_finalb);
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,finalb)
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_partial_buffer_delayed));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(1, 0);
	m_tc0100scn[0]->set_palette(m_tc0110pcr);
}

void taitof2_state::dondokod(machine_config &config)
{
	taito_f2_tc0220ioc(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::dondokod_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,dondokod)
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_partial_buffer_delayed));
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri_roz));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0280GRD(config, m_tc0280grd, 0);
	m_tc0280grd->set_color_base(0);
	m_tc0280grd->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::megab(machine_config &config)
{
	taito_f2_tc0220ioc(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::megab_map);
	m_maincpu->set_vblank_int("screen", FUNC(taitof2_state::megab_interrupt));

	TAITO_CCHIP(config, m_cchip, 24_MHz_XTAL/2); // 12MHz
	// the ports don't appear to hook up to anything

	TIMER(config, "cchip_irq_clear").configure_generic(FUNC(taitof2_state::cchip_irq_clear_cb));

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,megab)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri));
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_partial_buffer_delayed));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::thundfox(machine_config &config)
{
	taito_f2_tc0220ioc(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::thundfox_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,thundfox)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_thundfox));
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_partial_buffer_delayed_thundfox));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_offsets_flip(5, 0);
	m_tc0100scn[0]->set_offsets_fliptx(4, 1);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0100SCN(config, m_tc0100scn[1], 0);
	m_tc0100scn[1]->set_offsets(3, 0);
	m_tc0100scn[1]->set_offsets_flip(5, 0);
	m_tc0100scn[1]->set_offsets_fliptx(4, 1);
	m_tc0100scn[1]->set_multiscr_xoffs(tc0100scn_device::SINGLE_VDU);
	m_tc0100scn[1]->set_multiscr_hack(1);
	m_tc0100scn[1]->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::cameltry(machine_config &config)
{
	taito_f2_tc0220ioc(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::cameltry_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,dondokod)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri_roz));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0280GRD(config, m_tc0280grd, 0);
	m_tc0280grd->set_color_base(0);
	m_tc0280grd->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::driftoutct(machine_config &config)
{
	cameltry(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::driftoutct_map);
}

void taitof2_state::qtorimon(machine_config &config)
{
	taito_f2_tc0220ioc(config);
	taito_f2_tc0110pcr(config);
	config.device_remove("palette");

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::qtorimon_map);

	/* video hardware */
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_partial_buffer_delayed));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_gfxlayout(TC0100SCN_LAYOUT_1BPP);
	m_tc0100scn[0]->set_palette(m_tc0110pcr);
}

void taitof2_state::liquidk(machine_config &config)
{
	taito_f2_tc0220ioc(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::liquidk_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,megab)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri));
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_partial_buffer_delayed));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::quizhq(machine_config &config)
{
	taito_f2(config);
	taito_f2_tc0110pcr(config);
	config.device_remove("palette");

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::quizhq_map);

	/* video hardware */
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_partial_buffer_delayed));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_gfxlayout(TC0100SCN_LAYOUT_1BPP);
	m_tc0100scn[0]->set_palette(m_tc0110pcr);
}

void taitof2_state::ssi(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::ssi_map);

	m_palette->set_format(palette_device::RGBx_444, 4096);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,ssi)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_ssi));
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_partial_buffer_delayed_thundfox));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);
}

void taitof2_state::gunfront(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::gunfront_map);

	m_palette->set_format(palette_device::RGBx_444, 4096);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,gunfront)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri));
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_partial_buffer_delayed));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::growl(machine_config &config)
{
	taito_f2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::growl_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,growl)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri));

	m_palette->set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::mjnquest(machine_config &config)
{
	taito_f2(config);
	taito_f2_tc0110pcr(config);
	config.device_remove("palette");

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::mjnquest_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,mjnquest)

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_palette(m_tc0110pcr);
	m_tc0100scn[0]->set_tile_callback(FUNC(taitof2_state::mjnquest_tmap_cb));
}

void taitof2_state::footchmp(machine_config &config)
{
	taito_f2_te7750(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::footchmp_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,footchmp)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_deadconx));
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_full_buffer_delayed));

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(0x1d + 3, 0x08);
	m_tc0480scp->set_offsets_tx(-1, 0);
	m_tc0480scp->set_offsets_flip(-1, 0);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::footchmpbl(machine_config &config)
{
	footchmp(config);

	/* video hardware */
	m_tc0480scp->set_gfxlayout(TC0480SCP_LAYOUT_BOOTLEG);
	m_gfxdecode->set_info(gfx_footchmpbl);
}

void taitof2_state::hthero(machine_config &config)
{
	taito_f2_te7750(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::footchmp_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,hthero)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_deadconx));
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_full_buffer_delayed));

	TC0360PRI(config, m_tc0360pri, 0);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(0x33 + 3, -0x04);
	m_tc0480scp->set_offsets_tx(-1, 0);
	m_tc0480scp->set_offsets_flip(-1, 0);
}

void taitof2_state::koshien(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::koshien_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,koshien)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri));

	m_palette->set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(1, 0);
	m_tc0100scn[0]->set_offsets_flip(2, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::yuyugogo(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::yuyugogo_map);

	m_palette->set_format(palette_device::RGBx_444, 4096);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,yuyugogo)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_yesnoj));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_gfxlayout(TC0100SCN_LAYOUT_1BPP);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);
}

void taitof2_state::ninjak(machine_config &config)
{
	taito_f2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::ninjak_map);

	te7750_device &te7750(TE7750(config, "te7750"));
	te7750.in_port1_cb().set_ioport("DSWA");
	te7750.in_port2_cb().set_ioport("DSWB");
	te7750.in_port3_cb().set_ioport("IN0");
	te7750.in_port4_cb().set_ioport("IN1");
	te7750.in_port5_cb().set_ioport("IN3");
	te7750.in_port6_cb().set_ioport("IN4");
	te7750.in_port7_cb().set_ioport("IN2");
	te7750.out_port8_cb().set(FUNC(taitof2_state::_4p_coin_word_w));

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,ninjak)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(1, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::solfigtr(machine_config &config)
{
	taito_f2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::solfigtr_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,solfigtr)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_offsets_flip(6, 0);
	m_tc0100scn[0]->set_offsets_fliptx(6, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::qzquest(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::qzquest_map);

	/* video hardware */
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_partial_buffer_delayed));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_palette(m_palette);
}

void taitof2_state::pulirula(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::pulirula_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,pulirula)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri_roz));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0430GRW(config, m_tc0430grw, 0);
	m_tc0430grw->set_color_base(0);
	m_tc0430grw->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::metalb(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::metalb_map);

	/* video hardware */
	m_palette->set_format(palette_device::RRRRGGGGBBBBRGBx, 8192);

	MCFG_VIDEO_START_OVERRIDE(taitof2_state,metalb)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_metalb));

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(0x32 + 3, -0x04);
	m_tc0480scp->set_offsets_tx(1, 0);
	m_tc0480scp->set_offsets_flip(-1, 0);
	m_tc0480scp->set_col_base(4096);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::qzchikyu(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::qzchikyu_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,qzchikyu)
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_partial_buffer_delayed_qzchikyu));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(0, 0);
	m_tc0100scn[0]->set_offsets_flip(-4, 0);
	m_tc0100scn[0]->set_offsets_fliptx(-11, 0);
	m_tc0100scn[0]->set_palette(m_palette);
}

void taitof2_state::yesnoj(machine_config &config)
{
	taito_f2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::yesnoj_map);

	/* video hardware */
	m_palette->set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	MCFG_VIDEO_START_OVERRIDE(taitof2_state,yesnoj)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_yesnoj));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_gfxlayout(TC0100SCN_LAYOUT_1BPP);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC8521(config, "rtc", XTAL(32'768));
}

void taitof2_state::deadconx(machine_config &config)
{
	taito_f2_te7750(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::deadconx_map);

	/* video hardware */
	m_palette->set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	MCFG_VIDEO_START_OVERRIDE(taitof2_state,deadconx)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_deadconx));

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(0x1e + 3, 0x08);
	m_tc0480scp->set_offsets_tx(-1, 0);
	m_tc0480scp->set_offsets_flip(-1, 0);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::deadconxj(machine_config &config)
{
	taito_f2_te7750(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::deadconx_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,deadconxj)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_deadconx));

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(0x34 + 3, -0x05);
	m_tc0480scp->set_offsets_tx(-1, 0);
	m_tc0480scp->set_offsets_flip(-1, 0);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::dinorex(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::dinorex_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,dinorex)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri));

	m_palette->set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::qjinsei(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::qjinsei_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,quiz)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri));

	m_palette->set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::qcrayon(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::qcrayon_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,quiz)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri));

	m_palette->set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::qcrayon2(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::qcrayon2_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,quiz)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri));

	m_palette->set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::driftout(machine_config &config)
{
	taito_f2_tc0510nio(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::driftout_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(taitof2_state,driftout)
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri_roz));

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0430GRW(config, m_tc0430grw, 0);
	m_tc0430grw->set_color_base(0);
	m_tc0430grw->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);
}

void taitof2_state::cameltrya(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu,24000000/2);  /* verified on pcb  */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::cameltrya_map);
	m_maincpu->set_vblank_int("screen", FUNC(taitof2_state::interrupt));

	Z80(config, m_audiocpu, 24000000/4);    /* verifed on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitof2_state::cameltrya_sound_map);

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(taitof2_state::coin_nibble_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri_roz));
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_no_buffer));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_taitof2);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 4096);

	MCFG_VIDEO_START_OVERRIDE(taitof2_state,dondokod)

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0280GRD(config, m_tc0280grd, 0);
	m_tc0280grd->set_color_base(0);
	m_tc0280grd->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 24000000/8)); /* verified on pcb  */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_a_write_callback().set(FUNC(taitof2_state::cameltrya_porta_w));   /* not implemented */
	ymsnd.add_route(0, "mono", 0.20);
	ymsnd.add_route(1, "mono", 0.20);
	ymsnd.add_route(2, "mono", 0.20);
	ymsnd.add_route(3, "mono", 0.60);

	OKIM6295(config, m_oki, XTAL(4'224'000)/4, okim6295_device::PIN7_HIGH); /* verified on pcb */
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.10);

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}

void taitof2_state::driveout(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 14_MHz_XTAL);  // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &taitof2_state::driveout_map);
	m_maincpu->set_vblank_int("screen", FUNC(taitof2_state::interrupt));

	Z80(config, m_audiocpu, 8_MHz_XTAL / 2); // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitof2_state::driveout_sound_map);

	TC0510NIO(config, m_tc0510nio, 0);
	m_tc0510nio->read_0_callback().set_ioport("DSWA");
	m_tc0510nio->read_1_callback().set_ioport("DSWB");
	m_tc0510nio->read_2_callback().set_ioport("IN0");
	m_tc0510nio->read_3_callback().set_ioport("IN1");
	m_tc0510nio->write_4_callback().set(FUNC(taitof2_state::coin_nibble_w));
	m_tc0510nio->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitof2_state::screen_update_pri_roz));
	m_screen->screen_vblank().set(FUNC(taitof2_state::screen_vblank_no_buffer));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_taitof2);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 4096);

	MCFG_VIDEO_START_OVERRIDE(taitof2_state,driftout)

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(3, 0);
	m_tc0100scn[0]->set_palette(m_palette);

	TC0430GRW(config, m_tc0430grw, 0);
	m_tc0430grw->set_color_base(0);
	m_tc0430grw->set_palette(m_palette);

	TC0360PRI(config, m_tc0360pri, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();   /* does it ? */
	SPEAKER(config, "rspeaker").front_right();

	OKIM6295(config, m_oki, 8_MHz_XTAL / 8, okim6295_device::PIN7_LOW);  // verified on PCB
	m_oki->set_addrmap(0, &taitof2_state::driveout_oki_map);
	m_oki->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_oki->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


/***************************************************************************
                                  DRIVERS
***************************************************************************/

ROM_START( finalb )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "b82-09.ic23",  0x00000, 0x20000, CRC(632f1ecd) SHA1(aa3d1c2059b0dd619d1f6e3e0705b65b4f4be74e) )
	ROM_LOAD16_BYTE( "b82-17.ic11",  0x00001, 0x20000, CRC(e91b2ec9) SHA1(c854104b8d48d20ab9278ecd122c987c3d886a26) )

	ROM_REGION( 0x040000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_BYTE( "b82-06.ic33", 0x00001, 0x20000, CRC(fc450a25) SHA1(6929bd2d47549cab037e8807b778741b3c215788) )
	ROM_LOAD16_BYTE( "b82-07.ic34", 0x00000, 0x20000, CRC(ec3df577) SHA1(37a0bb87a12f0332c8e67b22f91c24584f3d46ce) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "b82-04.ic8", 0x000001, 0x80000, CRC(6346f98e) SHA1(3fac5ea56b5ae280cd7ca0e0c6c308376056e1ba) ) /* sprites 4-bit format*/
	ROM_LOAD16_BYTE( "b82-03.ic9", 0x000000, 0x80000, CRC(daa11561) SHA1(81dd596c1b36138904971c36466ec29d08d4fd84) ) /* sprites 4-bit format*/

	/* Note: this is intentional to load at 0x180000, not at 0x100000
	   because finalb_driver_init will move some bits around before data
	   will be 'gfxdecoded'. The whole thing is because this data is 2bits-
	   while above is 4bits-packed format, for a total of 6 bits per pixel. */

	ROM_REGION( 0x080000, "sprites_hi", 0 )
	ROM_LOAD ( "b82-05.ic7", 0x000000, 0x80000, CRC(aa90b93a) SHA1(06f41052659959c58d72c9f68f9f6069cb835672) ) /* sprites 2-bit format */

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "b82_10.ic5", 0x00000, 0x10000, CRC(a38aaaed) SHA1(d476ea516a797e71e0306da54c17ed1759fe1ccd) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b82-02.ic1", 0x00000, 0x80000, CRC(5dd06bdd) SHA1(6eeaec6743805ba429b0ef58a530bc0740646324) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b82-01.ic2", 0x00000, 0x80000, CRC(f0eb6846) SHA1(4697c3fd61ac0d55c0d2a4354ff74719947397c5) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "tibpal16l8.ic41", 0x0000, 0x0104, CRC(11a0a19a) SHA1(0c195a1808dad21130dd377531ed5b8228981581) )
	ROM_LOAD( "tibpal16l8.ic42", 0x0200, 0x0104, CRC(cc53deb8) SHA1(174da395d9ee74cfdc2bfd0d91297a8a8991c7c5) )
	ROM_LOAD( "tibpal16l8.ic51", 0x0400, 0x0104, CRC(f2878537) SHA1(196639bbcf292fe896dc2a3571ec51c5b45ced5e) )
	ROM_LOAD( "gal16v8.ic13",    0x0600, 0x0117, CRC(a4f75fd0) SHA1(fbcaa60fb4a9f605baa48ebb55551c158dcc9730) )
	ROM_LOAD( "gal16v8.ic35",    0x0800, 0x0117, CRC(ca4eb3e1) SHA1(139c03448f692d32566cd7280b0d5fde63842dc1) )
ROM_END

ROM_START( finalbj )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "b82-09.ic23",  0x00000, 0x20000, CRC(632f1ecd) SHA1(aa3d1c2059b0dd619d1f6e3e0705b65b4f4be74e) )
	ROM_LOAD16_BYTE( "b82-08.ic11",  0x00001, 0x20000, CRC(07154fe5) SHA1(4772362375c8c2984a305c3bb0320ea80a2e9a40) )

	ROM_REGION( 0x040000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_BYTE( "b82-06.ic33", 0x00001, 0x20000, CRC(fc450a25) SHA1(6929bd2d47549cab037e8807b778741b3c215788) )
	ROM_LOAD16_BYTE( "b82-07.ic34", 0x00000, 0x20000, CRC(ec3df577) SHA1(37a0bb87a12f0332c8e67b22f91c24584f3d46ce) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "b82-04.ic8", 0x000001, 0x80000, CRC(6346f98e) SHA1(3fac5ea56b5ae280cd7ca0e0c6c308376056e1ba) ) /* sprites 4-bit format*/
	ROM_LOAD16_BYTE( "b82-03.ic9", 0x000000, 0x80000, CRC(daa11561) SHA1(81dd596c1b36138904971c36466ec29d08d4fd84) ) /* sprites 4-bit format*/

	/* Note: this is intentional to load at 0x180000, not at 0x100000
	   because finalb_driver_init will move some bits around before data
	   will be 'gfxdecoded'. The whole thing is because this data is 2bits-
	   while above is 4bits-packed format, for a total of 6 bits per pixel. */

	ROM_REGION( 0x080000, "sprites_hi", 0 )
	ROM_LOAD ( "b82-05.ic7", 0x000000, 0x80000, CRC(aa90b93a) SHA1(06f41052659959c58d72c9f68f9f6069cb835672) ) /* sprites 2-bit format */

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "b82_10.ic5", 0x00000, 0x10000, CRC(a38aaaed) SHA1(d476ea516a797e71e0306da54c17ed1759fe1ccd) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b82-02.ic1", 0x00000, 0x80000, CRC(5dd06bdd) SHA1(6eeaec6743805ba429b0ef58a530bc0740646324) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b82-01.ic2", 0x00000, 0x80000, CRC(f0eb6846) SHA1(4697c3fd61ac0d55c0d2a4354ff74719947397c5) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "tibpal16l8.ic41", 0x0000, 0x0104, CRC(11a0a19a) SHA1(0c195a1808dad21130dd377531ed5b8228981581) )
	ROM_LOAD( "tibpal16l8.ic42", 0x0200, 0x0104, CRC(cc53deb8) SHA1(174da395d9ee74cfdc2bfd0d91297a8a8991c7c5) )
	ROM_LOAD( "tibpal16l8.ic51", 0x0400, 0x0104, CRC(f2878537) SHA1(196639bbcf292fe896dc2a3571ec51c5b45ced5e) )
	ROM_LOAD( "gal16v8.ic13",    0x0600, 0x0117, CRC(a4f75fd0) SHA1(fbcaa60fb4a9f605baa48ebb55551c158dcc9730) )
	ROM_LOAD( "gal16v8.ic35",    0x0800, 0x0117, CRC(ca4eb3e1) SHA1(139c03448f692d32566cd7280b0d5fde63842dc1) )
ROM_END

ROM_START( finalbu )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	/* are these even good dumps / legit ? there are some strange changes around 0x00fxx as well as the region byte */
	ROM_LOAD16_BYTE( "b82-09-1.ic23",  0x00000, 0x20000, CRC(66729cb9) SHA1(f265c07966cf3930a9b5e2dd63d49554705c60f7) )
	ROM_LOAD16_BYTE( "b82-16-1.ic11",  0x00001, 0x20000, CRC(879387fa) SHA1(9d7aa8ece6cfc66e7c131d9c7a3db792a0336e09) )

	ROM_REGION( 0x040000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_BYTE( "b82-06.ic33", 0x00001, 0x20000, CRC(fc450a25) SHA1(6929bd2d47549cab037e8807b778741b3c215788) )
	ROM_LOAD16_BYTE( "b82-07.ic34", 0x00000, 0x20000, CRC(ec3df577) SHA1(37a0bb87a12f0332c8e67b22f91c24584f3d46ce) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "b82-04.ic8", 0x000001, 0x80000, CRC(6346f98e) SHA1(3fac5ea56b5ae280cd7ca0e0c6c308376056e1ba) ) /* sprites 4-bit format*/
	ROM_LOAD16_BYTE( "b82-03.ic9", 0x000000, 0x80000, CRC(daa11561) SHA1(81dd596c1b36138904971c36466ec29d08d4fd84) ) /* sprites 4-bit format*/

	/* Note: this is intentional to load at 0x180000, not at 0x100000
	   because finalb_driver_init will move some bits around before data
	   will be 'gfxdecoded'. The whole thing is because this data is 2bits-
	   while above is 4bits-packed format, for a total of 6 bits per pixel. */

	ROM_REGION( 0x080000, "sprites_hi", 0 )
	ROM_LOAD ( "b82-05.ic7", 0x000000, 0x80000, CRC(aa90b93a) SHA1(06f41052659959c58d72c9f68f9f6069cb835672) ) /* sprites 2-bit format */

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "b82_10.ic5", 0x00000, 0x10000, CRC(a38aaaed) SHA1(d476ea516a797e71e0306da54c17ed1759fe1ccd) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b82-02.ic1", 0x00000, 0x80000, CRC(5dd06bdd) SHA1(6eeaec6743805ba429b0ef58a530bc0740646324) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b82-01.ic2", 0x00000, 0x80000, CRC(f0eb6846) SHA1(4697c3fd61ac0d55c0d2a4354ff74719947397c5) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "tibpal16l8.ic41", 0x0000, 0x0104, CRC(11a0a19a) SHA1(0c195a1808dad21130dd377531ed5b8228981581) )
	ROM_LOAD( "tibpal16l8.ic42", 0x0200, 0x0104, CRC(cc53deb8) SHA1(174da395d9ee74cfdc2bfd0d91297a8a8991c7c5) )
	ROM_LOAD( "tibpal16l8.ic51", 0x0400, 0x0104, CRC(f2878537) SHA1(196639bbcf292fe896dc2a3571ec51c5b45ced5e) )
	ROM_LOAD( "gal16v8.ic13",    0x0600, 0x0117, CRC(a4f75fd0) SHA1(fbcaa60fb4a9f605baa48ebb55551c158dcc9730) )
	ROM_LOAD( "gal16v8.ic35",    0x0800, 0x0117, CRC(ca4eb3e1) SHA1(139c03448f692d32566cd7280b0d5fde63842dc1) )
ROM_END

ROM_START( dondokod )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b95-12.bin",   0x00000, 0x20000, CRC(d0fce87a) SHA1(7b346d3b7cbaf0b5447d66a71e815202d796f140) )
	ROM_LOAD16_BYTE( "b95-11-1.bin", 0x00001, 0x20000, CRC(dad40cd3) SHA1(6c07ed3dd609a8743f5851caa4d205bce8db595e) )
	ROM_LOAD16_BYTE( "b95-10.bin",   0x40000, 0x20000, CRC(a46e1f0b) SHA1(4adfa7a788d31860e557c4059f77440fe12ac110) )
	ROM_LOAD16_BYTE( "b95-14.bin",   0x40001, 0x20000, CRC(6e4e1351) SHA1(207db5f08904d36c1d27cf326eb9260771a836c2) )   // needs name verfied

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "b95-02.bin", 0x00000, 0x80000, CRC(67b4e979) SHA1(e709cc24e001bccde1178f7e645fc7aec442540c) )

	ROM_REGION( 0x080000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "b95-01.bin", 0x00000, 0x80000, CRC(51c176ce) SHA1(2866e8bd57b301a0d4690f194be95784c53f9fcb) )

	ROM_REGION( 0x080000, "tc0280grd", 0 )   /* pivot gfx */
	ROM_LOAD( "b95-03.bin", 0x00000, 0x80000, CRC(543aa0d1) SHA1(38282ae36a94cc3a354d343cf7d5262e0e309d1f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "b95-08.bin",  0x00000, 0x10000, CRC(b5aa49e1) SHA1(83b0a3434e0d0b9aa581c1acdd0c70308362b923) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b95-04.bin",  0x00000, 0x80000, CRC(ac4c1716) SHA1(06a9def7fa3bd739438f4a1d7b55f70eb904bf54) )

	/* no Delta-T samples */
ROM_END

ROM_START( dondokodu )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b95-12.bin",   0x00000, 0x20000, CRC(d0fce87a) SHA1(7b346d3b7cbaf0b5447d66a71e815202d796f140) )
	ROM_LOAD16_BYTE( "b95-11-1.bin", 0x00001, 0x20000, CRC(dad40cd3) SHA1(6c07ed3dd609a8743f5851caa4d205bce8db595e) )
	ROM_LOAD16_BYTE( "b95-10.bin",   0x40000, 0x20000, CRC(a46e1f0b) SHA1(4adfa7a788d31860e557c4059f77440fe12ac110) )
	ROM_LOAD16_BYTE( "b95-13.bin",   0x40001, 0x20000, CRC(350d2c65) SHA1(60e8651256867648a24719e1bbd1367e89784e30) )   // needs name verfied

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "b95-02.bin", 0x00000, 0x80000, CRC(67b4e979) SHA1(e709cc24e001bccde1178f7e645fc7aec442540c) )

	ROM_REGION( 0x080000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "b95-01.bin", 0x00000, 0x80000, CRC(51c176ce) SHA1(2866e8bd57b301a0d4690f194be95784c53f9fcb) )

	ROM_REGION( 0x080000, "tc0280grd", 0 )   /* pivot gfx */
	ROM_LOAD( "b95-03.bin", 0x00000, 0x80000, CRC(543aa0d1) SHA1(38282ae36a94cc3a354d343cf7d5262e0e309d1f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "b95-08.bin",  0x00000, 0x10000, CRC(b5aa49e1) SHA1(83b0a3434e0d0b9aa581c1acdd0c70308362b923) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b95-04.bin",  0x00000, 0x80000, CRC(ac4c1716) SHA1(06a9def7fa3bd739438f4a1d7b55f70eb904bf54) )

	/* no Delta-T samples */
ROM_END

ROM_START( dondokodj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b95-12.bin",   0x00000, 0x20000, CRC(d0fce87a) SHA1(7b346d3b7cbaf0b5447d66a71e815202d796f140) )
	ROM_LOAD16_BYTE( "b95-11-1.bin", 0x00001, 0x20000, CRC(dad40cd3) SHA1(6c07ed3dd609a8743f5851caa4d205bce8db595e) )
	ROM_LOAD16_BYTE( "b95-10.bin",   0x40000, 0x20000, CRC(a46e1f0b) SHA1(4adfa7a788d31860e557c4059f77440fe12ac110) )
	ROM_LOAD16_BYTE( "b95-09.bin",   0x40001, 0x20000, CRC(d8c86d39) SHA1(43a6a9d545c953e72b6a10bc9d7b2aa2f0ab4764) )

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "b95-02.bin", 0x00000, 0x80000, CRC(67b4e979) SHA1(e709cc24e001bccde1178f7e645fc7aec442540c) )

	ROM_REGION( 0x080000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "b95-01.bin", 0x00000, 0x80000, CRC(51c176ce) SHA1(2866e8bd57b301a0d4690f194be95784c53f9fcb) )

	ROM_REGION( 0x080000, "tc0280grd", 0 )   /* pivot gfx */
	ROM_LOAD( "b95-03.bin", 0x00000, 0x80000, CRC(543aa0d1) SHA1(38282ae36a94cc3a354d343cf7d5262e0e309d1f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "b95-08.bin",  0x00000, 0x10000, CRC(b5aa49e1) SHA1(83b0a3434e0d0b9aa581c1acdd0c70308362b923) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b95-04.bin",  0x00000, 0x80000, CRC(ac4c1716) SHA1(06a9def7fa3bd739438f4a1d7b55f70eb904bf54) )

	/* no Delta-T samples */
ROM_END

ROM_START( megablst )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c11-07.55",  0x00000, 0x20000, CRC(11d228b6) SHA1(5f658a4a0ece3ad4e02ccad6e2852e16dd338dfd) )
	ROM_LOAD16_BYTE( "c11-08.39",  0x00001, 0x20000, CRC(a79d4dca) SHA1(72a97577981a303230374c5f5e201066f71d9cc5) )
	ROM_LOAD16_BYTE( "c11-06.54",  0x40000, 0x20000, CRC(7c249894) SHA1(88dff86b446bcbc4e8ab14cfc3c57b40d25cfa97) )
	ROM_LOAD16_BYTE( "c11-11.38",  0x40001, 0x20000, CRC(263ecbf9) SHA1(b49c59058d6d11ea0d9f9b041789e381e5742905) )

	ROM_REGION( 0x2000, "cchip:cchip_eprom", 0 )
	ROM_LOAD( "c11-15.ic59", 0x0000, 0x2000, CRC(af49ee7f) SHA1(824d7ed371c19f31768b20117027edba6ffc890e) )

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c11-05.58", 0x00000, 0x80000, CRC(733e6d8e) SHA1(47f3360f7c41b7e4a42e8198fc1bcce4e819181f) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "c11-03.32", 0x00000, 0x80000, CRC(46718c7a) SHA1(c10308a282bf0c618108e4afc7ce6f0f6cb8c6c0) )
	ROM_LOAD16_BYTE( "c11-04.31", 0x00001, 0x80000, CRC(663f33cc) SHA1(5d3d3e77b7a84f6a3d4e744eef9b63bef91180e8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c11-12.3", 0x00000, 0x10000, CRC(b11094f1) SHA1(a01e9f7d1f616bb379eaa85ad81b94173b067782) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c11-01.29", 0x00000, 0x80000, CRC(fd1ea532) SHA1(481698b747a421a17bfb8cef96065712d4f3997f) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c11-02.30", 0x00000, 0x80000, CRC(451cc187) SHA1(a682f70bbe6cba2fe2c0a6791e8d33db34eb2cee) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16l8b-b89-01.ic8",   0x0000, 0x0104, CRC(4095b97a) SHA1(ff63fe125e8e38c25cc7ef05c6cf8f7818fbdaa3) )
	ROM_LOAD( "pal16l8b-b89-02.ic28",  0x0200, 0x0104, CRC(6430b559) SHA1(e886d7ac44018cac8e788c822ddbb612c14390c9) )
	ROM_LOAD( "pal16l8b-b89-03.bin",   0x0400, 0x0104, CRC(634592e2) SHA1(8d9251958e7f9bdcf2e360b15dce3552a8da0c28) )
	ROM_LOAD( "palce16v8-b89-04.ic27", 0x0600, 0x0117, CRC(fc136ae2) SHA1(ac927a6ec6889b2eb24554fbf017c85e92afc866) )
	ROM_LOAD( "pal16l8b-c11-13.ic13",  0x0800, 0x0104, CRC(421d7ea8) SHA1(4791e334a1917c7efc57eaa7090317bd11138d6b) )  // MDEC 1
	ROM_LOAD( "pal16l8b-c11-14.ic23",  0x0a00, 0x0104, CRC(5c740aee) SHA1(995ccf52f85260f141ce5e3697c9391448c3d69d) )  // MDEC 2
ROM_END

ROM_START( megablstu )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c11-07.55",  0x00000, 0x20000, CRC(11d228b6) SHA1(5f658a4a0ece3ad4e02ccad6e2852e16dd338dfd) )
	ROM_LOAD16_BYTE( "c11-08.39",  0x00001, 0x20000, CRC(a79d4dca) SHA1(72a97577981a303230374c5f5e201066f71d9cc5) )
	ROM_LOAD16_BYTE( "c11-06.54",  0x40000, 0x20000, CRC(7c249894) SHA1(88dff86b446bcbc4e8ab14cfc3c57b40d25cfa97) )
	ROM_LOAD16_BYTE( "c11-10.38",  0x40001, 0x20000, CRC(bf379a43) SHA1(2a0294e55c2ce514caa2885b728e6387311ed482) )

	ROM_REGION( 0x2000, "cchip:cchip_eprom", 0 )
	ROM_LOAD( "c11-15.ic59", 0x0000, 0x2000, CRC(af49ee7f) SHA1(824d7ed371c19f31768b20117027edba6ffc890e) )

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c11-05.58", 0x00000, 0x80000, CRC(733e6d8e) SHA1(47f3360f7c41b7e4a42e8198fc1bcce4e819181f) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "c11-03.32", 0x00000, 0x80000, CRC(46718c7a) SHA1(c10308a282bf0c618108e4afc7ce6f0f6cb8c6c0) )
	ROM_LOAD16_BYTE( "c11-04.31", 0x00001, 0x80000, CRC(663f33cc) SHA1(5d3d3e77b7a84f6a3d4e744eef9b63bef91180e8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c11-12.3", 0x00000, 0x10000, CRC(b11094f1) SHA1(a01e9f7d1f616bb379eaa85ad81b94173b067782) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c11-01.29", 0x00000, 0x80000, CRC(fd1ea532) SHA1(481698b747a421a17bfb8cef96065712d4f3997f) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c11-02.30", 0x00000, 0x80000, CRC(451cc187) SHA1(a682f70bbe6cba2fe2c0a6791e8d33db34eb2cee) )
ROM_END

ROM_START( megablstj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c11-07.55",  0x00000, 0x20000, CRC(11d228b6) SHA1(5f658a4a0ece3ad4e02ccad6e2852e16dd338dfd) ) // c11-07.17
	ROM_LOAD16_BYTE( "c11-08.39",  0x00001, 0x20000, CRC(a79d4dca) SHA1(72a97577981a303230374c5f5e201066f71d9cc5) ) // c11-08.19
	ROM_LOAD16_BYTE( "c11-06.54",  0x40000, 0x20000, CRC(7c249894) SHA1(88dff86b446bcbc4e8ab14cfc3c57b40d25cfa97) ) // c11-06.16
	ROM_LOAD16_BYTE( "c11-09.38",  0x40001, 0x20000, CRC(c830aad5) SHA1(967ad3e052572300f5f49375e5f8348f2d595680) ) // c11-09.18

	ROM_REGION( 0x2000, "cchip:cchip_eprom", 0 )
	ROM_LOAD( "c11-15.ic59", 0x0000, 0x2000, CRC(af49ee7f) SHA1(824d7ed371c19f31768b20117027edba6ffc890e) )

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c11-05.58", 0x00000, 0x80000, CRC(733e6d8e) SHA1(47f3360f7c41b7e4a42e8198fc1bcce4e819181f) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "c11-03.32", 0x00000, 0x80000, CRC(46718c7a) SHA1(c10308a282bf0c618108e4afc7ce6f0f6cb8c6c0) )
	ROM_LOAD16_BYTE( "c11-04.31", 0x00001, 0x80000, CRC(663f33cc) SHA1(5d3d3e77b7a84f6a3d4e744eef9b63bef91180e8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c11-12.3", 0x00000, 0x10000, CRC(b11094f1) SHA1(a01e9f7d1f616bb379eaa85ad81b94173b067782) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c11-01.29", 0x00000, 0x80000, CRC(fd1ea532) SHA1(481698b747a421a17bfb8cef96065712d4f3997f) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c11-02.30", 0x00000, 0x80000, CRC(451cc187) SHA1(a682f70bbe6cba2fe2c0a6791e8d33db34eb2cee) )
ROM_END

ROM_START( thundfox )       /* Thunder Fox */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c28-13-1.51",  0x00000, 0x20000, CRC(acb07013) SHA1(5043d1859ae908c00f0c00c7b8e377362d908423) )
	ROM_LOAD16_BYTE( "c28-16-1.40",  0x00001, 0x20000, CRC(1e43d55b) SHA1(e5a389926ee95f19fc9f5d5bde97436d6f52124a) )
	ROM_LOAD16_BYTE( "c28-08.50",    0x40000, 0x20000, CRC(38e038f1) SHA1(4b8ed31e35927671ce313f4575e622ecab2c27cb) )
	ROM_LOAD16_BYTE( "c28-07.39",    0x40001, 0x20000, CRC(24419abb) SHA1(7d3e70213ae04dd921fc1bce8abb385747c90a38) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "c28-03.29", 0x00000, 0x80000, CRC(51bdc7af) SHA1(e36a063932fa5bd6609930c3708fee1e6feb5389) )
	ROM_LOAD16_BYTE( "c28-04.28", 0x00001, 0x80000, CRC(ba7ed535) SHA1(be7e010f6788d1b82cebc932c793a0a976647832) )

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c28-02.61", 0x000000, 0x80000, CRC(6230a09d) SHA1(780aff5d4511c5e08cbf78784b163d60358f9283) )    /* TC0100SCN #1 */

	ROM_REGION( 0x080000, "tc0100scn_2", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c28-01.63", 0x000000, 0x80000, CRC(44552b25) SHA1(850c085e3dacd4867f6bcdfab641eb07934e620f) )    /* TC0100SCN #2 */

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c28-14.3",  0x00000, 0x10000, CRC(45ef3616) SHA1(97bf1de7fd32a378839df1845f7522dae776d997) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c28-06.41", 0x00000, 0x80000, CRC(db6983db) SHA1(b72541aa35c48624478060e7453f01956ff1ceb2) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c28-05.42", 0x00000, 0x80000, CRC(d3b238fa) SHA1(b4a0cdd7174e60527e7a47018d6117adc5518da1) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16l8b-b89-01.ic19",  0x0000, 0x0104, CRC(4095b97a) SHA1(ff63fe125e8e38c25cc7ef05c6cf8f7818fbdaa3) )
	ROM_LOAD( "pal16l8b-b89-03.ic37",  0x0200, 0x0104, CRC(634592e2) SHA1(8d9251958e7f9bdcf2e360b15dce3552a8da0c28) )
	ROM_LOAD( "pal16v8b-b89-04.ic33",  0x0400, 0x0117, CRC(fc136ae2) SHA1(ac927a6ec6889b2eb24554fbf017c85e92afc866) )
	ROM_LOAD( "pal16l8b-c28-09.ic25",  0x0600, 0x0104, CRC(383e7305) SHA1(480956fb68da0913d6176d31919d33f34f55bee4) )
	ROM_LOAD( "pal16l8b-c28-10.ic26",  0x0800, 0x0104, CRC(47fccc07) SHA1(205386e8e9bba0a86955246974cf25b5aaa9c715) )  // MDEC 1
	ROM_LOAD( "pal16l8b-c28-11.ic35",  0x0a00, 0x0104, CRC(33414fe8) SHA1(7f497189fd48c6201d1048419eb67153c6cf730c) )  // MDEC 2
ROM_END

ROM_START( thundfoxu )      /* Thunder Fox */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c28-13-1.51",  0x00000, 0x20000, CRC(acb07013) SHA1(5043d1859ae908c00f0c00c7b8e377362d908423) )
	ROM_LOAD16_BYTE( "c28-15-1.40",  0x00001, 0x20000, CRC(874a84e1) SHA1(f2688030faf526bc64bbb06225d3938f423f0f8b) )
	ROM_LOAD16_BYTE( "c28-08.50",    0x40000, 0x20000, CRC(38e038f1) SHA1(4b8ed31e35927671ce313f4575e622ecab2c27cb) )
	ROM_LOAD16_BYTE( "c28-07.39",    0x40001, 0x20000, CRC(24419abb) SHA1(7d3e70213ae04dd921fc1bce8abb385747c90a38) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "c28-03.29", 0x00000, 0x80000, CRC(51bdc7af) SHA1(e36a063932fa5bd6609930c3708fee1e6feb5389) )
	ROM_LOAD16_BYTE( "c28-04.28", 0x00001, 0x80000, CRC(ba7ed535) SHA1(be7e010f6788d1b82cebc932c793a0a976647832) )

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c28-02.61", 0x000000, 0x80000, CRC(6230a09d) SHA1(780aff5d4511c5e08cbf78784b163d60358f9283) )    /* TC0100SCN #1 */

	ROM_REGION( 0x080000, "tc0100scn_2", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c28-01.63", 0x000000, 0x80000, CRC(44552b25) SHA1(850c085e3dacd4867f6bcdfab641eb07934e620f) )    /* TC0100SCN #2 */

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c28-14.3",  0x00000, 0x10000, CRC(45ef3616) SHA1(97bf1de7fd32a378839df1845f7522dae776d997) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c28-06.41", 0x00000, 0x80000, CRC(db6983db) SHA1(b72541aa35c48624478060e7453f01956ff1ceb2) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c28-05.42", 0x00000, 0x80000, CRC(d3b238fa) SHA1(b4a0cdd7174e60527e7a47018d6117adc5518da1) )

// Pals: c28-09.25  c28-10.26  c28-11.35  b89-01.19  b89-03.37  b89-04.33
ROM_END

ROM_START( thundfoxj )      /* Thunder Fox */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c28-13-1.51",  0x00000, 0x20000, CRC(acb07013) SHA1(5043d1859ae908c00f0c00c7b8e377362d908423) )
	ROM_LOAD16_BYTE( "c28-12-1.40",  0x00001, 0x20000, CRC(f04db477) SHA1(da66895b8cc79f1776f30d9c204c6907cab935db) )
	ROM_LOAD16_BYTE( "c28-08.50",    0x40000, 0x20000, CRC(38e038f1) SHA1(4b8ed31e35927671ce313f4575e622ecab2c27cb) )
	ROM_LOAD16_BYTE( "c28-07.39",    0x40001, 0x20000, CRC(24419abb) SHA1(7d3e70213ae04dd921fc1bce8abb385747c90a38) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "c28-03.29", 0x00000, 0x80000, CRC(51bdc7af) SHA1(e36a063932fa5bd6609930c3708fee1e6feb5389) )
	ROM_LOAD16_BYTE( "c28-04.28", 0x00001, 0x80000, CRC(ba7ed535) SHA1(be7e010f6788d1b82cebc932c793a0a976647832) )

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c28-02.61", 0x000000, 0x80000, CRC(6230a09d) SHA1(780aff5d4511c5e08cbf78784b163d60358f9283) )    /* TC0100SCN #1 */

	ROM_REGION( 0x080000, "tc0100scn_2", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c28-01.63", 0x000000, 0x80000, CRC(44552b25) SHA1(850c085e3dacd4867f6bcdfab641eb07934e620f) )    /* TC0100SCN #2 */

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c28-14.3",  0x00000, 0x10000, CRC(45ef3616) SHA1(97bf1de7fd32a378839df1845f7522dae776d997) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c28-06.41", 0x00000, 0x80000, CRC(db6983db) SHA1(b72541aa35c48624478060e7453f01956ff1ceb2) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c28-05.42", 0x00000, 0x80000, CRC(d3b238fa) SHA1(b4a0cdd7174e60527e7a47018d6117adc5518da1) )
ROM_END

ROM_START( cameltry )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c38-11", 0x00000, 0x20000, CRC(be172da0) SHA1(e4915bf25832175591a014aa1abac5edae09380d) )
	ROM_LOAD16_BYTE( "c38-14", 0x00001, 0x20000, CRC(ffa430de) SHA1(a3cdb35151a92ddfa2090c1f8710500925e7ad0c) )

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty! */

	ROM_REGION( 0x080000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c38-01.bin", 0x00000, 0x80000, CRC(c170ff36) SHA1(6a19cc99847ed35ac8a8e9ba0e2e91bfac662203) )

	ROM_REGION( 0x080000, "tc0280grd", 0 )   /* pivot gfx */
	ROM_LOAD( "c38-02.bin", 0x00000, 0x20000, CRC(1a11714b) SHA1(419f5ec37161fd6b4ca962768e720adf541271d5) )
	/* this is on the PCB twice, probably one for each ROZ layer, we load it twice to make this clear */
	ROM_LOAD( "c38-02.bin", 0x00000, 0x20000, CRC(1a11714b) SHA1(419f5ec37161fd6b4ca962768e720adf541271d5) )

	/* These are for a YM2610 */
	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c38-08.bin", 0x00000, 0x10000, CRC(7ff78873) SHA1(6574f1c707b8911fa957dd057e1cddc7a1cea99b) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c38-03.bin", 0x000000, 0x020000, CRC(59fa59a7) SHA1(161f11b96a47c8431c33e300f6a509bf804309af) )
	/* no Delta-T samples */
ROM_END

ROM_START( cameltryj )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c38-09.bin", 0x00000, 0x20000, CRC(2ae01120) SHA1(6da4155fde0edd76976264f929c5da3c79db5017) )
	ROM_LOAD16_BYTE( "c38-10.bin", 0x00001, 0x20000, CRC(48d8ff56) SHA1(73da47b0f9e67defcd0072b71a3661f2c3534f55) )

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty! */

	ROM_REGION( 0x080000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c38-01.bin", 0x00000, 0x80000, CRC(c170ff36) SHA1(6a19cc99847ed35ac8a8e9ba0e2e91bfac662203) )

	ROM_REGION( 0x080000, "tc0280grd", 0 )   /* pivot gfx */
	ROM_LOAD( "c38-02.bin", 0x00000, 0x20000, CRC(1a11714b) SHA1(419f5ec37161fd6b4ca962768e720adf541271d5) )
	/* this is on the PCB twice, probably one for each ROZ layer, we load it twice to make this clear */
	ROM_LOAD( "c38-02.bin", 0x00000, 0x20000, CRC(1a11714b) SHA1(419f5ec37161fd6b4ca962768e720adf541271d5) )

	/* These are for a YM2610 */
	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c38-08.bin", 0x00000, 0x10000, CRC(7ff78873) SHA1(6574f1c707b8911fa957dd057e1cddc7a1cea99b) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c38-03.bin", 0x000000, 0x020000, CRC(59fa59a7) SHA1(161f11b96a47c8431c33e300f6a509bf804309af) )

	/* no Delta-T samples */
ROM_END

ROM_START( cameltrya )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c38-11", 0x00000, 0x20000, CRC(be172da0) SHA1(e4915bf25832175591a014aa1abac5edae09380d) )
	ROM_LOAD16_BYTE( "c38-16", 0x00001, 0x20000, CRC(66ad6164) SHA1(2df22a6a1d6e194a467e6a6c6b6c2fc9f8441852) )

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty! */

	ROM_REGION( 0x080000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c38-01.bin", 0x00000, 0x80000, CRC(c170ff36) SHA1(6a19cc99847ed35ac8a8e9ba0e2e91bfac662203) )

	ROM_REGION( 0x080000, "tc0280grd", 0 )   /* pivot gfx */
	ROM_LOAD( "c38-02.bin", 0x00000, 0x20000, CRC(1a11714b) SHA1(419f5ec37161fd6b4ca962768e720adf541271d5) )
	/* this is on the PCB twice, probably one for each ROZ layer, we load it twice to make this clear */
	ROM_LOAD( "c38-02.bin", 0x00000, 0x20000, CRC(1a11714b) SHA1(419f5ec37161fd6b4ca962768e720adf541271d5) )

	/* these are for a YM2203 and OKIM6295 */
	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu (revised prog!?) */
	ROM_LOAD( "c38-15.bin", 0x00000, 0x10000, CRC(0e60faac) SHA1(cd124efb5127e5184c412c48b94c0d4a0b2ade64) )

	ROM_REGION( 0x80000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "c38-04.bin", 0x000000, 0x020000, CRC(53d330bb) SHA1(22982d889a69aefe482b24ac958ef755fd2c7601) )
ROM_END

ROM_START( cameltryau )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c38-11", 0x00000, 0x20000, CRC(be172da0) SHA1(e4915bf25832175591a014aa1abac5edae09380d) )
	ROM_LOAD16_BYTE( "c38-14", 0x00001, 0x20000, CRC(ffa430de) SHA1(a3cdb35151a92ddfa2090c1f8710500925e7ad0c) )

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty! */

	ROM_REGION( 0x080000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c38-01.bin", 0x00000, 0x80000, CRC(c170ff36) SHA1(6a19cc99847ed35ac8a8e9ba0e2e91bfac662203) )

	ROM_REGION( 0x080000, "tc0280grd", 0 )   /* pivot gfx */
	ROM_LOAD( "c38-02.bin", 0x00000, 0x20000, CRC(1a11714b) SHA1(419f5ec37161fd6b4ca962768e720adf541271d5) )
	/* this is on the PCB twice, probably one for each ROZ layer, we load it twice to make this clear */
	ROM_LOAD( "c38-02.bin", 0x00000, 0x20000, CRC(1a11714b) SHA1(419f5ec37161fd6b4ca962768e720adf541271d5) )

	/* these are for a YM2203 and OKIM6295 */
	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu (revised prog!?) */
	ROM_LOAD( "c38-15.bin", 0x00000, 0x10000, CRC(0e60faac) SHA1(cd124efb5127e5184c412c48b94c0d4a0b2ade64) )

	ROM_REGION( 0x80000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "c38-04.bin", 0x000000, 0x020000, CRC(53d330bb) SHA1(22982d889a69aefe482b24ac958ef755fd2c7601) )
ROM_END

ROM_START( qtorimon )   /* Quiz Torimonochou */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c41-04.bin",  0x00000, 0x20000, CRC(0fbf5223) SHA1(2aa8b3dd20ae922a3ff880db7b46e2bbb708698d) )
	ROM_LOAD16_BYTE( "c41-05.bin",  0x00001, 0x20000, CRC(174bd5db) SHA1(f7a4b2ac91b3bcd886e2a1e1d0415a95f14c9de7) )
	ROM_LOAD16_BYTE( "mask-51.bin", 0x40000, 0x20000, CRC(12e14aca) SHA1(8f7dc54f68984c82420abf96436743c0654a71ea) ) /* char defs, read by cpu */
	ROM_LOAD16_BYTE( "mask-52.bin", 0x40001, 0x20000, CRC(b3ef66f3) SHA1(4766a1ed9b4adcc2f0d2857633ce95194eb80694) ) /* char defs, read by cpu */

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty! */

	ROM_REGION( 0x040000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "c41-02.bin",  0x00000, 0x20000, CRC(05dcd36d) SHA1(f32c5b40e0adad7991bac29ecffcd5dff330e118) )
	ROM_LOAD16_BYTE( "c41-01.bin",  0x00001, 0x20000, CRC(39ff043c) SHA1(a4b0c6763c43a7ad16e98a938ffbb8aef4882eac) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c41-06.bin",    0x00000, 0x10000, CRC(753a98d8) SHA1(a832a4789194a67a2201da4e4484ab08210e5ccc) )

	ROM_REGION( 0x080000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c41-03.bin",  0x000000, 0x020000, CRC(b2c18e89) SHA1(32eca1721cd8f47e9a6dcb553208ddd0daa67f83) )

	/* no Delta-T samples */
ROM_END

ROM_START( liquidk )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c49-09.ic47",  0x00000, 0x20000, CRC(6ae09eb9) SHA1(0f2527a1b231ecf8c6a937bf56d1245fdd270695) )
	ROM_LOAD16_BYTE( "c49-11.ic48",  0x00001, 0x20000, CRC(42d2be6e) SHA1(c7953af2a561159d739d05dc06a4c905b6c40e64) )
	ROM_LOAD16_BYTE( "c49-10.ic45",  0x40000, 0x20000, CRC(50bef2e0) SHA1(54afd46dde81ac0fc272417c53aba1e9e8c90606) )
	ROM_LOAD16_BYTE( "c49-12.ic46",  0x40001, 0x20000, CRC(cb16bad5) SHA1(900c28761200f261cb217f09f492895753ef16f7) )

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c49-03.ic76",  0x00000, 0x80000, CRC(c3364f9b) SHA1(3512a8c352df8b8f19590c859afb8fdec758eb91) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c49-01.ic54", 0x00000, 0x80000, CRC(67cc3163) SHA1(f898d52c136f07497ec6be214f016cbadc700055) )
	ROM_LOAD( "c49-02.ic53", 0x80000, 0x80000, CRC(d2400710) SHA1(082aa0336dbc066d8bb0dd6eb362340f49e87b67) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "c49-08.ic32", 0x00000, 0x10000, CRC(413c310c) SHA1(cecb1c0c9fe3c8b744f95ce29009650a289107ab) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c49-04.ic33",  0x00000, 0x80000, CRC(474d45a4) SHA1(20cb818d753a185973098007e645f1aa75c5528d) )

	/* no Delta-T samples */
ROM_END

ROM_START( liquidku )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c49-09.ic47",  0x00000, 0x20000, CRC(6ae09eb9) SHA1(0f2527a1b231ecf8c6a937bf56d1245fdd270695) )
	ROM_LOAD16_BYTE( "c49-11.ic48",  0x00001, 0x20000, CRC(42d2be6e) SHA1(c7953af2a561159d739d05dc06a4c905b6c40e64) )
	ROM_LOAD16_BYTE( "c49-10.ic45",  0x40000, 0x20000, CRC(50bef2e0) SHA1(54afd46dde81ac0fc272417c53aba1e9e8c90606) )
	ROM_LOAD16_BYTE( "c49-14.ic46",  0x40001, 0x20000, CRC(bc118a43) SHA1(eb306a753ab43e67eacb9d6eff1c14ec78de965f) )

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c49-03.ic76",  0x00000, 0x80000, CRC(c3364f9b) SHA1(3512a8c352df8b8f19590c859afb8fdec758eb91) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c49-01.ic54", 0x00000, 0x80000, CRC(67cc3163) SHA1(f898d52c136f07497ec6be214f016cbadc700055) )
	ROM_LOAD( "c49-02.ic53", 0x80000, 0x80000, CRC(d2400710) SHA1(082aa0336dbc066d8bb0dd6eb362340f49e87b67) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "c49-08.ic32", 0x00000, 0x10000, CRC(413c310c) SHA1(cecb1c0c9fe3c8b744f95ce29009650a289107ab) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c49-04.ic33",  0x00000, 0x80000, CRC(474d45a4) SHA1(20cb818d753a185973098007e645f1aa75c5528d) )

	/* no Delta-T samples */
ROM_END

ROM_START( mizubaku )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c49-09.ic47",  0x00000, 0x20000, CRC(6ae09eb9) SHA1(0f2527a1b231ecf8c6a937bf56d1245fdd270695) )
	ROM_LOAD16_BYTE( "c49-11.ic48",  0x00001, 0x20000, CRC(42d2be6e) SHA1(c7953af2a561159d739d05dc06a4c905b6c40e64) )
	ROM_LOAD16_BYTE( "c49-10.ic45",  0x40000, 0x20000, CRC(50bef2e0) SHA1(54afd46dde81ac0fc272417c53aba1e9e8c90606) )
	ROM_LOAD16_BYTE( "c49-13.ic46",  0x40001, 0x20000, CRC(2518dbf9) SHA1(c5975d3bfbfbb34b37b5da1d1cd2adf3491f9196) )

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c49-03.ic76",  0x00000, 0x80000, CRC(c3364f9b) SHA1(3512a8c352df8b8f19590c859afb8fdec758eb91) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c49-01.ic54", 0x00000, 0x80000, CRC(67cc3163) SHA1(f898d52c136f07497ec6be214f016cbadc700055) )
	ROM_LOAD( "c49-02.ic53", 0x80000, 0x80000, CRC(d2400710) SHA1(082aa0336dbc066d8bb0dd6eb362340f49e87b67) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "c49-08.ic32", 0x00000, 0x10000, CRC(413c310c) SHA1(cecb1c0c9fe3c8b744f95ce29009650a289107ab) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c49-04.ic33",  0x00000, 0x80000, CRC(474d45a4) SHA1(20cb818d753a185973098007e645f1aa75c5528d) )

	/* no Delta-T samples */
ROM_END

ROM_START( quizhq ) /* Quiz HQ */
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c53-05.bin",  0x00000, 0x20000, CRC(c798fc20) SHA1(4467dde3620102f87cffb2f81f71d856c0df12f8) )
	ROM_LOAD16_BYTE( "c53-01.bin",  0x00001, 0x20000, CRC(bf44c93e) SHA1(6fd871f50da4a668767b3096660689905663f697) )
	ROM_LOAD16_BYTE( "c53-52.bin",  0x80000, 0x20000, CRC(12e14aca) SHA1(8f7dc54f68984c82420abf96436743c0654a71ea) ) /* char defs, read by cpu */
	ROM_LOAD16_BYTE( "c53-51.bin",  0x80001, 0x20000, CRC(b3ef66f3) SHA1(4766a1ed9b4adcc2f0d2857633ce95194eb80694) ) /* char defs, read by cpu */

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty */

	ROM_REGION( 0x080000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "c53-03.bin",  0x00000, 0x20000, CRC(47596e70) SHA1(3a4612d9dd2e18a4f7f4c4ed38877071afc9c279) )
	ROM_LOAD16_BYTE( "c53-07.bin",  0x00001, 0x20000, CRC(4f9fa82f) SHA1(ccd1ac17d38a51492b3716bad83e67b282da8bf9) )
	ROM_LOAD16_BYTE( "c53-02.bin",  0x40000, 0x20000, CRC(d704c6f4) SHA1(9b1c47ec3abaff53d641488dece8c97438b2e809) )
	ROM_LOAD16_BYTE( "c53-06.bin",  0x40001, 0x20000, CRC(f77f63fc) SHA1(28a509786817e88c0a5821dd68791ebd30d994c2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c53-08.bin",    0x00000, 0x10000, CRC(25187e81) SHA1(c549fbfff6963be93aaf349b240f15b1d578d1f1) )

	ROM_REGION( 0x080000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c53-04.bin",  0x000000, 0x020000, CRC(99890ad4) SHA1(c9be9d21dc72059c39de81e1b73849cc77d6b95d) )

	/* no Delta-T samples */
ROM_END

ROM_START( ssi )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c64_15-1.ic9", 0x00000, 0x40000, CRC(ce9308a6) SHA1(02653218fe949803742e574eeed01dd421b0a671) )
	ROM_LOAD16_BYTE( "c64_16-1.ic8", 0x00001, 0x40000, CRC(470a483a) SHA1(880d43aec8c3bbae1d58e7d6d7719eb6fe67cc56) )

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty! */

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c64-01.1",     0x000000, 0x100000, CRC(a1b4f486) SHA1(bdd6bf144e50fe7b1d4cf4504471a689669415a4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "c64-09.13",    0x00000, 0x10000, CRC(88d7f65c) SHA1(d6383bf8fd035772fa3c57b26b727eefe1aadd93) )

	ROM_REGION( 0x20000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c64-02.2",     0x00000, 0x20000, CRC(3cb0b907) SHA1(7cbe437fe584575a2f26a582095fd49665c7003e) )

	/* no Delta-T samples */

	ROM_REGION( 0x400, "plds", 0 ) // originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	ROM_LOAD( "c64-10.ic42", 0x000, 0x117, CRC(08e8c3d6) SHA1(9e1f0c8a70af7355d82c2bac249363415c2293e2) )
	ROM_LOAD( "c64-11.ic43", 0x200, 0x117, CRC(f116413e) SHA1(696a8c404ef3d6d832e0fcf530452344f96dd0b2) )
ROM_END

ROM_START( ssia )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c64_15.ic9", 0x00000, 0x40000, CRC(3a6d591b) SHA1(cc08aa89046e774046d1e47afb7d124c9a6b0b88) )
	ROM_LOAD16_BYTE( "c64_16.ic8", 0x00001, 0x40000, CRC(8a567a4f) SHA1(9d309dd3f3bdde180908c46f13f112a0055bcae2) )

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty! */

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c64-01.1",     0x000000, 0x100000, CRC(a1b4f486) SHA1(bdd6bf144e50fe7b1d4cf4504471a689669415a4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "c64-09.13",    0x00000, 0x10000, CRC(88d7f65c) SHA1(d6383bf8fd035772fa3c57b26b727eefe1aadd93) )

	ROM_REGION( 0x20000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c64-02.2",     0x00000, 0x20000, CRC(3cb0b907) SHA1(7cbe437fe584575a2f26a582095fd49665c7003e) )

	/* no Delta-T samples */

	ROM_REGION( 0x400, "plds", 0 ) // originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	ROM_LOAD( "c64-10.ic42", 0x000, 0x117, CRC(08e8c3d6) SHA1(9e1f0c8a70af7355d82c2bac249363415c2293e2) )
	ROM_LOAD( "c64-11.ic43", 0x200, 0x117, CRC(f116413e) SHA1(696a8c404ef3d6d832e0fcf530452344f96dd0b2) )
ROM_END

ROM_START( ssib )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c64_12.ic9", 0x00000, 0x40000, CRC(d5716d7e) SHA1(3a18d8ef1d16380946714910245b00bbcec39e3d) )
	ROM_LOAD16_BYTE( "c64_13.ic8", 0x00001, 0x40000, CRC(99e3dd1c) SHA1(631e6db268d5411acf73d2892ad5107401212a5d) )

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	// empty!

	ROM_REGION( 0x100000, "sprites", 0 )   // OBJ
	ROM_LOAD( "c64-01.ic51",     0x000000, 0x100000, CRC(a1b4f486) SHA1(bdd6bf144e50fe7b1d4cf4504471a689669415a4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c64-09.ic38",    0x00000, 0x10000, CRC(88d7f65c) SHA1(d6383bf8fd035772fa3c57b26b727eefe1aadd93) )

	ROM_REGION( 0x20000, "ymsnd:adpcma", 0 )   // ADPCM samples
	ROM_LOAD( "c64-02.ic1",     0x00000, 0x20000, CRC(3cb0b907) SHA1(7cbe437fe584575a2f26a582095fd49665c7003e) )

	// no Delta-T samples

	ROM_REGION( 0x400, "plds", 0 ) // originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	ROM_LOAD( "c64-10.ic42", 0x000, 0x117, CRC(08e8c3d6) SHA1(9e1f0c8a70af7355d82c2bac249363415c2293e2) )
	ROM_LOAD( "c64-11.ic43", 0x200, 0x117, CRC(f116413e) SHA1(696a8c404ef3d6d832e0fcf530452344f96dd0b2) )
ROM_END

ROM_START( majest12u )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c64_15-1.ic9", 0x00000, 0x40000, CRC(ce9308a6) SHA1(02653218fe949803742e574eeed01dd421b0a671) )
	ROM_LOAD16_BYTE( "c64_17-1.ic8", 0x00001, 0x40000, CRC(300d78ac) SHA1(7cd94dfa4a14a1d221acce61df3693727c6f570c) )

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty! */

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c64-01.1",     0x000000, 0x100000, CRC(a1b4f486) SHA1(bdd6bf144e50fe7b1d4cf4504471a689669415a4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "c64-09.13",    0x00000, 0x10000, CRC(88d7f65c) SHA1(d6383bf8fd035772fa3c57b26b727eefe1aadd93) )

	ROM_REGION( 0x20000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c64-02.2",     0x00000, 0x20000, CRC(3cb0b907) SHA1(7cbe437fe584575a2f26a582095fd49665c7003e) )

	/* no Delta-T samples */

	ROM_REGION( 0x400, "plds", 0 ) // originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	ROM_LOAD( "c64-10.ic42", 0x000, 0x117, CRC(08e8c3d6) SHA1(9e1f0c8a70af7355d82c2bac249363415c2293e2) )
	ROM_LOAD( "c64-11.ic43", 0x200, 0x117, CRC(f116413e) SHA1(696a8c404ef3d6d832e0fcf530452344f96dd0b2) )
ROM_END

ROM_START( majest12ua )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c64_15.ic9", 0x00000, 0x40000, CRC(3a6d591b) SHA1(cc08aa89046e774046d1e47afb7d124c9a6b0b88) )
	ROM_LOAD16_BYTE( "c64_17.ic8", 0x00001, 0x40000, CRC(fd514ad9) SHA1(14646ed90dad208b5874191a329396f35f3c0a0d) )

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty! */

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c64-01.1",     0x000000, 0x100000, CRC(a1b4f486) SHA1(bdd6bf144e50fe7b1d4cf4504471a689669415a4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "c64-09.13",    0x00000, 0x10000, CRC(88d7f65c) SHA1(d6383bf8fd035772fa3c57b26b727eefe1aadd93) )

	ROM_REGION( 0x20000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c64-02.2",     0x00000, 0x20000, CRC(3cb0b907) SHA1(7cbe437fe584575a2f26a582095fd49665c7003e) )

	/* no Delta-T samples */

	ROM_REGION( 0x400, "plds", 0 ) // originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	ROM_LOAD( "c64-10.ic42", 0x000, 0x117, CRC(08e8c3d6) SHA1(9e1f0c8a70af7355d82c2bac249363415c2293e2) )
	ROM_LOAD( "c64-11.ic43", 0x200, 0x117, CRC(f116413e) SHA1(696a8c404ef3d6d832e0fcf530452344f96dd0b2) )
ROM_END

ROM_START( majest12ub )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c64_12.ic9", 0x00000, 0x40000, CRC(d5716d7e) SHA1(3a18d8ef1d16380946714910245b00bbcec39e3d) )
	ROM_LOAD16_BYTE( "c64_14.ic8", 0x00001, 0x40000, CRC(eee4ed8a) SHA1(ad50dc12ede0d327ef9ded5ffd9dbd6e865ebcfc) )

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty! */

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c64-01.1",     0x000000, 0x100000, CRC(a1b4f486) SHA1(bdd6bf144e50fe7b1d4cf4504471a689669415a4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "c64-09.13",    0x00000, 0x10000, CRC(88d7f65c) SHA1(d6383bf8fd035772fa3c57b26b727eefe1aadd93) )

	ROM_REGION( 0x20000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c64-02.2",     0x00000, 0x20000, CRC(3cb0b907) SHA1(7cbe437fe584575a2f26a582095fd49665c7003e) )

	/* no Delta-T samples */

	ROM_REGION( 0x400, "plds", 0 ) // originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	ROM_LOAD( "c64-10.ic42", 0x000, 0x117, CRC(08e8c3d6) SHA1(9e1f0c8a70af7355d82c2bac249363415c2293e2) )
	ROM_LOAD( "c64-11.ic43", 0x200, 0x117, CRC(f116413e) SHA1(696a8c404ef3d6d832e0fcf530452344f96dd0b2) )
ROM_END

ROM_START( majest12j )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c64-07.10", 0x00000, 0x20000, CRC(f29ed5c9) SHA1(62283af1c08457db54057ee59a95fb7a3797b897) )
	ROM_LOAD16_BYTE( "c64-06.4",  0x40000, 0x20000, CRC(18dc71ac) SHA1(cb9c0b330ae98e20269f18cdb543feb294647245) )
	ROM_LOAD16_BYTE( "c64-08.11", 0x00001, 0x20000, CRC(ddfd33d5) SHA1(33cc5a0aedf8228b42466cd2a1fe3e06fbfbf141) )
	ROM_LOAD16_BYTE( "c64-05.5",  0x40001, 0x20000, CRC(b61866c0) SHA1(9c2096eae05782377a655c3607b65a2cd6a66272) )

	ROM_REGION( 0x100000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty! */

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c64-01.1",     0x000000, 0x100000, CRC(a1b4f486) SHA1(bdd6bf144e50fe7b1d4cf4504471a689669415a4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "c64-09.13",    0x00000, 0x10000, CRC(88d7f65c) SHA1(d6383bf8fd035772fa3c57b26b727eefe1aadd93) )

	ROM_REGION( 0x20000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c64-02.2",     0x00000, 0x20000, CRC(3cb0b907) SHA1(7cbe437fe584575a2f26a582095fd49665c7003e) )

	/* no Delta-T samples */

	ROM_REGION( 0x400, "plds", 0 ) // originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	ROM_LOAD( "c64-10.ic42", 0x000, 0x117, CRC(08e8c3d6) SHA1(9e1f0c8a70af7355d82c2bac249363415c2293e2) )
	ROM_LOAD( "c64-11.ic43", 0x200, 0x117, CRC(f116413e) SHA1(696a8c404ef3d6d832e0fcf530452344f96dd0b2) )
ROM_END


/*
Gun & Frontier
Taito, 1990

This game runs on Taito F2 hardware.


PCB Layout

K1100624A
J1100251A  (sticker K1100629A)

|-----------------------------------------------------|
|          TL074  YM3016F  C71-01.29  TC51832  TC51832|
|  MB3737  TL074                      TC51832  TC51832|
|                 YM2610   TC0530SYC                  |
|                                     TC51832  TC51832|
|   C71-12.49   Z80B  24MHz           TC51832  TC51832|
|   5563                                              |
|                                     TC51832  TC51832|
|     TC0260DAR           TC0520TBC   TC51832  TC51832|
|J              TC0360PRI                             |
|A                                    TC51832  TC51832|
|M  2088                              TC51832  TC51832|
|M                         TC0540OBN                  |
|A  TC51832                                C71-03.19  |
|                                                     |
|   TC51832                                           |
|              TC0100SCN                              |
|                    66256      66256    66256  66256 |
|   C71-02.59                                         |
|          MB3771    C71-16.38  C71-15.37             |
|   TC0510NIO        C71-10.40  C71-14.39             |
|         26.686MHz  C71-09.42  C71-08.41    68000    |
|   DSWB  DSWA                           PAL1    PAL2 |
|-----------------------------------------------------|
Notes:
      Clocks:
             68000 clock : 12.000MHz
             Z80 clock   : 4.000MHz
             YM2610 clock: 8.000MHz
             VSync       : 60Hz

      Taito Custom Chips:
                         TC0530SYC - Sound Communication  (QFP160)
                         TC0260DAR - Palette Controller   (QFP100)
                         TC0360PRI - Priority Controller  (QFP100)
                         TC0520TBC - \ Sprite Controllers (QFP100)
                         TC0540OBN - /                    (QFP160)
                         TC0100SCN - Tilemap Controller   (QFP160)
                         TC0510NIO - I/O                  (QFP100)

      RAM:
          62256  : 32K x8 SRAM
          TC51832: 32K x8 SRAM
          2088   : 8K x8 SRAM
          5563   : 8K x8 SRAM

      OTHER:
            PAL1  : PAL16L8 labelled 'C71-06' at location IC28
            PAL2  : PAL16L8 labelled 'C71-07' at location IC27
            TL074 : Quad JFET Op AMP
            MB3737: Main power AMP
*/

ROM_START( gunfront )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* 768k for 68000 code */
	ROM_LOAD16_BYTE( "c71-09.ic42",  0x00000, 0x20000, CRC(10a544a2) SHA1(3b46bbd494b432d36aed3fd4b429cef074050c1d) )
	ROM_LOAD16_BYTE( "c71-08.ic41",  0x00001, 0x20000, CRC(c17dc0a0) SHA1(f84e0d1afb403bb06480e8687558cd320d60099e) )
	ROM_LOAD16_BYTE( "c71-10.ic40",  0x40000, 0x20000, CRC(f39c0a06) SHA1(8217f0dd855d6e15756349d47f327742ab50db15) )
	ROM_LOAD16_BYTE( "c71-14.ic39",  0x40001, 0x20000, CRC(312da036) SHA1(44215c64ad9f8a4566cc9f407a7b38799a08d485) )
	ROM_LOAD16_BYTE( "c71-16.ic38",  0x80000, 0x20000, CRC(1bbcc2d4) SHA1(fe664f8d2b6d902f034cf51f42378cc68c970b53) )
	ROM_LOAD16_BYTE( "c71-15.ic37",  0x80001, 0x20000, CRC(df3e00bb) SHA1(9fe2ece7289945692099eba92f02e5a97a4d148c) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c71-02.ic59", 0x000000, 0x100000, CRC(2a600c92) SHA1(38a08ade2c6fa005a402d04fabf87ff10236d4c6) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c71-03.ic19", 0x000000, 0x100000, CRC(9133c605) SHA1(fa10c60cd4ca439a273c644bbf3810824a0ca523) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c71-12.ic49", 0x00000, 0x10000, CRC(0038c7f8) SHA1(405def36e67949219b6f9394333278ec60ad5783) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c71-01.ic29", 0x000000, 0x100000, CRC(0e73105a) SHA1(c5c9743f68a43273e16f5e5179557f2392505a1e) )

	/* no Delta-T samples */

// Pals c71-16.28  c71-07.27
ROM_END

ROM_START( gunfrontj )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* 768k for 68000 code */
	ROM_LOAD16_BYTE( "c71-09.ic42",  0x00000, 0x20000, CRC(10a544a2) SHA1(3b46bbd494b432d36aed3fd4b429cef074050c1d) )
	ROM_LOAD16_BYTE( "c71-08.ic41",  0x00001, 0x20000, CRC(c17dc0a0) SHA1(f84e0d1afb403bb06480e8687558cd320d60099e) )
	ROM_LOAD16_BYTE( "c71-10.ic40",  0x40000, 0x20000, CRC(f39c0a06) SHA1(8217f0dd855d6e15756349d47f327742ab50db15) )
	ROM_LOAD16_BYTE( "c71-11.ic39",  0x40001, 0x20000, CRC(df23c11a) SHA1(3bbe4715a022f2a78c23a7a5b8ca36ad43cdbca0) )
	ROM_LOAD16_BYTE( "c71-16.ic38",  0x80000, 0x20000, CRC(1bbcc2d4) SHA1(fe664f8d2b6d902f034cf51f42378cc68c970b53) )
	ROM_LOAD16_BYTE( "c71-15.ic37",  0x80001, 0x20000, CRC(df3e00bb) SHA1(9fe2ece7289945692099eba92f02e5a97a4d148c) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c71-02.ic59", 0x000000, 0x100000, CRC(2a600c92) SHA1(38a08ade2c6fa005a402d04fabf87ff10236d4c6) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c71-03.ic19", 0x000000, 0x100000, CRC(9133c605) SHA1(fa10c60cd4ca439a273c644bbf3810824a0ca523) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c71-12.ic49", 0x00000, 0x10000, CRC(0038c7f8) SHA1(405def36e67949219b6f9394333278ec60ad5783) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c71-01.ic29", 0x000000, 0x100000, CRC(0e73105a) SHA1(c5c9743f68a43273e16f5e5179557f2392505a1e) )

	/* no Delta-T samples */
ROM_END

ROM_START( growl )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "c74-10-1.ic59", 0x00000, 0x40000, CRC(8bf17a85) SHA1(e4aa1f6a107f7602ec25f1f3177c2541008bf1d5) ) // Rev 1 PRG ROM kit, to be applied to World, US, or JP sets
	ROM_LOAD16_BYTE( "c74-08-1.ic61", 0x00001, 0x40000, CRC(bc70396f) SHA1(04022962352e5e0d9a356bc39794b2813997e704) ) // "
	ROM_LOAD16_BYTE( "c74-11.ic58",   0x80000, 0x40000, CRC(ee3bd6d5) SHA1(71f961b4e3b3156bc52e489eb0a408a1fe537a61) )
	ROM_LOAD16_BYTE( "c74-14.ic60",   0x80001, 0x40000, CRC(b6c24ec7) SHA1(da8ac05d12c12a58bf5185d723358a0d1a0fe71e) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c74-01.ic34",   0x000000, 0x100000, CRC(3434ce80) SHA1(ef363107fba6f5088ef9c85dd692b5c98be67f75) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c74-03.ic12",   0x000000, 0x100000, CRC(1a0d8951) SHA1(62af40f7ca651273d93fed5d55af24cf91331ec7) )
	ROM_LOAD( "c74-02.ic11",   0x100000, 0x100000, CRC(15a21506) SHA1(3d8f066e1226e030ce549959d5a8dd4506a0e0a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c74-12.ic62", 0x00000, 0x10000, CRC(bb6ed668) SHA1(e8c3a15ccbc788ac57d42bd2cabcdb2db6305489) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c74-04.ic28",   0x000000, 0x100000, CRC(2d97edf2) SHA1(d3a995303facdad4f8e1fdda04eaaec4440ff371) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* Delta-T samples */
	ROM_LOAD( "c74-05.ic29",   0x000000, 0x080000, CRC(e29c0828) SHA1(f541d724f118130bb7a8f9e790582c68779cc6b6) )

//Pals c74-06.48  c74-07.47
ROM_END

ROM_START( growla )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "c74-10.ic59",  0x00000, 0x40000, CRC(ca81a20b) SHA1(75d665f3e3cf1ab389f5390d4a4d2c9e49543c56) )
	ROM_LOAD16_BYTE( "c74-08.ic61",  0x00001, 0x40000, CRC(aa35dd9e) SHA1(97229746f70c486bcf172ec09f7f9c9eede16006) )
	ROM_LOAD16_BYTE( "c74-11.ic58",  0x80000, 0x40000, CRC(ee3bd6d5) SHA1(71f961b4e3b3156bc52e489eb0a408a1fe537a61) )
	ROM_LOAD16_BYTE( "c74-14.ic60",  0x80001, 0x40000, CRC(b6c24ec7) SHA1(da8ac05d12c12a58bf5185d723358a0d1a0fe71e) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c74-01.ic34",   0x000000, 0x100000, CRC(3434ce80) SHA1(ef363107fba6f5088ef9c85dd692b5c98be67f75) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c74-03.ic12",   0x000000, 0x100000, CRC(1a0d8951) SHA1(62af40f7ca651273d93fed5d55af24cf91331ec7) )
	ROM_LOAD( "c74-02.ic11",   0x100000, 0x100000, CRC(15a21506) SHA1(3d8f066e1226e030ce549959d5a8dd4506a0e0a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c74-12.ic62", 0x00000, 0x10000, CRC(bb6ed668) SHA1(e8c3a15ccbc788ac57d42bd2cabcdb2db6305489) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c74-04.ic28",   0x000000, 0x100000, CRC(2d97edf2) SHA1(d3a995303facdad4f8e1fdda04eaaec4440ff371) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* Delta-T samples */
	ROM_LOAD( "c74-05.ic29",   0x000000, 0x080000, CRC(e29c0828) SHA1(f541d724f118130bb7a8f9e790582c68779cc6b6) )
ROM_END

ROM_START( growlu )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "c74-10.ic59",  0x00000, 0x40000, CRC(ca81a20b) SHA1(75d665f3e3cf1ab389f5390d4a4d2c9e49543c56) )
	ROM_LOAD16_BYTE( "c74-08.ic61",  0x00001, 0x40000, CRC(aa35dd9e) SHA1(97229746f70c486bcf172ec09f7f9c9eede16006) )
	ROM_LOAD16_BYTE( "c74-11.ic58",  0x80000, 0x40000, CRC(ee3bd6d5) SHA1(71f961b4e3b3156bc52e489eb0a408a1fe537a61) )
	ROM_LOAD16_BYTE( "c74-13.ic60",  0x80001, 0x40000, CRC(c1c57e51) SHA1(f6ffc1acf3e5ff75e64facd7e28fd675eb065c7b) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c74-01.ic34",   0x000000, 0x100000, CRC(3434ce80) SHA1(ef363107fba6f5088ef9c85dd692b5c98be67f75) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c74-03.ic12",   0x000000, 0x100000, CRC(1a0d8951) SHA1(62af40f7ca651273d93fed5d55af24cf91331ec7) )
	ROM_LOAD( "c74-02.ic11",   0x100000, 0x100000, CRC(15a21506) SHA1(3d8f066e1226e030ce549959d5a8dd4506a0e0a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c74-12.ic62", 0x00000, 0x10000, CRC(bb6ed668) SHA1(e8c3a15ccbc788ac57d42bd2cabcdb2db6305489) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c74-04.ic28",   0x000000, 0x100000, CRC(2d97edf2) SHA1(d3a995303facdad4f8e1fdda04eaaec4440ff371) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* Delta-T samples */
	ROM_LOAD( "c74-05.ic29",   0x000000, 0x080000, CRC(e29c0828) SHA1(f541d724f118130bb7a8f9e790582c68779cc6b6) )
ROM_END

ROM_START( runark )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "c74-10.ic59",  0x00000, 0x40000, CRC(ca81a20b) SHA1(75d665f3e3cf1ab389f5390d4a4d2c9e49543c56) )
	ROM_LOAD16_BYTE( "c74-08.ic61",  0x00001, 0x40000, CRC(aa35dd9e) SHA1(97229746f70c486bcf172ec09f7f9c9eede16006) )
	ROM_LOAD16_BYTE( "c74-11.ic58",  0x80000, 0x40000, CRC(ee3bd6d5) SHA1(71f961b4e3b3156bc52e489eb0a408a1fe537a61) )
	ROM_LOAD16_BYTE( "c74-09.ic14",  0x80001, 0x40000, CRC(58cc2feb) SHA1(7dc314a56345be116731dbb65dfa9e508c156513) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c74-01.ic34",   0x000000, 0x100000, CRC(3434ce80) SHA1(ef363107fba6f5088ef9c85dd692b5c98be67f75) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c74-03.ic12",   0x000000, 0x100000, CRC(1a0d8951) SHA1(62af40f7ca651273d93fed5d55af24cf91331ec7) )
	ROM_LOAD( "c74-02.ic11",   0x100000, 0x100000, CRC(15a21506) SHA1(3d8f066e1226e030ce549959d5a8dd4506a0e0a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c74-12.ic62", 0x00000, 0x10000, CRC(bb6ed668) SHA1(e8c3a15ccbc788ac57d42bd2cabcdb2db6305489) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c74-04.ic28",   0x000000, 0x100000, CRC(2d97edf2) SHA1(d3a995303facdad4f8e1fdda04eaaec4440ff371) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* Delta-T samples */
	ROM_LOAD( "c74-05.ic29",   0x000000, 0x080000, CRC(e29c0828) SHA1(f541d724f118130bb7a8f9e790582c68779cc6b6) )
ROM_END


ROM_START( growlp )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "growl_ic15_japan_0h_fb09.bin",   0x00000, 0x40000, CRC(3a9141dc) SHA1(5b0f6cd39a8964ba9a3fcca587565b37e45b24ac) )
	ROM_LOAD16_BYTE( "growl_ic13_japan_0l_a80a.bin",   0x00001, 0x40000, CRC(a8547fd6) SHA1(2c5cd70b23e03af80c6f72600dd5eef2c7b3724d) )
	ROM_LOAD16_BYTE( "growl_ic16_japan_1h_41bb.bin",   0x80000, 0x40000, CRC(64aa6f4b) SHA1(fd3f838cedee99d86a2ee5ff87f0381d164a2f90) )
	ROM_LOAD16_BYTE( "growl_ic14_europe_1l_726b.bin",  0x80001, 0x40000, CRC(c38bbb05) SHA1(a168f18ce903d9af295f475760f7209f6e0f8c82) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_BYTE( "growl_ic11_scro-0-l_a971.bin",    0x00001, 0x40000, CRC(769ddaab) SHA1(ab45547438998ccdf8cdb8123f6f9102008dd291) )
	ROM_LOAD16_BYTE( "growl_ic13_scro-0-0-h_2e7a.bin",  0x00000, 0x40000, CRC(4e220e34) SHA1(c0d69b178b3e2dd004295eee92311b387733228b) )
	ROM_LOAD16_BYTE( "growl_ic12_scro-1-l_026e.bin",    0x80001, 0x40000, CRC(486925b4) SHA1(7d804e45ab58a4b1a121438d354a4a294daa5177) )
	ROM_LOAD16_BYTE( "growl_ic14_scro-0-1-h_f0fa.bin",  0x80000, 0x40000, CRC(42c2a2d0) SHA1(638d563a3928b7b2e6dae7f8c607669116284899) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "growl_ic17_obj0-0-l_90b9.bin",  0x000000, 0x40000, CRC(cd94025a) SHA1(0da7a0f213f9a9d7a420a9dcdd6d3db2e4b6a9ef) )
	ROM_LOAD16_BYTE( "growl_ic19_obj0-0-h_b652.bin",  0x000001, 0x40000, CRC(6838c1b0) SHA1(0355bff010ab94f9d566e6bd404bd3559c407a37) )
	ROM_LOAD16_BYTE( "growl_ic18_obj0-1-l_a299.bin",  0x080000, 0x40000, CRC(0ddf592e) SHA1(2b630143dd52da6068aace2c3a02c89432d45044) )
	ROM_LOAD16_BYTE( "growl_ic20_obj0-1-h_9f1a.bin",  0x080001, 0x40000, CRC(0f0407f1) SHA1(05e897445d501d93a0aba6bc9dae1598e651df8d) )
	ROM_LOAD16_BYTE( "growl_ic4_obj1-l_7d96.bin",     0x100000, 0x40000, CRC(bed51bd6) SHA1(1a35b5a511261b70b5a4aece665d0a0e6386af78) )
	ROM_LOAD16_BYTE( "growl_ic6_obj1-0-h_3a22.bin",   0x100001, 0x40000, CRC(5b696d20) SHA1(e01e2d150ad61cf1cdddfcfcec9dac0980a8f0d6) )
	ROM_LOAD16_BYTE( "growl_ic5_obj1-1-l_d34f.bin",   0x180000, 0x40000, CRC(f34d83ec) SHA1(7b032d2c004735aa4658143fe399b0a05ab85ec8) )
	ROM_LOAD16_BYTE( "growl_ic7_obj1-1-h_b5af.bin",   0x180001, 0x40000, CRC(e9fda1fa) SHA1(37d48d26c45aca6364d74656d597c2b1f8a5c685) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "growl_ic3_snd.bin",   0x00000, 0x10000, CRC(f75929e0) SHA1(2dc278f4253d76853bbc3af099784545cfac65ce) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "growl_ic23_ch-a-0_b5d9.bin",   0x00000, 0x40000, CRC(cc9ffbf8) SHA1(834892ce0d40d1b016d202144e683fe98ef374ac) )
	ROM_LOAD( "growl_ic24_ch-a-1_3c70.bin",   0x40000, 0x40000, CRC(7177b4ad) SHA1(4603893588aba0261b9d50bf70f46c7dad8592a2) )
	ROM_LOAD( "growl_ic25_ch-a-2_9614.bin",   0x80000, 0x40000, CRC(7c9b1423) SHA1(a8d0d7340bf54d9792e21bd0e4be5edd1bcfbbd4) )
	ROM_LOAD( "growl_ic26_ch-a-3_fca6.bin",   0xc0000, 0x40000, CRC(db1ecefe) SHA1(7f7c40a9c9aceb41ba799249e57df5f38715d571) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* Delta-T samples */
	ROM_LOAD( "growl_ic21_ch-b-0_d743.bin",   0x00000, 0x40000, CRC(7a7eab62) SHA1(268d900f84162ba655cb652a1c1865dfc25da4de) )
	ROM_LOAD( "growl_ic22_ch-b-1_a5f1.bin",   0x40000, 0x40000, CRC(567df833) SHA1(1ad019a9f938ebe2f09bc68b57b6c2623ecd9f46) )
ROM_END

ROM_START( mjnquest )   /* Mahjong Quest */
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c77-09",  0x000000, 0x020000, CRC(0a005d01) SHA1(caf44fcdeca9f7f1bfdb0c29503bb82ce17f945c) )
	ROM_LOAD16_BYTE( "c77-08",  0x000001, 0x020000, CRC(4244f775) SHA1(801045d7433684195876e172676b2345827de7cc) )
	ROM_LOAD16_WORD_SWAP( "c77-04",  0x080000, 0x080000, CRC(c2e7e038) SHA1(6cf23bab587b34cbc1f78b8b82cbab5634b074dc) ) /* data rom */

	ROM_REGION( 0x200000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c77-01", 0x000000, 0x100000, CRC(5ba51205) SHA1(da0b6f56e7d2437ad75ada1ba07e35843d2b4704) )
	ROM_LOAD16_WORD_SWAP( "c77-02", 0x100000, 0x100000, CRC(6a6f3040) SHA1(61c3ce51fa935f52572affa899bb81b3a616df3a) )

	ROM_REGION( 0x080000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c77-05", 0x00000, 0x80000, CRC(c5a54678) SHA1(d0954acbdfdf9a5f14f554635c015eee23d52e0c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c77-10",    0x00000, 0x10000, CRC(f16b2c1e) SHA1(f6f13429a0b4e0e4c64991f1acc4ecf85dc64364) )

	ROM_REGION( 0x080000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c77-03",  0x000000, 0x080000, CRC(312f17b1) SHA1(3b45eeb6c6721c532451cb113e6a38da4d8a8cbf) )

	/* no Delta-T samples */
ROM_END

ROM_START( mjnquestb )  /* Mahjong Quest (No Nudity) */
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c77-09a", 0x000000, 0x020000, CRC(fc17f1c2) SHA1(438b0a780560940b2d3dab42b34dc2bc94a15f80) )
	ROM_LOAD16_BYTE( "c77-08",  0x000001, 0x020000, CRC(4244f775) SHA1(801045d7433684195876e172676b2345827de7cc) )
	ROM_LOAD16_WORD_SWAP( "c77-04",  0x080000, 0x080000, CRC(c2e7e038) SHA1(6cf23bab587b34cbc1f78b8b82cbab5634b074dc) ) /* data rom */

	ROM_REGION( 0x200000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c77-01", 0x000000, 0x100000, CRC(5ba51205) SHA1(da0b6f56e7d2437ad75ada1ba07e35843d2b4704) )
	ROM_LOAD16_WORD_SWAP( "c77-02", 0x100000, 0x100000, CRC(6a6f3040) SHA1(61c3ce51fa935f52572affa899bb81b3a616df3a) )

	ROM_REGION( 0x080000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c77-05", 0x00000, 0x80000, CRC(c5a54678) SHA1(d0954acbdfdf9a5f14f554635c015eee23d52e0c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c77-10",    0x00000, 0x10000, CRC(f16b2c1e) SHA1(f6f13429a0b4e0e4c64991f1acc4ecf85dc64364) )

	ROM_REGION( 0x080000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c77-03",  0x000000, 0x080000, CRC(312f17b1) SHA1(3b45eeb6c6721c532451cb113e6a38da4d8a8cbf) )

	/* no Delta-T samples */
ROM_END

ROM_START( footchmp )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c80-11.6", 0x00000, 0x20000, CRC(f78630fb) SHA1(37da34401f664caaf5113a9abad78e447f4f4651) )
	ROM_LOAD16_BYTE( "c80-10.4", 0x00001, 0x20000, CRC(32c109cb) SHA1(46a116127bcea18cc15ddf297e5e0d5cdcac9842) )
	ROM_LOAD16_BYTE( "c80-12.7", 0x40000, 0x20000, CRC(80d46fef) SHA1(cc81c8ba19321e8bae9054021bfb61cb11c2aba5) )
	ROM_LOAD16_BYTE( "c80-14.5", 0x40001, 0x20000, CRC(40ac4828) SHA1(9a2112b0ccd573a3e94d9817b78bb02909b972e1) ) // identical rom also found with c80-13 sticker, is this correct?

	ROM_REGION( 0x100000, "tc0480scp", 0 )   /* SCR */
	ROM_LOAD32_WORD( "c80-04.1", 0x00000, 0x80000, CRC(9a17fe8c) SHA1(d2ea72743151f0f7bf78f33dba526214afb07389) )
	ROM_LOAD32_WORD( "c80-05.2", 0x00002, 0x80000, CRC(acde7071) SHA1(23637238d122b13edb6025418bf482cc210ef6a9) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c80-01.9",  0x000000, 0x100000, CRC(f43782e6) SHA1(53ff6cc433673f307a91e8db74428aa6172ffad4) )
	ROM_LOAD( "c80-02.10", 0x100000, 0x100000, CRC(060a8b61) SHA1(b1888d8bce4c4624dc5bb64168c604ec64537c0e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for Z80 code */
	ROM_LOAD( "c80-15.70", 0x00000, 0x10000, CRC(05aa7fd7) SHA1(7eb10964ea9f43abcda8444f13733a0753a04580) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )     /* YM2610 samples */
	ROM_LOAD( "c80-03.57", 0x000000, 0x100000, CRC(609938d5) SHA1(54c7a7265dee5cb031fd402f4c74858d73bec652) )

	/* no Delta-T samples */

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "c80-08.ic45", 0x000, 0x104, CRC(6137dd15) SHA1(fb20df118da69b52767d5e6c81fa85174c50c5c5) ) // pal16l8bcn
	ROM_LOAD( "c80-09.ic46", 0x200, 0x104, CRC(4ca48869) SHA1(8ba46ab625d3cbf3431ca2e6f3a9207d402202ef) ) // pal16l8bcn
ROM_END

ROM_START( htherou )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "c80-11.6", 0x00000, 0x20000, CRC(f78630fb) SHA1(37da34401f664caaf5113a9abad78e447f4f4651) )
	ROM_LOAD16_BYTE( "c80-10.4", 0x00001, 0x20000, CRC(32c109cb) SHA1(46a116127bcea18cc15ddf297e5e0d5cdcac9842) )
	ROM_LOAD16_BYTE( "c80-12.7", 0x40000, 0x20000, CRC(80d46fef) SHA1(cc81c8ba19321e8bae9054021bfb61cb11c2aba5) )
	ROM_LOAD16_BYTE( "c80-13.5", 0x40001, 0x20000, CRC(37ab78be) SHA1(d258420cdf88f023577276c8abb54934d375b38f) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )   // SCR
	ROM_LOAD32_WORD( "c80-04.1", 0x00000, 0x80000, CRC(9a17fe8c) SHA1(d2ea72743151f0f7bf78f33dba526214afb07389) )
	ROM_LOAD32_WORD( "c80-05.2", 0x00002, 0x80000, CRC(acde7071) SHA1(23637238d122b13edb6025418bf482cc210ef6a9) )

	ROM_REGION( 0x200000, "sprites", 0 )   // OBJ
	ROM_LOAD( "c80-01.9",  0x000000, 0x100000, CRC(f43782e6) SHA1(53ff6cc433673f307a91e8db74428aa6172ffad4) )
	ROM_LOAD( "c80-02.10", 0x100000, 0x100000, CRC(060a8b61) SHA1(b1888d8bce4c4624dc5bb64168c604ec64537c0e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for Z80 code
	ROM_LOAD( "c80-15.70", 0x00000, 0x10000, CRC(05aa7fd7) SHA1(7eb10964ea9f43abcda8444f13733a0753a04580) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )     // YM2610 samples
	ROM_LOAD( "c80-03.57", 0x000000, 0x100000, CRC(609938d5) SHA1(54c7a7265dee5cb031fd402f4c74858d73bec652) )

	// no Delta-T samples

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "c80-08.ic45", 0x000, 0x104, CRC(6137dd15) SHA1(fb20df118da69b52767d5e6c81fa85174c50c5c5) ) // pal16l8bcn
	ROM_LOAD( "c80-09.ic46", 0x200, 0x104, CRC(4ca48869) SHA1(8ba46ab625d3cbf3431ca2e6f3a9207d402202ef) ) // pal16l8bcn
ROM_END

ROM_START( htheroj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c80-16.6", 0x00000, 0x20000, CRC(4e795b52) SHA1(90a32133a68de4d0410935e5039d4dec37836a13) )
	ROM_LOAD16_BYTE( "c80-17.4", 0x00001, 0x20000, CRC(42c0a838) SHA1(4ba96a7248715562668994a4bf974e8ce4c44fd3) )
	ROM_LOAD16_BYTE( "c80-12.7", 0x40000, 0x20000, CRC(80d46fef) SHA1(cc81c8ba19321e8bae9054021bfb61cb11c2aba5) )
	ROM_LOAD16_BYTE( "c80-18.5", 0x40001, 0x20000, CRC(aea22904) SHA1(907889f141fced8cada793f02244e80cf0f89c81) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )   /* SCR */
	ROM_LOAD32_WORD( "c80-04.1", 0x00000, 0x80000, CRC(9a17fe8c) SHA1(d2ea72743151f0f7bf78f33dba526214afb07389) )
	ROM_LOAD32_WORD( "c80-05.2", 0x00002, 0x80000, CRC(acde7071) SHA1(23637238d122b13edb6025418bf482cc210ef6a9) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c80-01.9",  0x000000, 0x100000, CRC(f43782e6) SHA1(53ff6cc433673f307a91e8db74428aa6172ffad4) )
	ROM_LOAD( "c80-02.10", 0x100000, 0x100000, CRC(060a8b61) SHA1(b1888d8bce4c4624dc5bb64168c604ec64537c0e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c80-15.70", 0x00000, 0x10000, CRC(05aa7fd7) SHA1(7eb10964ea9f43abcda8444f13733a0753a04580) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c80-03.57", 0x000000, 0x100000, CRC(609938d5) SHA1(54c7a7265dee5cb031fd402f4c74858d73bec652) )

	/* no Delta-T samples */
ROM_END

ROM_START( euroch92 )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "ec92_25.rom", 0x00000, 0x20000, CRC(98482202) SHA1(4fc03fb2a2c21f302d95047535f66d26421dcda2) )
	ROM_LOAD16_BYTE( "ec92_23.rom", 0x00001, 0x20000, CRC(ae5e75e9) SHA1(82d935684182bfb42367232a3b71d4664b170ffe) )
	ROM_LOAD16_BYTE( "ec92_26.rom", 0x40000, 0x20000, CRC(b986ccb2) SHA1(862a5da1bd4e8743d55f2e5bab2ade6c3dec682c) )
	ROM_LOAD16_BYTE( "ec92_24.rom", 0x40001, 0x20000, CRC(b31d94ac) SHA1(8a3328b7e061b584992dd27b0dda9826b4b6ed91) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )   /* SCR */
	ROM_LOAD32_WORD( "ec92_21.rom", 0x00000, 0x80000, CRC(5759ed37) SHA1(2a661ea40735afbda3d0141ce3f706c64281097b) )
	ROM_LOAD32_WORD( "ec92_22.rom", 0x00002, 0x80000, CRC(d9a0d38e) SHA1(192f0303f4f64df46dc20701ed4362a4e14e40e7) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "ec92_19.rom", 0x000000, 0x100000, CRC(219141a5) SHA1(b549e91049dcb796d4104b4426674dd87589efde) )
	ROM_LOAD( "c80-02.10",   0x100000, 0x100000, CRC(060a8b61) SHA1(b1888d8bce4c4624dc5bb64168c604ec64537c0e) ) // ec92_20.rom

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for Z80 code */
	ROM_LOAD( "ec92_27.rom", 0x00000, 0x10000, CRC(2db48e65) SHA1(43a47ebc91c043a996e966cf808d71256e158494) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* YM2610 samples */
	ROM_LOAD( "c80-03.57", 0x000000, 0x100000, CRC(609938d5) SHA1(54c7a7265dee5cb031fd402f4c74858d73bec652) )   // ec92_03.rom

	/* no Delta-T samples */
ROM_END

ROM_START( euroch92j )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "c80-25.ic6", 0x00000, 0x20000, CRC(98482202) SHA1(4fc03fb2a2c21f302d95047535f66d26421dcda2) )
	ROM_LOAD16_BYTE( "c80-23.ic4", 0x00001, 0x20000, CRC(ae5e75e9) SHA1(82d935684182bfb42367232a3b71d4664b170ffe) )
	ROM_LOAD16_BYTE( "c80-26.ic7", 0x40000, 0x20000, CRC(b986ccb2) SHA1(862a5da1bd4e8743d55f2e5bab2ade6c3dec682c) )
	ROM_LOAD16_BYTE( "c80-28.ic5", 0x40001, 0x20000, CRC(5d13f580) SHA1(402aad6ece4d735b0770bf64b80a08313abc3ac4) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )   // SCR
	ROM_LOAD32_WORD( "c80-21.ic1", 0x00000, 0x80000, CRC(5759ed37) SHA1(2a661ea40735afbda3d0141ce3f706c64281097b) )
	ROM_LOAD32_WORD( "c80-22.ic2", 0x00002, 0x80000, CRC(d9a0d38e) SHA1(192f0303f4f64df46dc20701ed4362a4e14e40e7) )

	ROM_REGION( 0x200000, "sprites", 0 )   // OBJ
	ROM_LOAD( "c80-19.ic9",  0x000000, 0x100000, CRC(219141a5) SHA1(b549e91049dcb796d4104b4426674dd87589efde) )
	ROM_LOAD( "c80-20.ic10", 0x100000, 0x100000, CRC(060a8b61) SHA1(b1888d8bce4c4624dc5bb64168c604ec64537c0e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for Z80 code
	ROM_LOAD( "c80-27.ic70", 0x00000, 0x10000, CRC(2db48e65) SHA1(43a47ebc91c043a996e966cf808d71256e158494) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  // YM2610 samples
	ROM_LOAD( "c80-03.ic57", 0x000000, 0x100000, CRC(609938d5) SHA1(54c7a7265dee5cb031fd402f4c74858d73bec652) )

	// no Delta-T samples
ROM_END

ROM_START( footchmpbl )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "cpu7.rom10", 0x00000, 0x40000, CRC(b2297030) SHA1(1ae3c9395a108c9f4b65173365a7b8934af4230c) )
	ROM_LOAD16_BYTE( "cpu8.rom11", 0x00001, 0x40000, CRC(741dacac) SHA1(f9b385da715b47bf88d5b0eaffebedacfc3e7913) )

	// there is twice as much data here as the original sets because the 2nd half of the roms contain flipped
	// versions of the tiles!
	ROM_REGION( 0x200000, "tc0480scp", 0 )   /* SCR */
	ROM_LOAD( "bk4.rom9", 0x000000, 0x80000, CRC(264e6ec0) SHA1(1907dab063b16f5c9de753b2d9a916f1c7d32079) )
	ROM_LOAD( "bk3.rom8", 0x080000, 0x80000, CRC(380b2565) SHA1(9d83d402a138786bad61d62722953dfdb98a80de) )
	ROM_LOAD( "bk2.rom7", 0x100000, 0x80000, CRC(79ce5b01) SHA1(454a9b8ca5178418c7e0976efb78cb883c553476) )
	ROM_LOAD( "bk1.rom6", 0x180000, 0x80000, CRC(6e4757c7) SHA1(a412bdb09440f80e0e323c297a79b143a9bdb7f2) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "ob17.rom1",  0x000000, 0x80000, CRC(a4891065) SHA1(69469e4c68ff72d3e50e3e74f5ca7b55e3a0bd6f) )
	ROM_LOAD( "ob16.rom2",  0x080000, 0x80000, CRC(3b65028b) SHA1(b2bb123b067b40ddacd90f82e42dff775e7e66c7) )
	ROM_LOAD( "ob15.rom3",  0x100000, 0x80000, CRC(dbd07328) SHA1(9d658f0a0ad119ae76a6c4749651440642ee671c) )
	ROM_LOAD( "ob14.rom4",  0x180000, 0x80000, CRC(28fcaefa) SHA1(f92a19dc24d5faac57a5934e7001e5b0bf9d847c) )

	// ?? more gfx? - should it ignore the uploaded text data and use these?
	ROM_REGION( 0x40000, "tc0480scp:textrom", 0 )    /* SCR */
	ROM_LOAD( "bk33.rom16", 0x000000, 0x10000, CRC(07a371fe) SHA1(27e7ba4ed7f0868206c9d7ca653322ca73929567) )
	ROM_LOAD( "bk32.rom15", 0x010000, 0x10000, CRC(89020973) SHA1(30174e504734a851a016acf0746d726981edb8f1) )
	ROM_LOAD( "bk31.rom14", 0x020000, 0x10000, CRC(02a0de4f) SHA1(7446d75608126e3d5693913e5dcb5636ae1e5500) )
	ROM_LOAD( "bk30.rom13", 0x030000, 0x10000, CRC(71cbe2b2) SHA1(2bbd19f0d4fb0adce29a66bf27e081b86853431a) )

	// sound hw is clearly different too, the sound program only contains 0x800 worth of code at 0x7800!
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "so7.rom12", 0x00000, 0x07800, CRC(5bf4ca7a) SHA1(dd8b9008eaeef2792cc07828d10b44d9b3e10508) )
	ROM_CONTINUE(          0x00000, 0x00800 )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )     /* samples (oki?) */
	ROM_LOAD( "so3.rom5", 0x000000, 0x80000, CRC(f00fca27) SHA1(9a3c8f99c35604a3df43cc8ff55f50407a358cd7) )
ROM_END


ROM_START( koshien )    /* Ah Eikou no Koshien */
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c81-11.bin", 0x000000, 0x020000, CRC(b44ea8c9) SHA1(f1d19f531b7a653f1c4244d612a339d95ce8cc7c) )
	ROM_LOAD16_BYTE( "c81-10.bin", 0x000001, 0x020000, CRC(8f98c40a) SHA1(f9471306c47ced10a56c09794954e55fdb6f6b85) )
	ROM_LOAD16_WORD_SWAP( "c81-04.bin", 0x080000, 0x080000, CRC(1592b460) SHA1(d42514b4d588d0376914832f0e07ce626d1cdee0) )  /* data rom */

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c81-03.bin", 0x000000, 0x100000, CRC(29bbf492) SHA1(bd370b1de256a432821b443a6653aab8507fb3a7) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c81-01.bin", 0x000000, 0x100000, CRC(64b15d2a) SHA1(18b3b405f77ad80781e3fce4ef021ba49f707ed6) )
	ROM_LOAD( "c81-02.bin", 0x100000, 0x100000, CRC(962461e8) SHA1(cb0313b00681c36110eed50eae41ad98eb22205d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c81-12.bin", 0x00000, 0x10000, CRC(6e8625b6) SHA1(212d384aa6ed43f5389739863afecbf0ad68af14) )

	ROM_REGION( 0x080000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c81-05.bin",  0x00000, 0x80000, CRC(9c3d71be) SHA1(79f1bb40d8356d9fc93b569c20be15e7fbf34580) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* Delta-T samples */
	ROM_LOAD( "c81-06.bin",  0x00000, 0x80000, CRC(927833b4) SHA1(c09240e4885d2eace1c64fa6425faeeea0296d98) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8b-c81-07.bin", 0x0000, 0x0104, CRC(46341732) SHA1(af652621cb96f656fd1f9ed20daeb076641aeb08) )
	ROM_LOAD( "pal16l8b-c81-08.bin", 0x0200, 0x0104, CRC(e7d2d300) SHA1(20a1a258230d5be73381c97509437a9e76de958c) )
	ROM_LOAD( "pal16l8b-c81-09.bin", 0x0400, 0x0104, CRC(e4c012a1) SHA1(56746ce42fd64bf04e17132811de291a1bfa5451) )
ROM_END

ROM_START( yuyugogo )   /* Yuuyu no QUIZ de GO!GO! */
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c83-10.3",  0x00000,  0x20000, CRC(4d185d03) SHA1(ab494c777a0acfe088d3db7c1a0488d40884ea0a) )
	ROM_LOAD16_BYTE( "c83-09.2",  0x00001,  0x20000, CRC(f9892792) SHA1(310d02833c821a34baa0ce537019b8cade6402b7) )

	ROM_REGION16_BE( 0x100000, "extra", 0 )
	/* extra ROM mapped at d00000 */
	ROM_LOAD16_WORD_SWAP( "c83-03.10", 0x000000, 0x100000, CRC(eed9acc2) SHA1(baa6a9aa5ed8fbbff7b289693407192b464cb7c7) )   /* data rom */

	ROM_REGION( 0x020000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD( "c83-05.21", 0x00000, 0x20000, CRC(eca57fb1) SHA1(c67b5e734f5fd8801a4376c0555ce4a891dcd6bc) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "c83-01.12",  0x000000, 0x100000, CRC(8bf0d416) SHA1(ec3d51422fcc8e3e822716d57adab56f639a2d02) )
	ROM_LOAD16_BYTE( "c83-02.11",  0x000001, 0x100000, CRC(20bb1c15) SHA1(33370b665d681f765e363e5a8e79e62f59ad25f7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c83-11.1"  , 0x00000, 0x10000, CRC(461e702a) SHA1(f1f4294a7e8acc50473df15a167ba84595c9eb15) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c83-04.5",  0x000000, 0x100000, CRC(2600093a) SHA1(824fde078e0ded58037bd06f888eac4e7487ac82) )

	/* no Delta-T samples */
ROM_END

ROM_START( ninjak )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c85-10x.ic50",  0x00000, 0x20000, CRC(ba7e6e74) SHA1(d6fc7529f1df348fedc6f32edd7691a6da35423d) ) /* ROM 0H */
	ROM_LOAD16_BYTE( "c85-13x.ic49",  0x00001, 0x20000, CRC(0ac2cba2) SHA1(18e1b458995051c8d4a0077a41fa24dbaa318afc) ) /* ROM 0L */
	ROM_LOAD16_BYTE( "c85-07.ic48",   0x40000, 0x20000, CRC(3eccfd0a) SHA1(54f20d852ab88a19e9ddee95b9c6057887a8d433) ) /* ROM 1LH */
	ROM_LOAD16_BYTE( "c85-06.ic47",   0x40001, 0x20000, CRC(d126ded1) SHA1(fc4b6504d0234e7e006a63e33fd061411f008c38) ) /* ROM 1LL */
	/* IC45 (ROM 1HL) and IC46 (1HH) not populated */

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c85-03.ic65",    0x00000, 0x80000, CRC(4cc7b9df) SHA1(aaf0e587b86a7bf9fbfd4e19127a7295da8a3676) ) /* SCREEN */

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c85-01.ic19",    0x000000, 0x100000, CRC(a711977c) SHA1(b6b79ff6086a7b6e242fe26eec448f025ab431af) ) /* OBJ-0 */
	ROM_LOAD( "c85-02.ic17",    0x100000, 0x100000, CRC(a6ad0f3d) SHA1(6d3a661807dd64f0b56ae6252a7e980fd678feef) ) /* OBJ-1 */

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c85-14.ic54",     0x00000, 0x10000, CRC(f2a52a51) SHA1(951793c65a3436a7fb36f3058bc7a3b4265a90bb) ) /* SND */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c85-04.ic31",    0x00000, 0x80000, CRC(5afb747e) SHA1(e4f03582221f3a97f0e24693aa77264663eb1b47) ) /* SCH-B */

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c85-05.ic33",    0x00000, 0x80000, CRC(3c1b0ed0) SHA1(08920dc55d695debae3eea5a8ff1d17bb11afd45) ) /* SCH-A */
ROM_END

ROM_START( ninjakj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c85-10x.ic50",  0x00000, 0x20000, CRC(ba7e6e74) SHA1(d6fc7529f1df348fedc6f32edd7691a6da35423d) ) /* ROM 0H */
	ROM_LOAD16_BYTE( "c85-11x.ic49",  0x00001, 0x20000, CRC(e4ccaa8e) SHA1(6e1b9dade70e3d91071a3ea590537232bb1215fe) ) /* ROM 0L */
	ROM_LOAD16_BYTE( "c85-07.ic48",   0x40000, 0x20000, CRC(3eccfd0a) SHA1(54f20d852ab88a19e9ddee95b9c6057887a8d433) ) /* ROM 1LH */
	ROM_LOAD16_BYTE( "c85-06.ic47",   0x40001, 0x20000, CRC(d126ded1) SHA1(fc4b6504d0234e7e006a63e33fd061411f008c38) ) /* ROM 1LL */
	/* IC45 (ROM 1HL) and IC46 (1HH) not populated */

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c85-03.ic65",    0x00000, 0x80000, CRC(4cc7b9df) SHA1(aaf0e587b86a7bf9fbfd4e19127a7295da8a3676) ) /* SCREEN */

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c85-01.ic19",    0x000000, 0x100000, CRC(a711977c) SHA1(b6b79ff6086a7b6e242fe26eec448f025ab431af) ) /* OBJ-0 */
	ROM_LOAD( "c85-02.ic17",    0x100000, 0x100000, CRC(a6ad0f3d) SHA1(6d3a661807dd64f0b56ae6252a7e980fd678feef) ) /* OBJ-1 */

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c85-14.ic54",     0x00000, 0x10000, CRC(f2a52a51) SHA1(951793c65a3436a7fb36f3058bc7a3b4265a90bb) ) /* SND */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c85-04.ic31",    0x00000, 0x80000, CRC(5afb747e) SHA1(e4f03582221f3a97f0e24693aa77264663eb1b47) ) /* SCH-B */

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c85-05.ic33",    0x00000, 0x80000, CRC(3c1b0ed0) SHA1(08920dc55d695debae3eea5a8ff1d17bb11afd45) ) /* SCH-A */
ROM_END

ROM_START( ninjaku )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c85-15x.ic50",  0x00000, 0x20000, CRC(719a481b) SHA1(aacd5b3db29e3771bbeb89c9502c49eba1538741) ) /* ROM 0H */
	ROM_LOAD16_BYTE( "c85-12x.ic49",  0x00001, 0x20000, CRC(6c3f7e1e) SHA1(919177bf3522717864850d4130867ecb0ecfc9c7) ) /* ROM 0L */
	ROM_LOAD16_BYTE( "c85-07.ic48",   0x40000, 0x20000, CRC(3eccfd0a) SHA1(54f20d852ab88a19e9ddee95b9c6057887a8d433) ) /* ROM 1LH */
	ROM_LOAD16_BYTE( "c85-06.ic47",   0x40001, 0x20000, CRC(d126ded1) SHA1(fc4b6504d0234e7e006a63e33fd061411f008c38) ) /* ROM 1LL */
	/* IC45 (ROM 1HL) and IC46 (1HH) not populated */

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c85-03.ic65",    0x00000, 0x80000, CRC(4cc7b9df) SHA1(aaf0e587b86a7bf9fbfd4e19127a7295da8a3676) ) /* SCREEN */

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c85-01.ic19",    0x000000, 0x100000, CRC(a711977c) SHA1(b6b79ff6086a7b6e242fe26eec448f025ab431af) ) /* OBJ-0 */
	ROM_LOAD( "c85-02.ic17",    0x100000, 0x100000, CRC(a6ad0f3d) SHA1(6d3a661807dd64f0b56ae6252a7e980fd678feef) ) /* OBJ-1 */

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c85-14.ic54",     0x00000, 0x10000, CRC(f2a52a51) SHA1(951793c65a3436a7fb36f3058bc7a3b4265a90bb) ) /* SND */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c85-04.ic31",    0x00000, 0x80000, CRC(5afb747e) SHA1(e4f03582221f3a97f0e24693aa77264663eb1b47) ) /* SCH-B */

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c85-05.ic33",    0x00000, 0x80000, CRC(3c1b0ed0) SHA1(08920dc55d695debae3eea5a8ff1d17bb11afd45) ) /* SCH-A */
ROM_END

ROM_START( solfigtr )   /* Solitary Fighter */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c91-05.59",  0x00000, 0x40000, CRC(c1260e7c) SHA1(406663b8f92380f37955061765d77a92dc51c17a) )
	ROM_LOAD16_BYTE( "c91-09.61",  0x00001, 0x40000, CRC(d82b5266) SHA1(670dc91067dd856b5d36b71e5ddf67e82220d83b) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c91-03.34", 0x000000, 0x100000, CRC(8965da12) SHA1(b06f3fc91c9ce1e20cb4187505f3c7921c54cc12) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c91-01.12", 0x000000, 0x100000, CRC(0f3f4e00) SHA1(5912eddc066435b276d615842d123f58c4852b2b) )
	ROM_LOAD( "c91-02.11", 0x100000, 0x100000, CRC(e14ab98e) SHA1(b2e559ec9ccf383e693b27436081c29d30f17835) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c91-07.62", 0x00000, 0x10000, CRC(e471a05a) SHA1(4d9c2b734aac27819673094dc1843e1ca5fe6994) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c91-04.28", 0x00000, 0x80000, CRC(390b1065) SHA1(ee7ba77634854e4896528ff1bbc308de92a5815b) ) /* Channel A */

	/* no Delta-T samples */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16l8b-c74-06.ic48", 0x0000, 0x0104, CRC(c868dc16) SHA1(b9d12f2016c6a3017b497aafc96a7dbab4a50c8b) )
ROM_END

ROM_START( qzquest )    /* Quiz Quest */
	ROM_REGION( 0x180000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c92-06.8", 0x000000, 0x020000, CRC(424be722) SHA1(ec064028bd68e8e2ec37705cab1c79c963188944) )
	ROM_LOAD16_BYTE( "c92-05.7", 0x000001, 0x020000, CRC(da470f93) SHA1(dc6d2142fc5662f4b3ffbe4cc9adb5b394d958ed) )
	ROM_LOAD16_WORD_SWAP( "c92-03.6", 0x100000, 0x080000, CRC(1d697606) SHA1(0af7ac3c3229f139fab1569adaa893b43999bc89) )    /* data rom */

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c92-02.10", 0x000000, 0x100000, CRC(2daccecf) SHA1(bd22f95210f2bb0b63d210a54e07535c3f0e1031) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c92-01.21", 0x000000, 0x100000, CRC(9976a285) SHA1(8575ee18a3a6d690c9aa09f0c540665a31f87216) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c92-07.5",  0x00000, 0x10000, CRC(3e313db9) SHA1(44f781c7d3df8eacd745dc0af180cec9e1164b89) )

	ROM_REGION( 0x080000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c92-04.9",  0x000000, 0x080000, CRC(e421bb43) SHA1(a89157e65b537b31f5837435dae848adde8a86d1) )

	/* no Delta-T samples */
ROM_END

// Dual PCB version (main and sound program ROMs cannot be used on the single PCB version)
ROM_START( pulirula )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* 768k for 68000 code */
	ROM_LOAD16_BYTE( "c98-12.rom", 0x00000, 0x40000, CRC(816d6cde) SHA1(cac583440cca9aa57373f4a6c9a68c5442a5258b) )
	ROM_LOAD16_BYTE( "c98-16.rom", 0x00001, 0x40000, CRC(59df5c77) SHA1(e6f222589dd8d046fb92dcf5f68ab0f6cd1b725b) )
	ROM_LOAD16_BYTE( "c98-06.rom", 0x80000, 0x20000, CRC(64a71b45) SHA1(40734c378f15cd47d4128f9713bf19b0d42c0517) )
	ROM_LOAD16_BYTE( "c98-07.rom", 0x80001, 0x20000, CRC(90195bc0) SHA1(0f7e48ee8964dec1fbc6f8dab57f3ae67b2494d2) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c98-04.rom", 0x000000, 0x100000, CRC(0e1fe3b2) SHA1(37bf8e946ee4239de104a07ff87727cb6e2a3932) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c98-02.rom", 0x000000, 0x100000, CRC(4a2ad2b3) SHA1(3296cf2855203d06170c991d187c65ccc3751952) )
	ROM_LOAD( "c98-03.rom", 0x100000, 0x100000, CRC(589a678f) SHA1(228b959046bec10b28599d83d9e58fd149273473) )

	ROM_REGION( 0x080000, "tc0430grw", 0 )   /* pivot gfx */
	ROM_LOAD( "c98-05.rom", 0x000000, 0x080000, CRC(9ddd9c39) SHA1(4005a540c5cef6754284361da79a9e5e15612146) )

	ROM_REGION( 0x20000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c98-14.rom", 0x00000, 0x20000, CRC(a858e17c) SHA1(4389f43035a94e776a25350a8989dcfdb3e4675a) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c98-01.rom", 0x000000, 0x100000, CRC(197f66f5) SHA1(cc5d104033f9ab7d614afd47eeb61a22ef5714ea) )

	/* no Delta-T samples */
ROM_END

ROM_START( pulirulaj )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* 768k for 68000 code */
	ROM_LOAD16_BYTE( "c98-12.rom", 0x00000, 0x40000, CRC(816d6cde) SHA1(cac583440cca9aa57373f4a6c9a68c5442a5258b) )
	ROM_LOAD16_BYTE( "c98-13.rom", 0x00001, 0x40000, CRC(b7d13d5b) SHA1(f2cac6931c5984d68c2d5303b53c54b5781b3aed) )
	ROM_LOAD16_BYTE( "c98-06.rom", 0x80000, 0x20000, CRC(64a71b45) SHA1(40734c378f15cd47d4128f9713bf19b0d42c0517) )
	ROM_LOAD16_BYTE( "c98-07.rom", 0x80001, 0x20000, CRC(90195bc0) SHA1(0f7e48ee8964dec1fbc6f8dab57f3ae67b2494d2) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c98-04.rom", 0x000000, 0x100000, CRC(0e1fe3b2) SHA1(37bf8e946ee4239de104a07ff87727cb6e2a3932) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c98-02.rom", 0x000000, 0x100000, CRC(4a2ad2b3) SHA1(3296cf2855203d06170c991d187c65ccc3751952) )
	ROM_LOAD( "c98-03.rom", 0x100000, 0x100000, CRC(589a678f) SHA1(228b959046bec10b28599d83d9e58fd149273473) )

	ROM_REGION( 0x080000, "tc0430grw", 0 )   /* pivot gfx */
	ROM_LOAD( "c98-05.rom", 0x000000, 0x080000, CRC(9ddd9c39) SHA1(4005a540c5cef6754284361da79a9e5e15612146) )

	ROM_REGION( 0x20000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "c98-14.rom", 0x00000, 0x20000, CRC(a858e17c) SHA1(4389f43035a94e776a25350a8989dcfdb3e4675a) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c98-01.rom", 0x000000, 0x100000, CRC(197f66f5) SHA1(cc5d104033f9ab7d614afd47eeb61a22ef5714ea) )

	/* no Delta-T samples */
ROM_END

// Single PCB version (main and sound program ROMs cannot be used on the dual PCB version)
ROM_START( pulirulaa ) // dumped from an original PCB without original ROM labels. The maincpu and audiocpu ROMs differ from the parent.
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* 768k for 68000 code */
	ROM_LOAD16_BYTE( "ic46.bin", 0x00000, 0x40000, CRC(584ae599) SHA1(c114442d93080aaf0641c1a9204569f0017af000) )
	ROM_LOAD16_BYTE( "ic45.bin", 0x00001, 0x40000, CRC(08024086) SHA1(09ebbc6c6c8034ee500bf116ce2a1e854e8e0bb7) )
	ROM_LOAD16_BYTE( "ic44.bin", 0x80000, 0x20000, CRC(da9d31fd) SHA1(9300f1e8888ac48c7a10c96f5773b2acc4de5692) )
	ROM_LOAD16_BYTE( "ic43.bin", 0x80001, 0x20000, CRC(1feea319) SHA1(9799cbbd825dbc7bca6c283643070cadf9ef16ea) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "c98-04.rom", 0x000000, 0x100000, CRC(0e1fe3b2) SHA1(37bf8e946ee4239de104a07ff87727cb6e2a3932) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "c98-02.rom", 0x000000, 0x100000, CRC(4a2ad2b3) SHA1(3296cf2855203d06170c991d187c65ccc3751952) )
	ROM_LOAD( "c98-03.rom", 0x100000, 0x100000, CRC(589a678f) SHA1(228b959046bec10b28599d83d9e58fd149273473) )

	ROM_REGION( 0x080000, "tc0430grw", 0 )   /* pivot gfx */
	ROM_LOAD( "c98-05.rom", 0x000000, 0x080000, CRC(9ddd9c39) SHA1(4005a540c5cef6754284361da79a9e5e15612146) )

	ROM_REGION( 0x20000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "ic50.bin", 0x00000, 0x20000, CRC(e8b68cb8) SHA1(e2ff9df8ae635ffaa3056a97e308b9f0396bdfc6) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c98-01.rom", 0x000000, 0x100000, CRC(197f66f5) SHA1(cc5d104033f9ab7d614afd47eeb61a22ef5714ea) )

	/* no Delta-T samples */
ROM_END

ROM_START( metalb )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* 768k for 68000 code */
	ROM_LOAD16_BYTE( "d16-16.8",   0x00000, 0x40000, CRC(3150be61) SHA1(63e0e6c6dc3a64da77bc83a160bbcd0f7d98ca52) )
	ROM_LOAD16_BYTE( "d16-18.7",   0x00001, 0x40000, CRC(5216d092) SHA1(d5a237112f0e76fa68f26a4a4e804818dc49a2fa) )
	ROM_LOAD16_BYTE( "d12-07.9",   0x80000, 0x20000, CRC(e07f5136) SHA1(27df1a1f21c27feb91801e3cc304ee534969f792) )
	ROM_LOAD16_BYTE( "d12-06.6",   0x80001, 0x20000, CRC(131df731) SHA1(537a9f404d797db051a5aaf0afa2cd1e9c0bdcfb) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )   /* SCR */
	ROM_LOAD32_WORD( "d12-03.14",  0x00000, 0x80000, CRC(46b498c0) SHA1(c2ec7ce9ac7874d1bc54ce4b5d428d73c5a16549) )
	ROM_LOAD32_WORD( "d12-04.13",  0x00002, 0x80000, CRC(ab66d141) SHA1(e7f82b297dd2ae0b5d29886e5393cece61b742bb) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "d12-01.20", 0x000000, 0x100000, CRC(b81523b9) SHA1(e688e88008db87fed0051fbcb28d3e3ae7e945a8) )

	ROM_REGION( 0x20000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "d12-13.5", 0x00000, 0x20000, CRC(bcca2649) SHA1(d932134416c951d849ae41598f75609d453ed520) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d12-02.10", 0x000000, 0x100000, CRC(79263e74) SHA1(f9ef222239855d593b5855dbf9ea0376ea349c4b) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* Delta-T samples */
	ROM_LOAD( "d12-05.16", 0x000000, 0x080000, CRC(7fd036c5) SHA1(f412c6302cfba73df110943d7d6679dc908479f7) )
ROM_END

// this set comes from a single PCB (K1100698A) and the program is the same revision as metalbj
ROM_START( metalba )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* 768k for 68000 code */
	ROM_LOAD16_BYTE( "ic48",          0x00000, 0x40000, CRC(556f82b2) SHA1(fbce771eda99fd8b778f64fdc314ada4b6fdffde) ) // blank label
	ROM_LOAD16_BYTE( "ic37",          0x00001, 0x40000, CRC(419083a1) SHA1(51bef1c60ad154c7a1a9ca761618e8dcdf2d56a4) ) // blank label
	ROM_LOAD16_BYTE( "d12-07.ic47",   0x80000, 0x20000, CRC(e07f5136) SHA1(27df1a1f21c27feb91801e3cc304ee534969f792) )
	ROM_LOAD16_BYTE( "d12-06.ic36",   0x80001, 0x20000, CRC(131df731) SHA1(537a9f404d797db051a5aaf0afa2cd1e9c0bdcfb) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )   /* SCR */
	ROM_LOAD32_WORD( "d12-03.ic55",  0x00000, 0x80000, CRC(46b498c0) SHA1(c2ec7ce9ac7874d1bc54ce4b5d428d73c5a16549) )
	ROM_LOAD32_WORD( "d12-04.ic54",  0x00002, 0x80000, CRC(ab66d141) SHA1(e7f82b297dd2ae0b5d29886e5393cece61b742bb) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "d12-01.ic19", 0x000000, 0x100000, CRC(b81523b9) SHA1(e688e88008db87fed0051fbcb28d3e3ae7e945a8) )

	ROM_REGION( 0x20000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "ic57", 0x00000, 0x20000, CRC(bcca2649) SHA1(d932134416c951d849ae41598f75609d453ed520) ) // blank label

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d12-02.ic29", 0x000000, 0x100000, CRC(79263e74) SHA1(f9ef222239855d593b5855dbf9ea0376ea349c4b) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* Delta-T samples */
	ROM_LOAD( "d12-05.ic30", 0x000000, 0x080000, CRC(7fd036c5) SHA1(f412c6302cfba73df110943d7d6679dc908479f7) )

	ROM_REGION( 0x600, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "d12-8.ic28",  0x000, 0x117, NO_DUMP ) // PAL16L8BCN
	ROM_LOAD( "d12-9.ic27",  0x200, 0x117, NO_DUMP ) // PAL16L8BCN
	ROM_LOAD( "d12-10.ic50", 0x400, 0x117, NO_DUMP ) // PAL16L8BCN
ROM_END

ROM_START( metalbj )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* 768k for 68000 code */
	ROM_LOAD16_BYTE( "d12-12.8",   0x00000, 0x40000, CRC(556f82b2) SHA1(fbce771eda99fd8b778f64fdc314ada4b6fdffde) )
	ROM_LOAD16_BYTE( "d12-11.7",   0x00001, 0x40000, CRC(af9ee28d) SHA1(2c2ced0240e929da6ebec0ea7aac641531833583) )
	ROM_LOAD16_BYTE( "d12-07.9",   0x80000, 0x20000, CRC(e07f5136) SHA1(27df1a1f21c27feb91801e3cc304ee534969f792) )
	ROM_LOAD16_BYTE( "d12-06.6",   0x80001, 0x20000, CRC(131df731) SHA1(537a9f404d797db051a5aaf0afa2cd1e9c0bdcfb) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )   /* SCR */
	ROM_LOAD32_WORD( "d12-03.14",  0x00000, 0x80000, CRC(46b498c0) SHA1(c2ec7ce9ac7874d1bc54ce4b5d428d73c5a16549) )
	ROM_LOAD32_WORD( "d12-04.13",  0x00002, 0x80000, CRC(ab66d141) SHA1(e7f82b297dd2ae0b5d29886e5393cece61b742bb) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "d12-01.20", 0x000000, 0x100000, CRC(b81523b9) SHA1(e688e88008db87fed0051fbcb28d3e3ae7e945a8) )

	ROM_REGION( 0x20000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "d12-13.5", 0x00000, 0x20000, CRC(bcca2649) SHA1(d932134416c951d849ae41598f75609d453ed520) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d12-02.10", 0x000000, 0x100000, CRC(79263e74) SHA1(f9ef222239855d593b5855dbf9ea0376ea349c4b) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* Delta-T samples */
	ROM_LOAD( "d12-05.16", 0x000000, 0x080000, CRC(7fd036c5) SHA1(f412c6302cfba73df110943d7d6679dc908479f7) )
ROM_END

ROM_START( qzchikyu )
	ROM_REGION( 0x180000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "d19-06.8",    0x000000, 0x020000, CRC(de8c8e55) SHA1(ffe177231193ff32da575a4f72fc655b88f08aa8) )
	ROM_LOAD16_BYTE( "d19-05.7",    0x000001, 0x020000, CRC(c6d099d0) SHA1(a291177e1ddbe993ffa91fbe41fc04a57a568fd0) )
	ROM_LOAD16_WORD_SWAP( "d19-03.6",   0x100000, 0x080000, CRC(5c1b92c0) SHA1(36af32584ef4b2856d397e5e3ee6d17d5be296fd) )  /* data rom */

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "d19-02.10",  0x000000, 0x100000, CRC(f2dce2f2) SHA1(29fd34f1177f0b587bfef40534adaea7afc0efcb) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "d19-01.21",  0x000000, 0x100000, CRC(6c4342d0) SHA1(197e51302c23f65b8808ec9a66391b972c275867) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "d19-07.5",   0x00000, 0x10000, CRC(a8935f84) SHA1(6f264cf7a52dfb8248b8aeb4ff34477d7ccb0b6d) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d19-04.9",   0x000000, 0x080000, CRC(d3c44905) SHA1(d00bd4f11523b2123383dd852ee5484d907ff904) )

	/* no Delta-T samples */

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8b-d19-08.bin",    0x0000, 0x0104, CRC(c6240d10) SHA1(7b573ea4d04fc58303b4a044e0d983183112abda) )
	ROM_LOAD( "pal16l8b-d19-09.bin",    0x0200, 0x0104, CRC(576f5db9) SHA1(110695b6d5a80b86df254c3eb970a735f510dfe8) )
	ROM_LOAD( "pal16l8b-d19-10.bin",    0x0400, 0x0104, CRC(ea1232a5) SHA1(a13199339be00db93300049d36f35b4a32f53967) )
ROM_END

ROM_START( yesnoj ) /* Yes/No Sinri Tokimeki Chart */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "d20-05-2.2",  0x00000, 0x40000, CRC(68adb929) SHA1(5238409708d67f03d251338883de536bdf76c6c1) )
	ROM_LOAD16_BYTE( "d20-04-2.4",  0x00001, 0x40000, CRC(a84762f8) SHA1(2950419fc6dca35a43a565575cec21de8efb9df1) )

	ROM_REGION( 0x080000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD( "d20-01.11", 0x00000, 0x80000, CRC(9d8a4d57) SHA1(d7afcd86bdfe2a4c94a7c2847e17ba7e41d4de79) )

	ROM_REGION( 0x100000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "d20-02.12",  0x00000, 0x80000, CRC(e71a8e40) SHA1(cda7f14ba90f178887b9cd57d5b85be7d5090ca7) )
	ROM_LOAD16_BYTE( "d20-03.13",  0x00001, 0x80000, CRC(6a51a1b4) SHA1(a2492a7775e137a705b3281de674724d4aab4fe1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "d20-06.5",  0x00000, 0x10000, CRC(3eb537dc) SHA1(368a03fc265157faf50612b823a78c820b2519f1) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", ROMREGION_ERASE00 )
	/* no ADPCM samples */

	/* no Delta-T samples */
ROM_END

ROM_START( deadconx )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "d28-06.3",  0x00000, 0x40000, CRC(5b4bff51) SHA1(a589cbc8425add9ed5f2add26f17bb18fe53ca8a) )
	ROM_LOAD16_BYTE( "d28-12.5",  0x00001, 0x40000, CRC(9b74e631) SHA1(6a3ec317492fdf44dcee66ef8323611c55cd8843) )
	ROM_LOAD16_BYTE( "d28-09.2",  0x80000, 0x40000, CRC(143a0cc1) SHA1(a0b658b6a1567651d6fe1a955e1c853b48301f30) )
	ROM_LOAD16_BYTE( "d28-08.4",  0x80001, 0x40000, CRC(4c872bd9) SHA1(a78eec00b40445ad49425ba5a9b392b8516a30ff) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )   /* SCR */
	ROM_LOAD32_WORD( "d28-04.16", 0x00000, 0x80000, CRC(dcabc26b) SHA1(2e5e9912710ede9fc7c87430572f4e33a2ab66e7) )
	ROM_LOAD32_WORD( "d28-05.17", 0x00002, 0x80000, CRC(862f9665) SHA1(c576c929f90ff454adca8f127fd7f74863d03239) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "d28-01.8", 0x000000, 0x100000, CRC(181d7b69) SHA1(fea7bd60224cf98d53d1389695f2e0d76e50b573) )
	ROM_LOAD( "d28-02.9", 0x100000, 0x100000, CRC(d301771c) SHA1(2756bb834b50a657bbcf4e9bec02f4af595ac7e9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "d28-10.6", 0x00000, 0x10000, CRC(40805d74) SHA1(172114be692c766622d1235e5c4aa83ad438d9d9) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d28-03.10", 0x000000, 0x100000, CRC(a1804b52) SHA1(f252c473a71a4cf80d9f984fec08e5ae7524a620) )

	/* no Delta-T samples */
ROM_END

ROM_START( deadconxj )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "d28-06.3",  0x00000, 0x40000, CRC(5b4bff51) SHA1(a589cbc8425add9ed5f2add26f17bb18fe53ca8a) )
	ROM_LOAD16_BYTE( "d28-07.5",  0x00001, 0x40000, CRC(3fb8954c) SHA1(3104c1537f5fda7e5619aacfb48b02e969399a89) )
	ROM_LOAD16_BYTE( "d28-09.2",  0x80000, 0x40000, CRC(143a0cc1) SHA1(a0b658b6a1567651d6fe1a955e1c853b48301f30) )
	ROM_LOAD16_BYTE( "d28-08.4",  0x80001, 0x40000, CRC(4c872bd9) SHA1(a78eec00b40445ad49425ba5a9b392b8516a30ff) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )   /* SCR */
	ROM_LOAD32_WORD( "d28-04.16", 0x00000, 0x80000, CRC(dcabc26b) SHA1(2e5e9912710ede9fc7c87430572f4e33a2ab66e7) )
	ROM_LOAD32_WORD( "d28-05.17", 0x00002, 0x80000, CRC(862f9665) SHA1(c576c929f90ff454adca8f127fd7f74863d03239) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "d28-01.8", 0x000000, 0x100000, CRC(181d7b69) SHA1(fea7bd60224cf98d53d1389695f2e0d76e50b573) )
	ROM_LOAD( "d28-02.9", 0x100000, 0x100000, CRC(d301771c) SHA1(2756bb834b50a657bbcf4e9bec02f4af595ac7e9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "d28-10.6", 0x00000, 0x10000, CRC(40805d74) SHA1(172114be692c766622d1235e5c4aa83ad438d9d9) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d28-03.10", 0x000000, 0x100000, CRC(a1804b52) SHA1(f252c473a71a4cf80d9f984fec08e5ae7524a620) )

	/* no Delta-T samples */
ROM_END

ROM_START( dinorex )
	ROM_REGION( 0x300000, "maincpu", 0 )     /* 1Mb for 68000 code */
	ROM_LOAD16_BYTE( "d39-14.9",    0x000000, 0x080000, CRC(e6aafdac) SHA1(083c6f27b9f7b983e93c7f5d30a9a286b925c10c) )
	ROM_LOAD16_BYTE( "d39-16.8",    0x000001, 0x080000, CRC(cedc8537) SHA1(b2063c2405a3d244157ae07d60a077fdd984dbb6) )
	ROM_LOAD16_WORD_SWAP( "d39-04.6",   0x100000, 0x100000, CRC(3800506d) SHA1(a75067e94071617cd5dafdd0ae0ec096dded520a) )  /* data rom */
	ROM_LOAD16_WORD_SWAP( "d39-05.7",   0x200000, 0x100000, CRC(e2ec3b5d) SHA1(143b72d0f2f5c40dbaeed1eee0672c3b95c2bda6) )  /* data rom */

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "d39-06.2",   0x000000, 0x100000, CRC(52f62835) SHA1(251c4f17bc98a5e81c224864fb81352cf1234377) )

	ROM_REGION( 0x600000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "d39-01.29",  0x000000, 0x200000, CRC(d10e9c7d) SHA1(42c13b271a91fac37be4ea92eb358ad6a6c540cf) )
	ROM_LOAD( "d39-02.28",  0x200000, 0x200000, CRC(6c304403) SHA1(97cd58bd7d00550b7ed5f77b066216c05206b513) )
	ROM_LOAD( "d39-03.27",  0x400000, 0x200000, CRC(fc9cdab4) SHA1(ce2dfac922d6ac0f008b7dfb92d76c1671ccabbd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "d39-12.5",   0x00000, 0x10000, CRC(8292c7c1) SHA1(2ff20726f6cc6d98d860d96b8eb3c10f46b87d58) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d39-07.10",  0x000000, 0x100000, CRC(28262816) SHA1(6df9a31a2edf1dfc23070b41b1da2c0a9e91d1b0) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* Delta-T samples */
	ROM_LOAD( "d39-08.4",   0x000000, 0x080000, CRC(377b8b7b) SHA1(4905ac3b9b52c70afe5f451f860fb9acd81a1dbb) )
ROM_END

ROM_START( dinorexj )
	ROM_REGION( 0x300000, "maincpu", 0 )     /* 1Mb for 68000 code */
	ROM_LOAD16_BYTE( "d39-14.9",    0x000000, 0x080000, CRC(e6aafdac) SHA1(083c6f27b9f7b983e93c7f5d30a9a286b925c10c) )
	ROM_LOAD16_BYTE( "d39-13.8",    0x000001, 0x080000, CRC(ae496b2f) SHA1(0e2ed2b77287590343820841d413ce6cb05b616d) )
	ROM_LOAD16_WORD_SWAP( "d39-04.6",   0x100000, 0x100000, CRC(3800506d) SHA1(a75067e94071617cd5dafdd0ae0ec096dded520a) )  /* data rom */
	ROM_LOAD16_WORD_SWAP( "d39-05.7",   0x200000, 0x100000, CRC(e2ec3b5d) SHA1(143b72d0f2f5c40dbaeed1eee0672c3b95c2bda6) )  /* data rom */

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "d39-06.2",   0x000000, 0x100000, CRC(52f62835) SHA1(251c4f17bc98a5e81c224864fb81352cf1234377) )

	ROM_REGION( 0x600000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "d39-01.29",  0x000000, 0x200000, CRC(d10e9c7d) SHA1(42c13b271a91fac37be4ea92eb358ad6a6c540cf) )
	ROM_LOAD( "d39-02.28",  0x200000, 0x200000, CRC(6c304403) SHA1(97cd58bd7d00550b7ed5f77b066216c05206b513) )
	ROM_LOAD( "d39-03.27",  0x400000, 0x200000, CRC(fc9cdab4) SHA1(ce2dfac922d6ac0f008b7dfb92d76c1671ccabbd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "d39-12.5",   0x00000, 0x10000, CRC(8292c7c1) SHA1(2ff20726f6cc6d98d860d96b8eb3c10f46b87d58) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d39-07.10",  0x000000, 0x100000, CRC(28262816) SHA1(6df9a31a2edf1dfc23070b41b1da2c0a9e91d1b0) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* Delta-T samples */
	ROM_LOAD( "d39-08.4",   0x000000, 0x080000, CRC(377b8b7b) SHA1(4905ac3b9b52c70afe5f451f860fb9acd81a1dbb) )
ROM_END

ROM_START( dinorexu )
	ROM_REGION( 0x300000, "maincpu", 0 )    /* 1Mb for 68000 code */
	ROM_LOAD16_BYTE( "d39-14.9",    0x000000, 0x080000, CRC(e6aafdac) SHA1(083c6f27b9f7b983e93c7f5d30a9a286b925c10c) )
	ROM_LOAD16_BYTE( "d39-15.8",    0x000001, 0x080000, CRC(fe96723b) SHA1(e68b2149cc935ff36efa501b086d5ffb1e0f5887) )
	ROM_LOAD16_WORD_SWAP( "d39-04.6",   0x100000, 0x100000, CRC(3800506d) SHA1(a75067e94071617cd5dafdd0ae0ec096dded520a) )  /* data rom */
	ROM_LOAD16_WORD_SWAP( "d39-05.7",   0x200000, 0x100000, CRC(e2ec3b5d) SHA1(143b72d0f2f5c40dbaeed1eee0672c3b95c2bda6) )  /* data rom */

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "d39-06.2",   0x000000, 0x100000, CRC(52f62835) SHA1(251c4f17bc98a5e81c224864fb81352cf1234377) )

	ROM_REGION( 0x600000, "sprites", 0 )   /* OBJ */
	ROM_LOAD( "d39-01.29",  0x000000, 0x200000, CRC(d10e9c7d) SHA1(42c13b271a91fac37be4ea92eb358ad6a6c540cf) )
	ROM_LOAD( "d39-02.28",  0x200000, 0x200000, CRC(6c304403) SHA1(97cd58bd7d00550b7ed5f77b066216c05206b513) )
	ROM_LOAD( "d39-03.27",  0x400000, 0x200000, CRC(fc9cdab4) SHA1(ce2dfac922d6ac0f008b7dfb92d76c1671ccabbd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "d39-12.5",   0x00000, 0x10000, CRC(8292c7c1) SHA1(2ff20726f6cc6d98d860d96b8eb3c10f46b87d58) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d39-07.10",  0x000000, 0x100000, CRC(28262816) SHA1(6df9a31a2edf1dfc23070b41b1da2c0a9e91d1b0) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   /* Delta-T samples */
	ROM_LOAD( "d39-08.4",   0x000000, 0x080000, CRC(377b8b7b) SHA1(4905ac3b9b52c70afe5f451f860fb9acd81a1dbb) )
ROM_END

ROM_START( qjinsei )    /* Quiz Jinsei Gekijoh */
	ROM_REGION( 0x200000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "d48-09",  0x000000, 0x040000, CRC(a573b68d) SHA1(fc12c7fb577c919cdb8e16e6d0ddba5603813a13) )
	ROM_LOAD16_BYTE( "d48-10",  0x000001, 0x040000, CRC(37143a5b) SHA1(8a06031618b60a0c8f38789027d0ed042e540c4d) )
	ROM_LOAD16_WORD_SWAP( "d48-03",  0x100000, 0x100000, CRC(fb5ea8dc) SHA1(2444042a85af6ae5b87e95ab09f661b877f215cc) ) /* data rom */

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "d48-04", 0x000000, 0x100000, CRC(61e4b078) SHA1(c01722af74220d0bb0daf3b78f53f8875209e066) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "d48-02", 0x000000, 0x100000, CRC(a7b68e63) SHA1(ecdd0b7f4c52eac8c6e4218f69c01cf9b2f98e2a) )
	ROM_LOAD16_BYTE( "d48-01", 0x000001, 0x100000, CRC(72a94b73) SHA1(f4f296886c5fdb227f43cb9231bb15742f8a77f1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "d48-11",    0x00000, 0x10000, CRC(656c5b54) SHA1(650bcc5920838db5c6613bcf30468d6e296ea017) )

	ROM_REGION( 0x080000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d48-05",  0x000000, 0x080000, CRC(3fefd058) SHA1(338c35c3a086041d28708d4b17e208b590c926d5) )

	/* no Delta-T samples */
ROM_END

ROM_START( qcrayon )    /* Quiz Crayon */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "d55-13",  0x00000, 0x40000, CRC(16afbfc7) SHA1(daf948603c78312aa8fb8e239097939ec89ecc64) )
	ROM_LOAD16_BYTE( "d55-14",  0x00001, 0x40000, CRC(2fb3057f) SHA1(3b107dc69c01ca1b90c78a122336896b89509a3e) )

	ROM_REGION16_BE( 0x100000, "extra", 0 )
	/* extra ROM mapped 0x300000 */
	ROM_LOAD16_WORD_SWAP( "d55-03", 0x000000, 0x100000, CRC(4d161e76) SHA1(96189294f91f165423ba585c650ee47fc8165725) )   /* data rom */

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )   /* SCR */
	ROM_LOAD16_WORD_SWAP( "d55-02", 0x000000, 0x100000, CRC(f3db2f1c) SHA1(a96d89dd2c851dae3a237141c478fe2a65dda9c3) )

	ROM_REGION( 0x200000, "sprites", 0 )   /* OBJ */
	ROM_LOAD16_BYTE( "d55-05", 0x000000, 0x100000, CRC(f0e59902) SHA1(44d93e0e9622a98796a128a0273065947f586a1d) )
	ROM_LOAD16_BYTE( "d55-04", 0x000001, 0x100000, CRC(412975ce) SHA1(32058a87947d6b6cdc8b147ddfcf359792f9c9fc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "d55-15",  0x00000, 0x10000, CRC(ba782eff) SHA1(ce24654db49b9694e444e93b9a8d529a86729e03) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d55-01",  0x000000, 0x100000, CRC(a8309af4) SHA1(dc30b2e019003c58aecaf899668b36dea6e1274e) )

	/* no Delta-T samples */
ROM_END

ROM_START( qcrayon2 )   /* Quiz Crayon 2 */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "d63-12",  0x00000, 0x40000, CRC(0f445a38) SHA1(cb9212c6c61d7dec7cfa039ebbbabd368dad52c3) )
	ROM_LOAD16_BYTE( "d63-13",  0x00001, 0x40000, CRC(74455752) SHA1(9835eb5ebad8df96abe8f7d5f8e4ff663b38015a) )

	ROM_REGION16_BE( 0x080000, "extra", 0 )
	/* extra ROM mapped at 600000 */
	ROM_LOAD16_WORD_SWAP( "d63-01", 0x00000, 0x80000, CRC(872e38b4) SHA1(dbb3728655dee03f6583db976708507c9ac5be16) )   /* data rom */

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )    /* SCR */
	ROM_LOAD16_WORD_SWAP( "d63-03", 0x000000, 0x100000, CRC(d24843af) SHA1(6508182f9038e4603230a6489e89ebae91c2f761) )

	ROM_REGION( 0x200000, "sprites", 0 )    /* OBJ */
	ROM_LOAD( "d63-06", 0x000000, 0x200000, CRC(58b1e4a8) SHA1(1b957c2d2d1cdada9972880d9d7b6c8c584edabb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "d63-11",    0x00000, 0x10000, CRC(2c7ac9e5) SHA1(2477fb4415781afddc2f4084eca52f53b7d40480) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "d63-02",  0x000000, 0x100000, CRC(162ae165) SHA1(8b503d096640ec91cf55e05374b293937965c672) )

	/* no Delta-T samples */
ROM_END

ROM_START( driftout )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "ic46.rom",  0x00000, 0x80000, CRC(71303738) SHA1(b473e1cfe2b64df41d57bdf421a62bebcc882304) )
	ROM_LOAD16_BYTE( "ic45.rom",  0x00001, 0x80000, CRC(43f81eca) SHA1(1a67105bc56cb5366ee85dc54eafe7673bb10ef0) )

	ROM_REGION( 0x080000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty */

	ROM_REGION( 0x080000, "sprites", 0 )      /* OBJ */
	ROM_LOAD( "do_obj.rom", 0x00000, 0x80000, CRC(5491f1c4) SHA1(a2e92a9a1e77d9f683f6720947e0622dde48287f) )

	ROM_REGION( 0x080000, "tc0430grw", 0 )      /* pivot gfx */
	ROM_LOAD( "do_piv.rom", 0x00000, 0x80000, CRC(c4f012f7) SHA1(4ad6a88f6a7f89b2b4c62c2b376d4e7b43c3d442) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "do_50.rom",  0x00000, 0x10000, CRC(ffe10124) SHA1(a47dfedfa7b352a5db39e7e1ccc666d3c5fb0d75) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "do_snd.rom", 0x00000, 0x80000, CRC(f2deb82b) SHA1(55e39173a475f5ab0b5f573a678a493fb6eefe64) )

	/* no Delta-T samples */
ROM_END

ROM_START( driftoutj )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "do_46.rom",  0x00000, 0x80000, CRC(f960363e) SHA1(3f64fd606d4e19198de460cf2d99331a2d2e7434) )
	ROM_LOAD16_BYTE( "do_45.rom",  0x00001, 0x80000, CRC(e3fe66b9) SHA1(6b197061be0c296af08a86dace08ba75c9574e19) )

	ROM_REGION( 0x080000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty */

	ROM_REGION( 0x080000, "sprites", 0 )      /* OBJ */
	ROM_LOAD( "do_obj.rom", 0x00000, 0x80000, CRC(5491f1c4) SHA1(a2e92a9a1e77d9f683f6720947e0622dde48287f) )

	ROM_REGION( 0x080000, "tc0430grw", 0 )      /* pivot gfx */
	ROM_LOAD( "do_piv.rom", 0x00000, 0x80000, CRC(c4f012f7) SHA1(4ad6a88f6a7f89b2b4c62c2b376d4e7b43c3d442) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu */
	ROM_LOAD( "do_50.rom",  0x00000, 0x10000, CRC(ffe10124) SHA1(a47dfedfa7b352a5db39e7e1ccc666d3c5fb0d75) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "do_snd.rom", 0x00000, 0x80000, CRC(f2deb82b) SHA1(55e39173a475f5ab0b5f573a678a493fb6eefe64) )

	/* no Delta-T samples */
ROM_END

ROM_START( driftoutct ) // conversion on Cameltry hardware
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "l.ic10",  0x00000, 0x80000, CRC(d1f779d3) SHA1(d70a52b74bed88e33da6bdaf34467c0353bef9a8) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "h.ic11",  0x00001, 0x80000, CRC(6ff1c660) SHA1(54e86795a166e49bf472577faddad5c640434414) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x080000, "tc0100scn_1", ROMREGION_ERASEFF )
	// empty

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "do_obj.ic1", 0x00000, 0x80000, CRC(5491f1c4) SHA1(a2e92a9a1e77d9f683f6720947e0622dde48287f) )

	ROM_REGION( 0x080000, "tc0280grd", 0 ) // two identical ROMs here, probably one for each ROZ layer, load them both to reflect this even if MAME only uses one
	ROM_LOAD( "do_piv.ic27", 0x00000, 0x80000, CRC(c4f012f7) SHA1(4ad6a88f6a7f89b2b4c62c2b376d4e7b43c3d442) )
	ROM_LOAD( "do_piv.ic29", 0x00000, 0x80000, CRC(c4f012f7) SHA1(4ad6a88f6a7f89b2b4c62c2b376d4e7b43c3d442) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "do_50.ic25",  0x00000, 0x10000, CRC(ffe10124) SHA1(a47dfedfa7b352a5db39e7e1ccc666d3c5fb0d75) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "do_snd.ic2", 0x00000, 0x80000, CRC(f2deb82b) SHA1(55e39173a475f5ab0b5f573a678a493fb6eefe64) )

	// no Delta-T samples

	ROM_REGION( 0x600, "plds", 0 ) // brute-forced
	ROM_LOAD( "pal16l8b.ic8",         0x0000, 0x0117, CRC(b2c1d05f) SHA1(163dcf1615098d7ecbd9725f9e679817b51175ba) )
	ROM_LOAD( "pal16l8b.ic9",         0x0200, 0x0117, CRC(0dd34b78) SHA1(c6f3483b3b3091332f4919a269a727b3809ab12a) )
	ROM_LOAD( "pal16l8b.c38-07.ic24", 0x0400, 0x0117, CRC(47fc77a9) SHA1(82372b7d60ccffbc4a5ec18d240e473a08940643) )
ROM_END

ROM_START( driveout )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "4.u3", 0x00000, 0x80000, CRC(dc431e4e) SHA1(6002cb7a2bd05e28a2413942998a5c7e11fc1432) )
	ROM_LOAD16_BYTE( "5.u2", 0x00001, 0x80000, CRC(6f9063f4) SHA1(7ea55126a2f6391322740432d835cd06450909ae) )

	ROM_REGION( 0x080000, "tc0100scn_1", ROMREGION_ERASEFF )
	/* empty */

	ROM_REGION( 0x080000, "sprites", 0 )      /* OBJ */
	ROM_LOAD16_BYTE( "6.u84", 0x00000, 0x40000, CRC(530ac420) SHA1(d66006958580205d0962871ba7d0b40a067bb9af) )
	ROM_LOAD16_BYTE( "7.u81", 0x00001, 0x40000, CRC(0e9a3e9e) SHA1(7bb21e6fc930a5e1913bffb626958d0ee22d5883) )

	ROM_REGION( 0x080000, "tc0430grw", 0 )      /* pivot gfx */
	ROM_LOAD( "8.u37",    0x00000, 0x80000, CRC(c4f012f7) SHA1(4ad6a88f6a7f89b2b4c62c2b376d4e7b43c3d442) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "3.u20",  0x0000,  0x8000, CRC(99aaeb2e) SHA1(c7eb174f2ddcd8fd2b73b5251f434a20a9627b49) ) // found on another PCB double sized with the first half 0xff filled

	ROM_REGION( 0xa0000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "1.u28",  0x00000, 0x80000, CRC(cbde0b66) SHA1(b264aa525ff40c7813182031825cd052db887000) ) /* banked */
	ROM_LOAD( "2.u29",  0x80000, 0x20000, CRC(0aba2026) SHA1(f592e3b294d44f499fdca4cc31646e55d8c3dfbf) ) /* sandwiched */
ROM_END


void taitof2_state::init_finalb()
{
	/* convert from 2bits into 4bits format */
	gfx_element *gx0 = m_gfxdecode->gfx(0);
	gfx_element *gx1 = m_gfxdecode->gfx(1);

	// allocate memory for the assembled data
	m_decoded_gfx = std::make_unique<u8[]>(gx0->elements() * gx0->width() * gx0->height());

	// loop over elements
	u8 *dest = m_decoded_gfx.get();
	for (int c = 0; c < gx0->elements(); c++)
	{
		const u8 *c0base = gx0->get_data(c);
		const u8 *c1base = gx1->get_data(c);

		// loop over height
		for (int y = 0; y < gx0->height(); y++)
		{
			const u8 *c0 = c0base;
			const u8 *c1 = c1base;

			for (int x = 0; x < gx0->width(); x++)
			{
				u8 hipix = *c1++;
				*dest++ = (*c0++ & 0xf) | ((hipix << 4) & 0x30);
			}
			c0base += gx0->rowbytes();
			c1base += gx1->rowbytes();
		}
	}

	gx0->set_raw_layout(m_decoded_gfx.get(), gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_colors(4096 / 64);
	gx0->set_granularity(64);
	m_gfxdecode->set_gfx(1, nullptr);
}

void taitof2_state::init_cameltry()
{
	m_last[0] = 0;
	m_last[1] = 0;

	save_item(NAME(m_last));
}


void taitof2_state::init_mjnquest()
{
	const u32 len = memregion("sprites")->bytes();
	u8 *gfx = memregion("sprites")->base();

	/* the bytes in each longword are in reversed order, put them in the
	   order used by the other games. */
	for (int i = 0; i < len; i += 2)
	{
		const u8 t = gfx[i];
		gfx[i] = (gfx[i + 1] >> 4) | (gfx[i + 1] << 4);
		gfx[i + 1] = (t >> 4) | (t << 4);
	}

	m_mjnquest_input = 0;

	save_item(NAME(m_mjnquest_input));
}

void taitof2_state::init_driveout()
{
	m_okibank->configure_entries(0, 4, memregion("oki")->base(), 0x20000);
	m_okibank->set_entry(0);
	m_driveout_sound_latch = 0;
	m_nibble = 0;

	save_item(NAME(m_driveout_sound_latch));
	save_item(NAME(m_nibble));
}

//    YEAR  NAME        PARENT    MACHINE    INPUT       CLASS          INIT           ROT     COMPANY                      FULLNAME

GAME( 1988, finalb,     0,        finalb,    finalb,     taitof2_state, init_finalb,   ROT0,   "Taito Corporation Japan",   "Final Blow (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, finalbu,    finalb,   finalb,    finalbu,    taitof2_state, init_finalb,   ROT0,   "Taito America Corporation", "Final Blow (US, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, finalbj,    finalb,   finalb,    finalbj,    taitof2_state, init_finalb,   ROT0,   "Taito Corporation",         "Final Blow (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, dondokod,   0,        dondokod,  dondokod,   taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Don Doko Don (World, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, dondokodu,  dondokod, dondokod,  dondokodu,  taitof2_state, empty_init,    ROT0,   "Taito America Corporation", "Don Doko Don (US, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, dondokodj,  dondokod, dondokod,  dondokodj,  taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Don Doko Don (Japan, rev 1)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, megablst,   0,        megab,     megab,      taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Mega Blast (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, megablstu,  megablst, megab,     megabu,     taitof2_state, empty_init,    ROT0,   "Taito America Corporation", "Mega Blast (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, megablstj,  megablst, megab,     megabj,     taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Mega Blast (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, thundfox,   0,        thundfox,  thundfox,   taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Thunder Fox (World, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, thundfoxu,  thundfox, thundfox,  thundfoxu,  taitof2_state, empty_init,    ROT0,   "Taito America Corporation", "Thunder Fox (US, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, thundfoxj,  thundfox, thundfox,  thundfoxj,  taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Thunder Fox (Japan, rev 1)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, cameltry,   0,        cameltry,  cameltry,   taitof2_state, init_cameltry, ROT0,   "Taito America Corporation", "Cameltry (US, YM2610)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, cameltryj,  cameltry, cameltry,  cameltryj,  taitof2_state, init_cameltry, ROT0,   "Taito Corporation",         "Cameltry (Japan, YM2610)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, cameltrya,  cameltry, cameltrya, cameltry,   taitof2_state, init_cameltry, ROT0,   "Taito America Corporation", "Cameltry (World, YM2203 + M6295)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, cameltryau, cameltry, cameltrya, cameltry,   taitof2_state, init_cameltry, ROT0,   "Taito America Corporation", "Cameltry (US, YM2203 + M6295)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, qtorimon,   0,        qtorimon,  qtorimon,   taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Quiz Torimonochou (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, liquidk,    0,        liquidk,   liquidk,    taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Liquid Kids (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, liquidku,   liquidk,  liquidk,   liquidku,   taitof2_state, empty_init,    ROT0,   "Taito America Corporation", "Liquid Kids (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, mizubaku,   liquidk,  liquidk,   mizubaku,   taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Mizubaku Daibouken (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, quizhq,     0,        quizhq,    quizhq,     taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Quiz H.Q. (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, ssi,        0,        ssi,       ssi,        taitof2_state, empty_init,    ROT270, "Taito Corporation Japan",   "Super Space Invaders '91 (World, revised code, Rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ssia,       ssi,      ssi,       ssi,        taitof2_state, empty_init,    ROT270, "Taito Corporation Japan",   "Super Space Invaders '91 (World, revised code)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ssib,       ssi,      ssi,       ssi,        taitof2_state, empty_init,    ROT270, "Taito Corporation Japan",   "Super Space Invaders '91 (World, earlier code base)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, majest12u,  ssi,      ssi,       majest12u,  taitof2_state, empty_init,    ROT270, "Taito America Corporation", "Majestic Twelve - The Space Invaders Part IV (US, revised code, Rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, majest12ua, ssi,      ssi,       majest12u,  taitof2_state, empty_init,    ROT270, "Taito America Corporation", "Majestic Twelve - The Space Invaders Part IV (US, revised code)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, majest12ub, ssi,      ssi,       majest12u,  taitof2_state, empty_init,    ROT270, "Taito America Corporation", "Majestic Twelve - The Space Invaders Part IV (US, earlier code base)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, majest12j,  ssi,      ssi,       majest12j,  taitof2_state, empty_init,    ROT270, "Taito Corporation",         "Majestic Twelve - The Space Invaders Part IV (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, gunfront,   0,        gunfront,  gunfront,   taitof2_state, empty_init,    ROT270, "Taito Corporation Japan",   "Gun & Frontier (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, gunfrontj,  gunfront, gunfront,  gunfrontj,  taitof2_state, empty_init,    ROT270, "Taito Corporation",         "Gun Frontier (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, growl,      0,        growl,     growl,      taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Growl (World, Rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, growla,     growl,    growl,     growl,      taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Growl (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, growlu,     growl,    growl,     growlu,     taitof2_state, empty_init,    ROT0,   "Taito America Corporation", "Growl (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, runark,     growl,    growl,     runark,     taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Runark (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, growlp,     growl,    growl,     growl,      taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Growl (World, prototype)", MACHINE_SUPPORTS_SAVE )


GAME( 1990, mjnquest,   0,        mjnquest,  mjnquest,   taitof2_state, init_mjnquest, ROT0,   "Taito Corporation",         "Mahjong Quest (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, mjnquestb,  mjnquest, mjnquest,  mjnquest,   taitof2_state, init_mjnquest, ROT0,   "Taito Corporation",         "Mahjong Quest (No Nudity)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, footchmp,   0,        footchmp,  footchmp,   taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Football Champ / Euro Football Champ (World)", MACHINE_SUPPORTS_SAVE ) // title depends on dipswitch
GAME( 1990, htherou,    footchmp, footchmp,  htherou,    taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Hat Trick Hero (US)", MACHINE_SUPPORTS_SAVE ) // Single PCB
GAME( 1990, htheroj,    footchmp, hthero,    htheroj,    taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Hat Trick Hero (Japan)", MACHINE_SUPPORTS_SAVE ) // Dual PCB
GAME( 1992, footchmpbl, footchmp, footchmpbl,footchmpbl, taitof2_state, empty_init,    ROT0,   "bootleg",                   "Football Champ / Euro Football Champ (World) (bootleg)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // very different hw register etc.

GAME( 1992, euroch92,   0,        footchmp,  footchmp,   taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Euro Champ '92 (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, euroch92j,  euroch92, footchmp,  footchmp,   taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Euro Champ '92 (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, koshien,    0,        koshien,   koshien,    taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Ah Eikou no Koshien (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, yuyugogo,   0,        yuyugogo,  yuyugogo,   taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Yuuyu no Quiz de GO!GO! (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, ninjak,     0,        ninjak,    ninjak,     taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "The Ninja Kids (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ninjaku,    ninjak,   ninjak,    ninjaku,    taitof2_state, empty_init,    ROT0,   "Taito America Corporation", "The Ninja Kids (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ninjakj,    ninjak,   ninjak,    ninjakj,    taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "The Ninja Kids (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, solfigtr,   0,        solfigtr,  solfigtr,   taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Solitary Fighter (World)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, qzquest,    0,        qzquest ,  qzquest,    taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Quiz Quest - Hime to Yuusha no Monogatari (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, pulirula,   0,        pulirula,  pulirula,   taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "PuLiRuLa (World, dual PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, pulirulaa,  pulirula, pulirula,  pulirulaj,  taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "PuLiRuLa (World, single PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, pulirulaj,  pulirula, pulirula,  pulirulaj,  taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "PuLiRuLa (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, metalb,     0,        metalb,    metalb,     taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Metal Black (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, metalba,    metalb,   metalb,    metalb,     taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Metal Black (World, single PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, metalbj,    metalb,   metalb,    metalbj,    taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Metal Black (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, qzchikyu,   0,        qzchikyu,  qzchikyu,   taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Quiz Chikyu Bouei Gun (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1992, yesnoj,     0,        yesnoj,    yesnoj,     taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Yes/No Sinri Tokimeki Chart", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_PRINTER )

GAME( 1992, deadconx,   0,        deadconx,  deadconx,   taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Dead Connection (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, deadconxj,  deadconx, deadconxj, deadconxj,  taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Dead Connection (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1992, dinorex,    0,        dinorex,   dinorex,    taitof2_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Dino Rex (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, dinorexu,   dinorex,  dinorex,   dinorexu,   taitof2_state, empty_init,    ROT0,   "Taito America Corporation", "Dino Rex (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, dinorexj,   dinorex,  dinorex,   dinorexj,   taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Dino Rex (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1992, qjinsei,    0,        qjinsei,   qjinsei,    taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Quiz Jinsei Gekijoh (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, qcrayon,    0,        qcrayon,   qcrayon,    taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Quiz Crayon Shinchan (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, qcrayon2,   0,        qcrayon2,  qcrayon2,   taitof2_state, empty_init,    ROT0,   "Taito Corporation",         "Crayon Shinchan Orato Asobo (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, driftout,   0,        driftout,  driftout,   taitof2_state, empty_init,    ROT270, "Visco",                     "Drift Out (Europe)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, driftoutct, driftout, driftoutct,driftoutct, taitof2_state, empty_init,    ROT270, "Visco",                     "Drift Out (Europe, Cameltry conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, driftoutj,  driftout, driftout,  driftout,   taitof2_state, empty_init,    ROT270, "Visco",                     "Drift Out (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, driveout,   driftout, driveout,  driftout,   taitof2_state, init_driveout, ROT270, "bootleg",                   "Drive Out (bootleg)", MACHINE_SUPPORTS_SAVE )
