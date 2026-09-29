// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

SIC! "Super Inexpensive Cartridge!"

TODO:
- Unknown flash types used, Altirra just observed a Winbond 29C020;
- Customizable Jumper/switch handling for pin signals;

**************************************************************************************************/

#include "emu.h"
#include "sic.h"

// device type definition
DEFINE_DEVICE_TYPE(A800_SIC_128KB, a800_sic_128kb_device, "a800_siccart_128kb", "Atari 8-bit SIC! 128KB flash ROM cart")
DEFINE_DEVICE_TYPE(A800_SIC_256KB, a800_sic_256kb_device, "a800_siccart_256kb", "Atari 8-bit SIC! 256KB flash ROM cart")
DEFINE_DEVICE_TYPE(A800_SIC_512KB, a800_sic_512kb_device, "a800_siccart_512kb", "Atari 8-bit SIC! 512KB flash ROM cart")

a800_sic_128kb_device::a800_sic_128kb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, type, tag, owner, clock)
	, m_flash(*this, "flash")
	, m_bank(0)
	, m_write_protect(true)
{
}

a800_sic_128kb_device::a800_sic_128kb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: a800_sic_128kb_device(mconfig, A800_SIC_128KB, tag, owner, clock)
{
}

void a800_sic_128kb_device::device_add_mconfig(machine_config &config)
{
	AMD_29F010(config, m_flash);
}

void a800_sic_128kb_device::device_start()
{
	save_item(NAME(m_bank));
	save_item(NAME(m_write_protect));
}

void a800_sic_128kb_device::device_reset()
{
	// TODO: ugly assignment, and shouldn't happen in device_reset
	// TODO: assert against intended size for slot
	memcpy(m_flash->base(), m_rom, get_rom_size());
	m_bank_mask = (get_rom_size() / 0x4000) - 1;

	// value of 0 for config_bank_w confirmed
	m_bank = 0;
	m_write_protect = true;
}

// SIC! maps rd4 and rd5 linearly
// so that 0x8000 maps to 0 and 0xa000 to 0x2000
u8 a800_sic_128kb_device::read(offs_t offset)
{
	return m_flash->read((offset & 0x3fff) | (m_bank * 0x4000));
}

void a800_sic_128kb_device::write(offs_t offset, u8 data)
{
	if (!m_write_protect)
		m_flash->write((offset & 0x3fff) | (m_bank * 0x4000), data);
}

void a800_sic_128kb_device::cart_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(a800_sic_128kb_device::read), FUNC(a800_sic_128kb_device::write));
}

/*
 * 1--- ---- enable flash write
 * -1-- ---- RD4 enable
 * --0- ---- RD5 enable
 * ---x xxxx ROM bank, where upper bits are unused for 128KB and 256KB versions
 */
void a800_sic_128kb_device::config_bank_w(offs_t offset, u8 data)
{
	m_bank = (data & m_bank_mask);
	rd4_w(BIT(data, 5));
	rd5_w(!(BIT(data, 6)));
	m_write_protect = !(BIT(data, 7));
}

void a800_sic_128kb_device::cctl_map(address_map &map)
{
	map(0x00, 0x1f).w(FUNC(a800_sic_128kb_device::config_bank_w));
}

// 256/512KB variants

a800_sic_256kb_device::a800_sic_256kb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: a800_sic_128kb_device(mconfig, A800_SIC_256KB, tag, owner, clock)
{
}

void a800_sic_256kb_device::device_add_mconfig(machine_config &config)
{
	AMD_29LV200T(config, m_flash);
}

a800_sic_512kb_device::a800_sic_512kb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: a800_sic_128kb_device(mconfig, A800_SIC_512KB, tag, owner, clock)
{
}

void a800_sic_512kb_device::device_add_mconfig(machine_config &config)
{
	AMD_29F040(config, m_flash);
}
