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
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(mon);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(mon);

	m_motor = mon;
}


//-------------------------------------------------
//  ls259_w -
//-------------------------------------------------

void prof80_state::ls259_w(int fa, int sa, int fb, int sb)
{
	switch (sa)
	{
	case 0: // C0/TDI
		m_rtc->data_in_w(fa);
		m_rtc->c0_w(fa);
		m_c0 = fa;
		break;

	case 1: // C1
		m_rtc->c1_w(fa);
		m_c1 = fa;
		break;

	case 2: // C2
		m_rtc->c2_w(fa);
		m_c2 = fa;
		break;

	case 3: // READY
		if (m_ready != fa)
		{
			m_fdc->set_ready_line_connected(!fa);
			m_fdc->ready_w(!fa);
			m_ready = fa;
		}
		break;

	case 4: // TCK
		m_rtc->clk_w(fa);
		break;

	case 5: // IN USE
		//m_floppy->inuse_w(fa);
		break;

	case 6: // _MOTOR
		if (fa)
		{
			// trigger floppy motor off NE555 timer
			int t = 110 * RES_M(10) * CAP_U(6.8); // t = 1.1 * R8 * C6

			timer_set(attotime::from_msec(t), TIMER_ID_MOTOR);
		}
		else
		{
			// turn on floppy motor
			motor(0);

			// reset floppy motor off NE555 timer
			timer_set(attotime::never, TIMER_ID_MOTOR);
		}
		break;

	case 7: // SELECT
		if (m_select != fa)
		{
			//m_fdc->set_select_lines_connected(fa);
			m_select = fa;
		}
		break;
	}

	switch (sb)
	{
	case 0: // RESF
		if (fb) m_fdc->soft_reset();
		break;

	case 1: // MINI
		break;

	case 2: // _RTS
		m_rs232a->write_rts(fb);
		break;

	case 3: // TX
		m_rs232a->write_txd(fb);
		break;

	case 4: // _MSTOP
		if (!fb)
		{
			// turn off floppy motor
			motor(1);

			// reset floppy motor off NE555 timer
			timer_set(attotime::never, TIMER_ID_MOTOR);
		}
		break;

	case 5: // TXP
		m_rs232b->write_txd(fb);
		break;

	case 6: // TSTB
		m_rtc->stb_w(fb);
		break;

	case 7: // MME
		m_mmu->mme_w(fb);
		break;
	}
}


