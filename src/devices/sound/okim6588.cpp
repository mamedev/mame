// license:BSD-3-Clause
// copyright-holders:hap
/*

OKI MSM6588 ADPCM Recorder

It has similar functionality to MSM6258.

TODO:
- it only supports MCU mode EXT playback, nothing else emulated yet
- status register read (eg. BUSY flag)

*/

#include "emu.h"
#include "okim6588.h"


DEFINE_DEVICE_TYPE(OKIM6588, okim6588_device, "okim6588", "OKI MSM6588 ADPCM Recorder")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

okim6588_device::okim6588_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, OKIM6588, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_write_mon(*this),
	m_chip_mode(CHIP_MODE_STANDALONE)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

// allow save_item on a non-fundamental type
ALLOW_SAVE_TYPE(okim6588_device::chip_mode);
ALLOW_SAVE_TYPE(okim6588_device::command_state);
ALLOW_SAVE_TYPE(okim6588_device::run_state);

void okim6588_device::device_start()
{
	// initialize
	m_stream = stream_alloc(0, 1, clock() / 128);

	m_adpcm_timer = timer_alloc(FUNC(okim6588_device::clock_adpcm), this);
	m_mon_timer = timer_alloc(FUNC(okim6588_device::set_mon), this);

	m_command_state = COMMAND_READY;
	m_run_state = RUN_STOP;
	m_adpcm_data = 0;

	m_vds_bit = (m_chip_mode == CHIP_MODE_MCU) ? 1 : 0;
	m_samp_fdiv = 512;
	m_rec_mode = false;

	// register for savestates
	save_item(NAME(m_chip_mode));
	save_item(NAME(m_command_state));
	save_item(NAME(m_run_state));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_adpcm.m_signal));
	save_item(NAME(m_adpcm.m_step));
	save_item(NAME(m_rec_mode));
	save_item(NAME(m_samp_fdiv));
	save_item(NAME(m_vds_bit));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void okim6588_device::device_reset()
{
	m_command_state = COMMAND_READY;
	m_run_state = RUN_STOP;
	reset_adpcm();

	m_adpcm_timer->adjust(attotime::never);
	m_mon_timer->adjust(attotime::never);
	m_write_mon(0);
}


//-------------------------------------------------
//  internal handlers
//-------------------------------------------------

void okim6588_device::sound_stream_update(sound_stream &stream)
{
	// simply fill the buffer with the current sample
	stream.fill(0, m_adpcm.output() / 2048.0);
}

TIMER_CALLBACK_MEMBER(okim6588_device::clock_adpcm)
{
	switch (m_run_state)
	{
		case RUN_STOP:
			reset_adpcm();
			break;

		case RUN_PLAY_EXT:
			// strobe MON
			m_write_mon(1);
			m_mon_timer->adjust(attotime::from_ticks(m_samp_fdiv / 4, clock()), 0);
			m_command_state = COMMAND_EXT;

			m_stream->update();
			get_adpcm_sample(m_adpcm_data);
			break;

		default:
			break;
	}

	if (m_run_state != RUN_STOP && m_run_state != RUN_PAUSE)
		m_adpcm_timer->adjust(attotime::from_ticks(m_samp_fdiv, clock()));
}

TIMER_CALLBACK_MEMBER(okim6588_device::set_mon)
{
	m_write_mon(param ? 1 : 0);
}

s16 okim6588_device::get_adpcm_sample(u8 data)
{
	// 4-bit or 3-bit input
	if (m_vds_bit)
		return m_adpcm.clock(data & 0xf);
	else
		return m_adpcm.clock((data & 0xc) | (data >> 1 & 1));
}

void okim6588_device::reset_adpcm()
{
	if (machine().time() > attotime::zero)
		m_stream->update();

	m_adpcm_data = 0;
	m_adpcm.reset();
}


//-------------------------------------------------
//  public handlers
//-------------------------------------------------

u8 okim6588_device::data_r()
{
	if (m_chip_mode != CHIP_MODE_MCU)
		return 0;

	return 0;
}

void okim6588_device::data_w(u8 data)
{
	if (m_chip_mode != CHIP_MODE_MCU)
		return;

	data &= 0xf;

	switch (m_command_state)
	{
		case COMMAND_READY:
			switch (data & 0xf)
			{
				// NOP
				case 0x0:
					break;

				// PLAY/REC
				case 0x2: case 0x3:
					m_rec_mode = bool(data & 1);
					break;

				// STOP
				case 0x5:
					m_run_state = RUN_STOP;
					break;

				// SAMP
				case 0x6:
					m_command_state = COMMAND_SAMP;
					break;

				// VDS
				case 0xc:
					m_command_state = COMMAND_VDS;
					break;

				// EXT
				case 0xb:
					m_run_state = m_rec_mode ? RUN_RECORD_EXT : RUN_PLAY_EXT;
					reset_adpcm();

					// minimum delay is 1 sample
					m_adpcm_timer->adjust(attotime::from_ticks(m_samp_fdiv, clock()));
					m_command_state = COMMAND_EXT;
					break;

				default:
					break;
			}
			break;

		case COMMAND_SAMP:
		{
			static const u16 div[4] = { 1024, 768, 640, 512 };
			m_samp_fdiv = div[data & 3];
			m_command_state = COMMAND_READY;
			break;
		}

		case COMMAND_EXT:
			m_adpcm_data = data;
			m_command_state = COMMAND_READY;
			break;

		case COMMAND_VDS:
			m_vds_bit = BIT(data, 2);
			m_command_state = COMMAND_READY;
			break;

		default:
			// shouldn't get here
			break;
	}
}
