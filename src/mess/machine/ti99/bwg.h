/****************************************************************************

    SNUG BwG Disk Controller
    Based on WD1770
    Double Density, Double-sided

    Michael Zapf, September 2010
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __BWG__
#define __BWG__

#include "ti99defs.h"
#include "imagedev/flopdrv.h"

extern const device_type TI99_BWG;

class snug_bwg_device : public ti_expansion_card_device
{
public:
	snug_bwg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	DECLARE_WRITE_LINE_MEMBER( intrq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );

	void crureadz(offs_t offset, UINT8 *value);
	void cruwrite(offs_t offset, UINT8 value);

protected:
	void device_start(void);
	void device_reset(void);
	const rom_entry *device_rom_region() const;
	machine_config_constructor device_mconfig_additions() const;
	ioport_constructor device_input_ports() const;

private:
	void handle_hold(void);
	void set_all_geometries(floppy_type_t type);
	void set_geometry(device_t *drive, floppy_type_t type);
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// Holds the status of the DRQ and IRQ lines.
	bool			m_DRQ, m_IRQ;

	// DIP switch state
	int				m_dip1, m_dip2, m_dip34;

	// Page selection for ROM and RAM
	int 			m_ram_page;  // 0-1
	int 			m_rom_page;  // 0-3

	// When TRUE the CPU is halted while DRQ/IRQ are true.
	bool			m_hold;

	/* Indicates whether the clock is mapped into the address space. */
	bool			m_rtc_enabled;

	/* When TRUE, keeps DVENA high. */
	bool			m_strobe_motor;

	// Signal DVENA. When TRUE, makes some drive turning.
	line_state		m_DVENA;

	/* Indicates which drive has been selected. Values are 0, 1, 2, and 4. */
	// 000 = no drive
	// 001 = drive 1
	// 010 = drive 2
	// 100 = drive 3
	int				m_DSEL;

	/* Signal SIDSEL. 0 or 1, indicates the selected head. */
	int				m_SIDE;

	// Link to the FDC1771 controller on the board.
	device_t*		m_controller;

	// Link to the real-time clock on the board.
	device_t*		m_clock;

	// count 4.23s from rising edge of motor_on
	emu_timer*		m_motor_on_timer;

	// DSR ROM
	UINT8*			m_dsrrom;

	// Buffer RAM
	UINT8*			m_buffer_ram;
};

#endif
