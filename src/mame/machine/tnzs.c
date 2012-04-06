/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

  The I8742 MCU takes care of handling the coin inputs and the tilt switch.
  To simulate this, we read the status in the interrupt handler for the main
  CPU and update the counters appropriately. We also must take care of
  handling the coin/credit settings ourselves.

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "includes/tnzs.h"


READ8_MEMBER(tnzs_state::mcu_tnzs_r)
{
	UINT8 data;

	data = upi41_master_r(m_mcu, offset & 1);
	device_yield(&space.device());

//  logerror("PC %04x: read %02x from mcu $c00%01x\n", cpu_get_previouspc(&space.device()), data, offset);

	return data;
}

WRITE8_MEMBER(tnzs_state::mcu_tnzs_w)
{
//  logerror("PC %04x: write %02x to mcu $c00%01x\n", cpu_get_previouspc(&space.device()), data, offset);

	upi41_master_w(m_mcu, offset & 1, data);
}


READ8_MEMBER(tnzs_state::tnzs_port1_r)
{
	int data = 0;

	switch (m_input_select & 0x0f)
	{
		case 0x0a:	data = input_port_read(machine(), "IN2"); break;
		case 0x0c:	data = input_port_read(machine(), "IN0"); break;
		case 0x0d:	data = input_port_read(machine(), "IN1"); break;
		default:	data = 0xff; break;
	}

//  logerror("I8742:%04x  Read %02x from port 1\n", cpu_get_previouspc(&space.device()), data);

	return data;
}

READ8_MEMBER(tnzs_state::tnzs_port2_r)
{
	int data = input_port_read(machine(), "IN2");

//  logerror("I8742:%04x  Read %02x from port 2\n", cpu_get_previouspc(&space.device()), data);

	return data;
}

WRITE8_MEMBER(tnzs_state::tnzs_port2_w)
{
//  logerror("I8742:%04x  Write %02x to port 2\n", cpu_get_previouspc(&space.device()), data);

	coin_lockout_w(machine(), 0, (data & 0x40));
	coin_lockout_w(machine(), 1, (data & 0x80));
	coin_counter_w(machine(), 0, (~data & 0x10));
	coin_counter_w(machine(), 1, (~data & 0x20));

	m_input_select = data;
}



READ8_MEMBER(tnzs_state::arknoid2_sh_f000_r)
{
	int val;

//  logerror("PC %04x: read input %04x\n", cpu_get_pc(&space.device()), 0xf000 + offset);

	val = input_port_read_safe(machine(), (offset / 2) ? "AN2" : "AN1", 0);
	if (offset & 1)
		return ((val >> 8) & 0xff);
	else
		return val & 0xff;
}


static void mcu_reset( running_machine &machine )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();

	state->m_mcu_initializing = 3;
	state->m_mcu_coinage_init = 0;
	state->m_mcu_coinage[0] = 1;
	state->m_mcu_coinage[1] = 1;
	state->m_mcu_coinage[2] = 1;
	state->m_mcu_coinage[3] = 1;
	state->m_mcu_coins_a = 0;
	state->m_mcu_coins_b = 0;
	state->m_mcu_credits = 0;
	state->m_mcu_reportcoin = 0;
	state->m_mcu_command = 0;
}

