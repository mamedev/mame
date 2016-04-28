// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

	PCB X-081-PC-A

	contains a large box marked


|-----------------------\_/--------------------|
|                                   NASCO-9000 |
|                                              |
|                  /-  NASCO  -\               |
|       /\         |  ORIGINAL |               |
|  NASCO\/YUVO     \- 0001941 -/               |
|                                              |
|                      PAT.P                   |
|   |---------------------------------------|  |
|   |        MASTER'S GOLF vers JAPAN       |  |
|   |                                       |  |
|   |            CUSTOM BOARD               |  |
|   |---------------------------------------|  |
|                                              |
|                 YUVO CO., LTD                |
|-----------------------------------------------

 next to rom M-GF_A10.12K
 the box must contain at least a Z80


*/

#include "emu.h"
#include "cpu/z80/z80.h"

class mastrglf_state : public driver_device
{
public:
	mastrglf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};

// before clearing 0xa000 - 0xffff the first time
// [:subcpu] ':subcpu' (006D): unmapped io memory write to 0000 = 00 & FF
// [:maincpu] ':maincpu' (1B71): unmapped program memory write to 63E1 = 00 & FF
// [:maincpu] ':maincpu' (1B73): unmapped io memory write to 0005 = 00 & FF
// [:maincpu] ':maincpu' (1ACA): unmapped io memory write to 0007 = 00 & FF

// 2nd time
// [:maincpu] ':maincpu' (1AD2): unmapped io memory write to 0007 = 01 & FF
// [:maincpu] ':maincpu' (1B71): unmapped program memory write to 63E1 = 01 & FF
// [:maincpu] ':maincpu' (1B73): unmapped io memory write to 0005 = 01 & FF
// [:maincpu] ':maincpu' (1ADC): unmapped io memory write to 0006 = 00 & FF

static ADDRESS_MAP_START( mastrglf_map, AS_PROGRAM, 8, mastrglf_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM

	AM_RANGE(0x9000, 0x9fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mastrglf_io, AS_IO, 8, mastrglf_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x05, 0x05) AM_WRITENOP // ram bank for 0xa000 ?
ADDRESS_MAP_END



static ADDRESS_MAP_START( mastrglf_submap, AS_PROGRAM, 8, mastrglf_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM

	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mastrglf_subio, AS_IO, 8, mastrglf_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( mastrglf )
INPUT_PORTS_END


void mastrglf_state::machine_start()
{
}

void mastrglf_state::machine_reset()
{
}

// single XTAL_18_432MHz

static MACHINE_CONFIG_START( mastrglf, mastrglf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,XTAL_18_432MHz/4)
	MCFG_CPU_PROGRAM_MAP(mastrglf_map)
	MCFG_CPU_IO_MAP(mastrglf_io)


	MCFG_CPU_ADD("subcpu", Z80,XTAL_18_432MHz/4)
	MCFG_CPU_PROGRAM_MAP(mastrglf_submap)
	MCFG_CPU_IO_MAP(mastrglf_subio)
MACHINE_CONFIG_END



ROM_START( mastrglf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "M-GF_A1.4A.27128", 0x00000, 0x04000, CRC(55b89e8f) SHA1(2860fd3f8e4241dc25bb9a14e8967cdcaf769432) )

	ROM_REGION( 0x48000, "maindata", 0 )
	ROM_LOAD( "M-GF_A2.5A.27256",   0x00000, 0x08000, CRC(98aa20d8) SHA1(64007c4706f8e2e3b57c4a8467b37d44e8be9a01) )
	ROM_LOAD( "M-GF_A3.7A.27256",   0x08000, 0x08000, CRC(3f62b979) SHA1(90cc784230f6ed7fd3dd943e0808f0c3d722806a) )
	ROM_LOAD( "M-GF_A4.8A.27256",   0x10000, 0x08000, CRC(08a470d1) SHA1(4dabff8fc915406b1d4f7936d925378eec0df915) )
	ROM_LOAD( "M-GF_A5.10A.27256",  0x18000, 0x08000, CRC(4397c8a0) SHA1(deb9de1cf7ce6ddc69addf18ff5bf2f25ed11602) )
	ROM_LOAD( "M-GF_A6.12A.27256",  0x20000, 0x08000, CRC(b1fccecf) SHA1(8fb5e40f34596d9faa73255afc2c2635e9008954) )
	ROM_LOAD( "M-GF_A7.13A.27256",  0x28000, 0x08000, CRC(06075e41) SHA1(3426f4ede8449288519e25bc8a1d679bb5137279) )
	ROM_LOAD( "M-GF_A8.15A.27256",  0x30000, 0x08000, CRC(9ea9183b) SHA1(55f54575cd662b6194f69532baa25c9b2272760f) )
	ROM_LOAD( "M-GF_A9.16A.27256",  0x38000, 0x08000, CRC(61ab715f) SHA1(6b9cccaa83a9a9e44a46bae796e2f9eaa9f9c951) )
	ROM_LOAD( "M-GF_A10.12K.27256", 0x40000, 0x08000, CRC(d145b144) SHA1(52370d56106f0280c52266b5a727493a3396a8e3) )

	ROM_REGION( 0x10000, "subcpu", 0 ) // next to large module
	ROM_LOAD( "M-GF_A10.12K.27256", 0x00000, 0x08000, CRC(d145b144) SHA1(52370d56106f0280c52266b5a727493a3396a8e3) )
ROM_END



GAME( 198?, mastrglf,  0,    mastrglf, mastrglf, driver_device,  0, ROT0, "Nasco", "Master's Golf", MACHINE_IS_SKELETON )
