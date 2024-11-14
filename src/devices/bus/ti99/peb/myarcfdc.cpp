// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*******************************************************************************
    Myarc Floppy Disk Controller (DDCC-1)
    Double Density, Double-sided
    May be used with a 1770 or 1772 controller

    EPROM 2764A (8Kx8) in two banks (2x 4000-4fff)
    SRAM HM6116 (2Kx8) (5000-57ff)

    DIP switches (SW1) set the head step rate. For the 1770 controller, closed
    switches mean 20ms, open switches mean 6ms. For the 1772 controller, closed
    means 2ms, open means 6 ms. There is an alternative ROM for the 1772 which
    allows for slower step rates (6ms-12ms).

    Another switch is included (SW2) labeled "Turbo" on schematics, but which
    is not present on all boards. It may require a different ROM.

    Michael Zapf
    March 2020

*******************************************************************************/

#include "emu.h"
#include "myarcfdc.h"
#include "formats/ti99_dsk.h"

// ----------------------------------
// Flags for debugging

#define LOG_WARN        (1U << 1)    // Warnings
#define LOG_CONFIG      (1U << 2)    // Configuration
#define LOG_EPROM       (1U << 3)    // Access to EPROM
#define LOG_CONTR       (1U << 4)    // Access to controller
#define LOG_RAM         (1U << 5)    // Access to SRAM
#define LOG_IRQ         (1U << 6)    // IRQ line
#define LOG_DRQ         (1U << 7)    // DRQ line
#define LOG_DRIVE       (1U << 8)    // Drive operations
#define LOG_CRU         (1U << 9)    // CRU operations

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_CONFIG)
#include "logmacro.h"

#define BUFFER_TAG "ram"
#define LATCH_TAG "ls259"
#define WD1770_TAG "wd1770"
#define WD1772_TAG "wd1772"
#define PAL_TAG "pal"

DEFINE_DEVICE_TYPE(TI99_DDCC1, bus::ti99::peb::myarc_fdc_device, "ti99_ddcc1", "Myarc Disk Controller Card")
DEFINE_DEVICE_TYPE(DDCC1_PAL, bus::ti99::peb::ddcc1_pal_device, PAL_TAG, "Myarc DDCC-1 PAL u1")

