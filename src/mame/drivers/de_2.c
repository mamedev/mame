/*
    DataEast/Sega Version 2
*/


#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "sound/2151intf.h"
#include "sound/msm5205.h"
#include "de2.lh"

// To start Secret Service, hold I, O and Left ALT while pressing Start.

// Data East CPU board is similar to Williams System 11, but without the generic audio board.
// For now, we'll presume the timings are the same.

// 6808 CPU's input clock is 4MHz
// but because it has an internal /4 divider, its E clock runs at 1/4 that frequency
#define E_CLOCK (XTAL_4MHz/4)

// Length of time in cycles between IRQs on the main 6808 CPU
// This length is determined by the settings of the W14 and W15 jumpers
// It can be 0x300, 0x380, 0x700 or 0x780 cycles long.
// IRQ length is always 32 cycles
#define S11_IRQ_CYCLES 0x380

class de_2_state : public genpin_class
{
public:
	de_2_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ym2151(*this, "ym2151"),
			m_pia21(*this, "pia21"),
			m_pia24(*this, "pia24"),
			m_pia28(*this, "pia28"),
			m_pia2c(*this, "pia2c"),
			m_pia30(*this, "pia30"),
			m_pia34(*this, "pia34"),
			m_audiocpu(*this, "audiocpu"),
			m_msm5205(*this, "msm5205")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<ym2151_device> m_ym2151;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia2c;
	required_device<pia6821_device> m_pia30;
	required_device<pia6821_device> m_pia34;

	// driver_device overrides
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	static const device_timer_id TIMER_IRQ = 0;

public:
	DECLARE_DRIVER_INIT(de_2);
	DECLARE_MACHINE_RESET(de_2);
	DECLARE_WRITE8_MEMBER(sample_w);
	DECLARE_WRITE8_MEMBER(pia34_pa_w);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_WRITE8_MEMBER(pia2c_pa_w);
	DECLARE_WRITE8_MEMBER(pia2c_pb_w);
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { }; // comma3&4
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { }; // comma1&2
	DECLARE_READ8_MEMBER(pia28_w7_r);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(lamp0_w);
	DECLARE_WRITE8_MEMBER(lamp1_w) { };
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	DECLARE_WRITE_LINE_MEMBER(msm5205_irq_w);
	DECLARE_WRITE_LINE_MEMBER(pia_irq);
	DECLARE_WRITE8_MEMBER(sol2_w) { }; // solenoids 8-15
	DECLARE_WRITE8_MEMBER(sol3_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w);
	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

	DECLARE_READ8_MEMBER(sound_latch_r);
	DECLARE_WRITE8_MEMBER(sample_bank_w);

	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm5205;
	UINT8 m_sample_data;
	bool m_more_data;
	bool m_nmi_enable;

private:
	UINT32 m_segment1;
	UINT32 m_segment2;
	UINT8 m_strobe;
	UINT8 m_kbdrow;
	UINT8 m_diag;
	bool m_ca1;
	emu_timer* m_irq_timer;
	bool m_irq_active;
	UINT8 m_sound_data;

	UINT8 m_sample_bank;
	UINT8 m_msm_prescaler;
};

