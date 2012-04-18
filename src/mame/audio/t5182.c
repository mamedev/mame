/***************************************************************************

Toshiba T5182 die map, by Jonathan Gevaryahu AKA Lord Nightmare,
with assistance from Kevin Horton.
T5182 supplied by Tomasz 'Dox' Slanina

Die Diagram:
|------------------------|
\ ROM  RAM  Z80    A     |
/ B    C    D   E  F  G  |
|------------------------|

The ROM is a 23128 wired as a 2364 by tying a13 to /ce
The RAM is a 2016
The Z80 is an NMOS Z80
Subdie A is a 7408 quad AND gate
Subdie B is a 74245 bidirectional bus transciever
Subdie C is a 74245 bidirectional bus transciever
Subdie D is a 74245 bidirectional bus transciever
Subdie E is a 74138 1 to 8 decoder/demultiplexer with active low outputs
Subdie F is a 74138 1 to 8 decoder/demultiplexer with active low outputs
Subdie G is a 7408 quad AND gate
Thanks to Kevin Horton for working out most of the logic gate types
from the diagram.


An updated version of Dox's t5182 pinout (originally from mustache.c) follows:

                       ______________________
                     _|*                     |_
               GND  |_|1                   50|_| Vcc
                     _|                      |_
                A8  |_|2                   49|_| A7
                     _|                      |_
                A9  |_|3                   48|_| A6
                     _|                      |_
               A10  |_|4                   47|_| A5
                     _|                      |_
               A11  |_|5                   46|_| A4
                     _|       TOSHIBA        |_
               A12  |_|6       T5182       45|_| A3
                     _|                      |_
               A13  |_|7                   44|_| A2
                     _|     JAPAN  8612      |_
               A14  |_|8                   43|_| A1
                     _|                      |_
               A15  |_|9                   42|_| A0
                     _|                      |_
                D4  |_|10                  41|_| D3
                     _|                      |_
                D5  |_|11                  40|_| D2
                     _|                      |_
                D6  |_|12                  39|_| D1
                     _|                      |_
                D7  |_|13                  38|_| D0
                     _|                      |_
         I/O /EN 2  |_|14                  37|_|  I/O /EN 1
                     _|                      |_
         I/O /EN 3  |_|15                  36|_|  I/O /EN 0
                     _|                      |_
         I/O /EN 4  |_|16                  35|_|  /EN 0x8000-0xFFFF
                     _|                      |_
         I/O /EN 5  |_|17                  34|_|  /EN 0x4000-0x7FFF
                     _|                      |_
  Z80 phi clock in  |_|18                  33|_|  N/C
                     _|                      |_
          Z80 /INT  |_|19                  32|_|  Z80 /RESET
                     _|                      |_
          Z80 /NMI  |_|20                  31|_|  Z80 /BUSRQ Test pin
                     _|                      |_
  Internal ROM /EN  |_|21                  30|_|  74245 'A'+'B' DIR Test pin
                     _|                      |_
 /EN 0x0000-0x1fff  |_|22                  29|_|  Z80 /BUSAK Test pin
                     _|                      |_
Z80 /MREQ Test pin  |_|23                  28|_|  Z80 /WR
                     _|                      |_
Z80 /IORQ Test pin  |_|24                  27|_|  Z80 /RD
                     _|                      |_
               GND  |_|25                  26|_|  Vcc
                      |______________________|

Based on sketch made by Tormod

Note: all pins marked as 'Test pin' are disabled internally and cannot be used
      without removing the chip cover and soldering together test pads.
Note: pins 21 and 22 are both shorted together on the pcb, and go active (low)
      while the internal rom is being read. The internal rom can be disabled by
      pulling /IORQ or /MREQ low, but both of those test pins are disabled, and
      also one would have to use the DIR test pin at the same time to feed the
      z80 a new internal rom. This is PROBABLY how Toshiba intended the t5182
      to be prototyped: a special t5182 with the internal jumpers all connected
      (except for the one between pins 21 and 22) would be given to a company
      who wanted to prototype the internal rom, allowing an external rom to be
      used instead of the internal one by using the test pins.

      However, the fact that pins 21 and 22 were NOT internally connected
      together by Toshiba on the *production* seibu t5182 means a huge
      security hole is opened:

      It is trivial, through external connections, without EVER opening the
      chip, to connect pin 22 to a trojan rom /CE and hence have a user trojan
      program run at 0x0000-0x1fff. Then, connect pin 21 to pin 34 to map the
      internal rom at 0x4000-0x7fff so it can be serially bit-banged out, or,
      even more easily, copied to an nvram chip attached to pin 35. Only 11
      bytes of code in the trojan rom are needed to do this.

      There is no internal protection in the chip at all which prevents the
      internal rom from being connected to an enable other than pin 22, which
      would have prevented this theoretical attack from working


Z80 Memory Map:
0x0000-0x1FFF - external space 0 (connected to internal rom /enable outside the
                chip)
0x2000-0x3fff - Internal RAM, repeated/mirrored 4 times
0x4000-0x7fff - external space 1 (used for communication shared memory?)
0x8000-0xFFFF - external space 2 (used for sound rom)

I/O map:
FEDCBA9876543210
xxxxxxxxx000xxxx i/o /EN 0 goes low
xxxxxxxxx001xxxx i/o /EN 1 goes low
xxxxxxxxx010xxxx i/o /EN 2 goes low
xxxxxxxxx011xxxx i/o /EN 3 goes low
xxxxxxxxx100xxxx i/o /EN 4 goes low
xxxxxxxxx101xxxx i/o /EN 5 goes low
xxxxxxxxx110xxxx i/o /EN 6\__ these two are unbonded pins, so are useless.
xxxxxxxxx111xxxx i/o /EN 7/

IMPORTANT: the data lines for the external rom on darkmist are scrambled on the
SEI8608B board as such:
CPU:     ROM:
D0       D0
D1       D6
D2       D5
D3       D4
D4       D3
D5       D2
D6       D1
D7       D7
Only the data lines are scrambled, the address lines are not.
These lines are NOT scrambled to the ym2151 or anything else, just the external
rom.

***************************************************************************/

