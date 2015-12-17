// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni, Nicola Salmoria, Tomasz Slanina
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
- The sq07.108 ROM came from a bootleg where the 8751 MCU was replaced by a
  model using external ROM. The ROM was bad (bit 3 was stuck high). It was
  recovered by carefully checking the disassembly but there might still be
  some mistakes.
  The bootleg MCU code is different from the original; it was modified by the
  bootleggers to avoid use of port 2.

- The MCU sends some ID to the Z80 on startup, but the Z80 happily ignores it.
  This happens in all sets. There appears to be code that would check part of
  the MCU init sequence ($5973 onwards), but it doesn't seem to be called.

- sqixb1 might be an earlier version because there is a bug with coin lockout:
  it is activated after inesrting 10 coins instead of 9. sqix doesn't have
  that bug and also inverts the coin lockout output.

- sqixb2 is a bootleg of sqixb1, with the MCU removed.

- Prebillian controls: (from the Japanese flyer):
  - pullout plunger for shot power (there's no on-screen power indicator in the game)
  - dial for aiming
  - button (fire M powerup, high score initials)


TODO:
- The way we generate NMI in sqix doesn't make much sense, but is a workaround
  for the slow gameplay you would otherwise get. Some interaction with vblank?

- I'm not sure about the NMI ack at 0418 in the original sqix, but the game hangs
  at the end of a game without it. Note that the bootleg replaces that call with
  something else.


Prebillian :
------------

PCB Layout (Prebillian)

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

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "includes/superqix.h"


SAMPLES_START_CB_MEMBER(superqix_state::pbillian_sh_start)
{
	UINT8 *src = memregion("samples")->base();
	int i, len = memregion("samples")->bytes();

	/* convert 8-bit unsigned samples to 8-bit signed */
	m_samplebuf = std::make_unique<INT16[]>(len);
	for (i = 0;i < len;i++)
		m_samplebuf[i] = (INT8)(src[i] ^ 0x80) * 256;
}

WRITE8_MEMBER(superqix_state::pbillian_sample_trigger_w)
{
	UINT8 *src = memregion("samples")->base();
	int len = memregion("samples")->bytes();
	int start,end;

	start = data << 7;
	/* look for end of sample marker */
	end = start;
	while (end < len && src[end] != 0xff)
		end++;

	m_samples->start_raw(0, m_samplebuf.get() + start, end - start, 5000); // 5khz ?
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


READ8_MEMBER(superqix_state::in4_mcu_r)
{
//  logerror("%04x: in4_mcu_r\n",space.device().safe_pc());
	return ioport("P2")->read() | (m_from_mcu_pending << 6) | (m_from_z80_pending << 7);
}

READ8_MEMBER(superqix_state::sqix_from_mcu_r)
{
//  logerror("%04x: read mcu answer (%02x)\n",space.device().safe_pc(),m_from_mcu);
	return m_from_mcu;
}

TIMER_CALLBACK_MEMBER(superqix_state::mcu_acknowledge_callback)
{
	m_from_z80_pending = 1;
	m_from_z80 = m_portb;
//  logerror("Z80->MCU %02x\n",m_from_z80);
}

READ8_MEMBER(superqix_state::mcu_acknowledge_r)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(superqix_state::mcu_acknowledge_callback),this));
	return 0;
}

WRITE8_MEMBER(superqix_state::sqix_z80_mcu_w)
{
//  logerror("%04x: sqix_z80_mcu_w %02x\n",space.device().safe_pc(),data);
	m_portb = data;
}

WRITE8_MEMBER(superqix_state::bootleg_mcu_p1_w)
{
	switch ((data & 0x0e) >> 1)
	{
		case 0:
			// ???
			break;
		case 1:
			coin_counter_w(machine(), 0,data & 1);
			break;
		case 2:
			coin_counter_w(machine(), 1,data & 1);
			break;
		case 3:
			coin_lockout_global_w(machine(), (data & 1) ^ m_invert_coin_lockout);
			break;
		case 4:
			flip_screen_set(data & 1);
			break;
		case 5:
			m_port1 = data;
			if ((m_port1 & 0x80) == 0)
			{
				m_port3_latch = m_port3;
			}
			break;
		case 6:
			m_from_mcu_pending = 0; // ????
			break;
		case 7:
			if ((data & 1) == 0)
			{
//              logerror("%04x: MCU -> Z80 %02x\n",space.device().safe_pc(),m_port3);
				m_from_mcu = m_port3_latch;
				m_from_mcu_pending = 1;
				m_from_z80_pending = 0; // ????
			}
			break;
	}
}

