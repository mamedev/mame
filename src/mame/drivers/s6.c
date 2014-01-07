/***********************************************************************************

    Pinball
    Williams System 6

    After starting a game, nothing much works.


ToDo:
- Almost Everything


************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/hc55516.h"
#include "sound/dac.h"
#include "s6.lh"


class s6_state : public genpin_class
{
public:
	s6_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_audiocpu(*this, "audiocpu"),
	m_dac(*this, "dac"),
	m_hc55516(*this, "hc55516"),
	m_pias(*this, "pias"),
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
	DECLARE_WRITE8_MEMBER(lamp1_w);
	DECLARE_READ8_MEMBER(dips_r);
	DECLARE_WRITE8_MEMBER(sol0_w);
	DECLARE_WRITE8_MEMBER(sol1_w);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_WRITE8_MEMBER(pias_pb_w) { }; // dummy to stop log filling up
	DECLARE_READ_LINE_MEMBER(pia28_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia28_cb1_r);
	DECLARE_READ_LINE_MEMBER(pias_cb1_r);
	DECLARE_WRITE_LINE_MEMBER(pias_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pias_cb2_w);
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
	DECLARE_MACHINE_RESET(s6);
protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<dac_device> m_dac;
	required_device<hc55516_device> m_hc55516;
	required_device<pia6821_device> m_pias;
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

static ADDRESS_MAP_START( s6_main_map, AS_PROGRAM, 8, s6_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2200, 0x2203) AM_DEVREADWRITE("pia22", pia6821_device, read, write) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_DEVREADWRITE("pia24", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("pia28", pia6821_device, read, write) // display
	AM_RANGE(0xb000, 0x3003) AM_DEVREADWRITE("pia30", pia6821_device, read, write) // inputs
	AM_RANGE(0x6000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( s6_audio_map, AS_PROGRAM, 8, s6_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0400, 0x0403) AM_MIRROR(0x8000) AM_DEVREADWRITE("pias", pia6821_device, read, write)
	AM_RANGE(0xA000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( s6 )
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
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
	PORT_BIT( 0xbf, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Music") PORT_CODE(KEYCODE_9) PORT_TOGGLE

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, s6_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, s6_state, main_nmi, 1)
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

MACHINE_RESET_MEMBER( s6_state, s6 )
{
	m_t_c = 0;
}

INPUT_CHANGED_MEMBER( s6_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( s6_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER( s6_state::sol0_w )
{
	if (BIT(data, 4))
		m_samples->start(2, 5); // outhole
}

WRITE8_MEMBER( s6_state::sol1_w )
{
	m_sound_data = ioport("SND")->read(); // 0xff or 0xbf

	if (BIT(data, 0))
		m_sound_data &= 0xfe;

	if (BIT(data, 1))
		m_sound_data &= 0xfd;

	if (BIT(data, 2))
		m_sound_data &= 0xfb;

	if (BIT(data, 3))
		m_sound_data &= 0xf7;

	if (BIT(data, 4))
		m_sound_data &= 0xef;

	m_cb1 = ((m_sound_data & 0x9f) != 0x9f);

	m_pias->cb1_w(m_cb1);

	if (BIT(data, 5))
		m_samples->start(0, 6); // knocker
}

WRITE8_MEMBER( s6_state::lamp0_w )
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

WRITE8_MEMBER( s6_state::lamp1_w )
{
}

READ_LINE_MEMBER( s6_state::pia28_ca1_r )
{
	return BIT(ioport("DIAGS")->read(), 2); // advance button
}

READ_LINE_MEMBER( s6_state::pia28_cb1_r )
{
	return BIT(ioport("DIAGS")->read(), 3); // auto/manual switch
}

READ8_MEMBER( s6_state::dips_r )
{
	if (BIT(ioport("DIAGS")->read(), 4) )
	{
		switch (m_strobe)
		{
		case 0:
			return ioport("DSW0")->read() & 15;
		case 1:
			return ioport("DSW0")->read() << 4;
		case 2:
			return ioport("DSW1")->read() & 15;
		case 3:
			return ioport("DSW1")->read() << 4;
		}
	}
	return 0xff;
}

WRITE8_MEMBER( s6_state::dig0_w )
{
	m_strobe = data;
	m_data_ok = true;
}

WRITE8_MEMBER( s6_state::dig1_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // MC14558
	if (m_data_ok)
	{
		output_set_digit_value(m_strobe+16, patterns[data&15]);
		output_set_digit_value(m_strobe, patterns[data>>4]);
	}
	m_data_ok = false;
}

READ8_MEMBER( s6_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"X%X",m_kbdrow);
	return ~ioport(kbdrow)->read();
}

WRITE8_MEMBER( s6_state::switch_w )
{
	m_kbdrow = data;
}

READ_LINE_MEMBER( s6_state::pias_cb1_r )
{
	return m_cb1;
}

WRITE_LINE_MEMBER( s6_state::pias_cb2_w )
{
// speech clock
	m_hc55516->clock_w(state);
}

WRITE_LINE_MEMBER( s6_state::pias_ca2_w )
{
// speech data
	m_hc55516->digit_w(state);
}

READ8_MEMBER( s6_state::dac_r )
{
	return m_sound_data;
}

WRITE8_MEMBER( s6_state::dac_w )
{
	m_dac->write_unsigned8(data);
}

TIMER_DEVICE_CALLBACK_MEMBER( s6_state::irq)
{
	if (m_t_c > 0x70)
		m_maincpu->set_input_line(M6800_IRQ_LINE, ASSERT_LINE);
	else
		m_t_c++;
}

static MACHINE_CONFIG_START( s6, s6_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 3580000) // 6802 or 6808 could be used here
	MCFG_CPU_PROGRAM_MAP(s6_main_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", s6_state, irq, attotime::from_hz(250))
	MCFG_MACHINE_RESET_OVERRIDE(s6_state, s6)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_s6)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_DEVICE_ADD("pia22", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s6_state, sol0_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s6_state, sol1_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s6_state, pia22_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s6_state, pia22_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_DEVICE_ADD("pia24", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s6_state, lamp0_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s6_state, lamp1_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s6_state, pia24_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s6_state, pia24_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_DEVICE_ADD("pia28", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(s6_state, dips_r))
	MCFG_PIA_READCA1_HANDLER(READLINE(s6_state, pia28_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(s6_state, pia28_cb1_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s6_state, dig0_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s6_state, dig1_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s6_state, pia28_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s6_state, pia28_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_DEVICE_ADD("pia30", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(s6_state, switch_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s6_state, switch_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s6_state, pia30_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s6_state, pia30_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_NVRAM_ADD_1FILL("nvram")

	/* Add the soundcard */
	MCFG_CPU_ADD("audiocpu", M6802, 3580000)
	MCFG_CPU_PROGRAM_MAP(s6_audio_map)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SPEAKER_STANDARD_MONO("speech")
	MCFG_SOUND_ADD("hc55516", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speech", 0.50)

	MCFG_DEVICE_ADD("pias", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(s6_state, dac_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(s6_state, pias_cb1_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s6_state, dac_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s6_state, pias_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s6_state, pias_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s6_state, pias_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("audiocpu", m6802_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("audiocpu", m6802_cpu_device, irq_line))
MACHINE_CONFIG_END


/*--------------------------------
/ Laser Ball - Sys.6 (Game #493)
/-------------------------------*/
ROM_START(lzbal_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(9c5ffe2f) SHA1(f0db627abaeb8c023a3ccc75262e236c998a5d6f))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound2.716", 0xf800, 0x0800, CRC(c9103a68) SHA1(cc77af54fdb192f0b334d9d1028210618c3f1d95))
ROM_END

ROM_START(lzbal_t2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(9c5ffe2f) SHA1(f0db627abaeb8c023a3ccc75262e236c998a5d6f))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716", 0x7800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound2.716", 0xf800, 0x0800, CRC(c9103a68) SHA1(cc77af54fdb192f0b334d9d1028210618c3f1d95))
ROM_END


/*----------------------------
/ Blackout - Sys.6 (Game #495)
/---------------------------*/
ROM_START(blkou_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(4b407ae2) SHA1(46a2afcfc2d969c5acae18b57a678265257a6102))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("v_ic7.532", 0xb000, 0x1000, CRC(87864071) SHA1(d03c71efc0431f30a07c8194c0614c96fb683710))
	ROM_LOAD("v_ic5.532", 0xc000, 0x1000, CRC(046a96d8) SHA1(879127a88b3640bbb202c64cbf8678869c964177))
	ROM_LOAD("v_ic6.532", 0xd000, 0x1000, CRC(0104e5c4) SHA1(c073cb4bdea189085ae074e9c16872752b6ffba0))
	ROM_LOAD("sound2.716", 0xf800, 0x0800, CRC(c9103a68) SHA1(cc77af54fdb192f0b334d9d1028210618c3f1d95))
ROM_END

ROM_START(blkou_t1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(4b407ae2) SHA1(46a2afcfc2d969c5acae18b57a678265257a6102))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716", 0x7800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("v_ic7.532", 0xb000, 0x1000, CRC(87864071) SHA1(d03c71efc0431f30a07c8194c0614c96fb683710))
	ROM_LOAD("v_ic5.532", 0xc000, 0x1000, CRC(046a96d8) SHA1(879127a88b3640bbb202c64cbf8678869c964177))
	ROM_LOAD("v_ic6.532", 0xd000, 0x1000, CRC(0104e5c4) SHA1(c073cb4bdea189085ae074e9c16872752b6ffba0))
	ROM_LOAD("sound2.716", 0xf800, 0x0800, CRC(c9103a68) SHA1(cc77af54fdb192f0b334d9d1028210618c3f1d95))
ROM_END

ROM_START(blkou_f1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(4b407ae2) SHA1(46a2afcfc2d969c5acae18b57a678265257a6102))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("speech7f.532", 0xb000, 0x1000, CRC(bdc1b0b1) SHA1(c78f8653dfe3ec58722a8a17da7924e4a76cf692))
	ROM_LOAD("speech6f.532", 0xc000, 0x1000, CRC(9b7e4ae9) SHA1(137b5ec871162329cb7ca3a62da3193382223d8a))
	ROM_LOAD("speech5f.532", 0xd000, 0x1000, CRC(9040f34a) SHA1(529eae0b58f3300f2b9bdf40c5ca7f4b29425dff))
	ROM_LOAD("speech4f.532", 0xe000, 0x1000, CRC(29c4abde) SHA1(b3af7b8d0c2548f5c0bb240aa1dc5cc59bb2af9a))
	ROM_LOAD("sound2.716", 0xf800, 0x0800, CRC(c9103a68) SHA1(cc77af54fdb192f0b334d9d1028210618c3f1d95))
ROM_END

/*--------------------------
/ Gorgar - Sys.6 (Game #496)
/-------------------------*/
ROM_START(grgar_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(1c6f3e48) SHA1(ba5536e6fbcaf3709277fe27827d7f75c1889ba3))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("v_ic7.532", 0xb000, 0x1000, CRC(0b1879e3) SHA1(2c34a815f598b4413e9229e8eb1322ec9e7cc9d6))
	ROM_LOAD("v_ic5.532", 0xc000, 0x1000, CRC(0ceaef37) SHA1(33b5f5286b8588162d56dbc5c9a8ccb70d3b9090))
	ROM_LOAD("v_ic6.532", 0xd000, 0x1000, CRC(218290b9) SHA1(6afeff1413895489e92a4bb1c05f6de5773dbb6a))
	ROM_LOAD("sound2.716", 0xf800, 0x0800, CRC(c9103a68) SHA1(cc77af54fdb192f0b334d9d1028210618c3f1d95))
ROM_END

ROM_START(grgar_t1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamerom.716", 0x6000, 0x0800, CRC(1c6f3e48) SHA1(ba5536e6fbcaf3709277fe27827d7f75c1889ba3))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716", 0x7800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("v_ic7.532", 0xb000, 0x1000, CRC(0b1879e3) SHA1(2c34a815f598b4413e9229e8eb1322ec9e7cc9d6))
	ROM_LOAD("v_ic5.532", 0xc000, 0x1000, CRC(0ceaef37) SHA1(33b5f5286b8588162d56dbc5c9a8ccb70d3b9090))
	ROM_LOAD("v_ic6.532", 0xd000, 0x1000, CRC(218290b9) SHA1(6afeff1413895489e92a4bb1c05f6de5773dbb6a))
	ROM_LOAD("sound2.716", 0xf800, 0x0800, CRC(c9103a68) SHA1(cc77af54fdb192f0b334d9d1028210618c3f1d95))
ROM_END

/*-------------------------------
/ Firepower - Sys.6 (Game #497)
/------------------------------*/
ROM_START(frpwr_l6)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.474", 0x6200, 0x0200, CRC(f75ade1a) SHA1(a5572c5c721dbcb82988b709f4ef2119118e37c2))
	ROM_LOAD("prom3.474", 0x6400, 0x0200, CRC(242ec687) SHA1(c3366c898a66c78034687e6a6000193d52be4141))
	ROM_LOAD("gamerom.716", 0x6800, 0x0800, CRC(fdd3b983) SHA1(fb5d1eb01589311cf4b2ef16e25db03d40bca2f7))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))
	ROM_LOAD("prom1_6.474", 0x6000, 0x0200, CRC(af6eb0b9) SHA1(28f8366737e09ffd60cb5ea55a5734143cdb9663))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("v_ic7.532", 0xb000, 0x1000, CRC(94c5c0a7) SHA1(ff7c618d1666c1d5c3319fdd72c1af2846659290))
	ROM_LOAD("v_ic5.532", 0xc000, 0x1000, CRC(1737fdd2) SHA1(6307e0ae715e97294ee8aaaeb2e2bebb0cb590c2))
	ROM_LOAD("v_ic6.532", 0xd000, 0x1000, CRC(e56f7aa2) SHA1(cb922c3f4d91285dda4ccae880c2d798a82fd51b))
	ROM_LOAD("sound3.716", 0xf800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

ROM_START(frpwr_t6)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom1_6.474", 0x6000, 0x0200, CRC(af6eb0b9) SHA1(28f8366737e09ffd60cb5ea55a5734143cdb9663))
	ROM_LOAD("prom2.474", 0x6200, 0x0200, CRC(f75ade1a) SHA1(a5572c5c721dbcb82988b709f4ef2119118e37c2))
	ROM_LOAD("prom3.474", 0x6400, 0x0200, CRC(242ec687) SHA1(c3366c898a66c78034687e6a6000193d52be4141))
	ROM_LOAD("gamerom.716", 0x6800, 0x0800, CRC(fdd3b983) SHA1(fb5d1eb01589311cf4b2ef16e25db03d40bca2f7))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716", 0x7800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("v_ic7.532", 0xb000, 0x1000, CRC(94c5c0a7) SHA1(ff7c618d1666c1d5c3319fdd72c1af2846659290))
	ROM_LOAD("v_ic5.532", 0xc000, 0x1000, CRC(1737fdd2) SHA1(6307e0ae715e97294ee8aaaeb2e2bebb0cb590c2))
	ROM_LOAD("v_ic6.532", 0xd000, 0x1000, CRC(e56f7aa2) SHA1(cb922c3f4d91285dda4ccae880c2d798a82fd51b))
	ROM_LOAD("sound3.716", 0xf800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

ROM_START(frpwr_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom1.474", 0x6000, 0x0200, CRC(fbb7299f) SHA1(0ae9dbdc6ed8315596bf755ece34691671dc8d44))
	ROM_LOAD("prom2.474", 0x6200, 0x0200, CRC(f75ade1a) SHA1(a5572c5c721dbcb82988b709f4ef2119118e37c2))
	ROM_LOAD("prom3.474", 0x6400, 0x0200, CRC(242ec687) SHA1(c3366c898a66c78034687e6a6000193d52be4141))
	ROM_LOAD("gamerom.716", 0x6800, 0x0800, CRC(fdd3b983) SHA1(fb5d1eb01589311cf4b2ef16e25db03d40bca2f7))
	ROM_LOAD("green1.716", 0x7000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716", 0x7800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("v_ic7.532", 0xb000, 0x1000, CRC(94c5c0a7) SHA1(ff7c618d1666c1d5c3319fdd72c1af2846659290))
	ROM_LOAD("v_ic5.532", 0xc000, 0x1000, CRC(1737fdd2) SHA1(6307e0ae715e97294ee8aaaeb2e2bebb0cb590c2))
	ROM_LOAD("v_ic6.532", 0xd000, 0x1000, CRC(e56f7aa2) SHA1(cb922c3f4d91285dda4ccae880c2d798a82fd51b))
	ROM_LOAD("sound3.716", 0xf800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END


GAME( 1979, lzbal_l2, 0,        s6, s6, driver_device, 0, ROT0, "Williams", "Laser Ball (L-2)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1980, lzbal_t2, lzbal_l2, s6, s6, driver_device, 0, ROT0, "Williams", "Laser Ball (T-2)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1979, blkou_l1, 0,        s6, s6, driver_device, 0, ROT0, "Williams", "Blackout (L-1)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1979, blkou_t1, blkou_l1, s6, s6, driver_device, 0, ROT0, "Williams", "Blackout (T-1)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1979, blkou_f1, blkou_l1, s6, s6, driver_device, 0, ROT0, "Williams", "Blackout (L-1, French Speech)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1979, grgar_l1, 0,        s6, s6, driver_device, 0, ROT0, "Williams", "Gorgar (L-1)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1979, grgar_t1, grgar_l1, s6, s6, driver_device, 0, ROT0, "Williams", "Gorgar (T-1)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1980, frpwr_l6, 0,        s6, s6, driver_device, 0, ROT0, "Williams", "Firepower (L-6)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1980, frpwr_t6, frpwr_l6, s6, s6, driver_device, 0, ROT0, "Williams", "Firepower (T-6)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
GAME( 1980, frpwr_l2, frpwr_l6, s6, s6, driver_device, 0, ROT0, "Williams", "Firepower (L-2)", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_NO_SOUND)
