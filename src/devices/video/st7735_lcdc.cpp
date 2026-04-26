// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "st7735_lcdc.h"

// The ST7735 does not allow user to upload color conversion table
// The ST7735S does allow user to upload conversion table (command 2d)

// what are the differences with ST7715 and ST7735SV? (and other models)
// PWCTR1 has only 2 params on the SV at least


DEFINE_DEVICE_TYPE(ST7735, st7735_lcdc_device, "st7735", "Sitronix ST7735 LCD Controller")

st7735_lcdc_device::st7735_lcdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ST7735, tag, owner, clock)
{
}

u32 st7735_lcdc_device::render_to_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_x; y <= cliprect.max_y; y++)
	{
		u32* dst = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int count = (y * 0x200) + x;

			u16 dat = m_displaybuffer[(count * 2) + 1] | (m_displaybuffer[(count * 2) + 0] << 8);

			int b = ((dat >> 0) & 0x1f) << 3;
			int g = ((dat >> 5) & 0x3f) << 2;
			int r = ((dat >> 11) & 0x1f) << 3;

			dst[x] = (r << 16) | (g << 8) | (b << 0);
		}
	}

	return 0;
}


void st7735_lcdc_device::lcdc_command_w(u8 data)
{
	m_command = data;
	m_commandstep = 0;

	if (m_command == 0x11)
	{
		logerror("command is %02x (SLEEP OUT - no params expected)\n", m_command);
	}
	else if (m_command == 0x29)
	{
		logerror("command is %02x (DISPLAY ON - no params expected)\n", m_command);
	}
	else if (m_command == 0x2a)
	{
		logerror("command is %02x (CASET - 4 params expected)\n", m_command);
	}
	else if (m_command == 0x2b)
	{
		logerror("command is %02x (RASET - 4 params expected)\n", m_command);
	}
	else if (m_command == 0x2c)
	{
		logerror("command is %02x (MEMORY WRITE - unlimited params)\n", m_command);
	}
	else if (m_command == 0x36)
	{
		logerror("command is %02x (MADCTL - 1 param expected)\n", m_command);
	}
	else if (m_command == 0x3a)
	{
		logerror("command is %02x (COLMOD - 1 param expected)\n", m_command);
	}
	else if (m_command == 0xb1)
	{
		logerror("command is %02x (FRMCTR1 - 3 params expected)\n", m_command);
	}
	else if (m_command == 0xb2)
	{
		logerror("command is %02x (FRMCTR2 - 3 params expected)\n", m_command);
	}
	else if (m_command == 0xb3)
	{
		logerror("command is %02x (FRMCTR3 - 6 params expected)\n", m_command);
	}
	else if (m_command == 0xb4)
	{
		logerror("command is %02x (INVCTR - 1 param expected)\n", m_command);
	}
	else if (m_command == 0xc0)
	{
		logerror("command is %02x (PWCTR1 - 2 or 3 params expected)\n", m_command);
	}
	else if (m_command == 0xc1)
	{
		logerror("command is %02x (PWCTR2 - 1 param expected)\n", m_command);
	}
	else if (m_command == 0xc2)
	{
		logerror("command is %02x (PWCTR3 - 2 params expected)\n", m_command);
	}
	else if (m_command == 0xc3)
	{
		logerror("command is %02x (PWCTR4 - 2 params expected)\n", m_command);
	}
	else if (m_command == 0xc4)
	{
		logerror("command is %02x (PWCTR5 - 2 params expected)\n", m_command);
	}
	else if (m_command == 0xc5)
	{
		logerror("command is %02x (VMCTR - 1 param expected)\n", m_command);
	}
	else if (m_command == 0xe0)
	{
		logerror("command is %02x (GMCTRP - 16 params expected)\n", m_command);
	}
	else if (m_command == 0xe1)
	{
		logerror("command is %02x (GMCTRN - 16 params expected)\n", m_command);
	}
	else
	{
		logerror("unknown command is %02x\n", m_command);
	}
}

u8 st7735_lcdc_device::lcdc_data_r()
{
	return 0;
}

