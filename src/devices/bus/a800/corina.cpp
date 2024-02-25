// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Corina

Known configs:
- 1MB Flash ROM (yakungfu)
- 512KB Flash ROM + 512KB SRAM (bombjake)

Both contains 8KB NVRAM

TODO:
- Pinpoint and convert to flash interface;
- Unknown behaviour when accessing the "reserved" view;

**************************************************************************************************/

#include "emu.h"
#include "corina.h"

DEFINE_DEVICE_TYPE(A800_ROM_CORINA,      a800_rom_corina_device,         "a800_corina",        "Atari 8-bit Corina 1MB flash ROM cart")
DEFINE_DEVICE_TYPE(A800_ROM_CORINA_SRAM, a800_rom_corina_sram_device,    "a800_corina_sram",   "Atari 8-bit Corina 512KB flash ROM + 512KB RAM cart")

a800_rom_corina_device::a800_rom_corina_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, type, tag, owner, clock)
	, m_nvram(*this, "nvram")
	, m_view(*this, "corina_view")
	, m_rom_bank(0)
{
}

a800_rom_corina_device::a800_rom_corina_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_corina_device(mconfig, A800_ROM_CORINA, tag, owner, clock)
{
}

void a800_rom_corina_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

void a800_rom_corina_device::device_start()
{
	const u32 nvram_size = 0x2000;
	m_nvram_ptr = std::make_unique<uint8_t[]>(nvram_size);
	m_nvram->set_base(m_nvram_ptr.get(), nvram_size);

	save_pointer(NAME(m_nvram_ptr), nvram_size);
	save_item(NAME(m_rom_bank));
}

void a800_rom_corina_device::device_reset()
{
	m_rom_bank = 0;
	m_view.select(0);
}

void a800_rom_corina_device::cart_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).view(m_view);
	// TODO: spam writes during loading, flash commands?
	m_view[0](0x0000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x3fff) + (m_rom_bank * 0x4000)]; })
	);
	m_view[1](0x0000, 0x3fff).rw(FUNC(a800_rom_corina_device::read_view_1), FUNC(a800_rom_corina_device::write_view_1));
	m_view[2](0x0000, 0x3fff).lrw8(
		NAME([this](offs_t offset) { return m_nvram_ptr[offset & 0x1fff]; }),
		NAME([this](offs_t offset, uint8_t data) { m_nvram_ptr[offset & 0x1fff] = data; })
	);
	m_view[3](0x0000, 0x3fff).unmaprw();
}

void a800_rom_corina_device::cctl_map(address_map &map)
{
	map(0x00, 0x00).w(FUNC(a800_rom_corina_device::ctrl_w));
}

uint8_t a800_rom_corina_device::read_view_1(offs_t offset)
{
	return m_rom[(offset & 0x3fff) + (m_rom_bank * 0x4000) + 0x80000];
}

void a800_rom_corina_device::write_view_1(offs_t offset, u8 data)
{
}

/*
 * 0--- ---- enable Corina window
 * 1--- ---- disable Corina and select main unit 8000-bfff window instead
 * -xx- ---- view select
 * -00- ---- first half of ROM
 * -01- ---- second half of ROM or RAM (^ depending on PCB config)
 * -10- ---- NVRAM
 * -11- ---- <reserved>
 * ---x xxxx ROM/RAM lower bank value,
 *           ignored if view select is not in ROM/RAM mode
 *           or Corina window is disabled
 */
void a800_rom_corina_device::ctrl_w(offs_t offset, uint8_t data)
{
	m_rom_bank = data & 0x1f;
	m_view.select((data & 0x60) >> 5);
	rd_both_w(!BIT(data, 7));
}

/*-------------------------------------------------

 SRAM variant overrides

 -------------------------------------------------*/

a800_rom_corina_sram_device::a800_rom_corina_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_corina_device(mconfig, A800_ROM_CORINA_SRAM, tag, owner, clock)
{
}

void a800_rom_corina_sram_device::device_start()
{
	a800_rom_corina_device::device_start();

	m_ram.resize(0x80000);
	save_item(NAME(m_ram));
}

uint8_t a800_rom_corina_sram_device::read_view_1(offs_t offset)
{
	return m_ram[(offset & 0x3fff) + (m_rom_bank * 0x4000)];
}

void a800_rom_corina_sram_device::write_view_1(offs_t offset, u8 data)
{
	m_ram[(offset & 0x3fff) + (m_rom_bank * 0x4000)] = data;
}
