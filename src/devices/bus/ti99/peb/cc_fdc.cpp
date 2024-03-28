// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*******************************************************************************
    CorComp Disk Controller
    Based on WD2793/WD1773
    Double Density, Double-sided

    Two flavors:

    * Original controller
      Named "PEB-DCC"
      Single 16K EPROM or two 8K EPROMs (selectable by jumper; only 2x8K emulated)
      Two PALs
      WD2793

    * Modified controller with redesigned PCB
      Named "CorComp FDC Rev A"
      Two 8K EPROMs (by Millers Graphics, 3rd party HW/SW contributor)
      Two PALs
      WD1773

    Michael Zapf
    March 2020

*******************************************************************************/

#include "emu.h"
#include "cc_fdc.h"
#include "formats/ti99_dsk.h"
#include "machine/rescap.h"

// ----------------------------------
// Flags for debugging

#define LOG_WARN        (1U << 1)    // Warnings
#define LOG_CONFIG      (1U << 2)    // Configuration
#define LOG_EPROM       (1U << 3)    // Access to EPROM
#define LOG_CONTR       (1U << 4)    // Access to controller
#define LOG_RAM         (1U << 5)    // Access to SRAM
#define LOG_READY       (1U << 6)    // READY line
#define LOG_SIGNALS     (1U << 7)    // IRQ and HLD lines
#define LOG_DRQ         (1U << 8)    // DRQ line (too noisy in SIGNALS)
#define LOG_DRIVE       (1U << 9)    // Drive operations
#define LOG_CRU         (1U << 10)   // CRU operations

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_CONFIG)
#include "logmacro.h"

#define CCDCC_TAG "ti99_ccdcc"
#define CCFDC_TAG "ti99_ccfdc"
#define WDC_TAG "wdfdc"
#define MOTORMF_TAG "motor_mf"
#define TMS9901_TAG "tms9901"

#define CCDCC_PALU2_TAG "palu2"
#define CCDCC_PALU1_TAG "palu1"
#define CCFDC_PALU12_TAG "palu12"
#define CCFDC_PALU6_TAG "palu6"

#define BUFFER "ram"

DEFINE_DEVICE_TYPE(TI99_CCDCC, bus::ti99::peb::corcomp_dcc_device, CCDCC_TAG, "CorComp Disk Controller Card")
DEFINE_DEVICE_TYPE(TI99_CCFDC, bus::ti99::peb::corcomp_fdca_device, CCFDC_TAG, "CorComp Floppy Disk Controller Card Rev A")

DEFINE_DEVICE_TYPE(CCDCC_PALU2, bus::ti99::peb::ccdcc_palu2_device, CCDCC_PALU2_TAG, "CorComp DCC PAL u2")
DEFINE_DEVICE_TYPE(CCDCC_PALU1, bus::ti99::peb::ccdcc_palu1_device, CCDCC_PALU1_TAG, "CorComp DCC PAL u1")

DEFINE_DEVICE_TYPE(CCFDC_PALU12, bus::ti99::peb::ccfdc_palu12_device, CCFDC_PALU12_TAG, "CorComp FDC PAL u12")
DEFINE_DEVICE_TYPE(CCFDC_PALU6, bus::ti99::peb::ccfdc_palu6_device, CCFDC_PALU6_TAG, "CorComp FDC PAL u6")

namespace bus::ti99::peb {

// ----------------------------------

corcomp_fdc_device::corcomp_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock):
	  device_t(mconfig, type, tag, owner, clock),
	  device_ti99_peribox_card_interface(mconfig, *this),
	  m_wdc(*this, WDC_TAG),
	  m_decpal(nullptr),
	  m_ctrlpal(nullptr),
	  m_motormf(*this, MOTORMF_TAG),
	  m_tms9901(*this, TMS9901_TAG),
	  m_buffer_ram(*this, BUFFER),
	  m_dsrrom(nullptr),
	  m_cardsel(false),
	  m_banksel(false),
	  m_selected_drive(0),
	  m_address(0),
	  m_writing(false)
{
}

