/******************************************************************************
    Motorola Evaluation Kit 6800 D2
    MEK6800D2

    system driver

    Juergen Buchmueller <pullmoll@t-online.de>, Jan 2000

    memory map

    range       short   description
    0000-00ff   RAM     256 bytes RAM
    0100-01ff   RAM     optional 256 bytes RAM
    6000-63ff   PROM    optional PROM
    or
    6000-67ff   ROM     optional ROM
    8004-8007   PIA     expansion port
    8008-8009   ACIA    cassette interface
    8020-8023   PIA     keyboard interface
    a000-a07f   RAM     128 bytes RAM (JBUG scratch)
    c000-c3ff   PROM    optional PROM
    or
    c000-c7ff   ROM     optional ROM
    e000-e3ff   ROM     JBUG monitor program
    e400-ffff   -/-     mirrors of monitor rom


    TODO
    - Cassette (it is extremely complex, with approx 10 chips)
    - Proper artwork

******************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "imagedev/cartslot.h"
#include "mekd2.lh"

#define XTAL_MEKD2 1228800

class mekd2_state : public driver_device
{
public:
	mekd2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_pia_s(*this, "pia_s"),
	m_pia_u(*this, "pia_u"),
	m_acia(*this, "acia")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<device_t> m_pia_s;
	required_device<device_t> m_pia_u;
	required_device<device_t> m_acia;
	DECLARE_READ_LINE_MEMBER( mekd2_key40_r );
	DECLARE_READ8_MEMBER( mekd2_key_r );
	DECLARE_WRITE_LINE_MEMBER( mekd2_nmi_w );
	DECLARE_WRITE8_MEMBER( mekd2_digit_w );
	DECLARE_WRITE8_MEMBER( mekd2_segment_w );
	UINT8 m_segment;
	UINT8 m_digit;
	UINT8 m_keydata;
	TIMER_CALLBACK_MEMBER(mekd2_trace);
};



/***********************************************************

    Address Map

************************************************************/

static ADDRESS_MAP_START( mekd2_mem , AS_PROGRAM, 8, mekd2_state)
	AM_RANGE(0x0000, 0x00ff) AM_RAM // user ram
	AM_RANGE(0x8004, 0x8007) AM_DEVREADWRITE("pia_u", pia6821_device, read, write)
	AM_RANGE(0x8008, 0x8008) AM_DEVREADWRITE("acia", acia6850_device, status_read, control_write)
	AM_RANGE(0x8009, 0x8009) AM_DEVREADWRITE("acia", acia6850_device, data_read, data_write)
	AM_RANGE(0x8020, 0x8023) AM_DEVREADWRITE("pia_s", pia6821_device, read, write)
	AM_RANGE(0xa000, 0xa07f) AM_RAM // system ram
	AM_RANGE(0xe000, 0xe3ff) AM_ROM AM_MIRROR(0x1c00)	/* JBUG ROM */
ADDRESS_MAP_END

/***********************************************************

    Keys

************************************************************/

/*

Enter the 4 digit address then the command key:

  - M : Examine and Change Memory (example: E000M, then G to skip to next, H to exit)
  - E : Escape (abort) operation (H key in our emulation)
  - R : Examine Registers
  - G : Begin execution at specified address
  - P : Punch data from memory to magnetic tape
  - L : Load memory from magnetic tape
  - N : Trace one instruction
  - V : Set (and remove) breakpoints

The keys are laid out as:

  P L N V

  7 8 9 A  M
  4 5 6 B  E
  1 2 3 C  R
  0 F E D  G

 */
static INPUT_PORTS_START( mekd2 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E (hex)") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E (escape)") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
INPUT_PORTS_END


/***********************************************************

    Trace hardware (what happens when N is pressed)

************************************************************/

