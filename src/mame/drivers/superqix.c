/***************************************************************************

Prebillian        (c) 1986 Taito
Hot Smash         (c) 1987 Taito
Super Qix         (c) 1987 Taito
Perestroika Girls (c) 1994 Promat (hack of Super Qix)

driver by Mirko Buffoni, Nicola Salmoria, Tomasz Slanina

Super Qix is a later revision of the hardware, featuring a bitmap layer that
is not present in the earlier games. It also has two 8910, while the earlier
games have one 8910 + a sample player.

Notes:
- The 8751 ROM, sq07.108, was bad: bit 3 was stuck high. I have recovered it
  by carefully checking the disassembly. There are probably some mistakes.
  Note that that ROM probably came from a bootleg in the first place.

- The MCU sends some ID to the Z80 on startup, but the Z80 happily ignores it.
  This happens in all sets. There appears to be code that would check part of
  the MCU init sequence ($5973 onwards), but it doesn't seem to be called.

- sqixa might be an earlier version because there is a bug with coin lockout:
  it is activated after inesrting 10 coins instead of 9. sqix doesn't have
  that bug and also inverts the coin lockout output.

- Note that the sqixa ROMs probably came from a bootleg board, where the 8751
  MCU was replaced by a model using external ROM (which was bad, and manually
  repaired). Even if it was a bootleg, at this point there is no reason to
  believe the ROMs had been tampered with.

- sqixbl is a bootleg of sqixa, with the MCU removed.

- Prebillian controls: (from the Japanese flyer):
  - pullout plunger for shot power (there's no on-screen power indicator in the game)
  - dial for aiming
  - button (fire M powerup, high score initials)
  They are mapped a bit differently in MAME. BUTTON1 simlates pulling out the
  plunger and releasing it. The plunger strength is controlled by an analog input
  (by default mapped to up/down arrows) and shown on screen. The dial is mapped
  as expected. The button is mapped on BUTTON2. BUTTON3 is also recognized by the
  game when entering initials, but was probably not present in the cabinet.


TODO:
- The way we generate NMI in sqix doesn't make much sense, but is a workaround
  for the slow gameplay you would otherwise get. Some interaction with vblank?

- I'm not sure about the NMI ack at 0418 in the original sqix, but the game hangs
  at the end of a game without it. Note that the bootleg replaces that call with
  something else.


Prebillian :
------------

PCB Layout (Prebillian, from The Guru ( http://unemulated.emuunlim.com/ )

 M6100211A
 -------------------------------------------------------------------
 |                    HM50464                                       |
 |  6                 HM50464                                       |
 |  5                 HM50464                               6116    |
 |  4                 HM50464                                       |
 |                                                                  |
 |                                                                  |
 |                                                               J  |
 |                                            68705P5 SW1(8)        |
 |               6264                                            A  |
 |                                              3     SW2(8)        |
 |                                                               M  |
 |                                                                  |
 |                                                               M  |
 |                                                                  |
 |                                   2                           A  |
 |                                                                  |
 |                                   1                              |
 |                                                                  |
 |                                   Z80B            AY-3-8910      |
 | 12MHz                                                            |
 --------------------------------------------------------------------

Notes:
       Vertical Sync: 60Hz
         Horiz. Sync: 15.67kHz
         Z80B Clock : 5.995MHz
     AY-3-8910 Clock: 1.499MHz



Hot (Vs) Smash :
----------------

Dips (not verified):

DSW1 stored @ $f236
76------ coin a
--54---- coin b
----3--- stored @ $f295 , tested @ $2a3b
------1- code @ $03ed, stored @ $f253 (flip screen)

DSW2 stored @ $f237
---4---- code @ $03b4, stored @ $f290
----32-- code @ $03d8, stored @ $f293 (3600/5400/2400/1200  -> bonus  ?)
------10 code @ $03be, stored @ $f291/92 (8,8/0,12/16,6/24,4 -> difficulty ? )

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "sound/samples.h"


extern UINT8 *superqix_videoram;
extern UINT8 *superqix_bitmapram,*superqix_bitmapram2;
extern int pbillian_show_power;


WRITE8_HANDLER( superqix_videoram_w );
WRITE8_HANDLER( superqix_bitmapram_w );
WRITE8_HANDLER( superqix_bitmapram2_w );
WRITE8_HANDLER( pbillian_0410_w );
WRITE8_HANDLER( superqix_0410_w );

VIDEO_START( pbillian );
VIDEO_UPDATE( pbillian );
VIDEO_START( superqix );
VIDEO_UPDATE( superqix );


/* pbillian sample playback */
static INT16 *samplebuf;

static void pbillian_sh_start(void)
{
	UINT8 *src = memory_region(REGION_SOUND1);
	int i;

	/* convert 8-bit unsigned samples to 8-bit signed */
	samplebuf = auto_malloc(memory_region_length(REGION_SOUND1) * 2);
	for (i = 0;i < memory_region_length(REGION_SOUND1);i++)
		samplebuf[i] = (INT8)(src[i] ^ 0x80) * 256;
}

