// license:BSD-3-Clause
// copyright-holders:David Haywood

// **** SKELETON DRIVER **** original removed due to unresolved licensing.

/***************************************************************************

    Popper

    Omori Electric CAD (OEC) 1983


    PPR-12


    18.432MHz                    6148 6148
                                 6148 4148
                    2128
                    2128
                    2128
     2128
                p3                p6
    z80A        p2                p5
      p0        p1             p4
                z80A
       8910
     8910              SW2 SW1    p.m4 p.m3


***************************************************************************/

#include "emu.h"


class popper_state : public driver_device
{
public:
	popper_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }
};


static INPUT_PORTS_START( popper )
INPUT_PORTS_END

static MACHINE_CONFIG_START( popper, popper_state )
MACHINE_CONFIG_END

ROM_START( popper )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1",   0x0000, 0x2000, CRC(56881b70) SHA1(d3ade7a54a6cb8a0babf0d667a6b27f492a739dc) )
	ROM_LOAD( "p2",   0x2000, 0x2000, CRC(a054d9d2) SHA1(fcd86e7247b40cf07ea595a64c104b99b0e93ced) )
	ROM_LOAD( "p3",   0x4000, 0x2000, CRC(6201928a) SHA1(53b571b9f2c0568f10cd974641863c2e00777b46) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p0",   0x0000, 0x1000, CRC(ef5f7c5b) SHA1(c63a3d9ef2868ad7eaacddec810d62d2e124dc15) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "p4",   0x0000, 0x2000, CRC(86203349) SHA1(cce2dd3fa786c2fb3ca80e7b93adf94db3b46b01) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "p5",   0x0000, 0x2000, CRC(a21ac194) SHA1(2c0e3df8981a12d383b1c4619a0b95a7c2d176a7) )
	ROM_LOAD( "p6",   0x2000, 0x2000, CRC(d99fa790) SHA1(201271ee4fb812236a38cb5f9070ac29e8186097) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "p.m3", 0x0000, 0x0020, CRC(713217aa) SHA1(6083c3432bf94c9e983fcc79171529f519c86105) )
	ROM_LOAD( "p.m4", 0x0020, 0x0020, CRC(384de5c1) SHA1(892c89a01c11671c5708113b4e4c27b84be37ea6) )
ROM_END

GAME( 1983, popper, 0, popper, popper, driver_device, 0, ROT90, "Omori Electric Co., Ltd.", "Popper", MACHINE_IS_SKELETON )
