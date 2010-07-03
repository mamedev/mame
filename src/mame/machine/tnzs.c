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


static READ8_HANDLER( mcu_tnzs_r )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;
	UINT8 data;

	data = upi41_master_r(state->mcu, offset & 1);
	cpu_yield(space->cpu);

//  logerror("PC %04x: read %02x from mcu $c00%01x\n", cpu_get_previouspc(space->cpu), data, offset);

	return data;
}

static WRITE8_HANDLER( mcu_tnzs_w )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;
//  logerror("PC %04x: write %02x to mcu $c00%01x\n", cpu_get_previouspc(space->cpu), data, offset);

	upi41_master_w(state->mcu, offset & 1, data);
}


READ8_HANDLER( tnzs_port1_r )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;
	int data = 0;

	switch (state->input_select & 0x0f)
	{
		case 0x0a:	data = input_port_read(space->machine, "IN2"); break;
		case 0x0c:	data = input_port_read(space->machine, "IN0"); break;
		case 0x0d:	data = input_port_read(space->machine, "IN1"); break;
		default:	data = 0xff; break;
	}

//  logerror("I8742:%04x  Read %02x from port 1\n", cpu_get_previouspc(space->cpu), data);

	return data;
}

READ8_HANDLER( tnzs_port2_r )
{
	int data = input_port_read(space->machine, "IN2");

//  logerror("I8742:%04x  Read %02x from port 2\n", cpu_get_previouspc(space->cpu), data);

	return data;
}

WRITE8_HANDLER( tnzs_port2_w )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;
//  logerror("I8742:%04x  Write %02x to port 2\n", cpu_get_previouspc(space->cpu), data);

	coin_lockout_w(space->machine, 0, (data & 0x40));
	coin_lockout_w(space->machine, 1, (data & 0x80));
	coin_counter_w(space->machine, 0, (~data & 0x10));
	coin_counter_w(space->machine, 1, (~data & 0x20));

	state->input_select = data;
}



READ8_HANDLER( arknoid2_sh_f000_r )
{
	int val;

//  logerror("PC %04x: read input %04x\n", cpu_get_pc(space->cpu), 0xf000 + offset);

	val = input_port_read_safe(space->machine, (offset / 2) ? "AN2" : "AN1", 0);
	if (offset & 1)
		return ((val >> 8) & 0xff);
	else
		return val & 0xff;
}


static void mcu_reset( running_machine *machine )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;

	state->mcu_initializing = 3;
	state->mcu_coinage_init = 0;
	state->mcu_coinage[0] = 1;
	state->mcu_coinage[1] = 1;
	state->mcu_coinage[2] = 1;
	state->mcu_coinage[3] = 1;
	state->mcu_coins_a = 0;
	state->mcu_coins_b = 0;
	state->mcu_credits = 0;
	state->mcu_reportcoin = 0;
	state->mcu_command = 0;
}

