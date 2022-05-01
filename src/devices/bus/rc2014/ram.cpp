// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 RAM Module

****************************************************************************/

#include "emu.h"
#include "machine/ram.h"
#include "ram.h"

class ram_32k_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	ram_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

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

void ram_32k_device::device_add_mconfig(machine_config &config)
{
	RAM(config, "ram").set_default_size("32K");
}

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_RAM_32K, device_rc2014_card_interface, ram_32k_device, "rc2014_32k", "RC2014 RAM 32K Module")
