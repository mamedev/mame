// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Signetics Intructor 50

    2010-04-08 Skeleton driver.
    2012-05-20 Connected digits, system boots. [Robbbert]
    2012-05-20 Connected keyboard, system mostly usable. [Robbbert]
    2013-10-15 Fixed various regressions. [Robbbert]

    From looking at a blurry picture of it, this is what I can determine:
    - Left side: 8 toggle switches, with a round red led above each one.
    - Below this is the Port Address Switch with choice of 'Non-Extended', 'Extended' or 'Memory'.
    - To the right of this is another toggle switch labelled 'Interrupt', the
      choices are 'Direct' and 'Indirect'.
    - Above this switch are 2 more round red leds: FLAG and RUN.
    - Middle: a 4 down x3 across keypad containing the function keys. The
      labels (from left to right, top to bottom) are:
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
    - Set PC register: Press R. Press C. Type in new address, Press UP.
    - Load a tape: Press L, enter file number (1 digit), press UP. On
      completion of a successful load, HELLO will be displayed.

    ToDO:
    - Connect round led for Run.
    - Last Address Register
    - Initial Jump Logic
    - Single-step and Breakpoint don't stop execution because of the above.
    - The "Port Address Switch" which selects which of the 3 sources will
      be used for port_r and port_w. Currently all 3 are selected at once.

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
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "mainram")
		, m_p_smiram(*this, "smiram")
		, m_p_extram(*this, "extram")
		, m_cass(*this, "cassette")
	{ }

	DECLARE_READ8_MEMBER(port_r);
	DECLARE_READ8_MEMBER(portfc_r);
	DECLARE_READ8_MEMBER(portfd_r);
	DECLARE_READ8_MEMBER(portfe_r);
	DECLARE_READ8_MEMBER(sense_r);
	DECLARE_WRITE_LINE_MEMBER(flag_w);
	DECLARE_WRITE8_MEMBER(port_w);
	DECLARE_WRITE8_MEMBER(portf8_w);
	DECLARE_WRITE8_MEMBER(portf9_w);
	DECLARE_WRITE8_MEMBER(portfa_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER(instruct);
	INTERRUPT_GEN_MEMBER(t2l_int);
private:
	virtual void machine_reset() override;
	UINT16 m_lar;
	UINT8 m_digit;
	bool m_valid_digit;
	bool m_cassin;
	bool m_irqstate;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_p_ram;
	required_shared_ptr<UINT8> m_p_smiram;
	required_shared_ptr<UINT8> m_p_extram;
	required_device<cassette_image_device> m_cass;
};

// flag led
WRITE_LINE_MEMBER( instruct_state::flag_w )
{
	output_set_value("led8", !state);
}

// user port
WRITE8_MEMBER( instruct_state::port_w )
{
	char ledname[8];
	for (int i = 0; i < 8; i++)
	{
		sprintf(ledname,"led%d",i);
		output_set_value(ledname, !BIT(data, i));
	}
}

// cassette port
WRITE8_MEMBER( instruct_state::portf8_w )
{
	if BIT(data, 4)
		m_cass->output(BIT(data, 3) ? -1.0 : +1.0);
	else
		m_cass->output(0.0);

	m_cassin = BIT(data, 7);
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

// user switches
READ8_MEMBER( instruct_state::port_r )
{
	return ioport("USW")->read();
}

// last address register A0-7 copied to 17E9 at boot
READ8_MEMBER( instruct_state::portfc_r )
{
	return m_lar;
}

// last address register A8-14 copied to 17E8 at boot
READ8_MEMBER( instruct_state::portfd_r )
{
	return (m_lar >> 8) & 0x7f;
}

// read keyboard
READ8_MEMBER( instruct_state::portfe_r )
{
	for (UINT8 i = 0; i < 6; i++)
	{
		if (BIT(m_digit, i))
		{
			char kbdrow[6];
			sprintf(kbdrow,"X%X",i);
			return ioport(kbdrow)->read();
		}
	}

	return 0xf;
}


// Read cassette and SENS key
READ8_MEMBER( instruct_state::sense_r )
{
	if (m_cassin)
		return (m_cass->input() > 0.03) ? 1 : 0;
	else
		return BIT(ioport("HW")->read(), 0);
}

INTERRUPT_GEN_MEMBER( instruct_state::t2l_int )
{
	UINT8 hwkeys = ioport("HW")->read();

	// check RST key
	if BIT(hwkeys, 3)
	{
		m_maincpu->set_state_int(S2650_PC, 0);
		return;
	}
	else
	// check MON key
	if BIT(hwkeys, 2)
	{
		m_maincpu->set_state_int(S2650_PC, 0x1800);
		return;
	}
	else
	{
		UINT8 switches = ioport("SW")->read();

		// Set vector from INDIRECT sw
		UINT8 vector = BIT(switches, 0) ? 0x87 : 0x07;

		// Check INT sw & key
		if BIT(switches, 1)
			device.execute().set_input_line_and_vector(0, BIT(hwkeys, 1) ? ASSERT_LINE : CLEAR_LINE, vector);
		else
		// process ac input
		{
			m_irqstate ^= 1;
			device.execute().set_input_line_and_vector(0, m_irqstate ? ASSERT_LINE : CLEAR_LINE, vector);
		}
	}
}

static ADDRESS_MAP_START( instruct_mem, AS_PROGRAM, 8, instruct_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0ffe) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x0fff, 0x0fff) AM_READWRITE(port_r,port_w)
	AM_RANGE(0x1780, 0x17ff) AM_RAM AM_SHARE("smiram")
	AM_RANGE(0x1800, 0x1fff) AM_ROM AM_REGION("roms",0)
	AM_RANGE(0x2000, 0x7fff) AM_RAM AM_SHARE("extram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( instruct_io, AS_IO, 8, instruct_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x07, 0x07) AM_READWRITE(port_r,port_w)
	AM_RANGE(0xf8, 0xf8) AM_WRITE(portf8_w)
	AM_RANGE(0xf9, 0xf9) AM_WRITE(portf9_w)
	AM_RANGE(0xfa, 0xfa) AM_WRITE(portfa_w)
	AM_RANGE(0xfc, 0xfc) AM_READ(portfc_r)
	AM_RANGE(0xfd, 0xfd) AM_READ(portfd_r)
	AM_RANGE(0xfe, 0xfe) AM_READ(portfe_r)
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_READWRITE(port_r,port_w)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(sense_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( instruct )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("WCAS") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RCAS") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STEP") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BKPT") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENT/NXT") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')

	PORT_START("HW")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SENS") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INT") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MON") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RST") PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START("SW")
	PORT_DIPNAME( 0x01, 0x00, "INT") // Interrupt jumps to 0007 or *0007
	PORT_DIPSETTING(    0x01, "Indirect")
	PORT_DIPSETTING(    0x00, "Direct")
	PORT_DIPNAME( 0x02, 0x00, "AC/INT") // Interrupt comes from INT key or from power supply
	PORT_DIPSETTING(    0x02, "INT")
	PORT_DIPSETTING(    0x00, "AC")

	PORT_START("USW")
	PORT_DIPNAME( 0x01, 0x00, "Switch A") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x02, "Switch B") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x04, "Switch C") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x00, "Switch D") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x10, 0x00, "Switch E") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x20, "Switch F") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "Switch G") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x00, "Switch H") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
