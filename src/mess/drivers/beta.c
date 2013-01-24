/***************************************************************************

    Beta Computer

    http://retro.hansotten.nl/index.php?page=beta-computer

    When first started, press G to begin.

    Pasting:
        0-F : as is
        M (inc) : ^
        AD : -
        DA : =
        GO : X

    Test Paste:
        X-0011=11^22^33^44^55^66^77^88^99^-0011
        Now press M to confirm the data has been entered.

****************************************************************************/

/*

    TODO:

    - write EPROM back to file

*/

#include "includes/beta.h"
#include "beta.lh"

/* Memory Maps */

static ADDRESS_MAP_START( beta_mem, AS_PROGRAM, 8, beta_state )
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x7f00) AM_RAM // 6532 RAM
	AM_RANGE(0x0080, 0x00ff) AM_MIRROR(0x7f00) AM_DEVREADWRITE_LEGACY(M6532_TAG, riot6532_r, riot6532_w)
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x7800) AM_ROM
ADDRESS_MAP_END

/* Input Ports */

INPUT_CHANGED_MEMBER( beta_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( beta )
	PORT_START("Q6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Q7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Q8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DA") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Q9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("AD") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, beta_state, trigger_reset, 0)
INPUT_PORTS_END

/* M6532 Interface */

TIMER_CALLBACK_MEMBER(beta_state::led_refresh)
{
	if (m_ls145_p < 6)
	{
		output_set_digit_value(m_ls145_p, m_segment);
	}
}

READ8_MEMBER( beta_state::riot_pa_r )
{
	/*

	    bit     description

	    PA0     2716 D0, segment D, key bit 0
	    PA1     2716 D1, segment E, key bit 1
	    PA2     2716 D2, segment C, key bit 2
	    PA3     2716 D3, segment G, key bit 3
	    PA4     2716 D4, segment F, key bit 4
	    PA5     2716 D5, segment B
	    PA6     2716 D6, segment A
	    PA7     2716 D7

	*/

	UINT8 data = 0xff;

	switch (m_ls145_p)
	{
	case 6: data &= m_q6->read(); break;
	case 7: data &= m_q7->read(); break;
	case 8: data &= m_q8->read(); break;
	case 9: data &= m_q9->read(); break;
	default:
		if (!m_eprom_oe && !m_eprom_ce)
		{
			data = m_eprom[m_eprom_addr & 0x7ff];
			popmessage("EPROM read %04x = %02x\n", m_eprom_addr & 0x7ff, data);
		}
	}

	return data;
}

WRITE8_MEMBER( beta_state::riot_pa_w )
{
	/*

	    bit     description

	    PA0     2716 D0, segment D, key bit 0
	    PA1     2716 D1, segment E, key bit 1
	    PA2     2716 D2, segment C, key bit 2
	    PA3     2716 D3, segment G, key bit 3
	    PA4     2716 D4, segment F, key bit 4
	    PA5     2716 D5, segment B
	    PA6     2716 D6, segment A
	    PA7     2716 D7

	*/

//  logerror("PA %02x\n", data);

	/* display */
	m_segment = BITSWAP8(data, 7, 3, 4, 1, 0, 2, 5, 6) & 0x7f;
	m_led_refresh_timer->adjust(attotime::from_usec(70));

	/* EPROM data */
	m_eprom_data = data;
}

READ8_MEMBER( beta_state::riot_pb_r )
{
	return 0;
}

WRITE8_MEMBER( beta_state::riot_pb_w )
{
	/*

	    bit     description

	    PB0     74LS145 P0
	    PB1     74LS145 P1
	    PB2     74LS145 P2
	    PB3     74LS145 P3, 74LS164 D
	    PB4     loudspeaker, data led
	    PB5     address led, 74LS164 CP
	    PB6     2716 _OE
	    PB7     2716 _CE

	*/

	//logerror("PB %02x %02x\n", data, olddata);

	/* display */
	m_ls145_p = data & 0x0f;

	/* speaker */
	speaker_level_w(m_speaker, !BIT(data, 4));

	/* address led */
	output_set_led_value(0, BIT(data, 5));

	/* data led */
	output_set_led_value(1, !BIT(data, 5));

	/* EPROM address shift */
	if (!BIT(m_old_data, 5) && BIT(data, 5))
	{
		m_eprom_addr <<= 1;
		m_eprom_addr |= BIT(m_old_data, 3);
	}

	/* EPROM output enable */
	m_eprom_oe = BIT(data, 6);

	/* EPROM chip enable */
	m_eprom_ce = BIT(data, 7);

	if (BIT(data, 6) && (!BIT(m_old_data, 7) && BIT(data, 7)))
	{
		popmessage("EPROM write %04x = %02x\n", m_eprom_addr & 0x7ff, m_eprom_data);
		m_eprom[m_eprom_addr & 0x7ff] &= m_eprom_data;
	}

	m_old_data = data;
}

static const riot6532_interface beta_riot_interface =
{
	DEVCB_DRIVER_MEMBER(beta_state, riot_pa_r),
	DEVCB_DRIVER_MEMBER(beta_state, riot_pb_r),
	DEVCB_DRIVER_MEMBER(beta_state, riot_pa_w),
	DEVCB_DRIVER_MEMBER(beta_state, riot_pb_w),
	DEVCB_CPU_INPUT_LINE(M6502_TAG, M6502_IRQ_LINE)
};

/* Quickload */

static DEVICE_IMAGE_UNLOAD( beta_eprom )
{
	UINT8 *ptr = image.device().machine().root_device().memregion(EPROM_TAG)->base();

	image.fwrite(ptr, 0x800);
}

/* Machine Initialization */

void beta_state::machine_start()
{
	m_led_refresh_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(beta_state::led_refresh),this));

	// find memory regions
	m_eprom = memregion(EPROM_TAG)->base();

	// state saving
	save_item(NAME(m_eprom_oe));
	save_item(NAME(m_eprom_ce));
	save_item(NAME(m_eprom_addr));
	save_item(NAME(m_eprom_data));
	save_item(NAME(m_old_data));
	save_item(NAME(m_ls145_p));
	save_item(NAME(m_segment));
}

/* Machine Driver */

static MACHINE_CONFIG_START( beta, beta_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_4MHz/4)
	MCFG_CPU_PROGRAM_MAP(beta_mem)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_beta )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_RIOT6532_ADD(M6532_TAG, XTAL_4MHz/4, beta_riot_interface)

	/* EPROM socket */
	MCFG_CARTSLOT_ADD(EPROM_TAG)
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_UNLOAD(beta_eprom)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( beta )
	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "beta.rom", 0x8000, 0x0800, CRC(d42fdb17) SHA1(595225a0cd43dd76c46b2aff6c0f27d5991cc4f0))

	ROM_REGION( 0x800, EPROM_TAG, ROMREGION_ERASEFF )
	ROM_CART_LOAD( EPROM_TAG, 0x0000, 0x0800, ROM_FULLSIZE )
ROM_END

/* System Drivers */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY    FULLNAME    FLAGS */
COMP( 1984, beta,   0,      0,      beta,   beta, driver_device,   0,    "Pitronics", "Beta", GAME_SUPPORTS_SAVE )