void corcomp_fdc_device::setaddress_dbin(offs_t offset, int state)
{
	// Do not allow setaddress for debugger
	if (machine().side_effects_disabled()) return;
	m_address = offset;
	m_writing = (state==CLEAR_LINE);
	operate_ready_line();
}

/*
    Provides the current address to the PALs.
*/
offs_t corcomp_fdc_device::get_address()
{
	return m_address;
}

/*
    Before the 9901 configures the P11 pin as output, it delivers Z output,
    which is pulled down by R10 on the board. We implement this by using a
    variable.
*/
bool corcomp_fdc_device::upper_bank()
{
	return m_banksel;
}

bool corcomp_fdc_device::card_selected()
{
	return m_cardsel;
}

bool corcomp_fdc_device::write_access()
{
	return m_writing;
}

/*
    The Debugging access must not have any side effect on the controller.
    We only allow access to the EPROM and RAM.
*/
void corcomp_fdc_device::debug_read(offs_t offset, uint8_t* value)
{
	offs_t saveaddress = m_address;

	m_address = offset;
	*value = 0x00;

	if (m_ctrlpal->selectram())
	{
		// SRAM selected
		*value = m_buffer_ram->pointer()[m_address & 0x7f] & 0xf0;  // only the first 4 bits
	}

	if (m_ctrlpal->selectdsr())
	{
		// EPROM selected
		offs_t base = m_banksel? 0x2000 : 0;
		uint8_t* rom = &m_dsrrom[base | (m_address & 0x1fff)];
		*value = *rom;
	}
	m_address = saveaddress;
}

void corcomp_fdc_device::debug_write(offs_t offset, uint8_t data)
{
	offs_t saveaddress = m_address;
	m_address = offset;
	if (m_ctrlpal->selectram())
	{
		m_buffer_ram->pointer()[m_address & 0x7f] = data & 0xf0;  // only the first 4 bits
	}
	m_address = saveaddress;
}

/*
    Operate the wait state logic.
*/
void corcomp_fdc_device::operate_ready_line()
{
	line_state ready = (line_state)m_ctrlpal->ready_out();
	m_slot->set_ready(ready);
}

/*
    Callbacks from the WDC chip
*/
void corcomp_fdc_device::fdc_irq_w(int state)
{
	LOGMASKED(LOG_SIGNALS, "INTRQ callback = %d\n", state);
	operate_ready_line();
}

void corcomp_fdc_device::fdc_drq_w(int state)
{
	LOGMASKED(LOG_DRQ, "DRQ callback = %d\n", state);
	operate_ready_line();
}

void corcomp_fdc_device::fdc_hld_w(int state)
{
	LOGMASKED(LOG_SIGNALS, "HLD callback = %d\n", state);
}

void corcomp_fdc_device::readz(offs_t offset, uint8_t *value)
{
	if (machine().side_effects_disabled())
	{
		debug_read(offset, value);
		return;
	}

	if (m_ctrlpal->selectram())
	{
		// SRAM selected
		*value = m_buffer_ram->pointer()[m_address & 0x7f] & 0xf0;  // only the first 4 bits
		LOGMASKED(LOG_RAM, "Read RAM: %04x -> %02x\n", m_address & 0xffff, *value);
	}

	if (m_ctrlpal->selectwdc())
	{
		// WDC selected
		*value = m_wdc->read((m_address >> 1)&0x03);
		LOGMASKED(LOG_CONTR, "Read FDC: %04x -> %02x\n", m_address & 0xffff, *value);
	}

	if (m_ctrlpal->selectdsr())
	{
		// EPROM selected
		offs_t base = m_banksel? 0x2000 : 0;
		uint8_t* rom = &m_dsrrom[base | (m_address & 0x1fff)];
		*value = *rom;

		if (WORD_ALIGNED(m_address))
		{
			uint16_t val = (*rom << 8) | (*(rom+1));
			LOGMASKED(LOG_EPROM, "Read DSR: %04x (page %d)-> %04x\n", m_address & 0xffff, base>>13, val);
		}
	}
}

