// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Standard Floppy Disk Controller Card
    See ti_fdc.c for documentation

    Michael Zapf

    September 2010
    January 2012: rewritten as class (MZ)

****************************************************************************/
#ifndef __TIFDC__
#define __TIFDC__

#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"

extern const device_type TI99_FDC;

class ti_fdc_device : public ti_expansion_card_device
{
public:
	ti_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin);

	DECLARE_WRITE_LINE_MEMBER( intrq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );

	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_WRITE_LINE_MEMBER( fdc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );
	// bool dvena_r();

protected:
	void device_start();
	void device_reset();
	void device_config_complete();

	const rom_entry *device_rom_region() const;
	machine_config_constructor device_mconfig_additions() const;

	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// Wait state logic
	void operate_ready_line();

	// Set the current floppy
	void set_drive();

	// Operate the floppy motors
	void set_floppy_motors_running(bool run);

	// Recent address
	int     m_address;

	// Holds the status of the DRQ and IRQ lines.
	line_state  m_DRQ, m_IRQ;

	// Needed for triggering the motor monoflop
	UINT8   m_lastval;

	// Signal DVENA. When TRUE, makes some drive turning.
	line_state  m_DVENA;

	// Set when address is in card area
	bool    m_inDsrArea;

	// When TRUE the CPU is halted while DRQ/IRQ are true.
	bool    m_WAITena;

	// WD chip selected
	bool    m_WDsel;

	// Indicates which drive has been selected. Values are 0, 1, 2, and 4.
	// 000 = no drive
	// 001 = drive 1
	// 010 = drive 2
	// 100 = drive 3
	int         m_DSEL;

	// Signal SIDSEL. 0 or 1, indicates the selected head.
	line_state        m_SIDSEL;

	// count 4.23s from rising edge of motor_on
	emu_timer*  m_motor_on_timer;

	// Link to the FDC1771 controller on the board.
	required_device<fd1771_t>   m_fd1771;

	// DSR ROM
	UINT8*  m_dsrrom;

	// Link to the attached floppy drives
	floppy_image_device*    m_floppy[3];

	// Currently selected floppy drive
	floppy_image_device*    m_current_floppy;

	// Debugging
	bool m_debug_dataout;
};
#endif
