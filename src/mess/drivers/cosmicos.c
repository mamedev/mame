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

    - fix direct update handler to make system work again
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
	static const char *const keynames[] = { "ROW1", "ROW2", "ROW3", "ROW4" };
	UINT8 data = 0;
	int i;

	for (i = 0; i < 4; i++)
	{
		if (BIT(m_keylatch, i))
		{
			UINT8 keydata = ioport(keynames[i])->read();

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
	AM_RANGE(0x0000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xcfff) AM_ROM AM_REGION(CDP1802_TAG, 0)
	AM_RANGE(0xff00, 0xffff) AM_RAM
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
	UINT8 data = ioport("DATA")->read();
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

INPUT_CHANGED_MEMBER( cosmicos_state::run )				{ if (!newval) set_cdp1802_mode(MODE_RUN); }
INPUT_CHANGED_MEMBER( cosmicos_state::load )			{ if (!newval) set_cdp1802_mode(MODE_LOAD); }
INPUT_CHANGED_MEMBER( cosmicos_state::cosmicos_pause )	{ if (!newval) set_cdp1802_mode(MODE_PAUSE); }
INPUT_CHANGED_MEMBER( cosmicos_state::reset )			{ if (!newval) set_cdp1802_mode(MODE_RESET); }

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

void cosmicos_state::set_ram_mode()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	if (m_ram_disable)
	{
		program.unmap_readwrite(0xff00, 0xffff);
	}
	else
	{
		if (m_ram_protect)
		{
			program.install_rom(0xff00, 0xffff, ram);
		}
		else
		{
			program.install_ram(0xff00, 0xffff, ram);
		}
	}
}

INPUT_CHANGED_MEMBER( cosmicos_state::memory_protect )
{
	m_ram_protect = newval;

	set_ram_mode();
}

INPUT_CHANGED_MEMBER( cosmicos_state::memory_disable )
{
	m_ram_disable = newval;

	set_ram_mode();
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

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')

	PORT_START("ROW4")
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

static CDP1864_INTERFACE( cosmicos_cdp1864_intf )
{
	CDP1802_TAG,
	SCREEN_TAG,
	CDP1864_INTERLACED,
	DEVCB_LINE_VCC,
	DEVCB_LINE_VCC,
	DEVCB_LINE_VCC,
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT),
	DEVCB_DRIVER_LINE_MEMBER(cosmicos_state, dmaout_w),
	DEVCB_DRIVER_LINE_MEMBER(cosmicos_state, efx_w),
	DEVCB_NULL,
	RES_K(2), // R2
	0, // not connected
	0, // not connected
	0  // not connected
};

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
	UINT8 special = ioport("SPECIAL")->read();

	return BIT(special, 0);
}

READ_LINE_MEMBER( cosmicos_state::ef2_r )
{
	UINT8 special = ioport("SPECIAL")->read();
	int casin = (m_cassette)->input() < 0.0;

	output_set_led_value(LED_CASSETTE, casin);

	return BIT(special, 1) | BIT(special, 3) | casin;
}

READ_LINE_MEMBER( cosmicos_state::ef3_r )
{
	UINT8 special = ioport("SPECIAL")->read();

	return BIT(special, 2) | BIT(special, 3);
}

READ_LINE_MEMBER( cosmicos_state::ef4_r )
{
	return BIT(ioport("BUTTONS")->read(), 0);
}

static COSMAC_SC_WRITE( cosmicos_sc_w )
{
	cosmicos_state *driver_state = device->machine().driver_data<cosmicos_state>();

	int sc1 = BIT(sc, 1);

	if (driver_state->m_sc1 && !sc1)
	{
		driver_state->clear_input_data();
	}

	if (sc1)
	{
		driver_state->m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, CLEAR_LINE);
		driver_state->m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAIN, CLEAR_LINE);
	}

	driver_state->m_sc1 = sc1;
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

static COSMAC_INTERFACE( cosmicos_config )
{
	DEVCB_DRIVER_LINE_MEMBER(cosmicos_state, wait_r),
	DEVCB_DRIVER_LINE_MEMBER(cosmicos_state, clear_r),
	DEVCB_DRIVER_LINE_MEMBER(cosmicos_state, ef1_r),
	DEVCB_DRIVER_LINE_MEMBER(cosmicos_state, ef2_r),
	DEVCB_DRIVER_LINE_MEMBER(cosmicos_state, ef3_r),
	DEVCB_DRIVER_LINE_MEMBER(cosmicos_state, ef4_r),
	DEVCB_DRIVER_LINE_MEMBER(cosmicos_state, q_w),
	DEVCB_DRIVER_MEMBER(cosmicos_state, dma_r),
	DEVCB_NULL,
	cosmicos_sc_w,
	DEVCB_NULL,
	DEVCB_NULL
};


