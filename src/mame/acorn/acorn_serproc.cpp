// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**************************************************************************************************

    Acorn Serial ULA (SERPROC)

    Two types:
    - BBC Micro   Ferranti Serial ULA (2C199E-7)
    - BBC Master  VLSI Serial Processor or SERPROC (VC2026/201648)

    TODO:
    - rewrite cassette handling, and lower the sample rate to 4800Hz

**************************************************************************************************/

#include "emu.h"
#include "acorn_serproc.h"

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(ACORN_SERPROC, acorn_serproc_device, "serproc", "Acorn Serial ULA (SERPROC)")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

acorn_serproc_device::acorn_serproc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_SERPROC, tag, owner, clock)
	, m_casin_cb(*this)
	, m_casout_cb(*this)
	, m_casmo_cb(*this)
	, m_dout_cb(*this)
	, m_rtso_cb(*this)
	, m_ctso_cb(*this)
	, m_txc_cb(*this)
	, m_rxc_cb(*this)
	, m_rxd_cb(*this)
	, m_dcd_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_serproc_device::device_start()
{
	m_casin_cb.resolve_safe(0);
	m_casout_cb.resolve_safe();

	m_rxd_cass = 0;
	m_nr_high_tones = 0;
	m_casout_enabled = 0;

	m_tape_timer = timer_alloc(FUNC(acorn_serproc_device::tape_timer), this);
	m_rxc_timer = timer_alloc(FUNC(acorn_serproc_device::rxc_timer), this);
	m_txc_timer = timer_alloc(FUNC(acorn_serproc_device::txc_timer), this);

	m_control = 0x64;

	save_item(NAME(m_control));
}


void acorn_serproc_device::write(uint8_t data)
{
	// Serial processor control
	// x--- ---- - Motor OFF(0) / ON(1)
	// -x-- ---- - Cassette(0) / RS243 input(1)
	// --xx x--- - Receive baud rate generator control
	// ---- -xxx - Transmit baud rate generator control

	static const int serial_clocks[8] =
	{
		1,    // 000 - 16MHz / 13 /   1 - 19200 baud
		16,   // 001 - 16MHz / 13 /  16 -  1200 baud
		4,    // 010 - 16MHz / 13 /   4 -  4800 baud
		128,  // 011 - 16MHz / 13 / 128 -   150 baud
		2,    // 100 - 16MHz / 13 /   2 -  9600 baud
		64,   // 101 - 16MHz / 13 /  64 -   300 baud
		8,    // 110 - 16MHz / 13 /   8 -  2400 baud
		256   // 111 - 16MHz / 13 / 256 -    75 baud
	};

	// cassette motor state changed
	if (BIT(m_control, 7) != BIT(data, 7))
	{
		m_casmo_cb(BIT(data, 7));

		if (BIT(data, 7))
		{
			m_tape_timer->adjust(attotime::zero, 0, attotime::from_hz(44100));
		}
		else
		{
			m_tape_timer->reset();
			m_len0 = 0;
			m_len1 = 0;
			m_len2 = 0;
			m_len3 = 0;
			m_wav_len = 0;
			m_casout_phase = 0;
			m_casout_samples_to_go = 4;
		}
	}

	m_control = data;

	update_rxd();
	update_dcd();
	update_ctso();

	// Set clock rates
	m_txc_timer->adjust(attotime::from_hz(clock() / serial_clocks[BIT(data, 0, 3)]), 0, attotime::from_hz(clock() / serial_clocks[BIT(data, 0, 3)]));
	m_rxc_timer->adjust(attotime::from_hz(clock() / serial_clocks[BIT(data, 3, 3)]), 0, attotime::from_hz(clock() / serial_clocks[BIT(data, 3, 3)]));
}

uint8_t acorn_serproc_device::read()
{
	// Reading has the same effect as writing 0xFE (high byte of address 0xFE10)
	write(0xfe);

	return 0;
}


void acorn_serproc_device::receive_clock(int new_clock)
{
	m_rxd_cass = new_clock;
	update_rxd();

	// Somehow the "serial processor" generates 16 clock signals towards the
	// 6850. Exact details are unknown, faking it with the following loop.
	for (int i = 0; i < 16; i++)
	{
		m_rxc_cb(1);
		m_rxc_cb(0);
	}
}

