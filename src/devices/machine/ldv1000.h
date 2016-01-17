// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    ldv1000.h

    Pioneer LD-V1000 laserdisc emulation.

*************************************************************************/

#pragma once

#ifndef __LDV1000_H__
#define __LDV1000_H__

#include "laserdsc.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/z80ctc.h"


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_LASERDISC_LDV1000_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PIONEER_LDV1000, 0)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type PIONEER_LDV1000;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pioneer_ldv1000_device

// base ldv1000 class
class pioneer_ldv1000_device : public laserdisc_device
{
public:
	// construction/destruction
	pioneer_ldv1000_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// input and output
	void data_w(UINT8 data);
	void enter_w(UINT8 data);
	UINT8 status_r() const { return m_status; }
	UINT8 status_strobe_r() const { return (m_portc1 & 0x20) ? ASSERT_LINE : CLEAR_LINE; }
	UINT8 command_strobe_r() const { return (m_portc1 & 0x10) ? ASSERT_LINE : CLEAR_LINE; }

protected:
	// timer IDs
	enum
	{
		TID_MULTIJUMP = TID_FIRST_PLAYER_TIMER,
		TID_VSYNC_OFF,
		TID_VBI_DATA_FETCH
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual INT32 player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

	// internal helpers
	bool focus_on() const { return !(m_portb1 & 0x01); }
	bool spdl_on() const { return !(m_portb1 & 0x02); }
	bool laser_on() const { return (m_portb1 & 0x40); }

public:
	// internal read/write handlers
	DECLARE_WRITE_LINE_MEMBER( ctc_interrupt );
	DECLARE_WRITE8_MEMBER( z80_decoder_display_port_w );
	DECLARE_READ8_MEMBER( z80_decoder_display_port_r );
	DECLARE_READ8_MEMBER( z80_controller_r );
	DECLARE_WRITE8_MEMBER( z80_controller_w );
	DECLARE_WRITE8_MEMBER( ppi0_porta_w );
	DECLARE_READ8_MEMBER( ppi0_portb_r );
	DECLARE_READ8_MEMBER( ppi0_portc_r );
	DECLARE_WRITE8_MEMBER( ppi0_portc_w );
	DECLARE_READ8_MEMBER( ppi1_porta_r );
	DECLARE_WRITE8_MEMBER( ppi1_portb_w );
	DECLARE_WRITE8_MEMBER( ppi1_portc_w );

protected:
	// internal state
	required_device<z80_device> m_z80_cpu;                  /* CPU index of the Z80 */
	required_device<z80ctc_device> m_z80_ctc;                   /* CTC device */
	emu_timer *         m_multitimer;           /* multi-jump timer device */

	/* communication status */
	UINT8               m_command;              /* command byte to the player */
	UINT8               m_status;                   /* status byte from the player */
	bool                m_vsync;                    /* VSYNC state */

	/* I/O port states */
	UINT8               m_counter_start;            /* starting value for counter */
	UINT8               m_counter;              /* current counter value */
	UINT8               m_portc0;                   /* port C on PPI 0 */
	UINT8               m_portb1;                   /* port B on PPI 1 */
	UINT8               m_portc1;                   /* port C on PPI 1 */

	/* display/decode circuit emulation */
	UINT8               m_portselect;               /* selection of which port to access */
	UINT8               m_display[2][20];           /* display lines */
	UINT8               m_dispindex;                /* index within the display line */
	UINT8               m_vbi[7*3];             /* VBI data */
	bool                m_vbiready;             /* VBI ready flag */
	UINT8               m_vbiindex;             /* index within the VBI data */
};


#endif
