/**********************************************************************************

  PINBALL
  Hankin
  Based on Bally BY35

***********************************************************************************/

#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "hankin.lh"

class hankin_state : public genpin_class
{
public:
	hankin_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_ic10(*this, "ic10")
		, m_ic11(*this, "ic11")
		, m_ic2(*this, "ic2")
	{ }

	DECLARE_DRIVER_INIT(hankin);
private:
	virtual void machine_reset();
	required_device<m6802_cpu_device> m_maincpu;
	required_device<m6802_cpu_device> m_audiocpu;
	required_device<pia6821_device> m_ic10;
	required_device<pia6821_device> m_ic11;
	required_device<pia6821_device> m_ic2;
};


static ADDRESS_MAP_START( hankin_map, AS_PROGRAM, 8, hankin_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM // internal to the cpu
	AM_RANGE(0x0088, 0x008b) AM_DEVREADWRITE("ic11", pia6821_device, read, write)
	AM_RANGE(0x0090, 0x0093) AM_DEVREADWRITE("ic10", pia6821_device, read, write)
	AM_RANGE(0x0200, 0x02ff) AM_RAM AM_SHARE("nvram") // 5101L 4-bit static ram
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hankin_sub_map, AS_PROGRAM, 8, hankin_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM // internal to the cpu
	AM_RANGE(0x0080, 0x0083) AM_DEVREADWRITE("ic2", pia6821_device, read, write)
	AM_RANGE(0x1000, 0x17ff) AM_ROM AM_MIRROR(0x800) AM_REGION("roms", 0x1000)
ADDRESS_MAP_END

static INPUT_PORTS_START( hankin )
INPUT_PORTS_END

void hankin_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(hankin_state,hankin)
{
}

static MACHINE_CONFIG_START( hankin, hankin_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6802, 3276800)
	MCFG_CPU_PROGRAM_MAP(hankin_map)

	MCFG_CPU_ADD("audiocpu", M6802, 3276800) // guess, xtal value not shown
	MCFG_CPU_PROGRAM_MAP(hankin_sub_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_hankin)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_DEVICE_ADD("ic10", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(hankin_state, ic10_a_r))
	//MCFG_PIA_WRITEPA_HANDLER(WRITE8(hankin_state, ic10_a_w))
	//MCFG_PIA_READPB_HANDLER(READ8(hankin_state, ic10_b_r))
	//MCFG_PIA_WRITEPB_HANDLER(WRITE8(hankin_state, ic10_b_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(hankin_state, ic10_ca2_w))
	//MCFG_PIA_CB2_HANDLER(WRITELINE(hankin_state, ic10_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6802_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6802_cpu_device, irq_line))

	MCFG_DEVICE_ADD("ic11", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(hankin_state, ic11_a_r))
	//MCFG_PIA_WRITEPA_HANDLER(WRITE8(hankin_state, ic11_a_w))
	//MCFG_PIA_READPB_HANDLER(READ8(hankin_state, ic11_b_r))
	//MCFG_PIA_WRITEPB_HANDLER(WRITE8(hankin_state, ic11_b_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(hankin_state, ic11_ca2_w))
	//MCFG_PIA_CB2_HANDLER(WRITELINE(hankin_state, ic11_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6802_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6802_cpu_device, irq_line))

	MCFG_DEVICE_ADD("ic2", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(hankin_state, ic2_a_r))
	//MCFG_PIA_WRITEPA_HANDLER(WRITE8(hankin_state, ic2_a_w))
	//MCFG_PIA_READPB_HANDLER(READ8(hankin_state, ic2_b_r))
	//MCFG_PIA_WRITEPB_HANDLER(WRITE8(hankin_state, ic2_b_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(hankin_state, ic2_ca2_w))
	//MCFG_PIA_CB2_HANDLER(WRITELINE(hankin_state, ic2_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("audiocpu", m6802_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("audiocpu", m6802_cpu_device, irq_line))
MACHINE_CONFIG_END

/*--------------------------------
/ FJ Holden
/-------------------------------*/
ROM_START(fjholden)
	ROM_REGION(0x1a00, "roms", 0)
	ROM_LOAD("fj_ic2.mpu",   0x0000, 0x0800, CRC(b47bc2c7) SHA1(42c985d83a9454fcd08b87e572e5563ebea0d052))
	ROM_LOAD("fj_ic3.mpu",   0x0800, 0x0800, CRC(ceaeb7d3) SHA1(9e479b985f8500983e71d6ff33ee94160e99650d))
	ROM_LOAD("fj_ic14.snd",  0x1000, 0x0800, CRC(34fe3587) SHA1(132714675a23c101ceb5a4d544818650ae5ccea2))
	ROM_LOAD("fj_ic3.snd",   0x1800, 0x0200, CRC(09d3f020) SHA1(274be0b94d341ee43357011691da82e83a7c4a00))
ROM_END

/*--------------------------------
/ Howzat!
/-------------------------------*/
ROM_START(howzat)
	ROM_REGION(0x1a00, "roms", 0)
	ROM_LOAD("hz_ic2.mpu",   0x0000, 0x0800, CRC(b47bc2c7) SHA1(42c985d83a9454fcd08b87e572e5563ebea0d052))
	ROM_LOAD("hz_ic3.mpu",   0x0800, 0x0800, CRC(d13df4bc) SHA1(27a70260698d3eaa7cf7a56edc5dd9a4af3f4103))
	ROM_LOAD("hz_ic14.snd",  0x1000, 0x0800, CRC(0e3fdb59) SHA1(cae3c85b2c32a0889785f770ece66b959bcf21e1))
	ROM_LOAD("hz_ic3.snd",   0x1800, 0x0200, CRC(dfc57606) SHA1(638853c8e46bf461f2ecde02b8b2aa68c2d414b8))
ROM_END

/*--------------------------------
/ Orbit 1
/-------------------------------*/
ROM_START(orbit1)
	ROM_REGION(0x1a00, "roms", 0)
	ROM_LOAD("o1_ic2.mpu",   0x0000, 0x0800, CRC(b47bc2c7) SHA1(42c985d83a9454fcd08b87e572e5563ebea0d052))
	ROM_LOAD("o1_ic3.mpu",   0x0800, 0x0800, CRC(fe7b61be) SHA1(c086b0433bb9ab3f2139c705d4372beb1656b27f))
	ROM_LOAD("o1_ic14.snd",  0x1000, 0x0800, CRC(323bfbd5) SHA1(2e89aa4fcd33f9bfeea5c310ffb0a5be45fb70a9))
	ROM_LOAD("o1_ic3.snd",   0x1800, 0x0200, CRC(dfc57606) SHA1(638853c8e46bf461f2ecde02b8b2aa68c2d414b8))
ROM_END

/*--------------------------------
/ Shark
/-------------------------------*/
ROM_START(shark)
	ROM_REGION(0x1a00, "roms", 0)
	ROM_LOAD("shk_ic2.mpu",  0x0000, 0x0800, CRC(b47bc2c7) SHA1(42c985d83a9454fcd08b87e572e5563ebea0d052))
	ROM_LOAD("shk_ic3.mpu",  0x0800, 0x0800, CRC(c3ef936c) SHA1(14668496d162a77e03c1142bef2956d5b76afc99))
	ROM_LOAD("shk_ic14.snd", 0x1000, 0x0800, CRC(8f8b0e48) SHA1(72d94aa9b32c603b1ca681b0ab3bf8ddbf5c9afe))
	ROM_LOAD("shk_ic3.snd",  0x1800, 0x0200, CRC(dfc57606) SHA1(638853c8e46bf461f2ecde02b8b2aa68c2d414b8))
ROM_END

/*--------------------------------
/ The Empire Strike Back
/-------------------------------*/
ROM_START(empsback)
	ROM_REGION(0x1a00, "roms", 0)
	ROM_LOAD("sw_ic2.mpu",   0x0000, 0x0800, CRC(b47bc2c7) SHA1(42c985d83a9454fcd08b87e572e5563ebea0d052))
	ROM_LOAD("sw_ic3.mpu",   0x0800, 0x0800, CRC(837ffe32) SHA1(9affc5d9345ce15394553d3204e5234cc6348d2e))
	ROM_LOAD("sw_ic14.snd",  0x1000, 0x0800, CRC(c1eeb53b) SHA1(7a800dd0a8ae392e14639e1819198d4215cc2251))
	ROM_LOAD("sw_ic3.snd",   0x1800, 0x0200, CRC(db214f65) SHA1(1a499cf2059a5c0d860d5a4251a89a5735937ef8))
ROM_END


GAME(1978,  fjholden,  0,  hankin,  hankin, hankin_state,  hankin,  ROT0,  "Hankin", "FJ Holden", GAME_IS_SKELETON_MECHANICAL)
GAME(1980,  howzat,    0,  hankin,  hankin, hankin_state,  hankin,  ROT0,  "Hankin", "Howzat!", GAME_IS_SKELETON_MECHANICAL)
GAME(1978,  orbit1,    0,  hankin,  hankin, hankin_state,  hankin,  ROT0,  "Hankin", "Orbit 1", GAME_IS_SKELETON_MECHANICAL)
GAME(1980,  shark,     0,  hankin,  hankin, hankin_state,  hankin,  ROT0,  "Hankin", "Shark", GAME_IS_SKELETON_MECHANICAL)
GAME(1981,  empsback,  0,  hankin,  hankin, hankin_state,  hankin,  ROT0,  "Hankin", "The Empire Strike Back", GAME_IS_SKELETON_MECHANICAL)
