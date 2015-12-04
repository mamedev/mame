// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*******************************************************************************
    SNUG BwG Disk Controller
    Based on WD1773
    Double Density, Double-sided

    * Supports Double Density.
    * As this card includes its own RAM, it does not need to allocate a portion
      of VDP RAM to store I/O buffers.
    * Includes a MM58274C RTC.
    * Support an additional floppy drive, for a total of 4 floppies.

    Reference:
    * BwG Disketten-Controller: Beschreibung der DSR (Description of the DSR)
        <http://home.t-online.de/home/harald.glaab/snug/bwg.pdf>

    +------------------------+
    |   32 KiB EPROM         | --- 1 of 4 pages--> 4000  +------------------+
    |                        |                           |   DSR space      |
    +------------------------+                           |   (Driver)       |
    |   2 KiB  RAM           | --- 1 of 2 pages--> 5c00  +------------------+
    +------------------------+                           |   RAM buffer     |
                                                   5fe0  +------------------+
                                                         |   RTC or WD1773  |
                                                   5fff  +------------------+

    Michael Zapf, September 2010
    January 2012: rewritten as class (MZ)
    February 2014: rewritten for new floppy subsystem (MZ)

    Known issues (Feb 2014):

   - The BwG controller cannot run with the Geneve or other non-9900 computers.
     The reason for that is the wait state logic. It assumes that when
     executing MOVB @>5FF6,*R2, first a value from 5FF7 is attempted to be read,
     just as the TI console does. In that case, wait states are inserted if
     necessary. The Geneve, however, will try to read a single byte from 5FF6
     only and therefore circumvent the wait state generation. This is in fact
     not an emulation glitch but the behavior of the real expansion card.


*******************************************************************************/

#include "emu.h"
#include "peribox.h"
#include "bwg.h"
#include "formats/ti99_dsk.h"
#include "imagedev/flopdrv.h"

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

#define MOTOR_TIMER 1
#define CLOCK_TAG "mm58274c"
#define FDC_TAG "wd1773"

#define BUFFER "ram"

/*
    Modern implementation
*/

snug_bwg_device::snug_bwg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
			: ti_expansion_card_device(mconfig, TI99_BWG, "SNUG BwG Floppy Controller", tag, owner, clock, "ti99_bwg", __FILE__), m_DRQ(), m_IRQ(), m_dip1(0), m_dip2(0), m_dip34(0), m_ram_page(0), m_rom_page(0), m_WAITena(false), m_inDsrArea(false), m_WDsel(false), m_WDsel0(false), m_RTCsel(false), m_lastK(false), m_dataregLB(false), m_rtc_enabled(false), m_MOTOR_ON(), m_lastval(0), m_address(0), m_DSEL(0), m_SIDSEL(), m_motor_on_timer(nullptr), m_dsrrom(nullptr), m_buffer_ram(nullptr), m_current_floppy(nullptr),
				m_wd1773(*this, FDC_TAG),
				m_clock(*this, CLOCK_TAG), m_debug_dataout(false)
		{ }

/*
    Operate the wait state logic.
*/
void snug_bwg_device::operate_ready_line()
{
	// This is the wait state logic
	if (TRACE_SIGNALS) logerror("bwg: address=%04x, DRQ=%d, INTRQ=%d, MOTOR=%d\n", m_address & 0xffff, m_DRQ, m_IRQ, m_MOTOR_ON);
	line_state nready = (m_dataregLB &&         // Are we accessing 5ff7
			m_WAITena &&                        // and the wait state generation is active (SBO 2)
			(m_DRQ==CLEAR_LINE) &&              // and we are waiting for a byte
			(m_IRQ==CLEAR_LINE) &&              // and there is no interrupt yet
			(m_MOTOR_ON==ASSERT_LINE)           // and the motor is turning?
			)? ASSERT_LINE : CLEAR_LINE;        // In that case, clear READY and thus trigger wait states

	if (TRACE_READY) if (nready==ASSERT_LINE) logerror("bwg: READY line = %d\n", (nready==CLEAR_LINE)? 1:0);
	m_slot->set_ready((nready==CLEAR_LINE)? ASSERT_LINE : CLEAR_LINE);
}

