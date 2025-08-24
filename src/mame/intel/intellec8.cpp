// license:BSD-3-Clause
// copyright-holders: Robbbert
/***************************************************************************

Intellec 8 MCS

A development machine from Intel for the 8008 CPU, with Front Panel.
It has the usual array of switches, lights and buttons.

****************************************************************************/

#include "emu.h"
#include "cpu/i8008/i8008.h"

namespace {

class intlc8_state : public driver_device
{
public:
	intlc8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void intlc8(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void intlc8_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();  // ROM - 2 KB expandable to 16 KB?
	map(0x2000, 0x3fff).ram();  // RAM - 8 KB expandable to 16 KB?
}

void intlc8_state::io_map(address_map &map)
{
	map.unmap_value_high();
}

// Input ports
static INPUT_PORTS_START( intlc8 )
INPUT_PORTS_END

void intlc8_state::machine_reset()
{
}

void intlc8_state::machine_start()
{
}

void intlc8_state::intlc8(machine_config &config)
{
	// basic machine hardware
	I8008(config, m_maincpu, 800000);   // 750 or 800 kHz clock?
	m_maincpu->set_addrmap(AS_PROGRAM, &intlc8_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &intlc8_state::io_map);
}


ROM_START( intlc8 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )  // order of roms is a guess and imo some are missing?
	ROM_LOAD( "miss0.bin",    0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "miss1.bin",    0x0100, 0x0100, NO_DUMP )
	ROM_LOAD( "rom1.bin",     0x0200, 0x0100, CRC(0ae76bc7) SHA1(374a545cad8406ad862a5f2f1f03c6b6434fd3d8) )
	ROM_LOAD( "rom4.bin",     0x0300, 0x0100, CRC(4340fdfe) SHA1(d37f3bd65c2970736ac075c7e6d3d87d018c3ea4) )
	ROM_LOAD( "rom2.bin",     0x0400, 0x0100, CRC(dd1a71f4) SHA1(e33a2b64bea18c0aa58230167eb23bae464431be) )
	ROM_LOAD( "rom5.bin",     0x0500, 0x0100, CRC(d631224c) SHA1(812d37ac98daf1252bc8d087da679f3ad1f9d961) )
	ROM_LOAD( "rom3.bin",     0x0600, 0x0100, CRC(97c7ab95) SHA1(650316b820b84393bb73c4c56c11e177d658ce4a) )
	ROM_LOAD( "miss7.bin",    0x0700, 0x0100, NO_DUMP )
ROM_END

// FIXME: these are for an 8080-based system and possibly don't belong here

ROM_START( intlc8m80 )
	ROM_DEFAULT_BIOS("884a")
	ROM_SYSTEM_BIOS(0, "880", "880")
	ROM_SYSTEM_BIOS(1, "884a", "884A")

	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )

	ROMX_LOAD( "3000_1702a.chip_0",                  0x0000, 0x0100, CRC(a72cdab9) SHA1(c9931f1b55558383054a02b40fa716e9658ce3c4), ROM_BIOS(0) )
	ROMX_LOAD( "3100h_1702a.chip_1",                 0x0100, 0x0100, CRC(62fbc181) SHA1(e704b5b198e43ff0b9c57067e375070a7674f3dd), ROM_BIOS(0) )
	ROMX_LOAD( "3200_1702a.chip_2",                  0x0200, 0x0100, CRC(65215a39) SHA1(aaaae0f226a0d0f4eeac51e836b1ea674eb0f996), ROM_BIOS(0) )
	// Empty sockets from CHIP 3 to CHIP 4
	ROMX_LOAD( "icon_2708-p_mon8_3800_1702a.chip_5", 0x0300, 0x0100, CRC(00445543) SHA1(63ab074ba8c43905e2a43761b5cc5e0618f6e403), ROM_BIOS(0) )
	ROMX_LOAD( "prog_3600_1702a.chip_6",             0x0400, 0x0100, CRC(61a99002) SHA1(2be84f3191d6de5e184a8c7a40312c663b4c1817), ROM_BIOS(0) )
	ROMX_LOAD( "1702a.chip_7",                       0x0500, 0x0100, CRC(90d9a76b) SHA1(3d66bc7aa5caa2abb487dfe28ab989b373fe6703), ROM_BIOS(0) )
	ROMX_LOAD( "3800_mon8_1702a.chip_8",             0x0600, 0x0100, CRC(ee7a08dc) SHA1(f12a690d72f08ef333e65634e0e60aff40746b7a), ROM_BIOS(0) )
	ROMX_LOAD( "3900h_mon8_1702a.chip_9",            0x0700, 0x0100, CRC(d2795e4d) SHA1(f3ef8e197fcfaf6752da8cee6ee776b7a450cef2), ROM_BIOS(0) )
	ROMX_LOAD( "3a00h_mon8_1702a.chip_a",            0x0800, 0x0100, CRC(c92f98e3) SHA1(dcb4316c6f037666a2a4b88df7e18ca74e754dd0), ROM_BIOS(0) )
	ROMX_LOAD( "3b00h_mon8_1702a.chip_b",            0x0900, 0x0100, CRC(23083008) SHA1(57af12b20f160d5faa99ad2bda597f21e52078c9), ROM_BIOS(0) )
	ROMX_LOAD( "3c00h_mon8_1702a.chip_c",            0x0a00, 0x0100, CRC(32f5c81b) SHA1(2371c0e087486c8bcb909575f158fd5ac9209bc8), ROM_BIOS(0) )
	ROMX_LOAD( "3d00h_mon8_1702a.chip_d",            0x0b00, 0x0100, CRC(5307307a) SHA1(f38adac5e1a8bb015e23f13be5ab434394e6495f), ROM_BIOS(0) )
	ROMX_LOAD( "3e00h_mon8_1702a.chip_e",            0x0c00, 0x0100, NO_DUMP,                                                      ROM_BIOS(0) )
	ROMX_LOAD( "3f00h_mon8_1702a.chip_f",            0x0d00, 0x0100, CRC(beca9bd7) SHA1(8162306bfbd94a373736b9e8e9f426af104d744e), ROM_BIOS(0) )

