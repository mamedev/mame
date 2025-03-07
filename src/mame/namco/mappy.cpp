// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Several Namco games from 1982-1985

driver by Nicola Salmoria
based on previous work by Aaron Giles, Manuel Abadia

The games supported by this driver all run on similar hardware.
They can be divided in three "families":
1) Super Pacman, the first version of the board. Pac & Pal and Grobda run on
   seemingly identical hardware.
   The hardware consists of two 6809, and several Namco custom ICs that provide
   a static tilemap and 2bpp sprites.
   Grobda is the only Namco game of this era that has speech (just a short
   sample). It appears to do this by manipulating 15XX channels to deliver
   non-waveform outputs (removing the 15XX from the board causes sound to
   disappear completely).
2) Phozon. This game runs on an unique board: the large number of sprites on
   screen at the same time required a 3rd 6809 to help with the calculations.
   The sprite hardware is also different from Super Pacman, featuring 8x8 sprites.
   Despite the hardware differences, the memory map is almost identical to
   Super Pacman, and that's why it's included in this driver.
   There is no information about the custom ICs used by this board. The video
   section is probably more similar to Gaplus than to Super Pacman: the sprite
   generator might be a 21XX (though Gaplus doesn't use 8x8 sprites), and the
   00XX and 04XX address generators are probably replaced by the single CUS20
   (which also handles the flip screen flag).
3) Mappy runs on a revised design of the Super Pacman board, where the 00XX
   custom is replaced by a 17XX, which provides a scrolling tilemap. The larger
   tilemap requires more RAM, so the memory map is slightly different. Also,
   sprites are 4bpp instead of 2bpp, so the final stage of video generation is
   a little different as well, though the custom ICs are still the same.
   Dig Dig II and Motos are almost the same, apart from larger ROMs.
   The Tower of Druaga also has 4x the amount of sprite color combinations so
   it's the most different of the four.


Custom ICs (Super Pacman and Mappy):
-----------------------------------
CPU board:
07XX     clock divider
15XX     sound control
16XX     I/O control
5xXX(x2) I/O
99XX     DAC with volume control (only Mappy, Super Pacman uses LS273, CD4066
         and binary-weighted resistors R34-37 and R22-25)

Video board:
00XX     tilemap address generator with scrolling capability (only Super Pacman)
04XX     sprite address generator
07XX     clock divider
11XX     gfx data shifter and mixer (16-bit in, 4-bit out) [1]
12XX     sprite generator
17XX     tilemap address generator with scrolling capability (only Mappy)

[1] Used differently: in Super Pacman it merges the 2bpp tilemap with the 2bpp
sprites; in Mappy it handles the 4bpp sprites, while the tilemap is handled by
standard LS components.

The I/O interface chips vary from game to game (see namco/namcoio.cpp)


Super Pac-Man memory map
------------------------
Pac & Pal and Grobda are the same.
Note: Part of the address decoding is done by PALs (SPC-5 and SPC-6) so the
tables are inferred by program behaviour.

MAIN CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
00000xxxxxxxxxxx R/W xxxxxxxx RAM 2E    tilemap RAM
00001xxxxxxxxxxx R/W xxxxxxxx RAM 2H    work RAM
000011111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (sprite number & color)
00010xxxxxxxxxxx R/W xxxxxxxx RAM 2K    work RAM
000101111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (x, y)
00011xxxxxxxxxxx R/W xxxxxxxx RAM 2J    work RAM
000111111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (x msb, flip, size)
00100----------- R/W -------x FLIP      screen flip (reading this address sets the bit, done by pacnpal)
01000-xxxxxxxxxx R/W xxxxxxxx SOUND     RAM (shared with sound CPU)
01000-0000xxxxxx R/W xxxxxxxx           portion holding the sound registers
01001-----xxxxxx R/W ----xxxx FBIT      I/O chips [1]
01010-------000x r/W -------- INT ON 2  sound CPU irq enable (data is in A0)
01010-------001x r/W -------- INT ON    main CPU irq enable (data is in A0)
01010-------010x   W -------- n.c.
01010-------011x r/W -------- SOUND ON  sound enable (data is in A0)
01010-------100x r/W -------- 4 RESET   reset I/O chips (data is in A0)
01010-------101x r/W -------- SUB RESET reset sound CPU (data is in A0)
01010-------110x   W -------- n.c.
01010-------111x   W -------- n.c.
10000----------- R/W -------- WDR       Watch Dog Reset
101xxxxxxxxxxxxx R   xxxxxxxx ROM 1D    program ROM
110xxxxxxxxxxxxx R   xxxxxxxx ROM 1C    program ROM
111xxxxxxxxxxxxx R   xxxxxxxx ROM 1B    program ROM

[1] only half of that space is actually used, because only 2 of the possible 4 I/O chips are present


SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
000---xxxxxxxxxx R/W xxxxxxxx RAM 3K/3L work RAM (shared with main CPU)
000---0000xxxxxx R/W xxxxxxxx           portion holding the sound registers
001---------000x   W -------- INT ON 2  sound CPU irq enable (data is in A0)
001---------001x   W -------- INT ON    main CPU irq enable (data is in A0)
001---------010x   W -------- n.c.
001---------011x   W -------- SOUND ON  sound enable (data is in A0)
001---------100x   W -------- 4 RESET   reset 58XX I/O chips (data is in A0)
001---------101x   W -------- SUB RESET reset sound CPU (data is in A0)
001---------110x   W -------- n.c.
001---------111x   W -------- n.c.
110-xxxxxxxxxxxx R   xxxxxxxx ROM 1J    program ROM (optional, not used by any game)
111xxxxxxxxxxxxx R   xxxxxxxx ROM 1K    program ROM (space for a 2764, but some games use a 2732)



Mappy memory map
----------------
The Tower of Druaga, Dig Dug 2, Motos are the same, with minor differences.
The main difference with Super Pac-Man is the increased video RAM, needed to
implement a scrolling tilemap.

Note: Part of the address decoding is done by PALs (SPC-5 and SPC-6, the same
as Super Pac-Man) so the tables are inferred by program behaviour.

MAIN CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
00000xxxxxxxxxxx R/W xxxxxxxx RAM 2H    tilemap RAM (tile number)
00001xxxxxxxxxxx R/W xxxxxxxx RAM 2J    tilemap RAM (tile color)
00010xxxxxxxxxxx R/W xxxxxxxx RAM 2N    work RAM
000101111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (sprite number & color)
00011xxxxxxxxxxx R/W xxxxxxxx RAM 2L    work RAM
000111111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (x, y)
00100xxxxxxxxxxx R/W xxxxxxxx RAM 2M    work RAM
001001111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (x msb, flip, size)
00111xxxxxxxx---   W -------- POSIV     tilemap scroll (data is in A3-A10)
01000-xxxxxxxxxx R/W xxxxxxxx SOUND     RAM (shared with sound CPU)
01000-0000xxxxxx R/W xxxxxxxx           portion holding the sound registers
01001-----xxxxxx R/W ----xxxx FBIT      I/O chips [1]
01010-------000x   W -------- INT ON 2  sound CPU irq enable (data is in A0)
01010-------001x   W -------- INT ON    main CPU irq enable (data is in A0)
01010-------010x   W -------- FLIP      screen flip (data is in A0)
01010-------011x   W -------- SOUND ON  sound enable (to 99XX custom) (data is in A0)
01010-------100x   W -------- 4 RESET   reset I/O chips (data is in A0)
01010-------101x   W -------- SUB RESET reset sound CPU (data is in A0)
01010-------110x   W -------- n.c.
01010-------111x   W -------- n.c.
10000----------- R/W -------- WDR       Watch Dog Reset [3]
101xxxxxxxxxxxxx R   xxxxxxxx ROM 1D    program ROM [2]
110xxxxxxxxxxxxx R   xxxxxxxx ROM 1C    program ROM [2]
111xxxxxxxxxxxxx R   xxxxxxxx ROM 1B    program ROM [2]

[1] only half of that space is actually used, because only 2 of the possible 4 I/O chips are present
[2] The Tower of Druaga, Dig Dug 2, Motos have 2 128k ROMs instead of 3 64k:
10xxxxxxxxxxxxxx R   xxxxxxxx ROM 1D    program ROM
11xxxxxxxxxxxxxx R   xxxxxxxx ROM 1B    program ROM
[3] Overlaps the ROM region. Dig Dug 2 reads it.

SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
000---xxxxxxxxxx R/W xxxxxxxx RAM 3K/3L work RAM (shared with main CPU)
000---0000xxxxxx R/W xxxxxxxx           portion holding the sound registers
001---------000x   W -------- INT ON 2  sound CPU irq enable (data is in A0)
001---------001x   W -------- INT ON    main CPU irq enable (data is in A0)
001---------010x   W -------- FLIP      screen flip (data is in A0)
001---------011x   W -------- SOUND ON  sound enable (to 99XX custom) (data is in A0)
001---------100x   W -------- 4 RESET   reset 58XX I/O chips (data is in A0)
001---------101x   W -------- SUB RESET reset sound CPU (data is in A0)
001---------110x   W -------- n.c.
001---------111x   W -------- n.c.
110-xxxxxxxxxxxx R   xxxxxxxx ROM 1J    program ROM (optional, not used by any game)
111xxxxxxxxxxxxx R   xxxxxxxx ROM 1K    program ROM


----------------------------------------------------------------------------


Grobda
Namco, 1984

PCB Layout
----------

Top Board

PCB Number: 22109611 (22109631)
  |---------------------------------|
  |  0773         1502              |
  |  PAL          GR1-3.3M          |
  |               8148       VOL    |
  |  68A09EP      8148              |
  |                                 |
  |  GR1-4.1K                       |
  |                               --|
  |                               |
  |                               --|
|--|                                |
|  |                               2|
|  |                               2|
|  |                                |
|  |                               W|
|  |                     5806      A|
|  |                               Y|
|  |                     DSWB       |
|--|                                |
  |RESET_SW                         |
  |  GR2-3.1D                     --|
  |                               |
  |  GR2-2.1C                     --|
  |                      5604       |
  |  GR2-1.1B                       |
  |            PAL       1603  DSWA |
  |  68A09EP                        |
  |                       18.432MHz |
  |---------------------------------|

Notes:
      Namco Customs:
                    0773 - Clock Divider/Sync Generator
                    1502 - Sound Generator
                    5806 - I/O interface
                    5604 - I/O interface
                    1603 - I/O control

      6809 clocks: 1.536MHz (both)
      VSync: 60.606060Hz
      22 Way Connector pinout same as Galaga, Pacland, Baraduke, Dragon Buster etc.
      8148 - SRAM 1k x4
      GR1-3.3M - PROM type 82S129
      All ROMs - EPROM type 2764


Bottom Board

PCB Number: 22109612 (22109632)
  |---------------------------------|
  |                           8148  |
  |                                 |
  |                           8148  |
  |                                 |
  |                   GR1-4.3L      |
  |                                 |
  |           2016                  |
  |                                 |
  |                                 |
|--|          2016    1204          |
|  |                                |
|  |                                |
|  |          2016                  |
|  |                                |
|  |                                |
|  |                                |
|  |          0433    GR1-5.3F      |
|--|                                |
  |                                 |
  |           2016    GR1-6.3E      |
  |                        GR1-5.4E |
  |                                 |
  |           0042    1108     PAL  |
  |                                 |
  |                        GR1-6.4C |
  |           0763    GR1-7.3C      |
  |                                 |
  |---------------------------------|

Notes:
      Namco Customs:
                    1204 - Motion Object Position
                    0433 - Motion Object Controller
                    0042 - Video Ram Addresser
                    0763 - Clock Divider/Sync Generator
                    1108 - Datashift Playfield Register

     8148 - SRAM 1k x4
     2016 - SRAM 2x x8

     GR1-4.3L   \  PROMs type 82S129
     GR1-5.4E   /
     GR1-6.4C   -  PROM  type 82S123

     GR1-5.3F   \
     GR1-6.3E   /  EPROM type 2764

     GR1-7.3C   -  EPROM type 2732

----------------------------------------------------------------------------

Pacman & Chomp Chomp
Namco, 1983

This game runs on the same PCB as Grobda. Some custom chips have
different numbers but probably have the same purpose.