void corcomp_fdc_device::write(offs_t offset, uint8_t data)
{
	if (machine().side_effects_disabled())
	{
		debug_write(offset, data);
		return;
	}
	if (m_ctrlpal->selectram())
	{
		// SRAM selected
		LOGMASKED(LOG_RAM, "Write RAM: %04x <- %02x\n", m_address & 0xffff, data&0xf0);
		m_buffer_ram->pointer()[m_address & 0x7f] = data & 0xf0;  // only the first 4 bits
	}

	if (m_ctrlpal->selectwdc())
	{
		// WDC selected
		LOGMASKED(LOG_CONTR, "Write FDC: %04x <- %02x\n", m_address & 0xffff, data);
		m_wdc->write((m_address >> 1)&0x03, data);
	}
}

void corcomp_fdc_device::crureadz(offs_t offset, uint8_t *value)
{
	m_address = offset; // Copy the CRU address on the address variable
	if (m_decpal->address9901())
	{
		// The S0 select line is inverted, which means that
		// a CRU address of 0x1120 is actually address 0x00 for the 9901,
		// while the CRU address 0x1100 is 0x20 for the 9901.
		// This trick is necessary to relocate the P0 line to the
		// first CRU address, which by convention turns on the EPROM
		// of the card.
		int bitno = ((offset & 0x3e)^0x20)>>1;
		*value = m_tms9901->read_bit(bitno)? 0x01 : 0x00;
		LOGMASKED(LOG_CRU, "cru %04x (bit %d) -> %d\n", offset, bitno, *value);
	}
}

void corcomp_fdc_device::cruwrite(offs_t offset, uint8_t data)
{
	m_address = offset; // Copy the CRU address on the address variable
	if (m_decpal->address9901())
	{
		int bitno = ((offset & 0x3e)^0x20)>>1;
		LOGMASKED(LOG_CRU, "cru %04x (bit %d) <- %d\n", offset, bitno, data);
		m_tms9901->write_bit(bitno, data!=0);
	}
}

void corcomp_fdc_device::clock_in(int state)
{
	m_tms9901->phi_line(state);
}

