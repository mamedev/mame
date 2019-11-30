// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-80 Owoco Super Smartaid cartridge emulation

    New BASIC commands:

    AUTO, CHANGE, CLEAR, CONT, DEL, DIR, DISP, ED, EX, FIND,
    FOR-NEXT, HELP, JOB, KEY, LIB, LIST, NEW, OLD, PEEK, PEW, POW,
    REN, RESUME, SPOOL, STACK, START, SYS, TAB, TIME/NOTIME, TRACE,
    VAR, Û (LIST)

    0x4000-0x4fff = 4KB ROM bank
    0x5000-0x57ff = 2KB NVRAM
    0x7800-0x7bff = 1KB ROM bank

*********************************************************************/

/*

PCB Layout
----------

Super Smartaid / Super Smartaid Magnum 4

[---------------------------------------------------|
|                       PROM    LS155   LS08        |
|                                                   |
||-----------CN1----------|     LS74    HC32        |
|                                                   |
|       ROM1        RAM                             |
|                                                   |
|LS245          LS126   LS393   LS00        BAT     |
|       ROM2                                        |
|               LS260   LS74    LS32                |
|---------------------------------------------------|

Notes:
    All IC's shown.

    RAM     - Hitachi HM6116LP-3 / Fujitsu MB8416A-15 2Kx8 SRAM
    ROM1    - Mitsubishi M5L2764K 8Kx8 EPROM
    ROM2    - SGS M2716F1 2Kx8 EPROM / NEC D27128D 16Kx8 EPROM
    PROM    - Signetics N82S131N 512x4 bipolar PROM
    CN1     - ABC bus connector
    BAT     - Varta 3/V150H 3.6V 140mAh Ni-MH battery

*/

/*

    TODO:

    - entering a bad command locks up the system

*/

#include "emu.h"
#include "ssa.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC_SUPER_SMARTAID, abc_super_smartaid_device, "abc_ssa", "Super Smartaid")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void abc_super_smartaid_device::device_add_mconfig(machine_config &config)
{
	ABCBUS_SLOT(config, m_bus, DERIVED_CLOCK(1, 1), abc80_cards, nullptr);
}


//-------------------------------------------------
//  ROM( super_smartaid )
//-------------------------------------------------

ROM_START( super_smartaid )
	ROM_REGION( 0x800, "ssa1", 0 )
	ROM_LOAD( "ssa1.bin", 0x000, 0x800, CRC(4c015aba) SHA1(d83d50fb3da04d6cfed7a0ab595afbbef2812951) )

	ROM_REGION( 0x2000, "ssa2", 0 )
	ROM_LOAD( "ssa2.bin", 0x0000, 0x2000, CRC(8e8bbd2a) SHA1(ff745e346ead247ebcc622bdd811e5f7de629639) )

	ROM_REGION( 0x200, "ssa3", 0 )
	ROM_LOAD( "ssa3.bin", 0x000, 0x200, CRC(9671e00b) SHA1(9010cb3426bc61711ae940551b79b6ecba67defa) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *abc_super_smartaid_device::device_rom_region() const
{
	return ROM_NAME( super_smartaid );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_super_smartaid_device - constructor
//-------------------------------------------------

abc_super_smartaid_device::abc_super_smartaid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ABC_SUPER_SMARTAID, tag, owner, clock),
	device_abcbus_card_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_bus(*this, "bus"),
	m_rom_1(*this, "ssa1"),
	m_rom_2(*this, "ssa2"),
	m_prom(*this, "ssa3"),
	m_nvram(*this, "nvram"),
	m_rom_bank(0),
	m_prom_bank(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_super_smartaid_device::device_start()
{
	// descramble ROMs
	for (offs_t i = 0; i < 0x800; i++)
	{
		m_rom_1->base()[i] = bitswap<8>(m_rom_1->base()[i], 7, 4, 3, 0, 5, 2, 1, 6);
	}
	for (offs_t i = 0; i < 0x2000; i++)
	{
		m_rom_2->base()[i] = bitswap<8>(m_rom_2->base()[i], 2, 6, 1, 4, 3, 5, 7, 0);
	}

	// allocate memory
	m_nvram.allocate(0x800);

	// state saving
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_prom_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc_super_smartaid_device::device_reset()
{
	m_rom_bank = 0;
	m_prom_bank = 0;
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_xmemfl -
//-------------------------------------------------

uint8_t abc_super_smartaid_device::abcbus_xmemfl(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_prom->base()[(m_prom_bank << 6) | (offset >> 10)])
	{
	case 0x08:
		m_prom_bank = 1;
		// fallthru
	case 0x0c: case 0x0f:
		data = m_rom_2->base()[(m_rom_bank << 12) | (offset & 0xfff)];
		break;
	case 0x0d:
		data = m_rom_1->base()[(m_rom_bank << 10) | (offset & 0x3ff)];
		break;
	case 0x0e:
		data = m_nvram[offset & 0x7ff];
		break;
	default:
		data = m_bus->xmemfl_r(offset);
		break;
	}

	return data;
}


//-------------------------------------------------
//  abcbus_xmemw -
//-------------------------------------------------

void abc_super_smartaid_device::abcbus_xmemw(offs_t offset, uint8_t data)
{
	if ((offset == 0x4040) || (offset == 0x4041))
	{
		m_rom_bank = BIT(offset, 0);
	}

	switch (m_prom->base()[(m_prom_bank << 6) | (offset >> 10)])
	{
	case 0x0e:
		m_nvram[offset & 0x7ff] = data;
		break;
	default:
		m_bus->xmemw_w(offset, data);
		break;
	}
}
