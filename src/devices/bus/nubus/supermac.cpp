// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "supermac.h"

#include "screen.h"

#include <utility>

#define LOG_CRTC        (1U << 1)
#define LOG_SHIFTREG    (1U << 2)

//#define VERBOSE (LOG_CRTC | LOG_SHIFTREG)
#define LOG_OUTPUT_FUNC [&device] (auto &&... args) { device.logerror(std::forward<decltype(args)>(args)...); }
#include "logmacro.h"

#define LOGCRTC(...)        LOGMASKED(LOG_CRTC, __VA_ARGS__)
#define LOGSHIFTREG(...)    LOGMASKED(LOG_SHIFTREG, __VA_ARGS__)


namespace {

void ctrc_reg_w(uint16_t &param, uint32_t offset, uint32_t data) noexcept
{
	param &= 0xff << (BIT(offset, 1) ? 0 : 8);
	param |= (~data & 0xff) << (BIT(offset, 1) ? 8 : 0);
}

} // anonymous namespace


void supermac_spec_crtc::register_save(device_t &device)
{
	device.save_item(NAME(m_hsync));
	device.save_item(NAME(m_hstart));
	device.save_item(NAME(m_hend));
	device.save_item(NAME(m_htotal));
	device.save_item(NAME(m_vsync));
	device.save_item(NAME(m_vstart));
	device.save_item(NAME(m_vend));
	device.save_item(NAME(m_vtotal));
}


void supermac_spec_crtc::reset() noexcept
{
	// invalid - let the firmware initialise it before changing screen parameters
	m_hsync = 0;
	m_hstart = 0;
	m_hend = 0;
	m_htotal = 0;
	m_vsync = 0;
	m_vstart = 0;
	m_vend = 0;
	m_vtotal = 0;
}


void supermac_spec_crtc::write(device_t &device, offs_t offset, uint32_t data)
{
	switch (offset)
	{
	case 0x04:
	case 0x06:
		ctrc_reg_w(m_hsync, offset, data);
		LOGCRTC("%s: CRTC H sync = %u\n", device.machine().describe_context(), m_hsync);
		break;

	case 0x08:
	case 0x0a:
		ctrc_reg_w(m_hstart, offset, data);
		LOGCRTC("%s: CRTC H start = %u\n", device.machine().describe_context(), m_hstart);
		break;

	case 0x0c:
	case 0x0e:
		ctrc_reg_w(m_hend, offset, data);
		LOGCRTC("%s: CRTC H end = %u\n", device.machine().describe_context(), m_hend);
		break;

	case 0x10:
	case 0x12:
		ctrc_reg_w(m_htotal, offset, data);
		LOGCRTC("%s: CRTC H total = %u\n", device.machine().describe_context(), m_htotal);
		break;

	case 0x18:
	case 0x1a:
		ctrc_reg_w(m_vsync, offset, data);
		LOGCRTC("%s: CRTC V sync = %u\n", device.machine().describe_context(), m_vsync);
		break;

	case 0x1c:
	case 0x1e:
		ctrc_reg_w(m_vstart, offset, data);
		LOGCRTC("%s: CRTC V start = %u\n", device.machine().describe_context(), m_vstart);
		break;

	case 0x24:
	case 0x26:
		ctrc_reg_w(m_vend, offset, data);
		LOGCRTC("%s: CRTC V end = %u\n", device.machine().describe_context(), m_vend);
		break;

	case 0x28:
	case 0x2a:
		ctrc_reg_w(m_vtotal, offset, data);
		LOGCRTC("%s: CRTC V total = %u\n", device.machine().describe_context(), m_vtotal);
		break;

	default:
		device.logerror("%s: unknown CRTC register %02x = %08x\n", device.machine().describe_context(), offset, data);
	}
}


bool supermac_spec_crtc::valid(device_t &device) const
{
	if ((m_hsync < m_hstart) && (m_hstart < m_hend) && (m_hend < m_htotal) && (m_vsync < m_vstart) && (m_vstart < m_vend) && (m_vend < m_vtotal))
	{
		return true;
	}
	else
	{
		LOGCRTC("invalid CRTC parameters (%u %u %u %u) (%u %u %u %u)\n",
				m_hsync, m_hstart, m_hend, m_htotal,
				m_vsync, m_vstart, m_vend, m_vtotal);
		return false;
	}
}


void supermac_spec_shift_reg::register_save(device_t &device)
{
	device.save_item(NAME(m_shift_data));
	device.save_item(NAME(m_shift_count));
}


void supermac_spec_shift_reg::reset() noexcept
{
	m_shift_data = 0;
	m_shift_count = 0;
}


void supermac_spec_shift_reg::write_control(device_t &device, uint32_t data)
{
	// Sequence is:
	// * 1 -> control
	// * 0 -> data
	// * 0 -> control
	// * two select bits to data
	// * eight data bits to data, least significant bit first
	if (!BIT(~data, 0))
		m_shift_count = 0;
}


void supermac_spec_shift_reg::write_data(device_t &device, uint32_t data)
{
	m_shift_data = (m_shift_data >> 1) | (BIT(~data, 0) << 15);
	++m_shift_count;
	if (ready())
		LOGSHIFTREG("%s: shift register parameter %d = %02x\n", device.machine().describe_context(), select(), value());
}
