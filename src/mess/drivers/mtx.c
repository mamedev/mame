/*************************************************************************

    driver/mtx.c

    Memotech MTX 500, MTX 512 and RS 128

**************************************************************************/

/*

    TODO:

    - cassette
    - cartridges
    - SDX/FDX floppy
    - HDX hard disk
    - HRX high resolution graphics
    - CBM (all RAM) mode
    - "Silicon" disks
    - Multi Effect Video Wall

*/

#include "emu.h"
#include "includes/mtx.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "imagedev/snapquik.h"
#include "machine/ctronics.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "video/tms9928a.h"
#include "sound/sn76496.h"

/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( mtx_mem )
-------------------------------------------------*/

static ADDRESS_MAP_START( mtx_mem, AS_PROGRAM, 8, mtx_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROMBANK("bank1")
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank2")
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK("bank3")
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank4")
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( mtx_io )
-------------------------------------------------*/

static ADDRESS_MAP_START( mtx_io, AS_IO, 8, mtx_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREAD_LEGACY(CENTRONICS_TAG, mtx_strobe_r) AM_WRITE(mtx_bankswitch_w)
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE("tms9929a", tms9929a_device, vram_read, vram_write)
	AM_RANGE(0x02, 0x02) AM_DEVREADWRITE("tms9929a", tms9929a_device, register_read, register_write)
	AM_RANGE(0x03, 0x03) AM_DEVREAD_LEGACY(SN76489A_TAG, mtx_sound_strobe_r) AM_DEVWRITE_LEGACY(CASSETTE_TAG, mtx_cst_w)
	AM_RANGE(0x04, 0x04) AM_DEVREAD_LEGACY(CENTRONICS_TAG, mtx_prt_r)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE(CENTRONICS_TAG, centronics_device, write)
	AM_RANGE(0x05, 0x05) AM_READWRITE(mtx_key_lo_r, mtx_sense_w)
	AM_RANGE(0x06, 0x06) AM_READWRITE(mtx_key_hi_r, mtx_sound_latch_w)
//  AM_RANGE(0x07, 0x07) PIO
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x30, 0x31) AM_WRITE(hrx_address_w)
	AM_RANGE(0x32, 0x32) AM_READWRITE(hrx_data_r, hrx_data_w)
	AM_RANGE(0x33, 0x33) AM_READWRITE(hrx_attr_r, hrx_attr_w)
//  AM_RANGE(0x38, 0x38) AM_DEVWRITE_LEGACY(MC6845_TAG, mc6845_address_w)
//  AM_RANGE(0x39, 0x39) AM_DEVWRITE_LEGACY(MC6845_TAG, mc6845_register_r, mc6845_register_w)
/*  AM_RANGE(0x40, 0x43) AM_DEVREADWRITE_LEGACY(FD1791_TAG, wd17xx_r, wd17xx_w)
    AM_RANGE(0x44, 0x44) AM_READWRITE_LEGACY(fdx_status_r, fdx_control_w)
    AM_RANGE(0x45, 0x45) AM_WRITE_LEGACY(fdx_drv_sel_w)
    AM_RANGE(0x46, 0x46) AM_WRITE_LEGACY(fdx_dma_lo_w)
    AM_RANGE(0x47, 0x47) AM_WRITE_LEGACY(fdx_dma_hi_w)*/
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( rs128_io )
-------------------------------------------------*/

static ADDRESS_MAP_START( rs128_io, AS_IO, 8, mtx_state )
	AM_IMPORT_FROM(mtx_io)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE_LEGACY(Z80DART_TAG, z80dart_cd_ba_r, z80dart_cd_ba_w)
ADDRESS_MAP_END

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*-------------------------------------------------
    INPUT_PORTS( mtx512 )
-------------------------------------------------*/

static INPUT_PORTS_START( mtx512 )
	PORT_START("ROW0")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)     PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)     PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)     PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)     PORT_CHAR('7')  PORT_CHAR('\'')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)     PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')  PORT_CHAR('=')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7 Page") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(PGDN)) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(CANCEL)) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)    PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("ROW1")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)    PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)      PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)      PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)      PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)      PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)      PORT_CHAR('0')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8 EOL") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5)        PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("ROW2")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('\t') PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)    PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("ROW3")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Linefeed") PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6)    PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("ROW4")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("AlphaLock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_START("ROW5")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)     PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)     PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("ROW6")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT)  PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)       PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)       PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)       PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)   PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)   PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RSHIFT)  PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("ROW7")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)      PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)      PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)      PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)      PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('_')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)  PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad Enter CLS") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)    PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("country_code")
	PORT_DIPNAME(0x04, 0x00, "Country Code Switch 1")
	PORT_DIPSETTING(0x04, DEF_STR(Off) )
	PORT_DIPSETTING(0x00, DEF_STR(On) )
	PORT_DIPNAME(0x08, 0x00, "Country Code Switch 0")
	PORT_DIPSETTING(0x08, DEF_STR(Off) )
	PORT_DIPSETTING(0x00, DEF_STR(On) )
	PORT_BIT( 0xf3, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    Z80CTC_INTERFACE( ctc_intf )
-------------------------------------------------*/

static TIMER_DEVICE_CALLBACK( ctc_tick )
{
	mtx_state *state = timer.machine().driver_data<mtx_state>();

	state->m_z80ctc->trg1(1);
	state->m_z80ctc->trg1(0);
	state->m_z80ctc->trg2(1);
	state->m_z80ctc->trg2(0);
}

static WRITE_LINE_DEVICE_HANDLER( ctc_trg1_w )
{
	mtx_state *driver_state = device->machine().driver_data<mtx_state>();

	if (driver_state->m_z80dart != NULL)
	{
		z80dart_rxca_w(driver_state->m_z80dart, state);
		z80dart_txca_w(driver_state->m_z80dart, state);
	}
}

static WRITE_LINE_DEVICE_HANDLER( ctc_trg2_w )
{
	mtx_state *driver_state = device->machine().driver_data<mtx_state>();

	if (driver_state->m_z80dart != NULL)
	{
		z80dart_rxtxcb_w(driver_state->m_z80dart, state);
	}
}

static Z80CTC_INTERFACE( ctc_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_LINE(ctc_trg1_w),
	DEVCB_LINE(ctc_trg2_w)
};

/*-------------------------------------------------
    Z80DART_INTERFACE( dart_intf )
-------------------------------------------------*/

static Z80DART_INTERFACE( dart_intf )
{
	0,
	0,
	0,
	0,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0)
};