/*
    Callbacks from the WD1773 chip
*/
WRITE_LINE_MEMBER( snug_bwg_device::fdc_irq_w )
{
	if (TRACE_SIGNALS) logerror("bwg: set intrq = %d\n", state);
	m_IRQ = (line_state)state;
	// Unlike the TI FDC, the BwG does not set the INTB line. Anyway, no one cares.
	// We need to explicitly set the READY line to release the datamux
	operate_ready_line();
}

WRITE_LINE_MEMBER( snug_bwg_device::fdc_drq_w )
{
	if (TRACE_SIGNALS) logerror("bwg: set drq = %d\n", state);
	m_DRQ = (line_state)state;

	// We need to explicitly set the READY line to release the datamux
	operate_ready_line();
}

SETADDRESS_DBIN_MEMBER( snug_bwg_device::setaddress_dbin )
{
	// Do not allow setaddress for debugger
	if (space.debugger_access()) return;

	// Selection login in the PAL and some circuits on the board

	// Is the card being selected?
	m_address = offset;
	m_inDsrArea = ((m_address & m_select_mask)==m_select_value);

	if (!m_inDsrArea) return;

	if (TRACE_ADDRESS) logerror("bwg: set address = %04x\n", offset & 0xffff);

	// Is the WD chip on the card being selected?
	// We need the even and odd addresses for the wait state generation,
	// but only the even addresses when we access it
	m_WDsel0 = m_inDsrArea && !m_rtc_enabled
			&& ((state==ASSERT_LINE && ((m_address & 0x1ff8)==0x1ff0))    // read
				|| (state==CLEAR_LINE && ((m_address & 0x1ff8)==0x1ff8)));  // write

	m_WDsel = m_WDsel0 && ((m_address & 1)==0);

	// Is the RTC selected on the card? (even addr)
	m_RTCsel = m_inDsrArea && m_rtc_enabled && ((m_address & 0x1fe1)==0x1fe0);

	// RTC disabled:
	// 5c00 - 5fef: RAM
	// 5ff0 - 5fff: Controller (f0 = status, f2 = track, f4 = sector, f6 = data)

	// RTC enabled:
	// 5c00 - 5fdf: RAM
	// 5fe0 - 5fff: Clock (even addr)

	// Is RAM selected? We just check for the last 1K and let the RTC or WD
	// just take control before
	m_lastK = m_inDsrArea && ((m_address & 0x1c00)==0x1c00);

	// Is the data register port of the WD being selected?
	// In fact, the address to read the data from is 5FF6, but the TI-99 datamux
	// fetches both bytes from 5FF7 and 5FF6, the odd one first. The BwG uses
	// the odd address to operate the READY line
	m_dataregLB = m_WDsel0 && ((m_address & 0x07)==0x07);

	// Clear or assert the outgoing READY line
	operate_ready_line();
}

/*
    Access for debugger. This is a stripped-down version of the
    main methods below. We only allow ROM and RAM access.
*/
void snug_bwg_device::debug_read(offs_t offset, UINT8* value)
{
	if (((offset & m_select_mask)==m_select_value) && m_selected)
	{
		if ((offset & 0x1c00)==0x1c00)
		{
			if ((offset & 0x1fe0)!=0x1fe0)
				*value = m_buffer_ram[(m_ram_page<<10) | (offset & 0x03ff)];
		}
		else
			*value = m_dsrrom[(m_rom_page<<13) | (offset & 0x1fff)];
	}
}

void snug_bwg_device::debug_write(offs_t offset, UINT8 data)
{
	if (((offset & m_select_mask)==m_select_value) && m_selected)
	{
		if (((offset & 0x1c00)==0x1c00) && ((offset & 0x1fe0)!=0x1fe0))
				m_buffer_ram[(m_ram_page<<10) | (m_address & 0x03ff)] = data;
	}
}

