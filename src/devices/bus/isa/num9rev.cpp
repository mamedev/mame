// license:BSD-3-Clause
// copyright-holders:Carl
// Number Nine Revolution 512x32/1024x8
// TODO: for 1024x768 mode the 7220 is programmed for 512x768, how does that work?

#include "emu.h"
#include "num9rev.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_NUM_9_REV = &device_creator<isa8_number_9_rev_device>;

static ADDRESS_MAP_START( upd7220_map, AS_0, 16, isa8_number_9_rev_device )
	AM_RANGE(0x00000, 0x3ffff) AM_NOP
ADDRESS_MAP_END

UPD7220_DISPLAY_PIXELS_MEMBER( isa8_number_9_rev_device::hgdc_display_pixels )
{
	palette_t *pal = m_palette->palette();
	if(!m_1024)
	{
		rgb_t color(0);
		UINT16 overlay;
		if(((address << 3) + 0xc0016) > (1024*1024))
			return;
		for(int i = 0; i < 16; i++)
		{
			UINT32 addr = (address << 3) + i;
			overlay = m_ram[addr + 0xc0000] << 1;
			overlay = m_overlay[overlay + ((m_mode & 8) ? 512 : 0)] | (m_overlay[overlay + 1 + ((m_mode & 8) ? 512 : 0)] << 8);
			color.set_r(pal->entry_color(m_ram[addr] | ((overlay & 0xf) << 8)).r());
			color.set_g(pal->entry_color(m_ram[addr + 0x40000] | ((overlay & 0xf0) << 4)).g());
			color.set_b(pal->entry_color(m_ram[addr + 0x80000] | (overlay & 0xf00)).b());
			bitmap.pix32(y, x + i) = color;
		}
	}
	else
	{
		if(((address << 3) + 16) > (1024*1024))
			return;
		for(int i = 0; i < 16; i++)
			bitmap.pix32(y, x + i) = pal->entry_color(m_ram[(address << 4) + i]);
	}
}