static void mcu_handle_coins( running_machine &machine, int coin )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();

	/* The coin inputs and coin counters are managed by the i8742 mcu. */
	/* Here we simulate it. */
	/* Credits are limited to 9, so more coins should be rejected */
	/* Coin/Play settings must also be taken into consideration */

	if (coin & 0x08)	/* tilt */
		state->m_mcu_reportcoin = coin;
	else if (coin && coin != state->m_insertcoin)
	{
		if (coin & 0x01)	/* coin A */
		{
//          logerror("Coin dropped into slot A\n");
			coin_counter_w(machine,0,1); coin_counter_w(machine,0,0); /* Count slot A */
			state->m_mcu_coins_a++;
			if (state->m_mcu_coins_a >= state->m_mcu_coinage[0])
			{
				state->m_mcu_coins_a -= state->m_mcu_coinage[0];
				state->m_mcu_credits += state->m_mcu_coinage[1];
				if (state->m_mcu_credits >= 9)
				{
					state->m_mcu_credits = 9;
					coin_lockout_global_w(machine, 1); /* Lock all coin slots */
				}
				else
				{
					coin_lockout_global_w(machine, 0); /* Unlock all coin slots */
				}
			}
		}

		if (coin & 0x02)	/* coin B */
		{
//          logerror("Coin dropped into slot B\n");
			coin_counter_w(machine,1,1); coin_counter_w(machine,1,0); /* Count slot B */
			state->m_mcu_coins_b++;
			if (state->m_mcu_coins_b >= state->m_mcu_coinage[2])
			{
				state->m_mcu_coins_b -= state->m_mcu_coinage[2];
				state->m_mcu_credits += state->m_mcu_coinage[3];
				if (state->m_mcu_credits >= 9)
				{
					state->m_mcu_credits = 9;
					coin_lockout_global_w(machine, 1); /* Lock all coin slots */
				}
				else
				{
					coin_lockout_global_w(machine, 0); /* Unlock all coin slots */
				}
			}
		}

		if (coin & 0x04)	/* service */
		{
//          logerror("Coin dropped into service slot C\n");
			state->m_mcu_credits++;
		}

		state->m_mcu_reportcoin = coin;
	}
	else
	{
		if (state->m_mcu_credits < 9)
			coin_lockout_global_w(machine, 0); /* Unlock all coin slots */

		state->m_mcu_reportcoin = 0;
	}
	state->m_insertcoin = coin;
}


READ8_MEMBER(tnzs_state::mcu_arknoid2_r)
{
	static const char mcu_startup[] = "\x55\xaa\x5a";

//  logerror("PC %04x: read mcu %04x\n", cpu_get_pc(&space.device()), 0xc000 + offset);

	if (offset == 0)
	{
		/* if the mcu has just been reset, return startup code */
		if (m_mcu_initializing)
		{
			m_mcu_initializing--;
			return mcu_startup[2 - m_mcu_initializing];
		}

		switch (m_mcu_command)
		{
			case 0x41:
				return m_mcu_credits;

			case 0xc1:
				/* Read the credit counter or the inputs */
				if (m_mcu_readcredits == 0)
				{
					m_mcu_readcredits = 1;
					if (m_mcu_reportcoin & 0x08)
					{
						m_mcu_initializing = 3;
						return 0xee;	/* tilt */
					}
					else return m_mcu_credits;
				}
				else return input_port_read(machine(), "IN0");	/* buttons */

			default:
				logerror("error, unknown mcu command\n");
				/* should not happen */
				return 0xff;
		}
	}
	else
	{
		/*
        status bits:
        0 = mcu is ready to send data (read from c000)
        1 = mcu has read data (from c000)
        2 = unused
        3 = unused
        4-7 = coin code
              0 = nothing
              1,2,3 = coin switch pressed
              e = tilt
        */
		if (m_mcu_reportcoin & 0x08) return 0xe1;	/* tilt */
		if (m_mcu_reportcoin & 0x01) return 0x11;	/* coin 1 (will trigger "coin inserted" sound) */
		if (m_mcu_reportcoin & 0x02) return 0x21;	/* coin 2 (will trigger "coin inserted" sound) */
		if (m_mcu_reportcoin & 0x04) return 0x31;	/* coin 3 (will trigger "coin inserted" sound) */
		return 0x01;
	}
}

WRITE8_MEMBER(tnzs_state::mcu_arknoid2_w)
{
	if (offset == 0)
	{
//      logerror("PC %04x: write %02x to mcu %04x\n", cpu_get_pc(&space.device()), data, 0xc000 + offset);
		if (m_mcu_command == 0x41)
		{
			m_mcu_credits = (m_mcu_credits + data) & 0xff;
		}
	}
	else
	{
		/*
        0xc1: read number of credits, then buttons
        0x54+0x41: add value to number of credits
        0x15: sub 1 credit (when "Continue Play" only)
        0x84: coin 1 lockout (issued only in test mode)
        0x88: coin 2 lockout (issued only in test mode)
        0x80: release coin lockout (issued only in test mode)
        during initialization, a sequence of 4 bytes sets coin/credit settings
        */
//      logerror("PC %04x: write %02x to mcu %04x\n", cpu_get_pc(&space.device()), data, 0xc000 + offset);

		if (m_mcu_initializing)
		{
			/* set up coin/credit settings */
			m_mcu_coinage[m_mcu_coinage_init++] = data;
			if (m_mcu_coinage_init == 4)
				m_mcu_coinage_init = 0;	/* must not happen */
		}

		if (data == 0xc1)
			m_mcu_readcredits = 0;	/* reset input port number */

		if (data == 0x15)
		{
			m_mcu_credits = (m_mcu_credits - 1) & 0xff;
			if (m_mcu_credits == 0xff)
				m_mcu_credits = 0;
		}
		m_mcu_command = data;
	}
}