/*
    Read a byte from ROM, RAM, FDC, or RTC. See setaddress_dbin for selection
    logic.
*/
READ8Z_MEMBER(snug_bwg_device::readz)
{
	if (space.debugger_access())
	{
		debug_read(offset, value);
		return;
	}

	if (m_inDsrArea && m_selected)
	{
		// 010x xxxx xxxx xxxx
		if (m_lastK)
		{
			// ...1 11xx xxxx xxxx
			if (m_rtc_enabled)
			{
				if (m_RTCsel)
				{
					// .... ..11 111x xxx0
					*value = m_clock->read(space, (m_address & 0x001e) >> 1);
					if (TRACE_RW) logerror("bwg: read RTC: %04x -> %02x\n", m_address & 0xffff, *value);
				}
				else
				{
					*value = m_buffer_ram[(m_ram_page<<10) | (m_address & 0x03ff)];
					if (TRACE_RW) logerror("bwg: read ram: %04x (page %d)-> %02x\n", m_address & 0xffff, m_ram_page, *value);
				}
			}
			else
			{
				if (m_WDsel)
				{
					// .... ..11 1111 0xx0
					// Note that the value is inverted again on the board,
					// so we can drop the inversion
					*value = m_wd1773->gen_r((m_address >> 1)&0x03);
					if (TRACE_RW) logerror("bwg: read FDC: %04x -> %02x\n", m_address & 0xffff, *value);
					if (TRACE_DATA)
					{
						if ((m_address & 0xffff)==0x5ff6) logerror("%02x ", *value);
						else logerror("\n%04x: %02x", m_address&0xffff, *value);
					}
				}
				else
				{
					*value = m_buffer_ram[(m_ram_page<<10) | (m_address & 0x03ff)];
					if (TRACE_RW) logerror("bwg: read ram: %04x (page %d)-> %02x\n", m_address & 0xffff, m_ram_page, *value);
				}
			}
		}
		else
		{
			*value = m_dsrrom[(m_rom_page<<13) | (m_address & 0x1fff)];
			if (TRACE_RW) logerror("bwg: read dsr: %04x (page %d)-> %02x\n", m_address & 0xffff, m_rom_page, *value);
		}
	}
}

/*
    Write a byte
    4000 - 5bff: ROM, ignore write (4 banks)

    rtc disabled:
    5c00 - 5fef: RAM
    5ff0 - 5fff: Controller (f8 = command, fa = track, fc = sector, fe = data)

    rtc enabled:
    5c00 - 5fdf: RAM
    5fe0 - 5fff: Clock (even addr)
*/
WRITE8_MEMBER(snug_bwg_device::write)
{
	if (space.debugger_access())
	{
		debug_write(offset, data);
		return;
	}

	if (m_inDsrArea && m_selected)
	{
		if (m_lastK)
		{
			if (m_rtc_enabled)
			{
				if (m_RTCsel)
				{
					// .... ..11 111x xxx0
					if (TRACE_RW) logerror("bwg: write RTC: %04x <- %02x\n", m_address & 0xffff, data);
					m_clock->write(space, (m_address & 0x001e) >> 1, data);
				}
				else
				{
					if (TRACE_RW) logerror("bwg: write ram: %04x (page %d) <- %02x\n", m_address & 0xffff, m_ram_page, data);
					m_buffer_ram[(m_ram_page<<10) | (m_address & 0x03ff)] = data;
				}
			}
			else
			{
				if (m_WDsel)
				{
					// .... ..11 1111 1xx0
					// Note that the value is inverted again on the board,
					// so we can drop the inversion
					if (TRACE_RW) logerror("bwg: write FDC: %04x <- %02x\n", m_address & 0xffff, data);
					m_wd1773->gen_w((m_address >> 1)&0x03, data);
				}
				else
				{
					if (TRACE_RW) logerror("bwg: write ram: %04x (page %d) <- %02x\n", m_address & 0xffff, m_ram_page, data);
					m_buffer_ram[(m_ram_page<<10) | (m_address & 0x03ff)] = data;
				}
			}
		}
	}
}

