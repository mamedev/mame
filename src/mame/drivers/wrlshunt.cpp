// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

	Wireless Hunting Video Game System skeleton driver

	System: Wireless Hunting Video Game System
	Publisher: Hamy / Kids Station Toys Inc
	Year: 2011
	ROM: FDI MSP55LV100G
	RAM: Micron Technology 48LC8M16A2

	Games:
		Secret Mission
		Predator
		Delta Force
		Toy Land
		Dream Forest
		Trophy Season
		Freedom Force
		Be Careful
		Net Power
		Open Training
		Super Archer
		Ultimate Frisbee
		UFO Shooting
		Happy Darts
		Balloon Shoot
		Avatair
		Angry Pirate
		Penguin War
		Ghost Shooter
		Duck Hunt


	ROM Board:
		Package: SO44
		Spacing: 1.27 mm
		Width: 16.14 mm
		Length: 27.78 mm
		Voltage: 3V
		Pinout:

			  A25  A24
				|  |
		  +--------------------------+
	A21 --|==   #  # `.__.'        ==|-- A20
	A18 --|==                      ==|-- A19
	A17 --|==                      ==|-- A8
	 A7 --|==                      ==|-- A9
	 A6 --|==                  o   ==|-- A10
	 A5 --|==  +----------------+  ==|-- A11
	 A4 --|==  |                |  ==|-- A12
	 A3 --|==  |  MSP55LV100G   |  ==|-- A13
	 A2 --|==  |  0834 M02H     |  ==|-- A14
	 A1 --|==  |  JAPAN         |  ==|-- A15
	 A0 --|==  |                |  ==|-- A16
	#CE --|==  |                |  ==|-- A23
	GND --|==  |                |  ==|-- A22
	#OE --|==  |                |  ==|-- Q15
	 Q0 --|==  |                |  ==|-- Q7
	 Q8 --|==  |                |  ==|-- Q14
	 Q1 --|==  +----------------+  ==|-- Q6
	 Q9 --|==                      ==|-- Q13
	 Q2 --|==       M55L100G       ==|-- Q5
	Q10 --|==                      ==|-- Q12
	 Q3 --|==                      ==|-- Q4
	Q11 --|==                      ==|-- VCC
		  +--------------------------+

	The only interesting string in this ROM is SPF2ALP,
	which is also found in the Wireless Air 60 ROM.

*******************************************************************************/

#include "emu.h"

#include "cpu/unsp/unsp.h"
#include "machine/spg2xx.h"

#include "screen.h"
#include "speaker.h"

class wrlshunt_state : public driver_device
{
public:
	wrlshunt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_spg(*this, "spg")
	{ }

	void wrlshunt(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void mem_map(address_map &map);

	required_device<unsp12_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<spg2xx_device> m_spg;
};


/************************************
 *
 *  Machine Hardware
 *
 ************************************/

void wrlshunt_state::machine_start()
{
}

void wrlshunt_state::machine_reset()
{
}

void wrlshunt_state::mem_map(address_map &map)
{
	map(0x008000, 0x00ffff).rom().region("maincpu", 0x10000);
	map(0x000000, 0x007fff).m(m_spg, FUNC(spg_wh_device::map));
	map(0x040000, 0x07ffff).rom().region("maincpu", 0x106f000);
}


/************************************
 *
 *  Inputs
 *
 ************************************/

static INPUT_PORTS_START( wrlshunt )
INPUT_PORTS_END


/************************************
 *
 *  Machine Configs
 *
 ************************************/

void wrlshunt_state::wrlshunt(machine_config &config)
{
	UNSP12(config, m_maincpu, XTAL(27'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &wrlshunt_state::mem_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("spg", FUNC(spg_wh_device::screen_update));
	m_screen->screen_vblank().set(m_spg, FUNC(spg_wh_device::vblank));

	SPG_WH(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	m_spg->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_spg->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
}


/************************************
 *
 *  ROM Loading
 *
 ************************************/

ROM_START( wrlshunt )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "wireless.bin", 0x0000, 0x8000000, CRC(a6ecc20e) SHA1(3645f23ba2bb218e92d4560a8ae29dddbaabf796) )
ROM_END

// valid looking code, but extended periperhal area (twice the size?)
CONS( 2011, wrlshunt, 0, 0, wrlshunt, wrlshunt, wrlshunt_state, empty_init, "Hamy / Kids Station Toys Inc", "Wireless Hunting Video Game System", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
