// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Myarc Hard and Floppy Disk Controller
    See hfdc.c for documentation

    January 2012: rewritten as class
    June 2014: rewritten for modern floppy implementation

    Michael Zapf
    July 2015

****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_HFDC_H
#define MAME_BUS_TI99_PEB_HFDC_H

#pragma once

#include "peribox.h"

#include "imagedev/floppy.h"
#include "imagedev/mfmhd.h"

#include "machine/mm58274c.h"
#include "machine/hdc92x4.h"
#include "machine/ram.h"

namespace bus::ti99::peb {

/*
    Implementation for modern floppy system.
*/
class myarc_hfdc_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	myarc_hfdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void readz(offs_t offset, uint8_t *value) override;
	virtual void write(offs_t offset, uint8_t data) override;
	virtual void setaddress_dbin(offs_t offset, int state) override;
	virtual void crureadz(offs_t offset, uint8_t *value) override;
	virtual void cruwrite(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(motor_off);

private:
	void dmarq_w(int state);
	void intrq_w(int state);
	void dip_w(int state);
	void auxbus_out(offs_t offset, uint8_t data);
	uint8_t read_buffer();
	void write_buffer(uint8_t data);

	static void floppy_formats(format_registration &fr);

	// Debug accessors
	void debug_read(offs_t offset, uint8_t* value);
	void debug_write(offs_t offset, uint8_t data);

	// Callbacks for the index hole and seek complete
	void floppy_index_callback(floppy_image_device *floppy, int state);
	void harddisk_index_callback(mfm_harddisk_device *harddisk, int state);
	void harddisk_ready_callback(mfm_harddisk_device *harddisk, int state);
	void harddisk_skcom_callback(mfm_harddisk_device *harddisk, int state);

	// Operate the floppy motors
	void set_floppy_motors_running(bool run);

	// Connect floppy drives
	void connect_floppy_unit(int index);

	// Connect harddisk drives
	void connect_harddisk_unit(int index);

	// Disconnect drives
	void disconnect_floppy_drives();
	void disconnect_hard_drives();

	// Pushes the drive status to the HDC
	void signal_drive_status();

	// Motor monoflop (4.23 sec)
	emu_timer*      m_motor_on_timer;

	// HDC9234 controller on the board
	required_device<hdc9234_device> m_hdc9234;

	// Clock chip on the board
	required_device<mm58274c_device> m_clock;

	// Link to the attached floppy drives
	required_device_array<floppy_connector, 4> m_floppy;

	// Link to the attached hard disks
	required_device_array<mfm_harddisk_connector, 3> m_harddisk;

	// Currently selected floppy drive
	floppy_image_device*    m_current_floppy;

	// Currently selected hard drive
	mfm_harddisk_device*    m_current_harddisk;

	// Currently selected floppy disk index
	int     m_current_floppy_index;

	// Currently selected hard disk index
	int     m_current_hd_index;

	// True: Access to DIP switch settings, false: access to line states
	bool    m_see_switches;

	// IRQ state
	int    m_irq;

	// DMA in Progress state
	int    m_dip;

	// When true, motor monoflop is high
	bool    m_motor_running;

	// Address in card area
	bool m_inDsrArea;

	// HDC selected
	bool m_HDCsel;

	// RTC selected
	bool m_RTCsel;

	// Tape selected
	bool m_tapesel;

	// RAM selected
	bool m_RAMsel;

	// RAM selected
	bool m_ROMsel;

	// Recent address
	int m_address;

	// Wait for HD. This was an addition in later cards.
	bool m_wait_for_hd1;

	// Device Service Routine ROM (firmware)
	uint8_t*  m_dsrrom;

	// ROM banks.
	int     m_rom_page;

	// HFDC on-board SRAM (8K or 32K)
	required_device<ram_device> m_buffer_ram;

	// RAM page registers
	int     m_ram_page[4];

	// Drive status latch (STB0)
	uint8_t   m_status_latch;

	// DMA address latch (in Gate Array) (STB1)
	uint32_t  m_dma_address;

	// Output 1 latch (STB2)
	uint8_t   m_output1_latch;

	// Output 2 latch (STB3)
	uint8_t   m_output2_latch;

	// Needed for triggering the motor monoflop
	uint8_t m_lastval;

	// Signal motor_on. When true, makes all drives turning.
	int m_MOTOR_ON;

	// Calculates the index from the bit
	int bit_to_index(int value);

	// Utility function to set or unset bits in a byte
	void set_bits(uint8_t& byte, int mask, bool set);

	// Joined ready line towards the controller
	int  m_readyflags;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_HFDC, bus::ti99::peb, myarc_hfdc_device)

#endif // MAME_BUS_TI99_PEB_HFDC_H