static void mcu_handle_coins( running_machine *machine, int coin )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;

	/* The coin inputs and coin counters are managed by the i8742 mcu. */
	/* Here we simulate it. */
	/* Credits are limited to 9, so more coins should be rejected */
	/* Coin/Play settings must also be taken into consideration */

	if (coin & 0x08)	/* tilt */
		state->mcu_reportcoin = coin;
	else if (coin && coin != state->insertcoin)
	{
		if (coin & 0x01)	/* coin A */
		{
//          logerror("Coin dropped into slot A\n");
			coin_counter_w(machine,0,1); coin_counter_w(machine,0,0); /* Count slot A */
			state->mcu_coins_a++;
			if (state->mcu_coins_a >= state->mcu_coinage[0])
			{
				state->mcu_coins_a -= state->mcu_coinage[0];
				state->mcu_credits += state->mcu_coinage[1];
				if (state->mcu_credits >= 9)
				{
					state->mcu_credits = 9;
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
			state->mcu_coins_b++;
			if (state->mcu_coins_b >= state->mcu_coinage[2])
			{
				state->mcu_coins_b -= state->mcu_coinage[2];
				state->mcu_credits += state->mcu_coinage[3];
				if (state->mcu_credits >= 9)
				{
					state->mcu_credits = 9;
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
			state->mcu_credits++;
		}

		state->mcu_reportcoin = coin;
	}
	else
	{
		if (state->mcu_credits < 9)
			coin_lockout_global_w(machine, 0); /* Unlock all coin slots */

		state->mcu_reportcoin = 0;
	}
	state->insertcoin = coin;
}


static READ8_HANDLER( mcu_arknoid2_r )
{
	static const char mcu_startup[] = "\x55\xaa\x5a";
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;

//  logerror("PC %04x: read mcu %04x\n", cpu_get_pc(space->cpu), 0xc000 + offset);

	if (offset == 0)
	{
		/* if the mcu has just been reset, return startup code */
		if (state->mcu_initializing)
		{
			state->mcu_initializing--;
			return mcu_startup[2 - state->mcu_initializing];
		}

		switch (state->mcu_command)
		{
			case 0x41:
				return state->mcu_credits;

			case 0xc1:
				/* Read the credit counter or the inputs */
				if (state->mcu_readcredits == 0)
				{
					state->mcu_readcredits = 1;
					if (state->mcu_reportcoin & 0x08)
					{
						state->mcu_initializing = 3;
						return 0xee;	/* tilt */
					}
					else return state->mcu_credits;
				}
				else return input_port_read(space->machine, "IN0");	/* buttons */

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
		if (state->mcu_reportcoin & 0x08) return 0xe1;	/* tilt */
		if (state->mcu_reportcoin & 0x01) return 0x11;	/* coin 1 (will trigger "coin inserted" sound) */
		if (state->mcu_reportcoin & 0x02) return 0x21;	/* coin 2 (will trigger "coin inserted" sound) */
		if (state->mcu_reportcoin & 0x04) return 0x31;	/* coin 3 (will trigger "coin inserted" sound) */
		return 0x01;
	}
}

static WRITE8_HANDLER( mcu_arknoid2_w )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;
	if (offset == 0)
	{
//      logerror("PC %04x: write %02x to mcu %04x\n", cpu_get_pc(space->cpu), data, 0xc000 + offset);
		if (state->mcu_command == 0x41)
		{
			state->mcu_credits = (state->mcu_credits + data) & 0xff;
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
//      logerror("PC %04x: write %02x to mcu %04x\n", cpu_get_pc(space->cpu), data, 0xc000 + offset);

		if (state->mcu_initializing)
		{
			/* set up coin/credit settings */
			state->mcu_coinage[state->mcu_coinage_init++] = data;
			if (state->mcu_coinage_init == 4)
				state->mcu_coinage_init = 0;	/* must not happen */
		}

		if (data == 0xc1)
			state->mcu_readcredits = 0;	/* reset input port number */

		if (data == 0x15)
		{
			state->mcu_credits = (state->mcu_credits - 1) & 0xff;
			if (state->mcu_credits == 0xff)
				state->mcu_credits = 0;
		}
		state->mcu_command = data;
	}
}


static READ8_HANDLER( mcu_extrmatn_r )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;
	static const char mcu_startup[] = "\x5a\xa5\x55";

//  logerror("PC %04x: read mcu %04x\n", cpu_get_pc(space->cpu), 0xc000 + offset);

	if (offset == 0)
	{
		/* if the mcu has just been reset, return startup code */
		if (state->mcu_initializing)
		{
			state->mcu_initializing--;
			return mcu_startup[2 - state->mcu_initializing];
		}

		switch (state->mcu_command)
		{
			case 0x01:
				return input_port_read(space->machine, "IN0") ^ 0xff;	/* player 1 joystick + buttons */

			case 0x02:
				return input_port_read(space->machine, "IN1") ^ 0xff;	/* player 2 joystick + buttons */

			case 0x1a:
				return (input_port_read(space->machine, "COIN1") | (input_port_read(space->machine, "COIN2") << 1));

			case 0x21:
				return input_port_read(space->machine, "IN2") & 0x0f;

			case 0x41:
				return state->mcu_credits;

			case 0xa0:
				/* Read the credit counter */
				if (state->mcu_reportcoin & 0x08)
				{
					state->mcu_initializing = 3;
					return 0xee;	/* tilt */
				}
				else return state->mcu_credits;

			case 0xa1:
				/* Read the credit counter or the inputs */
				if (state->mcu_readcredits == 0)
				{
					state->mcu_readcredits = 1;
					if (state->mcu_reportcoin & 0x08)
					{
						state->mcu_initializing = 3;
						return 0xee;	/* tilt */
//                      return 0x64;    /* theres a reset input somewhere */
					}
					else return state->mcu_credits;
				}
				/* buttons */
				else return ((input_port_read(space->machine, "IN0") & 0xf0) | (input_port_read(space->machine, "IN1") >> 4)) ^ 0xff;

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
		if (state->mcu_reportcoin & 0x08) return 0xe1;	/* tilt */
		if (state->mcu_reportcoin & 0x01) return 0x11;	/* coin 1 (will trigger "coin inserted" sound) */
		if (state->mcu_reportcoin & 0x02) return 0x21;	/* coin 2 (will trigger "coin inserted" sound) */
		if (state->mcu_reportcoin & 0x04) return 0x31;	/* coin 3 (will trigger "coin inserted" sound) */
		return 0x01;
	}
}

static WRITE8_HANDLER( mcu_extrmatn_w )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;
	if (offset == 0)
	{
//      logerror("PC %04x: write %02x to mcu %04x\n", cpu_get_pc(space->cpu), data, 0xc000 + offset);
		if (state->mcu_command == 0x41)
		{
			state->mcu_credits = (state->mcu_credits + data) & 0xff;
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

//      logerror("PC %04x: write %02x to mcu %04x\n", cpu_get_pc(space->cpu), data, 0xc000 + offset);

		if (state->mcu_initializing)
		{
			/* set up coin/credit settings */
			state->mcu_coinage[state->mcu_coinage_init++] = data;
			if (state->mcu_coinage_init == 4)
				state->mcu_coinage_init = 0;	/* must not happen */
		}

		if (data == 0xa1)
			state->mcu_readcredits = 0;	/* reset input port number */

		/* Dr Toppel decrements credits differently. So handle it */
		if ((data == 0x09) && (state->mcu_type == MCU_DRTOPPEL || state->mcu_type == MCU_PLUMPOP))
			state->mcu_credits = (state->mcu_credits - 1) & 0xff;		/* Player 1 start */
		if ((data == 0x18) && (state->mcu_type == MCU_DRTOPPEL || state->mcu_type == MCU_PLUMPOP))
			state->mcu_credits = (state->mcu_credits - 2) & 0xff;		/* Player 2 start */

		state->mcu_command = data;
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

static WRITE8_HANDLER( tnzs_sync_kludge_w )
{
    timer_call_after_resynch(space->machine, NULL, data,kludge_callback);
}
*/



DRIVER_INIT( plumpop )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	state->mcu_type = MCU_PLUMPOP;
}

DRIVER_INIT( extrmatn )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	state->mcu_type = MCU_EXTRMATN;
}

DRIVER_INIT( arknoid2 )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	state->mcu_type = MCU_ARKANOID;
}

DRIVER_INIT( drtoppel )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	state->mcu_type = MCU_DRTOPPEL;

	/* drtoppel writes to the palette RAM area even if it has PROMs! We have to patch it out. */
	memory_nop_write(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xf800, 0xfbff, 0, 0);
}

DRIVER_INIT( chukatai )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	state->mcu_type = MCU_CHUKATAI;
}

DRIVER_INIT( tnzs )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	state->mcu_type = MCU_TNZS;
	/* we need to install a kludge to avoid problems with a bug in the original code */
//  memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xef10, 0xef10, 0, 0, tnzs_sync_kludge_w);
}