#include "emu.h"
#include "t5182.h"
#include "sound/2151intf.h"


enum
{
	VECTOR_INIT,
	YM2151_ASSERT,
	YM2151_CLEAR,
	YM2151_ACK,
	CPU_ASSERT,
	CPU_CLEAR
};

static UINT8 *t5182_sharedram;
static int irqstate;

READ8_HANDLER(t5182_sharedram_r)
{
	return t5182_sharedram[offset];
}

WRITE8_HANDLER(t5182_sharedram_w)
{
	t5182_sharedram[offset] = data;
}

static TIMER_CALLBACK( setirq_callback )
{
	device_t *cpu;

	switch(param)
	{
		case YM2151_ASSERT:
			irqstate |= 1|4;
			break;

		case YM2151_CLEAR:
			irqstate &= ~1;
			break;

		case YM2151_ACK:
			irqstate &= ~4;
			break;

		case CPU_ASSERT:
			irqstate |= 2;	// also used by t5182_sharedram_semaphore_main_r
			break;

		case CPU_CLEAR:
			irqstate &= ~2;
			break;
	}

	cpu = machine.device(CPUTAG_T5182);

	if (cpu == NULL)
		return;

	if (irqstate == 0)	/* no IRQs pending */
		device_set_input_line(cpu,0,CLEAR_LINE);
	else	/* IRQ pending */
		device_set_input_line(cpu,0,ASSERT_LINE);
}



WRITE8_HANDLER( t5182_sound_irq_w )
{
	space->machine().scheduler().synchronize(FUNC(setirq_callback), CPU_ASSERT);
}

static WRITE8_HANDLER( t5182_ym2151_irq_ack_w )
{
	space->machine().scheduler().synchronize(FUNC(setirq_callback), YM2151_ACK);
}

