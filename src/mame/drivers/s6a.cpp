// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

  PINBALL
  Williams System 6a
  The only difference to System 6 is that the display is 7 digits instead of 6.

Diagnostic actions:
- You must be in game over mode. All buttons are in the number-pad. When you are
  finished, you must reboot.

- Setup: 6 must be in auto/up position. Press 5 to enter setup mode, press 6 to
         change direction.

- Tests: 6 must be in manual/down position. Press 5 twice and tests will begin.
         Press 5 and 6 together to get from test 1 to test 2. Press 6 to switch
         between auto/manual stepping.

- Auto Diag Test: Set Dips to SW6. Press 4. Press 9. Press 5. Tests will begin.

- Other: Set Dips to SW7 or SW8. Press 4. Press 9.


Each game has its own switches, you need to know the outhole and slam-tilt ones.
Note that T is also a tilt, but it may take 3 hits to activate it.


Game          Outhole   Tilt
------------------------------------
Algar         X
Alien Poker   X         =

Alien Poker: wait for the background sound before attempting to score.

ToDo:
- Mechanical sounds


************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/hc55516.h"
#include "sound/dac.h"
#include "s6a.lh"


class s6a_state : public genpin_class
{
public:
	s6a_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_dac(*this, "dac")
		, m_hc55516(*this, "hc55516")
		, m_pias(*this, "pias")
		, m_pia22(*this, "pia22")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
	{ }

	DECLARE_READ8_MEMBER(dac_r);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(lamp0_w);
	DECLARE_WRITE8_MEMBER(lamp1_w);
	DECLARE_WRITE8_MEMBER(sol0_w);
	DECLARE_WRITE8_MEMBER(sol1_w);
	DECLARE_READ8_MEMBER(dips_r);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_WRITE_LINE_MEMBER(pia22_ca2_w) { }; //ST5
	DECLARE_WRITE_LINE_MEMBER(pia22_cb2_w) { }; //ST-solenoids enable
	DECLARE_WRITE_LINE_MEMBER(pia24_ca2_w) { }; //ST2
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { }; //ST1
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { }; //diag leds enable
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { }; //ST6
	DECLARE_WRITE_LINE_MEMBER(pia30_ca2_w) { }; //ST4
	DECLARE_WRITE_LINE_MEMBER(pia30_cb2_w) { }; //ST3
	DECLARE_WRITE_LINE_MEMBER(pia_irq);
	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);
	DECLARE_MACHINE_RESET(s6a);
	DECLARE_DRIVER_INIT(s6a);
private:
	UINT8 m_sound_data;
	UINT8 m_strobe;
	UINT8 m_kbdrow;
	bool m_data_ok;
	emu_timer* m_irq_timer;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	static const device_timer_id TIMER_IRQ = 0;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<dac_device> m_dac;
	required_device<hc55516_device> m_hc55516;
	required_device<pia6821_device> m_pias;
	required_device<pia6821_device> m_pia22;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
};

