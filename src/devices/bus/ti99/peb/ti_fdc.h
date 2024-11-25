// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Standard Floppy Disk Controller Card
    See ti_fdc.c for documentation

    Michael Zapf

    September 2010
    January 2012: rewritten as class (MZ)

****************************************************************************/
#ifndef MAME_BUS_TI99_PEB_TI_FDC_H
#define MAME_BUS_TI99_PEB_TI_FDC_H

#pragma once

#include "peribox.h"
#include "machine/74259.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "machine/74123.h"

namespace bus::ti99::peb {

class ti_fdc_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	ti_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void setaddress_dbin(offs_t offset, int state) override;

	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;

	// bool dvena_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static void floppy_formats(format_registration &fr);

	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	void fdc_hld_w(int state);

	void dskpgena_w(int state);
	void kaclk_w(int state);
	void waiten_w(int state);
	void hlt_w(int state);
	void sidsel_w(int state);

	void dvena_w(int state);
	void dsel1_w(int state);
	void dsel2_w(int state);
	void dsel3_w(int state);

	void select_drive(int n, int state);

	// For debugger access
	void debug_read(offs_t offset, uint8_t* value);

	// Wait state logic
	void operate_ready_line();

	// Operate the floppy motors
	void set_floppy_motors_running(bool run);

	// Recent address
	int     m_address;

	// Holds the status of the DRQ, IRQ, and HLD lines.
	int  m_DRQ, m_IRQ, m_HLD;

	// Signal DVENA. When true, makes some drive turning.
	int  m_DVENA;

	// Set when address is in card area
	bool    m_inDsrArea;

	// When true the CPU is halted while DRQ/IRQ are true.
	bool    m_WAITena;

	// WD chip selected
	bool    m_WDsel;

	// Link to the FDC1771 controller on the board.
	required_device<fd1771_device>   m_fd1771;

	// Latched CRU outputs
	required_device<ls259_device> m_crulatch;

	// Motor monoflop
	required_device<ttl74123_device> m_motormf;

	// DSR ROM
	uint8_t*  m_dsrrom;

	// Link to the attached floppy drives
	required_device_array<floppy_connector, 3> m_floppy;

	// Currently selected floppy drive
	int  m_sel_floppy;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_FDC, bus::ti99::peb, ti_fdc_device)

#endif // MAME_BUS_TI99_PEB_TI_FDC_H