static ADDRESS_MAP_START( de_2_map, AS_PROGRAM, 8, de_2_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2100, 0x2103) AM_DEVREADWRITE("pia21", pia6821_device, read, write) // sound+solenoids
	AM_RANGE(0x2200, 0x2200) AM_WRITE(sol3_w) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_DEVREADWRITE("pia24", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("pia28", pia6821_device, read, write) // display
	AM_RANGE(0x2c00, 0x2c03) AM_DEVREADWRITE("pia2c", pia6821_device, read, write) // alphanumeric display
	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("pia30", pia6821_device, read, write) // inputs
	AM_RANGE(0x3400, 0x3403) AM_DEVREADWRITE("pia34", pia6821_device, read, write) // widget
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( de_2_audio_map, AS_PROGRAM, 8, de_2_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE("ym2151", ym2151_device, read, write)
	AM_RANGE(0x2400, 0x2400) AM_READ(sound_latch_r)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(sample_bank_w)
	// 0x2c00        - 4052(?)
	AM_RANGE(0x3000, 0x3000) AM_WRITE(sample_w)
	// 0x3800        - Watchdog reset
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("sample_bank")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( de_2 )
	PORT_START("INP0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INP1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("INP2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("INP4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("INP8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("INP10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("INP20")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("INP40")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INP80")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, de_2_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, de_2_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_9) PORT_TOGGLE
	PORT_CONFNAME( 0x10, 0x10, "Language" )
	PORT_CONFSETTING( 0x00, "German" )
	PORT_CONFSETTING( 0x10, "English" )
INPUT_PORTS_END

void de_2_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_IRQ:
		if(param == 1)
		{
			m_maincpu->set_input_line(M6800_IRQ_LINE,ASSERT_LINE);
			m_irq_timer->adjust(attotime::from_ticks(32,E_CLOCK),0);
			m_irq_active = true;
			m_pia28->ca1_w(BIT(ioport("DIAGS")->read(), 2));  // Advance
			m_pia28->cb1_w(BIT(ioport("DIAGS")->read(), 3));  // Up/Down
		}
		else
		{
			m_maincpu->set_input_line(M6800_IRQ_LINE,CLEAR_LINE);
			m_irq_timer->adjust(attotime::from_ticks(S11_IRQ_CYCLES,E_CLOCK),1);
			m_irq_active = false;
			m_pia28->ca1_w(1);
			m_pia28->cb1_w(1);
		}
		break;
	}
}


MACHINE_RESET_MEMBER(de_2_state, de_2)
{
	membank("sample_bank")->set_entry(0);
	m_more_data = false;
}

DRIVER_INIT_MEMBER(de_2_state, de_2)
{
	UINT8 *ROM = memregion("sound1")->base();
	m_irq_timer = timer_alloc(TIMER_IRQ);
	m_irq_timer->adjust(attotime::from_ticks(S11_IRQ_CYCLES,E_CLOCK),1);
	m_irq_active = false;
	membank("sample_bank")->configure_entries(0, 16, &ROM[0x0000], 0x4000);
	membank("sample_bank")->set_entry(0);
}

WRITE_LINE_MEMBER(de_2_state::ym2151_irq_w)
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE,state);
}

//WRITE_LINE_MEMBER(de_2_state::msm5205_irq_w)
static void msm5205_irq_w(device_t* device)
{
	de_2_state* state = device->machine().driver_data<de_2_state>();
	msm5205_data_w(state->m_msm5205,state->m_sample_data >> 4);
	if(state->m_more_data)
	{
		if(state->m_nmi_enable)
			state->m_audiocpu->set_input_line(INPUT_LINE_NMI,PULSE_LINE);  // generate NMI when we need more data
		state->m_more_data = false;
	}
	else
	{
		state->m_more_data = true;
		state->m_sample_data <<= 4;
	}
}

WRITE_LINE_MEMBER(de_2_state::pia_irq)
{
	if(state == CLEAR_LINE)
	{
		// restart IRQ timer
		m_irq_timer->adjust(attotime::from_ticks(S11_IRQ_CYCLES,E_CLOCK),1);
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

INPUT_CHANGED_MEMBER( de_2_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( de_2_state::audio_nmi )
{
	// Not on DECO board?
	// Diagnostic button sends a pulse to NMI pin
//	if (newval==CLEAR_LINE)
//		if(m_audiocpu)
//			m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

// 6821 PIA at 0x2000
WRITE8_MEMBER( de_2_state::sol3_w )
{
}

WRITE8_MEMBER( de_2_state::sound_w )
{
	m_sound_data = data;
	m_audiocpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}

WRITE_LINE_MEMBER( de_2_state::pia21_ca2_w )
{
// sound ns
	m_ca1 = state;
}

static const pia6821_interface pia21_intf =
{
	DEVCB_NULL, //DEVCB_DRIVER_MEMBER(de_2_state, dac_r),      /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_LINE_GND,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(de_2_state, sol2_w),        /* port A out */
	DEVCB_NULL,     /* port B out */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia21_ca2_w),       /* line CA2 out */
	DEVCB_NULL, //DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia21_cb2_w),       /* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia_irq),       /* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia_irq)        /* IRQB */
};


// 6821 PIA at 0x2400
WRITE8_MEMBER( de_2_state::lamp0_w )
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

static const pia6821_interface pia24_intf =
{
	DEVCB_NULL,     /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_LINE_GND,     /* line CA1 in */
	DEVCB_LINE_GND,     /* line CB1 in */
	DEVCB_LINE_VCC,     /* line CA2 in */
	DEVCB_LINE_VCC,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(de_2_state, lamp0_w),        /* port A out */
	DEVCB_DRIVER_MEMBER(de_2_state, lamp1_w),        /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL, //DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia24_cb2_w),       /* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia_irq),       /* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia_irq)        /* IRQB */
};

// 6821 PIA at 0x2800
WRITE8_MEMBER( de_2_state::dig0_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	data &= 0x7f;
	m_strobe = data & 15;
	m_diag = (data & 0x70) >> 4;
	output_set_digit_value(60, patterns[data>>4]); // diag digit
	m_segment1 = 0;
	m_segment2 = 0;
}

WRITE8_MEMBER( de_2_state::dig1_w )
{
	m_segment2 |= data;
	m_segment2 |= 0x30000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		output_set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 11, 15, 12, 10, 8, 14, 13, 9, 7, 6, 5, 4, 3, 2, 1, 0));
		m_segment2 |= 0x40000;
	}
}