/*-------------------------------------------------
    z80_daisy_config mtx_daisy_chain
-------------------------------------------------*/

static const z80_daisy_config mtx_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ NULL }
};

/*-------------------------------------------------
    z80_daisy_config rs128_daisy_chain
-------------------------------------------------*/

static const z80_daisy_config rs128_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80DART_TAG },
	{ NULL }
};

/*-------------------------------------------------
    cassette_interface mtx_cassette_interface
-------------------------------------------------*/

static TIMER_DEVICE_CALLBACK( cassette_tick )
{
	mtx_state *state = timer.machine().driver_data<mtx_state>();
	int data = ((state->m_cassette)->input() > +0.0) ? 0 : 1;

	state->m_z80ctc->trg3(data);
}

static const cassette_interface mtx_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};

/*-------------------------------------------------
    mtx_tms9928a_interface
-------------------------------------------------*/

static WRITE_LINE_DEVICE_HANDLER(mtx_tms9929a_interrupt)
{
    mtx_state *drv_state = device->machine().driver_data<mtx_state>();

    drv_state->m_z80ctc->trg0(state ? 0 : 1);
}

static TMS9928A_INTERFACE(mtx_tms9928a_interface)
{
	"screen",
    0x4000,
    DEVCB_LINE(mtx_tms9929a_interrupt)
};

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

/*-------------------------------------------------
    MACHINE_CONFIG_START( mtx512, mtx_state )
-------------------------------------------------*/

