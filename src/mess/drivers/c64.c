/*

    TODO:

    - floating bus writes to peripheral registers in m6502.c
    - sort out kernals between PAL/NTSC
    - tsuit215 test failures
        - IRQ (WRONG $DC0D)
        - NMI (WRONG $DD0D)
        - all CIA tests

    - 64C PLA dump
    - clean up inputs
    - Multiscreen crashes on boot

        805A: lda  $01
        805C: and  #$FE
        805E: sta  $01
        8060: m6502_brk#$00 <-- BOOM!

*/

#include "includes/c64.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define A15 BIT(offset, 15)
#define A14 BIT(offset, 14)
#define A13 BIT(offset, 13)
#define A12 BIT(offset, 12)
#define VA13 BIT(va, 13)
#define VA12 BIT(va, 12)



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

void c64_state::check_interrupts()
{
	int restore = BIT(ioport("SPECIAL")->read(), 7);

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_cia1_irq || m_vic_irq || m_exp_irq);
	m_maincpu->set_input_line(INPUT_LINE_NMI, m_cia2_irq || restore || m_exp_nmi);

	mos6526_flag_w(m_cia1, m_cass_rd && m_iec_srq);
}



//**************************************************************************
//  MEMORY MANAGEMENT UNIT
//**************************************************************************

//-------------------------------------------------
//  bankswitch -
//-------------------------------------------------

void c64_state::bankswitch(offs_t offset, offs_t va, int rw, int aec, int ba, int cas, int *casram, int *basic, int *kernal, int *charom, int *grw, int *io, int *roml, int *romh)
{
	int game = m_exp->game_r(offset, ba, rw, m_hiram);
	int exrom = m_exp->exrom_r(offset, ba, rw, m_hiram);

	UINT32 input = VA12 << 15 | VA13 << 14 | game << 13 | exrom << 12 | rw << 11 | aec << 10 | ba << 9 | A12 << 8 | A13 << 7 | A14 << 6 | A15 << 5 | m_va14 << 4 | m_charen << 3 | m_hiram << 2 | m_loram << 1 | cas;
	UINT32 data = m_pla->read(input);

	*casram = BIT(data, 0);
	*basic = BIT(data, 1);
	*kernal = BIT(data, 2);
	*charom = BIT(data, 3);
	*grw = BIT(data, 4);
	*io = BIT(data, 5);
	*roml = BIT(data, 6);
	*romh = BIT(data, 7);
}


//-------------------------------------------------
//  read_memory -
//-------------------------------------------------