static WRITE8_HANDLER( pbillian_sample_trigger_w )
{
	int start,end;

	start = data << 7;
	/* look for end of sample marker */
	end = start;
	while (end < 0x8000 && samplebuf[end] != (0xff^0x80))
		end++;

	sample_start_raw(0, samplebuf + start, end - start, 5000, 0); // 5khz ?
}



/**************************************************************************

Z80 <-> 8751 communication

This is quite hackish, because the communication protocol is not very clear.
Add to that that we are not sure the 8751 is behaving 100% correctly because
the ROM was bad...

The Z80 acts this way:
- wait for 8910 #0 port B, bit 6 to be 0
- write command for MCU to 8910 #1 port B
- read port 0408
- wait for 8910 #0 port B, bit 6 to be 1
- read answer from MCU from 8910 #1 port B
- read port 0408

also, in other places it waits for 8910 #0 port B, bit 7 to be 0

The MCU acts this way:
- write FF to latch
- fiddle with port 1
- wait for IN2 bit 7 to be 1
- read command from latch
- process command
- fiddle with port 1
- write answer to latch
- wait for IN2 bit 7 to be 1

**************************************************************************/

static UINT8 port1, port3, port3_latch, from_mcu, from_z80, portb;
static int from_mcu_pending, from_z80_pending, invert_coin_lockout;

static READ8_HANDLER( in4_mcu_r )
{
//  logerror("%04x: in4_mcu_r\n",activecpu_get_pc());
	return input_port_read_indexed(machine, 4) | (from_mcu_pending << 6) | (from_z80_pending << 7);
}

static READ8_HANDLER( sqix_from_mcu_r )
{
//  logerror("%04x: read mcu answer (%02x)\n",activecpu_get_pc(),from_mcu);
	return from_mcu;
}

static TIMER_CALLBACK( mcu_acknowledge_callback )
{
	from_z80_pending = 1;
	from_z80 = portb;
//  logerror("Z80->MCU %02x\n",from_z80);
}

static READ8_HANDLER( mcu_acknowledge_r )
{
	timer_call_after_resynch(NULL, 0, mcu_acknowledge_callback);
	return 0;
}

static WRITE8_HANDLER( sqix_z80_mcu_w )
{
//  logerror("%04x: sqix_z80_mcu_w %02x\n",activecpu_get_pc(),data);
	portb = data;
}

static WRITE8_HANDLER( mcu_p1_w )
{
	switch ((data & 0x0e) >> 1)
	{
		case 0:
			// ???
			break;
		case 1:
			coin_counter_w(0,data & 1);
			break;
		case 2:
			coin_counter_w(1,data & 1);
			break;
		case 3:
			coin_lockout_global_w((data & 1) ^ invert_coin_lockout);
			break;
		case 4:
			flip_screen_set(data & 1);
			break;
		case 5:
			port1 = data;
			if ((port1 & 0x80) == 0)
			{
				port3_latch = port3;
			}
			break;
		case 6:
			from_mcu_pending = 0;	// ????
			break;
		case 7:
			if ((data & 1) == 0)
			{
//              logerror("%04x: MCU -> Z80 %02x\n",activecpu_get_pc(),port3);
				from_mcu = port3_latch;
				from_mcu_pending = 1;
				from_z80_pending = 0;	// ????
			}
			break;
	}
}

static READ8_HANDLER( mcu_p3_r )
{
	if ((port1 & 0x10) == 0)
	{
		return input_port_read_indexed(machine, 0);
	}
	else if ((port1 & 0x20) == 0)
	{
		return input_port_read_indexed(machine, 2) | (from_mcu_pending << 6) | (from_z80_pending << 7);
	}
	else if ((port1 & 0x40) == 0)
	{
//      logerror("%04x: read Z80 command %02x\n",activecpu_get_pc(),from_z80);
		from_z80_pending = 0;
		return from_z80;
	}
	return 0;
}

static WRITE8_HANDLER( mcu_p3_w )
{
	port3 = data;
}


static READ8_HANDLER( nmi_ack_r )
{
	cpunum_set_input_line(Machine, 0, INPUT_LINE_NMI, CLEAR_LINE);
	return 0;
}

static READ8_HANDLER( bootleg_in0_r )
{
	return BITSWAP8(input_port_read_indexed(machine, 0), 0,1,2,3,4,5,6,7);
}

static WRITE8_HANDLER( bootleg_flipscreen_w )
{
	flip_screen_set(~data & 1);
}


/***************************************************************************

 Hot Smash 68705 protection interface

***************************************************************************/

/*
 * This wrapper routine is necessary because the dial is not connected to an
 * hardware counter as usual, but the DIR and CLOCK inputs are directly
 * connected to the 68705 which acts as a counter.
 */

static int read_dial(running_machine *machine, int player)
{
	int newpos;
	static int oldpos[2];
	static int sign[2];

	/* get the new position and adjust the result */
	newpos = input_port_read_indexed(machine, 3 + player);
	if (newpos != oldpos[player])
	{
		sign[player] = ((newpos - oldpos[player]) & 0x80) >> 7;
		oldpos[player] = newpos;
	}

	if (player == 0)
		return ((oldpos[player] & 1) << 2) | (sign[player] << 3);
	else	// player == 1
		return ((oldpos[player] & 1) << 3) | (sign[player] << 2);
}



