// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * Common handling of the cassette input by 
 * the SERPROC (BBC) and the main system ULA (electron).
 * 
 ****************************************************************************/

#include "emu.h"
#include "bbc_elk_casin.h"


DEFINE_DEVICE_TYPE(BBC_ELK_CASIN, bbc_elk_casin_device, "bbc_elk_casin", "BBC/Electron CASIN")


bbc_elk_casin_device::bbc_elk_casin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_ELK_CASIN, tag, owner, clock)
{
}


void bbc_elk_casin_device::device_start()
{
	save_item(NAME(m_last_tap_val));
	save_item(NAME(m_tap_val_length));
	save_item(NAME(m_len));
	save_item(NAME(m_casin));
	save_item(NAME(m_timeout));
}


void bbc_elk_casin_device::device_reset()
{
	reset();
}


void bbc_elk_casin_device::reset()
{
	m_last_tap_val = 0.0;
	m_tap_val_length = 0;
	m_casin = 0;
	for (int i = 0; i <= 3; i++)
		m_len[i] = 0;
}


bool bbc_elk_casin_device::input(double tap_val)
{
	bool bit_received = false;
	if ((tap_val >= 0.0 && m_last_tap_val < 0.0) || (tap_val < 0.0 && m_last_tap_val >= 0.0))
	{
		if (m_tap_val_length > (9 * 3))
		{
			for (int i = 0; i <= 3; i++)
				m_len[i] = 0;
			m_tap_val_length = 0;
			m_timeout = true;
		}
		else
		{
			m_timeout = false;
		}

		for (int i = 3; i > 0; i--)
			m_len[i] = m_len[i-1];
		m_len[0] = m_tap_val_length;

		m_tap_val_length = 0;

		if ((m_len[0] + m_len[1]) >= (18 + 18 - 5))
		{
			m_casin = 0;
			bit_received = true;

			for (int i = 0; i <= 3; i++)
				m_len[i] = 0;
		}

		if (((m_len[0] + m_len[1] + m_len[2] + m_len[3]) <= (18 + 18 + 5)) && (m_len[3] != 0))
		{
			m_casin = 1;
			bit_received = true;

			for (int i = 0; i <= 3; i++)
				m_len[i] = 0;
		}
	}
	m_tap_val_length++;
	m_last_tap_val = tap_val;
	return bit_received;
}
