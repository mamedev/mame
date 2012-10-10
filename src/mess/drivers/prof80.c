/*

    PROF-80 (Prozessor RAM-Floppy Kontroller)
    GRIP-1/2/3/4/5 (Grafik-Interface-Prozessor)
    UNIO-1 (?)

    http://www.prof80.de/
    http://oldcomputers.dyndns.org/public/pub/rechner/conitec/info.html

*/

/*

    TODO:

    - NE555 timeout is 10x too high
    - grip31 does not work
    - UNIO card (Z80-STI, Z80-SIO, 2x centronics)
    - GRIP-COLOR (192kB color RAM)
    - GRIP-5 (HD6345, 256KB RAM)
    - XR color card

*/

#include "includes/prof80.h"
#include "formats/mfi_dsk.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define BLK_RAM1	0x0b
#define BLK_RAM2	0x0a
#define BLK_RAM3	0x03
#define BLK_RAM4	0x02
#define BLK_EPROM	0x00



//**************************************************************************
//  MEMORY MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  bankswitch -
//-------------------------------------------------

void prof80_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();
	UINT8 *rom = memregion(Z80_TAG)->base();
	int bank;

	for (bank = 0; bank < 16; bank++)
	{
		UINT16 start_addr = bank * 0x1000;
		UINT16 end_addr = start_addr + 0xfff;
		int block = m_init ? m_mmu[bank] : BLK_EPROM;

		switch (block)
		{
		case BLK_RAM1:
			program.install_ram(start_addr, end_addr, ram + ((bank % 8) * 0x1000));
			break;

		case BLK_RAM2:
			program.install_ram(start_addr, end_addr, ram + 0x8000 + ((bank % 8) * 0x1000));
			break;

		case BLK_RAM3:
			program.install_ram(start_addr, end_addr, ram + 0x10000 + ((bank % 8) * 0x1000));
			break;

		case BLK_RAM4:
			program.install_ram(start_addr, end_addr, ram + 0x18000 + ((bank % 8) * 0x1000));
			break;

		case BLK_EPROM:
			program.install_rom(start_addr, end_addr, rom + ((bank % 2) * 0x1000));
			break;

		default:
			program.unmap_readwrite(start_addr, end_addr);
		}

		//logerror("Segment %u address %04x-%04x block %u\n", bank, start_addr, end_addr, block);
	}
}



//**************************************************************************
//  PERIPHERALS
//**************************************************************************

//-------------------------------------------------
//  floppy_motor_off -
//-------------------------------------------------