DRIVER_INIT( tnzsb )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	state->mcu_type = MCU_NONE_TNZSB;

	/* we need to install a kludge to avoid problems with a bug in the original code */
//  memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xef10, 0xef10, 0, 0, tnzs_sync_kludge_w);
}

DRIVER_INIT( kabukiz )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	UINT8 *SOUND = memory_region(machine, "audiocpu");
	state->mcu_type = MCU_NONE_KABUKIZ;

	memory_configure_bank(machine, "bank3", 0, 8, &SOUND[0x10000], 0x4000);
}

DRIVER_INIT( insectx )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	state->mcu_type = MCU_NONE_INSECTX;

	/* this game has no mcu, replace the handler with plain input port handlers */
	memory_install_read_port(cputag_get_address_space(machine, "sub", ADDRESS_SPACE_PROGRAM), 0xc000, 0xc000, 0, 0, "IN0" );
	memory_install_read_port(cputag_get_address_space(machine, "sub", ADDRESS_SPACE_PROGRAM), 0xc001, 0xc001, 0, 0, "IN1" );
	memory_install_read_port(cputag_get_address_space(machine, "sub", ADDRESS_SPACE_PROGRAM), 0xc002, 0xc002, 0, 0, "IN2" );
}

DRIVER_INIT( kageki )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	state->mcu_type = MCU_NONE_KAGEKI;
}


