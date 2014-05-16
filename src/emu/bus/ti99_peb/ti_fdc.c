// license:MAME|LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Standard Floppy Disk Controller Card
    Based on WD1771
    Single Density, Double-sided

    Michael Zapf
    September 2010
    January 2012: rewritten as class (MZ)

****************************************************************************/

#include "emu.h"
#include "peribox.h"
#include "ti_fdc.h"
#include "formats/ti99_dsk.h"

// ----------------------------------
// Flags for debugging

// Show read and write accesses
#define TRACE_RW 0

// Show CRU bit accesses
#define TRACE_CRU 0

// Show ready line activity
#define TRACE_READY 0

// Show detailed signal activity
#define TRACE_SIGNALS 0

// Show sector data
#define TRACE_DATA 0

// Show address bus operations
#define TRACE_ADDRESS 0

// Show address bus operations
#define TRACE_MOTOR 0

// ----------------------------------
#define FDC_TAG "fd1771"
#define MOTOR_TIMER 1

#define TI_FDC_TAG "ti_dssd_controller"

ti_fdc_device::ti_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
			: ti_expansion_card_device(mconfig, TI99_FDC, "TI-99 Standard DSSD Floppy Controller", tag, owner, clock, "ti99_fdc", __FILE__),
			m_fd1771(*this, FDC_TAG) { }

/*
    Operate the wait state logic.
*/
void ti_fdc_device::operate_ready_line()
{
	// This is the wait state logic
	if (TRACE_SIGNALS) logerror("tifdc: address=%04x, DRQ=%d, INTRQ=%d, MOTOR=%d\n", m_address & 0xffff, m_DRQ, m_IRQ, m_DVENA);
	line_state nready = (m_WDsel &&             // Are we accessing 5ffx (even addr)?
			m_WAITena &&                        // and the wait state generation is active (SBO 2)
			(m_DRQ==CLEAR_LINE) &&              // and we are waiting for a byte
			(m_IRQ==CLEAR_LINE) &&              // and there is no interrupt yet
			(m_DVENA==ASSERT_LINE)              // and the motor is turning?
			)? ASSERT_LINE : CLEAR_LINE;        // In that case, clear READY and thus trigger wait states

	if (TRACE_READY) logerror("tifdc: READY line = %d\n", (nready==CLEAR_LINE)? 1:0);
	m_slot->set_ready((nready==CLEAR_LINE)? ASSERT_LINE : CLEAR_LINE);
}

/*
 * Callbacks from the FD1771 chip
 */
WRITE_LINE_MEMBER( ti_fdc_device::fdc_irq_w )
{
	m_IRQ = state? ASSERT_LINE : CLEAR_LINE;
	if (TRACE_SIGNALS) logerror("tifdc: INTRQ callback = %d\n", m_IRQ);
	operate_ready_line();
}

WRITE_LINE_MEMBER( ti_fdc_device::fdc_drq_w )
{
	m_DRQ = state? ASSERT_LINE : CLEAR_LINE;
	if (TRACE_SIGNALS) logerror("tifdc: DRQ callback = %d\n", m_DRQ);
	operate_ready_line();
}

// bool ti_fdc_device::dvena_r()
// {
//  if (TRACE_SIGNALS) logerror("tifdc: reading DVENA = %d\n", m_DVENA);
//  return (m_DVENA==ASSERT_LINE);
// }

SETADDRESS_DBIN_MEMBER( ti_fdc_device::setaddress_dbin )
{
	// Selection login in the PAL and some circuits on the board

	// Is the card being selected?
	m_address = offset;
	m_inDsrArea = ((m_address & m_select_mask)==m_select_value);

	if (!m_inDsrArea || !m_selected) return;

	if (TRACE_ADDRESS) logerror("tifdc: set address = %04x\n", offset & 0xffff);

	// Is the WD chip on the card being selected?
	m_WDsel = m_inDsrArea && ((m_address & 0x1ff1)==0x1ff0);

	// Clear or assert the outgoing READY line
	operate_ready_line();
}

