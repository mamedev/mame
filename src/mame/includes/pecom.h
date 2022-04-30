// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_INCLUDES_PECOM_H
#define MAME_INCLUDES_PECOM_H

#pragma once

#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "sound/cdp1869.h"

class pecom_state : public driver_device
{
public:
	pecom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cdp1869(*this, "cdp1869")
		, m_cassette(*this, "cassette")
		, m_bank1(*this, "bank1")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_io_cnt(*this, "CNT")
		, m_io_keyboard(*this, "LINE%d", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(ef_w);
	void pecom64(machine_config &config);

private:
	uint8_t cdp1869_charram_r(offs_t offset);
	void cdp1869_charram_w(offs_t offset, uint8_t data);
	uint8_t cdp1869_pageram_r(offs_t offset);
	void cdp1869_pageram_w(offs_t offset, uint8_t data);
	void bank_w(uint8_t data);
	uint8_t keyboard_r();
	void cdp1869_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(reset_tick);
	DECLARE_READ_LINE_MEMBER(clear_r);
	DECLARE_READ_LINE_MEMBER(ef2_r);
	DECLARE_WRITE_LINE_MEMBER(q_w);
	void sc_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(prd_w);
	CDP1869_CHAR_RAM_READ_MEMBER(char_ram_r);
	CDP1869_CHAR_RAM_WRITE_MEMBER(char_ram_w);
	CDP1869_PCB_READ_MEMBER(pcb_r);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void cdp1869_page_ram(address_map &map);
	void io_map(address_map &map);
	void mem_map(address_map &map);

	std::unique_ptr<uint8_t[]> m_charram;           /* character generator ROM */
	bool m_reset = false;                /* CPU mode */
	bool m_dma = false;              /* memory refresh DMA */

	/* timers */
	emu_timer *m_reset_timer = nullptr;   /* power on reset timer */

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1869_device> m_cdp1869;
	required_device<cassette_image_device> m_cassette;
	required_memory_bank m_bank1;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_ioport m_io_cnt;
	required_ioport_array<26> m_io_keyboard;
};

#endif // MAME_INCLUDES_PECOM_H
