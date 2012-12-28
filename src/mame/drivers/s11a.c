/****************************************************************************************

    Pinball
    Williams System 11A

    Status of games:


ToDo:
- Doesn't react to the Advance button very well
- Some LEDs flicker

Note: To start a game, certain switches need to be activated.  You must first press and
      hold one of the trough switches (usually the left) and the ball shooter switch for
      about 1 second.  Then you are able to start a game.
      Example: For Pinbot, you must hold L and V for a second, then press start.

*****************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "sound/hc55516.h"
#include "sound/2151intf.h"
#include "sound/dac.h"
#include "s11a.lh"

// Length of time in cycles between IRQs on the main 6808 CPU
// This length is determined by the settings of the W14 and W15 jumpers
// It can be 0x300, 0x380, 0x700 or 0x780 cycles long.
// IRQ length is always 32 cycles
#define S11_IRQ_CYCLES 0x700

class s11a_state : public genpin_class
{
public:
	s11a_state(const machine_config &mconfig, device_type type, const char *tag)
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
	m_pia40(*this, "pia40"),
	m_ym(*this, "ym2151")
	{ }

	DECLARE_READ8_MEMBER(dac_r);
	DECLARE_WRITE8_MEMBER(dac_w);
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(bgbank_w);
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
	DECLARE_WRITE8_MEMBER(pia40_pb_w);
	DECLARE_WRITE_LINE_MEMBER(pia40_cb2_w);
	DECLARE_READ8_MEMBER(dips_r);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_READ_LINE_MEMBER(pias_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia21_ca1_r);
	DECLARE_READ8_MEMBER(pia28_w7_r);
	DECLARE_WRITE_LINE_MEMBER(pias_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pias_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_cb2_w) { }; // enable solenoids
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { }; // dummy to stop error log filling up
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { }; // comma3&4
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { }; // comma1&2
	DECLARE_WRITE_LINE_MEMBER(pia30_cb2_w) { }; // dummy to stop error log filling up
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	DECLARE_WRITE_LINE_MEMBER(pia_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);
	DECLARE_MACHINE_RESET(s11a);
	DECLARE_DRIVER_INIT(s11a);
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
	required_device<ym2151_device> m_ym;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
private:
	UINT8 m_sound_data;
	UINT8 m_strobe;
	UINT8 m_kbdrow;
	UINT8 m_diag;
	UINT32 m_segment1;
	UINT32 m_segment2;
	bool m_ca1;
	emu_timer* m_irq_timer;
	bool m_irq_active;

	static const device_timer_id TIMER_IRQ = 0;
};

static ADDRESS_MAP_START( s11a_main_map, AS_PROGRAM, 8, s11a_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2100, 0x2103) AM_MIRROR(0x00fc) AM_DEVREADWRITE("pia21", pia6821_device, read, write) // sound+solenoids
	AM_RANGE(0x2200, 0x2200) AM_MIRROR(0x01ff) AM_WRITE(sol3_w) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_MIRROR(0x03fc) AM_DEVREADWRITE("pia24", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_MIRROR(0x03fc) AM_DEVREADWRITE("pia28", pia6821_device, read, write) // display
	AM_RANGE(0x2c00, 0x2c03) AM_MIRROR(0x03fc) AM_DEVREADWRITE("pia2c", pia6821_device, read, write) // alphanumeric display
	AM_RANGE(0x3000, 0x3003) AM_MIRROR(0x03fc) AM_DEVREADWRITE("pia30", pia6821_device, read, write) // inputs
	AM_RANGE(0x3400, 0x3403) AM_MIRROR(0x0bfc) AM_DEVREADWRITE("pia34", pia6821_device, read, write) // widget
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( s11a_audio_map, AS_PROGRAM, 8, s11a_state )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x0800) AM_RAM
	AM_RANGE(0x1000, 0x1fff) AM_WRITE(bank_w)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE("pias", pia6821_device, read, write)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank0")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( s11a_bg_map, AS_PROGRAM, 8, s11a_state )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x1ffe) AM_DEVREADWRITE("ym2151", ym2151_device, read, write)
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE("pia40", pia6821_device, read, write)
	AM_RANGE(0x7800, 0x7fff) AM_WRITE(bgbank_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bgbank")
ADDRESS_MAP_END

static INPUT_PORTS_START( s11a )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, s11a_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, s11a_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_9) PORT_TOGGLE
	PORT_CONFNAME( 0x10, 0x10, "Language" )
	PORT_CONFSETTING( 0x00, "German" )
	PORT_CONFSETTING( 0x10, "English" )
INPUT_PORTS_END

void s11a_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_IRQ:
		if(param == 1)
		{
			m_maincpu->set_input_line(M6800_IRQ_LINE,ASSERT_LINE);
			m_irq_timer->adjust(attotime::from_ticks(32,XTAL_4MHz/2),0);
			m_pias->cb1_w(0);
			m_irq_active = true;
			m_pia28->ca1_w(BIT(ioport("DIAGS")->read(), 2));  // Advance
			m_pia28->cb1_w(BIT(ioport("DIAGS")->read(), 3));  // Up/Down
		}
		else
		{
			m_maincpu->set_input_line(M6800_IRQ_LINE,CLEAR_LINE);
			m_irq_timer->adjust(attotime::from_ticks(S11_IRQ_CYCLES,XTAL_4MHz/2),1);
			m_pias->cb1_w(1);
			m_irq_active = false;
			m_pia28->ca1_w(1);
			m_pia28->cb1_w(1);
		}
		break;
	}
}

MACHINE_RESET_MEMBER( s11a_state, s11a )
{
	membank("bank0")->set_entry(0);
	membank("bank1")->set_entry(0);
	membank("bgbank")->set_entry(0);
}

INPUT_CHANGED_MEMBER( s11a_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( s11a_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE_LINE_MEMBER( s11a_state::pia_irq )
{
	if(state == CLEAR_LINE)
	{
		// restart IRQ timer
		m_irq_timer->adjust(attotime::from_ticks(S11_IRQ_CYCLES,XTAL_4MHz/2),1);
		m_irq_active = false;
	}
	else
	{
		// disable IRQ timer while other IRQs are being handled
		// (counter is reset every 32 cycles while a PIA IRQ is handled)
		m_irq_timer->adjust(attotime::zero);
		m_irq_active = true;
	}
}

WRITE8_MEMBER( s11a_state::sol3_w )
{

}

WRITE8_MEMBER( s11a_state::sound_w )
{
	m_sound_data = data;
}

WRITE_LINE_MEMBER( s11a_state::pia21_ca2_w )
{
// sound ns
	m_ca1 = state;
	m_pias->ca1_w(m_ca1);
	m_pia40->cb2_w(m_ca1);
}

static const pia6821_interface pia21_intf =
{
	DEVCB_DRIVER_MEMBER(s11a_state, dac_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_LINE_GND,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11a_state, sound_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11a_state, sol2_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia21_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia21_cb2_w),		/* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia_irq),		/* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia_irq)		/* IRQB */
};

