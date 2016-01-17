// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
    DataEast/Sega Version 1 and 2

    Main CPU: 6808 @ 4MHz
    Audio CPU: 68B09E @ 8MHz (internally divided by 4)
    Audio: YM2151 @ 3.58MHz, MSM5205 @ 384kHz
*/


#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/decopincpu.h"
#include "machine/6821pia.h"
#include "sound/2151intf.h"
#include "sound/msm5205.h"
#include "de2.lh"
#include "de2a3.lh"

// To start Secret Service, hold I, O and Left ALT while pressing Start.
// To start Laser War, hold S, D, and F while pressing Start.
// To start Back to the Future, hold D and F while pressing Start.
// To start The Simpsons, hold D, F and G while pressing Start (can be tempremental)

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
	de_2_state(const machine_config &mconfig, device_type type, std::string tag)
		: genpin_class(mconfig, type, tag),
			m_ym2151(*this, "ym2151"),
			m_audiocpu(*this, "audiocpu"),
			m_msm5205(*this, "msm5205"),
			m_sample_bank(*this, "sample_bank")
	{ }

protected:

	// devices
	required_device<ym2151_device> m_ym2151;

public:
	DECLARE_DRIVER_INIT(de_2);
	DECLARE_MACHINE_RESET(de_2);
	DECLARE_MACHINE_RESET(de_2_alpha3);
	DECLARE_WRITE8_MEMBER(sample_w);
	DECLARE_WRITE8_MEMBER(pia34_pa_w);
	DECLARE_WRITE8_MEMBER(type2alpha3_pia34_pa_w);
	DECLARE_WRITE8_MEMBER(alpha3_pia34_pa_w);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_WRITE8_MEMBER(pia2c_pa_w);
	DECLARE_WRITE8_MEMBER(pia2c_pb_w);
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { }; // comma3&4
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { }; // comma1&2
	DECLARE_READ8_MEMBER(pia28_w7_r);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(type2alpha3_dig1_w);
	DECLARE_WRITE8_MEMBER(alpha3_dig1_w);
	DECLARE_WRITE8_MEMBER(lamp0_w);
	DECLARE_WRITE8_MEMBER(lamp1_w) { };
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	DECLARE_WRITE_LINE_MEMBER(msm5205_irq_w);
	DECLARE_WRITE8_MEMBER(sol2_w) { }; // solenoids 8-15
	DECLARE_WRITE8_MEMBER(sol3_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w);

	DECLARE_READ8_MEMBER(sound_latch_r);
	DECLARE_WRITE8_MEMBER(sample_bank_w);

	// devcb callbacks
	DECLARE_READ8_MEMBER(display_r);
	DECLARE_WRITE8_MEMBER(display_w);
	DECLARE_WRITE8_MEMBER(type2alpha3_display_w);
	DECLARE_WRITE8_MEMBER(alpha3_display_w);
	DECLARE_WRITE8_MEMBER(lamps_w);

	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm5205;
	required_memory_bank m_sample_bank;
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
	UINT8 m_sound_data;

	UINT8 m_sample_bank_num;
	UINT8 m_msm_prescaler;
};

/*static ADDRESS_MAP_START( de_2_map, AS_PROGRAM, 8, de_2_state )
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
*/
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

INPUT_PORTS_END


MACHINE_RESET_MEMBER(de_2_state, de_2)
{
	m_sample_bank->set_entry(0);
	m_more_data = false;
}

DRIVER_INIT_MEMBER(de_2_state, de_2)
{
	UINT8 *ROM = memregion("sound1")->base();
	m_sample_bank->configure_entries(0, 16, &ROM[0x0000], 0x4000);
	m_sample_bank->set_entry(0);
}

WRITE_LINE_MEMBER(de_2_state::ym2151_irq_w)
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE,state);
}

WRITE_LINE_MEMBER(de_2_state::msm5205_irq_w)
{
	m_msm5205->data_w(m_sample_data >> 4);
	if(m_more_data)
	{
		if(m_nmi_enable)
			m_audiocpu->set_input_line(INPUT_LINE_NMI,PULSE_LINE);  // generate NMI when we need more data
		m_more_data = false;
	}
	else
	{
		m_more_data = true;
		m_sample_data <<= 4;
	}
}

// 6821 PIA at 0x2100
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

