// license:BSD-3-Clause
// copyright-holders:

/*
   Skeleton driver for Blaupunkt CDS 32-ID terminal with colour screen.
   More hardware inforation and photos:  https://www.oldcomputers.es/terminal-blaupunkt-cds-32-id/

   Hardware setup:

 Printer PCB (silkscreened as "BLAUPUNKT  BTX-DRUCKER-INTERFACE  TYP 8668 305 585")
    ____________________________________________________________________________
   |                    ___________   ___________   ___________   ___________  |
 __|__                 |CONN_MAIN_|  |SN74ALS174N  |SN74LS164N|  |SN74ALS161BN |
|     | ___________   ___________    ___________    ___________                |
|DB15 ||SN74LS367AN  |SN74LS02N_|   |SN74LS244N|   |SN74LS374N|                |
| FEM |               __________________________    ________________           |
|     | ___________  | Motorola MC6821P        |   | NEC D4016C-3  |           |
|     ||SN74LS367AN  |                         |   |               |           |
|_____|              |_________________________|   |_______________|           |
   |        Xtal      __________________________    ___________   ___________  |
   |_     400 kHz    | Motorola MC6802P        |   |SN74LS161AN  |SN74AS04N_|  |
   - |               |                         |    ___________   ___________  |
   -_|<- Dips x 4    |_________________________|   |SN74LS161AN  |SN74AS02N_|  |
   |    ___________   ________________              ___________   ___________  |
   |   |_MC14069U_|  | EPROM         |             |SN74LS161AN  |SN74LS00N_|  |
   |                 |_______________|                                         |
   |___________________________________________________________________________|

Main PCB
   _____________________________________________________________________________
  |  _______________   _______________                            ___________  |
  | | NEC D4016C-3 |  | NEC D4016C-3 |                           |PC74HCT367P  |
  | |______________|  |______________|                             __________  |
  |  ___________       ___________                                CONN PRINTER |
  | |CD74HCT365E      |CD74HCT373E         Xtal 6 MHz                          |
  |  ___________       ___________   ____________________                      |
  | |CD74HCT365E      |CD74HCT373E  | M4613D/A          |                      |
  |  ____________      ___________  | 4782 DUG8629      |                      |
  | |CD74HCT373E|     |CD74HCT365E  |___________________|       _____________  |
  |  ___________       ___________   ___   ______________      | ASTEC       | |
  | |CD74HCT86E|      |_N82S153N_|  |  |  | NMC9816AN-25|      | AD1D12A10   | |
  |  _____________________   Xtal   |  |  |_____________|      |_____________| |
  | | MAB 8031AH 12P     | 11.0592  |_<-CD74HCT245E______                      |
  | |                    |   MHz          | D4016C-3    |                      |
  | |____________________|                |_____________|                      |
  |                                        ______________                      |
  |                                       | EPROM       |                      |
  |                                       |_____________|                      |
  |                                                                            |
  |____                                                                        |
      |                   -----------------------------------------            |
      |                   -------------- V24 INTERFACE ------------            |
      |_______________________      ___      ___      ___      ________________|
                              |____|   |____|   |____|   |____|
                             Keyboard   Tape      CVS     Modem

V24 Interface riser PCB
   ____________________________________________________________
  |  ___________   ___________    ___________    ___________  |
  | |_SN75189N_|  |_SN75188N_|   |PC74HCT00P|   |PC74HCT157P  |
  |_____________________                                      |
                        |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|

Keyboard PCB
  ___________________________________________________________________________________________
 |  _____________   ___________   ___________________   ____________                        |
 | |CD74HCT4514E|  |_74LS244N_|  | M5L8039P-11      |  | EPROM     |   ___________          |
 | |____________|    Xtal        |__________________|  |___________|  |PC74HCT373P          |
 |________________ 9.216 MHz _______________________________________________________________|

*/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/mcs51/mcs51.h"

#include "machine/6821pia.h"

namespace {

class blaucds32_state : public driver_device
{
public:
	blaucds32_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void blaucds32(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void blaucds32_state::machine_start()
{
}

void blaucds32_state::machine_reset()
{
}

static INPUT_PORTS_START( blaucds32 )
	// Printer interface config
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
INPUT_PORTS_END

void blaucds32_state::blaucds32(machine_config &config)
{
	I8031(config, m_maincpu, 11.0592_MHz_XTAL);

	// Printer interface
	M6802(config, "printercpu", 400_kHz_XTAL); // Motorola MC6802P
	PIA6821(config, "pia"); // Motorola MC6821P

	// Cherry 601-1415 keyboard (TODO: Convert to device)
	I8039(config, "kbdc", 9.216_MHz_XTAL); // Mitsubishi M5L8039P-11
}

ROM_START( blaucds32 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v4292_b0147-00.v4292", 0x00000, 0x10000, CRC(a25d8966) SHA1(72298dd4f3d8bb333c545fec2683bcae4bb18e26) )

	ROM_REGION( 0x2000, "printercpu", 0 )
	ROM_LOAD( "v5270_b0141-01.v270",  0x00000, 0x02000, CRC(95943b74) SHA1(4554b796b75c6790d07dc4bf5278df00f00b6804) )

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "426.bin",              0x00000, 0x00800, CRC(0465f6a7) SHA1(14d9d9ae58baad2f7ccbf1f35ef8599e32ec1ed1) )

	ROM_REGION( 0x0eb, "prom", 0 )
	ROM_LOAD( "n82s153n.v4245",       0x00000, 0x000eb, NO_DUMP ) // On main PCB
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY      FULLNAME     FLAGS
COMP( 19??, blaucds32, 0,      0,      blaucds32, blaucds32, blaucds32_state, empty_init, "Blaupunkt", "CDS 32-ID", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
