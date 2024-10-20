// license:BSD-3-Clause
// copyright-holders:

/***************************************************************************

    Skeleton driver for Unidesa/Cirsa "2080606" slot machines hardware.

    The "2080606" PCB (same as "615182080606") is used at least on the following games:
     - Bingo 7 Deluxe
     - Cat 'n' Mouse
     - Cleopatra
     - El Tesoro de Java
     - FBI Academy
     - Gran Hermano
     - Fruit Express
     - La Granja
     - La Perla del Caribe Plus
     - Las Joyas de Cleopatra
     - Leyendas de Oriente
     - Lucky Player
     - Máquina del Tiempo
     - Multi Star
     - Ocean's Treasures
     - Red Horse
     - Red Horse Light
     - Splash
     - Troya
     - Vikingos
     - Vikingos Video
     - Western
     - Yetimania


    Layout for PCB 615182080606-2 (CPU VIDEO CAN CCTALK):
                                       _________
  ____________________________________|VGA HD15|_________________________________
 |    ·······  ·······  ···· ··· ···  |________|  ····  ____  ____  ____  ___   |
 | ··          ·······       ··· ···             ·····  TEST  DESC  ARR   F.T. <- Four button switches
 |    ·······                                    ____    ____       _________  ·|
 |               ·········                      TJA1050 LV14A      |_8xDIPS_|  ·|
 |                                      ______     _____   ·······  _________  ·|
 |                    ____             |ADV7123   IDL615-3         |_8xDIPS_|  ·|
 |___________        IDT71V124         |______|    _____      ___               |
 ||          |       |   |   _________             LV14A     VB373              |
 || POWER    |       |___|  |XILINX  |                        ___               |
 || SUPPLY   |              |SPARTAN |           _______     VB373    ________  |
 || PCB      |              |XC3S1000|          |XILINX|      ___    |FLASH U6  |
 ||          |              |________|          XC9536XL     VB373   |_______|<-BBOX
 ||          |                                  |______|                        |
 ||          |                                           :::::                  |
 ||          |   ____________________________________________                   |
 ||          |  |__SIMM_BACKGROUND-1________________________|         ________  |
 ||          |   ____________________________________________        |FLASH U5  |
 ||          |  |__SIMM_BACKGROUND-2________________________|        |_______|<-PROGRAMA
 ||          |   ____________________________________________                   |
 ||__________|  |__SIMM_WINDOW-1____________________________|                   |
 |               ____________________________________________                   |
 |              |__SIMM_WINDOW-2____________________________|                   |
 |______________________________________________________________________________|


   Layout for sound PCB 2100125-2 (SONIDO CAN SD), used by most games on this hardware
  _____________________________________________________
 |                                                    |
 | ·                                                  |
 | ·                       ______                    ·|
 | ·                      STA559BW                   ·|
 |     _____              |     |                    ·|
 | ·  TJA1050             |_____|                     |
 | ·                                                  |
 | ·           ________                               |
 |            STM32F103                               |
 | ·          |       |                               |
 | ·          |_______|                               |
 | ·  _______   Xtal                                  |
 |   |MicroSD   8 MHz                                 |
 |   | Slot |           Volume                        |
 |   |______|  ···· <- Unpopulated microphone connector
 |____________________________________________________|

***************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class cirsa2080606_state : public driver_device
{
public:
	cirsa2080606_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_soundcpu(*this, "soundcpu")
	{ }

	void maquinati(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// devices
	optional_device<cpu_device> m_soundcpu;
};

static INPUT_PORTS_START(cirsa2080606)
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END

uint32_t cirsa2080606_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static GFXDECODE_START( gfx_cirsa2080606 )
GFXDECODE_END

void cirsa2080606_state::maquinati(machine_config &config)
{
	// Video hardware (probably wrong values)
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(cirsa2080606_state::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(512);

	// Sound hardware
	ARM9(config, m_soundcpu, 8_MHz_XTAL);

	SPEAKER(config, "mono").front_center();
}


/* Cirsa / Unidesa "La Máquina del Tiempo".
   Complete manual with schematics can be downloaded from https://www.recreativas.org/manuales/tragaperras  */
ROM_START( maquinati )
	ROM_REGION( 0x1000012, "maincpu", 0 )
	ROM_LOAD( "programa_js28f128j3c120.u5", 0x0000000, 0x1000012, NO_DUMP )

	ROM_REGION( 0x0080000, "bbox", 0 )
	ROM_LOAD( "bbox_m27w401-80k6.u6",       0x0000000, 0x0080000, NO_DUMP )

	ROM_REGION( 0xC000000, "gfx", 0 )
	ROM_LOAD( "la_maquina_del_tiempo_graf-es_1-3.u1",             0x0000000, 0x2000000, CRC(aafeeda9) SHA1(48bcae57d7f4ad6eff1c90bc41bc98ca4ad3b5a5) )
	ROM_LOAD( "la_maquina_del_tiempo_graf-es_1-3.u2",             0x2000000, 0x2000000, CRC(6d44e979) SHA1(cfed68cd9a64701977b2be4cf99f2b4717374c66) )
	ROM_LOAD( "la_maquina_del_tiempo_graf-es_bg2-2-3_g-2f119.u1", 0x4000000, 0x2000000, CRC(4d76a943) SHA1(f8819b509c416a29f6f3d41561e74c62ca7448e1) )
	ROM_LOAD( "la_maquina_del_tiempo_graf-es_bg2-2-3_g-2f119.u2", 0x6000000, 0x2000000, CRC(19d3040e) SHA1(c174dce45755d918d62f116aaee06cabef8021f8) )
	ROM_LOAD( "la_maquina_del_tiempo_graf-es_bg1-3-3_g-3f119.u1", 0x8000000, 0x2000000, CRC(067cf5f6) SHA1(3094d1b6f508def383b52a77007efd7a09c90ce1) )
	ROM_LOAD( "la_maquina_del_tiempo_graf-es_bg1-3-3_g-3f119.u2", 0xA000000, 0x2000000, CRC(c017126c) SHA1(7ba628037da96874894317e72f62bc7b1878520d) )

	// Reels PCB 2050104-2
	ROM_REGION( 0x8000, "reels", 0 )
	ROM_LOAD( "pic18f258.u2", 0x0000, 0x8000, NO_DUMP ) // 32KB internal ROM, undumped

	// Lower JAD central door PCB LVCAN 2090727-2
	ROM_REGION( 0x2000, "lvcan", 0 )
	ROM_LOAD( "pic24hj64gp502-i-sp.u1", 0x0000, 0x2000, NO_DUMP ) // 8KB internal ROM, undumped

	// Sound PCB 2100125-2

	// Sound ARM internal ROM
	ROM_REGION( 0x40000, "soundcpu", 0 )
	ROM_LOAD( "sn_2.00_22-09-15_stm32f103rct6.u1", 0x00000, 0x40000, NO_DUMP ) // 256KB internal ROM, undumped

	// Sound MicroSD card. Contains just an ARM ROM image file.
	DISK_REGION( "sdcard" )
	DISK_IMAGE( "maquinati_sdcard", 0, SHA1(5e13eeb7bca6c8445de161de35c5ffc1f40b6e53) )
ROM_END


} // Anonymous namespace

//    YEAR  NAME       PARENT  MACHINE    INPUT         CLASS               INIT        ROT   COMPANY          FULLNAME                   FLAGS
GAME( 2015, maquinati, 0,      maquinati, cirsa2080606, cirsa2080606_state, empty_init, ROT0, "Unidesa/Cirsa", u8"La Máquina del Tiempo", MACHINE_IS_SKELETON )