WRITE8_MEMBER( s11a_state::lamp0_w )
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
	DEVCB_DRIVER_MEMBER(s11a_state, lamp0_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11a_state, lamp1_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia24_cb2_w),		/* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia_irq),		/* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia_irq)		/* IRQB */
};

WRITE8_MEMBER( s11a_state::dig0_w )
{
	data &= 0x7f;
	m_strobe = data & 15;
	m_diag = (data & 0x70) >> 4;
	output_set_digit_value(60, 0);  // not connected to PA5 or PA6?
	output_set_digit_value(61, m_diag & 0x01);  // connected to PA4
	output_set_digit_value(62, 0);
	m_segment1 = 0;
	m_segment2 = 0;
}

WRITE8_MEMBER( s11a_state::dig1_w )
{
	m_segment2 |= data;
	m_segment2 |= 0x20000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		output_set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment2 |= 0x40000;
	}
}

READ8_MEMBER( s11a_state::pia28_w7_r)
{
	UINT8 ret = 0x80;

	ret |= m_strobe;
	ret |= m_diag << 4;

	if(BIT(ioport("DIAGS")->read(), 4))  // W7 Jumper
		ret &= ~0x80;

	return ret;
}

static const pia6821_interface pia28_intf =
{
	DEVCB_DRIVER_MEMBER(s11a_state, pia28_w7_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11a_state, dig0_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11a_state, dig1_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia28_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia28_cb2_w),		/* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia_irq),		/* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia_irq)		/* IRQB */
};