READ8Z_MEMBER(ti_fdc_device::readz)
{
	if (m_inDsrArea && m_selected)
	{
		// Read ports of 1771 are mapped to 5FF0,2,4,6: 0101 1111 1111 0xx0
		// Note that incoming/outgoing data are inverted for FD1771
		UINT8 reply = 0;

		if (m_WDsel && ((m_address & 9)==0))
		{
			if (!space.debugger_access()) reply = m_fd1771->gen_r((offset >> 1)&0x03);
			if (TRACE_DATA)
			{
				if ((m_address & 0xffff)==0x5ff6)
				{
					if (!m_debug_dataout) logerror("tifdc: Read data = ");
					m_debug_dataout = true;
					logerror("%02x ", ~reply & 0xff);
				}
				else
				{
					if (m_debug_dataout) logerror("\n");
					m_debug_dataout = false;
				}
			}
		}
		else
		{
			reply = m_dsrrom[m_address & 0x1fff];
		}
		*value = reply;
		if (TRACE_RW) logerror("tifdc: %04x -> %02x\n", offset & 0xffff, *value);
	}
}

WRITE8_MEMBER(ti_fdc_device::write)
{
	if (m_inDsrArea && m_selected)
	{
		// Write ports of 1771 are mapped to 5FF8,A,C,E: 0101 1111 1111 1xx0
		// This is important for the TI console: The TMS9900 CPU always performs a
		// read operation before the write operation, and if we did not use
		// different read and write ports, it would attempt to read from the
		// controller before passing a command or data
		// to it. In the best case, nothing happens; in the worst case, status
		// flags may be reset by the read operation.

		// Note that incoming/outgoing data are inverted for FD1771
		if (TRACE_RW) logerror("tifdc: %04x <- %02x\n", offset & 0xffff, ~data & 0xff);
		if (m_WDsel && ((m_address & 9)==8))
		{
			// As this is a memory-mapped access we must prevent the debugger
			// from messing with the operation
			if (!space.debugger_access()) m_fd1771->gen_w((offset >> 1)&0x03, data);
		}
	}
}

/*
    CRU read access

       7     6     5     4      3     2     1     0
    +-----+-----+-----+------+-----+-----+-----+-----+
    | Side|  1  |  0  |DVENA*| DSK3| DSK2| DSK1| HLD |
    +-----+-----+-----+------+-----+-----+-----+-----+

    We have only 8 bits for query; within this implementation this means
    we only use the base address (offset 0).
*/
READ8Z_MEMBER(ti_fdc_device::crureadz)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		UINT8 reply = 0;
		if ((offset & 0x07) == 0)
		{
			// Selected drive
			reply |= ((m_DSEL)<<1);
			// The DVENA state is returned as inverted
			if (m_DVENA==CLEAR_LINE) reply |= 0x10;
			// Always 1
			reply |= 0x40;
			// Selected side
			if (m_SIDSEL==ASSERT_LINE) reply |= 0x80;
		}
		*value = reply;
		if (TRACE_CRU) logerror("tifdc: Read CRU = %02x\n", *value);
	}
}

WRITE8_MEMBER(ti_fdc_device::cruwrite)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		int bit = (offset >> 1) & 0x07;
		switch (bit)
		{
		case 0:
			// (De)select the card. Indicated by a LED on the board.
			m_selected = (data!=0);
			if (TRACE_CRU) logerror("tifdc: Map DSR (bit 0) = %d\n", m_selected);
			break;

		case 1:
			// Activate motor
			if (data==1 && m_lastval==0)
			{   // On rising edge, set motor_running for 4.23s
				if (TRACE_CRU) logerror("tifdc: trigger motor (bit 1)\n");
				set_floppy_motors_running(true);
			}
			m_lastval = data;
			break;

		case 2:
			// Set disk ready/hold (bit 2)
			// 0: ignore IRQ and DRQ
			// 1: TMS9900 is stopped until IRQ or DRQ are set
			// OR the motor stops rotating - rotates for 4.23s after write
			// to CRU bit 1
			m_WAITena = (data != 0);
			if (TRACE_CRU) logerror("tifdc: arm wait state logic (bit 2) = %d\n", data);
			break;

		case 3:
			// Load disk heads (HLT pin) (bit 3). Not implemented.
			if (TRACE_CRU) logerror("tifdc: set head load (bit 3) = %d\n", data);
			break;

		case 4:
			m_DSEL = (data != 0)? (m_DSEL | 0x01) : (m_DSEL & 0xfe);
			set_drive();
			break;
		case 5:
			m_DSEL = (data != 0)? (m_DSEL | 0x02) : (m_DSEL & 0xfd);
			set_drive();
			break;
		case 6:
			m_DSEL = (data != 0)? (m_DSEL | 0x04) : (m_DSEL & 0xfb);
			set_drive();
			break;

		case 7:
			// Select side of disk (bit 7)
			m_SIDSEL = (data==1)? ASSERT_LINE : CLEAR_LINE;
			if (TRACE_CRU) logerror("tifdc: set side (bit 7) = %d\n", data);
			if (m_current_floppy != NULL) m_current_floppy->ss_w(data);
			break;

		default:
			break;
		}
	}
}