void st7735_lcdc_device::lcdc_data_w(u8 data)
{
	if (m_command == 0x2b)
	{
		switch (m_commandstep)
		{
		case 0: m_posminy = data << 8 | (m_posminy & 0xff); break;
		case 1: m_posminy = (m_posminy & 0xff00) | data; break;
		case 2: m_posmaxy = data << 8 | (m_posmaxy & 0xff); break;
		case 3:
			m_posmaxy = (m_posmaxy & 0xff00) | data; m_posy = m_posminy;
			logerror("Y Min %04x Y Max %04x\n", m_posminy, m_posmaxy);
			break;
		}
	}
	else if (m_command == 0x2a)
	{
		switch (m_commandstep)
		{
		case 0: m_posminx = data << 8 | (m_posminx & 0xff); break;
		case 1: m_posminx = (m_posminx & 0xff00) | data; break;
		case 2: m_posmaxx = data << 8 | (m_posmaxx & 0xff); break;
		case 3:
			m_posmaxx = (m_posmaxx & 0xff00) | data; m_posx = m_posminx << 1;
			logerror("X Min %04x X Max %04x\n", m_posminx, m_posmaxx);
			break;
		}
	}
	else if (m_command == 0x2c)
	{
		m_displaybuffer[((m_posx + (m_posy * 0x400))) & 0x1ffff] = data;

		m_posx++;
		if (m_posx > ((m_posmaxx << 1) + 1))
		{
			m_posx = m_posminx << 1;
			m_posy++;

			if (m_posy > m_posmaxy)
			{
				m_posy = m_posminy;
			}
		}
	}
	else if (m_command == 0x36) // MADCTL (Memory Data Access Control)
	{
		switch (m_commandstep)
		{
		case 0: logerror("MADCTL set to %02x\n", data); break;
		default: logerror("unexpected parameter for command %02x\n", m_command); break;
		}
	}
	else if (m_command == 0x3a) // COLMOD (Interface Pixel Format)
	{
		switch (m_commandstep)
		{
		case 0: logerror("COLMOD set to %02x\n", data); break;
		default: logerror("unexpected parameter for command %02x\n", m_command); break;
		}
	}
	else if (m_command == 0xb1) // FRMCTR1 (Frame Rate Control, normal mode / full colors)
	{
		switch (m_commandstep)
		{
		case 0: logerror("FRMCTR1 param 1 set to %02x\n", data); break;
		case 1: logerror("FRMCTR1 param 2 set to %02x\n", data); break;
		case 2: logerror("FRMCTR1 param 3 set to %02x\n", data); break;
		default: logerror("unexpected parameter for command %02x\n", m_command); break;
		}
	}
	else if (m_command == 0xb2) // FRMCTR2 (Frame Rate Control, idle mode / 8 colors)
	{
		switch (m_commandstep)
		{
		case 0: logerror("FRMCTR2 param 1 set to %02x\n", data); break;
		case 1: logerror("FRMCTR2 param 2 set to %02x\n", data); break;
		case 2: logerror("FRMCTR2 param 3 set to %02x\n", data); break;
		default: logerror("unexpected parameter for command %02x\n", m_command); break;
		}
	}
	else if (m_command == 0xb3) // FRMCTR3 (Frame Rate Control, partial mode / full colors)
	{
		switch (m_commandstep)
		{
		case 0: logerror("FRMCTR3 param 1 set to %02x\n", data); break;
		case 1: logerror("FRMCTR3 param 2 set to %02x\n", data); break;
		case 2: logerror("FRMCTR3 param 3 set to %02x\n", data); break;
		case 3: logerror("FRMCTR3 param 4 set to %02x\n", data); break;
		case 4: logerror("FRMCTR3 param 5 set to %02x\n", data); break;
		case 5: logerror("FRMCTR3 param 6 set to %02x\n", data); break;
		default: logerror("unexpected parameter for command %02x\n", m_command); break;
		}
	}
	else if (m_command == 0xb4) // INVCTR (Display Inversion Control)
	{
		switch (m_commandstep)
		{
		case 0: logerror("INVCTR set to %02x\n", data); break;
		default: logerror("unexpected parameter for command %02x\n", m_command); break;
		}
	}
	else if (m_command == 0xc0) // PWCTR1 (Power Control 1)
	{
		switch (m_commandstep)
		{
		case 0: logerror("PWCTR1 param 1 set to %02x\n", data); break;
		case 1: logerror("PWCTR1 param 2 set to %02x\n", data); break;
		case 2: logerror("PWCTR1 param 3 set to %02x\n", data); break; // ST7735 has a 3rd param here, ST7735V does not
		default: logerror("unexpected parameter for command %02x (%02x)\n", m_command, data); break;
		}
	}
	else if (m_command == 0xc1) // PWCTR2 (Power Control 2)
	{
		switch (m_commandstep)
		{
		case 0: logerror("PWCTR2 param 1 set to %02x\n", data); break;
		default: logerror("unexpected parameter for command %02x\n", m_command); break;
		}
	}
	else if (m_command == 0xc2) // PWCTR3 (Power Control 3, normal mode / full colors)
	{
		switch (m_commandstep)
		{
		case 0: logerror("PWCTR3 param 1 set to %02x\n", data); break;
		case 1: logerror("PWCTR3 param 2 set to %02x\n", data); break;
		default: logerror("unexpected parameter for command %02x\n", m_command); break;
		}
	}
	else if (m_command == 0xc3) // PWCTR4 (Power Control 4, idle mode / 8 colors)
	{
		switch (m_commandstep)
		{
		case 0: logerror("PWCTR4 param 1 set to %02x\n", data); break;
		case 1: logerror("PWCTR4 param 2 set to %02x\n", data); break;
		default: logerror("unexpected parameter for command %02x\n", m_command); break;
		}
	}
	else if (m_command == 0xc4) // PWCTR5 (Power Control 5, partial mode / full colors)
	{
		switch (m_commandstep)
		{
		case 0: logerror("PWCTR5 param 1 set to %02x\n", data); break;
		case 1: logerror("PWCTR5 param 2 set to %02x\n", data); break;
		default: logerror("unexpected parameter for command %02x\n", m_command); break;
		}
	}
	else if (m_command == 0xc5) // VMCTR (VCOM Control)
	{
		switch (m_commandstep)
		{
		case 0: logerror("VMCTR param 1 set to %02x\n", data); break;
		default: logerror("unexpected parameter for command %02x\n", m_command); break;
		}
	}
	else if (m_command == 0xe0) // GMCTRP (Gamma +polarity Correction Characteristics) 
	{
		if (m_commandstep < 16)
		{
			logerror("GMCTRP param %d set to %02x\n", m_commandstep + 1, data);
		}
		else
		{
			logerror("unexpected parameter for command %02x\n", m_command);
		}
	}
	else if (m_command == 0xe1) // GMCTRN (Gamma -polarity Correction Characteristics) 
	{
		if (m_commandstep < 16)
		{
			logerror("GMCTRN param %d set to %02x\n", m_commandstep + 1, data);
		}
		else
		{
			logerror("unexpected parameter for command %02x\n", m_command);
		}
	}

	m_commandstep++;

}

void st7735_lcdc_device::device_start()
{
	std::fill(std::begin(m_displaybuffer), std::end(m_displaybuffer), 0);
	m_posx = 0;
	m_posy = 0;
	m_posminx = 0;
	m_posmaxx = 0;
	m_posminy = 0;
	m_posmaxy = 0;
	m_command = 0;
	m_commandstep = 0;

	save_item(NAME(m_displaybuffer));
	save_item(NAME(m_posx));
	save_item(NAME(m_posy));
	save_item(NAME(m_posminx));
	save_item(NAME(m_posmaxx));
	save_item(NAME(m_posminy));
	save_item(NAME(m_posmaxy));
	save_item(NAME(m_command));
	save_item(NAME(m_commandstep));
}

void st7735_lcdc_device::device_reset()
{
}
