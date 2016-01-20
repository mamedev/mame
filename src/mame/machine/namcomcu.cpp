// license:BSD-3-Clause
// copyright-holders:Alex W. Jackson
/*
Mitsubishi M37702 MCUs with Namco custom labels.

Label   Used by system(s)
-------------------------
C69     NA-1
C70     NA-2
C74     System 22
C75     NB-1, NB-2, System FL
C76     System 11
*/

#include "emu.h"
#include "namcomcu.h"


const device_type NAMCO_C69 = &device_creator<namco_c69_device>;
const device_type NAMCO_C70 = &device_creator<namco_c70_device>;
const device_type NAMCO_C74 = &device_creator<namco_c74_device>;
const device_type NAMCO_C75 = &device_creator<namco_c75_device>;
const device_type NAMCO_C76 = &device_creator<namco_c76_device>;


ROM_START( c69 )
	ROM_REGION16_LE( 0x4000, M37710_INTERNAL_ROM_REGION, 0 )
	ROM_LOAD( "c69.bin", 0, 0x4000, CRC(349134d9) SHA1(61a4981fc2716c228b6121fedcbf1ed6f34dc2de) )
ROM_END


ROM_START( c70 )
	ROM_REGION16_LE( 0x4000, M37710_INTERNAL_ROM_REGION, 0 )
	ROM_LOAD( "c70.bin", 0, 0x4000, CRC(b4015f23) SHA1(7ce91eda76e86b5cab625e2b67c463b7d143832e) )
ROM_END


ROM_START( c74 )
	ROM_REGION16_LE( 0x4000, M37710_INTERNAL_ROM_REGION, 0 )
	ROM_LOAD( "c74.bin", 0, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )
ROM_END


ROM_START( c75 )
	ROM_REGION16_LE( 0x4000, M37710_INTERNAL_ROM_REGION, 0 )
	ROM_LOAD( "c75.bin", 0, 0x4000, CRC(42f539a5) SHA1(3103e5a0a2867620309fd4fe478a2be0effbeff8) )
ROM_END


ROM_START( c76 )
	ROM_REGION16_LE( 0x4000, M37710_INTERNAL_ROM_REGION, 0 )
	ROM_LOAD( "c76.bin", 0, 0x4000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )
ROM_END


namco_c69_device::namco_c69_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: m37702m2_device(mconfig, NAMCO_C69, "C69 (M37702)", tag, owner, clock, "namcoc69", __FILE__)
{
}

namco_c70_device::namco_c70_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: m37702m2_device(mconfig, NAMCO_C70, "C70 (M37702)", tag, owner, clock, "namcoc70", __FILE__)
{
}

namco_c74_device::namco_c74_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: m37702m2_device(mconfig, NAMCO_C74, "C74 (M37702)", tag, owner, clock, "namcoc74", __FILE__)
{
}

namco_c75_device::namco_c75_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: m37702m2_device(mconfig, NAMCO_C75, "C75 (M37702)", tag, owner, clock, "namcoc75", __FILE__)
{
}

namco_c76_device::namco_c76_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: m37702m2_device(mconfig, NAMCO_C76, "C76 (M37702)", tag, owner, clock, "namcoc76", __FILE__)
{
}

const rom_entry *namco_c69_device::device_rom_region() const
{
	return ROM_NAME(c69);
}

const rom_entry *namco_c70_device::device_rom_region() const
{
	return ROM_NAME(c70);
}

const rom_entry *namco_c74_device::device_rom_region() const
{
	return ROM_NAME(c74);
}

const rom_entry *namco_c75_device::device_rom_region() const
{
	return ROM_NAME(c75);
}

const rom_entry *namco_c76_device::device_rom_region() const
{
	return ROM_NAME(c76);
}