/*
    CRU read handler. *=inverted.
    bit 0: DSK4 connected*
    bit 1: DSK1 connected*
    bit 2: DSK2 connected*
    bit 3: DSK3 connected*
    bit 4: Dip 1
    bit 5: Dip 2
    bit 6: Dip 3
    bit 7: Dip 4
*/
READ8Z_MEMBER(snug_bwg_device::crureadz)
{
	UINT8 reply = 0;

	if ((offset & 0xff00)==m_cru_base)
	{
		if ((offset & 0x00ff)==0)
		{
			// Check what drives are not connected
			reply = ((m_floppy[0] != nullptr)? 0 : 0x02)       // DSK1
					| ((m_floppy[1] != nullptr)? 0 : 0x04) // DSK2
					| ((m_floppy[2] != nullptr)? 0 : 0x08) // DSK3
					| ((m_floppy[3] != nullptr)? 0 : 0x01);    // DSK4

			// DIP switches for step and date/time display
			if (m_dip1 != 0) reply |= 0x10;
			if (m_dip2 != 0) reply |= 0x20;

			// DIP switches for drive range selection
			// 00 = only DSK1; 01 = DSK1+DSK2, 10=DSK1+DSK2+DSK3, 11=all
			reply |= (m_dip34 << 6);

			// Invert all
			*value = ~reply;
		}
		else
			*value = 0;
		if (TRACE_CRU) logerror("bwg: Read CRU = %02x\n", *value);
	}
}

WRITE8_MEMBER(snug_bwg_device::cruwrite)
{
//  int drive, drivebit;

	if ((offset & 0xff00)==m_cru_base)
	{
		int bit = (offset >> 1) & 0x0f;
		switch (bit)
		{
		case 0:
			/* (De)select the card. Indicated by a LED on the board. */
			m_selected = (data != 0);
			if (TRACE_CRU) logerror("bwg: Map DSR (bit 0) = %d\n", m_selected);
			break;

		case 1:
			// Activate motor
			if (data==1 && m_lastval==0)
			{   // on rising edge, set motor_running for 4.23s
				if (TRACE_CRU) logerror("bwg: trigger motor (bit 1)\n");
				set_floppy_motors_running(true);
			}
			m_lastval = data;
			break;

		case 2:
			/* Set disk ready/hold (bit 2) */
			// 0: ignore IRQ and DRQ
			// 1: TMS9900 is stopped until IRQ or DRQ are set
			// OR the motor stops rotating - rotates for 4.23s after write
			// to CRU bit 1
			if (TRACE_CRU) logerror("bwg: arm wait state logic (bit 2) = %d\n", data);
			m_WAITena = (data != 0);
			break;

		case 3:
			// Load disk heads (HLT pin) (bit 3). Not implemented.
			if (TRACE_CRU) logerror("bwg: set head load (bit 3) = %d\n", data);
			break;

		case 4:
			// Select drive 0-2 (DSK1-DSK3) (bits 4-6)
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
			if (TRACE_CRU) logerror("bwg: set side (bit 7) = %d\n", data);
			if (m_current_floppy != nullptr) m_current_floppy->ss_w(data);
			break;

		case 8:
			// Select drive 3 (DSK4) (bit 8) */
			m_DSEL = (data != 0)? (m_DSEL | 0x08) : (m_DSEL & 0xf7);
			set_drive();
			break;

		case 10:
			/* double density enable (active low) */
			if (TRACE_CRU) logerror("bwg: set double density (bit 10) = %d\n", data);
			m_wd1773->dden_w(data != 0);
			break;

		case 11:
			/* EPROM A13 */
			if (data != 0)
				m_rom_page |= 1;
			else
				m_rom_page &= 0xfe;  // 11111110
			if (TRACE_CRU) logerror("bwg: set ROM page (bit 11) = %d, page = %d\n", bit, m_rom_page);
			break;

		case 13:
			/* RAM A10 */
			m_ram_page = data;
			if (TRACE_CRU) logerror("bwg: set RAM page (bit 13) = %d, page = %d\n", bit, m_ram_page);
			break;

		case 14:
			/* Override FDC with RTC (active high) */
			if (TRACE_CRU) logerror("bwg: turn on RTC (bit 14) = %d\n", data);
			m_rtc_enabled = (data != 0);
			break;

		case 15:
			/* EPROM A14 */
			if (data != 0)
				m_rom_page |= 2;
			else
				m_rom_page &= 0xfd; // 11111101
			if (TRACE_CRU) logerror("bwg: set ROM page (bit 15) = %d, page = %d\n", bit, m_rom_page);
			break;

		case 9:
		case 12:
			/* Unused (bit 3, 9 & 12) */
			if (TRACE_CRU) logerror("bwg: set unknown bit %d = %d\n", bit, data);
			break;
		}
	}
}

