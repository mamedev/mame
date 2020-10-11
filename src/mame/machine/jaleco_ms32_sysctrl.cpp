// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

	Jaleco MS32 System Control Unit

	A simple system controller for late 90s Jaleco HWs
	Known features: 
	- CRTC & screen(s?) control;
	- dot clock control;
	- reset/irq lines;
	- programmable timer;
	- watchdog;
	
	First use in MS32, then their later (?) 68k revision. 

	TODO:
	- actual chip name;
	- several unknowns on the Stepping Stage era HW interface, 
	  namely how the CRTC setup works.

***************************************************************************/

#include "emu.h"
#include "jaleco_ms32_sysctrl.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(JALECO_MS32_SYSCTRL, jaleco_ms32_sysctrl_device, "jaleco_ms32_sysctrl", "Jaleco MS32 System Control Unit")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  jaleco_ms32_sysctrl_device - constructor
//-------------------------------------------------

jaleco_ms32_sysctrl_device::jaleco_ms32_sysctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, JALECO_MS32_SYSCTRL, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_space_config("regs", ENDIANNESS_NATIVE, 16, 6, -1, address_map_constructor(FUNC(jaleco_ms32_sysctrl_device::io_map), this))
{
}

device_memory_interface::space_config_vector jaleco_ms32_sysctrl_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_IO, &m_space_config)
	};
}

void jaleco_ms32_sysctrl_device::io_map(address_map& map)
{
//	0xfce00000 in MS32, 0xba0000 in 68k (add * 2 for actual addresses) 
	map(0x00, 0x00).w(FUNC(jaleco_ms32_sysctrl_device::control_w));
	map(0x01, 0x01).w(FUNC(jaleco_ms32_sysctrl_device::hblank_w));
	map(0x02, 0x02).w(FUNC(jaleco_ms32_sysctrl_device::hdisplay_w));
	map(0x03, 0x03).w(FUNC(jaleco_ms32_sysctrl_device::hbp_w));
	map(0x04, 0x04).w(FUNC(jaleco_ms32_sysctrl_device::hfp_w));
	map(0x05, 0x05).w(FUNC(jaleco_ms32_sysctrl_device::vblank_w));
	map(0x06, 0x06).w(FUNC(jaleco_ms32_sysctrl_device::vdisplay_w));
	map(0x07, 0x07).w(FUNC(jaleco_ms32_sysctrl_device::vbp_w));
	map(0x08, 0x08).w(FUNC(jaleco_ms32_sysctrl_device::vfp_w));
}

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void jaleco_ms32_sysctrl_device::device_add_mconfig(machine_config &config)
{
	//DEVICE(config, ...);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void jaleco_ms32_sysctrl_device::device_start()
{
	save_item(NAME(m_dotclock));
	save_item(NAME(m_crtc.horz_blank));
	save_item(NAME(m_crtc.horz_display));
	save_item(NAME(m_crtc.vert_blank));
	save_item(NAME(m_crtc.vert_display));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void jaleco_ms32_sysctrl_device::device_reset()
{
	m_dotclock = 0;
	m_crtc.horz_blank = 64;
	m_crtc.horz_display = 320;
	m_crtc.vert_blank = 39;
	m_crtc.vert_display = 224;

}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

u16 jaleco_ms32_sysctrl_device::read(offs_t offset, u16 mem_mask)
{
	return this->space(AS_IO).read_word(offset, mem_mask);
}

void jaleco_ms32_sysctrl_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	this->space(AS_IO).write_word(offset, data, mem_mask);
}

// =================
// CRTC
// =================

inline u16 jaleco_ms32_sysctrl_device::crtc_write_reg(u16 raw_data)
{
	// each write has a 12 bit resolution, with 
	// TODO: nndmseal sets up bit 12, used as safeguard?
	return 0x1000 - (raw_data & 0xfff);
}

inline void jaleco_ms32_sysctrl_device::crtc_refresh_screen_params()
{
	rectangle visarea;
	const u16 htotal = m_crtc.horz_blank + m_crtc.horz_display;
	const u16 vtotal = m_crtc.vert_blank + m_crtc.vert_display;
	const u8 dot_divider = m_dotclock & 1 ? 6 : 8;
	const attoseconds_t refresh = HZ_TO_ATTOSECONDS(clock() / dot_divider) * htotal * vtotal;
	visarea.set(0, m_crtc.horz_display - 1, 0, m_crtc.vert_display - 1);
	screen().configure(htotal, vtotal, visarea, refresh);
}

void jaleco_ms32_sysctrl_device::control_w(u16 data)
{
	/* 
	 * ---- x--- programmable irq timer enable (1->0 in P47 Aces)
	 * ---- -x-- used by f1superb, network irq enable?
	 * ---- --x- flip screen
	 * ---- ---x dotclock select (1) 8 MHz (0) 6 MHz
	 */
	if ((data & 1) != m_dotclock)
	{
		m_dotclock = data & 0x01;
		crtc_refresh_screen_params();
	}
}

void jaleco_ms32_sysctrl_device::hblank_w(u16 data)
{
	m_crtc.horz_blank = crtc_write_reg(data);	    
	crtc_refresh_screen_params();
}

void jaleco_ms32_sysctrl_device::hdisplay_w(u16 data)
{
	m_crtc.horz_display = crtc_write_reg(data);	
	crtc_refresh_screen_params();
}

void jaleco_ms32_sysctrl_device::hbp_w(u16 data)
{
	logerror("%s: HSYNC back porch %d\n", this->tag(), 0x1000 - data); 
}

void jaleco_ms32_sysctrl_device::hfp_w(u16 data)
{
	logerror("%s: HSYNC front porch %d\n", this->tag(), 0x1000 - data); 
}

void jaleco_ms32_sysctrl_device::vblank_w(u16 data)
{
	m_crtc.vert_blank = crtc_write_reg(data);	    
	crtc_refresh_screen_params();
}

void jaleco_ms32_sysctrl_device::vdisplay_w(u16 data)
{
	m_crtc.vert_display = crtc_write_reg(data);	
	crtc_refresh_screen_params();
}

void jaleco_ms32_sysctrl_device::vbp_w(u16 data)
{
	logerror("%s: VSYNC back porch %d\n", this->tag(), 0x1000 - data); 
}

void jaleco_ms32_sysctrl_device::vfp_w(u16 data)
{
	logerror("%s: VSYNC front porch %d\n", this->tag(), 0x1000 - data); 
}