uint8_t corcomp_fdc_device::tms9901_input(offs_t offset)
{
	// Inputs
	// INT1: Switch 8
	// INT2: Switch 1
	// INT3: Switch 7
	// INT4: Switch 5
	// INT5: Switch 3
	// INT6: Switch 2
	// INT7: Switch 4
	// INT8: Switch 6
	// INT9: -
	// P9: MotorMF
	// P12: WDCu11.28 (HLD)
	const uint8_t dipswitch[] = { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

	switch (offset)
	{
	case tms9901_device::INT1:
		return ((ioport("HEADSTEP")->read() & dipswitch[8])!=0)? 0:1;
	case tms9901_device::INT2:
		return ((ioport("HEADSTEP")->read() & dipswitch[1])!=0)? 0:1;
	case tms9901_device::INT3:
		return ((ioport("HEADSTEP")->read() & dipswitch[7])!=0)? 0:1;
	case tms9901_device::INT4:
		return ((ioport("HEADSTEP")->read() & dipswitch[5])!=0)? 0:1;
	case tms9901_device::INT5:
		return ((ioport("HEADSTEP")->read() & dipswitch[3])!=0)? 0:1;
	case tms9901_device::INT6:
		return ((ioport("HEADSTEP")->read() & dipswitch[2])!=0)? 0:1;
	case tms9901_device::INT7_P15:
		return ((ioport("HEADSTEP")->read() & dipswitch[4])!=0)? 0:1;
	case tms9901_device::INT8_P14:
		return ((ioport("HEADSTEP")->read() & dipswitch[6])!=0)? 0:1;
	case tms9901_device::INT10_P12:
		return (m_wdc->hld_r()==ASSERT_LINE)? 1:0;
	case tms9901_device::INT13_P9:
		return (m_motormf->q_r()==0)? 1:0;
	default:
		return 1;
	}
}

void corcomp_fdc_device::select_dsk(int state)
{
	if (state == CLEAR_LINE)
	{
		if (   (!m_tms9901->read_bit(tms9901_device::P4))
			&& (!m_tms9901->read_bit(tms9901_device::P5))
			&& (!m_tms9901->read_bit(tms9901_device::P6))
			&& (!m_tms9901->read_bit(tms9901_device::INT14_P8)))
		{
			LOGMASKED(LOG_DRIVE, "Unselect all drives\n");
			m_wdc->set_floppy(nullptr);
			m_selected_drive = 0;
		}
	}
	else
	{
		if (m_tms9901->read_bit(tms9901_device::P4))
		{
			m_selected_drive = 1;
		}
		else if (m_tms9901->read_bit(tms9901_device::P5))
		{
			m_selected_drive = 2;
		}
		else if (m_tms9901->read_bit(tms9901_device::P6))
		{
			m_selected_drive = 3;
		}
		else if (m_tms9901->read_bit(tms9901_device::INT14_P8))
		{
			m_selected_drive = 4;
		}
		LOGMASKED(LOG_DRIVE, "Select drive DSK%d\n", m_selected_drive);

		if (m_floppy[m_selected_drive-1] != nullptr)
		{
			m_wdc->set_floppy(m_floppy[m_selected_drive-1]);
			m_floppy[m_selected_drive-1]->ss_w(m_tms9901->read_bit(tms9901_device::INT15_P7));
		}
	}
}

void corcomp_fdc_device::side_select(int state)
{
	// Select side of disk (bit 7)
	if (m_selected_drive != 0)
	{
		LOGMASKED(LOG_DRIVE, "Set side (bit 7) = %d on DSK%d\n", state, m_selected_drive);
		m_floppy[m_selected_drive-1]->ss_w(state);
	}
}

/*
    All floppy motors are operated by the same line.
*/
void corcomp_fdc_device::motor_w(int state)
{
	LOGMASKED(LOG_DRIVE, "Motor %s\n", state? "on" : "off");
	m_wdc->set_force_ready(state==ASSERT_LINE);

	// Set all motors
	for (auto & elem : m_floppy)
		if (elem != nullptr) elem->mon_w((state==ASSERT_LINE)? 0 : 1);
	operate_ready_line();
}

/*
    Push the P11 state to the variable.
*/
void corcomp_fdc_device::select_bank(int state)
{
	LOGMASKED(LOG_CRU, "Set bank %d\n", state);
	m_banksel = (state==ASSERT_LINE);
	operate_ready_line();
}

void corcomp_fdc_device::select_card(int state)
{
	LOGMASKED(LOG_CRU, "Select card = %d\n", state);
	m_cardsel = (state==ASSERT_LINE);
	operate_ready_line();
}

// =========================================================================

void corcomp_fdc_device::device_start()
{
	m_dsrrom = memregion(TI99_DSRROM)->base();
	save_item(NAME(m_address));
	save_item(NAME(m_writing));
	save_item(NAME(m_cardsel));
	save_item(NAME(m_banksel));
	save_item(NAME(m_selected_drive));
}

void corcomp_fdc_device::device_reset()
{
	for (int i=0; i < 4; i++)
	{
		if (m_floppy[i] != nullptr)
			LOGMASKED(LOG_CONFIG, "Connector %d with %s\n", i, m_floppy[i]->name());
		else
			LOGMASKED(LOG_CONFIG, "Connector %d has no floppy attached\n", i);
	}
}

void corcomp_fdc_device::connect_drives()
{
	for (auto & elem : m_floppy)
		elem = nullptr;

	if (subdevice("0")!=nullptr) m_floppy[0] = static_cast<floppy_image_device*>(subdevice("0")->subdevices().first());
	if (subdevice("1")!=nullptr) m_floppy[1] = static_cast<floppy_image_device*>(subdevice("1")->subdevices().first());
	if (subdevice("2")!=nullptr) m_floppy[2] = static_cast<floppy_image_device*>(subdevice("2")->subdevices().first());
	if (subdevice("3")!=nullptr) m_floppy[3] = static_cast<floppy_image_device*>(subdevice("3")->subdevices().first());
}

INPUT_PORTS_START( cc_fdc )
	PORT_START( "HEADSTEP" )
	PORT_DIPNAME( 0x03, 0x02, "DSK1 head step time" )
		PORT_DIPSETTING( 0x00, "15 ms")
		PORT_DIPSETTING( 0x01, "10 ms")
		PORT_DIPSETTING( 0x02, "6 ms")
		PORT_DIPSETTING( 0x03, "3 ms")
	PORT_DIPNAME( 0x0c, 0x08, "DSK2 head step time" )
		PORT_DIPSETTING( 0x00, "15 ms")
		PORT_DIPSETTING( 0x04, "10 ms")
		PORT_DIPSETTING( 0x08, "6 ms")
		PORT_DIPSETTING( 0x0c, "3 ms")
	PORT_DIPNAME( 0x30, 0x20, "DSK3 head step time" )
		PORT_DIPSETTING( 0x00, "15 ms")
		PORT_DIPSETTING( 0x10, "10 ms")
		PORT_DIPSETTING( 0x20, "6 ms")
		PORT_DIPSETTING( 0x30, "3 ms")
	PORT_DIPNAME( 0xc0, 0x80, "DSK4 head step time" )
		PORT_DIPSETTING( 0x00, "15 ms")
		PORT_DIPSETTING( 0x40, "10 ms")
		PORT_DIPSETTING( 0x80, "6 ms")
		PORT_DIPSETTING( 0xc0, "3 ms")
INPUT_PORTS_END

void corcomp_fdc_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_TI99_SDF_FORMAT);
	fr.add(FLOPPY_TI99_TDF_FORMAT);
}

