// license:BSD-3-Clause
// copyright-holders:Peter Bortas
/*
ABC CAD Mouse/Hi-res video card

The ABC CAD was an innovative electronics CAD accessory invented by
Marek Gorczyca, Betronex Elektronik AB 1983 to overcome the limitations
in the ABC80 microcomputer graphics which was very popular in Sweden at the time.

The mouse feature is in the form of a small box with a handle sticking out at the front.
The handle can be manipulated in one direction by turning it around its internal pivot
point left and right. The other dimension in handled by pulling and pushing the handle.
One button is available on the top of the handle.

The mouse is connected via the ABC bus, but also passes through the
ABC80<->Monitor power/AV cable

The mouse was sold with the PCB CAD program "CAD-ABC"

PCB Layout
----------


|-|   CN1    |------------------------------|
|       4164      CN2   CN3  CN4       PROM5|
|       4164    Z80                    PROM6|
|       4164                           PROM7|
|       4164  PROM3 PROM4                   |
|       4164                                |
|       4164                    EPROM0      |
|       4164                    EPROM1      |
|       4416                     4801       |
| PROM2        CR1                          |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    4801    - Mostek MK4801AN-2  1KiB SRAM 150ns
    4164    - TI TMS4164-15NL    8KiB DRAM
    4416    - TI TMS4416-15NL    16k x 4bit DRAM (= 8KiB)
    EPROM0  - Intel 2764-25      8KiB EPROM "D"
    EPROM1  - Intel 2764-25      8KiB EPROM "E"
    PROM2   - TI TBP18S030       256b  PROM "TBP18S030"
    PROM3   - Harris HM7602      256b  PROM "M3-7603-5 1"
    PROM4   - Signetics N82S129N 1024b PROM "N82S129N 1"
    PROM5   - Harris HM7602      256b  PROM "M3-7603-5 2"
    PROM6   - Signetics N82S129N 1024b PROM "N82S129N 2"
    PROM7   - Signetics N82S129N 1024b PROM "N82S129N 3"
    Z80     - Z80 CPU "Z 80/1C"
    CN1     - ABCBUS connector
    CN2     - ABC80 power/AV connector passthrough?
    CN3     - ABC80 power/AV connector passthrough?
    CN4     - ABC80 power/AV connector passthrough?
    CR1     - Crystal "8.000 OSI"
*/

#include "emu.h"
#include "cadmouse.h"

#define Z80_TAG "cardcpu"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC_CADMOUSE, abc_cadmouse_device, "cadabc", "CAD ABC Mouse/Hi-res Unit")

//-------------------------------------------------
//  ROM( abc_cadmouse )
//-------------------------------------------------

ROM_START( abc_cadmouse )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	// FIXME: The mapping of the EPROMs or if the map locally or on
	// the bus in unknown. 0x0 and 0x2k are just placeholders.
	ROM_LOAD( "eprom0.bin", 0x2000, 0x2000, CRC(c19d655d) SHA1(332ad862b77cff3ec55f0f78ac31b2b8cf93b7b3) )
	ROM_LOAD( "eprom1.bin", 0x0000, 0x2000, CRC(e71c9141) SHA1(07a6fae4e3fff3d7a4f67ad0791e4e297c1763aa) )

	ROM_REGION( 0x20, "cadmouse_prom2", 0 )
	ROM_LOAD( "prom2.bin", 0x0000, 0x0020, CRC(c6c3bc9b) SHA1(5944cce355657b7bdc693f47a72f6b01decdc02a) ) // 32x8
	ROM_REGION( 0x20, "cadmouse_prom3", 0 )
	ROM_LOAD( "prom3.bin", 0x0000, 0x0020, CRC(862fc73a) SHA1(8a5391cd2ab61e5c3e22bb8805ace48566f5f57d) ) // 32x8
	ROM_REGION( 0x100, "cadmouse_prom4", 0 )
	ROM_LOAD( "prom4.bin", 0x0000, 0x0100, CRC(df58aaa9) SHA1(a2ab3b19a85ba3da6d78d1b0d44e2c33b44de5bc) ) // 256x4
	ROM_REGION( 0x20, "cadmouse_prom5", 0 )
	ROM_LOAD( "prom5.bin", 0x0000, 0x0020, CRC(5efd8b94) SHA1(cbfd6ebee815b02667ae886bb0820efa29311d37) ) // 32x8
	ROM_REGION( 0x100, "cadmouse_prom6", 0 )
	ROM_LOAD( "prom6.bin", 0x0000, 0x0100, CRC(ee3d8b75) SHA1(1afb22e3cff6e36f49228f63d0c7830bc48cf3cf) ) // 256x4
	ROM_REGION( 0x100, "cadmouse_prom7", 0 )
	ROM_LOAD( "prom7.bin", 0x0000, 0x0100, CRC(395110bd) SHA1(54720d155b4990d9879b95c0d13592bb7534da09) ) // 256x4
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *abc_cadmouse_device::device_rom_region() const
{
	return ROM_NAME( abc_cadmouse );
}

//-------------------------------------------------
//  ADDRESS_MAP( abc_cadmouse_mem )
//-------------------------------------------------

void abc_cadmouse_device::abc_cadmouse_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).rom().region(Z80_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( abc_cadmouse_io )
//-------------------------------------------------

void abc_cadmouse_device::abc_cadmouse_io(address_map &map)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void abc_cadmouse_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(8'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &abc_cadmouse_device::abc_cadmouse_mem);
	m_maincpu->set_addrmap(AS_IO, &abc_cadmouse_device::abc_cadmouse_io);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_cadmouse_device - constructor
//-------------------------------------------------

abc_cadmouse_device::abc_cadmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ABC_CADMOUSE, tag, owner, clock),
		device_abcbus_card_interface(mconfig, *this),
		m_maincpu(*this, Z80_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_cadmouse_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc_cadmouse_device::device_reset()
{
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void abc_cadmouse_device::abcbus_cs(uint8_t data)
{
}
