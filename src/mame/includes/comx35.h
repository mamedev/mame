// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __COMX35__
#define __COMX35__

#include "emu.h"
#include "bus/comx35/exp.h"
#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "imagedev/printer.h"
#include "imagedev/snapquik.h"
#include "machine/cdp1871.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "sound/cdp1869.h"
#include "sound/wave.h"

#define SCREEN_TAG          "screen"

#define CDP1870_TAG         "u1"
#define CDP1869_TAG         "u2"
#define CDP1802_TAG         "u3"
#define CDP1871_TAG         "u4"
#define EXPANSION_TAG       "exp"

#define COMX35_CHARRAM_SIZE 0x800
#define COMX35_CHARRAM_MASK 0x7ff

class comx35_state : public driver_device
{
public:
	comx35_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, CDP1802_TAG),
			m_vis(*this, CDP1869_TAG),
			m_kbe(*this, CDP1871_TAG),
			m_cassette(*this, "cassette"),
			m_ram(*this, RAM_TAG),
			m_exp(*this, EXPANSION_TAG),
			m_rom(*this, CDP1802_TAG),
			m_char_ram(*this, "char_ram"),
			m_d6(*this, "D6"),
			m_modifiers(*this, "MODIFIERS")
	{ }

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1869_device> m_vis;
	required_device<cdp1871_device> m_kbe;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<comx_expansion_slot_device> m_exp;
	required_memory_region m_rom;
	optional_shared_ptr<uint8_t> m_char_ram;
	required_ioport m_d6;
	required_ioport m_modifiers;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	enum
	{
		TIMER_ID_RESET
	};

	void check_interrupt();

	uint8_t mem_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mem_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cdp1869_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int clear_r();
	int ef2_r();
	int ef4_r();
	void q_w(int state);
	void sc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_w(int state);
	void prd_w(int state);
	void trigger_reset(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	DECLARE_QUICKLOAD_LOAD_MEMBER( comx35_comx );
	void image_fread_memory(device_image_interface &image, uint16_t addr, uint32_t count);
	CDP1869_CHAR_RAM_READ_MEMBER(comx35_charram_r);
	CDP1869_CHAR_RAM_WRITE_MEMBER(comx35_charram_w);
	CDP1869_PCB_READ_MEMBER(comx35_pcb_r);

	// processor state
	int m_clear;                // CPU mode
	int m_q;                    // Q flag
	int m_iden;                 // interrupt/DMA enable
	int m_dma;                  // memory refresh DMA
	int m_int;                  // interrupt request
	int m_prd;                  // predisplay
	int m_cr1;                  // interrupt enable
};

// ---------- defined in video/comx35.c ----------

MACHINE_CONFIG_EXTERN( comx35_pal_video );
MACHINE_CONFIG_EXTERN( comx35_ntsc_video );

#endif