static void ccfdc_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);  // 40 tracks
	device.option_add("525qd", FLOPPY_525_QD);  // 80 tracks
	device.option_add("35dd", FLOPPY_35_DD);    // 80 tracks
}

void corcomp_fdc_device::common_config(machine_config& config)
{
	m_wdc->intrq_wr_callback().set(FUNC(corcomp_fdc_device::fdc_irq_w));
	m_wdc->drq_wr_callback().set(FUNC(corcomp_fdc_device::fdc_drq_w));
	m_wdc->hld_wr_callback().set(FUNC(corcomp_fdc_device::fdc_hld_w));

	TMS9901(config, m_tms9901, 0);
	m_tms9901->read_cb().set(FUNC(corcomp_fdc_device::tms9901_input));

	// Outputs
	// P0: LED (DSR?), PALu2.1
	// P1: MFu6.clk (Motor)
	// P2: PALu1.5 (WATEN)
	// P3: WDCu11.50 (HLT)
	// P4: DSK1 select
	// P5: DSK2 select
	// P6: DSK3 select
	// P7: SIDSEL
	// P8: DSK4 select
	// P10: WDCu11.37 (DDEN)
	// P11: ROMBNK
	// P13: -
	// P14: -
	// P15: -
	m_tms9901->p_out_cb(0).set(FUNC(corcomp_fdc_device::select_card));
	m_tms9901->p_out_cb(1).set(MOTORMF_TAG, FUNC(ttl74123_device::b_w));
	m_tms9901->p_out_cb(4).set(FUNC(corcomp_fdc_device::select_dsk));
	m_tms9901->p_out_cb(5).set(FUNC(corcomp_fdc_device::select_dsk));
	m_tms9901->p_out_cb(6).set(FUNC(corcomp_fdc_device::select_dsk));
	m_tms9901->p_out_cb(7).set(FUNC(corcomp_fdc_device::side_select));
	m_tms9901->p_out_cb(8).set(FUNC(corcomp_fdc_device::select_dsk));
	m_tms9901->p_out_cb(10).set(WDC_TAG, FUNC(wd_fdc_device_base::dden_w));
	m_tms9901->p_out_cb(11).set(FUNC(corcomp_fdc_device::select_bank));

	// Motor monoflop
	TTL74123(config, m_motormf, 0);
	m_motormf->set_connection_type(TTL74123_GROUNDED);
	m_motormf->set_resistor_value(RES_K(100));
	m_motormf->set_capacitor_value(CAP_U(47));
	m_motormf->set_a_pin_value(0);
	m_motormf->set_b_pin_value(1);
	m_motormf->set_clear_pin_value(1);
	m_motormf->out_cb().set(FUNC(corcomp_fdc_device::motor_w));

	FLOPPY_CONNECTOR(config, "0", ccfdc_floppies, "525dd", corcomp_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "1", ccfdc_floppies, "525dd", corcomp_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "2", ccfdc_floppies, nullptr, corcomp_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "3", ccfdc_floppies, nullptr, corcomp_fdc_device::floppy_formats).enable_sound(true);

	// SRAM 2114 1Kx4
	RAM(config, BUFFER).set_default_size("1k").set_default_value(0);
}

