// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    PROF-80 (Prozessor RAM-Floppy Kontroller)
    GRIP-1/2/3/4/5 (Grafik-Interface-Prozessor)
    UNIO-1 (?)

    http://www.prof80.de/
    http://oldcomputers.dyndns.org/public/pub/rechner/conitec/info.html

*/

/*

    TODO:

    - floppy Err on A: Select
    - NE555 timeout is 10x too high
    - grip31 does not work
    - UNIO card (Z80-STI, Z80-SIO, 2x centronics)
    - GRIP-COLOR (192kB color RAM)
    - GRIP-5 (HD6345, 256KB RAM)
    - XR color card

*/

#include "emu.h"
#include "includes/prof80.h"
#include "softlist.h"


//**************************************************************************
//  PERIPHERALS
//**************************************************************************

//-------------------------------------------------
//  motor -
//-------------------------------------------------

void prof80_state::motor(int mon)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(mon);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(mon);

	m_motor = mon;
}


WRITE_LINE_MEMBER(prof80_state::ready_w)
{
	if (m_ready != state)
	{
		m_fdc->set_ready_line_connected(!state);
		m_fdc->ready_w(!state);
		m_ready = state;
	}
}


WRITE_LINE_MEMBER(prof80_state::inuse_w)
{
	//m_floppy->inuse_w(state);
}


WRITE_LINE_MEMBER(prof80_state::motor_w)
{
	if (state)
	{
		// trigger floppy motor off NE555 timer
		int t = 110 * RES_M(10) * CAP_U(6.8); // t = 1.1 * R8 * C6

		m_floppy_motor_off_timer->adjust(attotime::from_msec(t));
	}
	else
	{
		// turn on floppy motor
		motor(0);

		// reset floppy motor off NE555 timer
		m_floppy_motor_off_timer->adjust(attotime::never);
	}
}


WRITE_LINE_MEMBER(prof80_state::select_w)
{
	if (m_select != state)
	{
		//m_fdc->set_select_lines_connected(state);
		m_select = state;
	}
}


WRITE_LINE_MEMBER(prof80_state::resf_w)
{
	if (state)
		m_fdc->soft_reset();
}


WRITE_LINE_MEMBER(prof80_state::mini_w)
{
}


WRITE_LINE_MEMBER(prof80_state::mstop_w)
{
	if (!state)
	{
		// turn off floppy motor
		motor(1);

		// reset floppy motor off NE555 timer
		m_floppy_motor_off_timer->adjust(attotime::never);
	}
}


//-------------------------------------------------
//  flr_w - flag register
//-------------------------------------------------

WRITE8_MEMBER( prof80_state::flr_w )
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

READ8_MEMBER( prof80_state::status_r )
{
	/*

	    bit     signal      description

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
	data |= !m_rs232a->rxd_r();

	// clear to send
	data |= m_rs232a->cts_r() << 4;
	data |= m_rs232b->cts_r() << 7;

	// floppy index
	data |= (m_floppy[0]->get_device() ? m_floppy[0]->get_device()->idx_r() : m_floppy[1]->get_device() ? m_floppy[1]->get_device()->idx_r() : 1) << 5;

	return data;
}


//-------------------------------------------------
//  status2_r -
//-------------------------------------------------

READ8_MEMBER( prof80_state::status2_r )
{
	/*

	    bit     signal      description

	    0       _MOTOR      floppy motor (0=on, 1=off)
	    1
	    2
	    3
	    4       JS4
	    5       JS5
	    6
	    7       _TDO

	*/

	uint8_t data = 0;
	int js4 = 0, js5 = 0;

	// floppy motor
	data |= m_motor;

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

	data |= js5 << 4;

	// RTC data
	data |= !m_rtc->data_out_r() << 7;

	return data;
}

