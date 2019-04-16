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
#include "machine/rescap.h"

#define LOG_WARN        (1U<<1)    // Warnings
#define LOG_CONFIG      (1U<<2)
#define LOG_RW          (1U<<3)
#define LOG_PORTS       (1U<<4)    // too noisy in RW
#define LOG_CRU         (1U<<5)
#define LOG_READY       (1U<<6)
#define LOG_SIGNALS     (1U<<7)
#define LOG_DRQ         (1U<<8)    // too noisy in SIGNALS
#define LOG_DATA        (1U<<9)
#define LOG_MOTOR       (1U<<10)
#define LOG_ADDRESS     (1U<<11)

#define VERBOSE ( LOG_CONFIG | LOG_WARN )
#include "logmacro.h"

DEFINE_DEVICE_TYPE_NS(TI99_FDC, bus::ti99::peb, ti_fdc_device, "ti99_fdc", "TI-99 Standard DSSD Floppy Controller")

namespace bus { namespace ti99 { namespace peb {

// ----------------------------------
#define FDC_TAG "fd1771"
#define MOTOR_TIMER 1

#define TI_FDC_TAG "ti_dssd_controller"

ti_fdc_device::ti_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_FDC, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_address(0),
	m_DRQ(0),
	m_IRQ(0),
	m_HLD(0),
	m_DVENA(0),
	m_inDsrArea(false),
	m_WDsel(false),
	m_fd1771(*this, FDC_TAG),
	m_crulatch(*this, "crulatch"),
	m_motormf(*this, "motormf"),
	m_dsrrom(nullptr),
	m_sel_floppy(0)
{
}

/*
    Operate the wait state logic.
*/
void ti_fdc_device::operate_ready_line()
{
	// This is the wait state logic
	line_state nready = (m_WDsel &&             // Are we accessing 5ffx (even addr)?
			m_crulatch->q2_r() &&               // and the wait state generation is active (SBO 2)
			(m_DRQ==CLEAR_LINE) &&              // and we are waiting for a byte
			(m_IRQ==CLEAR_LINE) &&              // and there is no interrupt yet
			(m_DVENA==ASSERT_LINE)              // and the motor is turning?
			)? ASSERT_LINE : CLEAR_LINE;        // In that case, clear READY and thus trigger wait states
	LOGMASKED(LOG_READY, "Address=%04x, DRQ=%d, INTRQ=%d, MOTOR=%d -> READY=%d\n", m_address & 0xffff, m_DRQ, m_IRQ, m_DVENA, (nready==CLEAR_LINE)? 1:0);
	m_slot->set_ready((nready==CLEAR_LINE)? ASSERT_LINE : CLEAR_LINE);
}

/*
 * Callbacks from the FD1771 chip
 */
WRITE_LINE_MEMBER( ti_fdc_device::fdc_irq_w )
{
	m_IRQ = state? ASSERT_LINE : CLEAR_LINE;
	LOGMASKED(LOG_SIGNALS, "INTRQ callback = %d\n", m_IRQ);
	operate_ready_line();
}

WRITE_LINE_MEMBER( ti_fdc_device::fdc_drq_w )
{
	m_DRQ = state? ASSERT_LINE : CLEAR_LINE;
	LOGMASKED(LOG_DRQ, "DRQ callback = %d\n", m_DRQ);
	operate_ready_line();
}

WRITE_LINE_MEMBER( ti_fdc_device::fdc_hld_w )
{
	m_HLD = state? ASSERT_LINE : CLEAR_LINE;
	LOGMASKED(LOG_SIGNALS, "HLD callback = %d\n", m_HLD);
}

SETADDRESS_DBIN_MEMBER( ti_fdc_device::setaddress_dbin )
{
	// Selection login in the PAL and some circuits on the board

	// Is the card being selected?
	m_address = offset;
	m_inDsrArea = ((m_address & m_select_mask)==m_select_value);

	if (!m_inDsrArea || !m_selected) return;

	LOGMASKED(LOG_ADDRESS, "Set address = %04x\n", offset & 0xffff);

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
			if (!machine().side_effects_disabled()) reply = m_fd1771->read((offset >> 1)&0x03);
			LOGMASKED(LOG_PORTS, "%04x -> %02x\n", offset & 0xffff, reply);
		}
		else
		{
			reply = m_dsrrom[m_address & 0x1fff];
			LOGMASKED(LOG_RW, "%04x -> %02x\n", offset & 0xffff, reply);
		}
		*value = reply;
	}
}

void ti_fdc_device::write(offs_t offset, uint8_t data)
{
	// As this is a memory-mapped access we must prevent the debugger
	// from messing with the operation
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
		if (m_WDsel)
		{
			if ((m_address & 9)==8)
			{
				m_fd1771->write((offset >> 1)&0x03, data);
				LOGMASKED(LOG_PORTS, "%04x <- %02x\n", offset & 0xffff, ~data & 0xff);
			}
			else
			{
				LOGMASKED(LOG_RW, "%04x <- %02x (ignored)\n", m_address & 0xffff, ~data & 0xff);
			}
		}
	}
}

