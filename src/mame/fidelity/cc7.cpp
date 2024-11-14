// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Fidelity Chess Challenger 7 (BCC)
---------------------------------
It was Fidelity's most sold chess computer. The first version was released in
1979, and a newer PCB revision was produced in 1980.

Zilog Z80A, 3.579MHz from XTAL
Z80 IRQ/NMI unused, no timer IC.
This is a cost-reduced design from CC10, no special I/O chips.

Known ROM labels (long production run, so there are several revisions):
CN19064N BCC, CN19103N BCC-REVB, 101-32016, 101-1005C01

Backgammon Challenger (BKC) is the same PCB, with the speaker connection going
to the display panel instead.

CC7 (BCC) was also bootlegged around 1981 by Splice Industria Brasileira,
as "Byte XD-300". Mostek MK3880N-4 @ 4MHz, ROM contents is same as BCC REVB.

RE information from netlist by Berger (1st version PCB)

Memory map:
-----------
0000-0FFF: 4K 2332 ROM
2000-2FFF: ROM/RAM bus conflict!
3000-3FFF: 256 bytes RAM (2111 SRAM x2)
4000-FFFF: Z80 A14/A15 not connected

Port map (Write):
-----------------
D0-D3: digit select and keypad mux
D4: CHECK led
D5: LOSE led
A0-A2: NE591 A0-A2
D7: NE591 D (_C not used)
NE591 Q0-Q6: digit segments A-G
NE591 Q7: buzzer

Port map (Read):
----------------
D0-D3: keypad row

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "fidel_bkc.lh"
#include "fidel_cc7.lh"


namespace {

class cc7_state : public driver_device
{
public:
	cc7_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void cc7(machine_config &config);
	void bkc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	optional_device<dac_1bit_device> m_dac;
	required_ioport_array<4> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_7seg_data = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	// I/O handlers
	u8 input_r();
	void control_w(offs_t offset, u8 data);
};

void cc7_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_7seg_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void cc7_state::control_w(offs_t offset, u8 data)
{
	// a0-a2,d7: digit segment data via NE591
	u8 mask = 1 << (offset & 7);
	m_7seg_data = (m_7seg_data & ~mask) | ((data & 0x80) ? mask : 0);

	// BCC: NE591 Q7 is speaker out
	if (m_dac != nullptr)
		m_dac->write(BIT(m_7seg_data, 7));

	// d0-d3: led select, input mux
	// d4,d5: upper leds(direct)
	m_display->matrix(data & 0x3f, m_7seg_data);
	m_inp_mux = data & 0xf;
}

u8 cc7_state::input_r()
{
	u8 data = 0;

	// d0-d3: multiplexed inputs
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void cc7_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x3fff);
	map(0x0000, 0x0fff).rom();
	map(0x3000, 0x30ff).mirror(0x0f00).ram();
}

void cc7_state::main_io(address_map &map)
{
	map.global_mask(0x07);
	map(0x00, 0x07).rw(FUNC(cc7_state::input_r), FUNC(cc7_state::control_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( cc7 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("EN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PV") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PB") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CB") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("DM") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LV") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E)
INPUT_PORTS_END

static INPUT_PORTS_START( bkc )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("EN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("GM") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PB") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PV") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cc7_state::bkc(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cc7_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &cc7_state::main_io);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 8);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_fidel_bkc);
}

void cc7_state::cc7(machine_config &config)
{
	bkc(config);
	config.set_default_layout(layout_fidel_cc7);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( cc7 ) // PCB label 510-380
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "101-32016", 0x0000, 0x1000, CRC(b9076c52) SHA1(09b17ac6cd6a1c5c62aea3649f3367bcf4405598) ) // 2332
ROM_END

ROM_START( cc7a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cn19103n_bcc-revb", 0x0000, 0x1000, CRC(a397d471) SHA1(9b12bc442fccee40f4d8500c792bc9d886c5e1a5) ) // 2332
ROM_END


ROM_START( backgamc ) // model BKC, PCB label P-380A-5
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cn19255n_101-32012", 0x0000, 0x1000, CRC(0a8a19b7) SHA1(d6f0dd44b33c9b79570cf0ceac02a036ec91ba57) ) // 2332
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, cc7,      0,      0,      cc7,     cc7,   cc7_state, empty_init, "Fidelity Electronics", "Chess Challenger \"7\" (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1979, cc7a,     cc7,    0,      cc7,     cc7,   cc7_state, empty_init, "Fidelity Electronics", "Chess Challenger \"7\" (set 2)", MACHINE_SUPPORTS_SAVE )

SYST( 1979, backgamc, 0,      0,      bkc,     bkc,   cc7_state, empty_init, "Fidelity Electronics", "Backgammon Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
