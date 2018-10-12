// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Conitec Datensysteme GRIP graphics card emulation

**********************************************************************/

#ifndef MAME_BUS_ECBBUS_GRIP_H
#define MAME_BUS_ECBBUS_GRIP_H

#pragma once

#include "ecbbus.h"

#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/i8255.h"
#include "machine/keyboard.h"
#include "machine/z80sti.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"
#include "emupal.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ecb_grip21_device

class ecb_grip21_device : public device_t, public device_ecbbus_card_interface
{
public:
	// construction/destruction
	ecb_grip21_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// device_ecbbus_card_interface overrides
	virtual uint8_t ecbbus_io_r(offs_t offset) override;
	virtual void ecbbus_io_w(offs_t offset, uint8_t data) override;

private:
	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_WRITE8_MEMBER( ppi_pa_w );
	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );
	DECLARE_READ8_MEMBER( sti_gpio_r );
	DECLARE_WRITE_LINE_MEMBER( speaker_w );

	void kb_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( write_centronics_busy );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_fault );

	MC6845_UPDATE_ROW( crtc_update_row );

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

	DECLARE_WRITE8_MEMBER( vol0_w );
	DECLARE_WRITE8_MEMBER( vol1_w );
	DECLARE_WRITE8_MEMBER( flash_w );
	DECLARE_WRITE8_MEMBER( page_w );
	DECLARE_READ8_MEMBER( stat_r );
	DECLARE_READ8_MEMBER( lrs_r );
	DECLARE_WRITE8_MEMBER( lrs_w );
	DECLARE_READ8_MEMBER( cxstb_r );
	DECLARE_WRITE8_MEMBER( cxstb_w );

	void grip_io(address_map &map);
	void grip_mem(address_map &map);

	/*
	required_device<hd6345_device> m_crtc;
	DECLARE_WRITE8_MEMBER( eprom_w );
	DECLARE_WRITE8_MEMBER( dpage_w );

	// video state
	int m_dpage;            // displayed video page
	*/

};


// device type definition
DECLARE_DEVICE_TYPE(ECB_GRIP21, ecb_grip21_device)

#endif // MAME_BUS_ECBBUS_GRIP_H
