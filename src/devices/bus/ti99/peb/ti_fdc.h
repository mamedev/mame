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

namespace bus { namespace ti99 { namespace peb {

class ti_fdc_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	ti_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin) override;

	DECLARE_READ8Z_MEMBER(crureadz) override;
	void cruwrite(offs_t offset, uint8_t data) override;

	// bool dvena_r();

protected:
	void device_start() override;
	void device_reset() override;
	void device_config_complete() override;

	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_hld_w);

	DECLARE_WRITE_LINE_MEMBER(dskpgena_w);
	DECLARE_WRITE_LINE_MEMBER(kaclk_w);
	DECLARE_WRITE_LINE_MEMBER(waiten_w);
	DECLARE_WRITE_LINE_MEMBER(hlt_w);
	DECLARE_WRITE_LINE_MEMBER(sidsel_w);

	DECLARE_WRITE_LINE_MEMBER(dvena_w);
	DECLARE_WRITE_LINE_MEMBER(dsel1_w);
	DECLARE_WRITE_LINE_MEMBER(dsel2_w);
	DECLARE_WRITE_LINE_MEMBER(dsel3_w);

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
	floppy_image_device*    m_floppy[3];

	// Currently selected floppy drive
	int  m_sel_floppy;
};

} } } // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_FDC, bus::ti99::peb, ti_fdc_device)

#endif // MAME_BUS_TI99_PEB_TI_FDC_H
