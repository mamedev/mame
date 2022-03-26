// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, Couriersud
/******************************************************************************

VTech Game Machine tabletop/handheld
It's an electronic game machine + calculator.

Hardware notes:
- Mostek MK3870 MCU (2KB internal ROM)
- 12 digits 7seg VFD/LED panel (LED handheld version has 4 extra LEDs)
- tabletop: MC1455P(555 timer) + bunch of discrete components for sound
- handheld: simple 1-bit sound

CPU Frequency should be around 4 to 4.5MHz. Sean's measurement and video of
the LED handheld version is around 4.4MHz, and there are other video references
on YouTube with slower speed. Sean's measurement of the bigger Game Machine was
around 2.1MHz, that's way too slow compared to video references and it was perhaps
a case of measuring equipment influencing CPU speed.

The I/O for the tabletop and handheld version is nearly identical, enough to
put them in the same MAME driver. The main difference is the more complex sound
on the tabletop version.

The first version should be the tabletop (lower ROM serial). It was created by
VTech, but they didn't distribute it by themselves* until later in 1980 as the
Computer Game System. VTech Computron/Lesson One has a very similar design.

*: Apparently, VTech (Hong Kong company) first couple of products were published
through foreign companies. It wasn't until around 1980 when they started publishing
under their own brand.

Known releases:

Tabletop version:
- Waddingtons 2001: The Game Machine, by Waddingtons
- Computer Game System, by VTech
- Computer Game System, by Cheryco (Japan)
- Bingo 2000: Der Spiele-Computer, by Cheryco
- Game Machine 2, by VTech (sequel with 5 games)

Handheld LED version (Speedway, Brain Drain, Blackjack, Calculator):
- 4 in 1 Electronic Games (model CGS-2011), by VTech
- Electronic Games (model 60-2143), by Tandy (Radio Shack brand)
- 4-in-1 Electronic Computer Game, by Grandstand
- Enterprise, by Videomaster (this is Waddingtons)
- Micro-Game Centre, by Prinztronic

Handheld VFD version (Code Hunter, Grand Prix, Sub Hunt, Blackjack):
- Mini Game Machine, by VTech
- Mini Game Machine, by House of Games (this is Waddingtons)
- The Game Machine, by Grandstand

BTANB:
- gamemach: some digit segments get stuck after crashing in the GP game

===============================================================================

Tabletop version notes:

After boot, press a number to start a game:
0: 4 Function Calculator (not a game)
1: Shooting Gallery
2: Black Jack
3: Code Hunter
4: Grand Prix

Screen and keypad overlays were provided for each game, though the default keypad
labels already show the alternate functions.

keypad reference (mapped to PC keyboard A-row and Z-row by default)

Calculator:
  [RET] [MS ] [MR ] [+/-] [.  ] [+= ] [-= ] [x  ] [/  ] [CL ]
  [0  ] [1  ] [2  ] [3  ] [4  ] [5  ] [6  ] [7  ] [8  ] [9  ]

Shooting Gallery:
  [RET] [Cyc] [Zig] [Rnd] [   ] [   ] [   ] [   ] [   ] [   ] * Cyclic, Zigzag, Random
  [   ] [   ] [   ] [   ] [   ] [   ] [   ] [   ] [   ] [   ] * + any of 20 buttons for shooting target

Black Jack:
  [RET] [Dl ] [   ] [   ] [   ] [   ] [   ] [   ] [Hit] [Stn] * Deal, Hit, Stand
  [   ] [   ] [   ] [   ] [   ] [   ] [   ] [   ] [   ] [   ]

Code Hunter:
  [RET] [Sta] [Dis] [   ] [   ] [Ent] [   ] [Crs] [R< ] [R> ] * Start, Display, Enter, Cursor key, Review back, Review ahead
  [   ] [   ] [   ] [   ] [   ] [   ] [   ] [   ] [   ] [   ]

Grand Prix:
  [RET] [Go ] [   ] [   ] [   ] [   ] [   ] [Up ] [Up ] [Up ]
  [Brk] [Gas] [   ] [   ] [   ] [   ] [   ] [Dwn] [Dwn] [Dwn]

******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "video/pwm.h"
#include "machine/f3853.h"
#include "machine/netlist.h"
#include "sound/spkrdev.h"

#include "speaker.h"

#include "audio/nl_gamemachine.h"

// internal artwork
#include "gamemach.lh"
#include "v4in1eg.lh"

namespace {

class gm_state : public driver_device
{
public:
	gm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_psu(*this, "psu"),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_snd_nl_pin(*this, "snd_nl:p%02u", 8U),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void v4in1eg(machine_config &config);
	void gamemach(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<f38t56_device> m_psu;
	required_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_device_array<netlist_mame_logic_input_device, 8> m_snd_nl_pin;
	required_ioport_array<2> m_inputs;

	void main_map(address_map &map);
	void main_io(address_map &map);

	void update_display();
	void mux1_w(u8 data);
	void mux2_w(u8 data);
	void digit_w(u8 data);
	u8 input_r();
	void sound_w(u8 data);
	void discrete_w(u8 data);
	u8 sound_r();

	u16 m_mux = 0;
	u8 m_seg_data = 0;
	u8 m_sound_data = 0;
};

void gm_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_mux));
	save_item(NAME(m_seg_data));
	save_item(NAME(m_sound_data));
}



/******************************************************************************
    I/O
******************************************************************************/

