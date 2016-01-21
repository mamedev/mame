// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    SNUG BwG Disk Controller
    Based on WD1770
    Double Density, Double-sided

    Michael Zapf, September 2010
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __BWG__
#define __BWG__

#include "imagedev/flopdrv.h"
#include "machine/mm58274c.h"
#include "machine/wd_fdc.h"

extern const device_type TI99_BWG;

class snug_bwg_device : public ti_expansion_card_device
{
public:
	snug_bwg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin) override;

	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_WRITE_LINE_MEMBER( fdc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );

protected:
	void device_start() override;
	void device_reset() override;
	void device_config_complete() override;

	const rom_entry *device_rom_region() const override;
	machine_config_constructor device_mconfig_additions() const override;
	ioport_constructor device_input_ports() const override;

private:
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// Debugger accessors
	void debug_read(offs_t offset, UINT8* value);
	void debug_write(offs_t offset, UINT8 data);

	// Wait state logic
	void operate_ready_line();

	// Set the current floppy
	void set_drive();

	// Holds the status of the DRQ and IRQ lines.
	line_state      m_DRQ, m_IRQ;

	// DIP switch state
	int m_dip1, m_dip2, m_dip34;

	// Page selection for ROM and RAM
	int m_ram_page;  // 0-1
	int m_rom_page;  // 0-3

	// Operate the floppy motors
	void set_floppy_motors_running(bool run);

	// When true, the READY line will be cleared (create wait states) when
	// waiting for data from the controller.
	bool m_WAITena;

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

	// Indicates whether the clock is mapped into the address space.
	bool m_rtc_enabled;

	// Signal motor_on. When TRUE, makes all drives turning.
	line_state m_MOTOR_ON;

	// Needed for triggering the motor monoflop
	UINT8 m_lastval;

	// Recent address
	int m_address;

	/* Indicates which drive has been selected. Values are 0, 1, 2, and 4. */
	// 000 = no drive
	// 001 = drive 1
	// 010 = drive 2
	// 100 = drive 3
	int m_DSEL;

	// Signal SIDSEL. 0 or 1, indicates the selected head.
	line_state m_SIDSEL;

	// count 4.23s from rising edge of motor_on
	emu_timer*      m_motor_on_timer;

	// DSR ROM
	UINT8*          m_dsrrom;

	// Buffer RAM
	UINT8*          m_buffer_ram;

	// Link to the attached floppy drives
	floppy_image_device*    m_floppy[4];

	// Currently selected floppy drive
	floppy_image_device*    m_current_floppy;

	// Link to the WD1773 controller on the board.
	required_device<wd1773_t>   m_wd1773;

	// Link to the real-time clock on the board.
	required_device<mm58274c_device> m_clock;

	// Debugging
	bool m_debug_dataout;
};
#endif
