// license:BSD-3-Clause
// copyright-holders:Aaron Giles
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


/*************************************
 *
 *  State saving
 *
 *************************************/

void midxunit_state::register_state_saving()
{
	save_item(NAME(m_cmos_write_enable));
	save_item(NAME(m_iodata));
	save_item(NAME(m_ioshuffle));
	save_item(NAME(m_analog_port));
	save_item(NAME(m_uart));
	save_item(NAME(m_security_bits));
}



/*************************************
 *
 *  CMOS reads/writes
 *
 *************************************/

uint16_t midxunit_state::midxunit_cmos_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return m_nvram[offset];
}

void midxunit_state::midxunit_cmos_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_nvram+offset);
}


/*************************************
 *
 *  General I/O writes
 *
 *************************************/

void midxunit_state::midxunit_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
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
			output().set_value("Player1_Gun_Recoil", data & 0x1 );
			output().set_value("Player2_Gun_Recoil", (data & 0x2) >> 1 );
			output().set_value("Player3_Gun_Recoil", (data & 0x4) >> 2 );
			output().set_value("Player1_Gun_LED", (~data & 0x10) >> 4 );
			output().set_value("Player2_Gun_LED", (~data & 0x20) >> 5 );
			output().set_value("Player3_Gun_LED", (~data & 0x40) >> 6 );

			logerror("%08X:I/O write to %d = %04X\n", space.device().safe_pc(), offset, data);
//          logerror("%08X:Unknown I/O write to %d = %04X\n", space.device().safe_pc(), offset, data);
			break;
	}
	m_iodata[offset] = newword;
}


void midxunit_state::midxunit_unknown_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int offs = offset / 0x40000;

	if (offs == 1 && ACCESSING_BITS_0_7)
		m_dcs->reset_w(data & 2);

	if (ACCESSING_BITS_0_7 && offset % 0x40000 == 0)
		logerror("%08X:midxunit_unknown_w @ %d = %02X\n", space.device().safe_pc(), offs, data & 0xff);
}



/*************************************
 *
 *  General I/O reads
 *
 *************************************/

uint16_t midxunit_state::midxunit_io_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "DSW" };

	offset = (offset / 2) % 8;

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return ioport(portnames[offset])->read();

		default:
			logerror("%08X:Unknown I/O read from %d\n", space.device().safe_pc(), offset);
			break;
	}
	return ~0;
}


uint16_t midxunit_state::midxunit_analog_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	static const char *const portnames[] = { "AN0", "AN1", "AN2", "AN3", "AN4", "AN5" };

	return ioport(portnames[m_analog_port])->read();
}


void midxunit_state::midxunit_analog_select_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0 && ACCESSING_BITS_0_7)
		m_analog_port = data - 8;
}


uint16_t midxunit_state::midxunit_status_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	/* low bit indicates whether the ADC is done reading the current input */
	return (m_midway_serial_pic->status_r(space,0) << 1) | 1;
}



/*************************************
 *
 *  Revolution X UART
 *
 *************************************/

void midxunit_state::midxunit_dcs_output_full(int state)
{
	/* only signal if not in loopback state */
	if (m_uart[1] != 0x66)
		m_maincpu->set_input_line(1, state ? ASSERT_LINE : CLEAR_LINE);
}


uint16_t midxunit_state::midxunit_uart_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	int result = 0;

	/* convert to a byte offset */
	if (offset & 1)
		return 0;
	offset /= 2;

	/* switch off the offset */
	switch (offset)
	{
		case 0: /* register 0 must return 0x13 in order to pass the self test */
			result = 0x13;
			break;

		case 1: /* register 1 contains the status */

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

		case 3: /* register 3 contains the data read */

			/* loopback case: feed back last data wrtten */
			if (m_uart[1] == 0x66)
				result = m_uart[3];

			/* non-loopback case: read from the DCS system */
			else
				result = midxunit_sound_r(space, 0, 0xffff);
			break;

		case 5: /* register 5 seems to be like 3, but with in/out swapped */

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

/*  logerror("%08X:UART R @ %X = %02X\n", space.device().safe_pc(), offset, result);*/
	return result;
}


void midxunit_state::midxunit_uart_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* convert to a byte offset, ignoring MSB writes */
	if ((offset & 1) || !ACCESSING_BITS_0_7)
		return;
	offset /= 2;
	data &= 0xff;

	/* switch off the offset */
	switch (offset)
	{
		case 3: /* register 3 contains the data to be sent */

			/* loopback case: don't feed through */
			if (m_uart[1] == 0x66)
				m_uart[3] = data;

			/* non-loopback case: send to the DCS system */
			else
				midxunit_sound_w(space, 0, data, mem_mask);
			break;

		case 5: /* register 5 write seems to reset things */
			m_dcs->data_r();
			break;

		default: /* everyone else just stores themselves */
			m_uart[offset] = data;
			break;
	}

/*  logerror("%08X:UART W @ %X = %02X\n", space.device().safe_pc(), offset, data);*/
}



/*************************************
 *
 *  X-unit init (DCS)
 *
 *  music: ADSP2101
 *
 *************************************/

/********************** Revolution X **********************/

void midxunit_state::init_revx()
{
	/* register for state saving */
	register_state_saving();
}

/*************************************
 *
 *  Machine init
 *
 *************************************/

void midxunit_state::machine_reset_midxunit()
{
	int i;

	/* reset sound */
	m_dcs->reset_w(1);
	m_dcs->reset_w(0);

	/* reset I/O shuffling */
	for (i = 0; i < 16; i++)
		m_ioshuffle[i] = i % 8;

	m_dcs->set_io_callbacks(write_line_delegate(FUNC(midxunit_state::midxunit_dcs_output_full),this), write_line_delegate());
}



/*************************************
 *
 *  Security chip I/O
 *
 *************************************/

uint16_t midxunit_state::midxunit_security_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return m_midway_serial_pic->read(space,0);
}

void midxunit_state::midxunit_security_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_security_bits = data & 0x0f;
}


void midxunit_state::midxunit_security_clock_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0 && ACCESSING_BITS_0_7)
		m_midway_serial_pic->write(space, 0, ((~data & 2) << 3) | m_security_bits);
}



/*************************************
 *
 *  Sound write handlers
 *
 *************************************/

uint16_t midxunit_state::midxunit_sound_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	logerror("%08X:Sound read\n", space.device().safe_pc());

	return m_dcs->data_r() & 0xff;
}


uint16_t midxunit_state::midxunit_sound_state_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return m_dcs->control_r();
}


void midxunit_state::midxunit_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* check for out-of-bounds accesses */
	if (offset)
	{
		logerror("%08X:Unexpected write to sound (hi) = %04X\n", space.device().safe_pc(), data);
		return;
	}

	/* call through based on the sound type */
	if (ACCESSING_BITS_0_7)
	{
		logerror("%08X:Sound write = %04X\n", space.device().safe_pc(), data);
		m_dcs->data_w(data & 0xff);
	}
}