void gm_state::update_display()
{
	m_display->matrix(m_mux, m_seg_data);
}

void gm_state::mux1_w(u8 data)
{
	// P00-P07: mux part
	m_mux = (m_mux & ~0xff0) | (data << 4 & 0xff0);
	update_display();
}

void gm_state::mux2_w(u8 data)
{
	// P14-P17: mux part
	m_mux = (m_mux & ~0xf) | (data >> 4 & 0xf);
	update_display();
}

void gm_state::digit_w(u8 data)
{
	// P50-P57: digit 7seg data
	m_seg_data = bitswap<8>(data,0,1,2,3,4,5,6,7);
	update_display();
}

u8 gm_state::input_r()
{
	u8 data = 0;

	// P12,P13: multiplexed inputs
	for (int i = 0; i < 12; i++)
		if (BIT(m_mux, i))
			for (int j = 0; j < 2; j++)
				data |= BIT(m_inputs[j]->read(), i) << j;

	return data << 2;
}

void gm_state::sound_w(u8 data)
{
	m_sound_data = data;

	// P40: speaker out
	m_speaker->level_w(data & 1);

	// P44-P47: 4 extra leds
	m_mux = (m_mux & ~0xf000) | (bitswap<4>(data,7,6,4,5) << 12);
	update_display();
}

void gm_state::discrete_w(u8 data)
{
	m_sound_data = data;

	// P40-P47: 555 to speaker (see nl_gamemachine.cpp)
	for (int i = 0; i < 8; i++)
		m_snd_nl_pin[i]->write_line(BIT(~data, i));
}

