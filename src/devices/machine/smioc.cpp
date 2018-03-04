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
    * SCC - Signetics SCC2698BC1A84 - U67
        * Memory - NEC D43256AGU-10LL 8948A9038 SRAM 32KB - U51
        * Memory - Mitsubishi M5M187AJ 046101-35 SRAM 64K X 1?? - U37
    * Memory - AT&T M79018DX-15B 2K X 9 Dual Port SRAM - U53
    * Memory - AT&T M79018DX-15B 2K X 9 Dual Port SRAM - U54

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
    * 0xC0080 - 0xC008F : KS82C37A - Probably RAM DMA
    * 0xC0090 - 0xC009F : KS82C37A - Serial DMA (Port 1 and 2?)
    * 0xC00A0 - 0xC00AF : KS82C37A - Serial DMA (Port 3 and 4?)
    * 0xC00B0 - 0xC00BF : KS82C37A - Serial DMA (Port 5 and 6?)
    * 0xC00C0 - 0xC00CF : KS82C37A - Serial DMA (Port 7 and 8?)

    IO Memory:
        * Unknown

    TODO:
    * Emulate SCC and hook up RS232 ports
    * Hook up console to RS232 port 1
    * Hook up System Monitor II to RS232 port 2
    * Dump 87C451 rom data and emulate MCU
    * Dump 87C51 on SMIOC interconnect box
    * Dump all PAL chips
    * Hook up status LEDs
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

ADDRESS_MAP_START(smioc_device::smioc_mem)
	AM_RANGE(0x00000, 0x07FFF) AM_RAM AM_SHARE("smioc_ram")
	AM_RANGE(0xC0080, 0xC008F) AM_DEVREADWRITE("dma8237_1",am9517a_device,read,write) // Probably RAM DMA
	AM_RANGE(0xC0090, 0xC009F) AM_DEVREADWRITE("dma8237_2",am9517a_device,read,write) // Serial DMA
	AM_RANGE(0xC00A0, 0xC00AF) AM_DEVREADWRITE("dma8237_3",am9517a_device,read,write) // Serial DMA
	AM_RANGE(0xC00B0, 0xC00BF) AM_DEVREADWRITE("dma8237_4",am9517a_device,read,write) // Serial DMA
	AM_RANGE(0xC00C0, 0xC00CF) AM_DEVREADWRITE("dma8237_5",am9517a_device,read,write) // Serial DMA
	AM_RANGE(0xF8000, 0xFFFFF) AM_ROM AM_REGION("rom", 0)
ADDRESS_MAP_END

MACHINE_CONFIG_START(smioc_device::device_add_mconfig)
	/* CPU - Intel 80C188 */
	MCFG_CPU_ADD(I188_TAG, I80188, XTAL(20'000'000) / 2) // Clock division unknown
	MCFG_CPU_PROGRAM_MAP(smioc_mem)

	/* DMA */
	MCFG_DEVICE_ADD("dma8237_1", AM9517A, XTAL(20'000'000) / 4) // Clock division unknown
	MCFG_DEVICE_ADD("dma8237_2", AM9517A, XTAL(20'000'000) / 4)
	MCFG_DEVICE_ADD("dma8237_3", AM9517A, XTAL(20'000'000) / 4)
	MCFG_DEVICE_ADD("dma8237_4", AM9517A, XTAL(20'000'000) / 4)
	MCFG_DEVICE_ADD("dma8237_5", AM9517A, XTAL(20'000'000) / 4)

	/* RS232 */
	/* Port 1: Console */
	MCFG_RS232_PORT_ADD("rs232_p1", default_rs232_devices, nullptr)
	MCFG_RS232_PORT_ADD("rs232_p2", default_rs232_devices, nullptr)
	MCFG_RS232_PORT_ADD("rs232_p3", default_rs232_devices, nullptr)
	MCFG_RS232_PORT_ADD("rs232_p4", default_rs232_devices, nullptr)
	MCFG_RS232_PORT_ADD("rs232_p5", default_rs232_devices, nullptr)
	MCFG_RS232_PORT_ADD("rs232_p6", default_rs232_devices, nullptr)
	MCFG_RS232_PORT_ADD("rs232_p7", default_rs232_devices, nullptr)
	MCFG_RS232_PORT_ADD("rs232_p8", default_rs232_devices, nullptr)
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
	m_dma8237_1(*this, "dma8237_1"),
	m_dma8237_2(*this, "dma8237_2"),
	m_dma8237_3(*this, "dma8237_3"),
	m_dma8237_4(*this, "dma8237_4"),
	m_dma8237_5(*this, "dma8237_5"),
	m_rs232_p1(*this, "rs232_p1"),
	m_rs232_p2(*this, "rs232_p2"),
	m_rs232_p3(*this, "rs232_p3"),
	m_rs232_p4(*this, "rs232_p4"),
	m_rs232_p5(*this, "rs232_p5"),
	m_rs232_p6(*this, "rs232_p6"),
	m_rs232_p7(*this, "rs232_p7"),
	m_rs232_p8(*this, "rs232_p8"),
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