WRITE8_MEMBER(superqix_state::mcu_p3_w)
{
	m_port3 = data;
}

READ8_MEMBER(superqix_state::bootleg_mcu_p3_r)
{
	if ((m_port1 & 0x10) == 0)
	{
		return ioport("DSW1")->read();
	}
	else if ((m_port1 & 0x20) == 0)
	{
		return sqix_system_status_r(space, 0);
	}
	else if ((m_port1 & 0x40) == 0)
	{
//      logerror("%04x: read Z80 command %02x\n",space.device().safe_pc(),m_from_z80);
		m_from_z80_pending = 0;
		return m_from_z80;
	}
	return 0;
}

READ8_MEMBER(superqix_state::sqix_system_status_r)
{
	return ioport("SYSTEM")->read() | (m_from_mcu_pending << 6) | (m_from_z80_pending << 7);
}

WRITE8_MEMBER(superqix_state::sqixu_mcu_p2_w)
{
	// bit 0 = unknown (clocked often)

	// bit 1 = coin cointer 1
	coin_counter_w(machine(), 0,data & 2);

	// bit 2 = coin counter 2
	coin_counter_w(machine(), 1,data & 4);

	// bit 3 = coin lockout
	coin_lockout_global_w(machine(), ~data & 8);

	// bit 4 = flip screen
	flip_screen_set(data & 0x10);

	// bit 5 = unknown (set on startup)

	// bit 6 = unknown
	if ((data & 0x40) == 0)
		m_from_mcu_pending = 0; // ????

	// bit 7 = clock latch from port 3 to Z80
	if ((m_port2 & 0x80) != 0 && (data & 0x80) == 0)
	{
//      logerror("%04x: MCU -> Z80 %02x\n",space.device().safe_pc(),m_port3);
		m_from_mcu = m_port3;
		m_from_mcu_pending = 1;
		m_from_z80_pending = 0; // ????
	}

	m_port2 = data;
}

READ8_MEMBER(superqix_state::sqixu_mcu_p3_r)
{
//  logerror("%04x: read Z80 command %02x\n",space.device().safe_pc(),m_from_z80);
	m_from_z80_pending = 0;
	return m_from_z80;
}


READ8_MEMBER(superqix_state::nmi_ack_r)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return sqix_system_status_r(space, 0);
}

READ8_MEMBER(superqix_state::bootleg_in0_r)
{
	return BITSWAP8(ioport("DSW1")->read(), 0,1,2,3,4,5,6,7);
}

WRITE8_MEMBER(superqix_state::bootleg_flipscreen_w)
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

int superqix_state::read_dial(int player)
{
	int newpos;

	/* get the new position and adjust the result */
	newpos = ioport(player ? "DIAL2" : "DIAL1")->read();
	if (newpos != m_oldpos[player])
	{
		m_sign[player] = ((newpos - m_oldpos[player]) & 0x80) >> 7;
		m_oldpos[player] = newpos;
	}

	if (player == 0)
		return ((m_oldpos[player] & 1) << 2) | (m_sign[player] << 3);
	else    // player == 1
		return ((m_oldpos[player] & 1) << 3) | (m_sign[player] << 2);
}



TIMER_CALLBACK_MEMBER(superqix_state::delayed_z80_mcu_w)
{
//  logerror("Z80 sends command %02x\n",param);
	m_from_z80 = param;
	m_from_mcu_pending = 0;
	m_mcu->set_input_line(0, HOLD_LINE);
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(200));
}

TIMER_CALLBACK_MEMBER(superqix_state::delayed_mcu_z80_w)
{
//  logerror("68705 sends answer %02x\n",param);
	m_from_mcu = param;
	m_from_mcu_pending = 1;
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


READ8_MEMBER(superqix_state::hotsmash_68705_portA_r)
{
//  logerror("%04x: 68705 reads port A = %02x\n",space.device().safe_pc(),m_portA_in);
	return m_portA_in;
}

WRITE8_MEMBER(superqix_state::hotsmash_68705_portB_w)
{
	m_portB_out = data;
}

READ8_MEMBER(superqix_state::hotsmash_68705_portC_r)
{
	return m_portC;
}

WRITE8_MEMBER(superqix_state::hotsmash_68705_portC_w)
{
	m_portC = data;

	if ((data & 0x08) == 0)
	{
		switch (data & 0x07)
		{
			case 0x0:   // dsw A
				m_portA_in = ioport("DSW1")->read();
				break;

			case 0x1:   // dsw B
				m_portA_in = ioport("DSW2")->read();
				break;

			case 0x2:
				break;

			case 0x3:   // command from Z80
				m_portA_in = m_from_z80;
//              logerror("%04x: z80 reads command %02x\n",space.device().safe_pc(),m_from_z80);
				break;

			case 0x4:
				break;

			case 0x5:   // answer to Z80
				machine().scheduler().synchronize(timer_expired_delegate(FUNC(superqix_state::delayed_mcu_z80_w),this), m_portB_out);
				break;

			case 0x6:
				m_portA_in = read_dial(0);
				break;

			case 0x7:
				m_portA_in = read_dial(1);
				break;
		}
	}
}

WRITE8_MEMBER(superqix_state::hotsmash_z80_mcu_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(superqix_state::delayed_z80_mcu_w),this), data);
}

