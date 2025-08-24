// license:BSD-3-Clause
// copyright-holders:

/***************************************************************************

 ETRA (https://www.grupoetra.com/) semaphore controller for a crosswalk
 (unknown model, unknown year).

Main PCB
  __________________________________________________
 |     _______    _______    _______                |
 |    |      |   |      |   |      |                |
 |    |______|   |______|   |______|                |
 |                           _________    _________ |
 |                          |_74LS03_|   DM74LS122N |
 |              _________                           |
 |             |_UM6114_|    _________    _________ |
 |                          |T74LS04B1   |_74LS90_| |__
 |                                                   __|
 |              _________    _________               __|
 |             |_UM6114_|   DM74LS155N    _________  __|
 |       ________________                DM74LS155N  __|
  \     | X2816CP-12    |  Xtal                      __|
  _\    |_______________|  4.000 MHz                 __|
 |__     ________________    ________________        __|
 |__    | AT2716 EPROM  |   | MC6802P       |        __|
 |__    |_______________|   |_______________|        __|
 |__                                                 __|
 |__     _________           ________________        __|
 |__    |_74LS156|          | MC6821P       |       |
 |__                        |_______________|       |
 |__                                                |
 |__     _________    _________    _________        |
 |__    |________|   |_74LS132|   |_74LS90_|        |
   |                                                |
   |________________________________________________|

Relays PCB
           _________________________________________
          |                                         |
          |                                         |
          |                        _________        |
          |                       |74LS122N|        |
          |                 ________________        |
         /                 | MC6821P       |        |
        /                  |_______________|        |
       /                                            |__
  ____/                   _______ _______            __|
 |       _______          MOC3020 MOC3020            __|
 |      TXAL2215B         _______ _______   _______  __|
 |       _______          MOC3020 MOC3020  |_7404N|  __|
  \     TXAL2215B         _______ _______   _______  __|
   \     _______          MOC3020 MOC3020  |_7404N|  __|
   |    TXAL2215B         _______ _______   _______  __|
   |     _______          MOC3020 MOC3020  |_7404N|  __|
   |    TXAL2215B         _______ _______            __|
   |     _______          MOC3020 MOC3020            __|
   |    TXAL2215B         _______ _______            __|
   |     _______          MOC3020 MOC3020           |
   |    TXAL2215B                                   |
   |                                                |
   |                                                |
   |                                                |
   |                                                |
   |________________________________________________|

Programmer PCB (keyboard)
    _______________________________________________________
   |  ______ ______ ______ ______ ______ ______           |
   | | ___ || ___ || ___ || ___ || ___ || ___ |           |
   | ||__| |||__| |||__| |||__| |||__| |||__| |  ___      |
   | ||__| |||__| |||__| |||__| |||__| |||__| | |  |      |
   | |_____||_____||_____||_____||_____||_____| |  |<-7407N
   |                                            |__|      |
   |       __________________________________             |
   |      | ____   ____   ____   ____   _   |             |
   |      || R |  | M |  | N |  | K |  (_)  |             |
   |      ||___|  |___|  |___|  |___|       |             |
   | ___  | ____   ____   ____   ____       |      SWITCH |
   ||  |  || 0 |  | 1 |  | 2 |  | 3 |   __  |             |
   ||  |  ||___|  |___|  |___|  |___|  (||) |             |
   ||__|  | ____   ____   ____   ____       |             |
 74LS155N || 4 |  | 5 |  | 6 |  | 7 |   __  |             |
   |      ||___|  |___|  |___|  |___|  (||) |             |
   | ___  | ____   ____   ____   ____       |    ___      |
   ||  |  || 8 |  | 9 |  | A |  | B |   __  |   |  |      |
   ||  |  ||___|  |___|  |___|  |___|  (||) |   |  |<-SN74LS03N
   ||__|  | ____   ____   ____   ____       |   |__|      |
SCL4052BE || C |  | D |  | E |  | F |       |    ___  ___ |
   |      ||___|  |___|  |___|  |___|       |   |  |<-TC4093BP
   | ___  |_________________________________|   |  | |  | |
   ||  |                                        |__| |__|<-7407N
   ||  |<-CD4093BE                                        |
   ||__|   ____  ____  ____  ____  ____  ____  ____  ____ |
   |       4N32  B250  4N32  B250  4N32  B250  4N32  B250 |
   |            C1000       C1000       C1000       C1000 |
   |                               __________             |
   |                              |  CONN   |             |
   |______________________________________________________|

Notes from one operator that used to work with this controller model:
 For programming the semaphore controller, you just put the memory values with the keyboard.
 From 100 to 200 you'll find the first program, from 200 to 300 the second, and so on up to
 seven programs, with 100 for green, 101 for yellow, 102 for clear, and then repeat it again.

***************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"


namespace {

class semcrossw_state : public driver_device
{
public:
	semcrossw_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia(*this, "pia%u", 1U)
	{
	}

	void semcrossw(machine_config &config);

private:
	// devices/pointers
	required_device<m6802_cpu_device> m_maincpu;
	required_device_array<pia6821_device, 2> m_pia;
};

static INPUT_PORTS_START(semcrossw)
INPUT_PORTS_END

void semcrossw_state::semcrossw(machine_config &config)
{
	M6802(config, m_maincpu, XTAL(4'000'000));

	PIA6821(config, m_pia[0]);

	PIA6821(config, m_pia[1]);
}

ROM_START(semcrossw)
	ROM_REGION(0x800, "maincpu", 0)
	ROM_LOAD("at27c16.bin",    0x000, 0x800, CRC(2e7b10b1) SHA1(fba6465db1baa38ab79ed24a85de460f8be488b9))

	ROM_REGION(0x800, "eeprom", 0)
	ROM_LOAD("x2816cp-12.bin", 0x000, 0x800, BAD_DUMP CRC(c2ef2e80) SHA1(6c3c4215169c2941a37053888174fe0499301bac)) // BAD_DUMP because dumped from an already configured machine
ROM_END

} // anonymous namespace


//   YEAR  NAME       PARENT COMPAT MACHINE    INPUT      CLASS            INIT        COMPANY  FULLNAME                                              FLAGS
SYST(19??, semcrossw, 0,     0,     semcrossw, semcrossw, semcrossw_state, empty_init, "Etra",  "Crosswalk traffic light controller (unknown model)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
