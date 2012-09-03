/***********************************************************************************

    Pinball
    Atari Generation/System 3

************************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"

class atari_s3_state : public driver_device
{
public:
	atari_s3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(atari_s3);
};


static ADDRESS_MAP_START( atari_s3_map, AS_PROGRAM, 8, atari_s3_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0700) AM_RAM
	AM_RANGE(0x0800, 0x08ff) AM_MIRROR(0x0700) AM_RAM // NVRAM
	//AM_RANGE(0x1000, 0x1007) AM_READ(sw_r)
	//AM_RANGE(0x1800, 0x1800) AM_WRITE(sound0_w)
	//AM_RANGE(0x1820, 0x1820) AM_WRITE(sound1_w)
	//AM_RANGE(0x1840, 0x1846) AM_WRITE(disp0_w)
	//AM_RANGE(0x1847, 0x1847) AM_WRITE(disp1_w)
	//AM_RANGE(0x1860, 0x1867) AM_WRITE(lamp_w)
	//AM_RANGE(0x1880, 0x1880) AM_WRITE(sol0_w)
	//AM_RANGE(0x18a0, 0x18a7) AM_WRITE(sol1_w)
	//AM_RANGE(0x18c0, 0x18c1) AM_WRITE(watchdog_w)
	//AM_RANGE(0x18e0, 0x18e0) AM_WRITE(intack_w)
	//AM_RANGE(0x2000, 0x2003) AM_READWRITE(dip_r,dip_w)
	//AM_RANGE(0x2008, 0x200b) AM_READ(dip2_r)
	AM_RANGE(0x2800, 0x3fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( atari_s3 )
INPUT_PORTS_END

void atari_s3_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(atari_s3_state,atari_s3)
{
}

static MACHINE_CONFIG_START( atari_s3, atari_s3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(atari_s3_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Road Runner (??/1979)
/-------------------------------------------------------------------*/
ROM_START(roadrunr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("0000.716", 0x2800, 0x0800, CRC(62f5f394) SHA1(ff91066d43d788119e3337788abd86e5c0bf2d92))
	ROM_LOAD("3000.716", 0x3000, 0x0800, CRC(2fc01359) SHA1(d3df20c764bb68a5316367bb18d34a03293e7fa6))
	ROM_LOAD("3800.716", 0x3800, 0x0800, CRC(77262408) SHA1(3045a732c39c96002f495f64ed752279f7d43ee7))
	ROM_REGION(0x1000, "sound1", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END


GAME( 1979,  roadrunr,  0,  atari_s3,  atari_s3, atari_s3_state,  atari_s3,  ROT0,  "Atari",    "Road Runner",     GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
