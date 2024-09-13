// license:BSD-3-Clause
// copyright-holders:S. Glaize

/***************************************************************************

    multiports_ext.cpp

    Emulation of the Alice Multiports Extension

    Features:
        The extension provides an extension doubler and two joystick ports.

        The extension also provides (for the whole Alice family and MC-10):
        - 16K of RAM expansion ($5000-$8FFF)
        - 64K of ROM expansion in two possible configurations:
            - 8K of ROM between $1000 and $2FFF, as 8 banks (Cartridge mode).
            - 16K of ROM between $C000 and $FFFF, as 4 banks (ROM mode).

        Only the RAM/ROM expansion is emulated here.

    Banks are selected by writing to:
        - $1000 to $1FFF in Cartridge mode (number of bank between 0 and 7)
        - $C000 to $CFFF in ROM mode (number of bank between 0 and 3)

***************************************************************************/

#include "emu.h"
#include "multiports_ext.h"


namespace {

//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

class mc10_multiports_ext_device : public device_t, public device_mc10cart_interface
{
public:
	mc10_multiports_ext_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual int max_rom_length() const override;

	virtual std::pair<std::error_condition, std::string> load() override;

protected:
	mc10_multiports_ext_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override;
	virtual void device_reset() override;

	void control_register_write(offs_t offset, u8 data);

	void multiports_mem(address_map &map);

private:
	memory_bank_creator m_bank;
	memory_share_creator<u8> m_extention_ram;
};

//-------------------------------------------------
//   IMPLEMENTATION
//-------------------------------------------------

mc10_multiports_ext_device::mc10_multiports_ext_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mc10_multiports_ext_device(mconfig, ALICE_MULTIPORTS_EXT, tag, owner, clock)
{
}

mc10_multiports_ext_device::mc10_multiports_ext_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mc10cart_interface(mconfig, *this)
	, m_bank(*this, "cart_bank")
	, m_extention_ram(*this, "ext_ram", 1024 * 16, ENDIANNESS_BIG)
{
}

int mc10_multiports_ext_device::max_rom_length() const
{
	return 1024 * 64;
}

void mc10_multiports_ext_device::multiports_mem(address_map &map)
{
	map(0x0000, 0x1fff).bankr("cart_bank").w(FUNC(mc10_multiports_ext_device::control_register_write));
}

//-------------------------------------------------

void mc10_multiports_ext_device::device_start()
{
	owning_slot().memspace().install_device(0x1000, 0x2fff, *this, &mc10_multiports_ext_device::multiports_mem);
	owning_slot().memspace().install_ram(0x5000, 0x8fff, &m_extention_ram[0]);
}

//-------------------------------------------------

void mc10_multiports_ext_device::device_reset()
{
	m_bank->set_entry(0);
}

void mc10_multiports_ext_device::control_register_write(offs_t offset, u8 data)
{
	if (offset < 0x1000)
	{
		m_bank->set_entry(data & 0x07);
	}
}

std::pair<std::error_condition, std::string> mc10_multiports_ext_device::load()
{
	memory_region *const romregion(memregion("^rom"));
	assert(romregion != nullptr);

	const u32 min_rom_length = 8 * 1024;
	const u32 block_rom_length = 8 * 1024;
	const u32 len = romregion->bytes();

	if (len < min_rom_length)
	{
		return std::make_pair(
			image_error::INVALIDLENGTH,
			util::string_format("Unsupported cartridge size (must be at least %u bytes)", min_rom_length));
	}
	else if (len % block_rom_length != 0)
	{
		return std::make_pair(
			image_error::INVALIDLENGTH,
			util::string_format("Unsupported cartridge size (must be a multiple of %u bytes)", block_rom_length));
	}

	m_bank->configure_entries(0, 8, romregion->base(), 0x2000);

	return std::make_pair(std::error_condition(), std::string());
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ALICE_MULTIPORTS_EXT, device_mc10cart_interface, mc10_multiports_ext_device, "mc10_multiports_ext", "Fred_72 and 6502man's Multiports Extension")
