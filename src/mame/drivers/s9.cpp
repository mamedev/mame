// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************

  PINBALL
  Williams System 9

When first started, it shows the game number and stops. Press F3 to reboot, then it works.

Rat Race is played in a one-player cocktail cabinet, the player uses a joystick
to tilt the board, to coax the ball into following lit arrows in a maze. After
a successful navigation, the maze changes to something else faster and harder.
It's almost an arcade game done mechanically. Obviously there is no way to emulate it
in its intended form. Probably would have been a nice game, but it never passed the
prototype stage. Currently it runs but the player display flashes randoms ones while
a sound is produced every couple of seconds.

Each game has its own switches, you need to know the outhole and slam-tilt ones.
Note that T is also a tilt, but it may take 3 hits to activate it.


Game              Outhole   Tilt         Notes
----------------------------------------------------------------------------------------
Sorcerer          X -       Y            To start, hold down X and minus, then press 1.
Space Shuttle     S D F     Right-shift  To start, hold down SDF, then press 1.
Comet             Y         Right-shift
Rat Race                                 Not working

ToDo:
- Mechanical sounds

*****************************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/genpin.h"
#include "sound/dac.h"
#include "sound/hc55516.h"
#include "sound/volt_reg.h"
#include "speaker.h"

#include "s9.lh"

class s9_state : public genpin_class
{
public:
	s9_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_hc55516(*this, "hc55516")
		, m_pias(*this, "pias")
		, m_pia21(*this, "pia21")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void s9(machine_config &config);

	void init_s9();

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

private:
	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(lamp0_w) { };
	DECLARE_WRITE8_MEMBER(lamp1_w) { };
	DECLARE_WRITE8_MEMBER(sol2_w) { }; // solenoids 8-15
	DECLARE_WRITE8_MEMBER(sol3_w) { }; // solenoids 0-7
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_READ_LINE_MEMBER(pia21_ca1_r);
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_cb2_w) { }; // enable solenoids
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { }; // dummy to stop error log filling up
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { }; // comma3&4
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { }; // comma1&2
	DECLARE_WRITE_LINE_MEMBER(pia_irq);
	DECLARE_MACHINE_RESET(s9);

	void s9_audio_map(address_map &map);
	void s9_main_map(address_map &map);

	uint8_t m_sound_data;
	uint8_t m_strobe;
	uint8_t m_kbdrow;
	bool m_data_ok;
	emu_timer* m_irq_timer;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	static const device_timer_id TIMER_IRQ = 0;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<hc55516_device> m_hc55516;
	required_device<pia6821_device> m_pias;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
	output_finder<61> m_digits;
};

void s9_state::s9_main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2100, 0x2103).rw(m_pia21, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).w(FUNC(s9_state::sol3_w)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x4000, 0x7fff).rom().region("roms", 0);
}

void s9_state::s9_audio_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x2003).rw(m_pias, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xffff).rom().region("audioroms", 0);
}

static INPUT_PORTS_START( s9 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("X80")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s9_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_4_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s9_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( s9_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( s9_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

WRITE8_MEMBER( s9_state::sound_w )
{
	m_sound_data = data;
}

READ_LINE_MEMBER( s9_state::pia21_ca1_r )
{
// sound busy
	return 1;
}

WRITE_LINE_MEMBER( s9_state::pia21_ca2_w )
{
// sound ns
	m_pias->ca1_w(state);
}

WRITE8_MEMBER( s9_state::dig0_w )
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	data &= 0x7f;
	m_strobe = data & 15;
	m_data_ok = true;
	m_digits[60] = patterns[data>>4]; // diag digit
}

WRITE8_MEMBER( s9_state::dig1_w )
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // MC14558
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data&15];
		m_digits[m_strobe] = patterns[data>>4];
	}
	m_data_ok = false;
}

READ8_MEMBER( s9_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"X%X",m_kbdrow);
	return ioport(kbdrow)->read() ^ 0xff;
}

WRITE8_MEMBER( s9_state::switch_w )
{
	m_kbdrow = data;
}

READ8_MEMBER( s9_state::sound_r )
{
	return m_sound_data;
}

