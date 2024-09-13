// license:BSD-3-Clause
// copyright-holders:giulioz

// Initial skeleton based on the RE work by nukeykt

#include "emu.h"
#include "roland_gp.h"

// Original chip (GP-2) TODO
// DEFINE_DEVICE_TYPE(TC24SC201AF, tc6116_device, "tc24sc201af", "Roland GP TC24SC201AF PCM")

// Newer chip (GP-4) including bugfixes and H8/500 cpu glue logic
DEFINE_DEVICE_TYPE(TC6116, tc6116_device, "tc6116", "Roland GP TC6116 PCM")


tc6116_device::tc6116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TC6116, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_int_callback(*this)
	, m_clock(0)
	, m_rate(0)
	, m_stream(nullptr)
	, m_sel_chn(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc6116_device::device_start()
{
	m_clock = clock() / 2;
	m_rate = m_clock / 512; // usually 32 KHz

	m_stream = stream_alloc(0, 2, m_rate);

	logerror("Roland GP: Clock %u, Rate %u\n", m_clock, m_rate);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc6116_device::device_reset()
{
	m_int_callback(CLEAR_LINE);
}

//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void tc6116_device::rom_bank_pre_change()
{
	// unused right now
	m_stream->update();
}


u8 tc6116_device::read(offs_t offset)
{
	return 0xff;
}

void tc6116_device::write(offs_t offset, u8 data)
{
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tc6116_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
}
