// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    COSMICOS

    http://retro.hansotten.nl/index.php?page=1802-cosmicos


    HEX-monitor

    0 - start user program
    1 - inspect and/or change memory
    2 - write memory block to cassette
    3 - read memory block from cassette
    4 - move memory block
    5 - write memory block to EPROM
    C - start user program from address 0000

*/

/*

    TODO:

    - display interface INH
    - 2 segment display
    - single step
    - ascii monitor
    - PPI 8255
    - Floppy WD1793
    - COM8017 UART to printer

*/

#include "includes/cosmicos.h"
#include "cosmicos.lh"

enum
{
	MODE_RUN,
	MODE_LOAD,
	MODE_PAUSE,
	MODE_RESET
};

/* Read/Write Handlers */

READ8_MEMBER( cosmicos_state::read )
{
	if (m_boot) offset |= 0xc0c0;

	UINT8 data = 0;

	if (offset < 0xc000)
	{
		// TODO
	}
	else if (offset < 0xd000)
	{
		data = m_rom->base()[offset & 0xfff];
	}
	else if (!m_ram_disable && (offset >= 0xff00))
	{
		data = m_ram->pointer()[offset & 0xff];
	}

	return data;
}

WRITE8_MEMBER( cosmicos_state::write )
{
	if (m_boot) offset |= 0xc0c0;

	if (offset < 0xc000)
	{
		// TODO
	}
	else if (!m_ram_disable && !m_ram_protect && (offset >= 0xff00))
	{
		m_ram->pointer()[offset & 0xff] = data;
	}
}

READ8_MEMBER( cosmicos_state::video_off_r )
{
	UINT8 data = 0;

	if (!m_q)
	{
		data = m_cti->dispoff_r(space, 0);
	}

	return data;
}

READ8_MEMBER( cosmicos_state::video_on_r )
{
	UINT8 data = 0;

	if (!m_q)
	{
		data = m_cti->dispon_r(space, 0);
	}

	return data;
}

WRITE8_MEMBER( cosmicos_state::audio_latch_w )
{
	if (m_q)
	{
		m_cti->tone_latch_w(space, 0, data);
	}
}

READ8_MEMBER( cosmicos_state::hex_keyboard_r )
{
	UINT8 data = 0;
	int i;

	for (i = 0; i < 4; i++)
	{
		if (BIT(m_keylatch, i))
		{
			UINT8 keydata = m_key_row[i]->read();

			if (BIT(keydata, 0)) data |= 0x01;
			if (BIT(keydata, 1)) data |= 0x02;
			if (BIT(keydata, 2)) data |= 0x04;
			if (BIT(keydata, 3)) data |= 0x06;
		}
	}

	return data;
}

WRITE8_MEMBER( cosmicos_state::hex_keylatch_w )
{
	m_keylatch = data & 0x0f;
}

READ8_MEMBER( cosmicos_state::reset_counter_r )
{
	m_counter = 0;

	return 0;
}

WRITE8_MEMBER( cosmicos_state::segment_w )
{
	m_counter++;

	if (m_counter == 10)
	{
		m_counter = 0;
	}

	if ((m_counter > 0) && (m_counter < 9))
	{
		output_set_digit_value(10 - m_counter, data);
	}
}

READ8_MEMBER( cosmicos_state::data_r )
{
	return m_data;
}

WRITE8_MEMBER( cosmicos_state::display_w )
{
	m_segment = data;
}

/* Memory Maps */

static ADDRESS_MAP_START( cosmicos_mem, AS_PROGRAM, 8, cosmicos_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cosmicos_io, AS_IO, 8, cosmicos_state )
//  AM_RANGE(0x00, 0x00)
	AM_RANGE(0x01, 0x01) AM_READ(video_on_r)
	AM_RANGE(0x02, 0x02) AM_READWRITE(video_off_r, audio_latch_w)
//  AM_RANGE(0x03, 0x03)
//  AM_RANGE(0x04, 0x04)
	AM_RANGE(0x05, 0x05) AM_READWRITE(hex_keyboard_r, hex_keylatch_w)
	AM_RANGE(0x06, 0x06) AM_READWRITE(reset_counter_r, segment_w)
	AM_RANGE(0x07, 0x07) AM_READWRITE(data_r, display_w)
ADDRESS_MAP_END

/* Input Ports */

INPUT_CHANGED_MEMBER( cosmicos_state::data )
{
	UINT8 data = m_io_data->read();
	int i;

	for (i = 0; i < 8; i++)
	{
		if (!BIT(data, i))
		{
			m_data |= (1 << i);
			output_set_led_value(LED_D0 - i, 1);
		}
	}
}

INPUT_CHANGED_MEMBER( cosmicos_state::enter )
{
	if (!newval && !m_wait && !m_clear)
	{
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAIN, ASSERT_LINE);
	}
}

INPUT_CHANGED_MEMBER( cosmicos_state::single_step )
{
	// if in PAUSE mode, set RUN mode until TPB=active
}