void ti_fdc_device::set_drive()
{
	int i = -1;
	switch (m_DSEL)
	{
	case 0:
		m_current_floppy = NULL;
		if (TRACE_CRU) logerror("tifdc: all drives deselected\n");
		break;
	case 1:
		i = 0;
		break;
	case 2:
		i = 1;
		break;
	case 3:
		// The schematics do not reveal any countermeasures against multiple selection
		// so we assume that the highest value wins.
		i = 1;
		logerror("tifdc: Warning - multiple drives selected\n");
		break;
	case 4:
		i = 2;
		break;
	default:
		i = 2;
		logerror("tifdc: Warning - multiple drives selected\n");
		break;
	}
	if (TRACE_CRU) logerror("tifdc: new DSEL = %d\n", m_DSEL);
	if (i != -1) m_current_floppy = m_floppy[i];

	m_fd1771->set_floppy(m_current_floppy);
}

/*
    Monoflop has gone back to the OFF state.
*/
void ti_fdc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	set_floppy_motors_running(false);
}

/*
    All floppy motors are operated by the same line.
*/
void ti_fdc_device::set_floppy_motors_running(bool run)
{
	if (run)
	{
		if (TRACE_MOTOR)
			if (m_DVENA==CLEAR_LINE) logerror("tifdc: Motor START\n");
		m_DVENA = ASSERT_LINE;
		m_motor_on_timer->adjust(attotime::from_msec(4230));
	}
	else
	{
		if (TRACE_MOTOR)
			if (m_DVENA==ASSERT_LINE) logerror("tifdc: Motor STOP\n");
		m_DVENA = CLEAR_LINE;
	}

	// The monoflop is connected to the READY line
	m_fd1771->set_force_ready(run);

	// Set all motors
	for (int i=0; i < 3; i++)
		if (m_floppy[i] != NULL) m_floppy[i]->mon_w((run)? 0 : 1);

	// The motor-on line also connects to the wait state logic
	operate_ready_line();
}

void ti_fdc_device::device_start()
{
	logerror("tifdc: TI FDC start\n");
	m_dsrrom = memregion(DSRROM)->base();
	m_motor_on_timer = timer_alloc(MOTOR_TIMER);
	m_cru_base = 0x1100;
	// In case we implement a callback after all:
	// m_fd1771->setup_ready_cb(wd_fdc_t::rline_cb(FUNC(ti_fdc_device::dvena_r), this));
}

void ti_fdc_device::device_reset()
{
	logerror("tifdc: TI FDC reset\n");
	m_lastval = 0;
	if (m_genmod)
	{
		m_select_mask = 0x1fe000;
		m_select_value = 0x174000;
	}
	else
	{
		m_select_mask = 0x7e000;
		m_select_value = 0x74000;
	}
	m_DRQ = CLEAR_LINE;
	m_IRQ = CLEAR_LINE;
	m_DVENA = CLEAR_LINE;
	m_fd1771->set_force_ready(false);

	m_DSEL = 0;
	m_WAITena = false;
	m_selected = false;
	m_debug_dataout = false;
	m_inDsrArea = false;
	m_WDsel = false;

	for (int i=0; i < 3; i++)
	{
		if (m_floppy[i] != NULL)
			logerror("tifdc: Connector %d with %s\n", i, m_floppy[i]->name());
		else
			logerror("tifdc: No floppy attached to connector %d\n", i);
	}

	m_fd1771->set_floppy(m_current_floppy = m_floppy[0]);
}

void ti_fdc_device::device_config_complete()
{
	// Seems to be null when doing a "-listslots"
	if (subdevice("0")!=NULL) m_floppy[0] = static_cast<floppy_image_device*>(subdevice("0")->first_subdevice());
	if (subdevice("1")!=NULL) m_floppy[1] = static_cast<floppy_image_device*>(subdevice("1")->first_subdevice());
	if (subdevice("2")!=NULL) m_floppy[2] = static_cast<floppy_image_device*>(subdevice("2")->first_subdevice());
}

