/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/m6805/m6805.h"
#include "includes/taitosj.h"


#define VERBOSE	1
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


MACHINE_START( taitosj )
{
	taitosj_state *state = machine->driver_data<taitosj_state>();
	memory_configure_bank(machine, "bank1", 0, 1, machine->region("maincpu")->base() + 0x6000, 0);
	memory_configure_bank(machine, "bank1", 1, 1, machine->region("maincpu")->base() + 0x10000, 0);

	state->save_item(NAME(state->fromz80));
	state->save_item(NAME(state->toz80));
	state->save_item(NAME(state->zaccept));
	state->save_item(NAME(state->zready));
	state->save_item(NAME(state->busreq));

	state->save_item(NAME(state->portA_in));
	state->save_item(NAME(state->portA_out));
	state->save_item(NAME(state->address));
	state->save_item(NAME(state->spacecr_prot_value));
	state->save_item(NAME(state->protection_value));
}

MACHINE_RESET( taitosj )
{
	taitosj_state *state = machine->driver_data<taitosj_state>();
	address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	/* set the default ROM bank (many games only have one bank and */
	/* never write to the bank selector register) */
	taitosj_bankswitch_w(space, 0, 0);


	state->zaccept = 1;
	state->zready = 0;
	state->busreq = 0;
	if (machine->device("mcu") != NULL)
		cputag_set_input_line(machine, "mcu", 0, CLEAR_LINE);

	state->spacecr_prot_value = 0;
}


WRITE8_HANDLER( taitosj_bankswitch_w )
{
	coin_lockout_global_w(space->machine, ~data & 1);

	if(data & 0x80) memory_set_bank(space->machine, "bank1", 1);
	else memory_set_bank(space->machine, "bank1", 0);
}



/***************************************************************************

                           PROTECTION HANDLING

 Some of the games running on this hardware are protected with a 68705 mcu.
 It can either be on a daughter board containing Z80+68705+one ROM, which
 replaces the Z80 on an unprotected main board; or it can be built-in on the
 main board. The two are fucntionally equivalent.

 The 68705 can read commands from the Z80, send back result codes, and has
 direct access to the Z80 memory space. It can also trigger IRQs on the Z80.

***************************************************************************/
READ8_HANDLER( taitosj_fake_data_r )
{
	LOG(("%04x: protection read\n",cpu_get_pc(space->cpu)));
	return 0;
}

WRITE8_HANDLER( taitosj_fake_data_w )
{
	LOG(("%04x: protection write %02x\n",cpu_get_pc(space->cpu),data));
}

READ8_HANDLER( taitosj_fake_status_r )
{
	LOG(("%04x: protection status read\n",cpu_get_pc(space->cpu)));
	return 0xff;
}


/* timer callback : */
READ8_HANDLER( taitosj_mcu_data_r )
{
	taitosj_state *state = space->machine->driver_data<taitosj_state>();
	LOG(("%04x: protection read %02x\n",cpu_get_pc(space->cpu),state->toz80));
	state->zaccept = 1;
	return state->toz80;
}

/* timer callback : */
static TIMER_CALLBACK( taitosj_mcu_real_data_w )
{
	taitosj_state *state = machine->driver_data<taitosj_state>();
	state->zready = 1;
	cputag_set_input_line(machine, "mcu", 0, ASSERT_LINE);
	state->fromz80 = param;
}

WRITE8_HANDLER( taitosj_mcu_data_w )
{
	LOG(("%04x: protection write %02x\n",cpu_get_pc(space->cpu),data));
	space->machine->scheduler().synchronize(FUNC(taitosj_mcu_real_data_w), data);
	/* temporarily boost the interleave to sync things up */
	space->machine->scheduler().boost_interleave(attotime::zero, attotime::from_usec(10));
}

READ8_HANDLER( taitosj_mcu_status_r )
{
	taitosj_state *state = space->machine->driver_data<taitosj_state>();
	/* temporarily boost the interleave to sync things up */
	space->machine->scheduler().boost_interleave(attotime::zero, attotime::from_usec(10));

	/* bit 0 = the 68705 has read data from the Z80 */
	/* bit 1 = the 68705 has written data for the Z80 */
	return ~((state->zready << 0) | (state->zaccept << 1));
}

READ8_HANDLER( taitosj_68705_portA_r )
{
	taitosj_state *state = space->machine->driver_data<taitosj_state>();
	LOG(("%04x: 68705 port A read %02x\n",cpu_get_pc(space->cpu),state->portA_in));
	return state->portA_in;
}

WRITE8_HANDLER( taitosj_68705_portA_w )
{
	taitosj_state *state = space->machine->driver_data<taitosj_state>();
	LOG(("%04x: 68705 port A write %02x\n",cpu_get_pc(space->cpu),data));
	state->portA_out = data;
}



