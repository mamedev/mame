// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    Hitachi HD66421 LCD Controller/Driver

    (c) 2001-2007 Tim Schuerewegen

*/

#include "emu.h"
#include "hd66421.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG_LEVEL  1
#define _logerror(level,x)  do { if (LOG_LEVEL > level) logerror x; } while (0)

#define HD66421_RAM_SIZE  (HD66421_WIDTH * HD66421_HEIGHT / 4) // 2-bits per pixel

// R0 - control register 1
#define LCD_R0_RMW      0x80 // read-modify-write mode
#define LCD_R0_DISP     0x40 // display on/off
#define LCD_R0_STBY     0x20 // standby (internal operation and power circuit halt)
#define LCD_R0_PWR      0x10
#define LCD_R0_AMP      0x08
#define LCD_R0_REV      0x04 // reverse
#define LCD_R0_HOLT     0x02
#define LCD_R0_ADC      0x01

// R1 - control register 2
#define LCD_R1_BIS1     0x80 // bias ratio (bit 1)
#define LCD_R1_BIS0     0x40 // bias ratio (bit 0)
#define LCD_R1_WLS      0x20
#define LCD_R1_GRAY     0x10 // grayscale palette 4/32
#define LCD_R1_DTY1     0x08 // display duty cycle (bit 1)
#define LCD_R1_DTY0     0x04 // display duty cycle (bit 0)
#define LCD_R1_INC      0x02
#define LCD_R1_BLK      0x01 // blink function

// register 0 to 16
#define LCD_REG_CONTROL_1   0x00 // control register 1
#define LCD_REG_CONTROL_2   0x01 // control register 2
#define LCD_REG_ADDR_X      0x02 // x address register
#define LCD_REG_ADDR_Y      0x03 // y address register
#define LCD_REG_RAM         0x04 // display ram access register
#define LCD_REG_START_Y     0x05 // display start line register
#define LCD_REG_BLINK_START 0x06 // blink start line register
#define LCD_REG_BLINK_END   0x07 // blink end line register
#define LCD_REG_BLINK_1     0x08 // blink register 1
#define LCD_REG_BLINK_2     0x09 // blink register 2
#define LCD_REG_BLINK_3     0x0A // blink register 3
#define LCD_REG_PARTIAL     0x0B // partial display block register
#define LCD_REG_COLOR_1     0x0C // gray scale palette 1 (0,0)
#define LCD_REG_COLOR_2     0x0D // gray scale palette 2 (0,1)
#define LCD_REG_COLOR_3     0x0E // gray scale palette 3 (1,0)
#define LCD_REG_COLOR_4     0x0F // gray scale palette 4 (1,1)
#define LCD_REG_CONTRAST    0x10 // contrast control register
#define LCD_REG_PLANE       0x11 // plane selection register

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type HD66421 = &device_creator<hd66421_device>;


// default address map
static ADDRESS_MAP_START( hd66421, AS_0, 8, hd66421_device )
	AM_RANGE(0x0000, HD66421_RAM_SIZE) AM_RAM