READ8_MEMBER(tnzs_state::mcu_extrmatn_r)
{
	static const char mcu_startup[] = "\x5a\xa5\x55";

//  logerror("PC %04x: read mcu %04x\n", cpu_get_pc(&space.device()), 0xc000 + offset);

	if (offset == 0)
	{
		/* if the mcu has just been reset, return startup code */
		if (m_mcu_initializing)
		{
			m_mcu_initializing--;
			return mcu_startup[2 - m_mcu_initializing];
		}

		switch (m_mcu_command)
		{
			case 0x01:
				return input_port_read(machine(), "IN0") ^ 0xff;	/* player 1 joystick + buttons */

			case 0x02:
				return input_port_read(machine(), "IN1") ^ 0xff;	/* player 2 joystick + buttons */

			case 0x1a:
				return (input_port_read(machine(), "COIN1") | (input_port_read(machine(), "COIN2") << 1));

			case 0x21:
				return input_port_read(machine(), "IN2") & 0x0f;

			case 0x41:
				return m_mcu_credits;

			case 0xa0:
				/* Read the credit counter */
				if (m_mcu_reportcoin & 0x08)
				{
					m_mcu_initializing = 3;
					return 0xee;	/* tilt */
				}
				else return m_mcu_credits;

			case 0xa1:
				/* Read the credit counter or the inputs */
				if (m_mcu_readcredits == 0)
				{
					m_mcu_readcredits = 1;
					if (m_mcu_reportcoin & 0x08)
					{
						m_mcu_initializing = 3;
						return 0xee;	/* tilt */
//                      return 0x64;    /* theres a reset input somewhere */
					}
					else return m_mcu_credits;
				}
				/* buttons */
				else return ((input_port_read(machine(), "IN0") & 0xf0) | (input_port_read(machine(), "IN1") >> 4)) ^ 0xff;

			default:
				logerror("error, unknown mcu command\n");
				/* should not happen */
				return 0xff;
		}
	}
	else
	{
		/*
        status bits:
        0 = mcu is ready to send data (read from c000)
        1 = mcu has read data (from c000)
        2 = unused
        3 = unused
        4-7 = coin code
              0 = nothing
              1,2,3 = coin switch pressed
              e = tilt
        */
		if (m_mcu_reportcoin & 0x08) return 0xe1;	/* tilt */
		if (m_mcu_reportcoin & 0x01) return 0x11;	/* coin 1 (will trigger "coin inserted" sound) */
		if (m_mcu_reportcoin & 0x02) return 0x21;	/* coin 2 (will trigger "coin inserted" sound) */
		if (m_mcu_reportcoin & 0x04) return 0x31;	/* coin 3 (will trigger "coin inserted" sound) */
		return 0x01;
	}
}