namespace bus::ti99::peb {

// ----------------------------------

myarc_fdc_device::myarc_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	  device_t(mconfig, TI99_DDCC1, tag, owner, clock),
	  device_ti99_peribox_card_interface(mconfig, *this),
	  m_wdc(nullptr),
	  m_wd1770(*this, WD1770_TAG),
	  m_wd1772(*this, WD1772_TAG),
	  m_drivelatch(*this, LATCH_TAG),
	  m_buffer_ram(*this, BUFFER_TAG),
	  m_pal(*this, PAL_TAG),
	  m_dsrrom(nullptr),
	  m_floppy(*this, "%u", 0),
	  m_banksel(false),
	  m_cardsel(false),
	  m_selected_drive(0),
	  m_address(0)
	  { }

void myarc_fdc_device::setaddress_dbin(offs_t offset, int state)
{
	// Do not allow setaddress for debugger
	if (machine().side_effects_disabled()) return;
	m_address = offset;
}

bool myarc_fdc_device::card_selected()
{
	return m_cardsel;
}
/*
    Provides the current address to the PAL.
*/
offs_t myarc_fdc_device::get_address()
{
	return m_address;
}

/*
    Debugger access.
*/
void myarc_fdc_device::debug_read(offs_t offset, uint8_t* value)
{
	offs_t addrcopy = m_address;

	if (!m_dec_high || amabc_is_set(offset))
	{
		m_address = offset;
		if (m_pal->ramsel())
		{
			*value = m_buffer_ram->pointer()[m_address & 0x07ff];
		}

		if (m_pal->romen())
		{
			// EPROM selected
			offs_t base = m_banksel? 0x1000 : 0;
			*value = m_dsrrom[base | (m_address & 0x0fff)];
		}
		m_address = addrcopy;
	}
}

/*
    Debugger access.
*/
void myarc_fdc_device::debug_write(offs_t offset, uint8_t data)
{
	offs_t addrcopy = m_address;

	if (!m_dec_high || amabc_is_set(offset))
	{
		m_address = offset;
		if (m_pal->ramsel())
		{
			m_buffer_ram->pointer()[m_address & 0x07ff] = data;
		}
		m_address = addrcopy;
	}
}

/*
    Read access to the RAM, EPROM, and controller chip.
*/
void myarc_fdc_device::readz(offs_t offset, uint8_t *value)
{
	if (machine().side_effects_disabled())
	{
		debug_read(offset, value);
		return;
	}

	// If we have an AMA/B/C decoder and it delivers false, exit here
	if (m_dec_high && !amabc_is_set(offset)) return;

	if (m_pal->ramsel())
	{
		// SRAM selected
		*value = m_buffer_ram->pointer()[m_address & 0x07ff];
		LOGMASKED(LOG_RAM, "Read RAM: %04x -> %02x\n", m_address & 0xffff, *value);
	}

	if (m_pal->romen())
	{
		// EPROM selected
		offs_t base = m_banksel? 0x1000 : 0;
		uint8_t* rom = &m_dsrrom[base | (m_address & 0x0fff)];
		*value = *rom;

		if (WORD_ALIGNED(m_address))
		{
			uint16_t val = (*rom << 8) | (*(rom+1));
			LOGMASKED(LOG_EPROM, "Read DSR: %04x (page %d)-> %04x\n", m_address & 0xffff, base>>12, val);
		}
	}

	if (m_pal->fdcsel())
	{
		// WDC selected
		*value = m_wdc->read((m_address >> 1)&0x03);
		LOGMASKED(LOG_CONTR, "Read FDC: %04x -> %02x\n", m_address & 0xffff, *value);
	}
}

/*
    Write access to RAM and the controller chip.
*/
void myarc_fdc_device::write(offs_t offset, uint8_t data)
{
	if (machine().side_effects_disabled())
	{
		debug_write(offset, data);
		return;
	}

	// If we have an AMA/B/C decoder and it delivers false, exit here
	if (m_dec_high && !amabc_is_set(offset)) return;

	if (m_pal->ramsel())
	{
		// SRAM selected
		LOGMASKED(LOG_RAM, "Write RAM: %04x <- %02x\n", m_address & 0xffff, data);
		m_buffer_ram->pointer()[m_address & 0x07ff] = data;
	}

	if (m_pal->fdcsel())
	{
		// WDC selected
		LOGMASKED(LOG_CONTR, "Write FDC: %04x <- %02x\n", m_address & 0xffff, data);
		m_wdc->write((m_address >> 1)&0x03, data);
	}
}

/*
    CRU read access to the LS251 multiplexer.
*/
void myarc_fdc_device::crureadz(offs_t offset, uint8_t *value)
{
	const uint8_t dipswitch[] = { 0x00, 0x01, 0x02, 0x04, 0x08 };
	offs_t addrcopy = m_address;
	m_address = offset; // Copy the CRU address on the address variable
	if (m_pal->cs251())
	{
		int bitno = ((offset & 0x0e)>>1);
		switch (bitno)
		{
		case 0:  // INTRQ
			*value = (m_wdc->intrq_r()==ASSERT_LINE)? 1:0;
			break;
		case 1:  // DRQ
			*value = (m_wdc->drq_r()==ASSERT_LINE)? 1:0;
			break;
		case 2:  // Turbo (not on every card)
			*value = ioport("SW2")->read();
			break;
		case 3: // NC
			break;
		default:
			// Open = 6ms, close = 20ms (2ms)
			*value = ((ioport("SW1")->read() & dipswitch[8-bitno])!=0)? 0:1;
			break;
		}
		LOGMASKED(LOG_CRU, "cru %04x (bit %d) -> %d\n", offset, bitno, *value);
	}
	m_address = addrcopy;
}

/*
    CRU write access to the LS259 latch.
*/
void myarc_fdc_device::cruwrite(offs_t offset, uint8_t data)
{
	offs_t addrcopy = m_address;
	m_address = offset; // Copy the CRU address on the address variable
	if (m_pal->cs259())
	{
		int bitno = ((offset & 0x0e)>>1);
		LOGMASKED(LOG_CRU, "cru %04x (bit %d) <- %d\n", offset, bitno, data);
		m_drivelatch->write_bit(bitno, BIT(data, 0));
	}
	m_address = addrcopy;
}

/*
    Callbacks from the WDC chip
*/
void myarc_fdc_device::fdc_irq_w(int state)
{
	LOGMASKED(LOG_IRQ, "INTRQ callback = %d\n", state);
}

void myarc_fdc_device::fdc_drq_w(int state)
{
	LOGMASKED(LOG_DRQ, "DRQ callback = %d\n", state);
}

void myarc_fdc_device::fdc_mon_w(int state)
{
	LOGMASKED(LOG_DRIVE, "MON callback = %d\n", state);
	// All MON lines are connected
	// Do not start the motors when no drive is selected. However, motors
	// can always be stopped.
	if (m_selected_drive != 0 || state==1)
		for (auto &flop : m_floppy)
			if (flop->get_device() != nullptr) flop->get_device()->mon_w(state);
}

/*
    Callbacks from the 74LS259 latch
*/
void myarc_fdc_device::den_w(int state)
{
	LOGMASKED(LOG_CRU, "Card enable = %d\n", state);
	m_cardsel = (state==1);
}

void myarc_fdc_device::wdreset_w(int state)
{
	LOGMASKED(LOG_CRU, "Controller reset = %d\n", state);
	m_wdc->mr_w(state);
}

void myarc_fdc_device::sidsel_w(int state)
{
	LOGMASKED(LOG_CRU, "Side select = %d\n", state);
	if (m_selected_drive != 0)
	{
		LOGMASKED(LOG_DRIVE, "Set side = %d on DSK%d\n", state, m_selected_drive);
		m_floppy[m_selected_drive-1]->get_device()->ss_w(state);
	}
}

// Selects the EPROM bank, and also controls the DDEN line
void myarc_fdc_device::bankdden_w(int state)
{
	LOGMASKED(LOG_CRU, "EPROM bank select = %d\n", state);
	m_banksel = (state==ASSERT_LINE);
	m_wdc->dden_w(state==1);
}

void myarc_fdc_device::drivesel_w(int state)
{
	int driveno = 0;

	// We do not know what happens when two drives are selected
	if (m_drivelatch->q7_r() != 0) driveno = 4;
	if (m_drivelatch->q6_r() != 0) driveno = 3;
	if (m_drivelatch->q5_r() != 0) driveno = 2;
	if (m_drivelatch->q4_r() != 0) driveno = 1;

	if (state == CLEAR_LINE)
	{
		// Only when no bit is set, unselect all drives.
		// The DSR actually selects the new drive first, then unselects
		// the old drive.
		if (driveno==0)
		{
			LOGMASKED(LOG_DRIVE, "Unselect all drives\n");
			m_wdc->set_floppy(nullptr);
			m_selected_drive = 0;
		}
	}
	else
	{
		if (m_floppy[driveno-1]->get_device() != nullptr)
		{
			m_selected_drive = driveno;
			LOGMASKED(LOG_DRIVE, "Select drive DSK%d\n", driveno);
			m_wdc->set_floppy(m_floppy[driveno-1]->get_device());
			m_floppy[driveno-1]->get_device()->ss_w(m_drivelatch->q2_r());
		}
	}
}

void myarc_fdc_device::device_start()
{
	m_dsrrom = memregion(TI99_DSRROM)->base();
	save_item(NAME(m_banksel));
	save_item(NAME(m_cardsel));
	save_item(NAME(m_selected_drive));
	save_item(NAME(m_address));
}

void myarc_fdc_device::device_reset()
{
	for (auto &flop : m_floppy)
	{
		if (flop->get_device() != nullptr)
			LOGMASKED(LOG_CONFIG, "Connector %d with %s\n", flop->basetag(), flop->get_device()->name());
		else
			LOGMASKED(LOG_CONFIG, "Connector %d has no floppy attached\n", flop->basetag());
	}

	if (ioport("CONTROLLER")->read()==0)
		m_wdc = m_wd1770;
	else
		m_wdc = m_wd1772;

	m_dec_high = (ioport("AMADECODE")->read()!=0);

	m_pal->set_board(this);
}

void myarc_fdc_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_TI99_SDF_FORMAT);
	fr.add(FLOPPY_TI99_TDF_FORMAT);
}

