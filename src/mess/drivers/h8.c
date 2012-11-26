/***************************************************************************

        Heathkit H8

        12/05/2009 Skeleton driver.

    STATUS:
    - It runs, keyboard works, you can enter data

    TODO:
    - Proper artwork
    - Fix interrupts
    - Seems to crash when GO pressed (needs a subject-matter expert to test)
    - Add load/dump facility (cassette port)

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/beep.h"
#include "h8.lh"


class h8_state : public driver_device
{
public:
	h8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	//m_cass(*this, CASSETTE_TAG),
	m_beep(*this, BEEPER_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	//required_device<cassette_image_device> m_cass;
	required_device<beep_device> m_beep;
	DECLARE_READ8_MEMBER(h8_f0_r);
	DECLARE_WRITE8_MEMBER(h8_f0_w);
	DECLARE_WRITE8_MEMBER(h8_f1_w);
	DECLARE_WRITE8_MEMBER(h8_status_callback);
	DECLARE_WRITE_LINE_MEMBER(h8_inte_callback);
	UINT8 m_digit;
	UINT8 m_segment;
	UINT8 m_irq_ctl;
	UINT8 m_ff_b;
	virtual void machine_reset();
	TIMER_DEVICE_CALLBACK_MEMBER(h8_irq_pulse);
};


#define H8_CLOCK (XTAL_12_288MHz / 6)
#define H8_BEEP_FRQ (H8_CLOCK / 1024)
#define H8_IRQ_PULSE (H8_BEEP_FRQ / 2)


TIMER_DEVICE_CALLBACK_MEMBER(h8_state::h8_irq_pulse)
{
	if (m_irq_ctl & 1)
		machine().device("maincpu")->execute().set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xcf);
}

READ8_MEMBER( h8_state::h8_f0_r )
{
    // reads the keyboard

    // The following not emulated, can occur any time even if keyboard not being scanned
    // - if 0 and RTM pressed, causes int10
    // - if 0 and RST pressed, resets cpu

	UINT8 i,keyin,data = 0xff;

	keyin = ioport("X0")->read();
	if (keyin != 0xff)
	{
		for (i = 1; i < 8; i++)
			if (!BIT(keyin,i))
				data &= ~(i<<1);
		data &= 0xfe;
	}

	keyin = ioport("X1")->read();
	if (keyin != 0xff)
	{
		for (i = 1; i < 8; i++)
			if (!BIT(keyin,i))
				data &= ~(i<<5);
		data &= 0xef;
	}
	return data;
}

WRITE8_MEMBER( h8_state::h8_f0_w )
{
    // this will always turn off int10 that was set by the timer
    // d0-d3 = digit select
    // d4 = int20 is allowed
    // d5 = mon led
    // d6 = int10 is allowed
    // d7 = beeper enable

	m_digit = data & 15;
	if (m_digit) output_set_digit_value(m_digit, m_segment);

	output_set_value("mon_led",(data & 0x20) ? 0 : 1);
	beep_set_state(m_beep, (data & 0x80) ? 0 : 1);

	machine().device("maincpu")->execute().set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	m_irq_ctl &= 0xf0;
	if (data & 0x40) m_irq_ctl |= 1;
	if (~data & 0x10) m_irq_ctl |= 2;
}

WRITE8_MEMBER( h8_state::h8_f1_w )
{
    //d7 segment dot
    //d6 segment f
    //d5 segment e
    //d4 segment d
    //d3 segment c
    //d2 segment b
    //d1 segment a
    //d0 segment g

	m_segment = 0xff ^ BITSWAP8(data, 7, 0, 6, 5, 4, 3, 2, 1);
	if (m_digit) output_set_digit_value(m_digit, m_segment);
}

static ADDRESS_MAP_START(h8_mem, AS_PROGRAM, 8, h8_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x2000, 0x9fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( h8_io, AS_IO, 8, h8_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf0) AM_READWRITE(h8_f0_r,h8_f0_w)
	AM_RANGE(0xf1, 0xf1) AM_WRITE(h8_f1_w)
	//AM_RANGE(0xf8, 0xf8) load and dump data port
	//AM_RANGE(0xf9, 0xf9) load and dump control port
	//AM_RANGE(0xfa, 0xfa) console data port
	//AM_RANGE(0xfb, 0xfb) console control port
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( h8 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 SP") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 AF") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 BC") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 DE GO") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 HL IN") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 PC OUT") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 SI") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 LOAD") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 DUMP") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("* CANCEL") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("// ALTER RST") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("# MEM RTM") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("@ REG") PORT_CODE(KEYCODE_R)
INPUT_PORTS_END

void h8_state::machine_reset()
{
	beep_set_frequency(m_beep, H8_BEEP_FRQ);
	output_set_value("pwr_led", 0);
	m_irq_ctl = 1;
}

WRITE_LINE_MEMBER( h8_state::h8_inte_callback )
{
        // operate the ION LED
	output_set_value("ion_led",(state) ? 0 : 1);
	m_irq_ctl &= 0x7f | ((state) ? 0 : 0x80);
}

WRITE8_MEMBER( h8_state::h8_status_callback )
{
/* This is rather messy, but basically there are 2 D flipflops, one drives the other,
the data is /INTE while the clock is /M1. If the system is in Single Instruction mode,
a int20 (output of 2nd flipflop) will occur after 4 M1 steps, to pause the running program.
But, all of this can only occur if bit 5 of port F0 is low. */

	UINT8 state = (data & I8085_STATUS_M1) ? 0 : 1;
	UINT8 c,a = (m_irq_ctl & 0x80) ? 1 : 0;

	if (m_irq_ctl & 2)
	{
		if (!state) // rising pulse to push data through flipflops
		{
			c=m_ff_b^1; // from /Q of 2nd flipflop
			m_ff_b=a; // from Q of 1st flipflop
			if (c)
				machine().device("maincpu")->execute().set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xd7);
		}
	}
	else
	{ // flipflops are 'set'
		c=0;
		m_ff_b=1;
	}


        // operate the RUN LED
	output_set_value("run_led", state);
}