WRITE8_MEMBER(tnzs_state::mcu_extrmatn_w)
{
	if (offset == 0)
	{
//      logerror("PC %04x: write %02x to mcu %04x\n", cpu_get_pc(&space.device()), data, 0xc000 + offset);
		if (m_mcu_command == 0x41)
		{
			m_mcu_credits = (m_mcu_credits + data) & 0xff;
		}
	}
	else
	{
		/*
        0xa0: read number of credits
        0xa1: read number of credits, then buttons
        0x01: read player 1 joystick + buttons
        0x02: read player 2 joystick + buttons
        0x1a: read coin switches
        0x21: read service & tilt switches
        0x4a+0x41: add value to number of credits
        0x84: coin 1 lockout (issued only in test mode)
        0x88: coin 2 lockout (issued only in test mode)
        0x80: release coin lockout (issued only in test mode)
        during initialization, a sequence of 4 bytes sets coin/credit settings
        */

//      logerror("PC %04x: write %02x to mcu %04x\n", cpu_get_pc(&space.device()), data, 0xc000 + offset);

		if (m_mcu_initializing)
		{
			/* set up coin/credit settings */
			m_mcu_coinage[m_mcu_coinage_init++] = data;
			if (m_mcu_coinage_init == 4)
				m_mcu_coinage_init = 0;	/* must not happen */
		}

		if (data == 0xa1)
			m_mcu_readcredits = 0;	/* reset input port number */

		/* Dr Toppel decrements credits differently. So handle it */
		if ((data == 0x09) && (m_mcu_type == MCU_DRTOPPEL || m_mcu_type == MCU_PLUMPOP))
			m_mcu_credits = (m_mcu_credits - 1) & 0xff;		/* Player 1 start */
		if ((data == 0x18) && (m_mcu_type == MCU_DRTOPPEL || m_mcu_type == MCU_PLUMPOP))
			m_mcu_credits = (m_mcu_credits - 2) & 0xff;		/* Player 2 start */

		m_mcu_command = data;
	}
}



/*********************************

TNZS sync bug kludge

In all TNZS versions there is code like this:

0C5E: ld   ($EF10),a
0C61: ld   a,($EF10)
0C64: inc  a
0C65: ret  nz
0C66: jr   $0C61

which is sometimes executed by the main cpu when it writes to shared RAM a
command for the second CPU. The intended purpose of the code is to wait an
acknowledge from the sub CPU: the sub CPU writes FF to the same location
after reading the command.

However the above code is wrong. The "ret nz" instruction means that the
loop will be exited only when the contents of $EF10 are *NOT* $FF!!
On the real board, this casues little harm: the main CPU will just write
the command, read it back and, since it's not $FF, return immediately. There
is a chance that the command might go lost, but this will cause no major
harm, the worse that can happen is that the background tune will not change.

In MAME, however, since CPU interleaving is not perfect, it can happen that
the main CPU ends its timeslice after writing to EF10 but before reading it
back. In the meantime, the sub CPU will run, read the command and write FF
there - therefore causing the main CPU to enter an endless loop.

Unlike the usual sync problems in MAME, which can be fixed by increasing the
interleave factor, in this case increasing it will actually INCREASE the
chance of entering the endless loop - because it will increase the chances of
the main CPU ending its timeslice at the wrong moment.

So what we do here is catch writes by the main CPU to the RAM location, and
process them using a timer, in order to
a) force a resync of the two CPUs
b) make sure the main CPU will be the first one to run after the location is
   changed

Since the answer from the sub CPU is ignored, we don't even need to boost
interleave.

*********************************/

/*
static TIMER_CALLBACK( kludge_callback )
{
    tnzs_sharedram[0x0f10] = param;
}

WRITE8_MEMBER(tnzs_state::tnzs_sync_kludge_w)
{
    machine().scheduler().synchronize(FUNC(kludge_callback), data);
}
*/



DRIVER_INIT( plumpop )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	state->m_mcu_type = MCU_PLUMPOP;
}

DRIVER_INIT( extrmatn )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	state->m_mcu_type = MCU_EXTRMATN;
}

DRIVER_INIT( arknoid2 )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	state->m_mcu_type = MCU_ARKANOID;
}

DRIVER_INIT( drtoppel )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	state->m_mcu_type = MCU_DRTOPPEL;

	/* drtoppel writes to the palette RAM area even if it has PROMs! We have to patch it out. */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->nop_write(0xf800, 0xfbff);
}

DRIVER_INIT( chukatai )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	state->m_mcu_type = MCU_CHUKATAI;
}

DRIVER_INIT( tnzs )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	state->m_mcu_type = MCU_TNZS;
	/* we need to install a kludge to avoid problems with a bug in the original code */
//  machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xef10, 0xef10, FUNC(tnzs_sync_kludge_w));
}

DRIVER_INIT( tnzsb )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	state->m_mcu_type = MCU_NONE_TNZSB;

	/* we need to install a kludge to avoid problems with a bug in the original code */
//  machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xef10, 0xef10, FUNC(tnzs_sync_kludge_w));
}

DRIVER_INIT( kabukiz )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	UINT8 *SOUND = machine.region("audiocpu")->base();
	state->m_mcu_type = MCU_NONE_KABUKIZ;

	memory_configure_bank(machine, "bank3", 0, 8, &SOUND[0x10000], 0x4000);
}

