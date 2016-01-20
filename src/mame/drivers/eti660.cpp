// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
/*****************************************************************************************

    ETI project 660, featured in the Australian magazine Electronics Today International.

    Commands:
    R - Reset (use this before any other instruction)
    S - Step
    0 - Enter modify memory mode
    2 - Save memory to cassette
    4 - Load memory from cassette
    6 - start binary program
    8 - start chip-8 program.

    To modify memory, press R, 0, enter 4-digit address, S, enter data, S, continue on.
    R to escape.

    To save a tape, enter the start address into 0400,0401 (big endian), and the end
    address into 0402,0403. Press R. Press record on the tape, press 2.

    To load a tape, enter the start and end addresses as above. Press R, 4. Screen goes
    black. Press play on tape. If the screen is still black after the tape ends, press R.

    All chip-8 programs start at 0600. The manual says the max end address is 7FF (gives
    512 bytes), but you can fill up all of memory (gives 2560 bytes).

    TODO:
    - sometimes there's no sound when started. You may need to hard reset until it beeps.
    - don't know why, but the expected behaviour of our 6821 pia isn't what this machine
      expects, so had to add a couple of hacks.
    - doesn't run programs for other chip-8 computers (this might be normal?)
    - we support BIN files, but have none to test with.
    - in Invaders, can't shoot them
    - in Maze, the result is rubbish (works in Emma02 emulator)

**************************************************************************************************/

#include "includes/eti660.h"

/* Read/Write Handlers */

READ8_MEMBER( eti660_state::pia_r )
{
	UINT8 pia_offset = m_maincpu->get_memory_address() & 0x03;

	return m_pia->read(space, pia_offset);
}

WRITE8_MEMBER( eti660_state::pia_w )
{
	UINT8 pia_offset = m_maincpu->get_memory_address() & 0x03;

	// Some PIA hacks here, as mentioned in the ToDo.
	if (pia_offset == 1)
	{
		// switch color on when requested (test with Wipeout game)
		if (data == 0x2c)
			m_color_on = 1;
		// enable keyboard
		if (data == 0x20)
			data = 0x24;
	}

	m_pia->write(space, pia_offset, data);
}

WRITE8_MEMBER( eti660_state::colorram_w )
{
	offset = m_maincpu->get_memory_address() - 0xc80;

	UINT8 colorram_offset = (((offset & 0x1f0) >> 1) | (offset & 0x07));

	if (colorram_offset < 0xc0)
		m_color_ram[colorram_offset] = data;
}

/* Memory Maps */

