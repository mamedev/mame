// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for ADDS Viewpoint 122 terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/mc68681.h"
#include "machine/pit8253.h"
//#include "video/scn2674.h"

class vp122_state : public driver_device
{
public:
	vp122_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		//, m_p_chargen(*this, "chargen")
	{ }

private:
	required_device<cpu_device> m_maincpu;
	//required_region_ptr<u8> m_p_chargen;
};

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, vp122_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0xa000, 0xa7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, vp122_state )
	AM_RANGE(0x01, 0x01) AM_READNOP
	AM_RANGE(0x10, 0x1f) AM_DEVREADWRITE("duart", scn2681_device, read, write)
	AM_RANGE(0x20, 0x20) AM_DEVREADWRITE("usart", i8251_device, data_r, data_w)
	AM_RANGE(0x21, 0x21) AM_DEVREADWRITE("usart", i8251_device, status_r, control_w)
	AM_RANGE(0x70, 0x73) AM_DEVREADWRITE("pit", pit8253_device, read, write)
ADDRESS_MAP_END

static INPUT_PORTS_START( vp122 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( vp122 )
	MCFG_CPU_ADD("maincpu", I8085A, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_DEVICE_ADD("duart", SCN2681, XTAL_3_6864MHz)
	MCFG_MC68681_IRQ_CALLBACK(INPUTLINE("maincpu", I8085_RST55_LINE))

	MCFG_DEVICE_ADD("usart", I8251, XTAL_8MHz / 4)

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
MACHINE_CONFIG_END

/**************************************************************************************************************

ADDS Viewpoint 122 (VPT-122).
Chips: D8085AC-2, SCN2674B, SCB2675T, D8251AFC, SCN2681A, D8253C-2, 5x MB8129-15, MX462020-20 (guess, it's unreadable)
Crystals: 22.096, 14.916, 3.6864, 8.000

***************************************************************************************************************/

ROM_START( vp122 )
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD( "223-48600.uj1", 0x0000, 0x4000, CRC(4d140c69) SHA1(04aa5a4f0c0e0d07b9dc983a6d626ee88ef8b8ba) )
	ROM_LOAD( "223-48500.ug1", 0x4000, 0x4000, CRC(4e98554d) SHA1(0cbb9cb7efd02a3209caed410ccc8495a5ec1772) )
	ROM_LOAD( "223-49400.uj2", 0x8000, 0x4000, CRC(447d90d3) SHA1(f8c0db824198b5a571eef80cc3eaf1e829aa2c2a) )

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "223-48700.uk4", 0x0000, 0x2000, CRC(4dbab4bd) SHA1(18e9a23ba22e2096fa529541fa329f5a56740e62) )
ROM_END

COMP( 1985, vp122, 0, 0, vp122, vp122, vp122_state, 0, "ADDS", "Viewpoint 122", MACHINE_IS_SKELETON )