static TIMER_CALLBACK( delayed_z80_mcu_w )
{
logerror("Z80 sends command %02x\n",param);
	from_z80 = param;
	from_mcu_pending = 0;
	cpunum_set_input_line(machine, 1, 0, HOLD_LINE);
	cpu_boost_interleave(attotime_zero, ATTOTIME_IN_USEC(200));
}

static TIMER_CALLBACK( delayed_mcu_z80_w )
{
logerror("68705 sends answer %02x\n",param);
	from_mcu = param;
	from_mcu_pending = 1;
}


/*
 *  Port C connections:
 *  (all lines active low)
 *
 *  0-2 W  select I/O; inputs are read from port A, outputs are written to port B
 *         000  dsw A (I)
 *         001  dsw B (I)
 *         010  not used
 *         011  from Z80 (I)
 *         100  not used
 *         101  to Z80 (O)
 *         110  P1 dial input (I)
 *         111  P2 dial input (I)
 *  3   W  clocks the active latch
 *  4-7 W  not used
 */

static UINT8 portA_in, portB_out, portC;

static READ8_HANDLER( hotsmash_68705_portA_r )
{
//logerror("%04x: 68705 reads port A = %02x\n",activecpu_get_pc(),portA_in);
	return portA_in;
}

static WRITE8_HANDLER( hotsmash_68705_portB_w )
{
	portB_out = data;
}

static READ8_HANDLER( hotsmash_68705_portC_r )
{
	return portC;
}

static WRITE8_HANDLER( hotsmash_68705_portC_w )
{
	portC = data;

	if ((data & 0x08) == 0)
	{
		switch (data & 0x07)
		{
			case 0x0:	// dsw A
				portA_in = input_port_read_indexed(machine, 0);
				break;

			case 0x1:	// dsw B
				portA_in = input_port_read_indexed(machine, 1);
				break;

			case 0x2:
				break;

			case 0x3:	// command from Z80
				portA_in = from_z80;
logerror("%04x: z80 reads command %02x\n",activecpu_get_pc(),from_z80);
				break;

			case 0x4:
				break;

			case 0x5:	// answer to Z80
				timer_call_after_resynch(NULL, portB_out, delayed_mcu_z80_w);
				break;

			case 0x6:
				portA_in = read_dial(machine, 0);
				break;

			case 0x7:
				portA_in = read_dial(machine, 1);
				break;
		}
	}
}

static WRITE8_HANDLER( hotsmash_z80_mcu_w )
{
	timer_call_after_resynch(NULL, data, delayed_z80_mcu_w);
}

static READ8_HANDLER(hotsmash_from_mcu_r)
{
logerror("%04x: z80 reads answer %02x\n",activecpu_get_pc(),from_mcu);
	from_mcu_pending = 0;
	return from_mcu;
}

static READ8_HANDLER(hotsmash_ay_port_a_r)
{
//logerror("%04x: ay_port_a_r and mcu_pending is %d\n",activecpu_get_pc(),from_mcu_pending);
	return input_port_read_indexed(machine, 2) | ((from_mcu_pending^1) << 7);
}

/**************************************************************************

pbillian MCU simulation

**************************************************************************/

static WRITE8_HANDLER( pbillian_z80_mcu_w )
{
	from_z80 = data;
}

static READ8_HANDLER(pbillian_from_mcu_r)
{
	static int curr_player;

	switch (from_z80)
	{
		case 0x01: return input_port_read_indexed(machine, 4 + 2 * curr_player);
		case 0x02: return input_port_read_indexed(machine, 5 + 2 * curr_player);
		case 0x04: return input_port_read_indexed(machine, 0);
		case 0x08: return input_port_read_indexed(machine, 1);
		case 0x80: curr_player = 0; return 0;
		case 0x81: curr_player = 1; return 0;
	}

	logerror("408[%x] r at %x\n",from_z80,activecpu_get_pc());
	return 0;
}

static READ8_HANDLER(pbillian_ay_port_a_r)
{
//  logerror("%04x: ay_port_a_r\n",activecpu_get_pc());
	 /* bits 76------  MCU status bits */
	return (mame_rand(Machine)&0xc0)|input_port_read_indexed(machine, 3);
}


static void machine_init_common(void)
{
	state_save_register_global(invert_coin_lockout);
	state_save_register_global(from_mcu_pending);
	state_save_register_global(from_z80_pending);
	state_save_register_global(port1);
	state_save_register_global(port3);
	state_save_register_global(port3_latch);
	state_save_register_global(from_mcu);
	state_save_register_global(from_z80);
	state_save_register_global(portb);

	// hotsmash ???
	state_save_register_global(portA_in);
	state_save_register_global(portB_out);
	state_save_register_global(portC);
}

static MACHINE_START( superqix )
{
	/* configure the banks */
	memory_configure_bank(1, 0, 4, memory_region(REGION_CPU1) + 0x10000, 0x4000);

	machine_init_common();
}