READ8_MEMBER(superqix_state::hotsmash_from_mcu_r)
{
//  logerror("%04x: z80 reads answer %02x\n",space.device().safe_pc(),m_from_mcu);
	m_from_mcu_pending = 0;
	return m_from_mcu;
}

READ8_MEMBER(superqix_state::hotsmash_ay_port_a_r)
{
//  logerror("%04x: ay_port_a_r and mcu_pending is %d\n",space.device().safe_pc(),m_from_mcu_pending);
	return ioport("SYSTEM")->read() | 0x40 | ((m_from_mcu_pending^1) << 7);
}

/**************************************************************************

pbillian MCU simulation

**************************************************************************/

WRITE8_MEMBER(superqix_state::pbillian_z80_mcu_w)
{
	m_from_z80 = data;
}

READ8_MEMBER(superqix_state::pbillian_from_mcu_r)
{
	switch (m_from_z80)
	{
		case 0x01:
		{
			UINT8 p = ioport(m_curr_player ? "PLUNGER2" : "PLUNGER1")->read() & 0xbf;
			if ((p & 0x3f) == 0) p |= 0x40;
			return p;
		}

		case 0x02: return ioport(m_curr_player ? "DIAL2" : "DIAL1")->read();

		case 0x04: return ioport("DSW1")->read();
		case 0x08: return ioport("DSW2")->read();

		case 0x80: m_curr_player = 0; return 0;
		case 0x81: m_curr_player = 1; return 0;
	}

//  logerror("408[%x] r at %x\n",m_from_z80,space.device().safe_pc());
	return 0;
}

READ8_MEMBER(superqix_state::pbillian_ay_port_a_r)
{
//  logerror("%04x: ay_port_a_r\n",space.device().safe_pc());
	/* bits 76------  MCU status bits */
	return (machine().rand() & 0xc0) | ioport("BUTTONS")->read();
}


void superqix_state::machine_init_common()
{
	save_item(NAME(m_invert_coin_lockout));
	save_item(NAME(m_from_mcu_pending));
	save_item(NAME(m_from_z80_pending));
	save_item(NAME(m_port1));
	save_item(NAME(m_port2));
	save_item(NAME(m_port3));
	save_item(NAME(m_port3_latch));
	save_item(NAME(m_from_mcu));
	save_item(NAME(m_from_z80));
	save_item(NAME(m_portb));
	save_item(NAME(m_nmi_mask));

	// hotsmash ???
	save_item(NAME(m_portA_in));
	save_item(NAME(m_portB_out));
	save_item(NAME(m_portC));
}

MACHINE_START_MEMBER(superqix_state,superqix)
{
	/* configure the banks */
	membank("bank1")->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x4000);

	machine_init_common();
}

MACHINE_START_MEMBER(superqix_state,pbillian)
{
	/* configure the banks */
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x4000);

	machine_init_common();
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, superqix_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe000, 0xe0ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe100, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(superqix_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pbillian_port_map, AS_IO, 8, superqix_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0401, 0x0401) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x0402, 0x0403) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE(0x0408, 0x0408) AM_READ(pbillian_from_mcu_r)
	AM_RANGE(0x0408, 0x0408) AM_WRITE(pbillian_z80_mcu_w)
	AM_RANGE(0x0410, 0x0410) AM_WRITE(pbillian_0410_w)
	AM_RANGE(0x0418, 0x0418) AM_READNOP  //?
	AM_RANGE(0x0419, 0x0419) AM_WRITENOP  //? watchdog ?
	AM_RANGE(0x041a, 0x041a) AM_WRITE(pbillian_sample_trigger_w)
	AM_RANGE(0x041b, 0x041b) AM_READNOP  // input related? but probably not used
