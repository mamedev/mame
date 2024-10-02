// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/*******************************************************************************

Schachcomputer SC 2 (G-5002.500)

2nd chess computer by VEB(Volkseigener Betrieb) Funkwerk Erfurt. The company
was renamed to VEB Mikroelektronik "Karl Marx" Erfurt in 1983, and formed into
X-FAB Semiconductor Foundries AG after the German unification. SC 2 chess
program is an unlicensed copy of Fidelity Chess Challenger 10 C, with some
patches and an extra 1KB ROM to deal with the different I/O.

3 versions known: initial version, revision E, revision EP.

Schachcomputer SC 1 was canceled before wide release, it's assumed to
be on similar hardware, but PCB photos show 10 ROM chips instead of 9.

keypad legend:

R - Rückstellen (reset)
K - Programmstufen (level)
W - Figurenwahl (white/black)
P - Problemeingabe (problem mode)
T - Tonabschaltung (sound on/off)
L - Löschen (clear)
Q - Quittierung (enter)

Fidelity CC10 synonyms: RE, LV, RV, PB, ♪, CL, EN

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "sc2.lh"


namespace {

class sc2_state : public driver_device
{
public:
	sc2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio(*this, "z80pio"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void sc2(machine_config &config);

	// Rückstellen is also tied to CPU RESET
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<4> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_digit_data = 0;

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	void update_display();
	u8 pio_port_b_r();
	void pio_port_a_w(u8 data);
	void pio_port_b_w(u8 data);
	u8 speaker_r(offs_t offset);
};

void sc2_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_digit_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void sc2_state::update_display()
{
	m_display->matrix(~m_inp_mux, m_digit_data);
}

u8 sc2_state::pio_port_b_r()
{
	u8 data = 0;

	// read keypad
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return data << 4 | 0xf;
}

void sc2_state::pio_port_a_w(u8 data)
{
	// digit segment data
	m_digit_data = bitswap<8>(data,7,0,1,2,3,4,5,6);
	update_display();
}

void sc2_state::pio_port_b_w(u8 data)
{
	// d0-d3: keypad mux(active high), led mux(active low)
	m_inp_mux = data;
	update_display();
}

u8 sc2_state::speaker_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_dac->write(BIT(~offset, 11));

	return 0xff;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void sc2_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x13ff).ram();
	map(0x2000, 0x33ff).rom();
	map(0x3400, 0x3400).select(0x800).r(FUNC(sc2_state::speaker_r));
}

void sc2_state::main_io(address_map &map)
{
	map.global_mask(0x03);
	map(0x00, 0x03).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( sc2 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A 1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B 2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C 3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D 4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E 5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F 6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G 7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H 8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, sc2_state, reset_button, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void sc2_state::sc2(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 9.8304_MHz_XTAL/4); // U880 Z80 clone
	m_maincpu->set_addrmap(AS_PROGRAM, &sc2_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &sc2_state::main_io);

	Z80PIO(config, m_pio, 9.8304_MHz_XTAL/4);
	m_pio->out_pa_callback().set(FUNC(sc2_state::pio_port_a_w));
	m_pio->in_pb_callback().set(FUNC(sc2_state::pio_port_b_r));
	m_pio->out_pb_callback().set(FUNC(sc2_state::pio_port_b_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_sc2);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sc2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bm008.bin", 0x0000, 0x0400, CRC(3023ea82) SHA1(07020d153d802c672c39e1af3c716dbe35e23f08) )
	ROM_LOAD( "bm009.bin", 0x0400, 0x0400, CRC(6a34814e) SHA1(e58ae6615297b028db135a48a8f9e186a4220f4f) )
	ROM_LOAD( "bm010.bin", 0x0800, 0x0400, CRC(deab0373) SHA1(81c9a7197eef8d9131e47ecd2ec35b943caee54e) )
	ROM_LOAD( "bm011.bin", 0x0c00, 0x0400, CRC(c8282339) SHA1(8d6b8861281e967a77609b6d77e80afd47d28ed2) )
	ROM_LOAD( "bm012.bin", 0x2000, 0x0400, CRC(2e6a4294) SHA1(7b9bd191c9ec73139a65c3a339ab88e1f3eb5ed2) )
	ROM_LOAD( "bm013.bin", 0x2400, 0x0400, CRC(3e02eb42) SHA1(2e4a9a8fd04c202c9518550d7e8cf9bfea394153) )
	ROM_LOAD( "bm014.bin", 0x2800, 0x0400, CRC(538d449e) SHA1(c4186995b69e97740e01eaff84a20d49d03d180f) )
	ROM_LOAD( "bm015.bin", 0x2c00, 0x0400, CRC(b4991dca) SHA1(6a6cdddf5c4afa24773acf693f58c34b99c8d328) )
	ROM_LOAD( "bm037.bin", 0x3000, 0x0400, CRC(2b67faf1) SHA1(5c65734acaeb766240dbd492a774c56fcfc382f7) )
ROM_END

ROM_START( sc2a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bm008.bin", 0x0000, 0x0400, CRC(3023ea82) SHA1(07020d153d802c672c39e1af3c716dbe35e23f08) )
	ROM_LOAD( "bm009.bin", 0x0400, 0x0400, CRC(6a34814e) SHA1(e58ae6615297b028db135a48a8f9e186a4220f4f) )
	ROM_LOAD( "bm010.bin", 0x0800, 0x0400, CRC(deab0373) SHA1(81c9a7197eef8d9131e47ecd2ec35b943caee54e) )
	ROM_LOAD( "bm011.bin", 0x0c00, 0x0400, CRC(c8282339) SHA1(8d6b8861281e967a77609b6d77e80afd47d28ed2) )
	ROM_LOAD( "bm012.bin", 0x2000, 0x0400, CRC(2e6a4294) SHA1(7b9bd191c9ec73139a65c3a339ab88e1f3eb5ed2) )
	ROM_LOAD( "bm013.bin", 0x2400, 0x0400, CRC(3e02eb42) SHA1(2e4a9a8fd04c202c9518550d7e8cf9bfea394153) )
	ROM_LOAD( "bm014.bin", 0x2800, 0x0400, CRC(538d449e) SHA1(c4186995b69e97740e01eaff84a20d49d03d180f) )
	ROM_LOAD( "bm015.bin", 0x2c00, 0x0400, CRC(b4991dca) SHA1(6a6cdddf5c4afa24773acf693f58c34b99c8d328) )
	ROM_LOAD( "bm016.bin", 0x3000, 0x0400, CRC(4fe0853a) SHA1(c2253e320778b0ea468fb54f26ae83d07f9700e6) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1981, sc2,  0,      0,      sc2,     sc2,   sc2_state, empty_init, "VEB Funkwerk Erfurt", "Schachcomputer SC 2 (rev. E)", MACHINE_SUPPORTS_SAVE )
SYST( 1981, sc2a, sc2,    0,      sc2,     sc2,   sc2_state, empty_init, "VEB Funkwerk Erfurt", "Schachcomputer SC 2", MACHINE_SUPPORTS_SAVE )
