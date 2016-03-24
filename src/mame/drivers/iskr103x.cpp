// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    drivers/iskr103x.c

    Driver file for Iskra-1030, 1031

    TODO
    - fix cyrillic chargen upload for CGA and MDA
    - replace DIP switch definition
    - keyboard test is not passing (code 301)
    - hard disk is connected but untested

***************************************************************************/


#include "emu.h"
#include "machine/genpc.h"
#include "cpu/i86/i86.h"
#include "cpu/nec/nec.h"
#include "bus/isa/xsu_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "machine/pc_lpt.h"
#include "machine/ram.h"
#include "softlist.h"

#define DBG_LOG(a,b,c)

class iskr103x_state : public driver_device
{
public:
	iskr103x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( iskr1031_map, AS_PROGRAM, 16, iskr103x_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START(iskr1031_io, AS_IO, 16, iskr103x_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE8("mb", ibm5160_mb_device, map, 0xffff)
ADDRESS_MAP_END


static DEVICE_INPUT_DEFAULTS_START(iskr1030m)
	DEVICE_INPUT_DEFAULTS("DSW0", 0x30, 0x20)
	DEVICE_INPUT_DEFAULTS("DSW0", 0x01, 0x01)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(iskr1031)
	DEVICE_INPUT_DEFAULTS("DSW0", 0x30, 0x20)
DEVICE_INPUT_DEFAULTS_END

// XXX
static MACHINE_CONFIG_START( iskr1030m, iskr103x_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",  I8086, 4772720)
	MCFG_CPU_PROGRAM_MAP(iskr1031_map)
	MCFG_CPU_IO_MAP(iskr1031_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(iskr1030m)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", iskr103x_isa8_cards, "cga_iskr1030m", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", iskr103x_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", iskr103x_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", iskr103x_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", iskr103x_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", iskr103x_isa8_cards, nullptr, false)

	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_EC_1841)
//  MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_ISKR_1030)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
	MCFG_RAM_EXTRA_OPTIONS("64K, 128K, 256K, 512K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( iskr1031, iskr1030m )
	MCFG_DEVICE_MODIFY("mb")
	MCFG_DEVICE_INPUT_DEFAULTS(iskr1031)
	MCFG_DEVICE_MODIFY("isa1")
	MCFG_DEVICE_SLOT_INTERFACE(iskr103x_isa8_cards, "cga_iskr1031", false)
MACHINE_CONFIG_END

ROM_START( iskr1030m )
	ROM_REGION16_LE(0x10000,"bios", 0)
	ROMX_LOAD( "iskra-1030m_0.rom", 0xc000, 0x2000, CRC(0d698e19) SHA1(2fe117c9f4f8c4b59085d5a41f919d743c425fdd), ROM_SKIP(1))
	ROMX_LOAD( "iskra-1030m_1.rom", 0xc001, 0x2000, CRC(fe808337) SHA1(b0b7ebe14324ada8aa9a6926a82b18e80f78a257), ROM_SKIP(1))
ROM_END

ROM_START( iskr1031 )
	ROM_REGION16_LE(0x10000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "v1", "v1")
	ROMX_LOAD( "150-02.bin", 0xc000, 0x2000, CRC(e33fb974) SHA1(f5f3ece67c025c0033716ff516e1a34fbeb32749), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "150-03.bin", 0xc001, 0x2000, CRC(8c482258) SHA1(90ef48955e0df556dc06a000a797ef42ccf430c5), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v2", "v2")
	ROMX_LOAD( "150-06.bin", 0xc000, 0x2000, CRC(1adbf969) SHA1(08c0a0fc50a75e6207b1987bae389cca60893eac), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "150-07.bin", 0xc001, 0x2000, CRC(0dc4b65a) SHA1(c96f066251a7343eac8113ea9dcb2cb12d0334d5), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR     NAME        PARENT      COMPAT  MACHINE     INPUT                       INIT        COMPANY     FULLNAME */
COMP ( 1989,    iskr1030m,  ibm5150,    0,      iskr1030m,  0,         driver_device,   0,          "Schetmash", "Iskra 1030M", MACHINE_NOT_WORKING)
COMP ( 1989,    iskr1031,   ibm5150,    0,      iskr1031,   0,         driver_device,   0,          "<unknown>", "Iskra 1031", 0)