void cosmicos_state::set_cdp1802_mode(int mode)
{
	output_set_led_value(LED_RUN, 0);
	output_set_led_value(LED_LOAD, 0);
	output_set_led_value(LED_PAUSE, 0);
	output_set_led_value(LED_RESET, 0);

	switch (mode)
	{
	case MODE_RUN:
		output_set_led_value(LED_RUN, 1);

		m_wait = 1;
		m_clear = 1;
		break;

	case MODE_LOAD:
		output_set_led_value(LED_LOAD, 1);

		m_wait = 0;
		m_clear = 0;
		break;

	case MODE_PAUSE:
		output_set_led_value(LED_PAUSE, 1);

		m_wait = 1;
		m_clear = 0;
		break;

	case MODE_RESET:
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, CLEAR_LINE);
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAIN, CLEAR_LINE);

		m_wait = 1;
		m_clear = 0;
		m_boot = 1;

		output_set_led_value(LED_RESET, 1);
		break;
	}
}

INPUT_CHANGED_MEMBER( cosmicos_state::run )             { if (!newval) set_cdp1802_mode(MODE_RUN); }
INPUT_CHANGED_MEMBER( cosmicos_state::load )            { if (!newval) set_cdp1802_mode(MODE_LOAD); }
INPUT_CHANGED_MEMBER( cosmicos_state::cosmicos_pause )  { if (!newval) set_cdp1802_mode(MODE_PAUSE); }
INPUT_CHANGED_MEMBER( cosmicos_state::reset )           { if (!newval) set_cdp1802_mode(MODE_RESET); }

void cosmicos_state::clear_input_data()
{
	int i;

	m_data = 0;

	for (i = 0; i < 8; i++)
	{
		output_set_led_value(LED_D0 - i, 0);
	}
}

INPUT_CHANGED_MEMBER( cosmicos_state::clear_data )
{
	clear_input_data();
}

INPUT_CHANGED_MEMBER( cosmicos_state::memory_protect )
{
	m_ram_protect = newval;
}

INPUT_CHANGED_MEMBER( cosmicos_state::memory_disable )
{
	m_ram_disable = newval;
}

static INPUT_PORTS_START( cosmicos )
	PORT_START("DATA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7') PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, data, 0)

	PORT_START("BUTTONS")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, enter, 0)
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Single Step") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, single_step, 0)
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_NAME("Run") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, run, 0)
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_NAME("Load") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, load, 0)
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_NAME(DEF_STR( Pause )) PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, cosmicos_pause, 0)
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, reset, 0)
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Clear Data") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, clear_data, 0)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_NAME("Memory Protect") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, memory_protect, 0) PORT_TOGGLE
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_NAME("Memory Disable") PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicos_state, memory_disable, 0) PORT_TOGGLE

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RET") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEC") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("REQ") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SEQ") PORT_CODE(KEYCODE_F4)
INPUT_PORTS_END

/* Video */

TIMER_DEVICE_CALLBACK_MEMBER(cosmicos_state::digit_tick)
{
	m_digit = !m_digit;

	output_set_digit_value(m_digit, m_segment);
}

TIMER_DEVICE_CALLBACK_MEMBER(cosmicos_state::int_tick)
{
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, ASSERT_LINE);
}

WRITE_LINE_MEMBER( cosmicos_state::dmaout_w )
{
	m_dmaout = state;
}

WRITE_LINE_MEMBER( cosmicos_state::efx_w )
{
	m_efx = state;
}

/* CDP1802 Configuration */

READ_LINE_MEMBER( cosmicos_state::wait_r )
{
	return m_wait;
}

READ_LINE_MEMBER( cosmicos_state::clear_r )
{
	return m_clear;
}

READ_LINE_MEMBER( cosmicos_state::ef1_r )
{
	UINT8 special = m_special->read();

	return BIT(special, 0);
}

READ_LINE_MEMBER( cosmicos_state::ef2_r )
{
	UINT8 special = m_special->read();
	int casin = (m_cassette)->input() < 0.0;

	output_set_led_value(LED_CASSETTE, casin);

	return BIT(special, 1) | BIT(special, 3) | casin;
}

READ_LINE_MEMBER( cosmicos_state::ef3_r )
{
	UINT8 special = m_special->read();

	return BIT(special, 2) | BIT(special, 3);
}

READ_LINE_MEMBER( cosmicos_state::ef4_r )
{
	return BIT(m_buttons->read(), 0);
}

WRITE_LINE_MEMBER( cosmicos_state::q_w )
{
	/* cassette */
	m_cassette->output(state ? +1.0 : -1.0);

	/* boot */
	if (state) m_boot = 0;

	/* CDP1864 audio enable */
	m_cti->aoe_w(state);

	m_q = state;
}

READ8_MEMBER( cosmicos_state::dma_r )
{
	return m_data;
}

