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

#include "wdxt_gen.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define WD1015_TAG      "u6"
#define WD11C00_17_TAG  "u11"
#define WD2010A_TAG     "u7"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ISA8_WDXT_GEN = &device_creator<wdxt_gen_device>;


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

const rom_entry *wdxt_gen_device::device_rom_region() const
{
	return ROM_NAME( wdxt_gen );
}


//-------------------------------------------------
//  ADDRESS_MAP( wd1015_io )
//-------------------------------------------------

static ADDRESS_MAP_START( wd1015_io, AS_IO, 8, wdxt_gen_device )
	AM_RANGE(0x00, 0xff) AM_DEVREADWRITE(WD11C00_17_TAG, wd11c00_17_device, read, write)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(wd1015_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(wd1015_t1_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(wd1015_p1_r, wd1015_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(wd1015_p2_r, wd1015_p2_w)
ADDRESS_MAP_END


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
//  MACHINE_DRIVER( wdxt_gen )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( wdxt_gen )
	MCFG_CPU_ADD(WD1015_TAG, I8049, 5000000)
	MCFG_CPU_IO_MAP(wd1015_io)

	MCFG_DEVICE_ADD(WD11C00_17_TAG, WD11C00_17, 5000000)
	MCFG_WD11C00_17_OUT_IRQ5_CB(WRITELINE(wdxt_gen_device, irq5_w))
	MCFG_WD11C00_17_OUT_DRQ3_CB(WRITELINE(wdxt_gen_device, drq3_w))
	MCFG_WD11C00_17_OUT_MR_CB(WRITELINE(wdxt_gen_device, mr_w))
	MCFG_WD11C00_17_OUT_RA3_CB(INPUTLINE(WD1015_TAG, MCS48_INPUT_IRQ))
	MCFG_WD11C00_17_IN_RD322_CB(READ8(wdxt_gen_device, rd322_r))
	MCFG_WD11C00_17_IN_RAMCS_CB(READ8(wdxt_gen_device, ram_r))
	MCFG_WD11C00_17_OUT_RAMWR_CB(WRITE8(wdxt_gen_device, ram_w))
	MCFG_WD11C00_17_IN_CS1010_CB(DEVREAD8(WD2010A_TAG, wd2010_device, read))
	MCFG_WD11C00_17_OUT_CS1010_CB(DEVWRITE8(WD2010A_TAG, wd2010_device, write))
	MCFG_DEVICE_ADD(WD2010A_TAG, WD2010, 5000000)
	MCFG_WD2010_OUT_BCR_CB(DEVWRITELINE(WD11C00_17_TAG, wd11c00_17_device, clct_w))
	MCFG_WD2010_IN_BCS_CB(DEVREAD8(WD11C00_17_TAG, wd11c00_17_device, read))
	MCFG_WD2010_OUT_BCS_CB(DEVWRITE8(WD11C00_17_TAG, wd11c00_17_device, write))
	MCFG_WD2010_IN_DRDY_CB(VCC)
	MCFG_WD2010_IN_INDEX_CB(VCC)
	MCFG_WD2010_IN_WF_CB(VCC)
	MCFG_WD2010_IN_TK000_CB(VCC)
	MCFG_WD2010_IN_SC_CB(VCC)

	MCFG_HARDDISK_ADD("hard0")
	MCFG_HARDDISK_ADD("hard1")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor wdxt_gen_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( wdxt_gen );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wdxt_gen_device - constructor
//-------------------------------------------------

wdxt_gen_device::wdxt_gen_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ISA8_WDXT_GEN, "Western Digital WDXT-GEN (Amstrad PC1512/1640)", tag, owner, clock, "wdxt_gen", __FILE__),
		device_isa8_card_interface(mconfig, *this),
		m_maincpu(*this, WD1015_TAG),
		m_host(*this, WD11C00_17_TAG),
		m_hdc(*this, WD2010A_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wdxt_gen_device::device_start()
{
	set_isa_device();
	m_isa->install_rom(this, 0xc8000, 0xc9fff, 0, 0, "hdc", "hdc");
	m_isa->install_device(0x0320, 0x0323, 0, 0, READ8_DEVICE_DELEGATE(m_host, wd11c00_17_device, io_r), WRITE8_DEVICE_DELEGATE(m_host, wd11c00_17_device, io_w));
	m_isa->set_dma_channel(3, this, FALSE);
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

UINT8 wdxt_gen_device::dack_r(int line)
{
	return m_host->dack_r();
}


//-------------------------------------------------
//  dack_w -
//-------------------------------------------------

void wdxt_gen_device::dack_w(int line, UINT8 data)
{
	m_host->dack_w(data);
}

//-------------------------------------------------
//  wd1015_t0_r -
//-------------------------------------------------

READ8_MEMBER( wdxt_gen_device::wd1015_t0_r )
{
	return m_host->busy_r();
}


//-------------------------------------------------
//  wd1015_t1_r -
//-------------------------------------------------

READ8_MEMBER( wdxt_gen_device::wd1015_t1_r )
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

	UINT8 data = 0;

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

	UINT8 data = 0x40;

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