PCB Layout
----------

Top Board

PCB Number: 22109611 (22109631)
  |---------------------------------|
  |  0773         1500              |
  |  PAL          PAP1-3.3M         |
  |               8148       VOL    |
  |  6809         8148              |
  |                                 |
  |  PAP1_4.1K                      |
  |                               --|
  |                               |
  |                               --|
|--|                                |
|  |                               2|
|  |                               2|
|  |                                |
|  |                               W|
|  |                     5601      A|
|  |                               Y|
|  |                     DSWB       |
|--|                                |
  |RESET_SW                         |
  |  PAP3_3.1D                    --|
  |                               |
  |  PAP3_2.1C                    --|
  |                      5902       |
  |  PAP3-1.1B                      |
  |            PAL       1600  DSWA |
  |  6809                           |
  |                       18.432MHz |
  |---------------------------------|

Notes:
      Namco Customs:
                    0773 - Clock Divider/Sync Generator
                    1500 - Sound Generator
                    5902 - ? (protection?)
                    5601 - ? (protection?)
                    1600 - ?

      6809 clocks: 1.536MHz (both)
      VSync: 60.606060Hz
      22 Way Connector pinout same as Galaga, Pacland, Baraduke, Dragon Buster etc.
      8148 - SRAM 1k x4
      PAP1-3.3M - PROM type 82S129
      All ROMs - EPROM type 2764


Bottom Board

PCB Number: 22109612 (22109632)
  |---------------------------------|
  |                           8148  |
  |                                 |
  |                           8148  |
  |                                 |
  |                  PAP2-4.3L      |
  |                                 |
  |           2016                  |
  |                                 |
  |                                 |
|--|          2016    1200          |
|  |                                |
|  |                                |
|  |          2016                  |
|  |                                |
|  |                                |
|  |                                |
|  |          0433    PAP2_5.3F     |
|--|                                |
  |                   3E            |
  |           2016                  |
  |                       PAP2-5.4E |
  |                                 |
  |           0044    1100     PAL  |
  |                                 |
  |                       PAP2-6.4C |
  |           0773    PAP2_6.3C     |
  |                                 |
  |---------------------------------|

Notes:
      Namco Customs:
                    1200 - Motion Object Position
                    0433 - Motion Object Controller
                    0044 - Video Ram Addresser
                    0773 - Clock Divider/Sync Generator
                    1100 - Datashift Playfield Register

     8148 - SRAM 1k x4
     2016 - SRAM 2x x8

     PAP2-4.3L   \  PROMs type 82S129
     PAP2-5.4E   /
     PAP2-6.4C   -  PROM  type 82S123

     PAP2_5.3F   -  EPROM type 2764
     PAP2_6.3C   -  EPROM type 2732
     3E          -  Empty socket


----------------------------------------------------------------------------


Easter eggs:
-----------
Abbreviations:
U, D, L, R: player 1 joystick directions
B1, B2: player 1 buttons
S1, S2: start buttons
S: service switch (the one that adds a credit, not the one to enter service mode)

- Super Pac-Man
  - enter service mode
  - keep B1 pressed and enter the following sequence:
    R D D L L L L U
  (c) (p) 1982 NAMCO LTD. will be added at the bottom of the screen.

- Pac & Pal
  - enter service mode
  - enter the following sequence:
    L 9xR 5xD 6xU
  (c) (p) 1982 NAMCO LTD. will appear on the screen.

- Phozon
  - enter service mode
  - enter the following sequence:
    U U U U R D L L
  (c) (p) 1983 NAMCO LTD. will appear on the screen.

- Mappy
  - enter service mode
  - keep L pressed to make the screen scroll left until the grid covers the whole screen
  - press S 3 times
  - keep B1 pressed and enter the following sequence:
    4xL 6xS1 3xL S2
  (c) 1983 NAMCO will appear on the screen.

- Mappy
  - play the game and reach the third bonus round (the one after round 10)
  - press B1 3 times, S1 3 times, S2 3 times
  - after the score of the bonus round is shown, this text will be added at
    the bottom of the screen:
    (c) 1983 NAMCO
    ALL RIGHTS RESERVED

- The Tower of Druaga
  - enter service mode
  - select sound 19
  - press S to display the grid
  - enter the following sequence:
    4xU D 2xR 6xL S2
  (c) NAMCO LTD. 1984 will appear on the screen.

- Dig Dug II (New Ver. only, Old Ver. doesn't have it)
  - enter service mode
  - select sound 1B
  - press S to display the grid
  - enter the following sequence:
    D D R L L D D L L L U U
  - press S again
  Some music will play and
  DIGDUGII
  (c) 1985
  NAMCO LTD.
  will be slowly drawn on the screen.

- Grobda
  - enter service mode
  - set coin A to 1 Coin / 1 Credit
  - set coin B to 1 Coin / 4 Credits
  - set Freeze to On
  - (the above means DSW A = 11101001)
  - set Lives to 1
  - set Difficulty to Rank C
  - set Demo Sounds to Off
  - set Level Select to Off
  - set Bonus Life to 10k
  - (the above means DSW B = 10011100)
  - press S to display the grid
  - press (all together) R + B1 + S2
  (c) NAMCO LTD. 1984 will appear on the screen.
  To exit, press S1 or S coin

- Motos
  - enter service mode
  - set coinage to 3 Coins / 1 Credit
  - set Lives to 3
  - set Difficulty to Rank A
  - set Bonus Life to 10k 30k 50k
  - set Demo Sounds to Off
  - (the above means DSW A = 11100001)
  - press S to display the grid
  - press (all together) R + B1 + S2
  (c) NAMCO LTD. 1985 will appear on the screen.
  To exit, press S1 or S


Notes:
-----
- Phozon: completing level 34 (the second round of world 9) causes the game to
  crash. This seems to be the correct behaviour: there is no data for levels
  after that one, and using the level select dip switch to jump to level 35
  (="22") causes the same crash. I think being able to complete level 34 is a
  bug, it should be impossible because there shouldn't be enough molecules so
  you would play it forever.

- Phozon: if you enter service mode and press service coin something
  like the following is written at the bottom of the screen:
  99,99999,9999,9999,9999
  99,99999,9999,9999,999999
  it seems to be a counter decremented while the game is running.

- Mappy: similarly, if you enter service mode and press
  P1 button + service coin the following is shown:
  99.99.999.9999.9999.9999
  99.99.999.9999.9999.0000

- The Tower of Druaga: keep button 1 pressed while pressing the start button to
  continue the previous game; you can choose which level to start from.

- Grobda: when the Level Select dip switch is On, after inserting a coin press
  button + start to be asked the level to start from.

- The only difference between "grobda2" and "grobda3" is a tiny patch to some
  code related to player shoots handling. Bugfix, I guess.

- Dig Dug II: when the "freeze" dip switch is on, at the beginning of the game
  select starting level with joystick up/down, then press button 2. During the
  game, press start to pause/resume.


TODO:
----

- Phozon: there appear to be two palette banks (sprites are the same, but
  characters change from a blue background to brown background). It is not
  known if and when the alternate palette should be used. The palette select
  bit might be at address 500E/500F; if that's the case, it is set to 0 on
  startup and I don't think it's ever set to 1 - which would mean the second
  bank is not used and colors are right as they are.

- Phozon: one unknown PROM, I don't know what it could be.

***************************************************************************/

#include "emu.h"
#include "mappy.h"

#include "cpu/m6809/m6809.h"
#include "machine/74157.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "speaker.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

static constexpr XTAL MASTER_CLOCK = XTAL(18'432'000);

static constexpr XTAL PIXEL_CLOCK = MASTER_CLOCK / 3;

// H counts from 128->511, HBLANK starts at 144 and ends at 240
static constexpr unsigned HTOTAL  = 384;
static constexpr unsigned HBEND   = 0;     // (96+16)
static constexpr unsigned HBSTART = 288;   // (16)

static constexpr unsigned VTOTAL  = 264;
static constexpr unsigned VBEND   = 0;     // (16)
static constexpr unsigned VBSTART = 224;   // (224+16)



/***************************************************************************/

void mappy_state::main_int_on_w(int state)
{
	m_main_irq_mask = state;
	if (!state)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void mappy_state::sub_int_on_w(int state)
{
	m_sub_irq_mask = state;
	if (!state)
		m_subcpu->set_input_line(0, CLEAR_LINE);
}

void phozon_state::sub2_int_on_w(int state)
{
	m_sub2_irq_mask = state;
	if (!state)
		m_subcpu2->set_input_line(0, CLEAR_LINE);
}


template<uint8_t Chip>
TIMER_CALLBACK_MEMBER(mappy_state::namcoio_run_timer)
{
	m_namcoio[Chip]->customio_run();
}

void mappy_state::vblank_irq(int state)
{
	if (!state)
		return;

	if (m_main_irq_mask)
		m_maincpu->set_input_line(0, ASSERT_LINE);

	if (!m_namcoio[0]->read_reset_line())   // give the cpu a tiny bit of time to write the command before processing it
		m_namcoio_run_timer[0]->adjust(attotime::from_usec(50));

	if (!m_namcoio[1]->read_reset_line())   // give the cpu a tiny bit of time to write the command before processing it
		m_namcoio_run_timer[1]->adjust(attotime::from_usec(50));

	if (m_sub_irq_mask)
		m_subcpu->set_input_line(0, ASSERT_LINE);
}

void phozon_state::phozon_vblank_irq(int state)
{
	if (!state)
		return;

	vblank_irq(state);

	if (m_sub2_irq_mask)
		m_subcpu2->set_input_line(0, ASSERT_LINE);
}

void mappy_state::superpac_main_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().w(FUNC(mappy_state::superpac_videoram_w)).share(m_videoram);
	map(0x0800, 0x1fff).ram().share(m_spriteram);   // work RAM with embedded sprite RAM
	map(0x2000, 0x2000).rw(FUNC(mappy_state::flipscreen_r), FUNC(mappy_state::flipscreen_w));
	map(0x4000, 0x43ff).rw(m_namco_15xx, FUNC(namco_15xx_device::sharedram_r), FUNC(namco_15xx_device::sharedram_w));   // shared RAM with the sound CPU
	map(0x4800, 0x480f).rw(m_namcoio[0], FUNC(namcoio_device::read), FUNC(namcoio_device::write));   // custom I/O chips interface
	map(0x4810, 0x481f).rw(m_namcoio[1], FUNC(namcoio_device::read), FUNC(namcoio_device::write));   // custom I/O chips interface
	map(0x5000, 0x500f).w("mainlatch", FUNC(ls259_device::write_a0));   // various control bits
	map(0x8000, 0x8000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xa000, 0xffff).rom();
}

void phozon_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().w(FUNC(phozon_state::superpac_videoram_w)).share(m_videoram);  // video RAM
	map(0x0800, 0x1fff).ram().share(m_spriteram);   // shared RAM with CPU #2/sprite RAM
	map(0x4000, 0x43ff).rw(m_namco_15xx, FUNC(namco_15xx_device::sharedram_r), FUNC(namco_15xx_device::sharedram_w));   // shared RAM with the sound CPU
	map(0x4800, 0x480f).rw(m_namcoio[0], FUNC(namcoio_device::read), FUNC(namcoio_device::write));   // custom I/O chips interface
	map(0x4810, 0x481f).rw(m_namcoio[1], FUNC(namcoio_device::read), FUNC(namcoio_device::write));   // custom I/O chips interface
	map(0x5000, 0x500f).w("mainlatch", FUNC(ls259_device::write_a0));   // various control bits
	map(0x7000, 0x7000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x8000, 0xffff).rom();
}

void mappy_state::mappy_main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().w(FUNC(mappy_state::mappy_videoram_w)).share(m_videoram);
	map(0x1000, 0x27ff).ram().share(m_spriteram);   // work RAM with embedded sprite RAM
	map(0x3800, 0x3fff).w(FUNC(mappy_state::mappy_scroll_w));   // scroll
	map(0x4000, 0x43ff).rw(m_namco_15xx, FUNC(namco_15xx_device::sharedram_r), FUNC(namco_15xx_device::sharedram_w));   // shared RAM with the sound CPU
	map(0x4800, 0x480f).rw(m_namcoio[0], FUNC(namcoio_device::read), FUNC(namcoio_device::write));   // custom I/O chips interface
	map(0x4810, 0x481f).rw(m_namcoio[1], FUNC(namcoio_device::read), FUNC(namcoio_device::write));   // custom I/O chips interface
	map(0x5000, 0x500f).w("mainlatch", FUNC(ls259_device::write_a0));   // various control bits
	map(0x8000, 0x8000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x8000, 0xffff).rom();  // only a000-ffff in Mappy
}

