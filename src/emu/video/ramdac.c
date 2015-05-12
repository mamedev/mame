// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Generic Palette RAMDAC device

    Written by Angelo Salese

    TODO:
    - masking register, almost likely it controls rollback on incrementing
      r/w palette access;
    - needs information about different models and what exactly they does

***************************************************************************/

#include "emu.h"
#include "video/ramdac.h"

// default address map
static ADDRESS_MAP_START( ramdac_palram, AS_0, 8, ramdac_device )
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
	: device_t(mconfig, RAMDAC, "RAMDAC", tag, owner, clock, "ramdac", __FILE__),
		device_memory_interface(mconfig, *this),
		m_space_config("videoram", ENDIANNESS_LITTLE, 8, 10, 0, NULL, *ADDRESS_MAP_NAME(ramdac_palram)),
		m_palette(*this),
		m_split_read_reg(0)
{
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void ramdac_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<ramdac_device &>(device).m_palette.set_tag(tag);
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
	return space().read_byte(address);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void ramdac_device::writebyte(offs_t address, UINT8 data)
{
	space().write_byte(address, data);
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void ramdac_device::device_validity_check(validity_checker &valid) const
{
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
	m_pal_index[0] = 0;
	m_int_index[0] = 0;
	m_pal_index[1] = 0;
	m_int_index[1] = 0;
	m_pal_mask = 0xff;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//  [0] = W register, [1] = R register
//**************************************************************************

inline void ramdac_device::reg_increment(UINT8 inc_type)
{
	m_int_index[inc_type]++;
	if(m_int_index[inc_type] == 3)
	{
		m_int_index[inc_type] = 0;
		m_pal_index[inc_type]++;
	}
}

READ8_MEMBER( ramdac_device::index_r )
{
	return m_pal_index[0];
}

WRITE8_MEMBER( ramdac_device::index_w )
{
	m_pal_index[0] = data;
	m_int_index[0] = 0;
}

WRITE8_MEMBER( ramdac_device::index_r_w )
{
	m_pal_index[1] = data;
	m_int_index[1] = 0;
}

READ8_MEMBER( ramdac_device::pal_r )
{
	UINT8 res;
	res = readbyte(m_pal_index[m_split_read_reg] | (m_int_index[m_split_read_reg] << 8));
	reg_increment(m_split_read_reg);
	return res;
}

WRITE8_MEMBER( ramdac_device::pal_w )
{
	writebyte(m_pal_index[0] | (m_int_index[0] << 8),data);
	reg_increment(0);
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

	m_palette->set_pen_color(offset&0xff,pal6bit(m_palram[pal_offs|0x000]),pal6bit(m_palram[pal_offs|0x100]),pal6bit(m_palram[pal_offs|0x200]));
}

WRITE8_MEMBER( ramdac_device::ramdac_rgb888_w )
{
	UINT16 pal_offs;

	m_palram[offset] = data;
	pal_offs = (offset & 0xff);

	m_palette->set_pen_color(offset&0xff,m_palram[pal_offs|0x000],m_palram[pal_offs|0x100],m_palram[pal_offs|0x200]);
}