READ8_MEMBER( de_2_state::pia28_w7_r )
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
	DEVCB_DRIVER_MEMBER(de_2_state, pia28_w7_r),     /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(de_2_state, dig0_w),     /* port A out */
	DEVCB_DRIVER_MEMBER(de_2_state, dig1_w),     /* port B out */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia28_ca2_w),       /* line CA2 out */  // comma 3+4
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia28_cb2_w),       /* line CB2 out */  // comma 1+2
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia_irq),       /* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia_irq)        /* IRQB */
};


// 6821 PIA at 0x2c00
WRITE8_MEMBER( de_2_state::pia2c_pa_w )
{
	m_segment1 |= (data<<8);
	m_segment1 |= 0x10000;
	if ((m_segment1 & 0x70000) == 0x30000)
	{
		output_set_digit_value(m_strobe, BITSWAP16(m_segment1, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment1 |= 0x40000;
	}
}

WRITE8_MEMBER( de_2_state::pia2c_pb_w )
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
	DEVCB_NULL,     /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(de_2_state, pia2c_pa_w),     /* port A out */
	DEVCB_DRIVER_MEMBER(de_2_state, pia2c_pb_w),     /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL,     /* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia_irq),       /* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia_irq)        /* IRQB */
};


// 6821 PIA at 0x3000
READ8_MEMBER( de_2_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"INP%X",m_kbdrow);
	return ~ioport(kbdrow)->read();
}

WRITE8_MEMBER( de_2_state::switch_w )
{
	m_kbdrow = data;
}

static const pia6821_interface pia30_intf =
{
	DEVCB_DRIVER_MEMBER(de_2_state, switch_r),       /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_LINE_GND,     /* line CA1 in */
	DEVCB_LINE_GND,     /* line CB1 in */
	DEVCB_LINE_VCC,     /* line CA2 in */
	DEVCB_LINE_VCC,     /* line CB2 in */
	DEVCB_NULL,     /* port A out */
	DEVCB_DRIVER_MEMBER(de_2_state, switch_w),       /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL, //DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia30_cb2_w),       /* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia_irq),       /* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia_irq)        /* IRQB */
};

// 6821 PIA at 0x3400
WRITE8_MEMBER( de_2_state::pia34_pa_w )
{
	// Not connected?
	m_segment2 |= (data<<8);
	m_segment2 |= 0x10000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		output_set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment2 |= 0x40000;
	}
}

static const pia6821_interface pia34_intf =
{
	DEVCB_NULL,     /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(de_2_state, pia34_pa_w),     /* port A out */
	DEVCB_DRIVER_MEMBER(de_2_state, sound_w),     /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL, //DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia34_cb2_w),       /* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia_irq),       /* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(de_2_state, pia_irq)        /* IRQB */
};


// Sound board
WRITE8_MEMBER(de_2_state::sample_w)
{
	m_sample_data = data;
}

READ8_MEMBER( de_2_state::sound_latch_r )
{
	m_audiocpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	return m_sound_data;
}