void mappy_state::superpac_sub_map(address_map &map)
{
	map(0x0000, 0x03ff).rw(m_namco_15xx, FUNC(namco_15xx_device::sharedram_r), FUNC(namco_15xx_device::sharedram_w));   // shared RAM with the main CPU (also sound registers)
	map(0x2000, 0x200f).w("mainlatch", FUNC(ls259_device::write_a0));   // various control bits
	map(0xe000, 0xffff).rom();
}

void phozon_state::sub_map(address_map &map)
{
	map(0x0000, 0x03ff).rw(m_namco_15xx, FUNC(namco_15xx_device::sharedram_r), FUNC(namco_15xx_device::sharedram_w));   // shared RAM with the main CPU + sound registers
	map(0xe000, 0xffff).rom();
}


// extra CPU only present in Phozon
void phozon_state::sub2_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().w(FUNC(phozon_state::superpac_videoram_w)).share(m_videoram);
	map(0x0800, 0x1fff).ram().share(m_spriteram);   // shared RAM with CPU #2/sprite RAM
	map(0x4000, 0x43ff).rw(m_namco_15xx, FUNC(namco_15xx_device::sharedram_r), FUNC(namco_15xx_device::sharedram_w));   // shared RAM with CPU #2
	map(0xa000, 0xa7ff).ram();
	map(0xe000, 0xffff).rom();
}



#define NAMCO_56IN0\
	PORT_START("P1")    /* 56XX #0 pins 22-29 */\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY\
	PORT_START("P2")    /* 56XX #0 pins 22-29 */\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL

#define NAMCO_5XIN0\
	PORT_START("P1") /* 56XX #0 pins 22-29 */\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY\
	PORT_START("P2") /* 56XX #0 pins 22-29 */\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

#define NAMCO_56IN1\
	PORT_START("BUTTONS")   /* 56XX #0 pins 30-33 and 38-41 */\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )\
	PORT_START("COINS") /* 56XX #0 pins 30-33 and 38-41 */\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )

#define NAMCO_56DSW0\
	PORT_START("DSW0")  /* 56XX #1 pins 30-33 */\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )\
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )\
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )\
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )\
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )

static INPUT_PORTS_START( superpac )
	NAMCO_56IN0
	NAMCO_56IN1
	NAMCO_56DSW0

	PORT_START("DSW1")  // 56XX #1 pins 22-29
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "Rank 0-Normal" )
	PORT_DIPSETTING(    0x0e, "Rank 1-Easiest" )
	PORT_DIPSETTING(    0x0d, "Rank 2" )
	PORT_DIPSETTING(    0x0c, "Rank 3" )
	PORT_DIPSETTING(    0x0b, "Rank 4" )
	PORT_DIPSETTING(    0x0a, "Rank 5" )
	PORT_DIPSETTING(    0x09, "Rank 6-Medium" )
	PORT_DIPSETTING(    0x08, "Rank 7" )
	PORT_DIPSETTING(    0x07, "Rank 8-Default" )
	PORT_DIPSETTING(    0x06, "Rank 9" )
	PORT_DIPSETTING(    0x05, "Rank A" )
	PORT_DIPSETTING(    0x04, "Rank B-Hardest" )
	PORT_DIPSETTING(    0x03, "Rank C-Easy Auto" )
	PORT_DIPSETTING(    0x02, "Rank D-Auto" )
	PORT_DIPSETTING(    0x01, "Rank E-Auto" )
	PORT_DIPSETTING(    0x00, "Rank F-Hard Auto" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	// When Freeze is on, press P1 button 1 to skip levels
	PORT_DIPNAME( 0x80, 0x80, "Freeze / Rack Test (Cheat)" ) PORT_TOGGLE PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  // 56XX #1 pins 38-41 multiplexed
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x08, "30k Only" )                  PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x30, "30k & 80k Only" )            PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x20, "30k, 80k & Every 80k" )      PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x38, "30k & 100k Only" )           PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x18, "30k, 100k & Every 100k" )    PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x28, "30k & 120k Only" )           PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x10, "30k, 120k & Every 120k" )    PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x10, "30k Only" )                  PORT_CONDITION("DSW2",0xc0,EQUALS,0x00) // Manual shows 100k only, Test Mode shows 30k which is what we use
	PORT_DIPSETTING(    0x38, "30k & 100k Only" )           PORT_CONDITION("DSW2",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "30k, 100k & Every 100k" )    PORT_CONDITION("DSW2",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "30k & 120k Only" )           PORT_CONDITION("DSW2",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x08, "40k Only" )                  PORT_CONDITION("DSW2",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x28, "40k & 120k Only" )           PORT_CONDITION("DSW2",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x18, "40k, 120k & Every 120k" )    PORT_CONDITION("DSW2",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x00, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( pacnpal )
	NAMCO_56IN0
	NAMCO_56IN1
	NAMCO_56DSW0

	PORT_START("DSW1")  // ???? #1 pins ??-??
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x38, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x20, "20k & 70k Only" )            PORT_CONDITION("DSW1",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x30, "20k, 70k & Every 70k" )      PORT_CONDITION("DSW1",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x00, "30k Only" )                  PORT_CONDITION("DSW1",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x18, "30k & 70k Only" )            PORT_CONDITION("DSW1",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x10, "30k & 80k Only" )            PORT_CONDITION("DSW1",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x28, "30k, 100k & Every 80k" )     PORT_CONDITION("DSW1",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x08, "30k & 100k Only" )           PORT_CONDITION("DSW1",0xc0,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x08, "30k Only" )                  PORT_CONDITION("DSW1",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "40k Only" )                  PORT_CONDITION("DSW1",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "30k & 80k Only" )            PORT_CONDITION("DSW1",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "30k, 80k & Every 80k" )      PORT_CONDITION("DSW1",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x18, "30k & 100k Only" )           PORT_CONDITION("DSW1",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "40k & 120k Only" )           PORT_CONDITION("DSW1",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x28, "40k, 100k & Every 100k" )    PORT_CONDITION("DSW1",0xc0,EQUALS,0x00)
	PORT_DIPSETTING(    0x38, DEF_STR( None ) )

	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW2")  // ???? #1 pins ??-??
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "Rank A" )
	PORT_DIPSETTING(    0x08, "Rank B" )
	PORT_DIPSETTING(    0x04, "Rank C" )
	PORT_DIPSETTING(    0x00, "Rank D" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( grobda )
	NAMCO_5XIN0
	NAMCO_56IN1

	PORT_START("DSW0")  // 56XX #1 pins 30-33
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )    // service mode again

	PORT_START("DSW1")  // 56XX #1 pins 22-29
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0e, 0x08, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x70, 0x40, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  // 56XX #1 pins 38-41 multiplexed
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "Rank A" )
	PORT_DIPSETTING(    0x08, "Rank B" )
	PORT_DIPSETTING(    0x04, "Rank C" )
	PORT_DIPSETTING(    0x00, "Rank D" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Level_Select ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, "10k, 50k & Every 50k" )
	PORT_DIPSETTING(    0x40, "10k & 30k Only" )
	PORT_DIPSETTING(    0xc0, "10k Only" )
	PORT_DIPSETTING(    0x80, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( phozon )
	NAMCO_5XIN0
	NAMCO_56IN1
	NAMCO_56DSW0

	PORT_START("DSW1")  // 56XX #1 pins 22-29
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(    0x0e, "Rank 0" )
	PORT_DIPSETTING(    0x0c, "Rank 1" )
	PORT_DIPSETTING(    0x0a, "Rank 2" )
	PORT_DIPSETTING(    0x08, "Rank 3" )
	PORT_DIPSETTING(    0x06, "Rank 4" )
	PORT_DIPSETTING(    0x04, "Rank 5" )
	PORT_DIPSETTING(    0x02, "Rank 6" )
	PORT_DIPSETTING(    0x00, "Rank 7" )
	// when level select is on, press P1 start during the game and move joystick to select level to jump to
	PORT_DIPNAME( 0x10, 0x10, "Level Select (Cheat)" )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// when stop mode is on, press P1 start to pause the game
	PORT_DIPNAME( 0x20, 0x20, "Stop Mode (Cheat)" )     PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")  // 56XX #1 pins 38-41 multiplexed and interleaved
	PORT_DIPNAME( 0x11, 0x11, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x01, "1" ) // 1 on, 2 off
	PORT_DIPSETTING(    0x11, "3" )
	PORT_DIPSETTING(    0x10, "4" ) // 2 on, 1 off
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x62, 0x62, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,3,5")
	PORT_DIPSETTING(    0x02, "20k & 80k Only" )            PORT_CONDITION("DSW2",0x01,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x40, "20k, 80k & Every 80k" )      PORT_CONDITION("DSW2",0x01,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x20, "30k Only" )                  PORT_CONDITION("DSW2",0x01,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x42, "30k & 60k Only" )            PORT_CONDITION("DSW2",0x01,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x62, "30k & 100k Only" )           PORT_CONDITION("DSW2",0x01,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x60, "30k & 100k Only" )           PORT_CONDITION("DSW2",0x01,NOTEQUALS,0x00)  // repeated
	PORT_DIPSETTING(    0x22, "30k, 120k & Every 120k" )    PORT_CONDITION("DSW2",0x01,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x22, "20k & 80k Only" )            PORT_CONDITION("DSW2",0x01,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "30k Only" )                  PORT_CONDITION("DSW2",0x01,EQUALS,0x00)
	PORT_DIPSETTING(    0x40, "30k, 100k & Every 100k" )    PORT_CONDITION("DSW2",0x01,EQUALS,0x00)
	PORT_DIPSETTING(    0x62, "30k & 100k Only" )           PORT_CONDITION("DSW2",0x01,EQUALS,0x00)
	PORT_DIPSETTING(    0x60, "30k & 100k Only" )           PORT_CONDITION("DSW2",0x01,EQUALS,0x00) // repeated
	PORT_DIPSETTING(    0x42, "40k & 80k Only" )            PORT_CONDITION("DSW2",0x01,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "100k Only" )                 PORT_CONDITION("DSW2",0x01,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x8c, 0x8c, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:6,8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x8c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x88, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x84, DEF_STR( 1C_7C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mappy )
	PORT_START("P1")    // 58XX #0 pins 22-29
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_START("P2")    // 58XX #0 pins 22-29
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL

	NAMCO_56IN1
	NAMCO_56DSW0

	PORT_START("DSW1")  // 58XX #1 pins 22-29
// According to the manual, 0x04, 0x08 and 0x10 should always be off, but...
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty )  )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "Rank A" )
	PORT_DIPSETTING(    0x06, "Rank B" )
	PORT_DIPSETTING(    0x05, "Rank C" )
	PORT_DIPSETTING(    0x04, "Rank D" )
	PORT_DIPSETTING(    0x03, "Rank E" )
	PORT_DIPSETTING(    0x02, "Rank F" )
	PORT_DIPSETTING(    0x01, "Rank G" )
	PORT_DIPSETTING(    0x00, "Rank H" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1)   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  // 58XX #1 pins 38-41 multiplexed
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x18, "20k Only" )                  PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x80)
	PORT_DIPSETTING(    0x30, "20k & 60k Only" )            PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x80)
	PORT_DIPSETTING(    0x38, "20k & 70k Only" )            PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x80)
	PORT_DIPSETTING(    0x10, "20k, 70k & Every 70k" )      PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x80)
	PORT_DIPSETTING(    0x28, "20k & 80k Only" )            PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x80)
	PORT_DIPSETTING(    0x08, "20k, 80k & Every 80k" )      PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x80)
	PORT_DIPSETTING(    0x20, "30k & 100k Only" )           PORT_CONDITION("DSW2",0xc0,NOTEQUALS,0x80)
	PORT_DIPSETTING(    0x20, "30k Only" )                  PORT_CONDITION("DSW2",0xc0,EQUALS,0x80)
	PORT_DIPSETTING(    0x38, "30k & 80k Only" )            PORT_CONDITION("DSW2",0xc0,EQUALS,0x80)
	PORT_DIPSETTING(    0x30, "30k & 100k Only" )           PORT_CONDITION("DSW2",0xc0,EQUALS,0x80)
	PORT_DIPSETTING(    0x10, "30k, 100k & Every 100k" )    PORT_CONDITION("DSW2",0xc0,EQUALS,0x80)
	PORT_DIPSETTING(    0x28, "30k & 120k Only" )           PORT_CONDITION("DSW2",0xc0,EQUALS,0x80)
	PORT_DIPSETTING(    0x18, "40k Only" )                  PORT_CONDITION("DSW2",0xc0,EQUALS,0x80)
	PORT_DIPSETTING(    0x08, "40k, 120k & Every 120k" )    PORT_CONDITION("DSW2",0xc0,EQUALS,0x80)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( todruaga )
	NAMCO_56IN0
	NAMCO_56IN1

	PORT_START("DSW0")  // 56XX #1 pins 30-33
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )    // service mode again

	PORT_START("DSW1")  // 56XX #1 pins 22-29
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, "Freeze" )                PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")  // 56XX #1 pins 38-41 multiplexed
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( digdug2 )
	NAMCO_56IN0
	NAMCO_56IN1

	PORT_START("DSW0")  // 56XX #1 pins 30-33
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )    // service mode again

	PORT_START("DSW1")  // 56XX #1 pins 22-29
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "30k 80k and ..." )
	PORT_DIPSETTING(    0x20, "30k 100k and ..." )
	PORT_DIPSETTING(    0x10, "30k 120k and ..." )
	PORT_DIPSETTING(    0x00, "30k 150k and..." )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Level_Select ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  // 56XX #1 pins 38-41 multiplexed
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( motos )
	NAMCO_5XIN0
	NAMCO_56IN1

	PORT_START("DSW0")  // 56XX #1 pins 30-33
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )    // service mode again

	PORT_START("DSW1")  // 56XX #1 pins 22-29
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Rank A" )
	PORT_DIPSETTING(    0x00, "Rank B" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "10k 30k and every 50k" )
	PORT_DIPSETTING(    0x40, "20k and every 50k" )
	PORT_DIPSETTING(    0x20, "30k and every 70k" )
	PORT_DIPSETTING(    0x00, "20k 70k" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")  // 56XX #1 pins 38-41 multiplexed
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ STEP2(0, 4) },
	{ STEP4(8*8, 1), STEP4(0, 1) },
	{ STEP8(0, 8) },
	8*8*2
};

