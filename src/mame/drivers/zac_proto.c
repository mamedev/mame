/*
    Zaccaria Prototype
*/

#include "emu.h"
#include "cpu/scmp/scmp.h"
#include "zac_proto.lh"

class zac_proto_state : public driver_device
{
public:
	zac_proto_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

	DECLARE_WRITE8_MEMBER(out0_w);
	DECLARE_WRITE8_MEMBER(out1_w);
	DECLARE_WRITE8_MEMBER(digit_w);
	DECLARE_WRITE8_MEMBER(sound_w);
protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(zac_proto);
};


static ADDRESS_MAP_START( zac_proto_map, AS_PROGRAM, 8, zac_proto_state )
	AM_RANGE(0x0000, 0x0bff) AM_ROM
	AM_RANGE(0x0c00, 0x0dff) AM_RAM
	AM_RANGE(0x0e00, 0x0e00) AM_READ_PORT("PL0")
	AM_RANGE(0x0e01, 0x0e01) AM_READ_PORT("PL1")
	AM_RANGE(0x0e02, 0x0e02) AM_READ_PORT("PL2")
	AM_RANGE(0x0e03, 0x0e03) AM_READ_PORT("PL3")
	AM_RANGE(0x0e04, 0x0e04) AM_READ_PORT("PL4")
	AM_RANGE(0x0e05, 0x0e05) AM_READ_PORT("PL5")
	AM_RANGE(0x0e06, 0x0e06) AM_READ_PORT("PL6")
	AM_RANGE(0x0e07, 0x0e07) AM_READ_PORT("PL7")
	AM_RANGE(0x0e00, 0x0e01) AM_WRITE(out0_w)
	AM_RANGE(0x0e02, 0x0e06) AM_WRITE(digit_w)
	AM_RANGE(0x0e07, 0x0e08) AM_WRITE(sound_w)
	AM_RANGE(0x0e09, 0x0e16) AM_WRITE(out1_w)
	AM_RANGE(0x1400, 0x1bff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( zac_proto )
	// playfield inputs
	PORT_START("PL0")
	PORT_START("PL1")
	PORT_START("PL2")
	PORT_START("PL3")
	PORT_START("PL4")
	// dipswitches
	PORT_START("PL5")
	PORT_START("PL6")
	PORT_START("PL7")
INPUT_PORTS_END

WRITE8_MEMBER( zac_proto_state::out0_w )
{
}

WRITE8_MEMBER( zac_proto_state::out1_w )
{
}

// need to implement blanking of leading zeroes
WRITE8_MEMBER( zac_proto_state::digit_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0, 0, 0, 0, 0, 0 };
	static const UINT8 decimals[10] = { 0, 0, 0x80, 0, 0, 0x80, 0, 0, 0, 0 };
	offset<<=1;
	output_set_digit_value(offset, patterns[data&15] | decimals[offset]);
	offset++;
	output_set_digit_value(offset, patterns[data>>4] | decimals[offset]);
}

WRITE8_MEMBER( zac_proto_state::sound_w )
{
}

void zac_proto_state::machine_reset()
{
	output_set_digit_value(10, 0x3f); // units shows zero all the time
}

DRIVER_INIT_MEMBER(zac_proto_state,zac_proto)
{
}

static MACHINE_CONFIG_START( zac_proto, zac_proto_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SCMP, 1000000)
	MCFG_CPU_PROGRAM_MAP(zac_proto_map)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_zac_proto)
MACHINE_CONFIG_END

/*--------------------------------
/ Strike
/-------------------------------*/
ROM_START(strike)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("strike1.bin", 0x0000, 0x0400, CRC(650abc54) SHA1(6a4f83016a38338ba6a04271532f0880264e61a7))
	ROM_LOAD("strike2.bin", 0x0400, 0x0400, CRC(13c5a168) SHA1(2da3a5bc0c28a2aacd8c1396dac95cf35f8797cd))
	ROM_LOAD("strike3.bin", 0x0800, 0x0400, CRC(ebbbf315) SHA1(c87e961c8e5e99b0672cd632c5e104ea52088b5d))
	ROM_LOAD("strike4.bin", 0x1400, 0x0400, CRC(ca0eddd0) SHA1(52f9faf791c56b68b1806e685d0479ea67aba019))
ROM_END

/*--------------------------------
/ Ski Jump (10/78)
/-------------------------------*/
ROM_START(skijump)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("skijump1.bin", 0x0000, 0x0400, CRC(c0c0e18c) SHA1(d28ec2541f6c2e86e5b5514c7f9e558df68be72a))
	ROM_LOAD("skijump2.bin", 0x0400, 0x0400, CRC(b08aafb5) SHA1(ff6df4efa20a4461d525209a487d04896eeef29e))
	ROM_LOAD("skijump3.bin", 0x0800, 0x0400, CRC(9a8731c0) SHA1(9f7aaa8c6df04b925c8beff8b426c59bc3696f50))
	ROM_LOAD("skijump4.bin", 0x1400, 0x0400, CRC(fa064b51) SHA1(d4d02ca661e4084805f00247f31c0701320ab62d))
ROM_END

/*--------------------------------
/ Space City (09/79)
/-------------------------------*/
ROM_START(spacecty)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("zsc1.dat", 0x0000, 0x0400, CRC(4405368f) SHA1(037ad7e7158424bb714b28e4effa2c96c8736ce4))
	ROM_LOAD("zsc2.dat", 0x0400, 0x0400, CRC(a6c41475) SHA1(7d7d851efb2db7d9a1988265cdff676260d753c3))
	ROM_LOAD("zsc3.dat", 0x0800, 0x0400, CRC(e6a2dcee) SHA1(d2dfff896ae90208c28179f9bbe43f93d7f2131c))
	ROM_LOAD("zsc4.dat", 0x1400, 0x0400, CRC(69e0bb95) SHA1(d9a1d0159bf49445b0ece0f9d7806ed80657c2b2))
ROM_END

GAME(1978,  skijump,   0,  zac_proto,  zac_proto, zac_proto_state,  zac_proto,  ROT0,  "Zaccaria",    "Ski Jump",        GAME_IS_SKELETON_MECHANICAL)
GAME(1979,  spacecty,  0,  zac_proto,  zac_proto, zac_proto_state,  zac_proto,  ROT0,  "Zaccaria",    "Space City",      GAME_IS_SKELETON_MECHANICAL)
GAME(1978,  strike,    0,  zac_proto,  zac_proto, zac_proto_state,  zac_proto,  ROT0,  "Zaccaria",    "Strike",          GAME_IS_SKELETON_MECHANICAL)