DRIVER_INIT( insectx )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	state->m_mcu_type = MCU_NONE_INSECTX;

	/* this game has no mcu, replace the handler with plain input port handlers */
	machine.device("sub")->memory().space(AS_PROGRAM)->install_read_port(0xc000, 0xc000, "IN0" );
	machine.device("sub")->memory().space(AS_PROGRAM)->install_read_port(0xc001, 0xc001, "IN1" );
	machine.device("sub")->memory().space(AS_PROGRAM)->install_read_port(0xc002, 0xc002, "IN2" );
}

DRIVER_INIT( kageki )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	state->m_mcu_type = MCU_NONE_KAGEKI;
}


READ8_MEMBER(tnzs_state::tnzs_mcu_r)
{
	switch (m_mcu_type)
	{
		case MCU_TNZS:
		case MCU_CHUKATAI:
			return mcu_tnzs_r(space, offset);
		case MCU_ARKANOID:
			return mcu_arknoid2_r(space, offset);
		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
			return mcu_extrmatn_r(space, offset);
		default:
			return 0xff;
	}
}

WRITE8_MEMBER(tnzs_state::tnzs_mcu_w)
{
	switch (m_mcu_type)
	{
		case MCU_TNZS:
		case MCU_CHUKATAI:
			mcu_tnzs_w(space, offset, data);
			break;
		case MCU_ARKANOID:
			mcu_arknoid2_w(space, offset, data);
			break;
		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
			mcu_extrmatn_w(space, offset, data);
			break;
		default:
			break;
	}
}

INTERRUPT_GEN( arknoid2_interrupt )
{
	tnzs_state *state = device->machine().driver_data<tnzs_state>();
	int coin;

	switch (state->m_mcu_type)
	{
		case MCU_ARKANOID:
		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
			coin  = 0;
			coin |= ((input_port_read(device->machine(), "COIN1") & 1) << 0);
			coin |= ((input_port_read(device->machine(), "COIN2") & 1) << 1);
			coin |= ((input_port_read(device->machine(), "IN2") & 3) << 2);
			coin ^= 0x0c;
			mcu_handle_coins(device->machine(), coin);
			break;
		default:
			break;
	}

	device_set_input_line(device, 0, HOLD_LINE);
}

MACHINE_RESET( tnzs )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	/* initialize the mcu simulation */
	switch (state->m_mcu_type)
	{
		case MCU_ARKANOID:
		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
			mcu_reset(machine);
			break;
		default:
			break;
	}

	state->m_screenflip = 0;
	state->m_kageki_csport_sel = 0;
	state->m_input_select = 0;
	state->m_mcu_readcredits = 0;	// this might belong to mcu_reset
	state->m_insertcoin = 0;		// this might belong to mcu_reset
}

MACHINE_RESET( jpopnics )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();

	state->m_screenflip = 0;
	state->m_mcu_type = -1;
}

static void tnzs_postload(running_machine &machine)
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	memory_set_bank(machine, "bank1", state->m_bank1);
	memory_set_bank(machine, "bank2", state->m_bank2);

	if (state->m_bank1 <= 1)
		space->install_write_bank(0x8000, 0xbfff, "bank1");
	else
		space->unmap_write(0x8000, 0xbfff);
}

MACHINE_START( tnzs )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	UINT8 *ROM = machine.region("maincpu")->base();
	UINT8 *SUB = machine.region("sub")->base();

	memory_configure_bank(machine, "bank1", 0, 8, &ROM[0x10000], 0x4000);
	memory_configure_bank(machine, "bank2", 0, 4, &SUB[0x10000], 0x2000);

	memory_set_bank(machine, "bank1", 2);
	memory_set_bank(machine, "bank2", 0);

	state->m_bank1 = 2;
	state->m_bank2 = 0;

	state->m_audiocpu = machine.device("audiocpu");
	state->m_subcpu = machine.device("sub");
	state->m_mcu = machine.device("mcu");

	state->save_item(NAME(state->m_screenflip));
	state->save_item(NAME(state->m_kageki_csport_sel));
	state->save_item(NAME(state->m_input_select));
	state->save_item(NAME(state->m_mcu_readcredits));
	state->save_item(NAME(state->m_insertcoin));
	state->save_item(NAME(state->m_mcu_initializing));
	state->save_item(NAME(state->m_mcu_coinage_init));
	state->save_item(NAME(state->m_mcu_coinage));
	state->save_item(NAME(state->m_mcu_coins_a));
	state->save_item(NAME(state->m_mcu_coins_b));
	state->save_item(NAME(state->m_mcu_credits));
	state->save_item(NAME(state->m_mcu_reportcoin));
	state->save_item(NAME(state->m_mcu_command));
	state->save_item(NAME(state->m_bank1));
	state->save_item(NAME(state->m_bank2));

	machine.save().register_postload(save_prepost_delegate(FUNC(tnzs_postload), &machine));
}