// 6821 PIA at 0x2400
WRITE8_MEMBER( de_2_state::lamp0_w )
{
}

// 6821 PIA at 0x2800
WRITE8_MEMBER( de_2_state::dig0_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	data &= 0x7f;
	m_strobe = data & 15;
	m_diag = (data & 0x70) >> 4;
	output().set_digit_value(60, patterns[data>>4]); // diag digit
	m_segment1 = 0;
	m_segment2 = 0;
}

WRITE8_MEMBER( de_2_state::dig1_w )
{
	m_segment2 |= data;
	m_segment2 |= 0x30000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		output().set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 11, 15, 12, 10, 8, 14, 13, 9, 7, 6, 5, 4, 3, 2, 1, 0));
		m_segment2 |= 0x40000;
	}
}

WRITE8_MEMBER( de_2_state::type2alpha3_dig1_w )
{
	m_segment2 |= data;
	m_segment2 |= 0x20000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		output().set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 11, 15, 12, 10, 8, 14, 13, 9, 7, 6, 5, 4, 3, 2, 1, 0));
		m_segment2 |= 0x40000;
	}
}

WRITE8_MEMBER( de_2_state::alpha3_dig1_w )
{
	m_segment2 |= data;
	m_segment2 |= 0x20000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		output().set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment2 |= 0x40000;
	}
}

READ8_MEMBER( de_2_state::pia28_w7_r )
{
	UINT8 ret = 0x80;

	ret |= m_strobe;
	ret |= m_diag << 4;

	return ret;
}

// 6821 PIA at 0x2c00
WRITE8_MEMBER( de_2_state::pia2c_pa_w )
{
	m_segment1 |= (data<<8);
	m_segment1 |= 0x10000;
	if ((m_segment1 & 0x70000) == 0x30000)
	{
		output().set_digit_value(m_strobe, BITSWAP16(m_segment1, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment1 |= 0x40000;
	}
}

WRITE8_MEMBER( de_2_state::pia2c_pb_w )
{
	m_segment1 |= data;
	m_segment1 |= 0x20000;
	if ((m_segment1 & 0x70000) == 0x30000)
	{
		output().set_digit_value(m_strobe, BITSWAP16(m_segment1, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment1 |= 0x40000;
	}
}


// 6821 PIA at 0x3000
READ8_MEMBER( de_2_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"INP%X",m_kbdrow);
	return ~ioport(kbdrow)->read();
}

WRITE8_MEMBER( de_2_state::switch_w )
{
	int x;

	// about every second, 0xFF is written here, but it would be impossible to select more than one set of switches
	// at once, so just return the first bit set.  Maybe 0xFF has special meaning, or is just a disable?
	for(x=0;x<8;x++)
	{
		if(data & (1<<x))
			break;
	}
	m_kbdrow = data & (1<<x);
}

// 6821 PIA at 0x3400
WRITE8_MEMBER( de_2_state::pia34_pa_w )
{
	// Not connected on alphanumeric type 2 boards
}

WRITE8_MEMBER( de_2_state::type2alpha3_pia34_pa_w )
{
	m_segment2 |= (data<<8);
	m_segment2 |= 0x10000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		output().set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		m_segment2 |= 0x40000;
	}
}

WRITE8_MEMBER( de_2_state::alpha3_pia34_pa_w )
{
	m_segment2 |= (data<<8);
	m_segment2 |= 0x10000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		output().set_digit_value(m_strobe+16, BITSWAP16(m_segment2, 11, 15, 12, 10, 8, 14, 13, 9, 7, 6, 5, 4, 3, 2, 1, 0));
		m_segment2 |= 0x40000;
	}
}


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

	m_sample_bank_num = (data & 0x07);
	m_sample_bank->set_entry(m_sample_bank_num);
	m_msm_prescaler = (data & 0x30) >> 4;
	m_nmi_enable = (~data & 0x80);
	m_msm5205->playmode_w(prescale[m_msm_prescaler]);
	m_msm5205->reset_w(data & 0x40);
}

READ8_MEMBER(de_2_state::display_r)
{
	UINT8 ret = 0x00;

	switch(offset)
	{
	case 0:
		ret = pia28_w7_r(space,0);
		break;
	}

	return ret;
}