static MACHINE_START( pbillian )
{
	/* configure the banks */
	memory_configure_bank(1, 0, 2, memory_region(REGION_CPU1) + 0x10000, 0x4000);

	machine_init_common();
}


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(1)
	AM_RANGE(0xe000, 0xe0ff) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xe100, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xefff) AM_READWRITE(SMH_RAM, superqix_videoram_w) AM_BASE(&superqix_videoram)
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pbillian_port_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_READWRITE(SMH_RAM, paletteram_BBGGRRII_w) AM_BASE(&paletteram)
	AM_RANGE(0x0401, 0x0401) AM_READ(AY8910_read_port_0_r)
	AM_RANGE(0x0402, 0x0402) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x0403, 0x0403) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x0408, 0x0408) AM_READ(pbillian_from_mcu_r)
	AM_RANGE(0x0408, 0x0408) AM_WRITE(pbillian_z80_mcu_w)
	AM_RANGE(0x0410, 0x0410) AM_WRITE(pbillian_0410_w)
	AM_RANGE(0x0418, 0x0418) AM_READ(SMH_NOP)  //?
	AM_RANGE(0x0419, 0x0419) AM_WRITE(SMH_NOP)  //? watchdog ?
	AM_RANGE(0x041a, 0x041a) AM_WRITE(pbillian_sample_trigger_w)
	AM_RANGE(0x041b, 0x041b) AM_READ(SMH_NOP)  // input related? but probably not used
ADDRESS_MAP_END

static ADDRESS_MAP_START( hotsmash_port_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_READWRITE(SMH_RAM, paletteram_BBGGRRII_w) AM_BASE(&paletteram)
	AM_RANGE(0x0401, 0x0401) AM_READ(AY8910_read_port_0_r)
	AM_RANGE(0x0402, 0x0402) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x0403, 0x0403) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x0408, 0x0408) AM_READ(hotsmash_from_mcu_r)
	AM_RANGE(0x0408, 0x0408) AM_WRITE(hotsmash_z80_mcu_w)
	AM_RANGE(0x0410, 0x0410) AM_WRITE(pbillian_0410_w)
	AM_RANGE(0x0418, 0x0418) AM_READ(SMH_NOP)  //?
	AM_RANGE(0x0419, 0x0419) AM_WRITE(SMH_NOP)  //? watchdog ?
	AM_RANGE(0x041a, 0x041a) AM_WRITE(pbillian_sample_trigger_w)
	AM_RANGE(0x041b, 0x041b) AM_READ(SMH_NOP)  // input related? but probably not used
ADDRESS_MAP_END

static ADDRESS_MAP_START( sqix_port_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x00ff) AM_READWRITE(SMH_RAM, paletteram_BBGGRRII_w) AM_BASE(&paletteram)
	AM_RANGE(0x0401, 0x0401) AM_READ(AY8910_read_port_0_r)
	AM_RANGE(0x0402, 0x0402) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x0403, 0x0403) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x0405, 0x0405) AM_READ(AY8910_read_port_1_r)
	AM_RANGE(0x0406, 0x0406) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0x0407, 0x0407) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x0408, 0x0408) AM_READ(mcu_acknowledge_r)
	AM_RANGE(0x0410, 0x0410) AM_WRITE(superqix_0410_w)	/* ROM bank, NMI enable, tile bank */
	AM_RANGE(0x0418, 0x0418) AM_READ(nmi_ack_r)
	AM_RANGE(0x0800, 0x77ff) AM_READWRITE(SMH_RAM, superqix_bitmapram_w) AM_BASE(&superqix_bitmapram)
	AM_RANGE(0x8800, 0xf7ff) AM_READWRITE(SMH_RAM, superqix_bitmapram2_w) AM_BASE(&superqix_bitmapram2)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bootleg_port_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x00ff) AM_READWRITE(SMH_RAM, paletteram_BBGGRRII_w) AM_BASE(&paletteram)
	AM_RANGE(0x0401, 0x0401) AM_READ(AY8910_read_port_0_r)
	AM_RANGE(0x0402, 0x0402) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x0403, 0x0403) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x0405, 0x0405) AM_READ(AY8910_read_port_1_r)
	AM_RANGE(0x0406, 0x0406) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0x0407, 0x0407) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x0408, 0x0408) AM_WRITE(bootleg_flipscreen_w)
	AM_RANGE(0x0410, 0x0410) AM_WRITE(superqix_0410_w)	/* ROM bank, NMI enable, tile bank */
	AM_RANGE(0x0418, 0x0418) AM_READ(input_port_2_r)
	AM_RANGE(0x0800, 0x77ff) AM_READWRITE(SMH_RAM, superqix_bitmapram_w) AM_BASE(&superqix_bitmapram)
	AM_RANGE(0x8800, 0xf7ff) AM_READWRITE(SMH_RAM, superqix_bitmapram2_w) AM_BASE(&superqix_bitmapram2)
ADDRESS_MAP_END


