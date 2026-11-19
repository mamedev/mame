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

*/

#include "emu.h"
#include "multiscreen.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MC6802P_TAG     "m6802"
#define MC6821P_0_TAG   "m6821_0"
#define MC6821P_1_TAG   "m6821_1"
#define MC6821P_2_TAG   "m6821_2"
#define EPROM_TAG       "eprom"


#define BANK_RAM        0x0d
#define BANK_NONE       0x0f



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_MULTISCREEN, c64_multiscreen_cartridge_device, "c64_mscr", "C64 Multiscreen cartridge")


//-------------------------------------------------
//  ROM( c64_multiscreen )
//-------------------------------------------------

ROM_START( c64_multiscreen )
	ROM_REGION( 0x20000, EPROM_TAG, 0 )
	ROM_LOAD( "cart-1.bin", 0x00000, 0x04000, CRC(a0dc670c) SHA1(bbee117340477a7416bba2e9abc2b8debd940cd4) )
	ROM_RELOAD(             0x04000, 0x04000 ) // 16Kx8 part in a 32Kx8 socket, A14 not connected
	ROM_LOAD( "cart-2.bin", 0x08000, 0x08000, CRC(2abaad8e) SHA1(897f98e954418e2e9313f219cd64b100c60c68b1) )
	ROM_LOAD( "cart-3.bin", 0x10000, 0x08000, CRC(ecddbbfc) SHA1(70c72b77dc3981be8bcbec9ba0cc38d9aa5936fd) )
	ROM_LOAD( "cart-4.bin", 0x18000, 0x08000, CRC(042678ef) SHA1(ff2582c617f72bf57be7d76bcf8270665144f924) )

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
	map(0x0084, 0x0087).rw(MC6821P_0_TAG, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0088, 0x008b).rw(MC6821P_1_TAG, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).rw(MC6821P_2_TAG, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0800, 0x0fff).ram();
	map(0x1000, 0x1fff).rom().region(MC6802P_TAG, 0x1000);
	map(0xf000, 0xffff).rom().region(MC6802P_TAG, 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_multiscreen_cartridge_device::device_add_mconfig(machine_config &config)
{
	m6802_cpu_device &cpu(M6802(config, MC6802P_TAG, XTAL(4'000'000)));
	cpu.set_addrmap(AS_PROGRAM, &c64_multiscreen_cartridge_device::multiscreen_mem);

	PIA6821(config, MC6821P_0_TAG);
	PIA6821(config, MC6821P_1_TAG);
	PIA6821(config, MC6821P_2_TAG);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_multiscreen_cartridge_device - constructor
//-------------------------------------------------

c64_multiscreen_cartridge_device::c64_multiscreen_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_MULTISCREEN, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_eprom(*this, EPROM_TAG),
	m_bank(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_multiscreen_cartridge_device::device_start()
{
	// allocate memory
	m_nvram = std::make_unique<uint8_t[]>(0x2000);
	save_pointer(NAME(m_nvram), 0x2000);

	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_multiscreen_cartridge_device::device_reset()
{
	m_exrom = 0;
	m_game = 1;
	m_bank = 0;
}


void c64_multiscreen_cartridge_device::nvram_default()
{
}


bool c64_multiscreen_cartridge_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_nvram.get(), 0x2000);
	return !err && (actual == 0x2000);
}


bool c64_multiscreen_cartridge_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_nvram.get(), 0x2000);
	return !err;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_multiscreen_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml || (!m_slot->loram() && (offset & 0xe000) == 0x8000))
	{
		int bank = m_bank & 0x0f;

		if (bank == BANK_RAM)
		{
			data = m_nvram[offset & 0x1fff];
		}
		else if (bank != BANK_NONE)
		{
			data = m_eprom->base()[(bank << 13) | (offset & 0x1fff)];
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