/*
    Set the current drive. See also ti_fdc.c
*/
void snug_bwg_device::set_drive()
{
	if (TRACE_CRU) logerror("bwg: new DSEL = %d\n", m_DSEL);

	if ((m_DSEL != 0) && (m_DSEL != 1) && (m_DSEL != 2) && (m_DSEL != 4) && (m_DSEL != 8))
	{
		logerror("bwg: Warning - multiple drives selected\n");
	}

	// The schematics do not reveal any countermeasures against multiple selection
	// so we assume that the highest value wins.

	int bits = m_DSEL & 0x0f;
	int i = -1;

	while (bits != 0)
	{
		bits >>= 1;
		i++;
	}
	if (i != -1)
	{
		m_current_floppy = m_floppy[i];
		if (TRACE_CRU) logerror("bwg: Selected floppy %d\n", i);
	}
	else
	{
		m_current_floppy = nullptr;
		if (TRACE_CRU) logerror("bwg: All drives deselected\n");
	}
	m_wd1773->set_floppy(m_current_floppy);
}

/*
    Monoflop has gone back to the OFF state.
*/
void snug_bwg_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	set_floppy_motors_running(false);
}

/*
    All floppy motors are operated by the same line.
*/
void snug_bwg_device::set_floppy_motors_running(bool run)
{
	if (run)
	{
		if (TRACE_MOTOR)
			if (m_MOTOR_ON==CLEAR_LINE) logerror("bwg: Motor START\n");
		m_MOTOR_ON = ASSERT_LINE;
		m_motor_on_timer->adjust(attotime::from_msec(4230));
	}
	else
	{
		if (TRACE_MOTOR)
			if (m_MOTOR_ON==ASSERT_LINE) logerror("bwg: Motor STOP\n");
		m_MOTOR_ON = CLEAR_LINE;
	}

	// The motor-on line is connected to pin 20 which is falsely called "MO"
	// in the schematics; should be called "READY" as we are using the WD1773.
	m_wd1773->set_force_ready(run);

	// Set all motors
	for (int i=0; i < 4; i++)
		if (m_floppy[i] != nullptr) m_floppy[i]->mon_w((run)? 0 : 1);

	// The motor-on line also connects to the wait state logic
	operate_ready_line();
}

void snug_bwg_device::device_start(void)
{
	logerror("bwg: BWG start\n");
	m_dsrrom = memregion(DSRROM)->base();
	m_buffer_ram = memregion(BUFFER)->base();
	m_motor_on_timer = timer_alloc(MOTOR_TIMER);
	m_cru_base = 0x1100;
}

void snug_bwg_device::device_reset()
{
	logerror("bwg: BWG reset\n");

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

	m_lastval = 0;

	m_DRQ = CLEAR_LINE;
	m_IRQ = CLEAR_LINE;
	m_MOTOR_ON = CLEAR_LINE;

	m_SIDSEL = CLEAR_LINE;

	m_wd1773->set_force_ready(false);

	m_DSEL = 0;
	m_WAITena = false;
	m_selected = false;
	m_debug_dataout = false;
	m_rtc_enabled = false;
	m_dataregLB = false;
	m_lastK = false;
	m_RTCsel = false;
	m_inDsrArea = false;
	m_address = 0;
	m_WDsel = false;
	m_WDsel0 = false;

	for (int i=0; i < 4; i++)
	{
		if (m_floppy[i] != nullptr)
			logerror("bwg: Connector %d with %s\n", i, m_floppy[i]->name());
		else
			logerror("bwg: Connector %d has no floppy attached\n", i);
	}

	m_wd1773->set_floppy(m_current_floppy = m_floppy[0]);

	m_dip1 = ioport("BWGDIP1")->read();
	m_dip2 = ioport("BWGDIP2")->read();
	m_dip34 = ioport("BWGDIP34")->read();

	m_rom_page = 0;
	m_ram_page = 0;
}

