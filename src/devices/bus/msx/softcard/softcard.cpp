// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "softcard.h"
#include "bus/msx/cart/softcard.h"


DECLARE_DEVICE_TYPE(SOFTCARD_NOMAPPER, softcard_nomapper_device)


void softcard(device_slot_interface &device)
{
	device.option_add_internal("nomapper", SOFTCARD_NOMAPPER);
}


softcard_interface::softcard_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "beecard")
	, m_exp(nullptr)
{
	for (int i = 0; i < 4; i++)
		m_page[i] = nullptr;
}

void softcard_interface::rom_alloc(u32 size)
{
	m_rom.resize(size);
	std::fill_n(m_rom.begin(), size, 0xff);
}

void softcard_interface::set_views(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3)
{
	m_page[0] = page0;
	m_page[1] = page1;
	m_page[2] = page2;
	m_page[3] = page3;
}


class softcard_nomapper_device : public device_t
								, public softcard_interface
{
public:
	softcard_nomapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override { }

	virtual void initialize_cartridge() override;

private:
	u32 m_start_address;
	u32 m_end_address;

	void install_memory();
};


DEFINE_DEVICE_TYPE(SOFTCARD_NOMAPPER, softcard_nomapper_device, "softcard_nomapper", "SoftCard ROM")


softcard_nomapper_device::softcard_nomapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SOFTCARD_NOMAPPER, tag, owner, clock)
	, softcard_interface(mconfig, *this)
	, m_start_address(0)
	, m_end_address(0)
{
}

void softcard_nomapper_device::install_memory()
{
	u32 start_address = m_start_address;
	u32 rom_offset = 0;

	if (m_start_address != 0x4000 && m_start_address != 0x8000)
	{
		fatalerror("softcard_nomapper: Unsupported start address %04x\n", m_start_address);
	}

	for (int i = 0; i < 4; i++)
	{
		if (start_address < (i + 1) * 0x4000 && start_address < m_end_address)
		{
			if (page(i))
				page(i)->install_rom(start_address, std::min<u32>(start_address + 0x3fff, m_end_address - 1), get_rom_base() + rom_offset);
			rom_offset += 0x4000;
			start_address += 0x4000;
		}
	}
}

void softcard_nomapper_device::initialize_cartridge()
{
	u32 size = get_rom_size();
	u8 *rom = get_rom_base();

	// determine start address, default to 0x4000
	m_start_address = 0x4000;

	switch (size)
	{
		// 16KB
		case 0x4000:
		{
			u16 start = rom[3] << 8 | rom[2];

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
				u16 start = rom[0x4003] << 8 | rom[0x4002];

				// start address of $0000 and call address in the $4000 region, or start address outside the $8000 region: $0000, else default
				if ((start == 0 && (rom[0x4005] & 0xc0) == 0x40) || start < 0x8000 || start >= 0xc000)
					m_start_address = 0;
			}

			break;
	}

	m_end_address = std::min<u32>(m_start_address + size, 0x10000);

	install_memory();
}