TIMER_CALLBACK_MEMBER(mekd2_state::mekd2_trace)
{
	machine().device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

WRITE_LINE_MEMBER( mekd2_state::mekd2_nmi_w )
{
	if (state)
		machine().device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	else
		machine().scheduler().timer_set(attotime::from_usec(18), timer_expired_delegate(FUNC(mekd2_state::mekd2_trace),this));
}



/***********************************************************

    Keyboard

************************************************************/

READ_LINE_MEMBER( mekd2_state::mekd2_key40_r )
{
	return BIT(m_keydata, 6);
}

READ8_MEMBER( mekd2_state::mekd2_key_r )
{
	char kbdrow[4];
	UINT8 i;
	m_keydata = 0xff;

	for (i = 0; i < 6; i++)
	{
		if (BIT(m_digit, i))
		{
			sprintf(kbdrow,"X%d",i);
			m_keydata &= ioport(kbdrow)->read();
		}
	}

	i = 0x80;
	if (m_digit < 0x40)
		i = BIT(m_keydata, 0) ? 0x80 : 0;
	else
	if (m_digit < 0x80)
		i = BIT(m_keydata, 1) ? 0x80 : 0;
	else
	if (m_digit < 0xc0)
		i = BIT(m_keydata, 2) ? 0x80 : 0;
	else
		i = BIT(m_keydata, 3) ? 0x80 : 0;

	return i | m_segment;
}



/***********************************************************

    LED display

************************************************************/

WRITE8_MEMBER( mekd2_state::mekd2_segment_w )
{
	m_segment = data & 0x7f;
}

WRITE8_MEMBER( mekd2_state::mekd2_digit_w )
{
	UINT8 i;
	if (data < 0x3f)
	{
		for (i = 0; i < 6; i++)
		{
			if (BIT(data, i))
				output_set_digit_value(i, ~m_segment & 0x7f);
		}
	}
	m_digit = data;
}



/***********************************************************

    Interfaces

************************************************************/

static const pia6821_interface mekd2_s_mc6821_intf =
{
	DEVCB_DRIVER_MEMBER(mekd2_state, mekd2_key_r),		/* port A input */
	DEVCB_NULL,						/* port B input */
	DEVCB_NULL,						/* CA1 input */
	DEVCB_DRIVER_LINE_MEMBER(mekd2_state, mekd2_key40_r),	/* CB1 input */
	DEVCB_NULL,						/* CA2 input */
	DEVCB_NULL,						/* CB2 input */
	DEVCB_DRIVER_MEMBER(mekd2_state, mekd2_segment_w),	/* port A output */
	DEVCB_DRIVER_MEMBER(mekd2_state, mekd2_digit_w),	/* port B output */
	DEVCB_DRIVER_LINE_MEMBER(mekd2_state, mekd2_nmi_w),	/* CA2 output */
	DEVCB_NULL,						/* CB2 output */
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_NMI),	/* IRQA output */
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_NMI)		/* IRQB output */
};

static const pia6821_interface mekd2_u_mc6821_intf =
{
	DEVCB_NULL,						/* port A input */
	DEVCB_NULL,						/* port B input */
	DEVCB_NULL,						/* CA1 input */
	DEVCB_NULL,						/* CB1 input */
	DEVCB_NULL,						/* CA2 input */
	DEVCB_NULL,						/* CB2 input */
	DEVCB_NULL,						/* port A output */
	DEVCB_NULL,						/* port B output */
	DEVCB_NULL,						/* CA2 output */
	DEVCB_NULL,						/* CB2 output */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),	/* IRQA output */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB output */
};

static ACIA6850_INTERFACE( mekd2_acia_intf )
{
	XTAL_MEKD2 / 256,	//connected to cassette circuit /* tx clock 4800Hz */
	XTAL_MEKD2 / 256,	//connected to cassette circuit /* rx clock varies, controlled by cassette circuit */
	DEVCB_NULL,//LINE(cass),//connected to cassette circuit /* in rxd func */
	DEVCB_NULL,//LINE(cass),//connected to cassette circuit /* out txd func */
	DEVCB_NULL,						/* in cts func */
	DEVCB_NULL,		//connected to cassette circuit /* out rts func */
	DEVCB_NULL,						/* in dcd func */
	DEVCB_NULL						/* out irq func */
};

static DEVICE_IMAGE_LOAD( mekd2_cart )
{
	static const char magic[] = "MEK6800D2";
	char buff[9];
	UINT16 addr, size;
	UINT8 ident, *RAM = image.device().machine().root_device().memregion("maincpu")->base();

	image.fread( buff, sizeof (buff));
	if (memcmp(buff, magic, sizeof (buff)))
	{
		logerror( "mekd2_rom_load: magic '%s' not found\n", magic);
		return IMAGE_INIT_FAIL;
	}
	image.fread( &addr, 2);
	addr = LITTLE_ENDIANIZE_INT16(addr);
	image.fread( &size, 2);
	size = LITTLE_ENDIANIZE_INT16(size);
	image.fread( &ident, 1);
	logerror("mekd2_rom_load: $%04X $%04X $%02X\n", addr, size, ident);
	while (size-- > 0)
		image.fread( &RAM[addr++], 1);

	return IMAGE_INIT_PASS;
}

/***********************************************************

    Machine

************************************************************/

static MACHINE_CONFIG_START( mekd2, mekd2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_MEKD2 / 2)        /* 614.4 kHz */
	MCFG_CPU_PROGRAM_MAP(mekd2_mem)

	MCFG_DEFAULT_LAYOUT(layout_mekd2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_CASSETTE_ADD( CASSETTE_TAG, default_cassette_interface )

	/* Cartslot ?? does not come with one.. */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("d2")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(mekd2_cart)

	/* Devices */
	MCFG_PIA6821_ADD("pia_s", mekd2_s_mc6821_intf)
	MCFG_PIA6821_ADD("pia_u", mekd2_u_mc6821_intf)
	MCFG_ACIA6850_ADD("acia", mekd2_acia_intf)
MACHINE_CONFIG_END

/***********************************************************

    ROMS

************************************************************/

ROM_START(mekd2)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("jbug.rom", 0xe000, 0x0400, CRC(5ed08792) SHA1(b06e74652a4c4e67c4a12ddc191ffb8c07f3332e) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT     INIT    COMPANY     FULLNAME */
CONS( 1977, mekd2,	0,	0,  mekd2,    mekd2, driver_device,	0,	"Motorola", "MEK6800D2" , GAME_NOT_WORKING )