u8 gm_state::sound_r()
{
	return m_sound_data;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void gm_state::main_map(address_map &map)
{
	map.global_mask(0x07ff);
	map(0x0000, 0x07ff).rom();
}

void gm_state::main_io(address_map &map)
{
	map(0x00, 0x00).w(FUNC(gm_state::mux1_w));
	map(0x01, 0x01).rw(FUNC(gm_state::input_r), FUNC(gm_state::mux2_w));
	map(0x04, 0x07).rw("psu", FUNC(f38t56_device::read), FUNC(f38t56_device::write));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( gamemach )
	PORT_START("IN.0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("CL")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME(u8"÷")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_CODE(KEYCODE_ASTERISK) PORT_CODE(KEYCODE_UP) PORT_NAME(u8"×")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-=")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("+=")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_END) PORT_NAME("MR")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_HOME) PORT_NAME("MS")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_R) PORT_NAME("Return")
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("7")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( v4in1eg )
	PORT_START("IN.0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("G2: Code / M+")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("G2: Manual / M-")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("G2: Exam / MR")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("G2: Enter / MC")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("G3: Black Jack")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("G2: Brain Drain")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("G4: Calc. / C/CE")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("G1: Speed Way")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("G3: Total / =")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(u8"G1: Up / ÷")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(u8"G1: Down / ×")
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("G3: Insur. / -")

	PORT_START("IN.1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("G3: Clear / +")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("G3: Bet / .")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G1: Gas / 7")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("G1: Brake / 4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("G3: Split / 3")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("G3: Double / 2")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("G3: Hit / 1")
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("G3: Stand / 0")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void gm_state::v4in1eg(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 4200000/2); // MK3870, approximation (internal /2 divider)
	m_maincpu->set_addrmap(AS_PROGRAM, &gm_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &gm_state::main_io);

	F38T56(config, m_psu, 4200000/2);
	m_psu->write_a().set(FUNC(gm_state::sound_w));
	m_psu->read_a().set(FUNC(gm_state::sound_r));
	m_psu->write_b().set(FUNC(gm_state::digit_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(12+4, 8);
	m_display->set_segmask(0xfff, 0xff);
	m_display->set_bri_levels(0.01, 0.1); // player led may be brighter
	config.set_default_layout(layout_v4in1eg);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void gm_state::gamemach(machine_config &config)
{
	v4in1eg(config);

	// basic machine hardware
	m_psu->write_a().set(FUNC(gm_state::discrete_w));

	config.set_default_layout(layout_gamemach);

	// sound hardware
	NETLIST_SOUND(config, "snd_nl", 48000).set_source(NETLIST_NAME(gamemachine)).add_route(ALL_OUTPUTS, "mono", 1.0);
	NETLIST_STREAM_OUTPUT(config, "snd_nl:cout0", 0, "SPK1.2").set_mult_offset(-10000.0 / 32768.0, 10000.0 * 3.75 / 32768.0);

	NETLIST_LOGIC_INPUT(config, "snd_nl:p08", "P08.IN", 0);
	NETLIST_LOGIC_INPUT(config, "snd_nl:p09", "P09.IN", 0);
	NETLIST_LOGIC_INPUT(config, "snd_nl:p10", "P10.IN", 0);
	NETLIST_LOGIC_INPUT(config, "snd_nl:p11", "P11.IN", 0);
	NETLIST_LOGIC_INPUT(config, "snd_nl:p12", "P12.IN", 0);
	NETLIST_LOGIC_INPUT(config, "snd_nl:p13", "P13.IN", 0);
	NETLIST_LOGIC_INPUT(config, "snd_nl:p14", "P14.IN", 0);
	NETLIST_LOGIC_INPUT(config, "snd_nl:p15", "P15.IN", 0);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( gamemach )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("mk14154n_2001", 0x0000, 0x0800, CRC(6d524c32) SHA1(73d84e59952b751c76dff8bf259b98e1f9136b41) )
ROM_END

ROM_START( v4in1eg )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("mk14336n_2011", 0x0000, 0x0800, CRC(1846a033) SHA1(8bceff44d80a5d3c1ef6f80a79d03f4083edc280) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE   INPUT     CLASS     INIT        COMPANY, FULLNAME, FLAGS
CONS( 1978, gamemach, 0,      0, gamemach, gamemach, gm_state, empty_init, "VTech / Waddingtons", "The Game Machine", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

CONS( 1979, v4in1eg,  0,      0, v4in1eg,  v4in1eg,  gm_state, empty_init, "VTech", "4 in 1 Electronic Games (VTech)", MACHINE_SUPPORTS_SAVE )