WRITE8_MEMBER(de_2_state::display_w)
{
	switch(offset)
	{
	case 0:
		dig0_w(space,0,data);
		break;
	case 1:
		dig1_w(space,0,data);
		break;
	case 2:
		pia2c_pa_w(space,0,data);
		break;
	case 3:
		pia2c_pb_w(space,0,data);
		break;
	case 4:
		pia34_pa_w(space,0,data);
		break;
	}
}

WRITE8_MEMBER(de_2_state::type2alpha3_display_w)
{
	switch(offset)
	{
	case 0:
		dig0_w(space,0,data);
		break;
	case 1:
		type2alpha3_dig1_w(space,0,data);
		break;
	case 2:
		pia2c_pa_w(space,0,data);
		break;
	case 3:
		pia2c_pb_w(space,0,data);
		break;
	case 4:
		type2alpha3_pia34_pa_w(space,0,data);
		break;
	}
}

WRITE8_MEMBER(de_2_state::alpha3_display_w)
{
	switch(offset)
	{
	case 0:
		dig0_w(space,0,data);
		break;
	case 1:
		alpha3_dig1_w(space,0,data);
		break;
	case 2:
		pia2c_pa_w(space,0,data);
		break;
	case 3:
		pia2c_pb_w(space,0,data);
		break;
	case 4:
		alpha3_pia34_pa_w(space,0,data);
		break;
	}
}

WRITE8_MEMBER(de_2_state::lamps_w)
{
	switch(offset)
	{
	case 0:
		lamp0_w(space,0,data);
		break;
	case 1:
		lamp1_w(space,0,data);
		break;
	}
}


static MACHINE_CONFIG_START( de_type1, de_2_state )
	/* basic machine hardware */
	MCFG_DECOCPU_TYPE1_ADD("decocpu",XTAL_8MHz / 2, ":maincpu")
	MCFG_DECOCPU_DISPLAY(READ8(de_2_state,display_r),WRITE8(de_2_state,display_w))
	MCFG_DECOCPU_SOUNDLATCH(WRITE8(de_2_state,sound_w))
	MCFG_DECOCPU_SWITCH(READ8(de_2_state,switch_r),WRITE8(de_2_state,switch_w))
	MCFG_DECOCPU_LAMP(WRITE8(de_2_state,lamps_w))
	MCFG_MACHINE_RESET_OVERRIDE(de_2_state, de_2)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_de2)

	MCFG_FRAGMENT_ADD( genpin_audio )

	/* sound CPU */
	MCFG_CPU_ADD("audiocpu", M6809E, 8000000) // MC68B09E
	MCFG_CPU_PROGRAM_MAP(de_2_audio_map)

	MCFG_SPEAKER_STANDARD_MONO("bg")
	MCFG_YM2151_ADD("ym2151", 3580000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(de_2_state, ym2151_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)
	MCFG_SOUND_ADD("msm5205", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(de_2_state, msm5205_irq_w))
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S96_4B)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( de_type2, de_2_state )
	/* basic machine hardware */
	MCFG_DECOCPU_TYPE2_ADD("decocpu",XTAL_8MHz / 2, ":maincpu")
	MCFG_DECOCPU_DISPLAY(READ8(de_2_state,display_r),WRITE8(de_2_state,display_w))
	MCFG_DECOCPU_SOUNDLATCH(WRITE8(de_2_state,sound_w))
	MCFG_DECOCPU_SWITCH(READ8(de_2_state,switch_r),WRITE8(de_2_state,switch_w))
	MCFG_DECOCPU_LAMP(WRITE8(de_2_state,lamps_w))
	MCFG_MACHINE_RESET_OVERRIDE(de_2_state, de_2)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_de2)

	MCFG_FRAGMENT_ADD( genpin_audio )

	/* sound CPU */
	MCFG_CPU_ADD("audiocpu", M6809E, 8000000) // MC68B09E
	MCFG_CPU_PROGRAM_MAP(de_2_audio_map)

	MCFG_SPEAKER_STANDARD_MONO("bg")
	MCFG_YM2151_ADD("ym2151", 3580000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(de_2_state, ym2151_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)
	MCFG_SOUND_ADD("msm5205", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(de_2_state, msm5205_irq_w))
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S96_4B)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( de_type2_alpha3, de_2_state )
	/* basic machine hardware */
	MCFG_DECOCPU_TYPE2_ADD("decocpu",XTAL_8MHz / 2, ":maincpu")
	MCFG_DECOCPU_DISPLAY(READ8(de_2_state,display_r),WRITE8(de_2_state,type2alpha3_display_w))
	MCFG_DECOCPU_SOUNDLATCH(WRITE8(de_2_state,sound_w))
	MCFG_DECOCPU_SWITCH(READ8(de_2_state,switch_r),WRITE8(de_2_state,switch_w))
	MCFG_DECOCPU_LAMP(WRITE8(de_2_state,lamps_w))
	MCFG_MACHINE_RESET_OVERRIDE(de_2_state, de_2)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_de2a3)

	MCFG_FRAGMENT_ADD( genpin_audio )

	/* sound CPU */
	MCFG_CPU_ADD("audiocpu", M6809E, 8000000) // MC68B09E
	MCFG_CPU_PROGRAM_MAP(de_2_audio_map)

	MCFG_SPEAKER_STANDARD_MONO("bg")
	MCFG_YM2151_ADD("ym2151", 3580000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(de_2_state, ym2151_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)
	MCFG_SOUND_ADD("msm5205", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(de_2_state, msm5205_irq_w))
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S96_4B)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( de_type3, de_2_state )
	/* basic machine hardware */
	MCFG_DECOCPU_TYPE3_ADD("decocpu",XTAL_8MHz / 2, ":maincpu")
	MCFG_DECOCPU_DISPLAY(READ8(de_2_state,display_r),WRITE8(de_2_state,alpha3_display_w))
	MCFG_DECOCPU_SOUNDLATCH(WRITE8(de_2_state,sound_w))
	MCFG_DECOCPU_SWITCH(READ8(de_2_state,switch_r),WRITE8(de_2_state,switch_w))
	MCFG_DECOCPU_LAMP(WRITE8(de_2_state,lamps_w))
	MCFG_MACHINE_RESET_OVERRIDE(de_2_state, de_2)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_de2a3)

	MCFG_FRAGMENT_ADD( genpin_audio )

	/* sound CPU */
	MCFG_CPU_ADD("audiocpu", M6809E, 8000000) // MC68B09E
	MCFG_CPU_PROGRAM_MAP(de_2_audio_map)

	MCFG_SPEAKER_STANDARD_MONO("bg")
	MCFG_YM2151_ADD("ym2151", 3580000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(de_2_state, ym2151_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)
	MCFG_SOUND_ADD("msm5205", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(de_2_state, msm5205_irq_w))
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S96_4B)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)
MACHINE_CONFIG_END


