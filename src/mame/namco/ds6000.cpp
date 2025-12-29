// license:BSD-3-Clause
// copyright-holders:

/*
Namco/Mitsubishi DS-6000 Driving Simulator

This is Namco System 22 based and drives 3 screens.
It comprises the following PCBs:

NAMCO S22DS MOTHER PCB - 8650961001 (8650971001)
32.000 MHz XTAL
Connectors

NAMCO SS22DS M-CPU PCB - 8650960103 (8650970103)
MC68020RP25E CPU
49.1250 MHz XTAL
4x ISSI IS61C1024-15K RAM
MC68EC020FG25 CPU
MC68882FN25A FPU
M37702E2AFP sound MCU
40.0000 MHz XTAL
2x Namco C383
Namco C352
4x Samsung K6T1008C2E-GB70 RAM
3x Namco C405
Namco C422
2x CY7C199-15VC RAM
CY7C182-25VC RAM
2x GAL16V8D
3x GAL22V10D
Altera EPM7?? (covered by sticker)
Atmel AT28C64B-15SC EEPROM
Intel PA28F200BX-T80 flash ROM
MB87078 electronic volume controller
bank of 8 switches
2x bank of 4 switches

3x NAMCO SYSTEM SUPER22 DSP PCB - 8646960302 (8646970302)
see PCB info in namco/namcos22.cpp

NAMCO SYSTEM SUPER 22 MPM(F8F) PCB - 8646963400 (8646973400)
riser board for main CPU ROMs
see PCB info in namco/namcos22.cpp

NAMCO SS22DS SPM(F2) PCB - 8650961900 (8650971900)
riser board for sound ROMs
8x LH28F160S5T-L70A
GAL22V10D
Connector

3x NAMCO SYSTEM SUPER22 VIDEO(C) PCB - 8646962700 (8646972700)
see PCB info in namco/namcos22.cpp

3x NAMCO SS22DS FLASH(C) PCB - 8650960300 (8650971304)
see PCB info in namco/namcos22.cpp for SS22DS FLASH PCB 8650961300 (almost identical)
the EPMXXXX chip described there is an Altera EPM7128ELC84-12 on this PCB type
all 3 PCBs are populated identically
there are 26x Sharp LH28F160S5T-L70A + 16x unpopulated spaces (serigraphed 28F032)
the CD contains the flash ROM data

NAMCO V164 I/O PCB - 2441960101 (2441970101)
TMP68301AFR-12
2x TMS 27C040-10 program ROM
2x 7-segment LED
Namco 158
Namco C422
Namco 137
49.1250 MHz XTAL
CY7C182-25VC RAM
bank of 8 switches
2x TD62003AP Darlington sink driver
2x GAL16V8D


NAMCO V164 STEERING DRIVE PCB - 2441960302 (2441970302)
MB90P745 CPU
M27C1024-10F1 program ROM
20 MHz XTAL
2x NEC D431000ACZ-70L RAM
bank of 6 switches


It also has a Plextor PX-40TSi SCSI CD-ROM drive
*/


#include "emu.h"

#include "namco_dsp.h"
#include "namcomcu.h"

#include "cpu/m68000/m68020.h"
#include "cpu/m68000/tmp68301.h"
#include "machine/eeprompar.h"
#include "machine/mb87078.h"
#include "sound/c352.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class ds6000_state : public driver_device
{
public:
	ds6000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void ds6000(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_program_map(address_map &map) ATTR_COLD;
	void io_program_map(address_map &map) ATTR_COLD;
};


uint32_t ds6000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void ds6000_state::main_program_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
}

void ds6000_state::io_program_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
}


static INPUT_PORTS_START( ds6000 )
INPUT_PORTS_END


// TODO: all dividers not verified
void ds6000_state::ds6000(machine_config &config)
{
	M68020(config, m_maincpu, 49.152_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ds6000_state::main_program_map);

	tmp68301_device &iocpu(TMP68301(config, "iocpu", 49.152_MHz_XTAL / 4));
	iocpu.set_addrmap(AS_PROGRAM, &ds6000_state::io_program_map);

	// TODO: MB90P745 (no core available)

	// TODO: DSPs

	// TODO: Namco customs

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 512-1, 0*8, 512-1);
	screen.set_screen_update(FUNC(ds6000_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x8000);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	c352_device &c352(C352(config, "c352", 49.152_MHz_XTAL / 2, 288)); // TODO: copied from namcos22.cpp, verify
	c352.add_route(0, "speaker", 1.0, 0);
	c352.add_route(1, "speaker", 1.0, 1);
}


