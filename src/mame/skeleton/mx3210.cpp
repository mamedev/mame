// license:BSD-3-Clause
// copyright-holders:
/************************************************************************************************************

    Skeleton driver for Xyplex MAXserver 3210 Local Router

    PCB for model MX-3210-001:
      _________________________________________________________________________________________________________
     |                                                                    __________________   __________    __|__
     |                                                                    |  AM7990PC/80    |  |HALO TD01|  |     |
     |                                                                    |_________________|               |     |
     |                                                                                     __________       |     |
     |                                                                                     |AM7992BPC|      |_____|
     |                                                                    __________________       Xtal        |
     |                                                                    |  AM7990PC/80    |     20.000 MHz   |
     |                                                                    |_________________|            FUSE  |
     |                                                                    ___________          __________      |
     |                                                                    |_PAL_U66__|         |HALO TD01|   __|__
     |                                                                                                      |     |
     |                                                                                                      |     |
     |                                                                                     __________       |     |
     |                                                                    ___________      |AM7992BPC|      |_____|
     |                                                                    |_PAL_U67__|            Xtal         |
     |                                                            Floppy conn->:::::::::::::::   20.000 MHz    |
     |                                                     _________  Xtal                                     |
     |                                                  FDC37C65CLJ P  ?? ___________   ___________            |
     o<-LED                                                |        |     |_74ALS574_|  |74BCT543NT|           |
     |                                                     |        |                                          |
     o<-LED                                                |________|     ___________   ___________            |
     |                                    ____________                    |_74ALS574_|  |74BCT543NT|           |
     o<-LED                               |MS7201AL-80PC                                                       |
     |                                    |__________|      ___________   ___________   ___________            |
     o<-LED                                                 |_PAL_U40__|  |_74ALS574_|  |74BCT543NT|           |
     |                                                                                                         |
    [ ]<-SWITCH                                             ___________   ___________   ___________            |
     |                                                      |_GAL_U41__|  |_74ALS574_|  |74BCT543NT|           |
LED->o                                                                                                         |
     |      ___________   ___________   ___________   ___________                                              |
LED->o     |74HCT259N_|  |_PAL_U25__|  |_9412H_A__|  |74ALS138N_|                                              |
     |                                                                                                         |
LED->o      ___________   ___________   ___________   ___________    ___________                               |
     |     |_74AS289AN|  |_74AS289AN|  |_74AS804AN|  |74ALS245AN|   |MC74F153N_|                               |
LED->o                                                                                                         |
     |      ___________   _______________             ___________    ___________                               |
LED->o     |_74AS289AN|  |              |            SN74BCT543NT   |MC74F153N_|                               |
     |                   | MC68020RP20E |                                                                      |
     |                   |              |             ___________    ___________   ___________   ___________   |
     |      ___________  |              |            |74ALS245AN|   |MC74F153N_|  |_GAL_U83__|  |_74ALS848N|   |
     |     |74HCT259N_|  |              |                                                                      |
     |                   |______________| ___________  ___________   ___________                               |
LED->o                                   |74ALS138AN| |74HCT259N_|  |MC74F153N_|                               |
     |   ______________                                                                                        |
LED->o  | EPROM U13   |                   ___________  ___________   ___________                               |
     |  |_____________|                  |74ALS138AN| |74ALS573CN|  |MC74F153N_|                               |
LED->o                                                                                                         |
     |      ___________   ___________                  ___________   ___________                               |
LED->o     |74HCT259N_|  |_GAL_U28__|                 |74ALS139N_|  |_SN74F374N|                               |
     |                                                                                                         |
LED->o      ___________   ___________   ___________    ___________   ___________                               |
     |     |__DS1234__|  | ACTEL    |  | CA82C55A |   |_GAL_U49__|  |_PAL_U59__|                               |
     |                   |          |  |          |                                                            |
     |      ___________  |          |  |          |    ___________   ___________                               |
     |     |RTC_72421B|  |          |  |__________|   |_GAL_U50__|  |_GAL_U60__|                               |
     |                   |__________|                                                                          |
     |                                                 ___________   ___________   Xtal                        |
     |                                                |_GAL_U51__|  |_GAL_U61__|  40.000 MHz                   |
     |                                                                                                         |
     |    BATT       _______   Xtal   ________   ___________   ___   ___________    : :                        |
     |              |CA82C52|   ??   |CA82C54|  |_74HCT259N| 93C06N |_74F5074N_|    : :                        |
     |              |_______|        |_______|                                                                 |
     |____________________________________________________________________________                           __|_
                                                                                  |        ________          |   |
                                                                                  |       MC145406P          |___|
                                                                                  |____________________________|
*/

