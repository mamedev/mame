/****************************************************************************************

    Pinball
    Williams System 11

    Status of games:


ToDo:
- Background music is slow in High Speed
- Can coin up but not start
- Doesn't react to the Advance button very well

*****************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "sound/hc55516.h"
#include "sound/2151intf.h"
#include "sound/dac.h"
#include "s11.lh"


class s11_state : public genpin_class
{
public:
	s11_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_audiocpu(*this, "audiocpu"),
	m_bgcpu(*this, "bgcpu"),
	m_dac(*this, "dac"),
	m_dac1(*this, "dac1"),
	m_hc55516(*this, "hc55516"),
	m_pias(*this, "pias"),
	m_pia21(*this, "pia21"),
	m_pia24(*this, "pia24"),
	m_pia28(*this, "pia28"),
	m_pia2c(*this, "pia2c"),
	m_pia30(*this, "pia30"),
	m_pia34(*this, "pia34"),
	m_pia40(*this, "pia40")
	{ }

	DECLARE_READ8_MEMBER(dac_r);
	DECLARE_WRITE8_MEMBER(dac_w);
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(lamp0_w);
	DECLARE_WRITE8_MEMBER(lamp1_w) { };
	DECLARE_WRITE8_MEMBER(sol2_w) { }; // solenoids 8-15
	DECLARE_WRITE8_MEMBER(sol3_w); // solenoids 0-7
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(pia2c_pa_w);
	DECLARE_WRITE8_MEMBER(pia2c_pb_w);
	DECLARE_WRITE8_MEMBER(pia34_pa_w);
	DECLARE_WRITE8_MEMBER(pia34_pb_w);
	DECLARE_WRITE_LINE_MEMBER(pia34_cb2_w);
	DECLARE_WRITE8_MEMBER(pia40_pa_w);
	DECLARE_READ8_MEMBER(dips_r);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_READ_LINE_MEMBER(pias_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia21_ca1_r);
	DECLARE_READ8_MEMBER(pia28_w7_r);
	DECLARE_READ_LINE_MEMBER(pia28_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia28_cb1_r);
	DECLARE_WRITE_LINE_MEMBER(pias_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pias_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_cb2_w) { }; // enable solenoids
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { }; // dummy to stop error log filling up
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { }; // comma3&4
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { }; // comma1&2
	DECLARE_WRITE_LINE_MEMBER(pia30_cb2_w) { }; // dummy to stop error log filling up
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);
	DECLARE_MACHINE_RESET(s11);
	DECLARE_DRIVER_INIT(s11);
protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_bgcpu;
	required_device<dac_device> m_dac;
	required_device<dac_device> m_dac1;
	required_device<hc55516_device> m_hc55516;
	required_device<pia6821_device> m_pias;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia2c;
	required_device<pia6821_device> m_pia30;
	required_device<pia6821_device> m_pia34;
	required_device<pia6821_device> m_pia40;
private:
	UINT8 m_t_c;
	UINT8 m_sound_data;
	UINT8 m_strobe;
	UINT8 m_kbdrow;
	UINT32 m_segment1;
	UINT32 m_segment2;
	bool m_ca1;
};

static ADDRESS_MAP_START( s11_main_map, AS_PROGRAM, 8, s11_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2100, 0x2103) AM_DEVREADWRITE("pia21", pia6821_device, read, write) // sound+solenoids
	AM_RANGE(0x2200, 0x2200) AM_WRITE(sol3_w) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_DEVREADWRITE("pia24", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("pia28", pia6821_device, read, write) // display
	AM_RANGE(0x2c00, 0x2c03) AM_DEVREADWRITE("pia2c", pia6821_device, read, write) // alphanumeric display
	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("pia30", pia6821_device, read, write) // inputs
	AM_RANGE(0x3400, 0x3403) AM_DEVREADWRITE("pia34", pia6821_device, read, write) // widget
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( s11_audio_map, AS_PROGRAM, 8, s11_state )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x0800) AM_RAM
	AM_RANGE(0x1000, 0x1fff) AM_WRITE(bank_w)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE("pias", pia6821_device, read, write)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank0")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( s11_bg_map, AS_PROGRAM, 8, s11_state )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x1ffe) AM_DEVREADWRITE("ym2151", ym2151_device, read, write)
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE("pia40", pia6821_device, read, write)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( s11 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, s11_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, s11_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_9) PORT_TOGGLE
	PORT_CONFNAME( 0x10, 0x00, "Language" )
	PORT_CONFSETTING( 0x00, "German" )
	PORT_CONFSETTING( 0x10, "English" )
INPUT_PORTS_END

MACHINE_RESET_MEMBER( s11_state, s11 )
{
	m_t_c = 0;
	membank("bank0")->set_entry(0);
	membank("bank1")->set_entry(0);
}

INPUT_CHANGED_MEMBER( s11_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( s11_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER( s11_state::sol3_w )
{

}

WRITE8_MEMBER( s11_state::sound_w )
{
	m_sound_data = data;
}

WRITE_LINE_MEMBER( s11_state::pia21_ca2_w )
{
// sound ns
	m_ca1 = state;
	m_pias->ca1_w(m_ca1);
	m_pia40->cb2_w(m_ca1);
}

static const pia6821_interface pia21_intf =
{
	DEVCB_DRIVER_MEMBER(s11_state, dac_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_LINE_GND,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11_state, sound_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, sol2_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia21_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia21_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

WRITE8_MEMBER( s11_state::lamp0_w )
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
	DEVCB_DRIVER_MEMBER(s11_state, lamp0_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, lamp1_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia24_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

READ_LINE_MEMBER( s11_state::pia28_ca1_r )
{
	return BIT(ioport("DIAGS")->read(), 2) ? 1 : 0; // advance button
}

READ_LINE_MEMBER( s11_state::pia28_cb1_r )
{
	return BIT(ioport("DIAGS")->read(), 3) ? 1 : 0; // up/down switch
}

WRITE8_MEMBER( s11_state::dig0_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	data &= 0x7f;
	m_strobe = data & 15;
	output_set_digit_value(60, patterns[data>>4]); // diag digit
	m_segment1 = 0;
	m_segment2 = 0;
}

WRITE8_MEMBER( s11_state::dig1_w )
{
	m_segment2 |= data;
	m_segment2 |= 0x20000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		output_set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment2 |= 0x40000;
	}
}

READ8_MEMBER( s11_state::pia28_w7_r)
{
	if(BIT(ioport("DIAGS")->read(), 4))  // W7 Jumper
		return 0x00;
	return 0x80;
}

static const pia6821_interface pia28_intf =
{
	DEVCB_DRIVER_MEMBER(s11_state, pia28_w7_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia28_ca1_r),		/* line CA1 in */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia28_cb1_r),		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11_state, dig0_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, dig1_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia28_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia28_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

