// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Electrocoin Older (original?) HW type

  this appears to be the earliest of the Electrocoin hardware
  it has a smaller rom space than ecoinfr.c and apparently
  lacks any form of protection PIC.

*/
#include "emu.h"
#include "cpu/z80/z80.h"


namespace {

class ecoinf1_state : public driver_device
{
public:
	ecoinf1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void ecoinf1_older(machine_config &config);

private:
	void older_memmap(address_map &map) ATTR_COLD;
	void older_portmap(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


void ecoinf1_state::older_memmap(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0x4000, 0x4fff).ram();
}

void ecoinf1_state::older_portmap(address_map &map)
{
	map.global_mask(0xff);
}



static INPUT_PORTS_START( ecoinf1 )
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


void ecoinf1_state::ecoinf1_older(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ecoinf1_state::older_memmap);
	m_maincpu->set_addrmap(AS_IO, &ecoinf1_state::older_portmap);
}


/********************************************************************************************************************
  Roms for OLDER hw type
********************************************************************************************************************/

// it's possible (likely) that all sets should use
// ROM_LOAD( "barxsnd.bin", 0x0000, 0x001000, CRC(7d37fda1) SHA1(fb906615067887d9daecdbc741cfa4ac710c4627) )
// for sound (currently in the ecoinfr.c sets)

ROM_START( ec_bar5 )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "bar5.5a4", 0x0000, 0x1000, CRC(0b12219d) SHA1(140a58afbf713e11f819e5154519b32e822bd1e3) )
	ROM_LOAD( "bar5.5a3", 0x1000, 0x1000, CRC(53185002) SHA1(9cd98ba871fdaa56dfcef0fc285c8537886ff4bd) )
	ROM_LOAD( "bar5.5a2", 0x2000, 0x1000, CRC(82b994e6) SHA1(19e63cb6f689787b74cad610a185f20ae3881238) )
ROM_END



ROM_START( ec_barxo )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "barxprog4.bin", 0x0000, 0x1000, CRC(b1a6924e) SHA1(e20f71073a74d0e26bb7abfa03b0bf5e977a4bfd) )
	ROM_LOAD( "barxprog3.bin", 0x1000, 0x1000, CRC(7febfb4e) SHA1(a9777db5a7ce43ab86fbdd1169a0fa129fda7774) )
	ROM_LOAD( "barxprog2.bin", 0x2000, 0x1000, CRC(f7abc4ee) SHA1(6996471bb45f7ad58ea28dcbe1270b7f7d844be7) )

	/* how these match up to the existing sets is uncertain, leave them here until things run well enough
	   to make a call, most just differ from existing roms by a few bytes so it's very difficult to tell */
	ROM_REGION( 0x3000, "altrevs", 0 )
	/* alt 'rom 2' dumps */
	ROM_LOAD( "a410p.bin", 0x0000, 0x1000, CRC(6c19d237) SHA1(9fa79bd0ab78685fed974e5b82ec419381337252) )
	ROM_LOAD( "a410p-.bin", 0x0000, 0x1000, CRC(0f1020f1) SHA1(e29cd3954f3cd0ae5c4a113f8922bd1f3be0e740) )

	/* incomplete set(?), no ROM 4 */
	ROM_LOAD( "barx6a3", 0x0000, 0x1000, CRC(8884d188) SHA1(64716d214ada873cca64a511fa569e96f1ade062) )
	ROM_LOAD( "barx6a2", 0x0000, 0x1000, CRC(522950ec) SHA1(89daf57b53d4752a4f5f4f0bef8d976a9fc877ce) )

	/* close to bx2010a2 */
	ROM_LOAD( "bx2010ha", 0x0000, 0x1000, CRC(db267418) SHA1(d4cc325aba62b0da5f63af37c64ea959ca77d91e) )

	/* Alt 'rom 2' roms similar to sets above */
	ROM_LOAD( "a20510.bin", 0x0000, 0x1000, CRC(b4a458a6) SHA1(acda6eece0c9e011bfb147a2f696dbdaa53ea9aa) )
	//ROM_LOAD( "a2054.bin", 0x0000, 0x1000, CRC(a77bcdb4) SHA1(bdb3fc19a933d609cea2a2a2dfc98d3589765484) ) // == bx5pa2
	ROM_LOAD( "a2058.bin", 0x0000, 0x1000, CRC(7b564e66) SHA1(eaec8efb566f9a017eb66cd2f4d8673971ab5db5) )
	ROM_LOAD( "a21010.bin", 0x0000, 0x1000, CRC(384b6bcf) SHA1(e9beba847b613ae881a3c7be637c2c38b8c1410f) )
	ROM_LOAD( "a2104.bin", 0x0000, 0x1000, CRC(2b94fedd) SHA1(d5da5604b1db9fadbae0a6bb7a1d76b1d80a19df) )
	ROM_LOAD( "a2108.bin", 0x0000, 0x1000, CRC(f7b97d0f) SHA1(a21512cf92a61fcdd9856f017fce06d280c222b7) )