static const gfx_layout spritelayout_2bpp =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ STEP2(0, 4) },
	{ STEP4(0, 1), STEP4(8*8, 1), STEP4(8*8*2, 1), STEP4(8*8*3, 1) },
	{ STEP8(0, 8), STEP8(8*8*4, 8) },
	16*16*2
};

static const gfx_layout spritelayout_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ STEP2(0, 4) },
	{ STEP4(0, 1), STEP4(8*8, 1) },
	{ STEP8(0, 8) },
	8*8*2
};

static const gfx_layout spritelayout_4bpp =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0, 4) },
	{ STEP4(0, 1), STEP4(8*16, 1), STEP4(8*16*2, 1), STEP4(8*16*3, 1) },
	{ STEP8(0, 16), STEP8(8*16*4, 16) },
	16*16*4
};



static GFXDECODE_START( gfx_superpac )
	GFXDECODE_ENTRY( "tiles",   0, charlayout,           0, 64 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout_2bpp, 64*4, 64 )
GFXDECODE_END

static GFXDECODE_START( gfx_phozon )
	GFXDECODE_ENTRY( "tiles",   0, charlayout,          0, 64 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout_8x8, 64*4, 64 )
GFXDECODE_END

static GFXDECODE_START( gfx_mappy )
	GFXDECODE_ENTRY( "tiles",   0, charlayout,           0, 64 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout_4bpp, 64*4, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_todruaga )
	GFXDECODE_ENTRY( "tiles",   0, charlayout,           0, 64 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout_4bpp, 64*4, 64 )
GFXDECODE_END


/***************************************************************************

  Custom I/O initialization

***************************************************************************/

void mappy_state::out_lamps(uint8_t data)
{
	m_leds[0] = BIT(data, 0);
	m_leds[1] = BIT(data, 1);
	machine().bookkeeping().coin_lockout_global_w(BIT(data, 2));
	machine().bookkeeping().coin_counter_w(0, BIT(~data, 3));
}

void mappy_state::machine_start()
{
	m_leds.resolve();

	m_namcoio_run_timer[0] = timer_alloc(FUNC(mappy_state::namcoio_run_timer<0>), this);
	m_namcoio_run_timer[1] = timer_alloc(FUNC(mappy_state::namcoio_run_timer<1>), this);

	save_item(NAME(m_main_irq_mask));
	save_item(NAME(m_sub_irq_mask));
}

void phozon_state::machine_start()
{
	mappy_state::machine_start();

	save_item(NAME(m_sub2_irq_mask));
}


void mappy_state::superpac_common(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, PIXEL_CLOCK/4);  // 1.536 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &mappy_state::superpac_main_map);

	MC6809E(config, m_subcpu, PIXEL_CLOCK/4);   // 1.536 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &mappy_state::superpac_sub_map);

	ls259_device &mainlatch(LS259(config, "mainlatch"));    // 2M on CPU board
	mainlatch.q_out_cb<0>().set(FUNC(mappy_state::sub_int_on_w));
	mainlatch.q_out_cb<1>().set(FUNC(mappy_state::main_int_on_w));
	mainlatch.q_out_cb<3>().set(m_namco_15xx, FUNC(namco_15xx_device::sound_enable_w));
	mainlatch.q_out_cb<4>().set(m_namcoio[0], FUNC(namcoio_device::set_reset_line)).invert();
	mainlatch.q_out_cb<4>().append(m_namcoio[1], FUNC(namcoio_device::set_reset_line)).invert();
	mainlatch.q_out_cb<5>().set_inputline(m_subcpu, INPUT_LINE_RESET).invert();

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 8);

	config.set_maximum_quantum(attotime::from_hz(6000));    // 100 CPU slices per frame - a high value to ensure proper synchronization of the CPUs

	ls157_device &dipmux(LS157(config, "dipmux"));
	dipmux.a_in_callback().set_ioport("DSW2");
	dipmux.b_in_callback().set_ioport("DSW2").rshift(4);

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_superpac);
	PALETTE(config, m_palette, FUNC(mappy_state::superpac_palette), 64*4+64*4, 32);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(mappy_state::screen_update_superpac));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(mappy_state::vblank_irq));   // cause IRQs on both CPUs; also update the custom I/O chips

	MCFG_VIDEO_START_OVERRIDE(mappy_state,superpac)

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	NAMCO_15XX(config, m_namco_15xx, MASTER_CLOCK/768);
	m_namco_15xx->set_voices(8);
	m_namco_15xx->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

void mappy_state::superpac(machine_config &config)
{
	superpac_common(config);

	NAMCO_56XX(config, m_namcoio[0], 0);
	m_namcoio[0]->in_callback<0>().set_ioport("COINS");
	m_namcoio[0]->in_callback<1>().set_ioport("P1");
	m_namcoio[0]->in_callback<2>().set_ioport("P2");
	m_namcoio[0]->in_callback<3>().set_ioport("BUTTONS");

	NAMCO_56XX(config, m_namcoio[1], 0);
	m_namcoio[1]->in_callback<0>().set("dipmux", FUNC(ls157_device::output_r));
	m_namcoio[1]->in_callback<1>().set_ioport("DSW1");
	m_namcoio[1]->in_callback<2>().set_ioport("DSW1").rshift(4);
	m_namcoio[1]->in_callback<3>().set_ioport("DSW0");
	m_namcoio[1]->out_callback<0>().set("dipmux", FUNC(ls157_device::select_w)).bit(0);
}

void mappy_state::pacnpal(machine_config &config)
{
	superpac_common(config);

	NAMCO_56XX(config, m_namcoio[0], 0);
	m_namcoio[0]->in_callback<0>().set_ioport("COINS");
	m_namcoio[0]->in_callback<1>().set_ioport("P1");
	m_namcoio[0]->in_callback<2>().set_ioport("P2");
	m_namcoio[0]->in_callback<3>().set_ioport("BUTTONS");
	m_namcoio[0]->out_callback<0>().set(FUNC(mappy_state::out_lamps));

	NAMCO_59XX(config, m_namcoio[1], 0);
	m_namcoio[1]->in_callback<0>().set("dipmux", FUNC(ls157_device::output_r));
	m_namcoio[1]->in_callback<1>().set_ioport("DSW1");
	m_namcoio[1]->in_callback<2>().set_ioport("DSW1").rshift(4);
	m_namcoio[1]->in_callback<3>().set_ioport("DSW0");
	m_namcoio[1]->out_callback<0>().set("dipmux", FUNC(ls157_device::select_w)).bit(0);
}

void mappy_state::grobda(machine_config &config)
{
	superpac_common(config);

	NAMCO_58XX(config, m_namcoio[0], 0);
	m_namcoio[0]->in_callback<0>().set_ioport("COINS");
	m_namcoio[0]->in_callback<1>().set_ioport("P1");
	m_namcoio[0]->in_callback<2>().set_ioport("P2");
	m_namcoio[0]->in_callback<3>().set_ioport("BUTTONS");

	NAMCO_56XX(config, m_namcoio[1], 0);
	m_namcoio[1]->in_callback<0>().set("dipmux", FUNC(ls157_device::output_r));
	m_namcoio[1]->in_callback<1>().set_ioport("DSW1");
	m_namcoio[1]->in_callback<2>().set_ioport("DSW1").rshift(4);
	m_namcoio[1]->in_callback<3>().set_ioport("DSW0");
	m_namcoio[1]->out_callback<0>().set("dipmux", FUNC(ls157_device::select_w)).bit(0);
}

void phozon_state::phozon(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, PIXEL_CLOCK/4);  // MAIN CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &phozon_state::main_map);

	MC6809E(config, m_subcpu, PIXEL_CLOCK/4);   // SOUND CPU
	m_subcpu->set_addrmap(AS_PROGRAM, &phozon_state::sub_map);

	MC6809E(config, m_subcpu2, PIXEL_CLOCK/4);  // SUB CPU
	m_subcpu2->set_addrmap(AS_PROGRAM, &phozon_state::sub2_map);

	ls259_device &mainlatch(LS259(config, "mainlatch"));    // 5C
	mainlatch.q_out_cb<0>().set(FUNC(phozon_state::sub_int_on_w));
	mainlatch.q_out_cb<1>().set(FUNC(phozon_state::main_int_on_w));
	mainlatch.q_out_cb<2>().set(FUNC(phozon_state::sub2_int_on_w));
	mainlatch.q_out_cb<3>().set(m_namco_15xx, FUNC(namco_15xx_device::sound_enable_w));
	mainlatch.q_out_cb<4>().set(m_namcoio[0], FUNC(namcoio_device::set_reset_line)).invert();
	mainlatch.q_out_cb<4>().append(m_namcoio[1], FUNC(namcoio_device::set_reset_line)).invert();
	mainlatch.q_out_cb<5>().set_inputline(m_subcpu, INPUT_LINE_RESET).invert();
	mainlatch.q_out_cb<6>().set_inputline(m_subcpu2, INPUT_LINE_RESET).invert();

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 8);

	config.set_maximum_quantum(attotime::from_hz(6000));    // 100 CPU slices per frame - a high value to ensure proper synchronization of the CPUs

	NAMCO_58XX(config, m_namcoio[0], 0);
	m_namcoio[0]->in_callback<0>().set_ioport("COINS");
	m_namcoio[0]->in_callback<1>().set_ioport("P1");
	m_namcoio[0]->in_callback<2>().set_ioport("P2");
	m_namcoio[0]->in_callback<3>().set_ioport("BUTTONS");

	NAMCO_56XX(config, m_namcoio[1], 0);
	m_namcoio[1]->in_callback<0>().set("dipmux", FUNC(ls157_device::output_r));
	m_namcoio[1]->in_callback<1>().set_ioport("DSW1");
	m_namcoio[1]->in_callback<2>().set_ioport("DSW1").rshift(4);
	m_namcoio[1]->in_callback<3>().set_ioport("DSW0");
	m_namcoio[1]->out_callback<0>().set("dipmux", FUNC(ls157_device::select_w)).bit(0);

	ls157_device &dipmux(LS157(config, "dipmux"));
	dipmux.a_in_callback().set_ioport("DSW2");
	dipmux.b_in_callback().set_ioport("DSW2").rshift(4);

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_phozon);
	PALETTE(config, m_palette, FUNC(phozon_state::palette), 64*4+64*4, 32);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(phozon_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(phozon_state::phozon_vblank_irq));   // cause IRQs on all three CPUs; also update the custom I/O chips

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	NAMCO_15XX(config, m_namco_15xx, MASTER_CLOCK/768);
	m_namco_15xx->set_voices(8);
	m_namco_15xx->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