ADDRESS_MAP_END

static ADDRESS_MAP_START( hotsmash_port_map, AS_IO, 8, superqix_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0401, 0x0401) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x0402, 0x0403) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE(0x0408, 0x0408) AM_READ(hotsmash_from_mcu_r)
	AM_RANGE(0x0408, 0x0408) AM_WRITE(hotsmash_z80_mcu_w)
	AM_RANGE(0x0410, 0x0410) AM_WRITE(pbillian_0410_w)
	AM_RANGE(0x0418, 0x0418) AM_READNOP  //?
	AM_RANGE(0x0419, 0x0419) AM_WRITENOP  //? watchdog ?
	AM_RANGE(0x041a, 0x041a) AM_WRITE(pbillian_sample_trigger_w)
	AM_RANGE(0x041b, 0x041b) AM_READNOP  // input related? but probably not used
ADDRESS_MAP_END

static ADDRESS_MAP_START( sqix_port_map, AS_IO, 8, superqix_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0401, 0x0401) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x0402, 0x0403) AM_DEVWRITE("ay1", ay8910_device, data_address_w)
	AM_RANGE(0x0405, 0x0405) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0x0406, 0x0407) AM_DEVWRITE("ay2", ay8910_device, data_address_w)
	AM_RANGE(0x0408, 0x0408) AM_READ(mcu_acknowledge_r)
	AM_RANGE(0x0410, 0x0410) AM_WRITE(superqix_0410_w)  /* ROM bank, NMI enable, tile bank */
	AM_RANGE(0x0418, 0x0418) AM_READ(nmi_ack_r)
	AM_RANGE(0x0800, 0x77ff) AM_RAM_WRITE(superqix_bitmapram_w) AM_SHARE("bitmapram")
	AM_RANGE(0x8800, 0xf7ff) AM_RAM_WRITE(superqix_bitmapram2_w) AM_SHARE("bitmapram2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( m68705_map, AS_PROGRAM, 8, superqix_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READ(hotsmash_68705_portA_r)
	AM_RANGE(0x0001, 0x0001) AM_WRITE(hotsmash_68705_portB_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(hotsmash_68705_portC_r, hotsmash_68705_portC_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END


/* I8751 memory handlers */

static ADDRESS_MAP_START( bootleg_mcu_io_map, AS_IO, 8, superqix_state )
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_WRITE(bootleg_mcu_p1_w)
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_READWRITE(bootleg_mcu_p3_r, mcu_p3_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sqixu_mcu_io_map, AS_IO, 8, superqix_state )
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P0) AM_READ(sqix_system_status_r)
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READ_PORT("DSW1")
	AM_RANGE(MCS51_PORT_P2, MCS51_PORT_P2) AM_WRITE(sqixu_mcu_p2_w)
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_READWRITE(sqixu_mcu_p3_r, mcu_p3_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( pbillian )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x18, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "10/20/300K Points" )
	PORT_DIPSETTING(    0x00, "10/30/500K Points" )
	PORT_DIPSETTING(    0x08, "20/30/400K Points" )
	PORT_DIPSETTING(    0x04, "30/40/500K Points" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )     // N/C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )    // P1 fire (M powerup) + high score initials
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     // N/C
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL  // P2 fire (M powerup) + high score initials
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )   // mcu status (pending mcu->z80)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )   // mcu status (pending z80->mcu)

	PORT_START("PLUNGER1")  // plunger mechanism for shot (BUTTON1 and PEDAL mapped to the same key in MAME)
	PORT_BIT( 0x3f, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0x3f) PORT_SENSITIVITY(100) PORT_KEYDELTA(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(20) PORT_KEYDELTA(8)

	PORT_START("PLUNGER2")
	PORT_BIT( 0x3f, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0x3f) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(20) PORT_KEYDELTA(8) PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( hotsmash )
	PORT_START("DSW1")
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

	PORT_START("DSW2")
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )//$49c
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )//$42d
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )   // ?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )   // mcu status (0 = pending mcu->z80)

	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(30) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(30) PORT_CENTERDELTA(0) PORT_PLAYER(2)

INPUT_PORTS_END


static INPUT_PORTS_START( superqix )
	PORT_START("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Freeze" )                    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")  /* DSW2 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "20000 50000" )
	PORT_DIPSETTING(    0x0c, "30000 100000" )
	PORT_DIPSETTING(    0x04, "50000 100000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, "Fill Area" )                 PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, "70%" )
	PORT_DIPSETTING(    0xc0, "75%" )
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x00, "85%" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )   // doesn't work in bootleg
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )   // Z80 status (pending mcu->z80)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )   // Z80 status (pending z80->mcu)

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")   /* ??? */
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )   // mcu status (pending mcu->z80)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )   // mcu status (pending z80->mcu)
INPUT_PORTS_END