/*--------------------------------------------------------------------------------
/ Back To the Future - CPU Rev 3 /Alpha Type 3 - 32K Roms - 32/64K Sound Roms
/--------------------------------------------------------------------------------*/
ROM_START(bttf_a27)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bttfb5.2-7", 0x4000, 0x4000, CRC(24b53174) SHA1(00a5e47e70ce4244873980c946479f0bbc414f2e))
	ROM_LOAD("bttfc5.2-7", 0x8000, 0x8000, CRC(c4d85d7e) SHA1(88bb91f9ed50335fc402b68983b49319c7dd4e99))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("bttfsf7.rom", 0x8000, 0x8000, CRC(7673146e) SHA1(d6bd7cf39c78c8aff0b1a0b6cfd46a2a8ce9e086))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("bttfsf6.rom", 0x00000, 0x10000, CRC(468a8d9c) SHA1(713cf84cc5f0531e2e9f7aaa58ebeb53c28ba395))
	ROM_LOAD("bttfsf5.rom", 0x10000, 0x10000, CRC(37a6f6b8) SHA1(ebd603d36527a2af25dcda1fde5cdf9a34d1f9cd))
ROM_END

ROM_START(bttf_a20)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bttfb5.2-0", 0x0000, 0x8000, CRC(c0d4df6b) SHA1(647d0d0a5af04f4255a588da41a6cdb2cf522875))
	ROM_LOAD("bttfc5.2-0", 0x8000, 0x8000, CRC(a189a189) SHA1(9669653280c78c811931ea3944817c717f3b5b77))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("bttfsf7.rom", 0x8000, 0x8000, CRC(7673146e) SHA1(d6bd7cf39c78c8aff0b1a0b6cfd46a2a8ce9e086))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("bttfsf6.rom", 0x00000, 0x10000, CRC(468a8d9c) SHA1(713cf84cc5f0531e2e9f7aaa58ebeb53c28ba395))
	ROM_LOAD("bttfsf5.rom", 0x10000, 0x10000, CRC(37a6f6b8) SHA1(ebd603d36527a2af25dcda1fde5cdf9a34d1f9cd))