static ADDRESS_MAP_START( eti660_map, AS_PROGRAM, 8, eti660_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x0400, 0x047f) AM_RAM
	AM_RANGE(0x0480, 0x05ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x0600, 0x0fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( eti660_io_map, AS_IO, 8, eti660_state )
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE(CDP1864_TAG, cdp1864_device, dispon_r, step_bgcolor_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(pia_r, pia_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(colorram_w)
	AM_RANGE(0x04, 0x04) AM_DEVREADWRITE(CDP1864_TAG, cdp1864_device, dispoff_r, tone_latch_w)
ADDRESS_MAP_END

/* Input Ports */
static INPUT_PORTS_START( eti660 )
	PORT_START("KEY.0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY.1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY.2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY.3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("STEP") PORT_CODE(KEYCODE_S)
INPUT_PORTS_END

/* Video */

READ_LINE_MEMBER( eti660_state::rdata_r )
{
	return BIT(m_color, 0);
}

READ_LINE_MEMBER( eti660_state::bdata_r )
{
	return BIT(m_color, 1);
}

READ_LINE_MEMBER( eti660_state::gdata_r )
{
	return BIT(m_color, 2);
}

/* CDP1802 Interface */

READ_LINE_MEMBER( eti660_state::clear_r )
{
	// A hack to make the machine reset itself on
	// boot, like the real one does.
	if (m_resetcnt < 0xffff)
		m_resetcnt++;
	if (m_resetcnt == 0xff00)
		return 0;
	return BIT(m_special->read(), 0); // R key
}

READ_LINE_MEMBER( eti660_state::ef2_r )
{
	return m_cassette->input() < 0;
}

READ_LINE_MEMBER( eti660_state::ef4_r )
{
	return BIT(m_special->read(), 1); // S key
}

WRITE_LINE_MEMBER( eti660_state::q_w )
{
	/* CDP1864 audio output enable */
	m_cti->aoe_w(state);

	/* PULSE led */
	output().set_led_value(LED_PULSE, state);

	/* tape output */
	m_cassette->output(state ? 1.0 : -1.0);
}

WRITE8_MEMBER( eti660_state::dma_w )
{
	offset -= 0x480;

	m_color = 7;

	if (m_color_on)
	{
		UINT8 colorram_offset = ((offset & 0x1f0) >> 1) | (offset & 0x07);

		if (colorram_offset < 0xc0)
			m_color = m_color_ram[colorram_offset];
	}
	else
		m_color = m_p_videoram[offset] ? 7 : 0;

	m_cti->con_w(0); // HACK
	m_cti->dma_w(space, offset, data);
}

/* PIA6821 Interface */

READ8_MEMBER( eti660_state::pia_pa_r )
{
	/*

	    bit     description

	    PA0     keyboard row 0
	    PA1     keyboard row 1
	    PA2     keyboard row 2
	    PA3     keyboard row 3
	    PA4     keyboard column 0
	    PA5     keyboard column 1
	    PA6     keyboard column 2
	    PA7     keyboard column 3

	*/

	UINT8 i, data = 0xff;

	for (i = 0; i < 4; i++)
		if BIT(m_keylatch, i)
			return m_io_keyboard[i]->read();

	return data;
}

WRITE8_MEMBER( eti660_state::pia_pa_w )
{
	/*

	    bit     description

	    PA0     keyboard row 0
	    PA1     keyboard row 1
	    PA2     keyboard row 2
	    PA3     keyboard row 3
	    PA4     keyboard column 0
	    PA5     keyboard column 1
	    PA6     keyboard column 2
	    PA7     keyboard column 3

	*/

	m_keylatch = data ^ 0xff;
}

void eti660_state::machine_reset()
{
	m_resetcnt = 0;
	m_color_on = 0;
	// fix for F3 soft reboot
	m_maincpu->set_state_int(COSMAC_R0, 0); // set R0 to start of rom
	m_maincpu->set_state_int(COSMAC_P, 0); // set R0 as the PC register
}

void eti660_state::machine_start()
{
	save_item(NAME(m_color_ram));
}

QUICKLOAD_LOAD_MEMBER( eti660_state, eti660 )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int i;
	int quick_addr = 0x600;
	int quick_length;
	dynamic_buffer quick_data;
	int read_;
	int result = IMAGE_INIT_FAIL;

	quick_length = image.length();
	quick_data.resize(quick_length);
	read_ = image.fread( &quick_data[0], quick_length);
	if (read_ != quick_length)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot read the file");
		image.message(" Cannot read the file");
	}
	else
	{
		for (i = 0; i < quick_length; i++)
			if ((quick_addr + i) < 0x1000)
				space.write_byte(i + quick_addr, quick_data[i]);

		/* display a message about the loaded quickload */
		if (strcmp(image.filetype(), "bin") == 0)
			image.message(" Quickload: size=%04X : start=%04X : end=%04X : Press 6 to start",quick_length,quick_addr,quick_addr+quick_length);
		else
			image.message(" Quickload: size=%04X : start=%04X : end=%04X : Press 8 to start",quick_length,quick_addr,quick_addr+quick_length);

		result = IMAGE_INIT_PASS;
	}

	return result;
}

/* Machine Drivers */

static MACHINE_CONFIG_START( eti660, eti660_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, XTAL_8_867238MHz/5)
	MCFG_CPU_PROGRAM_MAP(eti660_map)
	MCFG_CPU_IO_MAP(eti660_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(eti660_state, clear_r))
	MCFG_COSMAC_EF2_CALLBACK(READLINE(eti660_state, ef2_r))
	MCFG_COSMAC_EF4_CALLBACK(READLINE(eti660_state, ef4_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(eti660_state, q_w))
	MCFG_COSMAC_DMAW_CALLBACK(WRITE8(eti660_state, dma_w))

	/* video hardware */
	MCFG_CDP1864_SCREEN_ADD(SCREEN_TAG, XTAL_8_867238MHz/5)
	MCFG_SCREEN_UPDATE_DEVICE(CDP1864_TAG, cdp1864_device, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_CDP1864_ADD(CDP1864_TAG, SCREEN_TAG, XTAL_8_867238MHz/5, GND, INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT), INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_DMAOUT), INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1), NULL, READLINE(eti660_state, rdata_r), READLINE(eti660_state, bdata_r), READLINE(eti660_state, gdata_r))
	MCFG_CDP1864_CHROMINANCE(RES_K(2.2), RES_K(1), RES_K(4.7), RES_K(4.7)) // R7, R5, R6, R4
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_DEVICE_ADD(MC6821_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(eti660_state, pia_pa_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(eti660_state, pia_pa_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE(CDP1802_TAG, cosmac_device, int_w)) MCFG_DEVCB_INVERT
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE(CDP1802_TAG, cosmac_device, int_w)) MCFG_DEVCB_INVERT

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("3K")

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", eti660_state, eti660, "bin,c8,ch8", 2)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( eti660 )
	ROM_REGION( 0x10000, CDP1802_TAG, 0 )
	ROM_LOAD( "eti660.bin", 0x0000, 0x0400, CRC(811dfa62) SHA1(c0c4951e02f873f15560bdc3f35cdf3f99653922) )
ROM_END

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT   CLASS            INIT   COMPANY                             FULLNAME                FLAGS */
COMP( 1981, eti660,     0,      0,      eti660,     eti660, driver_device,    0,    "Electronics Today International",  "ETI-660",  0 )