// ============================================================================
// Original CorComp Disk Controller Card (PEB-DCC)
// ============================================================================

corcomp_dcc_device::corcomp_dcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	  corcomp_fdc_device(mconfig, TI99_CCDCC, tag, owner, clock)
{
}

void corcomp_dcc_device::device_add_mconfig(machine_config& config)
{
	WD2793(config, m_wdc, 4_MHz_XTAL / 4);
	common_config(config);

	// For the 2793, attach the HLT line
	m_tms9901->p_out_cb(3).set(WDC_TAG, FUNC(wd_fdc_device_base::hlt_w));

	// PAL circuits are connected in device_config_complete
	CCDCC_PALU2(config, CCDCC_PALU2_TAG, 0);
	CCDCC_PALU1(config, CCDCC_PALU1_TAG, 0);
}

ROM_START( cc_dcc )
	ROM_REGION(0x4000, TI99_DSRROM, 0)
	ROM_LOAD("ccdcc_v89.u3", 0x0000, 0x2000, CRC(de3f9476) SHA1(b88aea1141769dad4e4bea5f93ac4f63a627cc82)) /* 8K single ROM bank 1*/
	ROM_LOAD("ccdcc_v89.u4", 0x2000, 0x2000, CRC(9c4e5c08) SHA1(26f8096ae60f3839902b4e8764c5fde283ad4ba2)) /* 8K single ROM bank 2*/
ROM_END

void corcomp_dcc_device::device_config_complete()
{
	m_decpal = dynamic_cast<ccfdc_dec_pal_device*>(subdevice(CCDCC_PALU2_TAG));
	m_ctrlpal = dynamic_cast<ccfdc_sel_pal_device*>(subdevice(CCDCC_PALU1_TAG));
	if (m_decpal != nullptr) 
		connect_drives();
}

ioport_constructor corcomp_fdc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cc_fdc );
}

const tiny_rom_entry *corcomp_dcc_device::device_rom_region() const
{
	return ROM_NAME( cc_dcc );
}

// ========================================================================
//    PAL circuits on the CorComp board
// ========================================================================

ccfdc_dec_pal_device::ccfdc_dec_pal_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	:  device_t(mconfig, type, tag, owner, clock),
	   m_board(nullptr),
	   m_tms9901(*owner, TMS9901_TAG)
{
}

ccfdc_sel_pal_device::ccfdc_sel_pal_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	:  device_t(mconfig, type, tag, owner, clock),
	   m_board(nullptr),
	   m_decpal(nullptr),
	   m_motormf(*owner, MOTORMF_TAG),
	   m_tms9901(*owner, TMS9901_TAG),
	   m_wdc(*owner, WDC_TAG)
{
}

void ccfdc_dec_pal_device::device_config_complete()
{
	m_board = dynamic_cast<corcomp_fdc_device*>(owner());
	// owner is the empty_state during -listxml, so this will be nullptr
}

/*
    Indicates 9901 addressing.
*/
int ccfdc_dec_pal_device::address9901()
{
	return ((m_board->get_address() & 0xff80)==0x1100)? ASSERT_LINE : CLEAR_LINE;
}

/*
    Indicates SRAM addressing.
*/
int ccfdc_dec_pal_device::addressram()
{
	return ((m_board->card_selected()) &&
		(m_board->get_address() & 0xff80)==0x4000)? ASSERT_LINE : CLEAR_LINE;
}

