// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

  SunPlus "GCM394" (based on die pictures)

**********************************************************************/

#include "emu.h"
#include "sunplus_gcm394.h"

DEFINE_DEVICE_TYPE(GCM394, sunplus_gcm394_device, "gcm394", "SunPlus GCM394 System-on-a-Chip")

sunplus_gcm394_device::sunplus_gcm394_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sunplus_gcm394_base_device(mconfig, GCM394, tag, owner, clock)
{
}

READ16_MEMBER(sunplus_gcm394_base_device::unk_r)
{
	switch (offset)
	{

	case 0x80f:
		logerror("%s:sunplus_gcm394_base_device::unk_r @ 0x%04x\n", machine().describe_context(), offset + 0x7000);
		return 0x0002;

	case 0x8fb:
		logerror("%s:sunplus_gcm394_base_device::unk_r @ 0x%04x\n", machine().describe_context(), offset + 0x7000);
		m_78fb ^= 0x0100; // status flag for something?
		return m_78fb;


	case 0xabf:
		logerror("%s:sunplus_gcm394_base_device::unk_r @ 0x%04x\n", machine().describe_context(), offset + 0x7000);
		return 0x0001;

	}

	logerror("%s:sunplus_gcm394_base_device::unk_r @ 0x%04x\n", machine().describe_context(), offset + 0x7000);
	return 0x0000;
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unk_w)
{
	logerror("%s:sunplus_gcm394_base_device::unk_w @ 0x%04x (data 0x%04x)\n", machine().describe_context(), offset + 0x7000, data);
}

void sunplus_gcm394_base_device::map(address_map &map)
{
	map(0x000000, 0x006fff).ram();
	map(0x007000, 0x007fff).rw(FUNC(sunplus_gcm394_base_device::unk_r), FUNC(sunplus_gcm394_base_device::unk_w));
	
	map(0x007300, 0x0073ff).ram();

	map(0x007c00, 0x007cff).ram();
	map(0x007d00, 0x007dff).ram();
	map(0x007e00, 0x007eff).ram();
	map(0x007f00, 0x007fff).ram();

}

void sunplus_gcm394_base_device::device_start()
{
}

void sunplus_gcm394_base_device::device_reset()
{
	m_78fb = 0x0000;
}

void sunplus_gcm394_device::device_add_mconfig(machine_config &config)
{
	//SUNPLUS_GCM394_AUDIO(config, m_spg_audio, DERIVED_CLOCK(1, 1));
	//m_spg_audio->add_route(0, *this, 1.0, AUTO_ALLOC_INPUT, 0);
	//m_spg_audio->add_route(1, *this, 1.0, AUTO_ALLOC_INPUT, 1);
}