ROM_START( ds6000 )
	ROM_REGION( 0x400000, "maincpu", 0 ) // ver 1.02 ?
	ROM_LOAD32_BYTE( "kss1ver-j.2", 0x000000, 0x100000, CRC(c94e183a) SHA1(d491268382a2f1fb075e4c2bab134ca2d87a428e) )
	ROM_LOAD32_BYTE( "kss1ver-j.3", 0x000001, 0x100000, CRC(8a55a829) SHA1(5dcb0711a719ffd1f8b9429850ac520895ad60d4) )
	ROM_LOAD32_BYTE( "kss1ver-j.4", 0x000002, 0x100000, CRC(65a7c7b3) SHA1(f6a5dd3d2032f11350a4e478d8bf23ab59c78825) )
	ROM_LOAD32_BYTE( "kss1ver-j.1", 0x000003, 0x100000, CRC(72009838) SHA1(b6e48c927463ae435b712150a2df5bb45598187c) )

	ROM_REGION( 0x4000, "mcu", ROMREGION_ERASE00 )
	ROM_LOAD( "ds22sp1.6f", 0x0000, 0x4000, NO_DUMP ) // M37702E2AFP, internal ROM not dumped yet

	ROM_REGION( 0x40000, "sound_data", 0 )
	ROM_LOAD( "pa28f200bx.8j", 0x00000, 0x40000, CRC(d2280eca) SHA1(87f91d054e2ed0ddae8a26a7656fb67a2f38ec95) )

	ROM_REGION( 0x100000, "iocpu", 0 )
	ROM_LOAD16_BYTE( "kss1prog0h-0c.1b",  0x00000, 0x80000, CRC(c57f7139) SHA1(62da97a7823460bdd90fb218cf2e8f0aa5de6149) )
	ROM_LOAD16_BYTE( "kss1prog0l-0c.12b", 0x00001, 0x80000, CRC(f8bc97dc) SHA1(531ed0005852fcb5f443af1cbc51f49cd842595d) )

	ROM_REGION( 0x20000, "steeringcpu", 0 ) // // 96.3. 7  Ver. 4.11 string
	ROM_LOAD( "kss1prog0-0d.3a", 0x00000, 0x20000, CRC(e01ef77d) SHA1(fd0109c685ce61f62c13f42c34197fca4e8ca44d) )

	ROM_REGION( 0x2000, "eeprom", 0 )
	ROM_LOAD( "at28c64b-15sc.12e", 0x0000, 0x2000, NO_DUMP )

	DISK_REGION("cd")
	DISK_IMAGE_READONLY( "ver5.12", 0, SHA1(02f4e7a0331c759b19ce5c2ac1cae8ff0a20099b) ) // system software

	ROM_REGION( 0xd00, "cpupcb_plds", ROMREGION_ERASE00 )
	ROM_LOAD( "ds22c2.12e", 0x000, 0x2e5, NO_DUMP ) // GAL22V10D
	ROM_LOAD( "ds22c3.13a", 0x300, 0x117, NO_DUMP ) // GAL16V8D
	ROM_LOAD( "ds22c4.12d", 0x500, 0x2e5, NO_DUMP ) // GAL22V10D
	ROM_LOAD( "ds22c5.6c",  0x800, 0x2e5, NO_DUMP ) // GAL22V10D
	ROM_LOAD( "ds22c6.13k", 0xb00, 0x117, NO_DUMP ) // GAL16V8D

	ROM_REGION( 0x400, "flashpcb_plds", ROMREGION_ERASE00 )
	ROM_LOAD( "dsf4.10a", 0x000, 0x117, NO_DUMP ) // GAL16V8D
	ROM_LOAD( "dsf5a.8p", 0x200, 0x117, NO_DUMP ) // GAL16V8D

	ROM_REGION( 0x300, "soundriserpcb_plds", ROMREGION_ERASE00 )
	ROM_LOAD( "ds22s1.ic9", 0x000, 0x2e5, NO_DUMP ) // GAL22V10D

	ROM_REGION( 0x400, "iopcb_plds", ROMREGION_ERASE00 )
	ROM_LOAD( "ksspld1.6e", 0x000, 0x117, NO_DUMP ) // GAL16V8D
	ROM_LOAD( "ksspld2.9e", 0x200, 0x117, NO_DUMP ) // GAL16V8D

	ROM_REGION( 0x300, "steeringpcb_plds", ROMREGION_ERASE00 )
	ROM_LOAD( "ksspld0b.4f", 0x000, 0x2e5, NO_DUMP ) // exact type covered by sticker
ROM_END

} // anonymous namespace


GAME( 1996?, ds6000, 0, ds6000, ds6000, ds6000_state, empty_init, ROT0, "Namco / Mitsubishi", "DS-6000 Driving Simulator", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
