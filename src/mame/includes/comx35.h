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
	comx35_state(const machine_config &mconfig, device_type type, std::string tag)
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
	optional_shared_ptr<UINT8> m_char_ram;
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

	DECLARE_READ8_MEMBER( mem_r );
	DECLARE_WRITE8_MEMBER( mem_w );
	DECLARE_READ8_MEMBER( io_r );
	DECLARE_WRITE8_MEMBER( io_w );
	DECLARE_WRITE8_MEMBER( cdp1869_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef4_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_WRITE8_MEMBER( sc_w );
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( prd_w );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );
	DECLARE_QUICKLOAD_LOAD_MEMBER( comx35_comx );
	void image_fread_memory(device_image_interface &image, UINT16 addr, UINT32 count);
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