static I8085_CONFIG( h8_cpu_config )
{
	DEVCB_DRIVER_MEMBER(h8_state, h8_status_callback),		/* Status changed callback */
	DEVCB_DRIVER_LINE_MEMBER(h8_state, h8_inte_callback),			/* INTE changed callback */
	DEVCB_NULL,					/* SID changed callback (I8085A only) */
	DEVCB_NULL					/* SOD changed callback (I8085A only) */
};

static MACHINE_CONFIG_START( h8, h8_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, H8_CLOCK)
	MCFG_CPU_PROGRAM_MAP(h8_mem)
	MCFG_CPU_IO_MAP(h8_io)
	MCFG_CPU_CONFIG(h8_cpu_config)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("h8_timer", h8_state, h8_irq_pulse, attotime::from_hz(H8_IRQ_PULSE))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_h8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(BEEPER_TAG, BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( h8 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2708_444-13_pam8.rom", 0x0000, 0x0400, CRC(e0745513) SHA1(0e170077b6086be4e5cd10c17e012c0647688c39))

	ROM_REGION( 0x10000, "otherroms", ROMREGION_ERASEFF )
	ROM_LOAD( "2708_444-13_pam8go.rom", 0x0000, 0x0400, CRC(9dbad129) SHA1(72421102b881706877f50537625fc2ab0b507752))
	ROM_LOAD( "2716_444-13_pam8at.rom", 0x0000, 0x0800, CRC(fd95ddc1) SHA1(eb1f272439877239f745521139402f654e5403af))
	ROM_LOAD( "2716_444-19_h17.rom", 0x0000, 0x0800, CRC(26e80ae3) SHA1(0c0ee95d7cb1a760f924769e10c0db1678f2435c))
	ROM_LOAD( "2732_444-70_xcon8.rom", 0x0000, 0x1000, CRC(b04368f4) SHA1(965244277a3a8039a987e4c3593b52196e39b7e7))
	ROM_LOAD( "2732_444-140_pam37.rom", 0x0000, 0x1000, CRC(53a540db) SHA1(90082d02ffb1d27e8172b11fff465bd24343486e))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1977, h8,  0,       0,	h8, 	h8, driver_device,	 0, "Heath, Inc.", "Heathkit H8", GAME_NOT_WORKING )