WRITE8_MEMBER( cosmicos_state::sc_w )
{
	int sc1 = BIT(data, 1);

	if (m_sc1 && !sc1)
	{
		clear_input_data();
	}

	if (sc1)
	{
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, CLEAR_LINE);
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAIN, CLEAR_LINE);
	}

	m_sc1 = sc1;
}

/* Machine Initialization */

void cosmicos_state::machine_start()
{
	/* initialize LED display */
	m_led->rbi_w(1);

	// find keyboard rows
	m_key_row[0] = m_y1;
	m_key_row[1] = m_y2;
	m_key_row[2] = m_y3;
	m_key_row[3] = m_y4;

	/* register for state saving */
	save_item(NAME(m_wait));
	save_item(NAME(m_clear));
	save_item(NAME(m_sc1));
	save_item(NAME(m_data));
	save_item(NAME(m_boot));
	save_item(NAME(m_ram_protect));
	save_item(NAME(m_ram_disable));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_segment));
	save_item(NAME(m_digit));
	save_item(NAME(m_counter));
	save_item(NAME(m_q));
	save_item(NAME(m_dmaout));
	save_item(NAME(m_efx));
	save_item(NAME(m_video_on));
}

void cosmicos_state::machine_reset()
{
	set_cdp1802_mode(MODE_RESET);
}

/* Quickload */

QUICKLOAD_LOAD_MEMBER( cosmicos_state, cosmicos )
{
	UINT8 *ptr = m_rom->base();
	int size = image.length();

	/* load image to RAM */
	image.fread(ptr, size);

	return IMAGE_INIT_PASS;
}

/* Machine Driver */

static MACHINE_CONFIG_START( cosmicos, cosmicos_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, XTAL_1_75MHz)
	MCFG_CPU_PROGRAM_MAP(cosmicos_mem)
	MCFG_CPU_IO_MAP(cosmicos_io)
	MCFG_COSMAC_WAIT_CALLBACK(READLINE(cosmicos_state, wait_r))
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(cosmicos_state, clear_r))
	MCFG_COSMAC_EF1_CALLBACK(READLINE(cosmicos_state, ef1_r))
	MCFG_COSMAC_EF2_CALLBACK(READLINE(cosmicos_state, ef2_r))
	MCFG_COSMAC_EF3_CALLBACK(READLINE(cosmicos_state, ef3_r))
	MCFG_COSMAC_EF4_CALLBACK(READLINE(cosmicos_state, ef4_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(cosmicos_state, q_w))
	MCFG_COSMAC_DMAR_CALLBACK(READ8(cosmicos_state, dma_r))
	MCFG_COSMAC_SC_CALLBACK(WRITE8(cosmicos_state, sc_w))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_cosmicos )
	MCFG_DEVICE_ADD(DM9368_TAG, DM9368, 0)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("digit", cosmicos_state, digit_tick, attotime::from_hz(100))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("interrupt", cosmicos_state, int_tick, attotime::from_hz(1000))

	MCFG_CDP1864_SCREEN_ADD(SCREEN_TAG, XTAL_1_75MHz)
	MCFG_SCREEN_UPDATE_DEVICE(CDP1864_TAG, cdp1864_device, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_CDP1864_ADD(CDP1864_TAG, SCREEN_TAG, XTAL_1_75MHz, GND, INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT), WRITELINE(cosmicos_state, dmaout_w), WRITELINE(cosmicos_state, efx_w), NULL, VCC, VCC, VCC)
	MCFG_CDP1864_CHROMINANCE(RES_K(2), 0, 0, 0) // R2
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_QUICKLOAD_ADD("quickload", cosmicos_state, cosmicos, "bin", 0)
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256")
	MCFG_RAM_EXTRA_OPTIONS("4K,48K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( cosmicos )
	ROM_REGION( 0x1000, CDP1802_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "hex", "Hex Monitor" )
	ROMX_LOAD( "hex.ic6",   0x000, 0x400, BAD_DUMP CRC(d25124bf) SHA1(121215ba3a979e1962327ebe73cbadf784c568d9), ROM_BIOS(1) ) /* typed in from manual */
	ROMX_LOAD( "hex.ic7",   0x400, 0x400, BAD_DUMP CRC(364ac81b) SHA1(83936ee6a7ed44632eb290889b98fb9a500f15d4), ROM_BIOS(1) ) /* typed in from manual */
	ROM_SYSTEM_BIOS( 1, "ascii", "ASCII Monitor" )
	ROMX_LOAD( "ascii.ic6", 0x000, 0x400, NO_DUMP, ROM_BIOS(2) )
	ROMX_LOAD( "ascii.ic7", 0x400, 0x400, NO_DUMP, ROM_BIOS(2) )
ROM_END

/* System Drivers */

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT        COMPANY             FULLNAME    FLAGS */
COMP( 1979, cosmicos,   0,      0,      cosmicos,   cosmicos, driver_device,   0,   "Radio Bulletin",   "Cosmicos", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
