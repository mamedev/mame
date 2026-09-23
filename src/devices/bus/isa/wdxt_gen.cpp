// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WDXT-GEN ISA XT MFM Hard Disk Controller

**********************************************************************/

/*

PCB Layout
----------

ASSY 61-000222-00

|-------------------------------------------|
|   CN2     CN1                             |
|   CN3     7406                TMM2016     |
|           LS38    LS14                    |
| MC3486                                    |
|               WD1015          WD11C00     |
|           33.04MHz                        |
| MC3487                                    |
|               WD2010          LS244       |
|   WD10C20                                 |
|               LS260   LS13        ROM     |
|                                           |
|---|                                   |---|
    |-----------------------------------|

Notes:
    All IC's shown.

    ROM     - Toshiba TMM2464AP 8Kx8 ROM "3"
    TMM2016 - Toshiba TMM2016BP-10 2Kx8 SRAM
    WD1015  - Western Digital WD1015-PL-54-02 Buffer Manager Control Processor
    WD11C00 - Western Digital WD11C00L-JT-17-02 PC/XT Host Interface Logic Device
    WD10C20 - Western Digital WD10C20B-PH-05-05 Self-Adjusting Data Separator
    WD2010  - Western Digital WD2010A-PL-05-02 Winchester Disk Controller
    CN1     - 2x17 pin PCB header, control
    CN2     - 2x10 pin PCB header, drive 0 data
    CN3     - 2x10 pin PCB header, drive 1 data

*/

#include "emu.h"
#include "wdxt_gen.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_WDXT_GEN, wdxt_gen_device, "wdxt_gen", "Western Digital WDXT-GEN (Amstrad PC1512/1640)")


//-------------------------------------------------
//  ROM( wdxt_gen )
//-------------------------------------------------

ROM_START( wdxt_gen )
	ROM_REGION( 0x2000, "bios", 0 )
	ROM_LOAD( "62-000100-003.u13", 0x0000, 0x2000, CRC(fbcb5f91) SHA1(8c22bd664177eb6126f3011eda8c5655fffe0ef2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *wdxt_gen_device::device_rom_region() const
{
	return ROM_NAME( wdxt_gen );
}


//-------------------------------------------------
//  WD11C00_17 host interface glue
//-------------------------------------------------

void wdxt_gen_device::irq5_w(int state)
{
	m_isa->irq5_w(state);
}

void wdxt_gen_device::drq3_w(int state)
{
	m_isa->drq3_w(state);
}

uint8_t wdxt_gen_device::rd322_r()
{
	return 0xff;
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void wdxt_gen_device::device_add_mconfig(machine_config &config)
{
	WD11C00_17(config, m_host, 5000000);
	m_host->out_irq5_callback().set(FUNC(wdxt_gen_device::irq5_w));
	m_host->out_drq3_callback().set(FUNC(wdxt_gen_device::drq3_w));
	m_host->out_mr_callback().set(m_mcu, FUNC(wd1015_device::mr_w));
	m_host->out_busy_callback().set(m_mcu, FUNC(wd1015_device::busy_w));
	m_host->in_rd322_callback().set(FUNC(wdxt_gen_device::rd322_r));
	m_host->in_ramcs_callback().set(m_mcu, FUNC(wd1015_device::ram_r));
	m_host->out_ramwr_callback().set(m_mcu, FUNC(wd1015_device::ram_w));

	WD1010(config, m_hdc, 5000000);
	m_hdc->out_intrq_callback().set(m_mcu, FUNC(wd1015_device::hdc_intrq_w));
	m_hdc->in_data_callback().set(m_mcu, FUNC(wd1015_device::hdc_data_r));
	m_hdc->out_data_callback().set(m_mcu, FUNC(wd1015_device::hdc_data_w));

	HARDDISK(config, "hdc:0", "st_hdd");
	HARDDISK(config, "hdc:1", "st_hdd");

	WD1015PL5402(config, m_mcu, 0);
	m_mcu->set_host(*m_host);
	m_mcu->set_hdc(*m_hdc);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wdxt_gen_device - constructor
//-------------------------------------------------

wdxt_gen_device::wdxt_gen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_WDXT_GEN, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_host(*this, "u11")
	, m_hdc(*this, "hdc")
	, m_mcu(*this, "mcu")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wdxt_gen_device::device_start()
{
	set_isa_device();
	m_isa->install_rom(this, 0xc8000, 0xc9fff, "bios");
	m_isa->install_device(0x0320, 0x0323, read8sm_delegate(*m_host, FUNC(wd11c00_17_device::io_r)), write8sm_delegate(*m_host, FUNC(wd11c00_17_device::io_w)));
	m_isa->set_dma_channel(3, this, false);
}


//-------------------------------------------------
//  dack_r -
//-------------------------------------------------

uint8_t wdxt_gen_device::dack_r(int line)
{
	return m_host->dack_r();
}


//-------------------------------------------------
//  dack_w -
//-------------------------------------------------

void wdxt_gen_device::dack_w(int line, uint8_t data)
{
	m_host->dack_w(data);
}


//-------------------------------------------------
//  dack_line_w -
//-------------------------------------------------

void wdxt_gen_device::dack_line_w(int line, int state)
{
	m_host->dack3_w(state);
}
