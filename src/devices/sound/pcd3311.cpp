// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    PCD3311 DTMF/modem/musical tone generator emulation

    TODO:
    - derive tone frequencies from clock (ROM dividers need extracting)

**********************************************************************/

#include "emu.h"
#include "pcd3311.h"

#define LOG_DATA (1U << 1)
#define LOG_LINE (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PCD3311, pcd3311_device, "pcd3311", "PCD3311")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pcd3311_device - constructor
//-------------------------------------------------

pcd3311_device::pcd3311_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PCD3311, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_slave_address(PCD3311_SLAVE_ADDRESS)
	, m_scl(0)
	, m_sdaw(0)
	, m_sdar(1)
	, m_state(STATE_IDLE)
	, m_bits(0)
	, m_shift(0)
	, m_devsel(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pcd3311_device::device_start()
{
	m_stream = stream_alloc(0, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);

	m_mode = 0;
	m_strobe = 0;
	m_data = 0;

	m_freq[0] = m_freq[1] = 0;
	m_incr[0] = m_incr[1] = 0;
	m_signal[0] = m_signal[1] = 1.0;

	save_item(NAME(m_mode));
	save_item(NAME(m_strobe));
	save_item(NAME(m_data));

	save_item(NAME(m_scl));
	save_item(NAME(m_sdaw));
	save_item(NAME(m_sdar));
	save_item(NAME(m_state));
	save_item(NAME(m_bits));
	save_item(NAME(m_shift));
	save_item(NAME(m_devsel));

	save_item(NAME(m_freq));
	save_item(NAME(m_incr));
	save_item(NAME(m_signal));
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void pcd3311_device::sound_stream_update(sound_stream &stream)
{
	if (m_freq[0] == 0)
		return;

	// fill in the sample
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		for (int dac = 0; dac < 2; dac++)
		{
			m_incr[dac] -= m_freq[dac];
			while (m_incr[dac] < 0)
			{
				m_incr[dac] += stream.sample_rate() / 2;
				m_signal[dac] = -m_signal[dac];
			}
			stream.add(0, sampindex, m_signal[dac]);
		}
	}
}


void pcd3311_device::load_data(uint8_t data)
{
	m_stream->update();

	static const float  dtmf_tones[12] { 697.90, 770.46, 850.45, 943.23, 1206.45, 1341.66, 1482.21, 1638.24, 1875.1, 1970.0, 2358.1, 2470.4 };
	static const float modem_tones[12] { 1296.94, 2103.14, 1197.17, 2192.01, 978.82, 1179.03, 1073.33, 1265.30, 1655.66, 1852.77, 2021.20, 2223.32 };
	static const float music_tones[16] { 622.5, 659.5, 697.9, 741.1, 782.1, 832.3, 879.3, 931.9, 985.0, 1044.5, 1111.7, 1245.1, 1318.9, 1402.1, 1572.0, 1768.5 };

	m_freq[0] = m_freq[1] = 0;

	switch (data & 0x30)
	{
	case 0x00: // DTMF single tones; musical tones
		switch (data & 0x0f)
		{
		case 0x04: m_freq[0] = dtmf_tones[8]; break;
		case 0x05: m_freq[0] = dtmf_tones[9]; break;
		case 0x06: m_freq[0] = dtmf_tones[10]; break;
		case 0x07: m_freq[0] = dtmf_tones[11]; break;
		case 0x08: m_freq[0] = dtmf_tones[0]; break;
		case 0x09: m_freq[0] = dtmf_tones[1]; break;
		case 0x0a: m_freq[0] = dtmf_tones[2]; break;
		case 0x0b: m_freq[0] = dtmf_tones[3]; break;
		case 0x0c: m_freq[0] = dtmf_tones[4]; break;
		case 0x0d: m_freq[0] = dtmf_tones[5]; break;
		case 0x0e: m_freq[0] = dtmf_tones[6]; break;
		case 0x0f: m_freq[0] = dtmf_tones[7]; break;
		}
		break;
	case 0x10: // DTMF dual tones
		switch (data & 0x0f)
		{
		case 0x00: m_freq[0] = dtmf_tones[3]; m_freq[1] = dtmf_tones[5]; break;
		case 0x01: m_freq[0] = dtmf_tones[0]; m_freq[1] = dtmf_tones[4]; break;
		case 0x02: m_freq[0] = dtmf_tones[0]; m_freq[1] = dtmf_tones[5]; break;
		case 0x03: m_freq[0] = dtmf_tones[0]; m_freq[1] = dtmf_tones[6]; break;
		case 0x04: m_freq[0] = dtmf_tones[1]; m_freq[1] = dtmf_tones[4]; break;
		case 0x05: m_freq[0] = dtmf_tones[1]; m_freq[1] = dtmf_tones[5]; break;
		case 0x06: m_freq[0] = dtmf_tones[1]; m_freq[1] = dtmf_tones[6]; break;
		case 0x07: m_freq[0] = dtmf_tones[2]; m_freq[1] = dtmf_tones[4]; break;
		case 0x08: m_freq[0] = dtmf_tones[2]; m_freq[1] = dtmf_tones[5]; break;
		case 0x09: m_freq[0] = dtmf_tones[2]; m_freq[1] = dtmf_tones[6]; break;
		case 0x0a: m_freq[0] = dtmf_tones[0]; m_freq[1] = dtmf_tones[7]; break;
		case 0x0b: m_freq[0] = dtmf_tones[1]; m_freq[1] = dtmf_tones[7]; break;
		case 0x0c: m_freq[0] = dtmf_tones[2]; m_freq[1] = dtmf_tones[7]; break;
		case 0x0d: m_freq[0] = dtmf_tones[3]; m_freq[1] = dtmf_tones[7]; break;
		case 0x0e: m_freq[0] = dtmf_tones[3]; m_freq[1] = dtmf_tones[4]; break;
		case 0x0f: m_freq[0] = dtmf_tones[3]; m_freq[1] = dtmf_tones[6]; break;
		}
		break;
	case 0x20: // modem tones
		switch (data & 0x0f)
		{
		case 0x04: m_freq[0] = modem_tones[0]; break;
		case 0x05: m_freq[0] = modem_tones[1]; break;
		case 0x06: m_freq[0] = modem_tones[2]; break;
		case 0x07: m_freq[0] = modem_tones[3]; break;
		case 0x08: m_freq[0] = modem_tones[4]; break;
		case 0x09: m_freq[0] = modem_tones[5]; break;
		case 0x0a: m_freq[0] = modem_tones[6]; break;
		case 0x0b: m_freq[0] = modem_tones[7]; break;
		case 0x0c: m_freq[0] = modem_tones[8]; break;
		case 0x0d: m_freq[0] = modem_tones[9]; break;
		case 0x0e: m_freq[0] = modem_tones[10]; break;
		case 0x0f: m_freq[0] = modem_tones[11]; break;
		}
		break;
	case 0x30: // musical tones
		m_freq[0] = music_tones[data & 0x0f];
		break;
	}

	// restart waves from beginning
	m_incr[0] = m_incr[1] = 0;
	m_signal[0] = m_signal[1] = 1.0;
}


void pcd3311_device::a0_w(int state)
{
	state &= 1;
	if (BIT(m_slave_address, 1) != state)
	{
		LOGMASKED(LOG_LINE, "set a0 %d\n", state );
		m_slave_address = (m_slave_address & 0xfd) | (state << 1);
	}
}


void pcd3311_device::strobe_w(int state)
{
	if (m_strobe && !state)
		load_data(m_data);

	m_strobe = state;
}


void pcd3311_device::scl_w(int state)
{
	if (m_scl != state)
	{
		m_scl = state;
		LOGMASKED(LOG_LINE, "set_scl_line %d\n", m_scl);

		switch (m_state)
		{
		case STATE_DEVSEL:
		case STATE_DATAIN:
			if (m_bits < 8)
			{
				if (m_scl)
				{
					m_shift = ((m_shift << 1) | m_sdaw) & 0xff;
					m_bits++;
				}
			}
			else
			{
				if (m_scl)
				{
					m_bits++;
				}
				else
				{
					if (m_bits == 8)
					{
						switch (m_state)
						{
						case STATE_DEVSEL:
							m_devsel = m_shift;

							if ((m_devsel & 0xfe) != m_slave_address)
							{
								LOGMASKED(LOG_DATA, "devsel %02x: not this device\n", m_devsel);
								m_state = STATE_IDLE;
							}
							else if ((m_devsel & 1) == 0)
							{
								LOGMASKED(LOG_DATA, "devsel %02x: write\n", m_devsel);
								m_state = STATE_DATAIN;
							}
							break;

						case STATE_DATAIN:
							load_data(m_shift);

							LOGMASKED(LOG_DATA, "data %02x\n", m_shift);
							break;
						}

						if( m_state != STATE_IDLE )
						{
							m_sdar = 0 ;
						}
					}
					else
					{
						m_bits = 0;
						m_sdar = 1;
					}
				}
			}
			break;
		}
	}
}

void pcd3311_device::sda_w(int state)
{
	state &= 1;
	if (m_sdaw != state)
	{
		LOGMASKED(LOG_LINE, "set sda %d\n", state);
		m_sdaw = state;

		if (m_scl)
		{
			if (m_sdaw)
			{
				LOGMASKED(LOG_DATA, "stop\n");
				m_state = STATE_IDLE;
			}
			else
			{
				LOGMASKED(LOG_DATA, "start\n");
				m_state = STATE_DEVSEL;
				m_bits = 0;
			}

			m_sdar = 1;
		}
	}
}

int pcd3311_device::sda_r()
{
	int res = m_sdar & 1;

	LOGMASKED(LOG_LINE, "read sda %d\n", res);

	return res;
}