TIMER_CALLBACK_MEMBER(acorn_serproc_device::tape_timer)
{
	if (m_casout_enabled)
	{
		// 0 = 18-18 18-17-1
		// 1 = 9-9-9-9 9-9-9-8-1

		switch (m_casout_samples_to_go)
		{
		case 0:
			if (m_casout_phase == 0)
			{
				// get bit value
				m_casout_bit = m_txd;
				if (m_casout_bit)
				{
					m_casout_phase = 3;
					m_casout_samples_to_go = 9;
				}
				else
				{
					m_casout_phase = 1;
					m_casout_samples_to_go = 18;
				}
				m_casout_cb(+1.0);
			}
			else
			{
				// switch phase
				m_casout_phase--;
				m_casout_samples_to_go = m_casout_bit ? 9 : 18;
				m_casout_cb((m_casout_phase & 1) ? +1.0 : -1.0);
			}
			break;

		case 1:
			if (m_casout_phase == 0)
			{
				m_casout_cb(0.0);
			}
			break;
		}

		m_casout_samples_to_go--;
	}
	else
	{
		double dev_val = m_casin_cb();

		// look for edges on the cassette wave
		if (((dev_val >= 0.0) && (m_last_dev_val < 0.0)) || ((dev_val < 0.0) && (m_last_dev_val >= 0.0)))
		{
			if (m_wav_len > (9 * 3))
			{
				// this is too long to receive anything so reset the serial IC. This is a hack, this should be done as a timer in the MC6850 code.
				LOG("Cassette length %d\n", m_wav_len);
				m_nr_high_tones = 0;
				m_dcd_cass = 0;
				update_dcd();
				m_len0 = 0;
				m_len1 = 0;
				m_len2 = 0;
				m_len3 = 0;
				m_wav_len = 0;
			}

			m_len3 = m_len2;
			m_len2 = m_len1;
			m_len1 = m_len0;
			m_len0 = m_wav_len;

			m_wav_len = 0;
			LOG("cassette  %d  %d  %d  %d\n", m_len3, m_len2, m_len1, m_len0);

			if ((m_len0 + m_len1) >= (18 + 18 - 5))
			{
				// Clock a 0 onto the serial line
				LOG("Serial value 0\n");
				m_nr_high_tones = 0;
				m_dcd_cass = 0;
				update_dcd();
				receive_clock(0);
				m_len0 = 0;
				m_len1 = 0;
				m_len2 = 0;
				m_len3 = 0;
			}

			if (((m_len0 + m_len1 + m_len2 + m_len3) <= 41) && (m_len3 != 0))
			{
				// Clock a 1 onto the serial line
				LOG("Serial value 1\n");
				m_nr_high_tones++;
				if (m_nr_high_tones > 100)
				{
					m_dcd_cass = 1;
					update_dcd();
				}
				receive_clock(1);
				m_len0 = 0;
				m_len1 = 0;
				m_len2 = 0;
				m_len3 = 0;
			}
		}

		m_wav_len++;
		m_last_dev_val = dev_val;
	}
}

TIMER_CALLBACK_MEMBER(acorn_serproc_device::rxc_timer)
{
	if (BIT(m_control, 6))
	{
		m_rxc_cb(1);
		m_rxc_cb(0);
	}
}

TIMER_CALLBACK_MEMBER(acorn_serproc_device::txc_timer)
{
	m_txc_cb(1);
	m_txc_cb(0);
}


void acorn_serproc_device::update_rxd()
{
	m_rxd_cb(BIT(m_control, 6) ? m_rxd_serial : m_rxd_cass);
}

void acorn_serproc_device::update_dcd()
{
	m_dcd_cb(BIT(m_control, 6) ? 0 : m_dcd_cass);
}

void acorn_serproc_device::update_ctso()
{
	m_ctso_cb(BIT(m_control, 6) ? m_cts_serial : 0);
}


void acorn_serproc_device::write_txd(int state)
{
	if (BIT(m_control, 6))
	{
		m_dout_cb(state);
	}
	else
	{
		m_txd = state;
	}
}

void acorn_serproc_device::write_ctsi(int state)
{
	m_cts_serial = state;
	update_ctso();
}

void acorn_serproc_device::write_din(int state)
{
	m_rxd_serial = state;
	update_rxd();
}

void acorn_serproc_device::write_rtsi(int state)
{
	if (BIT(m_control, 6))
	{
		m_rtso_cb(state);
		m_casout_enabled = 0;
	}
	else
	{
		m_casout_enabled = state ? 0 : 1;
	}
}
