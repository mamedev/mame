/* Electrocoin 'OXO' hardware type (Phoenix?)

 at least some of these are multiple part cabs, with both top and bottom units all linked together
 see the 'Top Box Roms' in some of the sets.

 This HW seems similar, but not idential to the Pyramid HW in ecoinf3.c

*/


#include "emu.h"
#include "cpu/z180/z180.h"

class ecoinf2_state : public driver_device
{
public:
	ecoinf2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};


static WRITE8_HANDLER( ox_port5c_out_w )
{
	// Watchdog?
}

static ADDRESS_MAP_START( oxo_memmap, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( oxo_portmap, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x5c, 0x5c) AM_WRITE(ox_port5c_out_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( ecoinf2 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN0:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN0:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN0:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN0:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN0:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN0:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN0:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN1:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN1:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN1:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN1:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN1:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN1:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN1:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN2:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN2:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN2:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN2:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN2:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN2:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN2:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN3:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN3:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN3:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN3:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN3:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN3:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN3:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN4:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN4:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN4:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN4:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN4:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN4:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN4:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "IN5:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN5:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN5:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN5:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN5:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN5:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN5:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN5:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN6")
	PORT_DIPNAME( 0x01, 0x01, "IN6:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN6:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN6:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN6:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN6:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN6:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN6:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN6:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN7")
	PORT_DIPNAME( 0x01, 0x01, "IN7:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN7:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN7:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN7:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN7:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN7:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN7:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN7:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static MACHINE_CONFIG_START( ecoinf2_oxo, ecoinf2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180,4000000) // some of these hit invalid opcodes with a plain z80, some don't?
	MCFG_CPU_PROGRAM_MAP(oxo_memmap)
	MCFG_CPU_IO_MAP(oxo_portmap)
MACHINE_CONFIG_END



/********************************************************************************************************************
 ROMs for OXO Hw type
********************************************************************************************************************/

ROM_START( ec_oxocg )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// all just Z80 roms, no header information the 'TOP' rom is rather different to the rest
	ROM_LOAD( "ocla-4.1", 0x0000, 0x010000, CRC(fe1db86d) SHA1(7718ecafc562bad39cefa15a0df46f081e6045af) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ocla-4.1p", 0x0000, 0x010000, CRC(f24b2cac) SHA1(96f026df3f3915bee89ecc26725e4a7e861fddce) )
	ROM_LOAD( "ocsd-5.2", 0x0000, 0x010000, CRC(28c86aae) SHA1(cafdff7ebc57ef4163b40381e84dd2ac2c24937d) )
	ROM_LOAD( "ocsd-5.3p", 0x0000, 0x010000, CRC(9d422e21) SHA1(9e71ca53054c02c9fb6b23055fa7a5747648bac3) )
	ROM_LOAD( "oxo-btm4.0", 0x0000, 0x010000, CRC(70c8e340) SHA1(4219a493215e2e296a867a3c7ea4cf48356a8842) )
	ROM_LOAD( "oxo-btm4.1p", 0x0000, 0x010000, CRC(b970d6f2) SHA1(df2896bb8e540b67b7427c26f247b0627f6f5f15) )
	ROM_LOAD( "oxo-top4.0", 0x0000, 0x010000, CRC(1b3d8225) SHA1(1951849b3b6966019d5c4c7debef8c5cc6b0259c) )
ROM_END

/*
     ELECTROCOIN  OXO  CLUB

  Oxo-2.3n ---------- 54AE     ?25
  Oxo-2.3p ---------- 55AD    ?25
  Oxo-2-2T.box ----  9976     ?25
  Oxo-nv7.2-3 ------  3E15    ?25

  Oxo-1.6n ---------- EC97      ?5 / ?15
  Oxo-1.8p ---------- 13BD     ?5 / ?15
  Oxo-1-2T.box ---- 9D35      ?5 / ?15
*/

ROM_START( ec_oxocl )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// looks like a similar config to set above, the 't' roms being the TOP roms
	ROM_LOAD( "oxo-1.6n", 0x0000, 0x010000, CRC(5c4637c5) SHA1(923a8d50b2b8a7d97d6d1994dafde3aafe0f8c45) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ocn7 v18 non protocol.hex", 0x0000, 0x02680d, CRC(91755ca8) SHA1(38dea02258e4cf731680621c96ebd473e74ae0f6) ) // convert from HEX and check
//  ROM_LOAD( "oxo club.txt", 0x0000, 0x000127, CRC(2ae1750e) SHA1(e15bcc78bcdb4672a77dd46b8f40313dc4a88c59) )
	ROM_LOAD( "oxo-1-2t.box", 0x0000, 0x010000, CRC(8fd03d19) SHA1(b3df92a8a4e0f4b8f813758aa4e881f45a04c8e4) )
	ROM_LOAD( "oxo-1.8p", 0x0000, 0x010000, CRC(26a40f47) SHA1(2c61fa010efc4684e2c53d58a81bd8071246b3f1) )
	ROM_LOAD( "oxo-2-2t.box", 0x0000, 0x010000, CRC(5fac6c82) SHA1(94b9db912fe85dd4bff099492dedd0b2edbec954) )
	ROM_LOAD( "oxo-2.3n", 0x0000, 0x010000, CRC(37bdce39) SHA1(5f38a09a4acfddd63b9fb88eb429390bccec6d9c) )
	ROM_LOAD( "oxo-2.3p", 0x0000, 0x010000, CRC(123e733d) SHA1(41fcb8a15742115ad69d861685f9dffb6242c563) )
	ROM_LOAD( "oxo-nv7.2-3", 0x0000, 0x010000, CRC(7d53520b) SHA1(33af51b9e3ae9f4d923058a79850cb95a141a9a6) )
ROM_END


ROM_START( ec_oxogb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ocla54 non protocol.hex", 0x0000, 0x02680d, CRC(08c18728) SHA1(6cc004db3f7c43b8b7a685becc5de1c84c131048) ) // convert from HEX and check
ROM_END


ROM_START( ec_oxorl )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// again same type of thing as ec_oxocg / ec_oxocl
	ROM_LOAD( "oxo-btm4.0", 0x0000, 0x010000, CRC(70c8e340) SHA1(4219a493215e2e296a867a3c7ea4cf48356a8842) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "or25 v4.2 dereg non protocol.hex", 0x0000, 0x02680d, CRC(9a9489f5) SHA1(4587fe7bb0123559930726d9b7197d7a525218f8) ) // convert from HEX and check
	ROM_LOAD( "or25 v4.2 dereg protocol.hex", 0x0000, 0x02680d, CRC(4c3a2b4e) SHA1(e18c8c1b8c2fbc8c84c9632d6fcda76ed8a9303a) ) // convert from HEX and check
	ROM_LOAD( "or5 np.hex", 0x0000, 0x02680d, CRC(15a501eb) SHA1(b66209c02183a222f82a4671962348ae137dc162) ) // convert from HEX and check
	ROM_LOAD( "oxo-btm4.1p", 0x0000, 0x010000, CRC(b970d6f2) SHA1(df2896bb8e540b67b7427c26f247b0627f6f5f15) )
	ROM_LOAD( "oxo-top4.0", 0x0000, 0x010000, CRC(1b3d8225) SHA1(1951849b3b6966019d5c4c7debef8c5cc6b0259c) )
	ROM_LOAD( "oxoreels.2bt", 0x0000, 0x010000, CRC(bfa178ff) SHA1(d433c1f5bc216d76f311566cc80d148fb76eab71) )
	ROM_LOAD( "oxoreels.3dr", 0x0000, 0x010000, CRC(d629133b) SHA1(2a25540885d34bf38528cecd360953818beb6197) )
	ROM_LOAD( "oxoreels.btm", 0x0000, 0x010000, CRC(db408784) SHA1(e53d3419fc6fa04970c7ce52bf7afb9baf022a27) )
	ROM_LOAD( "oxoreels.top", 0x0000, 0x010000, CRC(1b3d8225) SHA1(1951849b3b6966019d5c4c7debef8c5cc6b0259c) )
ROM_END


ROM_START( ec_oxorv )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// again same type of thing as ec_oxocg / ec_oxocl
	ROM_LOAD( "rev-10-0.btm", 0x0000, 0x010000, CRC(dea90334) SHA1(1023e193fa0973e09e8fbbc559935ce5dd32a093) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "nrev 13.0 gala compak.hex", 0x0000, 0x02680d, CRC(1537716f) SHA1(0f9d2cd7387fca7db355fea69bede0b15dcb9c2f) ) // convert from HEX and check
	ROM_LOAD( "nrev 13.0 gala connexus.hex", 0x0000, 0x02680d, CRC(11eb0066) SHA1(4e836d1a05ba3d7b7ab2fa8e6decc7307daa0b6d) ) // convert from HEX and check
	ROM_LOAD( "nrev 13.0 non protocol.hex", 0x0000, 0x02680d, CRC(bd2145d5) SHA1(a15cf6081e2b6f4763bf577f31b7b8cc06e8e3de) ) // convert from HEX and check
	ROM_LOAD( "nrev 13.0 protocol.hex", 0x0000, 0x02680d, CRC(5ae33e51) SHA1(fdabedec9c9adde51fcd3a2ebe000b15c663bcfb) ) // convert from HEX and check
	ROM_LOAD( "nrev 13.0 rank non protocol.hex", 0x0000, 0x02680d, CRC(35d14c07) SHA1(a7a4a1dc71fe197e97704bcc971893123eb2bc55) ) // convert from HEX and check
	ROM_LOAD( "nrev 13.0 rank protocol.hex", 0x0000, 0x02680d, CRC(e37feebc) SHA1(185dc87b0187b89cc9bc66c8bd8b83217bdff82a) ) // convert from HEX and check
	ROM_LOAD( "rev-10-0.top", 0x0000, 0x010000, CRC(7ed49cd2) SHA1(45fc13d4fbd3d9839ad0c5ac1db391199f1d571e) )
	ROM_LOAD( "rev12-0.top", 0x0000, 0x010000, CRC(029b2036) SHA1(f94409de013d189074d1f64f80d211c888413c28) )
	ROM_LOAD( "rev13-0.bin", 0x0000, 0x010000, CRC(90741b8d) SHA1(5496e6e79efae6a657524b5ce050cae9ccbdd981) )
	ROM_LOAD( "rev13-0p.bin", 0x0000, 0x010000, CRC(9fafd48c) SHA1(f34130233e68fe84e5d4941619a93ebbb6c4f900) )
	ROM_LOAD( "revo120 top.hex", 0x0000, 0x02680d, CRC(0b578ff6) SHA1(956e5ce9fe91d28043fbcff83163663f5aa71909) )
	ROM_LOAD( "revo2-1.btm", 0x0000, 0x010000, CRC(5d30662f) SHA1(f808c925732c5802ba377034d88c3840cae11cb0) )
	ROM_LOAD( "revo2-1p.btm", 0x0000, 0x010000, CRC(52eba92e) SHA1(5223e69d5c9fa7b8819e7a0267c25fa79c020c64) )
ROM_END


ROM_START( ec_suprl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	// again same type of thing as ec_oxocg / ec_oxocl with the top / bottom roms
	ROM_LOAD( "srv11.btm", 0x0000, 0x010000, CRC(e68b5a8a) SHA1(b9a1b76f93ab62b5c5d8d56a1210e2d8194bb5b6) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "sr0520p.0 non protocol.hex", 0x0000, 0x02680d, CRC(864baa72) SHA1(3212dd51b5fe98b9c0b16f8285397c3d68ca4fd4) ) // convert from HEX and check
	ROM_LOAD( "sr0520p.0 protocol.hex", 0x0000, 0x02680d, CRC(afbbbef4) SHA1(a060db1b8d648b8890ed68f0cf9934b64abdb9fa) ) // convert from HEX and check
	ROM_LOAD( "sr05b1.8hex", 0x0000, 0x02680d, CRC(12fca690) SHA1(8408159ff7b4a5db6db5fcb08ae636a7e6a1a9b8) ) // convert from HEX and check
	ROM_LOAD( "sr25b16.hex", 0x0000, 0x02680d, CRC(87c33f5f) SHA1(f1ff058b8f670503f73b1fddb5a58becd671294b) ) // convert from HEX and check
	ROM_LOAD( "srle v1.0 protocol.hex", 0x0000, 0x02680d, CRC(57bec009) SHA1(ebf99f6ca5f20e9a30ba694cb3d17f6c8b5827f5) ) // convert from HEX and check
	ROM_LOAD( "srt30.hex", 0x0000, 0x02680d, CRC(d6b970fa) SHA1(d31cc4ae7a920b73f2b377d4e36be56422bc3632) ) // convert from HEX and check


	ROM_LOAD( "srv11.top", 0x0000, 0x010000, CRC(05712727) SHA1(b2e29faa7babe560ba928870e96afa3893ba8955) )
	ROM_LOAD( "srv3-0.btm", 0x0000, 0x010000, CRC(d629133b) SHA1(2a25540885d34bf38528cecd360953818beb6197) )
	ROM_LOAD( "srv3-0.top", 0x0000, 0x010000, CRC(05712727) SHA1(b2e29faa7babe560ba928870e96afa3893ba8955) )

	ROM_REGION( 0x400000, "oki", 0 )
	ROM_LOAD( "supersnd.hex", 0x0000, 0x26812e, CRC(90d96c92) SHA1(18d73c1dc9fe6c26ff832d024ddb9824ddeacf90) )
	ROM_LOAD( "srv3-0.snd", 0x0000, 0x100000, CRC(c40e0609) SHA1(00a2fe56786517b7fa3338918cb8a3bb226f09d8) )
	ROM_LOAD( "srv11.snd", 0x0000, 0x100000, CRC(cf4d217a) SHA1(28eec63bd0c8bab7524e4e939485d174a6852b10) )
ROM_END

// this has no rom for a top box.. might be missing
ROM_START( ec_rcc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// Just Z80 roms, no identification
	ROM_LOAD( "rcas20p4.5", 0x0000, 0x010000, CRC(54a1ddde) SHA1(e98b6dbf0256324fe1cdddbe4b89958d3d5f1233) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "rcas20p4.5d", 0x0000, 0x010000, CRC(b42e2415) SHA1(fcc76977a920b6116c5e9029340aa51abb2ab713) )
	ROM_LOAD( "rcas25p4.5", 0x0000, 0x010000, CRC(0aeb0332) SHA1(1b2f2332ac30736892f72b7771fa0825a95f19ad) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "rcas4-5.snd", 0x0000, 0x100000, CRC(8d9403e1) SHA1(8a8da6f99a524646a8c689861a5cd6aafeef700b) )
ROM_END

DRIVER_INIT( ecoinf2 )
{

}

// OXO wh type (Phoenix?) (watchdog on port 5c?)
GAME( 19??, ec_oxocg,   0		 , ecoinf2_oxo,   ecoinf2,   ecoinf2,	ROT0,  "Electrocoin", "Oxo Classic Gold (Electrocoin) (?)"		, GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 19??, ec_oxocl,   0		 , ecoinf2_oxo,   ecoinf2,   ecoinf2,	ROT0,  "Electrocoin", "Oxo Club  (Electrocoin) (?)"		, GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 19??, ec_oxogb,   0		 , ecoinf2_oxo,   ecoinf2,   ecoinf2,	ROT0,  "Electrocoin", "Oxo Golden Bars (Electrocoin) (?)"		, GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 19??, ec_oxorl,   0		 , ecoinf2_oxo,   ecoinf2,   ecoinf2,	ROT0,  "Electrocoin", "Oxo Reels (Electrocoin) (?)"		, GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 19??, ec_oxorv,   0		 , ecoinf2_oxo,   ecoinf2,   ecoinf2,	ROT0,  "Electrocoin", "Oxo Revolution (Electrocoin) (?)"		, GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 19??, ec_suprl,   0		 , ecoinf2_oxo,   ecoinf2,   ecoinf2,	ROT0,  "Electrocoin", "Super Reels (Electrocoin) (?)"		, GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 19??, ec_rcc,	    0		 , ecoinf2_oxo,   ecoinf2,   ecoinf2,	ROT0,  "Electrocoin", "Royal Casino Club (Electrocoin) (?)"		, GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