/* Machine Initialization */

void cosmicos_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* initialize LED display */
	m_led->rbi_w(1);

	/* setup memory banking */
	switch (m_ram->size())
	{
	case 256:
		program.unmap_readwrite(0x0000, 0xbfff);
		break;

	case 4*1024:
		program.unmap_readwrite(0x1000, 0xbfff);
		break;
	}

	set_ram_mode();

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

static QUICKLOAD_LOAD( cosmicos )
{
	UINT8 *ptr = image.device().machine().root_device().memregion(CDP1802_TAG)->base();
	int size = image.length();

	/* load image to RAM */
	image.fread(ptr, size);

	return IMAGE_INIT_PASS;
}

/* Machine Driver */

static const cassette_interface cosmicos_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};

static DM9368_INTERFACE( led_intf )
{
	0,
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_CONFIG_START( cosmicos, cosmicos_state )
	/* basic machine hardware */
    MCFG_CPU_ADD(CDP1802_TAG, COSMAC, XTAL_1_75MHz)
    MCFG_CPU_PROGRAM_MAP(cosmicos_mem)
    MCFG_CPU_IO_MAP(cosmicos_io)
	MCFG_CPU_CONFIG(cosmicos_config)

    /* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_cosmicos )
	MCFG_DM9368_ADD(DM9368_TAG, led_intf)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("digit", cosmicos_state, digit_tick, attotime::from_hz(100))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("interrupt", cosmicos_state, int_tick, attotime::from_hz(1000))

	MCFG_CDP1864_SCREEN_ADD(SCREEN_TAG, XTAL_1_75MHz)
	MCFG_SCREEN_UPDATE_DEVICE(CDP1864_TAG, cdp1864_device, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_CDP1864_ADD(CDP1864_TAG, XTAL_1_75MHz, cosmicos_cdp1864_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_QUICKLOAD_ADD("quickload", cosmicos, "bin", 0)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, cosmicos_cassette_interface)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256")
	MCFG_RAM_EXTRA_OPTIONS("4K,48K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( cosmicos )
	ROM_REGION( 0x1000, CDP1802_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "hex", "Hex Monitor" )
	ROMX_LOAD( "hex.ic6",	0x000, 0x400, BAD_DUMP CRC(d25124bf) SHA1(121215ba3a979e1962327ebe73cbadf784c568d9), ROM_BIOS(1) ) /* typed in from manual */
	ROMX_LOAD( "hex.ic7",	0x400, 0x400, BAD_DUMP CRC(364ac81b) SHA1(83936ee6a7ed44632eb290889b98fb9a500f15d4), ROM_BIOS(1) ) /* typed in from manual */
	ROM_SYSTEM_BIOS( 1, "ascii", "ASCII Monitor" )
	ROMX_LOAD( "ascii.ic6", 0x000, 0x400, NO_DUMP, ROM_BIOS(2) )
	ROMX_LOAD( "ascii.ic7", 0x400, 0x400, NO_DUMP, ROM_BIOS(2) )
ROM_END

/* System Drivers */

DIRECT_UPDATE_MEMBER(cosmicos_state::cosmicos_direct_update_handler)
{
	if (m_boot)
	{
		/* force A6 and A7 high */
		direct.explicit_configure(0x0000, 0xffff, 0x3f3f, memregion(CDP1802_TAG)->base() + 0xc0);
		return ~0;
	}

	return address;
}

DRIVER_INIT_MEMBER(cosmicos_state,cosmicos)
{
	address_space &program = machine().device(CDP1802_TAG)->memory().space(AS_PROGRAM);

	program.set_direct_update_handler(direct_update_delegate(FUNC(cosmicos_state::cosmicos_direct_update_handler), this));
}

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT        COMPANY             FULLNAME    FLAGS */
COMP( 1979, cosmicos,	0,		0,		cosmicos,	cosmicos, cosmicos_state,	cosmicos,	"Radio Bulletin",	"Cosmicos",	GAME_NOT_WORKING | GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS )
