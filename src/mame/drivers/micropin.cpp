// license:BSD-3-Clause
// copyright-holders:Robbbert
/*************************************************************************************

  PINBALL
  Micropin : Pentacup
  First version used a 6800, but a later revision used a 8085A.

Rev.2:
- Gets stuck waiting for 21a6 to become zero (twice).
- Possible interrupts are RST55 (0x2c) and RST65 (0x34), however
  neither of them fixes the 21a6 problem.
- No manuals or schematics available, ports connected as per Pinmame.
- Uses a different layout, not coded.

ToDo:
- Rev.2 not working
- Rev.1 can insert coin and start a game, but no inputs
- Rev.1 check sound; pinmame sound is higher pitched
- Mechanical sounds

**************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/i8085/i8085.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "speaker.h"

#include "micropin.lh"


class micropin_state : public genpin_class
{
public:
	micropin_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_v1cpu(*this, "v1cpu")
		, m_v2cpu(*this, "v2cpu")
		, m_pia51(*this, "pia51")
		, m_beep(*this, "beeper")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void pentacup2(machine_config &config);
	void micropin(machine_config &config);

	void init_micropin();

private:
	DECLARE_READ8_MEMBER(pia51_r);
	DECLARE_WRITE8_MEMBER(pia51_w);
	DECLARE_READ8_MEMBER(p51b_r);
	DECLARE_WRITE8_MEMBER(sol_w);
	DECLARE_WRITE_LINE_MEMBER(p50ca2_w);
	DECLARE_WRITE8_MEMBER(sw_w);
	DECLARE_WRITE8_MEMBER(lamp_w);
	DECLARE_WRITE8_MEMBER(p50a_w);
	DECLARE_WRITE8_MEMBER(p50b_w);
	DECLARE_WRITE8_MEMBER(p51a_w);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_a);
	void micropin_map(address_map &map);
	void pentacup2_io(address_map &map);
	void pentacup2_map(address_map &map);

	uint8_t m_row;
	uint8_t m_counter;
	uint8_t m_beep_time;
	uint8_t m_led_time[8];
	virtual void machine_reset() override;
	virtual void machine_start() override { m_digits.resolve(); }
	optional_device<m6800_cpu_device> m_v1cpu;
	optional_device<i8085a_cpu_device> m_v2cpu;
	optional_device<pia6821_device> m_pia51;
	optional_device<beep_device> m_beep;
	output_finder<76> m_digits;
};


void micropin_state::micropin_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x01ff).ram().share("nvram"); // 4x 6561 RAM
	map(0x4000, 0x4005).w(FUNC(micropin_state::sw_w));
	map(0x4000, 0x4000).portr("X1");
	map(0x4001, 0x4001).portr("X2");
	map(0x4002, 0x4002).portr("X3");
	map(0x4003, 0x4003).portr("X4");
	map(0x4004, 0x4004).portr("X5");
	map(0x5000, 0x5003).rw("pia50", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x5100, 0x5103).rw(FUNC(micropin_state::pia51_r), FUNC(micropin_state::pia51_w));
	map(0x5200, 0x5200).w(FUNC(micropin_state::sol_w));
	map(0x5202, 0x5202).w(FUNC(micropin_state::lamp_w));
	map(0x5203, 0x5203).nopw();
	map(0x6400, 0x7fff).rom().region("v1cpu", 0);
}

void micropin_state::pentacup2_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();
}

void micropin_state::pentacup2_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0e).w(FUNC(micropin_state::sw_w));
	map(0x0f, 0x0f).w(FUNC(micropin_state::lamp_w));
	map(0x00, 0x00).portr("X0");
	map(0x01, 0x01).portr("X1");
	map(0x02, 0x02).portr("X2");
	map(0x03, 0x03).portr("X3");
	map(0x04, 0x04).portr("X4");
	map(0x05, 0x05).portr("X5");
}

static INPUT_PORTS_START( micropin )
	PORT_START("X0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNUSED ) // 20=volume-up; 40=volume-down button
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT ) PORT_NAME("Tilt Alarm")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Tilt 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Tilt 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Tilt 3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Tilt 4")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD)
INPUT_PORTS_END

READ8_MEMBER( micropin_state::pia51_r )
{
	return m_pia51->read(offset) ^ 0xff;
}

WRITE8_MEMBER( micropin_state::pia51_w )
{
	m_pia51->write(offset, data ^ 0xff);
}

// lamps and disp strobes
WRITE8_MEMBER( micropin_state::lamp_w )
{
	m_row = data & 15;
	m_counter = 0;
	// lamps
}

// solenoids
WRITE8_MEMBER( micropin_state::sol_w )
{
}

// offs 0,5 = solenoids; else lamps
WRITE8_MEMBER( micropin_state::sw_w )
{
}

WRITE8_MEMBER( micropin_state::p50a_w )
{
	m_counter++;
	if (m_counter == 1)
	{
		static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7448
		m_digits[m_row] = patterns[data&15];
		m_digits[m_row+20] = patterns[data>>4];
	}
}

WRITE8_MEMBER( micropin_state::p50b_w )
{
	m_counter++;
	if (m_counter == 2)
	{
		static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7448
		m_digits[m_row+40] = patterns[data&15];
		m_digits[m_row+60] = patterns[data>>4];
	}
}

// round LEDs on score panel
WRITE_LINE_MEMBER( micropin_state::p50ca2_w )
{
	if ((!state) && (m_row < 8))
	{
		char wordnum[8];
		sprintf(wordnum,"led%d", m_row);
		m_led_time[m_row] = 48; // 12 gives blinking leds; they blink in pinmame but is it correct?
		output().set_value(wordnum, 0); // turn on
	}
}

// sound & volume
// Sound consists of a 16-resistor chain controlling the frequency of a NE555.
// The sound never gets muted, but is turned down with an electronic volume control,
//   which must be the most complex circuit in this machine. We use a beeper to
//   make the tones, and turn it off if no new commands arrive within .1 second.
WRITE8_MEMBER( micropin_state::p51a_w )
{
	static uint16_t frequency[16] = { 387, 435, 488, 517, 581, 652, 691, 775, 870, 977, 1035, 1161, 1304, 1381, 1550, 1740 };
	m_beep->set_clock(frequency[data & 15]);
	m_beep_time = 10; // number of 10ms intervals before it is silenced
	m_beep->set_state(1);
}

READ8_MEMBER( micropin_state::p51b_r )
{
	return ioport("X0")->read();
}

TIMER_DEVICE_CALLBACK_MEMBER( micropin_state::timer_a )
{
	// turn off beeper if it has timed out

	if (m_beep_time)
	{
		m_beep_time--;
		if (m_beep_time == 0)
			m_beep->set_state(0);
	}

	// turn off round leds that aren't being refreshed

	uint8_t i;
	char wordnum[8];

	for (i = 0; i < 8; i++)
	{
		if (m_led_time[i])
		{
			m_led_time[i]--;
			if (m_led_time[i] == 0)
			{
				sprintf(wordnum,"led%d", i);
				output().set_value(wordnum, 1); // turn off
			}
		}
	}
}

void micropin_state::machine_reset()
{
	uint8_t i;
	m_row = 0;
	m_beep_time = 5;
	for (i = 0; i < 8; i++)
		m_led_time[i] = 5;
}

void micropin_state::init_micropin()
{
}

MACHINE_CONFIG_START(micropin_state::micropin)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("v1cpu", M6800, XTAL(2'000'000) / 2)
	MCFG_DEVICE_PROGRAM_MAP(micropin_map)
	MCFG_DEVICE_PERIODIC_INT_DRIVER(micropin_state, irq0_line_hold, 500)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_micropin);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("beeper", BEEP, 387)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	pia6821_device &pia50(PIA6821(config, "pia50", 0));
	//pia50.readpa_handler().set(FUNC(micropin_state::p50a_r));
	pia50.writepa_handler().set(FUNC(micropin_state::p50a_w));
	//pia50.readpb_handler().set(FUNC(micropin_state::p50b_r));
	pia50.writepb_handler().set(FUNC(micropin_state::p50b_w));
	pia50.ca2_handler().set(FUNC(micropin_state::p50ca2_w));
	//pia50.cb2_handler().set(FUNC(micropin_state::p50cb2_w));

	PIA6821(config, m_pia51, 0);
	//m_pia51->readpa_handler().set(FUNC(micropin_state::p51a_r));
	m_pia51->writepa_handler().set(FUNC(micropin_state::p51a_w));
	m_pia51->readpb_handler().set(FUNC(micropin_state::p51b_r));
	//m_pia51->writepb_handler().set(FUNC(micropin_state::p51b_w));
	//m_pia51->ca2_handler().set(FUNC(micropin_state::p51ca2_w));
	//m_pia51->cb2_handler().set(FUNC(micropin_state::p51cb2_w));

	TIMER(config, "timer_a").configure_periodic(FUNC(micropin_state::timer_a), attotime::from_hz(100));
MACHINE_CONFIG_END

MACHINE_CONFIG_START(micropin_state::pentacup2)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("v2cpu", I8085A, 2000000)
	MCFG_DEVICE_PROGRAM_MAP(pentacup2_map)
	MCFG_DEVICE_IO_MAP(pentacup2_io)
	//MCFG_DEVICE_PERIODIC_INT_DRIVER(micropin_state, irq2_line_hold, 50)

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Sound */
	genpin_audio(config);
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Pentacup
/-------------------------------------------------------------------*/
ROM_START(pentacup)
	ROM_REGION(0x1c00, "v1cpu", 0)
	ROM_LOAD("ic2.bin", 0x0000, 0x0400, CRC(fa468a0f) SHA1(e9c8028bcd5b87d24f4588516536767a869c38ff))
	ROM_LOAD("ic3.bin", 0x0400, 0x0400, CRC(7bfdaec8) SHA1(f2037c0e2d4acf0477351ecafc9f0826e9d64d76))
	ROM_LOAD("ic4.bin", 0x0800, 0x0400, CRC(5e0fcb1f) SHA1(e529539c6eb1e174a799ad6abfce9e31870ff8af))
	ROM_LOAD("ic5.bin", 0x0c00, 0x0400, CRC(a26c6e0b) SHA1(21c4c306fbc2da52887e309b1c83a1ea69501c1f))
	ROM_LOAD("ic6.bin", 0x1000, 0x0400, CRC(4715ac34) SHA1(b6d8c20c487db8d7275e36f5793666cc591a6691))
	ROM_LOAD("ic7.bin", 0x1400, 0x0400, CRC(c58d13c0) SHA1(014958bc69ff326392a5a7782703af0980e6e170))
	ROM_LOAD("ic8.bin", 0x1800, 0x0400, CRC(9f67bc65) SHA1(504008d4c7c23a14fdf247c9e6fc00e95d907d7b))
ROM_END

ROM_START(pentacup2)
	ROM_REGION(0x2000, "v2cpu", 0)
	ROM_LOAD("micro_1.bin", 0x0000, 0x0800, CRC(4d6dc218) SHA1(745c553f3a42124f925ca8f2e52fd08d05999594))
	ROM_LOAD("micro_2.bin", 0x0800, 0x0800, CRC(33cd226d) SHA1(d1dff8445a0f35da09d560a16038c969845ff21f))
	ROM_LOAD("micro_3.bin", 0x1000, 0x0800, CRC(997bde74) SHA1(c3ea33f7afbdc7f2a22798a13ec323d7c6628dd4))
	ROM_LOAD("micro_4.bin", 0x1800, 0x0800, CRC(a804e7d6) SHA1(f414d6a5308266744645849940c00cd422e920d2))
	// 2 undumped proms DMA-01, DMA-02
ROM_END


GAME(1978,  pentacup,  0,         micropin,   micropin, micropin_state, init_micropin, ROT0, "Micropin", "Pentacup (rev. 1)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1980,  pentacup2, pentacup,  pentacup2,  micropin, micropin_state, init_micropin, ROT0, "Micropin", "Pentacup (rev. 2)", MACHINE_IS_SKELETON_MECHANICAL)