ROM_END

ROM_START(bttf_a21)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bktofutr.b5", 0x4000, 0x4000, CRC(a651f867) SHA1(99cff09a06a99abac505c7732bb4ed985f0946e4))
	ROM_LOAD("bktofutr.c5", 0x8000, 0x8000, CRC(118ae58e) SHA1(a17e4cc3c12ca770e6e0674cfbeb55482739f735))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("bttfsf7.rom", 0x8000, 0x8000, CRC(7673146e) SHA1(d6bd7cf39c78c8aff0b1a0b6cfd46a2a8ce9e086))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("bttfsf6.rom", 0x00000, 0x10000, CRC(468a8d9c) SHA1(713cf84cc5f0531e2e9f7aaa58ebeb53c28ba395))
	ROM_LOAD("bttfsf5.rom", 0x10000, 0x10000, CRC(37a6f6b8) SHA1(ebd603d36527a2af25dcda1fde5cdf9a34d1f9cd))
ROM_END

ROM_START(bttf_g27)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bttfb5g.2-7", 0x4000, 0x4000, CRC(5e3e3cfa) SHA1(2d489c48463c7d28614d56aa566ffbc745bf6a8b))
	ROM_LOAD("bttfc5g.2-7", 0x8000, 0x8000, CRC(31dec6d0) SHA1(b0f9323ace3f6d96790be7fe2df67b974c291a29))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("bttfsf7.rom", 0x8000, 0x8000, CRC(7673146e) SHA1(d6bd7cf39c78c8aff0b1a0b6cfd46a2a8ce9e086))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("bttfsf6.rom", 0x00000, 0x10000, CRC(468a8d9c) SHA1(713cf84cc5f0531e2e9f7aaa58ebeb53c28ba395))
	ROM_LOAD("bttfsf5.rom", 0x10000, 0x10000, CRC(37a6f6b8) SHA1(ebd603d36527a2af25dcda1fde5cdf9a34d1f9cd))
ROM_END

/*-------------------------------------------------------------------
/ Laser War - CPU Rev 1 /Alpha Type 1 - 32K ROM - 32/64K Sound Roms
/-------------------------------------------------------------------*/
ROM_START(lwar_a83)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lwar8-3.c5", 0x8000, 0x8000, CRC(eee158ee) SHA1(54db2342bdd15b16fee906dc65f183a957fd0012))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("lwar_e9.snd", 0x8000, 0x8000, CRC(9a6c834d) SHA1(c6e2c4658db4bd8dfcbb0351793837cdff30ba28))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("lwar_e6.snd", 0x00000, 0x10000, CRC(7307d795) SHA1(5d88b8d883a2f17ca9fa30c7e7ac29c9f236ac4d))
	ROM_LOAD("lwar_e7.snd", 0x10000, 0x10000, CRC(0285cff9) SHA1(2c5e3de649e419ec7944059f2a226aaf58fe2af5))
ROM_END

ROM_START(lwar_e90)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lwar9-0.e5", 0x8000, 0x8000, CRC(b596151f) SHA1(10dade79ded71625770ec7e21ea50b7aa64023d0))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("lwar_e9.snd", 0x8000, 0x8000, CRC(9a6c834d) SHA1(c6e2c4658db4bd8dfcbb0351793837cdff30ba28))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("lwar_e6.snd", 0x00000, 0x10000, CRC(7307d795) SHA1(5d88b8d883a2f17ca9fa30c7e7ac29c9f236ac4d))
	ROM_LOAD("lwar_e7.snd", 0x10000, 0x10000, CRC(0285cff9) SHA1(2c5e3de649e419ec7944059f2a226aaf58fe2af5))
ROM_END


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
// Display LED inputs are in the same order as for Monday Night Football, is this actually CPU type 2?
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

