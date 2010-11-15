#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/buggychl.h"

typedef struct _buggychl_mcu_state buggychl_mcu_state;
struct _buggychl_mcu_state
{
	UINT8       port_a_in, port_a_out, ddr_a;
	UINT8       port_b_in, port_b_out, ddr_b;
	UINT8       port_c_in, port_c_out, ddr_c;
	UINT8       from_main, from_mcu;
	int         mcu_sent, main_sent;
	running_device *mcu;
};


INLINE buggychl_mcu_state *get_safe_token( running_device *device )
{
	assert(device != NULL);
	assert(device->type() == BUGGYCHL_MCU);

	return (buggychl_mcu_state *)downcast<legacy_device_base *>(device)->token();
}

/***************************************************************************

 Buggy Challenge 68705 protection interface

 This is accurate. FairyLand Story seems to be identical.

***************************************************************************/

static READ8_DEVICE_HANDLER( buggychl_68705_port_a_r )
{
	buggychl_mcu_state *state = get_safe_token(device);
	//logerror("%04x: 68705 port A read %02x\n", cpu_get_pc(state->mcu), state->port_a_in);
	return (state->port_a_out & state->ddr_a) | (state->port_a_in & ~state->ddr_a);
}

static WRITE8_DEVICE_HANDLER( buggychl_68705_port_a_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	//logerror("%04x: 68705 port A write %02x\n", cpu_get_pc(state->mcu), data);
	state->port_a_out = data;
}

static WRITE8_DEVICE_HANDLER( buggychl_68705_ddr_a_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	state->ddr_a = data;
}



/*
 *  Port B connections:
 *  parts in [ ] are optional (not used by buggychl)
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   n.c.
 *  1   W  IRQ ack and enable latch which holds data from main Z80 memory
 *  2   W  loads latch to Z80
 *  3   W  to Z80 BUSRQ (put it on hold?)
 *  4   W  n.c.
 *  5   W  [selects Z80 memory access direction (0 = write 1 = read)]
 *  6   W  [loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access]
 *  7   W  [loads the latch which holds the high 8 bits of the address of
 *               the main Z80 memory location to access]
 */


static READ8_DEVICE_HANDLER( buggychl_68705_port_b_r )
{
	buggychl_mcu_state *state = get_safe_token(device);
	return (state->port_b_out & state->ddr_b) | (state->port_b_in & ~state->ddr_b);
}

static WRITE8_DEVICE_HANDLER( buggychl_68705_port_b_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	logerror("%04x: 68705 port B write %02x\n", cpu_get_pc(state->mcu), data);

	if ((state->ddr_b & 0x02) && (~data & 0x02) && (state->port_b_out & 0x02))
	{
		state->port_a_in = state->from_main;
		if (state->main_sent)
			cpu_set_input_line(state->mcu, 0, CLEAR_LINE);
		state->main_sent = 0;
		logerror("read command %02x from main cpu\n", state->port_a_in);
	}
	if ((state->ddr_b & 0x04) && (data & 0x04) && (~state->port_b_out & 0x04))
	{
		logerror("send command %02x to main cpu\n", state->port_a_out);
		state->from_mcu = state->port_a_out;
		state->mcu_sent = 1;
	}

	state->port_b_out = data;
}

static WRITE8_DEVICE_HANDLER( buggychl_68705_ddr_b_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	state->ddr_b = data;
}


/*
 *  Port C connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   R  1 when pending command Z80->68705
 *  1   R  0 when pending command 68705->Z80
 */

static READ8_DEVICE_HANDLER( buggychl_68705_port_c_r )
{
	buggychl_mcu_state *state = get_safe_token(device);
	state->port_c_in = 0;
	if (state->main_sent)
		state->port_c_in |= 0x01;
	if (!state->mcu_sent)
		state->port_c_in |= 0x02;
	logerror("%04x: 68705 port C read %02x\n", cpu_get_pc(state->mcu), state->port_c_in);
	return (state->port_c_out & state->ddr_c) | (state->port_c_in & ~state->ddr_c);
}