static const gfx_layout pbillian_charlayout =
{
	8,8,
	0x800,  /* doesn't use the whole ROM space */
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
	GFXDECODE_ENTRY( "gfx1", 0, pbillian_charlayout, 16*16, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,            0, 16 )
GFXDECODE_END

static GFXDECODE_START( sqix )
	GFXDECODE_ENTRY( "gfx1", 0x00000, sqix_charlayout,   0, 16 )    /* Chars */
	GFXDECODE_ENTRY( "gfx2", 0x00000, sqix_charlayout,   0, 16 )    /* Background tiles */
	GFXDECODE_ENTRY( "gfx3", 0x00000, spritelayout,      0, 16 )    /* Sprites */
GFXDECODE_END


INTERRUPT_GEN_MEMBER(superqix_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN_MEMBER(superqix_state::sqix_timer_irq)
{
	if (m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}



static MACHINE_CONFIG_START( pbillian, superqix_state )
	MCFG_CPU_ADD("maincpu", Z80,12000000/2)      /* 6 MHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(pbillian_port_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", superqix_state,  vblank_irq)

	MCFG_MACHINE_START_OVERRIDE(superqix_state,pbillian)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(superqix_state, screen_update_pbillian)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pbillian)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT_CLASS(1, superqix_state, BBGGRRII)

	MCFG_VIDEO_START_OVERRIDE(superqix_state,pbillian)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 12000000/8)
	MCFG_AY8910_PORT_A_READ_CB(READ8(superqix_state, pbillian_ay_port_a_r))   /* port Aread */
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("SYSTEM"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_START_CB(superqix_state, pbillian_sh_start)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hotsmash, superqix_state )
	MCFG_CPU_ADD("maincpu", Z80,12000000/2)      /* 6 MHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(hotsmash_port_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", superqix_state,  vblank_irq)

	MCFG_CPU_ADD("mcu", M68705, 4000000) /* ???? */
	MCFG_CPU_PROGRAM_MAP(m68705_map)

	MCFG_MACHINE_START_OVERRIDE(superqix_state,pbillian)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(superqix_state, screen_update_pbillian)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pbillian)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT_CLASS(1, superqix_state, BBGGRRII)

	MCFG_VIDEO_START_OVERRIDE(superqix_state,pbillian)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 12000000/8)
	MCFG_AY8910_PORT_A_READ_CB(READ8(superqix_state, hotsmash_ay_port_a_r))   /* port Aread */
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("SYSTEM"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_START_CB(superqix_state, pbillian_sh_start)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sqix, superqix_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 12000000/2)    /* 6 MHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(sqix_port_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(superqix_state, sqix_timer_irq,  4*60) /* ??? */

	MCFG_CPU_ADD("mcu", I8751, 12000000/3)  /* ??? */
	MCFG_CPU_IO_MAP(bootleg_mcu_io_map)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_MACHINE_START_OVERRIDE(superqix_state,superqix)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(superqix_state, screen_update_superqix)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sqix)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT_CLASS(1, superqix_state, BBGGRRII)

	MCFG_VIDEO_START_OVERRIDE(superqix_state,superqix)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 12000000/8)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("P1"))
	MCFG_AY8910_PORT_B_READ_CB(READ8(superqix_state, in4_mcu_r)) /* port Bread */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 12000000/8)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW2"))
	MCFG_AY8910_PORT_B_READ_CB(READ8(superqix_state, sqix_from_mcu_r)) /* port Bread */
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(superqix_state,sqix_z80_mcu_w)) /* port Bwrite */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sqixu, sqix )

	MCFG_CPU_MODIFY("mcu")
	MCFG_CPU_IO_MAP(sqixu_mcu_io_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( sqixbl, superqix_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 12000000/2)    /* 6 MHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(sqix_port_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(superqix_state, sqix_timer_irq,  4*60) /* ??? */

	MCFG_MACHINE_START_OVERRIDE(superqix_state,superqix)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(superqix_state, screen_update_superqix)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sqix)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT_CLASS(1, superqix_state, BBGGRRII)

	MCFG_VIDEO_START_OVERRIDE(superqix_state,superqix)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 12000000/8)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("P1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("P2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 12000000/8)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW2"))
	MCFG_AY8910_PORT_B_READ_CB(READ8(superqix_state, bootleg_in0_r)) /* port Bread */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pbillian )
	ROM_REGION( 0x018000, "maincpu", 0 )
	ROM_LOAD( "1.6c",  0x00000, 0x08000, CRC(d379fe23) SHA1(e147a9151b1cdeacb126d9713687bd0aa92980ac) )
	ROM_LOAD( "2.6d",  0x14000, 0x04000, CRC(1af522bc) SHA1(83e002dc831bfcedbd7096b350c9b34418b79674) )

	ROM_REGION( 0x0800, "cpu1", 0 )
	ROM_LOAD( "pbillian.mcu", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x8000, "samples", 0 )
	ROM_LOAD( "3.7j",  0x0000, 0x08000, CRC(3f9bc7f1) SHA1(0b0c2ec3bea6a7f3fc6c0c8b750318f3f9ec3d1f) )

	ROM_REGION( 0x018000, "gfx1", 0 )
	ROM_LOAD( "4.1n",  0x00000, 0x08000, CRC(9c08a072) SHA1(25f31fcf72216cf42528b07ad8c09113aa69861a) )
	ROM_LOAD( "5.1r",  0x08000, 0x08000, CRC(2dd5b83f) SHA1(b05e3a008050359d0207757b9cbd8cee87abc697) )
	ROM_LOAD( "6.1t",  0x10000, 0x08000, CRC(33b855b0) SHA1(5a1df4f82fc0d6f78883b759fd61f395942645eb) )
