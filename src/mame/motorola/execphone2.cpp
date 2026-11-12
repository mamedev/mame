// license:BSD-3-Clause
// copyright-holders:

/*****************************************************************************************

Skeleton driver for Motorola Executive Phone 2 mobile phone.

Double-Sided main PCB. Labeled "8409603N04.C  200-6  93116-1-3".
 ________________________________     ________________________________
|                                |   |                                |
|                                |   |        _______                 |
|                                |   |       |      |   _______       |
|                  Xtal          |   |       |68M02 |  |Motorola      |
|                 114.0          |   | ____  |630   |  |SHW5131       |
|                                |   ||K05|  |      |  |9631  |       |
|                                |   ||629|  |      |  |      |       |
|                 _________      |   |       |      |  |______|       |
|                |Motorola|      |   |       |______|                 |
|                |SC390148|      |   |                                |
|                |32D15   |      |   |                                |
|                |HAG9628 |      |   |                                |
|   _________                    |   |                                |
|  |Motorola|                    |   |                                |
|  |T38     |                    |   |             ____               |
|  |AARA9629|                    |   |            |K06|               |
|  |________|                    |   |            |629|               |
|                                |   |                                |
|  _____   _________   ________  |   |                                |
MC145158-2| EPROM  | Motorola 1994   |                         Xtal   |
|         | 27C1001| SC432098CPB |   |  Xtal                  16.8M   |
|         |________|  | 97C03 |  |   | 3.84M                          |
|                    0F36WllRY9628   |                                |
|                 _______        |   |  --    ________   ________     |
|                Motorola        |   |  --   |Motorola  |Motorola     |
|                SC380036FB      |   |  --   |99T37  |  |SC390207FB   |
|                F94P 85DO5      |   |  --   SC79954FB  |32D24 3054   |
|                AVAV9627        |   |       |HDP9627|  |AVAV9628     |
|                                |   |                                |
|___________          ___________|   |___________          ___________|
            |________|                           |________|

Screen and keyboard on a separate PCB with a Motorola Custom labeled "SC417836CDW  64C09  0F82TICTBN9552".
Screen is a 7-digits 7-segments (without dots) plus lights for "▲", "⊘", "📞", and "⏼" icons on the upper side.

******************************************************************************************/

#include "emu.h"

#include "cpu/mc68hc11/mc68hc11.h"

#include "emupal.h"
#include "speaker.h"


namespace {

class execphone2_state : public driver_device
{
public:
	execphone2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mic(*this, "mic"),
		m_speaker(*this, "speaker")
	{ }

	void execphone2(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<microphone_device> m_mic;
	required_device<speaker_device> m_speaker;

	void program_map(address_map &map) ATTR_COLD;
};

void execphone2_state::program_map(address_map &map)
{
}

static INPUT_PORTS_START(execphone2)
INPUT_PORTS_END

void execphone2_state::execphone2(machine_config &config)
{
	// Actually a Motorola custom, likely a Motorola MC68HC11
	MC68HC11A1(config, m_maincpu, 3.84_MHz_XTAL); // Clock unknown, but near a 3.84 MHz xtal
	m_maincpu->set_addrmap(AS_PROGRAM, &execphone2_state::program_map);

	MICROPHONE(config, m_mic, 1).front_center();
	SPEAKER(config, m_speaker).front_center();
}

ROM_START( execphone2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "a33a00_motorola_v.9550_sci9632ca_34c63_27c1001.bin", 0x00000, 0x20000, CRC(8361099a) SHA1(3deeea8a9429a3bb15cdb8303a5f961c40dd8e44) )
ROM_END

} // anonymous namespace

//    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY     FULLNAME             FLAGS
SYST( 1994, execphone2, 0,      0,      execphone2, execphone2, execphone2_state, empty_init, "Motorola", "Executive Phone 2", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