WRITE8_MEMBER( s11_state::pia2c_pa_w )
{
	m_segment1 |= (data<<8);
	m_segment1 |= 0x10000;
	if ((m_segment1 & 0x70000) == 0x30000)
	{
		output_set_digit_value(m_strobe, BITSWAP16(m_segment1, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment1 |= 0x40000;
	}
}

WRITE8_MEMBER( s11_state::pia2c_pb_w )
{
	m_segment1 |= data;
	m_segment1 |= 0x20000;
	if ((m_segment1 & 0x70000) == 0x30000)
	{
		output_set_digit_value(m_strobe, BITSWAP16(m_segment1, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment1 |= 0x40000;
	}
}

static const pia6821_interface pia2c_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11_state, pia2c_pa_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, pia2c_pb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),	/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

READ8_MEMBER( s11_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"X%X",m_kbdrow);
	return ~ioport(kbdrow)->read();
}

WRITE8_MEMBER( s11_state::switch_w )
{
	m_kbdrow = data;
}

static const pia6821_interface pia30_intf =
{
	DEVCB_DRIVER_MEMBER(s11_state, switch_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_LINE_GND,		/* line CA1 in */
	DEVCB_LINE_GND,		/* line CB1 in */
	DEVCB_LINE_VCC,		/* line CA2 in */
	DEVCB_LINE_VCC,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, switch_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia30_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),	/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

WRITE8_MEMBER( s11_state::pia34_pa_w )
{
	m_segment2 |= (data<<8);
	m_segment2 |= 0x10000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		output_set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment2 |= 0x40000;
	}
}

WRITE8_MEMBER( s11_state::pia34_pb_w )
{
	m_pia40->portb_w(data);
	m_pia40->cb1_w(1);
	m_pia40->cb1_w(0);
}

WRITE_LINE_MEMBER( s11_state::pia34_cb2_w )
{

}

static const pia6821_interface pia34_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11_state, pia34_pa_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, pia34_pb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia34_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),	/* IRQA */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB */
};

