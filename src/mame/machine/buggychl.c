#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/buggychl.h"

typedef struct _buggychl_mcu_state buggychl_mcu_state;
struct _buggychl_mcu_state
{
	UINT8       m_port_a_in;
	UINT8       m_port_a_out;
	UINT8       m_ddr_a;
	UINT8       m_port_b_in;
	UINT8       m_port_b_out;
	UINT8       m_ddr_b;
	UINT8       m_port_c_in;
	UINT8       m_port_c_out;
	UINT8       m_ddr_c;
	UINT8       m_from_main;
	UINT8       m_from_mcu;
	int         m_mcu_sent;
	int         m_main_sent;
	device_t *m_mcu;
};


INLINE buggychl_mcu_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == BUGGYCHL_MCU);

	return (buggychl_mcu_state *)downcast<buggychl_mcu_device *>(device)->token();
}

/***************************************************************************

 Buggy Challenge 68705 protection interface

 This is accurate. FairyLand Story seems to be identical.

***************************************************************************/

static READ8_DEVICE_HANDLER( buggychl_68705_port_a_r )
{
	buggychl_mcu_state *state = get_safe_token(device);
	//logerror("%04x: 68705 port A read %02x\n", state->m_mcu->safe_pc(), state->m_port_a_in);
	return (state->m_port_a_out & state->m_ddr_a) | (state->m_port_a_in & ~state->m_ddr_a);
}

static WRITE8_DEVICE_HANDLER( buggychl_68705_port_a_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	//logerror("%04x: 68705 port A write %02x\n", state->m_mcu->safe_pc(), data);
	state->m_port_a_out = data;
}

static WRITE8_DEVICE_HANDLER( buggychl_68705_ddr_a_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	state->m_ddr_a = data;
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
	return (state->m_port_b_out & state->m_ddr_b) | (state->m_port_b_in & ~state->m_ddr_b);
}

static WRITE8_DEVICE_HANDLER( buggychl_68705_port_b_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	logerror("%04x: 68705 port B write %02x\n", state->m_mcu->safe_pc(), data);

	if ((state->m_ddr_b & 0x02) && (~data & 0x02) && (state->m_port_b_out & 0x02))
	{
		state->m_port_a_in = state->m_from_main;
		if (state->m_main_sent)
			device_set_input_line(state->m_mcu, 0, CLEAR_LINE);
		state->m_main_sent = 0;
		logerror("read command %02x from main cpu\n", state->m_port_a_in);
	}
	if ((state->m_ddr_b & 0x04) && (data & 0x04) && (~state->m_port_b_out & 0x04))
	{
		logerror("send command %02x to main cpu\n", state->m_port_a_out);
		state->m_from_mcu = state->m_port_a_out;
		state->m_mcu_sent = 1;
	}

	state->m_port_b_out = data;
}

static WRITE8_DEVICE_HANDLER( buggychl_68705_ddr_b_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	state->m_ddr_b = data;
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
	state->m_port_c_in = 0;
	if (state->m_main_sent)
		state->m_port_c_in |= 0x01;
	if (!state->m_mcu_sent)
		state->m_port_c_in |= 0x02;
	logerror("%04x: 68705 port C read %02x\n", state->m_mcu->safe_pc(), state->m_port_c_in);
	return (state->m_port_c_out & state->m_ddr_c) | (state->m_port_c_in & ~state->m_ddr_c);
}

static WRITE8_DEVICE_HANDLER( buggychl_68705_port_c_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	logerror("%04x: 68705 port C write %02x\n", state->m_mcu->safe_pc(), data);
	state->m_port_c_out = data;
}

static WRITE8_DEVICE_HANDLER( buggychl_68705_ddr_c_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	state->m_ddr_c = data;
}


WRITE8_DEVICE_HANDLER( buggychl_mcu_w )
{
	buggychl_mcu_state *state = get_safe_token(device);
	logerror("%04x: mcu_w %02x\n", state->m_mcu->safe_pc(), data);
	state->m_from_main = data;
	state->m_main_sent = 1;
	device_set_input_line(state->m_mcu, 0, ASSERT_LINE);
}

READ8_DEVICE_HANDLER( buggychl_mcu_r )
{
	buggychl_mcu_state *state = get_safe_token(device);
	logerror("%04x: mcu_r %02x\n", state->m_mcu->safe_pc(), state->m_from_mcu);
	state->m_mcu_sent = 0;
	return state->m_from_mcu;
}

READ8_DEVICE_HANDLER( buggychl_mcu_status_r )
{
	buggychl_mcu_state *state = get_safe_token(device);
	int res = 0;

	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	/* bit 1 = when 1, mcu has sent data to the main cpu */
	//logerror("%04x: mcu_status_r\n",state->m_mcu->safe_pc());
	if (!state->m_main_sent)
		res |= 0x01;
	if (state->m_mcu_sent)
		res |= 0x02;

	return res;
}

ADDRESS_MAP_START( buggychl_mcu_map, AS_PROGRAM, 8, buggychl_mcu_device )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_DEVREADWRITE_LEGACY("bmcu", buggychl_68705_port_a_r, buggychl_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_DEVREADWRITE_LEGACY("bmcu", buggychl_68705_port_b_r, buggychl_68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_DEVREADWRITE_LEGACY("bmcu", buggychl_68705_port_c_r, buggychl_68705_port_c_w)
	AM_RANGE(0x0004, 0x0004) AM_DEVWRITE_LEGACY("bmcu", buggychl_68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_DEVWRITE_LEGACY("bmcu", buggychl_68705_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_DEVWRITE_LEGACY("bmcu", buggychl_68705_ddr_c_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END

static DEVICE_START( buggychl_mcu )
{
	buggychl_mcu_state *state = get_safe_token(device);

	state->m_mcu = device->machine().device("mcu");

	device->save_item(NAME(state->m_from_main));
	device->save_item(NAME(state->m_from_mcu));
	device->save_item(NAME(state->m_mcu_sent));
	device->save_item(NAME(state->m_main_sent));
	device->save_item(NAME(state->m_port_a_in));
	device->save_item(NAME(state->m_port_a_out));
	device->save_item(NAME(state->m_ddr_a));
	device->save_item(NAME(state->m_port_b_in));
	device->save_item(NAME(state->m_port_b_out));
	device->save_item(NAME(state->m_ddr_b));
	device->save_item(NAME(state->m_port_c_in));
	device->save_item(NAME(state->m_port_c_out));
	device->save_item(NAME(state->m_ddr_c));
}

static DEVICE_RESET( buggychl_mcu )
{
	buggychl_mcu_state *state = get_safe_token(device);

	state->m_mcu_sent = 0;
	state->m_main_sent = 0;
	state->m_from_main = 0;
	state->m_from_mcu = 0;
	state->m_port_a_in = 0;
	state->m_port_a_out = 0;
	state->m_ddr_a = 0;
	state->m_port_b_in = 0;
	state->m_port_b_out = 0;
	state->m_ddr_b = 0;
	state->m_port_c_in = 0;
	state->m_port_c_out = 0;
	state->m_ddr_c = 0;
}

const device_type BUGGYCHL_MCU = &device_creator<buggychl_mcu_device>;

buggychl_mcu_device::buggychl_mcu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, BUGGYCHL_MCU, "BuggyChl MCU", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(buggychl_mcu_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void buggychl_mcu_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void buggychl_mcu_device::device_start()
{
	DEVICE_START_NAME( buggychl_mcu )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void buggychl_mcu_device::device_reset()
{
	DEVICE_RESET_NAME( buggychl_mcu )(this);
}


