// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

Saitek Corona. This is a subclass of saitek_stratos_state.
Please refer to saitek_stratos.cpp for driver notes.

To be brief, Saitek Corona has two "HELIOS" chips, I/O addressing is completely
different compared to Stratos/Turbo King.

***************************************************************************/

#include "emu.h"
#include "includes/saitek_stratos.h"

#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"

#include "softlist.h"
#include "speaker.h"

// internal artwork
#include "saitek_corona.lh" // clickable


class saitek_corona_state : public saitek_stratos_state
{
public:
	saitek_corona_state(const machine_config &mconfig, device_type type, const char *tag) :
		saitek_stratos_state(mconfig, type, tag),
		m_board(*this, "board"),
		m_dac(*this, "dac")
	{ }

	// machine drivers
	void corona(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<sensorboard_device> m_board;
	required_device<dac_bit_interface> m_dac;

	void main_map(address_map &map);
};

void saitek_corona_state::machine_start()
{
	saitek_stratos_state::machine_start();
}

void saitek_corona_state::machine_reset()
{
	saitek_stratos_state::machine_reset();
}



/******************************************************************************
    I/O
******************************************************************************/

// HELIOS


/******************************************************************************
    Address Maps
******************************************************************************/

void saitek_corona_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( corona )
	PORT_INCLUDE( saitek_stratos )

	PORT_MODIFY("IN.6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void saitek_corona_state::corona(machine_config &config)
{
	/* basic machine hardware */
	M65C02(config, m_maincpu, 5_MHz_XTAL); // see set_cpu_freq
	m_maincpu->set_addrmap(AS_PROGRAM, &saitek_corona_state::main_map);
	m_maincpu->set_periodic_int(FUNC(saitek_corona_state::irq0_line_hold), attotime::from_hz(100));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(2+4, 8+1);
	config.set_default_layout(layout_saitek_corona);

	TIMER(config, "lcd_busy").configure_generic(timer_device::expired_delegate());

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* extension rom */
	GENERIC_CARTSLOT(config, m_extrom, generic_plain_slot, "saitek_egr", "bin");
	m_extrom->set_device_load(FUNC(saitek_corona_state::extrom_load), this);

	SOFTWARE_LIST(config, "cart_list").set_original("saitek_egr");
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( corona )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w2_708g_u2.u2", 0x0000, 0x8000, CRC(52568bb4) SHA1(83fe91787e17bbefc2b3ec651ddb11c88990060d) )
	ROM_LOAD("bw2_708a_u3.u3", 0x8000, 0x8000, CRC(32848f73) SHA1(a447543e3eb4757f9afed26fde77b66985eb96a7) )
ROM_END

ROM_START( coronaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w2_a14c_u2.u2", 0x0000, 0x8000, CRC(be82e199) SHA1(cfcc573774b6907ed137dca01fa7f3fce493a89f) )
	ROM_LOAD("bw2_a14_u3.u3", 0x8000, 0x8000, CRC(abe87285) SHA1(b15f7ddeac78d252cf413ba4085523e44c6d15df) )
ROM_END



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME      PARENT  CMP MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS */
CONS( 1988, corona,   0,       0, corona,  corona, saitek_corona_state, empty_init, "Saitek", "Kasparov Corona (ver. D+)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_CLICKABLE_ARTWORK ) // aka Corona II
CONS( 1988, coronaa,  corona,  0, corona,  corona, saitek_corona_state, empty_init, "Saitek", "Kasparov Corona (ver. D)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_CLICKABLE_ARTWORK )