ROM_END

ROM_START( hotsmash )
	ROM_REGION( 0x018000, "maincpu", 0 )
	ROM_LOAD( "b18-04",  0x00000, 0x08000, CRC(981bde2c) SHA1(ebcc901a036cde16b33d534d423500d74523b781) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "b18-06.mcu", 0x0000, 0x0800, CRC(67c0920a) SHA1(23a294892823d1d9216ea8ddfa9df1c8af149477) )

	ROM_REGION( 0x8000, "samples", 0 )
	ROM_LOAD( "b18-05",  0x0000, 0x08000, CRC(dab5e718) SHA1(6cf6486f283f5177dfdc657b1627fbfa3f0743e8) )

	ROM_REGION( 0x018000, "gfx1", 0 )
	ROM_LOAD( "b18-01",  0x00000, 0x08000, CRC(870a4c04) SHA1(a029108bcda40755c8320d2ee297f42d816aa7c0) )
	ROM_LOAD( "b18-02",  0x08000, 0x08000, CRC(4e625cac) SHA1(2c21b32240eaada9a5f909a2ec5b335372c8c994) )
	ROM_LOAD( "b18-03",  0x14000, 0x04000, CRC(1c82717d) SHA1(6942c8877e24ac51ed71036e771a1655d82f3491) )
ROM_END

