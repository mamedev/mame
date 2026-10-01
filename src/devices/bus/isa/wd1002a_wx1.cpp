// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WD1002A-WX1 Winchester Disk Controller emulation

**********************************************************************/

/*

PCB Layout
----------

ASSY 61-000003-19

|---------------------------------------------------|
|  J2   J3           REV                J1          |
|  |1   |1                               1|     34  |
|                                                   |
| MC3486D            74LS14D                        |
|            10MHz                                  |
|                                                   |
| MC3487    WD10C20B WD1010A  WD1015A  7406D 74LS244|
|                                                   |
|                                     74LS00D       |
|     74LS244 ROM        WD11C00-17     RAM         |
|                        17-02                      |
|     74LS260D 74LS13D                DIPSW         |
|                                                   |
|                                    J4             |
|---|                                        |------|
    |----------------------------------------|

Notes:
    All IC's shown.

    WD1010A     - Western Digital WD1010A-05 Winchester Disk Controller
    WD1015A     - Western Digital WD1015A-JM Buffer Manager Control Processor
    WD11C00-17  - Western Digital WD11C00-JT-17-02 PC/XT Host Interface Logic Device
    WD10C20B    - Western Digital WD10C20B-JH-05 Self-Adjusting Data Separator
    RAM         - NEC uPD446G-20L 2Kx8 static RAM, the shared sector buffer
    ROM         - 28-pin 8Kx8 mask ROM, the host BIOS extension
    MC3486D     - Motorola quad RS-422 line receiver, drive control/data lines
    MC3487      - Motorola quad RS-422 line driver, drive control/data lines
    DIPSW       - 8-position DIP switch, drive type/configuration select
    J1          - 2x17 pin PCB header, drive control cable (shared, both drives)
    J2          - 2x10 pin PCB header, drive 0 data cable
    J3          - 2x10 pin PCB header, drive 1 data cable
    J4          - 3-pin header, hard disk activity LED

*/

#include "emu.h"
#include "wd1002a_wx1.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_WD1002A_WX1, isa8_wd1002a_wx1_device, "wd1002a_wx1", "WD1002A-WX1")


//-------------------------------------------------
//  ROM( wd1002a_wx1 )
//-------------------------------------------------

ROM_START( wd1002a_wx1 )
	ROM_REGION( 0x2000, "wd1002a_wx1", 0 )
	ROM_LOAD( "600693-001 type 5.u12", 0x0000, 0x2000, CRC(f3daf85f) SHA1(3bd29538832d3084cbddeec92593988772755283) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_wd1002a_wx1_device::device_rom_region() const
{
	return ROM_NAME( wd1002a_wx1 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_wd1002a_wx1_device::device_add_mconfig(machine_config &config)
{
	WD11C00_17(config, m_host, XTAL(10'000'000)/2);
	m_host->out_irq5_callback().set(FUNC(isa8_wd1002a_wx1_device::irq5_w));
	m_host->out_drq3_callback().set(FUNC(isa8_wd1002a_wx1_device::drq3_w));
	m_host->out_mr_callback().set(m_mcu, FUNC(wd1015_device::mr_w));
	m_host->out_busy_callback().set(m_mcu, FUNC(wd1015_device::busy_w));
	m_host->in_rd322_callback().set(FUNC(isa8_wd1002a_wx1_device::rd322_r));
	m_host->in_ramcs_callback().set(m_mcu, FUNC(wd1015_device::ram_r));
	m_host->out_ramwr_callback().set(m_mcu, FUNC(wd1015_device::ram_w));

	WD1010(config, m_hdc, XTAL(10'000'000)/2);
	m_hdc->out_intrq_callback().set(m_mcu, FUNC(wd1015_device::hdc_intrq_w));
	m_hdc->in_data_callback().set(m_mcu, FUNC(wd1015_device::hdc_data_r));
	m_hdc->out_data_callback().set(m_mcu, FUNC(wd1015_device::hdc_data_w));

	HARDDISK(config, "hdc:0", 0);
	HARDDISK(config, "hdc:1", 0);

	WD1015(config, m_mcu, 0);
	m_mcu->set_host(*m_host);
	m_mcu->set_hdc(*m_hdc);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_wd1002a_wx1_device - constructor
//-------------------------------------------------

isa8_wd1002a_wx1_device::isa8_wd1002a_wx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_WD1002A_WX1, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_host(*this, "host")
	, m_hdc(*this, "hdc")
	, m_mcu(*this, "mcu")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_wd1002a_wx1_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc8000, 0xc9fff, "wd1002a_wx1");
	m_isa->install_device(0x0320, 0x0323, read8sm_delegate(*m_host, FUNC(wd11c00_17_device::io_r)), write8sm_delegate(*m_host, FUNC(wd11c00_17_device::io_w)));
	m_isa->set_dma_channel(3, this, false);
}


//-------------------------------------------------
//  device_isa8_card_interface overrides -- DMA channel 3
//-------------------------------------------------

uint8_t isa8_wd1002a_wx1_device::dack_r(int line)
{
	return m_host->dack_r();
}

void isa8_wd1002a_wx1_device::dack_w(int line, uint8_t data)
{
	m_host->dack_w(data);
}

void isa8_wd1002a_wx1_device::dack_line_w(int line, int state)
{
	m_host->dack3_w(state);
}


//-------------------------------------------------
//  WD11C00-17 host interface glue
//-------------------------------------------------

void isa8_wd1002a_wx1_device::irq5_w(int state)
{
	m_isa->irq5_w(state);
}

void isa8_wd1002a_wx1_device::drq3_w(int state)
{
	m_isa->drq3_w(state);
}

uint8_t isa8_wd1002a_wx1_device::rd322_r()
{
	return 0xff;
}