ADDRESS_MAP_END

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *hd66421_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline UINT8 hd66421_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void hd66421_device::writebyte(offs_t address, UINT8 data)
{
	space().write_byte(address, data);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hd66421_device - constructor
//-------------------------------------------------

hd66421_device::hd66421_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, HD66421, "Hitachi HD66421 LCD Controller", tag, owner, clock, "hd66421", __FILE__),
		device_memory_interface(mconfig, *this),
		m_space_config("videoram", ENDIANNESS_LITTLE, 8, 17, 0, nullptr, *ADDRESS_MAP_NAME(hd66421)),
		m_cmd(0),
		m_x(0),
		m_y(0),
		m_palette(*this, "palette")
{
	for (auto & elem : m_reg)
	{
		elem = 0;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd66421_device::device_start()
{
	// register for state saving
	save_item(NAME(m_cmd));
	save_item(NAME(m_reg));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
}

READ8_MEMBER( hd66421_device::reg_idx_r )
{
	_logerror( 2, ("reg_idx_r\n"));
	return m_cmd;
}

WRITE8_MEMBER( hd66421_device::reg_idx_w )
{
	_logerror( 2, ("reg_idx_w (%02X)\n", data));
	m_cmd = data;
}

READ8_MEMBER( hd66421_device::reg_dat_r )
{
	_logerror( 2, ("reg_dat_r\n"));
	return m_reg[m_cmd];
}

WRITE8_MEMBER( hd66421_device::reg_dat_w )
{
	_logerror( 2, ("reg_dat_w (%02X)\n", data));
	m_reg[m_cmd] = data;

	switch (m_cmd)
	{
		case LCD_REG_ADDR_X :
			m_x = data;
			break;

		case LCD_REG_ADDR_Y :
			m_y = data;
			break;

		case LCD_REG_RAM :
		{
			UINT8 r1;
			writebyte(m_y * (HD66421_WIDTH / 4) + m_x, data);
			r1 = m_reg[LCD_REG_CONTROL_2];
			if (r1 & 0x02)
				m_x++;
			else
				m_y++;

			if (m_x >= (HD66421_WIDTH / 4))
			{
				m_x = 0;
				m_y++;
			}

			if (m_y >= HD66421_HEIGHT)
				m_y = 0;
		}
		break;
	}
}

void hd66421_device::plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color)
{
	bitmap.pix16(y, x) = (UINT16)color;
}

UINT32 hd66421_device::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	pen_t pen[4];

	_logerror( 1, ("video_update_hd66421\n"));

	// update palette
	for (int i = 0; i < 4; i++)
	{
		double bright;
		int temp;
		temp = 31 - (m_reg[LCD_REG_COLOR_1 + i] - m_reg[LCD_REG_CONTRAST] + 0x03);
		if (temp <  0) temp =  0;
		if (temp > 31) temp = 31;
		bright = 1.0 * temp / 31;
		pen[i] = i;
		#ifdef HD66421_BRIGHTNESS_DOES_NOT_WORK
		m_palette->set_pen_color(pen[i], 255 * bright, 255 * bright, 255 * bright);
		#else
		m_palette->set_pen_contrast(pen[i], bright);
		#endif
	}

	// draw bitmap (bottom to top)
	if (m_reg[0] & LCD_R0_DISP)
	{
		int x, y;
		x = 0;
		y = HD66421_HEIGHT - 1;

		for (int i = 0; i < HD66421_RAM_SIZE; i++)
		{
			plot_pixel(bitmap, x++, y, pen[(readbyte(i) >> 6) & 3]);
			plot_pixel(bitmap, x++, y, pen[(readbyte(i) >> 4) & 3]);
			plot_pixel(bitmap, x++, y, pen[(readbyte(i) >> 2) & 3]);
			plot_pixel(bitmap, x++, y, pen[(readbyte(i) >> 0) & 3]);
			if (x >= HD66421_WIDTH)
			{
				x = 0;
				y = y - 1;
			}
		}
	}
	else
	{
		rectangle rect(0, HD66421_WIDTH - 1, 0, HD66421_HEIGHT - 1);
		bitmap.fill(m_palette->white_pen(), rect);
	}

	return 0;
}

PALETTE_INIT_MEMBER(hd66421_device, hd66421)
{
	// init palette
	for (int i = 0; i < 4; i++)
	{
		palette.set_pen_color(i, rgb_t::white);
#ifndef HD66421_BRIGHTNESS_DOES_NOT_WORK
		palette.set_pen_contrast(i, 1.0 * i / (4 - 1));
#endif
	}
}


static MACHINE_CONFIG_FRAGMENT( hd66421 )
	MCFG_PALETTE_ADD("palette", 4)
	MCFG_PALETTE_INIT_OWNER(hd66421_device, hd66421)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor hd66421_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( hd66421 );
}
