/***********************************************************************************

    Pinball
    Williams System 8

    Only 2 games used this system.
    The very first time run, the display will show the model number (526 or 543).
    Press F3 to clear this, then follow instructions below.

    - Pennant Fever, which isn't a true pinball, it is a baseball game where you aim
      for targets at the top of the playfield, and the players advance towards a
      home run. There are no bumpers or other 'usual' pinball items. 1 or 2 players.
      How to play:
      - Insert coin (credits shows in innings)
      - Start game
      - Player 1 is 'Visitors'; optional Player 2 is 'Home'
      - Press one of L,B,C,V to hit the ball; or comma,period,slash for a home run;
        or (F then A) for a Strike; or N,Z for Out.
      - Wait for score to start flashing
      - Press another key, etc
      - When you have 3 strikes, you are Out
      - When you have 3 Outs, your Innings ends (other player gets a turn)
      - After 3 Innings, it's game over.
      - Match digit appears in Outs digit.

    - Still Crazy, a novelty game where the playfield is completely vertical. It has
      4 flippers and the idea is to get the ball up to the alcohol 'still' before
      the 'revenuers' do. The idea didn't catch on, and the game was not officially
      released. 1 player. The display shows Score and Batch. There is no credit
      display.

ToDo:
- Get Still Crazy coin-in to register.
  Workaround:
  - Start in debug mode, g to run the game, go to memory view and enter credits
    into location 0x738. (example: 90 gives 90 credits). Quit.
  - Start in non-debug mode. Press 1 to start, it works fine, apart from the
    knocker making a lot of noise. Keys to use: A then any key on that row.

************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "s8.lh"


class s8_state : public genpin_class
{
public:
	s8_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_audiocpu(*this, "audiocpu"),
	m_dac(*this, "dac"),
	m_pias(*this, "pias"),
	m_pia21(*this, "pia21"),
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
	DECLARE_WRITE8_MEMBER(sol2_w) { }; // solenoids 8-15
	DECLARE_WRITE8_MEMBER(sol3_w); // solenoids 0-7
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(dips_r);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_READ_LINE_MEMBER(pia21_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia28_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia28_cb1_r);
	DECLARE_WRITE_LINE_MEMBER(pias_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pias_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_cb2_w) { }; // enable solenoids
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { }; // dummy to stop error log filling up
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { }; // comma3&4
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { }; // comma1&2
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);
	DECLARE_MACHINE_RESET(s8);
protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<dac_device> m_dac;
	required_device<pia6821_device> m_pias;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
private:
	UINT8 m_t_c;
	UINT8 m_sound_data;
	UINT8 m_strobe;
	UINT8 m_kbdrow;
	bool m_data_ok;
};

static ADDRESS_MAP_START( s8_main_map, AS_PROGRAM, 8, s8_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2100, 0x2103) AM_DEVREADWRITE("pia21", pia6821_device, read, write) // sound+solenoids
	AM_RANGE(0x2200, 0x2200) AM_WRITE(sol3_w) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_DEVREADWRITE("pia24", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("pia28", pia6821_device, read, write) // display
	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("pia30", pia6821_device, read, write) // inputs
	AM_RANGE(0x5000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( s8_audio_map, AS_PROGRAM, 8, s8_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x2000, 0x2003) AM_DEVREADWRITE("pias", pia6821_device, read, write) // stillcra sounds
	AM_RANGE(0x4000, 0x4003) AM_DEVREADWRITE("pias", pia6821_device, read, write) // pfevr sounds
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( s8 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, s8_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, s8_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_9)
INPUT_PORTS_END

MACHINE_RESET_MEMBER( s8_state, s8 )
{
	m_t_c = 0;
}

INPUT_CHANGED_MEMBER( s8_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( s8_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER( s8_state::sol3_w )
{
	if (BIT(data, 1))
		m_samples->start(0, 6); // knocker
}

WRITE8_MEMBER( s8_state::sound_w )
{
	m_sound_data = data;
}

READ_LINE_MEMBER( s8_state::pia21_ca1_r )
{
// sound busy
	return 1;
}

WRITE_LINE_MEMBER( s8_state::pia21_ca2_w )
{
// sound ns
	m_pias->ca1_w(state);
}

WRITE8_MEMBER( s8_state::lamp0_w )
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

READ_LINE_MEMBER( s8_state::pia28_ca1_r )
{
	return BIT(ioport("DIAGS")->read(), 2); // advance button
}

READ_LINE_MEMBER( s8_state::pia28_cb1_r )
{
	return BIT(ioport("DIAGS")->read(), 3); // up/down switch
}

WRITE8_MEMBER( s8_state::dig0_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	data &= 0x7f;
	m_strobe = data & 15;
	m_data_ok = true;
	output_set_digit_value(60, patterns[data>>4]); // diag digit
}

WRITE8_MEMBER( s8_state::dig1_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // MC14558
	if (m_data_ok)
	{
		output_set_digit_value(m_strobe+16, patterns[data&15]);
		output_set_digit_value(m_strobe, patterns[data>>4]);
	}
	m_data_ok = false;
}

READ8_MEMBER( s8_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"X%X",m_kbdrow);
	return ~ioport(kbdrow)->read();
}

WRITE8_MEMBER( s8_state::switch_w )
{
	m_kbdrow = data;
}

WRITE_LINE_MEMBER( s8_state::pias_ca2_w )
{
// speech clock
}

WRITE_LINE_MEMBER( s8_state::pias_cb2_w )
{
// speech data
}

READ8_MEMBER( s8_state::dac_r )
{
	return m_sound_data;
}

WRITE8_MEMBER( s8_state::dac_w )
{
	m_dac->write_unsigned8(data);
}

TIMER_DEVICE_CALLBACK_MEMBER( s8_state::irq)
{
	if (m_t_c > 0x70)
		m_maincpu->set_input_line(M6800_IRQ_LINE, ASSERT_LINE);
	else
		m_t_c++;
}

static MACHINE_CONFIG_START( s8, s8_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6802, 4000000)
	MCFG_CPU_PROGRAM_MAP(s8_main_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", s8_state, irq, attotime::from_hz(250))
	MCFG_MACHINE_RESET_OVERRIDE(s8_state, s8)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_s8)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_DEVICE_ADD("pia21", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(s8_state, dac_r))
	MCFG_PIA_READCA1_HANDLER(READLINE(s8_state, pia21_ca1_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s8_state, sound_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s8_state, sol2_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s8_state, pia21_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s8_state, pia21_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6802_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6802_cpu_device, irq_line))

	MCFG_DEVICE_ADD("pia24", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s8_state, lamp0_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s8_state, lamp1_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s8_state, pia24_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6802_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6802_cpu_device, irq_line))

	MCFG_DEVICE_ADD("pia28", PIA6821, 0)
	MCFG_PIA_READCA1_HANDLER(READLINE(s8_state, pia28_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(s8_state, pia28_cb1_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s8_state, dig0_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s8_state, dig1_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s8_state, pia28_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s8_state, pia28_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6802_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6802_cpu_device, irq_line))

	MCFG_DEVICE_ADD("pia30", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(s8_state, switch_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s8_state, switch_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6802_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6802_cpu_device, irq_line))

	MCFG_NVRAM_ADD_1FILL("nvram")

	/* Add the soundcard */
	MCFG_CPU_ADD("audiocpu", M6808, 4000000)
	MCFG_CPU_PROGRAM_MAP(s8_audio_map)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_ADD("pias", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(s8_state, dac_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s8_state, sound_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s8_state, dac_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s8_state, pias_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s8_state, pias_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("audiocpu", m6808_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("audiocpu", m6808_cpu_device, irq_line))
MACHINE_CONFIG_END

/*------------------------------
/ Pennant Fever (#526) 05/1984
/-------------------------------*/
ROM_START(pfevr_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pf-rom1.u19", 0x5000, 0x1000, CRC(00be42bd) SHA1(72ca21c96e3ffa3c43499165f3339b669c8e94a5))
	ROM_LOAD("pf-rom2.u20", 0x6000, 0x2000, CRC(7b101534) SHA1(21e886d5872104d71bb528b9affb12230268597a))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("cpu_u49.128", 0xc000, 0x4000, CRC(b0161712) SHA1(5850f1f1f11e3ac9b9629cff2b26c4ad32436b55))
ROM_END

ROM_START(pfevr_p3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_u19.732", 0x5000, 0x1000, CRC(03796c6d) SHA1(38c95fcce9d0f357a74f041f0df006b9c6f6efc7))
	ROM_LOAD("cpu_u20.764", 0x6000, 0x2000, CRC(3a3acb39) SHA1(7844cc30a9486f718a556850fc9cef3be82f26b7))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("cpu_u49.128", 0xc000, 0x4000, CRC(b0161712) SHA1(5850f1f1f11e3ac9b9629cff2b26c4ad32436b55))
ROM_END

/*----------------------------
/ Still Crazy (#543) 06/1984
/-----------------------------*/
ROM_START(stillcra)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic20.bin", 0x6000, 0x2000, CRC(b0df42e6) SHA1(bb10268d7b820d1de0c20e1b79aba558badd072b) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("ic49.bin", 0xc000, 0x4000, CRC(bcc8ccc4) SHA1(2312f9cc4f5a2dadfbfa61d13c31bb5838adf152) )
ROM_END

GAME(1984,pfevr_l2, 0,        s8, s8, driver_device, 0, ROT0, "Williams", "Pennant Fever (L-2)", GAME_MECHANICAL)
GAME(1984,pfevr_p3, pfevr_l2, s8, s8, driver_device, 0, ROT0, "Williams", "Pennant Fever (P-3)", GAME_MECHANICAL)
GAME(1984,stillcra, 0,        s8, s8, driver_device, 0, ROT0, "Williams", "Still Crazy", GAME_MECHANICAL | GAME_NOT_WORKING)
