// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Corcomp Disk Controller
    Based on WD2793/WD1773
    Double Density, Double-sided

    Michael Zapf, March 2020

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_CORCOMP_H
#define MAME_BUS_TI99_PEB_CORCOMP_H

#pragma once

#include "peribox.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "machine/74123.h"
#include "machine/tms9901.h"
#include "machine/ram.h"

namespace bus::ti99::peb {

class ccfdc_dec_pal_device;
class ccfdc_sel_pal_device;
class ccfdc_palu12_device;
class ccfdc_palu6_device;

class corcomp_fdc_device : public device_t, public device_ti99_peribox_card_interface
{
	friend class ccfdc_dec_pal_device;
	friend class ccfdc_sel_pal_device;
	friend class ccdcc_palu1_device;
	friend class ccfdc_palu6_device;
	friend class ccfdc_palu12_device;

public:
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void setaddress_dbin(offs_t offset, int state) override;

	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;
	DECLARE_WRITE_LINE_MEMBER(clock_in) override;

	DECLARE_WRITE_LINE_MEMBER( fdc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_hld_w );
	uint8_t tms9901_input(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER( select_dsk );
	DECLARE_WRITE_LINE_MEMBER( side_select );
	DECLARE_WRITE_LINE_MEMBER( motor_w );
	DECLARE_WRITE_LINE_MEMBER( select_card );
	DECLARE_WRITE_LINE_MEMBER( select_bank );

protected:
	corcomp_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void device_start() override;
	void device_reset() override;
	void connect_drives();

	virtual void device_add_mconfig(machine_config &config) override =0;
	ioport_constructor device_input_ports() const override;

	void common_config(machine_config& config);

	static void floppy_formats(format_registration &fr);

	// Link to the WD controller on the board.
	required_device<wd_fdc_device_base>   m_wdc;

	// PALs
	ccfdc_dec_pal_device* m_decpal;
	ccfdc_sel_pal_device* m_ctrlpal;

	// Lines that are polled by the PAL chips
	bool card_selected();
	bool upper_bank();
	bool write_access();
	bool ready_trap_active();

	// Deliver the current state of the address bus
	offs_t get_address();

	// Wait state logic
	void operate_ready_line();

	// Link to the attached floppy drives
	floppy_image_device*    m_floppy[4];

	// Motor monoflop
	required_device<ttl74123_device> m_motormf;

	// Interface chip
	required_device<tms9901_device> m_tms9901;

	// Debugger accessors
	void debug_read(offs_t offset, uint8_t* value);
	void debug_write(offs_t offset, uint8_t data);

	// Buffer RAM
	required_device<ram_device> m_buffer_ram;

	// DSR ROM
	uint8_t* m_dsrrom;

	// State of the bank select line. Pulled down initially.
	bool m_cardsel;
	bool m_banksel;

	// Selected drive
	int m_selected_drive;

	// Recent address
	int m_address;

	// Write access
	bool m_writing;
};


// ============================================================================
// Original CorComp disk controller card
// ============================================================================

class corcomp_dcc_device : public corcomp_fdc_device
{
public:
	corcomp_dcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
private:
	void device_add_mconfig(machine_config &config) override;
	void device_config_complete() override;
	const tiny_rom_entry *device_rom_region() const override;
};

// =========== Decoder PAL circuit ================
class ccfdc_dec_pal_device : public device_t
{
public:
	DECLARE_READ_LINE_MEMBER(addresswdc);
	DECLARE_READ_LINE_MEMBER(address4);
	DECLARE_READ_LINE_MEMBER(addressram);
	virtual DECLARE_READ_LINE_MEMBER(address9901);

protected:
	ccfdc_dec_pal_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void device_start() override { }
	void device_config_complete() override;

	corcomp_fdc_device* m_board;
	required_device<tms9901_device> m_tms9901;
};

// =========== Selector PAL circuit ================

class ccfdc_sel_pal_device : public device_t
{
public:
	DECLARE_READ_LINE_MEMBER(selectram);
	virtual DECLARE_READ_LINE_MEMBER(selectwdc);
	virtual DECLARE_READ_LINE_MEMBER(selectdsr);
	virtual DECLARE_READ_LINE_MEMBER(ready_out) =0;

protected:
	ccfdc_sel_pal_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void device_start() override { }
	virtual void device_config_complete() override =0;

	corcomp_fdc_device* m_board;
	ccfdc_dec_pal_device* m_decpal;
	required_device<ttl74123_device> m_motormf;
	required_device<tms9901_device> m_tms9901;
	required_device<wd_fdc_device_base> m_wdc;
};

// =========== Specific decoder PAL circuit of the CCDCC ================

class ccdcc_palu2_device : public ccfdc_dec_pal_device
{
public:
	ccdcc_palu2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// =========== Specific selector PAL circuit of the CCDCC ================

class ccdcc_palu1_device : public ccfdc_sel_pal_device
{
public:
	ccdcc_palu1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ_LINE_MEMBER(ready_out) override;

private:
	void device_config_complete() override;
};

// ============================================================================
// Revised CorComp floppy disk controller card rev a
// ============================================================================

class corcomp_fdca_device : public corcomp_fdc_device
{
	friend class ccfdc_palu6_device;

public:
	corcomp_fdca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
private:
	void device_add_mconfig(machine_config &config) override;
	void device_config_complete() override;
	const tiny_rom_entry *device_rom_region() const override;
	bool ready_trap_active();
};

// =========== Specific decoder PAL circuit of the CCFDC ================

class ccfdc_palu12_device : public ccfdc_dec_pal_device
{
public:
	ccfdc_palu12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ_LINE_MEMBER(address9901) override;
};

// =========== Specific selector PAL circuit of the CCFDC ================

class ccfdc_palu6_device : public ccfdc_sel_pal_device
{
public:
	ccfdc_palu6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ_LINE_MEMBER(selectwdc) override;
	DECLARE_READ_LINE_MEMBER(selectdsr) override;

	DECLARE_READ_LINE_MEMBER(ready_out) override;

private:
	void device_config_complete() override;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_CCDCC, bus::ti99::peb, corcomp_dcc_device)
DECLARE_DEVICE_TYPE_NS(TI99_CCFDC, bus::ti99::peb, corcomp_fdca_device)
DECLARE_DEVICE_TYPE_NS(CCDCC_PALU2, bus::ti99::peb, ccdcc_palu2_device)
DECLARE_DEVICE_TYPE_NS(CCDCC_PALU1, bus::ti99::peb, ccdcc_palu1_device)
DECLARE_DEVICE_TYPE_NS(CCFDC_PALU12, bus::ti99::peb, ccfdc_palu12_device)
DECLARE_DEVICE_TYPE_NS(CCFDC_PALU6, bus::ti99::peb, ccfdc_palu6_device)

#endif // MAME_BUS_TI99_PEB_CORCOMP_H
