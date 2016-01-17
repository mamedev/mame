// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

   PINBALL
   Williams System 3

   Typical of Williams hardware: Motorola 8-bit CPUs, and lots of PIAs.

   Schematic and PinMAME used as references.

   Written during October 2012 [Robbbert]

   When first used, the nvram gets initialised but is otherwise unusable. A reboot
   will get it going.

   By pressing numpad2, you can select a different set of sounds. This is switch SW2
   on the real board.

Each game has its own switches, you need to know the outhole and slam-tilt ones.
Note that T is also a tilt, but it may take 3 hits to activate it.

Game          Outhole   Tilt
------------------------------------
Hot Tip       A         S
Lucky Seven   M         =
World Cup     H         J
Contact       V         ,
Disco         N         Enter - =
Phoenix       Left      M
Pokerino      X         ,


ToDo:
- Mechanical


************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "s3.lh"


class s3_state : public genpin_class
{
public:
	s3_state(const machine_config &mconfig, device_type type, std::string tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_dac(*this, "dac")
		, m_pia22(*this, "pia22")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
		, m_pias(*this, "pias")
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
	DECLARE_READ_LINE_MEMBER(pia28_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia28_cb1_r);
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
	DECLARE_MACHINE_RESET(s3);
	DECLARE_MACHINE_RESET(s3a);
private:
	UINT8 m_t_c;
	UINT8 m_sound_data;
	UINT8 m_strobe;
	UINT8 m_kbdrow;
	bool m_data_ok;
	bool m_chimes;
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<dac_device> m_dac;
	required_device<pia6821_device> m_pia22;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
	optional_device<pia6821_device> m_pias;
};

static ADDRESS_MAP_START( s3_main_map, AS_PROGRAM, 8, s3_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2200, 0x2203) AM_DEVREADWRITE("pia22", pia6821_device, read, write) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_DEVREADWRITE("pia24", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("pia28", pia6821_device, read, write) // display
	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("pia30", pia6821_device, read, write) // inputs
	AM_RANGE(0x6000, 0x7fff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( s3_audio_map, AS_PROGRAM, 8, s3_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0400, 0x0403) AM_DEVREADWRITE("pias", pia6821_device, read, write) // sounds
	AM_RANGE(0x0800, 0x0fff) AM_ROM AM_REGION("audioroms", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( s3 )
	PORT_START("X0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_TILT ) // 3 touches before it tilts
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA) // 1 touch tilt

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("X20")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("X40")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("X80")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SND")
	PORT_BIT( 0xbf, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Music") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s3_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_4_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s3_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Manual/Auto") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("DS1")
	PORT_DIPNAME( 0xf0, 0xf0, "Data units" )
	PORT_DIPSETTING(    0xf0, "0" )
	PORT_DIPSETTING(    0x70, "1" )
	PORT_DIPSETTING(    0xb0, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0xd0, "4" )
	PORT_DIPSETTING(    0x50, "5" )
	PORT_DIPSETTING(    0x90, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0xe0, "8" )
	PORT_DIPSETTING(    0x60, "9" )
	PORT_DIPNAME( 0x0f, 0x0f, "Data tens" )
	PORT_DIPSETTING(    0x0f, "0" )
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x0b, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x0d, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x09, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x0e, "8" )
	PORT_DIPSETTING(    0x06, "9" )

	PORT_START("DS2")
	PORT_DIPNAME( 0xff, 0xff, "Function" )
	PORT_DIPSETTING(    0xff, "0" )
	PORT_DIPSETTING(    0x7f, "1" )
	PORT_DIPSETTING(    0xbf, "2" )
	PORT_DIPSETTING(    0x3f, "3" )
	PORT_DIPSETTING(    0xdf, "4" )
	PORT_DIPSETTING(    0x5f, "5" )
	PORT_DIPSETTING(    0x9f, "6" )
	PORT_DIPSETTING(    0x1f, "7" )
	PORT_DIPSETTING(    0xef, "8" )
	PORT_DIPSETTING(    0x6f, "9" )
	PORT_DIPSETTING(    0xaf, "10" )
	PORT_DIPSETTING(    0x2f, "11" )
	PORT_DIPSETTING(    0xcf, "12" )
	PORT_DIPSETTING(    0x4f, "13" )
	PORT_DIPSETTING(    0x8f, "14" )
	PORT_DIPSETTING(    0x0f, "15" )
	PORT_DIPSETTING(    0xf7, "16" )
	PORT_DIPSETTING(    0x77, "17" )
	PORT_DIPSETTING(    0xb7, "18" )
	PORT_DIPSETTING(    0x37, "19" )
	PORT_DIPSETTING(    0xd7, "20" )
	PORT_DIPSETTING(    0x57, "21" )
	PORT_DIPSETTING(    0x97, "22" )
	PORT_DIPSETTING(    0x17, "23" )
	PORT_DIPSETTING(    0xe7, "24" )
	PORT_DIPSETTING(    0x67, "25" )
	PORT_DIPSETTING(    0xa7, "26" )
	PORT_DIPSETTING(    0x27, "27" )
	PORT_DIPSETTING(    0xc7, "28" )
	PORT_DIPSETTING(    0x47, "29" )
	PORT_DIPSETTING(    0x87, "30" )
	PORT_DIPSETTING(    0x07, "31" )
INPUT_PORTS_END

MACHINE_RESET_MEMBER( s3_state, s3 )
{
	m_t_c = 0;
	m_chimes = 1;
}

MACHINE_RESET_MEMBER( s3_state, s3a )
{
	m_t_c = 0;
	m_chimes = 0;
}

INPUT_CHANGED_MEMBER( s3_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( s3_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if ((newval==CLEAR_LINE) && !m_chimes)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER( s3_state::sol0_w )
{
	if (BIT(data, 4))
		m_samples->start(5, 5); // outhole
}

WRITE8_MEMBER( s3_state::sol1_w )
{
	if (m_chimes)
	{
		if (BIT(data, 0))
			m_samples->start(1, 1); // 10 chime

		if (BIT(data, 1))
			m_samples->start(2, 2); // 100 chime

		if (BIT(data, 2))
			m_samples->start(3, 3); // 1000 chime

		if (BIT(data, 3))
			m_samples->start(4, 4); // 10k chime
	}
	else
	{
		UINT8 sound_data = ioport("SND")->read(); // 0xff or 0xbf
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

		bool cb1 = ((sound_data & 0xbf) != 0xbf);

		if (cb1)
			m_sound_data = sound_data;

		m_pias->cb1_w(cb1);
	}

	if (BIT(data, 5))
		m_samples->start(0, 6); // knocker
}

WRITE8_MEMBER( s3_state::lamp0_w )
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

WRITE8_MEMBER( s3_state::lamp1_w )
{
}

READ_LINE_MEMBER( s3_state::pia28_ca1_r )
{
	return BIT(ioport("DIAGS")->read(), 2); // advance button
}

READ_LINE_MEMBER( s3_state::pia28_cb1_r )
{
	return BIT(ioport("DIAGS")->read(), 3); // auto/manual switch
}

READ8_MEMBER( s3_state::dips_r )
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

WRITE8_MEMBER( s3_state::dig0_w )
{
	m_strobe = data & 15;
	m_data_ok = true;
	output().set_value("led0", !BIT(data, 4));
	output().set_value("led1", !BIT(data, 5));
}

WRITE8_MEMBER( s3_state::dig1_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // MC14558
	if (m_data_ok)
	{
		output().set_digit_value(m_strobe+16, patterns[data&15]);
		output().set_digit_value(m_strobe, patterns[data>>4]);
	}
	m_data_ok = false;
}

READ8_MEMBER( s3_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"X%X",m_kbdrow);
	return ioport(kbdrow)->read();
}

WRITE8_MEMBER( s3_state::switch_w )
{
	m_kbdrow = data;
}

READ8_MEMBER( s3_state::dac_r )
{
	return m_sound_data;
}

TIMER_DEVICE_CALLBACK_MEMBER( s3_state::irq )
{
	if (m_t_c > 0x70)
		m_maincpu->set_input_line(M6800_IRQ_LINE, ASSERT_LINE);
	else
		m_t_c++;
}

static MACHINE_CONFIG_START( s3, s3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 3580000)
	MCFG_CPU_PROGRAM_MAP(s3_main_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", s3_state, irq, attotime::from_hz(250))
	MCFG_MACHINE_RESET_OVERRIDE(s3_state, s3)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_s3)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_DEVICE_ADD("pia22", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s3_state, sol0_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s3_state, sol1_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s3_state, pia22_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s3_state, pia22_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_DEVICE_ADD("pia24", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s3_state, lamp0_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s3_state, lamp1_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s3_state, pia24_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s3_state, pia24_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_DEVICE_ADD("pia28", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(s3_state, dips_r))
	MCFG_PIA_READCA1_HANDLER(READLINE(s3_state, pia28_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(s3_state, pia28_cb1_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s3_state, dig0_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s3_state, dig1_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s3_state, pia28_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s3_state, pia28_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_DEVICE_ADD("pia30", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(s3_state, switch_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s3_state, switch_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s3_state, pia30_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s3_state, pia30_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( s3a, s3 )
	/* Add the soundcard */
	MCFG_CPU_ADD("audiocpu", M6802, 3580000)
	MCFG_CPU_PROGRAM_MAP(s3_audio_map)
	MCFG_MACHINE_RESET_OVERRIDE(s3_state, s3a)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_DEVICE_ADD("pias", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(s3_state, dac_r))
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("dac", dac_device, write_unsigned8))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("audiocpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("audiocpu", m6800_cpu_device, irq_line))
MACHINE_CONFIG_END


//***************************************** SYSTEM 3 ******************************************************


/*----------------------------
/ Hot Tip - Sys.3 (Game #477) - No Sound board
/----------------------------*/
ROM_START(httip_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(b1d4fd9b) SHA1(e55ecf1328a55979c4cf8f3fb4e6761747e0abc4))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716",   0x1800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))
ROM_END

/*---------------------------------
/ Lucky Seven - Sys.3 (Game #480) - No Sound board
/---------------------------------*/
ROM_START(lucky_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(7cfbd4c7) SHA1(825e2245fd1615e932973f5e2b5ed5f2da9309e7))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716",   0x1800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))
ROM_END

/*-------------------------------------
/ World Cup Soccer - Sys.3 (Game #481)
/-------------------------------------*/
ROM_START(wldcp_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(c8071956) SHA1(0452aaf2ec1bcc5717fe52a6c541d79402bebb17))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2wc.716", 0x1800, 0x0800, CRC(618d15b5) SHA1(527387893eeb2cd4aa563a4cfb1948a15d2ed741))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*-------------------------------------
/ Contact - Sys.3 (Game #482)
/-------------------------------------*/
ROM_START(cntct_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(35359b60) SHA1(ab4c3328d93bdb4c952090b327c91b0ded36152c))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716",   0x1800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*-------------------------------------
/ Disco Fever - Sys.3 (Game #483)
/-------------------------------------*/
ROM_START(disco_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(831d8adb) SHA1(99a9c3d5c8cbcdf3bb9c210ad9d05c34905b272e))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716",   0x1800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*--------------------------------
/ Phoenix - Sys.4 (Game #485)
/-------------------------------*/
ROM_START(phnix_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(3aba6eac) SHA1(3a9f669216b3214bc42a1501aa2b10cfbcc36315))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716",   0x1800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*--------------------------------
/ Pokerino - Sys.4 (Game #488)
/-------------------------------*/
ROM_START(pkrno_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(9b4d01a8) SHA1(1bd51745f38381ffc66fde4b28b76aab33b573ca))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716",   0x1800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

GAME( 1977, httip_l1, 0, s3,  s3, driver_device, 0, ROT0, "Williams", "Hot Tip (L-1)", MACHINE_MECHANICAL )
GAME( 1977, lucky_l1, 0, s3,  s3, driver_device, 0, ROT0, "Williams", "Lucky Seven (L-1)", MACHINE_MECHANICAL )
GAME( 1978, wldcp_l1, 0, s3a, s3, driver_device, 0, ROT0, "Williams", "World Cup Soccer (L-1)", MACHINE_MECHANICAL )
GAME( 1978, cntct_l1, 0, s3a, s3, driver_device, 0, ROT0, "Williams", "Contact (L-1)", MACHINE_MECHANICAL )
GAME( 1978, disco_l1, 0, s3a, s3, driver_device, 0, ROT0, "Williams", "Disco Fever (L-1)", MACHINE_MECHANICAL )
GAME( 1978, phnix_l1, 0, s3a, s3, driver_device, 0, ROT0, "Williams", "Phoenix (L-1)", MACHINE_MECHANICAL )
GAME( 1978, pkrno_l1, 0, s3a, s3, driver_device, 0, ROT0, "Williams", "Pokerino (L-1)", MACHINE_MECHANICAL )