WRITE_LINE_MEMBER( s9_state::pia_irq )
{
	if(state == CLEAR_LINE)
	{
		// restart IRQ timer
		m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
	}
	else
	{
		// disable IRQ timer while other IRQs are being handled
		// (counter is reset every 32 cycles while a PIA IRQ is handled)
		m_irq_timer->adjust(attotime::zero);
	}
}

void s9_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_IRQ:
		if(param == 1)
		{
			m_maincpu->set_input_line(M6808_IRQ_LINE, ASSERT_LINE);
			m_irq_timer->adjust(attotime::from_ticks(32,1e6),0);
			m_pia28->ca1_w(BIT(ioport("DIAGS")->read(), 2));  // Advance
			m_pia28->cb1_w(BIT(ioport("DIAGS")->read(), 3));  // Up/Down
		}
		else
		{
			m_maincpu->set_input_line(M6808_IRQ_LINE, CLEAR_LINE);
			m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
			m_pia28->ca1_w(1);
			m_pia28->cb1_w(1);
		}
		break;
	}
}

MACHINE_RESET_MEMBER( s9_state, s9 )
{
}

void s9_state::init_s9()
{
	m_irq_timer = timer_alloc(TIMER_IRQ);
	m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
}

void s9_state::s9(machine_config &config)
{
	/* basic machine hardware */
	M6808(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &s9_state::s9_main_map);
	MCFG_MACHINE_RESET_OVERRIDE(s9_state, s9)

	/* Video */
	config.set_default_layout(layout_s9);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia21, 0);
	m_pia21->readpa_handler().set(FUNC(s9_state::sound_r));
	m_pia21->readca1_handler().set(FUNC(s9_state::pia21_ca1_r));
	m_pia21->writepa_handler().set(FUNC(s9_state::sound_w));
	m_pia21->writepb_handler().set(FUNC(s9_state::sol2_w));
	m_pia21->ca2_handler().set(FUNC(s9_state::pia21_ca2_w));
	m_pia21->cb2_handler().set(FUNC(s9_state::pia21_cb2_w));
	m_pia21->irqa_handler().set(FUNC(s9_state::pia_irq));
	m_pia21->irqb_handler().set(FUNC(s9_state::pia_irq));

	PIA6821(config, m_pia24, 0);
	m_pia24->writepa_handler().set(FUNC(s9_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s9_state::lamp1_w));
	m_pia24->cb2_handler().set(FUNC(s9_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(FUNC(s9_state::pia_irq));
	m_pia24->irqb_handler().set(FUNC(s9_state::pia_irq));

	PIA6821(config, m_pia28, 0);
	m_pia28->writepa_handler().set(FUNC(s9_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s9_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s9_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s9_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(FUNC(s9_state::pia_irq));
	m_pia28->irqb_handler().set(FUNC(s9_state::pia_irq));

	PIA6821(config, m_pia30, 0);
	m_pia30->readpa_handler().set(FUNC(s9_state::switch_r));
	m_pia30->writepb_handler().set(FUNC(s9_state::switch_w));
	m_pia30->irqa_handler().set(FUNC(s9_state::pia_irq));
	m_pia30->irqb_handler().set(FUNC(s9_state::pia_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Add the soundcard */
	M6808(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &s9_state::s9_audio_map);

	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

	SPEAKER(config, "speech").front_center();
	HC55516(config, m_hc55516, 0).add_route(ALL_OUTPUTS, "speech", 1.00);

	PIA6821(config, m_pias, 0);
	m_pias->readpa_handler().set(FUNC(s9_state::sound_r));
	m_pias->writepb_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pias->ca2_handler().set("hc55516", FUNC(hc55516_device::clock_w));
	m_pias->cb2_handler().set("hc55516", FUNC(hc55516_device::digit_w));
	m_pias->irqa_handler().set_inputline("audiocpu", M6808_IRQ_LINE);
	m_pias->irqa_handler().set_inputline("audiocpu", M6808_IRQ_LINE);
}

/*-----------------------------
/ Rat Race - Sys.9 (Game #527)- Prototype (displays as #500)
/-----------------------------*/
ROM_START(ratrc_l1)
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("ic20.532", 0x1000, 0x1000, CRC(0c5c7c09) SHA1(c93b39ba1460feee5850fcd3ca7cacb72c4c8ff3))
	ROM_LOAD("ic14.532", 0x2000, 0x1000, CRC(c6f4bcf4) SHA1(d71c86299139abe3dd376a324315a039be82875c))
	ROM_LOAD("ic17.532", 0x3000, 0x1000, CRC(0800c214) SHA1(3343c07fd550bb0759032628e01bb750135dab15))

	ROM_REGION(0x8000, "audioroms", 0)
	ROM_LOAD("b486.bin", 0x6000, 0x2000, CRC(c54b9402) SHA1(c56fc5f105fc2c1166e3b22bb09b72af79e0aec1))
ROM_END

/*-----------------------------
/ Sorcerer (S9) 03/85 (#532)
/------------------------------*/
ROM_START(sorcr_l1)
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("cpu_u19.732", 0x1000, 0x1000, CRC(88b6837d) SHA1(d26b06342741443406a72ba48a70e82df62bb26e))
	ROM_LOAD("cpu_u20.764", 0x2000, 0x2000, CRC(c235b692) SHA1(d3b97fad2d501c894570601b387933c7644f64e6))

	ROM_REGION(0x8000, "audioroms", 0)
	ROM_LOAD("spch_u7.732", 0x0000, 0x1000, CRC(bba9ed18) SHA1(8e37ba8cb6bbc1e0afeef230088beda4513adddb))
	ROM_LOAD("spch_u5.732", 0x1000, 0x1000, CRC(d48c68ad) SHA1(b1391b87519ad47be3dcce7f8581f871e6a3669f))
	ROM_LOAD("spch_u6.732", 0x2000, 0x1000, CRC(a5c54d47) SHA1(4e1206412ecf52ae61c9df2055e0715749a6325d))
	ROM_LOAD("spch_u4.732", 0x3000, 0x1000, CRC(0c81902d) SHA1(6d8f703327e5c73a321fc4aa3a67ce68fff82d70))
	ROM_LOAD("cpu_u49.128", 0x4000, 0x4000, CRC(a0bae1e4) SHA1(dc5172aa1d59191d4119da20757cb2c2469f8fe3))
ROM_END

ROM_START(sorcr_l2)
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("cpu_u19.l2",  0x1000, 0x1000, CRC(faf738db) SHA1(a3b3f4160dc837ddf5379e1edb0eafeefcc11e3d))
	ROM_LOAD("cpu_u20.l2",  0x2000, 0x2000, CRC(74fc8117) SHA1(c228c76ade670603f77bb324e6794ec6dd358285))

	ROM_REGION(0x8000, "audioroms", 0)
	ROM_LOAD("spch_u7.732", 0x0000, 0x1000, CRC(bba9ed18) SHA1(8e37ba8cb6bbc1e0afeef230088beda4513adddb))
	ROM_LOAD("spch_u5.732", 0x1000, 0x1000, CRC(d48c68ad) SHA1(b1391b87519ad47be3dcce7f8581f871e6a3669f))
	ROM_LOAD("spch_u6.732", 0x2000, 0x1000, CRC(a5c54d47) SHA1(4e1206412ecf52ae61c9df2055e0715749a6325d))
	ROM_LOAD("spch_u4.732", 0x3000, 0x1000, CRC(0c81902d) SHA1(6d8f703327e5c73a321fc4aa3a67ce68fff82d70))
	ROM_LOAD("cpu_u49.128", 0x4000, 0x4000, CRC(a0bae1e4) SHA1(dc5172aa1d59191d4119da20757cb2c2469f8fe3))
ROM_END

/*---------------------------------
/ Space Shuttle (S9) 12/84 (#535)
/----------------------------------*/
ROM_START(sshtl_l7)
	// Spanish licensed version by Stargame is identical to this set
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("cpu_u20.128", 0x0000, 0x4000, CRC(848ad54c) SHA1(4e4ce5fb970da37706472f94a27fd912e1ecb1a0))

	ROM_REGION(0x8000, "audioroms", 0)
	ROM_LOAD("spch_u5.732", 0x1000, 0x1000, CRC(13edd4e5) SHA1(46c4052c31ddc20bb87445636f8fe3b6f7bff856))
	ROM_LOAD("spch_u6.732", 0x2000, 0x1000, CRC(cf48b2e7) SHA1(fe55419a5d40b3a4e8c02a92746b25a075b8efd3))
	ROM_LOAD("spch_u4.732", 0x3000, 0x1000, CRC(b0d03c5e) SHA1(46b952f71a7ecc03e22e427875f6e16a9d124067))
	ROM_LOAD("cpu_u49.128", 0x4000, 0x4000, CRC(8050ae27) SHA1(e3f5e9398f61b075620ecd075617a8dac3c07d0e))
ROM_END

ROM_START(sshtl_l3)
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("cpu_u20.l3", 0x0000, 0x4000, CRC(dc5f08e0) SHA1(67869c1db4e1f49f38588978d4ed32fe7d62e2d6))

	ROM_REGION(0x8000, "audioroms", 0)
	ROM_LOAD("spch_u5.732", 0x1000, 0x1000, CRC(13edd4e5) SHA1(46c4052c31ddc20bb87445636f8fe3b6f7bff856))
	ROM_LOAD("spch_u6.732", 0x2000, 0x1000, CRC(cf48b2e7) SHA1(fe55419a5d40b3a4e8c02a92746b25a075b8efd3))
	ROM_LOAD("spch_u4.732", 0x3000, 0x1000, CRC(b0d03c5e) SHA1(46b952f71a7ecc03e22e427875f6e16a9d124067))
	ROM_LOAD("cpu_u49.128", 0x4000, 0x4000, CRC(8050ae27) SHA1(e3f5e9398f61b075620ecd075617a8dac3c07d0e))
ROM_END

/*-------------------------
/ Comet (S9) 06/85 (#540)
/--------------------------*/
ROM_START(comet_l4)
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("cpu_u20.128", 0x0000, 0x4000, CRC(36193600) SHA1(efdc44ef26c2def8f860a0296e27b2c3dac55ec8))

	ROM_REGION(0x8000, "audioroms", 0)
	ROM_LOAD("spch_u7.732", 0x0000, 0x1000, CRC(36545b22) SHA1(f4a026f3fa58dce81b439d76120a6769f4632955))
	ROM_LOAD("spch_u5.732", 0x1000, 0x1000, CRC(89f7ede5) SHA1(bbfbd991c9e005c2fa36d8458803b121f4933618))
	ROM_LOAD("spch_u6.732", 0x2000, 0x1000, CRC(6ba2aba6) SHA1(783b4e9b38db8677d91f86cb4805f0fa1ae8f856))
	ROM_LOAD("spch_u4.732", 0x3000, 0x1000, CRC(d0215c49) SHA1(4f0925a826199b6e8baa5e7fbff5cde9e31d505b))
	ROM_LOAD("cpu_u49.128", 0x4000, 0x4000, CRC(f1db0cbe) SHA1(59b7f36fb2003b90b288abeff56df62ce50f10c6))
ROM_END

ROM_START(comet_l5)
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("cpu_u20.l5",  0x0000, 0x4000, CRC(d153d9ab) SHA1(0b97591b8ba35207b1427900486d69078ae122bc))

	ROM_REGION(0x8000, "audioroms", 0)
	ROM_LOAD("spch_u7.732", 0x0000, 0x1000, CRC(36545b22) SHA1(f4a026f3fa58dce81b439d76120a6769f4632955))
	ROM_LOAD("spch_u5.732", 0x1000, 0x1000, CRC(89f7ede5) SHA1(bbfbd991c9e005c2fa36d8458803b121f4933618))
	ROM_LOAD("spch_u6.732", 0x2000, 0x1000, CRC(6ba2aba6) SHA1(783b4e9b38db8677d91f86cb4805f0fa1ae8f856))
	ROM_LOAD("spch_u4.732", 0x3000, 0x1000, CRC(d0215c49) SHA1(4f0925a826199b6e8baa5e7fbff5cde9e31d505b))
	ROM_LOAD("cpu_u49.128", 0x4000, 0x4000, CRC(f1db0cbe) SHA1(59b7f36fb2003b90b288abeff56df62ce50f10c6))
ROM_END

/*--------------------------------
/ Strike Zone (Shuffle) (#916)
/---------------------------------*/
ROM_START(szone_l5)
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("sz_u19r5.732", 0x1000, 0x1000, CRC(c79c46cb) SHA1(422ba74ae67bebbe02f85a9a8df0e3072f3cebc0))
	ROM_LOAD("sz_u20r5.764", 0x2000, 0x2000, CRC(9b5b3be2) SHA1(fce051a60b6eecd9bc07273892b14046b251b372))

	ROM_REGION(0x8000, "audioroms", 0)
	ROM_LOAD("szs_u49.128",  0x4000, 0x4000, CRC(144c3c07) SHA1(57be6f336f200079cd698b13f8fa4755cf694274))
ROM_END

ROM_START(szone_l2)
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("sz_u19r2.732", 0x1000, 0x1000, CRC(c0e4238b) SHA1(eae60ccd5b5001671cd6d2685fd588494d052d1e))
	ROM_LOAD("sz_u20r2.764", 0x2000, 0x2000, CRC(91c08137) SHA1(86da08f346f85810fceceaa7b9824ab76a68da54))

	ROM_REGION(0x8000, "audioroms", 0)
	ROM_LOAD("szs_u49.128",  0x4000, 0x4000, CRC(144c3c07) SHA1(57be6f336f200079cd698b13f8fa4755cf694274))
ROM_END

/*------------------------------
/ Alley Cats (Shuffle) (#918)
/-------------------------------*/
ROM_START(alcat_l7)
	ROM_REGION(0x4000, "roms", 0) // system 9 mainboard
	ROM_LOAD("u26_rev7.rom", 0x1000, 0x1000, CRC(4d274dd3) SHA1(80d72bd0f85ce2cac04f6d9f59dc1fcccc86d402))
	ROM_LOAD("u27_rev7.rom", 0x2000, 0x2000, CRC(9c7faf8a) SHA1(dc1a561948b9a303f7924d7bebcd972db766827b))

	ROM_REGION(0x20000, "audiocpu", 0) // System 11 soundboard
	ROM_LOAD("acs_u21.bin", 0x18000, 0x8000, CRC(c54cd329) SHA1(4b86b10e60a30c4de5d97129074f5657447be676))
	ROM_LOAD("acs_u22.bin", 0x10000, 0x8000, CRC(56c1011a) SHA1(c817a3410c643617f3643897b8f529ae78546b0d))

	ROM_REGION(0x8000, "audioroms", ROMREGION_ERASEFF)
ROM_END


GAME( 1983, ratrc_l1, 0,        s9, s9, s9_state, init_s9, ROT0, "Williams", "Rat Race (L-1)",              MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1985, sorcr_l1, sorcr_l2, s9, s9, s9_state, init_s9, ROT0, "Williams", "Sorcerer (L-1)",              MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1985, sorcr_l2, 0,        s9, s9, s9_state, init_s9, ROT0, "Williams", "Sorcerer (L-2)",              MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1984, sshtl_l7, 0,        s9, s9, s9_state, init_s9, ROT0, "Williams", "Space Shuttle (L-7)",         MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1984, sshtl_l3, sshtl_l7, s9, s9, s9_state, init_s9, ROT0, "Williams", "Space Shuttle (L-3)",         MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1985, comet_l4, comet_l5, s9, s9, s9_state, init_s9, ROT0, "Williams", "Comet (L-4)",                 MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1985, comet_l5, 0,        s9, s9, s9_state, init_s9, ROT0, "Williams", "Comet (L-5)",                 MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1984, szone_l5, 0,        s9, s9, s9_state, init_s9, ROT0, "Williams", "Strike Zone (Shuffle) (L-5)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1984, szone_l2, szone_l5, s9, s9, s9_state, init_s9, ROT0, "Williams", "Strike Zone (Shuffle) (L-2)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1985, alcat_l7, 0,        s9, s9, s9_state, init_s9, ROT0, "Williams", "Alley Cats (Shuffle) (L-7)",  MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
