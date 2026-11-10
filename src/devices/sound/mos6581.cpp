// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 6581/8580 Sound Interface Device emulation

    The actual sound generation is provided by the reSIDfp engine.

**********************************************************************/

#include "emu.h"
#include "mos6581.h"

#include "residfp/residfp.h"

#include <algorithm>


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

// the sinc resampler requires 125*clock/sample_rate < 16384
static constexpr int MIN_SAMPLE_RATE = 8000;



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MOS6581, mos6581_device, "mos6581", "MOS 6581 SID")
DEFINE_DEVICE_TYPE(MOS8580, mos8580_device, "mos8580", "MOS 8580 SID")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6581_device - constructor
//-------------------------------------------------

mos6581_device::mos6581_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_read_potx(*this, 0xff)
	, m_read_poty(*this, 0xff)
	, m_stream(nullptr)
	, m_sid_state_size(0)
{
}

mos6581_device::mos6581_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6581_device(mconfig, MOS6581, tag, owner, clock)
{
}

mos6581_device::~mos6581_device()
{
}


//-------------------------------------------------
//  mos8580_device - constructor
//-------------------------------------------------

mos8580_device::mos8580_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6581_device(mconfig, MOS8580, tag, owner, clock)
{
}


//-------------------------------------------------
//  configure_sampling - set up the resampler for
//  the current clock and stream rate
//-------------------------------------------------

void mos6581_device::configure_sampling()
{
	m_sid->setSamplingParameters(double(clock()), reSIDfp::RESAMPLE, double(m_stream->sample_rate()));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6581_device::device_start()
{
	if (!clock())
		throw emu_fatalerror("%s: a clock is required\n", tag());

	// create sound stream
	m_stream = stream_alloc(0, 1, std::max(machine().sample_rate(), MIN_SAMPLE_RATE));

	// initialize SID engine
	m_sid = std::make_unique<reSIDfp::residfp>();
	m_sid->setChipModel((type() == MOS8580) ? reSIDfp::CSG8580 : reSIDfp::MOS6581);
	m_sid->enableFilter(true);

	// these are the values the filter models are constructed with; setting them
	// explicitly keeps them out of the save state as uninitialized data
	m_sid->setFilter6581Curve(0.5);
	m_sid->setFilter6581Range(19.0 / 39.0);
	m_sid->setFilter8580Curve(0.5);
	m_sid->enableOld6581caps(false);

	configure_sampling();
	m_sid->reset();

	m_sid_state_size = m_sid->stateSize();
	m_sid_state = make_unique_clear<uint8_t []>(m_sid_state_size);

	save_pointer(NAME(m_sid_state), m_sid_state_size);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6581_device::device_reset()
{
	m_stream->update();

	m_sid->reset();
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void mos6581_device::device_clock_changed()
{
	if (!m_sid || !clock())
		return;

	m_stream->update();

	configure_sampling();
}


//-------------------------------------------------
//  device_pre_save - device-specific pre-save
//-------------------------------------------------

void mos6581_device::device_pre_save()
{
	m_sid->saveState(reinterpret_cast<char *>(m_sid_state.get()), m_sid_state_size);
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void mos6581_device::device_post_load()
{
	m_sid->restoreState(reinterpret_cast<char *>(m_sid_state.get()), m_sid_state_size);
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void mos6581_device::sound_stream_update(sound_stream &stream)
{
	int const samples = stream.samples();

	if (!samples)
		return;

	if (!clock())
	{
		stream.fill(0, 0.0);
		return;
	}

	if (m_buffer.size() < unsigned(samples))
		m_buffer.resize(samples);

	m_sid->clock(m_buffer.data(), samples);

	for (int sample = 0; sample < samples; sample++)
		stream.put_int(0, sample, m_buffer[sample], 32768);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t mos6581_device::read(offs_t offset)
{
	offset &= 0x1f;

	if (machine().side_effects_disabled())
	{
		switch (offset)
		{
		case 0x19: return m_read_potx(0);
		case 0x1a: return m_read_poty(0);
		default:   return m_sid->peek(offset);
		}
	}

	m_stream->update();

	switch (offset)
	{
	case 0x19:
		m_sid->setPaddle(m_read_potx(0), m_sid->peek(0x1a));
		break;

	case 0x1a:
		m_sid->setPaddle(m_sid->peek(0x19), m_read_poty(0));
		break;
	}

	return m_sid->read(offset);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void mos6581_device::write(offs_t offset, uint8_t data)
{
	m_stream->update();

	m_sid->write(offset & 0x1f, data);
}