void snug_bwg_device::device_config_complete()
{
	for (int i=0; i < 4; i++)
		m_floppy[i] = nullptr;

	// Seems to be null when doing a "-listslots"
	if (subdevice("0")!=nullptr) m_floppy[0] = static_cast<floppy_image_device*>(subdevice("0")->first_subdevice());
	if (subdevice("1")!=nullptr) m_floppy[1] = static_cast<floppy_image_device*>(subdevice("1")->first_subdevice());
	if (subdevice("2")!=nullptr) m_floppy[2] = static_cast<floppy_image_device*>(subdevice("2")->first_subdevice());
	if (subdevice("3")!=nullptr) m_floppy[3] = static_cast<floppy_image_device*>(subdevice("3")->first_subdevice());
}

INPUT_PORTS_START( bwg_fdc )
	PORT_START( "BWGDIP1" )
	PORT_DIPNAME( 0x01, 0x00, "BwG step rate" )
		PORT_DIPSETTING( 0x00, "6 ms")
		PORT_DIPSETTING( 0x01, "20 ms")

	PORT_START( "BWGDIP2" )
	PORT_DIPNAME( 0x01, 0x00, "BwG date/time display" )
		PORT_DIPSETTING( 0x00, "Hide")
		PORT_DIPSETTING( 0x01, "Show")

	PORT_START( "BWGDIP34" )
	PORT_DIPNAME( 0x03, 0x03, "BwG drives" )
		PORT_DIPSETTING( 0x00, "DSK1 only")
		PORT_DIPSETTING( 0x01, "DSK1-DSK2")
		PORT_DIPSETTING( 0x02, "DSK1-DSK3")
		PORT_DIPSETTING( 0x03, "DSK1-DSK4")
INPUT_PORTS_END

FLOPPY_FORMATS_MEMBER(snug_bwg_device::floppy_formats)
	FLOPPY_TI99_SDF_FORMAT,
	FLOPPY_TI99_TDF_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( bwg_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )        // 40 tracks
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )        // 80 tracks
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )          // 80 tracks
SLOT_INTERFACE_END

MACHINE_CONFIG_FRAGMENT( bwg_fdc )
	MCFG_WD1773_ADD(FDC_TAG, XTAL_8MHz)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(snug_bwg_device, fdc_irq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(snug_bwg_device, fdc_drq_w))

	MCFG_DEVICE_ADD(CLOCK_TAG, MM58274C, 0)
	MCFG_MM58274C_MODE24(1) // 24 hour
	MCFG_MM58274C_DAY1(0)   // sunday

	MCFG_FLOPPY_DRIVE_ADD("0", bwg_floppies, "525dd", snug_bwg_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("1", bwg_floppies, "525dd", snug_bwg_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("2", bwg_floppies, nullptr, snug_bwg_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("3", bwg_floppies, nullptr, snug_bwg_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

ROM_START( bwg_fdc )
	ROM_REGION(0x8000, DSRROM, 0)
	ROM_LOAD("bwg.bin", 0x0000, 0x8000, CRC(06f1ec89) SHA1(6ad77033ed268f986d9a5439e65f7d391c4b7651)) /* BwG disk DSR ROM */
	ROM_REGION(0x0800, BUFFER, 0)  /* BwG RAM buffer */
	ROM_FILL(0x0000, 0x0400, nullptr)
ROM_END

machine_config_constructor snug_bwg_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( bwg_fdc );
}

ioport_constructor snug_bwg_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bwg_fdc );
}

const rom_entry *snug_bwg_device::device_rom_region() const
{
	return ROM_NAME( bwg_fdc );
}

const device_type TI99_BWG = &device_creator<snug_bwg_device>;
