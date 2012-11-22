/***************************************************************************

        Sage II

        For memory map look at :
            http://www.thebattles.net/sage/img/SDT.pdf  (pages 14-)


        06/12/2009 Skeleton driver.

****************************************************************************/

/*

    TODO:

    - floppy loading
    - PROM test fails
    - i8251 test fails on boot

        001BD8: move.b  D6, $c071.w
        001BDC: moveq   #$e, D7
        001BDE: dbra    D7, $1bde
        001BE2: btst    #$2, $c073.w
        001BE8: bne     $1bec

    - TMS9914 IEEE-488 controller
    - board 2 (4x 2651 USART)
    - Winchester controller

*/

#include "includes/sage2.h"
#include "formats/mfi_dsk.h"


//**************************************************************************
//  MEMORY MANAGEMENT UNIT
//**************************************************************************

//-------------------------------------------------
//  mmu_r -
//-------------------------------------------------

READ8_MEMBER( sage2_state::mmu_r )
{
	UINT8 data = 0xff;

	if (m_reset || (offset >= 0xfe0000))
	{
		data = memregion(M68000_TAG)->base()[offset & 0x1fff];
	}
	else if (offset < 0x080000)
	{
		data = m_ram->pointer()[offset];
	}

	return data;
}


//-------------------------------------------------
//  mmu_w -
//-------------------------------------------------