static void myarc_ddcc_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);  // 40 tracks
	device.option_add("525qd", FLOPPY_525_QD);  // 80 tracks
	device.option_add("35dd", FLOPPY_35_DD);    // 80 tracks
}

INPUT_PORTS_START(myarc_ddcc )
	PORT_START( "CONTROLLER" )
	PORT_CONFNAME( 0x01, 0x00, "Controller chip")
		PORT_CONFSETTING(0x00, "WD1770")
		PORT_CONFSETTING(0x01, "WD1772")

	PORT_START( "SW2" )
	PORT_DIPNAME( 0x01, 0x00, "Turbo switch" )
		PORT_DIPSETTING(0x00, DEF_STR( Off ))
		PORT_DIPSETTING(0x01, DEF_STR( On ))

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x01, 0x00, "DSK1 head step time" )
		PORT_DIPSETTING( 0x00, "6ms")          // 6 ms for 1770
		PORT_DIPSETTING( 0x01, "20ms/2ms")     // 20 ms for 1770 and 2ms for 1772
	PORT_DIPNAME( 0x02, 0x00, "DSK2 head step time" )
		PORT_DIPSETTING( 0x00, "6ms")
		PORT_DIPSETTING( 0x02, "20ms/2ms")
	PORT_DIPNAME( 0x04, 0x00, "DSK3 head step time" )
		PORT_DIPSETTING( 0x00, "6ms")
		PORT_DIPSETTING( 0x04, "20ms/2ms")
	PORT_DIPNAME( 0x08, 0x00, "DSK4 head step time" )
		PORT_DIPSETTING( 0x00, "6ms")
		PORT_DIPSETTING( 0x08, "20ms/2ms")

	PORT_START( "AMADECODE" )
	PORT_CONFNAME( 0x01, 0x01, "Decode AMA/AMB/AMC lines" )
		PORT_CONFSETTING( 0x00, DEF_STR( Off ))
		PORT_CONFSETTING( 0x01, DEF_STR( On ))