static ADDRESS_MAP_START( m68705_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READ(hotsmash_68705_portA_r)
	AM_RANGE(0x0001, 0x0001) AM_WRITE(hotsmash_68705_portB_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(hotsmash_68705_portC_r, hotsmash_68705_portC_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END


/* I8751 memory handlers */
static ADDRESS_MAP_START( mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM	AM_REGION(REGION_CPU2, 0) // AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x01, 0x01) AM_WRITE(mcu_p1_w)
	AM_RANGE(0x03, 0x03) AM_READWRITE(mcu_p3_r, mcu_p3_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( pbillian )
	PORT_START
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "10/20/300K Points" )
	PORT_DIPSETTING(    0x00, "10/30/500K Points" )
	PORT_DIPSETTING(    0x08, "20/30/400K Points" )
	PORT_DIPSETTING(    0x04, "30/40/500K Points" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )	// high score initials
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )	// fire (M powerup) + high score initials
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL	// high score initials
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL	// fire (M powerup) + high score initials
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )	// mcu status (pending mcu->z80)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	// mcu status (pending z80->mcu)

	PORT_START
	PORT_BIT( 0x3f, 0x00, IPT_PADDLE_V  ) PORT_MINMAX(0,0x3f) PORT_SENSITIVITY(30) PORT_KEYDELTA(3) PORT_CENTERDELTA(0) PORT_REVERSE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10)

	PORT_START
	PORT_BIT( 0x3f, 0x00, IPT_PADDLE_V  ) PORT_MINMAX(0,0x3f) PORT_SENSITIVITY(30) PORT_KEYDELTA(3) PORT_CENTERDELTA(0) PORT_REVERSE  PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( hotsmash )
	PORT_START
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START
	PORT_DIPNAME( 0x03, 0x03, "Difficulty vs. CPU" )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty vs. 2P" )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, "Points per game" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )//$49c
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )//$42d
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	// mcu status (0 = pending mcu->z80)

	PORT_START
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(30) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(30) PORT_CENTERDELTA(0) PORT_PLAYER(2)

INPUT_PORTS_END


static INPUT_PORTS_START( superqix )
	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Freeze" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START	/* DSW2 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "20000 50000" )
	PORT_DIPSETTING(    0x0c, "30000 100000" )
	PORT_DIPSETTING(    0x04, "50000 100000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, "Fill Area" )
	PORT_DIPSETTING(    0x80, "70%" )
	PORT_DIPSETTING(    0xc0, "75%" )
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x00, "85%" )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )	// doesn't work in bootleg
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	// Z80 status (pending z80->mcu)

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_VBLANK )	/* ??? */
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )	// mcu status (pending mcu->z80)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	// mcu status (pending z80->mcu)
INPUT_PORTS_END



static const gfx_layout pbillian_charlayout =
{
	8,8,
	0x800,	/* doesn't use the whole ROM space */
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout sqix_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8
};


static GFXDECODE_START( pbillian )
	GFXDECODE_ENTRY( REGION_GFX1, 0, pbillian_charlayout, 16*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX1, 0, spritelayout,            0, 16 )
GFXDECODE_END

static GFXDECODE_START( sqix )
	GFXDECODE_ENTRY( REGION_GFX1, 0x00000, sqix_charlayout,   0, 16 )	/* Chars */
	GFXDECODE_ENTRY( REGION_GFX2, 0x00000, sqix_charlayout,   0, 16 )	/* Background tiles */
	GFXDECODE_ENTRY( REGION_GFX3, 0x00000, spritelayout,      0, 16 )	/* Sprites */
GFXDECODE_END



static const struct Samplesinterface pbillian_samples_interface =
{
	1,
	NULL,
	pbillian_sh_start
};

static const struct AY8910interface pbillian_ay8910_interface =
{
	pbillian_ay_port_a_r,			/* port Aread */
	input_port_2_r					/* port Bread */
};

static const struct AY8910interface hotsmash_ay8910_interface =
{
	hotsmash_ay_port_a_r,			/* port Aread */
	input_port_2_r					/* port Bread */
};

static const struct AY8910interface sqix_ay8910_interface_1 =
{
	input_port_3_r,	/* port Aread */
	in4_mcu_r,		/* port Bread */
};

static const struct AY8910interface sqix_ay8910_interface_2 =
{
	input_port_1_r,	/* port Aread */
	sqix_from_mcu_r,		/* port Bread */
	0,								/* port Awrite */
	sqix_z80_mcu_w				/* port Bwrite */
};

static const struct AY8910interface bootleg_ay8910_interface_1 =
{
	input_port_3_r,	/* port Aread */
	input_port_4_r	/* port Bread */
};

static const struct AY8910interface bootleg_ay8910_interface_2 =
{
	input_port_1_r,	/* port Aread */
	bootleg_in0_r	/* port Bread */
};



static INTERRUPT_GEN( sqix_interrupt )
{
	/* highly suspicious... */
	if (cpu_getiloops() <= 3)
		nmi_line_assert(machine, cpunum);
}

static INTERRUPT_GEN( bootleg_interrupt )
{
	/* highly suspicious... */
	if (cpu_getiloops() <= 3)
		nmi_line_pulse(machine, cpunum);
}



