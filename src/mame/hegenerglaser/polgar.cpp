// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/*******************************************************************************

Mephisto Polgar

Hardware notes:
- RP65C02G @ 4.91MHz
- 64KB ROM (25% unused)
- Mephisto modular display module
- Mephisto modular chessboard

The 10MHz version has a W65C02P-8 @ 9.83MHz.

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"
#include "mmdisplay2.h"

#include "cpu/m6502/m65c02.h"
#include "cpu/m6502/r65c02.h"
#include "machine/74259.h"
#include "machine/nvram.h"

// internal artwork
#include "mephisto_polgar.lh"


namespace {

class polgar_state : public driver_device
{
public:
	polgar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_keys(*this, "KEY")
	{ }

	void polgar(machine_config &config);
	void polgar10(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_ioport m_keys;

	void polgar_mem(address_map &map) ATTR_COLD;

	u8 keys_r(offs_t offset);
};



/*******************************************************************************
    I/O
*******************************************************************************/

u8 polgar_state::keys_r(offs_t offset)
{
	return ~(BIT(m_keys->read(), offset) << 7);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void polgar_state::polgar_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x2000).w("display", FUNC(mephisto_display2_device::latch_w));
	map(0x2004, 0x2004).w("display", FUNC(mephisto_display2_device::io_w));
	map(0x2400, 0x2400).w("board", FUNC(mephisto_board_device::led_w));
	map(0x2800, 0x2800).w("board", FUNC(mephisto_board_device::mux_w));
	map(0x2c00, 0x2c07).r(FUNC(polgar_state::keys_r));
	map(0x3000, 0x3000).r("board", FUNC(mephisto_board_device::input_r));
	map(0x3400, 0x3407).w("outlatch", FUNC(hc259_device::write_d7));
	map(0x4000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( polgar )
	PORT_START("KEY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("TRN / Pawn")      PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("INFO / Knight")   PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("MEM / Bishop")    PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("POS / Rook")      PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("LEV / Queen")     PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("FCT / King")      PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("ENT / New Game")  PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_F1) // combine for NEW GAME
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("CL / New Game")   PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_F1) // "
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void polgar_state::polgar(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 4.9152_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &polgar_state::polgar_mem);

	const attotime nmi_period = attotime::from_hz(4.9152_MHz_XTAL / 0x2000);
	m_maincpu->set_periodic_int(FUNC(polgar_state::nmi_line_pulse), nmi_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	hc259_device &outlatch(HC259(config, "outlatch"));
	outlatch.q_out_cb<0>().set_output("led100");
	outlatch.q_out_cb<1>().set_output("led101");
	outlatch.q_out_cb<2>().set_output("led102");
	outlatch.q_out_cb<3>().set_output("led103");
	outlatch.q_out_cb<4>().set_output("led104");
	outlatch.q_out_cb<5>().set_output("led105");

	MEPHISTO_SENSORS_BOARD(config, "board");
	MEPHISTO_DISPLAY_MODULE2(config, "display");
	config.set_default_layout(layout_mephisto_polgar);
}

void polgar_state::polgar10(machine_config &config)
{
	polgar(config);

	// basic machine hardware
	M65C02(config.replace(), m_maincpu, 9.8304_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &polgar_state::polgar_mem);

	const attotime nmi_period = attotime::from_hz(9.8304_MHz_XTAL / 0x2000);
	m_maincpu->set_periodic_int(FUNC(polgar_state::nmi_line_pulse), nmi_period);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( polgar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("polgar_1.5_01.02.1990", 0x0000, 0x10000, CRC(88d55c0f) SHA1(e86d088ec3ac68deaf90f6b3b97e3e31b1515913) )
ROM_END

ROM_START( polgara )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("polgar_1.10_04.08.1989", 0x0000, 0x10000, CRC(a4519c55) SHA1(35463a4cbcf20ebbd5ac5bc7664a862b1557c65f) ) // TC57512AD-15
ROM_END

ROM_START( polgar101 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("polg_101.bin", 0x0000, 0x10000, CRC(8fb6afa4) SHA1(d1cf868302a665ff351686b26a149ced0045fc81) )
ROM_END

ROM_START( polgar10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("polg.10mhz_v_10.0", 0x0000, 0x10000, CRC(7c1960d4) SHA1(4d15b51f9e6f7943815945cd56078ca512a964d4) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT   COMPAT  MACHINE   INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1990, polgar,    0,       0,      polgar,   polgar, polgar_state, empty_init, "Hegener + Glaser", "Mephisto Polgar (v1.50)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, polgara,   polgar,  0,      polgar,   polgar, polgar_state, empty_init, "Hegener + Glaser", "Mephisto Polgar (v1.10)", MACHINE_SUPPORTS_SAVE )
SYST( 1990, polgar101, polgar,  0,      polgar10, polgar, polgar_state, empty_init, "Hegener + Glaser", "Mephisto Polgar 10 MHz (v10.1)", MACHINE_SUPPORTS_SAVE )
SYST( 1990, polgar10,  polgar,  0,      polgar10, polgar, polgar_state, empty_init, "Hegener + Glaser", "Mephisto Polgar 10 MHz (v10.0)", MACHINE_SUPPORTS_SAVE )