// UNIO
/*
WRITE8_MEMBER( prof80_state::unio_ctrl_w )
{
//  int flag = BIT(data, 0);
    int flad = (data >> 1) & 0x07;

    switch (flad)
    {
    case 0: // CG1
    case 1: // CG2
    case 2: // _STB1
    case 3: // _STB2
    case 4: // _INIT
    case 5: // JSO0
    case 6: // JSO1
    case 7: // JSO2
        break;
    }
}
*/



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
//  map(0x80, 0x8f).mirror(0xff00).rw(UNIO_Z80STI_TAG, FUNC(z80sti_device::read), FUNC(z80sti_device::write));
//  map(0x94, 0x95).mirror(0xff00).rw(UNIO_Z80SIO_TAG, FUNC(z80sio_device::z80sio_d_r), FUNC(z80sio_device::z80sio_d_w)); // TODO: these methods don't exist anymore
//  map(0x96, 0x97).mirror(0xff00).rw(UNIO_Z80SIO_TAG, FUNC(z80sio_device::z80sio_c_r), FUNC(z80sio_device::z80sio_c_w)); // TODO: these methods don't exist anymore
//  map(0x9e, 0x9e).mirror(0xff00).w(FUNC(prof80_state::unio_ctrl_w));
//  map(0x9c, 0x9c).mirror(0xff00).w(UNIO_CENTRONICS1_TAG, FUNC(centronics_device::write));
//  map(0x9d, 0x9d).mirror(0xff00).w(UNIO_CENTRONICS1_TAG, FUNC(centronics_device::write));
//  map(0xc0, 0xc0).mirror(0xff00).r(FUNC(prof80_state::gripc_r));
//  map(0xc1, 0xc1).mirror(0xff00).rw(FUNC(prof80_state::gripd_r), FUNC(prof80_state::gripd_w));
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
//  device_timer - handler timer events
//-------------------------------------------------

void prof80_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ID_MOTOR:
		motor(1);
		break;
	}
}


//-------------------------------------------------
//  MACHINE_START( prof80 )
//-------------------------------------------------

void prof80_state::machine_start()
{
	// initialize RTC
	m_rtc->cs_w(1);
	m_rtc->oe_w(1);

	// create timer
	m_floppy_motor_off_timer = timer_alloc(TIMER_ID_MOTOR);

	// register for state saving
	save_item(NAME(m_motor));
	save_item(NAME(m_ready));
	save_item(NAME(m_select));
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( prof80 )
//-------------------------------------------------

void prof80_state::prof80(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(6'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &prof80_state::prof80_mem);
	m_maincpu->set_addrmap(AS_IO, &prof80_state::prof80_io);

	// MMU
	PROF80_MMU(config, m_mmu, 0);
	m_mmu->set_addrmap(AS_PROGRAM, &prof80_state::prof80_mmu);

	// RTC
	UPD1990A(config, m_rtc);

	// FDC
	UPD765A(config, m_fdc, 8'000'000, true, true);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":0", prof80_floppies, "525qd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":1", prof80_floppies, "525qd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":2", prof80_floppies, nullptr, floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":3", prof80_floppies, nullptr, floppy_image_device::default_floppy_formats);

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
	m_flrb->q_out_cb<0>().set(FUNC(prof80_state::resf_w)); // RESF
	m_flrb->q_out_cb<1>().set(FUNC(prof80_state::mini_w)); // MINI
	m_flrb->q_out_cb<2>().set(m_rs232a, FUNC(rs232_port_device::write_rts)); // _RTS
	m_flrb->q_out_cb<3>().set(m_rs232a, FUNC(rs232_port_device::write_txd)); // TX
	m_flrb->q_out_cb<4>().set(FUNC(prof80_state::mstop_w)); // _MSTOP
	m_flrb->q_out_cb<5>().set(m_rs232b, FUNC(rs232_port_device::write_txd)); // TXP
	m_flrb->q_out_cb<6>().set(m_rtc, FUNC(upd1990a_device::stb_w)); // TSTB
	m_flrb->q_out_cb<7>().set(m_mmu, FUNC(prof80_mmu_device::mme_w)); // MME

	// ECB bus
	ECBBUS(config, m_ecb);
	ECBBUS_SLOT(config, "ecb_1", m_ecb, 1, ecbbus_cards, "grip21");
	ECBBUS_SLOT(config, "ecb_2", m_ecb, 2, ecbbus_cards, nullptr);
	ECBBUS_SLOT(config, "ecb_3", m_ecb, 3, ecbbus_cards, nullptr);
	ECBBUS_SLOT(config, "ecb_4", m_ecb, 4, ecbbus_cards, nullptr);
	ECBBUS_SLOT(config, "ecb_5", m_ecb, 5, ecbbus_cards, nullptr);

	// V24
	RS232_PORT(config, m_rs232a, default_rs232_devices, nullptr);
	RS232_PORT(config, m_rs232b, default_rs232_devices, nullptr);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("128K");

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("prof80");
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



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   STATE         INIT        COMPANY                 FULLNAME    FLAGS
COMP( 1984, prof80,  0,      0,      prof80,  prof80, prof80_state, empty_init, "Conitec Datensysteme", "PROF-80",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
