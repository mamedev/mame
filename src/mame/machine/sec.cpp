// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Serial Electronic Counter (SEC) */

#include "emu.h"
#include "sec.h"

DEFINE_DEVICE_TYPE(SEC, sec_device, "sec", "Barcrest/Bell Fruit Serial Electronic Counter (SEC)")

sec_device::sec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SEC, tag, owner, clock)
{
}

void sec_device::device_start()
{
	save_item(NAME(m_counters));
	save_item(NAME(m_strings));
	save_item(NAME(m_market));
	save_item(NAME(m_nocnt));
	save_item(NAME(m_last));

	save_item(NAME(m_curbyte));
	save_item(NAME(m_data));

	save_item(NAME(m_clk));
	save_item(NAME(m_clks));
	save_item(NAME(m_rxpos));
	save_item(NAME(m_rxclk));
	save_item(NAME(m_rxdat));
	save_item(NAME(m_rxlen));
	save_item(NAME(m_chars_left));

	save_item(NAME(m_reqpos));

	save_item(NAME(m_request));
	save_item(NAME(m_reply));

	save_item(NAME(m_enabled));
}

void sec_device::device_reset()
{
	m_clk = 1;
	m_curbyte = 0;
	m_clks = 0;
	m_reqpos = 0;
	m_rxclk = 0;
	m_rxlen = 0;
	m_rxpos = 0;
}

WRITE_LINE_MEMBER(sec_device::cs_w)
{
	if (state)
	{
		if (!m_enabled)
		{
			m_enabled = true;
			m_rxdat = 0;
		}
	}
	else
	{
		if (m_enabled)
		{
			m_rxdat = 1;
		}

		m_enabled = false;
		reset();
	}
}

WRITE_LINE_MEMBER(sec_device::data_w)
{
	m_data = (uint8_t)state;
}

int sec_device::data_r(void)
{
	return m_rxdat;
}

WRITE_LINE_MEMBER(sec_device::clk_w)
{
	state = state ? 1 : 0;

	if (m_clk ^ state)
	{
		if (!state)
		{
			m_curbyte = (m_curbyte << 1) | m_data;
			if (m_rxclk == 8)
			{
				m_rxclk = 0;
				m_rxpos++;
				m_rxlen--;
			}

			if (m_rxlen)
				m_rxdat = BIT(m_reply[m_rxpos], 7);
			else
				m_rxdat = m_enabled ? 0 : 1;
		}
		else
		{
			m_clks++;
			if (m_rxlen)
			{
				m_reply[m_rxpos] <<= 1;
				m_rxclk++;
			}

			if (m_clks == 8)
			{
				m_clks = 0;
				if (!m_rxlen)
				{
					m_request[m_reqpos++] = m_curbyte;
					if (m_chars_left)
					{
						m_chars_left--;
						if (!m_chars_left)
						{
							m_reqpos--;
							do_command();
							m_reqpos = 0;
						}
					}
					if (m_reqpos == 3)
					{
						m_chars_left = m_curbyte + 1;
					}
				}
			}
		}
		m_clk = state;
	}
}



void sec_device::do_command(void)
{
	m_last = m_request[1];
	switch (m_request[0])
	{
		case SEC_REQUEST_VERSION:    cmd_get_ver(); break;
		case SEC_REQUEST_STATUS:     cmd_get_sta(); break;
		case SEC_REQUEST_MARKET:     cmd_get_mrk(); break;
		case SEC_REQEUST_LAST_ERROR: cmd_get_err(); break;
		case SEC_REQUEST_FINGERPRNT: cmd_get_fpr(); break;
		case SEC_REQUEST_LAST_CMD:   cmd_get_lst(); break;
		case SEC_REQUEST_COUNT_VAL:  cmd_get_cnt(); break;
		case SEC_SET_NUM_COUNTERS:   cmd_set_ncn(); break;
		case SEC_SET_MARKET:         cmd_set_mrk(); break;
		case SEC_SET_COUNTER_TXT:    cmd_set_txt(); break;
		case SEC_COUNT_INC_SMALL:    cmd_inc_sml(); break;
		case SEC_COUNT_INC_MED:      cmd_inc_med(); break;
		case SEC_COUNT_INC_LARGE:    cmd_inc_lrg(); break;

		/* acknowledge these without doing anything */
		case SEC_SHOW_TEXT:          cmd_nop();     break;
		case SEC_SHOW_COUNTER_VAL:   cmd_nop();     break;
		case SEC_SHOW_COUNTER_TXT:   cmd_nop();     break;
		case SEC_SHOW_BITPATTERN:    cmd_nop();     break;
		case SEC_COUNT_CYCLE_DISP:   cmd_nop();     break;
		case SEC_STOP_CYCLE:         cmd_nop();     break;
		case SEC_SELF_TEST:          cmd_nop();     break;
	}
}

