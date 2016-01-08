// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Curt Coder
/**********************************************************************

    MOS 6581/8580 Sound Interface Device emulation

**********************************************************************/

#include "mos6581.h"
#include "sid.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type MOS6581 = &device_creator<mos6581_device>;
const device_type MOS8580 = &device_creator<mos8580_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6581_device - constructor
//-------------------------------------------------

mos6581_device::mos6581_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_sound_interface(mconfig, *this),
		m_read_potx(*this),
		m_read_poty(*this),
		m_stream(nullptr),
		m_variant(variant)
{
	m_token = global_alloc_clear<SID6581_t>();
}

mos6581_device::mos6581_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MOS6581, "MOS6581", tag, owner, clock, "mos6581", __FILE__),
		device_sound_interface(mconfig, *this),
		m_read_potx(*this),
		m_read_poty(*this),
		m_stream(nullptr),
		m_variant(TYPE_6581)
{
	m_token = global_alloc_clear<SID6581_t>();
}

mos6581_device::~mos6581_device()
{
	global_free(m_token);
}

//-------------------------------------------------
//  mos8580_device - constructor
//-------------------------------------------------

mos8580_device::mos8580_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mos6581_device(mconfig, MOS8580, "MOS8580", tag, owner, clock, TYPE_8580, "mos8580", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6581_device::device_start()
{
	// resolve callbacks
	m_read_potx.resolve_safe(0xff);
	m_read_poty.resolve_safe(0xff);

	// create sound stream
	m_stream = machine().sound().stream_alloc(*this, 0, 1, machine().sample_rate());

	// initialize SID engine
	m_token->device = this;
	m_token->mixer_channel = m_stream;
	m_token->PCMfreq = machine().sample_rate();
	m_token->clock = clock();
	m_token->type = m_variant;

	sid6581_init(m_token);
	sidInitWaveformTables(m_variant);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6581_device::device_reset()
{
	sidEmuReset(m_token);
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void mos6581_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	sidEmuFillBuffer(m_token, outputs[0], samples);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( mos6581_device::read )
{
	UINT8 data;

	switch (offset & 0x1f)
	{
	case 0x19:
		data = m_read_potx(0);
		break;

	case 0x1a:
		data = m_read_poty(0);
		break;

	default:
		data = sid6581_port_r(machine(), m_token, offset);
		break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( mos6581_device::write )
{
	sid6581_port_w(m_token, offset, data);
}