/*
    Indicates WDC addressing.
*/
int ccfdc_dec_pal_device::addresswdc()
{
	return ((m_board->card_selected()) &&
		(m_board->get_address() & 0xff80)==0x5f80)? ASSERT_LINE : CLEAR_LINE;
}

/*
    Indicates DSR addressing.
*/
int ccfdc_dec_pal_device::address4()
{
	return ((m_board->card_selected()) &&
		(m_board->get_address() & 0xe000)==0x4000)? ASSERT_LINE : CLEAR_LINE;
}

/*
    Indicates SRAM selection.
*/
int ccfdc_sel_pal_device::selectram()
{
	return (m_decpal->addressram() && (m_board->upper_bank()))
		? ASSERT_LINE : CLEAR_LINE;
}

/*
    Indicates WDC selection.
*/
int ccfdc_sel_pal_device::selectwdc()
{
	return (m_decpal->addresswdc() && ((m_board->get_address()&1)==0))? ASSERT_LINE : CLEAR_LINE;
}

/*
    Indicates EPROM selection.
*/
int ccfdc_sel_pal_device::selectdsr()
{
	return (m_decpal->address4()
		&& !m_decpal->addresswdc()
		&& !(m_decpal->addressram() && (m_board->upper_bank())))
		? ASSERT_LINE : CLEAR_LINE;
}

// ========================================================================
//    PAL circuits on the original CorComp board
//    PAL u2 is the address decoder, delivering its results to the
//    selector PAL u1.
// ========================================================================

ccdcc_palu2_device::ccdcc_palu2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ccfdc_dec_pal_device(mconfig, CCDCC_PALU2, tag, owner, clock)
{
}

ccdcc_palu1_device::ccdcc_palu1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ccfdc_sel_pal_device(mconfig, CCDCC_PALU1, tag, owner, clock)
{
}

/*
    Wait state logic
*/
int ccdcc_palu1_device::ready_out()
{
	bool wdc = m_decpal->addresswdc();                         // Addressing the WDC
	bool lastdig = (m_board->get_address()&7)==6;              // Address ends with 6 or e (5ff6, 5ffe)
	bool trap = m_tms9901->read_bit(tms9901_device::P2);       // Wait state generation is active (SBO 2)
	bool waitbyte = m_wdc->drq_r()==CLEAR_LINE;                // We are waiting for a byte
	bool noterm = m_wdc->intrq_r()==CLEAR_LINE;                // There is no interrupt yet
	bool motor = (m_motormf->q_r()==1);                        // The disk is spinning

	line_state ready = (wdc && lastdig && trap && waitbyte && noterm && motor)? CLEAR_LINE : ASSERT_LINE; // then clear READY and thus trigger wait states

	LOGMASKED(LOG_READY, "READY = %d (%d,%d,%d,%d,%d,%d)\n", ready, wdc, lastdig, trap, waitbyte, noterm, motor);
	return ready;
}

void ccdcc_palu1_device::device_config_complete()
{
	m_board = dynamic_cast<corcomp_fdc_device*>(owner());
	m_decpal = dynamic_cast<ccfdc_dec_pal_device*>(owner()->subdevice(CCDCC_PALU2_TAG));
	// owner is the empty_state during -listxml, so this will be nullptr
}

// ============================================================================
// Revised CorComp floppy disk controller card REV A
// ============================================================================

corcomp_fdca_device::corcomp_fdca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	  corcomp_fdc_device(mconfig, TI99_CCFDC, tag, owner, clock)
{
}

void corcomp_fdca_device::device_add_mconfig(machine_config& config)
{
	WD1773(config, m_wdc, 8_MHz_XTAL);
	common_config(config);

	// PAL circuits are connected in device_config_complete
	CCFDC_PALU12(config, CCFDC_PALU12_TAG, 0);
	CCFDC_PALU6(config, CCFDC_PALU6_TAG, 0);
}

/*
    READY trap circuitry on the revised board (U10)
*/
bool corcomp_fdca_device::ready_trap_active()
{
	return m_tms9901->read_bit(tms9901_device::P2)
		&& ((m_address & 6)==6)
		&& (m_motormf->q_r()==1);
}

