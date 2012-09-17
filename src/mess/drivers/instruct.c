/***************************************************************************

    Signetics Intructor 50

    08/04/2010 Skeleton driver.
    20/05/2012 Connected digits, system boots. [Robbbert]
    20/05/2012 Connected keyboard, system mostly usable. [Robbbert]

    The eprom and 128 bytes of ram are in a 2656 chip. There is no
    useful info on this device on the net. It does appear though,
    that it divides the 3.58MHz crystal by 4 for the CPU.

    The system also has 512 bytes of ram in an ordinary ram chip.

    There are no known schematics for this computer. There is a block
    diagram which imparts little.

    From looking at a blurry picture of it, this is what I can determine:
    - Left side: 8 toggle switches, with a round red led above each one.
    - Below this is another toggle switch with choice of 'Extended' or 'I/O port 07'.
    - To the right of this is another toggle switch labelled 'Interrupt', the
      choices are 'Direct' and 'Indirect'.
    - Above this switch are 2 more round red leds: FLAG and RUN.
    - Middle: a 4 down x3 across keypad containing the function keys. The
      labels (from left to right, top to bottom) seem to be:
      SENS, WCAS, BKPT, INT, RCAS, REG, MON, STEP, MEM, RST, RUN, ENT/NXT.
    - Right side: a 4x4 hexadecimal keypad. The keys are:
      C, D, E, F, 8, 9, A, B, 4, 5, 6, 7, 0, 1, 2, 3
    - Above, extending from one side to the other is a metal plate with
      printed mnemonics. At the right edge are sockets to connect up the
      MIC and EAR cords to a cassette player.
    - At the back is a S100 interface.

    Quick usage:
    - Look at memory: Press minus key. Enter an address. Press UP key to see the next.
    - Look at registers: Press R. Press 0. Press UP key to see the next.
    - Look at PC register: Press R. Press C.
    - Load a tape: Press L, enter file number (1 digit), press UP. On
      completion of a successful load, HELLO will be displayed.

    ToDO:
    - Keys are mostly correct. It seems the SENS, INT, MON, RST keys are
      not in the matrix, but are connected to hardware directly. This needs
      to be emulated. (SENS key done)
    - Connect 10 toggle switches and 10 round red leds.

****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "instruct.lh"

class instruct_state : public driver_device
{
public:
	instruct_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_p_ram(*this, "p_ram"),
	m_cass(*this, CASSETTE_TAG)
	{ }

	DECLARE_READ8_MEMBER(portfc_r);
	DECLARE_READ8_MEMBER(portfd_r);
	DECLARE_READ8_MEMBER(portfe_r);
	DECLARE_READ8_MEMBER(sense_r);
	DECLARE_WRITE8_MEMBER(portf8_w);
	DECLARE_WRITE8_MEMBER(portf9_w);
	DECLARE_WRITE8_MEMBER(portfa_w);
	virtual void machine_reset();
	UINT8 m_digit;
	bool m_valid_digit;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_p_ram;
	required_device<cassette_image_device> m_cass;
};

// cassette port
// when loading, bit 7 is continuously toggled
// saving can use bits 3,4,5
WRITE8_MEMBER( instruct_state::portf8_w )
{
	m_cass->output(BIT(data, 3) ? -1.0 : +1.0);
}

// segment output
WRITE8_MEMBER( instruct_state::portf9_w )
{
	if (m_valid_digit)
		output_set_digit_value(m_digit, data);
	m_valid_digit = false;
}

// digit & keyrow-scan select
WRITE8_MEMBER( instruct_state::portfa_w )
{
	m_digit = data;
	m_valid_digit = true;
}

// unknown - copied to 17E9 at boot
READ8_MEMBER( instruct_state::portfc_r )
{
	return 0x55;
}

// unknown - copied to 17E8 at boot
READ8_MEMBER( instruct_state::portfd_r )
{
	return 0xAA;
}

// read keyboard
READ8_MEMBER( instruct_state::portfe_r )
{
	UINT8 i;

	for (i = 0; i < 6; i++)
	{
		if (BIT(m_digit, i))
		{
			char kbdrow[6];
			sprintf(kbdrow,"X%X",i);
			return ioport(kbdrow)->read();
		}
	}

	return 0xff;
}


// Read cassette and SENS key
READ8_MEMBER( instruct_state::sense_r )
{
	return ( (m_cass->input() > 0.03) ? 1 : 0) | (ioport("HW")->read() & 1);
}

static ADDRESS_MAP_START( instruct_mem, AS_PROGRAM, 8, instruct_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x01ff) AM_RAM AM_SHARE("p_ram") // 512 bytes onboard ram
	AM_RANGE(0x0200, 0x177f) AM_RAM // expansion ram needed by quickloads
	AM_RANGE(0x1780, 0x17ff) AM_RAM // 128 bytes in s2656 chip
	AM_RANGE(0x1800, 0x1fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( instruct_io, AS_IO, 8, instruct_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xf8, 0xf8) AM_WRITE(portf8_w)
	AM_RANGE(0xf9, 0xf9) AM_WRITE(portf9_w)
	AM_RANGE(0xfa, 0xfa) AM_WRITE(portfa_w)
	AM_RANGE(0xfc, 0xfc) AM_READ(portfc_r)
	AM_RANGE(0xfd, 0xfd) AM_READ(portfd_r)
	AM_RANGE(0xfe, 0xfe) AM_READ(portfe_r)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(sense_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( instruct )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("WCAS") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RCAS") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STEP") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BKPT") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENT/NXT") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("HW")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SENS") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INT") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MON") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RST") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
INPUT_PORTS_END


void instruct_state::machine_reset()
{
	// copy the roms into ram so it can boot
	UINT8* ROM = memregion("maincpu")->base();
	memcpy(m_p_ram, ROM+0x1800, 0x0200);
}

QUICKLOAD_LOAD( instruct )
{
	address_space &space = *image.device().machine().device("maincpu")->memory().space(AS_PROGRAM);
	int i;
	int quick_addr = 0x0100;
	int exec_addr;
	int quick_length;
	UINT8 *quick_data;
	int read_;

	quick_length = image.length();
	quick_data = (UINT8*)malloc(quick_length);
	if (!quick_data)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot open file");
		image.message(" Cannot open file");
		return IMAGE_INIT_FAIL;
	}

	read_ = image.fread( quick_data, quick_length);
	if (read_ != quick_length)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot read the file");
		image.message(" Cannot read the file");
		return IMAGE_INIT_FAIL;
	}

	if (quick_data[0] != 0xc5)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Invalid header");
		image.message(" Invalid header");
		return IMAGE_INIT_FAIL;
	}

	exec_addr = quick_data[1] * 256 + quick_data[2];

	if (exec_addr >= quick_length)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Exec address beyond end of file");
		image.message(" Exec address beyond end of file");
		return IMAGE_INIT_FAIL;
	}

	if (quick_length < 0x104)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too short");
		image.message(" File too short");
		return IMAGE_INIT_FAIL;
	}

	if (quick_length > 0x17c0)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too long");
		image.message(" File too long");
		return IMAGE_INIT_FAIL;
	}

	for (i = quick_addr; i < quick_length; i++)
	{
		space.write_byte(i, quick_data[i]);
	}

	/* display a message about the loaded quickload */
	image.message(" Quickload: size=%04X : exec=%04X",quick_length,exec_addr);

	// Start the quickload
	image.device().machine().device("maincpu")->state().set_pc(exec_addr);
	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( instruct, instruct_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_3_579545MHz / 4)
	MCFG_CPU_PROGRAM_MAP(instruct_mem)
	MCFG_CPU_IO_MAP(instruct_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_instruct)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", instruct, "pgm", 1)

	/* cassette */
	MCFG_CASSETTE_ADD( CASSETTE_TAG, default_cassette_interface )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( instruct )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "instruct.rom", 0x1800, 0x0800, CRC(131715a6) SHA1(4930b87d09046113ab172ba3fb31f5e455068ec7) )
ROM_END

/* Driver */

/*    YEAR  NAME       PARENT   COMPAT   MACHINE    INPUT     INIT    COMPANY     FULLNAME                    FLAGS */
COMP( 1978, instruct,  0,       0,       instruct,  instruct, driver_device, 0,    "Signetics", "Signetics Instructor 50", 0 )