void prof80_state::floppy_motor_off()
{
	if(m_floppy0)
		m_floppy0->mon_w(true);
	if(m_floppy1)
		m_floppy1->mon_w(true);

	m_motor = 0;
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

	case 3:	// READY
		m_fdc->ready_w(fa);
		break;

	case 4: // TCK
		m_rtc->clk_w(fa);
		break;

	case 5:	// IN USE
		output_set_led_value(0, fa);
		break;

	case 6:	// _MOTOR
		if (fa)
		{
			// trigger floppy motor off NE555 timer
			int t = 110 * RES_M(10) * CAP_U(6.8); // t = 1.1 * R8 * C6

			timer_set(attotime::from_msec(t), TIMER_ID_MOTOR);
		}
		else
		{
			// turn on floppy motor
			if(m_floppy0)
				m_floppy0->mon_w(false);
			if(m_floppy1)
				m_floppy1->mon_w(false);

			m_motor = 1;

			// reset floppy motor off NE555 timer
			timer_set(attotime::never, TIMER_ID_MOTOR);
		}
		break;

	case 7:	// SELECT
		break;
	}

	switch (sb)
	{
	case 0: // RESF
		if (fb) m_fdc->reset();
		break;

	case 1: // MINI
		break;

	case 2: // _RTS
		break;

	case 3: // TX
		break;

	case 4: // _MSTOP
		if (!fb)
		{
			// immediately turn off floppy motor
			synchronize(TIMER_ID_MOTOR);
		}
		break;

	case 5: // TXP
		break;

	case 6: // TSTB
		m_rtc->stb_w(fb);
		break;

	case 7: // MME
		//logerror("INIT %u\n", fb);
		m_init = fb;
		bankswitch();
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

	// clear to send
	data |= 0x10;

	// floppy index
	if(m_floppy0)
		data |= m_floppy0->idx_r() << 5;

	if(m_floppy1)
		data |= m_floppy1->idx_r() << 5;

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
	data |= !m_motor;

	// JS4
	switch (ioport("J4")->read())
	{
	case 0: js4 = 0; break;
	case 1: js4 = 1; break;
	case 2: js4 = !m_c0; break;
	case 3: js4 = !m_c1; break;
	case 4: js4 = !m_c2; break;
	}

	data |= js4 << 4;

	// JS5
	switch (ioport("J5")->read())
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


//-------------------------------------------------
//  par_w -
//-------------------------------------------------

WRITE8_MEMBER( prof80_state::par_w )
{
	int bank = offset >> 12;

	m_mmu[bank] = data & 0x0f;

	//logerror("MMU bank %u block %u\n", bank, data & 0x0f);

	bankswitch();
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
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( prof80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( prof80_io, AS_IO, 8, prof80_state )
	AM_RANGE(0x00, 0xd7) AM_MIRROR(0xff00) AM_DEVREADWRITE(ECBBUS_TAG, ecbbus_device, io_r, io_w)
//  AM_RANGE(0x80, 0x8f) AM_MIRROR(0xff00) AM_DEVREADWRITE_LEGACY(UNIO_Z80STI_TAG, z80sti_r, z80sti_w)
//  AM_RANGE(0x94, 0x95) AM_MIRROR(0xff00) AM_DEVREADWRITE_LEGACY(UNIO_Z80SIO_TAG, z80sio_d_r, z80sio_d_w)
//  AM_RANGE(0x96, 0x97) AM_MIRROR(0xff00) AM_DEVREADWRITE_LEGACY(UNIO_Z80SIO_TAG, z80sio_c_r, z80sio_c_w)
//  AM_RANGE(0x9e, 0x9e) AM_MIRROR(0xff00) AM_WRITE(unio_ctrl_w)
//  AM_RANGE(0x9c, 0x9c) AM_MIRROR(0xff00) AM_DEVWRITE_LEGACY(UNIO_CENTRONICS1_TAG, centronics_data_w)
//  AM_RANGE(0x9d, 0x9d) AM_MIRROR(0xff00) AM_DEVWRITE_LEGACY(UNIO_CENTRONICS1_TAG, centronics_data_w)
//  AM_RANGE(0xc0, 0xc0) AM_MIRROR(0xff00) AM_READ(gripc_r)
//  AM_RANGE(0xc1, 0xc1) AM_MIRROR(0xff00) AM_READWRITE(gripd_r, gripd_w)
	AM_RANGE(0xd8, 0xd8) AM_MIRROR(0xff00) AM_WRITE(flr_w)
	AM_RANGE(0xda, 0xda) AM_MIRROR(0xff00) AM_READ(status_r)
	AM_RANGE(0xdb, 0xdb) AM_MIRROR(0xff00) AM_READ(status2_r)
	AM_RANGE(0xdc, 0xdd) AM_MIRROR(0xff00) AM_DEVICE(UPD765_TAG, upd765a_device, map)
	AM_RANGE(0xde, 0xde) AM_MIRROR(0xff01) AM_MASK(0xff00) AM_WRITE(par_w)
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
//  UPD1990A_INTERFACE( rtc_intf )
//-------------------------------------------------

static UPD1990A_INTERFACE( rtc_intf )
{
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

static const floppy_format_type prof80_floppy_formats[] = {
	FLOPPY_MFI_FORMAT,
	NULL
};

static SLOT_INTERFACE_START( prof80_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  ECB_BUS_INTERFACE( ecb_intf )
//-------------------------------------------------

static ECBBUS_INTERFACE( ecb_intf )
{
	DEVCB_NULL,
	DEVCB_NULL
};

static SLOT_INTERFACE_START( prof80_ecb_cards )
	SLOT_INTERFACE("grip21", ECB_GRIP21)
/*  SLOT_INTERFACE("grip25", ECB_GRIP25)
    SLOT_INTERFACE("grip26", ECB_GRIP26)
    SLOT_INTERFACE("grip31", ECB_GRIP31)
    SLOT_INTERFACE("grip562", ECB_GRIP562)
    SLOT_INTERFACE("grips115", ECB_GRIPS115)*/
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
		floppy_motor_off();
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

	// bank switch
	bankswitch();

	// register for state saving
	save_item(NAME(m_c0));
	save_item(NAME(m_c1));
	save_item(NAME(m_c2));
	save_item(NAME(m_mmu));
	save_item(NAME(m_init));
	save_item(NAME(m_fdc_index));
}


//-------------------------------------------------
//  MACHINE_RESET( prof80 )
//-------------------------------------------------

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

    // video hardware
    MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
    MCFG_SCREEN_REFRESH_RATE(50)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
    MCFG_SCREEN_UPDATE_DEVICE(ECBBUS_TAG, ecbbus_device, screen_update)
    MCFG_SCREEN_SIZE(640, 480)
    MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
    MCFG_PALETTE_LENGTH(2)
    MCFG_PALETTE_INIT(black_and_white)

	// devices
	MCFG_UPD1990A_ADD(UPD1990A_TAG, XTAL_32_768kHz, rtc_intf)
	MCFG_UPD765A_ADD(UPD765_TAG, false, true)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", prof80_floppies, "525hd", 0, prof80_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", prof80_floppies, "525hd", 0, prof80_floppy_formats)

	// ECB bus
	MCFG_ECBBUS_ADD(Z80_TAG, ecb_intf)
	MCFG_ECBBUS_SLOT_ADD(1, "ecb_1", prof80_ecb_cards, "grip21", NULL)
	MCFG_ECBBUS_SLOT_ADD(2, "ecb_2", prof80_ecb_cards, NULL, NULL)
	MCFG_ECBBUS_SLOT_ADD(3, "ecb_3", prof80_ecb_cards, NULL, NULL)
	MCFG_ECBBUS_SLOT_ADD(4, "ecb_4", prof80_ecb_cards, NULL, NULL)
	MCFG_ECBBUS_SLOT_ADD(5, "ecb_5", prof80_ecb_cards, NULL, NULL)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
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
COMP( 1984, prof80,     0,		0,		prof80,	prof80, driver_device,	0,		"Conitec Datensysteme",	"PROF-80",				GAME_NOT_WORKING | GAME_NO_SOUND)