/*
    CRU read access

       7     6     5     4      3     2     1     0
    +-----+-----+-----+------+-----+-----+-----+-----+
    | Side|  1  |  0  |DVENA*| D3C*| D2C*| D1C*| HLD |
    +-----+-----+-----+------+-----+-----+-----+-----+

    See schematics for the meaning of the bits.
*/
READ8Z_MEMBER(ti_fdc_device::crureadz)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		if ((offset & 0x0070) == 0)
		{
			switch ((offset >> 1) & 0x07)
			{
			case 0: *value = (m_HLD==ASSERT_LINE)? 1:0; break;
			case 1: *value = (m_crulatch->q4_r()==ASSERT_LINE && m_DVENA==ASSERT_LINE)? 1:0; break;
			case 2: *value = (m_crulatch->q5_r()==ASSERT_LINE && m_DVENA==ASSERT_LINE)? 1:0; break;
			case 3: *value = (m_crulatch->q6_r()==ASSERT_LINE && m_DVENA==ASSERT_LINE)? 1:0; break;
			case 4: *value = (m_DVENA==CLEAR_LINE)? 1:0; break;
			case 5: *value = 0; break;
			case 6: *value = 1; break;
			case 7: *value = (m_crulatch->q7_r()==ASSERT_LINE)? 1:0; break;
			}
		}
		else *value = 0;
		LOGMASKED(LOG_CRU, "Read CRU %04x = %02x\n", offset, *value);
	}
}

void ti_fdc_device::cruwrite(offs_t offset, uint8_t data)
{
	if ((offset & 0xff00)==m_cru_base)
		m_crulatch->write_bit((offset >> 1) & 0x07, BIT(data, 0));
}

WRITE_LINE_MEMBER(ti_fdc_device::dskpgena_w)
{
	// (De)select the card. Indicated by a LED on the board.
	m_selected = state;
	LOGMASKED(LOG_CRU, "Map DSR (bit 0) = %d\n", m_selected);
}

/*
    Trigger the motor monoflop.
*/
WRITE_LINE_MEMBER(ti_fdc_device::kaclk_w)
{
	m_motormf->b_w(state);
}

WRITE_LINE_MEMBER(ti_fdc_device::dvena_w)
{
	m_DVENA = state;
	LOGMASKED(LOG_MOTOR, "Motor %s\n", state? "on" : "off");

	// The monoflop is connected to the READY line
	m_fd1771->set_force_ready(state==ASSERT_LINE);

	// Set all motors
	for (auto & elem : m_floppy)
		if (elem != nullptr) elem->mon_w((state==ASSERT_LINE)? 0 : 1);

	// The motor-on line also connects to the wait state logic
	operate_ready_line();
}

WRITE_LINE_MEMBER(ti_fdc_device::waiten_w)
{
	// Set disk ready/hold (bit 2)
	// 0: ignore IRQ and DRQ
	// 1: TMS9900 is stopped until IRQ or DRQ are set
	// OR the motor stops rotating - rotates for 4.23s after write
	// to CRU bit 1
	LOGMASKED(LOG_CRU, "Arm wait state logic (bit 2) = %d\n", state);
}

WRITE_LINE_MEMBER(ti_fdc_device::hlt_w)
{
	// Load disk heads (HLT pin) (bit 3). Not implemented.
	LOGMASKED(LOG_CRU, "Set head load (bit 3) = %d\n", state);
}

WRITE_LINE_MEMBER(ti_fdc_device::sidsel_w)
{
	// Select side of disk (bit 7)
	LOGMASKED(LOG_CRU, "Set side (bit 7) = %d\n", state);
	if (m_sel_floppy != 0) m_floppy[m_sel_floppy-1]->ss_w(state);
}