WRITE8_MEMBER( de_2_state::sample_bank_w )
{
	static const UINT8 prescale[4] = { MSM5205_S96_4B, MSM5205_S48_4B, MSM5205_S64_4B, 0 };

	m_sample_bank = (data & 0x07);
	membank("sample_bank")->set_entry(m_sample_bank);
	m_msm_prescaler = (data & 0x30) >> 4;
	m_nmi_enable = (~data & 0x80);
	msm5205_playmode_w(m_msm5205,prescale[m_msm_prescaler]);
	msm5205_reset_w(m_msm5205,data & 0x40);
}

static const msm5205_interface msm5205_intf =
{
	msm5205_irq_w,
	MSM5205_S96_4B
};

static MACHINE_CONFIG_START( de_2, de_2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6808, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(de_2_map)
	MCFG_MACHINE_RESET_OVERRIDE(de_2_state, de_2)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_de2)

	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_PIA6821_ADD("pia21", pia21_intf)
	MCFG_PIA6821_ADD("pia24", pia24_intf)
	MCFG_PIA6821_ADD("pia28", pia28_intf)
	MCFG_PIA6821_ADD("pia2c", pia2c_intf)
	MCFG_PIA6821_ADD("pia30", pia30_intf)
	MCFG_PIA6821_ADD("pia34", pia34_intf)
	MCFG_NVRAM_ADD_1FILL("nvram")

	/* sound CPU */
	MCFG_CPU_ADD("audiocpu", M6809E, 8000000) // MC68B09E
	MCFG_CPU_PROGRAM_MAP(de_2_audio_map)

	MCFG_SPEAKER_STANDARD_MONO("bg")
	MCFG_YM2151_ADD("ym2151", 3580000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(de_2_state, ym2151_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)
	MCFG_SOUND_ADD("msm5205", MSM5205, 384000)
	MCFG_SOUND_CONFIG(msm5205_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)
MACHINE_CONFIG_END

/*-----------------------------------------------------------------------------------
/ Monday Night Football - CPU Rev 2 /Alpha Type 3 16/32K Roms - 32/64K Sound Roms
/----------------------------------------------------------------------------------*/
ROM_START(mnfb_c27)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mnfb2-7.b5", 0x4000, 0x4000, CRC(995eb9b8) SHA1(d05d74393fda59ffd8d7b5546313779cdb10d23e))
	ROM_LOAD("mnfb2-7.c5", 0x8000, 0x8000, CRC(579d81df) SHA1(9c96da34d37d3369513003e208222bd6e8698638))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("mnf-f7.256", 0x8000, 0x8000, CRC(fbc2d6f6) SHA1(33173c081de776d32e926481e94b265ec48d770b))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("mnf-f5-6.512", 0x00000, 0x10000, CRC(0c6ea963) SHA1(8c88fa588222ef8a6c872b8c5b49639b108384d4))
	ROM_LOAD("mnf-f4-5.512", 0x10000, 0x10000, CRC(efca5d80) SHA1(9655c885dd64aa170205170b6a0c052bd9367379))
ROM_END

/*-------------------------------------------------------------------------------
/ Phantom of the Opera - CPU Rev 3 /Alpha Type 3 16/32K Roms - 32/64K Sound Roms
/-------------------------------------------------------------------------------*/
ROM_START(poto_a32)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("potob5.3-2", 0x4000, 0x4000, CRC(bdc39205) SHA1(67b3f56655ef2cc056912ab6e351cf83352abaa9))
	ROM_LOAD("potoc5.3-2", 0x8000, 0x8000, CRC(e6026455) SHA1(c1441fda6181e9014a8a6f93b7405998a952f508))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("potof7.rom", 0x8000, 0x8000, CRC(2e60b2e3) SHA1(0be89fc9b2c6548392febb35c1ace0eb912fc73f))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("potof6.rom", 0x00000, 0x10000, CRC(62b8f74b) SHA1(f82c706b88f49341bab9014bd83371259eb53b47))
	ROM_LOAD("potof5.rom", 0x10000, 0x10000, CRC(5a0537a8) SHA1(26724441d7e2edd7725337b262d95448499151ad))
ROM_END

/*-----------------------------------------------------------------------------------
/ Playboy 35th Anniversary - CPU Rev 2 /Alpha Type 2 - 32K Roms - 32/64K Sound Roms
/-----------------------------------------------------------------------------------*/
ROM_START(play_a24)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("play2-4.b5", 0x0000, 0x8000, CRC(bc8d7b32) SHA1(3b57dea2feb12315586283548e0bffdc8173b8fb))
	ROM_LOAD("play2-4.c5", 0x8000, 0x8000, CRC(47c30bc2) SHA1(c62e192ec01f4884226e9628baa2cad10cc57bd9))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("pbsnd7.dat", 0x8000, 0x8000, CRC(c2cf2cc5) SHA1(1277704b1b38558c341b52da5e06ffa9f07942ad))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("pbsnd6.dat", 0x00000, 0x10000, CRC(c2570631) SHA1(135db5b923689884c73aa5ce48f566db7f1cf831))
	ROM_LOAD("pbsnd5.dat", 0x10000, 0x10000, CRC(0fd30569) SHA1(0bf53fe4b5dffb5e15212c3371f51e98ad14e258))
ROM_END

/*------------------------------------------------------------------
/ Robocop - CPU Rev 3 /Alpha Type 3 - 32K Roms - 32/64K Sound Roms
/-----------------------------------------------------------------*/
ROM_START(robo_a34)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("robob5.a34", 0x0000, 0x8000, CRC(5a611004) SHA1(08722f8f4386bbc467cfbe8854f0d45c4537bdc6))
	ROM_LOAD("roboc5.a34", 0x8000, 0x8000, CRC(c8705f47) SHA1(a29ad9e4e0269ab19dae77b1e70ff84c8c8d9e85))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("robof7.rom", 0x8000, 0x8000, CRC(fa0891bd) SHA1(332d03c7802989abf717564230993b54819ebc0d))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("robof6.rom", 0x00000, 0x10000, CRC(9246e107) SHA1(e8e72c0d099b17ea9e59ea7794011bad4c072c5e))
	ROM_LOAD("robof4.rom", 0x10000, 0x10000, CRC(27d31df3) SHA1(1611a508ce74eb62a07296d69782ea4fa14503fc))
ROM_END

/*-------------------------------------------------------------------------
/ Secret Service - CPU Rev 2 /Alpha Type 2 - 32K Roms - 32K/64K Sound Roms
/-------------------------------------------------------------------------*/
ROM_START(ssvc_a26)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ssvc2-6.b5", 0x0000, 0x8000, CRC(e5eab8cd) SHA1(63cb678084d4fb2131ba64ed9de1294830057960))
	ROM_LOAD("ssvc2-6.c5", 0x8000, 0x8000, CRC(171b97ae) SHA1(9d678b7b91a5d50ea3cf4f2352094c2355f917b2))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sssndf7.rom", 0x8000, 0x8000, CRC(980778d0) SHA1(7c1f14d327b6d0e6d0fef058f96bb1cb440c9330))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("ssv1f6.rom", 0x00000, 0x10000, CRC(ccbc72f8) SHA1(c5c13fb8d05d7fb4005636655073d88b4d12d65e))
	ROM_LOAD("ssv2f4.rom", 0x10000, 0x10000, CRC(53832d16) SHA1(2227eb784e0221f1bf2bdf7ea48ecd122433f1ea))
ROM_END

ROM_START(ssvc_b26)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ssvc2-6.b5", 0x0000, 0x8000, CRC(e5eab8cd) SHA1(63cb678084d4fb2131ba64ed9de1294830057960))
	ROM_LOAD("ssvc2-6.c5", 0x8000, 0x8000, CRC(171b97ae) SHA1(9d678b7b91a5d50ea3cf4f2352094c2355f917b2))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sssndf7b.rom", 0x8000, 0x8000, CRC(4bd6b16a) SHA1(b9438a16cd35820628fe6eb82287b2c39fe4b1c6))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("ssv1f6.rom", 0x00000, 0x10000, CRC(ccbc72f8) SHA1(c5c13fb8d05d7fb4005636655073d88b4d12d65e))
	ROM_LOAD("ssv2f4.rom", 0x10000, 0x10000, CRC(53832d16) SHA1(2227eb784e0221f1bf2bdf7ea48ecd122433f1ea))
ROM_END

/*--------------------------------------------------------------------------
/ Time Machine - CPU Rev 2 /Alpha Type 2 16/32K Roms - 32/64K Sound Roms
/--------------------------------------------------------------------------*/
ROM_START(tmac_a24)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmach2-4.b5", 0x4000, 0x4000, CRC(6ef3cf07) SHA1(3fabfbb2166273bf5bfab06d92fff094d3331d1a))
	ROM_LOAD("tmach2-4.c5", 0x8000, 0x8000, CRC(b61035f5) SHA1(08436b68f37323f50c1fec86aba303a1690af653))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("tmachf7.rom", 0x8000, 0x8000, CRC(0f518bd4) SHA1(05e24ca0e76d576c65d9d2a01417f1ad2aa984bb))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("tmachf6.rom", 0x00000, 0x10000, CRC(47e61641) SHA1(93cd946ebc9f69d82512429a9ae5f2754499b00a))
	ROM_LOAD("tmachf4.rom", 0x10000, 0x10000, CRC(51e3aade) SHA1(38fc0f3a9c727bfd07fbcb16c3ca6d0560dc65c3))
ROM_END

ROM_START(tmac_a18)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmach1-8.b5", 0x4000, 0x4000, CRC(5dabdc4c) SHA1(67fe261888ddaa088abe2f8a331eaa5ac34be92e))
	ROM_LOAD("tmach1-8.c5", 0x8000, 0x8000, CRC(5a348def) SHA1(bf2b9a69d516d38e6f87c5886e0ba768c2dc28ab))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("tmachf7.rom", 0x8000, 0x8000, CRC(0f518bd4) SHA1(05e24ca0e76d576c65d9d2a01417f1ad2aa984bb))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("tmachf6.rom", 0x00000, 0x10000, CRC(47e61641) SHA1(93cd946ebc9f69d82512429a9ae5f2754499b00a))
	ROM_LOAD("tmachf4.rom", 0x10000, 0x10000, CRC(51e3aade) SHA1(38fc0f3a9c727bfd07fbcb16c3ca6d0560dc65c3))
ROM_END

/*-----------------------------------------------------------------------
/ Torpedo Alley - CPU Rev 2 /Alpha Type 2 - 32K Roms - 32/64K Sound Roms
/------------------------------------------------------------------------*/
ROM_START(torp_e21)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("torpe2-1.b5", 0x0000, 0x8000, CRC(ac0b03e3) SHA1(0ac57b2fec29cdc90ab35cba49844f0cf545d959))
	ROM_LOAD("torpe2-1.c5", 0x8000, 0x8000, CRC(9ad33882) SHA1(c4504d8e136f667652f79b54d4e8d775169c6ac3))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("torpef7.rom", 0x8000, 0x8000, CRC(26f4c33e) SHA1(114f85e93e7b699c4cd6ce1298f95228d439deba))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("torpef6.rom", 0x00000, 0x10000, CRC(b214a7ea) SHA1(d972148395581844e3eaed08f755f3e2217dbbc0))
	ROM_LOAD("torpef4.rom", 0x10000, 0x10000, CRC(83a4e7f3) SHA1(96deac9251fe68cc0319ac009becd424c4e444c5))
ROM_END

GAME(1989,  mnfb_c27,       0,          de_2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Monday Night Football (2.7, 50cts)",       GAME_IS_SKELETON_MECHANICAL)
GAME(1990,  poto_a32,       0,          de_2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "The Phantom of the Opera (3.2)",           GAME_IS_SKELETON_MECHANICAL)
GAME(1989,  play_a24,       0,          de_2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Playboy 35th Anniversary (2.4)",           GAME_IS_SKELETON_MECHANICAL)
GAME(1989,  robo_a34,       0,          de_2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Robocop (3.4)",                            GAME_IS_SKELETON_MECHANICAL)
GAME(1988,  ssvc_a26,       0,          de_2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Secret Service (2.6)",                     GAME_IS_SKELETON_MECHANICAL)
GAME(1988,  ssvc_b26,       ssvc_a26,   de_2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Secret Service (2.6 alternate sound)",     GAME_IS_SKELETON_MECHANICAL)
GAME(1988,  tmac_a24,       0,          de_2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Time Machine (2.4)",                       GAME_IS_SKELETON_MECHANICAL)
GAME(1988,  tmac_a18,       tmac_a24,   de_2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Time Machine (1.8)",                       GAME_IS_SKELETON_MECHANICAL)
GAME(1988,  torp_e21,       0,          de_2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Torpedo Alley (2.1, Europe)",              GAME_IS_SKELETON_MECHANICAL)