#include "emu.h"

#include "cpu/m68000/m68020.h"

#include "machine/am79c90.h"
#include "machine/upd765.h"

//#include "imagedev/floppy.h"

namespace {

class mx3210_state : public driver_device
{
public:
	mx3210_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		//, m_floppy(*this, "floppy")
	{
	}

	void mx3210(machine_config &config);

private:
	required_device<m68020_device> m_maincpu;
	required_device<upd765_family_device> m_fdc;
	//required_device<floppy_connector> m_floppy;
};

static INPUT_PORTS_START(mx3210)
INPUT_PORTS_END

void mx3210_state::mx3210(machine_config &config)
{
	M68020(config, m_maincpu, 40_MHz_XTAL/2);

	WD37C65C(config, m_fdc, 20_MHz_XTAL/2, 20_MHz_XTAL/2); // FDC37C65CLJ-P, unknown clock
	//FLOPPY_CONNECTOR(...)

	AM7990(config, "lance1", 0); // AMD AM7990PC/80
	AM7990(config, "lance2", 0); // AMD AM7990PC/80
}

ROM_START(mx3210)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("sc_4315-b.u13",     0x000000, 0x010000, CRC(9b934ca9) SHA1(152240f0d468783f26854bc491fcfad0ced9d9fd))

	ROM_REGION(0x168000, "floppy", 0)
	ROM_LOAD("mx3210_floppy.img", 0x000000, 0x168000, CRC(b215e9d1) SHA1(7b18cc81a7c6f6e44cfc804a7ba6c7fc0c9bbb92))

	ROM_REGION(0x200, "plds", 0)
	ROM_LOAD("atm_4238_b_palce20v8h-25pc-4.u59", 0x000, 0x157, CRC(ddc38476) SHA1(ab4f20c0f15521febb6f66a6272fff0ea63920c5))
	ROM_LOAD("atm_4242_a_palce16v8h-25pc-4.u40", 0x000, 0x117, CRC(c40e26a3) SHA1(c72d88f6218fc292a0680a8dd8782957a768f605))
	ROM_LOAD("m5v_4233_a_gal20v8a.u41",          0x000, 0x157, CRC(d3a0317e) SHA1(e0377fbaf2a802f235540eac8556edbd492e0e5c))
	ROM_LOAD("m5v_4240_b_gal20v8a.u83",          0x000, 0x157, CRC(adfbd1fb) SHA1(b7ad2cac2c205be751d6ffa98ea2de467df5936e))
	ROM_LOAD("m5v_4308_a_tibpal20l8-7.u25",      0x000, 0x144, CRC(217e7c63) SHA1(acdbc4c8bfba99d5170ccc1c7ee87aed35b5b0f8))
	ROM_LOAD("m5y_4236_a_gal16v8b.u50",          0x000, 0x117, CRC(f696922a) SHA1(528051a221d4c2502502e3f92c6d25089e99c8cb))
	ROM_LOAD("sc_4239-c_palce16v8h-15pc-4.u66",  0x000, 0x117, CRC(985c5117) SHA1(cd00458bf1d4c600e9e672ff955049ffc160975e))
	ROM_LOAD("sc_4239-c_palce16v8h-15pc-4.u67",  0x000, 0x117, CRC(985c5117) SHA1(cd00458bf1d4c600e9e672ff955049ffc160975e)) // Same as U66
	ROM_LOAD("sc_4241-a_gal16v8b.u51",           0x000, 0x117, CRC(1fb46b28) SHA1(5540631603960d50f5fb7593f1fd390d8415f8c2))
	ROM_LOAD("sc_4243-a_gal16v8b.u49",           0x000, 0x117, CRC(7e574a22) SHA1(b6d8ddaa490cafccffb8274c2d242725ce59d911))
	ROM_LOAD("sc_4276-a_palce16v8h-25pc-4.u61",  0x000, 0x117, CRC(3ac53a84) SHA1(6611a979803ad537e7c509b9b534b5d94af7e265))
	ROM_LOAD("sc_4318-a_gal16v8b.u28",           0x000, 0x117, CRC(0332f842) SHA1(299ecd64c4e702e5f0d3b0279353ea40995316dd))
	ROM_LOAD("sc_4321-a_gal20v8a.u60",           0x000, 0x157, CRC(2fb25010) SHA1(32d8a07728f2a7e82112279de845a65bc49dad78))
ROM_END

} // anonymous namespace

SYST(1994, mx3210, 0, 0, mx3210, mx3210, mx3210_state, empty_init, "Xyplex Inc.", "MAXserver MX-3210 Local Router", MACHINE_IS_SKELETON)
