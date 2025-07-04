// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    EPROM Switch Card

    https://site.acornatom.nl/hardware/memory-boards/schakelkaart/

**********************************************************************/

#include "emu.h"
#include "switch.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"


namespace {

class atom_switch_device : public device_t, public device_acorn_bus_interface
{
public:
	atom_switch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ATOM_SWITCH, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_rom_a(*this, "rom_a%x", 0)
		, m_rom_e(*this, "rom_e%u", 0)
		, m_switch(0)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device_array<generic_slot_device, 8> m_rom_a;
	required_device_array<generic_slot_device, 2> m_rom_e;

	uint8_t m_switch;

	uint8_t switch_r();
	void switch_w(uint8_t data);
	uint8_t rom_a000_r(offs_t offset);
	uint8_t rom_e000_r(offs_t offset);
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_switch_device::device_add_mconfig(machine_config &config)
{
	GENERIC_SOCKET(config, m_rom_a[0], generic_linear_slot, "atom_cart", "bin,rom");
	GENERIC_SOCKET(config, m_rom_a[1], generic_linear_slot, "atom_cart", "bin,rom");
	GENERIC_SOCKET(config, m_rom_a[2], generic_linear_slot, "atom_cart", "bin,rom");
	GENERIC_SOCKET(config, m_rom_a[3], generic_linear_slot, "atom_cart", "bin,rom");
	GENERIC_SOCKET(config, m_rom_a[4], generic_linear_slot, "atom_cart", "bin,rom");
	GENERIC_SOCKET(config, m_rom_a[5], generic_linear_slot, "atom_cart", "bin,rom");
	GENERIC_SOCKET(config, m_rom_a[6], generic_linear_slot, "atom_cart", "bin,rom");
	GENERIC_SOCKET(config, m_rom_a[7], generic_linear_slot, "atom_cart", "bin,rom");

	GENERIC_SOCKET(config, m_rom_e[0], generic_linear_slot, "atom_cart", "bin,rom");
	GENERIC_SOCKET(config, m_rom_e[1], generic_linear_slot, "atom_cart", "bin,rom");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atom_switch_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_read_handler(0xa000, 0xafff, emu::rw_delegate(*this, FUNC(atom_switch_device::rom_a000_r)));
	space.install_readwrite_handler(0xbfff, 0xbfff, emu::rw_delegate(*this, FUNC(atom_switch_device::switch_r)), emu::rw_delegate(*this, FUNC(atom_switch_device::switch_w)));
	space.install_read_handler(0xe000, 0xefff, emu::rw_delegate(*this, FUNC(atom_switch_device::rom_e000_r)));

	save_item(NAME(m_switch));
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t atom_switch_device::switch_r()
{
	return m_switch;
}

void atom_switch_device::switch_w(uint8_t data)
{
	/*
	   bit     description
	    0       block A bit 0
	    1       block A bit 1
	    2       block A bit 2
	    7       block E
	*/

	m_switch = data;
}


uint8_t atom_switch_device::rom_a000_r(offs_t offset)
{
	return m_rom_a[BIT(m_switch, 0, 3)]->read_rom(offset);
}

uint8_t atom_switch_device::rom_e000_r(offs_t offset)
{
	return m_rom_e[BIT(m_switch, 7, 1)]->read_rom(offset);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ATOM_SWITCH, device_acorn_bus_interface, atom_switch_device, "atom_switch", "Atom EPROM Switch Card")