UINT8 c64_state::read_memory(address_space &space, offs_t offset, int ba, int casram, int basic, int kernal, int charom, int io, int roml, int romh)
{
	int io1 = 1, io2 = 1;

	UINT8 data = 0xff;

	if (ba)
	{
		data = m_vic->bus_r();
	}

	if (!casram)
	{
		data = m_ram->pointer()[offset];
	}
	else if (!basic)
	{
		data = m_basic[offset & 0x1fff];
	}
	else if (!kernal)
	{
		data = m_kernal[offset & 0x1fff];
	}
	else if (!charom)
	{
		data = m_charom[offset & 0xfff];
	}
	else if (!io)
	{
		switch ((offset >> 10) & 0x03)
		{
		case 0: // VIC
			data = m_vic->read(space, offset & 0x3f);
			break;

		case 1: // SID
			data = m_sid->read(space, offset & 0x1f);
			break;

		case 2: // COLOR
			data = m_color_ram[offset & 0x3ff] | 0xf0;
			break;

		case 3: // CIAS
			switch ((offset >> 8) & 0x03)
			{
			case 0: // CIA1
				if (offset & 1)
					cia_set_port_mask_value(m_cia1, 1, ioport("CTRLSEL")->read() & 0x80 ? c64_keyline[9] : c64_keyline[8] );
				else
					cia_set_port_mask_value(m_cia1, 0, ioport("CTRLSEL")->read() & 0x80 ? c64_keyline[8] : c64_keyline[9] );

				data = m_cia1->read(space, offset & 0x0f);
				break;

			case 1: // CIA2
				data = m_cia2->read(space, offset & 0x0f);
				break;

			case 2: // I/O1
				io1 = 0;
				break;

			case 3: // I/O2
				io2 = 0;
				break;
			}
			break;
		}
	}

	return m_exp->cd_r(space, offset, data, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( c64_state::read )
{
	offs_t va = 0;
	int rw = 1, aec = 0, ba = 1, cas = 0;
	int casram, basic, kernal, charom, grw, io, roml, romh;

	bankswitch(offset, va, rw, aec, ba, cas, &casram, &basic, &kernal, &charom, &grw, &io, &roml, &romh);

	return read_memory(space, offset, ba, casram, basic, kernal, charom, io, roml, romh);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( c64_state::write )
{
	offs_t va = 0;
	int rw = 0, aec = 0, ba = 1, cas = 0;
	int io1 = 1, io2 = 1;
	int casram, basic, kernal, charom, grw, io, roml, romh;

	bankswitch(offset, va, rw, aec, ba, cas, &casram, &basic, &kernal, &charom, &grw, &io, &roml, &romh);

	if (offset < 0x0002)
	{
		// write to internal CPU register
		data = m_vic->bus_r();
	}

	if (!casram)
	{
		m_ram->pointer()[offset] = data;
	}
	else if (!io)
	{
		switch ((offset >> 10) & 0x03)
		{
		case 0: // VIC
			m_vic->write(space, offset & 0x3f, data);
			break;

		case 1: // SID
			m_sid->write(space, offset & 0x1f, data);
			break;

		case 2: // COLOR
			if (!grw) m_color_ram[offset & 0x3ff] = data & 0x0f;
			break;

		case 3: // CIAS
			switch ((offset >> 8) & 0x03)
			{
			case 0: // CIA1
				m_cia1->write(space, offset & 0x0f, data);
				break;

			case 1: // CIA2
				m_cia2->write(space, offset & 0x0f, data);
				break;

			case 2: // I/O1
				io1 = 0;
				break;

			case 3: // I/O2
				io2 = 0;
				break;
			}
			break;
		}
	}

	m_exp->cd_w(space, offset, data, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  vic_videoram_r -
//-------------------------------------------------

READ8_MEMBER( c64_state::vic_videoram_r )
{
	offset = (!m_va15 << 15) | (!m_va14 << 14) | offset;

	int rw = 1, aec = 1, ba = 0, cas = 0;
	int casram, basic, kernal, charom, grw, io, roml, romh;
	bankswitch(0xffff, offset, rw, aec, ba, cas, &casram, &basic, &kernal, &charom, &grw, &io, &roml, &romh);

	return read_memory(space, offset, ba, casram, basic, kernal, charom, io, roml, romh);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( c64_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c64_mem, AS_PROGRAM, 8, c64_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_videoram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_videoram_map, AS_0, 8, c64_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ(vic_videoram_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic_colorram_map )
//-------------------------------------------------

static ADDRESS_MAP_START( vic_colorram_map, AS_1, 8, c64_state )
	AM_RANGE(0x000, 0x3ff) AM_RAM AM_SHARE("color_ram")
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( c64 )
//-------------------------------------------------

static INPUT_PORTS_START( c64 )
	PORT_INCLUDE( common_cbm_keyboard )		// ROW0 -> ROW7

	PORT_INCLUDE( c64_special )				// SPECIAL

	PORT_INCLUDE( c64_controls )			// CTRLSEL, JOY0, JOY1, PADDLE0 -> PADDLE3, TRACKX, TRACKY, LIGHTX, LIGHTY, OTHER
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c64sw )
//-------------------------------------------------

static INPUT_PORTS_START( c64sw )
	PORT_INCLUDE( c64 )

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\xa5") PORT_CODE(KEYCODE_OPENBRACE)	PORT_CHAR('\xA5')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)								PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)							PORT_CHAR('=')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)								PORT_CHAR('-')

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\xa4") PORT_CODE(KEYCODE_BACKSLASH)	PORT_CHAR('\xA4')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\xb6") PORT_CODE(KEYCODE_QUOTE)		PORT_CHAR('\xB6')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)						PORT_CHAR('@')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)						PORT_CHAR(':') PORT_CHAR('*')
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c64gs )
//-------------------------------------------------

static INPUT_PORTS_START( c64gs )
	PORT_INCLUDE( c64 )

	// 2008 FP: This has to be cleaned up later
	// C64gs should simply not scan these inputs
	// as a temporary solution, we keep PeT IPT_UNUSED shortcut

	PORT_MODIFY( "ROW0" ) // no keyboard
	PORT_BIT (0xff, 0x00, IPT_UNUSED )
	PORT_MODIFY( "ROW1" ) // no keyboard
	PORT_BIT (0xff, 0x00, IPT_UNUSED )
	PORT_MODIFY( "ROW2" ) // no keyboard
	PORT_BIT (0xff, 0x00, IPT_UNUSED )
	PORT_MODIFY( "ROW3" ) // no keyboard
	PORT_BIT (0xff, 0x00, IPT_UNUSED )
	PORT_MODIFY( "ROW4" ) // no keyboard
	PORT_BIT (0xff, 0x00, IPT_UNUSED )
	PORT_MODIFY( "ROW5" ) // no keyboard
	PORT_BIT (0xff, 0x00, IPT_UNUSED )
	PORT_MODIFY( "ROW6" ) // no keyboard
	PORT_BIT (0xff, 0x00, IPT_UNUSED )
	PORT_MODIFY( "ROW7" ) // no keyboard
	PORT_BIT (0xff, 0x00, IPT_UNUSED )
	PORT_MODIFY( "SPECIAL" ) // no keyboard
	PORT_BIT (0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  vic2_interface vic_intf
//-------------------------------------------------

static INTERRUPT_GEN( c64_frame_interrupt )
{
	c64_state *state = device->machine().driver_data<c64_state>();

	state->check_interrupts();
	cbm_common_interrupt(device);
}

WRITE_LINE_MEMBER( c64_state::vic_irq_w )
{
	m_vic_irq = state;

	check_interrupts();
}

READ8_MEMBER( c64_state::vic_lightpen_x_cb )
{
	return ioport("LIGHTX")->read() & ~0x01;
}

READ8_MEMBER( c64_state::vic_lightpen_y_cb )
{
	return ioport("LIGHTY")->read() & ~0x01;
}

READ8_MEMBER( c64_state::vic_lightpen_button_cb )
{
	return ioport("OTHER")->read() & 0x04;
}

READ8_MEMBER( c64_state::vic_rdy_cb )
{
	return ioport("CYCLES")->read() & 0x07;
}

static MOS6567_INTERFACE( vic_intf )
{
	SCREEN_TAG,
	M6510_TAG,
	DEVCB_DRIVER_LINE_MEMBER(c64_state, vic_irq_w),
	DEVCB_NULL, // RDY
	DEVCB_DRIVER_MEMBER(c64_state, vic_lightpen_x_cb),
	DEVCB_DRIVER_MEMBER(c64_state, vic_lightpen_y_cb),
	DEVCB_DRIVER_MEMBER(c64_state, vic_lightpen_button_cb),
	DEVCB_DRIVER_MEMBER(c64_state, vic_rdy_cb)
};


//-------------------------------------------------
//  MOS6581_INTERFACE( sid_intf )
//-------------------------------------------------

READ8_MEMBER( c64_state::sid_potx_r )
{
	UINT8 cia1_pa = mos6526_pa_r(m_cia1, space, 0);
	
	int sela = BIT(cia1_pa, 6);
	int selb = BIT(cia1_pa, 7);

	UINT8 data = 0;

	if (sela) data = m_joy1->pot_x_r();
	if (selb) data = m_joy2->pot_x_r();

	return data;
}

READ8_MEMBER( c64_state::sid_poty_r )
{
	UINT8 cia1_pa = mos6526_pa_r(m_cia1, space, 0);
	
	int sela = BIT(cia1_pa, 6);
	int selb = BIT(cia1_pa, 7);

	UINT8 data = 0;

	if (sela) data = m_joy1->pot_y_r();
	if (selb) data = m_joy2->pot_y_r();

	return data;
}

static MOS6581_INTERFACE( sid_intf )
{
	DEVCB_DRIVER_MEMBER(c64_state, sid_potx_r),
	DEVCB_DRIVER_MEMBER(c64_state, sid_poty_r)
};


//-------------------------------------------------
//  mos6526_interface cia1_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_state::cia1_irq_w )
{
	m_cia1_irq = state;

	check_interrupts();
}

READ8_MEMBER( c64_state::cia1_pa_r )
{
	/*

        bit     description

        PA0     COL0, JOY B0
        PA1     COL1, JOY B1
        PA2     COL2, JOY B2
        PA3     COL3, JOY B3
        PA4     COL4, BTNB
        PA5     COL5
        PA6     COL6
        PA7     COL7

    */

	UINT8 cia0portb = mos6526_pb_r(m_cia1, space, 0);

	return cbm_common_cia0_port_a_r(m_cia1, cia0portb);
}

READ8_MEMBER( c64_state::cia1_pb_r )
{
	/*

        bit     description

        PB0     JOY A0
        PB1     JOY A1
        PB2     JOY A2
        PB3     JOY A3
        PB4     BTNA/_LP
        PB5
        PB6
        PB7

    */

	UINT8 cia0porta = mos6526_pa_r(m_cia1, space, 0);

	return cbm_common_cia0_port_b_r(m_cia1, cia0porta);
}

WRITE8_MEMBER( c64_state::cia1_pb_w )
{
	/*

        bit     description

        PB0     ROW0
        PB1     ROW1
        PB2     ROW2
        PB3     ROW3
        PB4     ROW4
        PB5     ROW5
        PB6     ROW6
        PB7     ROW7

    */

	m_vic->lp_w(BIT(data, 4));
}

static const mos6526_interface cia1_intf =
{
	DEVCB_DRIVER_LINE_MEMBER(c64_state, cia1_irq_w),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, sp1_w),
	DEVCB_DEVICE_LINE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, cnt1_w),
	DEVCB_DRIVER_MEMBER(c64_state, cia1_pa_r),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(c64_state, cia1_pb_r),
	DEVCB_DRIVER_MEMBER(c64_state, cia1_pb_w)
};


//-------------------------------------------------
//  mos6526_interface cia2_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_state::cia2_irq_w )
{
	m_cia2_irq = state;

	check_interrupts();
}

READ8_MEMBER( c64_state::cia2_pa_r )
{
	/*

        bit     description

        PA0
        PA1
        PA2     USER PORT
        PA3
        PA4
        PA5
        PA6     CLK
        PA7     DATA

    */

	UINT8 data = 0;

	// user port
	data |= m_user->pa2_r() << 2;

	// IEC bus
	data |= m_iec->clk_r() << 6;
	data |= m_iec->data_r() << 7;

	return data;
}

WRITE8_MEMBER( c64_state::cia2_pa_w )
{
	/*

        bit     description

        PA0     _VA14
        PA1     _VA15
        PA2     USER PORT
        PA3     ATN OUT
        PA4     CLK OUT
        PA5     DATA OUT
        PA6
        PA7

    */

	// VIC banking
	m_va14 = BIT(data, 0);
	m_va15 = BIT(data, 1);

	// user port
	m_user->pa2_w(BIT(data, 2));

	// IEC bus
	m_iec->atn_w(!BIT(data, 3));
	m_iec->clk_w(!BIT(data, 4));
	m_iec->data_w(!BIT(data, 5));
}

static const mos6526_interface cia2_intf =
{
	DEVCB_DRIVER_LINE_MEMBER(c64_state, cia2_irq_w),
	DEVCB_DEVICE_LINE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, pc2_w),
	DEVCB_DEVICE_LINE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, sp2_w),
	DEVCB_DEVICE_LINE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, cnt2_w),
	DEVCB_DRIVER_MEMBER(c64_state, cia2_pa_r),
	DEVCB_DRIVER_MEMBER(c64_state, cia2_pa_w),
	DEVCB_DEVICE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, pb_r),
	DEVCB_DEVICE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, pb_w)
};


