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
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

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

//**************************************************************************
//  ram_64k_base
//**************************************************************************

class ram_64k_base : public device_t
{
protected:
	// construction/destruction
	ram_64k_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override { update_banks(); }

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void page_w(int state) { m_bank = state; update_banks(); }

	virtual void update_banks() = 0;

	// base-class members
	int m_bank;
	std::unique_ptr<u8[]> m_ram;
	required_ioport m_start_addr;
	required_ioport m_paged;
};

ram_64k_base::ram_64k_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_bank(0)
	, m_ram(nullptr)
	, m_start_addr(*this, "START_ADDR")
	, m_paged(*this, "PAGED")
{
}

void ram_64k_base::device_start()
{
	m_ram = std::make_unique<u8[]>(0x10000);
	std::fill_n(m_ram.get(), 0x10000, 0xff);
	save_pointer(NAME(m_ram), 0x10000);
	save_item(NAME(m_bank));
}

void ram_64k_base::device_reset()
{
	m_bank = 0;
	update_banks();
}

static INPUT_PORTS_START( ram_64k_jumpers )
	PORT_START("START_ADDR")
	PORT_CONFNAME( 0x7, 0x0, "Start address" )
	PORT_CONFSETTING( 0x0, "0x0000" )
	PORT_CONFSETTING( 0x1, "0x1000" )
	PORT_CONFSETTING( 0x2, "0x2000" )
	PORT_CONFSETTING( 0x4, "0x4000" )
	PORT_START("PAGED")
	PORT_CONFNAME( 0x1, 0x1, "Paged" )
	PORT_CONFSETTING( 0x0, DEF_STR( No ) )
	PORT_CONFSETTING( 0x1, DEF_STR( Yes ) )
INPUT_PORTS_END

ioport_constructor ram_64k_base::device_input_ports() const
{
	return INPUT_PORTS_NAME( ram_64k_jumpers );
}

//**************************************************************************
//  RC2014 64K RAM module in extended bus
//**************************************************************************

class ram_64k_device : public ram_64k_base, public device_rc2014_ext_card_interface
{
public:
	// construction/destruction
	ram_64k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_reset() override ATTR_COLD;

	// base-class overrides
	void update_banks() override;

	virtual void card_page_w(int state) override { page_w(state); }
};

ram_64k_device::ram_64k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ram_64k_base(mconfig, RC2014_RAM_64K, tag, owner, clock)
	, device_rc2014_ext_card_interface(mconfig, *this)
{
}

void ram_64k_device::device_reset()
{
	ram_64k_base::device_reset();
	if (m_paged->read())
		m_bus->installer(AS_PROGRAM)->install_ram(0x8000, 0xffff, m_ram.get() + 0x8000);
	else
		m_bus->installer(AS_PROGRAM)->install_ram(m_start_addr->read() * 0x1000, 0xffff, m_ram.get() + m_start_addr->read() * 0x1000);
}

void ram_64k_device::update_banks()
{
	if (m_paged->read() == 0) return; // If not paged skip

	if (m_bank == 0)
		m_bus->installer(AS_PROGRAM)->install_writeonly(0x0000, 0x7fff, m_ram.get());
	else
		m_bus->installer(AS_PROGRAM)->install_ram(0x0000, 0x7fff, m_ram.get());
}

//**************************************************************************
//  RC2014 64K RAM module in standard bus
//**************************************************************************

class ram_64k_device_40pin : public ram_64k_base, public device_rc2014_card_interface
{
public:
	// construction/destruction
	ram_64k_device_40pin(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_reset() override ATTR_COLD;

	// base-class overrides
	void update_banks() override {};
};

ram_64k_device_40pin::ram_64k_device_40pin(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ram_64k_base(mconfig, RC2014_RAM_64K_40P, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
{
}

void ram_64k_device_40pin::device_reset()
{
	ram_64k_base::device_reset();
	// page pin not connected so we can set all at once
	if (m_paged->read())
	{
		m_bus->installer(AS_PROGRAM)->install_ram(0x8000, 0xffff, m_ram.get() + 0x8000);
		m_bus->installer(AS_PROGRAM)->install_writeonly(0x0000, 0x7fff, m_ram.get());
	}
	else
	{
		m_bus->installer(AS_PROGRAM)->install_ram(m_start_addr->read() * 0x1000, 0xffff, m_ram.get() + m_start_addr->read() * 0x1000);
	}
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_RAM_32K, device_rc2014_card_interface, ram_32k_device, "rc2014_ram_32k", "RC2014 32K RAM module")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_RAM_64K, device_rc2014_ext_card_interface, ram_64k_device, "rc2014_ram_64k", "RC2014 64K RAM module")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_RAM_64K_40P, device_rc2014_card_interface, ram_64k_device_40pin, "rc2014_ram_64k_40p", "RC2014 64K RAM module (40 pin)")
