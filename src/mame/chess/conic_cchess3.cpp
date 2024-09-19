// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Conic Computer Chess (model 07013), more commonly known as Conic "Korchnoi".

The interface is similar to the previous model (07012), where the user needs
to press Enter after their move.

Hardware notes:
- Synertek 6502A @ 2MHz
- OKI MSM5840H-41RS @ 3.57MHz (2KB internal ROM)
- 2*4KB ROM(AMI), 1KB RAM(2*2114)
- beeper, 8*8+4 leds, magnets chessboard

TODO:
- It does not work, MSM5840 is unemulated, and the internal ROM is not dumped.
  MCU handles inputs, leds, sound, and it communicates with maincpu after
  triggering an IRQ.

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
//#include "conic_cchess3.lh"


namespace {

class cchess3_state : public driver_device
{
public:
	cchess3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac")
	{ }

	// machine configs
	void cncchess3(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;

	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void main_comm_w(u8 data);
	u8 main_comm_r();
};

void cchess3_state::machine_start()
{
}



/*******************************************************************************
    I/O
*******************************************************************************/

void cchess3_state::main_comm_w(u8 data)
{
}

u8 cchess3_state::main_comm_r()
{
	return 0;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void cchess3_state::main_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x03ff).ram();
	map(0x1000, 0x1000).rw(FUNC(cchess3_state::main_comm_r), FUNC(cchess3_state::main_comm_w));
	map(0x2000, 0x3fff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( cncchess3 )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cchess3_state::cncchess3(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cchess3_state::main_map);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8+1, 8);
	//config.set_default_layout(layout_conic_cchess3);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( cncchess3 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("ci07013-1", 0x2000, 0x1000, CRC(3251a529) SHA1(729b22d7653761ff0951ce1da58fdfcd474a700d) ) // AMI 2332
	ROM_LOAD("ci07013-2", 0x3000, 0x1000, CRC(0f38dcef) SHA1(f8fb7e12b41753fe52dd2eb2edb843211b5ca7c1) ) // "

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD("msm5840h-41rs", 0x0000, 0x0800, NO_DUMP )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1981, cncchess3, 0,      0,      cncchess3, cncchess3, cchess3_state, empty_init, "Conic", "Computer Chess (Conic, model 7013)", MACHINE_SUPPORTS_SAVE | MACHINE_IS_SKELETON )
