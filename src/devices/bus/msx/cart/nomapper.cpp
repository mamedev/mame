// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "nomapper.h"

namespace {

class msx_cart_nomapper_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_nomapper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MSX_CART_NOMAPPER, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_start_address(0)
		, m_end_address(0)
	{ }

	// device_t implementation
	virtual void device_start() override { }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

private:
	uint32_t m_start_address;
	uint32_t m_end_address;

	void install_memory();
};

void msx_cart_nomapper_device::install_memory()
{
	u32 start_address = m_start_address;
	u32 rom_offset = 0;

	if (m_start_address != 0x0000 && m_start_address != 0x4000 && m_start_address != 0x8000 && m_start_address != 0xc000)
	{
		fatalerror("nomapper: Unsupported start address %04x\n", m_start_address);
	}

	for (int i = 0; i < 4; i++)
	{
		if (start_address < (i + 1) * 0x4000 && start_address < m_end_address)
		{
			if (page(i))
			{
				page(i)->install_rom(start_address, std::min<uint32_t>(start_address + 0x3fff, m_end_address - 1), cart_rom_region()->base() + rom_offset);
			}
			rom_offset += 0x4000;
			start_address += 0x4000;
		}
	}
}

std::error_condition msx_cart_nomapper_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_nomapper_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	u8 *rom = cart_rom_region()->base();

	// 64KB images fill all pages.
	if (size == 0x10000)
	{
		m_start_address = 0;
	}
	else if (is_loaded_through_softlist())
	{
		const char *start_page_str = get_feature("start_page");
		if (!start_page_str)
		{
			message = "msx_cart_nomapper_device: Feature 'start_page' was not found.";
			return image_error::BADSOFTWARE;
		}
		u32 start_page = strtol(start_page_str, nullptr, 0);
		if (start_page > 2)
		{
			message = "msx_cart_nomapper_device: Invalid value for 'start_page', allowed values are 0, 1, and 2";
			return image_error::BADSOFTWARE;
		}

		m_start_address = start_page * 0x4000;
	}
	else
	{
		// Try to guess the start address from the rom contents.
		// determine start address, default to 0x4000
		m_start_address = 0x4000;

		switch (size)
		{
			// 8KB/16KB
			case 0x2000: case 0x4000:
			{
				uint16_t start = rom[3] << 8 | rom[2];

				// start address of $0000: call address in the $4000 region: $4000, else $8000
				if (start == 0)
				{
					if ((rom[5] & 0xc0) == 0x40)
						m_start_address = 0x4000;
					else
						m_start_address = 0x8000;
				}

				// start address in the $8000 region: $8000, else default
				else if ((start & 0xc000) == 0x8000)
					m_start_address = 0x8000;

				break;
			}

			// 32KB
			case 0x8000:
				// take default, check when no "AB" at $0000, but "AB" at $4000
				if (rom[0] != 'A' && rom[1] != 'B' && rom[0x4000] == 'A' && rom[0x4001] == 'B')
				{
					uint16_t start = rom[0x4003] << 8 | rom[0x4002];

					// start address of $0000 and call address in the $4000 region, or start address outside the $8000 region: $0000, else default
					if ((start == 0 && (rom[0x4005] & 0xc0) == 0x40) || start < 0x8000 || start >= 0xc000)
						m_start_address = 0;
				}

				break;

			// 48KB
			case 0xc000:
				// "AB" at $0000, but no "AB" at $4000, not "AB": $0000
				if (rom[0] == 'A' && rom[1] == 'B' && rom[0x4000] != 'A' && rom[0x4001] != 'B')
					m_start_address = 0x4000;
				else
					m_start_address = 0;

				break;
		}
	}

	m_end_address = std::min<u32>(m_start_address + size, 0x10000);

	install_memory();

	return std::error_condition();
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_NOMAPPER, msx_cart_interface, msx_cart_nomapper_device, "msx_cart_nomapper", "MSX Cartridge - ROM")