static WRITE8_HANDLER( t5182_cpu_irq_ack_w )
{
	space->machine().scheduler().synchronize(FUNC(setirq_callback), CPU_CLEAR);
}

static void t5182_ym2151_irq_handler(device_t *device, int irq)
{
	if (irq)
		device->machine().scheduler().synchronize(FUNC(setirq_callback), YM2151_ASSERT);
	else
		device->machine().scheduler().synchronize(FUNC(setirq_callback), YM2151_CLEAR);
}



static int semaphore_main, semaphore_snd;

READ8_HANDLER(t5182_sharedram_semaphore_snd_r)
{
	return semaphore_snd;
}

WRITE8_HANDLER(t5182_sharedram_semaphore_main_acquire_w)
{
	semaphore_main = 1;
}

WRITE8_HANDLER(t5182_sharedram_semaphore_main_release_w)
{
	semaphore_main = 0;
}

static WRITE8_HANDLER(t5182_sharedram_semaphore_snd_acquire_w)
{
	semaphore_snd = 1;
}

static WRITE8_HANDLER(t5182_sharedram_semaphore_snd_release_w)
{
	semaphore_snd = 0;
}

static READ8_HANDLER(t5182_sharedram_semaphore_main_r)
{
	return semaphore_main | (irqstate & 2);
}


const ym2151_interface t5182_ym2151_interface =
{
	DEVCB_LINE(t5182_ym2151_irq_handler)
};




	// 4000-407F    RAM shared with main CPU
	// 4000 output queue length
	// 4001-4020 output queue
	// answers:
	//  80XX finished playing sound XX
	//  A0XX short contact on coin slot XX (coin error)
	//  A1XX inserted coin in slot XX
	// 4021 input queue length
	// 4022-4041 input queue
	// commands:
	//  80XX play sound XX
	//  81XX stop sound XX
	//  82XX stop all voices associated with timer A/B/both where XX = 01/02/03
	//  84XX play sound XX if it isn't already playing
	//  90XX reset
	//  A0XX
	// rest unused
ADDRESS_MAP_START( t5182_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_ROM	// internal ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM	AM_MIRROR(0x1800) // internal RAM
	AM_RANGE(0x4000, 0x40ff) AM_RAM AM_MIRROR(0x3F00) AM_BASE_LEGACY(&t5182_sharedram) // 2016 with four 74ls245s, one each for main and t5182 address and data. pins 23, 22, 20, 19, 18 are all tied low so only 256 bytes are usable
	AM_RANGE(0x8000, 0xffff) AM_ROM	// external ROM
ADDRESS_MAP_END


	// 00  W YM2151 address
	// 01 RW YM2151 data
	// 10  W semaphore for shared RAM: set as in use
	// 11  W semaphore for shared RAM: set as not in use
	// 12  W clear IRQ from YM2151
	// 13  W clear IRQ from main CPU
	// 20 R  flags bit 0 = main CPU is accessing shared RAM????  bit 1 = main CPU generated IRQ
	// 30 R  coin inputs (bits 0 and 1, active high)
	// 40  W external ROM banking? (the only 0 bit enables a ROM)
	// 50  W test mode status flags (bit 0 = ROM test fail, bit 1 = RAM test fail, bit 2 = YM2151 IRQ not received)
ADDRESS_MAP_START( t5182_io, AS_IO, 8, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x10, 0x10) AM_WRITE_LEGACY(t5182_sharedram_semaphore_snd_acquire_w)
	AM_RANGE(0x11, 0x11) AM_WRITE_LEGACY(t5182_sharedram_semaphore_snd_release_w)
	AM_RANGE(0x12, 0x12) AM_WRITE_LEGACY(t5182_ym2151_irq_ack_w)
	AM_RANGE(0x13, 0x13) AM_WRITE_LEGACY(t5182_cpu_irq_ack_w)
	AM_RANGE(0x20, 0x20) AM_READ_LEGACY(t5182_sharedram_semaphore_main_r)
	AM_RANGE(0x30, 0x30) AM_READ_PORT(T5182COINPORT)
ADDRESS_MAP_END
