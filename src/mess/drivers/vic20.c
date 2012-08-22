/*

[CBM systems which belong to this driver (info to be moved to sysinfo.dat soon)]
(most of the informations are taken from http://www.zimmers.net/cbmpics/ )


* VIC-1001 (1981, Japan)

  The first model released was the Japanese one. It featured support for the
Japanese katakana character.

CPU: MOS Technology 6502 (1.01 MHz)
RAM: 5 kilobytes (Expanded to 21k though an external 16k unit)
ROM: 20 kilobytes
Video: MOS Technology 6560 "VIC"(Text: 22 columns, 23 rows; Hires: 176x184
pixels bitmapped; 8 text colors, 16 background colors)
Sound: MOS Technology 6560 "VIC" (3 voices -square wave-, noise and volume)
Ports: 6522 VIA x2 (1 Joystick/Mouse port; CBM Serial port; 'Cartridge /
    Game / Expansion' port; CBM Monitor port; CBM 'USER' port; Power and
    reset switches; Power connector)
Keyboard: Full-sized QWERTY 66 key (8 programmable function keys; 2 sets of
    Keyboardable graphic characters; 2 key direction cursor-pad)


* VIC 20 (1981)

  This system was the first computer to sell more than one million units
worldwide. It was sold both in Europe and in the US. In Germany the
computer was renamed as VC 20 (apparently, it stands for 'VolksComputer'

CPU: MOS Technology 6502A (1.01 MHz)
RAM: 5 kilobytes (Expanded to 32k)
ROM: 20 kilobytes
Video: MOS Technology 6560 "VIC"(Text: 22 columns, 23 rows; Hires: 176x184
pixels bitmapped; 8 text colors, 16 background colors)
Sound: MOS Technology 6560 "VIC" (3 voices -square wave-, noise and volume)
Ports: 6522 VIA x2 (1 Joystick/Mouse port; CBM Serial port; 'Cartridge /
    Game / Expansion' port; CBM Monitor port; CBM 'USER' port; Power and
    reset switches; Power connector)
Keyboard: Full-sized QWERTY 66 key (8 programmable function keys; 2 sets of
    Keyboardable graphic characters; 2 key direction cursor-pad)


* VIC 21 (1983)

  It consists of a VIC 20 with built-in RAM expansion, to reach a RAM
  capability of 21 kilobytes.


* VIC 20CR

  CR stands for Cost Reduced, as it consisted of a board with only 2 (larger)
block of RAM instead of 8.

*******************************************************************************

    TODO:

    - C1540 is not working currently
    - access violation in mos6560.c
        * In the Chips (Japan, USA).60
        * K-Star Patrol (Europe).60
        * Seafox (Japan, USA).60
    - mos6560_port_r/w should respond at 0x1000-0x100f
    - SHIFT LOCK
    - restore key
    - light pen
    - VIC21 (built in 21K ram)

*/

#include "includes/vic20.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

enum
{
	BLK0 = 0,
	BLK1,
	BLK2,
	BLK3,
	BLK4,
	BLK5,
	BLK6,
	BLK7
};


enum
{
	RAM0 = 0,
	RAM1,
	RAM2,
	RAM3,
	RAM4,
	RAM5,
	RAM6,
	RAM7
};


enum
{
	IO0 = 4,
	COLOR = 5,
	IO2 = 6,
	IO3 = 7
};