void mappy_state::mappy_common(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, PIXEL_CLOCK/4);  // 1.536 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &mappy_state::mappy_main_map);

	MC6809E(config, m_subcpu, PIXEL_CLOCK/4);   // 1.536 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &mappy_state::superpac_sub_map);

	ls259_device &mainlatch(LS259(config, "mainlatch"));    // 2M on CPU board
	mainlatch.q_out_cb<0>().set(FUNC(mappy_state::sub_int_on_w));
	mainlatch.q_out_cb<1>().set(FUNC(mappy_state::main_int_on_w));
	mainlatch.q_out_cb<2>().set(FUNC(mappy_state::flip_screen_set));
	mainlatch.q_out_cb<3>().set(m_namco_15xx, FUNC(namco_15xx_device::sound_enable_w));
	mainlatch.q_out_cb<4>().set(m_namcoio[0], FUNC(namcoio_device::set_reset_line)).invert();
	mainlatch.q_out_cb<4>().append(m_namcoio[1], FUNC(namcoio_device::set_reset_line)).invert();
	mainlatch.q_out_cb<5>().set_inputline(m_subcpu, INPUT_LINE_RESET).invert();

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 8);

	config.set_maximum_quantum(attotime::from_hz(6000));    // 100 CPU slices per frame - a high value to ensure proper synchronization of the CPUs

	ls157_device &dipmux(LS157(config, "dipmux"));
	dipmux.a_in_callback().set_ioport("DSW2");
	dipmux.b_in_callback().set_ioport("DSW2").rshift(4);

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mappy);
	PALETTE(config, m_palette, FUNC(mappy_state::mappy_palette), 64*4+16*16, 32);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(mappy_state::screen_update_mappy));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(mappy_state::vblank_irq));   // cause IRQs on both CPUs; also update the custom I/O chips

	MCFG_VIDEO_START_OVERRIDE(mappy_state,mappy)

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	NAMCO_15XX(config, m_namco_15xx, MASTER_CLOCK/768);
	m_namco_15xx->set_voices(8);
	m_namco_15xx->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

void mappy_state::mappy(machine_config &config)
{
	mappy_common(config);

	NAMCO_58XX(config, m_namcoio[0], 0);
	m_namcoio[0]->in_callback<0>().set_ioport("COINS");
	m_namcoio[0]->in_callback<1>().set_ioport("P1");
	m_namcoio[0]->in_callback<2>().set_ioport("P2");
	m_namcoio[0]->in_callback<3>().set_ioport("BUTTONS");

	NAMCO_58XX(config, m_namcoio[1], 0);
	m_namcoio[1]->in_callback<0>().set("dipmux", FUNC(ls157_device::output_r));
	m_namcoio[1]->in_callback<1>().set_ioport("DSW1");
	m_namcoio[1]->in_callback<2>().set_ioport("DSW1").rshift(4);
	m_namcoio[1]->in_callback<3>().set_ioport("DSW0");
	m_namcoio[1]->out_callback<0>().set("dipmux", FUNC(ls157_device::select_w)).bit(0);
}

void mappy_state::digdug2(machine_config &config)
{
	mappy_common(config);

	subdevice<watchdog_timer_device>("watchdog")->set_vblank_count("screen", 0);

	NAMCO_58XX(config, m_namcoio[0], 0);
	m_namcoio[0]->in_callback<0>().set_ioport("COINS");
	m_namcoio[0]->in_callback<1>().set_ioport("P1");
	m_namcoio[0]->in_callback<2>().set_ioport("P2");
	m_namcoio[0]->in_callback<3>().set_ioport("BUTTONS");

	NAMCO_56XX(config, m_namcoio[1], 0);
	m_namcoio[1]->in_callback<0>().set("dipmux", FUNC(ls157_device::output_r));
	m_namcoio[1]->in_callback<1>().set_ioport("DSW1");
	m_namcoio[1]->in_callback<2>().set_ioport("DSW1").rshift(4);
	m_namcoio[1]->in_callback<3>().set_ioport("DSW0");
	m_namcoio[1]->out_callback<0>().set("dipmux", FUNC(ls157_device::select_w)).bit(0);
}

void mappy_state::todruaga(machine_config &config)
{
	digdug2(config);

	// video hardware
	m_gfxdecode->set_info(gfx_todruaga);
	m_palette->set_entries(64*4+64*16);
}

void mappy_state::motos(machine_config &config)
{
	mappy_common(config);

	NAMCO_56XX(config, m_namcoio[0], 0);
	m_namcoio[0]->in_callback<0>().set_ioport("COINS");
	m_namcoio[0]->in_callback<1>().set_ioport("P1");
	m_namcoio[0]->in_callback<2>().set_ioport("P2");
	m_namcoio[0]->in_callback<3>().set_ioport("BUTTONS");
	m_namcoio[0]->out_callback<0>().set(FUNC(mappy_state::out_lamps));

	NAMCO_56XX(config, m_namcoio[1], 0);
	m_namcoio[1]->in_callback<0>().set("dipmux", FUNC(ls157_device::output_r));
	m_namcoio[1]->in_callback<1>().set_ioport("DSW1");
	m_namcoio[1]->in_callback<2>().set_ioport("DSW1").rshift(4);
	m_namcoio[1]->in_callback<3>().set_ioport("DSW0");
	m_namcoio[1]->out_callback<0>().set("dipmux", FUNC(ls157_device::select_w)).bit(0);
}



