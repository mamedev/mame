// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Multiscreen cartridge emulation

**********************************************************************/

/*

    PCB Layout
    ----------

                    |===================|
                    |           ROM6    |
                    |  MC14066          |
                    |                   |
    |===============|           ROM5    |
    |=|                                 |
    |=|                                 |
    |=|    RAM         BAT      ROM4    |
    |=|                                 |
    |=|                                 |
    |=|    ROM0                 ROM3    |
    |=|                                 |
    |=|                                 |
    |===============|  LS138    ROM2    |
                    |  LS138            |
                    |  LS174            |
                    |  LS133    ROM1    |
                    |===================|

    BAT   - BR2325 lithium battery
    RAM   - ? 8Kx8 RAM
    ROM0  - ? 16Kx8 EPROM
    ROM1  - ? 32Kx8 EPROM
    ROM2  - ? 32Kx8 EPROM
    ROM3  - ? 32Kx8 EPROM
    ROM4  - not populated
    ROM5  - not populated
    ROM6  - not populated

*/

/*

    TODO:

    - M6802 board
    - crashes on boot

        805A: lda  $01
        805C: and  #$FE
        805E: sta  $01
        8060: m6502_brk#$00 <-- BOOM!

*/

#include "emu.h"
#include "multiscreen.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MC6802P_TAG     "m6802"
#define MC6821P_0_TAG   "m6821_0"
#define MC6821P_1_TAG   "m6821_1"


#define BANK_RAM        0x0d



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_MULTISCREEN, c64_multiscreen_cartridge_device, "c64_mscr", "C64 Multiscreen cartridge")


//-------------------------------------------------
//  ROM( c64_multiscreen )
//-------------------------------------------------

ROM_START( c64_multiscreen )
	ROM_REGION( 0x2000, MC6802P_TAG, 0 )
	ROM_LOAD( "1",    0x0000, 0x1000, CRC(35be02a8) SHA1(5912bc3d8e0c0949c1e66c19116d6b71c7574e46) )
	ROM_LOAD( "2 cr", 0x1000, 0x1000, CRC(76a9ac6d) SHA1(87e7335e626bdb73498b46c28c7baab72df38d1f) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c64_multiscreen_cartridge_device::device_rom_region() const
{
	return ROM_NAME( c64_multiscreen );
}


void c64_multiscreen_cartridge_device::multiscreen_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom().region(MC6802P_TAG, 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_multiscreen_cartridge_device::device_add_mconfig(machine_config &config)
{
	m6802_cpu_device &cpu(M6802(config, MC6802P_TAG, XTAL(4'000'000)));
	cpu.set_addrmap(AS_PROGRAM, &c64_multiscreen_cartridge_device::multiscreen_mem);

	PIA6821(config, MC6821P_0_TAG, 0);
	PIA6821(config, MC6821P_1_TAG, 0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_multiscreen_cartridge_device - constructor
//-------------------------------------------------

c64_multiscreen_cartridge_device::c64_multiscreen_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_MULTISCREEN, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this), m_bank(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_multiscreen_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_multiscreen_cartridge_device::device_reset()
{
	m_bank = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_multiscreen_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		int bank = m_bank & 0x0f;

		if (bank == BANK_RAM)
		{
			data = m_nvram[offset & 0x1fff];
		}
		else
		{
			data = m_roml[(bank << 14) | (offset & 0x3fff)];
		}
	}
	else if (!romh)
	{
		int bank = m_bank & 0x0f;

		if (bank == BANK_RAM)
		{
			data = m_roml[offset & 0x3fff];
		}
		else
		{
			data = m_roml[(bank << 14) | (offset & 0x3fff)];
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_multiscreen_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (offset >= 0x8000 && offset < 0xa000)
	{
		int bank = m_bank & 0x0f;

		if (bank == BANK_RAM)
		{
			m_nvram[offset & 0x1fff] = data;
		}
	}
	else if (!io2)
	{
		m_bank = data;
	}
}