static MACHINE_DRIVER_START( pbillian )
	MDRV_CPU_ADD(Z80,12000000/2)		 /* 6 MHz */
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_IO_MAP(pbillian_port_map,0)
	MDRV_CPU_VBLANK_INT("main", nmi_line_pulse)

	MDRV_MACHINE_START(pbillian)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(pbillian)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(pbillian)
	MDRV_VIDEO_UPDATE(pbillian)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 12000000/8)
	MDRV_SOUND_CONFIG(pbillian_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(pbillian_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( hotsmash )
	MDRV_CPU_ADD(Z80,12000000/2)		 /* 6 MHz */
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_IO_MAP(hotsmash_port_map,0)
	MDRV_CPU_VBLANK_INT("main", nmi_line_pulse)

	MDRV_CPU_ADD(M68705, 4000000) /* ???? */
	MDRV_CPU_PROGRAM_MAP(m68705_map,0)

	MDRV_MACHINE_START(pbillian)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(pbillian)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(pbillian)
	MDRV_VIDEO_UPDATE(pbillian)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 12000000/8)
	MDRV_SOUND_CONFIG(hotsmash_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(pbillian_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( sqix )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 12000000/2)	/* 6 MHz */
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_IO_MAP(sqix_port_map,0)
	MDRV_CPU_VBLANK_INT_HACK(sqix_interrupt,3)	/* ??? */

	MDRV_CPU_ADD(I8751, 12000000/3)	/* ??? */
	MDRV_CPU_PROGRAM_MAP(mcu_map,0)
	MDRV_CPU_IO_MAP(mcu_io_map,0)

	MDRV_INTERLEAVE(500)

	MDRV_MACHINE_START(superqix)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(sqix)
	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(superqix)
	MDRV_VIDEO_UPDATE(superqix)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 12000000/8)
	MDRV_SOUND_CONFIG(sqix_ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, 12000000/8)
	MDRV_SOUND_CONFIG(sqix_ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( sqixbl )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 12000000/2)	/* 6 MHz */
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_IO_MAP(bootleg_port_map,0)
	MDRV_CPU_VBLANK_INT_HACK(bootleg_interrupt,3)	/* ??? */

	MDRV_MACHINE_START(superqix)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(sqix)
	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(superqix)
	MDRV_VIDEO_UPDATE(superqix)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 12000000/8)
	MDRV_SOUND_CONFIG(bootleg_ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, 12000000/8)
	MDRV_SOUND_CONFIG(bootleg_ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pbillian )
	ROM_REGION( 0x018000, REGION_CPU1, 0 )
	ROM_LOAD( "1.6c",  0x00000, 0x08000, CRC(d379fe23) SHA1(e147a9151b1cdeacb126d9713687bd0aa92980ac) )
	ROM_LOAD( "2.6d",  0x14000, 0x04000, CRC(1af522bc) SHA1(83e002dc831bfcedbd7096b350c9b34418b79674) )

	ROM_REGION( 0x0800, REGION_CPU2, 0 )
	ROM_LOAD( "pbillian.mcu", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x8000, REGION_SOUND1, 0 )
	ROM_LOAD( "3.7j",  0x0000, 0x08000, CRC(3f9bc7f1) SHA1(0b0c2ec3bea6a7f3fc6c0c8b750318f3f9ec3d1f) )

	ROM_REGION( 0x018000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "4.1n",  0x00000, 0x08000, CRC(9c08a072) SHA1(25f31fcf72216cf42528b07ad8c09113aa69861a) )
	ROM_LOAD( "5.1r",  0x08000, 0x08000, CRC(2dd5b83f) SHA1(b05e3a008050359d0207757b9cbd8cee87abc697) )
	ROM_LOAD( "6.1t",  0x10000, 0x08000, CRC(33b855b0) SHA1(5a1df4f82fc0d6f78883b759fd61f395942645eb) )
ROM_END

ROM_START( hotsmash )
	ROM_REGION( 0x018000, REGION_CPU1, 0 )
	ROM_LOAD( "b18-04",  0x00000, 0x08000, CRC(981bde2c) SHA1(ebcc901a036cde16b33d534d423500d74523b781) )

	ROM_REGION( 0x0800, REGION_CPU2, 0 )
	ROM_LOAD( "b18-06.mcu", 0x0000, 0x0800, CRC(67c0920a) SHA1(23a294892823d1d9216ea8ddfa9df1c8af149477) )

	ROM_REGION( 0x8000, REGION_SOUND1, 0 )
	ROM_LOAD( "b18-05",  0x0000, 0x08000, CRC(dab5e718) SHA1(6cf6486f283f5177dfdc657b1627fbfa3f0743e8) )

	ROM_REGION( 0x018000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "b18-01",  0x00000, 0x08000, CRC(870a4c04) SHA1(a029108bcda40755c8320d2ee297f42d816aa7c0) )
	ROM_LOAD( "b18-02",  0x08000, 0x08000, CRC(4e625cac) SHA1(2c21b32240eaada9a5f909a2ec5b335372c8c994) )
	ROM_LOAD( "b18-03",  0x14000, 0x04000, CRC(1c82717d) SHA1(6942c8877e24ac51ed71036e771a1655d82f3491) )