ROM_START( superpac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp1-2.1c",     0xc000, 0x2000, CRC(4bb33d9c) SHA1(dd87f71b4db090a32a6b791079eedd17580cc741) )
	ROM_LOAD( "sp1-1.1b",     0xe000, 0x2000, CRC(846fbb4a) SHA1(f6bf90281986b9b7a3ef1dbbeddb722182e84d7c) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "spc-3.1k",     0xf000, 0x1000, CRC(04445ddb) SHA1(ce7d14963d5ddaefdeaf433a6f82c43cd1611d9b) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "sp1-6.3c",     0x0000, 0x1000, CRC(91c5935c) SHA1(10579edabc26a0910253fab7d41b4c19ecdaaa09) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "spv-2.3f",     0x0000, 0x2000, CRC(670a42f2) SHA1(9171922df07e31fd1dc415766f7d2cc50a9d10dc) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "superpac.4c",  0x0000, 0x0020, CRC(9ce22c46) SHA1(d97f53ef4c5ef26659a22ed0de4ce7ef3758c924) )    // palette
	ROM_LOAD( "superpac.4e",  0x0020, 0x0100, CRC(1253c5c1) SHA1(df46a90170e9761d45c90fbd04ef2aa1e8c9944b) )    // chars
	ROM_LOAD( "superpac.3l",  0x0120, 0x0100, CRC(d4d7026f) SHA1(a486573437c54bfb503424574ad82655491e85e1) )    // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "superpac.3m",  0x0000, 0x0100, CRC(ad43688f) SHA1(072f427453efb1dda8147da61804fff06e1bc4d5) )
ROM_END

ROM_START( superpacm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spc-2.1c",     0xc000, 0x2000, CRC(1a38c30e) SHA1(ae0ee9f3df0991a80698fe745a7a853a4bb60710) )
	ROM_LOAD( "spc-1.1b",     0xe000, 0x2000, CRC(730e95a9) SHA1(ca73c8bcb03c2f5c05968c707a5d3f7f9956b886) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "spc-3.1k",     0xf000, 0x1000, CRC(04445ddb) SHA1(ce7d14963d5ddaefdeaf433a6f82c43cd1611d9b) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "spv-1.3c",     0x0000, 0x1000, CRC(78337e74) SHA1(11222adb55e6bce508896ccb1f6dbab0c1d44e5b) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "spv-2.3f",     0x0000, 0x2000, CRC(670a42f2) SHA1(9171922df07e31fd1dc415766f7d2cc50a9d10dc) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "superpac.4c",  0x0000, 0x0020, CRC(9ce22c46) SHA1(d97f53ef4c5ef26659a22ed0de4ce7ef3758c924) )    // palette
	ROM_LOAD( "superpac.4e",  0x0020, 0x0100, CRC(1253c5c1) SHA1(df46a90170e9761d45c90fbd04ef2aa1e8c9944b) )    // chars
	ROM_LOAD( "superpac.3l",  0x0120, 0x0100, CRC(d4d7026f) SHA1(a486573437c54bfb503424574ad82655491e85e1) )    // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "superpac.3m",  0x0000, 0x0100, CRC(ad43688f) SHA1(072f427453efb1dda8147da61804fff06e1bc4d5) )
ROM_END

ROM_START( pacnpal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pap1-3b.1d",    0xa000, 0x2000, CRC(ed64a565) SHA1(b16930981490d97486d4df96acbb3d1cddbd3a80) )
	ROM_LOAD( "pap1-2b.1c",    0xc000, 0x2000, CRC(15308bcf) SHA1(334603f8904f8968d05edc420b5f9e3b483ee86d) )
	ROM_LOAD( "pap3-1.1b",     0xe000, 0x2000, CRC(3cac401c) SHA1(38a14228469fa4a20cbc5d862198dc901842682e) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "pap1-4.1k",     0xf000, 0x1000, CRC(330e20de) SHA1(5b23e5dcc38dc644a36efc8b03eba34cea540bea) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "pap1-6.3c",     0x0000, 0x1000, CRC(a36b96cb) SHA1(e0a11b5a43cbf756ddb045c743973d0a55dbb979) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "pap1-5.3f",     0x0000, 0x2000, CRC(fb6f56e3) SHA1(fd10d2ee49b4e059e9ef6046bc86d97e3185164d) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pap1-6.4c",     0x0000, 0x0020, CRC(52634b41) SHA1(dfb109c8e2c62ae1612ba0e3272468d152123842) )   // palette
	ROM_LOAD( "pap1-5.4e",     0x0020, 0x0100, CRC(ac46203c) SHA1(3f47f1991aab9640c0d5f70fad85d20d6cf2ea3d) )   // chars
	ROM_LOAD( "pap1-4.3l",     0x0120, 0x0100, CRC(686bde84) SHA1(541d08b43dbfb789c2867955635d2c9e051fedd9) )   // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "pap1-3.3m",     0x0000, 0x0100, CRC(94782db5) SHA1(ac0114f0611c81dfac9469253048ae0214d570ee) )
ROM_END

ROM_START( pacnpal2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pap1-3.1d",    0xa000, 0x2000, CRC(d7ec2719) SHA1(b633a5360a199d528bcef209c06a21f266525769) )
	ROM_LOAD( "pap1-2.1c",    0xc000, 0x2000, CRC(0245396e) SHA1(7e8467e317879621a7b31bc922b5187f20fcea78) )
	ROM_LOAD( "pap1-1.1b",    0xe000, 0x2000, CRC(7f046b58) SHA1(2024019e5fafb698bb5775075c9b88c5ed35f7ba) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "pap1-4.1k",     0xf000, 0x1000, CRC(330e20de) SHA1(5b23e5dcc38dc644a36efc8b03eba34cea540bea) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "pap1-6.3c",     0x0000, 0x1000, CRC(a36b96cb) SHA1(e0a11b5a43cbf756ddb045c743973d0a55dbb979) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "pap1-5.3f",     0x0000, 0x2000, CRC(fb6f56e3) SHA1(fd10d2ee49b4e059e9ef6046bc86d97e3185164d) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pap1-6.4c",     0x0000, 0x0020, CRC(52634b41) SHA1(dfb109c8e2c62ae1612ba0e3272468d152123842) )   // palette
	ROM_LOAD( "pap1-5.4e",     0x0020, 0x0100, CRC(ac46203c) SHA1(3f47f1991aab9640c0d5f70fad85d20d6cf2ea3d) )   // chars
	ROM_LOAD( "pap1-4.3l",     0x0120, 0x0100, CRC(686bde84) SHA1(541d08b43dbfb789c2867955635d2c9e051fedd9) )   // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "pap1-3.3m",     0x0000, 0x0100, CRC(94782db5) SHA1(ac0114f0611c81dfac9469253048ae0214d570ee) )
ROM_END

// should there be a pacnchmp set with pap2-x program roms?
ROM_START( pacnchmp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pap3-3.1d",      0xa000, 0x2000, CRC(20a07d3d) SHA1(2135ad154b575a73cfb1b0f0f282dfc013672aec) )
	ROM_LOAD( "pap3-2.1c",      0xc000, 0x2000, CRC(505bae56) SHA1(590ce9f0e92115a71eb76b71ab4eac16ffa2a28e) )
	ROM_LOAD( "pap3-1.1b",      0xe000, 0x2000, CRC(3cac401c) SHA1(38a14228469fa4a20cbc5d862198dc901842682e) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "pap1-4.1k",     0xf000, 0x1000, CRC(330e20de) SHA1(5b23e5dcc38dc644a36efc8b03eba34cea540bea) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "pap2-6.3c",      0x0000, 0x1000, CRC(93d15c30) SHA1(5da4120b680726c83a651b445254604cbf7cc883) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "pap2-5.3f",      0x0000, 0x2000, CRC(39f44aa4) SHA1(0696539cb2c7fcda2f6c295c7d65678dac18950b) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pap2-6.4c",     0x0000, 0x0020, CRC(18c3db79) SHA1(a37d3cbfc5d4bd740b02ae69a374292e937215e2)  )  // palette
	ROM_LOAD( "pap2-5.4e",     0x0020, 0x0100, CRC(875b49bb) SHA1(34b4622eecefd9fe0e9d883246d5c0e0c7f9ad43)  )  // chars
	ROM_LOAD( "pap2-4.3l",     0x0120, 0x0100, CRC(23701566) SHA1(afa22f5b9eb77679b5d5c2ed27d6590776a59f6f)  )  // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "pap1-3.3m",     0x0000, 0x0100, CRC(94782db5) SHA1(ac0114f0611c81dfac9469253048ae0214d570ee) )
ROM_END

ROM_START( grobda )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the first CPU
	ROM_LOAD( "gr2-3.1d",  0xa000, 0x2000, CRC(8e3a23be) SHA1(e54c1366adc561609a3817e074b01245fb335153) )
	ROM_LOAD( "gr2-2.1c",  0xc000, 0x2000, CRC(19ffa83d) SHA1(9f4faf5e0de783868d984f166b92ebcf8bb0f93f) )
	ROM_LOAD( "gr2-1.1b",  0xe000, 0x2000, CRC(0089b13a) SHA1(286d6a60fc46a6db9a52c19c4e33114717747caf) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "gr1-4.1k",  0xe000, 0x2000, CRC(3fe78c08) SHA1(dd49a96e613e0ced5b82eafcaf935e136e7db53a) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "gr1-7.3c",  0x0000, 0x1000, CRC(4ebfabfd) SHA1(fffce05f59e090c4281aca0c0494825027b764fb) )   // characters

	ROM_REGION( 0x4000, "sprites", 0 )
	ROM_LOAD( "gr1-5.3f",  0x0000, 0x2000, CRC(eed43487) SHA1(d2b39651f39bdfca3754f7bbd7a52e7bf843dabe) )   // sprites
	ROM_LOAD( "gr1-6.3e",  0x2000, 0x2000, CRC(cebb7362) SHA1(6efd57f9fa0f93f70e60efc387b3a782fad2665c) )   // sprites

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "gr1-6.4c",  0x0000, 0x0020, CRC(c65efa77) SHA1(ead74917744cb11556153bd6c09a987bc9c6ef08) )   // palette
	ROM_LOAD( "gr1-5.4e",  0x0020, 0x0100, CRC(a0f66911) SHA1(e08a56327055994e3d9e2c3816d57a3cc2434c88) )   // characters
	ROM_LOAD( "gr1-4.3l",  0x0120, 0x0100, CRC(f1f2c234) SHA1(d59879e7a598a363d8d9ac9e630ae698f14833f7) )   // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "gr1-3.3m",  0x0000, 0x0100, CRC(66eb1467) SHA1(02b99ced4afd9ac139f634739769f7bf353274f9) )
ROM_END

ROM_START( grobda2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the first CPU
	ROM_LOAD( "gr1-3.1d",  0xa000, 0x2000, CRC(4ef4a7c1) SHA1(33367e63531601c3d4f4a7b2170cb1c87f6d72a7) )
	ROM_LOAD( "gr2-2a.1c", 0xc000, 0x2000, CRC(f93e82ae) SHA1(cb591bbcaab5ef26f097e7bab9b3638990465d4c) )
	ROM_LOAD( "gr1-1.1b",  0xe000, 0x2000, CRC(32d42f22) SHA1(f83d17029f19fc2e8bac183771dbf9d786a56681) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "gr1-4.1k",  0xe000, 0x2000, CRC(3fe78c08) SHA1(dd49a96e613e0ced5b82eafcaf935e136e7db53a) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "gr1-7.3c",  0x0000, 0x1000, CRC(4ebfabfd) SHA1(fffce05f59e090c4281aca0c0494825027b764fb) )   // characters

	ROM_REGION( 0x4000, "sprites", 0 )
	ROM_LOAD( "gr1-5.3f",  0x0000, 0x2000, CRC(eed43487) SHA1(d2b39651f39bdfca3754f7bbd7a52e7bf843dabe) )   // sprites
	ROM_LOAD( "gr1-6.3e",  0x2000, 0x2000, CRC(cebb7362) SHA1(6efd57f9fa0f93f70e60efc387b3a782fad2665c) )   // sprites

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "gr1-6.4c",  0x0000, 0x0020, CRC(c65efa77) SHA1(ead74917744cb11556153bd6c09a987bc9c6ef08) )   // palette
	ROM_LOAD( "gr1-5.4e",  0x0020, 0x0100, CRC(a0f66911) SHA1(e08a56327055994e3d9e2c3816d57a3cc2434c88) )   // characters
	ROM_LOAD( "gr1-4.3l",  0x0120, 0x0100, CRC(f1f2c234) SHA1(d59879e7a598a363d8d9ac9e630ae698f14833f7) )   // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "gr1-3.3m",  0x0000, 0x0100, CRC(66eb1467) SHA1(02b99ced4afd9ac139f634739769f7bf353274f9) )
ROM_END

ROM_START( grobda3 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the first CPU
	ROM_LOAD( "gr1-3.1d",  0xa000, 0x2000, CRC(4ef4a7c1) SHA1(33367e63531601c3d4f4a7b2170cb1c87f6d72a7) )
	ROM_LOAD( "gr1-2.1c",  0xc000, 0x2000, CRC(7dcc6e8e) SHA1(7580686b7082432a79217c3d7b5ebfa0c25952e3) )
	ROM_LOAD( "gr1-1.1b",  0xe000, 0x2000, CRC(32d42f22) SHA1(f83d17029f19fc2e8bac183771dbf9d786a56681) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "gr1-4.1k",  0xe000, 0x2000, CRC(3fe78c08) SHA1(dd49a96e613e0ced5b82eafcaf935e136e7db53a) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "gr1-7.3c",  0x0000, 0x1000, CRC(4ebfabfd) SHA1(fffce05f59e090c4281aca0c0494825027b764fb) )   // characters

	ROM_REGION( 0x4000, "sprites", 0 )
	ROM_LOAD( "gr1-5.3f",  0x0000, 0x2000, CRC(eed43487) SHA1(d2b39651f39bdfca3754f7bbd7a52e7bf843dabe) )   // sprites
	ROM_LOAD( "gr1-6.3e",  0x2000, 0x2000, CRC(cebb7362) SHA1(6efd57f9fa0f93f70e60efc387b3a782fad2665c) )   // sprites

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "gr1-6.4c",  0x0000, 0x0020, CRC(c65efa77) SHA1(ead74917744cb11556153bd6c09a987bc9c6ef08) )   // palette
	ROM_LOAD( "gr1-5.4e",  0x0020, 0x0100, CRC(a0f66911) SHA1(e08a56327055994e3d9e2c3816d57a3cc2434c88) )   // characters
	ROM_LOAD( "gr1-4.3l",  0x0120, 0x0100, CRC(f1f2c234) SHA1(d59879e7a598a363d8d9ac9e630ae698f14833f7) )   // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "gr1-3.3m",  0x0000, 0x0100, CRC(66eb1467) SHA1(02b99ced4afd9ac139f634739769f7bf353274f9) )
ROM_END

ROM_START( phozon ) // 22139611 PCB
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the MAIN CPU
	ROM_LOAD( "6e.rom", 0x8000, 0x2000, CRC(a6686af1) SHA1(87a948b289356675d0418c87c3c0ae36ceba3ee0) )
	ROM_LOAD( "6h.rom", 0xa000, 0x2000, CRC(72a65ba0) SHA1(b1d5146c009469d4c6695f08ea2c6ad5d05b5b9b) )
	ROM_LOAD( "6c.rom", 0xc000, 0x2000, CRC(f1fda22e) SHA1(789881e94743efae01c63c1e3ce8d039cfa0324c) )
	ROM_LOAD( "6d.rom", 0xe000, 0x2000, CRC(f40e6df0) SHA1(48585ac1eff8fb7ed35f56c767d725cae88ff128) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the SOUND CPU
	ROM_LOAD( "3b.rom", 0xe000, 0x2000, CRC(5a4b3a79) SHA1(2774681ea668403de31ea218d5df3ce64e3b9243) )

	ROM_REGION( 0x10000, "sub2", 0 )    // 64k for the SUB CPU
	ROM_LOAD( "9r.rom", 0xe000, 0x2000, CRC(5d9f0a28) SHA1(2caef680229180b237f8c4becf052f1a96592efd) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "7j.rom", 0x0000, 0x1000, CRC(27f9db5b) SHA1(12ef817136b45927d7f279952fa19049a1349f60) )  // characters (set 1)
	ROM_LOAD( "8j.rom", 0x1000, 0x1000, CRC(15b12ef8) SHA1(e3303656b4e8b988e55a9551e5344e289958f677) )  // characters (set 2)

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "5t.rom", 0x0000, 0x2000, CRC(d50f08f8) SHA1(4e9dda0d5ad1c1b8b3be7edb05b3060f5f63a9c7) )  // sprites

	ROM_REGION( 0x0520, "proms", 0 )
	ROM_LOAD( "red.prm",     0x0000, 0x0100, CRC(a2880667) SHA1(b24d9b3354d20a7ecc02c428245669c6c86bfd61) ) // red palette ROM (4 bits)
	ROM_LOAD( "green.prm",   0x0100, 0x0100, CRC(d6e08bef) SHA1(b0ca7f8a77b7208cf974a8cc565fc91b7f40f51f) ) // green palette ROM (4 bits)
	ROM_LOAD( "blue.prm",    0x0200, 0x0100, CRC(b2d69c72) SHA1(e7b1ed698ab0e87872cb3a8f3ec102ca3a753259) ) // blue palette ROM (4 bits)
	ROM_LOAD( "chr.prm",     0x0300, 0x0100, CRC(429e8fee) SHA1(7b1899ca3f33f4561b572de1f24d9ea9d7d84b59) ) // characters
	ROM_LOAD( "sprite.prm",  0x0400, 0x0100, CRC(9061db07) SHA1(4305d37e613e1d15d37539b152c948648189c2cd) ) // sprites
	ROM_LOAD( "palette.prm", 0x0500, 0x0020, CRC(60e856ed) SHA1(dcc9a2dfc728b9ca1ab895008de07e20ebed9da3) ) // unused - timing?

	ROM_REGION( 0x0100, "namco", 0 )    // sound PROMs
	ROM_LOAD( "sound.prm", 0x0000, 0x0100, CRC(ad43688f) SHA1(072f427453efb1dda8147da61804fff06e1bc4d5) )

	ROM_REGION( 0x600, "plds", ROMREGION_ERASE00 ) // bruteforced, target device is GAL16V8
	ROM_LOAD( "ph1-7.7b", 0x000, 0x117, CRC(9a00af9a) SHA1(b5d5a3d8a091e0e3d62b2cce34ae9b8dc1cfcbff) )
	ROM_LOAD( "ph1-8.4b", 0x200, 0x117, CRC(fb180f00) SHA1(c0a355fb5287e715b0c39b78eacfdcd00db953b1) )
	ROM_LOAD( "ph1-9.7r", 0x400, 0x117, CRC(3b21d560) SHA1(905f9a6b7440b2eeb34475879751748300b42428) )
ROM_END

ROM_START( phozons )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the MAIN CPU
	ROM_LOAD( "6e.bin", 0x8000, 0x2000, CRC(ef822900) SHA1(3551a63bf7067dccee58690775cfec775bc2a472) )
	ROM_LOAD( "6h.bin", 0xa000, 0x2000, CRC(acb7869e) SHA1(947bd12e4db03caa46315cea2ef330dc29bfabde) )
	ROM_LOAD( "6c.bin", 0xc000, 0x2000, CRC(8ffa3e0e) SHA1(4962a60b55baf35d66b1039602def1f5bd15f9de) )
	ROM_LOAD( "6d.bin", 0xe000, 0x2000, CRC(8e6800b3) SHA1(27fa22d4cad255a54ba93b308cef301c2488e057) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the SOUND CPU
	ROM_LOAD( "3b.rom", 0xe000, 0x2000, CRC(5a4b3a79) SHA1(2774681ea668403de31ea218d5df3ce64e3b9243) )  // 3b.bin

	ROM_REGION( 0x10000, "sub2", 0 )    // 64k for the SUB CPU
	ROM_LOAD( "9r.rom", 0xe000, 0x2000, CRC(5d9f0a28) SHA1(2caef680229180b237f8c4becf052f1a96592efd) )  // 9r.bin

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "7j.bin", 0x0000, 0x1000, CRC(312b3ece) SHA1(1d4d3371a42321644ec3669f95abcfe860020868) )  // characters (set 1)
	ROM_LOAD( "8j.bin", 0x1000, 0x1000, CRC(d21422a2) SHA1(0268651628d66dc67a3b6bb8fb682b668e7ebbad) )  // characters (set 2)

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "5t.rom", 0x0000, 0x2000, CRC(d50f08f8) SHA1(4e9dda0d5ad1c1b8b3be7edb05b3060f5f63a9c7) )  // sprites - 5t.bin

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "ph1-2.1r", 0x0000, 0x0100, CRC(a2880667) SHA1(b24d9b3354d20a7ecc02c428245669c6c86bfd61) ) // red palette ROM (4 bits)
	ROM_LOAD( "ph1-1.1s", 0x0100, 0x0100, CRC(d6e08bef) SHA1(b0ca7f8a77b7208cf974a8cc565fc91b7f40f51f) ) // green palette ROM (4 bits)
	ROM_LOAD( "ph1-3.1p", 0x0200, 0x0100, CRC(b2d69c72) SHA1(e7b1ed698ab0e87872cb3a8f3ec102ca3a753259) ) // blue palette ROM (4 bits)
	ROM_LOAD( "ph1-5.5l", 0x0300, 0x0100, CRC(429e8fee) SHA1(7b1899ca3f33f4561b572de1f24d9ea9d7d84b59) ) // characters
	ROM_LOAD( "ph1-6.6r", 0x0400, 0x0100, CRC(9061db07) SHA1(4305d37e613e1d15d37539b152c948648189c2cd) ) // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound PROMs
	ROM_LOAD( "ph1-4.3e", 0x0000, 0x0100, CRC(ad43688f) SHA1(072f427453efb1dda8147da61804fff06e1bc4d5) )

	ROM_REGION( 0x0600, "plds", ROMREGION_ERASE00 ) // bruteforced, target device is GAL16V8
	ROM_LOAD( "ph1-7.7b", 0x0000, 0x0117, CRC(9a00af9a) SHA1(b5d5a3d8a091e0e3d62b2cce34ae9b8dc1cfcbff) )
	ROM_LOAD( "ph1-8.4b", 0x0200, 0x0117, CRC(fb180f00) SHA1(c0a355fb5287e715b0c39b78eacfdcd00db953b1) )
	ROM_LOAD( "ph1-9.7r", 0x0400, 0x0117, CRC(3b21d560) SHA1(905f9a6b7440b2eeb34475879751748300b42428) )
ROM_END

ROM_START( mappy )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the first CPU
	ROM_LOAD( "mpx_3.1d",   0xa000, 0x2000, CRC(52e6c708) SHA1(b9722941438e93325e84691ada4e95620bec73b2) )
	ROM_LOAD( "mp1_2.1c",   0xc000, 0x2000, CRC(a958a61c) SHA1(e5198703cdf47b2cd7fc9f2a5fde7bf4ab2275db) )
	ROM_LOAD( "mpx_1.1b",   0xe000, 0x2000, CRC(203766d4) SHA1(1dbc4f42d4c16a08240a221bec27dcc3a8dd7461) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "mp1_4.1k",   0xe000, 0x2000, CRC(8182dd5b) SHA1(f36b57f7f1e79f00b3f07afe1960bca5f5325ee2) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "mp1_5.3b",   0x0000, 0x1000, CRC(16498b9f) SHA1(76610149c65f955484fef1c033ddc3fed3f4e568) )

	ROM_REGION( 0x4000, "sprites", 0 )
	ROM_LOAD16_BYTE( "mp1_6.3m",   0x0000, 0x2000, CRC(f2d9647a) SHA1(3cc216793c6a5f73c437ad2524563deb3b5e2890) )
	ROM_LOAD16_BYTE( "mp1_7.3n",   0x0001, 0x2000, CRC(757cf2b6) SHA1(8dfbf03953d5219d9eb5fc654ec3392442ba1dc4) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "mp1-5.5b",   0x0000, 0x0020, CRC(56531268) SHA1(2e356706c07f43eeb67783fb122bdc7fed1b3589) )  // palette
	ROM_LOAD( "mp1-6.4c",   0x0020, 0x0100, CRC(50765082) SHA1(f578e14f15783acb2073644db4a2f0d196cc0957) )  // characters
	ROM_LOAD( "mp1-7.5k",   0x0120, 0x0100, CRC(5396bd78) SHA1(2e387e5d8b8cab005f67f821b4db65d0ae8bd362) )  // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "mp1-3.3m",   0x0000, 0x0100, CRC(16a9166a) SHA1(847cbaf7c88616576c410177e066ae1d792ac0ba) )
ROM_END

ROM_START( mappyj )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the first CPU
	ROM_LOAD( "mp1_3.1d",   0xa000, 0x2000, CRC(db9d5ab5) SHA1(32a0190f96f9c00c541b24dd17d6ad487938a8bf) )
	ROM_LOAD( "mp1_2.1c",   0xc000, 0x2000, CRC(a958a61c) SHA1(e5198703cdf47b2cd7fc9f2a5fde7bf4ab2275db) )
	ROM_LOAD( "mp1_1.1b",   0xe000, 0x2000, CRC(77c0b492) SHA1(631b73560ac59c3612e692fa59558773639ceda7) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "mp1_4.1k",   0xe000, 0x2000, CRC(8182dd5b) SHA1(f36b57f7f1e79f00b3f07afe1960bca5f5325ee2) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "mp1_5.3b",   0x0000, 0x1000, CRC(16498b9f) SHA1(76610149c65f955484fef1c033ddc3fed3f4e568) )

	ROM_REGION( 0x4000, "sprites", 0 )
	ROM_LOAD16_BYTE( "mp1_6.3m",   0x0000, 0x2000, CRC(f2d9647a) SHA1(3cc216793c6a5f73c437ad2524563deb3b5e2890) )
	ROM_LOAD16_BYTE( "mp1_7.3n",   0x0001, 0x2000, CRC(757cf2b6) SHA1(8dfbf03953d5219d9eb5fc654ec3392442ba1dc4) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "mp1-5.5b",   0x0000, 0x0020, CRC(56531268) SHA1(2e356706c07f43eeb67783fb122bdc7fed1b3589) )  // palette
	ROM_LOAD( "mp1-6.4c",   0x0020, 0x0100, CRC(50765082) SHA1(f578e14f15783acb2073644db4a2f0d196cc0957) )  // characters
	ROM_LOAD( "mp1-7.5k",   0x0120, 0x0100, CRC(5396bd78) SHA1(2e387e5d8b8cab005f67f821b4db65d0ae8bd362) )  // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "mp1-3.3m",   0x0000, 0x0100, CRC(16a9166a) SHA1(847cbaf7c88616576c410177e066ae1d792ac0ba) )
ROM_END

ROM_START( todruaga )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the first CPU
	ROM_LOAD( "td2_3.1d",   0x8000, 0x4000, CRC(fbf16299) SHA1(9abbaaaf0a53aff38df8287f62d091b13146cf13) )
	ROM_LOAD( "td2_1.1b",   0xc000, 0x4000, CRC(b238d723) SHA1(ab8eadd45638ff1ab2dacbd5ab2c6870b9f79086) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "td1_4.1k",   0xe000, 0x2000, CRC(ae9d06d9) SHA1(3d8621fdd74fafa61f342886faa37f0aab50c5a7) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "td1_5.3b",   0x0000, 0x1000, CRC(d32b249f) SHA1(7d7cee4101ef615fb92c3702f89a9823a6231195) )

	ROM_REGION( 0x4000, "sprites", 0 )
	ROM_LOAD16_BYTE( "td1_6.3m",   0x0000, 0x2000, CRC(e827e787) SHA1(74e0af4c7d6e334bcd211a33eb18dddc8a182aa7) )
	ROM_LOAD16_BYTE( "td1_7.3n",   0x0001, 0x2000, CRC(962bd060) SHA1(74cdcafc26475bda085bf62ed17e6474ed782453) )

	ROM_REGION( 0x0520, "proms", 0 )
	ROM_LOAD( "td1-5.5b",   0x0000, 0x0020, CRC(122cc395) SHA1(a648c53f2e95634bb5b27d79be3fd908021d056e) )  // palette
	ROM_LOAD( "td1-6.4c",   0x0020, 0x0100, CRC(8c661d6a) SHA1(1340e4f657f4f2c4ef651a441c3b51632e757d0b) )  // characters
	ROM_LOAD( "td1-7.5k",   0x0120, 0x0400, CRC(a86c74dd) SHA1(dfd7d6b2740761c3bcab4c7999d2699d920843e7) )  // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "td1-3.3m",   0x0000, 0x0100, CRC(07104c40) SHA1(16db55525034bacb71e7dc8bd2a7c3c4464d4808) )
ROM_END

ROM_START( todruagao )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the first CPU
	ROM_LOAD( "td1_3.1d",   0x8000, 0x4000, CRC(7ab4f5b2) SHA1(65035a5ecdff14bf23e01fe0f5e0935d156d94ff) )
	ROM_LOAD( "td1_1.1b",   0xc000, 0x4000, CRC(8c20ef10) SHA1(12ea4875ce4d4590b88862139d3379ab9f5cec03) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "td1_4.1k",   0xe000, 0x2000, CRC(ae9d06d9) SHA1(3d8621fdd74fafa61f342886faa37f0aab50c5a7) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "td1_5.3b",   0x0000, 0x1000, CRC(d32b249f) SHA1(7d7cee4101ef615fb92c3702f89a9823a6231195) )

	ROM_REGION( 0x4000, "sprites", 0 )
	ROM_LOAD16_BYTE( "td1_6.3m",   0x0000, 0x2000, CRC(e827e787) SHA1(74e0af4c7d6e334bcd211a33eb18dddc8a182aa7) )
	ROM_LOAD16_BYTE( "td1_7.3n",   0x0001, 0x2000, CRC(962bd060) SHA1(74cdcafc26475bda085bf62ed17e6474ed782453) )

	ROM_REGION( 0x0520, "proms", 0 )
	ROM_LOAD( "td1-5.5b",   0x0000, 0x0020, CRC(122cc395) SHA1(a648c53f2e95634bb5b27d79be3fd908021d056e) )  // palette
	ROM_LOAD( "td1-6.4c",   0x0020, 0x0100, CRC(8c661d6a) SHA1(1340e4f657f4f2c4ef651a441c3b51632e757d0b) )  // characters
	ROM_LOAD( "td1-7.5k",   0x0120, 0x0400, CRC(a86c74dd) SHA1(dfd7d6b2740761c3bcab4c7999d2699d920843e7) )  // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "td1-3.3m",   0x0000, 0x0100, CRC(07104c40) SHA1(16db55525034bacb71e7dc8bd2a7c3c4464d4808) )
ROM_END

ROM_START( todruagas )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the first CPU
	ROM_LOAD( "3b.bin", 0x8000, 0x4000, CRC(85d052d9) SHA1(03d4d4b5b0cd49d2a4acd8479bc9975a5fde6c4c) )
	ROM_LOAD( "1b.bin", 0xc000, 0x4000, CRC(a5db267a) SHA1(f140fd7b4d7e939f3a20ca5a15c16e5b8999c47d) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "td1_4.1k",   0xe000, 0x2000, CRC(ae9d06d9) SHA1(3d8621fdd74fafa61f342886faa37f0aab50c5a7) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "td1_5.3b",   0x0000, 0x1000, CRC(d32b249f) SHA1(7d7cee4101ef615fb92c3702f89a9823a6231195) )

	ROM_REGION( 0x4000, "sprites", 0 )
	ROM_LOAD16_BYTE( "td1_6.3m",   0x0000, 0x2000, CRC(e827e787) SHA1(74e0af4c7d6e334bcd211a33eb18dddc8a182aa7) )
	ROM_LOAD16_BYTE( "td1_7.3n",   0x0001, 0x2000, CRC(962bd060) SHA1(74cdcafc26475bda085bf62ed17e6474ed782453) )

	ROM_REGION( 0x0520, "proms", 0 )
	ROM_LOAD( "td1-5.5b",   0x0000, 0x0020, CRC(122cc395) SHA1(a648c53f2e95634bb5b27d79be3fd908021d056e) )  // palette
	ROM_LOAD( "td1-6.4c",   0x0020, 0x0100, CRC(8c661d6a) SHA1(1340e4f657f4f2c4ef651a441c3b51632e757d0b) )  // characters
	ROM_LOAD( "td1-7.5k",   0x0120, 0x0400, CRC(a86c74dd) SHA1(dfd7d6b2740761c3bcab4c7999d2699d920843e7) )  // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "td1-3.3m",   0x0000, 0x0100, CRC(07104c40) SHA1(16db55525034bacb71e7dc8bd2a7c3c4464d4808) )
ROM_END

ROM_START( digdug2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the first CPU
	ROM_LOAD( "d23_3.1d",   0x8000, 0x4000, CRC(cc155338) SHA1(d6796479ebb00081e9ae281380a4ce75f730766e) )
	ROM_LOAD( "d23_1.1b",   0xc000, 0x4000, CRC(40e46af8) SHA1(698a5c425e23627331d85216a4edee9c391e5749) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "d21_4.1k",   0xe000, 0x2000, CRC(737443b1) SHA1(0e46204089cc6e5ffab0d2a62f9a1728f8c35948) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "d21_5.3b",   0x0000, 0x1000, CRC(afcb4509) SHA1(c9a54df22b0b92efbe7417a00200587225906b46) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD16_BYTE( "d21_6.3m",   0x0000, 0x4000, CRC(df1f4ad8) SHA1(004fba630018dbf03c4b0e284c98077e19fface3) )
	ROM_LOAD16_BYTE( "d21_7.3n",   0x0001, 0x4000, CRC(ccadb3ea) SHA1(77d8d8e6039272f73e63c8f76084138ec613365a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "d21-5.5b",   0x0000, 0x0020, CRC(9b169db5) SHA1(77e840d10ab59708a051c3b15305b33d431ee06d) )  // palette
	ROM_LOAD( "d21-6.4c",   0x0020, 0x0100, CRC(55a88695) SHA1(bd6bd641c9f220b6a2cc414a1117d5c089571400) )  // characters
	ROM_LOAD( "d21-7.5k",   0x0120, 0x0100, CRC(9c55feda) SHA1(30a4593726f5a4791e0812fd593e592087e730e3) )  // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "d21-3.3m",   0x0000, 0x0100, CRC(e0074ee2) SHA1(f4f02977130110be52f4dd82fc3c0d02f45778b9) )
ROM_END

ROM_START( digdug2o )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the first CPU
	ROM_LOAD( "d21_3.1d",   0x8000, 0x4000, CRC(be7ec80b) SHA1(a053274ffbf3200e9b89a8be1bd91744acb4a823) )
	ROM_LOAD( "d21_1.1b",   0xc000, 0x4000, CRC(5c77c0d4) SHA1(56709e5db1686fd996d21c1005accf34e2d863e1) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "d21_4.1k",   0xe000, 0x2000, CRC(737443b1) SHA1(0e46204089cc6e5ffab0d2a62f9a1728f8c35948) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "d21_5.3b",   0x0000, 0x1000, CRC(afcb4509) SHA1(c9a54df22b0b92efbe7417a00200587225906b46) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD16_BYTE( "d21_6.3m",   0x0000, 0x4000, CRC(df1f4ad8) SHA1(004fba630018dbf03c4b0e284c98077e19fface3) )
	ROM_LOAD16_BYTE( "d21_7.3n",   0x0001, 0x4000, CRC(ccadb3ea) SHA1(77d8d8e6039272f73e63c8f76084138ec613365a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "d21-5.5b",   0x0000, 0x0020, CRC(9b169db5) SHA1(77e840d10ab59708a051c3b15305b33d431ee06d) )  // palette
	ROM_LOAD( "d21-6.4c",   0x0020, 0x0100, CRC(55a88695) SHA1(bd6bd641c9f220b6a2cc414a1117d5c089571400) )  // characters
	ROM_LOAD( "d2x-7.5k",   0x0120, 0x0100, CRC(1525a4d1) SHA1(bbedb0cf5957671fca1229d38cb33086356813e1) )  // sprites
	// the sprite lookup table is different from the other set, could be a bad dump

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "d21-3.3m",   0x0000, 0x0100, CRC(e0074ee2) SHA1(f4f02977130110be52f4dd82fc3c0d02f45778b9) )
ROM_END

ROM_START( motos )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 64k for code for the first CPU
	ROM_LOAD( "mo1_3.1d",   0x8000, 0x4000, CRC(1104abb2) SHA1(ade809a73ac24494b9f95f65b7592df5f86dce60) )
	ROM_LOAD( "mo1_1.1b",   0xc000, 0x4000, CRC(57b157e2) SHA1(b050495fcc4a4d93551b29d4f05e49f64017c870) )

	ROM_REGION( 0x10000, "sub", 0 ) // 64k for the second CPU
	ROM_LOAD( "mo1_4.1k",   0xe000, 0x2000, CRC(55e45d21) SHA1(a8b195acfec542734751de29c9dafc2b165a5881) )

	ROM_REGION( 0x1000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "mo1_5.3b",   0x0000, 0x1000, CRC(5d4a2a22) SHA1(4af9bf2ae9bb78d2d029ef38809181ffa3c0eb66) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD16_BYTE( "mo1_6.3m",   0x0000, 0x4000, CRC(2f0e396e) SHA1(664679f9d3d74a3fccb086af910392b4fe40c9bc) )
	ROM_LOAD16_BYTE( "mo1_7.3n",   0x0001, 0x4000, CRC(cf8a3b86) SHA1(2b49cdec516e23783f2a291633d81ab8bd0245fc) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "mo1-5.5b",   0x0000, 0x0020, CRC(71972383) SHA1(66b0619affcc5168b099108800a941d6e2416ab0) )  // palette
	ROM_LOAD( "mo1-6.4c",   0x0020, 0x0100, CRC(730ba7fb) SHA1(24eb167266752b064689662e3ef0f62d0407ac26) )  // characters
	ROM_LOAD( "mo1-7.5k",   0x0120, 0x0100, CRC(7721275d) SHA1(543adb5348db81ea82a5c039451001ebd82735e3) )  // sprites

	ROM_REGION( 0x0100, "namco", 0 )    // sound prom
	ROM_LOAD( "mo1-3.3m",   0x0000, 0x0100, CRC(2accdfb4) SHA1(e21a0618c0f8e35ce26666b6850ac9c0d95d7971) )
ROM_END


void mappy_state::init_digdug2()
{
	// appears to not use the watchdog
	m_maincpu->space(AS_PROGRAM).nop_write(0x8000, 0x8000);
}


// 2x6809, static tilemap, 2bpp sprites (Super Pacman type)
GAME( 1982, superpac,  0,        superpac, superpac,  mappy_state,  empty_init,   ROT90, "Namco", "Super Pac-Man", MACHINE_SUPPORTS_SAVE )
GAME( 1982, superpacm, superpac, superpac, superpac,  mappy_state,  empty_init,   ROT90, "Namco (Bally Midway license)", "Super Pac-Man (Midway)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, pacnpal,   0,        pacnpal,  pacnpal,   mappy_state,  empty_init,   ROT90, "Namco", "Pac & Pal", MACHINE_SUPPORTS_SAVE )
GAME( 1983, pacnpal2,  pacnpal,  pacnpal,  pacnpal,   mappy_state,  empty_init,   ROT90, "Namco", "Pac & Pal (older)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, pacnchmp,  pacnpal,  pacnpal,  pacnpal,   mappy_state,  empty_init,   ROT90, "Namco", "Pac-Man & Chomp Chomp", MACHINE_SUPPORTS_SAVE )
GAME( 1984, grobda,    0,        grobda,   grobda,    mappy_state,  empty_init,   ROT90, "Namco", "Grobda (New Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, grobda2,   grobda,   grobda,   grobda,    mappy_state,  empty_init,   ROT90, "Namco", "Grobda (Old Ver. set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, grobda3,   grobda,   grobda,   grobda,    mappy_state,  empty_init,   ROT90, "Namco", "Grobda (Old Ver. set 2)", MACHINE_SUPPORTS_SAVE )

// 3x6809, static tilemap, 2bpp sprites (Gaplus type)
GAME( 1983, phozon,    0,        phozon,    phozon,   phozon_state, empty_init,   ROT90, "Namco", "Phozon (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, phozons,   phozon,   phozon,    phozon,   phozon_state, empty_init,   ROT90, "Namco (Sidam license)", "Phozon (Sidam)", MACHINE_SUPPORTS_SAVE )

// 2x6809, scroling tilemap, 4bpp sprites (Super Pacman type)
GAME( 1983, mappy,     0,        mappy,     mappy,    mappy_state,  empty_init,   ROT90, "Namco", "Mappy (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, mappyj,    mappy,    mappy,     mappy,    mappy_state,  empty_init,   ROT90, "Namco", "Mappy (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, todruaga,  0,        todruaga,  todruaga, mappy_state,  empty_init,   ROT90, "Namco", "The Tower of Druaga (New Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, todruagao, todruaga, todruaga,  todruaga, mappy_state,  empty_init,   ROT90, "Namco", "The Tower of Druaga (Old Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, todruagas, todruaga, todruaga,  todruaga, mappy_state,  empty_init,   ROT90, "bootleg? (Sidam)", "The Tower of Druaga (Sidam)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, digdug2,   0,        digdug2,   digdug2,  mappy_state,  init_digdug2, ROT90, "Namco", "Dig Dug II (New Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, digdug2o,  digdug2,  digdug2,   digdug2,  mappy_state,  init_digdug2, ROT90, "Namco", "Dig Dug II (Old Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, motos,     0,        motos,     motos,    mappy_state,  empty_init,   ROT90, "Namco", "Motos", MACHINE_SUPPORTS_SAVE )