uint8_t sec_device::calc_byte_sum(int length)
{
	uint8_t csum = 0;
	for (int i = 0; i < 3 + length; i++)
	{
		csum += m_reply[i];
	}
	return csum;
}

void sec_device::cmd_get_err(void)
{
	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 1;

	m_reply[3] = 0; // Last Error

	m_reply[4] = calc_byte_sum(1);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 5;
}

void sec_device::cmd_get_fpr(void)
{
	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 4;

	// fingerprint
	m_reply[3] = 0x11;
	m_reply[4] = 0x01;
	m_reply[5] = 0x00;
	m_reply[6] = 0x00;

	m_reply[7] = calc_byte_sum(4);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 8;
}

void sec_device::cmd_get_lst(void)
{
	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 4;

	// Last Command (is this really 4 bytes?)
	m_reply[3] = m_last;
	m_reply[4] = 0x00;
	m_reply[5] = 0x00;
	m_reply[6] = 0x00;

	m_reply[7] = calc_byte_sum(4);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 8;
}

void sec_device::cmd_get_ver(void)
{
	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 3;

	// version
	m_reply[3] = '0';
	m_reply[4] = '2';
	m_reply[5] = 'E';

	m_reply[6] = calc_byte_sum(3);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 7;
}

void sec_device::cmd_get_cnt(void)
{
	char temp[10];

	sprintf(temp,"%07d0", m_counters[m_request[3]]);

	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 4;

	m_reply[3] = ((temp[0] - 0x30) << 4) + (temp[1] - 0x30);
	m_reply[4] = ((temp[2] - 0x30) << 4) + (temp[3] - 0x30);
	m_reply[5] = ((temp[4] - 0x30) << 4) + (temp[5] - 0x30);
	m_reply[6] = ((temp[6] - 0x30) << 4) + (temp[7] - 0x30);

	m_reply[7] = calc_byte_sum(4);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 8;
}

void sec_device::cmd_get_sta(void)
{
	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 1;

	m_reply[3] = 0x20; // Status

	m_reply[4] = calc_byte_sum(1);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 5;
}

void sec_device::cmd_get_mrk(void)
{
	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 1;

	m_reply[3] = m_market;

	m_reply[4] = calc_byte_sum(1);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 5;
}

void sec_device::cmd_set_ncn(void)
{
	m_nocnt = m_request[3];

	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = calc_byte_sum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}

void sec_device::cmd_set_mrk(void)
{
	m_market = m_request[3];

	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = calc_byte_sum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}

void sec_device::cmd_set_txt(void)
{
	char valbuf[8];
	uint8_t meter = m_request[3];
	memcpy(valbuf, &m_request[4], 7);
	valbuf[7] = 0;
	strcpy(m_strings[meter], valbuf);

	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = calc_byte_sum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}


void sec_device::cmd_inc_sml(void)
{
	uint8_t meter = m_request[3];
	uint8_t value = m_request[4] & 0xf;
	m_counters[meter] += value;

	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = calc_byte_sum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}

void sec_device::cmd_inc_med(void)
{
	uint8_t meter = m_request[3];
	uint8_t value = m_request[4];
	m_counters[meter] += value;

	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = calc_byte_sum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}

void sec_device::cmd_inc_lrg(void)
{
	uint8_t meter = m_request[3];
	uint8_t value = m_request[4] + 256 * m_request[5];
	m_counters[meter] += value;

	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = calc_byte_sum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}

void sec_device::cmd_nop(void)
{
	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = calc_byte_sum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}