MACHINE_START( jpopnics )
{
	tnzs_state *state = machine.driver_data<tnzs_state>();
	UINT8 *ROM = machine.region("maincpu")->base();
	UINT8 *SUB = machine.region("sub")->base();

	memory_configure_bank(machine, "bank1", 0, 8, &ROM[0x10000], 0x4000);
	memory_configure_bank(machine, "bank2", 0, 4, &SUB[0x10000], 0x2000);

	state->m_subcpu = machine.device("sub");
	state->m_mcu = NULL;

	state->m_bank1 = 2;
	state->m_bank2 = 0;

	state->save_item(NAME(state->m_screenflip));
	state->save_item(NAME(state->m_bank1));
	state->save_item(NAME(state->m_bank2));

	machine.save().register_postload(save_prepost_delegate(FUNC(tnzs_postload), &machine));
}


WRITE8_MEMBER(tnzs_state::tnzs_bankswitch_w)
{

//  logerror("PC %04x: writing %02x to bankswitch\n", cpu_get_pc(&space.device()),data);

	/* bit 4 resets the second CPU */
	if (data & 0x10)
		device_set_input_line(m_subcpu, INPUT_LINE_RESET, CLEAR_LINE);
	else
		device_set_input_line(m_subcpu, INPUT_LINE_RESET, ASSERT_LINE);

	/* bits 0-2 select RAM/ROM bank */
	m_bank1 = data & 0x07;
	memory_set_bank(machine(), "bank1", m_bank1);

	if (m_bank1 <= 1)
		space.install_write_bank(0x8000, 0xbfff, "bank1");
	else
		space.unmap_write(0x8000, 0xbfff);
}

WRITE8_MEMBER(tnzs_state::tnzs_bankswitch1_w)
{
//  logerror("PC %04x: writing %02x to bankswitch 1\n", cpu_get_pc(&space.device()),data);

	switch (m_mcu_type)
	{
		case MCU_TNZS:
		case MCU_CHUKATAI:
				/* bit 2 resets the mcu */
				if (data & 0x04)
				{
					if (m_mcu != NULL && m_mcu->type() == I8742)
						device_set_input_line(m_mcu, INPUT_LINE_RESET, PULSE_LINE);
				}
				/* Coin count and lockout is handled by the i8742 */
				break;
		case MCU_NONE_INSECTX:
				coin_lockout_w(machine(), 0, (~data & 0x04));
				coin_lockout_w(machine(), 1, (~data & 0x08));
				coin_counter_w(machine(), 0, (data & 0x10));
				coin_counter_w(machine(), 1, (data & 0x20));
				break;
		case MCU_NONE_TNZSB:
		case MCU_NONE_KABUKIZ:
				coin_lockout_w(machine(), 0, (~data & 0x10));
				coin_lockout_w(machine(), 1, (~data & 0x20));
				coin_counter_w(machine(), 0, (data & 0x04));
				coin_counter_w(machine(), 1, (data & 0x08));
				break;
		case MCU_NONE_KAGEKI:
				coin_lockout_global_w(machine(), (~data & 0x20));
				coin_counter_w(machine(), 0, (data & 0x04));
				coin_counter_w(machine(), 1, (data & 0x08));
				break;
		case MCU_ARKANOID:
		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
				/* bit 2 resets the mcu */
				if (data & 0x04)
					mcu_reset(machine());
				break;
		default:
				break;
	}

	/* bits 0-1 select ROM bank */
	m_bank2 = data & 0x03;
	memory_set_bank(machine(), "bank2", m_bank2);
}
