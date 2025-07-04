// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    awacs_macrisc.cpp

    AWACS/Screamer 16-bit audio I/O for "MacRisc" architecture Macs (PCI-based)

    AWACS and Screamer are audio CODECs that comply with a specification.
    Data transfer to and from them is done in terms of serial frames.

    Screamer is back-compatible with AWACS but supports more mixer inputs and
    better power management.

***************************************************************************/

#include "emu.h"
#include "awacs_macrisc.h"

#define LOG_REGISTERS (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

constexpr u16 REGISTER_1_MUTE = 0x100;

// device type definition
DEFINE_DEVICE_TYPE(AWACS_MACRISC, awacs_macrisc_device, "awacsmr", "AWACS MacRisc audio I/O")
DEFINE_DEVICE_TYPE(SCREAMER, screamer_device, "screamer", "Screamer audio I/O")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  awacs_macrisc_device - constructor
//-------------------------------------------------

awacs_macrisc_device::awacs_macrisc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_output_cb(*this, 0)
	, m_input_cb(*this)
	, m_stream(nullptr)
{
}

awacs_macrisc_device::awacs_macrisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: awacs_macrisc_device(mconfig, AWACS_MACRISC, tag, owner, clock)
 {
 }

 screamer_device::screamer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	 : awacs_macrisc_device(mconfig, SCREAMER, tag, owner, clock)
 {
 }

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void awacs_macrisc_device::device_start()
{
	// create the stream
	m_stream = stream_alloc(0, 2, clock()/1024, STREAM_SYNCHRONOUS);

	save_item(NAME(m_phase));
	save_item(NAME(m_active));
	save_item(NAME(m_registers));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void awacs_macrisc_device::device_reset()
{
	m_phase = 0;
	m_active = ACTIVE_OUT;      // AWACS is always running, Screamer has a real enable/disable bit
	m_registers[1] = REGISTER_1_MUTE;
	m_stream->set_sample_rate(clock() / 1024);
}

void screamer_device::device_reset()
{
	awacs_macrisc_device::device_reset();
	m_active = 0;
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void awacs_macrisc_device::sound_stream_update(sound_stream &stream)
{
	// if we're active and not muted
	if ((m_active & ACTIVE_OUT) && !(m_registers[1] & REGISTER_1_MUTE))
	{
		const s16 atten_L = 0xf - ((m_registers[2] >> 6) & 0xf);
		const s16 atten_R = 0xf - (m_registers[2] & 0xf);

		const u32 data = swapendian_int32(m_output_cb(m_phase));
		const s16 l_raw = (s16)(data >> 16);
		const s16 r_raw = (s16)(data & 0xffff);

		const s32 left = ((s32)l_raw * atten_L) >> 4;
		const s32 right = ((s32)r_raw * atten_R) >> 4;
		stream.put_int(0, 0, left, 32768);
		stream.put_int(1, 0, right, 32768);
	}
	else
	{
		stream.put_int(0, 0, 0, 32768);
		stream.put_int(1, 0, 0, 32768);
	}

	m_phase = (m_phase + 1) & 0xfff;
}

uint32_t awacs_macrisc_device::read_macrisc(offs_t offset)
{
	LOGMASKED(LOG_REGISTERS, "read AWACS @ %x\n", offset);
	switch (offset)
	{
		case 0:     // Audio Control
			return 0;

		case 4:     // Audio CODEC Control
			return 0;

		case 8:     // Audio CODEC Status
			return 0x314000;    // Screamer info
	}

	return 0;
}

void awacs_macrisc_device::write_macrisc(offs_t offset, uint32_t data)
{
	static const int rates[8] = { 512, 768, 1024, 1280, 1536, 2048, 2560, 3072 };

	LOGMASKED(LOG_REGISTERS, "%s: %08x @ %x\n", tag(), data, offset*4);
	switch (offset)
	{
		case 0: // Audio Control
			m_stream->set_sample_rate(clock() / rates[(data >> 8) & 7]);
			LOG("%s: sample rate to %d Hz\n", tag(), clock() / rates[(data >> 8) & 7]);
			break;

		case 4: // Audio CODEC Control
			{
				int subframe = (data >> 22) & 0x3;
				int codec_addr = (data >> 12) & 0xfff;
				int codec_data = (data & 0xfff);

				LOGMASKED(LOG_REGISTERS, "%s: CODEC control: %x to addr %x (subframe %d)\n", tag(), codec_data, codec_addr, subframe);

				m_registers[codec_addr] = codec_data;
			}
			break;

		case 8: // Audio CODEC Status
			break;

		case 12: // Byte swap
			printf("CODEC byte swap: %08x\n", data);
			break;
	}
}

// Screamer
uint32_t screamer_device::read_macrisc(offs_t offset)
{
	switch (offset)
	{
		case 0: // Audio Control
				return 0;

		case 4: // Audio CODEC Control
				return 0;

		case 8:                  // Audio CODEC Status
				return swapendian_int32((0x40 << 8) |    // indicate CODEC is present
				(1 << 16) |             // manufacturer is Crystal Semiconductor
				(3 << 20));              // CODEC version 3 (Screamer)
	}

	return 0;
}

void screamer_device::write_macrisc(offs_t offset, uint32_t data)
{
	awacs_macrisc_device::write_macrisc(offset, data);

	// if IDLE bit is off, we're active
	if (!BIT(m_registers[6], 1))
	{
		m_active |= ACTIVE_OUT;
	}
	else
	{
		m_active &= ~ACTIVE_OUT;
	}
	LOG("%s: Playback %s reg 6 %x)\n", tag(), !BIT(m_registers[6], 1) ? "on" : "off", m_registers[6]);
}
