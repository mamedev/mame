// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

has a sticker marked
Part # 04-0008
Main PCB  w/ Dark-Slide
Serial# : 0115

2x Z80



There were several different designs for this, it's possible they used
different speech roms etc.


*/

#include "emu.h"
#include "cpu/z80/z80.h"

class instantm_state : public driver_device
{
public:
	instantm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( instantm_map, AS_PROGRAM, 8, instantm_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( instantm_submap, AS_PROGRAM, 8, instantm_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( instantm )
INPUT_PORTS_END


void instantm_state::machine_start()
{
}

void instantm_state::machine_reset()
{
}

// OSC1 = XTAL_3_579545MHz

static MACHINE_CONFIG_START( instantm, instantm_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(instantm_map)

	MCFG_CPU_ADD("subcpu", Z80,XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(instantm_submap)
MACHINE_CONFIG_END



ROM_START( instantm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "System5.3Beta.u16", 0x00000, 0x02000, CRC(a1701f4b) SHA1(fa5b0234bd2b666e478aa41129479bb6cec2bcf5) )

	ROM_REGION( 0x10000, "subcpu", 0 )
	ROM_LOAD( "SpeechUS10.u20", 0x00000, 0x10000, CRC(1797bcee) SHA1(c6fb7fbe8592dfae3ba44b49b5ce447206515b77) )
ROM_END


GAME( 199?, instantm,  0,    instantm, instantm, driver_device,  0, ROT0, "Capcom / Polaroid", "Polaroid Instant Memories", MACHINE_IS_SKELETON )
