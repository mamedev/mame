/**************************************************************************************

Pinball
Videodens

***************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"

class vd_state : public driver_device
{
public:
	vd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(vd);
};


static ADDRESS_MAP_START( vd_map, AS_PROGRAM, 8, vd_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( vd_io, AS_IO, 8, vd_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x00,0x05) AM_READ(sw_r)
	//AM_RANGE(0x20,0x27) AM_WRITE(lamp_w)
	//AM_RANGE(0x28,0x28) AM_WRITE(sol_w)
	//AM_RANGE(0x40,0x44) AM_WRITE(disp_w)
	//AM_RANGE(0x60,0x60) AM_WRITE(AY8910_control_port_0_w)
	//AM_RANGE(0x61,0x61) AM_READ(sw0_r)
	//AM_RANGE(0x62,0x62) AM_WRITE(AY8910_write_port_0_w)
	//AM_RANGE(0x80,0x80) AM_WRITE(AY8910_control_port_1_w)
	//AM_RANGE(0x82,0x82) AM_WRITE(AY8910_write_port_1_w)
	//AM_RANGE(0xa0,0xa0) AM_READ(AY8910_read_port_1_r)
	//AM_RANGE(0xc0,0xc0) AM_WRITE(col_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( vd )
INPUT_PORTS_END

void vd_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(vd_state,vd)
{
}

static MACHINE_CONFIG_START( vd, vd_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(vd_map)
	MCFG_CPU_IO_MAP(vd_io)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Break '86 (1986)
/-------------------------------------------------------------------*/
ROM_START(break86)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("break1.cpu", 0x0000, 0x2000, CRC(c187d263) SHA1(1790566799ccc41cd5445936e86f945150e24e8a))
	ROM_LOAD("break2.cpu", 0x2000, 0x2000, CRC(ed8f84ab) SHA1(ff5d7e3c373ca345205e8b92c6ce7b02f36a3d95))
	ROM_LOAD("break3.cpu", 0x4000, 0x2000, CRC(3cdfedc2) SHA1(309fd04c81b8facdf705e6297c0f4d507957ae1f))
ROM_END

/*-------------------------------------------------------------------
/ Papillon (1986)
/-------------------------------------------------------------------*/
ROM_START(papillon)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u4.dat", 0x0000, 0x2000, CRC(e57bfcdd) SHA1(d0d5c798552a2436693dfee0e2ebf4b6f465b194))
	ROM_LOAD("u5.dat", 0x2000, 0x2000, CRC(6d2ef02a) SHA1(0b67b2edd85624531630c162ae31af8078be01e3))
	ROM_LOAD("u6.dat", 0x4000, 0x2000, CRC(6b2867b3) SHA1(720fe8a65b447e839b0eb9ea21e0b3cb0e50cf7a))
ROM_END

#if 0
/*-------------------------------------------------------------------
/ Ator (19??)
/-------------------------------------------------------------------*/
ROM_START(ator)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ator.u4", 0x0000, 0x2000, NO_DUMP)
	ROM_LOAD("ator.u5", 0x2000, 0x2000, NO_DUMP)
	ROM_LOAD("ator.u6", 0x4000, 0x2000, CRC(21aad5c4) SHA1(e78da5d80682710db34cbbfeae5af54241c73371))
ROM_END
#endif

GAME(1986,  break86,  0,    vd,  vd, vd_state,  vd,  ROT0,  "Videodens",    "Break '86", GAME_IS_SKELETON_MECHANICAL)
GAME(1986,  papillon, 0,    vd,  vd, vd_state,  vd,  ROT0,  "Videodens",    "Papillon",  GAME_IS_SKELETON_MECHANICAL)
//GAME(19??,  ator,     0,    vd,  vd, vd_state,  vd,  ROT0,  "Videodens",    "Ator",      GAME_IS_SKELETON_MECHANICAL)