//-------------------------------------------------
//  M6510_INTERFACE( cpu_intf )
//-------------------------------------------------

READ8_MEMBER( c64_state::cpu_r )
{
	/*

        bit     description

        P0      1
        P1      1
        P2      1
        P3
        P4      CASS SENS
        P5      0

    */

	UINT8 data = 0x07;

	data |= m_cassette->sense_r() << 4;

	return data;
}

WRITE8_MEMBER( c64_state::cpu_w )
{
	/*

        bit     description

        P0      LORAM
        P1      HIRAM
        P2      CHAREN
        P3      CASS WRT
        P4
        P5      CASS MOTOR

    */

    // memory banking
	m_loram = BIT(data, 0);
	m_hiram = BIT(data, 1);
	m_charen = BIT(data, 2);

	// cassette write
	m_cassette->write(BIT(data, 3));

	// cassette motor
	m_cassette->motor_w(BIT(data, 5));
}

static M6510_INTERFACE( cpu_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(c64_state, cpu_r),
	DEVCB_DRIVER_MEMBER(c64_state, cpu_w),
	0x17,
	0x20
};


//-------------------------------------------------
//  M6510_INTERFACE( sx64_cpu_intf )
//-------------------------------------------------

READ8_MEMBER( sx64_state::cpu_r )
{
	/*

        bit     description

        P0      1
        P1      1
        P2      1
        P3
        P4
        P5

    */

	return 0x07;
}

WRITE8_MEMBER( sx64_state::cpu_w )
{
	/*

        bit     description

        P0      LORAM
        P1      HIRAM
        P2      CHAREN
        P3
        P4
        P5

    */

    // memory banking
	m_loram = BIT(data, 0);
	m_hiram = BIT(data, 1);
	m_charen = BIT(data, 2);
}

static M6510_INTERFACE( sx64_cpu_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(sx64_state, cpu_r),
	DEVCB_DRIVER_MEMBER(sx64_state, cpu_w),
	0x07,
	0x00
};


//-------------------------------------------------
//  M6510_INTERFACE( c64gs_cpu_intf )
//-------------------------------------------------

READ8_MEMBER( c64gs_state::cpu_r )
{
	/*

        bit     description

        P0      1
        P1      1
        P2      1
        P3
        P4
        P5

    */

	return 0x07;
}