ROM_START( sqix )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b03-01-2.f3",   0x00000, 0x08000, CRC(5ded636b) SHA1(827954001b4617b3bd439be75094d8dca06ea32b) )
	ROM_LOAD( "b03-02.h3",     0x10000, 0x10000, CRC(9c23cb64) SHA1(7e04cb18cabdc0031621162cbc228cd95875a022) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* I8751 code */
	ROM_LOAD( "b03-03.l2",    0x00000, 0x1000, NO_DUMP ) /* Original Taito ID code for this set's MCU */
	/* sq07.108 is from the sqixb1 set, it will be removed once the actual MCU code from b03-03.l2 is decapped / dumped */
	ROM_LOAD( "sq07.108",     0x00000, 0x1000, BAD_DUMP CRC(d11411fb) SHA1(31183f433596c4d2503c01f6dc8d91024f2cf5de) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b03-04.s8",    0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "sq-iu3.p8",    0x00000, 0x20000, CRC(b8d0c493) SHA1(ef5d62ef3835c7ae088a7aa98945f747130fe0ec) ) /* Sharp LH231041 28 pin 128K x 8bit mask rom */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "b03-05.t8",    0x00000, 0x10000, CRC(df326540) SHA1(1fe025edcd38202e24c4e1005f478b6a88533453) )
ROM_END

ROM_START( sqixr1 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b03-01-1.f3",   0x00000, 0x08000, CRC(ad614117) SHA1(c461f00a2aecde1bc3860c15a3c31091b14665a2) )
	ROM_LOAD( "b03-02.h3",     0x10000, 0x10000, CRC(9c23cb64) SHA1(7e04cb18cabdc0031621162cbc228cd95875a022) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* I8751 code */
	ROM_LOAD( "b03-03.l2",    0x00000, 0x1000, NO_DUMP ) /* Original Taito ID code for this set's MCU */
	/* sq07.108 is from the sqixb1 set, it will be removed once the actual MCU code from b03-03.l2 is decapped / dumped */
	ROM_LOAD( "sq07.108",     0x00000, 0x1000, BAD_DUMP CRC(d11411fb) SHA1(31183f433596c4d2503c01f6dc8d91024f2cf5de) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b03-04.s8",    0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "sq-iu3.p8",    0x00000, 0x20000, CRC(b8d0c493) SHA1(ef5d62ef3835c7ae088a7aa98945f747130fe0ec) ) /* Sharp LH231041 28 pin 128K x 8bit mask rom */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "b03-05.t8",    0x00000, 0x10000, CRC(df326540) SHA1(1fe025edcd38202e24c4e1005f478b6a88533453) )
ROM_END

ROM_START( sqixu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b03-06.f3",    0x00000, 0x08000, CRC(4f59f7af) SHA1(6ea627ea8505cf8d1a5a1350258180c61fbd1ed9) )
	ROM_LOAD( "b03-07.h3",    0x10000, 0x10000, CRC(4c417d4a) SHA1(de46551da1b27312dca40240a210e77595cf9dbd) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* I8751 code */
	ROM_LOAD( "b03-08.l2",    0x00000, 0x01000, CRC(7c338c0f) SHA1(b91468c881641f807067835b2dd490cd3e3c577e) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b03-04.s8",    0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "sq-iu3.p8",    0x00000, 0x20000, CRC(b8d0c493) SHA1(ef5d62ef3835c7ae088a7aa98945f747130fe0ec) ) /* Sharp LH231041 28 pin 128K x 8bit mask rom */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "b03-09.t8",    0x00000, 0x10000, CRC(69d2a84a) SHA1(b461d8a01f73c6aaa4aac85602c688c111bdca5d) )
ROM_END

ROM_START( sqixb1 ) /* this was probably a bootleg */
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sq01.97",       0x00000, 0x08000, CRC(0888b7de) SHA1(de3e4637436de185f43d2ad4186d4cfdcd4d33d9) )
	ROM_LOAD( "b03-02.h3",     0x10000, 0x10000, CRC(9c23cb64) SHA1(7e04cb18cabdc0031621162cbc228cd95875a022) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* I8751 code */
	ROM_LOAD( "sq07.108",     0x00000, 0x1000, BAD_DUMP CRC(d11411fb) SHA1(31183f433596c4d2503c01f6dc8d91024f2cf5de) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b03-04.s8",    0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "b03-03",       0x00000, 0x10000, CRC(6e8b6a67) SHA1(c71117cc880a124c46397c446d1edc1cbf681200) ) /* 1st half of sq-iu3.p8 */
	ROM_LOAD( "b03-06",       0x10000, 0x10000, CRC(38154517) SHA1(703ad4cfe54a4786c67aedcca5998b57f39fd857) ) /* 2nd half of sq-iu3.p8 */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "b03-05.t8",    0x00000, 0x10000, CRC(df326540) SHA1(1fe025edcd38202e24c4e1005f478b6a88533453) )
ROM_END

ROM_START( sqixb2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cpu.2",         0x00000, 0x08000, CRC(682e28e3) SHA1(fe9221d26d7397be5a0fc8fdc51672b5924f3cf2) )
	ROM_LOAD( "b03-02.h3",     0x10000, 0x10000, CRC(9c23cb64) SHA1(7e04cb18cabdc0031621162cbc228cd95875a022) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b03-04.s8",    0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "b03-03",       0x00000, 0x10000, CRC(6e8b6a67) SHA1(c71117cc880a124c46397c446d1edc1cbf681200) ) /* 1st half of sq-iu3.p8 */
	ROM_LOAD( "b03-06",       0x10000, 0x10000, CRC(38154517) SHA1(703ad4cfe54a4786c67aedcca5998b57f39fd857) ) /* 2nd half of sq-iu3.p8 */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "b03-05.t8",    0x00000, 0x10000, CRC(df326540) SHA1(1fe025edcd38202e24c4e1005f478b6a88533453) )
ROM_END

ROM_START( perestrof )
	ROM_REGION( 0x20000, "maincpu", 0 )
	/* 0x8000 - 0x10000 in the rom is empty anyway */
	ROM_LOAD( "rom1.bin",        0x00000, 0x20000, CRC(0cbf96c1) SHA1(cf2b1367887d1b8812a56aa55593e742578f220c) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "rom4.bin",       0x00000, 0x10000, CRC(c56122a8) SHA1(1d24b2f0358e14aca5681f92175869224584a6ea) ) /* both halves identical */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "rom2.bin",       0x00000, 0x20000, CRC(36f93701) SHA1(452cb23efd955c6c155cef2b1b650e253e195738) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "rom3.bin",       0x00000, 0x10000, CRC(00c91d5a) SHA1(fdde56d3689a47e6bfb296e442207b93b887ec7a) )
ROM_END

ROM_START( perestro )
	ROM_REGION( 0x20000, "maincpu", 0 )
	/* 0x8000 - 0x10000 in the rom is empty anyway */
	ROM_LOAD( "rom1.bin",        0x00000, 0x20000, CRC(0cbf96c1) SHA1(cf2b1367887d1b8812a56aa55593e742578f220c) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "rom4.bin",       0x00000, 0x10000, CRC(c56122a8) SHA1(1d24b2f0358e14aca5681f92175869224584a6ea) ) /* both halves identical */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "rom2.bin",       0x00000, 0x20000, CRC(36f93701) SHA1(452cb23efd955c6c155cef2b1b650e253e195738) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "rom3a.bin",       0x00000, 0x10000, CRC(7a2a563f) SHA1(e3654091b858cc80ec1991281447fc3622a0d4f9) )
ROM_END



DRIVER_INIT_MEMBER(superqix_state,sqix)
{
	m_invert_coin_lockout = 1;
}

DRIVER_INIT_MEMBER(superqix_state,sqixa)
{
	m_invert_coin_lockout = 0;
}

DRIVER_INIT_MEMBER(superqix_state,perestro)
{
	UINT8 *src;
	int len;
	UINT8 temp[16];
	int i,j;

	/* decrypt program code; the address lines are shuffled around in a non-trivial way */
	src = memregion("maincpu")->base();
	len = memregion("maincpu")->bytes();
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
	src = memregion("gfx1")->base();
	len = memregion("gfx1")->bytes();
	for (i = 0;i < len;i += 16)
	{
		memcpy(temp,&src[i],16);
		for (j = 0;j < 16;j++)
		{
			src[i+j] = temp[BITSWAP8(j,7,6,5,4,3,2,0,1)];
		}
	}

	src = memregion("gfx2")->base();
	len = memregion("gfx2")->bytes();
	for (i = 0;i < len;i += 16)
	{
		memcpy(temp,&src[i],16);
		for (j = 0;j < 16;j++)
		{
			src[i+j] = temp[BITSWAP8(j,7,6,5,4,0,1,2,3)];
		}
	}

	src = memregion("gfx3")->base();
	len = memregion("gfx3")->bytes();
	for (i = 0;i < len;i += 16)
	{
		memcpy(temp,&src[i],16);
		for (j = 0;j < 16;j++)
		{
			src[i+j] = temp[BITSWAP8(j,7,6,5,4,1,0,3,2)];
		}
	}
}



GAME( 1986, pbillian, 0,        pbillian, pbillian, driver_device, 0,        ROT0,  "Kaneko / Taito", "Prebillian", MACHINE_SUPPORTS_SAVE )
GAME( 1987, hotsmash, 0,        hotsmash, hotsmash, driver_device, 0,        ROT90, "Kaneko / Taito", "Vs. Hot Smash", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sqix,     0,        sqix,     superqix, superqix_state, sqix,     ROT90, "Kaneko / Taito", "Super Qix (World, Rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sqixr1,   sqix,     sqix,     superqix, superqix_state, sqix,     ROT90, "Kaneko / Taito", "Super Qix (World, Rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sqixu,    sqix,     sqixu,    superqix, driver_device, 0,        ROT90, "Kaneko / Taito (Romstar License)", "Super Qix (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sqixb1,   sqix,     sqix,     superqix, superqix_state, sqixa,    ROT90, "bootleg", "Super Qix (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sqixb2,   sqix,     sqixbl,   superqix, driver_device, 0,        ROT90, "bootleg", "Super Qix (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, perestro, 0,        sqixbl,   superqix, superqix_state, perestro, ROT90, "Promat", "Perestroika Girls", MACHINE_SUPPORTS_SAVE )
GAME( 1993, perestrof,perestro, sqixbl,   superqix, superqix_state, perestro, ROT90, "Promat (Fuuki license)", "Perestroika Girls (Fuuki license)", MACHINE_SUPPORTS_SAVE )
