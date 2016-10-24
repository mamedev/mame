// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef __PECOM__
#define __PECOM__

#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"

#define SCREEN_TAG  "screen"
#define CDP1802_TAG "cdp1802"
#define CDP1869_TAG "cdp1869"

#define PECOM_CHAR_RAM_SIZE 0x800

class pecom_state : public driver_device
{
public:
	pecom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cdp1802(*this, CDP1802_TAG),
		m_cdp1869(*this, CDP1869_TAG),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_io_cnt(*this, "CNT") { }

	required_device<cosmac_device> m_cdp1802;
	required_device<cdp1869_device> m_cdp1869;

	std::unique_ptr<uint8_t[]> m_charram;           /* character generator ROM */
	int m_reset;                /* CPU mode */
	int m_dma;              /* memory refresh DMA */

	/* timers */
	emu_timer *m_reset_timer;   /* power on reset timer */

	uint8_t pecom_cdp1869_charram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pecom_cdp1869_charram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pecom_cdp1869_pageram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pecom_cdp1869_pageram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pecom_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pecom_keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pecom_cdp1869_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void video_start_pecom();
	void ef_w(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void reset_tick(void *ptr, int32_t param);
	int clear_r();
	int ef2_r();
	void q_w(int state);
	void sc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pecom_prd_w(int state);
	CDP1869_CHAR_RAM_READ_MEMBER(pecom_char_ram_r);
	CDP1869_CHAR_RAM_WRITE_MEMBER(pecom_char_ram_w);
	CDP1869_PCB_READ_MEMBER(pecom_pcb_r);
protected:
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_ioport m_io_cnt;
	ioport_port *m_io_ports[26];
};

/* ---------- defined in video/pecom.c ---------- */

MACHINE_CONFIG_EXTERN( pecom_video );

#endif