static ADDRESS_MAP_START( s6a_main_map, AS_PROGRAM, 8, s6a_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2200, 0x2203) AM_DEVREADWRITE("pia22", pia6821_device, read, write) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_DEVREADWRITE("pia24", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("pia28", pia6821_device, read, write) // display
	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("pia30", pia6821_device, read, write) // inputs
	AM_RANGE(0x6000, 0x7fff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( s6a_audio_map, AS_PROGRAM, 8, s6a_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0400, 0x0403) AM_MIRROR(0x8000) AM_DEVREADWRITE("pias", pia6821_device, read, write)
	AM_RANGE(0xb000, 0xffff) AM_ROM AM_REGION("audioroms", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( s6a )
	PORT_START("X0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
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
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Speech") PORT_CODE(KEYCODE_3_PAD) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Music") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s6a_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_4_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s6a_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Manual/Auto") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("DS1") // DS1 only 3 switches do anything
	PORT_DIPNAME( 0x70, 0x70, "Diagnostic" )
	PORT_DIPSETTING(    0x70, "Off" )
	PORT_DIPSETTING(    0x60, "SW8 - Zero Audit Tables" )
	PORT_DIPSETTING(    0x50, "SW7 - Reset to Defaults" )
	PORT_DIPSETTING(    0x30, "SW6 - Auto Diagnostic Test" )
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DS2") // DS2 switches exist but do nothing
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( s6a_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( s6a_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER( s6a_state::sol0_w )
{
	if (BIT(data, 4))
		m_samples->start(2, 5); // outhole
}

WRITE8_MEMBER( s6a_state::sol1_w )
{
	UINT8 sound_data = ioport("SND")->read();
	if (BIT(data, 0))
		sound_data &= 0xfe;

	if (BIT(data, 1))
		sound_data &= 0xfd;

	if (BIT(data, 2))
		sound_data &= 0xfb;

	if (BIT(data, 3))
		sound_data &= 0xf7;

	if (BIT(data, 4))
		sound_data &= 0xef;

	bool cb1 = ((sound_data & 0x9f) != 0x9f);

	if (cb1)
		m_sound_data = sound_data;

	m_pias->cb1_w(cb1);

	if (BIT(data, 5))
		m_samples->start(0, 6); // knocker
}

WRITE8_MEMBER( s6a_state::lamp0_w )
{
}

WRITE8_MEMBER( s6a_state::lamp1_w )
{
}

READ8_MEMBER( s6a_state::dips_r )
{
	if (BIT(ioport("DIAGS")->read(), 4) )
	{
		switch (m_strobe)
		{
		case 0:
			return ioport("DS2")->read();
		case 1:
			return ioport("DS2")->read() << 4;
		case 2:
			return ioport("DS1")->read();
		case 3:
			return ioport("DS1")->read() << 4;
		}
	}
	return 0xff;
}

WRITE8_MEMBER( s6a_state::dig0_w )
{
	m_strobe = data & 15;
	m_data_ok = true;
	output_set_value("led0", !BIT(data, 4));
	output_set_value("led1", !BIT(data, 5));
}

WRITE8_MEMBER( s6a_state::dig1_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // MC14558
	if (m_data_ok)
	{
		output_set_digit_value(m_strobe+16, patterns[data&15]);
		output_set_digit_value(m_strobe, patterns[data>>4]);
	}
	m_data_ok = false;
}

READ8_MEMBER( s6a_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"X%X",m_kbdrow);
	return ioport(kbdrow)->read() ^ 0xff;
}

WRITE8_MEMBER( s6a_state::switch_w )
{
	m_kbdrow = data;
}

READ8_MEMBER( s6a_state::dac_r )
{
	return m_sound_data;
}

WRITE_LINE_MEMBER( s6a_state::pia_irq )
{
	if(state == CLEAR_LINE)
	{
		// restart IRQ timer
		m_irq_timer->adjust(attotime::from_ticks(980,3580000/4),1);
	}
	else
	{
		// disable IRQ timer while other IRQs are being handled
		// (counter is reset every 32 cycles while a PIA IRQ is handled)
		m_irq_timer->adjust(attotime::zero);
	}
}

void s6a_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_IRQ:
		if(param == 1)
		{
			m_maincpu->set_input_line(M6800_IRQ_LINE,ASSERT_LINE);
			m_irq_timer->adjust(attotime::from_ticks(32,3580000/4),0);
			m_pia28->ca1_w(BIT(ioport("DIAGS")->read(), 2));  // Advance
			m_pia28->cb1_w(BIT(ioport("DIAGS")->read(), 3));  // Up/Down
		}
		else
		{
			m_maincpu->set_input_line(M6800_IRQ_LINE,CLEAR_LINE);
			m_irq_timer->adjust(attotime::from_ticks(980,3580000/4),1);
			m_pia28->ca1_w(1);
			m_pia28->cb1_w(1);
		}
		break;
	}
}

MACHINE_RESET_MEMBER( s6a_state, s6a )
{
}

DRIVER_INIT_MEMBER( s6a_state, s6a )
{
	m_irq_timer = timer_alloc(TIMER_IRQ);
	m_irq_timer->adjust(attotime::from_ticks(980,3580000/4),1);
}

static MACHINE_CONFIG_START( s6a, s6a_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6808, 3580000)
	MCFG_CPU_PROGRAM_MAP(s6a_main_map)
	MCFG_MACHINE_RESET_OVERRIDE(s6a_state, s6a)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_s6a)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_DEVICE_ADD("pia22", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s6a_state, sol0_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s6a_state, sol1_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s6a_state, pia22_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s6a_state, pia22_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(s6a_state, pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(s6a_state, pia_irq))

	MCFG_DEVICE_ADD("pia24", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s6a_state, lamp0_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s6a_state, lamp1_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s6a_state, pia24_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s6a_state, pia24_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(s6a_state, pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(s6a_state, pia_irq))

	MCFG_DEVICE_ADD("pia28", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(s6a_state, dips_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s6a_state, dig0_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s6a_state, dig1_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s6a_state, pia28_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s6a_state, pia28_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(s6a_state, pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(s6a_state, pia_irq))

	MCFG_DEVICE_ADD("pia30", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(s6a_state, switch_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s6a_state, switch_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s6a_state, pia30_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s6a_state, pia30_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(s6a_state, pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(s6a_state, pia_irq))

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Add the soundcard */
	MCFG_CPU_ADD("audiocpu", M6802, 3580000)
	MCFG_CPU_PROGRAM_MAP(s6a_audio_map)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SPEAKER_STANDARD_MONO("speech")
	MCFG_SOUND_ADD("hc55516", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speech", 1.00)

	MCFG_DEVICE_ADD("pias", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(s6a_state, dac_r))
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("dac", dac_device, write_unsigned8))
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE("hc55516", hc55516_device, digit_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE("hc55516", hc55516_device, clock_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("audiocpu", m6802_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("audiocpu", m6802_cpu_device, irq_line))
MACHINE_CONFIG_END


/*--------------------------
/ Algar - Sys.6 (Game #499)
/-------------------------*/
ROM_START(algar_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716", 0x0000, 0x0800, CRC(6711da23) SHA1(80a46f5a2630977bc1c6e17466e8865083eb9a18))
	ROM_LOAD("green1.716",  0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",  0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x5000, "audioroms", 0)
	ROM_LOAD("sound4.716",  0x4800, 0x0800, CRC(67ea12e7) SHA1(f81e97183442736d5766a7e5e074bc6539e8ced0))
ROM_END

/*-------------------------------
/ Alien Poker - Sys.6 (Game #501)
/-------------------------------*/
ROM_START(alpok_l6)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom6.716", 0x0000, 0x0800, CRC(20538a4a) SHA1(6cdd6b7ded76b3cbd954d371e126e1bbd95a6219))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x5000, "audioroms", 0)
	ROM_LOAD("v_ic7.532",    0x0000, 0x1000, CRC(a66c7ca6) SHA1(6e90081f853fcf66bfeac0a8ee1c762b3760b90b))
	ROM_LOAD("v_ic5.532",    0x1000, 0x1000, CRC(f16a237a) SHA1(a904138fad5cbc19946bcf0de824e27537dcd621))
	ROM_LOAD("v_ic6.532",    0x2000, 0x1000, CRC(15a3cc85) SHA1(86002ac78189415ae912e8bc23c92b3b67610d87))
	ROM_LOAD("sound3.716",   0x4800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

ROM_START(alpok_l2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(79c07603) SHA1(526a45b139394e475fc052636e98d880a8908168))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x5000, "audioroms", 0)
	ROM_LOAD("v_ic7.532",    0x0000, 0x1000, CRC(a66c7ca6) SHA1(6e90081f853fcf66bfeac0a8ee1c762b3760b90b))
	ROM_LOAD("v_ic5.532",    0x1000, 0x1000, CRC(f16a237a) SHA1(a904138fad5cbc19946bcf0de824e27537dcd621))
	ROM_LOAD("v_ic6.532",    0x2000, 0x1000, CRC(15a3cc85) SHA1(86002ac78189415ae912e8bc23c92b3b67610d87))
	ROM_LOAD("sound3.716",   0x4800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

ROM_START(alpok_f6)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom6.716", 0x0000, 0x0800, CRC(20538a4a) SHA1(6cdd6b7ded76b3cbd954d371e126e1bbd95a6219))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x5000, "audioroms", 0)
	ROM_LOAD("5t5014fr.dat", 0x0000, 0x1000, CRC(1d961517) SHA1(c71ee324becfc8cdbecabd1e64b11b5a39ff2483))
	ROM_LOAD("5t5015fr.dat", 0x1000, 0x1000, CRC(8d065f80) SHA1(0ab22c9b20ab6fe41abab620435ad03652db7a8e))
	ROM_LOAD("5t5016fr.dat", 0x2000, 0x1000, CRC(0ddf91e9) SHA1(48f5fdfc0c5a66dd318fecb7c90e5f5a684a3876))
	ROM_LOAD("5t5017fr.dat", 0x3000, 0x1000, CRC(7e546dc1) SHA1(58f8286403978b0d929987189089881d754a9a83))
	ROM_LOAD("sound3.716",   0x4800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END


GAME(1980,algar_l1, 0,       s6a, s6a, s6a_state, s6a, ROT0, "Williams", "Algar (L-1)", MACHINE_MECHANICAL )
GAME(1980,alpok_l6, 0,       s6a, s6a, s6a_state, s6a, ROT0, "Williams", "Alien Poker (L-6)", MACHINE_MECHANICAL )
GAME(1980,alpok_l2, alpok_l6,s6a, s6a, s6a_state, s6a, ROT0, "Williams", "Alien Poker (L-2)", MACHINE_MECHANICAL )
GAME(1980,alpok_f6, alpok_l6,s6a, s6a, s6a_state, s6a, ROT0, "Williams", "Alien Poker (L-6 French speech)", MACHINE_MECHANICAL )
