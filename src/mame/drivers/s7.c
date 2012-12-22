/****************************************************************************************

    Pinball
    Williams System 7

    Status of games:
    - Inputs don't work so unable to play.
    - Display works.
    - Sound test works.

ToDo:

*****************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/hc55516.h"
#include "sound/dac.h"
#include "s7.lh"


class s7_state : public genpin_class
{
public:
	s7_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_audiocpu(*this, "audiocpu"),
	m_dac(*this, "dac"),
	m_hc55516(*this, "hc55516"),
	m_pias(*this, "pias"),
	m_pia21(*this, "pia21"),
	m_pia22(*this, "pia22"),
	m_pia24(*this, "pia24"),
	m_pia28(*this, "pia28"),
	m_pia30(*this, "pia30")
	{ }

	DECLARE_READ8_MEMBER(dac_r);
	DECLARE_WRITE8_MEMBER(dac_w);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(lamp0_w);
	DECLARE_WRITE8_MEMBER(lamp1_w) { };
	DECLARE_WRITE8_MEMBER(sol0_w) { };
	DECLARE_WRITE8_MEMBER(sol1_w) { };
	DECLARE_WRITE8_MEMBER(sol2_w);
	DECLARE_WRITE8_MEMBER(sol3_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(dips_r);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_READ_LINE_MEMBER(pias_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia21_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia28_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia28_cb1_r);
	DECLARE_WRITE_LINE_MEMBER(pias_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pias_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w) { };
	DECLARE_WRITE_LINE_MEMBER(pia21_cb2_w) { }; // enable solenoids
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
	DECLARE_MACHINE_RESET(s7);
protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<dac_device> m_dac;
	required_device<hc55516_device> m_hc55516;
	required_device<pia6821_device> m_pias;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia22;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
private:
	UINT8 m_t_c;
	UINT8 m_sound_data;
	UINT8 m_strobe;
	UINT8 m_kbdrow;
	bool m_cb1;
	bool m_data_ok;
};

static ADDRESS_MAP_START( s7_main_map, AS_PROGRAM, 8, s7_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x0200, 0x13ff) AM_RAM
	AM_RANGE(0x2100, 0x2103) AM_DEVREADWRITE("pia21", pia6821_device, read, write) // sound+solenoids
	AM_RANGE(0x2200, 0x2203) AM_DEVREADWRITE("pia22", pia6821_device, read, write) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_DEVREADWRITE("pia24", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("pia28", pia6821_device, read, write) // display
	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("pia30", pia6821_device, read, write) // inputs
	AM_RANGE(0x4000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( s7_audio_map, AS_PROGRAM, 8, s7_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0400, 0x0403) AM_MIRROR(0x8000) AM_DEVREADWRITE("pias", pia6821_device, read, write)
	AM_RANGE(0xA000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( s7 )
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

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, s7_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, s7_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_9)

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

MACHINE_RESET_MEMBER( s7_state, s7 )
{
	m_t_c = 0;
}

INPUT_CHANGED_MEMBER( s7_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( s7_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER( s7_state::sol2_w )
{
	m_pia21->ca1_w(BIT(data, 5));
}

WRITE8_MEMBER( s7_state::sol3_w )
{
//  if (BIT(data, 1))
//      m_samples->start(0, 6); // knocker
}

WRITE8_MEMBER( s7_state::sound_w )
{
	m_sound_data = data;
	m_cb1 = ((m_sound_data & 0x8f) != 0x8f);
	m_pias->cb1_w(m_cb1);
}

static const pia6821_interface pia21_intf =
{
	DEVCB_DRIVER_MEMBER(s7_state, dac_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_LINE_GND,		/* line CB1 in */
	DEVCB_LINE_VCC,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s7_state, sound_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s7_state, sol2_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pia21_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pia21_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

static const pia6821_interface pia22_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_LINE_GND,		/* line CA1 in */
	DEVCB_LINE_GND,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s7_state, sol0_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s7_state, sol1_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pia22_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pia22_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