READ8_HANDLER( tnzs_mcu_r )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;
	switch (state->mcu_type)
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

WRITE8_HANDLER( tnzs_mcu_w )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;
	switch (state->mcu_type)
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
	tnzs_state *state = (tnzs_state *)device->machine->driver_data;
	int coin;

	switch (state->mcu_type)
	{
		case MCU_ARKANOID:
		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
			coin  = 0;
			coin |= ((input_port_read(device->machine, "COIN1") & 1) << 0);
			coin |= ((input_port_read(device->machine, "COIN2") & 1) << 1);
			coin |= ((input_port_read(device->machine, "IN2") & 3) << 2);
			coin ^= 0x0c;
			mcu_handle_coins(device->machine, coin);
			break;
		default:
			break;
	}

	cpu_set_input_line(device, 0, HOLD_LINE);
}

MACHINE_RESET( tnzs )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	/* initialize the mcu simulation */
	switch (state->mcu_type)
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

	state->screenflip = 0;
	state->kageki_csport_sel = 0;
	state->input_select = 0;
	state->mcu_readcredits = 0;	// this might belong to mcu_reset
	state->insertcoin = 0;		// this might belong to mcu_reset
}

MACHINE_RESET( jpopnics )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;

	state->screenflip = 0;
	state->mcu_type = -1;
}

static STATE_POSTLOAD( tnzs_postload )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	memory_set_bank(machine, "bank1", state->bank1);
	memory_set_bank(machine, "bank2", state->bank2);

	if (state->bank1 <= 1)
		memory_install_write_bank(space, 0x8000, 0xbfff, 0, 0, "bank1");
	else
		memory_unmap_write(space, 0x8000, 0xbfff, 0, 0);
}

MACHINE_START( tnzs )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");
	UINT8 *SUB = memory_region(machine, "sub");

	memory_configure_bank(machine, "bank1", 0, 8, &ROM[0x10000], 0x4000);
	memory_configure_bank(machine, "bank2", 0, 4, &SUB[0x10000], 0x2000);

	memory_set_bank(machine, "bank1", 2);
	memory_set_bank(machine, "bank2", 0);

	state->bank1 = 2;
	state->bank2 = 0;

	state->audiocpu = devtag_get_device(machine, "audiocpu");
	state->subcpu = devtag_get_device(machine, "sub");
	state->mcu = devtag_get_device(machine, "mcu");

	state_save_register_global(machine, state->screenflip);
	state_save_register_global(machine, state->kageki_csport_sel);
	state_save_register_global(machine, state->input_select);
	state_save_register_global(machine, state->mcu_readcredits);
	state_save_register_global(machine, state->insertcoin);
	state_save_register_global(machine, state->mcu_initializing);
	state_save_register_global(machine, state->mcu_coinage_init);
	state_save_register_global_array(machine, state->mcu_coinage);
	state_save_register_global(machine, state->mcu_coins_a);
	state_save_register_global(machine, state->mcu_coins_b);
	state_save_register_global(machine, state->mcu_credits);
	state_save_register_global(machine, state->mcu_reportcoin);
	state_save_register_global(machine, state->mcu_command);
	state_save_register_global(machine, state->bank1);
	state_save_register_global(machine, state->bank2);

	state_save_register_postload(machine, tnzs_postload, NULL);
}