static MACHINE_CONFIG_START( mtx512, mtx_state )

	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(mtx_mem)
	MCFG_CPU_IO_MAP(mtx_io)
	MCFG_CPU_CONFIG(mtx_daisy_chain)

	MCFG_MACHINE_START_OVERRIDE(mtx_state,mtx512)
	MCFG_MACHINE_RESET_OVERRIDE(mtx_state,mtx512)

	/* video hardware */
	MCFG_TMS9928A_ADD( "tms9929a", TMS9929A, mtx_tms9928a_interface )
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9929a", tms9929a_device, screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489A_TAG, SN76489A, XTAL_4MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* devices */
	MCFG_Z80CTC_ADD(Z80CTC_TAG, XTAL_4MHz, ctc_intf )
	MCFG_TIMER_ADD_PERIODIC("z80ctc_timer", ctc_tick, attotime::from_hz(XTAL_4MHz/13))
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, standard_centronics)
	MCFG_SNAPSHOT_ADD("snapshot", mtx, "mtb", 0.5)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, mtx_cassette_interface)
	MCFG_TIMER_ADD_PERIODIC("cassette_timer", cassette_tick, attotime::from_hz(44100))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_EXTRA_OPTIONS("96K,128K,160K,192K,224K,256K,288K,320K,352K,384K,416K,448K,480K,512K")
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_CONFIG_DERIVED( mtx500, mtx512 )
-------------------------------------------------*/

static MACHINE_CONFIG_DERIVED( mtx500, mtx512 )

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("64K,96K,128K,160K,192K,224K,256K,288K,320K,352K,384K,416K,448K,480K,512K")
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_CONFIG_DERIVED( rs128, mtx512 )
-------------------------------------------------*/

static MACHINE_CONFIG_DERIVED( rs128, mtx512 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY(Z80_TAG)
	MCFG_CPU_IO_MAP(rs128_io)
	MCFG_CPU_CONFIG(rs128_daisy_chain)

	/* devices */
	MCFG_Z80DART_ADD(Z80DART_TAG, XTAL_4MHz, dart_intf)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("160K,192K,224K,256K,288K,320K,352K,384K,416K,448K,480K,512K")
MACHINE_CONFIG_END

/***************************************************************************
    ROMS
***************************************************************************/

ROM_START( mtx512 )
	ROM_REGION( 0x02000, "user1", 0 )
	ROM_LOAD( "osrom",    0x0000, 0x2000, CRC(9ca858cc) SHA1(3804503a58f0bcdea96bb6488833782ebd03976d) )

	ROM_REGION( 0x10000, "user2", 0 )
	ROM_LOAD( "basicrom", 0x0000, 0x2000, CRC(87b4e59c) SHA1(c49782a82a7f068c1195cd967882ba9edd546eaf) )
	ROM_LOAD( "assemrom", 0x2000, 0x2000, CRC(9d7538c3) SHA1(d1882c4ea61a68b1715bd634ded5603e18a99c5f) )
	ROM_LOAD( "newword0", 0x4000, 0x2000, CRC(5433bd01) SHA1(56897f385476864337b7b994c1b46f50eaa57128) )
	ROM_LOAD( "newword1", 0x6000, 0x2000, CRC(10980c03) SHA1(8245c42fb1eda43b67fa483e50e4ee3bc5f3f29f) )
	ROM_LOAD( "newword2", 0x8000, 0x2000, CRC(cbff7130) SHA1(b4ab27def6020ba8e32959a1b988fce77bcc1d7f) )
	ROM_LOAD( "newword3", 0xa000, 0x2000, CRC(e6bbc33b) SHA1(26faa4a8d952c4659f610e94608b630456ab89aa) )

	ROM_REGION( 0x2000, "sdx", 0 )
	ROM_LOAD( "sdx", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x2000, "mtx", 0 )
	ROM_LOAD( "sm2 fdcx1 v03", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x1000, MC6845_TAG, 0 )
	ROM_LOAD( "80z.bin", 0x0000, 0x1000, CRC(ea6fe865) SHA1(f84883f79bed34501e5828336894fad929bddbb5) )
ROM_END

#define rom_mtx500  rom_mtx512
#define rom_rs128   rom_mtx512

/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     INIT    COMPANY          FULLNAME   FLAGS */
COMP( 1983, mtx512,   0,		0,      mtx512,   mtx512, driver_device,   0,		"Memotech Ltd", "MTX 512", 0 )
COMP( 1983, mtx500,   mtx512,   0,      mtx500,   mtx512, driver_device,   0,		"Memotech Ltd", "MTX 500", 0 )
COMP( 1984, rs128,    mtx512,   0,      rs128,    mtx512, driver_device,   0,		"Memotech Ltd", "RS 128",  0 )
