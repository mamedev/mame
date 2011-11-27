/*
    Unknown PCP platform
    Z80 based
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/z80/z80.h"

class unkpcp_state : public driver_device
{
public:
	unkpcp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( unkpcp_map, AS_PROGRAM, 8, unkpcp_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( unkpcp_portmap, AS_IO, 8, unkpcp_state )
ADDRESS_MAP_END


static INPUT_PORTS_START( unkpcp )
INPUT_PORTS_END


static MACHINE_CONFIG_START( unkpcp, unkpcp_state )
	MCFG_CPU_ADD("maincpu", Z80, 4000000) /* ?? Mhz */
	MCFG_CPU_PROGRAM_MAP(unkpcp_map)
	MCFG_CPU_IO_MAP(unkpcp_portmap)
MACHINE_CONFIG_END


ROM_START( up_bluec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bchip.p1", 0x0000, 0x002000, CRC(fe41206d) SHA1(41c9ecc26a26def5cf63209274f6b08e00de7889) )
	ROM_LOAD( "bchip.p2", 0x2000, 0x002000, CRC(c70dfaf1) SHA1(6628a7e66f7e22c3a45515c8be6ed88e0e19b10c) )
	ROM_LOAD( "bchip.p3", 0x4000, 0x002000, CRC(6649e6fb) SHA1(628594c924fc5029add5ddedd7c4f3aefbdbd0b6) )
	ROM_LOAD( "bchip.p4", 0x6000, 0x002000, CRC(5fb53b5a) SHA1(d6449d19ea440a51ce13564b34fbbab53a216a18) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "bluechiparcadeirelandp1.bin", 0x0000, 0x002000, CRC(dbeaf017) SHA1(e5020acdb4248a6373aca475ee2aab042860052c) )
	ROM_LOAD( "bluechiparcadeirelandp2.bin", 0x0000, 0x002000, CRC(c70dfaf1) SHA1(6628a7e66f7e22c3a45515c8be6ed88e0e19b10c) )
	ROM_LOAD( "bluechiparcadeirelandp3.bin", 0x0000, 0x002000, CRC(cd39fb8a) SHA1(b604dfe105b530431d4f5b7d6f839f6169b02c37) )
	ROM_LOAD( "bluechiparcadeirelandp4.bin", 0x0000, 0x002000, CRC(daa1e5bb) SHA1(1f629bc8f9015b73cc9b5b798a98f4479644ac8b) )
	ROM_LOAD( "bluechiparcadestdp1.bin", 0x0000, 0x002000, CRC(dafa4a2b) SHA1(bd4aaaffb6980de5d6279a004c07a62e331e47f4) )
	ROM_LOAD( "bluechiparcadestdp2.bin", 0x0000, 0x002000, CRC(e18108f8) SHA1(4f94c8d35094ca5ae29e6ba61dd0697515316e21) )
	ROM_LOAD( "bluechiparcadestdp3.bin", 0x0000, 0x002000, CRC(bb6cc903) SHA1(cb8d87d60d9925338cd00cdf02cf324f6eaa077c) )
	ROM_LOAD( "bluechiparcadestdp4.bin", 0x0000, 0x002000, CRC(0acc6341) SHA1(106f924458fd0fdd40f40714ee97e96a89ef73e0) )
	ROM_LOAD( "bluechipcombined.bin", 0x0000, 0x008000, CRC(24af158c) SHA1(74bc19d4a4d6d34f35dfaf33c75b2e46d87e8ac5) )
	ROM_LOAD( "bluechipp1.bin", 0x0000, 0x002000, CRC(dbc8c9a8) SHA1(d17cc912c2eb5a46a14b27a6ad4b3ec155a9a653) )
	ROM_LOAD( "bluechipp2.bin", 0x0000, 0x002000, CRC(41a7dd01) SHA1(43fa39fde4cdb5c8d5aaaa8733e43cb925fb1c41) )
	ROM_LOAD( "bluechipp3.bin", 0x0000, 0x002000, CRC(cee5f9cc) SHA1(b38bfe0222554b743a7ae20ee74d029293c17424) )
	ROM_LOAD( "bluechipp4.bin", 0x0000, 0x002000, CRC(89278a76) SHA1(6effb53a0ff9ff04125b47bf12ceebe6347e080a) )
ROM_END


ROM_START( up_dbldx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "double deluxe 14.2.90 p1.bin", 0x0000, 0x002000, CRC(d04288e1) SHA1(0101aa58b327322b192bef20ca698c0d3a9a02d0) )
	ROM_LOAD( "double deluxe 14.2.90 p2.bin", 0x2000, 0x002000, CRC(210607a1) SHA1(e08866fa18d4102a29a40c78e0e682232dcdaf19) )
	ROM_LOAD( "double deluxe 14.2.90 p3.bin", 0x4000, 0x002000, CRC(637e310a) SHA1(58801577b684d3fb2e928659871960a46d39aa78) )
	ROM_LOAD( "double deluxe 14.2.90 p4.bin", 0x6000, 0x002000, CRC(df027038) SHA1(ec9fda9dc53a628ea476f593f73cb42c8e227f05) )
ROM_END


ROM_START( up_nudbk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nbreak 10p 4.80 p1 jan 1990.bin", 0x0000, 0x002000, CRC(8438b94b) SHA1(5791069ddb986321ed1abe03e234c503c3c40d0c) )
	ROM_LOAD( "nbreak 10p 4.80 p2 jan 1990.bin", 0x2000, 0x002000, CRC(e9718ee6) SHA1(ad3a309cb0ed29f458187810a6606632a8019d70) )
	ROM_LOAD( "nbreak 10p 4.80 p3 jan 1990.bin", 0x4000, 0x002000, CRC(b2e6d735) SHA1(979f48e5d8503a517fd936fbcaad84804e4b52ab) )
	ROM_LOAD( "nbreak 10p 4.80 p4 jan 1990.bin", 0x6000, 0x002000, CRC(7c2e67dd) SHA1(0d2f15793e586a6a0e08e22b207ec37d64fb8402) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "nbreak 2p 2.40 p1 jan 1990.bin", 0x0000, 0x002000, CRC(ee1d3a85) SHA1(f26e07e975a8d640f03bb938ba4c8b4920b39fbb) )
	ROM_LOAD( "nbreak 2p 2.40 p2 jan 1990.bin", 0x0000, 0x002000, CRC(f297ce8b) SHA1(760bcdd6acedee7ea9e2847be5de045da96dd0c6) )
	ROM_LOAD( "nbreak 2p 2.40 p3 jan 1990.bin", 0x0000, 0x002000, CRC(a85092c6) SHA1(cd0080f3d6743c76f9f13cacd3b61a7f39b24908) )
	ROM_LOAD( "nbreak 2p 2.40 p4 jan 1990.bin", 0x0000, 0x002000, CRC(0bab299e) SHA1(b3ab09d94cd1da4e5b38a0f757579a8da2a2063d) )
	ROM_LOAD( "nbreak 5p 2.40 p1 jan 1990.bin", 0x0000, 0x002000, CRC(6ea4df7f) SHA1(2dc221ede6d91c5e28988c08a37b7458b29c0994) )
	ROM_LOAD( "nbreak 5p 2.40 p2 jan 1990.bin", 0x0000, 0x002000, CRC(b58e988a) SHA1(bea5796a7d1e111aba0c9daefa5c9a9332d49490) )
	ROM_LOAD( "nbreak 5p 2.40 p3 jan 1990.bin", 0x0000, 0x002000, CRC(1e80969b) SHA1(7aa465ec52f5e8691c62bb82ad7d085bc132dcd4) )
	ROM_LOAD( "nbreak 5p 2.40 p4 jan 1990.bin", 0x0000, 0x002000, CRC(0bab299e) SHA1(b3ab09d94cd1da4e5b38a0f757579a8da2a2063d) )
	ROM_LOAD( "nudgebreak 2p play 2 p1.02 22.11.89.bin", 0x0000, 0x002000, CRC(b712ec30) SHA1(f80f2946f3f9aca62392740d28cc8091e156223d) )
	ROM_LOAD( "nudgebreak 2p play 2 p2.02 22.11.89.bin", 0x0000, 0x002000, CRC(b473f132) SHA1(bbfe8b55edfe9c02b0c9f666dd3ccfd41e22ce16) )
	ROM_LOAD( "nudgebreak 2p play 2 p3.02 22.11.89.bin", 0x0000, 0x002000, CRC(83148533) SHA1(88604433a1d39c57e1105bab6320e9f93bf59c2e) )
	ROM_LOAD( "nudgebreak 2p play 2 p4.02 22.11.89.bin", 0x0000, 0x002000, CRC(34bd98fb) SHA1(40038c14a8f43f8bf78e8eab1cd2f309a0213d33) )
	ROM_LOAD( "nudgebreak 5p play 2 p1.bin", 0x0000, 0x002000, CRC(8409469d) SHA1(dbf5654b2bd112409084977523e7ac39e5676626) )
	ROM_LOAD( "nudgebreak 5p play 2 p2.01 22.11.89.bin", 0x0000, 0x002000, CRC(4ad98f24) SHA1(deb74daa3c42a1acdefd64c77887b1dafb801ccc) )
	ROM_LOAD( "nudgebreak 5p play 2 p3.01 22.11.89.bin", 0x0000, 0x002000, CRC(9ede1bed) SHA1(352cd4deb3ba0e59a315f54590c62d5f95786396) )
	ROM_LOAD( "nudgebreak 5p play 2 p4.01 22.11.89.bin", 0x0000, 0x002000, CRC(2528204b) SHA1(b77b04f74ddf38430c5c7f9a08230a705047164d) )
ROM_END


ROM_START( up_sstrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "starstruck 4.80 cash p1.bin", 0x0000, 0x002000, CRC(4b504e9b) SHA1(0d4de867b5373c944b9819479cb0aeb9786ceac9) )
	ROM_LOAD( "starstruck 4.80 cash p2.bin", 0x2000, 0x002000, CRC(b13fb64a) SHA1(e01ae32968d6c4c086c5f47ef4f525c10dfb3319) )
	ROM_LOAD( "starstruck 4.80 cash p3.bin", 0x4000, 0x002000, CRC(80ecc41e) SHA1(7f9494e8892ff7c36d092a56380fd0ad20724f55) )
	ROM_LOAD( "starstruck 4.80 cash p4.bin", 0x6000, 0x002000, CRC(0bc4979e) SHA1(a59bb6a841ed34cf1c23913ceaa4ede8c6973a2c) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "starstruck irish iom p1 3.5.90 4.bin", 0x0000, 0x002000, CRC(95127749) SHA1(0e806181149995ca30342d2dcdea106fc91c7f17) )
	ROM_LOAD( "starstruck irish iom p2 3.5.90 4.bin", 0x0000, 0x002000, CRC(182b71fe) SHA1(5bd9957b6d78bfdbad805ddaa35c089e393ab0c7) )
	ROM_LOAD( "starstruck irish iom p3 3.5.90 4.bin", 0x0000, 0x002000, CRC(0fd123a3) SHA1(5f2805822d5167f6e3d99fec87c9219892c8cc7d) )
	ROM_LOAD( "starstruck irish iom p4 3.5.90 4.bin", 0x0000, 0x002000, CRC(1469ff05) SHA1(2abc0afdc113f3bad57b685980754d6b8d546d28) )
	ROM_LOAD( "starstruck p1 v2 23.4.90 std.bin", 0x0000, 0x002000, CRC(9ed618df) SHA1(5838c8006aaf7ca37c5ef97373c40f69ce1d54bb) )
	ROM_LOAD( "starstruck p2 v2 23.4.90 std.bin", 0x0000, 0x002000, CRC(f364a450) SHA1(35123f645396ec8ed0de64c0b0dbc55c0bea3db9) )
	ROM_LOAD( "starstruck p3 v2 23.4.90 std.bin", 0x0000, 0x002000, CRC(d07764b3) SHA1(18707df2330d3ba9793511fc06c3d79a9a40cce9) )
	ROM_LOAD( "starstruck p4 v2 23.4.90 std.bin", 0x0000, 0x002000, CRC(ba2533f0) SHA1(db9364c328d11955df4adde280c312e5eb9415ce) )
ROM_END


ROM_START( up_xpres )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xpressp1.bin", 0x0000, 0x002000, CRC(f5acbde7) SHA1(c88f5768c488663c23fd4b6a62ffeb7d07dbb317) )
	ROM_LOAD( "xpressp2.bin", 0x2000, 0x002000, CRC(dc4fa3a8) SHA1(bca83eb7cb7443206da13401f82347fcbee353ba) )
	ROM_LOAD( "xpressp3.bin", 0x4000, 0x002000, CRC(dca1495f) SHA1(ac8adbeb1461deb56377349dedcf10a853042dbf) )
	ROM_LOAD( "xpressp4.bin", 0x6000, 0x002000, CRC(31f654c1) SHA1(0ff6516cc60369f28555e81018865d3eb7e21ffd) )
ROM_END


ROM_START( up_roll )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roll up 10p 4.80 p1 26.1.90.bin", 0x0000, 0x002000, CRC(e985005a) SHA1(62a7e5afaf9d1d73073fc94e716f87bc4b9609c0) )
	ROM_LOAD( "roll up 10p 4.80 p2 26.1.90.bin", 0x2000, 0x002000, CRC(a4d4e9f6) SHA1(0f921e048772e1e04e20d25606621fe63fdacae5) )
	ROM_LOAD( "roll up 10p 4.80 p3 26.1.90.bin", 0x4000, 0x002000, CRC(daba0c2e) SHA1(44105352f901f8d97a15dcf90c9d9a50fda363e7) )
	ROM_LOAD( "roll up 10p 4.80 p4 26.1.90.bin", 0x6000, 0x002000, CRC(79093603) SHA1(30c811405951b44f3280326bb41880cb88cc2a4c) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "roll up 2p 2.40 p1.2 23.1.90.bin", 0x0000, 0x002000, CRC(eae5cb06) SHA1(7b460e7a0a42e6adf327908acf906dfa94566d36) )
	ROM_LOAD( "roll up 2p 2.40 p2.2 23.1.90.bin", 0x0000, 0x002000, CRC(ad0086ed) SHA1(bdb5f42de8e30de84ec0482fb63dcbbd403804f2) )
	ROM_LOAD( "roll up 2p 2.40 p3.2 23.1.90.bin", 0x0000, 0x002000, CRC(72adf89b) SHA1(aedca79946e3f756ba8e14a89c4e868e649d9c45) )
	ROM_LOAD( "roll up 2p 2.40 p4.2 23.1.90.bin", 0x0000, 0x002000, CRC(22bd5c84) SHA1(f4d0739e6981bc96095a81661a9a4d2a863349fb) )

	ROM_LOAD( "roll up 5p 2.40 p1 19.1.90.bin", 0x0000, 0x002000, CRC(72adf89b) SHA1(aedca79946e3f756ba8e14a89c4e868e649d9c45) )
	ROM_LOAD( "roll up 5p 2.40 p2 19.1.90.bin", 0x0000, 0x002000, CRC(4181ee7b) SHA1(cb5ad6a87bcbd387343c04a6b09a2b56bef539e5) )
	ROM_LOAD( "roll up 5p 2.40 p3 19.1.90.bin", 0x0000, 0x002000, CRC(225aaec7) SHA1(d444268935d260006fedff0079dfa0d08a3a69f3) )
	ROM_LOAD( "roll up 5p 2.40 p4 19.1.90.bin", 0x0000, 0x002000, CRC(8d2ddb6c) SHA1(4c0e8975c9b65fbacb519455e06a6cca9c3993c1) )
ROM_END


DRIVER_INIT( unkpcp )
{
}

GAME(199?,  up_dbldx,  0,  unkpcp,  unkpcp,  unkpcp,  ROT0,  "Pcp",    "Double Deluxe (Pcp)",   GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME(199?,  up_nudbk,  0,  unkpcp,  unkpcp,  unkpcp,  ROT0,  "Pcp",    "Nudge Break (Pcp)",     GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME(199?,  up_sstrk,  0,  unkpcp,  unkpcp,  unkpcp,  ROT0,  "Pcp",    "Starstruck (Pcp)",      GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME(199?,  up_xpres,  0,  unkpcp,  unkpcp,  unkpcp,  ROT0,  "Pcp",    "Xpress (Pcp)",          GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME(199?,  up_roll,   0,  unkpcp,  unkpcp,  unkpcp,  ROT0,  "Pcp",    "Roll Up (Pcp)",         GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME(199?,  up_bluec,  0,  unkpcp,  unkpcp,  unkpcp,  ROT0,  "Pcp",    "Blue Chip (Pcp)",       GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
