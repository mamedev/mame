// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Bosconian  (c) 1981 Namco
Galaga     (c) 1981 Namco
Xevious    (c) 1982 Namco
Dig Dug    (c) 1982 Namco

driver by Nicola Salmoria
based on previous work by Martin Scragg, Mirko Buffoni, Aaron Giles


All these games are based on the same 3xZ80, shared memory, CPU design.
Bosconian and Galaga use the same CPU board, with minor differences
(Galaga has one missing RAM and no 50XX custom)
Xevious is physically different, but logically identical.
Dig Dug is the only one a bit different, because it reads the DIP switches
through a custom chip instead of having them mapped in memory.

The video board, on the other hand, is completely different for all the games,
that's why they use separate video/ source files.


Custom ICs:
----------
Bosconian:
---------
CPU board:
06XX     interface to custom 5xXX
07XX     clock divider
08XX(x3) bus controller
50XX     player score control (protection)
51XX     I/O
54XX     explosion sound generator

Video board:
03XX(x2) ?
05XX     starfield generator
06XX     interface to custom 5xXX
07XX     clock divider
50XX     player score control (only used as protection check)
52XX     sample player

Galaga:
------
CPU board:
06XX     interface to custom 5xXX
07XX     clock divider
08XX(x3) bus controller
51XX     I/O
54XX     explosion sound generator

Video board:
00XX     tilemap address generator with scrolling capability (only Super Pacman)
02XX     gfx data shifter and mixer (16-bit in, 4-bit out)
04XX     sprite address generator
05XX     starfield generator
07XX     clock divider


Memory maps:
-----------
Bosconian:
---------
MAIN CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3N    program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 3M    program ROM
0010xxxxxxxxxxxx R   xxxxxxxx ROM 3L    program ROM
0011xxxxxxxxxxxx R   xxxxxxxx ROM 3K    program ROM
the rest of the memory map is common to the other CPUs

SUB CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3J    program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 3H    program ROM
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3E    program ROM
0001------------              n.c.
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

COMMON:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
01000-----------              n.c.
01001-----------              n.c.
01010-----------              n.c.
01011-----------              n.c.
01100-----------              n.c.
01101-----00xxxx   W ----xxxx RAM 2A    \ sound control registers
01101-----01xxxx   W ----xxxx RAM 2B    /
01101-----10-000   W -------x IRQ1      main CPU irq enable/acknowledge
01101-----10-001   W -------x IRQ2      motion CPU irq enable/acknowledge
01101-----10-010   W -------x NMION     sound CPU nmi enable
01101-----10-011   W -------x RESET     reset sub and sound CPU, and 5xXX chips on CPU board
01101-----10-100   W -------x n.c.
01101-----10-101   W -------x MOD 0     unused?
01101-----10-110   W -------x MOD 1     unused?
01101-----10-111   W -------x MOD 2     unused?
01101-----11----   W -------- WDR       watchdog reset
01101-----00-xxx R   -------x DIP SW    dip switch B
01101-----00-xxx R   ------x- DIP SW    dip switch A
01101-----01---- R            n.c.
01101-----10---- R            n.c.
01101-----11---- R            n.c.
01110--0-------- R/W xxxxxxxx I/O       custom 06XX data
01110--1-------- R/W xxxxxxxx I/O       custom 06XX control
01111xxxxxxxxxxx R/W xxxxxxxx RAM 2N    work RAM (not present in Galaga)
10000xxxxxxxxxxx R/W xxxxxxxx DHRAM     tilemap RAM (tile code) [1]
10001xxxxxxxxxxx R/W xxxxxxxx VCRAM     tilemap RAM (tile attr) [1]
10010--0-------- R/W xxxxxxxx EXCS      custom 06XX #2 data
10010--1-------- R/W xxxxxxxx EXCS      custom 06XX #2 control
10011----000xxxx   W ----xxxx SOWR      bullets shape and X pos msb [2]
10011----001----   W xxxxxxxx POSI X    playfield X scroll
10011----010----   W xxxxxxxx POSI Y    playfield Y scroll
10011----011----   W -----xxx STAR      to 05XX: starfield X scroll speed
10011----011----   W --xxx--- STAR      to 05XX: starfield Y scroll speed
10011----100----   W -------- STARCLR   to 05XX: unknown
10011----101----   W          n.c.
10011----110----   W          n.c.
10011----111-000   W -------x FLIP      flip screen
10011----111-001   W -------x n.c.
10011----111-010   W -------x n.c.
10011----111-011   W -------x n.c.
10011----111-100   W -------x BLK 0     \ to 05XX: starfield blink
10011----111-101   W -------x BLK 1     /          (select active subset)
10011----111-110   W -------x n.c.
10011----111-111   W -------x RESET     reset 5xXX chips on video board
10100-----------              n.c.
10101-----------              n.c.
10110-----------              n.c.
10111-----------              n.c.

[1] 1st half is radar + sprite registers, 2nd half is scrolling playfield
[2] SO = Small Objects? Only locations 4-F are used.


Galaga:
------
MAIN CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3N    program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 3M    program ROM
0010xxxxxxxxxxxx R   xxxxxxxx ROM 3L    program ROM
0011xxxxxxxxxxxx R   xxxxxxxx ROM 3K    program ROM
the rest of the memory map is common to the other CPUs

SUB CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3J    program ROM
0001------------              n.c.
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3E    program ROM
0001------------              n.c.
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

COMMON:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
01000-----------              n.c.
01001-----------              n.c.
01010-----------              n.c.
01011-----------              n.c.
01100-----------              n.c.
01101-----00----   W ----xxxx RAM 2A    \ sound control registers
01101-----01----   W ----xxxx RAM 2B    /
01101-----10-000   W -------x IRQ1      main CPU irq enable/acknowledge
01101-----10-001   W -------x IRQ2      motion CPU irq enable/acknowledge
01101-----10-010   W -------x NMION     sound CPU nmi enable
01101-----10-011   W -------x RESET     reset sub and sound CPU, and 5xXX chips on CPU board
01101-----10-100   W -------x n.c.
01101-----10-101   W -------x MOD 0     unused?
01101-----10-110   W -------x MOD 1     unused?
01101-----10-111   W -------x MOD 2     unused?
01101-----11----   W -------- WDR       watchdog reset
01101-----00-xxx R   -------x DIP SW    dip switch B
01101-----00-xxx R   ------x- DIP SW    dip switch A
01101-----01---- R            n.c.
01101-----10---- R            n.c.
01101-----11---- R            n.c.
01110--0-------- R/W xxxxxxxx I/O       custom 06XX data
01110--1-------- R/W xxxxxxxx I/O       custom 06XX control
10000xxxxxxxxxxx R/W xxxxxxxx RAM 1K    tilemap RAM
10001-xxxxxxxxxx R/W xxxxxxxx RAM 3E/3F work RAM
10001-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers
10010-xxxxxxxxxx R/W xxxxxxxx RAM 3K/3L work RAM
10010-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers
10011-xxxxxxxxxx R/W xxxxxxxx RAM 3H/3J work RAM
10011-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers
10100--------000   W -------x           \
10100--------001   W -------x            > to 05XX: starfield X scroll speed
10100--------010   W -------x           /
10100--------011   W -------x           \ to 05XX: starfield blink
10100--------100   W -------x           /          (select active subset)
10100--------101   W -------x           to 05XX: unknown. It is the same as STARCLR in Bosconian
10100--------110   W -------x n.c.
10100--------111   W -------x FLIP      flip screen
10101-----------              n.c.
10110-----------              n.c.
10111-----------              n.c.


Namco vs Midway ROM names and locations
---------------------------------------
Location  ID         Location  ID
--------  ----       --------  -----
CPU 3P    GG1-1      CPU 3N    3200A
CPU 3M    GG1-2      CPU 3M    3300B
CPU 2M    GG1-3      CPU 3L    3400C
CPU 2L    GG1-4      CPU 3K    3500D
CPU 3F    GG1-5      CPU 3J    3600E
CPU 2C    GG1-7      CPU 3E    3700G
CPU 1D    GG1-1[bpr] CPU 1D
CPU 5C    GG1-2[bpr] CPU 5C

VID 4L    GG1-9      VID 4L    2600J
VID 4F    GG1-10     VID 4F    2700K
VID 4D    GG1-11     VID 4D    2800L
VID 1C    GG1-3[bpr] VID 1C
VID 2N    GG1-4[bpr] VID 2N
VID 5N    GG1-5[bpr] VID 5N