static MACHINE_CONFIG_FRAGMENT( num_9_rev )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(512, 448)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 448-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(isa8_number_9_rev_device, screen_update)
	MCFG_PALETTE_ADD("palette", 4096)

	MCFG_DEVICE_ADD("upd7220", UPD7220, XTAL_4_433619MHz/2) // unknown clock
	MCFG_DEVICE_ADDRESS_MAP(AS_0, upd7220_map)
	MCFG_UPD7220_DISPLAY_PIXELS_CALLBACK_OWNER(isa8_number_9_rev_device, hgdc_display_pixels)
	MCFG_VIDEO_SET_SCREEN("screen")
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_number_9_rev_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( num_9_rev );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa8_number_9_rev_device::isa8_number_9_rev_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA8_NUM_9_REV, "Number Nine Revolution 512x32/1024x8", tag, owner, clock, "number_9_rev", __FILE__),
		device_isa8_card_interface(mconfig, *this),
		m_upd7220(*this, "upd7220"),
		m_palette(*this, "palette"),
		m_ram(1024*1024),
		m_overlay(1024), m_bank(0), m_mode(0), m_1024(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_number_9_rev_device::device_start()
{
	set_isa_device();

	m_isa->install_memory(0xc0000, 0xc0001, 0, 0, read8_delegate(FUNC(upd7220_device::read), (upd7220_device *)m_upd7220), write8_delegate(FUNC(upd7220_device::write), (upd7220_device *)m_upd7220));
	m_isa->install_memory(0xc0100, 0xc03ff, 0, 0, read8_delegate(FUNC(isa8_number_9_rev_device::pal8_r), this), write8_delegate(FUNC(isa8_number_9_rev_device::pal8_w), this));
	m_isa->install_memory(0xc0400, 0xc0401, 0, 0, read8_delegate(FUNC(isa8_number_9_rev_device::bank_r), this), write8_delegate(FUNC(isa8_number_9_rev_device::bank_w), this));
	m_isa->install_memory(0xc0500, 0xc06ff, 0, 0, read8_delegate(FUNC(isa8_number_9_rev_device::overlay_r), this), write8_delegate(FUNC(isa8_number_9_rev_device::overlay_w), this));
	m_isa->install_memory(0xc0700, 0xc070f, 0, 0, read8_delegate(FUNC(isa8_number_9_rev_device::ctrl_r), this), write8_delegate(FUNC(isa8_number_9_rev_device::ctrl_w), this));
	m_isa->install_memory(0xc1000, 0xc3fff, 0, 0, read8_delegate(FUNC(isa8_number_9_rev_device::pal12_r), this), write8_delegate(FUNC(isa8_number_9_rev_device::pal12_w), this));
	m_isa->install_memory(0xa0000, 0xaffff, 0, 0, read8_delegate(FUNC(isa8_number_9_rev_device::read8), this), write8_delegate(FUNC(isa8_number_9_rev_device::write8), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_number_9_rev_device::device_reset()
{
	m_bank = 0;
	m_mode = 0;
	m_1024 = false;
}

READ8_MEMBER(isa8_number_9_rev_device::read8)
{
	if((m_mode & 1) && !m_1024)
		return m_ram[offset + ((m_mode & 0xc) << 14)];
	else if((m_mode & 4) && !m_1024)
	{
		UINT32 newoff = ((offset & 3) << 18) | (m_bank << 14) | ((offset >> 2) & 0x3fff);
		return m_ram[newoff];
	}
	else
		return m_ram[offset + (m_bank << 16)];
}

WRITE8_MEMBER(isa8_number_9_rev_device::write8)
{
	if(m_1024 || ((m_mode & 6) == 0))
		m_ram[offset + (m_bank << 16)] = data;
	else if((m_mode & 1) || ((m_mode & 6) == 2))
	{
		UINT8 bank = m_bank;
		if(m_mode & 1)
			bank = (m_mode & 0xc) >> 2;
		else
		{
			if(m_bank >= 12)
			{
				m_ram[offset + (m_bank << 16)] = data;
				return;
			}
			bank &= 3;
		}

		m_ram[offset + (bank << 16)] = data;
		m_ram[offset + ((bank + 4) << 16)] = data;
		m_ram[offset + ((bank + 8) << 16)] = data;
	}
	else if(m_mode & 4)
	{
		UINT32 newoff = ((offset & 3) << 18) | (m_bank << 14) | ((offset >> 2) & 0x3fff);
		if((newoff >= 0xc0000) && ((m_mode & 6) == 6))
			return;
		m_ram[newoff] = data;
	}
}

READ8_MEMBER(isa8_number_9_rev_device::pal8_r)
{
	offset += 0x100;
	palette_t *pal = m_palette->palette();
	switch(offset & 0xf00)
	{
		case 0x100:
			return pal->entry_color(offset).r();
		case 0x200:
			return pal->entry_color(offset).g();
		case 0x300:
			return pal->entry_color(offset).b();
	}
	return 0;
}

WRITE8_MEMBER(isa8_number_9_rev_device::pal8_w)
{
	offset += 0x100;
	palette_t *pal = m_palette->palette();
	rgb_t pen = pal->entry_color(offset);
	switch(offset & 0xf00)
	{
		case 0x100:
			pen.set_r(data);
			break;
		case 0x200:
			pen.set_g(data);
			break;
		case 0x300:
			pen.set_b(data);
			break;
	}
	pal->entry_set_color(offset, pen);
}

READ8_MEMBER(isa8_number_9_rev_device::pal12_r)
{
	UINT16 color = offset & 0xfff;
	palette_t *pal = m_palette->palette();
	switch(offset & 0xf000)
	{
		case 0x0000:
			return pal->entry_color(color).r();
		case 0x1000:
			return pal->entry_color(color).g();
		case 0x2000:
			return pal->entry_color(color).b();
	}
	return 0;
}

WRITE8_MEMBER(isa8_number_9_rev_device::pal12_w)
{
	UINT16 color = offset & 0xfff;
	palette_t *pal = m_palette->palette();
	rgb_t pen = pal->entry_color(color);
	switch(offset & 0xf000)
	{
		case 0x0000:
			pen.set_r(data);
			break;
		case 0x1000:
			pen.set_g(data);
			break;
		case 0x2000:
			pen.set_b(data);
			break;
	}
	pal->entry_set_color(color, pen);
}

READ8_MEMBER(isa8_number_9_rev_device::overlay_r)
{
	return m_overlay[offset + ((m_mode & 8) ? 512 : 0)];
}
WRITE8_MEMBER(isa8_number_9_rev_device::overlay_w)
{
	m_overlay[offset + ((m_mode & 8) ? 512 : 0)] = data;
}

READ8_MEMBER(isa8_number_9_rev_device::bank_r)
{
	return m_bank;
}

WRITE8_MEMBER(isa8_number_9_rev_device::bank_w)
{
	m_bank = data & 0xf;
}

READ8_MEMBER(isa8_number_9_rev_device::ctrl_r)
{
	switch(offset & 0xf)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			// zoom, set to same value as 7220 external zoom factor
			break;
		case 4:
			return (m_mode & 2) ? 0xff : 0;
		case 5:
			return (m_mode & 4) ? 0xff : 0;
		case 6:
			return (m_mode & 8) ? 0xff : 0;
		case 15:
			return (m_mode & 1) ? 0xff : 0;
	}
	return 0;
}

WRITE8_MEMBER(isa8_number_9_rev_device::ctrl_w)
{
	switch(offset & 0xf)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			// zoom
			break;
		case 4:
			if(data & 0x80)
				m_mode |= 2;
			else
				m_mode &= ~2;
			break;
		case 5:
			if(data & 0x80)
				m_mode |= 4;
			else
				m_mode &= ~4;
			break;
		case 6:
			if(data & 0x80)
				m_mode |= 8;
			else
				m_mode &= ~8;
			break;
		case 15:
			if(data & 0x80)
				m_mode |= 1;
			else
				m_mode &= ~1;
			break;
	}
}

UINT32 isa8_number_9_rev_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle visarea = screen.visible_area();
	// try to support the 1024x8 or at least don't crash as there's no way to detect it
	m_1024 = (visarea.width() * visarea.height()) > (512 * 512);
	return m_upd7220->screen_update(screen, bitmap, cliprect);
}
