// license:BSD-3-Clause
// copyright-holders:Robbbert
/*******************************************************************************

  PINBALL
  JVH : Escape, and Movie Masters

********************************************************************************/
#include "emu.h"
#include "cpu/tms9900/tms9980a.h"
#include "cpu/m6800/m6800.h"

class jvh_state : public driver_device
{
public:
	jvh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override;
public:
	DECLARE_DRIVER_INIT(jvh);
};



static ADDRESS_MAP_START( jvh_map, AS_PROGRAM, 8, jvh_state )
	AM_RANGE(0x0000, 0x3bff) AM_ROM
	AM_RANGE(0x3c00, 0x3cff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( escape_io, AS_IO, 8, jvh_state )
	//AM_RANGE(0x01, 0x02) AM_READ(sw1_r)
	//AM_RANGE(0x03, 0x05) AM_READ(dip_r)
	//AM_RANGE(0x06, 0x07) AM_READ(sw6_r)
	//AM_RANGE(0x10, 0x15) AM_WRITE(snd_w)
	//AM_RANGE(0x16, 0x16) AM_WRITE(latch_w)
	//AM_RANGE(0x17, 0x19) AM_WRITE(out1a_w)
	//AM_RANGE(0x1a, 0x1a) AM_WRITE(enable_w)
	//AM_RANGE(0x1b, 0x1f) AM_WRITE(out1b_w)
	//AM_RANGE(0x20, 0x27) AM_WRITE(out2a_w)
	//AM_RANGE(0x28, 0x2f) AM_WRITE(out2b_w)
	//AM_RANGE(0x30, 0x37) AM_WRITE(out3a_w)
	//AM_RANGE(0x3e, 0x3e) AM_WRITE(irq_enable)
	//AM_RANGE(0x3f, 0x3f) AM_WRITE(zc_enable)
	//AM_RANGE(0x40, 0x47) AM_WRITE(digit_w)
	//AM_RANGE(0x48, 0x4b) AM_WRITE(bcd_w)
	//AM_RANGE(0x4c, 0x50) AM_WRITE(panel_w)
	//AM_RANGE(0x51, 0x55) AM_WRITE(col_w)
	//AM_RANGE(0x58, 0x5f) AM_WRITE(out5b_w)
	//AM_RANGE(0x60, 0x67) AM_WRITE(out6a_w)
	//AM_RANGE(0x68, 0x6f) AM_WRITE(out6b_w)
	//AM_RANGE(0x70, 0x74) AM_WRITE(out7a_w)
	//AM_RANGE(0x75, 0x7f) AM_WRITE(sol_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( movmastr_io, AS_IO, 8, jvh_state )
	//AM_RANGE(0x01, 0x02) AM_READ(sw1_r)
	//AM_RANGE(0x03, 0x05) AM_READ(dip_r)
	//AM_RANGE(0x08, 0x09) AM_READ(sw6_r)
	//AM_RANGE(0x00, 0x07) AM_WRITE(out0a2_w)
	//AM_RANGE(0x08, 0x0f) AM_WRITE(out0b2_w)
	//AM_RANGE(0x10, 0x17) AM_WRITE(out1a2_w)
	//AM_RANGE(0x18, 0x1f) AM_WRITE(out1b2_w)
	//AM_RANGE(0x20, 0x27) AM_WRITE(out2a2_w)
	//AM_RANGE(0x28, 0x2f) AM_WRITE(out2b2_w)
	//AM_RANGE(0x30, 0x30) AM_WRITE(out3a2_w)
	//AM_RANGE(0x31, 0x36) AM_WRITE(snd_w)
	//AM_RANGE(0x37, 0x37) AM_WRITE(latch_w)
	//AM_RANGE(0x3e, 0x3e) AM_WRITE(irq_enable)
	//AM_RANGE(0x3f, 0x3f) AM_WRITE(zc_enable)
	//AM_RANGE(0x40, 0x47) AM_WRITE(out4a2_w)
	//AM_RANGE(0x48, 0x4a) AM_WRITE(out4b2_w)
	//AM_RANGE(0x4b, 0x4b) AM_WRITE(enable_w)
	//AM_RANGE(0x4c, 0x4f) AM_WRITE(out4c2_w)
	//AM_RANGE(0x50, 0x55) AM_WRITE(col_w)
	//AM_RANGE(0x57, 0x5a) AM_WRITE(bcd_w)
	//AM_RANGE(0x5b, 0x5f) AM_WRITE(panel_w)
	//AM_RANGE(0x60, 0x67) AM_WRITE(digit_w)
	//AM_RANGE(0x68, 0x6f) AM_WRITE(out6b2_w)
	//AM_RANGE(0x70, 0x74) AM_WRITE(out7a2_w)
	//AM_RANGE(0x75, 0x7f) AM_WRITE(sol_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( jvh_sub_map, AS_PROGRAM, 8, jvh_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	//AM_RANGE(0x0080, 0x008f) via6522_r,w
	AM_RANGE(0xc000, 0xdfff) AM_MIRROR(0x2000) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( jvh )
INPUT_PORTS_END

void jvh_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(jvh_state,jvh)
{
}

static MACHINE_CONFIG_START( jvh, jvh_state )
	// CPU TMS9980A; no line connections
	MCFG_TMS99xx_ADD("maincpu", TMS9980A, 1000000, jvh_map, escape_io)
	MCFG_CPU_ADD("cpu2", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(jvh_sub_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( jvh2, jvh_state )
	// CPU TMS9980At; no line connections
	MCFG_TMS99xx_ADD("maincpu", TMS9980A, 1000000, jvh_map, movmastr_io)
	MCFG_CPU_ADD("cpu2", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(jvh_sub_map)
MACHINE_CONFIG_END



/*-------------------------------------------------------------------
/ Escape
/-------------------------------------------------------------------*/
ROM_START(escape)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("cpu_ic1.bin", 0x0000, 0x2000, CRC(fadb8f9a) SHA1(b7e7ea8e33847c14a3414f5e367e304f12c0bc00))
	ROM_LOAD("cpu_ic7.bin", 0x2000, 0x2000, CRC(2f9402b4) SHA1(3d3bae7e4e5ad40e3c8019d55392defdffd21cc4))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd.bin",     0xc000, 0x2000, CRC(2477bbe2) SHA1(f636952822153f43e9d09f8211edde1057249203))
ROM_END

/*-------------------------------------------------------------------
/ Movie Masters
/-------------------------------------------------------------------*/
ROM_START(movmastr)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("mm_ic1.764", 0x0000, 0x2000, CRC(fb59920d) SHA1(05536c4c036a8d73516766e14f4449665b2ec180))
	ROM_LOAD("mm_ic7.764", 0x2000, 0x2000, CRC(9b47af41) SHA1(ae795c22aa437d6c71312d93de8a87f43ee500fb))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("snd.bin", 0xc000, 0x2000, NO_DUMP)
ROM_END


GAME(1987,  escape,    0,  jvh,  jvh, jvh_state,  jvh,  ROT0,  "Jac Van Ham (Royal)",    "Escape",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  movmastr,  0,  jvh2, jvh, jvh_state,  jvh,  ROT0,  "Jac Van Ham (Royal)",    "Movie Masters",      MACHINE_IS_SKELETON_MECHANICAL)