Gatsbee (Galaga mod/bootleg)
----------------------------
This game runs on modified bootleg Galaga hardware (blue board with PCB numbers DG-09-02 and DG-07-02)

ROM8: is a 2764. pins 1, 26, 27, 28 tied together.
      pin2 out of socket, has wire that is tied to pin 4 of a LS259 that sits on top of the main Z80
      CPU located at 5B/6B

Z80: There are 2 logic chips sitting on top of it which are wired up to the Z80 and to each other.
     Looks like this....
     |-------------------|
     |  LS32   LS259     <
     |-------------------|

Bend all the legs outwards.
Line up the LS259 so pin 16 is in line with Z80 pin 11
Line up the LS32 so pin 7 is in line with Z80 pin 29
Atach the 2 chips to the top of the Z80 with some glue
Connect like this....

LS32 pin 1 tied to Z80 pin 22 (WR)
LS32 pin 2 tied to Z80 pin 19 (MREQ)
LS32 pin 3,4 tied together
LS32 pin 5 tied to Z80 pin 4 (A14)
LS32 pin 6 tied to pin 10 LS32
LS32 pin 7 tied to Z80 pin 29 (GND)
LS32 pin 8 tied to LS259 pin 14
LS32 pin 9 tied to Z80 pin 5 (A15)
LS32 pins 11, 12, 13 have NC
LS32 pin 14 tied to Z80 pin 11 (+5V)

LS259 pin 1 tied to Z80 pin 30 (A0)
LS259 pin 2 tied to Z80 pin 31 (A1)
LS259 pin 3 tied to Z80 pin 32 (A2)
LS259 pin 4 to ROM 8 (as above)
LS259 pins 5, 6, 7 have NC
LS259 pin 8 tied to Z80 pin 29 (GND)
LS259 pins 9, 10, 11, 12 have NC
LS259 pin 13 tied to Z80 pin 14 (D0)
LS259 pin 15 tied to Z80 pin 26 (RESET)
LS259 pin 16 tied to Z80 pin 11 (+5V)



Easter eggs:
-----------
- Bosconian:
  - enter service mode
  - keep B1 pressed and enter the following sequence:
    5xU 6xR 1xD 4xL
  (c) 1981 NAMCO LTD. will be added at the bottom of the screen.

- Galaga:
  - enter service mode
  - keep B1 pressed and enter the following sequence:
    5xR 6xL 3xR 7xL
  (c) 1981 NAMCO LTD. will appear on the screen.


Notes:
-----
- The Cabinet Type "DIP switch" actually comes from the edge connector, but is mapped
  in memory in place of dip switch #8. DIP switch #8 selects single/dual coin counters
  and is entirely handled by hardware.

- galaga: there is a bug in the sound CPU program. During initialization, it enables
  NMI before clearing RAM, but the NMI handler doesn't save the registers, so it cannot
  interrupt program execution. If the NMI happens before the LDIR that clears RAM has
  finished, the program will crash.

- galaga: there were "fast shoot" hacks available, which are not supported.
  Their effects can be replicated with this line in cheat.dat:
  galaga:1:070D:0D:100:Fast Shoot

- bosco: there appears to be a bug in the code at 0BB1, which handles communication
  with the 06XX custom chip. First it saves in A' the command to write, then if a
  transfer is still in progress it jumps to 0BC1, does other things, then restores
  the command from A' and stores it in RAM. At that point (0BE1) it checks again if
  a transfer is in progress. If the transfer has terminated, it jumps to 0BEB, which
  restores the command from RAM, and jumps back to 0BBA to send the command. However,
  the instruction at 0BBA is ex af,af', so the command is overwritten with garbage.
  There's also an exx at 0BBB which seems unnecessary but that's harmless.
  Anyway, what this bug means is that we must make sure that the 06XX generates NMIs
  quickly enough to ensure that 0BB1 is usually not called with a transfer still is
  progress. It doesn't seem possible to prevent it altogether though, so we can only
  hope that the transfer doesn't terminate in the middle of the function.

- bosco: we have two dumps of the sound shape ROM, "prom.1d" and "bosco.spr". Music
  changes a lot from one version to the other.
  I'm using the former because it is more similar to the other Namco games. The latter,
  after masking off the unused top 4 bits and inverting bit 3, matches the Galaga one,
  so it might have come from a (bootleg?) conversion.

- bosco & galaga: the Midway arcade cabinet had an optional rapid fire board, using
  a 556 to generate autofire while the button was held. That really makes little
  sense in Galaga! For Bosconian, I guess it was for the boscomdo set I, because the
  other sets have autofire built-in.

- the bosconian video system is (apart from the starfield) almost identical functionally
  to Rally-X, but the hardware is quite different: Rally-X has no custom ICs.

- gallag is identical to galagao, apart from the title changed to "GALLAG" and the
  copyright notice changed from "(c) 1981 NAMCO LTD" to "1 9 8 2" (and the Namco logo
  removed from the gfx). The only interesting thing about it is the 4th Z80, used to
  simulate the custom 5xXX chips of the original.
  It also has different explosion and starfield circuitries, to do without the Namco
  custom chips.


TODO:
----
- bosco: is the screen horizontal resolution maybe 285? PCB videos do show a slightly
  larger right border though

- gallag/gatsbee: explosions are not emulated since the bootleg board doesn't have
  the 54XX custom. Should probably use samples like Battles?

***************************************************************************/

#include "emu.h"
#include "bosco.h"
#include "galaga.h"

#include "namco06.h"
#include "namco50.h"
#include "namco51.h"
#include "namco52.h"
#include "namco53.h"
#include "namco54.h"

#include "cpu/mb88xx/mb88xx.h"
#include "cpu/z80/z80.h"
#include "machine/rescap.h"
#include "machine/watchdog.h"
#include "sound/samples.h"

#include "speaker.h"


#define MASTER_CLOCK (XTAL(18'432'000))


#define STARFIELD_X_OFFSET_GALAGA       16
#define STARFIELD_X_LIMIT_GALAGA    256 + STARFIELD_X_OFFSET_GALAGA

#define STARFIELD_Y_OFFSET_BOSCO        16
#define STARFIELD_X_LIMIT_BOSCO     224


uint8_t galaga_state::bosco_dsw_r(offs_t offset)
{
	int bit0,bit1;

	bit0 = (ioport("DSWB")->read() >> offset) & 1;
	bit1 = (ioport("DSWA")->read() >> offset) & 1;

	return bit0 | (bit1 << 1);
}