WRITE8_MEMBER( s11a_state::pia2c_pa_w )
{
	m_segment1 |= (data<<8);
	m_segment1 |= 0x10000;
	if ((m_segment1 & 0x70000) == 0x30000)
	{
		output_set_digit_value(m_strobe, BITSWAP16(m_segment1, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment1 |= 0x40000;
	}
}

WRITE8_MEMBER( s11a_state::pia2c_pb_w )
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
	DEVCB_DRIVER_MEMBER(s11a_state, pia2c_pa_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11a_state, pia2c_pb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia_irq),		/* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia_irq)		/* IRQB */
};

READ8_MEMBER( s11a_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"X%X",m_kbdrow);
	return ~ioport(kbdrow)->read();
}

WRITE8_MEMBER( s11a_state::switch_w )
{
	m_kbdrow = data;
}

static const pia6821_interface pia30_intf =
{
	DEVCB_DRIVER_MEMBER(s11a_state, switch_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_LINE_GND,		/* line CA1 in */
	DEVCB_LINE_GND,		/* line CB1 in */
	DEVCB_LINE_VCC,		/* line CA2 in */
	DEVCB_LINE_VCC,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_DRIVER_MEMBER(s11a_state, switch_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia30_cb2_w),		/* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia_irq),		/* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia_irq)		/* IRQB */
};

WRITE8_MEMBER( s11a_state::pia34_pa_w )
{
	m_segment2 |= (data<<8);
	m_segment2 |= 0x10000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		output_set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment2 |= 0x40000;
	}
}

WRITE8_MEMBER( s11a_state::pia34_pb_w )
{
	m_pia40->portb_w(data);
}

WRITE_LINE_MEMBER( s11a_state::pia34_cb2_w )
{
	m_pia40->cb1_w(state);  // MCB2 through CPU interface
}

static const pia6821_interface pia34_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11a_state, pia34_pa_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11a_state, pia34_pb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia34_cb2_w),		/* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia_irq),		/* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia_irq)		/* IRQB */
};

WRITE8_MEMBER( s11a_state::bank_w )
{
	membank("bank0")->set_entry(BIT(data, 1));
	membank("bank1")->set_entry(BIT(data, 0));
}

WRITE8_MEMBER( s11a_state::bgbank_w )
{
	membank("bgbank")->set_entry(BIT(data, 0));
}

READ_LINE_MEMBER( s11a_state::pias_ca1_r )
{
	return m_ca1;
}

WRITE_LINE_MEMBER( s11a_state::pias_ca2_w )
{
// speech clock
	hc55516_clock_w(m_hc55516, state);
}

WRITE_LINE_MEMBER( s11a_state::pias_cb2_w )
{
// speech data
	hc55516_digit_w(m_hc55516, state);
}

READ8_MEMBER( s11a_state::dac_r )
{
	return m_sound_data;
}

WRITE8_MEMBER( s11a_state::dac_w )
{
	m_dac->write_unsigned8(data);
}

WRITE_LINE_MEMBER( s11a_state::pia40_cb2_w)
{
	m_pia34->cb1_w(state);  // To Widget MCB1 through CPU Data interface
}

static const pia6821_interface pias_intf =
{
	DEVCB_DRIVER_MEMBER(s11a_state, dac_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pias_ca1_r),		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11a_state, sound_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11a_state, dac_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pia40_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("audiocpu", M6800_IRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("audiocpu", M6800_IRQ_LINE)		/* IRQB */
};

WRITE8_MEMBER( s11a_state::pia40_pa_w )
{
	m_dac1->write_unsigned8(data);
}

WRITE8_MEMBER( s11a_state::pia40_pb_w )
{
	m_pia34->portb_w(data);
}

WRITE_LINE_MEMBER( s11a_state::ym2151_irq_w)
{
	if(state == CLEAR_LINE)
		m_pia40->ca1_w(1);
	else
		m_pia40->ca1_w(0);
}

static const pia6821_interface pia40_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pias_ca1_r),		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_LINE_VCC,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11a_state, pia40_pa_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(s11a_state, pia40_pb_w),		/* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pias_ca2_w),		/* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11a_state, pias_cb2_w),		/* line CB2 out */
	DEVCB_CPU_INPUT_LINE("bgcpu", M6809_FIRQ_LINE),		/* IRQA */
	DEVCB_CPU_INPUT_LINE("bgcpu", INPUT_LINE_NMI)		/* IRQB */
};

