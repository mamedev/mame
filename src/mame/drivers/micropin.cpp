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

#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "cpu/i8085/i8085.h"
#include "machine/6821pia.h"
#include "sound/beep.h"
#include "micropin.lh"

class micropin_state : public genpin_class
{
public:
	micropin_state(const machine_config &mconfig, device_type type, std::string tag)
		: genpin_class(mconfig, type, tag)
		, m_v1cpu(*this, "v1cpu")
		, m_v2cpu(*this, "v2cpu")
		, m_pia51(*this, "pia51")
		, m_beep(*this, "beeper")
	{ }

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
	DECLARE_DRIVER_INIT(micropin);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_a);
private:
	UINT8 m_row;
	UINT8 m_counter;
	UINT8 m_beep_time;
	UINT8 m_led_time[8];
	virtual void machine_reset() override;
	optional_device<m6800_cpu_device> m_v1cpu;
	optional_device<i8085a_cpu_device> m_v2cpu;
	optional_device<pia6821_device> m_pia51;
	optional_device<beep_device> m_beep;
};


static ADDRESS_MAP_START( micropin_map, AS_PROGRAM, 8, micropin_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM AM_SHARE("nvram") // 4x 6561 RAM
	AM_RANGE(0x4000, 0x4005) AM_WRITE(sw_w)
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("X1")
	AM_RANGE(0x4001, 0x4001) AM_READ_PORT("X2")
	AM_RANGE(0x4002, 0x4002) AM_READ_PORT("X3")
	AM_RANGE(0x4003, 0x4003) AM_READ_PORT("X4")
	AM_RANGE(0x4004, 0x4004) AM_READ_PORT("X5")
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("pia50", pia6821_device, read, write)
	AM_RANGE(0x5100, 0x5103) AM_READWRITE(pia51_r,pia51_w)
	AM_RANGE(0x5200, 0x5200) AM_WRITE(sol_w);
	AM_RANGE(0x5202, 0x5202) AM_WRITE(lamp_w);
	AM_RANGE(0x5203, 0x5203) AM_WRITENOP
	AM_RANGE(0x6400, 0x7fff) AM_ROM AM_REGION("v1cpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pentacup2_map, AS_PROGRAM, 8, micropin_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pentacup2_io, AS_IO, 8, micropin_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x0e) AM_WRITE(sw_w)
	AM_RANGE(0x0f, 0x0f) AM_WRITE(lamp_w)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("X0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("X1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("X2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("X3")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("X4")
	AM_RANGE(0x05, 0x05) AM_READ_PORT("X5")
ADDRESS_MAP_END

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
	return m_pia51->read(space, offset) ^ 0xff;
}

WRITE8_MEMBER( micropin_state::pia51_w )
{
	m_pia51->write(space, offset, data ^ 0xff);
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
		static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7448
		output().set_digit_value(m_row, patterns[data&15]);
		output().set_digit_value(m_row+20, patterns[data>>4]);
	}
}

WRITE8_MEMBER( micropin_state::p50b_w )
{
	m_counter++;
	if (m_counter == 2)
	{
		static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7448
		output().set_digit_value(m_row+40, patterns[data&15]);
		output().set_digit_value(m_row+60, patterns[data>>4]);
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
	static UINT16 frequency[16] = { 387, 435, 488, 517, 581, 652, 691, 775, 870, 977, 1035, 1161, 1304, 1381, 1550, 1740 };
	m_beep->set_frequency(frequency[data & 15]);
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

	UINT8 i;
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
	UINT8 i;
	m_row = 0;
	m_beep_time = 5;
	for (i = 0; i < 8; i++)
		m_led_time[i] = 5;
}

DRIVER_INIT_MEMBER( micropin_state, micropin )
{
}

static MACHINE_CONFIG_START( micropin, micropin_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("v1cpu", M6800, XTAL_2MHz / 2)
	MCFG_CPU_PROGRAM_MAP(micropin_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(micropin_state, irq0_line_hold, 500)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_micropin)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	MCFG_DEVICE_ADD("pia50", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(micropin_state, p50a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(micropin_state, p50a_w))
	//MCFG_PIA_READPB_HANDLER(READ8(micropin_state, p50b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(micropin_state, p50b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(micropin_state, p50ca2_w))
	//MCFG_PIA_CB2_HANDLER(WRITELINE(micropin_state, p50cb2_w))

	MCFG_DEVICE_ADD("pia51", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(micropin_state, p51a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(micropin_state, p51a_w))
	MCFG_PIA_READPB_HANDLER(READ8(micropin_state, p51b_r))
	//MCFG_PIA_WRITEPB_HANDLER(WRITE8(micropin_state, p51b_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(micropin_state, p51ca2_w))
	//MCFG_PIA_CB2_HANDLER(WRITELINE(micropin_state, p51cb2_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_a", micropin_state, timer_a, attotime::from_hz(100))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pentacup2, micropin_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("v2cpu", I8085A, 2000000)
	MCFG_CPU_PROGRAM_MAP(pentacup2_map)
	MCFG_CPU_IO_MAP(pentacup2_io)
	//MCFG_CPU_PERIODIC_INT_DRIVER(micropin_state, irq2_line_hold, 50)

	//MCFG_NVRAM_ADD_0FILL("nvram")

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
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


GAME(1978,  pentacup,  0,         micropin,   micropin, micropin_state,  micropin,  ROT0, "Micropin", "Pentacup (rev. 1)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1980,  pentacup2, pentacup,  pentacup2,  micropin, micropin_state,  micropin,  ROT0, "Micropin", "Pentacup (rev. 2)", MACHINE_IS_SKELETON_MECHANICAL)