INPUT_PORTS_END


void instruct_state::machine_reset()
{
	m_cassin = 0;
	address_space &space = m_maincpu->space(AS_IO);
	port_w(space, 0, 0); // turn round leds off
	m_maincpu->set_state_int(S2650_PC, 0x1800);
}

QUICKLOAD_LOAD_MEMBER( instruct_state, instruct )
{
	UINT16 i, exec_addr, quick_length, read_;
	int result = IMAGE_INIT_FAIL;

	quick_length = image.length();
	if (quick_length < 0x0100)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too short");
		image.message(" File too short");
	}
	else
	if (quick_length > 0x8000)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too long");
		image.message(" File too long");
	}
	else
	{
		dynamic_buffer quick_data(quick_length);
		read_ = image.fread( &quick_data[0], quick_length);
		if (read_ != quick_length)
		{
			image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot read the file");
			image.message(" Cannot read the file");
		}
		else if (quick_data[0] != 0xc5)
		{
			image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Invalid header");
			image.message(" Invalid header");
		}
		else
		{
			exec_addr = quick_data[1] * 256 + quick_data[2];

			if (exec_addr >= quick_length)
			{
				image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Exec address beyond end of file");
				image.message(" Exec address beyond end of file");
			}
			else
			{
				// load to 0000-0FFE (standard ram + extra)
				read_ = 0xfff;
				if (quick_length < 0xfff)
					read_ = quick_length;
				m_p_ram[0] = 0x1f;  // add jump for RST key
				for (i = 1; i < read_; i++)
					m_p_ram[i] = quick_data[i];

				// load to 1780-17BF (spare ram inside 2656)
				read_ = 0x17c0;
				if (quick_length < 0x17c0)
					read_ = quick_length;
				if (quick_length > 0x1780)
					for (i = 0x1780; i < read_; i++)
						m_p_smiram[i-0x1780] = quick_data[i];

				// put start address into PC so it can be debugged
				m_p_smiram[0x68] = m_p_ram[1];
				m_p_smiram[0x69] = m_p_ram[2];

				// load to 2000-7FFF (optional extra ram)
				if (quick_length > 0x2000)
					for (i = 0x2000; i < quick_length; i++)
						m_p_extram[i-0x2000] = quick_data[i];

				/* display a message about the loaded quickload */
				image.message(" Quickload: size=%04X : exec=%04X",quick_length,exec_addr);

				// Start the quickload - JP exec_addr
				m_maincpu->set_state_int(S2650_PC, 0);

				result = IMAGE_INIT_PASS;
			}
		}
	}

	return result;
}

static MACHINE_CONFIG_START( instruct, instruct_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_3_579545MHz / 4)
	MCFG_CPU_PROGRAM_MAP(instruct_mem)
	MCFG_CPU_IO_MAP(instruct_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(instruct_state, t2l_int, 120)
	MCFG_S2650_FLAG_HANDLER(WRITELINE(instruct_state, flag_w))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_instruct)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", instruct_state, instruct, "pgm", 1)

	/* cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( instruct )
	ROM_REGION( 0x0800, "roms", 0 )
	ROM_LOAD( "instruct.rom", 0x0000, 0x0800, CRC(131715a6) SHA1(4930b87d09046113ab172ba3fb31f5e455068ec7) )

	ROM_REGION( 0x8020, "proms", 0 )
	ROM_LOAD( "82s123.33",    0x0000, 0x0020, CRC(b7aecef0) SHA1(b39fb35e8b6ab67b31f8f310fd5d56304bcd4123) )
	ROM_LOAD( "82s103.20",    0x0020, 0x8000, NO_DUMP )
ROM_END

/* Driver */

/*    YEAR  NAME       PARENT   COMPAT   MACHINE    INPUT     INIT    COMPANY     FULLNAME                    FLAGS */
COMP( 1978, instruct,  0,       0,       instruct,  instruct, driver_device, 0,    "Signetics", "Signetics Instructor 50", 0 )
