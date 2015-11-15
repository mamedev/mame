// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC-PM001 Winchester Disk Controller emulation

**********************************************************************/

#include "wdc.h"
#include "bus/scsi/scsihd.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define OPTION_ID       0x01

#define Z80_TAG         "l53"
#define MK3882_TAG      "l07"

#define OPTION_DREQ1    BIT(m_option, 1)
#define OPTION_DREQ2    BIT(m_option, 2)
#define OPTION_DREQ3    BIT(m_option, 3)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type WANGPC_WDC = &device_creator<wangpc_wdc_device>;


//-------------------------------------------------
//  ROM( wangpc_wdc )
//-------------------------------------------------

ROM_START( wangpc_wdc )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "378-9040 r9.l19", 0x0000, 0x1000, CRC(282770d2) SHA1(a0e3bad5041e0dfd6087907015b07a093b576bc0) )

	ROM_REGION( 0x1000, "address", 0 )
	ROM_LOAD( "378-9041.l54", 0x0000, 0x1000, CRC(94e9a17d) SHA1(060c576d70069ece2d0dbce86ffc448df2b169e7) )

	ROM_REGION( 0x100, "prom", 0 )
	ROM_LOAD( "376-8002.l66", 0x000, 0x100, NO_DUMP ) // DL2212-105
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *wangpc_wdc_device::device_rom_region() const
{
	return ROM_NAME( wangpc_wdc );
}


//-------------------------------------------------
//  ADDRESS_MAP( wangpc_wdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( wangpc_wdc_mem, AS_PROGRAM, 8, wangpc_wdc_device )
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION(Z80_TAG, 0)
	AM_RANGE(0x1000, 0x17ff) AM_RAM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( wangpc_wdc_io )
//-------------------------------------------------

static ADDRESS_MAP_START( wangpc_wdc_io, AS_IO, 8, wangpc_wdc_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_READ(port_r)
	AM_RANGE(0x03, 0x03) AM_WRITE(status_w)
	AM_RANGE(0x10, 0x10) AM_READWRITE(ctc_ch0_r, ctc_ch0_w)
	AM_RANGE(0x14, 0x14) AM_READWRITE(ctc_ch1_r, ctc_ch1_w)
	AM_RANGE(0x18, 0x18) AM_READWRITE(ctc_ch2_r, ctc_ch2_w)
	AM_RANGE(0x1c, 0x1c) AM_READWRITE(ctc_ch3_r, ctc_ch3_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( wangpc_wdc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( wangpc_wdc )
	MCFG_CPU_ADD(Z80_TAG, Z80, 2000000) // XTAL_10MHz / ?
	//MCFG_CPU_CONFIG(wangpc_wdc_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(wangpc_wdc_mem)
	MCFG_CPU_IO_MAP(wangpc_wdc_io)

	MCFG_DEVICE_ADD(MK3882_TAG, Z80CTC, 2000000)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("harddisk0", SCSIHD, 0)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor wangpc_wdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( wangpc_wdc );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_irq -
//-------------------------------------------------

inline void wangpc_wdc_device::set_irq(int state)
{
	m_irq = state;

	if (OPTION_DREQ1) m_bus->irq5_w(m_irq);
	if (OPTION_DREQ2) m_bus->irq6_w(m_irq);
	if (OPTION_DREQ3) m_bus->irq7_w(m_irq);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpc_wdc_device - constructor
//-------------------------------------------------

wangpc_wdc_device::wangpc_wdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WANGPC_WDC, "Wang PC-PM001", tag, owner, clock, "wangpc_wdc", __FILE__),
	device_wangpcbus_card_interface(mconfig, *this),
	m_maincpu(*this, Z80_TAG),
	m_ctc(*this, MK3882_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpc_wdc_device::device_start()
{
	// state saving
	save_item(NAME(m_status));
	save_item(NAME(m_option));
	save_item(NAME(m_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wangpc_wdc_device::device_reset()
{
	m_status = 0;
	m_option = 0;

	set_irq(CLEAR_LINE);
}


//-------------------------------------------------
//  wangpcbus_mrdc_r - memory read
//-------------------------------------------------

UINT16 wangpc_wdc_device::wangpcbus_mrdc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

	return data;
}


//-------------------------------------------------
//  wangpcbus_amwc_w - memory write
//-------------------------------------------------

void wangpc_wdc_device::wangpcbus_amwc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
{
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

UINT16 wangpc_wdc_device::wangpcbus_iorc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0x00/2:
			data = m_status;
			break;

		case 0x02/2:
			// TODO operation status register
			break;

		case 0x04/2:
			set_irq(CLEAR_LINE);
			break;

		case 0xfe/2:
			data = 0xff00 | (m_irq << 7) | OPTION_ID;
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_aiowc_w - I/O write
//-------------------------------------------------

void wangpc_wdc_device::wangpcbus_aiowc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
{
	if (sad(offset) && ACCESSING_BITS_0_7)
	{
		switch (offset & 0x7f)
		{
		case 0x02/2:
			// TODO command register
			break;

		case 0xfc/2:
			device_reset();
			break;

		case 0xfe/2:
			{
				bool irq = (m_irq == ASSERT_LINE);
				bool changed = ((m_option & 0x0e) != (data & 0x0e));

				if (irq && changed) set_irq(CLEAR_LINE);

				m_option = data & 0xff;

				if (irq && changed) set_irq(ASSERT_LINE);
			}
			break;
		}
	}
}


//-------------------------------------------------
//  wangpcbus_dack_r - DMA acknowledge read
//-------------------------------------------------

UINT8 wangpc_wdc_device::wangpcbus_dack_r(address_space &space, int line)
{
	return 0;
}


//-------------------------------------------------
//  wangpcbus_dack_r - DMA acknowledge write
//-------------------------------------------------

void wangpc_wdc_device::wangpcbus_dack_w(address_space &space, int line, UINT8 data)
{
}


//-------------------------------------------------
//  wangpcbus_have_dack -
//-------------------------------------------------

bool wangpc_wdc_device::wangpcbus_have_dack(int line)
{
	return (OPTION_DREQ1 && (line == 1)) || (OPTION_DREQ2 && (line == 2)) || (OPTION_DREQ3 && (line == 3));
}


//-------------------------------------------------
//  port_r -
//-------------------------------------------------

READ8_MEMBER( wangpc_wdc_device::port_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	return 0x72; // TODO
}


//-------------------------------------------------
//  status_w - status register write
//-------------------------------------------------

WRITE8_MEMBER( wangpc_wdc_device::status_w )
{
	logerror("WDC status %02x\n", data);

	m_status = data;
}


READ8_MEMBER( wangpc_wdc_device::ctc_ch0_r ) { return m_ctc->read(space, 0); }
WRITE8_MEMBER( wangpc_wdc_device::ctc_ch0_w ) { m_ctc->write(space, 0, data); }
READ8_MEMBER( wangpc_wdc_device::ctc_ch1_r ) { return m_ctc->read(space, 1); }
WRITE8_MEMBER( wangpc_wdc_device::ctc_ch1_w ) { m_ctc->write(space, 1, data); }
READ8_MEMBER( wangpc_wdc_device::ctc_ch2_r ) { return m_ctc->read(space, 2); }
WRITE8_MEMBER( wangpc_wdc_device::ctc_ch2_w ) { m_ctc->write(space, 2, data); }
READ8_MEMBER( wangpc_wdc_device::ctc_ch3_r ) { return m_ctc->read(space, 3); }
WRITE8_MEMBER( wangpc_wdc_device::ctc_ch3_w ) { m_ctc->write(space, 3, data); }
