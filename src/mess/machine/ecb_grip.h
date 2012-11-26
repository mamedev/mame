/**********************************************************************

    Conitec Datensysteme GRIP graphics card emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __GRIP__
#define __GRIP__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/ctronics.h"
#include "machine/ecbbus.h"
#include "machine/i8255.h"
#include "machine/keyboard.h"
#include "machine/z80sti.h"
#include "sound/speaker.h"
#include "video/mc6845.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> grip_device

class grip_device :	public device_t,
					public device_ecbbus_card_interface
{
public:
	// construction/destruction
	grip_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	// not really public
	DECLARE_WRITE8_MEMBER( vol0_w );
	DECLARE_WRITE8_MEMBER( vol1_w );
	DECLARE_WRITE8_MEMBER( flash_w );
	DECLARE_WRITE8_MEMBER( page_w );
	DECLARE_READ8_MEMBER( stat_r );
	DECLARE_READ8_MEMBER( lrs_r );
	DECLARE_WRITE8_MEMBER( lrs_w );
	DECLARE_READ8_MEMBER( cxstb_r );
	DECLARE_WRITE8_MEMBER( cxstb_w );
	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_WRITE8_MEMBER( ppi_pa_w );
	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );
	DECLARE_READ8_MEMBER( sti_gpio_r );
	DECLARE_WRITE_LINE_MEMBER( speaker_w );
	DECLARE_WRITE8_MEMBER( kb_w );

	void crtc_update_row(mc6845_device *device, bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16 ma, UINT8 ra, UINT16 y, UINT8 x_count, INT8 cursor_x, void *param);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
    virtual void device_config_complete() { m_shortname = "grip"; }

	// device_ecbbus_card_interface overrides
	virtual UINT8 ecbbus_io_r(offs_t offset);
	virtual void ecbbus_io_w(offs_t offset, UINT8 data);
	virtual UINT32 ecbbus_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	required_device<i8255_device> m_ppi;
	required_device<z80sti_device> m_sti;
	required_device<mc6845_device> m_crtc;
	required_device<centronics_device> m_centronics;
	required_device<speaker_sound_device> m_speaker;

	// sound state
	int m_vol0;
	int m_vol1;

	// keyboard state
	int m_ia;				// PPI port A interrupt
	int m_ib;				// PPI port B interrupt
	UINT8 m_keydata;		// keyboard data
	int m_kbf;				// keyboard buffer full

	// video state
	UINT8 *m_video_ram;		// video RAM
	int m_lps;				// light pen sense
	int m_page;				// video page
	int m_flash;			// flash

	// ECB bus state
	UINT8 m_base;			// ECB base address
	UINT8 m_ppi_pa;			// PPI port A data
	UINT8 m_ppi_pc;			// PPI port C data

	// timers
	emu_timer *m_kb_timer;
};


	/*
    required_device<hd6345_device> m_crtc;
    DECLARE_WRITE8_MEMBER( eprom_w );
    DECLARE_WRITE8_MEMBER( dpage_w );

    // video state
    int m_dpage;            // displayed video page
    */



// device type definition
extern const device_type ECB_GRIP21;

#endif
