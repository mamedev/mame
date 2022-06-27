// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Williams/Midway X-unit games.

**************************************************************************/

#include "emu.h"
#include "midxunit.h"

#define LOG_IO      (1 << 0)
#define LOG_UART    (1 << 1)
#define LOG_UNKNOWN (1 << 2)
#define LOG_SOUND   (1 << 3)

#define VERBOSE     (0)
#include "logmacro.h"


/*************************************
 *
 *  CMOS reads/writes
 *
 *************************************/

uint8_t midxunit_state::midxunit_cmos_r(offs_t offset)
{
	return m_nvram_data[offset];
}

void midxunit_state::midxunit_cmos_w(offs_t offset, uint8_t data)
{
	m_nvram_data[offset] = data;
}


/*************************************
 *
 *  General I/O writes
 *
 *************************************/

void midxunit_state::midxunit_io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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
			// Gun Outputs for RevX
			// Note: The Gun for the Coin slot you use is supposed to rumble when you insert coins, and it doesn't for P3.
			// Perhaps an Input is hooked up wrong.
			m_gun_recoil[0] = BIT(data, 0);
			m_gun_recoil[1] = BIT(data, 1);
			m_gun_recoil[2] = BIT(data, 2);
			m_gun_led[0] = BIT(~data, 4);
			m_gun_led[1] = BIT(~data, 5);
			m_gun_led[2] = BIT(~data, 6);

			LOGMASKED(LOG_IO, "%s: I/O write to %d = %04X\n", machine().describe_context(), offset, data);
			break;
	}
	m_iodata[offset] = newword;
}


void midxunit_state::midxunit_unknown_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int offs = offset / 0x40000;

	if (offs == 1 && ACCESSING_BITS_0_7)
		m_dcs->reset_w(~data & 2);

	if (ACCESSING_BITS_0_7 && offset % 0x40000 == 0)
		LOGMASKED(LOG_UNKNOWN, "%s: midxunit_unknown_w @ %d = %02X\n", machine().describe_context(), offs, data & 0xff);
}


WRITE_LINE_MEMBER(midxunit_state::adc_int_w)
{
	m_adc_int = (state != CLEAR_LINE);
}



/*************************************
 *
 *  General I/O reads
 *
 *************************************/

uint32_t midxunit_state::midxunit_status_r()
{
	/* low bit indicates whether the ADC is done reading the current input */
	return (m_pic_status << 1) | (m_adc_int ? 1 : 0);
}



/*************************************
 *
 *  Revolution X UART
 *
 *************************************/

WRITE_LINE_MEMBER(midxunit_state::midxunit_dcs_output_full)
{
	/* only signal if not in loopback state */
	if (m_uart[1] != 0x66)
		m_maincpu->set_input_line(1, state ? ASSERT_LINE : CLEAR_LINE);
}


uint8_t midxunit_state::midxunit_uart_r(offs_t offset)
{
	uint8_t result = 0;

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
				int temp = m_dcs->control_r();
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
			{
				LOGMASKED(LOG_SOUND, "%08X:Sound read\n", m_maincpu->pc());

				result = m_dcs->data_r();
			}
			break;

		case 5: /* register 5 seems to be like 3, but with in/out swapped */

			/* loopback case: data always ready, and always ok to send */
			if (m_uart[1] == 0x66)
				result |= 5;

			/* non-loopback case: bit 0 means data ready, bit 2 means ok to send */
			else
			{
				int temp = m_dcs->control_r();
				result |= (temp & 0x800) >> 11;
				result |= (~temp & 0x400) >> 8;
				machine().scheduler().synchronize();
			}
			break;

		default: /* everyone else reads themselves */
			result = m_uart[offset];
			break;
	}

	LOGMASKED(LOG_UART, "%s: UART R @ %X = %02X\n", machine().describe_context(), offset, result);
	return result;
}


void midxunit_state::midxunit_uart_w(offs_t offset, uint8_t data)
{
	/* switch off the offset */
	switch (offset)
	{
		case 3: /* register 3 contains the data to be sent */

			/* loopback case: don't feed through */
			if (m_uart[1] == 0x66)
				m_uart[3] = data;

			/* non-loopback case: send to the DCS system */
			else
				m_dcs->data_w(data);
			break;

		case 5: /* register 5 write seems to reset things */
			m_dcs->data_r();
			break;

		default: /* everyone else just stores themselves */
			m_uart[offset] = data;
			break;
	}

	LOGMASKED(LOG_UART, "%s: UART W @ %X = %02X\n", machine().describe_context(), offset, data);
}



/*************************************
 *
 *  X-unit init (DCS)
 *
 *  music: ADSP2101
 *
 *************************************/

/********************** Revolution X **********************/

/*************************************
 *
 *  Machine init
 *
 *************************************/

void midxunit_state::machine_start()
{
	m_gun_recoil.resolve();
	m_gun_led.resolve();

	m_nvram_data = std::make_unique<uint8_t[]>(0x2000);
	m_nvram->set_base(m_nvram_data.get(), 0x2000);

	save_item(NAME(m_cmos_write_enable));
	save_item(NAME(m_iodata));
	save_item(NAME(m_uart));
	save_item(NAME(m_adc_int));
	save_pointer(NAME(m_nvram_data), 0x2000);

	save_item(NAME(m_pic_command));
	save_item(NAME(m_pic_data));
	save_item(NAME(m_pic_clk));
	save_item(NAME(m_pic_status));
}

void midxunit_state::machine_reset()
{
	/* reset sound */
	m_dcs->reset_w(0);
	m_dcs->reset_w(1);

	m_pic_command = 0;
	m_pic_data = 0;
	m_pic_clk = 0;
	m_pic_status = 0;

	m_dcs->set_io_callbacks(write_line_delegate(*this, FUNC(midxunit_state::midxunit_dcs_output_full)), write_line_delegate(*this));
}



/*************************************
 *
 *  Security chip I/O
 *
 *************************************/

uint32_t midxunit_state::midxunit_security_r()
{
	return m_pic_data;
}

void midxunit_state::midxunit_security_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_pic_command = data & 0x0f;
}


void midxunit_state::midxunit_security_clock_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_pic_clk = BIT(data, 1);
}



/*************************************
 *
 *  DMA registers (inverted word select)
 *
 *************************************/

uint32_t midxunit_state::midxunit_dma_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;

	if (ACCESSING_BITS_16_31)
		result |= m_video->midtunit_dma_r(offset * 2);
	if (ACCESSING_BITS_0_15)
		result |= uint32_t(m_video->midtunit_dma_r(offset * 2 + 1)) << 16;

	return result;
}


void midxunit_state::midxunit_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
		m_video->midtunit_dma_w(offset * 2, data & 0xffff);
	if (ACCESSING_BITS_0_15)
		m_video->midtunit_dma_w(offset * 2 + 1, data >> 16);
}