ROM_START( cc_fdcmg )
	ROM_REGION(0x8000, TI99_DSRROM, 0)
	ROM_LOAD("ccfdc_v89mg.u1", 0x0000, 0x2000, CRC(f010e273) SHA1(bd30103d80c43d4b35e0669145cef7b5c6b9813b)) /* 16K single ROM */
	ROM_LOAD("ccfdc_v89mg.u2", 0x2000, 0x2000, CRC(0cad8f5b) SHA1(7744f777b51eedf614f766576bbc3f8c2c2e0042)) /* 16K single ROM */
ROM_END

void corcomp_fdca_device::device_config_complete()
{
	m_decpal = static_cast<ccfdc_dec_pal_device*>(subdevice(CCFDC_PALU12_TAG));
	m_ctrlpal = static_cast<ccfdc_sel_pal_device*>(subdevice(CCFDC_PALU6_TAG));
	connect_drives();
}

const tiny_rom_entry *corcomp_fdca_device::device_rom_region() const
{
	return ROM_NAME( cc_fdcmg );
}

// ========================================================================
//    PAL circuits on the revised CorComp board
//    PAL u12 is the address decoder, delivering its results to the
//    selector PAL u6.
// ========================================================================

ccfdc_palu12_device::ccfdc_palu12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ccfdc_dec_pal_device(mconfig, CCFDC_PALU12, tag, owner, clock)
{
}

/*
    Indicates 9901 addressing. In this PAL version, the A9 address line is
    also used.
*/
int ccfdc_palu12_device::address9901()
{
	return ((m_board->get_address() & 0xffc0)==0x1100)? ASSERT_LINE : CLEAR_LINE;
}

ccfdc_palu6_device::ccfdc_palu6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ccfdc_sel_pal_device(mconfig, CCFDC_PALU6, tag, owner, clock)
{
}

/*
    Indicates WDC selection. Also checks whether A12 and /WE match.
    That is, when writing (/WE=0), A12 must be 1 (addresses 5ff8..e),
    otherwise (/WE=1), A12 must be 0 (addresses 5ff0..6)
*/
int ccfdc_palu6_device::selectwdc()
{
	return (m_decpal->addresswdc()
		&& ((m_board->get_address()&1)==0)
		&& (((m_board->get_address()&8)!=0)==(m_board->write_access())))? ASSERT_LINE : CLEAR_LINE;
}

/*
    Indicates EPROM selection. The Rev A selector PAL leads back some of
    its outputs for this calculation.
*/
int ccfdc_palu6_device::selectdsr()
{
	return (m_decpal->address4() && !selectwdc() && !selectram())? ASSERT_LINE : CLEAR_LINE;
}

/*
    Wait state logic. The Rev A selector relies on an AND circuit on the
    board which evaluates whether the trap is active.
*/

int ccfdc_palu6_device::ready_out()
{
	bool wdc = m_decpal->addresswdc();                   // Addressing the WDC
	bool even = (m_board->get_address()&1)==0;          // A15 = 0
	bool trap = static_cast<corcomp_fdca_device*>(m_board)->ready_trap_active();   // READY trap active
	bool waitbyte = m_wdc->drq_r()==CLEAR_LINE;          // We are waiting for a byte
	bool noterm = m_wdc->intrq_r()==CLEAR_LINE;          // There is no interrupt yet

	line_state ready = (wdc && even && trap && waitbyte && noterm)? CLEAR_LINE : ASSERT_LINE; // then clear READY and thus trigger wait states

	LOGMASKED(LOG_READY, "READY = %d (%d,%d,%d,%d,%d)\n", ready, wdc, even, trap, waitbyte, noterm);
	return ready;
}

void ccfdc_palu6_device::device_config_complete()
{
	m_board = dynamic_cast<corcomp_fdca_device*>(owner());
	m_decpal = dynamic_cast<ccfdc_dec_pal_device*>(owner()->subdevice(CCFDC_PALU12_TAG));
}

} // end namespace bus::ti99::peb
