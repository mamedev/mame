// license:BSD-3-Clause
// copyright-holders:David Haywood

/* San Remo / Elsy games

presumably gambling games, maybe missing a sub-board?

Devices
1x unknown quad SMT chip etched "ELSY" main
1x M30624FGAFP 16-bit Single-Chip Microcomputer - main (internal ROM not dumped)
1x TDA2003 Audio Amplifier - sound
1x oscillator 40.000
1x oscillator R100YXA62

ROMs
2x HY29LV160BT or equivalent

RAMs
2x HY628100BLG_70-70W or equivalent

Others
1x 28x2 JAMMA edge connector
1x 50 pins flat cable connector
1x 4 pins black connector
1x RS232 connectors
1x 8 DIP switches bank
1x battery 3.6V
1x battery 5.5V


M30624FG (M16C/62A family) needs CPU core and dumping of internal ROM
*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "emupal.h"
#include "screen.h"


namespace {

class sanremmg_state : public driver_device
{
public:
	sanremmg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void sanremmg(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update_sanremmg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sanremmg_map(address_map &map) ATTR_COLD;
};


void sanremmg_state::video_start()
{
}

uint32_t sanremmg_state::screen_update_sanremmg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}



void sanremmg_state::sanremmg_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom();
}


static INPUT_PORTS_START( sanremmg )
INPUT_PORTS_END

void sanremmg_state::sanremmg(machine_config &config)
{
	/* basic machine hardware */
	ARM7(config, m_maincpu, 50000000); // wrong, this is an M30624FG (M16C/62A family) with 256K internal ROM, no CPU core available
	m_maincpu->set_addrmap(AS_PROGRAM, &sanremmg_state::sanremmg_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(sanremmg_state::screen_update_sanremmg));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x200);
}

/*
PCB is marked: "ELSY CE" and "2-028B" and "San Remo Games - Via Val D'OLIVI 295 - 18038 SANREMO (IM)" on component side
*/
ROM_START( sanremmg )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "sanremmg_m30624fg.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION(0x400000, "data", 0 ) // start of 1.bin has 'Tue Sep 03 11:37:03' Not sure if 03 is year or day, start of string is erased with boot vector?
	ROM_LOAD( "1.bin",   0x000000, 0x200000, CRC(67fa5e76) SHA1(92beb90e1b370763966017d47cb748106014d371) ) // HY29LV160BT
	ROM_LOAD( "2.bin",   0x200000, 0x200000, CRC(61f69735) SHA1(ff46362ce6fe239089c85e698add1b8090bb39bb) )
	// there is space for what looks like a 3rd rom
ROM_END

/*
PCB is marked: "ELSY CE" and "OM1 2-030B" on component side
PCB is marked: "23:04E" and "N2557 TE" on solder side
PCB is labeled: "ELECTROSYSTEM ELSY" and "TEST OK 26" and "F03E04 Ver. 2.2" on component side
*/
ROM_START( elsypokr )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "elsypokr_m30624fg.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION(0x400000, "data", 0 ) // start of first ROM has 'Mon Apr 05 10:25:38'. Not sure if 05 is year or day, start of string is erased with boot vector?
	ROM_LOAD( "mx29lv160bb.1.bin",   0x000000, 0x200000, CRC(da620fa6) SHA1(f2eea0146f6ddcaa4049f6fe7797d755faeace88) ) // MX29LV160BBTC-70
	ROM_LOAD( "mx29lv160bb.2.bin",   0x200000, 0x200000, CRC(7a0c3e38) SHA1(dd98d6f56272bf3cc0ed1a14234a8c6e0bc4dd37) )
	// there is space for what looks like a 3rd rom
ROM_END

/*
PCB is marked: "ELSY CE" and "2-0291" on component side
PCB is labelled: "Fruit Diamont", "3", "G09004 Ver.1.6" and "TEST OK 02/46" on component side
*/
ROM_START( elsygame )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "elsygame_m30624fg.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION(0x400000, "data", 0 ) // start of first ROM has 'Thu Nov 07 12:12:25'. Not sure if 07 is year or day, start of string is erased with boot vector?
	ROM_LOAD( "mx29lv160bb.1.bin",   0x000000, 0x200000, CRC(cef067b2) SHA1(16fb83d6053532872db89259b64761f8be02de71) ) // MX29LV160BBTC-70
	ROM_LOAD( "mx29lv160bb.2.bin",   0x200000, 0x200000, CRC(9b0cb755) SHA1(e66bac00c219d345cb6a9478e23bee2a2e79398b) )
	// there is space for what looks like a 3rd rom
ROM_END

} // anonymous namespace


GAME( 2003, sanremmg, 0,        sanremmg,  sanremmg, sanremmg_state, empty_init,  ROT0, "San Remo Games", "unknown San Remo / Elsy Multigame", MACHINE_IS_SKELETON )
GAME( 200?, elsypokr, 0,        sanremmg,  sanremmg, sanremmg_state, empty_init,  ROT0, "Electro System (Elsy)", "unknown Elsy poker", MACHINE_IS_SKELETON )
GAME( 2002, elsygame, 0,        sanremmg,  sanremmg, sanremmg_state, empty_init,  ROT0, "Electro System (Elsy)", "unknown Elsy game", MACHINE_IS_SKELETON ) // Fruit Diamont (sic)?
