// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

	Driver for "Stop", a screenless coinop machine from Spain (unknown
	manufacturer and date).

	The RAM holds no valid data at power up, so the game has to be set up
	before it can be played. In MAME:

	  1. hold Stop (button 1) and Coin 1 down;
	  2. reset the machine with F3 while both are held;
	  3. release Stop, keeping Coin 1 held;
	  4. release Coin 1.

	The machine answers with a long buzz and restarts ready to play. Until
	then the four figures of the big display all address the same latch, so
	one digit flickers through hexadecimal letters while the other three
	stay dark. Releasing Stop at step 3 with the coin input idle enters a
	bookkeeping mode instead, where the button steps through the coin
	counter and the five prize counters, and the last screen clears the
	payout counter.

	One coin buys five games. A four digit counter, each digit built from
	28 leds, runs freely on the marquee until the player presses the only
	button or the time runs out. The aim is to stop it as close as possible
	to the target number rolled on the smaller seven segment display, but
	without going over it. The closer the result, the higher a light climbs
	the row of lamps on the cabinet front and the bigger the prize won.
	Prizes are charged to a payout counter fed by the coins taken, and when
	there is not enough left the program nudges one digit of the counter so
	that the player just misses. The single seven segment digit next to the
	target display shows the remaining games.

	Four PCBs:

   _____________________________________________________________
  | _____________                                              |
  || EPROM      |     __________  __________                   |
  ||____________|    |GD74LS373| |TC4011BP_|                   |
  |                                                            |
  | _____________     ______________________  _____            |
  ||TC5517CPL-15|    |  SCN8031HCCN40      | |Xtal|    ______  |
  ||____________|    |_____________________| 8.000 MHz MC7805CT|
  |                        __________________                  |
  |_______________________|      DB25       |__________________|
						  |_________________|
   CPU PCB

   __________________________________________________________
 _|__ ····················································· |  Each digit (28 leds)
|   | · __________ · __________ · __________ · __________ · |      o o o o
|   | ·|_ULN2803A| ·|_ULN2803A| ·|_ULN2803A| ·|_ULN2803A| · |     o       o
|   | ····················································· |     o       o
|   | · _________  · _________  · _________  · _________  · |     o       o
|___| · |MC14495P| · |MC14495P| · |MC14495P| · |MC14495P| · |     o       o
|   | ····················································· |      o o o o
|   |   __________             ____                         |     o       o
|   |  |GD74LS154|             LM386     [22K volume trim]  |     o       o
|___|                                                       |     o       o
  |_________________________________________________________|     o       o
 4 digits display PCB                                              o o o o

 ___________________________
|                _________ |
| ...........   |MC14495P| |
| ·  4 x    ·    _________ |
| · 7 seg   ·   |MC14495P| |
| · display ·    _________ |
| ·         ·   |MC14495P| |
| ·         ·    _________ |
| ...........   |MC14495P| |
|        ______            |
|        7805CT            |
| ...........   _________  |
| · 1 x 7seg·  |MC14495P|  |
| · display .              |
|   ___________________    |
|__|       DB25       |____|
   |__________________|
 4 + 1 (7 segments) digits display PCB

   ______________________________________________
 _|__            |ooooooooo|ooooooooo|          |
|   |     ____                                  |
|   |    |   |     _________      _________     |
|   |   74LS154   |ULN2803A|     |ULN2803A|     |
|   |    |   |     ________  ________  ________ |
|   |    |___|    |74LS04N| |74LS04N| |74LS04N| |
|___|  _____                                    |
  |___|oooo|____________________________________|
 Lamp driver PCB

	The ROM contains the string "Programa desarrollado y realizado por
	Victoriano Angel Martinez Sanchez TORREJON DE ARDOZ (Madrid)".

	The five prize levels stop the climbing light on lamps 3, 6, 9, 12 and
	15, from the smallest prize to the exact hit, and the animation repeats
	until the player presses Stop again.

	TODO:
	- Lamp order on the cabinet front not verified.

***************************************************************************/

#include "emu.h"

#include "cpu/mcs51/i8051.h"
#include "sound/spkrdev.h"

#include "speaker.h"

#include "stop.lh"


namespace
{

class stop_state : public driver_device
{
public:
	stop_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_digits(*this, "digit%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
	{
	}

	void stop(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	static constexpr u8 MC14495_SEGMENTS[16] = {
		0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
		0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71
	};

	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	void mux_w(u8 data);
	void ctrl_w(u8 data);
	void update_outputs();

	required_device<i8031_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	output_finder<16> m_digits;
	output_finder<16> m_lamps;

	u8 m_mux = 0xff;
	u8 m_ctrl = 0xff;
};


void stop_state::machine_start()
{
	save_item(NAME(m_mux));
	save_item(NAME(m_ctrl));
}

void stop_state::mux_w(u8 data)
{
	m_mux = data;

	update_outputs();
}

void stop_state::ctrl_w(u8 data)
{
	m_ctrl = data;

	update_outputs();
	m_speaker->level_w(BIT(data, 4));
}

void stop_state::update_outputs()
{
	if (!BIT(m_ctrl, 0))
		m_digits[m_mux >> 4] = MC14495_SEGMENTS[m_mux & 0x0f];

	for (int i = 0; i < 16; i++)
		m_lamps[i] = !BIT(m_ctrl, 1) && i == (m_mux & 0x0f);
}


void stop_state::program_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("maincpu", 0);
}

void stop_state::data_map(address_map &map)
{
	map(0x1000, 0x17ff).ram(); // TC5517CPL-15
}


INPUT_PORTS_START(stop)
	PORT_START("IN0")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Stop")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void stop_state::stop(machine_config &config)
{
	// basic machine hardware
	I8031(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &stop_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &stop_state::data_map);
	m_maincpu->port_out_cb<1>().set(FUNC(stop_state::mux_w));
	m_maincpu->port_in_cb<3>().set_ioport("IN0");
	m_maincpu->port_out_cb<3>().set(FUNC(stop_state::ctrl_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START(stop)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("m2732.bin", 0x0000, 0x1000, CRC(a596988b) SHA1(1b1a2028b6c4644cc942e46fad764b34173d25d0))
ROM_END

} // anonymous namespace


//     YEAR  NAME  PARENT MACHINE INPUT CLASS       INIT        ROT   COMPANY      FULLNAME FLAGS                  LAYOUT
GAMEL( 19??, stop, 0,     stop,   stop, stop_state, empty_init, ROT0, "<unknown>", "Stop",  MACHINE_SUPPORTS_SAVE, layout_stop )