WRITE8_MEMBER( s7_state::lamp0_w )
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

static const pia6821_interface pia24_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_LINE_GND,		/* line CA1 in */
	DEVCB_LINE_GND,		/* line CB1 in */
	DEVCB_LINE_VCC,		/* line CA2 in */
	DEVCB_LINE_VCC,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s7_state, lamp0_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s7_state, lamp1_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pia24_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pia24_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

READ_LINE_MEMBER( s7_state::pia28_ca1_r )
{
	return BIT(ioport("DIAGS")->read(), 2); // advance button
}

READ_LINE_MEMBER( s7_state::pia28_cb1_r )
{
	return BIT(ioport("DIAGS")->read(), 3); // up/down switch
}

WRITE8_MEMBER( s7_state::dig0_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	m_strobe = data & 15;
	m_data_ok = true;
	output_set_digit_value(60, patterns[data>>4]); // diag digit
}

WRITE8_MEMBER( s7_state::dig1_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // MC14558
	if (m_data_ok)
	{
		output_set_digit_value(m_strobe+16, patterns[data&15]);
		output_set_digit_value(m_strobe, patterns[data>>4]);
	}
	m_data_ok = false;
}

READ8_MEMBER( s7_state::dips_r )
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

static const pia6821_interface pia28_intf =
{
	DEVCB_DRIVER_MEMBER(s7_state, dips_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pia28_ca1_r),		/* line CA1 in */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pia28_cb1_r),		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s7_state, dig0_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s7_state, dig1_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pia28_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pia28_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

READ8_MEMBER( s7_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"X%X",m_kbdrow);
	return ~ioport(kbdrow)->read();
}

WRITE8_MEMBER( s7_state::switch_w )
{
	m_kbdrow = data;
}