FLOPPY_FORMATS_MEMBER(ti_fdc_device::floppy_formats)
	FLOPPY_TI99_SDF_FORMAT,
	FLOPPY_TI99_TDF_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( tifdc_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

MACHINE_CONFIG_FRAGMENT( ti_fdc )
	MCFG_FD1771x_ADD(FDC_TAG, XTAL_1MHz)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(ti_fdc_device, fdc_irq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(ti_fdc_device, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("0", tifdc_floppies, "525dd", ti_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("1", tifdc_floppies, "525dd", ti_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("2", tifdc_floppies, NULL, ti_fdc_device::floppy_formats)
MACHINE_CONFIG_END

ROM_START( ti_fdc )
	ROM_REGION(0x2000, DSRROM, 0)
	ROM_LOAD("disk.bin", 0x0000, 0x2000, CRC(8f7df93f) SHA1(ed91d48c1eaa8ca37d5055bcf67127ea51c4cad5)) /* TI disk DSR ROM */
ROM_END

machine_config_constructor ti_fdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ti_fdc );
}

const rom_entry *ti_fdc_device::device_rom_region() const
{
	return ROM_NAME( ti_fdc );
}

const device_type TI99_FDC = &device_creator<ti_fdc_device>;
//===========================================================================

/***********************************************

     Legacy implementation, to be removed

***********************************************/

#define TI_FDCLEG_TAG "ti_dssd_controller_legacy"
#define FDCLEG_TAG "wd1771"

ti_fdc_legacy_device::ti_fdc_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
			: ti_expansion_card_device(mconfig, TI99_FDC_LEG, "TI-99 Standard DSSD Floppy Controller LEGACY", tag, owner, clock, "ti99_fdc_leg", __FILE__),
			m_fd1771(*this, FDCLEG_TAG) { }

/*
    callback called at the end of DVENA pulse
*/
void ti_fdc_legacy_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_DVENA = CLEAR_LINE;
	if (TRACE_MOTOR) logerror("tifdc: Motor off\n");
	set_ready_line();
}

/*
    Operate the wait state logic.
*/
void ti_fdc_legacy_device::set_ready_line()
{
	// This is the wait state logic
	if (TRACE_SIGNALS) logerror("tifdc: address=%04x, DRQ=%d, INTRQ=%d, MOTOR=%d\n", m_address & 0xffff, m_DRQ, m_IRQ, m_DVENA);
	line_state nready = (m_WDsel &&             // Are we accessing 5ffx (even addr)?
			m_WAITena &&                        // and the wait state generation is active (SBO 2)
			(m_DRQ==CLEAR_LINE) &&              // and we are waiting for a byte
			(m_IRQ==CLEAR_LINE) &&              // and there is no interrupt yet
			(m_DVENA==ASSERT_LINE)              // and the motor is turning?
			)? ASSERT_LINE : CLEAR_LINE;        // In that case, clear READY and thus trigger wait states

	if (TRACE_READY) logerror("tifdc: READY line = %d\n", (nready==CLEAR_LINE)? 1:0);
	m_slot->set_ready((nready==CLEAR_LINE)? ASSERT_LINE : CLEAR_LINE);
}

SETADDRESS_DBIN_MEMBER( ti_fdc_legacy_device::setaddress_dbin )
{
	// Selection login in the PAL and some circuits on the board

	// Is the card being selected?
	m_address = offset;
	m_inDsrArea = ((m_address & m_select_mask)==m_select_value);

	if (!m_inDsrArea) return;

	if (TRACE_ADDRESS) logerror("tifdc: set address = %04x\n", offset & 0xffff);

	// Is the WD chip on the card being selected?
	m_WDsel = m_inDsrArea && ((m_address & 0x1ff1)==0x1ff0);

	// Clear or assert the outgoing READY line
	set_ready_line();
}

READ8Z_MEMBER(ti_fdc_legacy_device::readz)
{
	if (m_inDsrArea && m_selected)
	{
		// only use the even addresses from 1ff0 to 1ff6.
		// Note that data is inverted.
		// 0101 1111 1111 0xx0
		UINT8 reply = 0;

		if (m_WDsel && ((m_address & 9)==0))
		{
			if (!space.debugger_access()) reply = m_fd1771->read(space, (offset >> 1)&0x03);
			if (TRACE_DATA)
			{
				if ((m_address & 0xffff)==0x5ff6) logerror("%02x ", ~reply & 0xff);
				else logerror("\n%04x: %02x", m_address&0xffff, ~reply & 0xff);
			}
		}
		else
		{
			reply = m_dsrrom[m_address & 0x1fff];
		}
		*value = reply;
		if (TRACE_RW) logerror("ti_fdc: %04x -> %02x\n", offset & 0xffff, *value);
	}
}

WRITE8_MEMBER(ti_fdc_legacy_device::write)
{
	if (m_inDsrArea && m_selected)
	{
		if (TRACE_RW) logerror("ti_fdc: %04x <- %02x\n", offset & 0xffff, ~data & 0xff);
		// only use the even addresses from 1ff8 to 1ffe.
		// Note that data is inverted.
		// 0101 1111 1111 1xx0
		if (m_WDsel && ((m_address & 9)==8))
		{
			if (!space.debugger_access()) m_fd1771->write(space, (offset >> 1)&0x03, data);
		}
	}
}

/*
    The CRU read handler.
    bit 0: HLD pin
    bit 1-3: drive n active
    bit 4: 0: motor strobe on
    bit 5: always 0
    bit 6: always 1
    bit 7: selected side
*/
READ8Z_MEMBER(ti_fdc_legacy_device::crureadz)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		int addr = offset & 0x07;
		UINT8 reply = 0;
		if (addr == 0)
		{
			// deliver bits 0-7
			// TODO: HLD pin
			// The DVENA state is returned inverted
			if (m_DVENA==ASSERT_LINE) reply |= ((m_DSEL)<<1);
			else reply |= 0x10;
			reply |= 0x40;
			if (m_SIDSEL) reply |= 0x80;
		}
		*value = reply;
		if (TRACE_CRU) logerror("tifdc: Read CRU = %02x\n", *value);
	}
}