ROM_END


ROM_START( ec_barxoa )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "bx2010a4", 0x0000, 0x1000, CRC(1d29d010) SHA1(b3f7a8b839770402b463d8ec72787c6ddade34bd) )
	ROM_LOAD( "bx2010a3", 0x1000, 0x1000, CRC(21903339) SHA1(7b515269a08ed3f181e1cd35bf4896a011f77806) )
	ROM_LOAD( "bx2010a2", 0x2000, 0x1000, CRC(6a28ea78) SHA1(bcdeabff309346103050f1da427913a23198c699) )
ROM_END

ROM_START( ec_barxob )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "barx5a4", 0x0000, 0x1000, CRC(7baa7ac3) SHA1(b8124ed5be68f9c4e81977018003f707064bbd58) )
	ROM_LOAD( "barx5a3", 0x1000, 0x1000, CRC(82bf22c7) SHA1(0b31c0f38181f3523776b44b211ee6b2f0fde341) )
	ROM_LOAD( "barx5a2", 0x2000, 0x1000, CRC(0f1839b9) SHA1(ab0f0dfa887d9c113a4971392b12a768b5b5977f) )
ROM_END

ROM_START( ec_barxoc )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "barx44c", 0x0000, 0x1000, CRC(bd8c9431) SHA1(b8393ec87969541ff56243b7ea1e5c908d8bf027) )
	ROM_LOAD( "barx34c", 0x1000, 0x1000, CRC(d105cbaa) SHA1(a38ed5fa437fbdd2d9efc575fe05a94180dbd90f) )
	ROM_LOAD( "barx24c", 0x2000, 0x1000, CRC(a513263e) SHA1(f83008ff34bc67bcf15f5433cfe2f6051763b75f) )
ROM_END

ROM_START( ec_barxod )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "bx54a4", 0x0000, 0x1000, CRC(7dc2d19e) SHA1(ad012de848b586ae8355ea300edce96d0f0ce2a8) )
	ROM_LOAD( "bx54a3", 0x1000, 0x1000, CRC(96cb0c73) SHA1(6fa1fc61cb2761871999516c6663b3948b35f6dc) )
	ROM_LOAD( "bx54a2", 0x2000, 0x1000, CRC(5b2d42ec) SHA1(abc394cad55786df99d8bea7a4497a338ec180d8) )
ROM_END

ROM_START( ec_barxoe )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "bx5pa4", 0x0000, 0x1000, CRC(34b4d7cb) SHA1(b2ff3c79e635fff8f02edc9c953cc619fb409aa5) )
	ROM_LOAD( "bx5pa3", 0x1000, 0x1000, CRC(2f3c45ed) SHA1(f18aba5ceb9385e37b5857ba28f80230388d0cd2) )
	ROM_LOAD( "bx5pa2", 0x2000, 0x1000, CRC(a77bcdb4) SHA1(bdb3fc19a933d609cea2a2a2dfc98d3589765484) )