//-------------------------------------------------
//  flr_w -
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

	int fa = BIT(data, 7);
	int sa = (data >> 4) & 0x07;

	int fb = BIT(data, 0);
	int sb = (data >> 1) & 0x07;

	ls259_w(fa, sa, fb, sb);
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

	UINT8 data = 0;

	// serial receive
	data |= !m_rs232a->rxd_r();

	// clear to send
	data |= m_rs232a->cts_r() << 4;
	data |= m_rs232b->cts_r() << 7;

	// floppy index
	data |= (m_floppy0->get_device() ? m_floppy0->get_device()->idx_r() : m_floppy1->get_device() ? m_floppy1->get_device()->idx_r() : 1) << 5;

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

	UINT8 data = 0;
	int js4 = 0, js5 = 0;

	// floppy motor
	data |= m_motor;

	// JS4
	switch (m_j4->read())
	{
	case 0: js4 = 0; break;
	case 1: js4 = 1; break;
	case 2: js4 = !m_c0; break;
	case 3: js4 = !m_c1; break;
	case 4: js4 = !m_c2; break;
	}

	data |= js4 << 4;

	// JS5
	switch (m_j5->read())
	{
	case 0: js5 = 0; break;
	case 1: js5 = 1; break;
	case 2: js5 = !m_c0; break;
	case 3: js5 = !m_c1; break;
	case 4: js5 = !m_c2; break;
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

static ADDRESS_MAP_START( prof80_mem, AS_PROGRAM, 8, prof80_state )
	AM_RANGE(0x0000, 0xffff) AM_DEVICE(MMU_TAG, prof80_mmu_device, z80_program_map)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( prof80_mmu )
//-------------------------------------------------

static ADDRESS_MAP_START( prof80_mmu, AS_PROGRAM, 8, prof80_state )
	AM_RANGE(0x40000, 0x5ffff) AM_RAM
	AM_RANGE(0xc0000, 0xdffff) AM_RAM
	AM_RANGE(0xf0000, 0xf1fff) AM_MIRROR(0xe000) AM_ROM AM_REGION(Z80_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( prof80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( prof80_io, AS_IO, 8, prof80_state )
	AM_RANGE(0x00, 0xd7) AM_MIRROR(0xff00) AM_DEVREADWRITE(ECBBUS_TAG, ecbbus_device, io_r, io_w)
//  AM_RANGE(0x80, 0x8f) AM_MIRROR(0xff00) AM_DEVREADWRITE(UNIO_Z80STI_TAG, z80sti_device, read, write)
//  AM_RANGE(0x94, 0x95) AM_MIRROR(0xff00) AM_DEVREADWRITE_LEGACY(UNIO_Z80SIO_TAG, z80sio_d_r, z80sio_d_w)
//  AM_RANGE(0x96, 0x97) AM_MIRROR(0xff00) AM_DEVREADWRITE_LEGACY(UNIO_Z80SIO_TAG, z80sio_c_r, z80sio_c_w)
//  AM_RANGE(0x9e, 0x9e) AM_MIRROR(0xff00) AM_WRITE(unio_ctrl_w)
//  AM_RANGE(0x9c, 0x9c) AM_MIRROR(0xff00) AM_DEVWRITE(UNIO_CENTRONICS1_TAG, centronics_device, write)
//  AM_RANGE(0x9d, 0x9d) AM_MIRROR(0xff00) AM_DEVWRITE(UNIO_CENTRONICS1_TAG, centronics_device, write)
//  AM_RANGE(0xc0, 0xc0) AM_MIRROR(0xff00) AM_READ(gripc_r)
//  AM_RANGE(0xc1, 0xc1) AM_MIRROR(0xff00) AM_READWRITE(gripd_r, gripd_w)
	AM_RANGE(0xd8, 0xd8) AM_MIRROR(0xff00) AM_WRITE(flr_w)
	AM_RANGE(0xda, 0xda) AM_MIRROR(0xff00) AM_READ(status_r)
	AM_RANGE(0xdb, 0xdb) AM_MIRROR(0xff00) AM_READ(status2_r)
	AM_RANGE(0xdc, 0xdd) AM_MIRROR(0xff00) AM_DEVICE(UPD765_TAG, upd765a_device, map)
	AM_RANGE(0xde, 0xde) AM_MIRROR(0xff01) AM_MASK(0xff00) AM_DEVWRITE(MMU_TAG, prof80_mmu_device, par_w)
ADDRESS_MAP_END



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

static SLOT_INTERFACE_START( prof80_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END



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

	// register for state saving
	save_item(NAME(m_c0));
	save_item(NAME(m_c1));
	save_item(NAME(m_c2));
	save_item(NAME(m_motor));
	save_item(NAME(m_ready));
	save_item(NAME(m_select));
}


void prof80_state::machine_reset()
{
	int i;

	for (i = 0; i < 8; i++)
	{
		ls259_w(0, i, 0, i);
	}
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( prof80 )
//-------------------------------------------------

static MACHINE_CONFIG_START( prof80, prof80_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP(prof80_mem)
	MCFG_CPU_IO_MAP(prof80_io)

	// devices
	MCFG_PROF80_MMU_ADD(MMU_TAG, prof80_mmu)
	MCFG_UPD1990A_ADD(UPD1990A_TAG, XTAL_32_768kHz, NULL, NULL)
	MCFG_UPD765A_ADD(UPD765_TAG, true, true)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", prof80_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", prof80_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":2", prof80_floppies, NULL,    floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":3", prof80_floppies, NULL,    floppy_image_device::default_floppy_formats)

	// ECB bus
	MCFG_ECBBUS_ADD()
	MCFG_ECBBUS_SLOT_ADD(1, "ecb_1", ecbbus_cards, "grip21")
	MCFG_ECBBUS_SLOT_ADD(2, "ecb_2", ecbbus_cards, NULL)
	MCFG_ECBBUS_SLOT_ADD(3, "ecb_3", ecbbus_cards, NULL)
	MCFG_ECBBUS_SLOT_ADD(4, "ecb_4", ecbbus_cards, NULL)
	MCFG_ECBBUS_SLOT_ADD(5, "ecb_5", ecbbus_cards, NULL)

	// V24
	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, NULL)
	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, NULL)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "prof80")
MACHINE_CONFIG_END



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
	ROMX_LOAD( "prof80v15.z7", 0x0000, 0x2000, CRC(8f74134c) SHA1(83f9dcdbbe1a2f50006b41d406364f4d580daa1f), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v16", "v1.6" )
	ROMX_LOAD( "prof80v16.z7", 0x0000, 0x2000, CRC(7d3927b3) SHA1(bcc15fd04dbf1d6640115be595255c7b9d2a7281), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "v17", "v1.7" )
	ROMX_LOAD( "prof80v17.z7", 0x0000, 0x2000, CRC(53305ff4) SHA1(3ea209093ac5ac8a5db618a47d75b705965cdf44), ROM_BIOS(3) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    INIT    COMPANY                          FULLNAME        FLAGS
COMP( 1984, prof80,     0,      0,      prof80, prof80, driver_device,  0,      "Conitec Datensysteme", "PROF-80",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
