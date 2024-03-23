// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************************************

 Sega AI card emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


namespace {

class segaai_rom_128_device : public device_t,
								public segaai_card_interface
{
public:
	segaai_rom_128_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: segaai_rom_128_device(mconfig, SEGAAI_ROM_128, tag, owner, clock)
	{ }

	virtual void install_memory_handlers(address_space *space) override;

protected:
	segaai_rom_128_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
		, segaai_card_interface(mconfig, *this)
	{ }

	virtual void device_start() override { }
};

void segaai_rom_128_device::install_memory_handlers(address_space *space)
{
	space->install_rom(0xa0000, 0xbffff, cart_rom_region()->base());
}



class segaai_rom_256_device : public segaai_rom_128_device
{
public:
	segaai_rom_256_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: segaai_rom_128_device(mconfig, SEGAAI_ROM_256, tag, owner, clock)
		, m_rom_bank(*this, "rombank%u", 0U)
	{ }

	virtual void install_memory_handlers(address_space *space) override;

private:
	template <int Bank> void bank_w(u8 data);
	void unknown0_w(u8 data);
	void unknown1_w(u8 data);

	memory_bank_array_creator<2> m_rom_bank;
};

void segaai_rom_256_device::install_memory_handlers(address_space *space)
{
	for (int i = 0; i < 2; i++)
		m_rom_bank[i]->configure_entries(0, cart_rom_region()->bytes() / 0x4000, cart_rom_region()->base(), 0x4000);

	space->install_rom(0xa0000, 0xa3fff, 0x10000, cart_rom_region()->base());
	space->install_read_bank(0xa4000, 0xa7fff, 0x10000, m_rom_bank[0]);
	space->install_read_bank(0xa8000, 0xabfff, 0x10000, m_rom_bank[1]);
	space->install_write_handler(0xafffc, 0xafffc, 0, 0x10000, 0, emu::rw_delegate(*this, FUNC(segaai_rom_256_device::unknown0_w)));
	space->install_write_handler(0xafffd, 0xafffd, 0, 0x10000, 0, emu::rw_delegate(*this, FUNC(segaai_rom_256_device::unknown1_w)));
	space->install_write_handler(0xafffe, 0xafffe, 0, 0x10000, 0, emu::rw_delegate(*this, FUNC(segaai_rom_256_device::bank_w<0>)));
	space->install_write_handler(0xaffff, 0xaffff, 0, 0x10000, 0, emu::rw_delegate(*this, FUNC(segaai_rom_256_device::bank_w<1>)));
}

template <int Bank>
void segaai_rom_256_device::bank_w(u8 data)
{
	m_rom_bank[Bank]->set_entry(data & 0x0f);
}

void segaai_rom_256_device::unknown0_w(u8 data)
{
	// Unknown, upon start $00 is written and sometimes $80 after that
}

void segaai_rom_256_device::unknown1_w(u8 data)
{
	// Unknown, upon start $00 is written
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(SEGAAI_ROM_128, segaai_card_interface, segaai_rom_128_device, "segaai_rom_128", "Sega AI Card - 128KB")
DEFINE_DEVICE_TYPE_PRIVATE(SEGAAI_ROM_256, segaai_card_interface, segaai_rom_256_device, "segaai_rom_256", "Sega AI Card - 256KB")
