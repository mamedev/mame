// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    PDC Clipper 3" floppy disk drive emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_CLIPPER_FDD_H
#define MAME_BUS_CBMIEC_CLIPPER_FDD_H

#pragma once

#include "cbmiec.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/floppy.h"
#include "machine/6522via.h"
#include "machine/upd765.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> clipper_fdd_device

class clipper_fdd_device : public device_t, public device_cbm_iec_interface
{
public:
	// construction/destruction
	clipper_fdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void floppy_formats(format_registration &fr);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_cbm_iec_interface overrides
	virtual void cbm_iec_atn(int state) override;
	virtual void cbm_iec_reset(int state) override;

private:
	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	output_finder<> m_led;

	void mem_map(address_map &map) ATTR_COLD;

	uint8_t via_pb_r();
	void via_pb_w(uint8_t data);
	uint8_t latch_r();
	void latch_w(uint8_t data);
	uint8_t tc_r();
	void tc_w(uint8_t data);

	int atn_ack() { return !m_bus->atn_r() ^ m_atna; }

	TIMER_CALLBACK_MEMBER(iec_sync_tick);
	TIMER_CALLBACK_MEMBER(mtr_on_tick);

	emu_timer *m_iec_sync_timer;
	emu_timer *m_mtr_on_timer;

	int m_iec_clk;
	int m_iec_data;
	int m_atna;
	uint8_t m_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(CLIPPER_FDD, clipper_fdd_device)


#endif // MAME_BUS_CBMIEC_CLIPPER_FDD_H
