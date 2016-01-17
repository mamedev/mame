// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Alesis HR-16 sound (DM3AG + PCM54) emulation

    TODO:
    - volume
    - panning
    - output 2

****************************************************************************/

#include "emu.h"
#include "includes/alesis.h"

#define LOG 1

// device type definition
const device_type ALESIS_DM3AG = &device_creator<alesis_dm3ag_device>;

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static MACHINE_CONFIG_FRAGMENT( alesis_dm3ag )
	MCFG_SPEAKER_STANDARD_STEREO("out1_left", "out1_right")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "out1_left",  1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "out1_right", 1.0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  alesis_dm3ag_device - constructor
//-------------------------------------------------

alesis_dm3ag_device::alesis_dm3ag_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ALESIS_DM3AG, "Alesis DM3AG", tag, owner, clock, "alesis_dm3ag", __FILE__),
		m_dac(*this, "dac"),
		m_samples(*this, DEVICE_SELF)
{
}

//-------------------------------------------------
//  device_mconfig_additions
//-------------------------------------------------

machine_config_constructor alesis_dm3ag_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( alesis_dm3ag );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void alesis_dm3ag_device::device_start()
{
	m_dac_update_timer = timer_alloc(TIMER_DAC_UPDATE);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void alesis_dm3ag_device::device_reset()
{
	m_dac_update_timer->adjust(attotime::from_hz(48000), 0, attotime::from_hz(48000));

	m_output_active = false;
	m_count = 0;
	m_cur_sample = 0;
	m_shift = 0;
	memset(m_cmd, 0, sizeof(m_cmd));
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------
void alesis_dm3ag_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (m_output_active)
	{
		INT16 sample = m_samples[m_cur_sample++];
		int count = 0;

		while (sample == -128)
		{
			count++;

			if (count == 1 && m_shift)
			{
				/*
				    The HR-16 seems to use a simple scheme to generate 16-bit samples from its 8-bit sample ROMs.
				    When the sound starts the 8-bit sample is sent to the most significant bits of the DAC and every
				    time a -1 sample is found the data is shifted one position to right.
				*/
				m_shift--;

				if (LOG)    logerror("DM3AG '%s' shift: %02x\n", tag().c_str(), m_shift);
			}

			// every block ends with three or more -1 samples
			if (m_cur_sample == 0xfffff || count >= 3)
			{
				m_output_active = false;
				sample = 0;

				if (LOG)    logerror("DM3AG '%s' stop: %d, len: %d\n", tag().c_str(), m_cur_sample, m_cur_sample-((m_cmd[0]<<12) | (m_cmd[1]<<4) | ((m_cmd[2]>>4) & 0x0f)));

				break;
			}

			sample = m_samples[m_cur_sample++];
		}

		m_dac->write_signed16((sample << m_shift) + 0x8000);
	}
}

WRITE8_MEMBER(alesis_dm3ag_device::write)
{
	if (LOG)    logerror("DM3AG '%s' write: %02x\n", tag().c_str(), data);

	m_cmd[m_count++] = data;

	if (m_count == 5)
	{
		/*
		    commands are sent in block of 5 bytes (40 bits)

		    bit 00-19       sample position in the roms
		    bit 20-23       ???
		    bit 24-31       volume
		    bit 32-34       panning
		    bit 35          output selector: 0 = out2, 1 = out1
		    bit 36-39       ???
		*/

		m_cur_sample = (m_cmd[0]<<12) | (m_cmd[1]<<4) | ((m_cmd[2]>>4) & 0x0f);

		if (m_cur_sample > 0)
		{
			m_output_active = true;
			m_shift = 8;

			if (LOG)
			{
				bool good_pos = (m_cur_sample<2 || m_samples[m_cur_sample-2] == -128);

				logerror("DM3AG '%s' start: %d (%s), vol: %02x out: %d pan: %d\n", tag().c_str(), m_cur_sample, good_pos ? "ok": "no", m_cmd[3], m_cmd[4] & 0x10 ? 1 : 2, (m_cmd[4]>>5)&7);
			}
		}

		m_count = 0;
	}
}
