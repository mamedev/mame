// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    Neo Geo Memory card functions

    JEIDA V3 SRAM cards.  The BIOS supports 8-bit and 16-bit cards,
    in 2KiB, 4KiB, 6KiB, 8KiB, 10KiB, 14KiB and 16KiB capacities.

    8-bit cards are connected to the least significant byte of the
    bus, but the memory card is enabled by the /UDS signal.  This
    means only word accesses or accesses to the most significant
    byte will access the card.

    SNK sold 2K*8 cards cards as NEO-IC8.  Two variants are known,
    both using Sharp SRAMs and soldered CR2016 lithium coin cells:
    * C10075-X2-2 PCB with LH5116NA-10 SRAM
    * EZ866 PCB wtih LH5116HN-10 SRAM

    SNK cards had no attribute EEPROMs.

*********************************************************************/

#include "emu.h"
#include "ng_memcard.h"

#include "emuopts.h"


// device type definition
DEFINE_DEVICE_TYPE(NG_MEMCARD, ng_memcard_device, "ng_memcard", "NeoGeo Memory Card")

//-------------------------------------------------
//  ng_memcard_device - constructor
//-------------------------------------------------

ng_memcard_device::ng_memcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NG_MEMCARD, tag, owner, clock)
	, device_memcard_image_interface(mconfig, *this)
	, m_lock1(1)
	, m_unlock2(1)
	, m_regsel(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ng_memcard_device::device_start()
{
	save_item(NAME(m_memcard_data));
	save_item(NAME(m_lock1));
	save_item(NAME(m_unlock2));
	save_item(NAME(m_regsel));
}

/*-------------------------------------------------
    memcard_insert - insert an existing memory card
    with the given index
-------------------------------------------------*/

std::pair<std::error_condition, std::string> ng_memcard_device::call_load()
{
	if (length() != 0x800)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported memory card size (only 2K cards are supported)");

	fseek(0, SEEK_SET);
	size_t ret = fread(m_memcard_data, 0x800);
	if (ret != 0x800)
		return std::make_pair(image_error::UNSPECIFIED, "Error reading file");

	return std::make_pair(std::error_condition(), std::string());
}

void ng_memcard_device::call_unload()
{
	fseek(0, SEEK_SET);
	fwrite(m_memcard_data, 0x800);
}

std::pair<std::error_condition, std::string> ng_memcard_device::call_create(int format_type, util::option_resolution *format_options)
{
	memset(m_memcard_data, 0, 0x800);

	size_t const ret = fwrite(m_memcard_data, 0x800);
	if (ret != 0x800)
		return std::make_pair(image_error::UNSPECIFIED, "Error writing file");

	return std::make_pair(std::error_condition(), std::string());
}


uint16_t ng_memcard_device::read(offs_t offset)
{
	if (m_regsel)
		return 0xff00 | m_memcard_data[offset & 0x07ff];
	else
		return 0xffff;
}

void ng_memcard_device::write(offs_t offset, uint16_t data)
{
	if (!m_lock1 && m_unlock2)
		m_memcard_data[offset & 0x07ff] = uint8_t(data & 0x00ff);
}


void ng_memcard_device::lock1_w(int state)
{
	m_lock1 = state;
}

void ng_memcard_device::unlock2_w(int state)
{
	m_unlock2 = state;
}

void ng_memcard_device::regsel_w(int state)
{
	m_regsel = state;
}
