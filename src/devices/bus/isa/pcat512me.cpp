// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM Personal Computer AT 512Kb Memory Expansion.
 *
 * Sources:
 *  - IBM Personal Computer Hardware Reference Library, August 31 1984, International Business Machines Corporation.
 *
 */
#include "emu.h"
#include "pcat512me.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class isa16_pcat512me_device
	: public device_t
	, public device_isa16_card_interface
{
public:
	isa16_pcat512me_device(machine_config const &mconfig, char const *const tag, device_t *owner, u32 clock)
		: device_t(mconfig, ISA16_PCAT512ME, tag, owner, clock)
		, device_isa16_card_interface(mconfig, *this)
		, m_bank(*this, "BANK")
		, m_ram(nullptr)
		, m_installed(false)
	{
	}

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	static constexpr u32 RAM_SIZE = 0x8'0000;

	required_ioport m_bank;
	std::unique_ptr<u8[]> m_ram;

	bool m_installed;
};

void isa16_pcat512me_device::device_start()
{
	m_ram = std::make_unique<u8[]>(RAM_SIZE);

	save_pointer(NAME(m_ram), RAM_SIZE);

	set_isa_device();
}

void isa16_pcat512me_device::device_reset()
{
	if (!m_installed)
	{
		u32 const base = BIT(m_bank->read(), 11, 5) * RAM_SIZE;
		LOG("ram installed at 0x%06x-0x%06x\n", base, base + RAM_SIZE -  1);

		m_isa->memspace().install_ram(base, base + RAM_SIZE - 1, m_ram.get());

		m_installed = true;
	}
}

INPUT_PORTS_START(pcat512me)
	PORT_START("BANK")
	PORT_DIPNAME(0xffff, 0x1015, "Base Address") PORT_DIPLOCATION("W:16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1")

	// invalid settings for a PC AT (conflict with motherboard resources)
	PORT_DIPSETTING(0x0005, "0x00'0000") // valid for PC RT coprocessor
	PORT_DIPSETTING(0x080d, "0x08'0000")

	// valid settings for a PC AT
	PORT_DIPSETTING(0x1015, "0x10'0000")
	PORT_DIPSETTING(0x181d, "0x18'0000")
	PORT_DIPSETTING(0x2025, "0x20'0000")
	PORT_DIPSETTING(0x282d, "0x28'0000")
	PORT_DIPSETTING(0x3035, "0x30'0000")
	PORT_DIPSETTING(0x383d, "0x38'0000")
	PORT_DIPSETTING(0x4045, "0x40'0000")
	PORT_DIPSETTING(0x484d, "0x48'0000")
	PORT_DIPSETTING(0x5055, "0x50'0000")
	PORT_DIPSETTING(0x585d, "0x58'0000")
	PORT_DIPSETTING(0x6065, "0x60'0000")
	PORT_DIPSETTING(0x686d, "0x68'0000")
	PORT_DIPSETTING(0x7075, "0x70'0000")
	PORT_DIPSETTING(0x787d, "0x78'0000")
	PORT_DIPSETTING(0x8085, "0x80'0000")
	PORT_DIPSETTING(0x888d, "0x88'0000")
	PORT_DIPSETTING(0x9095, "0x90'0000")
	PORT_DIPSETTING(0x989d, "0x98'0000")
	PORT_DIPSETTING(0xa0a5, "0xa0'0000")
	PORT_DIPSETTING(0xa8ad, "0xa8'0000")
	PORT_DIPSETTING(0xb0b5, "0xb0'0000")
	PORT_DIPSETTING(0xb8bd, "0xb8'0000")
	PORT_DIPSETTING(0xc0c5, "0xc0'0000")
	PORT_DIPSETTING(0xc8cd, "0xc8'0000")
	PORT_DIPSETTING(0xd0d5, "0xd0'0000")
	PORT_DIPSETTING(0xd8dd, "0xd8'0000")
	PORT_DIPSETTING(0xe0e5, "0xe0'0000")
	PORT_DIPSETTING(0xe8ed, "0xe8'0000")
	PORT_DIPSETTING(0xf0f5, "0xf0'0000")
	PORT_DIPSETTING(0xf8fd, "0xf8'0000")
INPUT_PORTS_END

ioport_constructor isa16_pcat512me_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(pcat512me);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ISA16_PCAT512ME, device_isa16_card_interface, isa16_pcat512me_device, "pcat512me", "IBM Personal Computer AT 512Kb Memory Expansion")