WRITE8_MEMBER(ti_fdc_legacy_device::cruwrite)
{
	int drive, drivebit;

	if ((offset & 0xff00)==m_cru_base)
	{
		int bit = (offset >> 1) & 0x07;
		switch (bit)
		{
		case 0:
			/* (De)select the card. Indicated by a LED on the board. */
			m_selected = (data!=0);
			if (TRACE_CRU) logerror("tifdc: Map DSR (bit 0) = %d\n", m_selected);
			break;
		case 1:
			/* Activate motor */
			if (data==1 && m_lastval==0)
			{   /* on rising edge, set motor_running for 4.23s */
				if (TRACE_CRU) logerror("tifdc: trigger motor (bit 1)\n");
				m_DVENA = ASSERT_LINE;
				if (TRACE_MOTOR) logerror("tifdc: motor on\n");
				set_ready_line();
				m_motor_on_timer->adjust(attotime::from_msec(4230));
			}
			m_lastval = data;
			break;

		case 2:
			/* Set disk ready/hold (bit 2) */
			// 0: ignore IRQ and DRQ
			// 1: TMS9900 is stopped until IRQ or DRQ are set
			// OR the motor stops rotating - rotates for 4.23s after write
			// to CRU bit 1
			m_WAITena = (data != 0);
			if (TRACE_CRU) logerror("tifdc: arm wait state logic (bit 2) = %d\n", data);
			break;

		case 3:
			/* Load disk heads (HLT pin) (bit 3). Not implemented. */
			if (TRACE_CRU) logerror("tifdc: set head load (bit 3) = %d\n", data);
			break;

		case 4:
		case 5:
		case 6:
			/* Select drive X (bits 4-6) */
			drive = bit-4;                  /* drive # (0-2) */
			if (TRACE_CRU) logerror("tifdc: set drive (bit %d) = %d\n", bit, data);
			drivebit = 1<<drive;

			if (data != 0)
			{
				if ((m_DSEL & drivebit) == 0)         /* select drive */
				{
					if (m_DSEL != 0)
						logerror("tifdc: Multiple drives selected, %02x\n", m_DSEL);
					m_DSEL |= drivebit;
					m_fd1771->set_drive(drive);
				}
			}
			else
				m_DSEL &= ~drivebit;
			break;

		case 7:
			/* Select side of disk (bit 7) */
			m_SIDSEL = data;
			if (TRACE_CRU) logerror("tifdc: set side (bit 7) = %d\n", data);
			m_fd1771->set_side(data);
			break;
		}
	}
}

