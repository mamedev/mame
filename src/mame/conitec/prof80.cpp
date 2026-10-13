// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    PROF-80 (Prozessor RAM-Floppy Kontroller)
    GRIP-1/2/3/4/5 (Grafik-Interface-Prozessor)
    UNIO-1 (?)

	https://www.wolfgangrobel.de/prof80/

*/

#include "emu.h"
#include "bus/ecbbus/ecbbus.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/74259.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/upd1990a.h"
#include "machine/upd765.h"
#include "machine/z80daisy.h"
#include "prof80mmu.h"
#include "softlist_dev.h"

#define Z80_TAG         "z1"
#define UPD765_TAG      "z38"
#define UPD1990A_TAG    "z43"

namespace {

class prof80_state : public driver_device
{
public:
	prof80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_mmu(*this, "mmu"),
		m_rtc(*this, UPD1990A_TAG),
		m_fdc(*this, UPD765_TAG),
		m_ram(*this, RAM_TAG),
		m_floppy(*this, UPD765_TAG":%u", 0U),
		m_ecb(*this, "ecbbus"),
		m_rs232a(*this, "rs232a"),
		m_rs232b(*this, "rs232b"),
		m_flra(*this, "z44"),
		m_flrb(*this, "z45"),
		m_rom(*this, Z80_TAG),
		m_j4(*this, "J4"),
		m_j5(*this, "J5")
	{ }