WRITE8_MEMBER( sage2_state::mmu_w )
{
	if (offset < 0x080000)
	{
		m_ram->pointer()[offset] = data;
	}
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( sage2_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( sage2_mem, AS_PROGRAM, 16, sage2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0xfeffff) AM_READWRITE8(mmu_r, mmu_w, 0xffff)
	AM_RANGE(0xffc000, 0xffc007) AM_DEVREADWRITE8_LEGACY(I8253_1_TAG, pit8253_r, pit8253_w, 0x00ff)
//  AM_RANGE(0xffc010, 0xffc01f) AM_DEVREADWRITE8(TMS9914_TAG, tms9914_device, read, write, 0x00ff)
	AM_RANGE(0xffc020, 0xffc027) AM_DEVREADWRITE8(I8255A_0_TAG, i8255_device, read, write, 0x00ff) // i8255, DIPs + Floppy ctrl port
	AM_RANGE(0xffc030, 0xffc031) AM_DEVREADWRITE8(I8251_1_TAG, i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xffc032, 0xffc033) AM_DEVREADWRITE8(I8251_1_TAG, i8251_device, status_r, control_w, 0x00ff)
	AM_RANGE(0xffc040, 0xffc043) AM_DEVREADWRITE8_LEGACY(I8259_TAG, pic8259_r, pic8259_w, 0x00ff)
	AM_RANGE(0xffc050, 0xffc053) AM_DEVICE8(UPD765_TAG, upd765a_device, map, 0x00ff)
	AM_RANGE(0xffc060, 0xffc067) AM_DEVREADWRITE8(I8255A_0_TAG, i8255_device, read, write, 0x00ff) // i8255, Printer
	AM_RANGE(0xffc070, 0xffc071) AM_DEVREAD8(I8251_0_TAG, i8251_device, data_r, 0x00ff) AM_DEVWRITE8(TERMINAL_TAG, generic_terminal_device, write, 0x00ff)
//  AM_RANGE(0xffc070, 0xffc071) AM_DEVREADWRITE8(I8251_0_TAG, i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xffc072, 0xffc073) AM_DEVREADWRITE8(I8251_0_TAG, i8251_device, status_r, control_w, 0x00ff)
	AM_RANGE(0xffc080, 0xffc087) AM_MIRROR(0x78) AM_DEVREADWRITE8_LEGACY(I8253_0_TAG, pit8253_r, pit8253_w, 0x00ff)
//  AM_RANGE(0xffc400, 0xffc407) AM_DEVREADWRITE8(S2651_0_TAG, s2651_device, read, write, 0x00ff)
//  AM_RANGE(0xffc440, 0xffc447) AM_DEVREADWRITE8(S2651_1_TAG, s2651_device, read, write, 0x00ff)
//  AM_RANGE(0xffc480, 0xffc487) AM_DEVREADWRITE8(S2651_2_TAG, s2651_device, read, write, 0x00ff)
//  AM_RANGE(0xffc4c0, 0xffc4c7) AM_DEVREADWRITE8(S2651_3_TAG, s2651_device, read, write, 0x00ff)
//  AM_RANGE(0xffc500, 0xffc7ff) // Winchester drive ports
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( sage2 )
//-------------------------------------------------

static INPUT_PORTS_START( sage2 )
	PORT_START("J7")
	PORT_DIPNAME( 0x07, 0x07, "Terminal Baud Rate" ) PORT_DIPLOCATION("J7:1,2,3")
	PORT_DIPSETTING(    0x07, "19200" )
	PORT_DIPSETTING(    0x06, "9600" )
	PORT_DIPSETTING(    0x05, "4800" )
	PORT_DIPSETTING(    0x04, "2400" )
	PORT_DIPSETTING(    0x03, "1200" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x00, "Reserved (19200)" )
	PORT_DIPNAME( 0x08, 0x08, "Parity Control" ) PORT_DIPLOCATION("J7:4")
	PORT_DIPSETTING(    0x08, "Even Parity" )
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPNAME( 0x30, 0x20, "Boot Device" ) PORT_DIPLOCATION("J7:5,6")
	PORT_DIPSETTING(    0x30, "Debugger" )
	PORT_DIPSETTING(    0x20, "Floppy Drive 0" )
	PORT_DIPSETTING(    0x10, "Winchester" )
	PORT_DIPSETTING(    0x00, "Reserved (Debugger)" )
	PORT_DIPNAME( 0x40, 0x40, "Floppy Configuration" ) PORT_DIPLOCATION("J7:7")
	PORT_DIPSETTING(    0x40, "96 TPI" )
	PORT_DIPSETTING(    0x00, "48 TPI" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("J7:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("J6")
	PORT_DIPNAME( 0x1f, 0x07, "IEEE-488 Bus Address" ) PORT_DIPLOCATION("J6:1,2,3,4,5")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )
	PORT_DIPSETTING(    0x0a, "10" )
	PORT_DIPSETTING(    0x0b, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0d, "13" )
	PORT_DIPSETTING(    0x0e, "14" )
	PORT_DIPSETTING(    0x0f, "15" )
	PORT_DIPSETTING(    0x10, "16" )
	PORT_DIPSETTING(    0x11, "17" )
	PORT_DIPSETTING(    0x12, "18" )
	PORT_DIPSETTING(    0x13, "19" )
	PORT_DIPSETTING(    0x14, "20" )
	PORT_DIPSETTING(    0x15, "21" )
	PORT_DIPSETTING(    0x16, "22" )
	PORT_DIPSETTING(    0x17, "23" )
	PORT_DIPSETTING(    0x18, "24" )
	PORT_DIPSETTING(    0x19, "25" )
	PORT_DIPSETTING(    0x1a, "26" )
	PORT_DIPSETTING(    0x1b, "27" )
	PORT_DIPSETTING(    0x1c, "28" )
	PORT_DIPSETTING(    0x1d, "29" )
	PORT_DIPSETTING(    0x1e, "30" )
	PORT_DIPSETTING(    0x1f, "31" )
	PORT_DIPNAME( 0x20, 0x00, "IEEE-488 TALK" ) PORT_DIPLOCATION("J6:6")
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPSETTING(    0x20, "Enabled" )
	PORT_DIPNAME( 0x40, 0x00, "IEEE-488 LISTEN" ) PORT_DIPLOCATION("J6:7")
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPSETTING(    0x40, "Enabled" )
	PORT_DIPNAME( 0x80, 0x00, "IEEE-488 Consecutive Adresses" ) PORT_DIPLOCATION("J6:8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  I8255A_INTERFACE( ppi0_intf )
//-------------------------------------------------

/*

    IR0     U74 OUT2
    IR1     RX2I+
    IR2     TX1I+
    IR3     TX2I+
    IR4     MI-
    IR5     CNI+
    IR6     U74 OUT0
    IR7     SI+

*/

static const struct pic8259_interface pic_intf =
{
	DEVCB_CPU_INPUT_LINE(M68000_TAG, M68K_IRQ_1),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};


//-------------------------------------------------
//  I8255A_INTERFACE( ppi0_intf )
//-------------------------------------------------

WRITE8_MEMBER( sage2_state::ppi0_pc_w )
{
	/*

        bit     signal

        PC0     TC+
        PC1     RDY+
        PC2     FDIE+
        PC3     SL0-
        PC4     SL1-
        PC5     MOT-
        PC6     PCRMP-
        PC7     FRES+

    */

	// floppy terminal count
	m_fdc->tc_w(BIT(data, 0));

	// floppy ready
	m_fdc->ready_w(BIT(data, 1));

	// floppy interrupt enable
	m_fdie = BIT(data, 2);
	update_fdc_int();

	// drive select
	m_sl0 = BIT(data, 3);
	m_sl1 = BIT(data, 4);

	if(m_sl0)
		m_fdc->set_floppy(m_floppy0);
	else if(m_sl1)
		m_fdc->set_floppy(m_floppy1);
	else
		m_fdc->set_floppy(NULL);

	// floppy motor
	m_floppy0->mon_w(BIT(data, 5));
	m_floppy1->mon_w(BIT(data, 5));

	// FDC reset
	if(BIT(data, 7))
		m_fdc->reset();
}

static I8255A_INTERFACE( ppi0_intf )
{
	DEVCB_INPUT_PORT("J7"),
	DEVCB_NULL,
	DEVCB_INPUT_PORT("J6"),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(sage2_state, ppi0_pc_w)
};


//-------------------------------------------------
//  I8255A_INTERFACE( ppi1_intf )
//-------------------------------------------------

READ8_MEMBER( sage2_state::ppi1_pb_r )
{
	/*

        bit     signal

        PB0     FDI+
        PB1     WP+
        PB2     RG-
        PB3     CD-
        PB4     BUSY
        PB5     PAPER
        PB6     SEL
        PB7     FAULT-

    */

	UINT8 data = 0;

	// floppy interrupt
	data = m_fdc->get_irq();

	// floppy write protected
	if (!m_sl0) data |= floppy_wpt_r(m_floppy0) << 1;
	if (!m_sl1) data |= floppy_wpt_r(m_floppy1) << 1;

	// RS-232 ring indicator

	// RS-232 carrier detect

	// centronics
	data |= m_centronics->busy_r() << 4;
	data |= m_centronics->pe_r() << 5;
	data |= m_centronics->vcc_r() << 6;
	data |= m_centronics->fault_r() << 7;

	return data;
}

WRITE8_MEMBER( sage2_state::ppi1_pc_w )
{
	/*

        bit     signal

        PC0     PRES-
        PC1     U8 SC+
        PC2     SI+
        PC3     LEDR+
        PC4     STROBE-
        PC5     PRIME-
        PC6     U38 CL-
        PC7     RMI-

    */

	if (!BIT(data, 0))
	{
		// clear parity error interrupt
		m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
	}

	// s? interrupt
	pic8259_ir7_w(m_pic, BIT(data, 2));

	// processor LED
	output_set_led_value(0, BIT(data, 3));

	// centronics
	m_centronics->strobe_w(BIT(data, 4));
	m_centronics->init_prime_w(BIT(data, 5));

	if (!BIT(data, 6))
	{
		// clear ACK interrupt
		pic8259_ir5_w(m_pic, CLEAR_LINE);
	}

	if (!BIT(data, 7))
	{
		// clear modem interrupt
		pic8259_ir4_w(m_pic, CLEAR_LINE);
	}
}

static I8255A_INTERFACE( ppi1_intf )
{
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, write),
	DEVCB_DRIVER_MEMBER(sage2_state, ppi1_pb_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(sage2_state, ppi1_pc_w)
};


//-------------------------------------------------
//  pit8253_config pit0_intf
//-------------------------------------------------

static const struct pit8253_config pit0_intf =
{
	{
		{
			0, // from U75 OUT0
			DEVCB_LINE_VCC,
			DEVCB_DEVICE_LINE(I8259_TAG, pic8259_ir6_w)
		}, {
			XTAL_16MHz/2/125,
			DEVCB_LINE_VCC,
			DEVCB_DEVICE_LINE(I8253_0_TAG, pit8253_clk2_w)
		}, {
			0, // from OUT2
			DEVCB_LINE_VCC,
			DEVCB_DEVICE_LINE(I8259_TAG, pic8259_ir0_w)
		}
	}
};


//-------------------------------------------------
//  pit8253_config pit1_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( sage2_state::br1_w )
{
	m_usart0->transmit_clock();
	m_usart0->receive_clock();
}

WRITE_LINE_MEMBER( sage2_state::br2_w )
{
	m_usart1->transmit_clock();
	m_usart1->receive_clock();
}

static const struct pit8253_config pit1_intf =
{
	{
		{
			XTAL_16MHz/2/125,
			DEVCB_LINE_VCC,
			DEVCB_DEVICE_LINE(I8253_0_TAG, pit8253_clk0_w)
		}, {
			XTAL_16MHz/2/13,
			DEVCB_LINE_VCC,
			DEVCB_DRIVER_LINE_MEMBER(sage2_state, br1_w)
		}, {
			XTAL_16MHz/2/13,
			DEVCB_LINE_VCC,
			DEVCB_DRIVER_LINE_MEMBER(sage2_state, br2_w)
		}
	}
};


//-------------------------------------------------
//  i8251_interface usart0_intf
//-------------------------------------------------

static const i8251_interface usart0_intf =
{
	DEVCB_NULL, //DEVCB_DEVICE_LINE(TERMINAL_TAG, terminal_serial_r),
	DEVCB_NULL, //DEVCB_DEVICE_LINE(TERMINAL_TAG, terminal_serial_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_CPU_INPUT_LINE(M68000_TAG, M68K_IRQ_5),
	DEVCB_DEVICE_LINE(I8259_TAG, pic8259_ir2_w),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  i8251_interface usart1_intf
//-------------------------------------------------

static const i8251_interface usart1_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE(I8259_TAG, pic8259_ir1_w),
	DEVCB_DEVICE_LINE(I8259_TAG, pic8259_ir3_w),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

static SLOT_INTERFACE_START( sage2_floppies )
	 SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

void sage2_state::update_fdc_int()
{
	m_maincpu->set_input_line(M68K_IRQ_6, m_fdie & m_fdc_int);
}

void sage2_state::fdc_irq(bool state)
{
	m_fdc_int = state;
	update_fdc_int();
}

void sage2_state::machine_start()
{
	m_fdc->setup_intrq_cb(upd765a_device::line_cb(FUNC(sage2_state::fdc_irq), this));
}

//-------------------------------------------------
//  centronics_interface centronics_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( sage2_state::ack_w )
{
	if (!state)
	{
		pic8259_ir5_w(m_pic, ASSERT_LINE);
	}
}

static const centronics_interface centronics_intf =
{
	DEVCB_DRIVER_LINE_MEMBER(sage2_state, ack_w),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  IEEE488_INTERFACE( ieee488_intf )
//-------------------------------------------------

static IEEE488_INTERFACE( ieee488_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  GENERIC_TERMINAL_INTERFACE( terminal_intf )
//-------------------------------------------------

WRITE8_MEMBER( sage2_state::kbd_put )
{
	m_usart0->receive_character(data);
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(sage2_state, kbd_put)
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_RESET( sage2 )
//-------------------------------------------------

void sage2_state::machine_reset()
{
	m_reset = 1;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( sage2 )
//-------------------------------------------------

static MACHINE_CONFIG_START( sage2, sage2_state )
	// basic machine hardware
	MCFG_CPU_ADD(M68000_TAG, M68000, XTAL_16MHz/2)
	MCFG_CPU_PROGRAM_MAP(sage2_mem)

	// video hardware
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)

	// devices
	MCFG_PIC8259_ADD(I8259_TAG, pic_intf)
	MCFG_I8255A_ADD(I8255A_0_TAG, ppi0_intf)
	MCFG_I8255A_ADD(I8255A_1_TAG, ppi1_intf)
	MCFG_PIT8253_ADD(I8253_0_TAG, pit0_intf)
	MCFG_PIT8253_ADD(I8253_1_TAG, pit1_intf)
	MCFG_I8251_ADD(I8251_0_TAG, usart0_intf)
	MCFG_I8251_ADD(I8251_1_TAG, usart1_intf)
	MCFG_UPD765A_ADD(UPD765_TAG, false, false)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, centronics_intf)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", sage2_floppies, "525dd", 0, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", sage2_floppies, "525dd", 0, floppy_image_device::default_floppy_formats)
	MCFG_IEEE488_BUS_ADD(ieee488_intf)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "sage2")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( sage2 )
//-------------------------------------------------

ROM_START( sage2 )
	ROM_REGION( 0x2000, M68000_TAG, 0 )
	ROM_LOAD16_BYTE( "sage2.u18", 0x0000, 0x1000, CRC(ca9b312d) SHA1(99436a6d166aa5280c3b2d28355c4d20528fe48c) )
	ROM_LOAD16_BYTE( "sage2.u17", 0x0001, 0x1000, CRC(27e25045) SHA1(041cd9d4617473d089f31f18cbb375046c3b61bb) )
ROM_END



//**************************************************************************
//  DRIVER INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  DRIVER_INIT( sage2 )
//-------------------------------------------------

DIRECT_UPDATE_MEMBER(sage2_state::sage2_direct_update_handler)
{
	if (m_reset && address >= 0xfe0000)
	{
		m_reset = 0;
	}

	return address;
}

DRIVER_INIT_MEMBER(sage2_state,sage2)
{
	address_space &program = machine().device<cpu_device>(M68000_TAG)->space(AS_PROGRAM);
	program.set_direct_update_handler(direct_update_delegate(FUNC(sage2_state::sage2_direct_update_handler), this));

	// patch out i8251 test
	machine().root_device().memregion(M68000_TAG)->base()[0x1be8] = 0xd6;
	machine().root_device().memregion(M68000_TAG)->base()[0x1be9] = 0x4e;
}



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY                             FULLNAME    FLAGS
COMP( 1982, sage2,  0,       0,      sage2,     sage2, sage2_state,    sage2, "Sage Technology", "Sage II", GAME_NOT_WORKING | GAME_NO_SOUND )
