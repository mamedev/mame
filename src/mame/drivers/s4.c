/***********************************************************************************

    Pinball
    Williams System 4

    No schematics have been located as yet.

    Phoenix and Pokerino are listed as System 4 systems, but use System 3 roms.
    They have been moved to s3.c, and are working there.

    Only Flash and Stellar Wars are pinball machines. The remainder are so-called
    "Shuffle", which is a flat board with an air-driven puck and 10 bowling pins.
    You must push the puck as if it was a bowling ball, and score strikes and spares.
    Since the maximum score is 300, the displays have 4 digits and 6 can play. They
    are Williams System 5 and should be moved to a separate driver.

    Stellar Wars - it works, but the music is half-missing.
    Flash - coins and start works, but not much else.

ToDo:
- Almost Everything



************************************************************************************/

#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "s4.lh"


class s4_state : public genpin_class
{
public:
	s4_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_audiocpu(*this, "audiocpu"),
	m_dac(*this, "dac"),
	m_pia22(*this, "pia22"),
	m_pia24(*this, "pia24"),
	m_pia28(*this, "pia28"),
	m_pia30(*this, "pia30"),
	m_pias(*this, "pias")
	{ }

	DECLARE_READ8_MEMBER(dac_r);
	DECLARE_WRITE8_MEMBER(dac_w);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(lamp0_w);
	DECLARE_WRITE8_MEMBER(lamp1_w);
	DECLARE_WRITE8_MEMBER(sol0_w);
	DECLARE_WRITE8_MEMBER(sol1_w);
	DECLARE_READ8_MEMBER(dips_r);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_READ_LINE_MEMBER(pia28_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia28_cb1_r);
	DECLARE_READ_LINE_MEMBER(pias_cb1_r);
	DECLARE_WRITE_LINE_MEMBER(pia22_ca2_w) { }; //ST5
	DECLARE_WRITE_LINE_MEMBER(pia22_cb2_w) { }; //ST-solenoids enable
	DECLARE_WRITE_LINE_MEMBER(pia24_ca2_w) { }; //ST2
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { }; //ST1
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { }; //diag leds enable
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { }; //ST6
	DECLARE_WRITE_LINE_MEMBER(pia30_ca2_w) { }; //ST4
	DECLARE_WRITE_LINE_MEMBER(pia30_cb2_w) { }; //ST3
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);
	DECLARE_MACHINE_RESET(s4);
	DECLARE_MACHINE_RESET(s4a);
protected:

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<dac_device> m_dac;
	required_device<pia6821_device> m_pia22;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
	optional_device<pia6821_device> m_pias;
private:
	UINT8 m_t_c;
	UINT8 m_sound_data;
	UINT8 m_strobe;
	UINT8 m_kbdrow;
	bool m_cb1;
	bool m_data_ok;
	bool m_chimes;
};

