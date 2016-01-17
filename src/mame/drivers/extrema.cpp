// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Extrema Russian Video Fruit Machines */
/*
  Appear to be Z80 based, some rom scramble?

  Maski Show has what appears to be a HDD image in 'WHX' (WinHex Backup) format - convert it to CHD
  Other games don't seem to have such images, missing.

  Roms are strange sizes, they appear to be cut at the end of the last block of data, some sets contain
  roms padded to normal sizes.

*/

#include "emu.h"
#include "cpu/z80/z80.h"


class extrema_state : public driver_device
{
public:
	extrema_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	required_device<cpu_device> m_maincpu;
};




static ADDRESS_MAP_START( extrema_map, AS_PROGRAM, 8, extrema_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( extrema_portmap, AS_IO, 8, extrema_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


static INPUT_PORTS_START( extrema )
INPUT_PORTS_END


static MACHINE_CONFIG_START( extrema, extrema_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(extrema_map)
	MCFG_CPU_IO_MAP(extrema_portmap)
MACHINE_CONFIG_END



ROM_START( maski )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "maskishow_sys_v.42.05.bin", 0x0000, 0x068000, CRC(211e6e34) SHA1(e7b5b2724de2c93c1a61bbb09646204d3e33d975) )

	ROM_REGION( 0x100000, "altrevs", 0 ) // alt revisions - split later
	ROM_LOAD( "maskishow_sys_v.42.11.bin", 0x0000, 0x068000, CRC(e21b817c) SHA1(2f951c37e617d3b5bc127735069d0e13f27e002a) )
	ROM_LOAD( "maski show_sys_v43.07.bin", 0x0000, 0x070000, CRC(4f4b6b05) SHA1(e1b46bf4f40e5ba92d53b9efee426a9700597537) )
	ROM_LOAD( "maski show_sys_v43.09.bin", 0x0000, 0x070000, CRC(d0862569) SHA1(e5eb8fcf60ed02d5c9090841e1f859d431ff1138) )

	ROM_REGION( 0x2000000, "drive", 0 ) // HDD? (if so convert to CHD)
	ROM_LOAD( "maski show 42.05.whx", 0x0000, 0x18bd8dd, CRC(e4c6b921) SHA1(c1349a5d5b6ca457696fbeef7027e75a96f91c37) )
ROM_END

#define MISSING_DISK \
	DISK_REGION( "ata:0:hdd:image" ) \
	DISK_IMAGE( "extrema_hdd", 0, NO_DUMP )

ROM_START( adults )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "adult only_4317rus.bin", 0x0000, 0x068000, CRC(fecb5f82) SHA1(5f90391a5307edf1957432ea14fd468f33ad6a6b) )
	ROM_REGION( 0x100000, "altrevs", 0 ) // alt revisions - split later
	ROM_LOAD( "aov43_23.bin", 0x0000, 0x080000, CRC(25eb34ab) SHA1(d92391809764fc2bf4336b81b1f3f2e6d9bcd811) )

	MISSING_DISK
ROM_END

ROM_START( bloto )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "bloto_3008.bin", 0x0000, 0x020000, CRC(dcd46ab5) SHA1(b4817d6e6db6096e8fe2dd3ebad5657c3e72fe9a) )
	ROM_REGION( 0x100000, "altrevs", 0 ) // alt revisions - split later
	ROM_LOAD( "bloto_3016.bin", 0x0000, 0x020000, CRC(05731d21) SHA1(a12816d79d465649fd3da5ad341caf67831ada65) )

	MISSING_DISK
ROM_END

ROM_START( blpearl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "blpearl_sys_42.01.bin", 0x0000, 0x058000, CRC(6e3b3eda) SHA1(7cda7dd7ed487c8fe80967934fe444c80c368768) )
	ROM_REGION( 0x100000, "altrevs", 0 ) // alt revisions - split later
	ROM_LOAD( "blpearl_sys_42.03.bin", 0x0000, 0x058000, CRC(758bde12) SHA1(adfc4193edc6e0b71b16b20083a9b0635862ef8a) )
	ROM_LOAD( "blpearl_sys_45.02.bin", 0x0000, 0x058000, CRC(00e9b39d) SHA1(d2bd7175fef041106b50f52ce71361ccd127ce6c) )
	ROM_LOAD( "black pearl_v43.04.bin", 0x0000, 0x058000, CRC(e7c20ff5) SHA1(ef6b63cfe8f1f3a50942f397c5c15e0640e39942) )
	ROM_LOAD( "black pearl_v43.07.bin", 0x0000, 0x058000, CRC(6d839184) SHA1(94586b29b35e443c120524ff91cf353c221f30ee) )

	MISSING_DISK
ROM_END


ROM_START( grancan )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "grand canyon_sys_43.05.bin", 0x0000, 0x068000, CRC(bdf5426b) SHA1(3f7853482c12dd96f7ab67aac8799fdbcf2757b9) )
	ROM_REGION( 0x100000, "altrevs", 0 ) // alt revisions - split later
	ROM_LOAD( "grand canyon_sys_43.09.bin", 0x0000, 0x068000, CRC(9d27dc85) SHA1(5dce3ab40399a32b2cb7f07fa3d0cc621f3287d9) )
	ROM_LOAD( "grcanyon_sys_42.06.bin", 0x0000, 0x068000, CRC(6591e970) SHA1(69d8d8bb074b9a30593072f13e8ab6e0722e2aa3) )
	ROM_LOAD( "grcanyon_sys_42.13.bin", 0x0000, 0x068000, CRC(46f4562d) SHA1(01d3d1d3f0d1c3d5bf8f3c87da81ab5ba8356322) )

	MISSING_DISK
