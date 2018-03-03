// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for HP-700 series terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"

class hp700_state : public driver_device
{
public:
	hp700_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_duart(*this, "duart")
	{ }

	void hp700_92(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_device<scn2681_device> m_duart;
};

ADDRESS_MAP_START(hp700_state::mem_map)
	AM_RANGE(0x00000, 0x1ffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x20000, 0x2ffff) AM_RAM
	AM_RANGE(0x30000, 0x31fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x34000, 0x34000) AM_READNOP
	;map(0x38000, 0x38fff).lrw8("duart_rw", [this](address_space &space, offs_t offset, u8 mem_mask){ return m_duart->read(space, offset >> 8, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask){ m_duart->write(space, offset >> 8, data, mem_mask); });
	AM_RANGE(0xffff0, 0xfffff) AM_ROM AM_REGION("maincpu", 0x1fff0)
ADDRESS_MAP_END

ADDRESS_MAP_START(hp700_state::io_map)
	AM_RANGE(0x00f2, 0x00f2) AM_WRITENOP
ADDRESS_MAP_END

static INPUT_PORTS_START( hp700_92 )
INPUT_PORTS_END

MACHINE_CONFIG_START(hp700_state::hp700_92)
	MCFG_CPU_ADD("maincpu", V20, XTAL(29'491'200) / 3) // divider not verified
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEVICE_ADD("duart", SCN2681, XTAL(29'491'200) / 8) // divider not verified
	MCFG_MC68681_IRQ_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
MACHINE_CONFIG_END


/**************************************************************************************************************

Hewlett-Packard HP-700/92.
Chips: TC5564APL-15, proprietary square chip, D70108C-10 (V20), SCN2681, Beeper
Crystals: 29.4912

***************************************************************************************************************/

ROM_START( hp700_92 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD( "5181-8672.u803", 0x00000, 0x20000, CRC(21440d2f) SHA1(69a3de064ae2b18adc46c2fdd0bf69620375efe7) )
ROM_END

COMP( 1987, hp700_92, 0, 0, hp700_92, hp700_92, hp700_state, 0, "HP", "HP-700/92", MACHINE_IS_SKELETON )