/*
    Resets the drive geometry. This is required because the heuristic of
    the default implementation sets the drive geometry to the geometry
    of the medium.
*/
void ti_fdc_legacy_device::set_geometry(legacy_floppy_image_device *drive, floppy_type_t type)
{
	// This assertion may fail when the names of the floppy devices change.
	// Unfortunately, the wd17xx device assumes the floppy drives at root
	// level, so we use an explicitly qualified tag. See peribox.h.
	assert (drive!=NULL);
	drive->floppy_drive_set_geometry(type);
}

void ti_fdc_legacy_device::set_all_geometries(floppy_type_t type)
{
	set_geometry(machine().device<legacy_floppy_image_device>(PFLOPPY_0), type);
	set_geometry(machine().device<legacy_floppy_image_device>(PFLOPPY_1), type);
	set_geometry(machine().device<legacy_floppy_image_device>(PFLOPPY_2), type);
}

/*
    Callback, called from the controller chip whenever DRQ/IRQ state change
*/
WRITE_LINE_MEMBER( ti_fdc_legacy_device::intrq_w )
{
	if (TRACE_SIGNALS) logerror("ti_fdc: set irq = %d\n", state);
	m_IRQ = (line_state)state;
	// Note that INTB is actually not used in the TI-99 family. But the
	// controller asserts the line nevertheless, probably intended for
	// use in another planned TI system
	m_slot->set_intb(state);
	set_ready_line();
}

WRITE_LINE_MEMBER( ti_fdc_legacy_device::drq_w )
{
	if (TRACE_SIGNALS) logerror("ti_fdc: set drq = %d\n", state);
	m_DRQ = (line_state)state;
	set_ready_line();
}

void ti_fdc_legacy_device::device_start(void)
{
	logerror("ti_fdc: TI FDC (legacy) start\n");
	m_dsrrom = memregion(DSRROM)->base();
	m_motor_on_timer = timer_alloc(MOTOR_TIMER);
	m_cru_base = 0x1100;
}

void ti_fdc_legacy_device::device_reset(void)
{
	logerror("ti_fdc: TI FDC (legacy) reset\n");
	m_DSEL = 0;
	m_SIDSEL = 0;
	m_DVENA = CLEAR_LINE;
	m_lastval = 0;
	if (m_genmod)
	{
		m_select_mask = 0x1fe000;
		m_select_value = 0x174000;
	}
	else
	{
		m_select_mask = 0x7e000;
		m_select_value = 0x74000;
	}
	m_DRQ = CLEAR_LINE;
	m_IRQ = CLEAR_LINE;
	m_WAITena = false;
	m_selected = false;
	m_inDsrArea = false;
	m_WDsel = false;

	ti99_set_80_track_drives(FALSE);
	floppy_type_t type = FLOPPY_STANDARD_5_25_DSDD_40;
	set_all_geometries(type);
}

MACHINE_CONFIG_FRAGMENT( ti_fdc_legacy )
	MCFG_DEVICE_ADD(FDCLEG_TAG, FD1771, 0)
	MCFG_WD17XX_DRIVE_TAGS(PFLOPPY_0, PFLOPPY_1, PFLOPPY_2, NULL)
	MCFG_WD17XX_INTRQ_CALLBACK(WRITELINE(ti_fdc_legacy_device, intrq_w))
	MCFG_WD17XX_DRQ_CALLBACK(WRITELINE(ti_fdc_legacy_device, drq_w))
MACHINE_CONFIG_END

ROM_START( ti_fdc_legacy )
	ROM_REGION(0x2000, DSRROM, 0)
	ROM_LOAD("disk.bin", 0x0000, 0x2000, CRC(8f7df93f) SHA1(ed91d48c1eaa8ca37d5055bcf67127ea51c4cad5)) /* TI disk DSR ROM */
ROM_END

machine_config_constructor ti_fdc_legacy_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ti_fdc_legacy );
}

const rom_entry *ti_fdc_legacy_device::device_rom_region() const
{
	return ROM_NAME( ti_fdc_legacy );
}

const device_type TI99_FDC_LEG = &device_creator<ti_fdc_legacy_device>;