/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   W  !68INTRQ
 *  1   W  !68LRD (enables latch which holds command from the Z80)
 *  2   W  !68LWR (loads the latch which holds data for the Z80, and sets a
 *                 status bit so the Z80 knows there's data waiting)
 *  3   W  to Z80 !BUSRQ (aka !WAIT) pin
 *  4   W  !68WRITE (triggers write to main Z80 memory area and increases low
 *                   8 bits of the latched address)
 *  5   W  !68READ (triggers read from main Z80 memory area and increases low
 *                   8 bits of the latched address)
 *  6   W  !LAL (loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access)
 *  7   W  !UAL (loads the latch which holds the high 8 bits of the address of
 *               the main Z80 memory location to access)
 */

READ8_HANDLER( taitosj_68705_portB_r )
{
	return 0xff;
}

/* timer callback : 68705 is going to read data from the Z80 */
static TIMER_CALLBACK( taitosj_mcu_data_real_r )
{
	taitosj_state *state = machine->driver_data<taitosj_state>();
	state->zready = 0;
}

/* timer callback : 68705 is writing data for the Z80 */
static TIMER_CALLBACK( taitosj_mcu_status_real_w )
{
	taitosj_state *state = machine->driver_data<taitosj_state>();
	state->toz80 = param;
	state->zaccept = 0;
}

WRITE8_HANDLER( taitosj_68705_portB_w )
{
	taitosj_state *state = space->machine->driver_data<taitosj_state>();
	LOG(("%04x: 68705 port B write %02x\n", cpu_get_pc(space->cpu), data));

	if (~data & 0x01)
	{
		LOG(("%04x: 68705  68INTRQ **NOT SUPPORTED**!\n", cpu_get_pc(space->cpu)));
	}
	if (~data & 0x02)
	{
		/* 68705 is going to read data from the Z80 */
		space->machine->scheduler().synchronize(FUNC(taitosj_mcu_data_real_r));
		cputag_set_input_line(space->machine, "mcu", 0, CLEAR_LINE);
		state->portA_in = state->fromz80;
		LOG(("%04x: 68705 <- Z80 %02x\n", cpu_get_pc(space->cpu), state->portA_in));
	}
	if (~data & 0x08)
		state->busreq = 1;
	else
		state->busreq = 0;
	if (~data & 0x04)
	{
		LOG(("%04x: 68705 -> Z80 %02x\n", cpu_get_pc(space->cpu), state->portA_out));

		/* 68705 is writing data for the Z80 */
		space->machine->scheduler().synchronize(FUNC(taitosj_mcu_status_real_w), state->portA_out);
	}
	if (~data & 0x10)
	{
		address_space *cpu0space = cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
		LOG(("%04x: 68705 write %02x to address %04x\n",cpu_get_pc(space->cpu), state->portA_out, state->address));

		cpu0space->write_byte(state->address, state->portA_out);

		/* increase low 8 bits of latched address for burst writes */
		state->address = (state->address & 0xff00) | ((state->address + 1) & 0xff);
	}
	if (~data & 0x20)
	{
		address_space *cpu0space = cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
		state->portA_in = cpu0space->read_byte(state->address);
		LOG(("%04x: 68705 read %02x from address %04x\n", cpu_get_pc(space->cpu), state->portA_in, state->address));
	}
	if (~data & 0x40)
	{
		LOG(("%04x: 68705 address low %02x\n", cpu_get_pc(space->cpu), state->portA_out));
		state->address = (state->address & 0xff00) | state->portA_out;
	}
	if (~data & 0x80)
	{
		LOG(("%04x: 68705 address high %02x\n", cpu_get_pc(space->cpu), state->portA_out));
		state->address = (state->address & 0x00ff) | (state->portA_out << 8);
	}
}

/*
 *  Port C connections:
 *
 *  0   R  ZREADY (1 when the Z80 has written a command in the latch)
 *  1   R  ZACCEPT (1 when the Z80 has read data from the latch)
 *  2   R  from Z80 !BUSAK pin
 *  3   R  68INTAK (goes 0 when the interrupt request done with 68INTRQ
 *                  passes through)
 */

READ8_HANDLER( taitosj_68705_portC_r )
{
	taitosj_state *state = space->machine->driver_data<taitosj_state>();
	int res;

	res = (state->zready << 0) | (state->zaccept << 1) | ((state->busreq^1) << 2);
	LOG(("%04x: 68705 port C read %02x\n",cpu_get_pc(space->cpu),res));
	return res;
}


/* Space Cruiser protection (otherwise the game resets on the asteroids level) */

READ8_HANDLER( spacecr_prot_r )
{
	taitosj_state *state = space->machine->driver_data<taitosj_state>();
	int pc = cpu_get_pc(space->cpu);

	if( pc != 0x368A && pc != 0x36A6 )
		logerror("Read protection from an unknown location: %04X\n",pc);

	state->spacecr_prot_value ^= 0xff;

	return state->spacecr_prot_value;
}


/* Alpine Ski protection crack routines */

WRITE8_HANDLER( alpine_protection_w )
{
	taitosj_state *state = space->machine->driver_data<taitosj_state>();
	switch (data)
	{
	case 0x05:
		state->protection_value = 0x18;
		break;
	case 0x07:
	case 0x0c:
	case 0x0f:
		state->protection_value = 0x00;		/* not used as far as I can tell */
		break;
	case 0x16:
		state->protection_value = 0x08;
		break;
	case 0x1d:
		state->protection_value = 0x18;
		break;
	default:
		state->protection_value = data;		/* not used as far as I can tell */
		break;
	}
}

WRITE8_HANDLER( alpinea_bankswitch_w )
{
	taitosj_state *state = space->machine->driver_data<taitosj_state>();
    taitosj_bankswitch_w(space, offset, data);
	state->protection_value = data >> 2;
}

READ8_HANDLER( alpine_port_2_r )
{
	taitosj_state *state = space->machine->driver_data<taitosj_state>();
	return input_port_read(space->machine, "IN2") | state->protection_value;
}
