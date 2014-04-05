// license:MAME|LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Myarc Hard and Floppy Disk Controller
    See hfdc.c for documentation

    Michael Zapf, September 2010

    January 2012: rewritten as class

****************************************************************************/

#ifndef __HFDC__
#define __HFDC__

#include "emu.h"
#include "peribox.h"
#include "machine/ti99_hd.h"
#include "machine/smc92x4.h"
#include "machine/mm58274c.h"

#define HFDC_MAX_FLOPPY 4
#define HFDC_MAX_HARD 4

extern const device_type TI99_HFDC;

class myarc_hfdc_device : public ti_expansion_card_device
{
public:
	myarc_hfdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);

	DECLARE_WRITE_LINE_MEMBER( intrq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );
	DECLARE_WRITE_LINE_MEMBER( dip_w );
	DECLARE_READ8_MEMBER( auxbus_in );
	DECLARE_WRITE8_MEMBER( auxbus_out );
	DECLARE_READ8_MEMBER( read_buffer );
	DECLARE_WRITE8_MEMBER( write_buffer );

protected:
	virtual void device_start(void);
	virtual void device_reset(void);
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);


private:
	// Calculates a simple version of a binary logarithm
	int             slog2(int value);

	// When true, triggers motor monoflop.
	bool            m_trigger_motor;

	// When true, motor monoflop is high
	bool            m_motor_running;

	/* Clock divider bit 0. Unused in this emulation. */
	int             m_CD0;

	/* Clock divider bit 1. Unused in this emulation. */
	int             m_CD1;

	/* count 4.23s from rising edge of motor_on */
	emu_timer*      m_motor_on_timer;

	// Link to the HDC9234 controller on the board. In fact, the proper name
	// is HDC 9234, the manufacturer is Standard Microsystems Corp.
	required_device<smc92x4_device> m_hdc9234;

	/* Link to the clock chip on the board. */
	required_device<mm58274c_device> m_clock;

	/* Determines whether we have access to the CRU bits. */
	bool            m_cru_select;

	/* IRQ state */
	bool            m_irq;

	/* DMA in Progress state */
	bool            m_dip;

	/* Output 1 latch */
	UINT8           m_output1_latch;

	/* Output 2 latch */
	UINT8           m_output2_latch;

	/* Connected floppy drives. */
	legacy_floppy_image_device*       m_floppy_unit[HFDC_MAX_FLOPPY];

	/* Connected harddisk drives. */
	mfm_harddisk_device*       m_harddisk_unit[HFDC_MAX_HARD];

	/* DMA address latch */
	UINT32          m_dma_address;

	// Device Service Routine ROM
	UINT8*          m_dsrrom;

	// ROM banks.
	int             m_rom_page;

	// HFDC RAM
	UINT8*          m_buffer_ram;

	// RAM page registers
	int             m_ram_page[4];
};


#endif