	ROMX_LOAD( "1702a.chip_0",                       0x0000, 0x0100, CRC(64a1aa3a) SHA1(7158e866eb222b1fbc1f573cdc748f5aedd6d0d4), ROM_BIOS(1) )
	ROMX_LOAD( "1702a.chip_1",                       0x0100, 0x0100, CRC(4583b6c3) SHA1(f9073ecfe0c043756437af595aa87b7224bf370d), ROM_BIOS(1) )
	ROMX_LOAD( "1702a.chip_2",                       0x0200, 0x0100, CRC(5a2951cc) SHA1(d9252365a330cd390ebe029517c83d9e579750a2), ROM_BIOS(1) )
	ROMX_LOAD( "1702a.chip_3",                       0x0300, 0x0100, CRC(37b1b90b) SHA1(b88872d56d03efe6ebc550fc9608e55ca0cae3cd), ROM_BIOS(1) )
	ROMX_LOAD( "1702a.chip_4",                       0x0400, 0x0100, CRC(dcbcc405) SHA1(1dc65e9c73416353e2650158183ff4ff33a9eb0e), ROM_BIOS(1) )
	// Empty sockets from CHIP 5 to CHIP 6
	ROMX_LOAD( "3700_1702a.chip_7",                  0x0500, 0x0100, CRC(cf5a0f6e) SHA1(13841fe28ac310f00d4c55ed07fee8be556ecc9b), ROM_BIOS(1) )
	ROMX_LOAD( "1702a.chip_8",                       0x0600, 0x0100, CRC(fee2425f) SHA1(0210012153d9e294f46ab3653b4eb439125b6cfe), ROM_BIOS(1) )
	ROMX_LOAD( "3900_1702a.chip_9",                  0x0700, 0x0100, CRC(d2795e4d) SHA1(f3ef8e197fcfaf6752da8cee6ee776b7a450cef2), ROM_BIOS(1) )
	ROMX_LOAD( "1702a.chip_a",                       0x0800, 0x0100, CRC(c92f98e3) SHA1(dcb4316c6f037666a2a4b88df7e18ca74e754dd0), ROM_BIOS(1) )
	ROMX_LOAD( "1702a.chip_b",                       0x0900, 0x0100, CRC(23083008) SHA1(57af12b20f160d5faa99ad2bda597f21e52078c9), ROM_BIOS(1) )
	ROMX_LOAD( "3c00_1702a.chip_c",                  0x0a00, 0x0100, CRC(32f5c81b) SHA1(2371c0e087486c8bcb909575f158fd5ac9209bc8), ROM_BIOS(1) )
	ROMX_LOAD( "1702a.chip_d",                       0x0b00, 0x0100, CRC(5307307a) SHA1(f38adac5e1a8bb015e23f13be5ab434394e6495f), ROM_BIOS(1) )
	ROMX_LOAD( "3e00_1702a.chip_e",                  0x0c00, 0x0100, CRC(a90bd1d4) SHA1(b85a4a3d6515aa4ae298a800192077716a060f85), ROM_BIOS(1) )
	ROMX_LOAD( "1702a.chip_f",                       0x0d00, 0x0100, CRC(ae7c919b) SHA1(ab9e2f70ef19d969ce7238784118ae53a5c7ac85), ROM_BIOS(1) )
ROM_END

} // Anonymous namespace

//    YEAR  NAME        PARENT      COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME             FLAGS
COMP( 1973, intlc8,     0,          0,      intlc8,  intlc8, intlc8_state, empty_init, "Intel", "intellec 8",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // MOD8 or MOD80?
COMP( 1974, intlc8m80,  0,          0,      intlc8,  intlc8, intlc8_state, empty_init, "Intel", "intellec 8/Mod 80", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