//**************************************************************************
//  MEMORY MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( vic20_state::read )
{
	UINT8 data = 0;

	int ram1 = 1, ram2 = 1, ram3 = 1;
	int blk1 = 1, blk2 = 1, blk3 = 1, blk5 = 1;
	int io2 = 1, io3 = 1;

	switch ((offset >> 13) & 0x07)
	{
	case BLK0:
		switch ((offset >> 10) & 0x07)
		{
		case RAM0:
			data = m_ram->pointer()[offset];
			break;

		case RAM1: ram1 = 0; break;
		case RAM2: ram2 = 0; break;
		case RAM3: ram3 = 0; break;

		default:
			data = m_ram->pointer()[0x400 + (offset & 0xfff)];
			break;
		}
		break;

	case BLK1: blk1 = 0; break;
	case BLK2: blk2 = 0; break;
	case BLK3: blk3 = 0; break;

	case BLK4:
		switch ((offset >> 10) & 0x07)
		{
		default:
			data = m_charom[offset & 0xfff];
			break;

		case IO0:
			if (BIT(offset, 4))
			{
				data = m_via0->read(space, offset & 0x0f);
			}
			else if (BIT(offset, 5))
			{
				data = m_via1->read(space, offset & 0x0f);
			}
			else if (offset >= 0x9000 && offset < 0x9010)
			{
				data = mos6560_port_r(m_vic, offset & 0x0f);
			}
			break;

		case COLOR:
			data = m_color_ram[offset & 0x3ff];
			break;

		case IO2: io2 = 0; break;
		case IO3: io3 = 0; break;
		}
		break;

	case BLK5: blk5 = 0; break;

	case BLK6:
		data = m_basic[offset & 0x1fff];
		break;

	case BLK7:
		data = m_kernal[offset & 0x1fff];
		break;
	}

	data |= m_exp->cd_r(space, offset & 0x1fff, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( vic20_state::write )
{
	int ram1 = 1, ram2 = 1, ram3 = 1;
	int blk1 = 1, blk2 = 1, blk3 = 1, blk5 = 1;
	int io2 = 1, io3 = 1;

	switch ((offset >> 13) & 0x07)
	{
	case BLK0:
		switch ((offset >> 10) & 0x07)
		{
		case RAM0:
			m_ram->pointer()[offset] = data;
			break;

		case RAM1: ram1 = 0; break;
		case RAM2: ram2 = 0; break;
		case RAM3: ram3 = 0; break;

		default:
			m_ram->pointer()[0x400 + (offset & 0xfff)] = data;
			break;
		}
		break;

	case BLK1: blk1 = 0; break;
	case BLK2: blk2 = 0; break;
	case BLK3: blk3 = 0; break;

	case BLK4:
		switch ((offset >> 10) & 0x07)
		{
		case IO0:
			if (BIT(offset, 4))
			{
				m_via0->write(space, offset & 0x0f, data);
			}
			else if (BIT(offset, 5))
			{
				m_via1->write(space, offset & 0x0f, data);
			}
			else if (offset >= 0x9000 && offset < 0x9010)
			{
				mos6560_port_w(m_vic, offset & 0x0f, data);
			}
			break;

		case COLOR:
			m_color_ram[offset & 0x3ff] = data;
			break;

		case IO2: io2 = 0; break;
		case IO3: io3 = 0; break;
		}
		break;

	case BLK5: blk5 = 0; break;
	}

	m_exp->cd_w(space, offset & 0x1fff, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
}



//**************************************************************************
//  VIDEO
//**************************************************************************

static const unsigned char mos6560_palette[] =
{
// ripped from vice, a very excellent emulator
// black, white, red, cyan
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0xf0,
// purple, green, blue, yellow
	0x60, 0x00, 0x60, 0x00, 0xa0, 0x00, 0x00, 0x00, 0xf0, 0xd0, 0xd0, 0x00,
// orange, light orange, pink, light cyan,
	0xc0, 0xa0, 0x00, 0xff, 0xa0, 0x00, 0xf0, 0x80, 0x80, 0x00, 0xff, 0xff,
// light violett, light green, light blue, light yellow
	0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0xa0, 0xff, 0xff, 0xff, 0x00
};

static PALETTE_INIT( vic20 )
{
	int i;

	for (i = 0; i < sizeof(mos6560_palette) / 3; i++)
	{
		palette_set_color_rgb(machine, i, mos6560_palette[i * 3], mos6560_palette[i * 3 + 1], mos6560_palette[i * 3 + 2]);
	}
}

static SCREEN_UPDATE_IND16( vic20 )
{
	vic20_state *state = screen.machine().driver_data<vic20_state>();
	mos6560_video_update(state->m_vic, bitmap, cliprect);
	return 0;
}

static INTERRUPT_GEN( vic20_raster_interrupt )
{
	vic20_state *state = device->machine().driver_data<vic20_state>();
	mos6560_raster_interrupt_gen(state->m_vic);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( vic20_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( vic20_mem, AS_PROGRAM, 8, vic20_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

#ifdef UNUSED_FUNCTION

//-------------------------------------------------
//  INPUT_PORTS( vic_lightpen_6560 )
//-------------------------------------------------

static INPUT_PORTS_START( vic_lightpen_6560 )
	PORT_START( "LIGHTX" )
	PORT_BIT( 0xff, 0, IPT_PADDLE ) PORT_NAME("Lightpen X Axis") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(2) PORT_MINMAX(0,(MOS6560_MAME_XSIZE - 1)) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START( "LIGHTY" )
	PORT_BIT( 0xff, 0, IPT_PADDLE ) PORT_NAME("Lightpen Y Axis") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(2) PORT_MINMAX(0,(MOS6560_MAME_YSIZE - 1)) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( vic_lightpen_6561 )
//-------------------------------------------------

static INPUT_PORTS_START( vic_lightpen_6561 )
	PORT_START( "LIGHTX" )
	PORT_BIT( 0xff, 0, IPT_PADDLE ) PORT_NAME("Lightpen X Axis") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(2) PORT_MINMAX(0,(MOS6560_MAME_XSIZE - 1)) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START( "LIGHTY" )
	PORT_BIT( 0x1ff, 0, IPT_PADDLE ) PORT_NAME("Lightpen Y Axis") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(2) PORT_MINMAX(0,(MOS6561_MAME_YSIZE - 1)) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)
INPUT_PORTS_END
#endif


//-------------------------------------------------
//  INPUT_PORTS( vic20 )
//-------------------------------------------------

static INPUT_PORTS_START( vic20 )
	PORT_INCLUDE( vic_keyboard )       // ROW0 -> ROW7

	PORT_INCLUDE( vic_special )        // SPECIAL

	PORT_INCLUDE( vic_controls )       // CTRLSEL, JOY, FAKE0, FAKE1, PADDLE0, PADDLE1
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( vic1001 )
//-------------------------------------------------

static INPUT_PORTS_START( vic1001 )
	PORT_INCLUDE( vic20 )

	PORT_MODIFY( "ROW0" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('\xA5')
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( vic20s )
//-------------------------------------------------

static INPUT_PORTS_START( vic20s )
	PORT_INCLUDE( vic20 )

	PORT_MODIFY( "ROW0" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)		PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)			PORT_CHAR('-')

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)		PORT_CHAR('@')

	PORT_MODIFY( "ROW2" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)			PORT_CHAR(0x00C4)

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)		PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)			PORT_CHAR(0x00D6)

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)		PORT_CHAR(0x00C5)

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)			PORT_CHAR('=')
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  via6522_interface via0_intf
//-------------------------------------------------

