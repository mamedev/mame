// license:GPL-2.0+
// copyright-holders:Brandon Munger
/**********************************************************************

    ROLM 9751 9005 System Monitor Input/Output Card emulation

**********************************************************************/

/*
    Device SMIOC
    Board Copyright - IBM Corp 1989 Made in USA

    Labels:
        * 98R0083
          MN 90770AR

        * EC# A65048R

    Hardware:
        * CPU - Intel N80C188 L0450591 @ ??MHz - U23
        * MCU - Signetics SC87C451CCA68 220CP079109KA 97D8641 - U70
        * DMA - KS82C37A - U46, U47, U48, U49, U50
        * Memory - NEC D43256AGU-10LL 8948A9038 SRAM 32KB - U51
        * Memory - Mitsubishi M5M187AJ 046101-35 SRAM 64K X 1?? - U37
    Logic:
        * U8 - 22V10-25JC
	* U33 - 22V10-25JC
	* U61 - 22V10-25JC
	* U63 - 22V10-25JC
	* U87 - 22V10-20JC
	* U88 - 22V10-20JC
	* U102 - 22V10-25JC
	* U111 - 22V10-25JC

    Switches:
        * S1 - Board reset

    Program Memory:
        * 0x00000 - 0x07FFF : SRAM D43256AGU-10LL 32KB
        * 0xF8000 - 0xFFFFF : ROM 27C256 PLCC32 32KB

    IO Memory:
        * Unknown

*/

#include "emu.h"
#include "smioc.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I188_TAG     "smioc_i188" // U23

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SMIOC, smioc_device, "rolm_smioc", "ROLM SMIOC")

//-------------------------------------------------
//  ROM( SMIOC )
//-------------------------------------------------

ROM_START( smioc )
	ROM_REGION( 0x8000, "rom", 0 )
	ROM_LOAD( "smioc.27256.u65", 0x0000, 0x8000, CRC(25b93192) SHA1(8ee9879033623490ce6703ba5429af2b16dbae84) )
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *smioc_device::device_rom_region() const
{
	return ROM_NAME( smioc );
}

//-------------------------------------------------
//  ADDRESS_MAP( smioc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( smioc_mem, AS_PROGRAM, 8, smioc_device )
	AM_RANGE(0x00000, 0x07FFF) AM_RAM AM_SHARE("smioc_ram")
	AM_RANGE(0xF8000, 0xFFFFF) AM_ROM AM_REGION("rom", 0)
ADDRESS_MAP_END

MACHINE_CONFIG_MEMBER( smioc_device::device_add_mconfig )
	/* CPU - Intel 80C188 */
	MCFG_CPU_ADD(I188_TAG, I80188, XTAL_20MHz / 2) // Clock division unknown
	MCFG_CPU_PROGRAM_MAP(smioc_mem)
MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  smioc_device - constructor
//-------------------------------------------------

smioc_device::smioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMIOC, tag, owner, clock),
	m_smioccpu(*this, I188_TAG),
	m_smioc_ram(*this, "smioc_ram")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void smioc_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void smioc_device::device_reset()
{
	/* Reset CPU */
	m_smioccpu->reset();
}
