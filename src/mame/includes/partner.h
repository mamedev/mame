// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/partner.h
 *
 ****************************************************************************/

#ifndef partner_H_
#define partner_H_

#include "machine/i8255.h"
#include "machine/wd_fdc.h"
#include "machine/ram.h"

class partner_state : public radio86_state
{
public:
	partner_state(const machine_config &mconfig, device_type type, const char *tag)
		: radio86_state(mconfig, type, tag),
		m_ram(*this, RAM_TAG),
		m_fdc(*this, "wd1793") { }

	uint8_t m_mem_page;
	uint8_t m_win_mem_page;
	uint8_t partner_floppy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void partner_floppy_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void partner_win_memory_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void partner_mem_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_partner();
	void machine_start_partner();
	void machine_reset_partner();
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);

	void partner_window_1(uint8_t bank_num, uint16_t offset,uint8_t *rom);
	void partner_window_2(uint8_t bank_num, uint16_t offset,uint8_t *rom);
	void partner_iomap_bank(uint8_t *rom);
	void partner_bank_switch();
	required_device<ram_device> m_ram;
	required_device<fd1793_t> m_fdc;
	DECLARE_FLOPPY_FORMATS( floppy_formats );
};


#endif /* partner_H_ */
