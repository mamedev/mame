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

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "speaker.h"

#include "s3.lh"


namespace {

class s3_state : public genpin_class
{
public:
	s3_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia22(*this, "pia22")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_swarray(*this, "SW.%u", 0U)
		, m_dips(*this, "DS%u", 1U)
	{
	}

	void s3(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia22;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;

private:
	void dig0_w(uint8_t data);
	void dig1_w(uint8_t data);
	void lamp0_w(uint8_t data);
	void lamp1_w(uint8_t data);
	void sol0_w(uint8_t data);
	void sol1_chimes_w(uint8_t data);
	uint8_t dips_r();
	uint8_t switch_r();
	void switch_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(pia22_ca2_w) { } //ST5
	DECLARE_WRITE_LINE_MEMBER(pia22_cb2_w) { } //ST-solenoids enable
	DECLARE_WRITE_LINE_MEMBER(pia24_ca2_w) { } //ST2
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { } //ST1
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { } //diag leds enable
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { } //ST6
	DECLARE_WRITE_LINE_MEMBER(pia30_ca2_w) { } //ST4
	DECLARE_WRITE_LINE_MEMBER(pia30_cb2_w) { } //ST3
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	void s3_main_map(address_map &map);

	output_finder<32> m_digits;
	output_finder<2> m_leds;
	required_ioport_array<8> m_swarray;
	required_ioport_array<2> m_dips;

	uint8_t m_t_c;
	uint8_t m_strobe;
	uint8_t m_switch_col;
	bool m_data_ok;
};


class s3a_state : public s3_state
{
public:
	s3a_state(const machine_config &mconfig, device_type type, const char *tag)
		: s3_state(mconfig, type, tag)
		, m_audiocpu(*this, "audiocpu")
		, m_pias(*this, "pias")
		, m_io_snd(*this, "SND")
	{
	}

	void s3a(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

protected:
	required_device<cpu_device> m_audiocpu;
	required_device<pia6821_device> m_pias;

private:
	uint8_t sound_r();
	void s3a_sol1_w(uint8_t data);

	void s3_audio_map(address_map &map);

	required_ioport m_io_snd;

	uint8_t m_sound_data;
};


void s3_state::s3_main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x00ff).ram();
	map(0x0100, 0x01ff).ram().share("nvram");
	map(0x2200, 0x2203).rw(m_pia22, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x6000, 0x7fff).rom().region("roms", 0);
}

void s3a_state::s3_audio_map(address_map &map)
{
	map.global_mask(0xfff);
	map(0x0400, 0x0403).rw(m_pias, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sounds
	map(0x0800, 0x0fff).rom().region("audioroms", 0);
}


static INPUT_PORTS_START( s3 )
	PORT_START("SW.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_TILT ) // 3 touches before it tilts
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )

	PORT_START("SW.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("SW.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA) // 1 touch tilt

	PORT_START("SW.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("SW.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("SW.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("SW.6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SW.7")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DIAGS")
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

static INPUT_PORTS_START( s3a )
	PORT_INCLUDE(s3)

	PORT_START("SND")
	PORT_BIT( 0xbf, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Music") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE

	PORT_MODIFY("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s3a_state, audio_nmi, 1)
INPUT_PORTS_END


void s3_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	m_strobe = 0;
}

void s3_state::machine_reset()
{
	m_t_c = 0;
}

INPUT_CHANGED_MEMBER( s3_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( s3a_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void s3_state::sol0_w(uint8_t data)
{
	if (BIT(data, 4))
		m_samples->start(5, 5); // outhole
}

void s3_state::sol1_chimes_w(uint8_t data)
{
	if (BIT(data, 0))
		m_samples->start(1, 1); // 10 chime

	if (BIT(data, 1))
		m_samples->start(2, 2); // 100 chime

	if (BIT(data, 2))
		m_samples->start(3, 3); // 1000 chime

	if (BIT(data, 3))
		m_samples->start(4, 4); // 10k chime

	if (BIT(data, 5))
		m_samples->start(0, 6); // knocker
}

void s3a_state::s3a_sol1_w(uint8_t data)
{
	uint8_t sound_data = m_io_snd->read(); // 0xff or 0xbf
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

	if (BIT(data, 5))
		m_samples->start(0, 6); // knocker
}

void s3_state::lamp0_w(uint8_t data)
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

void s3_state::lamp1_w(uint8_t data)
{
}

uint8_t s3_state::dips_r()
{
	if (BIT(ioport("DIAGS")->read(), 4))
		return m_dips[BIT(~m_strobe, 1)]->read() << (BIT(m_strobe, 0) ? 4 : 0);

	return 0xff;
}

void s3_state::dig0_w(uint8_t data)
{
	m_strobe = data & 15;
	m_data_ok = true;
	m_leds[0] = !BIT(data, 4);
	m_leds[1] = !BIT(data, 5);
}

void s3_state::dig1_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // MC14558
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data&15];
		m_digits[m_strobe] = patterns[data>>4];
	}
	m_data_ok = false;
}

uint8_t s3_state::switch_r()
{
	uint8_t retval = 0xff;
	// scan all 8 input columns, since multiple can be selected at once
	for (int i = 0; i < 7; i++)
	{
		if (m_switch_col & (1<<i))
			retval &= m_swarray[i]->read();
	}
	return ~retval;
}

void s3_state::switch_w(uint8_t data)
{
	// this drives the pulldown 7406 quad open collector inverters at IC17 and IC18, each inverter drives one column of the switch matrix low
	// it is possible for multiple columns to be enabled at once, this is handled in switch_r above.
	m_switch_col = data;
}

uint8_t s3a_state::sound_r()
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

void s3_state::s3(machine_config &config)
{
	// basic machine hardware
	M6800(config, m_maincpu, 3580000);
	m_maincpu->set_addrmap(AS_PROGRAM, &s3_state::s3_main_map);
	TIMER(config, "irq").configure_periodic(FUNC(s3_state::irq), attotime::from_hz(250));

	// Video
	config.set_default_layout(layout_s3);

	// Sound
	genpin_audio(config);

	// Devices
	PIA6821(config, m_pia22, 0);
	m_pia22->writepa_handler().set(FUNC(s3_state::sol0_w));
	m_pia22->writepb_handler().set(FUNC(s3_state::sol1_chimes_w));
	m_pia22->ca2_handler().set(FUNC(s3_state::pia22_ca2_w));
	m_pia22->cb2_handler().set(FUNC(s3_state::pia22_cb2_w));
	m_pia22->irqa_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia22->irqb_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);

	PIA6821(config, m_pia24, 0);
	m_pia24->writepa_handler().set(FUNC(s3_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s3_state::lamp1_w));
	m_pia24->ca2_handler().set(FUNC(s3_state::pia24_ca2_w));
	m_pia24->cb2_handler().set(FUNC(s3_state::pia24_cb2_w));
	m_pia24->irqa_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia24->irqb_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);

	PIA6821(config, m_pia28, 0);
	m_pia28->readpa_handler().set(FUNC(s3_state::dips_r));
	m_pia28->set_port_a_input_overrides_output_mask(0xff);
	m_pia28->readca1_handler().set_ioport("DIAGS").bit(2); // advance button
	m_pia28->readcb1_handler().set_ioport("DIAGS").bit(3); // auto/manual switch
	m_pia28->writepa_handler().set(FUNC(s3_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s3_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s3_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s3_state::pia28_cb2_w));
	m_pia28->irqa_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia28->irqb_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);

