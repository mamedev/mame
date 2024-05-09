// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************************************

    Skeleton driver for:
    "Ganbare Momotarou Oni Taiji" (がんばれ ももたろう おにたいじ). Electromechanical arcade by Shoken.

    DISPLAY PCB
     ______________________________________________________________________________________________
    |__               __________________    _________________    _____________    _____________   |
    ||:|             |·················|   |················|   |············|   |············|   |
    ||:|  ___________   ___________   ___________   ___________   ___________         ___________ |
    ||:| |TD62783AP_|  |_ULN2803A_|  |_ULN2803A_|  |_ULN2803A_|  |_ULN2803A_|        |JW25N-DC12V |
    ||:|                                                                             |___________||
    ||:|  ___________   ___________   ___________   ___________   ___________     ___________     |
    ||_| |TD62783AP_|  |_ULN2803A_|  |_ULN2803A_|  |_ULN2803A_|  |_ULN2803A_|    |JW25N-DC12V   __|
    |                                                                            |___________| |:||
    |     ___________      ___________              ___________                   ___________     |
    |    |SN74LS273N|     |LC3517BSL_|             |SN74LS138N|                  |JW25N-DC12V     |
    |      ________     ______________                                           |___________|  __|
    |     | Xtal  |    | EPROM       |                                            ___________  |:||
    |     | 20.000 MHz |_____________|                                           |JW25N-DC12V  |:||
    |     ___________  _________________           ___________   ___________     |___________|    |
    |    |SN74LS74AN|  | Z08040004PSC  |          |_74LS245N_|  |_74LS240N_|      ___________     |
    |                  |_______________|            ___________  ___________     |JW25N-DC12V     |
    |     ___________   ___________   ___________  |_TPL521-4_| |_TPL521-4_|     |___________|  __|
    |    |74HC4020AP|  |_SN74LS04N|  |_SN74LS02N|                                 ___________  |:||
    |                                                                            |JW25N-DC12V  |·||
    |                             SHOKEN M005 DISPLAY                            |___________|    |
    |                                              _____________   ________   _______________     |
    |                                             |············|  |·······|  |··············|     |
    |_____________________________________________________________________________________________|

    SOUND PCB
     ___________________________________________________________________________________________
    |         ___________   ________   ________      ______      ____________                  |
    |__      |··········|  |·······|  |·······|     |·····|     |···········|                : |
    ||:|                                                                                     : |
    ||:|        _________________     (o)<- Reset switch     ____                            : |
    |          | D8255AC-2      |                           8212CPA            o o o <- LEDs : |
    |          |________________|   _________  _________    _________________                __|
    |__         _________________  SN74LS138N  SN74LS00N   | D8255AC-2      |               |:||
    ||:|       | Z08040004PSC   |   _________  _________   |________________|               |:||
    ||:|       |________________| SN74LS139AN SN74LS138N    ________  _______   _________   |:||
    |       _________   _________   _________  _________   |DIPSx6_| |DIPSx4|  |ULN2803A|      |
    |__    SN74LS245N  SN74LS244N  |SN74LS10N SN74LS74AN    _________________   _________    __|
    ||:|    ___________  _________  _________  _________   | D8255AC-2      |  |ULN2803A|   |:||
    ||:|   | EPROM    |  SN74LS04N  SN74LS04N SN74LS74AN   |________________|               |:||
    |      |__________|  _________  _________  _________    _________________   _________   |:||
    | ____    _________  SN74LS02N  SN74LS32N SN74LS688N   | AY38910A/P     |  |ULN2803A|      |
    | BATT    LC3517BSL             _________  _________   |________________|   _________    __|
    |       _________   _________  SN74LS161AN SN74LS161AN                     | RELAY  |   |:||
    |      |74HC00AP|  SN74LS74AN           ____________            Resonator               |:||
    |  _________   _________   _________   | EPROM     |  _________  _________              |:||
    | SN74LS161AN |TC4020BP|   SN74LS09N   |___________| |TC4020BP| |OKI M5205  _________      |
    |                                                                          |uPC2500H|    __|
    |   Xtal 12.000 MHz        _________   _________      _________                         |:||
    |                         SN74LS161AN SN74LS161AN    |_LM324N_|    SHOKEN M904-A        |:||
    |__________________________________________________________________________________________|

*/

#include "emu.h"

#include "cpu/z80/z80.h"

#include "sound/ay8910.h"
#include "sound/msm5205.h"

#include "speaker.h"

namespace {

class ganbaremo_state : public driver_device
{
public:
	ganbaremo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_ay8910(*this, "aysnd")
		, m_5205(*this, "musicrom")
	{
	}

	void ganbaremo(machine_config &config);

private:
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<ay8910_device> m_ay8910;
	required_device<msm5205_device> m_5205;
};

static INPUT_PORTS_START(ganbaremo)
	PORT_START("DSW0")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
INPUT_PORTS_END

void ganbaremo_state::ganbaremo(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 4); // Guess

	// Sound hardware

	Z80(config, m_audiocpu, 12_MHz_XTAL / 4); // Guess

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay8910, 12_MHz_XTAL / 8); // Guess
	m_ay8910->port_a_read_callback().set_ioport("DSW0");
	m_ay8910->port_b_read_callback().set_ioport("DSW1");
	m_ay8910->add_route(ALL_OUTPUTS, "mono", 0.25);

	MSM5205(config, m_5205, XTAL(384'000));
}

ROM_START(ganbaremo)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("m005_xx.bin", 0x00000, 0x08000, CRC(42a7aa23) SHA1(6140f4a7769ab35cb32e3079adbee6468f3ce880))

	ROM_REGION(0x08000, "audiocpu", 0)
	ROM_LOAD("m005_p5.bin", 0x00000, 0x08000, CRC(dddb97a6) SHA1(47465bb2cd26ddd0f80c729ef3bb3b187b684d97))

	ROM_REGION(0x10000, "musicrom", 0)
	ROM_LOAD("m005_v.bin",  0x00000, 0x10000, CRC(e3cb69a8) SHA1(a49878adae08d56d78d168367659ac322d7fb5eb))
ROM_END

} // anonymous namespace

GAME(19??, ganbaremo, 0, ganbaremo, ganbaremo, ganbaremo_state, empty_init, ROT0, "Shoken", "Ganbare Momotarou Oni Taiji", MACHINE_IS_SKELETON_MECHANICAL)
