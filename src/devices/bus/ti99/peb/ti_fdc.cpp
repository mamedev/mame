// license:LGPL-2.1+
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
#include "ti_fdc.h"
#include "formats/ti99_dsk.h"

DEFINE_DEVICE_TYPE_NS(TI99_FDC, bus::ti99::peb, ti_fdc_device, "ti99_fdc", "TI-99 Standard DSSD Floppy Controller")

namespace bus { namespace ti99 { namespace peb {

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
#define NONE -1

#define TI_FDC_TAG "ti_dssd_controller"

ti_fdc_device::ti_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_FDC, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_address(0),
	m_DRQ(0),
	m_IRQ(0),
	m_crulatch(*this, "crulatch"),
	m_DVENA(0),
	m_inDsrArea(false),
	m_WAITena(false),
	m_WDsel(false),
	m_DSEL(0),
	m_SIDSEL(0),
	m_motor_on_timer(nullptr),
	m_fd1771(*this, FDC_TAG),
	m_dsrrom(nullptr),
	m_current(NONE),
	m_debug_dataout(false)
{
}

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

/*
    Access for debugger. This is a stripped-down version of the
    main methods below. We only allow ROM access.
*/
void ti_fdc_device::debug_read(offs_t offset, uint8_t* value)
{
	if (((offset & m_select_mask)==m_select_value) && m_selected)
	{
		if ((offset & 0x1ff1)!=0x1ff0)
			*value = m_dsrrom[offset & 0x1fff];
	}
}

READ8Z_MEMBER(ti_fdc_device::readz)
{
	if (machine().side_effects_disabled())
	{
		debug_read(offset, value);
		return;
	}

	if (m_inDsrArea && m_selected)
	{
		// Read ports of 1771 are mapped to 5FF0,2,4,6: 0101 1111 1111 0xx0
		// Note that incoming/outgoing data are inverted for FD1771
		uint8_t reply = 0;

		if (m_WDsel && ((m_address & 9)==0))
		{
			if (!machine().side_effects_disabled()) reply = m_fd1771->gen_r((offset >> 1)&0x03);
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
	if (machine().side_effects_disabled()) return;

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
			if (!machine().side_effects_disabled()) m_fd1771->gen_w((offset >> 1)&0x03, data);
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
		uint8_t reply = 0;
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
		m_crulatch->write_bit((offset >> 1) & 0x07, BIT(data, 0));
}

WRITE_LINE_MEMBER(ti_fdc_device::dskpgena_w)
{
	// (De)select the card. Indicated by a LED on the board.
	m_selected = state;
	if (TRACE_CRU) logerror("tifdc: Map DSR (bit 0) = %d\n", m_selected);
}

WRITE_LINE_MEMBER(ti_fdc_device::kaclk_w)
{
	// Activate motor
	if (state)
	{   // On rising edge, set motor_running for 4.23s
		if (TRACE_CRU) logerror("tifdc: trigger motor (bit 1)\n");
		set_floppy_motors_running(true);
	}
}

WRITE_LINE_MEMBER(ti_fdc_device::waiten_w)
{
	// Set disk ready/hold (bit 2)
	// 0: ignore IRQ and DRQ
	// 1: TMS9900 is stopped until IRQ or DRQ are set
	// OR the motor stops rotating - rotates for 4.23s after write
	// to CRU bit 1
	m_WAITena = state;
	if (TRACE_CRU) logerror("tifdc: arm wait state logic (bit 2) = %d\n", state);
}

WRITE_LINE_MEMBER(ti_fdc_device::hlt_w)
{
	// Load disk heads (HLT pin) (bit 3). Not implemented.
	if (TRACE_CRU) logerror("tifdc: set head load (bit 3) = %d\n", state);
}

WRITE_LINE_MEMBER(ti_fdc_device::dsel_w)
{
	m_DSEL = m_crulatch->q4_r() | (m_crulatch->q5_r() << 1) | (m_crulatch->q6_r() << 2);
	set_drive();
}

WRITE_LINE_MEMBER(ti_fdc_device::sidsel_w)
{
	// Select side of disk (bit 7)
	m_SIDSEL = state ? ASSERT_LINE : CLEAR_LINE;
	if (TRACE_CRU) logerror("tifdc: set side (bit 7) = %d\n", state);
	if (m_current != NONE) m_floppy[m_current]->ss_w(state);
}

void ti_fdc_device::set_drive()
{
	switch (m_DSEL)
	{
	case 0:
		m_current = NONE;
		if (TRACE_CRU) logerror("tifdc: all drives deselected\n");
		break;
	case 1:
		m_current = 0;
		break;
	case 2:
		m_current = 1;
		break;
	case 3:
		// The schematics do not reveal any countermeasures against multiple selection
		// so we assume that the highest value wins.
		m_current = 1;
		logerror("tifdc: Warning - multiple drives selected\n");
		break;
	case 4:
		m_current = 2;
		break;
	default:
		m_current = 2;
		logerror("tifdc: Warning - multiple drives selected\n");
		break;
	}
	if (TRACE_CRU) logerror("tifdc: new DSEL = %d\n", m_DSEL);

	m_fd1771->set_floppy((m_current == NONE)? nullptr : m_floppy[m_current]);
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
	for (auto & elem : m_floppy)
		if (elem != nullptr) elem->mon_w((run)? 0 : 1);

	// The motor-on line also connects to the wait state logic
	operate_ready_line();
}

void ti_fdc_device::device_start()
{
	logerror("tifdc: TI FDC start\n");
	m_dsrrom = memregion(TI99_DSRROM)->base();
	m_motor_on_timer = timer_alloc(MOTOR_TIMER);
	m_cru_base = 0x1100;
	// In case we implement a callback after all:
	// m_fd1771->setup_ready_cb(wd_fdc_device::rline_cb(&ti_fdc_device::dvena_r, this));

	save_item(NAME(m_address));
	save_item(NAME(m_DRQ));
	save_item(NAME(m_IRQ));
	save_item(NAME(m_DVENA));
	save_item(NAME(m_inDsrArea));
	save_item(NAME(m_WAITena));
	save_item(NAME(m_WDsel));
	save_item(NAME(m_DSEL));
	save_item(NAME(m_SIDSEL));
	save_item(NAME(m_current));
}

void ti_fdc_device::device_reset()
{
	logerror("tifdc: TI FDC reset\n");
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
		if (m_floppy[i] != nullptr)
			logerror("tifdc: Connector %d with %s\n", i, m_floppy[i]->name());
		else
			logerror("tifdc: No floppy attached to connector %d\n", i);
	}

	m_current = 0;
	m_fd1771->set_floppy(m_floppy[m_current]);
}

void ti_fdc_device::device_config_complete()
{
	// Seems to be null when doing a "-listslots"
	if (subdevice("0")!=nullptr) m_floppy[0] = static_cast<floppy_image_device*>(subdevice("0")->subdevices().first());
	if (subdevice("1")!=nullptr) m_floppy[1] = static_cast<floppy_image_device*>(subdevice("1")->subdevices().first());
	if (subdevice("2")!=nullptr) m_floppy[2] = static_cast<floppy_image_device*>(subdevice("2")->subdevices().first());
}

FLOPPY_FORMATS_MEMBER(ti_fdc_device::floppy_formats)
	FLOPPY_TI99_SDF_FORMAT,
	FLOPPY_TI99_TDF_FORMAT
FLOPPY_FORMATS_END

static void tifdc_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

ROM_START( ti_fdc )
	ROM_REGION(0x2000, TI99_DSRROM, 0)
	ROM_LOAD("fdc_dsr.u26", 0x0000, 0x1000, CRC(693c6b6e) SHA1(0c24fb4944843ad3f08b0b139244a6bb05e1c6c2)) /* TI disk DSR ROM first 4K */
	ROM_LOAD("fdc_dsr.u27", 0x1000, 0x1000, CRC(2c921087) SHA1(3646c3bcd2dce16b918ee01ea65312f36ae811d2)) /* TI disk DSR ROM second 4K */
ROM_END

MACHINE_CONFIG_START(ti_fdc_device::device_add_mconfig)
	MCFG_FD1771_ADD(FDC_TAG, XTAL(1'000'000))
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, ti_fdc_device, fdc_irq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, ti_fdc_device, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("0", tifdc_floppies, "525dd", ti_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("1", tifdc_floppies, "525dd", ti_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("2", tifdc_floppies, nullptr, ti_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_DEVICE_ADD("crulatch", LS259, 0) // U23
	MCFG_ADDRESSABLE_LATCH_Q0_OUT_CB(WRITELINE(*this, ti_fdc_device, dskpgena_w))
	MCFG_ADDRESSABLE_LATCH_Q1_OUT_CB(WRITELINE(*this, ti_fdc_device, kaclk_w))
	MCFG_ADDRESSABLE_LATCH_Q2_OUT_CB(WRITELINE(*this, ti_fdc_device, waiten_w))
	MCFG_ADDRESSABLE_LATCH_Q3_OUT_CB(WRITELINE(*this, ti_fdc_device, hlt_w))
	MCFG_ADDRESSABLE_LATCH_Q4_OUT_CB(WRITELINE(*this, ti_fdc_device, dsel_w))
	MCFG_ADDRESSABLE_LATCH_Q5_OUT_CB(WRITELINE(*this, ti_fdc_device, dsel_w))
	MCFG_ADDRESSABLE_LATCH_Q6_OUT_CB(WRITELINE(*this, ti_fdc_device, dsel_w))
	MCFG_ADDRESSABLE_LATCH_Q7_OUT_CB(WRITELINE(*this, ti_fdc_device, sidsel_w))
MACHINE_CONFIG_END

const tiny_rom_entry *ti_fdc_device::device_rom_region() const
{
	return ROM_NAME( ti_fdc );
}

} } } // end namespace bus::ti99::peb
