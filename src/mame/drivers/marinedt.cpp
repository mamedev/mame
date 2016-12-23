// license:BSD-3-Clause
// copyright-holders:David Haywood

// **** SKELETON DRIVER **** original removed due to unresolved licensing.

/*

Marine Date
Taito 1981

PCB Layout
----------

Top board

MGO70001
MGN00001
 |---------------------------------------------|
 | VOL   VR1  VR2  VR3  VR4  VR5  VR6  VR7     |
 |  LM3900 LM3900 LM3900 LM3900 LM3900 LM3900 |-|
 |MB3712                                      |P|
 |   4006  LM3900 LM3900 LM3900               | |
 |2  4030                                     |-|
 |2                                            |
 |W                                  DSW(8)    |
 |A                                           |-|
 |Y   HD14584     NE555       MG17   DSW(8)   |Q|
 |                                            | |
 |    HD14584                                 |-|
 |          HD14584                            |
 |---------------------------------------------|
Notes: (PCB contains lots of resistors/caps/transistors etc)
      MG17    - 82S123 bipolar PROM (no location on PCB)
      MB3712  - Hitachi MB3712 Audio Power Amplifier
      LM3900  - Texas Instruments LM3900 Quad Operational Amplifier
      HD14584 - Hitachi HD14584 Hex schmitt Trigger
      NE555   - NE555 Timer
      4006    - RCA CD4006 18-Stage Static Register
      4030    - RCA CD4030 Quad Exclusive-Or Gate
      VR*     - Volume pots for each sound
      VOL     - Master Volume pot


Middle board

MGO70002
MGN00002
 |---------------------------------------------|
 |                                    MG15.1A  |
|-|                                   MG14.2A |-|
|S|                                           |Q|
| |                                           | |
|-|               MG16.4E                     |-|
 |                                             |
 |                                             |
|-|    MG13.6H              MG12.6C           |-|
|R|                                           |P|
| |                                   PC3259  | |
|-|                                   PC3259  |-|
 |                                             |
 |---------------------------------------------|
Notes:
      MG12/13    - Hitachi HN462532 4kx8 EPROM
      MG14/15/16 - 82S123 bipolar PROM
      PC3259     - PC3259 8025 H08 unknown DIP24 IC. Package design indicates it was manufactured by Fujitsu


Lower board

AA017779
sticker: MGN00003
sticker: CLN00002
 |---------------------------------------------|
 | 9.987MHz               2114                 |
|-|                       2114                 |
|R|             MG07.10D       2114            |
| |             MG06.9D        2114            |
|-|                            2114           1|
 |              MG05.7D                       8|Edge
 |              MG04.6D                       W|Connector 'T'
|-|             MG03.5D                       A|
|S|             MG02.4D                       Y|
| |             MG01.3D  MG09.4F               |
|-|                      MG10.3F               |
 |              Z80      MG11.1F               |
 |---------------------------------------------|
Notes:
      Z80  - Clock 2.49675MHz [9.987/4]
      2114 - 1kx4 SRAM
      All EPROMs are 2716
      Wire jumpers for ROM configuration - J1 open
                                           J2 1-2, 3-9, 4-8, 5-7
                                           J4 1-2, 4-5, 7-8, 10-11

Top and Middle PCBs are plugged in with the solder-sides together.
Lower PCB is plugged in with components facing up.
-------------------------------------------------------------------------
*/

#include "emu.h"

class marinedt_state : public driver_device
{
public:
	marinedt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }
};


static INPUT_PORTS_START( marinedt )
INPUT_PORTS_END


static MACHINE_CONFIG_START( marinedt, marinedt_state )
MACHINE_CONFIG_END

ROM_START( marinedt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mg01.3d",     0x0000, 0x0800, CRC(ad09f04d) SHA1(932fc973b4a2fbbebd7e6437ed30c8444e3d4afb))
	ROM_LOAD( "mg02.4d",     0x0800, 0x0800, CRC(555a2b0f) SHA1(143a8953ce5070c31dc4c1f623833b2a5a2cf657))
	ROM_LOAD( "mg03.5d",     0x1000, 0x0800, CRC(2abc79b3) SHA1(1afb331a2c0e320b6d026bc5cb47a53ac3356c2a))
	ROM_LOAD( "mg04.6d",     0x1800, 0x0800, CRC(be928364) SHA1(8d9ae71e2751c009187e41d84fbad9519ab551e1) )
	ROM_LOAD( "mg05.7d",     0x2000, 0x0800, CRC(44cd114a) SHA1(833165c5c00c6e505acf29fef4a3ae3f9647b443) )
	ROM_LOAD( "mg06.9d",     0x2800, 0x0800, CRC(a7e2c69b) SHA1(614fc479d13c1726382fe7b4b0379c1dd4915af0) )
	ROM_LOAD( "mg07.10d",    0x3000, 0x0800, CRC(b85d1f9a) SHA1(4fd3e76b1816912df84477dba4655d395f5e7072) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "mg09.4f",     0x0000, 0x0800, CRC(f4c349ca) SHA1(077f65eeac616a778d6c42bb95677fa2892ab697) )
	ROM_LOAD( "mg10.3f",     0x0800, 0x0800, CRC(b41251e3) SHA1(e125a971b401c78efeb4b03d0fab43e392d3fc14) )
	ROM_LOAD( "mg11.1f",     0x1000, 0x0800, CRC(50d66dd7) SHA1(858d1d2a75e091b0e382d964c5e4ddcd8e6f07dd))

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "mg12.6c",     0x0000, 0x1000, CRC(7c6486d5) SHA1(a7f17a803937937f05fc90621883a0fd44b297a0) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "mg13.6h",     0x0000, 0x1000, CRC(17817044) SHA1(8c9b96620e3c414952e6d85c6e81b0df85c88e7a) )

	ROM_REGION( 0x0080, "proms", 0 )
	ROM_LOAD( "mg14.2a",  0x0000, 0x0020, CRC(f75f4e3a) SHA1(36e665987f475c57435fa8c224a2a3ce0c5e672b) )
	ROM_LOAD( "mg15.1a",  0x0020, 0x0020, CRC(cd3ab489) SHA1(a77478fb94d0cf8f4317f89cc9579def7c294b4f) )
	ROM_LOAD( "mg16.4e",  0x0040, 0x0020, CRC(92c868bc) SHA1(483ae6f47845ddacb701528e82bd388d7d66a0fb) )
	ROM_LOAD( "mg17.bpr", 0x0060, 0x0020, CRC(13261a02) SHA1(050edd18e4f79d19d5206f55f329340432fd4099) )
ROM_END

GAME( 1981, marinedt, 0, marinedt, marinedt, driver_device, 0, ROT270, "Taito", "Marine Date", MACHINE_IS_SKELETON )
