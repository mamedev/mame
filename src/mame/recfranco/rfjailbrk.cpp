// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

 Skeleton driver for Recreativos Franco slot games on PCB "53396607".
                  __________________________         _________________         __________________________
       _________|          J1             |________|     J5         |________|          J2             |_____
      |         |_________________________|        |________________|        |_________________________|    |
      |                                                                                                     |
      |          4116R-1    4116R-1     4116R-1                       4116R-1  ___ ULN2803A   ULN2803A ___  |
      |    ___    |  |   ___  |  |  ___  |  |  ___  ___  ___  ___  ___  |  |  |  |  |  |   ___  |  |  |  |  |
      |   |  |    |  |  |  |  |  | |  |  |  | |  | |  | |  | |  | |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
      |   |__|    |__|  |__|  |__| |__|  |__| |__| |__| |__| |__| |__|  |__|  |__|  |__|  |__|  |__|  |__|  |
      | 74HC4052D     74HC4052D  74HC4052D           5xH11AA1       ___      ULN2803A   74HC4052D   ULN2803A|
      |                      ___   ___   ___   ___                 |  |                                     |
      |                     |  |  |  |  |  |  |  |                 |__<-74HC4052D                           |
      | _______   _______   |__|  |__|  |__|  |__|               _______    _______    _______    _______   |
      ||OKI   |  |OKI   |    ___   ___   ___   ___<- 8x         |OKI   |   |OKI   |   |OKI   |   |OKI   |   |
      ||M82C55A  |M82C55A   |  |  |  |  |  |  |  | H1N202CBN    |M82C55A   |M82C55A   |M82C55A   |M82C55A   |
      ||______|  |______|   |__|  |__|  |__|  |__|              |______|   |______|   |______|   |______|   |
      | ___     ___                                                                            ___   ___    |
      ||SW1    |SW2                                                                           |SW4  |SW3    |
      ||__|    |  |            _____    _____    _____    _____    _____               ___    |  |  |  |    |
      |        |__|           |OKI |   |OKI |   |OKI |   |OKI |   |OKI |   ··   ··    |  |    |__|  |__|    |
      |          __________   M82C51A  M82C51A  M82C51A  M82C51A  M82C51A JMP11 JMP12 |__<-74HC139M         |
      |  Xtal   N80C188XL20   ..  ___  ..  ___  ..  ___  ..  ___  ..  ___     __________                    |
      | 24 MHz  |         | 74HC4024D 74HC4024D 74HC4024D 74HC4024D 74HC4024D| OKI     |   ___   ___        |
      |         |         |   .. |__|  .. |__|  .. |__|  .. |__|  .. |__|    | M6379   |  |  |  | <-LM380N  |
      | ___ ___ |_________|   ..       ..       ..       ..       ..         |_________| H11AA1 |__|        |
74LS20->  ||  |              JMP3     JMP4     JMP8     JMP9     JMP10  ___                                 |
      ||__||__|  JMP1->..  ..  ..         _____     _____     ___      |SW5      JMP5 ..  JMP6 ..  JMP7 ..  |
      |   74LS04       .. JMP2 JMP13     |CS82C59A |CS82C59A |HC138A   |  |           ..       ..       ..  |
      |  ___  ___  _____   _____   _____ |____|    |____|    |__|      |__|        _____   _____   _____    |
      | |  | |  | |EPROM  |EPROM  DS1644      Xtal               _______          |U39 |  |U40 |  |U41 |    |
      | |  | |  | |H   |  |L   |  |    |  ___2.4576 ___         |OKI   |          |EMPTY  |EMPTY  |EMPTY    |
      | |__| |__| |    |  |    |  |    | |  | MHz   |  |        |M82C55A          |    |  |    |  |    |    |
      2x74HC573AN |    |  |    |  |    | |__|       |__|  ·  ·  |______|          |    |  |    |  |    |    |
      |           |____|  |____|  |____|74HC4060  74HC74  J3 J4                   |____|  |____|  |____|    |
      |_________________________________________________test_test___________________________________________|

-JMP3, JMP4, JMP8, JMP9, and JMP10 are for setting up the baud rate of each OKI M82C51A USART (you can
 configure 1200, 2400, 4800, 9600, 19200 or 38400 bps.

****************************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "sound/okim6376.h"
#include "speaker.h"

namespace {

class rfjailbrk_state : public driver_device
{
public:
	rfjailbrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
	{
	}

	void rfjailbrk(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<i80188_cpu_device> m_maincpu;
	required_device<okim6376_device> m_oki;
};

void rfjailbrk_state::machine_start()
{
}

static INPUT_PORTS_START(rfjailbrk)
INPUT_PORTS_END

void rfjailbrk_state::rfjailbrk(machine_config &config)
{
	I80188(config, m_maincpu, 24_MHz_XTAL); // Intel N80C188XL20

	I8255A(config, "pia2"); // OKI M82C55A-2V
	I8255A(config, "pia1"); // OKI M82C55A-2V
	I8255A(config, "pia7"); // OKI M82C55A-2V
	I8255A(config, "pia3"); // OKI M82C55A-2V
	I8255A(config, "pia5"); // OKI M82C55A-2V
	I8255A(config, "pia6"); // OKI M82C55A-2V

	PIC8259(config, "pic1", 0); // CS82C59A
	PIC8259(config, "pic2", 0); // CS82C59A

	// Sound hardware

	SPEAKER(config, "mono").front_center();

	OKIM6376(config, m_oki, 2.4576_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 1.0); // Guess
}

// Only one PCB found, probably the game uses more PCBs for reels, etc.
ROM_START(rfjailbrk)
	ROM_REGION(0x090000, "maincpu", 0)
	ROM_LOAD("jail_break_high_0322b.eprom_h", 0x00000, 0x80000, CRC(93bf9b55) SHA1(3296099b768b26c750a2164678b3585374c93a6d))
	ROM_LOAD("jail_break_low_0322c.eprom_l",  0x80000, 0x10000, CRC(737291dc) SHA1(ce95acf88eda6a1e151d42dfdd7c69b27dc0a9e3)) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION(0x180000, "oki", 0)
	// Three sockets, all of them unpopulated on the found PCB
	ROM_LOAD("rfjailbrk_sound.u39", 0x000000, 0x080000, NO_DUMP)
	ROM_LOAD("rfjailbrk_sound.u40", 0x080000, 0x080000, NO_DUMP)
	ROM_LOAD("rfjailbrk_sound.u41", 0x100000, 0x080000, NO_DUMP)

	ROM_REGION(0x2000, "nvram", 0)
	ROM_LOAD("ds1644.ram", 0x0000, 0x2000, CRC(dc9cb822) SHA1(7e3ab8c0ad49a8e0efd66a2061b881d51877bc75))
ROM_END

} // anonymous namespace

//   YEAR  NAME       PARENT  MACHINE    INPUT      CLASS            INIT        ROT   COMPANY               FULLNAME                                        FLAGS
GAME(2000, rfjailbrk, 0,      rfjailbrk, rfjailbrk, rfjailbrk_state, empty_init, ROT0, "Recreativos Franco", "Jail Break (Recreativos Franco slot machine)", MACHINE_IS_SKELETON_MECHANICAL)