ROM_END

ROM_START( luckshel )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "luckyshell_v.42.10.bin", 0x0000, 0x068000, CRC(dc8d3d45) SHA1(7a6659f6ed5f0d162f394ed80c39c0cff915ab41) )
	ROM_REGION( 0x100000, "altrevs", 0 ) // alt revisions - split later
	ROM_LOAD( "luckyshell_v.42.25.bin", 0x0000, 0x068000, CRC(0e89e575) SHA1(60af8390106db28301a24340cd4e79ea8b8bfb5c) )

	MISSING_DISK
ROM_END

ROM_START( exsafar )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "safari_sys_v.42.04.bin", 0x0000, 0x070000, CRC(c4fbcd52) SHA1(0e40194909cd9dad49a597df2f2d793c72535544) )
	ROM_REGION( 0x100000, "altrevs", 0 ) // alt revisions - split later
	ROM_LOAD( "safari_sys_v.42.07.bin", 0x0000, 0x070000, CRC(d4d02370) SHA1(69ca8d074fc2a7a64012cb3db6c82e91ae8ed56f) )
	ROM_LOAD( "safari_sys_v43.08.bin", 0x0000, 0x070000, CRC(2a35ab64) SHA1(288624f9bedc57f5229f3d88677b0f2bf141c857) )
	ROM_LOAD( "safari_sys_v43.14.bin", 0x0000, 0x070000, CRC(61def527) SHA1(4b60f8c2c5845ece23244f41d5a71d6c6093b19c) )

	MISSING_DISK
ROM_END

ROM_START( strlink )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "strong link_43.48.bin", 0x0000, 0x058000, CRC(fed4610e) SHA1(dd43765c1db9a86b46c56db6f5de4ddf7eb7b5f4) )
	ROM_REGION( 0x100000, "altrevs", 0 ) // alt revisions - split later
	ROM_LOAD( "stronglink_v43.45.bin", 0x0000, 0x058000, CRC(07dcc10b) SHA1(07a99d8ca4ad088177b5593fab49b4bc37a069a4) )

	MISSING_DISK
ROM_END

ROM_START( extrmth )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "trhunt_sys_34.02.bin", 0x0000, 0x048000, CRC(961d7011) SHA1(44c527d498061da5d84ed134c16ee7f95ed75376) )
	ROM_REGION( 0x100000, "altrevs", 0 ) // alt revisions - split later
	ROM_LOAD( "trhunt_sys_34.03.bin", 0x0000, 0x048000, CRC(003639db) SHA1(65d5b530deb424fe575ed51039b7bc4b271423e6) )

	MISSING_DISK
ROM_END

ROM_START( extrmti )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tresure island   sys v32.49.bin", 0x0000, 0x050000, CRC(73450ee4) SHA1(6b3583461d0aed2d4d6a53d9e18392f94bd7bbd6) )

	MISSING_DISK
ROM_END



GAME( 200?, maski,    0,          extrema, extrema, driver_device,  0,             ROT0,  "Extrema", "Maski Show (Russia) (Extrema)", MACHINE_IS_SKELETON)
GAME( 200?, adults,   0,          extrema, extrema, driver_device,  0,             ROT0,  "Extrema", "Adults Only (Russia) (Extrema)", MACHINE_IS_SKELETON)
GAME( 200?, bloto,    0,          extrema, extrema, driver_device,  0,             ROT0,  "Extrema", "Blits Loto (Russia) (Extrema)", MACHINE_IS_SKELETON)
GAME( 200?, blpearl,  0,          extrema, extrema, driver_device,  0,             ROT0,  "Extrema", "Black Pearl (Russia) (Extrema)", MACHINE_IS_SKELETON)
GAME( 200?, grancan,  0,          extrema, extrema, driver_device,  0,             ROT0,  "Extrema", "Grand Canyon (Russia) (Extrema)", MACHINE_IS_SKELETON)
GAME( 200?, luckshel, 0,          extrema, extrema, driver_device,  0,             ROT0,  "Extrema", "Lucky Shell (Russia) (Extrema)", MACHINE_IS_SKELETON)
GAME( 200?, exsafar,  0,          extrema, extrema, driver_device,  0,             ROT0,  "Extrema", "Safari (Russia) (Extrema)", MACHINE_IS_SKELETON)
GAME( 200?, strlink,  0,          extrema, extrema, driver_device,  0,             ROT0,  "Extrema", "Strong Link (Russia) (Extrema)", MACHINE_IS_SKELETON)
GAME( 200?, extrmth,  0,          extrema, extrema, driver_device,  0,             ROT0,  "Extrema", "Treasure Hunt (Russia) (Extrema)", MACHINE_IS_SKELETON)
GAME( 200?, extrmti,  0,          extrema, extrema, driver_device,  0,             ROT0,  "Extrema", "Treasure Island (Russia) (Extrema)", MACHINE_IS_SKELETON)