DRIVER_INIT_MEMBER( s11a_state, s11a )
{
	UINT8 *ROM = memregion("audiocpu")->base();
	UINT8 *BGROM = memregion("bgcpu")->base();
	membank("bank0")->configure_entries(0, 2, &ROM[0x10000], 0x4000);
	membank("bank1")->configure_entries(0, 2, &ROM[0x18000], 0x4000);
	membank("bgbank")->configure_entries(0, 2, &BGROM[0x10000], 0x8000);
	membank("bank0")->set_entry(0);
	membank("bank1")->set_entry(0);
	membank("bgbank")->set_entry(0);
	m_irq_timer = timer_alloc(TIMER_IRQ);
	m_irq_timer->adjust(attotime::from_ticks(S11_IRQ_CYCLES,XTAL_4MHz/2),1);
	m_irq_active = false;
}

static MACHINE_CONFIG_START( s11a, s11a_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6808, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(s11a_main_map)
	MCFG_MACHINE_RESET_OVERRIDE(s11a_state, s11a)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_s11a)

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
	MCFG_CPU_ADD("audiocpu", M6802, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(s11a_audio_map)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SPEAKER_STANDARD_MONO("speech")
	MCFG_SOUND_ADD("hc55516", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speech", 0.50)

	MCFG_PIA6821_ADD("pias", pias_intf)

	/* Add the background music card */
	MCFG_CPU_ADD("bgcpu", M6809E, 8000000) // MC68B09E
	MCFG_CPU_PROGRAM_MAP(s11a_bg_map)

	MCFG_SPEAKER_STANDARD_MONO("bg")
	MCFG_YM2151_ADD("ym2151", 3580000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(s11a_state, ym2151_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)

	MCFG_PIA6821_ADD("pia40", pia40_intf)
MACHINE_CONFIG_END

/*------------------------
/ F14 Tomcat 5/87 (#554)
/-------------------------*/

ROM_START(f14_p3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("f14_l3.u26", 0x4000, 0x4000, CRC(cd607556) SHA1(2ec95085784370a071cbf5df7ae5c6b4749605e2))
	ROM_LOAD("f14_l3.u27", 0x8000, 0x8000, CRC(72951fd1) SHA1(b5f3fe1859e0abf9ab558b4b4f6754134d528c23))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u21.l1", 0x18000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x10000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))

	ROM_REGION(0x20000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u4.l1", 0x10000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_LOAD("f14_u19.l1", 0x18000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
ROM_END

ROM_START(f14_p4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26_l4.128", 0x4000, 0x4000, CRC(7b39706a) SHA1(0dc0b1a1dfd12bc73e6fd8b825fe72ddc8fc1497))
	ROM_LOAD("u27_l4.256", 0x8000, 0x8000, CRC(189f9488) SHA1(7536d56cb83bf29f8d8b03b226a5f60200776095))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u21.l1", 0x18000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x10000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))

	ROM_REGION(0x20000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u4.l1", 0x10000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_LOAD("f14_u19.l1", 0x18000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
ROM_END

ROM_START(f14_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("f14_u26.l1", 0x4000, 0x4000, CRC(62c2e615) SHA1(456ce0d1f74fa5e619c272880ba8ac6819848ddc))
	ROM_LOAD("f14_u27.l1", 0x8000, 0x8000, CRC(da1740f7) SHA1(1395a4f3891a043cfedc5426ec88af35eab8d4ea))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u21.l1", 0x18000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x10000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))

	ROM_REGION(0x20000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u4.l1", 0x10000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_LOAD("f14_u19.l1", 0x18000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
ROM_END

/*--------------------
/ Fire! 8/87 (#556)
/--------------------*/
ROM_START(fire_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("fire_u26.l3", 0x4000, 0x4000, CRC(48abae33) SHA1(00ce24316aa007eec090ae74818003e11a141214))
	ROM_LOAD("fire_u27.l3", 0x8000, 0x8000, CRC(4ebf4888) SHA1(45dc0231404ed70be2ab5d599a673aac6271550e))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("fire_u21.l2", 0x18000, 0x8000, CRC(2edde0a4) SHA1(de292a340a3a06b0b996fc69fee73eb7bbfbbe64))
	ROM_LOAD("fire_u22.l2", 0x10000, 0x8000, CRC(16145c97) SHA1(523e99df3907a2c843c6e27df4d16799c4136a46))

	ROM_REGION(0x20000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("fire_u4.l1", 0x10000, 0x8000, CRC(0e058918) SHA1(4d6bf2290141119174787f8dd653c47ea4c73693))
ROM_END

/*--------------------------------------
/ Fire! Champagne Edition 9/87 (#556SE)
/---------------------------------------*/

/*-------------------------
/ Millionaire 1/87 (#555)
/--------------------------*/
ROM_START(milln_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mill_u26.l3", 0x4000, 0x4000, CRC(07bc9fff) SHA1(b16082fb51df3e4d2fb786cb8894b1c232521ef3))
	ROM_LOAD("mill_u27.l3", 0x8000, 0x8000, CRC(ba789c43) SHA1(c066a304882bea4cba1e215642416fcb22585aa4))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("mill_u21.l1", 0x18000, 0x8000, CRC(4cd1ee90) SHA1(4e24b96138ced16eff9036303ca6347e3423dbfc))
	ROM_LOAD("mill_u22.l1", 0x10000, 0x8000, CRC(73735cfc) SHA1(f74c873a20990263e0d6b35609fc51c08c9f8e31))

	ROM_REGION(0x20000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("mill_u4.l1", 0x10000, 0x8000, CRC(cf766506) SHA1(a6e4df19a513102abbce2653d4f72245f54407b1))
	ROM_LOAD("mill_u19.l1", 0x18000, 0x8000, CRC(e073245a) SHA1(cbaddde6bb19292ace574a8329e18c97c2ee9763))
ROM_END

/*--------------------
/ Pinbot 10/86 (#549)
/--------------------*/
ROM_START(pb_l5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pbot_u26.l5", 0x4000, 0x4000, CRC(daa0c8e4) SHA1(47289b350eb0d84aa0d37e53383e18625451bbe8))
	ROM_LOAD("pbot_u27.l5", 0x8000, 0x8000, CRC(e625d6ce) SHA1(1858dc2183954342b8e2e5eb9a14edcaa8dad5ae))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u21.l1", 0x18000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x10000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))

	ROM_REGION(0x20000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u4.l1", 0x10000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_LOAD("pbot_u19.l1", 0x18000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
ROM_END

ROM_START(pb_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l2.rom", 0x4000, 0x4000, CRC(e3b94ca4) SHA1(1db2acb025941cc165cc7ec70a160e07ab1eeb2e))
	ROM_LOAD("u27-l2.rom", 0x8000, 0x8000, CRC(0a334fc5) SHA1(d08afe6ddc141e37f97ea588d184a316ff7f6db7))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u21.l1", 0x18000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x10000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))

	ROM_REGION(0x20000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u4.l1", 0x10000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_LOAD("pbot_u19.l1", 0x18000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
ROM_END

ROM_START(pb_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l2.rom", 0x4000, 0x4000, CRC(e3b94ca4) SHA1(1db2acb025941cc165cc7ec70a160e07ab1eeb2e))
	ROM_LOAD("u27-l3.rom", 0x8000, 0x8000, CRC(6f40ee84) SHA1(85453137e3fdb1e422e3903dd053e04c9f2b9607))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u21.l1", 0x18000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x10000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))

	ROM_REGION(0x20000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u4.l1", 0x10000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_LOAD("pbot_u19.l1", 0x18000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
ROM_END

GAME(1987, f14_l1,   0,      s11a, s11a, s11a_state, s11a, ROT0, "Williams", "F14 Tomcat (L-1)", GAME_IS_SKELETON_MECHANICAL)
GAME(1987, f14_p3,   f14_l1, s11a, s11a, s11a_state, s11a, ROT0, "Williams", "F14 Tomcat (P-3)", GAME_IS_SKELETON_MECHANICAL)
GAME(1987, f14_p4,   f14_l1, s11a, s11a, s11a_state, s11a, ROT0, "Williams", "F14 Tomcat (P-4)", GAME_IS_SKELETON_MECHANICAL)
GAME(1987, fire_l3,  0,      s11a, s11a, s11a_state, s11a, ROT0, "Williams", "Fire! (L-3)", GAME_IS_SKELETON_MECHANICAL)
GAME(1987, milln_l3, 0,      s11a, s11a, s11a_state, s11a, ROT0, "Williams", "Millionaire (L-3)", GAME_IS_SKELETON_MECHANICAL)
GAME(1986, pb_l5,    0,      s11a, s11a, s11a_state, s11a, ROT0, "Williams", "Pin-Bot (L-5)", GAME_IS_SKELETON_MECHANICAL)
GAME(1986, pb_l2,    pb_l5,  s11a, s11a, s11a_state, s11a, ROT0, "Williams", "Pin-Bot (L-2)", GAME_IS_SKELETON_MECHANICAL)
GAME(1986, pb_l3,    pb_l5,  s11a, s11a, s11a_state, s11a, ROT0, "Williams", "Pin-Bot (L-3)", GAME_IS_SKELETON_MECHANICAL)
