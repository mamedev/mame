// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Mega-Cart cartridge emulation

**********************************************************************/

#include "emu.h"
#include "megacart.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC20_MEGACART, vic20_megacart_device, "vic20_megacart", "VIC-20 Mega-Cart")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_megacart_device - constructor
//-------------------------------------------------

vic20_megacart_device::vic20_megacart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC20_MEGACART, tag, owner, clock)
	, device_vic20_expansion_card_interface(mconfig, *this)
	, device_nvram_interface(mconfig, *this)
	, m_ram(*this, "ram", 0x8000, ENDIANNESS_LITTLE)
	, m_nvram(*this, "nvram", 0x2000, ENDIANNESS_LITTLE)
	, m_rom(nullptr)
	, m_reset_timer(nullptr)
	, m_nvram_en(1)
	, m_oe(0)
	, m_software_reset(0)
	, m_bank_lo(0)
	, m_bank_hi(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_megacart_device::device_start()
{
	m_reset_timer = timer_alloc(FUNC(vic20_megacart_device::reset_tick), this);

	m_slot->ram1()[0].install_ram(0x000, 0x3ff, &m_nvram[0x0400]);
	m_slot->ram2()[0].install_ram(0x000, 0x3ff, &m_nvram[0x0800]);
	m_slot->ram3()[0].install_ram(0x000, 0x3ff, &m_nvram[0x0c00]);
	m_slot->io2()[0].install_ram(0x000, 0x3ff, &m_nvram[0x1800]);
	m_slot->io3()[0].install_rom(0x000, 0x3ff, &m_nvram[0x1c00]);
	m_slot->io3()[0].install_write_handler(0x000, 0x3ff, write8sm_delegate(*this, FUNC(vic20_megacart_device::io3_w)));
	m_slot->io3()[1].install_write_handler(0x000, 0x3ff, write8sm_delegate(*this, FUNC(vic20_megacart_device::io3_w)));

	// state saving
	save_item(NAME(m_nvram_en));
	save_item(NAME(m_oe));
	save_item(NAME(m_software_reset));
	save_item(NAME(m_bank_lo));
	save_item(NAME(m_bank_hi));

	machine().save().register_postload(save_prepost_delegate(FUNC(vic20_megacart_device::update_map), this));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic20_megacart_device::device_reset()
{
	memory_region *const rom = m_slot->memregion("rom");
	m_rom = rom ? rom->base() : nullptr;

	m_oe = m_software_reset ? !m_oe : 0;
	m_software_reset = 0;

	update_map();
}


//-------------------------------------------------
//  reset_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(vic20_megacart_device::reset_tick)
{
	m_software_reset = 1;

	m_slot->res_w(0);
	m_slot->res_w(1);
}


//-------------------------------------------------
//  io3_w -
//-------------------------------------------------

void vic20_megacart_device::io3_w(offs_t offset, uint8_t data)
{
	if (m_nvram_en)
		m_nvram[0x1c00 | offset] = data;

	switch (offset & 0x180)
	{
	case 0x080:
		m_bank_hi = data;
		break;

	case 0x100:
		m_bank_lo = data;
		break;

	case 0x180:
		m_nvram_en = !BIT(data, 0);
		m_bank_hi = data;
		m_bank_lo = data;
		break;
	}

	if (BIT(offset, 9))
		m_reset_timer->adjust(attotime::zero);

	update_map();
}


//-------------------------------------------------
//  update_map -
//-------------------------------------------------

void vic20_megacart_device::update_map()
{
	if (m_nvram_en)
	{
		m_slot->ram1().select(0);
		m_slot->ram2().select(0);
		m_slot->ram3().select(0);
		m_slot->io2().select(0);
		m_slot->io3().select(0);
	}
	else
	{
		m_slot->ram1().unmap();
		m_slot->ram2().unmap();
		m_slot->ram3().unmap();
		m_slot->io2().unmap();
		m_slot->io3().select(1);
	}

	if (!m_rom)
		return;

	uint8_t const bank_lo = m_oe ? m_bank_lo : 0x7f;
	uint8_t const bank_hi = m_oe ? m_bank_hi : 0x7f;
	bool const ram_lo = BIT(bank_lo, 7);
	bool const ram_hi = BIT(bank_hi, 7);
	bool const ram_we = ram_lo && ram_hi && BIT(bank_hi, 6);
	uint8_t *const rom_lo = &m_rom[bank_lo << 13];
	uint8_t *const rom_hi = &m_rom[0x100000 | (bank_hi << 13)];

	vic20_expansion_window *const blk123[] = { &m_slot->blk1(), &m_slot->blk2(), &m_slot->blk3() };

	for (int i = 0; i < 3; i++)
	{
		uint8_t *const ram = &m_ram[0x2000 * (i + 1)];

		if (!ram_lo)
		{
			(*blk123[i])[0].install_rom(0x0000, 0x1fff, rom_lo);
			blk123[i]->select(0);
		}
		else if (!ram_hi)
		{
			blk123[i]->unmap();
		}
		else if (ram_we)
		{
			(*blk123[i])[1].install_ram(0x0000, 0x1fff, ram);
			blk123[i]->select(1);
		}
		else
		{
			(*blk123[i])[2].install_rom(0x0000, 0x1fff, ram);
			blk123[i]->select(2);
		}
	}

	if (!ram_hi)
	{
		m_slot->blk5()[0].install_rom(0x0000, 0x1fff, rom_hi);
		m_slot->blk5().select(0);
	}
	else if (!ram_lo)
	{
		m_slot->blk5()[0].install_rom(0x0000, 0x1fff, rom_lo);
		m_slot->blk5().select(0);
	}
	else if (ram_we)
	{
		m_slot->blk5()[1].install_ram(0x0000, 0x1fff, &m_ram[0x0000]);
		m_slot->blk5().select(1);
	}
	else
	{
		m_slot->blk5()[2].install_rom(0x0000, 0x1fff, &m_ram[0x0000]);
		m_slot->blk5().select(2);
	}
}


void vic20_megacart_device::nvram_default()
{
}


bool vic20_megacart_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_nvram, 0x2000);
	return !err && (actual == 0x2000);
}


bool vic20_megacart_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_nvram, 0x2000);
	return !err;
}