static ADDRESS_MAP_START( s4_main_map, AS_PROGRAM, 8, s4_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2200, 0x2203) AM_DEVREADWRITE("pia22", pia6821_device, read, write) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_DEVREADWRITE("pia24", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("pia28", pia6821_device, read, write) // display
	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("pia30", pia6821_device, read, write) // inputs
	AM_RANGE(0x6000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( s4_audio_map, AS_PROGRAM, 8, s4_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0400, 0x0403) AM_DEVREADWRITE("pias", pia6821_device, read, write) // sounds
	AM_RANGE(0x7800, 0x7fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( s4 )
	PORT_START("X0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("X20")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("X40")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("X80")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SND")
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Music1") PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Music2") PORT_CODE(KEYCODE_4) PORT_TOGGLE

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, s4_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, s4_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Manual/Auto") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_8)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "SW01" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x02, "SW02" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x04, "SW03" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x08, "SW04" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x10, "SW05" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x20, "SW06" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x40, "SW07" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x80, "SW08" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "SW11" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x02, "SW12" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x04, "SW13" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x08, "SW14" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x10, "SW15" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x20, "SW16" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x40, "SW17" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x80, "SW18" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
INPUT_PORTS_END

MACHINE_RESET_MEMBER( s4_state, s4 )
{
	m_t_c = 0;
	m_chimes = 1;
}

MACHINE_RESET_MEMBER( s4_state, s4a )
{
	m_t_c = 0;
	m_chimes = 0;
}

INPUT_CHANGED_MEMBER( s4_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( s4_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if ((newval==CLEAR_LINE) && !m_chimes)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER( s4_state::sol0_w )
{
	if (BIT(data, 4))
		m_samples->start(2, 5); // outhole
}

WRITE8_MEMBER( s4_state::sol1_w )
{
	if (m_chimes)
	{
		if (BIT(data, 0))
			m_samples->start(1, 1); // 10 chime

		if (BIT(data, 1))
			m_samples->start(2, 2); // 100 chime

		if (BIT(data, 2))
			m_samples->start(3, 3); // 1000 chime

		// we don't have a 10k chime in samples yet
		//if (BIT(data, 3))
			//m_samples->start(1, x); // 10k chime
	}
	else
	{
		m_sound_data = ioport("SND")->read();
		if (BIT(data, 0))
			m_sound_data &= 0xfe;

		if (BIT(data, 1))
			m_sound_data &= 0xfd;

		if (BIT(data, 2))
			m_sound_data &= 0xfb;

		if (BIT(data, 3))
			m_sound_data &= 0xf7;

		if (BIT(data, 4))
			m_sound_data &= 0x7f;

		m_cb1 = ((m_sound_data & 0x9f) != 0x9f);

		m_pias->cb1_w(m_cb1);
	}

	if (BIT(data, 5))
		m_samples->start(0, 6); // knocker
}

static const pia6821_interface pia22_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_LINE_GND,		/* line CA1 in */
	DEVCB_LINE_GND,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s4_state, sol0_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s4_state, sol1_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s4_state, pia22_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s4_state, pia22_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

WRITE8_MEMBER( s4_state::lamp0_w )
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

WRITE8_MEMBER( s4_state::lamp1_w )
{
}

static const pia6821_interface pia24_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_LINE_GND,		/* line CA1 in */
	DEVCB_LINE_GND,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s4_state, lamp0_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s4_state, lamp1_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s4_state, pia24_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s4_state, pia24_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

READ_LINE_MEMBER( s4_state::pia28_ca1_r )
{
	return BIT(ioport("DIAGS")->read(), 2); // advance button
}

READ_LINE_MEMBER( s4_state::pia28_cb1_r )
{
	return BIT(ioport("DIAGS")->read(), 3); // auto/manual switch
}

READ8_MEMBER( s4_state::dips_r )
{
	if (BIT(ioport("DIAGS")->read(), 4) )
	{
		switch (m_strobe)
		{
		case 0:
			return ioport("DSW0")->read() & 15;
			break;
		case 1:
			return ioport("DSW0")->read() << 4;
			break;
		case 2:
			return ioport("DSW1")->read() & 15;
			break;
		case 3:
			return ioport("DSW1")->read() << 4;
			break;
		}
	}
	return 0xff;
}

WRITE8_MEMBER( s4_state::dig0_w )
{
	m_strobe = data & 15;
	m_data_ok = true;
	output_set_value("led0", BIT(data, 4));
	output_set_value("led1", BIT(data, 5));
}

WRITE8_MEMBER( s4_state::dig1_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // MC14558
	if (m_data_ok)
	{
		output_set_digit_value(m_strobe+16, patterns[data&15]);
		output_set_digit_value(m_strobe, patterns[data>>4]);
	}
	m_data_ok = false;
}

static const pia6821_interface pia28_intf =
{
	DEVCB_DRIVER_MEMBER(s4_state, dips_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_DRIVER_LINE_MEMBER(s4_state, pia28_ca1_r),		/* line CA1 in */
	DEVCB_DRIVER_LINE_MEMBER(s4_state, pia28_cb1_r),		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s4_state, dig0_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s4_state, dig1_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s4_state, pia28_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s4_state, pia28_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

READ8_MEMBER( s4_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"X%X",m_kbdrow);
	return ~ioport(kbdrow)->read();
}

WRITE8_MEMBER( s4_state::switch_w )
{
	m_kbdrow = data;
}

static const pia6821_interface pia30_intf =
{
	DEVCB_DRIVER_MEMBER(s4_state, switch_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_LINE_GND,		/* line CA1 in */
	DEVCB_LINE_GND,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_DRIVER_MEMBER(s4_state, switch_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s4_state, pia30_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s4_state, pia30_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

READ_LINE_MEMBER( s4_state::pias_cb1_r )
{
	return m_cb1;
}

READ8_MEMBER( s4_state::dac_r )
{
	return m_sound_data;
}

WRITE8_MEMBER( s4_state::dac_w )
{
	m_dac->write_unsigned8(data);
}

static const pia6821_interface pias_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_DRIVER_MEMBER(s4_state, dac_r),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_DRIVER_LINE_MEMBER(s4_state, pias_cb1_r),		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s4_state, dac_w),		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("audiocpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("audiocpu", M6800_IRQ_LINE)		/* IRQB */
};

TIMER_DEVICE_CALLBACK_MEMBER( s4_state::irq)
{
	if (m_t_c > 0x70)
		m_maincpu->set_input_line(M6800_IRQ_LINE, ASSERT_LINE);
	else
		m_t_c++;
}

static MACHINE_CONFIG_START( s4, s4_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 3580000)
	MCFG_CPU_PROGRAM_MAP(s4_main_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", s4_state, irq, attotime::from_hz(250))
	MCFG_MACHINE_RESET_OVERRIDE(s4_state, s4)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_s4)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_PIA6821_ADD("pia22", pia22_intf)
	MCFG_PIA6821_ADD("pia24", pia24_intf)
	MCFG_PIA6821_ADD("pia28", pia28_intf)
	MCFG_PIA6821_ADD("pia30", pia30_intf)
	MCFG_NVRAM_ADD_1FILL("nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( s4a, s4 )
	/* Add the soundcard */
	MCFG_CPU_ADD("audiocpu", M6808, 3580000)
	MCFG_CPU_PROGRAM_MAP(s4_audio_map)
	MCFG_MACHINE_RESET_OVERRIDE(s4_state, s4a)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_PIA6821_ADD("pias", pias_intf)
MACHINE_CONFIG_END


/*--------------------------------
/ Flash - Sys.4 (Game #486)
/-------------------------------*/
ROM_START(flash_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(287f12d6) SHA1(ede0c5b0ea2586d8bdf71ecadbd9cc8552bd6934))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(flash_t1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(287f12d6) SHA1(ede0c5b0ea2586d8bdf71ecadbd9cc8552bd6934))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716", 0x7800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*--------------------------------
/ Tri Zone - Sys.4 (Game #487)
/-------------------------------*/
ROM_START(trizn_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(757091c5) SHA1(00dac6c19b08d2528ea293619c4a39499a1a02c2))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(trizn_t1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(757091c5) SHA1(00dac6c19b08d2528ea293619c4a39499a1a02c2))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716", 0x7800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*--------------------------------
/ Time Warp - Sys.4 (Game #489)
/-------------------------------*/
ROM_START(tmwrp_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(b168df09) SHA1(d4c97714636ce51be2e5f8cc5af89e10a2f82ac7))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(tmwrp_t2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(b168df09) SHA1(d4c97714636ce51be2e5f8cc5af89e10a2f82ac7))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716", 0x7800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*--------------------------------
/ Stellar Wars - Sys.4 (Game #490)
/-------------------------------*/
ROM_START(stlwr_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(874e7ef7) SHA1(271aeac2a0e61cb195811ae2e8d908cb1ab45874))
	ROM_LOAD("yellow1.716", 0x7000, 0x0800, CRC(d251738c) SHA1(65ddbf5c36e429243331a4c5d2339df87a8a7f64))
	ROM_LOAD("yellow2.716", 0x7800, 0x0800, CRC(5049326d) SHA1(3b2f4ea054962bf4ba41d46663b7d3d9a77590ef))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*-----------------------------
/ Scorpion - Sys.6 (Game #494)
/----------------------------*/
ROM_START(scrpn_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(881109a9) SHA1(53d4275c76b47b68a74209fe660d943a51e90eca))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(scrpn_t1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(881109a9) SHA1(53d4275c76b47b68a74209fe660d943a51e90eca))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716", 0x7800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/* From here, are NOT pinball machines */

/*----------------------------
/ Pompeii (Shuffle)
/----------------------------*/
ROM_START(pomp_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(0f069ac2) SHA1(d651d49cdb50cf444e420241a1f9ed48c878feee))
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("soundx.716", 0x7800, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ Aristocrat (Shuffle) same roms as Pompeii
/----------------------------*/
ROM_START(arist_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(0f069ac2) SHA1(d651d49cdb50cf444e420241a1f9ed48c878feee))
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("soundx.716", 0x7800, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ Topaz (Shuffle)
/----------------------------*/
ROM_START(topaz_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(cb287b10) SHA1(7fb6b6a26237cf85d5e02cf35271231267f90fc1))
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.716", 0x7800, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*----------------------------
/ Taurus (Shuffle)
/----------------------------*/
ROM_START(taurs_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(3246e285) SHA1(4f76784ecb5063a49c24795ae61db043a51e2c89))
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("soundx.716", 0x7800, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ King Tut
/----------------------------*/
ROM_START(kingt_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(54d3280a) SHA1(ca74636e35d2c3e0b3133f89b1ff1233d5d72a5c))
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("soundx.716", 0x7800, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ Omni (Shuffle)
/----------------------------*/
ROM_START(omni_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("omni-1a.u21", 0x6000, 0x0800, CRC(443bd170) SHA1(cc1ebd72d77ec2014cbd84534380e5ea1f12c022))
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound.716", 0x7800, 0x0800, CRC(db085cbb) SHA1(9a57abbad183ba16b3dba16d16923c3bfc46a0c3))
ROM_END

/*----------------------------
/ Big Strike (Shuffle)
/----------------------------*/
ROM_START(bstrk_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(323dbcde) SHA1(a75cbb5de97cb9afc1d36e9b6ff593bb482fcf8b))
	ROM_LOAD("b_ic20.716", 0x7000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
ROM_END

/*----------------------------
/ Triple Strike (Shuffle)
/----------------------------*/
ROM_START(tstrk_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(b034c059) SHA1(76b3926b87b3c137fcaf33021a586827e3c030af))
	ROM_LOAD("ic20.716", 0x7000, 0x0800, CRC(f163fc88) SHA1(988b60626f3d4dc8f4a1dbd0c99282418bc53aae))
	ROM_LOAD("b_ic17.716", 0x7800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
ROM_END


GAME( 1979, flash_l1, 0,        s4a, s4, driver_device, 0, ROT0, "Williams", "Flash (L-1)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1979, flash_t1, flash_l1, s4a, s4, driver_device, 0, ROT0, "Williams", "Flash (T-1) Ted Estes", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1978, trizn_l1, 0,        s4a, s4, driver_device, 0, ROT0, "Williams", "Tri Zone (L-1)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1978, trizn_t1, trizn_l1, s4a, s4, driver_device, 0, ROT0, "Williams", "Tri Zone (T-1)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1979, tmwrp_l2, 0,        s4a, s4, driver_device, 0, ROT0, "Williams", "Time Warp (L-2)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1979, tmwrp_t2, tmwrp_l2, s4a, s4, driver_device, 0, ROT0, "Williams", "Time Warp (T-2)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1979, stlwr_l2, 0,        s4a, s4, driver_device, 0, ROT0, "Williams", "Stellar Wars (L-2)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1980, scrpn_l1, 0,        s4a, s4, driver_device, 0, ROT0, "Williams", "Scorpion (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1980, scrpn_t1, scrpn_l1, s4a, s4, driver_device, 0, ROT0, "Williams", "Scorpion (T-1)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1978, pomp_l1,  0,        s4a, s4, driver_device, 0, ROT0, "Williams", "Pompeii (Shuffle) (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1978, arist_l1, 0,        s4a, s4, driver_device, 0, ROT0, "Williams", "Aristocrat (Shuffle) (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1978, topaz_l1, 0,        s4a, s4, driver_device, 0, ROT0, "Williams", "Topaz (Shuffle) (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1979, taurs_l1, 0,        s4a, s4, driver_device, 0, ROT0, "Williams", "Taurus (Shuffle) (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1979, kingt_l1, 0,        s4a, s4, driver_device, 0, ROT0, "Williams", "King Tut (Shuffle) (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1980, omni_l1,  0,        s4a, s4, driver_device, 0, ROT0, "Williams", "Omni (Shuffle) (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1983, bstrk_l1, 0,        s4,  s4, driver_device, 0, ROT0, "Williams", "Big Strike (Shuffle) (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1983, tstrk_l1, 0,        s4,  s4, driver_device, 0, ROT0, "Williams", "Triple Strike (Shuffle) (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
