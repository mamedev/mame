// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    ldv1000.h

    Pioneer LD-V1000 laserdisc emulation.

*************************************************************************/

#ifndef MAME_MACHINE_LDV1000_H
#define MAME_MACHINE_LDV1000_H

#pragma once

#include "laserdsc.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/z80ctc.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(PIONEER_LDV1000, pioneer_ldv1000_device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pioneer_ldv1000_device

// base ldv1000 class
class pioneer_ldv1000_device : public parallel_laserdisc_device
{
public:
	// construction/destruction
	pioneer_ldv1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto command_strobe_callback() { return m_command_strobe_cb.bind(); }

	// input and output
	virtual void data_w(uint8_t data) override;
	virtual void enter_w(int state) override { }
	virtual uint8_t data_r() override { return m_status; }
	virtual int status_strobe_r() override { return BIT(m_portc1, 5); }
	virtual int ready_r() override { return BIT(m_portc1, 4); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual int32_t player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

	TIMER_CALLBACK_MEMBER(vsync_off);
	TIMER_CALLBACK_MEMBER(multijump_tick);
	TIMER_CALLBACK_MEMBER(process_vbi_data);

private:
	// internal helpers
	bool focus_on() const { return !(m_portb1 & 0x01); }
	bool spdl_on() const { return !(m_portb1 & 0x02); }
	bool laser_on() const { return (m_portb1 & 0x40); }

	// internal read/write handlers
	void z80_decoder_display_port_w(offs_t offset, uint8_t data);
	uint8_t z80_decoder_display_port_r(offs_t offset);
	uint8_t z80_controller_r();
	void z80_controller_w(uint8_t data);

	// internal read/write handlers
	void ctc_interrupt(int state);
	void ppi0_porta_w(uint8_t data);
	uint8_t ppi0_portb_r();
	uint8_t ppi0_portc_r();
	void ppi0_portc_w(uint8_t data);
	uint8_t ppi1_porta_r();
	void ppi1_portb_w(uint8_t data);
	void ppi1_portc_w(uint8_t data);

	void ldv1000_map(address_map &map) ATTR_COLD;
	void ldv1000_portmap(address_map &map) ATTR_COLD;

	// internal state
	required_device<z80_device> m_z80_cpu;            /* CPU index of the Z80 */
	required_device<z80ctc_device> m_z80_ctc;         /* CTC device */
	emu_timer *         m_multitimer;                 /* multi-jump timer device */
	emu_timer *         m_vsync_off_timer;            /* vsync-shutoff timer device */
	emu_timer *         m_process_vbi_timer;          /* VBI processing timer device */
	devcb_write_line    m_command_strobe_cb;

	/* communication status */
	uint8_t               m_command;                  /* command byte to the player */
	uint8_t               m_status;                   /* status byte from the player */
	bool                  m_vsync;                    /* VSYNC state */

	/* I/O port states */
	uint8_t               m_counter_start;            /* starting value for counter */
	uint8_t               m_counter;                  /* current counter value */
	uint8_t               m_portc0;                   /* port C on PPI 0 */
	uint8_t               m_portb1;                   /* port B on PPI 1 */
	uint8_t               m_portc1;                   /* port C on PPI 1 */

	/* display/decode circuit emulation */
	uint8_t               m_portselect;               /* selection of which port to access */
	uint8_t               m_display[2][20];           /* display lines */
	uint8_t               m_dispindex;                /* index within the display line */
	uint8_t               m_vbi[7*3];                 /* VBI data */
	bool                  m_vbiready;                 /* VBI ready flag */
	uint8_t               m_vbiindex;                 /* index within the VBI data */

};

#endif // MAME_MACHINE_LDV1000_H
