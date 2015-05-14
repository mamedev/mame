// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Myarc Hard and Floppy Disk Controller
    See hfdc.c for documentation

    Michael Zapf, September 2010

    January 2012: rewritten as class
    June 2014: rewritten for modern floppy implementation

    WORK IN PROGRESS

****************************************************************************/

#ifndef __HFDC__
#define __HFDC__

#define HFDC_MAX_FLOPPY 4
#define HFDC_MAX_HARD 3

#include "imagedev/floppy.h"
#include "machine/mm58274c.h"
#include "machine/hdc9234.h"
#include "machine/ti99_hd.h"

extern const device_type TI99_HFDC;

/*
    Implementation for modern floppy system.
*/
class myarc_hfdc_device : public ti_expansion_card_device
{
public:
	myarc_hfdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin);
	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);

	DECLARE_WRITE_LINE_MEMBER( dmarq_w );
	DECLARE_WRITE_LINE_MEMBER( intrq_w );
	DECLARE_WRITE_LINE_MEMBER( dip_w );
	DECLARE_WRITE8_MEMBER( auxbus_out );
	DECLARE_READ8_MEMBER( read_buffer );
	DECLARE_WRITE8_MEMBER( write_buffer );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	void device_config_complete();

private:
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	void device_start();
	void device_reset();

	const rom_entry *device_rom_region() const;
	machine_config_constructor device_mconfig_additions() const;
	ioport_constructor device_input_ports() const;

	// Debug accessors
	void debug_read(offs_t offset, UINT8* value);
	void debug_write(offs_t offset, UINT8 data);

	// Callbacks for the index hole and seek complete
	void floppy_index_callback(floppy_image_device *floppy, int state);
	void harddisk_index_callback(mfm_harddisk_device *harddisk, int state);
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
	floppy_image_device*    m_floppy_unit[4];

	// Link to the attached hard disks
	mfm_harddisk_device*    m_harddisk_unit[3];

	// Currently selected floppy drive
	floppy_image_device*    m_current_floppy;

	// Currently selected hard drive
	mfm_harddisk_device*    m_current_harddisk;

	// True: Access to DIP switch settings, false: access to line states
	bool    m_see_switches;

	// IRQ state
	line_state    m_irq;

	// DMA in Progress state
	line_state    m_dip;

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

	// Device Service Routine ROM (firmware)
	UINT8*  m_dsrrom;

	// ROM banks.
	int     m_rom_page;

	// HFDC on-board SRAM (8K or 32K)
	UINT8*  m_buffer_ram;

	// RAM page registers
	int     m_ram_page[4];

	// Drive status latch (STB0)
	UINT8   m_status_latch;

	// DMA address latch (in Gate Array) (STB1)
	UINT32  m_dma_address;

	// Output 1 latch (STB2)
	UINT8   m_output1_latch;

	// Output 2 latch (STB3)
	UINT8   m_output2_latch;

	// Needed for triggering the motor monoflop
	UINT8 m_lastval;

	// Signal motor_on. When TRUE, makes all drives turning.
	line_state m_MOTOR_ON;

	// Calculates the index from the bit
	int bit_to_index(int value);

	// Utility function to set or unset bits in a byte
	void set_bits(UINT8& byte, int mask, bool set);

	// Joined ready line towards the controller
	void set_ready(int dev, bool ready);
	int  m_readyflags;
};

/* Connector for a MFM hard disk. See also floppy.c */
class mfm_harddisk_connector : public device_t,
								public device_slot_interface
{
public:
	mfm_harddisk_connector(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mfm_harddisk_connector();

	mfm_harddisk_device *get_device();

protected:
	void device_start();
};

extern const device_type MFM_HD_CONNECTOR;

#define MCFG_MFM_HARDDISK_ADD(_tag, _slot_intf, _def_slot)  \
	MCFG_DEVICE_ADD(_tag, MFM_HD_CONNECTOR, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false);

// =========================================================================

/*
    Legacy implementation.
*/
extern const device_type TI99_HFDC_LEG;

#include "machine/smc92x4.h"

class myarc_hfdc_legacy_device : public ti_expansion_card_device
{
public:
	myarc_hfdc_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
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
	mfm_harddisk_legacy_device*       m_harddisk_unit[HFDC_MAX_HARD];

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
