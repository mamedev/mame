// license:BSD-3-Clause
// copyright-holders:
/*
Konami Cougar system
Used for coin pushers

Hardware:
GSDRA PWB(A2) 0000327125 Cougar (with 378 117 11119970 sticker)
* 6417751R SH-4 BP240 (7751R with PCI controller) CPU SoC with 20.000 MHz and 32.768 KHz XTALs nearby
* SiS 315 VGA/AGP
* 2x 48LC8M16A2 128Mb SDRAM (near SH-4)
* 8x 48LC8M16A2 128Mb SDRAM (near SiS 315)
* R1LV0416CSB 4M SRAM
* 2x Xilinx Spartan XC2S50E FPGA
* Xilinx XC95144XL CPLD with 66.0000 MHz XTAL nearby
* SMSC LAN91C111-NU Ethernet controller with 25.000 MHz XTAL nearby
* CR2032 3V battery
* 4-dip bank
* S29AL016D70TFI020 16Mb boot sector FLASH (on solder side)
* R4543 B RTC (on solder side)

GSGPB PWB(B) 111436540000 AI AM-1
* YMZ280B-F with 16.9344 MHz XTAL nearby
* Xilinx XC95144XL CPLD with 66.0000 MHz XTAL nearby
* 4x TC58FVM6T5BTG65 64Mb Flash
* 2x 8-dip bank
* 64x Sharp PC817 photocoupler
*/

#include "emu.h"

#include "cpu/sh/sh4.h"
#include "machine/intelfsh.h"
#include "machine/rtc4543.h"
#include "sound/ymz280b.h"

#include "screen.h"
#include "speaker.h"


namespace {

class cougar_state : public driver_device
{
public:
	cougar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void cougar(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
};

void cougar_state::program_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom();
}

static INPUT_PORTS_START( cougar )
INPUT_PORTS_END


void cougar_state::cougar(machine_config &config)
{
	SH4LE(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cougar_state::program_map);
	m_maincpu->set_force_no_drc(true);

	RTC4543(config, "rtc", 32.768_kHz_XTAL);

	//SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16.9344_MHz_XTAL));
	ymz.add_route(0, "lspeaker", 1.0);
	ymz.add_route(1, "rspeaker", 1.0);
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( spinfev )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 )
	ROM_LOAD( "11060030.ic17", 0x000000, 0x200000, CRC(f39ef488) SHA1(83d04ee6edae3273511d1d036c93029214a15252) )

	ROM_REGION( 0x810000, "sndflash0", 0)
	ROM_LOAD( "flash.ic120", 0x000000, 0x810000, CRC(724ab5c0) SHA1(4ff2c4440586f58f7805ff9310dcbbbbc3c35157) )

	ROM_REGION( 0x810000, "sndflash1", 0)
	ROM_LOAD( "flash.ic118", 0x000000, 0x810000, CRC(31b8d161) SHA1(0baf5ef251abbfdba5c1ab905e6238ba21674130) )

	ROM_REGION( 0x810000, "sndflash2", 0)
	ROM_LOAD( "flash.ic119", 0x000000, 0x810000, CRC(a7980e6f) SHA1(150f92fcd18f39ba256602941cb45d6647ca1099) )

	ROM_REGION( 0x810000, "sndflash3", ROMREGION_ERASE00) // empty on this PCB

	DISK_REGION( "ide:0:hdd" ) // dumped from a Seagate Barracuda 7200.10 80 Gbytes (ST380815A8)
	DISK_IMAGE( "spinfev", 0, SHA1(c1ca74bb05335e66f64bf2198df7f21fda9c4a03) )
ROM_END

} // anonymous namespace


GAME( 2008, spinfev,  0,   cougar, cougar, cougar_state, empty_init, ROT0, "Konami", "Spin Fever",  MACHINE_IS_SKELETON ) // 'GPB-JB-F01 2008-04-17' and 'SPIN FEVER (GSGPB)  BOOT SCRIPT' strings in HDD