/*------------------------------------------------------------------------
/ The Simpsons - CPU Rev 3 /Alpha Type 3 16/32K Roms - 32/128K Sound Roms
/------------------------------------------------------------------------*/
ROM_START(simp_a27)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("simpb5.2-7", 0x4000, 0x4000, CRC(701c4a4b) SHA1(2a19e2340d119e8813df27a9455aefb599c20a61))
	ROM_LOAD("simpc5.2-7", 0x8000, 0x8000, CRC(400a98b2) SHA1(8d11063712dd718ff8badc29586c700208e7442c))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("simpf7.rom", 0x8000, 0x8000, CRC(a36febbc) SHA1(3b96e05f797dd0dc0d4d52544ed995d477991a9f))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("simpf6.rom", 0x00000, 0x20000, CRC(2eb32ed0) SHA1(e7bc3291cb88bf70010865f64496a3ca393257e7))
	ROM_LOAD("simpf5.rom", 0x20000, 0x20000, CRC(bd0671ae) SHA1(b116a23db956a3dd9fc138ec25af250885ba4ef5))
ROM_END

ROM_START(simp_a20)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("simpa2-0.b5", 0x4000, 0x4000, CRC(e67038d1) SHA1(f3eae2ed45caca97a1eb53d847366c52ea68bbee))
	ROM_LOAD("simpa2-0.c5", 0x8000, 0x8000, CRC(43662bc3) SHA1(d8171a5c083eb8bffa61353b74db6b3ebab96923))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("simpf7.rom", 0x8000, 0x8000, CRC(a36febbc) SHA1(3b96e05f797dd0dc0d4d52544ed995d477991a9f))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("simpf6.rom", 0x00000, 0x20000, CRC(2eb32ed0) SHA1(e7bc3291cb88bf70010865f64496a3ca393257e7))
	ROM_LOAD("simpf5.rom", 0x20000, 0x20000, CRC(bd0671ae) SHA1(b116a23db956a3dd9fc138ec25af250885ba4ef5))
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


GAME(1990,  bttf_a27,       0,          de_type3,   de_2, de_2_state,   de_2,   ROT0,   "Data East",    "Back To the Future (2.7)",                                     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  bttf_a20,       bttf_a27,   de_type3,   de_2, de_2_state,   de_2,   ROT0,   "Data East",    "Back To the Future (2.0)",                                     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  bttf_a21,       bttf_a27,   de_type3,   de_2, de_2_state,   de_2,   ROT0,   "Data East",    "Back To The Future (2.1)",                                     MACHINE_IS_SKELETON_MECHANICAL)
GAME(199?,  bttf_g27,       bttf_a27,   de_type3,   de_2, de_2_state,   de_2,   ROT0,   "Data East",    "Back To the Future (2.7 Germany)",                             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987,  lwar_a83,       0,          de_type1,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Laser War (8.3)",           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987,  lwar_e90,       lwar_a83,   de_type1,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Laser War (9.0 Europe)",    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1989,  mnfb_c27,       0,          de_type2_alpha3,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Monday Night Football (2.7, 50cts)",       MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  poto_a32,       0,          de_type2_alpha3,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "The Phantom of the Opera (3.2)",           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1989,  play_a24,       0,          de_type2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Playboy 35th Anniversary (2.4)",           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1989,  robo_a34,       0,          de_type3,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Robocop (3.4)",                            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1988,  ssvc_a26,       0,          de_type2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Secret Service (2.6)",                     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1988,  ssvc_b26,       ssvc_a26,   de_type2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Secret Service (2.6 alternate sound)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  simp_a27,       0,          de_type3,   de_2, de_2_state,   de_2,   ROT0,   "Data East",    "The Simpsons (2.7)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  simp_a20,       simp_a27,   de_type3,   de_2, de_2_state,   de_2,   ROT0,   "Data East",    "The Simpsons (2.0)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1988,  tmac_a24,       0,          de_type2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Time Machine (2.4)",                       MACHINE_IS_SKELETON_MECHANICAL)
GAME(1988,  tmac_a18,       tmac_a24,   de_type2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Time Machine (1.8)",                       MACHINE_IS_SKELETON_MECHANICAL)
GAME(1988,  torp_e21,       0,          de_type2,   de_2, de_2_state,   de_2,   ROT0,   "Data East",        "Torpedo Alley (2.1, Europe)",              MACHINE_IS_SKELETON_MECHANICAL)
