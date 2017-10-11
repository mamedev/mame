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


DEFINE_DEVICE_TYPE(NAMCO_C69, namco_c69_device, "namcoc69", "Namco C69 (M37702)")
DEFINE_DEVICE_TYPE(NAMCO_C70, namco_c70_device, "namcoc70", "Namco C70 (M37702)")
DEFINE_DEVICE_TYPE(NAMCO_C74, namco_c74_device, "namcoc74", "Namco C74 (M37702)")
DEFINE_DEVICE_TYPE(NAMCO_C75, namco_c75_device, "namcoc75", "Namco C75 (M37702)")
DEFINE_DEVICE_TYPE(NAMCO_C76, namco_c76_device, "namcoc76", "Namco C76 (M37702)")


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


namco_c69_device::namco_c69_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m37702m2_device(mconfig, NAMCO_C69, tag, owner, clock)
{
}

namco_c70_device::namco_c70_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m37702m2_device(mconfig, NAMCO_C70, tag, owner, clock)
{
}

namco_c74_device::namco_c74_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m37702m2_device(mconfig, NAMCO_C74, tag, owner, clock)
{
}

namco_c75_device::namco_c75_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m37702m2_device(mconfig, NAMCO_C75, tag, owner, clock)
{
}

namco_c76_device::namco_c76_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m37702m2_device(mconfig, NAMCO_C76, tag, owner, clock)
{
}

const tiny_rom_entry *namco_c69_device::device_rom_region() const
{
	return ROM_NAME(c69);
}

const tiny_rom_entry *namco_c70_device::device_rom_region() const
{
	return ROM_NAME(c70);
}

const tiny_rom_entry *namco_c74_device::device_rom_region() const
{
	return ROM_NAME(c74);
}

const tiny_rom_entry *namco_c75_device::device_rom_region() const
{
	return ROM_NAME(c75);
}

const tiny_rom_entry *namco_c76_device::device_rom_region() const
{
	return ROM_NAME(c76);
}
