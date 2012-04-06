/*************************************************************************

    Driver for Williams/Midway Wolf-unit games.

**************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/m6809/m6809.h"
#include "audio/dcs.h"
#include "includes/midtunit.h"
#include "includes/midxunit.h"
#include "midwayic.h"


/* prototype */
static void midxunit_dcs_output_full(running_machine &machine, int state);



/*************************************
 *
 *  State saving
 *
 *************************************/

static void register_state_saving(running_machine &machine)
{
	midxunit_state *state = machine.driver_data<midxunit_state>();
	state_save_register_global(machine, state->m_cmos_write_enable);
	state_save_register_global_array(machine, state->m_iodata);
	state_save_register_global_array(machine, state->m_ioshuffle);
	state_save_register_global(machine, state->m_analog_port);
	state_save_register_global_array(machine, state->m_uart);
	state_save_register_global(machine, state->m_security_bits);
}



/*************************************
 *
 *  CMOS reads/writes
 *
 *************************************/

READ16_MEMBER(midxunit_state::midxunit_cmos_r)
{
	return m_nvram[offset];
}

WRITE16_MEMBER(midxunit_state::midxunit_cmos_w)
{
	COMBINE_DATA(m_nvram+offset);
}


/*************************************
 *
 *  General I/O writes
 *
 *************************************/

WRITE16_MEMBER(midxunit_state::midxunit_io_w)
{
	int oldword, newword;

	offset = (offset / 2) % 8;
	oldword = m_iodata[offset];
	newword = oldword;
	COMBINE_DATA(&newword);

	switch (offset)
	{
		case 2:
			/* watchdog reset */
//          watchdog_reset_w(0,0);
			break;

		default:
			/* Gun Outputs for RevX */
			/* Note: The Gun for the Coin slot you use is supposed to rumble when you insert coins, and it doesn't for P3 */
			/* Perhaps an Input is hooked up wrong??? */
			output_set_value("Player1_Gun_Recoil", data & 0x1 );
			output_set_value("Player2_Gun_Recoil", (data & 0x2) >> 1 );
			output_set_value("Player3_Gun_Recoil", (data & 0x4) >> 2 );
			output_set_value("Player1_Gun_LED", (~data & 0x10) >> 4 );
			output_set_value("Player2_Gun_LED", (~data & 0x20) >> 5 );
			output_set_value("Player3_Gun_LED", (~data & 0x40) >> 6 );

			logerror("%08X:I/O write to %d = %04X\n", cpu_get_pc(&space.device()), offset, data);
//          logerror("%08X:Unknown I/O write to %d = %04X\n", cpu_get_pc(&space.device()), offset, data);
			break;
	}
	m_iodata[offset] = newword;
}


WRITE16_MEMBER(midxunit_state::midxunit_unknown_w)
{
	int offs = offset / 0x40000;

	if (offs == 1 && ACCESSING_BITS_0_7)
		dcs_reset_w(machine(), data & 2);

	if (ACCESSING_BITS_0_7 && offset % 0x40000 == 0)
		logerror("%08X:midxunit_unknown_w @ %d = %02X\n", cpu_get_pc(&space.device()), offs, data & 0xff);
}



/*************************************
 *
 *  General I/O reads
 *
 *************************************/

READ16_MEMBER(midxunit_state::midxunit_io_r)
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "DSW" };

	offset = (offset / 2) % 8;

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return input_port_read(machine(), portnames[offset]);

		default:
			logerror("%08X:Unknown I/O read from %d\n", cpu_get_pc(&space.device()), offset);
			break;
	}
	return ~0;
}


READ16_MEMBER(midxunit_state::midxunit_analog_r)
{
	static const char *const portnames[] = { "AN0", "AN1", "AN2", "AN3", "AN4", "AN5" };

	return input_port_read(machine(), portnames[m_analog_port]);
}


WRITE16_MEMBER(midxunit_state::midxunit_analog_select_w)
{
	if (offset == 0 && ACCESSING_BITS_0_7)
		m_analog_port = data - 8;
}


READ16_MEMBER(midxunit_state::midxunit_status_r)
{
	/* low bit indicates whether the ADC is done reading the current input */
	return (midway_serial_pic_status_r() << 1) | 1;
}



/*************************************
 *
 *  Revolution X UART
 *
 *************************************/

static void midxunit_dcs_output_full(running_machine &machine, int state)
{
	midxunit_state *drvstate = machine.driver_data<midxunit_state>();
	/* only signal if not in loopback state */
	if (drvstate->m_uart[1] != 0x66)
		cputag_set_input_line(machine, "maincpu", 1, state ? ASSERT_LINE : CLEAR_LINE);
}


