// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Xicor X2201 1024 x 1 bit Nonvolatile Static RAM

    This was Xicor's first NOVRAM product, combining static RAM with
    an E²PROM overlay. Besides having separate data input and output
    lines (and only one of each), its interface differs slightly from
    the later NOVRAMs in requiring an active chip select during recall
    and store operations. The CS and STORE lines, however, may be
    deactivated before the operation completes (in 2 to 4 ms).

    While US patent 4535411A suggests that this device was sometimes
    used in groups of eight to simulate a byte-wide memory, the
    emulation (for the most part) assumes each bit will be accessed
    separately and packs them into bytes for storage efficiency.

    X2202 has the same internal organization and data addressing as
    X2201, but its recall operation works on individual bits instead
    of the entire array.

**********************************************************************/

#include "emu.h"
#include "machine/x2201.h"

#include <algorithm>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(X2201, x2201_device, "x2201", "Xicor X2201 1024x1 NOVRAM")


//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  x2201_device - constructor
//-------------------------------------------------

x2201_device::x2201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, X2201, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_default_data(*this, DEVICE_SELF)
	, m_cs(false)
	, m_store(false)
	, m_array_recall(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void x2201_device::device_start()
{
	// create arrays
	m_ram = make_unique_clear<u8[]>(1024 / 8);
	m_eeprom = std::make_unique<u8[]>(1024 / 8);

	// register state for saving
	save_pointer(NAME(m_ram), 1024 / 8);
	save_pointer(NAME(m_eeprom), 1024 / 8);
	save_item(NAME(m_cs));
	save_item(NAME(m_store));
	save_item(NAME(m_array_recall));
}


//**************************************************************************
//  NVRAM INTERFACE
//**************************************************************************

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void x2201_device::nvram_default()
{
	// erase to ones unless region overrides
	if (m_default_data.found())
		std::copy_n(&m_default_data[0], 1024 / 8, &m_eeprom[0]);
	else
		std::fill_n(&m_eeprom[0], 1024 / 8, 0xff);
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  specified file
//-------------------------------------------------

bool x2201_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(&m_eeprom[0], 1024 / 8, actual) && actual == 1024 / 8;
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  specified file
//-------------------------------------------------

bool x2201_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(&m_eeprom[0], 1024 / 8, actual) && actual == 1024 / 8;
}


//**************************************************************************
//  MEMORY ACCESS
//**************************************************************************

//-------------------------------------------------
//  read - read the addressed bit of RAM
//-------------------------------------------------

u8 x2201_device::read(offs_t offset)
{
	return BIT(m_ram[(offset >> 3) & 127], offset & 7);
}


//-------------------------------------------------
//  write - write one bit of data to RAM
//-------------------------------------------------

void x2201_device::write(offs_t offset, u8 data)
{
	offs_t address = (offset >> 3) & 127;
	if (BIT(data, 0))
		m_ram[address] |= 1 << (offset & 7);
	else
		m_ram[address] &= ~(1 << (offset & 7));
}


//**************************************************************************
//  CONTROL STROBES
//**************************************************************************

//-------------------------------------------------
//  cs_w - write to the CS line (active low)
//-------------------------------------------------

WRITE_LINE_MEMBER(x2201_device::cs_w)
{
	m_cs = !state;
}


//-------------------------------------------------
//  store_w - trigger to store RAM data in EEPROM
//  (active low)
//-------------------------------------------------

WRITE_LINE_MEMBER(x2201_device::store_w)
{
	if (m_cs && !state && !m_store)
		std::copy_n(&m_ram[0], 1024 / 8, &m_eeprom[0]);

	m_array_recall = !state;
}


//-------------------------------------------------
//  array_recall_w - trigger to pull EEPROM data
//  into RAM (active low)
//-------------------------------------------------

WRITE_LINE_MEMBER(x2201_device::array_recall_w)
{
	if (m_cs && !state && !m_array_recall)
		std::copy_n(&m_eeprom[0], 1024 / 8, &m_ram[0]);

	m_array_recall = !state;
}