READ8_MEMBER( vic20_state::via0_pa_r )
{
	/*

        bit     description

        PA0     SERIAL CLK IN
        PA1     SERIAL DATA IN
        PA2     JOY 0
        PA3     JOY 1
        PA4     JOY 2
        PA5     LITE PEN
        PA6     CASS SWITCH
        PA7

    */

	UINT8 data = 0;

	// serial clock in
	data |= m_iec->clk_r();

	// serial data in
	data |= m_iec->data_r() << 1;

	// user port
	data |= m_user->joy0_r() << 2;
	data |= m_user->joy1_r() << 3;
	data |= m_user->joy2_r() << 4;
	data |= m_user->light_pen_r() << 5;
	data |= m_user->cassette_switch_r() << 6;

	// joystick
	data &= ~(ioport("JOY")->read() & 0x3c);

	// cassette switch
	if ((m_cassette->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED)
		data &= ~0x40;

	return data;
}

WRITE8_MEMBER( vic20_state::via0_pa_w )
{
	/*

        bit     description

        PA0
        PA1
        PA2
        PA3
        PA4
        PA5
        PA6
        PA7     SERIAL ATN OUT

    */

	// serial attention out
	m_iec->atn_w(!BIT(data, 7));
}

WRITE_LINE_MEMBER( vic20_state::via0_ca2_w )
{
	if (!state)
	{
		m_cassette->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
		m_cassette_timer->enable(true);
	}
	else
	{
		m_cassette->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
		m_cassette_timer->enable(false);
	}
}

static const via6522_interface via0_intf =
{
	DEVCB_DRIVER_MEMBER(vic20_state, via0_pa_r),
	DEVCB_DEVICE_MEMBER(VIC20_USER_PORT_TAG, vic20_user_port_device, pb_r),
	DEVCB_NULL, // RESTORE
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(vic20_state, via0_pa_w),
	DEVCB_DEVICE_MEMBER(VIC20_USER_PORT_TAG, vic20_user_port_device, pb_w),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(VIC20_USER_PORT_TAG, vic20_user_port_device, cb1_w),
	DEVCB_DRIVER_LINE_MEMBER(vic20_state, via0_ca2_w), // CASS MOTOR
	DEVCB_DEVICE_LINE_MEMBER(VIC20_USER_PORT_TAG, vic20_user_port_device, cb2_w),
	DEVCB_CPU_INPUT_LINE(M6502_TAG, INPUT_LINE_NMI)
};


//-------------------------------------------------
//  via6522_interface via1_intf
//-------------------------------------------------

READ8_MEMBER( vic20_state::via1_pa_r )
{
	/*

        bit     description

        PA0     ROW 0
        PA1     ROW 1
        PA2     ROW 2
        PA3     ROW 3
        PA4     ROW 4
        PA5     ROW 5
        PA6     ROW 6
        PA7     ROW 7

    */

	UINT8 data = 0xff;

	if (!BIT(m_key_col, 0)) data &= ioport("ROW0")->read();
	if (!BIT(m_key_col, 1)) data &= ioport("ROW1")->read();
	if (!BIT(m_key_col, 2)) data &= ioport("ROW2")->read();
	if (!BIT(m_key_col, 3)) data &= ioport("ROW3")->read();
	if (!BIT(m_key_col, 4)) data &= ioport("ROW4")->read();
	if (!BIT(m_key_col, 5)) data &= ioport("ROW5")->read();
	if (!BIT(m_key_col, 6)) data &= ioport("ROW6")->read();
	if (!BIT(m_key_col, 7)) data &= ioport("ROW7")->read();

	return data;
}

READ8_MEMBER( vic20_state::via1_pb_r )
{
	/*

        bit     description

        PB0     COL 0
        PB1     COL 1
        PB2     COL 2
        PB3     COL 3, CASS WRITE
        PB4     COL 4
        PB5     COL 5
        PB6     COL 6
        PB7     COL 7, JOY 3

    */

	UINT8 data = 0xff;

	// joystick
	data &= ~(ioport("JOY")->read() & 0x80);

	return data;
}

WRITE8_MEMBER( vic20_state::via1_pb_w )
{
	/*

        bit     description

        PB0     COL 0
        PB1     COL 1
        PB2     COL 2
        PB3     COL 3, CASS WRITE
        PB4     COL 4
        PB5     COL 5
        PB6     COL 6
        PB7     COL 7, JOY 3

    */

	// cassette write
	m_cassette->output(BIT(data, 3) ? -(0x5a9e >> 1) : +(0x5a9e >> 1));

	// keyboard column
	m_key_col = data;
}

WRITE_LINE_MEMBER( vic20_state::via1_ca2_w )
{
	// serial clock out
	m_iec->clk_w(!state);
}

WRITE_LINE_MEMBER( vic20_state::via1_cb2_w )
{
	// serial data out
	m_iec->data_w(!state);
}

static const via6522_interface via1_intf =
{
	DEVCB_DRIVER_MEMBER(vic20_state, via1_pa_r),
	DEVCB_DRIVER_MEMBER(vic20_state, via1_pb_r),
	DEVCB_NULL, // CASS READ
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(vic20_state, via1_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(vic20_state, via1_ca2_w),
	DEVCB_DRIVER_LINE_MEMBER(vic20_state, via1_cb2_w),

	DEVCB_CPU_INPUT_LINE(M6502_TAG, M6502_IRQ_LINE)
};


//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK( cassette_tick )
//-------------------------------------------------

static TIMER_DEVICE_CALLBACK( cassette_tick )
{
	vic20_state *state = timer.machine().driver_data<vic20_state>();
	int data = (state->m_cassette->input() > +0.0) ? 1 : 0;

	state->m_via1->write_ca1(data);
}


//-------------------------------------------------
//  CBM_IEC_INTERFACE( cbm_iec_intf )
//-------------------------------------------------

static CBM_IEC_INTERFACE( cbm_iec_intf )
{
	DEVCB_DEVICE_LINE_MEMBER(M6522_1_TAG, via6522_device, write_cb1),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  mos6560_interface vic_ntsc_intf
//-------------------------------------------------

#define VC20ADDR2MOS6560ADDR(a) (((a) > 0x8000) ? ((a) & 0x1fff) : ((a) | 0x2000))
#define MOS6560ADDR2VC20ADDR(a) (((a) > 0x2000) ? ((a) & 0x1fff) : ((a) | 0x8000))

static int vic20_dma_read_color( running_machine &machine, int offset )
{
	vic20_state *state = machine.driver_data<vic20_state>();

	return state->m_color_ram[offset & 0x3ff];
}

static int vic20_dma_read( running_machine &machine, int offset )
{
	address_space *program = machine.device(M6502_TAG)->memory().space(AS_PROGRAM);

	return program->read_byte(MOS6560ADDR2VC20ADDR(offset));
}

static UINT8 vic20_lightx_cb( running_machine &machine )
{
	return (machine.root_device().ioport("LIGHTX")->read_safe(0) & ~0x01);
}

static UINT8 vic20_lighty_cb( running_machine &machine )
{
	return (machine.root_device().ioport("LIGHTY")->read_safe(0) & ~0x01);
}

static UINT8 vic20_lightbut_cb( running_machine &machine )
{
	return (((machine.root_device().ioport("CTRLSEL")->read() & 0xf0) == 0x20) && (machine.root_device().ioport("JOY")->read() & 0x40));
}

static UINT8 vic20_paddle0_cb( running_machine &machine )
{
	return machine.root_device().ioport("PADDLE0")->read();
}

static UINT8 vic20_paddle1_cb( running_machine &machine )
{
	return machine.root_device().ioport("PADDLE1")->read();
}

static const mos6560_interface vic_ntsc_intf =
{
	SCREEN_TAG,
	MOS6560,
	vic20_lightx_cb, vic20_lighty_cb, vic20_lightbut_cb,	// lightgun cb
	vic20_paddle0_cb, vic20_paddle1_cb,		// paddle cb
	vic20_dma_read, vic20_dma_read_color	// DMA
};


//-------------------------------------------------
//  mos6560_interface vic_pal_intf
//-------------------------------------------------

static const mos6560_interface vic_pal_intf =
{
	SCREEN_TAG,
	MOS6561,
	vic20_lightx_cb, vic20_lighty_cb, vic20_lightbut_cb,	// lightgun cb
	vic20_paddle0_cb, vic20_paddle1_cb,		// paddle cb
	vic20_dma_read, vic20_dma_read_color	// DMA
};


//-------------------------------------------------
//  VIC20_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( vic20_state::exp_reset_w )
{
	if (state == ASSERT_LINE)
	{
		machine_reset();
	}
}

static VIC20_EXPANSION_INTERFACE( expansion_intf )
{
	DEVCB_CPU_INPUT_LINE(M6502_TAG, INPUT_LINE_IRQ0),
	DEVCB_CPU_INPUT_LINE(M6502_TAG, INPUT_LINE_NMI),
	DEVCB_DRIVER_LINE_MEMBER(vic20_state, exp_reset_w)
};


//-------------------------------------------------
//  VIC20_USER_PORT_INTERFACE( user_intf )
//-------------------------------------------------

static VIC20_USER_PORT_INTERFACE( user_intf )
{
	DEVCB_DEVICE_LINE_MEMBER(M6522_0_TAG, via6522_device, write_cb1),
	DEVCB_DEVICE_LINE_MEMBER(M6522_0_TAG, via6522_device, write_cb2),
	DEVCB_DRIVER_LINE_MEMBER(vic20_state, exp_reset_w)
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( vic20 )
//-------------------------------------------------

void vic20_state::machine_start()
{
	// find memory regions
	m_basic = memregion("basic")->base();
	m_kernal = memregion("kernal")->base();
	m_charom = memregion("charom")->base();

	// allocate memory
	m_color_ram = auto_alloc_array(machine(), UINT8, 0x400);

	// state saving
	save_item(NAME(m_key_col));
}


//-------------------------------------------------
//  MACHINE_RESET( vic20 )
//-------------------------------------------------

void vic20_state::machine_reset()
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
//  MACHINE_CONFIG( vic20_common )
//-------------------------------------------------

static MACHINE_CONFIG_START( vic20_common, vic20_state )
	MCFG_TIMER_ADD_PERIODIC(TIMER_C1530_TAG, cassette_tick, attotime::from_hz(44100))

	// devices
	MCFG_VIA6522_ADD(M6522_0_TAG, 0, via0_intf)
	MCFG_VIA6522_ADD(M6522_1_TAG, 0, via1_intf)

	MCFG_QUICKLOAD_ADD("quickload", cbm_vc20, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, cbm_cassette_interface )

	MCFG_CBM_IEC_ADD(cbm_iec_intf, "c1541")

	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vic20_control_port_devices, NULL, NULL)
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vic20_control_port_devices, NULL, NULL)
	MCFG_VIC20_USER_PORT_ADD(VIC20_USER_PORT_TAG, user_intf, vic20_user_port_cards, NULL, NULL)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cart_list", "vic1001_cart")
	MCFG_SOFTWARE_LIST_ADD("disk_list", "vic1001_flop")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("5K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( vic20_ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( vic20_ntsc, vic20_common )
	// basic machine hardware
	MCFG_CPU_ADD(M6502_TAG, M6502, MOS6560_CLOCK)
	MCFG_CPU_PROGRAM_MAP(vic20_mem)
	MCFG_CPU_PERIODIC_INT(vic20_raster_interrupt, MOS656X_HRETRACERATE)

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(MOS6560_VRETRACERATE)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_SIZE((MOS6560_XSIZE + 7) & ~7, MOS6560_YSIZE)
	MCFG_SCREEN_VISIBLE_AREA(MOS6560_MAME_XPOS, MOS6560_MAME_XPOS + MOS6560_MAME_XSIZE - 1, MOS6560_MAME_YPOS, MOS6560_MAME_YPOS + MOS6560_MAME_YSIZE - 1)
	MCFG_SCREEN_UPDATE_STATIC( vic20 )

	MCFG_PALETTE_LENGTH(16)
	MCFG_PALETTE_INIT( vic20 )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_MOS656X_ADD(M6560_TAG, vic_ntsc_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_VIC20_EXPANSION_SLOT_ADD(VIC20_EXPANSION_SLOT_TAG, MOS6560_CLOCK, expansion_intf, vic20_expansion_cards, NULL, NULL)

	// software lists
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "NTSC")
	MCFG_SOFTWARE_LIST_FILTER("disk_list", "NTSC")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( vic20_pal )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( vic20_pal, vic20_common )
	// basic machine hardware
	MCFG_CPU_ADD(M6502_TAG, M6502, MOS6561_CLOCK)
	MCFG_CPU_PROGRAM_MAP(vic20_mem)
	MCFG_CPU_PERIODIC_INT(vic20_raster_interrupt, MOS656X_HRETRACERATE)

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(MOS6561_VRETRACERATE)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_SIZE((MOS6561_XSIZE + 7) & ~7, MOS6561_YSIZE)
	MCFG_SCREEN_VISIBLE_AREA(MOS6561_MAME_XPOS, MOS6561_MAME_XPOS + MOS6561_MAME_XSIZE - 1, MOS6561_MAME_YPOS, MOS6561_MAME_YPOS + MOS6561_MAME_YSIZE - 1)
	MCFG_SCREEN_UPDATE_STATIC( vic20 )

	MCFG_PALETTE_LENGTH(16)
	MCFG_PALETTE_INIT( vic20 )

	// devices
	MCFG_VIC20_EXPANSION_SLOT_ADD(VIC20_EXPANSION_SLOT_TAG, MOS6561_CLOCK, expansion_intf, vic20_expansion_cards, NULL, NULL)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_MOS656X_ADD(M6560_TAG, vic_pal_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// software lists
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "PAL")
	MCFG_SOFTWARE_LIST_FILTER("disk_list", "PAL")
MACHINE_CONFIG_END



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

//-------------------------------------------------
//  ROM( vic1001 )
//-------------------------------------------------

ROM_START( vic1001 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901486-01", 0x0000, 0x2000, CRC(db4c43c1) SHA1(587d1e90950675ab6b12d91248a3f0d640d02e8d) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "901486-02", 0x0000, 0x2000, CRC(336900d7) SHA1(c9ead45e6674d1042ca6199160e8583c23aeac22) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901460-02", 0x0000, 0x1000, CRC(fcfd8a4b) SHA1(dae61ac03065aa2904af5c123ce821855898c555) )
ROM_END


//-------------------------------------------------
//  ROM( vic20 )
//-------------------------------------------------

ROM_START( vic20 )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901486-01.ue11", 0x0000, 0x2000, CRC(db4c43c1) SHA1(587d1e90950675ab6b12d91248a3f0d640d02e8d) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_SYSTEM_BIOS( 0, "cbm", "Original" )
	ROMX_LOAD( "901486-06.ue12", 0x0000, 0x2000, CRC(e5e7c174) SHA1(06de7ec017a5e78bd6746d89c2ecebb646efeb19), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "jiffydos", "JiffyDOS" )
	ROMX_LOAD( "jiffydos vic-20 ntsc.ue12", 0x0000, 0x2000, CRC(683a757f) SHA1(83fb83e97b5a840311dbf7e1fe56fe828f41936d), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901460-03.ud7", 0x0000, 0x1000, CRC(83e032a6) SHA1(4fd85ab6647ee2ac7ba40f729323f2472d35b9b4) )
ROM_END


//-------------------------------------------------
//  ROM( vic20p )
//-------------------------------------------------

ROM_START( vic20p )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901486-01.ue11", 0x0000, 0x2000, CRC(db4c43c1) SHA1(587d1e90950675ab6b12d91248a3f0d640d02e8d) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_SYSTEM_BIOS( 0, "cbm", "Original" )
	ROMX_LOAD( "901486-07.ue12", 0x0000, 0x2000, CRC(4be07cb4) SHA1(ce0137ed69f003a299f43538fa9eee27898e621e), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "jiffydos", "JiffyDOS" )
	ROMX_LOAD( "jiffydos vic-20 pal.ue12", 0x0000, 0x2000, CRC(705e7810) SHA1(5a03623a4b855531b8bffd756f701306f128be2d), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "901460-03.ud7", 0x0000, 0x1000, CRC(83e032a6) SHA1(4fd85ab6647ee2ac7ba40f729323f2472d35b9b4) )
ROM_END


//-------------------------------------------------
//  ROM( vic20s )
//-------------------------------------------------

ROM_START( vic20s )
	ROM_REGION( 0x2000, "basic", 0 )
	ROM_LOAD( "901486-01.ue11",	0x0000, 0x2000, CRC(db4c43c1) SHA1(587d1e90950675ab6b12d91248a3f0d640d02e8d) )

	ROM_REGION( 0x2000, "kernal", 0 )
	ROM_LOAD( "nec22081.206", 0x0000, 0x2000, CRC(b2a60662) SHA1(cb3e2f6e661ea7f567977751846ce9ad524651a3) )

	ROM_REGION( 0x1000, "charom", 0 )
	ROM_LOAD( "nec22101.207", 0x0000, 0x1000, CRC(d808551d) SHA1(f403f0b0ce5922bd61bbd768bdd6f0b38e648c9f) )
ROM_END



//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT        INIT        COMPANY                             FULLNAME                    FLAGS
COMP( 1980, vic1001,    0,          0,      vic20_ntsc,  vic1001, driver_device,    0,          "Commodore Business Machines",      "VIC-1001 (Japan)",         GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
COMP( 1981, vic20,      vic1001,    0,      vic20_ntsc,  vic20, driver_device,      0,          "Commodore Business Machines",      "VIC-20 (NTSC)",            GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
COMP( 1981, vic20p,     vic1001,    0,      vic20_pal,   vic20, driver_device,      0,          "Commodore Business Machines",      "VIC-20 / VC-20 (PAL)",     GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
COMP( 1981, vic20s,     vic1001,    0,      vic20_pal,   vic20s, driver_device,     0,          "Commodore Business Machines",      "VIC-20 (Sweden/Finland)",  GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