static WRITE8_DEVICE_HANDLER( buggychl_68705_port_c_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	logerror("%04x: 68705 port C write %02x\n", cpu_get_pc(state->mcu), data);
	state->port_c_out = data;
}

static WRITE8_DEVICE_HANDLER( buggychl_68705_ddr_c_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	state->ddr_c = data;
}


WRITE8_DEVICE_HANDLER( buggychl_mcu_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	logerror("%04x: mcu_w %02x\n", cpu_get_pc(state->mcu), data);
	state->from_main = data;
	state->main_sent = 1;
	cpu_set_input_line(state->mcu, 0, ASSERT_LINE);
}

READ8_DEVICE_HANDLER( buggychl_mcu_r )
{
	buggychl_mcu_state *state = get_safe_token(device);
	logerror("%04x: mcu_r %02x\n", cpu_get_pc(state->mcu), state->from_mcu);
	state->mcu_sent = 0;
	return state->from_mcu;
}

READ8_DEVICE_HANDLER( buggychl_mcu_status_r )
{
	buggychl_mcu_state *state = get_safe_token(device);
	int res = 0;

	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	/* bit 1 = when 1, mcu has sent data to the main cpu */
	//logerror("%04x: mcu_status_r\n",cpu_get_pc(state->mcu));
	if (!state->main_sent)
		res |= 0x01;
	if (state->mcu_sent)
		res |= 0x02;

	return res;
}

ADDRESS_MAP_START( buggychl_mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_DEVREADWRITE("bmcu", buggychl_68705_port_a_r, buggychl_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_DEVREADWRITE("bmcu", buggychl_68705_port_b_r, buggychl_68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_DEVREADWRITE("bmcu", buggychl_68705_port_c_r, buggychl_68705_port_c_w)
	AM_RANGE(0x0004, 0x0004) AM_DEVWRITE("bmcu", buggychl_68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_DEVWRITE("bmcu", buggychl_68705_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_DEVWRITE("bmcu", buggychl_68705_ddr_c_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END

static DEVICE_START( buggychl_mcu )
{
	buggychl_mcu_state *state = get_safe_token(device);

	state->mcu = device->machine->device("mcu");

	state_save_register_device_item(device, 0, state->from_main);
	state_save_register_device_item(device, 0, state->from_mcu);
	state_save_register_device_item(device, 0, state->mcu_sent);
	state_save_register_device_item(device, 0, state->main_sent);
	state_save_register_device_item(device, 0, state->port_a_in);
	state_save_register_device_item(device, 0, state->port_a_out);
	state_save_register_device_item(device, 0, state->ddr_a);
	state_save_register_device_item(device, 0, state->port_b_in);
	state_save_register_device_item(device, 0, state->port_b_out);
	state_save_register_device_item(device, 0, state->ddr_b);
	state_save_register_device_item(device, 0, state->port_c_in);
	state_save_register_device_item(device, 0, state->port_c_out);
	state_save_register_device_item(device, 0, state->ddr_c);
}

static DEVICE_RESET( buggychl_mcu )
{
	buggychl_mcu_state *state = get_safe_token(device);

	state->mcu_sent = 0;
	state->main_sent = 0;
	state->from_main = 0;
	state->from_mcu = 0;
	state->port_a_in = 0;
	state->port_a_out = 0;
	state->ddr_a = 0;
	state->port_b_in = 0;
	state->port_b_out = 0;
	state->ddr_b = 0;
	state->port_c_in = 0;
	state->port_c_out = 0;
	state->ddr_c = 0;
}

/*****************************************************************************
    DEVICE DEFINITION
*****************************************************************************/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)				p##buggychl_mcu##s
#define DEVTEMPLATE_FEATURES			DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME				"BuggyChl MCU"
#define DEVTEMPLATE_FAMILY				"BuggyChl MCU IC"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(BUGGYCHL_MCU, buggychl_mcu);
