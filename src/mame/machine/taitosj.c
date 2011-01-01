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


static UINT8 fromz80,toz80;
static UINT8 zaccept,zready,busreq;
static UINT8 portA_in,portA_out;

static UINT8 spacecr_prot_value;
static UINT8 protection_value;
static UINT32 address;

MACHINE_START( taitosj )
{
	memory_configure_bank(machine, "bank1", 0, 1, machine->region("maincpu")->base() + 0x6000, 0);
	memory_configure_bank(machine, "bank1", 1, 1, machine->region("maincpu")->base() + 0x10000, 0);

	state_save_register_global(machine, fromz80);
	state_save_register_global(machine, toz80);
	state_save_register_global(machine, zaccept);
	state_save_register_global(machine, zready);
	state_save_register_global(machine, busreq);

	state_save_register_global(machine, portA_in);
	state_save_register_global(machine, portA_out);
	state_save_register_global(machine, address);
	state_save_register_global(machine, spacecr_prot_value);
	state_save_register_global(machine, protection_value);
}

MACHINE_RESET( taitosj )
{
	address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	/* set the default ROM bank (many games only have one bank and */
	/* never write to the bank selector register) */
	taitosj_bankswitch_w(space, 0, 0);


	zaccept = 1;
	zready = 0;
	busreq = 0;
	if (machine->device("mcu") != NULL)
		cputag_set_input_line(machine, "mcu", 0, CLEAR_LINE);

	spacecr_prot_value = 0;
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
	LOG(("%04x: protection read %02x\n",cpu_get_pc(space->cpu),toz80));
	zaccept = 1;
	return toz80;
}

/* timer callback : */
static TIMER_CALLBACK( taitosj_mcu_real_data_w )
{
	zready = 1;
	cputag_set_input_line(machine, "mcu", 0, ASSERT_LINE);
	fromz80 = param;
}

WRITE8_HANDLER( taitosj_mcu_data_w )
{
	LOG(("%04x: protection write %02x\n",cpu_get_pc(space->cpu),data));
	timer_call_after_resynch(space->machine, NULL, data,taitosj_mcu_real_data_w);
	/* temporarily boost the interleave to sync things up */
	cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(10));
}

READ8_HANDLER( taitosj_mcu_status_r )
{
	/* temporarily boost the interleave to sync things up */
	cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(10));

	/* bit 0 = the 68705 has read data from the Z80 */
	/* bit 1 = the 68705 has written data for the Z80 */
	return ~((zready << 0) | (zaccept << 1));
}

READ8_HANDLER( taitosj_68705_portA_r )
{
	LOG(("%04x: 68705 port A read %02x\n",cpu_get_pc(space->cpu),portA_in));
	return portA_in;
}

WRITE8_HANDLER( taitosj_68705_portA_w )
{
	LOG(("%04x: 68705 port A write %02x\n",cpu_get_pc(space->cpu),data));
	portA_out = data;
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
	zready = 0;
}

/* timer callback : 68705 is writing data for the Z80 */
static TIMER_CALLBACK( taitosj_mcu_status_real_w )
{
	toz80 = param;
	zaccept = 0;
}

WRITE8_HANDLER( taitosj_68705_portB_w )
{
	LOG(("%04x: 68705 port B write %02x\n", cpu_get_pc(space->cpu), data));

	if (~data & 0x01)
	{
		LOG(("%04x: 68705  68INTRQ **NOT SUPPORTED**!\n", cpu_get_pc(space->cpu)));
	}
	if (~data & 0x02)
	{
		/* 68705 is going to read data from the Z80 */
		timer_call_after_resynch(space->machine, NULL, 0, taitosj_mcu_data_real_r);
		cputag_set_input_line(space->machine, "mcu", 0, CLEAR_LINE);
		portA_in = fromz80;
		LOG(("%04x: 68705 <- Z80 %02x\n", cpu_get_pc(space->cpu), portA_in));
	}
	if (~data & 0x08)
		busreq = 1;
	else
		busreq = 0;
	if (~data & 0x04)
	{
		LOG(("%04x: 68705 -> Z80 %02x\n", cpu_get_pc(space->cpu), portA_out));

		/* 68705 is writing data for the Z80 */
		timer_call_after_resynch(space->machine, NULL, portA_out, taitosj_mcu_status_real_w);
	}
	if (~data & 0x10)
	{
		address_space *cpu0space = cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
		LOG(("%04x: 68705 write %02x to address %04x\n",cpu_get_pc(space->cpu), portA_out, address));

		cpu0space->write_byte(address, portA_out);

		/* increase low 8 bits of latched address for burst writes */
		address = (address & 0xff00) | ((address + 1) & 0xff);
	}
	if (~data & 0x20)
	{
		address_space *cpu0space = cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
		portA_in = cpu0space->read_byte(address);
		LOG(("%04x: 68705 read %02x from address %04x\n", cpu_get_pc(space->cpu), portA_in, address));
	}
	if (~data & 0x40)
	{
		LOG(("%04x: 68705 address low %02x\n", cpu_get_pc(space->cpu), portA_out));
		address = (address & 0xff00) | portA_out;
	}
	if (~data & 0x80)
	{
		LOG(("%04x: 68705 address high %02x\n", cpu_get_pc(space->cpu), portA_out));
		address = (address & 0x00ff) | (portA_out << 8);
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
	int res;

	res = (zready << 0) | (zaccept << 1) | ((busreq^1) << 2);
	LOG(("%04x: 68705 port C read %02x\n",cpu_get_pc(space->cpu),res));
	return res;
}


/* Space Cruiser protection (otherwise the game resets on the asteroids level) */

READ8_HANDLER( spacecr_prot_r )
{
	int pc = cpu_get_pc(space->cpu);

	if( pc != 0x368A && pc != 0x36A6 )
		logerror("Read protection from an unknown location: %04X\n",pc);

	spacecr_prot_value ^= 0xff;

	return spacecr_prot_value;
}


/* Alpine Ski protection crack routines */

WRITE8_HANDLER( alpine_protection_w )
{
	switch (data)
	{
	case 0x05:
		protection_value = 0x18;
		break;
	case 0x07:
	case 0x0c:
	case 0x0f:
		protection_value = 0x00;		/* not used as far as I can tell */
		break;
	case 0x16:
		protection_value = 0x08;
		break;
	case 0x1d:
		protection_value = 0x18;
		break;
	default:
		protection_value = data;		/* not used as far as I can tell */
		break;
	}
}

WRITE8_HANDLER( alpinea_bankswitch_w )
{
    taitosj_bankswitch_w(space, offset, data);
	protection_value = data >> 2;
}

READ8_HANDLER( alpine_port_2_r )
{
	return input_port_read(space->machine, "IN2") | protection_value;
}