ROM_END

ROM_START( sqix )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )
	ROM_LOAD( "b03-01-1",     0x00000, 0x08000, CRC(ad614117) SHA1(c461f00a2aecde1bc3860c15a3c31091b14665a2) )
	ROM_LOAD( "b03-02",       0x10000, 0x10000, CRC(9c23cb64) SHA1(7e04cb18cabdc0031621162cbc228cd95875a022) )

	ROM_REGION( 0x1000, REGION_CPU2, 0 )	/* I8751 code */
	ROM_LOAD( "sq07.108",     0x00000, 0x1000, BAD_DUMP CRC(8be4d2a8) SHA1(a0c72cd87b2cddc67070b0a533fca111dbb9a984) )	// from sqixa

	ROM_REGION( 0x08000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "b03-04",       0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "b03-03",       0x00000, 0x10000, CRC(6e8b6a67) SHA1(c71117cc880a124c46397c446d1edc1cbf681200) )
	ROM_LOAD( "b03-06",       0x10000, 0x10000, CRC(38154517) SHA1(703ad4cfe54a4786c67aedcca5998b57f39fd857) )

	ROM_REGION( 0x10000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "b03-05",       0x00000, 0x10000, CRC(df326540) SHA1(1fe025edcd38202e24c4e1005f478b6a88533453) )
ROM_END

ROM_START( sqixu )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )
	ROM_LOAD( "b03-06.f3",     0x00000, 0x08000, CRC(4f59f7af) SHA1(6ea627ea8505cf8d1a5a1350258180c61fbd1ed9) )
	ROM_LOAD( "b03-07.h3",     0x10000, 0x10000, CRC(4c417d4a) SHA1(de46551da1b27312dca40240a210e77595cf9dbd) )

	ROM_REGION( 0x1000, REGION_CPU2, 0 )	/* I8751 code */
	ROM_LOAD( "b03-08",     0x00000, 0x1000, BAD_DUMP CRC(8be4d2a8) SHA1(a0c72cd87b2cddc67070b0a533fca111dbb9a984) )	// from sqixa

	ROM_REGION( 0x08000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "b03-04",       0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "b03-03",       0x00000, 0x10000, CRC(6e8b6a67) SHA1(c71117cc880a124c46397c446d1edc1cbf681200) )
	ROM_LOAD( "b03-06",       0x10000, 0x10000, CRC(38154517) SHA1(703ad4cfe54a4786c67aedcca5998b57f39fd857) )

	ROM_REGION( 0x10000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "b03-09.t8",    0x00000, 0x10000, CRC(69d2a84a) SHA1(b461d8a01f73c6aaa4aac85602c688c111bdca5d) )
ROM_END

/* this was probably a bootleg */
ROM_START( sqixa )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )
	ROM_LOAD( "sq01.97",      0x00000, 0x08000, CRC(0888b7de) SHA1(de3e4637436de185f43d2ad4186d4cfdcd4d33d9) )
	ROM_LOAD( "b03-02",       0x10000, 0x10000, CRC(9c23cb64) SHA1(7e04cb18cabdc0031621162cbc228cd95875a022) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* I8751 code */
	ROM_LOAD( "sq07.108",     0x00000, 0x1000, BAD_DUMP CRC(8be4d2a8) SHA1(a0c72cd87b2cddc67070b0a533fca111dbb9a984) )

	ROM_REGION( 0x08000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "b03-04",       0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "b03-03",       0x00000, 0x10000, CRC(6e8b6a67) SHA1(c71117cc880a124c46397c446d1edc1cbf681200) )
	ROM_LOAD( "b03-06",       0x10000, 0x10000, CRC(38154517) SHA1(703ad4cfe54a4786c67aedcca5998b57f39fd857) )

	ROM_REGION( 0x10000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "b03-05",       0x00000, 0x10000, CRC(df326540) SHA1(1fe025edcd38202e24c4e1005f478b6a88533453) )
ROM_END

ROM_START( sqixbl )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )
	ROM_LOAD( "cpu.2",        0x00000, 0x08000, CRC(682e28e3) SHA1(fe9221d26d7397be5a0fc8fdc51672b5924f3cf2) )
	ROM_LOAD( "b03-02",       0x10000, 0x10000, CRC(9c23cb64) SHA1(7e04cb18cabdc0031621162cbc228cd95875a022) )

	ROM_REGION( 0x08000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "b03-04",       0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "b03-03",       0x00000, 0x10000, CRC(6e8b6a67) SHA1(c71117cc880a124c46397c446d1edc1cbf681200) )
	ROM_LOAD( "b03-06",       0x10000, 0x10000, CRC(38154517) SHA1(703ad4cfe54a4786c67aedcca5998b57f39fd857) )

	ROM_REGION( 0x10000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "b03-05",       0x00000, 0x10000, CRC(df326540) SHA1(1fe025edcd38202e24c4e1005f478b6a88533453) )
ROM_END

