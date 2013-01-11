/***************************************************************************

        Psion Organiser II LZ series custom LCD controller

***************************************************************************/

#include "emu.h"
#include "includes/psion.h"

// devices
const device_type PSION_CUSTOM_LCDC = &device_creator<psion_custom_lcdc>;


//**************************************************************************
//  live device
//**************************************************************************

//-------------------------------------------------
//  psion_custom_lcdc - constructor
//-------------------------------------------------

psion_custom_lcdc::psion_custom_lcdc(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	hd44780_device(mconfig, PSION_CUSTOM_LCDC, "Psion Custom LCD Controller", tag, owner, clock)
{
}


//**************************************************************************
//  device interface
//**************************************************************************

UINT32 psion_custom_lcdc::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	assert(height*9 <= bitmap.height() && width*6 <= bitmap.width());

	bitmap.fill(0, cliprect);

	if (m_display_on)
		for (int l=0; l<height; l++)
			for (int i=0; i<width; i++)
			{
				static const UINT8 psion_display_layout[] =
				{
					0x00, 0x01, 0x02, 0x03, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
					0x40, 0x41, 0x42, 0x43, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
					0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
					0x44, 0x45, 0x46, 0x47, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67
				};

				INT8 char_pos = psion_display_layout[l*width + i];

				for (int y=0; y<8; y++)
					for (int x=0; x<5; x++)
						if (m_ddram[char_pos] <= 0x10)
						{
							//draw CGRAM characters
							bitmap.pix16(l*9 + y, i*6 + x) = BIT(m_cgram[(m_ddram[char_pos]&0x07)*8+y], 4-x);
						}
						else
						{
							//draw CGROM characters
							if (region()->bytes() <= 0x800)
							{
								bitmap.pix16(l*9 + y, i*6 + x) = BIT(region()->u8(m_ddram[char_pos]*8+y), 4-x);
							}
							else
							{
								if(m_ddram[char_pos] < 0xe0)
									bitmap.pix16(l*9 + y, i*6 + x) = BIT(region()->u8(m_ddram[char_pos]*8+y), 4-x);
								else
									bitmap.pix16(l*9 + y, i*6 + x) = BIT(region()->u8(0x700+((m_ddram[char_pos]-0xe0)*11)+y), 4-x);
							}
						}

				// if is the correct position draw cursor and blink
				if (char_pos == m_cursor_pos)
				{
					//draw the cursor
					if (m_cursor_on)
						for (int x=0; x<5; x++)
							bitmap.pix16(l*9 + 7, i * 6 + x) = 1;

					if (!m_blink && m_blink_on)
						for (int y=0; y<7; y++)
							for (int x=0; x<5; x++)
								bitmap.pix16(l*9 + y, i * 6 + x) = 1;
				}
			}

	return 0;
}

WRITE8_MEMBER(psion_custom_lcdc::control_write)
{
	if (BIT(data, 7)) // Set DDRAM Address
	{
		m_ac_mode = 0;
		m_ac = data & 0x7f;
		if (data != 0x81)
			m_cursor_pos = m_ac;
		set_busy_flag(37);
	}
	else
		hd44780_device::control_write(space, offset, data);
}