READ16_MEMBER(midxunit_state::midxunit_uart_r)
{
	int result = 0;

	/* convert to a byte offset */
	if (offset & 1)
		return 0;
	offset /= 2;

	/* switch off the offset */
	switch (offset)
	{
		case 0:	/* register 0 must return 0x13 in order to pass the self test */
			result = 0x13;
			break;

		case 1:	/* register 1 contains the status */

			/* loopback case: data always ready, and always ok to send */
			if (m_uart[1] == 0x66)
				result |= 5;

			/* non-loopback case: bit 0 means data ready, bit 2 means ok to send */
			else
			{
				int temp = midxunit_sound_state_r(space, 0, 0xffff);
				result |= (temp & 0x800) >> 9;
				result |= (~temp & 0x400) >> 10;
				machine().scheduler().synchronize();
			}
			break;

		case 3:	/* register 3 contains the data read */

			/* loopback case: feed back last data wrtten */
			if (m_uart[1] == 0x66)
				result = m_uart[3];

			/* non-loopback case: read from the DCS system */
			else
				result = midxunit_sound_r(space, 0, 0xffff);
			break;

		case 5:	/* register 5 seems to be like 3, but with in/out swapped */

			/* loopback case: data always ready, and always ok to send */
			if (m_uart[1] == 0x66)
				result |= 5;

			/* non-loopback case: bit 0 means data ready, bit 2 means ok to send */
			else
			{
				int temp = midxunit_sound_state_r(space, 0, 0xffff);
				result |= (temp & 0x800) >> 11;
				result |= (~temp & 0x400) >> 8;
				machine().scheduler().synchronize();
			}
			break;

		default: /* everyone else reads themselves */
			result = m_uart[offset];
			break;
	}

/*  logerror("%08X:UART R @ %X = %02X\n", cpu_get_pc(&space.device()), offset, result);*/
	return result;
}


WRITE16_MEMBER(midxunit_state::midxunit_uart_w)
{
	/* convert to a byte offset, ignoring MSB writes */
	if ((offset & 1) || !ACCESSING_BITS_0_7)
		return;
	offset /= 2;
	data &= 0xff;

	/* switch off the offset */
	switch (offset)
	{
		case 3:	/* register 3 contains the data to be sent */

			/* loopback case: don't feed through */
			if (m_uart[1] == 0x66)
				m_uart[3] = data;

			/* non-loopback case: send to the DCS system */
			else
				midxunit_sound_w(space, 0, data, mem_mask);
			break;

		case 5:	/* register 5 write seems to reset things */
			dcs_data_r(machine());
			break;

		default: /* everyone else just stores themselves */
			m_uart[offset] = data;
			break;
	}

/*  logerror("%08X:UART W @ %X = %02X\n", cpu_get_pc(&space.device()), offset, data);*/
}



/*************************************
 *
 *  X-unit init (DCS)
 *
 *  music: ADSP2101
 *
 *************************************/

/********************** Revolution X **********************/

DRIVER_INIT( revx )
{
	midxunit_state *state = machine.driver_data<midxunit_state>();
	UINT8 *base;
	int i, j, len;

	/* register for state saving */
	register_state_saving(machine);

	/* load the graphics ROMs -- quadruples */
	midtunit_gfx_rom = base = machine.region("gfx1")->base();
	len = machine.region("gfx1")->bytes();
	for (i = 0; i < len / 0x200000; i++)
	{
		memcpy(state->m_decode_memory, base, 0x200000);
		for (j = 0; j < 0x80000; j++)
		{
			*base++ = state->m_decode_memory[0x000000 + j];
			*base++ = state->m_decode_memory[0x080000 + j];
			*base++ = state->m_decode_memory[0x100000 + j];
			*base++ = state->m_decode_memory[0x180000 + j];
		}
	}

	/* init sound */
	dcs_init(machine);

	/* serial prefixes 419, 420 */
	midway_serial_pic_init(machine, 419);
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_RESET( midxunit )
{
	midxunit_state *state = machine.driver_data<midxunit_state>();
	int i;

	/* reset sound */
	dcs_reset_w(machine, 1);
	dcs_reset_w(machine, 0);

	/* reset I/O shuffling */
	for (i = 0; i < 16; i++)
		state->m_ioshuffle[i] = i % 8;

	dcs_set_io_callbacks(midxunit_dcs_output_full, NULL);
}



/*************************************
 *
 *  Security chip I/O
 *
 *************************************/

READ16_MEMBER(midxunit_state::midxunit_security_r)
{
	return midway_serial_pic_r(&space);
}

WRITE16_MEMBER(midxunit_state::midxunit_security_w)
{
	if (ACCESSING_BITS_0_7)
		m_security_bits = data & 0x0f;
}


WRITE16_MEMBER(midxunit_state::midxunit_security_clock_w)
{
	if (offset == 0 && ACCESSING_BITS_0_7)
		midway_serial_pic_w(&space, ((~data & 2) << 3) | m_security_bits);
}



/*************************************
 *
 *  Sound write handlers
 *
 *************************************/

READ16_MEMBER(midxunit_state::midxunit_sound_r)
{
	logerror("%08X:Sound read\n", cpu_get_pc(&space.device()));

	return dcs_data_r(machine()) & 0xff;
}


READ16_MEMBER(midxunit_state::midxunit_sound_state_r)
{
	return dcs_control_r(machine());
}


WRITE16_MEMBER(midxunit_state::midxunit_sound_w)
{
	/* check for out-of-bounds accesses */
	if (offset)
	{
		logerror("%08X:Unexpected write to sound (hi) = %04X\n", cpu_get_pc(&space.device()), data);
		return;
	}

	/* call through based on the sound type */
	if (ACCESSING_BITS_0_7)
	{
		logerror("%08X:Sound write = %04X\n", cpu_get_pc(&space.device()), data);
		dcs_data_w(machine(), data & 0xff);
	}
}
