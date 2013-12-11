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

#define TI_FDC_TAG "ti_dssd_controller"

#define FDC_TAG "wd1771"

#define MOTOR_TIMER 1

const wd17xx_interface ti_wd17xx_interface =
{
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ti_fdc_device, intrq_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ti_fdc_device, drq_w),
	{ PFLOPPY_0, PFLOPPY_1, PFLOPPY_2, NULL }
};

ti_fdc_device::ti_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
			: ti_expansion_card_device(mconfig, TI99_FDC, "TI-99 Standard DSSD Floppy Controller", tag, owner, clock, "ti99_fdc", __FILE__),
			m_fd1771(*this, FDC_TAG) { }

/*
    callback called at the end of DVENA pulse
*/
void ti_fdc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_DVENA = CLEAR_LINE;
	if (TRACE_MOTOR) logerror("tifdc: Motor off\n");
	set_ready_line();
}

/*
    Operate the wait state logic.
*/
void ti_fdc_device::set_ready_line()
{
	// This is the wait state logic
	if (TRACE_SIGNALS) logerror("tifdc: address=%04x, DRQ=%d, INTRQ=%d, MOTOR=%d\n", m_address & 0xffff, m_DRQ, m_IRQ, m_DVENA);
	line_state nready = (m_WDsel &&             // Are we accessing 5ffx?
			m_WAITena &&                        // and the wait state generation is active (SBO 2)
			(m_DRQ==CLEAR_LINE) &&              // and we are waiting for a byte
			(m_IRQ==CLEAR_LINE) &&              // and there is no interrupt yet
			(m_DVENA==ASSERT_LINE)              // and the motor is turning?
			)? ASSERT_LINE : CLEAR_LINE;        // In that case, clear READY and thus trigger wait states

	if (TRACE_READY) if (nready==ASSERT_LINE) logerror("tifdc: READY line = %d\n", (nready==CLEAR_LINE)? 1:0);
	m_slot->set_ready((nready==CLEAR_LINE)? ASSERT_LINE : CLEAR_LINE);
}

SETADDRESS_DBIN_MEMBER( ti_fdc_device::setaddress_dbin )
{
	// Selection login in the PAL and some circuits on the board

	// Is the card being selected?
	m_address = offset;
	m_inDsrArea = ((m_address & m_select_mask)==m_select_value);

	if (!m_inDsrArea) return;

	if (TRACE_ADDRESS) logerror("tifdc: set address = %04x\n", offset & 0xffff);

	// Is the WD chip on the card being selected?
	m_WDsel = m_inDsrArea && ((m_address & 0x1ff0)==0x1ff0);

	// Clear or assert the outgoing READY line
	set_ready_line();
}

READ8Z_MEMBER(ti_fdc_device::readz)
{
	if (m_inDsrArea && m_selected)
	{
		// only use the even addresses from 1ff0 to 1ff6.
		// Note that data is inverted.
		// 0101 1111 1111 0xx0
		UINT8 reply = 0;

		if (m_WDsel && ((m_address & 1)==0))
		{
			if (!space.debugger_access()) reply = wd17xx_r(m_fd1771, space, (offset >> 1)&0x03);
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

WRITE8_MEMBER(ti_fdc_device::write)
{
	if (m_inDsrArea && m_selected)
	{
		if (TRACE_RW) logerror("ti_fdc: %04x <- %02x\n", offset & 0xffff, ~data & 0xff);
		// only use the even addresses from 1ff8 to 1ffe.
		// Note that data is inverted.
		// 0101 1111 1111 1xx0
		if (m_WDsel && ((m_address & 9)==8))
		{
			if (!space.debugger_access()) wd17xx_w(m_fd1771, space, (offset >> 1)&0x03, data);
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
READ8Z_MEMBER(ti_fdc_device::crureadz)
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

WRITE8_MEMBER(ti_fdc_device::cruwrite)
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
					wd17xx_set_drive(m_fd1771, drive);
				}
			}
			else
				m_DSEL &= ~drivebit;
			break;

		case 7:
			/* Select side of disk (bit 7) */
			m_SIDSEL = data;
			if (TRACE_CRU) logerror("tifdc: set side (bit 7) = %d\n", data);
			wd17xx_set_side(m_fd1771, data);
			break;
		}
	}
}

/*
    Resets the drive geometry. This is required because the heuristic of
    the default implementation sets the drive geometry to the geometry
    of the medium.
*/
void ti_fdc_device::set_geometry(device_t *drive, floppy_type_t type)
{
	// This assertion may fail when the names of the floppy devices change.
	// Unfortunately, the wd17xx device assumes the floppy drives at root
	// level, so we use an explicitly qualified tag. See peribox.h.
	assert (drive!=NULL);
	floppy_drive_set_geometry(drive, type);
}

void ti_fdc_device::set_all_geometries(floppy_type_t type)
{
	set_geometry(machine().device(PFLOPPY_0), type);
	set_geometry(machine().device(PFLOPPY_1), type);
	set_geometry(machine().device(PFLOPPY_2), type);
}

/*
    Callback, called from the controller chip whenever DRQ/IRQ state change
*/
WRITE_LINE_MEMBER( ti_fdc_device::intrq_w )
{
	if (TRACE_SIGNALS) logerror("ti_fdc: set irq = %d\n", state);
	m_IRQ = (line_state)state;
	// Note that INTB is actually not used in the TI-99 family. But the
	// controller asserts the line nevertheless, probably intended for
	// use in another planned TI system
	m_slot->set_intb(state);
	set_ready_line();
}

WRITE_LINE_MEMBER( ti_fdc_device::drq_w )
{
	if (TRACE_SIGNALS) logerror("ti_fdc: set drq = %d\n", state);
	m_DRQ = (line_state)state;
	set_ready_line();
}

void ti_fdc_device::device_start(void)
{
	logerror("ti_fdc: TI FDC start\n");
	m_dsrrom = memregion(DSRROM)->base();
	m_motor_on_timer = timer_alloc(MOTOR_TIMER);
	m_cru_base = 0x1100;
}

void ti_fdc_device::device_reset(void)
{
	logerror("ti_fdc: TI FDC reset\n");
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

	ti99_set_80_track_drives(FALSE);
	floppy_type_t type = FLOPPY_STANDARD_5_25_DSDD_40;
	set_all_geometries(type);
}

MACHINE_CONFIG_FRAGMENT( ti_fdc )
	MCFG_FD1771_ADD(FDC_TAG, ti_wd17xx_interface )
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
