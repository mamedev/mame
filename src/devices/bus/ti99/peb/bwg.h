// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    SNUG BwG Disk Controller
    Based on WD1770
    Double Density, Double-sided

    Michael Zapf, September 2010
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PED_BWG_H
#define MAME_BUS_TI99_PED_BWG_H

#pragma once

#include "peribox.h"
#include "imagedev/floppy.h"
#include "machine/mm58274c.h"
#include "machine/wd_fdc.h"
#include "machine/ram.h"
#include "machine/74259.h"
#include "machine/74123.h"

namespace bus::ti99::peb {

class snug_bwg_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	snug_bwg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void setaddress_dbin(offs_t offset, int state) override;

	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	static void floppy_formats(format_registration &fr);

	void fdc_irq_w(int state);
	void fdc_drq_w(int state);

	// Latch callbacks
	void den_w(int state);
	void mop_w(int state);
	void waiten_w(int state);
	void hlt_w(int state);
	void dsel1_w(int state);
	void dsel2_w(int state);
	void dsel3_w(int state);
	void dsel4_w(int state);
	void sidsel_w(int state);
	void dden_w(int state);

	void motorona_w(int state);

	void select_drive(int n, int state);

	// Debugger accessors
	void debug_read(offs_t offset, uint8_t* value);
	void debug_write(offs_t offset, uint8_t data);

	// Wait state logic
	void operate_ready_line();

	// Set the current floppy
	void set_drive();

	// Holds the status of the DRQ and IRQ lines.
	int m_DRQ, m_IRQ;

	// DIP switch state
	int m_dip1, m_dip2, m_dip34;

	// Address in card area
	bool m_inDsrArea;

	// WD selected
	bool m_WDsel, m_WDsel0;

	// RTC selected
	bool m_RTCsel;

	// last 1K area selected
	bool m_lastK;

	// Data register +1 selected
	bool m_dataregLB;

	// Signal motor_on. When true, makes all drives turning.
	int m_MOTOR_ON;

	// Recent address
	int m_address;

	// DSR ROM
	uint8_t*          m_dsrrom;

	// Buffer RAM
	required_device<ram_device> m_buffer_ram;

	// Link to the attached floppy drives
	required_device_array<floppy_connector, 4> m_floppy;

	// Currently selected floppy drive (1-4, 0=none)
	int m_sel_floppy;

	// Link to the WD1773 controller on the board.
	required_device<wd1773_device>   m_wd1773;

	// Link to the real-time clock on the board.
	required_device<mm58274c_device> m_clock;

	// Latched CRU outputs
	required_device<hc259_device> m_crulatch0_7;
	required_device<hc259_device> m_crulatch8_15;

	// Motor monoflop
	required_device<ttl74123_device> m_motormf;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_BWG, bus::ti99::peb, snug_bwg_device)

#endif // MAME_BUS_TI99_PED_BWG_H
