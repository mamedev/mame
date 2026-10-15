// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 128 VDC video RAM emulation

**********************************************************************/

#include "emu.h"
#include "vdcram.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C128_VDC_RAM_SLOT, c128_vdc_ram_slot_device, "c128_vdc_ram_slot", "C128 VDC RAM slot")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

device_c128_vdc_ram_interface::device_c128_vdc_ram_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "c128vdcram")
{
}



//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

c128_vdc_ram_slot_device::c128_vdc_ram_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C128_VDC_RAM_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_c128_vdc_ram_interface>(mconfig, *this)
{
}

void c128_vdc_ram_slot_device::map(address_map &map)
{
	device_c128_vdc_ram_interface *const card = get_card_device();

	if (card)
		map(0x0000, 0xffff).m(*card, FUNC(device_c128_vdc_ram_interface::map));
}



//**************************************************************************
//  CARDS
//**************************************************************************

namespace {

class c128_vdc_ram_4416_device : public device_t, public device_c128_vdc_ram_interface
{
public:
	c128_vdc_ram_4416_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		device_t(mconfig, C128_VDC_RAM_4416, tag, owner, clock),
		device_c128_vdc_ram_interface(mconfig, *this),
		m_ram(*this, "ram", 0x4000, ENDIANNESS_LITTLE)
	{
	}

	// device_c128_vdc_ram_interface implementation
	virtual void map(address_map &map) override ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD { }

private:
	memory_share_creator<uint8_t> m_ram;

	static offs_t decode(offs_t offset) { return ((offset & 0x7e00) >> 1) | (offset & 0x00ff); }

	uint8_t read(offs_t offset) { return m_ram[decode(offset)]; }
	void write(offs_t offset, uint8_t data) { m_ram[decode(offset)] = data; }
};

void c128_vdc_ram_4416_device::map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(c128_vdc_ram_4416_device::read), FUNC(c128_vdc_ram_4416_device::write));
}


class c128_vdc_ram_4464_device : public device_t, public device_c128_vdc_ram_interface
{
public:
	c128_vdc_ram_4464_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		device_t(mconfig, C128_VDC_RAM_4464, tag, owner, clock),
		device_c128_vdc_ram_interface(mconfig, *this)
	{
	}

	// device_c128_vdc_ram_interface implementation
	virtual void map(address_map &map) override ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD { }
};

void c128_vdc_ram_4464_device::map(address_map &map)
{
	map(0x0000, 0xffff).ram();
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(C128_VDC_RAM_4416, device_c128_vdc_ram_interface, c128_vdc_ram_4416_device, "c128_vdc_ram_4416", "C128 VDC RAM 16K (2x 4416)")
DEFINE_DEVICE_TYPE_PRIVATE(C128_VDC_RAM_4464, device_c128_vdc_ram_interface, c128_vdc_ram_4464_device, "c128_vdc_ram_4464", "C128 VDC RAM 64K (2x 4464)")



//**************************************************************************
//  SLOT OPTIONS
//**************************************************************************

void c128_vdc_ram_cards(device_slot_interface &device)
{
	device.option_add("16k", C128_VDC_RAM_4416);
	device.option_add("64k", C128_VDC_RAM_4464);
}
