// license:BSD-3-Clause
// copyright-holders:David Haywood

// **** SKELETON DRIVER **** original removed due to unresolved licensing.

/***************************************************************************

    Car Jamboree
    Omori Electric CAD (OEC) 1983

    c14                c.d19
    c13                c.d18           c10
    c12                                c9
    c11         2125   2125
                2125   2125
                2125   2125  2114 2114
                2125   2125  2114 2114
                2125   2125            c8
                2125   2125            c7
                                       c6
                                       c5
                                       c4
                                       c3
    5101                               c2
    5101                               c1
                                       6116
    18.432MHz
              6116
    Z80A      c15
                                       Z80A
           8910         SW
           8910

***************************************************************************/

#include "emu.h"

class carjmbre_state : public driver_device
{
public:
	carjmbre_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{}
};


static INPUT_PORTS_START( carjmbre )
INPUT_PORTS_END

static MACHINE_CONFIG_START( carjmbre, carjmbre_state )
MACHINE_CONFIG_END

ROM_START( carjmbre )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c1",      0x0000, 0x1000, CRC(62b21739) SHA1(710e5c52f27603aa8f864f6f28d7272f21271d60) )
	ROM_LOAD( "c2",      0x1000, 0x1000, CRC(9ab1a0fa) SHA1(519cf67b98e62b2b42232788ba01ab6637880afc) )
	ROM_LOAD( "c3",      0x2000, 0x1000, CRC(bb29e100) SHA1(93e3cfcf7f8b0b36327f402d9a64c04c3b2c7549) )
	ROM_LOAD( "c4",      0x3000, 0x1000, CRC(c63d8f97) SHA1(9f08fd1cd24a1fb4011864c06580985e009d9af4) )
	ROM_LOAD( "c5",      0x4000, 0x1000, CRC(4d593942) SHA1(30cc649a4be3d7f3705f55d8d0dadb0b63d59ec9) )
	ROM_LOAD( "c6",      0x5000, 0x1000, CRC(fb576963) SHA1(5bf5c54a7c12aa55272629c12b414bf49cda0f1f) )
	ROM_LOAD( "c7",      0x6000, 0x1000, CRC(2b8c4511) SHA1(428a48d6b14455d66720a115bc5f35293dc50de7) )
	ROM_LOAD( "c8",      0x7000, 0x1000, CRC(51cc22a7) SHA1(f614368bfee04f084c70bf145801ac46e5631acb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c15",     0x0000, 0x1000, CRC(7d7779d1) SHA1(f8f5246be4cc9632076d3330fc3d3343b911dfee) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "c9",     0x0000, 0x1000, CRC(2accb821) SHA1(ce2804536fc1abd3377dc864c8c9976ca28c1b6e) )
	ROM_LOAD( "c10",    0x1000, 0x1000, CRC(75ddbe56) SHA1(5e1363967a822265618793ccb74bf3ef5e0e00b5) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "c11",    0x0000, 0x1000, CRC(d90cd126) SHA1(7ee110cf19b45ee654016ba0ce92f3db6ea2ed92) )
	ROM_LOAD( "c12",    0x1000, 0x1000, CRC(b3bb39d7) SHA1(89c901be6fae2356ce4d2653e94bf28d6bcf41fe) )
	ROM_LOAD( "c13",    0x2000, 0x1000, CRC(3004010b) SHA1(00d5d2185014159112eb90d8ed50092a3b4ab664) )
	ROM_LOAD( "c14",    0x3000, 0x1000, CRC(fb5f0d31) SHA1(7a27af91efc836bb48c6ed3b283b7c5f7b31c4b5) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "c.d19",  0x0000, 0x0020, CRC(220bceeb) SHA1(46b9f867d014596e2aa7503f104dc721965f0ed5) )
	ROM_LOAD( "c.d18",  0x0020, 0x0020, CRC(7b9ed1b0) SHA1(ec5e1f56e5a2fc726083866c08ac0e1de0ed6ace) )
ROM_END

GAME( 1983, carjmbre, 0, carjmbre, carjmbre, driver_device, 0, ROT90, "Omori Electric Co., Ltd.", "Car Jamboree", MACHINE_IS_SKELETON )
