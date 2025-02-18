// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Nigel Barnes
/******************************************************************************

    Acorn Electron driver

******************************************************************************/

#include "emu.h"
#include "electron.h"


uint8_t electron_state::keyboard_r(offs_t offset)
{
	uint8_t data = 0;

	for (int i = 0; i < 14; i++)
	{
		if (!BIT(offset, i))
			data |= m_keybd[i]->read() & 0x0f;
	}

	return data;
}


uint8_t electron_state::fetch_r(offs_t offset)
{
	m_vdu_drivers = (offset & 0xe000) == 0xc000 ? true : false;

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}


uint8_t electron_state::ram_r(offs_t offset)
{
	uint8_t data = 0xff;

	data &= m_exp->expbus_r(offset);

	switch (m_mrb.read_safe(0))
	{
	case 0x00: /* Normal */
		data &= m_ula->read(offset);
		break;

	case 0x01: /* Turbo */
		if (m_mrb_mapped && offset < 0x3000) offset += 0x8000;
		data &= m_ram->read(offset);
		break;

	case 0x02: /* Shadow */
		if (m_mrb_mapped && (offset < 0x3000 || !m_vdu_drivers)) offset += 0x8000;
		data &= m_ram->read(offset);
		break;
	}

	return data;
}

void electron_state::ram_w(offs_t offset, uint8_t data)
{
	m_exp->expbus_w(offset, data);

	switch (m_mrb.read_safe(0))
	{
	case 0x00: /* Normal */
		m_ula->write(offset, data);
		break;

	case 0x01: /* Turbo */
		if (m_mrb_mapped && offset < 0x3000) offset += 0x8000;
		m_ram->write(offset, data);
		break;

	case 0x02: /* Shadow */
		if (m_mrb_mapped && (offset < 0x3000 || !m_vdu_drivers)) offset += 0x8000;
		m_ram->write(offset, data);
		break;
	}
}


uint8_t electron_state::rom_r(offs_t offset)
{
	/* 0,1   Second external socket on the expansion module (SK2) */
	/* 2,3   First external socket on the expansion module (SK1)  */
	/* 4     Disc                                                 */
	/* 5,6   USER applications                                    */
	/* 7     Modem interface ROM                                  */
	/* 8,9   Keyboard                                             */
	/* 10,11 BASIC                                                */
	/* 12    Expansion module operating system                    */
	/* 13    High priority slot in expansion module               */
	/* 14    ECONET                                               */
	/* 15    Reserved                                             */

	uint8_t data = 0xff;

	offset += 0x8000;

	data &= m_ula->read(offset);
	data &= m_exp->expbus_r(offset);

	return data;
}

void electron_state::rom_w(offs_t offset, uint8_t data)
{
	offset += 0x8000;

	m_ula->write(offset, data);
	m_exp->expbus_w(offset, data);
}

uint8_t electronsp_state::rom_r(offs_t offset)
{
	uint8_t data = electron_state::rom_r(offset);

	switch (offset & 0x4000)
	{
	case 0x0000:
		if ((m_rompage & 0x0e) == m_rompages->read())
		{
			data = m_romi[m_rompage & 0x01]->read_rom(offset);
		}
		else
		{
			switch (m_rompage)
			{
			case 10:
				/* SP64 ROM utilises the spare BASIC ROM page */
				if (BIT(m_sp64_bank, 7) && (offset & 0x2000))
				{
					data = m_sp64_ram[offset & 0x1fff];
				}
				else
				{
					data = m_region_sp64->base()[(!BIT(m_sp64_bank, 0) << 14) | offset];
				}
				break;
			}
		}
		break;
	}

	return data;
}

void electronsp_state::rom_w(offs_t offset, uint8_t data)
{
	electron_state::rom_w(offset, data);

	switch (offset & 0x4000)
	{
	case 0x0000:
		if ((m_rompage & 0x0e) == m_rompages->read())
		{
			/* TODO: sockets are writeable if RAM */
		}
		else
		{
			switch (m_rompage)
			{
			case 10:
				/* SP64 ROM utilises the spare BASIC ROM page */
				if (BIT(m_sp64_bank, 7) && (offset & 0x2000))
				{
					m_sp64_ram[offset & 0x1fff] = data;
				}
				break;
			}
		}
		break;
	}
}


uint8_t electron_state::io_r(offs_t offset)
{
	uint8_t data = 0xff;

	offset += 0xfc00;

	data &= m_ula->read(offset);
	data &= m_exp->expbus_r(offset);

	return data;
}

void electron_state::io_w(offs_t offset, uint8_t data)
{
	m_ula->write(0xfc00 + offset, data);

	offset += 0xfc00;

	/* Master RAM Board */
	if (offset == 0xfc7f)
	{
		m_mrb_mapped = !BIT(data, 7);
	}

	m_exp->expbus_w(offset, data);
}


uint8_t electronsp_state::io_r(offs_t offset)
{
	uint8_t data = electron_state::io_r(offset);

	offset += 0xfc00;

	if ((offset & 0xfff0) == 0xfcb0)
	{
		data = m_via->read(offset & 0x0f);
	}

	return data;
}

void electronsp_state::io_w(offs_t offset, uint8_t data)
{
	electron_state::io_w(offset, data);

	offset += 0xfc00;

	if ((offset & 0xfff0) == 0xfcb0)
	{
		m_via->write(offset & 0x0f, data);
	}
	else if (offset == 0xfcfa)
	{
		m_sp64_bank = data;
	}
	else if ((offset == 0xfe05) && !(data & 0xf0))
	{
		m_rompage = data & 0x0f;
	}
}


/**************************************
   Machine Initialisation functions
***************************************/

void electron_state::machine_start()
{
	m_capslock_led.resolve();

	/* set ULA RAM/ROM pointers */
	m_ula->set_ram(m_ram->pointer());
	m_ula->set_rom(m_region_mos->base());

	/* register save states */
	save_item(NAME(m_mrb_mapped));
	save_item(NAME(m_vdu_drivers));
}

void electron_state::machine_reset()
{
	m_mrb_mapped = true;
	m_vdu_drivers = false;
}

void electronsp_state::machine_start()
{
	electron_state::machine_start();

	m_sp64_ram = std::make_unique<uint8_t[]>(0x2000);

	/* register save states */
	save_item(NAME(m_sp64_bank));
	save_pointer(NAME(m_sp64_ram), 0x2000);
}


std::pair<std::error_condition, std::string> electronsp_state::load_rom(device_image_interface &image, generic_slot_device *slot)
{
	uint32_t size = slot->common_get_size("rom");

	// socket accepts 8K and 16K ROM only
	if (size != 0x2000 && size != 0x4000)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid size: Only 8K/16K is supported");

	slot->rom_alloc(0x4000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	// mirror 8K ROMs
	uint8_t *crt = slot->get_rom_base();
	if (size <= 0x2000) memcpy(crt + 0x2000, crt, 0x2000);

	return std::make_pair(std::error_condition(), std::string());
}
