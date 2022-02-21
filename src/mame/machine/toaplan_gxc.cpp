// license:BSD-3-Clause
// copyright-holders:Quench
/***************************************************************************

    Toaplan GXC-0x family DSP
    TI TMS320C10 DSP with pre-programmed on-chip ROM

    used with common interface (see toaplan_dsp_interface.cpp for more info),
    some bootlegs cloned these.

    There's 4 Known revisions:
        GXL-01 (Hishou Zame)
        GXL-02 (Flying Shark, Sky Shark, Wardner, Pyros, Wardner no Mori)
        GXL-03 (Kyukyoku Tiger)
        GXL-04 (Twin Cobra, Demon's World, Horror Story)

    TODO:
    - GXC-04 internal ROM is not dumped, use bootleg code temporarily

***************************************************************************/

#include "emu.h"
#include "machine/toaplan_gxc.h"


DEFINE_DEVICE_TYPE(TOAPLAN_GXC_01, toaplan_gxc_01_device, "toaplan_gxc_01", "Toaplan GXC-01 (TMS320C10)")
DEFINE_DEVICE_TYPE(TOAPLAN_GXC_02, toaplan_gxc_02_device, "toaplan_gxc_02", "Toaplan GXC-02 (TMS320C10)")
DEFINE_DEVICE_TYPE(TOAPLAN_GXC_03, toaplan_gxc_03_device, "toaplan_gxc_03", "Toaplan GXC-03 (TMS320C10)")
DEFINE_DEVICE_TYPE(TOAPLAN_GXC_04, toaplan_gxc_04_device, "toaplan_gxc_04", "Toaplan GXC-04 (TMS320C10)")


toaplan_gxc_01_device::toaplan_gxc_01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms32010_device(mconfig, TOAPLAN_GXC_01, tag, owner, clock, address_map_constructor())
{
	set_mp_mc(false); // use internal ROM
}


toaplan_gxc_02_device::toaplan_gxc_02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms32010_device(mconfig, TOAPLAN_GXC_02, tag, owner, clock, address_map_constructor())
{
	set_mp_mc(false); // use internal ROM
}


toaplan_gxc_03_device::toaplan_gxc_03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms32010_device(mconfig, TOAPLAN_GXC_03, tag, owner, clock, address_map_constructor())
{
	set_mp_mc(false); // use internal ROM
}


toaplan_gxc_04_device::toaplan_gxc_04_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms32010_device(mconfig, TOAPLAN_GXC_04, tag, owner, clock, address_map_constructor())
{
	set_mp_mc(false); // use internal ROM
}


ROM_START( gxc01 )
	ROM_REGION16_BE( 0xc00, "internal", 0 )
	ROM_LOAD( "d70011u_gxc-01_mcu_64000", 0x000, 0xc00, CRC(1ca63774) SHA1(e534325af9433fb0e9ccdf82ee3a192d2459b18f) ) // decapped from D70011U GXC-01 MCU 64000 (hishouza)
ROM_END


ROM_START( gxc02 )
	ROM_REGION16_BE( 0xc00, "internal", 0 )
	ROM_LOAD( "d70012u_gxc-02_mcu_71001",  0x000, 0xc00, CRC(eee0ff59) SHA1(dad4570815ec444e34cc73f7cd90f9ca8f7b3eb8) ) // decapped from D70012U GXC-02 MCU ^ 71001 (fshark)
	// 71400 (skyshark), 71900 (wardner) revision exists; they are interchangeable
ROM_END


ROM_START( gxc03 )
	ROM_REGION16_BE( 0xc00, "internal", 0 )
	ROM_LOAD( "d70015u_gxc-03_mcu_74002", 0x000, 0xc00, CRC(265b6f32) SHA1(1b548edeada4144baf732aba7e7013281c8e9608) ) // decapped from D70015U GXC-03 MCU ^ 74002 (ktiger)
ROM_END


ROM_START( gxc04 )
	ROM_REGION16_BE( 0x2000, "internal", 0 )
	// these are from bootleg; actual chip not decapped
	ROM_LOAD16_BYTE( "dsp_21.bin",  0x0000, 0x0800, BAD_DUMP CRC(2d135376) SHA1(67a2cc774d272ee1cd6e6bc1c5fc33fc6968837e) )
	ROM_LOAD16_BYTE( "dsp_22.bin",  0x0001, 0x0800, BAD_DUMP CRC(79389a71) SHA1(14ec4c1c9b06702319e89a7a250d0038393437f4) )
	// 74000 (twincobr), 74500, 91300 (demonwld) revision exists; they are interchangeable
ROM_END


const tiny_rom_entry *toaplan_gxc_01_device::device_rom_region() const
{
	return ROM_NAME(gxc01);
}


const tiny_rom_entry *toaplan_gxc_02_device::device_rom_region() const
{
	return ROM_NAME(gxc02);
}


const tiny_rom_entry *toaplan_gxc_03_device::device_rom_region() const
{
	return ROM_NAME(gxc03);
}


const tiny_rom_entry *toaplan_gxc_04_device::device_rom_region() const
{
	return ROM_NAME(gxc04);
}