void galaga_state::irq1_clear_w(int state)
{
	m_main_irq_mask = state;
	if (!m_main_irq_mask)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void galaga_state::irq2_clear_w(int state)
{
	m_sub_irq_mask = state;
	if (!m_sub_irq_mask)
		m_subcpu->set_input_line(0, CLEAR_LINE);
}

void galaga_state::nmion_w(int state)
{
	m_sub2_nmi_mask = !state;
}

void galaga_state::out(uint8_t data)
{
	m_leds[1] = BIT(data, 0);
	m_leds[0] = BIT(data, 1);
	machine().bookkeeping().coin_counter_w(1,~data & 4);
	machine().bookkeeping().coin_counter_w(0,~data & 8);
}

void galaga_state::lockout(int state)
{
	machine().bookkeeping().coin_lockout_global_w(state);
}

uint8_t galaga_state::namco_52xx_rom_r(offs_t offset)
{
	uint32_t length = memregion("52xx")->bytes();
//printf("ROM read %04X\n", offset);
	if (!(offset & 0x1000))
		offset = (offset & 0xfff) | 0x0000;
	else if (!(offset & 0x2000))
		offset = (offset & 0xfff) | 0x1000;
	else if (!(offset & 0x4000))
		offset = (offset & 0xfff) | 0x2000;
	else if (!(offset & 0x8000))
		offset = (offset & 0xfff) | 0x3000;
	return (offset < length) ? memregion("52xx")->base()[offset] : 0xff;
}

uint8_t galaga_state::namco_52xx_si_r()
{
	/* pulled to GND */
	return 0;
}

TIMER_CALLBACK_MEMBER(galaga_state::cpu3_interrupt_callback)
{
	int scanline = param;

	if(m_sub2_nmi_mask)
		m_subcpu2->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

	scanline = scanline + 128;
	if (scanline >= 272)
		scanline = 64;

	/* the vertical synch chain is clocked by H256 -- this is probably not important, but oh well */
	m_cpu3_interrupt_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


void galaga_state::machine_start()
{
	/* create the interrupt timer */
	m_cpu3_interrupt_timer = timer_alloc(FUNC(galaga_state::cpu3_interrupt_callback), this);
	save_item(NAME(m_main_irq_mask));
	save_item(NAME(m_sub_irq_mask));
	save_item(NAME(m_sub2_nmi_mask));
}

void galaga_state::machine_reset()
{
	m_cpu3_interrupt_timer->adjust(m_screen->time_until_pos(64), 64);
}


/* the same memory map is used by all three CPUs; all RAM areas are shared */
void bosco_state::bosco_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().nopw();         /* the only area different for each CPU */
	map(0x6800, 0x6807).r(FUNC(bosco_state::bosco_dsw_r));
	map(0x6800, 0x681f).w(m_namco_sound, FUNC(namco_wsg_device::pacman_sound_w));
	map(0x6820, 0x6827).w("misclatch", FUNC(ls259_device::write_d0));
	map(0x6830, 0x6830).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x7000, 0x70ff).rw("06xx_0", FUNC(namco_06xx_device::data_r), FUNC(namco_06xx_device::data_w));
	map(0x7100, 0x7100).rw("06xx_0", FUNC(namco_06xx_device::ctrl_r), FUNC(namco_06xx_device::ctrl_w));
	map(0x7800, 0x7fff).ram().share("share1");
	map(0x8000, 0x8fff).ram().w(FUNC(bosco_state::bosco_videoram_w)).share("videoram");/* + sprite registers */
	map(0x9000, 0x90ff).rw("06xx_1", FUNC(namco_06xx_device::data_r), FUNC(namco_06xx_device::data_w));
	map(0x9100, 0x9100).rw("06xx_1", FUNC(namco_06xx_device::ctrl_r), FUNC(namco_06xx_device::ctrl_w));
	map(0x9800, 0x980f).writeonly().share("bosco_radarattr");
	map(0x9810, 0x9810).w(FUNC(bosco_state::bosco_scrollx_w));
	map(0x9820, 0x9820).w(FUNC(bosco_state::bosco_scrolly_w));
	map(0x9830, 0x9830).writeonly().share("starcontrol");
	map(0x9840, 0x9840).w(FUNC(bosco_state::bosco_starclr_w));
	map(0x9870, 0x9877).w(m_videolatch, FUNC(ls259_device::write_d0));
}


void galaga_state::galaga_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().nopw();         /* the only area different for each CPU */
	map(0x6800, 0x6807).r(FUNC(galaga_state::bosco_dsw_r));
	map(0x6800, 0x681f).w(m_namco_sound, FUNC(namco_wsg_device::pacman_sound_w));
	map(0x6820, 0x6827).w("misclatch", FUNC(ls259_device::write_d0));
	map(0x6830, 0x6830).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x7000, 0x70ff).rw("06xx", FUNC(namco_06xx_device::data_r), FUNC(namco_06xx_device::data_w));
	map(0x7100, 0x7100).rw("06xx", FUNC(namco_06xx_device::ctrl_r), FUNC(namco_06xx_device::ctrl_w));
	map(0x8000, 0x87ff).ram().w(FUNC(galaga_state::galaga_videoram_w)).share("videoram");
	map(0x8800, 0x8bff).ram().share("galaga_ram1");
	map(0x9000, 0x93ff).ram().share("galaga_ram2");
	map(0x9800, 0x9bff).ram().share("galaga_ram3");
	map(0xa000, 0xa007).w(m_videolatch, FUNC(ls259_device::write_d0));
}

void galaga_state::gatsbee_main_map(address_map &map)
{
	galaga_map(map);
	map(0x0000, 0x0007).mirror(0x3ff8).w("extralatch", FUNC(ls259_device::write_d0));
}



/* bootleg 4th CPU replacing the 5xXX chips */
void galaga_state::galaga_mem4(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x107f).ram();
}


static INPUT_PORTS_START( bosco )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) ) // factory default = "Yes"
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Freeze" )                    PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	/* bonus scores are different for 5 lives */
	PORT_DIPNAME( 0x38, 0x20, "Bonus Fighter" )         PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x30, "15K and 50K Only" )      PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0) /* Began with 1, 2 or 3 fighters */
	PORT_DIPSETTING(    0x38, "20K and 70K Only" )      PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "10K, 50K, Every 50K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "15K, 50K, Every 50K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x18, "15K, 70K, Every 70K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x20, "20K, 70K, Every 70K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0) // factory default = "20K, 70K, Every70K"
	PORT_DIPSETTING(    0x28, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x30, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0) /* Began with 5 fighters */
	PORT_DIPSETTING(    0x38, "30K, 120K, Every 120K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "15K and 70K Only" )      PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "20K and 70K Only" )      PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x18, "20K and 100K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x20, "30K and 120K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "30K, 80K, Every 80K" )   PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" ) // factory default = "3"
	PORT_DIPSETTING(    0xc0, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( boscomd )
	PORT_INCLUDE( bosco )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits Game" )            PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x01, "2 Players" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )                    PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( galaga )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:3" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Freeze" )                PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Rack Test" )             PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x20, "20K, 60K, Every 60K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0) /* Began with 2, 3 or 4 fighters */
	PORT_DIPSETTING(    0x18, "20K and 60K Only" )      PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "20K, 70K, Every 70K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0) // factory default = "20K, 70K, Every70K"
	PORT_DIPSETTING(    0x30, "20K, 80K, Every 80K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x38, "30K and 80K Only" )      PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "30K, 120K, Every 120K" ) PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x20, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0) /* Began with 5 fighters */
	PORT_DIPSETTING(    0x18, "30K and 150K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "30K, 120K, Every 120K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x30, "30K, 150K, Every 150K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x38, "30K Only" )              PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "30K and 100K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "30K and 120K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x80, "3" ) // factory default = "3"
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0xc0, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( galagamw )
	PORT_INCLUDE( galaga )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits Game" )        PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x01, "2 Players" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hardest ) )
INPUT_PORTS_END

/* the same as galaga but with vertical movement */
static INPUT_PORTS_START( gatsbee )
	PORT_INCLUDE( galaga )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL
INPUT_PORTS_END



static const gfx_layout charlayout_2bpp =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(0*8,1) },
	{ STEP8(0*8,8) },
	16*8
};

static const gfx_layout spritelayout_bosco =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1), STEP4(0*8,1) },
	{ STEP8(0*8,8), STEP8(32*8,8) },
	64*8
};

const gfx_layout spritelayout_galaga =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(0*8,1), STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1) },
	{ STEP8(0*8,8), STEP8(32*8,8) },
	64*8
};

static const gfx_layout dotlayout =
{
	4,4,
	8,
	3,  /* 2 bits color + 1 bit transparency */
	{ 5, 6, 7 },
	{ STEP4(0,8) },
	{ STEP4(0,32) },
	16*8
};

static GFXDECODE_START( gfx_bosco )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_2bpp,       0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout_bosco, 64*4, 64 )
	GFXDECODE_ENTRY( "gfx3", 0, dotlayout,     64*4+64*4,  1 )
GFXDECODE_END

static GFXDECODE_START( gfx_galaga )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_2bpp,        0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout_galaga, 64*4, 64 )
GFXDECODE_END


void galaga_state::vblank_irq(int state)
{
	if (state && m_main_irq_mask)
		m_maincpu->set_input_line(0, ASSERT_LINE);

	if (state && m_sub_irq_mask)
		m_subcpu->set_input_line(0, ASSERT_LINE);
}

