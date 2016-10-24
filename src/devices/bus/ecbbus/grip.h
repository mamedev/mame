// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Conitec Datensysteme GRIP graphics card emulation

**********************************************************************/

#pragma once

#ifndef __GRIP__
#define __GRIP__

#include "emu.h"
#include "ecbbus.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8255.h"
#include "machine/keyboard.h"
#include "machine/z80sti.h"
#include "sound/speaker.h"
#include "video/mc6845.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> grip_device

class grip_device : public device_t,
					public device_ecbbus_card_interface
{
public:
	// construction/destruction
	grip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	// not really public
	void vol0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vol1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flash_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t stat_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t lrs_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lrs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cxstb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cxstb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ppi_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ppi_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ppi_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ppi_pc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sti_gpio_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void speaker_w(int state);
	void kb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void write_centronics_busy(int state);
	void write_centronics_fault(int state);

	MC6845_UPDATE_ROW( crtc_update_row );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_ecbbus_card_interface overrides
	virtual uint8_t ecbbus_io_r(offs_t offset) override;
	virtual void ecbbus_io_w(offs_t offset, uint8_t data) override;

private:
	required_device<i8255_device> m_ppi;
	required_device<z80sti_device> m_sti;
	required_device<mc6845_device> m_crtc;
	required_device<centronics_device> m_centronics;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_speaker;
	optional_shared_ptr<uint8_t> m_video_ram;
	required_ioport m_j3a;
	required_ioport m_j3b;
	required_ioport m_j7;

	int m_centronics_busy;
	int m_centronics_fault;

	// sound state
	int m_vol0;
	int m_vol1;

	// keyboard state
	int m_ia;               // PPI port A interrupt
	int m_ib;               // PPI port B interrupt
	uint8_t m_keydata;        // keyboard data
	int m_kbf;              // keyboard buffer full

	// video state
	int m_lps;              // light pen sense
	int m_page;             // video page
	int m_flash;            // flash

	// ECB bus state
	uint8_t m_base;           // ECB base address
	uint8_t m_ppi_pa;         // PPI port A data
	uint8_t m_ppi_pc;         // PPI port C data

	// timers
	emu_timer *m_kb_timer;
};


	/*
	required_device<hd6345_device> m_crtc;
	void eprom_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dpage_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// video state
	int m_dpage;            // displayed video page
	*/



// device type definition
extern const device_type ECB_GRIP21;

#endif
