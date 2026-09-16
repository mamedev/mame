// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Epoch Super Cassette Vision cart emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


namespace {

class scv_rom8_device : public device_t,
						public device_scv_cart_interface
{
public:
	scv_rom8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: scv_rom8_device(mconfig, SCV_ROM8K, tag, owner, clock)
	{ }

	virtual void install_memory_handlers(address_space *space) override;

protected:
	scv_rom8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock), device_scv_cart_interface(mconfig, *this)
	{ }

	virtual void device_start() override { }
	virtual void device_reset() override { }

	static constexpr u16 CARTRIDGE_ADDRESS_END = 0xff7f;
};

void scv_rom8_device::install_memory_handlers(address_space *space)
{
	space->install_rom(0x8000, 0x9fff, cart_rom_region()->base());
	space->install_rom(0xa000, 0xbfff, cart_rom_region()->base());
	space->install_rom(0xc000, 0xdfff, cart_rom_region()->base());
	space->install_rom(0xe000, CARTRIDGE_ADDRESS_END, cart_rom_region()->base());
}



class scv_rom16_device : public scv_rom8_device
{
public:
	scv_rom16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: scv_rom8_device(mconfig, SCV_ROM16K, tag, owner, clock)
	{ }

	virtual void install_memory_handlers(address_space *space) override;
};

void scv_rom16_device::install_memory_handlers(address_space *space)
{
	space->install_rom(0x8000, 0xbfff, cart_rom_region()->base());
	space->install_rom(0xc000, CARTRIDGE_ADDRESS_END, cart_rom_region()->base());
}



class scv_rom32_device : public scv_rom8_device
{
public:
	scv_rom32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: scv_rom8_device(mconfig, SCV_ROM32K, tag, owner, clock)
	{ }

	virtual void install_memory_handlers(address_space *space) override;
};

void scv_rom32_device::install_memory_handlers(address_space *space)
{
	space->install_rom(0x8000, CARTRIDGE_ADDRESS_END, cart_rom_region()->base());
}



class scv_rom32ram8_device : public scv_rom8_device
{
public:
	scv_rom32ram8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: scv_rom8_device(mconfig, SCV_ROM32K_RAM8K, tag, owner, clock)
		, m_view(*this, "view")
	{ }

	virtual void install_memory_handlers(address_space *space) override;
	virtual void write_bank(uint8_t data) override;

protected:
	virtual void device_reset() override ATTR_COLD;

private:
	memory_view m_view;
};

void scv_rom32ram8_device::device_reset()
{
	m_view.select(1);
}

void scv_rom32ram8_device::install_memory_handlers(address_space *space)
{
	space->install_rom(0x8000, CARTRIDGE_ADDRESS_END, cart_rom_region()->base());
	space->install_view(0xe000, CARTRIDGE_ADDRESS_END, m_view);
	m_view[0];
	m_view[1].install_ram(0xe000, CARTRIDGE_ADDRESS_END, cart_ram_region()->base());
}

void scv_rom32ram8_device::write_bank(uint8_t data)
{
	m_view.select(BIT(data, 5));
}



class scv_rom64_device : public scv_rom8_device
{
public:
	scv_rom64_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: scv_rom8_device(mconfig, SCV_ROM64K, tag, owner, clock)
		, m_bank(*this, "bank")
	{ }

	virtual void install_memory_handlers(address_space *space) override;
	virtual void write_bank(uint8_t data) override;

protected:
	virtual void device_reset() override ATTR_COLD;

private:
	memory_bank_creator m_bank;
};

void scv_rom64_device::device_reset()
{
	m_bank->set_entry(0);
}

void scv_rom64_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 2, cart_rom_region()->base(), 0x8000);
	space->install_read_bank(0x8000, CARTRIDGE_ADDRESS_END, m_bank);
}

void scv_rom64_device::write_bank(uint8_t data)
{
	m_bank->set_entry(BIT(data, 5));
}



class scv_rom128_device : public scv_rom8_device
{
public:
	scv_rom128_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: scv_rom8_device(mconfig, SCV_ROM128K, tag, owner, clock)
		, m_bank(*this, "bank")
	{ }

	virtual void install_memory_handlers(address_space *space) override;
	virtual void write_bank(uint8_t data) override;

protected:
	virtual void device_reset() override ATTR_COLD;

private:
	memory_bank_creator m_bank;
};

void scv_rom128_device::device_reset()
{
	m_bank->set_entry(0);
}

void scv_rom128_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 4, cart_rom_region()->base(), 0x8000);
	space->install_read_bank(0x8000, CARTRIDGE_ADDRESS_END, m_bank);
}

void scv_rom128_device::write_bank(uint8_t data)
{
	m_bank->set_entry((data >> 5) & 0x03);
}



class scv_rom128ram4_device : public scv_rom8_device
{
public:
	scv_rom128ram4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: scv_rom8_device(mconfig, SCV_ROM128K_RAM4K, tag, owner, clock)
		, m_bank(*this, "bank")
		, m_view(*this, "view")
	{ }

	virtual void install_memory_handlers(address_space *space) override;
	virtual void write_bank(uint8_t data) override;

protected:
	virtual void device_reset() override ATTR_COLD;

private:
	memory_bank_creator m_bank;
	memory_view m_view;
};

void scv_rom128ram4_device::device_reset()
{
	m_bank->set_entry(0);
	m_view.select(0);
}

void scv_rom128ram4_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 4, cart_rom_region()->base(), 0x8000);
	space->install_read_bank(0x8000, CARTRIDGE_ADDRESS_END, m_bank);
	space->install_view(0xf000, CARTRIDGE_ADDRESS_END, m_view);
	m_view[0];
	m_view[1].install_ram(0xf000, CARTRIDGE_ADDRESS_END, cart_ram_region()->base());
}

void scv_rom128ram4_device::write_bank(uint8_t data)
{
	m_bank->set_entry((data >> 5) & 0x03);
	m_view.select(BIT(data, 6));
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(SCV_ROM8K,         device_scv_cart_interface, scv_rom8_device,       "scv_rom8",        "SCV 8K Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(SCV_ROM16K,        device_scv_cart_interface, scv_rom16_device,      "scv_rom16",       "SCV 16K Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(SCV_ROM32K,        device_scv_cart_interface, scv_rom32_device,      "scv_rom32",       "SCV 32K Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(SCV_ROM32K_RAM8K,  device_scv_cart_interface, scv_rom32ram8_device,  "scv_rom32_ram8",  "SCV 32K + RAM 8K Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(SCV_ROM64K,        device_scv_cart_interface, scv_rom64_device,      "scv_rom64",       "SCV 64K Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(SCV_ROM128K,       device_scv_cart_interface, scv_rom128_device,     "scv_rom128",      "SCV 128K Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(SCV_ROM128K_RAM4K, device_scv_cart_interface, scv_rom128ram4_device, "scv_rom128_ram4", "SCV 128K + RAM 4K Cartridge")
