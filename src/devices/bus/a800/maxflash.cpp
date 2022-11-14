// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Maxflash

TODO:
- alt flash configs;
- alt buggy revision of 1MB version, with m_bank starting at 0x7f;

**************************************************************************************************/

#include "emu.h"
#include "maxflash.h"

// device type definition
DEFINE_DEVICE_TYPE(A800_MAXFLASH_128KB, a800_maxflash_128kb_device, "maxflash_128kb", "Atari 8-bit Atarimax Maxflash 128K flash ROM cart")
DEFINE_DEVICE_TYPE(A800_MAXFLASH_1MB,   a800_maxflash_1mb_device,   "maxflash_1mb",   "Atari 8-bit Atarimax Maxflash 1MB flash ROM cart")

a800_maxflash_128kb_device::a800_maxflash_128kb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, type, tag, owner, clock)
	, m_flash(*this, "flash")
	, m_bank(0)
{
}

a800_maxflash_128kb_device::a800_maxflash_128kb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: a800_maxflash_128kb_device(mconfig, A800_MAXFLASH_128KB, tag, owner, clock)
{
}

void a800_maxflash_128kb_device::device_add_mconfig(machine_config &config)
{
	AMD_29F010(config, m_flash);

	// TODO: alt config with Micron M29F010B ($20/$20)
}

void a800_maxflash_128kb_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_maxflash_128kb_device::device_reset()
{
	// TODO: ugly assignment, and shouldn't happen in device_reset
	// TODO: assert against intended size for slot
	memcpy(m_flash->base(), m_rom, get_rom_size());

	// NB: this starts with 0x7f in the older Atarimax 1MB version
	m_bank = 0;
}

void a800_maxflash_128kb_device::cart_map(address_map &map)
{
	map(0x2000, 0x3fff).lrw8(
		NAME([this](offs_t offset) {
			return m_flash->read((offset & 0x1fff) + (m_bank * 0x2000));
		}),
		NAME([this](offs_t offset, u8 data) {
			m_flash->write((offset & 0x1fff) + (m_bank * 0x2000), data);
		})
	);
}

void a800_maxflash_128kb_device::cctl_map(address_map &map)
{
	map(0x00, 0x0f).rw(FUNC(a800_maxflash_128kb_device::rom_bank_r), FUNC(a800_maxflash_128kb_device::rom_bank_w));
	map(0x10, 0x1f).rw(FUNC(a800_maxflash_128kb_device::disable_rom_r), FUNC(a800_maxflash_128kb_device::disable_rom_w));
}

uint8_t a800_maxflash_128kb_device::disable_rom_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
		rd5_w(0);

	return 0xff;
}

void a800_maxflash_128kb_device::disable_rom_w(offs_t offset, uint8_t data)
{
	rd5_w(0);
}

uint8_t a800_maxflash_128kb_device::rom_bank_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
	{
		rd5_w(1);
		m_bank = (offset & m_bank_mask);
	}
	return 0xff;
}

void a800_maxflash_128kb_device::rom_bank_w(offs_t offset, uint8_t data)
{
	rd5_w(1);
	m_bank = (offset & m_bank_mask);
}

// 8MB overrides

a800_maxflash_1mb_device::a800_maxflash_1mb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: a800_maxflash_128kb_device(mconfig, A800_MAXFLASH_1MB, tag, owner, clock)
{
}

void a800_maxflash_1mb_device::device_add_mconfig(machine_config &config)
{
	// TODO: AMD_29F040B according to Altirra?
	// AMD_29F040 doesn't match 0x100000 size requirement, 'B is still 4 megabits?
	// replace it with a suitable flash device for now
	INTEL_E28F008SA(config, m_flash);

	// TODO: alt config with Bright BM29F040 ($ad/$40)
}


void a800_maxflash_1mb_device::cctl_map(address_map &map)
{
	map(0x00, 0x7f).rw(FUNC(a800_maxflash_1mb_device::rom_bank_r), FUNC(a800_maxflash_1mb_device::rom_bank_w));
	map(0x80, 0xff).rw(FUNC(a800_maxflash_1mb_device::disable_rom_r), FUNC(a800_maxflash_1mb_device::disable_rom_w));
}