INPUT_PORTS_END


ROM_START( myarc_ddcc )
	ROM_REGION(0x2000, TI99_DSRROM, 0)
	ROM_LOAD("ddcc1.u3", 0x0000, 0x2000, CRC(042968a9) SHA1(17af3130d3631b1bc1aa4d7e32a1b1feb66bbfb6))
ROM_END

void myarc_fdc_device::device_add_mconfig(machine_config& config)
{
	// Cards appeared with one of those controllers
	WD1770(config, m_wd1770, 8_MHz_XTAL);
	WD1772(config, m_wd1772, 8_MHz_XTAL);

	m_wd1770->intrq_wr_callback().set(FUNC(myarc_fdc_device::fdc_irq_w));
	m_wd1770->drq_wr_callback().set(FUNC(myarc_fdc_device::fdc_drq_w));
	m_wd1770->mon_wr_callback().set(FUNC(myarc_fdc_device::fdc_mon_w));
	m_wd1770->set_disable_motor_control(true);

	m_wd1772->intrq_wr_callback().set(FUNC(myarc_fdc_device::fdc_irq_w));
	m_wd1772->drq_wr_callback().set(FUNC(myarc_fdc_device::fdc_drq_w));
	m_wd1772->mon_wr_callback().set(FUNC(myarc_fdc_device::fdc_mon_w));
	m_wd1772->set_disable_motor_control(true);

	LS259(config, m_drivelatch); // U10
	m_drivelatch->q_out_cb<0>().set(FUNC(myarc_fdc_device::den_w));
	m_drivelatch->q_out_cb<1>().set(FUNC(myarc_fdc_device::wdreset_w));
	m_drivelatch->q_out_cb<2>().set(FUNC(myarc_fdc_device::sidsel_w));
	m_drivelatch->q_out_cb<3>().set(FUNC(myarc_fdc_device::bankdden_w));
	m_drivelatch->q_out_cb<4>().set(FUNC(myarc_fdc_device::drivesel_w));
	m_drivelatch->q_out_cb<5>().set(FUNC(myarc_fdc_device::drivesel_w));
	m_drivelatch->q_out_cb<6>().set(FUNC(myarc_fdc_device::drivesel_w));
	m_drivelatch->q_out_cb<7>().set(FUNC(myarc_fdc_device::drivesel_w));

	// SRAM 6114 2Kx8
	RAM(config, BUFFER_TAG).set_default_size("2k").set_default_value(0);

	// PAL circuit
	DDCC1_PAL(config, PAL_TAG, 0);

	// Floppy drives
	FLOPPY_CONNECTOR(config, m_floppy[0], myarc_ddcc_floppies, "525dd", myarc_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], myarc_ddcc_floppies, "525dd", myarc_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], myarc_ddcc_floppies, nullptr, myarc_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[3], myarc_ddcc_floppies, nullptr, myarc_fdc_device::floppy_formats).enable_sound(true);
}

ioport_constructor myarc_fdc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( myarc_ddcc );
}

const tiny_rom_entry *myarc_fdc_device::device_rom_region() const
{
	return ROM_NAME( myarc_ddcc );
}

// ========================================================================
//    PAL circuit on the DDCC-1 board
// ========================================================================

ddcc1_pal_device::ddcc1_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	:  device_t(mconfig, DDCC1_PAL, tag, owner, clock),
	   m_board(nullptr)
{
}

bool ddcc1_pal_device::ramsel()
{
	return (((m_board->get_address() & 0xf800)==0x5000) && (m_board->card_selected()));
}

bool ddcc1_pal_device::romen()
{
	return (((m_board->get_address() & 0xf000)==0x4000) && (m_board->card_selected()));
}

bool ddcc1_pal_device::fdcsel()
{
	// The memory mapping of the DDCC-1 differs from the usual scheme, using
	// addresses 5F01, 5F03, 5F05, 5F07
	return (((m_board->get_address() & 0xff01)==0x5f01) && (m_board->card_selected()));
}

bool ddcc1_pal_device::cs251()
{
	return ((m_board->get_address() & 0xff00)==0x1100);
}

bool ddcc1_pal_device::cs259()
{
	return ((m_board->get_address() & 0xff00)==0x1100);
}

} // end namespace bus::ti99::peb