/*
    Drive selects
*/
WRITE_LINE_MEMBER(ti_fdc_device::dsel1_w)
{
	select_drive(1, state);
}

WRITE_LINE_MEMBER(ti_fdc_device::dsel2_w)
{
	select_drive(2, state);
}

WRITE_LINE_MEMBER(ti_fdc_device::dsel3_w)
{
	select_drive(3, state);
}

void ti_fdc_device::select_drive(int n, int state)
{
	if (state == CLEAR_LINE)
	{
		LOGMASKED(LOG_CRU, "Unselect drive DSK%d\n", n);

		// Only when no bit is set, unselect all drives.
		if ((m_crulatch->q4_r() == 0) && (m_crulatch->q5_r() == 0)
			&& (m_crulatch->q6_r() == 0))
		{
			m_fd1771->set_floppy(nullptr);
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

		if (m_floppy[n-1] != nullptr)
		{
			m_sel_floppy = n;
			m_fd1771->set_floppy(m_floppy[n-1]);
			m_floppy[n-1]->ss_w(m_crulatch->q7_r());
		}
	}
}

void ti_fdc_device::device_start()
{
	m_dsrrom = memregion(TI99_DSRROM)->base();
	m_cru_base = 0x1100;

	save_item(NAME(m_address));
	save_item(NAME(m_DRQ));
	save_item(NAME(m_IRQ));
	save_item(NAME(m_DVENA));
	save_item(NAME(m_inDsrArea));
	save_item(NAME(m_WDsel));
	save_item(NAME(m_sel_floppy));
}

void ti_fdc_device::device_reset()
{
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

	m_selected = false;
	m_inDsrArea = false;
	m_WDsel = false;

	for (int i=0; i < 3; i++)
	{
		if (m_floppy[i] != nullptr)
			LOGMASKED(LOG_CONFIG, "Connector %d with %s\n", i, m_floppy[i]->name());
		else
			LOGMASKED(LOG_CONFIG, "No floppy attached to connector %d\n", i);
	}

	m_sel_floppy = 0;
}

void ti_fdc_device::device_config_complete()
{
	// Seems to be null when doing a "-listslots"
	for (auto &elem : m_floppy)
		elem = nullptr;
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

void ti_fdc_device::device_add_mconfig(machine_config& config)
{
	FD1771(config, m_fd1771, 1_MHz_XTAL);
	m_fd1771->intrq_wr_callback().set(FUNC(ti_fdc_device::fdc_irq_w));
	m_fd1771->drq_wr_callback().set(FUNC(ti_fdc_device::fdc_drq_w));
	m_fd1771->hld_wr_callback().set(FUNC(ti_fdc_device::fdc_hld_w));

	FLOPPY_CONNECTOR(config, "0", tifdc_floppies, "525dd", ti_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "1", tifdc_floppies, "525dd", ti_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "2", tifdc_floppies, nullptr, ti_fdc_device::floppy_formats).enable_sound(true);

	LS259(config, m_crulatch); // U23
	m_crulatch->q_out_cb<0>().set(FUNC(ti_fdc_device::dskpgena_w));
	m_crulatch->q_out_cb<1>().set(FUNC(ti_fdc_device::kaclk_w));
	m_crulatch->q_out_cb<2>().set(FUNC(ti_fdc_device::waiten_w));
	m_crulatch->q_out_cb<3>().set(FUNC(ti_fdc_device::hlt_w));
	m_crulatch->q_out_cb<4>().set(FUNC(ti_fdc_device::dsel1_w));
	m_crulatch->q_out_cb<5>().set(FUNC(ti_fdc_device::dsel2_w));
	m_crulatch->q_out_cb<6>().set(FUNC(ti_fdc_device::dsel3_w));
	m_crulatch->q_out_cb<7>().set(FUNC(ti_fdc_device::sidsel_w));

	TTL74123(config, m_motormf, 0);
	m_motormf->out_cb().set(FUNC(ti_fdc_device::dvena_w));
	m_motormf->set_connection_type(TTL74123_GROUNDED);
	m_motormf->set_resistor_value(RES_K(200));
	m_motormf->set_capacitor_value(CAP_U(47));
	m_motormf->set_clear_pin_value(1);
}

const tiny_rom_entry *ti_fdc_device::device_rom_region() const
{
	return ROM_NAME( ti_fdc );
}

} } } // end namespace bus::ti99::peb