WRITE8_MEMBER( s11_state::bank_w )
{
	membank("bank0")->set_entry(BIT(data, 1));
	membank("bank1")->set_entry(BIT(data, 0));
}

READ_LINE_MEMBER( s11_state::pias_ca1_r )
{
	return m_ca1;
}

WRITE_LINE_MEMBER( s11_state::pias_ca2_w )
{
// speech clock
	hc55516_clock_w(m_hc55516, state);
}

WRITE_LINE_MEMBER( s11_state::pias_cb2_w )
{
// speech data
	hc55516_digit_w(m_hc55516, state);
}

READ8_MEMBER( s11_state::dac_r )
{
	return m_sound_data;
}

WRITE8_MEMBER( s11_state::dac_w )
{
	m_dac->write_unsigned8(data);
}

static const pia6821_interface pias_intf =
{
	DEVCB_DRIVER_MEMBER(s11_state, dac_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pias_ca1_r),		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11_state, sound_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, dac_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pias_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pias_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("audiocpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("audiocpu", M6800_IRQ_LINE)		/* IRQB */
};

WRITE8_MEMBER( s11_state::pia40_pa_w )
{
	m_dac1->write_unsigned8(data);
}

WRITE_LINE_MEMBER( s11_state::ym2151_irq_w)
{
	m_pia40->ca1_w(!state);
}

static const pia6821_interface pia40_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,//_DRIVER_MEMBER(s11_state, dac_r),      /* port B in */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pias_ca1_r),		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_LINE_VCC,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11_state, pia40_pa_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, dac_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pias_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pias_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("bgcpu", M6809_FIRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("bgcpu", INPUT_LINE_NMI)		/* IRQB */
};

DRIVER_INIT_MEMBER( s11_state, s11 )
{
	UINT8 *ROM = memregion("audiocpu")->base();
	membank("bank0")->configure_entries(0, 2, &ROM[0x10000], 0x4000);
	membank("bank1")->configure_entries(0, 2, &ROM[0x18000], 0x4000);
	membank("bank0")->set_entry(0);
	membank("bank1")->set_entry(0);
}

TIMER_DEVICE_CALLBACK_MEMBER( s11_state::irq)
{
	if (m_t_c > 0x70)
	{
		m_maincpu->set_input_line(M6800_IRQ_LINE, ASSERT_LINE);
		m_pias->cb1_w(0);
	}
	else
		m_t_c++;
}

static MACHINE_CONFIG_START( s11, s11_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 4000000)
	MCFG_CPU_PROGRAM_MAP(s11_main_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", s11_state, irq, attotime::from_hz(500))
	MCFG_MACHINE_RESET_OVERRIDE(s11_state, s11)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_s11)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_PIA6821_ADD("pia21", pia21_intf)
	MCFG_PIA6821_ADD("pia24", pia24_intf)
	MCFG_PIA6821_ADD("pia28", pia28_intf)
	MCFG_PIA6821_ADD("pia2c", pia2c_intf)
	MCFG_PIA6821_ADD("pia30", pia30_intf)
	MCFG_PIA6821_ADD("pia34", pia34_intf)
	MCFG_NVRAM_ADD_1FILL("nvram")

	/* Add the soundcard */
	MCFG_CPU_ADD("audiocpu", M6808, 3580000)
	MCFG_CPU_PROGRAM_MAP(s11_audio_map)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SPEAKER_STANDARD_MONO("speech")
	MCFG_SOUND_ADD("hc55516", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speech", 0.50)

	MCFG_PIA6821_ADD("pias", pias_intf)

	/* Add the background music card */
	MCFG_CPU_ADD("bgcpu", M6809E, 4000000) // MC68B09E
	MCFG_CPU_PROGRAM_MAP(s11_bg_map)

	MCFG_SPEAKER_STANDARD_MONO("bg")
	MCFG_YM2151_ADD("ym2151", 3580000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(s11_state, ym2151_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)

	MCFG_PIA6821_ADD("pia40", pia40_intf)
MACHINE_CONFIG_END


/*----------------------------
/ Grand Lizard 04/86 (#523)
/-----------------------------*/
ROM_START(grand_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lzrd_u26.l4", 0x4000, 0x2000, CRC(5fe50db6) SHA1(7e2adfefce5c33ad605606574dbdfb2642aa0e85))
	ROM_RELOAD( 0x6000, 0x2000)
	ROM_LOAD("lzrd_u27.l4", 0x8000, 0x8000, CRC(6462ca55) SHA1(0ebfa998d3cefc213ada9ed815d44977120e5d6d))
	ROM_FILL(0x6035, 1, 0) // default to English

	// according to the manual these should be 32K roms just like the other games here
	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("lzrd_u21.l1", 0x1c000, 0x4000, CRC(98859d37) SHA1(08429b9e6a3b3007815373dc280b985e3441aa9f))
	ROM_RELOAD( 0x18000, 0x4000)
	ROM_LOAD("lzrd_u22.l1", 0x14000, 0x4000, CRC(4e782eba) SHA1(b44ab499128300175bdb57f07ffe2992c82e47e4))
	ROM_RELOAD( 0x10000, 0x4000)

	ROM_REGION(0x10000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("lzrd_u4.l1", 0x8000, 0x8000, CRC(4baafc11) SHA1(3507f5f37e02688fa56cf5bb303eaccdcedede06))
ROM_END

/*-------------------------
/ High Speed 01/86 (#541)
/--------------------------*/
ROM_START(hs_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hs_u26.l4", 0x4000, 0x2000, CRC(38b73830) SHA1(df89670f3df2b657dcf1f8ee08e506e54e016028))
	ROM_RELOAD( 0x6000, 0x2000)
	ROM_LOAD("hs_u27.l4", 0x8000, 0x8000, CRC(24c6f7f0) SHA1(bb0058650ec0908f88b6a202df79e971b46f8594))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("hs_u21.l2", 0x18000, 0x8000, CRC(c0580037) SHA1(675ca65a6a20f8607232c532b4d127641f77d837))
	ROM_LOAD("hs_u22.l2", 0x10000, 0x8000, CRC(c03be631) SHA1(53823e0f55377a45aa181882c310dd307cf368f5))

	ROM_REGION(0x10000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("hs_u4.l1", 0x8000, 0x8000, CRC(0f96e094) SHA1(58650705a02a71ced85f5c2a243722a35282cbf7))
ROM_END

ROM_START(hs_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l3.rom", 0x4000, 0x2000, CRC(fd587959) SHA1(20fe6d7bd617b1fa886362ce520393a25be9a632))
	ROM_RELOAD( 0x6000, 0x2000)
	ROM_LOAD("hs_u27.l4", 0x8000, 0x8000, CRC(24c6f7f0) SHA1(bb0058650ec0908f88b6a202df79e971b46f8594))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("hs_u21.l2", 0x18000, 0x8000, CRC(c0580037) SHA1(675ca65a6a20f8607232c532b4d127641f77d837))
	ROM_LOAD("hs_u22.l2", 0x10000, 0x8000, CRC(c03be631) SHA1(53823e0f55377a45aa181882c310dd307cf368f5))

	ROM_REGION(0x10000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("hs_u4.l1", 0x8000, 0x8000, CRC(0f96e094) SHA1(58650705a02a71ced85f5c2a243722a35282cbf7))
ROM_END

/*-------------------------
/ Road Kings 07/86 (#542)
/--------------------------*/

ROM_START(rdkng_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("road_u26.l1", 0x4000, 0x4000, CRC(19abe96b) SHA1(d6c3b6dab328f23cc4506e4f56cd0beeb06fb3cb))
	ROM_LOAD("road_u27.l1", 0x8000, 0x8000, CRC(3dcad794) SHA1(0cf06f8e16d738f0bc0111e2e12351a26e2f02c6))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u21.l1", 0x18000, 0x8000, CRC(f34efbf4) SHA1(cb5ffe9818994f4681e3492a5cd46f410d2e5353))
	ROM_LOAD("road_u22.l1", 0x10000, 0x8000, CRC(a9803804) SHA1(a400d4621c3f7a6e47546b2f33dc4920183a5a74))

	ROM_REGION(0x10000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u4.l1", 0x8000, 0x8000, CRC(4395b48f) SHA1(2325ce6ba7f6f92f884c302e6f053c31229dc774))
ROM_END

ROM_START(rdkng_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("road_u26.l1", 0x4000, 0x4000, CRC(19abe96b) SHA1(d6c3b6dab328f23cc4506e4f56cd0beeb06fb3cb))
	ROM_LOAD("road_u27.l2", 0x8000, 0x8000, CRC(aff45e2b) SHA1(c52aca20639f519a940951ef04c2bd179a596b30))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u21.l1", 0x18000, 0x8000, CRC(f34efbf4) SHA1(cb5ffe9818994f4681e3492a5cd46f410d2e5353))
	ROM_LOAD("road_u22.l1", 0x10000, 0x8000, CRC(a9803804) SHA1(a400d4621c3f7a6e47546b2f33dc4920183a5a74))

	ROM_REGION(0x10000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u4.l1", 0x8000, 0x8000, CRC(4395b48f) SHA1(2325ce6ba7f6f92f884c302e6f053c31229dc774))
ROM_END

ROM_START(rdkng_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("road_u26.l3", 0x4000, 0x4000, CRC(9bade45d) SHA1(c1791724761cdd1d863e12b02655c5fed8936162))
	ROM_LOAD("road_u27.l3", 0x8000, 0x8000, CRC(97b599dc) SHA1(18524d22a75b0569bb480d847cef8047ee51f91e))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u21.l1", 0x18000, 0x8000, CRC(f34efbf4) SHA1(cb5ffe9818994f4681e3492a5cd46f410d2e5353))
	ROM_LOAD("road_u22.l1", 0x10000, 0x8000, CRC(a9803804) SHA1(a400d4621c3f7a6e47546b2f33dc4920183a5a74))

	ROM_REGION(0x10000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u4.l1", 0x8000, 0x8000, CRC(4395b48f) SHA1(2325ce6ba7f6f92f884c302e6f053c31229dc774))
ROM_END

ROM_START(rdkng_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("road_u26.l4", 0x4000, 0x4000, CRC(4ea27d67) SHA1(cf46e8c5e417999150403d6d40adf8c36b1c0347))
	ROM_LOAD("road_u27.l4", 0x8000, 0x8000, CRC(5b88e755) SHA1(6438505bb335f670e0892126764819a48eec9b88))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u21.l1", 0x18000, 0x8000, CRC(f34efbf4) SHA1(cb5ffe9818994f4681e3492a5cd46f410d2e5353))
	ROM_LOAD("road_u22.l1", 0x10000, 0x8000, CRC(a9803804) SHA1(a400d4621c3f7a6e47546b2f33dc4920183a5a74))

	ROM_REGION(0x10000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u4.l1", 0x8000, 0x8000, CRC(4395b48f) SHA1(2325ce6ba7f6f92f884c302e6f053c31229dc774))
ROM_END

/************************ From here, not pinball machines **************************************/

/*--------------------
/ Tic-Tac-Strike (#919)
/--------------------*/
ROM_START(tts_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u27_l2.128", 0x8000, 0x4000, CRC(edbcab92) SHA1(0f6b2dc01874984f9a17ee873f2fa0b6c9bba5be))
	ROM_RELOAD( 0xc000, 0x4000)

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("tts_u21.256", 0x18000, 0x8000, NO_DUMP)
	ROM_LOAD("tts_u22.256", 0x10000, 0x8000, NO_DUMP)

	ROM_REGION(0x10000, "bgcpu", ROMREGION_ERASEFF)
ROM_END

ROM_START(tts_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tts_u27.128", 0x8000, 0x4000, CRC(f540c53c) SHA1(1c7a318278ad1afdcbe6aaf81f9b774882b069d6))
	ROM_RELOAD( 0xc000, 0x4000)

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("tts_u21.256", 0x18000, 0x8000, NO_DUMP)
	ROM_LOAD("tts_u22.256", 0x10000, 0x8000, NO_DUMP)

	ROM_REGION(0x10000, "bgcpu", ROMREGION_ERASEFF)
ROM_END

/*-------------------------------
/ Gold Mine (Shuffle) (#920) s11b
/--------------------------------*/
ROM_START(gmine_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u27.128", 0x8000, 0x4000, CRC(99c6e049) SHA1(356faec0598a54892050a28857e9eb5cdbf35833))
	ROM_RELOAD( 0xc000, 0x4000)

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("u21.256", 0x18000, 0x8000, CRC(3b801570) SHA1(50b50ff826dcb031a30940fa3099bd3a8d773831))
	ROM_LOAD("u22.256", 0x10000, 0x8000, CRC(08352101) SHA1(a7437847a71cf037a80686292f9616b1e08922df))

	ROM_REGION(0x10000, "bgcpu", ROMREGION_ERASEFF)
ROM_END

/*-------------------------
/ Top Dawg (Shuffle) (#921)
/--------------------------*/
ROM_START(tdawg_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tdu27r1.128", 0x8000, 0x4000, CRC(0b4bb586) SHA1(a927ebf7167609cc84b38c22aa35d0c4d259dd8b))
	ROM_RELOAD( 0xc000, 0x4000)

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("tdsu21r1.256", 0x18000, 0x8000, CRC(6a323227) SHA1(7c7263754e5672c654a2ee9582f0b278e637a909))
	ROM_LOAD("tdsu22r1.256", 0x10000, 0x8000, CRC(58407eb4) SHA1(6bd9b304c88d9470eae5afb6621187f4a8313573))

	ROM_REGION(0x10000, "bgcpu", ROMREGION_ERASEFF)
ROM_END

/*----------------------------
/ Shuffle Inn (Shuffle) (#922)
/-----------------------------*/
ROM_START(shfin_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u27rom-1.rv1", 0x8000, 0x4000, CRC(40cfb74a) SHA1(8cee4212ea8bb6b360060391df3208e1e129d7e5))
	ROM_RELOAD( 0xc000, 0x4000)

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("u21snd-2.rv1", 0x18000, 0x8000, CRC(80ddce05) SHA1(9498260e5ccd2fe0eb03ff321dd34eb945b0213a))
	ROM_LOAD("u22snd-2.rv1", 0x10000, 0x8000, CRC(6894abaf) SHA1(2d661765fbfce33a73a20778c41233c0bd9933e9))

	ROM_REGION(0x10000, "bgcpu", ROMREGION_ERASEFF)
ROM_END

GAME( 1986, grand_l4, 0,        s11, s11, s11_state, s11, ROT0, "Williams", "Grand Lizard (L-4)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1986, hs_l4,    0,        s11, s11, s11_state, s11, ROT0, "Williams", "High Speed (L-4)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1986, hs_l3,    hs_l4,    s11, s11, s11_state, s11, ROT0, "Williams", "High Speed (L-3)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1986, rdkng_l4, 0,        s11, s11, s11_state, s11, ROT0, "Williams", "Road Kings (L-4)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1986, rdkng_l1, rdkng_l4, s11, s11, s11_state, s11, ROT0, "Williams", "Road Kings (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1986, rdkng_l2, rdkng_l4, s11, s11, s11_state, s11, ROT0, "Williams", "Road Kings (L-2)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1986, rdkng_l3, rdkng_l4, s11, s11, s11_state, s11, ROT0, "Williams", "Road Kings (L-3)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1986, tts_l2,   0,        s11, s11, s11_state, s11, ROT0, "Williams", "Tic-Tac-Strike (Shuffle) (L-2)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1986, tts_l1,   tts_l2,   s11, s11, s11_state, s11, ROT0, "Williams", "Tic-Tac-Strike (Shuffle) (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1987, gmine_l2, 0,        s11, s11, s11_state, s11, ROT0, "Williams", "Gold Mine (Shuffle) (L-2)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1987, tdawg_l1, 0,        s11, s11, s11_state, s11, ROT0, "Williams", "Top Dawg (Shuffle) (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING)
GAME( 1987, shfin_l1, 0,        s11, s11, s11_state, s11, ROT0, "Williams", "Shuffle Inn (Shuffle) (L-1)", GAME_MECHANICAL | GAME_NOT_WORKING)
