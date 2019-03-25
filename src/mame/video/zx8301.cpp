// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair ZX8301 emulation

**********************************************************************/

/*

    TODO:

    - wait state on memory access during video update
    - proper video timing
    - get rid of flash timer

*/

#include "emu.h"
#include "zx8301.h"

#include "screen.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


// low resolution palette
static const int ZX8301_COLOR_MODE4[] = { 0, 2, 4, 7 };


static const rgb_t PALETTE_ZX8301[] =
{
	rgb_t(0x00, 0x00, 0x00), // black
	rgb_t(0x00, 0x00, 0xff), // blue
	rgb_t(0xff, 0x00, 0x00), // red
	rgb_t(0xff, 0x00, 0xff), // magenta
	rgb_t(0x00, 0xff, 0x00), // green
	rgb_t(0x00, 0xff, 0xff), // cyan
	rgb_t(0xff, 0xff, 0x00), // yellow
	rgb_t(0xff, 0xff, 0xff) // white
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(ZX8301, zx8301_device, "zx8301", "Sinclair ZX8301")


// default address map
void zx8301_device::zx8301(address_map &map)
{
	if (!has_configured_map(0))
		map(0x00000, 0x1ffff).ram();
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector zx8301_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline uint8_t zx8301_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void zx8301_device::writebyte(offs_t address, uint8_t data)
{
	space().write_byte(address, data);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  zx8301_device - constructor
//-------------------------------------------------

zx8301_device::zx8301_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ZX8301, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_space_config("videoram", ENDIANNESS_LITTLE, 8, 17, 0, address_map_constructor(FUNC(zx8301_device::zx8301), this))
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_write_vsync(*this)
	, m_dispoff(1)
	, m_mode8(0)
	, m_base(0)
	, m_flash(1)
	, m_vsync(1)
	, m_vda(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void zx8301_device::device_start()
{
	// resolve callbacks
	m_write_vsync.resolve_safe();

	// allocate timers
	m_vsync_timer = timer_alloc(TIMER_VSYNC);
	m_flash_timer = timer_alloc(TIMER_FLASH);

	// adjust timer periods
	m_vsync_timer->adjust(attotime::zero, 0, attotime::from_hz(50));
	m_flash_timer->adjust(attotime::from_hz(2), 0, attotime::from_hz(2));

	// register for state saving
	save_item(NAME(m_dispoff));
	save_item(NAME(m_mode8));
	save_item(NAME(m_base));
	save_item(NAME(m_flash));
	save_item(NAME(m_vsync));
	save_item(NAME(m_vda));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void zx8301_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_VSYNC:
		//m_vsync = !m_vsync;
		m_write_vsync(m_vsync);
		break;

	case TIMER_FLASH:
		m_flash = !m_flash;
		break;
	}
}


//-------------------------------------------------
//  control_w - display control register
//-------------------------------------------------

WRITE8_MEMBER( zx8301_device::control_w )
{
	/*

	    bit     description

	    0
	    1       display off
	    2
	    3       graphics mode
	    4
	    5
	    6
	    7       display base address

	*/

	if (LOG) logerror("ZX8301 Control: %02x\n", data);

	// display off
	m_dispoff = BIT(data, 1);

	// graphics mode
	m_mode8 = BIT(data, 3);

	// display base address
	m_base = BIT(data, 7);
}


//-------------------------------------------------
//  data_r - RAM read
//-------------------------------------------------

READ8_MEMBER( zx8301_device::data_r )
{
	if (LOG) logerror("ZX8301 RAM Read: %06x\n", offset);

	if (m_vda)
	{
		m_cpu->spin_until_time(screen().time_until_pos(256, 0));
	}

	return readbyte(offset);
}


//-------------------------------------------------
//  data_w - RAM write
//-------------------------------------------------

WRITE8_MEMBER( zx8301_device::data_w )
{
	if (LOG) logerror("ZX8301 RAM Write: %06x = %02x\n", offset, data);

	if (m_vda)
	{
		m_cpu->spin_until_time(screen().time_until_pos(256, 0));
	}

	writebyte(offset, data);
}


//-------------------------------------------------
//  draw_line_mode4 - draw mode 4 line
//-------------------------------------------------

void zx8301_device::draw_line_mode4(bitmap_rgb32 &bitmap, int y, uint16_t da)
{
	int x = 0;

	for (int word = 0; word < 64; word++)
	{
		uint8_t byte_high = readbyte(da++);
		uint8_t byte_low = readbyte(da++);

		for (int pixel = 0; pixel < 8; pixel++)
		{
			int red = BIT(byte_low, 7);
			int green = BIT(byte_high, 7);
			int color = (green << 1) | red;

			bitmap.pix32(y, x++) = PALETTE_ZX8301[ZX8301_COLOR_MODE4[color]];

			byte_high <<= 1;
			byte_low <<= 1;
		}
	}
}


//-------------------------------------------------
//  draw_line_mode8 - draw mode 8 line
//-------------------------------------------------

void zx8301_device::draw_line_mode8(bitmap_rgb32 &bitmap, int y, uint16_t da)
{
	int x = 0;

	for (int word = 0; word < 64; word++)
	{
		uint8_t byte_high = readbyte(da++);
		uint8_t byte_low = readbyte(da++);

		for (int pixel = 0; pixel < 4; pixel++)
		{
			int red = BIT(byte_low, 7);
			int green = BIT(byte_high, 7);
			int blue = BIT(byte_low, 6);
			int flash = BIT(byte_high, 6);

			int color = (green << 2) | (red << 1) | blue;

			if (flash && m_flash)
			{
				color = 0;
			}

			bitmap.pix32(y, x++) = PALETTE_ZX8301[color];
			bitmap.pix32(y, x++) = PALETTE_ZX8301[color];

			byte_high <<= 2;
			byte_low <<= 2;
		}
	}
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

uint32_t zx8301_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_dispoff)
	{
		uint32_t da = m_base << 15;

		for (int y = 0; y < 256; y++)
		{
			if (m_mode8)
			{
				draw_line_mode8(bitmap, y, da);
			}
			else
			{
				draw_line_mode4(bitmap, y, da);
			}

			da += 128;
		}
	}
	else
	{
		bitmap.fill(rgb_t::black(), cliprect);
	}

	return 0;
}