void bosco_state::bosco(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/6);   /* 3.072 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &bosco_state::bosco_map);

	Z80(config, m_subcpu, MASTER_CLOCK/6);    /* 3.072 MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &bosco_state::bosco_map);

	Z80(config, m_subcpu2, MASTER_CLOCK/6);   /* 3.072 MHz */
	m_subcpu2->set_addrmap(AS_PROGRAM, &bosco_state::bosco_map);

	ls259_device &misclatch(LS259(config, "misclatch")); // 3C on CPU board
	misclatch.q_out_cb<0>().set(FUNC(bosco_state::irq1_clear_w));
	misclatch.q_out_cb<1>().set(FUNC(bosco_state::irq2_clear_w));
	misclatch.q_out_cb<2>().set(FUNC(bosco_state::nmion_w));
	misclatch.q_out_cb<3>().set_inputline("sub", INPUT_LINE_RESET).invert();
	misclatch.q_out_cb<3>().append_inputline("sub2", INPUT_LINE_RESET).invert();
	misclatch.q_out_cb<3>().append("50xx_1", FUNC(namco_50xx_device::reset));
	misclatch.q_out_cb<3>().append("51xx", FUNC(namco_51xx_device::reset));
	misclatch.q_out_cb<3>().append("54xx", FUNC(namco_54xx_device::reset));

	NAMCO_50XX(config, "50xx_1", MASTER_CLOCK/6/2); /* 1.536 MHz */
	NAMCO_50XX(config, "50xx_2", MASTER_CLOCK/6/2); /* 1.536 MHz */

	namco_51xx_device &n51xx(NAMCO_51XX(config, "51xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n51xx.input_callback<0>().set_ioport("IN0").mask(0x0f);
	n51xx.input_callback<1>().set_ioport("IN0").rshift(4);
	n51xx.input_callback<2>().set_ioport("IN1").mask(0x0f);
	n51xx.input_callback<3>().set_ioport("IN1").rshift(4);
	n51xx.output_callback().set(FUNC(bosco_state::out));
	n51xx.lockout_callback().set(FUNC(bosco_state::lockout));

	namco_52xx_device &n52xx(NAMCO_52XX(config, "52xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n52xx.set_discrete("discrete");
	n52xx.set_basenote(NODE_04);
	n52xx.set_extclock(ATTOSECONDS_IN_NSEC(PERIOD_OF_555_ASTABLE_NSEC(RES_K(33), RES_K(10), CAP_U(0.0047))));
	n52xx.romread_callback().set(FUNC(bosco_state::namco_52xx_rom_r));
	n52xx.si_callback().set(FUNC(bosco_state::namco_52xx_si_r));

	namco_54xx_device &n54xx(NAMCO_54XX(config, "54xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n54xx.set_discrete("discrete");
	n54xx.set_basenote(NODE_01);

	namco_06xx_device &n06xx_0(NAMCO_06XX(config, "06xx_0", MASTER_CLOCK/6/64));
	n06xx_0.set_maincpu(m_maincpu);
	n06xx_0.chip_select_callback<0>().set("51xx", FUNC(namco_51xx_device::chip_select));
	n06xx_0.rw_callback<0>().set("51xx", FUNC(namco_51xx_device::rw));
	n06xx_0.read_callback<0>().set("51xx", FUNC(namco_51xx_device::read));
	n06xx_0.write_callback<0>().set("51xx", FUNC(namco_51xx_device::write));
	n06xx_0.chip_select_callback<2>().set("50xx_1", FUNC(namco_50xx_device::chip_select));
	n06xx_0.rw_callback<2>().set("50xx_1", FUNC(namco_50xx_device::rw));
	n06xx_0.read_callback<2>().set("50xx_1", FUNC(namco_50xx_device::read));
	n06xx_0.write_callback<2>().set("50xx_1", FUNC(namco_50xx_device::write));
	n06xx_0.chip_select_callback<3>().set("54xx", FUNC(namco_54xx_device::chip_select));
	n06xx_0.write_callback<3>().set("54xx", FUNC(namco_54xx_device::write));

	// The clock should be hblank, but approx with 512.
	namco_06xx_device &n06xx_1(NAMCO_06XX(config, "06xx_1", MASTER_CLOCK/6/512));
	n06xx_1.set_maincpu(m_subcpu);
	n06xx_1.read_callback<0>().set("50xx_2", FUNC(namco_50xx_device::read));
	n06xx_1.chip_select_callback<0>().set("50xx_2", FUNC(namco_50xx_device::chip_select));
	n06xx_1.rw_callback<0>().set("50xx_2", FUNC(namco_50xx_device::rw));
	n06xx_1.write_callback<0>().set("50xx_2", FUNC(namco_50xx_device::write));
	n06xx_1.write_callback<1>().set("52xx", FUNC(namco_52xx_device::write));
	n06xx_1.chip_select_callback<1>().set("52xx", FUNC(namco_52xx_device::chip_select));

	LS259(config, m_videolatch); // 1B on video board
	m_videolatch->q_out_cb<0>().set(FUNC(bosco_state::flip_screen_set)).invert();
	// Q4-Q5 to 05XX for starfield blink
	m_videolatch->q_out_cb<7>().set("50xx_2", FUNC(namco_50xx_device::reset));
	m_videolatch->q_out_cb<7>().append("52xx", FUNC(namco_52xx_device::reset));

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count(m_screen, 8);

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	SCREEN(config, m_screen);
	m_screen->set_raw(MASTER_CLOCK/3, 384, 0, 288, 264, 16, 224+16);
	m_screen->set_screen_update(FUNC(bosco_state::screen_update_bosco));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE); // starfield lfsr
	m_screen->screen_vblank().set(FUNC(bosco_state::screen_vblank_bosco));
	m_screen->screen_vblank().append(FUNC(bosco_state::vblank_irq));
	m_screen->screen_vblank().append("51xx", FUNC(namco_51xx_device::vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bosco);
	PALETTE(config, m_palette, FUNC(bosco_state::bosco_palette), 64*4 + 64*4 + 4 + 64, 32+64);

	STARFIELD_05XX(config, m_starfield);
	m_starfield->set_starfield_config(0, STARFIELD_Y_OFFSET_BOSCO, STARFIELD_X_LIMIT_BOSCO);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NAMCO_WSG(config, m_namco_sound, MASTER_CLOCK/6/32);
	m_namco_sound->add_route(ALL_OUTPUTS, "mono", 0.90 * 10.0 / 16.0);

	/* discrete circuit on the 54XX outputs */
	DISCRETE(config, "discrete", bosco_discrete).add_route(ALL_OUTPUTS, "mono", 0.90);
}

void galaga_state::galaga(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/6);   /* 3.072 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &galaga_state::galaga_map);

	Z80(config, m_subcpu, MASTER_CLOCK/6);    /* 3.072 MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &galaga_state::galaga_map);

	Z80(config, m_subcpu2, MASTER_CLOCK/6);   /* 3.072 MHz */
	m_subcpu2->set_addrmap(AS_PROGRAM, &galaga_state::galaga_map);

	ls259_device &misclatch(LS259(config, "misclatch")); // 3C on CPU board
	misclatch.q_out_cb<0>().set(FUNC(galaga_state::irq1_clear_w));
	misclatch.q_out_cb<1>().set(FUNC(galaga_state::irq2_clear_w));
	misclatch.q_out_cb<2>().set(FUNC(galaga_state::nmion_w));
	misclatch.q_out_cb<3>().set_inputline("sub", INPUT_LINE_RESET).invert();
	misclatch.q_out_cb<3>().append_inputline("sub2", INPUT_LINE_RESET).invert();
	misclatch.q_out_cb<3>().append("51xx", FUNC(namco_51xx_device::reset));
	misclatch.q_out_cb<3>().append("54xx", FUNC(namco_54xx_device::reset));

	namco_51xx_device &n51xx(NAMCO_51XX(config, "51xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n51xx.input_callback<0>().set_ioport("IN0").mask(0x0f);
	n51xx.input_callback<1>().set_ioport("IN0").rshift(4);
	n51xx.input_callback<2>().set_ioport("IN1").mask(0x0f);
	n51xx.input_callback<3>().set_ioport("IN1").rshift(4);
	n51xx.output_callback().set(FUNC(galaga_state::out));
	n51xx.lockout_callback().set(FUNC(galaga_state::lockout));

	namco_54xx_device &n54xx(NAMCO_54XX(config, "54xx", MASTER_CLOCK/6/2));      /* 1.536 MHz */
	n54xx.set_discrete("discrete");
	n54xx.set_basenote(NODE_01);

	namco_06xx_device &n06xx(NAMCO_06XX(config, "06xx", MASTER_CLOCK/6/64));
	n06xx.set_maincpu(m_maincpu);
	n06xx.chip_select_callback<0>().set("51xx", FUNC(namco_51xx_device::chip_select));
	n06xx.rw_callback<0>().set("51xx", FUNC(namco_51xx_device::rw));
	n06xx.read_callback<0>().set("51xx", FUNC(namco_51xx_device::read));
	n06xx.write_callback<0>().set("51xx", FUNC(namco_51xx_device::write));
	n06xx.write_callback<3>().set("54xx", FUNC(namco_54xx_device::write));
	n06xx.chip_select_callback<3>().set("54xx", FUNC(namco_54xx_device::chip_select));

	LS259(config, m_videolatch); // 5K on video board
	// Q0-Q5 to 05XX for starfield control
	m_videolatch->q_out_cb<7>().set(FUNC(galaga_state::flip_screen_set));

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count(m_screen, 8);

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	SCREEN(config, m_screen);
	m_screen->set_raw(MASTER_CLOCK/3, 384, 0, 288, 264, 0, 224);
	m_screen->set_screen_update(FUNC(galaga_state::screen_update_galaga));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE); // starfield lfsr
	m_screen->screen_vblank().set(FUNC(galaga_state::screen_vblank_galaga));
	m_screen->screen_vblank().append(FUNC(galaga_state::vblank_irq));
	m_screen->screen_vblank().append("51xx", FUNC(namco_51xx_device::vblank));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_galaga);
	PALETTE(config, m_palette, FUNC(galaga_state::galaga_palette), 64*4 + 64*4 + 4 + 64, 32+64);

	STARFIELD_05XX(config, m_starfield);
	m_starfield->set_starfield_config(STARFIELD_X_OFFSET_GALAGA, 0, STARFIELD_X_LIMIT_GALAGA);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NAMCO_WSG(config, m_namco_sound, MASTER_CLOCK/6/32);
	m_namco_sound->add_route(ALL_OUTPUTS, "mono", 0.90 * 10.0 / 16.0);

	/* discrete circuit on the 54XX outputs */
	DISCRETE(config, "discrete", galaga_discrete).add_route(ALL_OUTPUTS, "mono", 0.90);
}

void galaga_state::galagab(machine_config &config)
{
	galaga(config);

	/* basic machine hardware */
	config.device_remove("06xx");
	config.device_remove("54xx");
	ls259_device* misclatch = reinterpret_cast<ls259_device*>(config.device("misclatch"));
	// galaga has the custom chips on this line, so just set the resets this board has
	misclatch->q_out_cb<3>().set_inputline("sub", INPUT_LINE_RESET).invert();
	misclatch->q_out_cb<3>().append_inputline("sub2", INPUT_LINE_RESET).invert();
	misclatch->q_out_cb<3>().append_inputline("sub3", INPUT_LINE_RESET).invert();

	/* FIXME: bootlegs should not have any Namco custom chip. However, this workaround is needed atm */
	namco_06xx_device &n06xx(NAMCO_06XX(config, "06xx", MASTER_CLOCK/6/64));
	n06xx.set_maincpu(m_maincpu);
	n06xx.chip_select_callback<0>().set("51xx", FUNC(namco_51xx_device::chip_select));
	n06xx.rw_callback<0>().set("51xx", FUNC(namco_51xx_device::rw));
	n06xx.read_callback<0>().set("51xx", FUNC(namco_51xx_device::read));
	n06xx.write_callback<0>().set("51xx", FUNC(namco_51xx_device::write));

	z80_device &sub3(Z80(config, "sub3", MASTER_CLOCK/6));   /* 3.072 MHz */
	sub3.set_addrmap(AS_PROGRAM, &galaga_state::galaga_mem4);

	/* sound hardware */
	config.device_remove("discrete");
}

void galaga_state::gatsbee(machine_config &config)
{
	galaga(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &galaga_state::gatsbee_main_map);

	ls259_device &extralatch(LS259(config, "extralatch"));
	extralatch.q_out_cb<0>().set(FUNC(galaga_state::gatsbee_bank_w));
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

/**********************************************************************************************
  Bosconian & clones
**********************************************************************************************/
/*

Bosconian
Namco/Midway, 1981

*/

ROM_START( bosco ) // 23209611 (23209631) main PCB + 23169612 (23169632) sub PCB
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "bos5_1.3p",    0x0000, 0x1000, CRC(b1482ad1) SHA1(32d0402fc4882cae2c3655f24087f9f1911f99c1) )
	ROM_LOAD( "bos5_2.3m",    0x1000, 0x1000, CRC(e0828ef8) SHA1(2633c7518bf0918f33dac8fbed7aa7053a4793f0) )
	ROM_LOAD( "bos5_3.2m",    0x2000, 0x1000, CRC(229edd51) SHA1(6e34837b1d18637b94b3e772ff1b61ffe70e8fcc) )
	ROM_LOAD( "bos5_4.2l",   0x3000, 0x1000, CRC(928a39a0) SHA1(dfc5a7ff62a0eabc900b43ef6b8e86e4d46125fe) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "bos5_5.3f",   0x0000, 0x1000, CRC(84f7c1ea) SHA1(53a1242490575938fca9e738546c93edf82ad7d3) )
	ROM_LOAD( "bos5_6.3j",   0x1000, 0x1000, CRC(7fa34d5e) SHA1(c99feb051ab62ef3f7278a411938cd09e731fc19) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "bos1_7.2c",    0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bos1_14.5d",   0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "bos1_13.5e",   0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "bos1-4.2r",    0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bos1-6.6b",    0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bos1-5.4m",    0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "bos1-3.2d",    0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "bos1-7.7h",    0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "bos1-1.1d",    0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "bos1-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "bos1_9.5n",    0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "bos1_10.5m",   0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "bos1_11.5k",   0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )
ROM_END

ROM_START( bosco3 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "bos3_1.3n",    0x0000, 0x1000, CRC(96021267) SHA1(bd49b0caabcccf9df45a272d767456a4fc8a7c07) )
	ROM_LOAD( "bos1_2.3m",    0x1000, 0x1000, CRC(2d8f3ebe) SHA1(75de1cba7531ae4bf7fbbef7b8e37b9fec4ed0d0) )
	ROM_LOAD( "bos1_3.3l",    0x2000, 0x1000, CRC(c80ccfa5) SHA1(f2bbec2ea9846d4601f06c0b4242744447a88fda) )
	ROM_LOAD( "bos1_4b.3k",   0x3000, 0x1000, CRC(a3f7f4ab) SHA1(eb26184311bae0767c7a5593926e6eadcbcb680e) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "bos1_5c.3j",   0x0000, 0x1000, CRC(a7c8e432) SHA1(3607be75daa10f1f98dbfd9e600c5ba513130d44) )
	ROM_LOAD( "bos3_6.3h",    0x1000, 0x1000, CRC(4543cf82) SHA1(50ad7d1ab6694eb8fab88d0fa79ee04f6984f3ca) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "bos1_7.3e",    0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bos1_14.5d",   0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "bos1_13.5e",   0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "bos1-4.2r",    0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bos1-6.6b",    0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bos1-5.4m",    0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "bos1-3.2d",    0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "bos1-7.7h",    0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "bos1-1.1d",    0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "bos1-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "bos1_9.5n",    0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "bos1_10.5m",   0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "bos1_11.5k",   0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )
ROM_END

ROM_START( bosco1 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "bos1_1.3n",    0x0000, 0x1000, CRC(0d9920e7) SHA1(e7633233f603ccb5b7a970ed5b58ef361ef2c94e) )
	ROM_LOAD( "bos1_2.3m",    0x1000, 0x1000, CRC(2d8f3ebe) SHA1(75de1cba7531ae4bf7fbbef7b8e37b9fec4ed0d0) )
	ROM_LOAD( "bos1_3.3l",    0x2000, 0x1000, CRC(c80ccfa5) SHA1(f2bbec2ea9846d4601f06c0b4242744447a88fda) )
	ROM_LOAD( "bos1_4b.3k",   0x3000, 0x1000, CRC(a3f7f4ab) SHA1(eb26184311bae0767c7a5593926e6eadcbcb680e) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "bos1_5c.3j",   0x0000, 0x1000, CRC(a7c8e432) SHA1(3607be75daa10f1f98dbfd9e600c5ba513130d44) )
	ROM_LOAD( "bos1_6.3h",    0x1000, 0x1000, CRC(31b8c648) SHA1(de0db24d385d2361ec989bf32388df8202ad535c) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "bos1_7.3e",    0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bos1_14.5d",   0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "bos1_13.5e",   0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "bos1-4.2r",    0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bos1-6.6b",    0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bos1-5.4m",    0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "bos1-3.2d",    0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "bos1-7.7h",    0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "bos1-1.1d",    0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "bos1-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "bos1_9.5n",    0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "bos1_10.5m",   0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "bos1_11.5k",   0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )
ROM_END

ROM_START( bosco1o )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "bos1_1.3n",    0x0000, 0x1000, CRC(0d9920e7) SHA1(e7633233f603ccb5b7a970ed5b58ef361ef2c94e) )
	ROM_LOAD( "bos1_2.3m",    0x1000, 0x1000, CRC(2d8f3ebe) SHA1(75de1cba7531ae4bf7fbbef7b8e37b9fec4ed0d0) )
	ROM_LOAD( "bos1_3.3l",    0x2000, 0x1000, CRC(c80ccfa5) SHA1(f2bbec2ea9846d4601f06c0b4242744447a88fda) )
	ROM_LOAD( "bos1_4.3k",    0x3000, 0x1000, CRC(7ebea2b8) SHA1(92fc66526ed77f3efd947b7d321b255aba4a0140) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "bos1_5b.3j",   0x0000, 0x1000, CRC(3d6955a8) SHA1(f89860d74865da5ced2f5b2196bdaa8eeb5e2322) )
	ROM_LOAD( "bos1_6.3h",    0x1000, 0x1000, CRC(31b8c648) SHA1(de0db24d385d2361ec989bf32388df8202ad535c) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "bos1_7.3e",    0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bos1_14.5d",   0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "bos1_13.5e",   0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "bos1-4.2r",    0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bos1-6.6b",    0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bos1-5.4m",    0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "bos1-3.2d",    0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "bos1-7.7h",    0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "bos1-1.1d",    0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "bos1-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "bos1_9.5n",    0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "bos1_10.5m",   0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "bos1_11.5k",   0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )
ROM_END

/*
    Bosconian - Midway Version

    CPU/Sound Board: A084-91412-B550
    Video Board:     A084-91413-B550
*/

ROM_START( boscomd )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "3n",       0x0000, 0x1000, CRC(441b501a) SHA1(7b4921ff40b3c56950fd32aa0ec5563b02a00929) )
	ROM_LOAD( "3m",       0x1000, 0x1000, CRC(a3c5c7ef) SHA1(70a095a8dbca857245a70404f803916f519e0cbc) )
	ROM_LOAD( "3l",       0x2000, 0x1000, CRC(6ca9a0cf) SHA1(8f70e29beae921e63cd65689a618ca678dd14614) )
	ROM_LOAD( "3k",       0x3000, 0x1000, CRC(d83bacc5) SHA1(cf2fbfa81dabb9b6bcf436d61992e705723776fb) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "3j",       0x0000, 0x1000, CRC(4374e39a) SHA1(7571fd5961f49a0e9ba4301ddd0aca52e94e2f8b) )
	ROM_LOAD( "3h",       0x1000, 0x1000, CRC(04e9fcef) SHA1(2115a9718d511854848704e2693f9efa1c80a307) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "2900.3e",      0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5300.5d",      0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "5200.5e",      0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "prom.2d",      0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bosco.6b",     0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bosco.4m",     0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "prom.2r",      0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "prom.7h",      0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom.1d",      0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "prom.5c",      0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "4900.5n",      0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "5000.5m",      0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "5100.5l",      0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )

	ROM_REGION( 0x0001, "pal_vidbd", 0 ) /* PAL located on the video board */
	ROM_LOAD( "0066-005xx-xxqx.5a", 0x00000, 0x00001, NO_DUMP ) /* According to the manual it's a PAL. What type is unknown. */
ROM_END

ROM_START( boscomdo )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "2300.3n",      0x0000, 0x1000, CRC(db6128b0) SHA1(ddd285f7e00d5e58ab9b15838528e0020d47fcd2) )
	ROM_LOAD( "2400.3m",      0x1000, 0x1000, CRC(86907614) SHA1(3295ab6c5171a069875c2239b3325296c1df6031) )
	ROM_LOAD( "2500.3l",      0x2000, 0x1000, CRC(a21fae11) SHA1(dff38d90ee30558274d2d399edc3281c2ef5cb69) )
	ROM_LOAD( "2600.3k",      0x3000, 0x1000, CRC(11d6ae23) SHA1(f2f72f5c777b684f7ffd53b9c034560211113499) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "2700.3j",      0x0000, 0x1000, CRC(7254e65e) SHA1(c2ee29fcb5173e8d46a80a8a1b931a53dbdeae66) )
	ROM_LOAD( "2800.3h",      0x1000, 0x1000, CRC(31b8c648) SHA1(de0db24d385d2361ec989bf32388df8202ad535c) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "2900.3e",      0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5300.5d",      0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "5200.5e",      0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "prom.2d",      0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bosco.6b",     0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bosco.4m",     0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "prom.2r",      0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "prom.7h",      0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom.1d",      0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "prom.5c",      0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "4900.5n",      0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "5000.5m",      0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "5100.5l",      0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )

	ROM_REGION( 0x0001, "pal_vidbd", 0 ) /* PAL located on the video board */
	ROM_LOAD( "0066-005xx-xxqx.5a", 0x00000, 0x00001, NO_DUMP ) /* According to the manual it's a PAL. What type is unknown. */
ROM_END

/**********************************************************************************************
  Galaga & clones
**********************************************************************************************/
/*

Galaga
Namco/Midway, 1982

PCB Layout
----------

Top board

23149611 (23149631
|------------------------------------------|
|                         LM324            |
|          04M_G01.3N                      |
|                       Z80        5400    |
|          04K_G02.3M                      |
|                                          |
|   0600   04J_G03.3L                    |-|
|                                 DSW1   |
|          04H_G04.3K             DSW2   |-|
|                                          |
|   0801   04E_G05.3J   Z80               4|
|                                         4|
|                                         W|
|          *            5100              A|
|   0801                                  Y|
|                                TD62064   |
|          04D_G06.3E   Z80                |
|                                        |-|
|   0801                                 |
|          *                             |-|
|                       0702     VOL       |
|                                    MB3730|
|GG1-1.1D                                  |
|                       GG1-2.5C           |
|       3101                               |
|       3101  4066                18.432MHz|
|------------------------------------------|
Notes:
      GG1-1.1D & GG1-2.5C are PROMs, type MB7052 (equivalent to TBP24S10 and 82S129).
      All other ROMs are 2732 EPROMs (i.e. 04*.*).
      *: Unpopulated sockets

      VSync           : 60.606060Hz
      Z80 clocks (all): 1.536MHz
      5400 clock      : 1.536MHz
      5100 clock      : 1.536MHz

      3101   : 16bytes x4 bit Bipolar SRAM, compatible with 7489, MB461 & AM31L01 (trivia - This was Intel's first product, released in 1969!)
      MB3730 : Sound AMP
      TD62064: Darlington transistor for driving coin counters.
      4066   : Quad Bilateral Switch logic IC, used to mix several sound sources to one output.

      NAMCO customs:
                    0600 (DIP28): Bus Interface IC
                    0801 (DIP28): Multi CPU Bus Controller IC
                    5100 (DIP42): Controls player input, coins, DSW's (custom 4 bit I/O Microcontroller)
                    0702 (DIP28): Sync Generator/Clock Divider IC
                    5400 (DIP28): MUX 4-channel Audio Generator IC. Generates 'death bang'.
                                  This is not a Z80 with swapped pins as many sites have reported.

      Pinouts:
      Galaga PCB edge connector pinouts

      Parts Side    Pin   Pin Solder Side
      ----------------------------------
      Logic Ground   A     1  Logic Ground
      Speaker +      B     2  Speaker -
                     C     3  Coin Counter 1
      P1 Start Lamp  D     4  P2 Start Lamp
      +12            E     5  +12
      +5             F     6  +5
      Ground         H     7  Ground
      Service Credit J     8  Test
      Coin 1         K     9  Coin 2
      Player 1 Start L     10 Player 2 Start
      P1 Fire        M     11 P2 Fire
      P1 Left        N     12 P2 Left
                     P     13
      P1 Right       R     14 P2 Right
                     S     15
                     T     16
                     U     17
                     V     18
                     W     19
                     X     20
      Coin Counter 2 Y     21 Cocktail Mode
      Ground         Z     22 Ground

      Pin21: Ground this pin for cocktail mode


Bottom board

23149612 (23149632
|------------------------------------------|
| 0700     GG1-4.2N           GG1-5.5N     |
|                        *             RGBS|
| 0015                                     |
|                                          |
|              2114                        |
| 6116                               8147  |
|              2114    07M_G08.4L          |
|                                    8147  |
|              2114                        |
| 0400                               8147  |
|              2114                        |
|                                    8147  |
|              2114    0200                |
|                                          |
|              2114                        |
|                      07H_G09.4F    8147  |
|                                          |
|                                    8147  |
|                                          |
|                      07E_G10.4D    8147  |
|                                          |
|                                    8147  |
| GG1-3.1C                                 |
|                                          |
|                                          |
|------------------------------------------|
Notes:
      RGBS: Video output socket (Red, Green Blue, Sync to monitor)
      GG1*  are PROMs, type MB7052 (equivalent to TBP24S10 and 82S129).
      All other ROMs are 2732 EPROMs.
      *: Unpopulated socket

      2114    : 1K x4 SRAM
      6116    : 2K x8 SRAM
      8147    : 4K x1 SRAM (Note - you can remove the eight 8147 RAMs and install two 2148s (1K x 4) in their place at positions 6H and 6B.

      Bootup RAM Errors
      Error Code    Meaning
      RAM OK        All RAMs are good
      RAM 0L        RAM located on Video PC board at position 1K is bad
      RAM 0H        RAM located on Video PC board at position 1K is bad
      RAM 1L        RAM located on Video PC board at position 1K is bad
      RAM 1H        RAM located on Video PC board at position 1K is bad
      RAM 2L        RAM located on Video PC board at position 3E is bad
      RAM 2H        RAM located on Video PC board at position 3F is bad
      RAM 3L        RAM located on Video PC board at position 3K is bad
      RAM 3H        RAM located on Video PC board at position 3L is bad
      RAM 4L        RAM located on Video PC board at position 3H is bad
      RAM 4H        RAM located on Video PC board at position 3J is bad

      Bootup ROM Errors
      Error Code   Meaning
      ROM OK       All ROMs are good
      ROM 01       ROM located on CPU PC board at position 3N is bad
      ROM 02       ROM located on CPU PC board at position 3M is bad
      ROM 03       ROM located on CPU PC board at position 3L is bad
      ROM 04       ROM located on CPU PC board at position 3K is bad
      ROM 11       ROM located on CPU PC board at position 3J is bad
      ROM 21       ROM located on CPU PC board at position 3E is bad

      NAMCO customs:
                    0015 (DIP28): Video RAM addresser IC
                    0200 (DIP28): Graphics ROM Data Custom Shift Register IC
                    0400 (DIP28): Motion Object and Scratch RAM to CPU Bus Interface IC
                    0702 (DIP28): Sync Generator/Clock Divider IC

There's also a Sidam version of Galaga (Sidam PCB 11200, official license for the Italian market),
but, unlike Sidam's Dig Dug, this one does not use any unique ROM, but a mix between the two Namco sets:
     bottom-10_2532.4d  = gg1_11.4d  galaga   Galaga (Namco rev. B)
     bottom-8_2532.4h   = gg1_9.4l   galaga   Galaga (Namco rev. B)
     bottom-9_2532.4f   = gg1_10.4f  galaga   Galaga (Namco rev. B)
     top-0_2532.3n      = gg1-1.3p   galagao  Galaga (Namco)
     top-1_2532.3m      = gg1-2.3m   galagao  Galaga (Namco)
     top-2_2532.3l      = gg1_3.2m   galaga   Galaga (Namco rev. B)
     top-3_2532.3k      = gg1-4.2l   galagao  Galaga (Namco)
     top-4_2532.3j      = gg1_5b.3f  galaga   Galaga (Namco rev. B)
     top-6_2532.3e      = gg1_7b.2c  galaga   Galaga (Namco rev. B)
*/

ROM_START( galaga )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "gg1_1b.3p",    0x0000, 0x1000, CRC(ab036c9f) SHA1(ca7f5da42d4e76fd89bb0b35198a23c01462fbfe) )
	ROM_LOAD( "gg1_2b.3m",    0x1000, 0x1000, CRC(d9232240) SHA1(ab202aa259c3d332ef13dfb8fc8580ce2a5a253d) )
	ROM_LOAD( "gg1_3.2m",     0x2000, 0x1000, CRC(753ce503) SHA1(481f443aea3ed3504ec2f3a6bfcf3cd47e2f8f81) )
	ROM_LOAD( "gg1_4b.2l",    0x3000, 0x1000, CRC(499fcc76) SHA1(ddb8b121903646c320939c7d13f4aa4ebb130378) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gg1_5b.3f",    0x0000, 0x1000, CRC(bb5caae3) SHA1(e957a581463caac27bc37ca2e2a90f27e4f62b6f) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gg1_7b.2c",    0x0000, 0x1000, CRC(d016686b) SHA1(44c1a04fba3c7c826ff484185cb881b4b22e6657) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gg1_9.4l",     0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gg1_11.4d",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gg1_10.4f",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( galagao )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "gg1-1.3p",     0x0000, 0x1000, CRC(a3a0f743) SHA1(6907773db7c002ecde5e41853603d53387c5c7cd) )
	ROM_LOAD( "gg1-2.3m",     0x1000, 0x1000, CRC(43bb0d5c) SHA1(666975aed5ce84f09794c54b550d64d95ab311f0) )
	ROM_LOAD( "gg1-3.2m",     0x2000, 0x1000, CRC(753ce503) SHA1(481f443aea3ed3504ec2f3a6bfcf3cd47e2f8f81) )
	ROM_LOAD( "gg1-4.2l",     0x3000, 0x1000, CRC(83874442) SHA1(366cb0dbd31b787e64f88d182108b670d03b393e) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gg1-5.3f",     0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gg1-7.2c",     0x0000, 0x1000, CRC(8995088d) SHA1(d6cb439de0718826d1a0363c9d77de8740b18ecf) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gg1-9.4l",     0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gg1-11.4d",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gg1-10.4f",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( galagamw )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "3200a.bin",    0x0000, 0x1000, CRC(3ef0b053) SHA1(0c04a362b737998c0952a753fb3fd8c8a17e9b46) )
	ROM_LOAD( "3300b.bin",    0x1000, 0x1000, CRC(1b280831) SHA1(f7ea12e61929717ebe43a4198a97f109845a2c62) )
	ROM_LOAD( "3400c.bin",    0x2000, 0x1000, CRC(16233d33) SHA1(a7eb799be5e23058754a92b15e6527bfbb47a354) )
	ROM_LOAD( "3500d.bin",    0x3000, 0x1000, CRC(0aaf5c23) SHA1(3f4b0bb960bf002261e9c1278c88f594c6aa8ab6) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "3600e.bin",    0x0000, 0x1000, CRC(bc556e76) SHA1(0d3d68243c4571d985b4d8f7e0ea9f6fcffa2116) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "3700g.bin",    0x0000, 0x1000, CRC(b07f0aa4) SHA1(7528644a8480d0be2d0d37069515ed319e94778f) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "2600j.bin",    0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "2800l.bin",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "2700k.bin",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( galagamf )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "3200a.bin",    0x0000, 0x1000, CRC(3ef0b053) SHA1(0c04a362b737998c0952a753fb3fd8c8a17e9b46) )
	ROM_LOAD( "3300b.bin",    0x1000, 0x1000, CRC(1b280831) SHA1(f7ea12e61929717ebe43a4198a97f109845a2c62) )
	ROM_LOAD( "3400c.bin",    0x2000, 0x1000, CRC(16233d33) SHA1(a7eb799be5e23058754a92b15e6527bfbb47a354) )
	ROM_LOAD( "3500d.bin",    0x3000, 0x1000, CRC(0aaf5c23) SHA1(3f4b0bb960bf002261e9c1278c88f594c6aa8ab6) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "3600fast.bin", 0x0000, 0x1000, CRC(23d586e5) SHA1(43346c69385e9091e64cff6c027ac2689cafcbb9) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "3700g.bin",    0x0000, 0x1000, CRC(b07f0aa4) SHA1(7528644a8480d0be2d0d37069515ed319e94778f) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "2600j.bin",    0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "2800l.bin",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "2700k.bin",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( galagamk )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "mk2-1",        0x0000, 0x1000, CRC(23cea1e2) SHA1(18db33ade0ca6e47cc48aa151d2ccbb4646e3ae3) )
	ROM_LOAD( "mk2-2",        0x1000, 0x1000, CRC(89695b1a) SHA1(fda5557018884e903f855bf3b69a25d75ed8a767) )
	ROM_LOAD( "3400c.bin",    0x2000, 0x1000, CRC(16233d33) SHA1(a7eb799be5e23058754a92b15e6527bfbb47a354) )
	ROM_LOAD( "mk2-4",        0x3000, 0x1000, CRC(24b767f5) SHA1(d4c03e2ed582cfa7f8168ac352f790ef7af54cb8) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gg1-5.3f",     0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gg1-7b.2c",    0x0000, 0x1000, CRC(d016686b) SHA1(44c1a04fba3c7c826ff484185cb881b4b22e6657) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gg1-9.4l",     0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gg1-11.4d",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gg1-10.4f",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( gallag )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "gallag.1",     0x0000, 0x1000, CRC(a3a0f743) SHA1(6907773db7c002ecde5e41853603d53387c5c7cd) )
	ROM_LOAD( "gallag.2",     0x1000, 0x1000, CRC(5eda60a7) SHA1(853d7b974dd04abd7af3a8ba2681dfabce4dce18) )
	ROM_LOAD( "gallag.3",     0x2000, 0x1000, CRC(753ce503) SHA1(481f443aea3ed3504ec2f3a6bfcf3cd47e2f8f81) )
	ROM_LOAD( "gallag.4",     0x3000, 0x1000, CRC(83874442) SHA1(366cb0dbd31b787e64f88d182108b670d03b393e) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gallag.5",     0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gallag.7",     0x0000, 0x1000, CRC(8995088d) SHA1(d6cb439de0718826d1a0363c9d77de8740b18ecf) )

	ROM_REGION( 0x10000, "sub3", 0 )    /* 64k for a Z80 which emulates the custom I/O chip (not used) */
	ROM_LOAD( "gallag.6",     0x0000, 0x1000, CRC(001b70bc) SHA1(b465eee91e75257b7b049d49c0064ab5fd66c576) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gallag.8",     0x0000, 0x1000, CRC(169a98a4) SHA1(edbeb11076061e744ea88d9899dbdfe0964c7e78) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gallag.a",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gallag.9",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( gatsbee )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "1.4b",         0x0000, 0x1000, CRC(9fb8e28b) SHA1(7171e3fb37b0d6cc8f7a023c1775080d5986de99) )
	ROM_LOAD( "2.4c",         0x1000, 0x1000, CRC(bf6cb840) SHA1(5763140d32d35a38cdcb49e6de1fd5b07a9e8cc2) )
	ROM_LOAD( "3.4d",         0x2000, 0x1000, CRC(3604e2dd) SHA1(1736cf8497f7ac28e92ca94fa137c144353dc192) )
	ROM_LOAD( "4.4e",         0x3000, 0x1000, CRC(bf9f613b) SHA1(41c852fc77f0f35bf48a5b81a19234ed99871c89) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gg1-5.3f",     0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )    // 5.4j

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gg1-7.2c",     0x0000, 0x1000, CRC(8995088d) SHA1(d6cb439de0718826d1a0363c9d77de8740b18ecf) )    // 7.4k

	ROM_REGION( 0x10000, "sub3", 0 )    /* 64k for a Z80 which emulates the custom I/O chip (not used) */
	ROM_LOAD( "gallag.6",     0x0000, 0x1000, CRC(001b70bc) SHA1(b465eee91e75257b7b049d49c0064ab5fd66c576) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "8.5r",  0x0000, 0x2000, CRC(b324f650) SHA1(7bcb254f7cf03bd84291b9fdc27b8962b3e12aa4) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "9.6a",         0x0000, 0x1000, CRC(22e339d5) SHA1(9ac2887ede802d28daa4ad0a0a54bcf7b1155a2e) )
	ROM_LOAD( "10.7a",        0x1000, 0x1000, CRC(60dcf940) SHA1(6530aa5b4afef4a8422ece76a93d0c5b1d93355e) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( nebulbee )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nebulbee.01",  0x0000, 0x1000, CRC(f405f2c4) SHA1(9249afeffd8df0f24539ea9b4f88c23a6ad58d8c) )
	ROM_LOAD( "nebulbee.02",  0x1000, 0x1000, CRC(31022b60) SHA1(90e64afb4128c6dfeeee89635ea9f97a34f70f5f) )
	ROM_LOAD( "gg1_3.2m",     0x2000, 0x1000, CRC(753ce503) SHA1(481f443aea3ed3504ec2f3a6bfcf3cd47e2f8f81) )
	ROM_LOAD( "nebulbee.04",  0x3000, 0x1000, CRC(d76788a5) SHA1(adcb83cf64951d86c701a99b410e9230912f8a48) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gg1-5",        0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "gg1-7",        0x0000, 0x1000, CRC(8995088d) SHA1(d6cb439de0718826d1a0363c9d77de8740b18ecf) )

	ROM_REGION( 0x10000, "sub3", 0 )    /* 64k for a Z80 which emulates the custom I/O chip (not used) */
	ROM_LOAD( "nebulbee.07",     0x0000, 0x1000, CRC(035e300c) SHA1(cfda2467e71c27381b7150ff8fc7b69d61df123a) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gg1_9.4l",     0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gg1_11.4d",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gg1_10.4f",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "prom-5.5n",       0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )
	ROM_LOAD( "2n.bin",       0x0020, 0x0100, CRC(a547d33b) SHA1(7323084320bb61ae1530d916f5edd8835d4d2461) )
	ROM_LOAD( "1c.bin",       0x0120, 0x0100, CRC(b6f585fb) SHA1(dd10147c4f05fede7ae6e7a760681700a660e87e) )
	ROM_LOAD( "5c.bin",       0x0220, 0x0100, CRC(8bd565f6) SHA1(bedba65816abfc2ebeacac6ee335ca6f136e3e3d) )

	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "1d.bin",       0x0000, 0x0100, CRC(86d92b24) SHA1(6bef9102b97c83025a2cf84e89d95f2d44c3d2ed) )
ROM_END

void galaga_state::init_galaga()
{
	// TODO: handle in drawing routines not here
	/* swap bytes for flipped character so we can decode them together with normal characters */
	uint8_t *rom = memregion("gfx1")->base();
	int len = memregion("gfx1")->bytes();

	for (int i = 0; i < len; i++)
	{
		if ((i & 0x0808) == 0x0800)
		{
			int t = rom[i];
			rom[i] = rom[i+8];
			rom[i+8] = t;
		}
	}
}


/* Original Namco hardware, with Namco Customs */

//    YEAR, NAME,      PARENT,   MACHINE, INPUT,    STATE,         INIT,         MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1981, bosco,     0,        bosco,   bosco,    bosco_state,   empty_init,   ROT0,   "Namco", "Bosconian - Star Destroyer (version 5)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, bosco3,    bosco,    bosco,   bosco,    bosco_state,   empty_init,   ROT0,   "Namco", "Bosconian - Star Destroyer (version 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, bosco1,    bosco,    bosco,   bosco,    bosco_state,   empty_init,   ROT0,   "Namco", "Bosconian - Star Destroyer (version 1, newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, bosco1o,   bosco,    bosco,   bosco,    bosco_state,   empty_init,   ROT0,   "Namco", "Bosconian - Star Destroyer (version 1, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, boscomd,   bosco,    bosco,   boscomd,  bosco_state,   empty_init,   ROT0,   "Namco (Midway license)", "Bosconian - Star Destroyer (Midway, new version)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, boscomdo,  bosco,    bosco,   boscomd,  bosco_state,   empty_init,   ROT0,   "Namco (Midway license)", "Bosconian - Star Destroyer (Midway, old version)", MACHINE_SUPPORTS_SAVE )

GAME( 1981, galaga,    0,        galaga,  galaga,   galaga_state,  init_galaga,  ROT90,  "Namco", "Galaga (Namco rev. B)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, galagao,   galaga,   galaga,  galaga,   galaga_state,  init_galaga,  ROT90,  "Namco", "Galaga (Namco)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, galagamw,  galaga,   galaga,  galagamw, galaga_state,  init_galaga,  ROT90,  "Namco (Midway license)", "Galaga (Midway set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, galagamk,  galaga,   galaga,  galaga,   galaga_state,  init_galaga,  ROT90,  "Namco (Midway license)", "Galaga (Midway set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, galagamf,  galaga,   galaga,  galaga,   galaga_state,  init_galaga,  ROT90,  "Namco (Midway license)", "Galaga (Midway set 1 with fast shoot hack)", MACHINE_SUPPORTS_SAVE )

/* Bootlegs with replacement I/O chips */

GAME( 1982, gallag,    galaga,   galagab, galaga,   galaga_state,  init_galaga,  ROT90,  "bootleg", "Gallag", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1984, gatsbee,   galaga,   gatsbee, gatsbee,  galaga_state,  init_galaga,  ROT90,  "hack (Uchida)", "Gatsbee", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1981, nebulbee,  galaga,   galagab, galaga,   galaga_state,  init_galaga,  ROT90,  "bootleg", "Nebulous Bee", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
