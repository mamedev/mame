// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 RAM Module

****************************************************************************/

#include "emu.h"
#include "ram.h"

namespace {

//**************************************************************************
//  RC2014 32K RAM module
//  Module author: Spencer Owen
//**************************************************************************

class ram_32k_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	ram_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	std::unique_ptr<u8[]> m_ram;
};

ram_32k_device::ram_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_RAM_32K, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_ram(nullptr)
{
}

void ram_32k_device::device_start()
{
	m_ram = std::make_unique<u8[]>(0x8000);
	std::fill_n(m_ram.get(), 0x8000, 0xff);
	save_pointer(NAME(m_ram), 0x8000);

	m_bus->installer(AS_PROGRAM)->install_ram(0x8000, 0xffff, m_ram.get());
}

//**************************************************************************
//  RC2014 64K RAM module
//  Module author: Spencer Owen
//**************************************************************************

class ram_64k_device : public device_t, public device_rc2014_ext_card_interface
{
public:
	// construction/destruction
	ram_64k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_resolve_objects() override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER( page_w ) { m_bank = state; update_banks(); }
	void ram_w(offs_t offset, uint8_t data) { m_ram[offset] = data; }

	void update_banks();
private:
	int m_bank;
	std::unique_ptr<u8[]> m_ram;
	required_ioport m_start_addr;
	required_ioport m_paged;
};

ram_64k_device::ram_64k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_RAM_64K, tag, owner, clock)
	, device_rc2014_ext_card_interface(mconfig, *this)
	, m_ram(nullptr)
	, m_start_addr(*this, "START_ADDR")
	, m_paged(*this, "PAGED")
{
}

void ram_64k_device::device_start()
{
	m_ram = std::make_unique<u8[]>(0x10000);
	std::fill_n(m_ram.get(), 0x10000, 0xff);
	save_pointer(NAME(m_ram), 0x10000);
}

void ram_64k_device::device_reset()
{
	m_bank = 0;
	update_banks();
	if (m_paged->read())
		m_bus->installer(AS_PROGRAM)->install_ram(0x8000, 0xffff, m_ram.get() + 0x8000);
	else
		m_bus->installer(AS_PROGRAM)->install_ram(m_start_addr->read() * 0x1000, 0xffff, m_ram.get() + m_start_addr->read() * 0x1000);
}

void ram_64k_device::device_resolve_objects()
{
	m_bus->page_callback().append(*this, FUNC(ram_64k_device::page_w));
}

void ram_64k_device::update_banks()
{
	if (m_paged->read() == 0) return; // If not paged skip

	if (m_bank == 0) {
		m_bus->installer(AS_PROGRAM)->install_write_handler(0x0000, 0x7fff, write8sm_delegate(*this, FUNC(ram_64k_device::ram_w)));
	} else {
		m_bus->installer(AS_PROGRAM)->install_ram(0x0000, 0x7fff, m_ram.get());
	}
}

static INPUT_PORTS_START( ram_64k_jumpers )
	PORT_START("START_ADDR")
	PORT_CONFNAME( 0x4, 0x0, "Start address" )
	PORT_CONFSETTING( 0x0, "0x0000" )
	PORT_CONFSETTING( 0x1, "0x1000" )
	PORT_CONFSETTING( 0x2, "0x2000" )
	PORT_CONFSETTING( 0x4, "0x4000" )
	PORT_START("PAGED")
	PORT_CONFNAME( 0x1, 0x1, "Paged" )
	PORT_CONFSETTING( 0x0, DEF_STR( No ) )
	PORT_CONFSETTING( 0x1, DEF_STR( Yes ) )
INPUT_PORTS_END

ioport_constructor ram_64k_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ram_64k_jumpers );
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_RAM_32K, device_rc2014_card_interface, ram_32k_device, "rc2014_ram_32k", "RC2014 32K RAM module")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_RAM_64K, device_rc2014_ext_card_interface, ram_64k_device, "rc2014_ram_64k", "RC2014 64K RAM module")
