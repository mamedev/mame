// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 SuperFX add-on chip emulation (for SNES/SFC)

 ***********************************************************************************************************/


#include "emu.h"
#include "sfx.h"

//-------------------------------------------------
//  sns_rom_superfx_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(SNS_LOROM_SUPERFX1, sns_rom_superfx1_device, "sns_rom_superfx1", "SNES Cart (LoROM) + SuperFX 1")
DEFINE_DEVICE_TYPE(SNS_LOROM_SUPERFX2, sns_rom_superfx2_device, "sns_rom_superfx2", "SNES Cart (LoROM) + SuperFX 2")


sns_rom_superfx_device::sns_rom_superfx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_device(mconfig, type, tag, owner, clock)
	, m_superfx(*this, "superfx")
{
}

sns_rom_superfx1_device::sns_rom_superfx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_superfx_device(mconfig, SNS_LOROM_SUPERFX1, tag, owner, clock)
{
}

sns_rom_superfx2_device::sns_rom_superfx2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_superfx_device(mconfig, SNS_LOROM_SUPERFX2, tag, owner, clock)
{
}

void sns_rom_superfx_device::device_start()
{
	save_item(NAME(sfx_ram));
}

void sns_rom_superfx_device::device_reset()
{
	memset(sfx_ram, 0x00, sizeof(sfx_ram));
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

// LoROM + SuperFX (GSU-1,2)
// TODO: mask sfx_ram based on the actual RAM...

uint8_t sns_rom_superfx_device::superfx_r_bank1(offs_t offset)
{
	return m_rom[rom_bank_map[offset / 0x10000] * 0x8000 + (offset & 0x7fff)];
}

uint8_t sns_rom_superfx_device::superfx_r_bank2(offs_t offset)
{
	return m_rom[rom_bank_map[offset / 0x8000] * 0x8000 + (offset & 0x7fff)];
}

uint8_t sns_rom_superfx_device::superfx_r_bank3(offs_t offset)
{
	return sfx_ram[offset & 0xfffff];
}

void sns_rom_superfx_device::superfx_w_bank1(offs_t offset, uint8_t data)
{
}

void sns_rom_superfx_device::superfx_w_bank2(offs_t offset, uint8_t data)
{
}

void sns_rom_superfx_device::superfx_w_bank3(offs_t offset, uint8_t data)
{
	sfx_ram[offset & 0xfffff] = data;
}

void sns_rom_superfx_device::sfx_map(address_map &map)
{
	map(0x000000, 0x3fffff).rw(FUNC(sns_rom_superfx_device::superfx_r_bank1), FUNC(sns_rom_superfx_device::superfx_w_bank1));
	map(0x400000, 0x5fffff).rw(FUNC(sns_rom_superfx_device::superfx_r_bank2), FUNC(sns_rom_superfx_device::superfx_w_bank2));
	map(0x600000, 0x7dffff).rw(FUNC(sns_rom_superfx_device::superfx_r_bank3), FUNC(sns_rom_superfx_device::superfx_w_bank3));
	map(0x800000, 0xbfffff).rw(FUNC(sns_rom_superfx_device::superfx_r_bank1), FUNC(sns_rom_superfx_device::superfx_w_bank1));
	map(0xc00000, 0xdfffff).rw(FUNC(sns_rom_superfx_device::superfx_r_bank2), FUNC(sns_rom_superfx_device::superfx_w_bank2));
	map(0xe00000, 0xffffff).rw(FUNC(sns_rom_superfx_device::superfx_r_bank3), FUNC(sns_rom_superfx_device::superfx_w_bank3));
}


WRITE_LINE_MEMBER(sns_rom_superfx_device::snes_extern_irq_w)
{
	write_irq(state);
}


void sns_rom_superfx1_device::device_add_mconfig(machine_config &config)
{
	SUPERFX1(config, m_superfx, DERIVED_CLOCK(1, 1));  /* 21.48MHz, with internal /2 divider */
	m_superfx->set_addrmap(AS_PROGRAM, &sns_rom_superfx1_device::sfx_map);
	m_superfx->irq().set(FUNC(sns_rom_superfx1_device::snes_extern_irq_w));  /* IRQ line from cart */
}

void sns_rom_superfx2_device::device_add_mconfig(machine_config &config)
{
	SUPERFX2(config, m_superfx, DERIVED_CLOCK(1, 1));  /* 21.48MHz */
	m_superfx->set_addrmap(AS_PROGRAM, &sns_rom_superfx2_device::sfx_map);
	m_superfx->irq().set(FUNC(sns_rom_superfx2_device::snes_extern_irq_w));  /* IRQ line from cart */
}

uint8_t sns_rom_superfx_device::chip_read(offs_t offset)
{
	return m_superfx->mmio_read(offset);
}

void sns_rom_superfx_device::chip_write(offs_t offset, uint8_t data)
{
	m_superfx->mmio_write(offset, data);
}


uint8_t sns_rom_superfx_device::read_l(offs_t offset)
{
	return read_h(offset);
}

uint8_t sns_rom_superfx_device::read_h(offs_t offset)
{
	if (offset < 0x400000)
		return m_rom[rom_bank_map[offset / 0x10000] * 0x8000 + (offset & 0x7fff)];
	else if (offset < 0x600000)
	{
		if (m_superfx->access_rom())
		{
			return m_rom[rom_bank_map[(offset - 0x400000) / 0x8000] * 0x8000 + (offset & 0x7fff)];
		}
		else
		{
			static const uint8_t sfx_data[16] = {
				0x00, 0x01, 0x00, 0x01, 0x04, 0x01, 0x00, 0x01,
				0x00, 0x01, 0x08, 0x01, 0x00, 0x01, 0x0c, 0x01,
			};
			return sfx_data[offset & 0x0f];
		}
	}
	return 0xff;    // this handler should never be called for [60-7f]/[e0-ff] ranges
}

uint8_t sns_rom_superfx_device::read_ram(offs_t offset)
{
	if (m_superfx->access_ram())
		return sfx_ram[offset & 0xfffff];
	return 0xff;    // should be open bus...
}

void sns_rom_superfx_device::write_ram(offs_t offset, uint8_t data)
{
	if (m_superfx->access_ram())
		sfx_ram[offset & 0xfffff] = data;
}