ROM_START( perestrf )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )
	/* 0x8000 - 0x10000 in the rom is empty anyway */
	ROM_LOAD( "rom1.bin",        0x00000, 0x20000, CRC(0cbf96c1) SHA1(cf2b1367887d1b8812a56aa55593e742578f220c) )

	ROM_REGION( 0x10000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "rom4.bin",       0x00000, 0x10000, CRC(c56122a8) SHA1(1d24b2f0358e14aca5681f92175869224584a6ea) ) /* both halves identical */

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "rom2.bin",       0x00000, 0x20000, CRC(36f93701) SHA1(452cb23efd955c6c155cef2b1b650e253e195738) )

	ROM_REGION( 0x10000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "rom3.bin",       0x00000, 0x10000, CRC(00c91d5a) SHA1(fdde56d3689a47e6bfb296e442207b93b887ec7a) )
ROM_END

ROM_START( perestro )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )
	/* 0x8000 - 0x10000 in the rom is empty anyway */
	ROM_LOAD( "rom1.bin",        0x00000, 0x20000, CRC(0cbf96c1) SHA1(cf2b1367887d1b8812a56aa55593e742578f220c) )

	ROM_REGION( 0x10000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "rom4.bin",       0x00000, 0x10000, CRC(c56122a8) SHA1(1d24b2f0358e14aca5681f92175869224584a6ea) ) /* both halves identical */

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "rom2.bin",       0x00000, 0x20000, CRC(36f93701) SHA1(452cb23efd955c6c155cef2b1b650e253e195738) )

	ROM_REGION( 0x10000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "rom3a.bin",       0x00000, 0x10000, CRC(7a2a563f) SHA1(e3654091b858cc80ec1991281447fc3622a0d4f9) )
ROM_END



static DRIVER_INIT( pbillian )
{
	pbillian_show_power = 1;
}

static DRIVER_INIT( hotsmash )
{
	pbillian_show_power = 0;
}

static DRIVER_INIT( sqix )
{
	invert_coin_lockout = 1;
}

static DRIVER_INIT( sqixa )
{
	invert_coin_lockout = 0;
}

static DRIVER_INIT( perestro )
{
	UINT8 *src;
	int len;
	UINT8 temp[16];
	int i,j;

	/* decrypt program code; the address lines are shuffled around in a non-trivial way */
	src = memory_region(REGION_CPU1);
	len = memory_region_length(REGION_CPU1);
	for (i = 0;i < len;i += 16)
	{
		memcpy(temp,&src[i],16);
		for (j = 0;j < 16;j++)
		{
			static const int convtable[16] =
			{
				0xc, 0x9, 0xb, 0xa,
				0x8, 0xd, 0xf, 0xe,
				0x4, 0x1, 0x3, 0x2,
				0x0, 0x5, 0x7, 0x6
			};

			src[i+j] = temp[convtable[j]];
		}
	}

	/* decrypt gfx ROMs; simple bit swap on the address lines */
	src = memory_region(REGION_GFX1);
	len = memory_region_length(REGION_GFX1);
	for (i = 0;i < len;i += 16)
	{
		memcpy(temp,&src[i],16);
		for (j = 0;j < 16;j++)
		{
			src[i+j] = temp[BITSWAP8(j,7,6,5,4,3,2,0,1)];
		}
	}

	src = memory_region(REGION_GFX2);
	len = memory_region_length(REGION_GFX2);
	for (i = 0;i < len;i += 16)
	{
		memcpy(temp,&src[i],16);
		for (j = 0;j < 16;j++)
		{
			src[i+j] = temp[BITSWAP8(j,7,6,5,4,0,1,2,3)];
		}
	}

	src = memory_region(REGION_GFX3);
	len = memory_region_length(REGION_GFX3);
	for (i = 0;i < len;i += 16)
	{
		memcpy(temp,&src[i],16);
		for (j = 0;j < 16;j++)
		{
			src[i+j] = temp[BITSWAP8(j,7,6,5,4,1,0,3,2)];
		}
	}
}



GAME( 1986, pbillian, 0,        pbillian, pbillian, pbillian, ROT0,  "Taito", "Prebillian", GAME_SUPPORTS_SAVE )
GAME( 1987, hotsmash, 0,        hotsmash, hotsmash, hotsmash, ROT90, "Taito", "Vs. Hot Smash", GAME_SUPPORTS_SAVE )
GAME( 1987, sqix,     0,        sqix,     superqix, sqix,     ROT90, "Taito", "Super Qix (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1987, sqixa,    sqix,     sqix,     superqix, sqixa,    ROT90, "Taito", "Super Qix (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1987, sqixu,    sqix,     sqix,     superqix, sqix,     ROT90, "Taito (Romstar License)", "Super Qix (US)", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING ) // different MCU?
GAME( 1987, sqixbl,   sqix,     sqixbl,   superqix, 0,        ROT90, "bootleg", "Super Qix (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1994, perestro, 0,        sqixbl,   superqix, perestro, ROT90, "Promat", "Perestroika Girls", GAME_SUPPORTS_SAVE )
GAME( 1993, perestrf, perestro, sqixbl,   superqix, perestro, ROT90, "Promat (Fuuki license)", "Perestroika Girls (Fuuki license)", GAME_SUPPORTS_SAVE )