static const pia6821_interface pia30_intf =
{
	DEVCB_DRIVER_MEMBER(s7_state, switch_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_LINE_GND,		/* line CA1 in */
	DEVCB_LINE_GND,		/* line CB1 in */
	DEVCB_LINE_VCC,		/* line CA2 in */
	DEVCB_LINE_VCC,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_DRIVER_MEMBER(s7_state, switch_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pia30_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pia30_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),	/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

WRITE_LINE_MEMBER( s7_state::pias_cb2_w )
{
// speech clock
	hc55516_clock_w(m_hc55516, state);
}

WRITE_LINE_MEMBER( s7_state::pias_ca2_w )
{
// speech data
	hc55516_digit_w(m_hc55516, state);
}

READ8_MEMBER( s7_state::dac_r )
{
	return m_sound_data;
}

WRITE8_MEMBER( s7_state::dac_w )
{
	m_dac->write_unsigned8(data);
}

static const pia6821_interface pias_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_DRIVER_MEMBER(s7_state, dac_r),		/* port B in */
	DEVCB_LINE_VCC,		/* line CA1 in */
	DEVCB_LINE_GND,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s7_state, dac_w),		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pias_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s7_state, pias_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("audiocpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("audiocpu", M6800_IRQ_LINE)		/* IRQB */
};

TIMER_DEVICE_CALLBACK_MEMBER( s7_state::irq)
{
	if (m_t_c > 0x70)
		m_maincpu->set_input_line(M6800_IRQ_LINE, ASSERT_LINE);
	else
		m_t_c++;
}

static MACHINE_CONFIG_START( s7, s7_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6808, 4000000)
	MCFG_CPU_PROGRAM_MAP(s7_main_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", s7_state, irq, attotime::from_hz(250))
	MCFG_MACHINE_RESET_OVERRIDE(s7_state, s7)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_s7)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_PIA6821_ADD("pia21", pia21_intf)
	MCFG_PIA6821_ADD("pia22", pia22_intf)
	MCFG_PIA6821_ADD("pia24", pia24_intf)
	MCFG_PIA6821_ADD("pia28", pia28_intf)
	MCFG_PIA6821_ADD("pia30", pia30_intf)
	MCFG_NVRAM_ADD_1FILL("nvram")
	/* Add the soundcard */
	MCFG_CPU_ADD("audiocpu", M6808, 4000000)
	MCFG_CPU_PROGRAM_MAP(s7_audio_map)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SPEAKER_STANDARD_MONO("speech")
	MCFG_SOUND_ADD("hc55516", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speech", 0.50)
	MCFG_PIA6821_ADD("pias", pias_intf)
MACHINE_CONFIG_END



/*----------------------------
/ Black Knight - Sys.7 (Game #500)
/----------------------------*/
ROM_START(bk_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(104b78da) SHA1(c3af2563b3b380fe0e154b737799f6beacf8998c) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(fcbe3d44) SHA1(92ec4d41beea205ba29530624b68dd1139053535) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("speech7.532", 0xb000, 0x1000, CRC(c7e229bf) SHA1(3b2ab41031f507963af828639f1690dc350737af))
	ROM_LOAD("speech5.532", 0xc000, 0x1000, CRC(411bc92f) SHA1(6c8d26fd13ed5eeba5cc40886d39c65a64beb377))
	ROM_LOAD("speech6.532", 0xd000, 0x1000, CRC(fc985005) SHA1(9df4ad12cf98a5a92b8f933e6b6788a292c8776b))
	ROM_LOAD("speech4.532", 0xe000, 0x1000, CRC(f36f12e5) SHA1(24fb192ad029cd35c08f4899b76d527776a4895b))
	ROM_LOAD("sound12.716", 0xf800, 0x0800, CRC(6d454c0e) SHA1(21640b9ed3bdbae8bf27629891f355304e467c64))
ROM_END

ROM_START(bk_f4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(104b78da) SHA1(c3af2563b3b380fe0e154b737799f6beacf8998c) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(fcbe3d44) SHA1(92ec4d41beea205ba29530624b68dd1139053535) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("speech7f.532", 0xb000, 0x1000, CRC(01debff6) SHA1(dc02199b63ae3309fdac819985f7a40010831634))
	ROM_LOAD("speech5f.532", 0xc000, 0x1000, CRC(2d310dce) SHA1(ad2ad3844659787ee9be4db50b17b8af6f5d0d42))
	ROM_LOAD("speech6f.532", 0xd000, 0x1000, CRC(96bb719b) SHA1(d602129ce1af1902e46ca26645a9a51324a788d0))
	ROM_LOAD("speech4f.532", 0xe000, 0x1000, CRC(8ee8fc3c) SHA1(ba7c00f16bdbd7413cec025c28f8b7e7bbcb12bb))
	ROM_LOAD("sound12.716", 0xf800, 0x0800, CRC(6d454c0e) SHA1(21640b9ed3bdbae8bf27629891f355304e467c64))
ROM_END

ROM_START(bk_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bkl3_26.bin", 0x5800, 0x0800, CRC(6acc34a0) SHA1(3adad61d27e6416630f96554687bb66d3016166a) )
	ROM_LOAD("bkl3_14.bin", 0x6000, 0x0800, CRC(74c37e4f) SHA1(8946b110901d0660676fba0c204aa2bc78223508) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("speech7.532", 0xb000, 0x1000, CRC(c7e229bf) SHA1(3b2ab41031f507963af828639f1690dc350737af))
	ROM_LOAD("speech5.532", 0xc000, 0x1000, CRC(411bc92f) SHA1(6c8d26fd13ed5eeba5cc40886d39c65a64beb377))
	ROM_LOAD("speech6.532", 0xd000, 0x1000, CRC(fc985005) SHA1(9df4ad12cf98a5a92b8f933e6b6788a292c8776b))
	ROM_LOAD("speech4.532", 0xe000, 0x1000, CRC(f36f12e5) SHA1(24fb192ad029cd35c08f4899b76d527776a4895b))
	ROM_LOAD("sound12.716", 0xf800, 0x0800, CRC(6d454c0e) SHA1(21640b9ed3bdbae8bf27629891f355304e467c64))
ROM_END

/*-----------------------------------
/ Cosmic Gunfight - Sys.7 (Game #502)
/-----------------------------------*/
ROM_START(csmic_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(a259eba0) SHA1(0c5acae3beacb8abb0160dd8a580d3514ca557fe) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(ac66c0dc) SHA1(9e2ac0e956008c2d56ffd564c983e127bc4af7ae) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound12.716", 0xf800, 0x0800, CRC(af41737b) SHA1(8be4e7cebe5a821e859550c0350f0fc9cc00b2a9))
ROM_END

/*--------------------------------
/ Jungle Lord - Sys.7 (Game #503)
/--------------------------------*/
ROM_START(jngld_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(4714b1f1) SHA1(01f8593a926df69fb8ae79260f11c5f6b868cd51) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(6e5a6374) SHA1(738ecef807de9fee6fd1e832b35511c11173914c) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("speech7.532", 0xb000, 0x1000, CRC(83ffb695) SHA1(f9151bdfdefd5c178ca7eb5122f62b700d64f41a))
	ROM_LOAD("speech5.532", 0xc000, 0x1000, CRC(754bd474) SHA1(c05f48bb07085683de469603880eafd28dffd9f5))
	ROM_LOAD("speech6.532", 0xd000, 0x1000, CRC(f2ac6a52) SHA1(5b3e743eac382d571fd049f92ea9955342b9ffa0))
	ROM_LOAD("sound3.716", 0xf800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

/*--------------------------------
/ Pharaoh - Sys.7 (Game #504)
/--------------------------------*/
ROM_START(pharo_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(2afbcd1f) SHA1(98bb3a74548b7d9c5d7b8432369658ed32e8be07) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(cef00088) SHA1(e0c6b776eddc060c42a483de6cc96a1c9f2afcf7) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("speech7.532", 0xb000, 0x1000, CRC(e087f8a1) SHA1(49c2ad60d82d02f0529329f7cb4b57339d6546c6))
	ROM_LOAD("speech5.532", 0xc000, 0x1000, CRC(d72863dc) SHA1(e24ad970ed202165230fab999be42bea0f861fdd))
	ROM_LOAD("speech6.532", 0xd000, 0x1000, CRC(d29830bd) SHA1(88f6c508f2a7000bbf6c9c26e1029cf9a241d5ca))
	ROM_LOAD("speech4.532", 0xe000, 0x1000, CRC(9ecc23fd) SHA1(bf5947d186141504fd182065533d4efbfd27441d))
	ROM_LOAD("sound12.716", 0xf800, 0x0800, CRC(b0e3a04b) SHA1(eac54376fe77acf46e485ab561a01220910c1fd6))
ROM_END

/*-----------------------------------
/ Solar Fire - Sys.7 (Game #507)
/-----------------------------------*/
ROM_START(solar_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(b667ee32) SHA1(bb4b5270d9cd36207b68e8c6883538d08aae1778) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(cec19a55) SHA1(a1c0f7cc36e5fc7be4e8bcc80896f77eb4c23b1a) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound12.716", 0xf800, 0x0800, CRC(05a2230c) SHA1(c57cd7628310aa8f68ca24217aad1ead066a1a82))
ROM_END

/*-----------------------------------
/ Thunderball - Sys.7 (Game #508) - Prototype
/-----------------------------------*/
ROM_START(thund_p1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic20.532", 0x5000, 0x1000, CRC(aa3f07dc) SHA1(f31662972046f9a874380a8dcd1bc9259de5f6ba))
	ROM_LOAD("ic14.532", 0x6000, 0x1000, CRC(1cd34f1f) SHA1(3f5b5a319570c26a3d34d640fef2ac6c04b83b70))
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("speech7.532", 0xb000, 0x1000, CRC(33e1b041) SHA1(f50c0311bde69fa6e8071e297a81cc3ef3dcf44f))
	ROM_LOAD("speech5.532", 0xc000, 0x1000, CRC(11780c80) SHA1(bcc5efcd69b4f776feef32484a872863847d64cd))
	ROM_LOAD("speech6.532", 0xd000, 0x1000, CRC(ab688698) SHA1(e0cbac44a6fe30a49da478c32500a0b43903cc2b))
	ROM_LOAD("speech4.532", 0xe000, 0x1000, CRC(2a4d6f4b) SHA1(e6f8a1a6e6abc81f980a4938d98abb250e8e1e3b))
	ROM_LOAD("sound12.532", 0xf000, 0x1000, CRC(cc70af52) SHA1(d9c2840acdcd69aab39fc647dd4819eccc06af33))
ROM_END

/*-------------------------------
/ Hyperball - Sys.7 - (Game #509)
/-------------------------------*/
ROM_START(hypbl_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic20.532", 0x5000, 0x1000, CRC(d13962e8) SHA1(e23310be100060c9803682680066b965aa5efb16))
	ROM_LOAD("ic14.532", 0x6000, 0x1000, CRC(8090fe71) SHA1(0f1f40c0ee8da5b2fd51efeb8be7c20d6465239e))
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(6f4c0c4c) SHA1(1036067e2c85da867983e6e51ee2a7b5135000df))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound12.532", 0xf000, 0x1000, CRC(06051e5e) SHA1(f0ab4be812ceaf771829dd549f2a612156102a93))
ROM_END

/*----------------------------
/ Barracora- Sys.7 (Game #510)
/----------------------------*/
ROM_START(barra_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(2a0e0171) SHA1(f1f2d4c1baed698d3b7cf2e88a2c28056e859920) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(522e944e) SHA1(0fa17b7912f8129e40de5fed8c3ccccc0a2a9366) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound4.716", 0xf800, 0x0800, CRC(67ea12e7) SHA1(f81e97183442736d5766a7e5e074bc6539e8ced0))
ROM_END

/*----------------------------
/ Varkon- Sys.7 (Game #512)
/----------------------------*/
ROM_START(vrkon_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(df20330c) SHA1(22157c6480ad38b9c53c390f5e7bfa63a8abd0e8) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(3baba324) SHA1(522654e0d81458d8b31150dcb0cb53c29b334358) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound12.716", 0xf800, 0x0800, CRC(d13db2bb) SHA1(862546bbdd1476906948f7324b7434c29df79baa))
ROM_END

/*-----------------------------
/ Time Fantasy - Sys.7 (Game #515)
/-----------------------------*/
ROM_START(tmfnt_l5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(0f86947c) SHA1(e775f44b4ca5dae5ec2626fa84fae83c4f0c5c33) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(56b8e5ad) SHA1(84d6ab59032282cdccb3bdce0365c1fc766d0e5b) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound3.716", 0xf800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

/*----------------------------
/ Warlok- Sys.7 (Game #516)
/----------------------------*/
ROM_START(wrlok_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(44f8b507) SHA1(cdd8455c1e34584e8f1b75d430b8b37d4dd7dff0) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(291be241) SHA1(77fffa878f760583ef152a7939867621a61d58dc) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound12.716", 0xf800, 0x0800, CRC(5d8e46d6) SHA1(68f8760ad85b8ada81f6ed00eadb9daf37191c53))
ROM_END

/*----------------------------
/ Defender - Sys.7 (Game #517)
/----------------------------*/
// Multiplex solenoid requires custom solenoid handler.
ROM_START(dfndr_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic20.532", 0x5000, 0x1000, CRC(e99e64a2) SHA1(a6cde9cb771063778cae706c740b73ce9bce9aa5))
	ROM_LOAD("ic14.532", 0x6000, 0x1000, CRC(959ec419) SHA1(f400d3a1feba0e149d24f4e1a8d240fe900b3f0b))
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound12.716", 0xf800, 0x0800, CRC(cabaec58) SHA1(9605a1c299ed109a4ebcfa7ed6985ecc815c9e0c))
ROM_END

/*---------------------------
/ Joust - Sys.7 (Game #519)
/--------------------------*/
ROM_START(jst_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(63eea5d8) SHA1(55c26ee94809f087bd886575a5e47efc93160190) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(c4cae4bf) SHA1(ff6e48364561402b16e40a41fa1b89e7723dd38a) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound12.532", 0xf000, 0x1000, CRC(3bbc90bf) SHA1(82154e719ceca5c72d1ab034bc4ff5e3ebb36832))
ROM_END

/*---------------------------
/ Laser Cue - Sys.7 (Game #520)
/--------------------------*/
ROM_START(lsrcu_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(db4a09e7) SHA1(5ea454c852303e12cc606c2c1e403b72e0a99f25) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(39fc350d) SHA1(46e95f4016907c21c69472e6ef4a68a9adc3be77) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound12.716", 0xf800, 0x0800, CRC(1888c635) SHA1(5dcdaee437a69c6027c24310f0cd2cae4e89fa05))
ROM_END

/*--------------------------------
/ Firepower II- Sys.7 (Game #521)
/-------------------------------*/
ROM_START(fpwr2_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic26.716", 0x5800, 0x0800, CRC(1068939d) SHA1(f15c3a149bafee6d74e359399de88fd122b93441) )
	ROM_LOAD("ic14.716", 0x6000, 0x0800, CRC(a29688dd) SHA1(83815154bbaf51dd789112664d772a876efee3da) )
	ROM_LOAD("ic20.716", 0x6800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound3.716", 0xf800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

/*-----------------------------
/ Star Light - Sys.7 (Game #530)
/-----------------------------*/
ROM_START(strlt_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic20.532", 0x5000, 0x1000, CRC(66876b56) SHA1(6fab43fbb67c7b602ca595c20a41fc1553afdb65))
	ROM_LOAD("ic14.532", 0x6000, 0x1000, CRC(292f1c4a) SHA1(0b5d50331364655672be16236d38d72b28f6dec2))
	ROM_LOAD("ic17.532", 0x7000, 0x1000, CRC(a43d8518) SHA1(fb2289bb7380838d0d817e78c39e5bcb2709373f))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound3.716", 0xf800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END



GAME( 1982, vrkon_l1, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Varkon (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1981, barra_l1, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Barracora (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1980, bk_l4,    0,     s7, s7, driver_device, 0, ROT0, "Williams", "Black Knight (L-4)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1980, bk_f4,    bk_l4, s7, s7, driver_device, 0, ROT0, "Williams", "Black Knight (L-4, French speech)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1980, bk_l3,    bk_l4, s7, s7, driver_device, 0, ROT0, "Williams", "Black Knight (L-3)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1980, csmic_l1, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Cosmic Gunfight (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1982, dfndr_l4, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Defender (L-4)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1983, fpwr2_l2, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Firepower II (L-2)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1981, hypbl_l4, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "HyperBall (L-4)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1983, jst_l2,   0,     s7, s7, driver_device, 0, ROT0, "Williams", "Joust (L-2)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1981, jngld_l2, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Jungle Lord (L-2)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1983, lsrcu_l2, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Laser Cue (L-2)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1981, pharo_l2, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Pharaoh (L-2)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1981, solar_l2, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Solar Fire (L-2)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1984, strlt_l1, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Star Light (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1982, tmfnt_l5, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Time Fantasy (L-5)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1982, wrlok_l3, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Warlok (L-3)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1982, thund_p1, 0,     s7, s7, driver_device, 0, ROT0, "Williams", "Thunderball (P-1)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
