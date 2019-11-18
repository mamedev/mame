// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_INCLUDES_PECOM_H
#define MAME_INCLUDES_PECOM_H

#pragma once

#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "sound/cdp1869.h"

#define SCREEN_TAG  "screen"
#define CDP1802_TAG "cdp1802"
#define CDP1869_TAG "cdp1869"

#define PECOM_CHAR_RAM_SIZE 0x800

class pecom_state : public driver_device
{
public:
	pecom_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cdp1802(*this, CDP1802_TAG),
		m_cdp1869(*this, CDP1869_TAG),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_io_cnt(*this, "CNT"),
		m_io_ports(*this, "LINE%u", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(ef_w);
	void pecom64(machine_config &config);

protected:
	DECLARE_READ8_MEMBER(pecom_cdp1869_charram_r);
	DECLARE_WRITE8_MEMBER(pecom_cdp1869_charram_w);
	DECLARE_READ8_MEMBER(pecom_cdp1869_pageram_r);
	DECLARE_WRITE8_MEMBER(pecom_cdp1869_pageram_w);
	DECLARE_WRITE8_MEMBER(pecom_bank_w);
	DECLARE_READ8_MEMBER(pecom_keyboard_r);
	DECLARE_WRITE8_MEMBER(pecom_cdp1869_w);
	TIMER_CALLBACK_MEMBER(reset_tick);
	DECLARE_READ_LINE_MEMBER(clear_r);
	DECLARE_READ_LINE_MEMBER(ef2_r);
	DECLARE_WRITE_LINE_MEMBER(q_w);
	DECLARE_WRITE8_MEMBER( sc_w );
	DECLARE_WRITE_LINE_MEMBER(pecom_prd_w);
	CDP1869_CHAR_RAM_READ_MEMBER(pecom_char_ram_r);
	CDP1869_CHAR_RAM_WRITE_MEMBER(pecom_char_ram_w);
	CDP1869_PCB_READ_MEMBER(pecom_pcb_r);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void pecom_video(machine_config &config);
	void cdp1869_page_ram(address_map &map);
	void pecom64_io(address_map &map);
	void pecom64_mem(address_map &map);

private:
	required_device<cosmac_device> m_cdp1802;
	required_device<cdp1869_device> m_cdp1869;

	std::unique_ptr<uint8_t[]> m_charram;           /* character generator ROM */
	int m_reset;                /* CPU mode */
	int m_dma;              /* memory refresh DMA */

	/* timers */
	emu_timer *m_reset_timer;   /* power on reset timer */

	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_ioport m_io_cnt;
	required_ioport_array<26> m_io_ports;
};

#endif // MAME_INCLUDES_PECOM_H
