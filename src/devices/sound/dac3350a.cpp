// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
    Micronas DAC 3550A Stereo Audio DAC
    i2c code based on mas3507d

    Accepts 8-bit and 16-bit I2C write operations and has no read operations
*/

#include "emu.h"
#include "dac3350a.h"

#define VERBOSE      (LOG_GENERAL)

#include "logmacro.h"

enum
{
	CMD_DEV_WRITE = 0x9a,
};

enum
{
	IDLE = 0,
	STARTED,
	NAK,
	ACK,
	ACK2,
};

enum
{
	UNKNOWN = 0,
	VALIDATED,
	WRONG,
};

enum
{
	REG_UNKNOWN = 0,
	REG_SR = 1,
	REG_AVOL,
	REG_GCFG,
};

enum
{
	// sample rate control register
	SR_LR_SEL_BIT = 4, // L/R-bit
	SR_LR_SEL_LEFT = 0, // (WSI = 0 -> left channel
	SR_LR_SEL_RIGHT = 1, // (WSI = 0 -> right channel

	SR_SP_SEL_BIT = 3, // delay bit
	SR_SP_SEL_NO_DELAY = 0,
	SR_SP_SEL_1_BIT_DELAY = 1,

	SR_SRC_BIT = 0, // 3 bits wide, sample rate control
	SR_SRC_48 = 0, // 32-48 kHz
	SR_SRC_32, // 26-32 kHz
	SR_SRC_24, // 20-26 kHz
	SR_SRC_16, // 14-20 kHz
	SR_SRC_12, // 10-14 kHz
	SR_SRC_8, // 8-10 kHz
	SR_SRC_A, // autoselect
};

enum
{
	// analog volume register
	AVOL_DEEM_BIT = 14, // deemphasis on/off
	AVOL_DEEM_OFF = 0,
	AVOL_DEEM_ON = 1,

	AVOL_L_BIT = 8, // 6 bits wide, analog audio volume level left
	AVOL_R_BIT = 0, // 6 bits wide, analog audio volume level right
};

enum
{
	// global configuration register
	GCFG_SEL_53V_BIT = 6, // select 3V-5V mode
	GCFG_SEL_53V_3V = 0,
	GCFG_SEL_53V_5V = 1,

	GCFG_PWMD_BIT = 5, // power-mode
	GCFG_PWMD_NORMAL = 0,
	GCFG_PWMD_LOW_POWER = 1,

	GCFG_INSEL_AUX2_BIT = 4, // AUX2 select
	GCFG_INSEL_AUX2_OFF = 0,
	GCFG_INSEL_AUX2_ON = 1,

	GCFG_INSEL_AUX1_BIT = 3, // AUX1 select
	GCFG_INSEL_AUX1_OFF = 0,
	GCFG_INSEL_AUX1_ON = 1,

	GCFG_INSEL_DAC_BIT = 2, // DAC select
	GCFG_INSEL_DAC_OFF = 0,
	GCFG_INSEL_DAC_ON = 1,

	GCFG_AUX_MS_BIT = 1, // aux-mono/stereo
	GCFG_AUX_MS_STEREO = 0,
	GCFG_AUX_MS_MONO = 1,

	GCFG_IPRA_BIT = 0, // invert right power amplifier
	GCFG_IPRA_NOT_INVERTED = 0,
	GCFG_IPRA_INVERTED = 1,
};


DEFINE_DEVICE_TYPE(DAC3350A, dac3350a_device, "dac3350a", "Micronas DAC 3550A Stereo Audio DAC")

dac3350a_device::dac3350a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DAC3350A, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}

void dac3350a_device::device_start()
{
	// TODO: use configured clock for sample rate and respond to device_clock_changed
	m_stream = stream_alloc(2, 2, 44100);

	save_item(NAME(m_i2c_bus_state));
	save_item(NAME(m_i2c_bus_address));
	save_item(NAME(m_i2c_scli));
	save_item(NAME(m_i2c_sdai));
	save_item(NAME(m_i2c_bus_curbit));
	save_item(NAME(m_i2c_bus_curval));
	save_item(NAME(m_i2c_bytecount));

	save_item(NAME(m_i2c_subadr));
	save_item(NAME(m_i2c_data));

	save_item(NAME(m_dac_enable));
	save_item(NAME(m_volume));
}

void dac3350a_device::device_reset()
{
	m_i2c_scli = m_i2c_sdai = 1;
	m_i2c_bus_state = IDLE;
	m_i2c_bus_address = UNKNOWN;
	m_i2c_bus_curbit = -1;
	m_i2c_bus_curval = 0;

	m_i2c_subadr = 0;
	m_i2c_data = 0;

	m_dac_enable = true;

	std::fill(std::begin(m_volume), std::end(m_volume), 1.0);
}

