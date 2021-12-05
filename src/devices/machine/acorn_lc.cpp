// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Acorn LC ASIC (for Acorn A4)

    No public documentation exists, this is mostly based on the A4 TRM.

*********************************************************************/

#include "emu.h"
#include "acorn_lc.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(ACORN_LC, acorn_lc_device, "acorn_lc", "Acorn LC ASIC")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  acorn_lc_device - constructor
//-------------------------------------------------

acorn_lc_device::acorn_lc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ACORN_LC, tag, owner, clock)
	//, device_memory_interface(mconfig, *this)
	//, device_palette_interface(mconfig, *this)
	//, device_video_interface(mconfig, *this)
	, m_vdsr(0)
	, m_vdlr(0)
	, m_hdsr(0)
	, m_hdlr(0)
	, m_licr(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_lc_device::device_start()
{
	// state saving
	save_item(NAME(m_vdsr));
	save_item(NAME(m_vdlr));
	save_item(NAME(m_hdsr));
	save_item(NAME(m_hdlr));
	save_item(NAME(m_licr));
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

u8 acorn_lc_device::read(offs_t offset)
{
	u8 data = 0xff;

	switch (offset & 0x0f)
	{
	case LC_LICR_L:
		data = m_licr & 0x0f;
		break;
	case LC_LICR_M:
		data = (m_licr >> 4) & 0x0f;
		break;
	case LC_LICR_H:
		data = (m_licr >> 8) & 0x0f;
		break;

	case LC_RESET:
		data = 0x04;
		break;
	}
	logerror("lc_r: % 04x % 02x\n", offset, data);
	return data;
}

void acorn_lc_device::write(offs_t offset, u8 data)
{
	logerror("lc_w: % 04x % 02x\n", offset, data);
	switch (offset & 0x0f)
	{
	case LC_VDSR_L:
		m_vdsr = (m_vdsr & 0x0ff0) | (data & 0x0f);
		break;
	case LC_VDSR_M:
		m_vdsr = (m_vdsr & 0x0f0f) | ((data & 0x0f) << 4);
		break;
	case LC_VDSR_H:
		m_vdsr = (m_vdsr & 0x00ff) | ((data & 0x0f) << 8);
		break;

	case LC_VDLR_L:
		m_vdlr = (m_vdlr & 0x0ff0) | (data & 0x0f);
		break;
	case LC_VDLR_M:
		m_vdlr = (m_vdlr & 0x0f0f) | ((data & 0x0f) << 4);
		break;
	case LC_VDLR_H:
		m_vdlr = (m_vdlr & 0x00ff) | ((data & 0x0f) << 8);
		break;

	case LC_HDSR_L:
		m_hdsr = (m_hdsr & 0x0ff0) | (data & 0x0f);
		break;
	case LC_HDSR_M:
		m_hdsr = (m_hdsr & 0x0f0f) | ((data & 0x0f) << 4);
		break;
	case LC_HDSR_H:
		m_hdsr = (m_hdsr & 0x00ff) | ((data & 0x0f) << 8);
		break;

	case LC_HDLR_L:
		m_hdlr = (m_hdlr & 0x0ff0) | (data & 0x0f);
		break;
	case LC_HDLR_M:
		m_hdlr = (m_hdlr & 0x0f0f) | ((data & 0x0f) << 4);
		break;
	case LC_HDLR_H:
		m_hdlr = (m_hdlr & 0x00ff) | ((data & 0x0f) << 8);
		break;

	case LC_LICR_L:
		m_licr = (m_licr & 0x0ff0) | (data & 0x0f);
		break;
	case LC_LICR_M:
		m_licr = (m_licr & 0x0f0f) | ((data & 0x0f) << 4);
		switch (m_licr & LICR_CLOCK_MASK)
		{
		case LICR_CLOCK_CRYS:
			//m_vidc->set_unscaled_clock(24_MHz_XTAL);
			break;
		case LICR_CLOCK_CRYS2:
			//m_vidc->set_unscaled_clock(24_MHz_XTAL / 2);
			break;
		case LICR_CLOCK_IOEB:
		default:
			//m_vidc->set_unscaled_cloc(ioeb_clock_select);
			break;
		}
		break;
	case LC_LICR_H:
		m_licr = (m_licr & 0x00ff) | ((data & 0x0f) << 8);
		break;
	}
	//if (offset & 0x40)
		//pal[(offset >> 2) & 0xf] = lc_palette[data & 0xf];
}
