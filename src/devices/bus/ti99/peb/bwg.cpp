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
#include "bwg.h"
#include "formats/ti99_dsk.h"
#include "machine/rescap.h"

// ----------------------------------
// Flags for debugging

#define LOG_WARN        (1U << 1)    // Warnings
#define LOG_RW          (1U << 2)    // Read and write accesses
#define LOG_CRU         (1U << 3)    // Show CRU bit accesses
#define LOG_CRUD        (1U << 4)    // Show CRU bit accesses (details)
#define LOG_READY       (1U << 5)    // Show ready line activity
#define LOG_SIGNALS     (1U << 6)    // Show detailed signal activity
#define LOG_ADDRESS     (1U << 7)    // Show address bus operations
#define LOG_MOTOR       (1U << 8)    // Show motor operations
#define LOG_CONFIG      (1U << 9)    // Configuration

#define VERBOSE (LOG_GENERAL | LOG_CONFIG | LOG_WARN)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_BWG, bus::ti99::peb::snug_bwg_device, "ti99_bwg", "SNUG BwG Floppy Controller")

namespace bus::ti99::peb {

// ----------------------------------

#define MOTOR_TIMER 1
#define CLOCK_TAG "mm58274c"
#define FDC_TAG "wd1773"

#define BUFFER "ram"

/*
    Modern implementation
*/

snug_bwg_device::snug_bwg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	  device_t(mconfig, TI99_BWG, tag, owner, clock),
	  device_ti99_peribox_card_interface(mconfig, *this),
	  m_DRQ(), m_IRQ(),
	  m_dip1(0), m_dip2(0), m_dip34(0),
	  m_inDsrArea(false),
	  m_WDsel(false),
	  m_WDsel0(false),
	  m_RTCsel(false),
	  m_lastK(false),
	  m_dataregLB(false),
	  m_MOTOR_ON(),
	  m_address(0),
	  m_dsrrom(nullptr),
	  m_buffer_ram(*this, BUFFER),
	  m_floppy(*this, "%u", 0),
	  m_sel_floppy(0),
	  m_wd1773(*this, FDC_TAG),
	  m_clock(*this, CLOCK_TAG),
	  m_crulatch0_7(*this, "crulatch0_7"),
	  m_crulatch8_15(*this, "crulatch8_15"),
	  m_motormf(*this, "motormf")
	  { }

/*
    Operate the wait state logic.
*/
void snug_bwg_device::operate_ready_line()
{
	// This is the wait state logic
	LOGMASKED(LOG_SIGNALS, "address=%04x, DRQ=%d, INTRQ=%d, MOTOR=%d\n", m_address & 0xffff, m_DRQ, m_IRQ, m_MOTOR_ON);
	line_state nready = (m_dataregLB &&         // Are we accessing 5ff7
			m_crulatch0_7->q2_r() &&            // and the wait state generation is active (SBO 2)
			(m_DRQ==CLEAR_LINE) &&              // and we are waiting for a byte
			(m_IRQ==CLEAR_LINE) &&              // and there is no interrupt yet
			(m_MOTOR_ON==ASSERT_LINE)           // and the motor is turning?
			)? ASSERT_LINE : CLEAR_LINE;        // In that case, clear READY and thus trigger wait states

	if (nready==ASSERT_LINE) LOGMASKED(LOG_READY, "READY line = %d\n", (nready==CLEAR_LINE)? 1:0);
	m_slot->set_ready((nready==CLEAR_LINE)? ASSERT_LINE : CLEAR_LINE);
}

/*
    Callbacks from the WD1773 chip
*/
void snug_bwg_device::fdc_irq_w(int state)
{
	LOGMASKED(LOG_SIGNALS, "set intrq = %d\n", state);
	m_IRQ = (line_state)state;
	// Unlike the TI FDC, the BwG does not set the INTB line. Anyway, no one cares.
	// We need to explicitly set the READY line to release the datamux
	operate_ready_line();
}

void snug_bwg_device::fdc_drq_w(int state)
{
	LOGMASKED(LOG_SIGNALS, "set drq = %d\n", state);
	m_DRQ = (line_state)state;

	// We need to explicitly set the READY line to release the datamux
	operate_ready_line();
}

void snug_bwg_device::setaddress_dbin(offs_t offset, int state)
{
	// Do not allow setaddress for debugger
	if (machine().side_effects_disabled()) return;

	// Selection login in the PAL and some circuits on the board

	// Is the card being selected?
	m_address = offset;
	m_inDsrArea = in_dsr_space(m_address, true);

	if (!m_inDsrArea) return;

	LOGMASKED(LOG_ADDRESS, "set address = %04x\n", offset & 0xffff);

	// Is the WD chip on the card being selected?
	// We need the even and odd addresses for the wait state generation,
	// but only the even addresses when we access it
	m_WDsel0 = m_inDsrArea && m_crulatch8_15->q6_r()==CLEAR_LINE
			&& ((state==ASSERT_LINE && ((m_address & 0x1ff8)==0x1ff0))    // read
				|| (state==CLEAR_LINE && ((m_address & 0x1ff8)==0x1ff8)));  // write

	m_WDsel = m_WDsel0 && WORD_ALIGNED(m_address);

	// Is the RTC selected on the card? (even addr)
	m_RTCsel = m_inDsrArea && m_crulatch8_15->q6_r()==ASSERT_LINE && ((m_address & 0x1fe1)==0x1fe0);

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
void snug_bwg_device::debug_read(offs_t offset, uint8_t* value)
{
	if (in_dsr_space(offset, true) && m_selected)
	{
		if ((offset & 0x1c00)==0x1c00)
		{
			if ((offset & 0x1fe0)!=0x1fe0)
				*value = m_buffer_ram->pointer()[(m_crulatch8_15->q5_r()<<10) | (offset & 0x03ff)];
		}
		else
			*value = m_dsrrom[(m_crulatch8_15->q7_r()<<14) | (m_crulatch8_15->q3_r()<<13) | (offset & 0x1fff)];
	}
}

void snug_bwg_device::debug_write(offs_t offset, uint8_t data)
{
	if (in_dsr_space(offset, true) && m_selected)
	{
		if (((offset & 0x1c00)==0x1c00) && ((offset & 0x1fe0)!=0x1fe0))
			m_buffer_ram->pointer()[(m_crulatch8_15->q5_r()<<10) | (m_address & 0x03ff)] = data;
	}
}

/*
    Read a byte from ROM, RAM, FDC, or RTC. See setaddress_dbin for selection
    logic.
*/
void snug_bwg_device::readz(offs_t offset, uint8_t *value)
{
	if (machine().side_effects_disabled())
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
			int rampage = m_crulatch8_15->q5_r();
			if (m_crulatch8_15->q6_r()==ASSERT_LINE)
			{
				if (m_RTCsel)
				{
					// .... ..11 111x xxx0
					*value = m_clock->read((m_address & 0x001e) >> 1);
					LOGMASKED(LOG_RW, "read RTC: %04x -> %02x\n", m_address & 0xffff, *value);
				}
				else
				{
					*value = m_buffer_ram->pointer()[(rampage<<10) | (m_address & 0x03ff)];
					LOGMASKED(LOG_RW, "read ram: %04x (page %d)-> %02x\n", m_address & 0xffff, rampage, *value);
				}
			}
			else
			{
				if (m_WDsel)
				{
					// .... ..11 1111 0xx0
					// Note that the value is inverted again on the board,
					// so we can drop the inversion
					*value = m_wd1773->read((m_address >> 1)&0x03);
					LOGMASKED(LOG_RW, "read FDC: %04x -> %02x\n", m_address & 0xffff, *value);
				}
				else
				{
					*value = m_buffer_ram->pointer()[(rampage<<10) | (m_address & 0x03ff)];
					LOGMASKED(LOG_RW, "read ram: %04x (page %d)-> %02x\n", m_address & 0xffff, rampage, *value);
				}
			}
		}
		else
		{
			int rompage = (m_crulatch8_15->q7_r()<<1) | m_crulatch8_15->q3_r();
			*value = m_dsrrom[(rompage << 13) | (m_address & 0x1fff)];
			LOGMASKED(LOG_RW, "read dsr: %04x (page %d)-> %02x\n", m_address & 0xffff, rompage, *value);
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
void snug_bwg_device::write(offs_t offset, uint8_t data)
{
	if (machine().side_effects_disabled())
	{
		debug_write(offset, data);
		return;
	}

	if (m_inDsrArea && m_selected)
	{
		if (m_lastK)
		{
			int rampage = m_crulatch8_15->q5_r();
			if (m_crulatch8_15->q6_r()==ASSERT_LINE)
			{
				if (m_RTCsel)
				{
					// .... ..11 111x xxx0
					LOGMASKED(LOG_RW, "write RTC: %04x <- %02x\n", m_address & 0xffff, data);
					m_clock->write((m_address & 0x001e) >> 1, data);
				}
				else
				{
					LOGMASKED(LOG_RW, "write ram: %04x (page %d) <- %02x\n", m_address & 0xffff, rampage, data);
					m_buffer_ram->pointer()[(rampage<<10) | (m_address & 0x03ff)] = data;
				}
			}
			else
			{
				if (m_WDsel)
				{
					// .... ..11 1111 1xx0
					// Note that the value is inverted again on the board,
					// so we can drop the inversion
					LOGMASKED(LOG_RW, "write FDC: %04x <- %02x\n", m_address & 0xffff, data);
					m_wd1773->write((m_address >> 1)&0x03, data);
				}
				else
				{
					LOGMASKED(LOG_RW, "write ram: %04x (page %d) <- %02x\n", m_address & 0xffff, rampage, data);
					m_buffer_ram->pointer()[(rampage<<10) | (m_address & 0x03ff)] = data;
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
void snug_bwg_device::crureadz(offs_t offset, uint8_t *value)
{
	uint8_t reply;

	if ((offset & 0xff00)==m_cru_base)
	{
		if ((offset & 0x00f0)==0)
		{
			// Check what drives are not connected
			reply = ((m_floppy[0]->get_device() != nullptr)? 0 : 0x02)       // DSK1
					| ((m_floppy[1]->get_device() != nullptr)? 0 : 0x04) // DSK2
					| ((m_floppy[2]->get_device() != nullptr)? 0 : 0x08) // DSK3
					| ((m_floppy[3]->get_device() != nullptr)? 0 : 0x01);    // DSK4

			// DIP switches for step and date/time display
			if (m_dip1 != 0) reply |= 0x10;
			if (m_dip2 != 0) reply |= 0x20;

			// DIP switches for drive range selection
			// 00 = only DSK1; 01 = DSK1+DSK2, 10=DSK1+DSK2+DSK3, 11=all
			reply |= (m_dip34 << 6);

			// Invert all
			*value = ~BIT(reply, (offset >> 1) & 7);
		}
		else
			*value = 0;
		LOGMASKED(LOG_CRUD, "Read CRU = %02x\n", *value);
	}
}

void snug_bwg_device::cruwrite(offs_t offset, uint8_t data)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		LOGMASKED(LOG_CRUD, "Write CRU address %04x (bit %d) = %d\n", offset, (offset & 0xff)>>1, data);
		int bitnumber = (offset >> 1) & 0x0f;
		if (bitnumber < 8)
			m_crulatch0_7->write_bit(bitnumber, BIT(data, 0));
		else
			m_crulatch8_15->write_bit(bitnumber & 0x07, BIT(data, 0));
	}
}

void snug_bwg_device::den_w(int state)
{
	// (De)select the card. Indicated by a LED on the board.
	m_selected = state;
	LOGMASKED(LOG_CRU, "Map DSR (bit 0) = %d\n", m_selected);
}

void snug_bwg_device::mop_w(int state)
{
	m_motormf->b_w(state);
}

void snug_bwg_device::waiten_w(int state)
{
	/* Set disk ready/hold (bit 2) */
	// 0: ignore IRQ and DRQ
	// 1: TMS9900 is stopped until IRQ or DRQ are set
	// OR the motor stops rotating - rotates for 4.23s after write
	// to CRU bit 1
	LOGMASKED(LOG_CRU, "Arm wait state logic (bit 2) = %d\n", state);
}

void snug_bwg_device::hlt_w(int state)
{
	// Load disk heads (HLT pin) (bit 3). Not implemented.
	LOGMASKED(LOG_CRU, "Set head load (bit 3) = %d\n", state);
}

/* Drive selects */
void snug_bwg_device::dsel1_w(int state)
{
	select_drive(1, state);
}

void snug_bwg_device::dsel2_w(int state)
{
	select_drive(2, state);
}

void snug_bwg_device::dsel3_w(int state)
{
	select_drive(3, state);
}

void snug_bwg_device::dsel4_w(int state)
{
	select_drive(4, state);
}

void snug_bwg_device::select_drive(int n, int state)
{
	if (state == CLEAR_LINE)
	{
		LOGMASKED(LOG_CRU, "Unselect drive DSK%d\n", n);

		// Only when no bit is set, unselect all drives.
		// The DSR actually selects the new drive first, then unselects
		// the old drive.
		if ((m_crulatch0_7->q4_r() == 0) && (m_crulatch0_7->q5_r() == 0)
			&& (m_crulatch0_7->q6_r() == 0) && (m_crulatch8_15->q0_r() == 0))
		{
			m_wd1773->set_floppy(nullptr);
			m_sel_floppy = 0;
		}
	}
	else
	{
		LOGMASKED(LOG_CRU, "Select drive DSK%d\n", n);
		if (m_sel_floppy != 0 && m_sel_floppy != n)
		{
			LOGMASKED(LOG_WARN, "Warning: DSK%d selected while DSK%d not yet unselected\n", n, m_sel_floppy);
		}

		if (m_floppy[n-1]->get_device() != nullptr)
		{
			m_sel_floppy = n;
			m_wd1773->set_floppy(m_floppy[n-1]->get_device());
			m_floppy[n-1]->get_device()->ss_w(m_crulatch0_7->q7_r());
		}
	}
}

void snug_bwg_device::sidsel_w(int state)
{
	// Select side of disk (bit 7)
	if (m_sel_floppy != 0)
	{
		LOGMASKED(LOG_CRU, "Set side (bit 7) = %d on DSK%d\n", state, m_sel_floppy);
		m_floppy[m_sel_floppy-1]->get_device()->ss_w(m_crulatch0_7->q7_r());
	}
}

void snug_bwg_device::dden_w(int state)
{
	/* double density enable (active low) */
	LOGMASKED(LOG_CRU, "Set density (bit 10) = %d (%s)\n", state, (state!=0)? "single" : "double");
	m_wd1773->dden_w(state != 0);
}

/*
    All floppy motors are operated by the same line.
*/
void snug_bwg_device::motorona_w(int state)
{
	m_MOTOR_ON = state;
	LOGMASKED(LOG_MOTOR, "Motor %s\n", state? "on" : "off");

	// The monoflop is connected to the READY line
	m_wd1773->set_force_ready(state==ASSERT_LINE);

	// Set all motors
	for (auto & elem : m_floppy)
		if (elem->get_device() != nullptr) 
			elem->get_device()->mon_w((state==ASSERT_LINE)? 0 : 1);

	// The motor-on line also connects to the wait state logic
	operate_ready_line();
}

void snug_bwg_device::device_start()
{
	m_dsrrom = memregion(TI99_DSRROM)->base();
	m_cru_base = 0x1100;

	save_item(NAME(m_DRQ));
	save_item(NAME(m_IRQ));
	save_item(NAME(m_inDsrArea));
	save_item(NAME(m_WDsel));
	save_item(NAME(m_WDsel0));
	save_item(NAME(m_RTCsel));
	save_item(NAME(m_lastK));
	save_item(NAME(m_dataregLB));
	save_item(NAME(m_MOTOR_ON));
	save_item(NAME(m_address));
}

void snug_bwg_device::device_reset()
{
	m_DRQ = CLEAR_LINE;
	m_IRQ = CLEAR_LINE;
	m_MOTOR_ON = CLEAR_LINE;

	m_wd1773->set_force_ready(false);

	m_selected = false;
	m_dataregLB = false;
	m_lastK = false;
	m_RTCsel = false;
	m_inDsrArea = false;
	m_address = 0;
	m_WDsel = false;
	m_WDsel0 = false;
	
	for (auto &flop : m_floppy)
	{
		if (flop->get_device() != nullptr)
			LOGMASKED(LOG_CONFIG, "Connector %d with %s\n", flop->basetag(), flop->get_device()->name());
		else
			LOGMASKED(LOG_CONFIG, "Connector %d has no floppy attached\n", flop->basetag());
	}

	m_wd1773->set_floppy(nullptr);

	m_dip1 = ioport("BWGDIP1")->read();
	m_dip2 = ioport("BWGDIP2")->read();
	m_dip34 = ioport("BWGDIP34")->read();
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

void snug_bwg_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_TI99_SDF_FORMAT);
	fr.add(FLOPPY_TI99_TDF_FORMAT);
}

static void bwg_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);  // 40 tracks
	device.option_add("525qd", FLOPPY_525_QD);  // 80 tracks
	device.option_add("35dd", FLOPPY_35_DD);    // 80 tracks
}

ROM_START( bwg_fdc )
	ROM_REGION(0x8000, TI99_DSRROM, 0)
	ROM_LOAD("bwg_dsr.u15", 0x0000, 0x8000, CRC(06f1ec89) SHA1(6ad77033ed268f986d9a5439e65f7d391c4b7651)) /* BwG disk DSR ROM */
ROM_END

void snug_bwg_device::device_add_mconfig(machine_config& config)
{
	WD1773(config, m_wd1773, 8_MHz_XTAL);
	m_wd1773->intrq_wr_callback().set(FUNC(snug_bwg_device::fdc_irq_w));
	m_wd1773->drq_wr_callback().set(FUNC(snug_bwg_device::fdc_drq_w));

	MM58274C(config, CLOCK_TAG, 32.768_kHz_XTAL).set_mode_and_day(1, 0); // 24h, sunday

	FLOPPY_CONNECTOR(config, m_floppy[0], bwg_floppies, "525dd", snug_bwg_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], bwg_floppies, "525dd", snug_bwg_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], bwg_floppies, nullptr, snug_bwg_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[3], bwg_floppies, nullptr, snug_bwg_device::floppy_formats).enable_sound(true);

	RAM(config, BUFFER).set_default_size("2K").set_default_value(0);

	HC259(config, m_crulatch0_7); // U13
	m_crulatch0_7->q_out_cb<0>().set(FUNC(snug_bwg_device::den_w));
	m_crulatch0_7->q_out_cb<1>().set(FUNC(snug_bwg_device::mop_w));
	m_crulatch0_7->q_out_cb<2>().set(FUNC(snug_bwg_device::waiten_w));
	m_crulatch0_7->q_out_cb<3>().set(FUNC(snug_bwg_device::hlt_w));
	m_crulatch0_7->q_out_cb<4>().set(FUNC(snug_bwg_device::dsel1_w));
	m_crulatch0_7->q_out_cb<5>().set(FUNC(snug_bwg_device::dsel2_w));
	m_crulatch0_7->q_out_cb<6>().set(FUNC(snug_bwg_device::dsel3_w));
	m_crulatch0_7->q_out_cb<7>().set(FUNC(snug_bwg_device::sidsel_w));

	HC259(config, m_crulatch8_15); // U12
	m_crulatch8_15->q_out_cb<0>().set(FUNC(snug_bwg_device::dsel4_w));
	m_crulatch8_15->q_out_cb<2>().set(FUNC(snug_bwg_device::dden_w));

	// TODO: Replace this by the actual 74HC4538
	TTL74123(config, m_motormf, 0);
	m_motormf->out_cb().set(FUNC(snug_bwg_device::motorona_w));
	m_motormf->set_connection_type(TTL74123_GROUNDED);
	m_motormf->set_resistor_value(RES_K(200));
	m_motormf->set_capacitor_value(CAP_U(47));
	m_motormf->set_clear_pin_value(1);
}

ioport_constructor snug_bwg_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bwg_fdc );
}

const tiny_rom_entry *snug_bwg_device::device_rom_region() const
{
	return ROM_NAME( bwg_fdc );
}

} // end namespace bus::ti99::peb