ROM_END



ROM_START( ec_casbxo )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "bx4c.a4", 0x0000, 0x1000, CRC(f3da815d) SHA1(5eb20a0c384f9bd864bceb5e8f8b622e17b907fd) )
	ROM_LOAD( "bx4c.a3", 0x1000, 0x1000, CRC(a472d49f) SHA1(4814b28ed46afa931c5e4f19d829374ebd1f20c9) )
	ROM_LOAD( "bx4c.a2", 0x2000, 0x1000, CRC(f86c221d) SHA1(99f6abd91870221a7d56a6dc062a687d0458546d) )
ROM_END

ROM_START( ec_casbxoa )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "bx5c10p.a4", 0x0000, 0x1000, CRC(0c7df970) SHA1(25fb113a28fd446467bf9a7edf97dc8aaf936eb6) )
	ROM_LOAD( "bx5c10p.a3", 0x1000, 0x1000, CRC(12640d16) SHA1(fd30abe0551734eea83cefcb5cac15a380a97586) )
	ROM_LOAD( "bx5c10p.a2", 0x2000, 0x1000, CRC(0bd21303) SHA1(eb60749d3097ce77f0955586fc8ed1d16993286a) )
ROM_END

} // anonymous namespace


// all roms might really be for the same game, just in different cabinet styles
// these ALL contain "Ver 3 BAR-X V1:84 TYPE T" strings
GAME( 19??, ec_barxo,   0,        ecoinf1_older, ecoinf1, ecoinf1_state, empty_init, ROT0, "Electrocoin", "Bar X (older PCB) (Electrocoin) (set 1)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME( 19??, ec_barxoa,  ec_barxo, ecoinf1_older, ecoinf1, ecoinf1_state, empty_init, ROT0, "Electrocoin", "Bar X (older PCB) (Electrocoin) (set 2)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME( 19??, ec_barxob,  ec_barxo, ecoinf1_older, ecoinf1, ecoinf1_state, empty_init, ROT0, "Electrocoin", "Bar X (older PCB) (Electrocoin) (set 3)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME( 19??, ec_barxoc,  ec_barxo, ecoinf1_older, ecoinf1, ecoinf1_state, empty_init, ROT0, "Electrocoin", "Bar X (older PCB) (Electrocoin) (set 4)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME( 19??, ec_barxod,  ec_barxo, ecoinf1_older, ecoinf1, ecoinf1_state, empty_init, ROT0, "Electrocoin", "Bar X (older PCB) (Electrocoin) (set 5)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME( 19??, ec_barxoe,  ec_barxo, ecoinf1_older, ecoinf1, ecoinf1_state, empty_init, ROT0, "Electrocoin", "Bar X (older PCB) (Electrocoin) (set 6)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME( 19??, ec_bar5,    ec_barxo, ecoinf1_older, ecoinf1, ecoinf1_state, empty_init, ROT0, "Electrocoin", "Bar 5 (older PCB) (Electrocoin)",                MACHINE_IS_SKELETON_MECHANICAL) // or just another Bar X set?
GAME( 19??, ec_casbxo,  ec_barxo, ecoinf1_older, ecoinf1, ecoinf1_state, empty_init, ROT0, "Electrocoin", "Casino Bar X (older PCB) (Electrocoin) (set 1)", MACHINE_IS_SKELETON_MECHANICAL) // this one actually has some code offset changes
GAME( 19??, ec_casbxoa, ec_barxo, ecoinf1_older, ecoinf1, ecoinf1_state, empty_init, ROT0, "Electrocoin", "Casino Bar X (older PCB) (Electrocoin) (set 2)", MACHINE_IS_SKELETON_MECHANICAL)