WRITE8_MEMBER( c64gs_state::cpu_w )
{
	/*

        bit     description

        P0      LORAM
        P1      HIRAM
        P2      CHAREN
        P3
        P4
        P5

    */

    // memory banking
	m_loram = BIT(data, 0);
	m_hiram = BIT(data, 1);
	m_charen = BIT(data, 2);
}

static M6510_INTERFACE( c64gs_cpu_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(c64gs_state, cpu_r),
	DEVCB_DRIVER_MEMBER(c64gs_state, cpu_w),
	0x07,
	0x00
};


//-------------------------------------------------
//  PET_DATASSETTE_PORT_INTERFACE( datassette_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_state::tape_read_w )
{
	m_cass_rd = state;

	check_interrupts();
}

static PET_DATASSETTE_PORT_INTERFACE( datassette_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(c64_state, tape_read_w),
};


//-------------------------------------------------
//  CBM_IEC_INTERFACE( iec_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_state::iec_srq_w )
{
	m_iec_srq = state;

	check_interrupts();
}

static CBM_IEC_INTERFACE( iec_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(c64_state, iec_srq_w),
	DEVCB_DEVICE_LINE_MEMBER(C64_USER_PORT_TAG, c64_user_port_device, atn_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  C64_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

READ8_MEMBER( c64_state::exp_dma_r )
{
	return m_maincpu->space(AS_PROGRAM)->read_byte(offset);
}

WRITE8_MEMBER( c64_state::exp_dma_w )
{
	m_maincpu->space(AS_PROGRAM)->write_byte(offset, data);
}

WRITE_LINE_MEMBER( c64_state::exp_irq_w )
{
	m_exp_irq = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( c64_state::exp_nmi_w )
{
	m_exp_nmi = state;

	check_interrupts();
}

WRITE_LINE_MEMBER( c64_state::exp_reset_w )
{
	if (state == ASSERT_LINE)
	{
		machine_reset();
	}
}

static C64_EXPANSION_INTERFACE( expansion_intf )
{
	DEVCB_DRIVER_MEMBER(c64_state, exp_dma_r),
	DEVCB_DRIVER_MEMBER(c64_state, exp_dma_w),
	DEVCB_DRIVER_LINE_MEMBER(c64_state, exp_irq_w),
	DEVCB_DRIVER_LINE_MEMBER(c64_state, exp_nmi_w),
	DEVCB_CPU_INPUT_LINE(M6510_TAG, INPUT_LINE_HALT),
	DEVCB_DRIVER_LINE_MEMBER(c64_state, exp_reset_w)
};


//-------------------------------------------------
//  C64_USER_PORT_INTERFACE( user_intf )
//-------------------------------------------------

static C64_USER_PORT_INTERFACE( user_intf )
{
	DEVCB_DEVICE_LINE(MOS6526_1_TAG, mos6526_sp_w),
	DEVCB_DEVICE_LINE(MOS6526_1_TAG, mos6526_cnt_w),
	DEVCB_DEVICE_LINE(MOS6526_2_TAG, mos6526_sp_w),
	DEVCB_DEVICE_LINE(MOS6526_2_TAG, mos6526_cnt_w),
	DEVCB_DEVICE_LINE(MOS6526_2_TAG, mos6526_flag_w),
	DEVCB_DRIVER_LINE_MEMBER(c64_state, exp_reset_w)
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( c64 )
//-------------------------------------------------

void c64_state::machine_start()
{
	cbm_common_init();

	// find memory regions
	m_basic = memregion("basic")->base();
	m_kernal = memregion("kernal")->base();
	m_charom = memregion("charom")->base();

	// state saving
	save_item(NAME(m_loram));
	save_item(NAME(m_hiram));
	save_item(NAME(m_charen));
	save_item(NAME(m_va14));
	save_item(NAME(m_va15));
	save_item(NAME(m_cia1_irq));
	save_item(NAME(m_cia2_irq));
	save_item(NAME(m_vic_irq));
	save_item(NAME(m_exp_irq));
	save_item(NAME(m_exp_nmi));
	save_item(NAME(m_cass_rd));
	save_item(NAME(m_iec_srq));
}


//-------------------------------------------------
//  MACHINE_START( c64c )
//-------------------------------------------------

void c64c_state::machine_start()
{
	c64_state::machine_start();

	// find memory regions
	m_basic = memregion(M6510_TAG)->base();
	m_kernal = memregion(M6510_TAG)->base() + 0x2000;
}


//-------------------------------------------------
//  MACHINE_START( c64gs )
//-------------------------------------------------

void c64gs_state::machine_start()
{
	c64c_state::machine_start();
}


//-------------------------------------------------
//  MACHINE_RESET( c64 )
//-------------------------------------------------

void c64_state::machine_reset()
{
	m_maincpu->reset();

	m_iec->reset();
	m_exp->reset();
	m_user->reset();
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_START( ntsc, c64_state )
	// basic hardware
	MCFG_CPU_ADD(M6510_TAG, M6510, VIC6567_CLOCK)
	MCFG_CPU_PROGRAM_MAP(c64_mem)
	MCFG_CPU_CONFIG(cpu_intf)
	MCFG_CPU_VBLANK_INT(SCREEN_TAG, c64_frame_interrupt)
	MCFG_QUANTUM_PERFECT_CPU(M6510_TAG)

	// video hardware
	MCFG_MOS6567_ADD(MOS6567_TAG, SCREEN_TAG, VIC6567_CLOCK, vic_intf, vic_videoram_map, vic_colorram_map)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6851_TAG, SID6581, VIC6567_CLOCK)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_PLS100_ADD(PLA_TAG)
	MCFG_MOS6526R1_ADD(MOS6526_1_TAG, VIC6567_CLOCK, 60, cia1_intf)
	MCFG_MOS6526R1_ADD(MOS6526_2_TAG, VIC6567_CLOCK, 60, cia2_intf)
	MCFG_QUICKLOAD_ADD("quickload", cbm_c64, "p00,prg,t64", CBM_QUICKLOAD_DELAY_SECONDS)
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, datassette_intf, cbm_datassette_devices, "c1530", NULL)
	MCFG_CBM_IEC_ADD(iec_intf, "c1541")
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, VIC6567_CLOCK, expansion_intf, c64_expansion_cards, NULL, NULL)
	MCFG_C64_USER_PORT_ADD(C64_USER_PORT_TAG, user_intf, c64_user_port_cards, NULL, NULL)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list_vic10", "vic10")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_vic10", "NTSC")
	MCFG_SOFTWARE_LIST_ADD("cart_list_c64", "c64_cart")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_c64", "NTSC")
	MCFG_SOFTWARE_LIST_ADD("disk_list", "c64_flop")
	MCFG_SOFTWARE_LIST_FILTER("disk_list", "NTSC")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pet64 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( pet64, ntsc )
	// TODO monochrome green palette
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( ntsc_sx )
//-------------------------------------------------

static MACHINE_CONFIG_START( ntsc_sx, sx64_state )
	MCFG_FRAGMENT_ADD(ntsc)

	// basic hardware
	MCFG_CPU_MODIFY(M6510_TAG)
	MCFG_CPU_CONFIG(sx64_cpu_intf)

	// devices
	MCFG_DEVICE_REMOVE("iec8")
	MCFG_CBM_IEC_SLOT_ADD("iec8", 8, sx1541_iec_devices, "sx1541", NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( ntsc_dx )
//-------------------------------------------------

static MACHINE_CONFIG_START( ntsc_dx, sx64_state )
	MCFG_FRAGMENT_ADD(ntsc_sx)

	// devices
	MCFG_DEVICE_REMOVE("iec9")
	MCFG_CBM_IEC_SLOT_ADD("iec9", 8, sx1541_iec_devices, "sx1541", NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( ntsc_c )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED_CLASS( ntsc_c, ntsc, c64c_state )
	MCFG_SOUND_REPLACE(MOS6851_TAG, SID8580, VIC6567_CLOCK)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pal )
//-------------------------------------------------

static MACHINE_CONFIG_START( pal, c64_state )
	// basic hardware
	MCFG_CPU_ADD(M6510_TAG, M6510, VIC6569_CLOCK)
	MCFG_CPU_PROGRAM_MAP(c64_mem)
	MCFG_CPU_CONFIG(cpu_intf)
	MCFG_CPU_VBLANK_INT(SCREEN_TAG, c64_frame_interrupt)
	MCFG_QUANTUM_PERFECT_CPU(M6510_TAG)

	// video hardware
	MCFG_MOS6569_ADD(MOS6569_TAG, SCREEN_TAG, VIC6569_CLOCK, vic_intf, vic_videoram_map, vic_colorram_map)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6851_TAG, SID6581, VIC6569_CLOCK)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_PLS100_ADD(PLA_TAG)
	MCFG_MOS6526R1_ADD(MOS6526_1_TAG, VIC6569_CLOCK, 50, cia1_intf)
	MCFG_MOS6526R1_ADD(MOS6526_2_TAG, VIC6569_CLOCK, 50, cia2_intf)
	MCFG_QUICKLOAD_ADD("quickload", cbm_c64, "p00,prg,t64", CBM_QUICKLOAD_DELAY_SECONDS)
	MCFG_PET_DATASSETTE_PORT_ADD(PET_DATASSETTE_PORT_TAG, datassette_intf, cbm_datassette_devices, "c1530", NULL)
	MCFG_CBM_IEC_ADD(iec_intf, "c1541")
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, VIC6569_CLOCK, expansion_intf, c64_expansion_cards, NULL, NULL)
	MCFG_C64_USER_PORT_ADD(C64_USER_PORT_TAG, user_intf, c64_user_port_cards, NULL, NULL)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list_vic10", "vic10")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_vic10", "PAL")
	MCFG_SOFTWARE_LIST_ADD("cart_list_c64", "c64_cart")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_c64", "PAL")
	MCFG_SOFTWARE_LIST_ADD("disk_list", "c64_flop")
	MCFG_SOFTWARE_LIST_FILTER("disk_list", "PAL")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pal_sx )
