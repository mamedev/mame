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
//  MACROS / CONSTANTS
//**************************************************************************

#define WD1015_TAG      "u6"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_WDXT_GEN, wdxt_gen_device, "wdxt_gen", "Western Digital WDXT-GEN (Amstrad PC1512/1640)")


//-------------------------------------------------
//  ROM( wdxt_gen )
//-------------------------------------------------

ROM_START( wdxt_gen )
	ROM_REGION( 0x800, WD1015_TAG, 0 )
	ROM_LOAD( "wd1015-pl-54-02.u6", 0x000, 0x800, CRC(116e0608) SHA1(bcbd6b39c5a7e16e3bae9372b53d54d6761ba6bc) )

	ROM_REGION( 0x2000, "hdc", 0 )
	ROM_LOAD( "3.u13", 0x0000, 0x2000, CRC(fbcb5f91) SHA1(8c22bd664177eb6126f3011eda8c5655fffe0ef2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *wdxt_gen_device::device_rom_region() const
{
	return ROM_NAME( wdxt_gen );
}


//-------------------------------------------------
//  ADDRESS_MAP( wd1015_io )
//-------------------------------------------------

void wdxt_gen_device::wd1015_io(address_map &map)
{
	map(0x00, 0xff).rw(m_host, FUNC(wd11c00_17_device::read), FUNC(wd11c00_17_device::write));
}


//-------------------------------------------------
//  WD11C00_17_INTERFACE( host_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( wdxt_gen_device::irq5_w )
{
	m_isa->irq5_w(state);
}

WRITE_LINE_MEMBER( wdxt_gen_device::drq3_w )
{
	m_isa->drq3_w(state);
}

WRITE_LINE_MEMBER( wdxt_gen_device::mr_w )
{
	if (state == ASSERT_LINE)
	{
		device_reset();
	}
}

READ8_MEMBER( wdxt_gen_device::rd322_r )
{
	return 0xff;
}

READ8_MEMBER( wdxt_gen_device::ram_r )
{
	return m_ram[offset];
}

WRITE8_MEMBER( wdxt_gen_device::ram_w )
{
	m_ram[offset] = data;
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void wdxt_gen_device::device_add_mconfig(machine_config &config)
{
	mcs48_cpu_device &cpu(I8049(config, m_maincpu, 5000000));
	cpu.set_addrmap(AS_IO, &wdxt_gen_device::wd1015_io);
	cpu.t0_in_cb().set(m_host, FUNC(wd11c00_17_device::busy_r));
	cpu.t1_in_cb().set(FUNC(wdxt_gen_device::wd1015_t1_r));
	cpu.p1_in_cb().set(FUNC(wdxt_gen_device::wd1015_p1_r));
	cpu.p1_out_cb().set(FUNC(wdxt_gen_device::wd1015_p1_w));
	cpu.p2_in_cb().set(FUNC(wdxt_gen_device::wd1015_p2_r));
	cpu.p2_out_cb().set(FUNC(wdxt_gen_device::wd1015_p2_w));

	WD11C00_17(config, m_host, 5000000);
	m_host->out_irq5_callback().set(FUNC(wdxt_gen_device::irq5_w));
	m_host->out_drq3_callback().set(FUNC(wdxt_gen_device::drq3_w));
	m_host->out_mr_callback().set(FUNC(wdxt_gen_device::mr_w));
	m_host->out_ra3_callback().set_inputline(m_maincpu, MCS48_INPUT_IRQ);
	m_host->in_rd322_callback().set(FUNC(wdxt_gen_device::rd322_r));
	m_host->in_ramcs_callback().set(FUNC(wdxt_gen_device::ram_r));
	m_host->out_ramwr_callback().set(FUNC(wdxt_gen_device::ram_w));
	m_host->in_cs1010_callback().set(m_hdc, FUNC(wd2010_device::read));
	m_host->out_cs1010_callback().set(m_hdc, FUNC(wd2010_device::write));

	WD2010(config, m_hdc, 5000000);
	m_hdc->out_bcr_callback().set(m_host, FUNC(wd11c00_17_device::clct_w));
	m_hdc->in_bcs_callback().set(m_host, FUNC(wd11c00_17_device::read));
	m_hdc->out_bcs_callback().set(m_host, FUNC(wd11c00_17_device::write));
	m_hdc->in_drdy_callback().set_constant(1);
	m_hdc->in_index_callback().set_constant(1);
	m_hdc->in_wf_callback().set_constant(1);
	m_hdc->in_tk000_callback().set_constant(1);
	m_hdc->in_sc_callback().set_constant(1);

	HARDDISK(config, "hard0", 0);
	HARDDISK(config, "hard1", 0);
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
	, m_maincpu(*this, WD1015_TAG)
	, m_host(*this, "u11")
	, m_hdc(*this, "u7")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wdxt_gen_device::device_start()
{
	set_isa_device();
	m_isa->install_rom(this, 0xc8000, 0xc9fff, "hdc", "hdc");
	m_isa->install_device(0x0320, 0x0323, read8_delegate(*m_host, FUNC(wd11c00_17_device::io_r)), write8_delegate(*m_host, FUNC(wd11c00_17_device::io_w)));
	m_isa->set_dma_channel(3, this, false);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wdxt_gen_device::device_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
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


//-------------------------------------------------
//  wd1015_t1_r -
//-------------------------------------------------

READ_LINE_MEMBER( wdxt_gen_device::wd1015_t1_r )
{
	return 0; // TODO
}


//-------------------------------------------------
//  wd1015_p1_r -
//-------------------------------------------------

READ8_MEMBER( wdxt_gen_device::wd1015_p1_r )
{
	/*

	    bit     description

	    P10
	    P11
	    P12
	    P13
	    P14
	    P15
	    P16
	    P17

	*/

	uint8_t data = 0;

	logerror("%s P1 read %02x\n", machine().describe_context(), data);

	return data;
}


//-------------------------------------------------
//  wd1015_p1_w -
//-------------------------------------------------

WRITE8_MEMBER( wdxt_gen_device::wd1015_p1_w )
{
	/*

	    bit     description

	    P10     HSEL0
	    P11     HSEL1
	    P12     HSEL2
	    P13     _DSEL0
	    P14     _DSEL1
	    P15
	    P16     IREQ
	    P17     _DIRIN

	*/

	logerror("%s P1 %02x\n", machine().describe_context(), data);

	m_host->ireq_w(BIT(data, 6));
}


//-------------------------------------------------
//  wd1015_p2_r -
//-------------------------------------------------

READ8_MEMBER( wdxt_gen_device::wd1015_p2_r )
{
	/*

	    bit     description

	    P20
	    P21
	    P22
	    P23
	    P24
	    P25
	    P26     TK000
	    P27     ECC NOT 0

	*/

	uint8_t data = 0x40;

	data |= m_host->ecc_not_0_r() << 7;

	logerror("%s P2 read %02x\n", machine().describe_context(), data);

	return data;
}


//-------------------------------------------------
//  wd1015_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( wdxt_gen_device::wd1015_p2_w )
{
	/*

	    bit     description

	    P20     STEP
	    P21     ?
	    P22     MODE?
	    P23     ?
	    P24     ?
	    P25     ?
	    P26
	    P27

	*/

	logerror("%s P2 %02x\n", machine().describe_context(), data);

	m_host->mode_w(BIT(data, 2));
}