	PIA6821(config, m_pia30, 0);
	m_pia30->readpa_handler().set(FUNC(s3_state::switch_r));
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(s3_state::switch_w));
	m_pia30->ca2_handler().set(FUNC(s3_state::pia30_ca2_w));
	m_pia30->cb2_handler().set(FUNC(s3_state::pia30_cb2_w));
	m_pia30->irqa_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia30->irqb_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void s3a_state::s3a(machine_config &config)
{
	s3(config);

	m_pia22->writepb_handler().set(FUNC(s3a_state::s3a_sol1_w));

	// Add the soundcard
	M6802(config, m_audiocpu, 3580000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &s3a_state::s3_audio_map);

	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);

	PIA6821(config, m_pias, 0);
	m_pias->readpb_handler().set(FUNC(s3a_state::sound_r));
	m_pias->writepa_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pias->irqa_handler().set_inputline(m_audiocpu, M6802_IRQ_LINE);
	m_pias->irqb_handler().set_inputline(m_audiocpu, M6802_IRQ_LINE);
}


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
	ROM_LOAD("481_s0_world_cup.716",   0x0000, 0x0800, CRC(cf012812) SHA1(26074f6a44075a94e6f91de1dbf92f8ec3ff8ca4))
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
	ROM_LOAD("482_s0_contact.716",   0x0000, 0x0800, CRC(d3c713da) SHA1(1fc4a8fadf472e9a04b3a86f60a9d625d07764e1))
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
	ROM_LOAD("483_s0_disco_fever.716",   0x0000, 0x0800, CRC(d1cb5047) SHA1(7f36296975df19feecc6456ffb91f4a23bcad037))
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
	ROM_LOAD("485_s0_phoenix.716",   0x0000, 0x0800, CRC(1c3dea6e) SHA1(04bfe952be2eab66f023b204c21a1bd461ea572f))
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
	ROM_LOAD("488_s0_pokerino.716",   0x0000, 0x0800, CRC(5de02e62) SHA1(f838439a731511a264e508a576ae7193d9fed1af))
ROM_END

} // Anonymous namespace

GAME( 1977, httip_l1, 0, s3,  s3,  s3_state,  empty_init, ROT0, "Williams", "Hot Tip (L-1)",          MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1977, lucky_l1, 0, s3,  s3,  s3_state,  empty_init, ROT0, "Williams", "Lucky Seven (L-1)",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1978, wldcp_l1, 0, s3a, s3a, s3a_state, empty_init, ROT0, "Williams", "World Cup (L-1)",        MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1978, cntct_l1, 0, s3a, s3a, s3a_state, empty_init, ROT0, "Williams", "Contact (L-1)",          MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1978, disco_l1, 0, s3a, s3a, s3a_state, empty_init, ROT0, "Williams", "Disco Fever (L-1)",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1978, phnix_l1, 0, s3a, s3a, s3a_state, empty_init, ROT0, "Williams", "Phoenix (L-1)",          MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1978, pkrno_l1, 0, s3a, s3a, s3a_state, empty_init, ROT0, "Williams", "Pokerino (L-1)",         MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
