// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/partner.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_PARTNER_H
#define MAME_INCLUDES_PARTNER_H

#pragma once

#include "includes/radio86.h"

#include "imagedev/floppy.h"
#include "machine/i8255.h"
#include "machine/wd_fdc.h"
#include "machine/ram.h"

class partner_state : public radio86_state
{
public:
	partner_state(const machine_config &mconfig, device_type type, const char *tag)
		: radio86_state(mconfig, type, tag)
		, m_ram(*this, RAM_TAG)
		, m_fdc(*this, "wd1793")
		, m_bank(*this, "bank%u", 1U)
	{
	}

	uint8_t partner_floppy_r(offs_t offset);
	void partner_floppy_w(offs_t offset, uint8_t data);
	void partner_win_memory_page_w(uint8_t data);
	void partner_mem_page_w(uint8_t data);
	void init_partner();

	I8275_DRAW_CHARACTER_MEMBER(display_pixels);

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	void partner(machine_config &config);
	void partner_mem(address_map &map);
protected:
	void partner_window_1(uint8_t bank_num, uint16_t offset,uint8_t *rom);
	void partner_window_2(uint8_t bank_num, uint16_t offset,uint8_t *rom);
	void partner_iomap_bank(uint8_t *rom);
	void partner_bank_switch();

	uint8_t m_mem_page;
	uint8_t m_win_mem_page;

	required_device<ram_device> m_ram;
	required_device<fd1793_device> m_fdc;
	required_memory_bank_array<13> m_bank;

	virtual void machine_reset() override;
};


#endif // MAME_INCLUDES_PARTNER_H
