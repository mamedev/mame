// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Myarc Disk Controller
    Double Density, Double-sided

    Michael Zapf, March 2020

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_DDCC_H
#define MAME_BUS_TI99_PEB_DDCC_H

#pragma once

#include "peribox.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "machine/74259.h"
#include "machine/ram.h"

namespace bus::ti99::peb {

class ddcc1_pal_device;

class myarc_fdc_device : public device_t, public device_ti99_peribox_card_interface
{
	friend class ddcc1_pal_device;

public:
	myarc_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

	// Callback methods
	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	void fdc_mon_w(int state);

	void den_w(int state);
	void wdreset_w(int state);
	void sidsel_w(int state);
	void bankdden_w(int state);
	void drivesel_w(int state);

	// Deliver the current state of the address bus
	offs_t get_address();

	// Polled from the PAL
	bool card_selected();

	// Link to the WD controller on the board.
	wd_fdc_digital_device_base*   m_wdc;

	// Two options for the controller chip. Can be selected by the configuration switch.
	required_device<wd1770_device> m_wd1770;
	required_device<wd1772_device> m_wd1772;

	// Latch
	required_device<ls259_device> m_drivelatch;

	// Buffer RAM
	required_device<ram_device> m_buffer_ram;

	// Decoder PAL
	required_device<ddcc1_pal_device> m_pal;

	// DSR ROM
	uint8_t* m_dsrrom;

	// Link to the attached floppy drives
	required_device_array<floppy_connector, 4> m_floppy;

	// Debugger accessors
	void debug_read(offs_t offset, uint8_t* value);
	void debug_write(offs_t offset, uint8_t data);

	// Upper bank selected
	bool m_banksel;

	// Card enabled
	bool m_cardsel;

	// Selected drive
	int m_selected_drive;

	// Recent address
	int m_address;

	// AMA/B/C decoding active
	bool m_dec_high;
};

// =========== Decoder PAL circuit ================
class ddcc1_pal_device : public device_t
{
	friend class myarc_fdc_device;

public:
	ddcc1_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Selector output lines of the PAL
	bool ramsel();
	bool romen();
	bool fdcsel();
	bool cs251();
	bool cs259();

private:
	void device_start() override { }
	myarc_fdc_device* m_board;
	void set_board(myarc_fdc_device* board) { m_board = board; }
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_DDCC1, bus::ti99::peb, myarc_fdc_device)
DECLARE_DEVICE_TYPE_NS(DDCC1_PAL, bus::ti99::peb, ddcc1_pal_device)

#endif // MAME_BUS_TI99_PEB_DDCC_H