void dac3350a_device::i2c_scl_w(int line)
{
	if (line == m_i2c_scli)
		return;

	m_i2c_scli = line;

	if (m_i2c_scli)
	{
		if (m_i2c_bus_state == STARTED)
		{
			m_i2c_bus_curval |= m_i2c_sdai << m_i2c_bus_curbit;
			m_i2c_bus_curbit--;

			if (m_i2c_bus_curbit == -1)
			{
				if (m_i2c_bus_address == UNKNOWN)
				{
					if (m_i2c_bus_curval == CMD_DEV_WRITE)
					{
						m_i2c_bus_state = ACK;
						m_i2c_bus_address = VALIDATED;
						m_i2c_bus_curval = 0;
					}
					else
					{
						m_i2c_bus_state = NAK;
						m_i2c_bus_address = WRONG;
					}
				}
				else if (m_i2c_bus_address == VALIDATED)
				{
					if (m_i2c_bytecount < 3)
					{
						m_i2c_bus_state = ACK;

						if (m_i2c_bytecount == 0)
						{
							m_i2c_subadr = m_i2c_bus_curval;
						}
						else
						{
							m_i2c_data <<= 8;
							m_i2c_data |= m_i2c_bus_curval;
						}

						m_i2c_bytecount++;
					}
					else
					{
						// Only accept 8-bit and 16-bit writes
						m_i2c_bus_state = NAK;
					}
				}
			}
		}
		else if (m_i2c_bus_state == ACK)
		{
			m_i2c_bus_state = ACK2;
		}
	}
	else
	{
		if (m_i2c_bus_state == ACK2)
		{
			m_i2c_bus_state = STARTED;
			m_i2c_bus_curbit = 7;
			m_i2c_bus_curval = 0;
		}
	}
}

void dac3350a_device::i2c_sda_w(int line)
{
	if (line == m_i2c_sdai)
		return;

	m_i2c_sdai = line;

	if (m_i2c_scli)
	{
		if (m_i2c_sdai)
			i2c_device_handle_write();

		m_i2c_bus_state = m_i2c_sdai ? IDLE : STARTED;
		m_i2c_bus_address = UNKNOWN;
		m_i2c_bus_curbit = 7;
		m_i2c_bus_curval = 0;
	}
}

void dac3350a_device::i2c_device_handle_write()
{
	const auto bytecount = m_i2c_bytecount;

	m_i2c_bytecount = 0;

	if (bytecount < 2) // not a valid command, ignore
		return;

	// const int mcs = BIT(m_i2c_subadr, 6, 2); // chip select, 3 = MPEG mode
	const int reg_adr = BIT(m_i2c_subadr, 0, 2);

	if (reg_adr == REG_SR)
	{
		// sample rate control
		const int sample_rate = BIT(m_i2c_data, SR_SRC_BIT, 3);
		const int delay = BIT(m_i2c_data, SR_SP_SEL_BIT);
		const int lr = BIT(m_i2c_data, SR_LR_SEL_BIT);

		LOG("DAC: SR register %d %d %d\n", sample_rate, delay, lr);
	}
	else if (reg_adr == REG_AVOL)
	{
		// set volume, requires 2 bytes
		if (bytecount <= 2)
			logerror("AVOL command requires 2 bytes of data, found %d\n", bytecount);

		const int vol_r = BIT(m_i2c_data, AVOL_R_BIT, 6);
		const int vol_l = BIT(m_i2c_data, AVOL_L_BIT, 6);
		const int deemph = BIT(m_i2c_data, AVOL_DEEM_BIT);

		m_volume[0] = calculate_volume(vol_l);
		m_volume[1] = calculate_volume(vol_r);

		LOG("DAC: AVOL register %d %d %d (%f %f)\n", vol_r, vol_l, deemph, m_volume[0], m_volume[1]);
	}
	else if (reg_adr == REG_GCFG)
	{
		// global configuration
		const int ipra = BIT(m_i2c_data, GCFG_IPRA_BIT);
		const int aux_ms = BIT(m_i2c_data, GCFG_AUX_MS_BIT);
		const int insel_dac = BIT(m_i2c_data, GCFG_INSEL_DAC_BIT);
		const int insel_aux1 = BIT(m_i2c_data, GCFG_INSEL_AUX1_BIT);
		const int insel_aux2 = BIT(m_i2c_data, GCFG_INSEL_AUX2_BIT);
		const int pwmd = BIT(m_i2c_data, GCFG_PWMD_BIT);
		const int sel_53v = BIT(m_i2c_data, GCFG_SEL_53V_BIT);

		LOG("DAC: GCFG register %d %d %d %d %d %d %d\n", ipra, aux_ms, insel_dac, insel_aux1, insel_aux2, pwmd, sel_53v);
	}
	else
	{
		LOG("DAC: Unknown register selected! %d\n", reg_adr);
	}
}

float dac3350a_device::calculate_volume(int val)
{
	if (val == 0)
		return 0.0; // mute

	double db = 0;
	if (val <= 7)
	{
		// -75 dB ... -54 dB: 3 dB steps
		db = -54 - ((8 - val) * 3);
	}
	else
	{
		// -54 dB ... +18 dB: 1.5 dB steps
		db = -54 + ((val - 8) * 1.5);
	}

	db = std::clamp(db, -75.0, 18.0); // range is +18 dB to -75 dB

	return powf(10.0, db / 20.0);
}

void dac3350a_device::sound_stream_update(sound_stream &stream)
{
	const sound_stream::sample_t enable_scale = m_dac_enable ? 1.0 : 0.0;

	for (int channel = 0; channel < 2 && channel < stream.output_count(); channel++)
	{
		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
			stream.put(channel, sampindex, stream.get(channel, sampindex) * enable_scale * m_volume[channel]);
	}
}
