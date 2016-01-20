/*
  Brother MT735 thermal printer
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"

class mt735_state : public driver_device
{
public:
	mt735_state(const machine_config &mconfig, device_type type, std::string tag);

	required_device<m68000_device> m_cpu;

	DECLARE_READ8_MEMBER(p4_r);
	DECLARE_READ8_MEMBER(p5_r);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};

mt735_state::mt735_state(const machine_config &mconfig, device_type type, std::string tag) :
	driver_device(mconfig, type, tag),
	m_cpu(*this, "maincpu")
{
}

void mt735_state::machine_start()
{
}

void mt735_state::machine_reset()
{
}

READ8_MEMBER(mt735_state::p4_r)
{
	logerror("p4_r (%06x)\n", space.device().safe_pc());
	return 0xe0;
}

READ8_MEMBER(mt735_state::p5_r)
{
	logerror("p5_r (%06x)\n", space.device().safe_pc());
	return 0x00;
}

static ADDRESS_MAP_START( mt735_map, AS_PROGRAM, 16, mt735_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x278000, 0x287fff) AM_RAM
	AM_RANGE(0x400000, 0x4fffff) AM_RAM
	AM_RANGE(0xff8004, 0xff8005) AM_READ8(p4_r, 0xff00)
	AM_RANGE(0xff8004, 0xff8005) AM_READ8(p5_r, 0x00ff)
ADDRESS_MAP_END

static INPUT_PORTS_START( mt735 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( mt735, mt735_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_48MHz/6)
	MCFG_CPU_PROGRAM_MAP(mt735_map)
MACHINE_CONFIG_END

ROM_START( mt735 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "spg_m_e_ic103.bin", 0, 0x20000, CRC(1ab58bc9) SHA1(c10d50f38819c037d28435b77e09f2b6923e8369) )
	ROM_LOAD16_BYTE( "spg_m_o_ic102.bin", 1, 0x20000, CRC(84d8446b) SHA1(b1cedd8b09556eb8118f79b012aeec5b61e3ff32) )
ROM_END

COMP( ????, mt735, 0, 0, mt735, mt735, driver_device, 0, "Brother", "MT735", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
