// license:BSD-3-Clause
// copyright-holders:Carl
// Number Nine Revolution 512x32/1024x8
// TODO: for 1024x768 mode the 7220 is programmed for 512x768, how does that work?

#include "emu.h"
#include "num9rev.h"
#include "screen.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_NUM_9_REV, isa8_number_9_rev_device, "number_9_rev", "Number Nine Revolution 512x32/1024x8")

void isa8_number_9_rev_device::upd7220_map(address_map &map)
{
	map(0x00000, 0x3ffff).noprw();
}

UPD7220_DISPLAY_PIXELS_MEMBER( isa8_number_9_rev_device::hgdc_display_pixels )
{
	palette_t *pal = m_palette->palette();
	if(!m_1024)
	{
		rgb_t color(0);
		uint16_t overlay;
		if(((address << 3) + 0xc0016) > (1024*1024))
			return;
		for(int i = 0; i < 16; i++)
		{
			uint32_t addr = (address << 3) + i;
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


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_number_9_rev_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(512, 448);
	screen.set_visarea(0, 512-1, 0, 448-1);
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(isa8_number_9_rev_device::screen_update));
	PALETTE(config, m_palette).set_entries(4096);

	UPD7220(config, m_upd7220, XTAL(4'433'619)/2); // unknown clock
	m_upd7220->set_addrmap(0, &isa8_number_9_rev_device::upd7220_map);
	m_upd7220->set_display_pixels(FUNC(isa8_number_9_rev_device::hgdc_display_pixels));
	m_upd7220->set_screen("screen");
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa8_number_9_rev_device::isa8_number_9_rev_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_NUM_9_REV, tag, owner, clock),
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

	m_isa->install_memory(0xc0000, 0xc0001, read8sm_delegate(*m_upd7220, FUNC(upd7220_device::read)), write8sm_delegate(*m_upd7220, FUNC(upd7220_device::write)));
	m_isa->install_memory(0xc0100, 0xc03ff, read8_delegate(*this, FUNC(isa8_number_9_rev_device::pal8_r)), write8_delegate(*this, FUNC(isa8_number_9_rev_device::pal8_w)));
	m_isa->install_memory(0xc0400, 0xc0401, read8_delegate(*this, FUNC(isa8_number_9_rev_device::bank_r)), write8_delegate(*this, FUNC(isa8_number_9_rev_device::bank_w)));
	m_isa->install_memory(0xc0500, 0xc06ff, read8_delegate(*this, FUNC(isa8_number_9_rev_device::overlay_r)), write8_delegate(*this, FUNC(isa8_number_9_rev_device::overlay_w)));
	m_isa->install_memory(0xc0700, 0xc070f, read8_delegate(*this, FUNC(isa8_number_9_rev_device::ctrl_r)), write8_delegate(*this, FUNC(isa8_number_9_rev_device::ctrl_w)));
	m_isa->install_memory(0xc1000, 0xc3fff, read8_delegate(*this, FUNC(isa8_number_9_rev_device::pal12_r)), write8_delegate(*this, FUNC(isa8_number_9_rev_device::pal12_w)));
	m_isa->install_memory(0xa0000, 0xaffff, read8_delegate(*this, FUNC(isa8_number_9_rev_device::read8)), write8_delegate(*this, FUNC(isa8_number_9_rev_device::write8)));
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
		uint32_t newoff = ((offset & 3) << 18) | (m_bank << 14) | ((offset >> 2) & 0x3fff);
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
		uint8_t bank = m_bank;
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
		uint32_t newoff = ((offset & 3) << 18) | (m_bank << 14) | ((offset >> 2) & 0x3fff);
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
	uint16_t color = offset & 0xfff;
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
	uint16_t color = offset & 0xfff;
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

uint32_t isa8_number_9_rev_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle visarea = screen.visible_area();
	// try to support the 1024x8 or at least don't crash as there's no way to detect it
	m_1024 = (visarea.width() * visarea.height()) > (512 * 512);
	return m_upd7220->screen_update(screen, bitmap, cliprect);
}