MACHINE_START( jpopnics )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");
	UINT8 *SUB = memory_region(machine, "sub");

	memory_configure_bank(machine, "bank1", 0, 8, &ROM[0x10000], 0x4000);
	memory_configure_bank(machine, "bank2", 0, 4, &SUB[0x10000], 0x2000);

	state->subcpu = devtag_get_device(machine, "sub");
	state->mcu = NULL;

	state->bank1 = 2;
	state->bank2 = 0;

	state_save_register_global(machine, state->screenflip);
	state_save_register_global(machine, state->bank1);
	state_save_register_global(machine, state->bank2);

	state_save_register_postload(machine, tnzs_postload, NULL);
}


WRITE8_HANDLER( tnzs_bankswitch_w )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;

//  logerror("PC %04x: writing %02x to bankswitch\n", cpu_get_pc(space->cpu),data);

	/* bit 4 resets the second CPU */
	if (data & 0x10)
		cpu_set_input_line(state->subcpu, INPUT_LINE_RESET, CLEAR_LINE);
	else
		cpu_set_input_line(state->subcpu, INPUT_LINE_RESET, ASSERT_LINE);

	/* bits 0-2 select RAM/ROM bank */
	state->bank1 = data & 0x07;
	memory_set_bank(space->machine, "bank1", state->bank1);

	if (state->bank1 <= 1)
		memory_install_write_bank(space, 0x8000, 0xbfff, 0, 0, "bank1");
	else
		memory_unmap_write(space, 0x8000, 0xbfff, 0, 0);
}

WRITE8_HANDLER( tnzs_bankswitch1_w )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;
//  logerror("PC %04x: writing %02x to bankswitch 1\n", cpu_get_pc(space->cpu),data);

	switch (state->mcu_type)
	{
		case MCU_TNZS:
		case MCU_CHUKATAI:
				/* bit 2 resets the mcu */
				if (data & 0x04)
				{
					if (state->mcu != NULL && state->mcu->type() == I8742)
						cpu_set_input_line(state->mcu, INPUT_LINE_RESET, PULSE_LINE);
				}
				/* Coin count and lockout is handled by the i8742 */
				break;
		case MCU_NONE_INSECTX:
				coin_lockout_w(space->machine, 0, (~data & 0x04));
				coin_lockout_w(space->machine, 1, (~data & 0x08));
				coin_counter_w(space->machine, 0, (data & 0x10));
				coin_counter_w(space->machine, 1, (data & 0x20));
				break;
		case MCU_NONE_TNZSB:
		case MCU_NONE_KABUKIZ:
				coin_lockout_w(space->machine, 0, (~data & 0x10));
				coin_lockout_w(space->machine, 1, (~data & 0x20));
				coin_counter_w(space->machine, 0, (data & 0x04));
				coin_counter_w(space->machine, 1, (data & 0x08));
				break;
		case MCU_NONE_KAGEKI:
				coin_lockout_global_w(space->machine, (~data & 0x20));
				coin_counter_w(space->machine, 0, (data & 0x04));
				coin_counter_w(space->machine, 1, (data & 0x08));
				break;
		case MCU_ARKANOID:
		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
				/* bit 2 resets the mcu */
				if (data & 0x04)
					mcu_reset(space->machine);
				break;
		default:
				break;
	}

	/* bits 0-1 select ROM bank */
	state->bank2 = data & 0x03;
	memory_set_bank(space->machine, "bank2", state->bank2);
}