//-------------------------------------------------

static MACHINE_CONFIG_START( pal_sx, sx64_state )
	MCFG_FRAGMENT_ADD(pal)

	// basic hardware
	MCFG_CPU_MODIFY(M6510_TAG)
	MCFG_CPU_CONFIG(sx64_cpu_intf)

	// devices
	MCFG_DEVICE_REMOVE("iec8")
	MCFG_CBM_IEC_SLOT_ADD("iec8", 8, sx1541_iec_devices, "sx1541", NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pal_c )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED_CLASS( pal_c, pal, c64c_state )
	MCFG_SOUND_REPLACE(MOS6851_TAG, SID8580, VIC6569_CLOCK)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( pal_gs )
//-------------------------------------------------

static MACHINE_CONFIG_START( pal_gs, c64gs_state )
	// basic hardware
	MCFG_CPU_ADD(M6510_TAG, M6510, VIC6569_CLOCK)
	MCFG_CPU_PROGRAM_MAP(c64_mem)
	MCFG_CPU_CONFIG(c64gs_cpu_intf)
	MCFG_CPU_VBLANK_INT(SCREEN_TAG, c64_frame_interrupt)
	MCFG_QUANTUM_PERFECT_CPU(M6510_TAG)

	// video hardware
	MCFG_MOS8565_ADD(MOS6569_TAG, SCREEN_TAG, VIC6569_CLOCK, vic_intf, vic_videoram_map, vic_colorram_map)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(MOS6851_TAG, SID8580, VIC6569_CLOCK)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_PLS100_ADD(PLA_TAG)
	MCFG_MOS6526R1_ADD(MOS6526_1_TAG, VIC6569_CLOCK, 50, cia1_intf)
	MCFG_MOS6526R1_ADD(MOS6526_2_TAG, VIC6569_CLOCK, 50, cia2_intf)
	MCFG_CBM_IEC_BUS_ADD(iec_intf)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, NULL, NULL)
	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, VIC6569_CLOCK, expansion_intf, c64_expansion_cards, NULL, NULL)
	MCFG_C64_USER_PORT_ADD(C64_USER_PORT_TAG, user_intf, c64_user_port_cards, NULL, NULL)

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list_vic10", "vic10")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_vic10", "PAL")
	MCFG_SOFTWARE_LIST_ADD("cart_list_c64", "c64_cart")
	MCFG_SOFTWARE_LIST_FILTER("cart_list_c64", "PAL")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( c64n )
//-------------------------------------------------