	void prof80(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<prof80_mmu_device> m_mmu;
	required_device<upd1990a_device> m_rtc;
	required_device<upd765a_device> m_fdc;
	required_device<ram_device> m_ram;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<ecbbus_device> m_ecb;
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_rs232b;
	required_device<ls259_device> m_flra;
	required_device<ls259_device> m_flrb;
	required_memory_region m_rom;
	required_ioport m_j4;
	required_ioport m_j5;

	void flr_w(uint8_t data);
	uint8_t status_r();
	uint8_t status2_r();

	void motor(int state);
	void restore_floppy_ready() { ready_w(m_flra->q3_r()); }
	TIMER_CALLBACK_MEMBER(motor_off_tick) { motor(1); }
	void index_w(int state) { m_index = !state; };

	void ready_w(int state)
	{
		m_fdc->set_ready_line_connected(!state);
		m_fdc->ready_w(!state);
	}
	void inuse_w(int state) { };
	void motor_w(int state) { if (!state) motor(0); };
	void mstop_w(int state) { if (!state) motor(1); };
	void select_w(int state) { m_fdc->set_select_lines_connected(state); };
	void mini_w(int state) { m_fdc->set_rate(state ? 250'000 : 500'000); };

	int m_motor = 1;
	int m_index = 1;

	// timers
	emu_timer *m_motor_off_timer = nullptr;

	void prof80_io(address_map &map) ATTR_COLD;
	void prof80_mem(address_map &map) ATTR_COLD;
	void prof80_mmu(address_map &map) ATTR_COLD;
};


//**************************************************************************
//  PERIPHERALS
//**************************************************************************

//-------------------------------------------------
//  motor -
//-------------------------------------------------

void prof80_state::motor(int state)
{
	for (auto &connector : m_floppy)
	{
		floppy_image_device *drive = connector->get_device();

		if (drive)
		{
			drive->mon_w(state);
			m_motor = state;
		}
	}

	if (!state)
	{
		double const t = 1.1 * RES_M(10) * CAP_U(6.8); // NE555 monostable
		m_motor_off_timer->adjust(attotime::from_double(t));
	} else {
		m_motor_off_timer->adjust(attotime::never);
	}

	m_motor = state;
}


//-------------------------------------------------
//  flr_w - flag register
//-------------------------------------------------

void prof80_state::flr_w(uint8_t data)
{
	/*

	    bit     description

	    0       FB
	    1       SB0
	    2       SB1
	    3       SB2
	    4       SA0
	    5       SA1
	    6       SA2
	    7       FA

	*/

	m_flra->write_bit((data >> 4) & 0x07, BIT(data, 7));
	m_flrb->write_bit((data >> 1) & 0x07, BIT(data, 0));
}


//-------------------------------------------------
//  status_r -
//-------------------------------------------------

uint8_t prof80_state::status_r()
{
	/*

	    bit     signal

	    0       _RX
	    1
	    2
	    3
	    4       CTS
	    5       _INDEX
	    6
	    7       CTSP

	*/

	uint8_t data = 0;

	// serial receive
	data |= m_rs232a->rxd_r();

	// clear to send
	data |= !m_rs232a->cts_r() << 4;
	data |= !m_rs232b->cts_r() << 7;

	// floppy index
	data |= m_index << 5;

	return data;
}


//-------------------------------------------------
//  status2_r -
//-------------------------------------------------

uint8_t prof80_state::status2_r()
{
	/*

	    bit     signal

	    0		MTR
	    1
	    2
	    3
	    4       JS4
	    5       JS5
	    6
	    7       _TDO

	*/

	uint8_t data = 0;

	data |= m_motor;

	int js4 = 0, js5 = 0;

	// JS4
	switch (m_j4->read())
	{
	case 0: js4 = 0; break;
	case 1: js4 = 1; break;
	case 2: js4 = !m_flra->q0_r(); break;
	case 3: js4 = !m_flra->q1_r(); break;
	case 4: js4 = !m_flra->q2_r(); break;
	}

	data |= js4 << 4;

	// JS5
	switch (m_j5->read())
	{
	case 0: js5 = 0; break;
	case 1: js5 = 1; break;
	case 2: js5 = !m_flra->q0_r(); break;
	case 3: js5 = !m_flra->q1_r(); break;
	case 4: js5 = !m_flra->q2_r(); break;
	}

	data |= js5 << 5;

	// RTC data
	data |= !m_rtc->data_out_r() << 7;

	return data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( prof80_mem )
//-------------------------------------------------

void prof80_state::prof80_mem(address_map &map)
{
	map(0x0000, 0xffff).m(m_mmu, FUNC(prof80_mmu_device::z80_program_map));
}


//-------------------------------------------------
//  ADDRESS_MAP( prof80_mmu )
//-------------------------------------------------

void prof80_state::prof80_mmu(address_map &map)
{
	map(0x40000, 0x5ffff).ram();
	map(0xc0000, 0xdffff).ram();
	map(0xf0000, 0xf1fff).mirror(0xe000).rom().region(Z80_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( prof80_io )
//-------------------------------------------------

void prof80_state::prof80_io(address_map &map)
{
	map(0x00, 0xd7).mirror(0xff00).rw(m_ecb, FUNC(ecbbus_device::io_r), FUNC(ecbbus_device::io_w));
	map(0xd8, 0xd8).mirror(0xff00).w(FUNC(prof80_state::flr_w));
	map(0xda, 0xda).mirror(0xff00).r(FUNC(prof80_state::status_r));
	map(0xdb, 0xdb).mirror(0xff00).r(FUNC(prof80_state::status2_r));
	map(0xdc, 0xdd).mirror(0xff00).m(m_fdc, FUNC(upd765a_device::map));
	map(0xde, 0xde).mirror(0x0001).select(0xff00).w(m_mmu, FUNC(prof80_mmu_device::par_w));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( prof80 )
//-------------------------------------------------

static INPUT_PORTS_START( prof80 )
	PORT_START("J1")
	PORT_CONFNAME( 0x01, 0x00, "J1 RDY/HDLD")
	PORT_CONFSETTING( 0x00, "HDLD" )
	PORT_CONFSETTING( 0x01, "READY" )

	PORT_START("J2")
	PORT_CONFNAME( 0x01, 0x01, "J2 RDY/DCHG")
	PORT_CONFSETTING( 0x00, "DCHG" )
	PORT_CONFSETTING( 0x01, "READY" )

	PORT_START("J3")
	PORT_CONFNAME( 0x01, 0x00, "J3 Port Address")
	PORT_CONFSETTING( 0x00, "D8-DF" )
	PORT_CONFSETTING( 0x01, "E8-EF" )

	PORT_START("J4")
	PORT_CONFNAME( 0x07, 0x00, "J4 Console")
	PORT_CONFSETTING( 0x00, "GRIP-1" )
	PORT_CONFSETTING( 0x01, "V24 DUPLEX" )
	PORT_CONFSETTING( 0x02, "USER1" )
	PORT_CONFSETTING( 0x03, "USER2" )
	PORT_CONFSETTING( 0x04, "CP/M" )

	PORT_START("J5")
	PORT_CONFNAME( 0x07, 0x00, "J5 Baud")
	PORT_CONFSETTING( 0x00, "9600" )
	PORT_CONFSETTING( 0x01, "4800" )
	PORT_CONFSETTING( 0x02, "2400" )
	PORT_CONFSETTING( 0x03, "1200" )
	PORT_CONFSETTING( 0x04, "300" )

	PORT_START("J6")
	PORT_CONFNAME( 0x01, 0x01, "J6 Interrupt")
	PORT_CONFSETTING( 0x00, "Serial" )
	PORT_CONFSETTING( 0x01, "ECB" )

	PORT_START("J7")
	PORT_CONFNAME( 0x01, 0x01, "J7 DMA MMU")
	PORT_CONFSETTING( 0x00, "PROF" )
	PORT_CONFSETTING( 0x01, "DMA Card" )

	PORT_START("J8")
	PORT_CONFNAME( 0x01, 0x01, "J8 Active Mode")
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )

	PORT_START("J9")
	PORT_CONFNAME( 0x01, 0x00, "J9 EPROM Type")
	PORT_CONFSETTING( 0x00, "2732/2764" )
	PORT_CONFSETTING( 0x01, "27128" )

	PORT_START("J10")
	PORT_CONFNAME( 0x03, 0x00, "J10 Wait States")
	PORT_CONFSETTING( 0x00, "On all memory accesses" )
	PORT_CONFSETTING( 0x01, "On internal memory accesses" )
	PORT_CONFSETTING( 0x02, DEF_STR( None ) )

	PORT_START("L1")
	PORT_CONFNAME( 0x01, 0x00, "L1 Write Polarity")
	PORT_CONFSETTING( 0x00, "Inverted" )
	PORT_CONFSETTING( 0x01, "Normal" )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_7)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END




//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

static void prof80_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void prof80_state::machine_start()
{
	// initialize RTC
	m_rtc->cs_w(1);
	m_rtc->oe_w(1);

	// create timer
	m_motor_off_timer = timer_alloc(FUNC(prof80_state::motor_off_tick), this);

	// state saving
	save_item(NAME(m_motor));
	save_item(NAME(m_index));
	machine().save().register_postload(save_prepost_delegate(FUNC(prof80_state::restore_floppy_ready), this));
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config
//-------------------------------------------------

void prof80_state::prof80(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(6'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &prof80_state::prof80_mem);
	m_maincpu->set_addrmap(AS_IO, &prof80_state::prof80_io);

	// MMU
	PROF80_MMU(config, m_mmu);
	m_mmu->set_addrmap(AS_PROGRAM, &prof80_state::prof80_mmu);

	// RTC
	UPD1990A(config, m_rtc);

	// FDC
	UPD765A(config, m_fdc, XTAL(16'000'000)/2, true, true);
	m_fdc->idx_wr_callback().set(FUNC(prof80_state::index_w));

	FLOPPY_CONNECTOR(config, UPD765_TAG ":0", prof80_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":1", prof80_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":2", prof80_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":3", prof80_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	// DEMUX latches
	LS259(config, m_flra);
	m_flra->q_out_cb<0>().set(m_rtc, FUNC(upd1990a_device::data_in_w)); // TDI
	m_flra->q_out_cb<0>().append(m_rtc, FUNC(upd1990a_device::c0_w)); // C0
	m_flra->q_out_cb<1>().set(m_rtc, FUNC(upd1990a_device::c1_w)); // C1
	m_flra->q_out_cb<2>().set(m_rtc, FUNC(upd1990a_device::c2_w)); // C2
	m_flra->q_out_cb<3>().set(FUNC(prof80_state::ready_w)); // READY
	m_flra->q_out_cb<4>().set(m_rtc, FUNC(upd1990a_device::clk_w)); // TCK
	m_flra->q_out_cb<5>().set(FUNC(prof80_state::inuse_w)); // IN USE
	m_flra->q_out_cb<6>().set(FUNC(prof80_state::motor_w)); // _MOTOR
	m_flra->q_out_cb<7>().set(FUNC(prof80_state::select_w)); // SELECT
	
	LS259(config, m_flrb);
	m_flrb->q_out_cb<0>().set(m_fdc, FUNC(upd765a_device::reset_w)); // RESF
	m_flrb->q_out_cb<1>().set(FUNC(prof80_state::mini_w)); // MINI
	m_flrb->q_out_cb<2>().set(m_rs232a, FUNC(rs232_port_device::write_rts)); // _RTS
	m_flrb->q_out_cb<3>().set(m_rs232a, FUNC(rs232_port_device::write_txd)); // TX
	m_flrb->q_out_cb<4>().set(FUNC(prof80_state::mstop_w)); // _MSTOP
	m_flrb->q_out_cb<5>().set(m_rs232b, FUNC(rs232_port_device::write_txd)); // TXP
	m_flrb->q_out_cb<6>().set(m_rtc, FUNC(upd1990a_device::stb_w)); // TSTB
	m_flrb->q_out_cb<7>().set(m_mmu, FUNC(prof80_mmu_device::mme_w)); // MME

	// ECB bus
	ECBBUS(config, m_ecb);
	ECBBUS_SLOT(config, "bus1", m_ecb, 1, ecbbus_cards, "grip21");
	ECBBUS_SLOT(config, "bus2", m_ecb, 2, ecbbus_cards, nullptr);
	ECBBUS_SLOT(config, "bus3", m_ecb, 3, ecbbus_cards, nullptr);
	ECBBUS_SLOT(config, "bus4", m_ecb, 4, ecbbus_cards, nullptr);
	ECBBUS_SLOT(config, "bus5", m_ecb, 5, ecbbus_cards, nullptr);
	
	// V24
	RS232_PORT(config, m_rs232a, default_rs232_devices, nullptr);
	m_rs232a->rxd_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0).invert();
	m_rs232a->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	RS232_PORT(config, m_rs232b, default_rs232_devices, nullptr);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("128K");

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("prof80_flop");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( prof80 )
//-------------------------------------------------

ROM_START( prof80 )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v17" )
	ROM_SYSTEM_BIOS( 0, "v15", "v1.5" )
	ROMX_LOAD( "prof80v15.z7", 0x0000, 0x2000, CRC(8f74134c) SHA1(83f9dcdbbe1a2f50006b41d406364f4d580daa1f), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v16", "v1.6" )
	ROMX_LOAD( "prof80v16.z7", 0x0000, 0x2000, CRC(7d3927b3) SHA1(bcc15fd04dbf1d6640115be595255c7b9d2a7281), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v17", "v1.7" )
	ROMX_LOAD( "prof80v17.z7", 0x0000, 0x2000, CRC(53305ff4) SHA1(3ea209093ac5ac8a5db618a47d75b705965cdf44), ROM_BIOS(2) )
ROM_END

} // anonymous namespace



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   STATE         INIT        COMPANY                 FULLNAME   FLAGS
COMP( 1984, prof80,  0,      0,      prof80,  prof80, prof80_state, empty_init, "Conitec Datensysteme", "PROF-80", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
