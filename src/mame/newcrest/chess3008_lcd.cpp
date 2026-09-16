// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Newcrest CXG Chess 3008 LCD, with Sanyo LC7580 or LC7582.

Embedded LCD hardware used in:
- CXG Chess 3008
- CXG Super Enterprise C
- CXG Sphinx Titan
- CXG Sphinx Galaxy
- CXG Sphinx Dominator
- CXG Sphinx Commander

It drives 2 small digital watch LCDs, with several unused segments (eg. S, PM).
The only reason it's in a device, is so the digit unscrambling won't need to be
copy-pasted into several drivers.

TODO:
- add output callbacks if necessary

*******************************************************************************/

#include "emu.h"
#include "chess3008_lcd.h"


DEFINE_DEVICE_TYPE(CHESS3008_LCD, chess3008_lcd_device, "chess3008_lcd", "Newcrest CXG Chess 3008 LCD")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

chess3008_lcd_device::chess3008_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, CHESS3008_LCD, tag, owner, clock),
	m_lcd(*this, "lcd"),
	m_out_digit(*this, "digit%u", 0U),
	m_out_lcd(*this, "s%u.%u", 0U, 0U)
{
}


//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void chess3008_lcd_device::device_add_mconfig(machine_config &config)
{
	LC7580(config, m_lcd);
	m_lcd->write_segs().set(FUNC(chess3008_lcd_device::lcd_output_w));
}


//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void chess3008_lcd_device::device_start()
{
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

void chess3008_lcd_device::lcd_output_w(offs_t offset, u64 data)
{
	// output individual segments
	for (int i = 0; i < 52; i++)
		m_out_lcd[offset][i] = BIT(data, i);

	// unscramble digit 7segs
	static const u8 seg2digit[4*7] =
	{
		0x03, 0x04, 0x00, 0x40, 0x41, 0x02, 0x42,
		0x05, 0x06, 0x07, 0x48, 0x44, 0x45, 0x46,
		0x0c, 0x0d, 0x0b, 0x0a, 0x4a, 0x4c, 0x4b,
		0x0e, 0x0f, 0x10, 0x50, 0x4d, 0x4e, 0x4f
	};

	for (int i = 0; i < 8; i++)
	{
		u8 digit = 0;
		for (int seg = 0; seg < 7; seg++)
		{
			u8 bit = seg2digit[7 * (i & 3) + seg] + 26 * (i >> 2);
			digit |= m_out_lcd[BIT(bit, 6)][bit & 0x3f] << seg;
		}
		m_out_digit[i] = digit;
	}
}

void chess3008_lcd_device::lcd_w(u8 data)
{
	// d0: LC7580 DATA
	// d1: LC7580 CLK
	// d2: LC7580 CE
	m_lcd->data_w(BIT(data, 0));
	m_lcd->clk_w(BIT(data, 1));
	m_lcd->ce_w(BIT(data, 2));
}