ROM_START( c64n )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.u3", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x4000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r3")
	ROM_SYSTEM_BIOS(0, "r1", "Kernal rev. 1" )
	ROMX_LOAD( "901227-01.u4", 0x0000, 0x2000, CRC(dce782fa) SHA1(87cc04d61fc748b82df09856847bb5c2754a2033), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "r2", "Kernal rev. 2" )
	ROMX_LOAD( "901227-02.u4", 0x0000, 0x2000, CRC(a5c687b3) SHA1(0e2e4ee3f2d41f00bed72f9ab588b83e306fdb13), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "r3", "Kernal rev. 3" )
	ROMX_LOAD( "901227-03.u4", 0x0000, 0x2000, CRC(dbe3e7c7) SHA1(1d503e56df85a62fee696e7618dc5b4e781df1bb), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos c64.u4", 0x0000, 0x2000, CRC(2f79984c) SHA1(31e73e66eccb28732daea8ec3ad1addd9b39a017), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(4, "speeddos", "SpeedDOS" )
	ROMX_LOAD( "speed-dos.u4", 0x0000, 0x2000, CRC(5beb9ac8) SHA1(8896c8de9e26ef1396eb46020b2de346a3eeab7e), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS(5, "speeddos20", "SpeedDOS-Plus+ v2.0" )
	ROMX_LOAD( "speed-dosplus.u4", 0x0000, 0x2000, CRC(10aee0ae) SHA1(6cebd4dc0c5e8c0b073586a3f1c43cc3349b9736), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS(6, "speeddos27", "SpeedDOS-Plus+ v2.7" )
	ROMX_LOAD( "speed-dosplus27.u4", 0x0000, 0x2000, CRC(ff59995e) SHA1(c8d864e5fc7089af8afce97dc0a0224df11df1c3), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS(7, "prodos", "Professional-DOS v1" )
	ROMX_LOAD( "prodos.u4", 0x0000, 0x2000, CRC(37ed83a2) SHA1(35f4f0fe03c0b7b3762b526ba855de41b496fb60), ROM_BIOS(8) )
	ROM_SYSTEM_BIOS(8, "prodos", "Professional-DOS Release 2/4L2" )
	ROMX_LOAD( "prodos24l2.u4", 0x0000, 0x2000, CRC(41dad9fe) SHA1(fbf3dcc2ed40e58b07595740ea6fbff7ab19ebad), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS(9, "prodos", "Professional-DOS Release 3/5L2" )
	ROMX_LOAD( "prodos35l2.u4", 0x0000, 0x2000, CRC(2822eee7) SHA1(77356b84c1648018863d1c8dd5bc3a37485bc00e), ROM_BIOS(10) )
	ROM_SYSTEM_BIOS(10, "turborom", "Cockroach Turbo-ROM" )
	ROMX_LOAD( "turborom.u4", 0x0000, 0x2000, CRC(e6c763a2) SHA1(eff5a4b6bc65daa9421bd3856dd99a3195068e1c), ROM_BIOS(11) )
	ROM_SYSTEM_BIOS(11, "dosrom", "DOS-ROM v1.2" )
	ROMX_LOAD( "dosrom12.u4", 0x0000, 0x2000, CRC(ac030fc0) SHA1(0e4b38e81b49f55d52162154a44f0fffd2b0d04f), ROM_BIOS(12) )
	ROM_SYSTEM_BIOS(12, "turborom2", "Datel Turbo ROM II (PAL)" )
	ROMX_LOAD( "turborom2.u4", 0x0000, 0x2000, CRC(ea3ba683) SHA1(4bb23f764a3d255119fbae37202ca820caa04e1f), ROM_BIOS(13) )
	ROM_SYSTEM_BIOS(13, "mercury", "Mercury-ROM v3 (NTSC)" )
	ROMX_LOAD( "mercury3.u4", 0x0000, 0x2000, CRC(6eac46a2) SHA1(4e351aa5fcb97c4c21e565aa2c830cc09bd47533), ROM_BIOS(14) )
	ROM_SYSTEM_BIOS(14, "dolphin", "Dolphin-DOS 1.0" )
	ROMX_LOAD( "kernal-10-mager.u4", 0x0000, 0x2000, CRC(c9bb21bc) SHA1(e305216e50ff8a7acf102be6c6343e3d44a16233), ROM_BIOS(15) )
	ROM_SYSTEM_BIOS(15, "dolphin201au", "Dolphin-DOS 2.0 1 au" )
	ROMX_LOAD( "kernal-20-1_au.u4", 0x0000, 0x2000, CRC(7068bbcc) SHA1(325ce7e32609a8fc704aaa76f5eb4cd7d8099a92), ROM_BIOS(16) )
	ROM_SYSTEM_BIOS(16, "dolphin201", "Dolphin-DOS 2.0 1" )
	ROMX_LOAD( "kernal-20-1.u4", 0x0000, 0x2000, CRC(c9c4c44e) SHA1(7f5d8f08c5ed2182ffb415a3d777fdd922496d02), ROM_BIOS(17) )
	ROM_SYSTEM_BIOS(17, "dolphin202", "Dolphin-DOS 2.0 2" )
	ROMX_LOAD( "kernal-20-2.u4", 0x0000, 0x2000, CRC(ffaeb9bc) SHA1(5f6c1bad379da16f77bccb58e80910f307dfd5f8), ROM_BIOS(18) )
	ROM_SYSTEM_BIOS(18, "dolphin203", "Dolphin-DOS 2.0 3" )
	ROMX_LOAD( "kernal-20-3.u4", 0x0000, 0x2000, CRC(4fd511f2) SHA1(316fba280dcb29496d593c0c4e3ee9a19844054e), ROM_BIOS(19) )
	ROM_SYSTEM_BIOS(19, "dolphin30", "Dolphin-DOS 3.0" )
	ROMX_LOAD( "kernal-30.u4", 0x0000, 0x2000, CRC(5402d643) SHA1(733acb96fead2fb4df77840c5bb618f08439fc7e), ROM_BIOS(20) )
	ROM_SYSTEM_BIOS(20, "taccess", "TurboAccess v2.6" )
	ROMX_LOAD( "turboaccess26.u4", 0x0000, 0x2000, CRC(93de6cd9) SHA1(a74478f3b9153c13176eac80ebfacc512ae7cbf0), ROM_BIOS(21) )
	ROM_SYSTEM_BIOS(21, "ttrans301", "TurboTrans v3.0 1" )
	ROMX_LOAD( "turboaccess301.u4", 0x0000, 0x2000, CRC(b3304dcf) SHA1(4d47a265ef65e4823f862cfc3d514c2a71473580), ROM_BIOS(22) )
	ROM_SYSTEM_BIOS(22, "ttrans302", "TurboTrans v3.0 2" )
	ROMX_LOAD( "turboaccess302.u4", 0x0000, 0x2000, CRC(9e696a7b) SHA1(5afae75d66d539f4bb4af763f029f0ef6523a4eb), ROM_BIOS(23) )
	ROM_SYSTEM_BIOS(23, "tprocess", "Turbo-Process (PAL)" )
	ROMX_LOAD( "turboprocess.u4", 0x0000, 0x2000, CRC(e5610d76) SHA1(e3f35777cfd16cce4717858f77ff354763395ba9), ROM_BIOS(24) )
	ROM_SYSTEM_BIOS(24, "tprocess", "Turbo-Process (NTSC)" )
	ROMX_LOAD( "turboprocessus.u4", 0x0000, 0x2000, CRC(7480b76a) SHA1(ef1664b5057ae3cc6d104fc2f5c1fb29ee5a1b2b), ROM_BIOS(25) )
	ROM_SYSTEM_BIOS(25, "exos3", "EXOS v3" )
	ROMX_LOAD( "exos3.u4", 0x0000, 0x2000, CRC(4e54d020) SHA1(f8931b7c0b26807f4de0cc241f0b1e2c8f5271e9), ROM_BIOS(26) )
	ROM_SYSTEM_BIOS(26, "exos4", "EXOS v4" )
	ROMX_LOAD( "exos4.u4", 0x0000, 0x2000, CRC(d5cf83a9) SHA1(d5f03a5c0e9d00032d4751ecc6bcd6385879c9c7), ROM_BIOS(27) )
	ROM_SYSTEM_BIOS(27, "pdc", "ProLogic-DOS Classic" )
	ROMX_LOAD( "pdc.u4", 0x0000, 0x4000, CRC(6b653b9c) SHA1(0f44a9c62619424a0cd48a90e1b377b987b494e0), ROM_BIOS(28) )
	ROM_SYSTEM_BIOS(28, "digidos", "DigiDOS" )
	ROMX_LOAD( "digidos.u4", 0x0000, 0x2000, CRC(2b0c8e89) SHA1(542d6f61c318bced0642e7c2d4d3b34a0f13e634), ROM_BIOS(29) )
	ROM_SYSTEM_BIOS(29, "magnum", "Magnum Load" )
	ROMX_LOAD( "magnum.u4", 0x0000, 0x2000, CRC(b2cffcc6) SHA1(827c782c1723b5d0992c05c00738ae4b2133b641), ROM_BIOS(30) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u5", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64j )
//-------------------------------------------------

ROM_START( c64j )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.u3", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "906145-02.u4", 0x0000, 0x2000, CRC(3a9ef6f1) SHA1(4ff0f11e80f4b57430d8f0c3799ed0f0e0f4565d) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "906143-02.u5", 0x0000, 0x1000, CRC(1604f6c1) SHA1(0fad19dbcdb12461c99657b2979dbb5c2e47b527) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64p )
//-------------------------------------------------

#define rom_c64p rom_c64n


//-------------------------------------------------
//  ROM( c64sw )
//-------------------------------------------------

ROM_START( c64sw )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.u3", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "kernel.u4",	0x0000, 0x2000, CRC(f10c2c25) SHA1(e4f52d9b36c030eb94524eb49f6f0774c1d02e5e) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_SYSTEM_BIOS( 0, "default", "Swedish Characters" )
	ROMX_LOAD( "charswe.u5", 0x0000, 0x1000, CRC(bee9b3fd) SHA1(446ae58f7110d74d434301491209299f66798d8a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "alt", "Swedish Characters (Alt)" )
	ROMX_LOAD( "charswe2.u5", 0x0000, 0x1000, CRC(377a382b) SHA1(20df25e0ba1c88f31689c1521397c96968967fac), ROM_BIOS(2) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( pet64 )
//-------------------------------------------------

ROM_START( pet64 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.u3", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "901246-01.u4", 0x0000, 0x2000, CRC(789c8cc5) SHA1(6c4fa9465f6091b174df27dfe679499df447503c) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u5", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.u17", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( edu64 )
//-------------------------------------------------

#define rom_edu64	rom_c64n


//-------------------------------------------------
//  ROM( sx64n )
//-------------------------------------------------

ROM_START( sx64n )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.ud4", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_SYSTEM_BIOS(0, "cbm", "Original" )
	ROMX_LOAD( "251104-04.ud3", 0x0000, 0x2000, CRC(2c5965d4) SHA1(aa136e91ecf3c5ac64f696b3dbcbfc5ba0871c98), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos sx64.ud3", 0x0000, 0x2000, CRC(2b5a88f5) SHA1(942c2150123dc30f40b3df6086132ef0a3c43948), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "1541flash", "1541 FLASH!" )
	ROMX_LOAD( "1541 flash.ud3", 0x0000, 0x2000, CRC(0a1c9b85) SHA1(0bfcaab0ae453b663a6e01cd59a9764805419e00), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "turborom", "Cockroach Turbo-ROM" )
	ROMX_LOAD( "turboromsx.u4", 0x0000, 0x2000, CRC(48579c30) SHA1(6c907fdd07c14e162eb8c8fb750b1bbaf69dccb4), ROM_BIOS(4) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.ud1", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.ue4", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( rom_sx64p )
//-------------------------------------------------

#define rom_sx64p	rom_sx64n


//-------------------------------------------------
//  ROM( vip64 )
//-------------------------------------------------

ROM_START( vip64 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901226-01.ud4", 0x0000, 0x2000, CRC(f833d117) SHA1(79015323128650c742a3694c9429aa91f355905e) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "kernelsx.ud3", 0x0000, 0x2000, CRC(7858d3d7) SHA1(097cda60469492a8916c2677b7cce4e12a944bc0) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "charswe.ud1", 0x0000, 0x1000, CRC(bee9b3fd) SHA1(446ae58f7110d74d434301491209299f66798d8a) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "906114-01.ue4", 0x00, 0xf5, CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( dx64 )
//-------------------------------------------------

// ROM_LOAD( "dx64kern.bin", 0x0000, 0x2000, CRC(58065128) ) TODO where is this illusive ROM?
#define rom_dx64	rom_sx64n


//-------------------------------------------------
//  ROM( c64cn )
//-------------------------------------------------

ROM_START( c64cn )
	ROM_REGION( 0x4000, M6510_TAG, 0 )
	ROM_LOAD( "251913-01.u4", 0x0000, 0x4000, CRC(0010ec31) SHA1(765372a0e16cbb0adf23a07b80f6b682b39fbf88) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u5", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "252715-01.u8", 0x00, 0xf5, BAD_DUMP CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64cp )
//-------------------------------------------------

#define rom_c64cp		rom_c64cn


//-------------------------------------------------
//  ROM( c64g )
//-------------------------------------------------

#define rom_c64g		rom_c64cn


//-------------------------------------------------
//  ROM( c64csw )
//-------------------------------------------------

ROM_START( c64csw )
	ROM_REGION( 0x4000, M6510_TAG, 0 )
	ROM_LOAD( "325182-01.u4", 0x0000, 0x4000, CRC(2aff27d3) SHA1(267654823c4fdf2167050f41faa118218d2569ce) ) // 128/64 FI

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "cbm 64 skand.gen.u5", 0x0000, 0x1000, CRC(377a382b) SHA1(20df25e0ba1c88f31689c1521397c96968967fac) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "252715-01.u8", 0x00, 0xf5, BAD_DUMP CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END


//-------------------------------------------------
//  ROM( c64gs )
//-------------------------------------------------

ROM_START( c64gs )
	ROM_REGION( 0x4000, M6510_TAG, 0 )
	ROM_LOAD( "390852-01.u4", 0x0000, 0x4000, CRC(b0a9c2da) SHA1(21940ef5f1bfe67d7537164f7ca130a1095b067a) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901225-01.u5", 0x0000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "252535-01.u8", 0x00, 0xf5, BAD_DUMP CRC(54c89351) SHA1(efb315f560b6f72444b8f0b2ca4b0ccbcd144a1b) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT   INIT                        COMPANY                        FULLNAME                                     FLAGS
COMP( 1982,	c64n,	0,  	0,		ntsc,		c64,	driver_device,		0,		"Commodore Business Machines", "Commodore 64 (NTSC)",						GAME_SUPPORTS_SAVE )
COMP( 1982,	c64j,	c64n,	0,		ntsc,		c64,	driver_device,		0,		"Commodore Business Machines", "Commodore 64 (Japan)",						GAME_SUPPORTS_SAVE )
COMP( 1982,	c64p,	c64n,	0,		pal,		c64,	driver_device,		0,		"Commodore Business Machines", "Commodore 64 (PAL)",						GAME_SUPPORTS_SAVE )
COMP( 1982,	c64sw,	c64n,	0,		pal,		c64sw,	driver_device,		0,		"Commodore Business Machines", "Commodore 64 / VIC-64S (Sweden/Finland)",	GAME_SUPPORTS_SAVE )
COMP( 1983, pet64,	c64n,	0,  	pet64,  	c64,	driver_device,		0,  	"Commodore Business Machines", "PET 64 / CBM 4064 (NTSC)",					GAME_SUPPORTS_SAVE | GAME_WRONG_COLORS )
COMP( 1983, edu64,  c64n,	0,  	pet64,  	c64,	driver_device,		0,  	"Commodore Business Machines", "Educator 64 (NTSC)",						GAME_SUPPORTS_SAVE | GAME_WRONG_COLORS )
COMP( 1984, sx64n,	c64n,	0,		ntsc_sx,	c64,	driver_device,		0,		"Commodore Business Machines", "SX-64 / Executive 64 (NTSC)",				GAME_SUPPORTS_SAVE )
COMP( 1984, sx64p,	c64n,	0,		pal_sx,		c64,	driver_device,		0,		"Commodore Business Machines", "SX-64 / Executive 64 (PAL)",				GAME_SUPPORTS_SAVE )
COMP( 1984, vip64,	c64n,	0,		pal_sx,		c64sw,	driver_device,		0,		"Commodore Business Machines", "VIP-64 (Sweden/Finland)",					GAME_SUPPORTS_SAVE )
COMP( 1984, dx64,	c64n,	0,		ntsc_dx,	c64,	driver_device,		0,		"Commodore Business Machines", "DX-64 (NTSC)",								GAME_SUPPORTS_SAVE )
//COMP(1983, clipper,  c64,  0, c64pal,  clipper, XXX_CLASS, c64pal,  "PDC", "Clipper", GAME_NOT_WORKING) // C64 in a briefcase with 3" floppy, electroluminescent flat screen, thermal printer
//COMP(1983, tesa6240, c64,  0, c64pal,  c64, XXX_CLASS,     c64pal,  "Tesa", "6240", GAME_NOT_WORKING) // modified SX64 with label printer
COMP( 1986, c64cn,	c64n,	0,  	ntsc_c,		c64,	driver_device,		0,		"Commodore Business Machines", "Commodore 64C (NTSC)",						GAME_SUPPORTS_SAVE )
COMP( 1986, c64cp,	c64n,	0,  	pal_c,		c64,	driver_device,		0,		"Commodore Business Machines", "Commodore 64C (PAL)",						GAME_SUPPORTS_SAVE )
COMP( 1986, c64csw,	c64n,	0,  	pal_c,		c64sw,	driver_device,		0,		"Commodore Business Machines", "Commodore 64C (Sweden/Finland)",			GAME_SUPPORTS_SAVE )
COMP( 1986, c64g,	c64n,	0,		pal_c,		c64,	driver_device,		0,		"Commodore Business Machines", "Commodore 64G (PAL)",						GAME_SUPPORTS_SAVE )
CONS( 1990, c64gs,	c64n,	0,		pal_gs,		c64gs,	driver_device,		0,		"Commodore Business Machines", "Commodore 64 Games System (PAL)",			GAME_SUPPORTS_SAVE )
