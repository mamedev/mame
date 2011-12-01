/***************************************************************************

	Generic Palette RAMDAC device

	Written by Angelo Salese

	TODO:
	- read operation
	- masking

***************************************************************************/

#include "emu.h"
#include "video/ramdac.h"

// default address map
static ADDRESS_MAP_START( ramdac_palram, AS_0, 8 )
	AM_RANGE(0x000, 0x0ff) AM_RAM // R bank
	AM_RANGE(0x100, 0x1ff) AM_RAM // G bank
	AM_RANGE(0x200, 0x2ff) AM_RAM // B bank
	AM_RANGE(0x300, 0x3ff) AM_NOP
ADDRESS_MAP_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type RAMDAC = &device_creator<ramdac_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ramdac_device - constructor
//-------------------------------------------------

ramdac_device::ramdac_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RAMDAC, "ramdac", tag, owner, clock),
	  device_memory_interface(mconfig, *this),
		m_pal_index(0),
		m_pal_mask(0),
		m_space_config("videoram", ENDIANNESS_LITTLE, 8, 10, 0, NULL, *ADDRESS_MAP_NAME(ramdac_palram))
{

}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *ramdac_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline UINT8 ramdac_device::readbyte(offs_t address)
{
	return space()->read_byte(address);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void ramdac_device::writebyte(offs_t address, UINT8 data)
{
	space()->write_byte(address, data);
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

bool ramdac_device::device_validity_check(emu_options &options, const game_driver &driver) const
{
	bool error = false;
	return error;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ramdac_device::device_start()
{
	m_palram = auto_alloc_array_clear(machine(), UINT8, 1 << 10);

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ramdac_device::device_reset()
{
	m_pal_index = 0;
	m_pal_mask = 0;
	m_int_index = 0;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE8_MEMBER( ramdac_device::index_w )
{
	m_pal_index = data;
	m_int_index = 0;
}

WRITE8_MEMBER( ramdac_device::pal_w )
{
	writebyte(m_pal_index | (m_int_index << 8),data);

	m_int_index++;
	if(m_int_index == 3)
	{
		m_int_index = 0;
		m_pal_index++;
	}
}

WRITE8_MEMBER( ramdac_device::mask_w )
{
	m_pal_mask = data;
}

//**************************************************************************
//  Generic bank read/write handlers
//**************************************************************************

READ8_MEMBER( ramdac_device::ramdac_pal_r )
{
	return m_palram[offset];
}

WRITE8_MEMBER( ramdac_device::ramdac_rgb666_w )
{
	UINT16 pal_offs;

	m_palram[offset] = data & 0x3f;
	pal_offs = (offset & 0xff);

	palette_set_color_rgb(machine(),offset&0xff,pal6bit(m_palram[pal_offs|0x000]),pal6bit(m_palram[pal_offs|0x100]),pal6bit(m_palram[pal_offs|0x200]));
}

WRITE8_MEMBER( ramdac_device::ramdac_rgb888_w )
{
	UINT16 pal_offs;

	m_palram[offset] = data;
	pal_offs = (offset & 0xff);

	palette_set_color_rgb(machine(),offset&0xff,m_palram[pal_offs|0x000],m_palram[pal_offs|0x100],m_palram[pal_offs|0x200]);
}

