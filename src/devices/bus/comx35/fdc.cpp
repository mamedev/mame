// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 Disk Controller Card emulation

**********************************************************************/

/*

(c) 1984 Comx World Operations

PCB Layout
----------

F-001-FD-REV0

    |---------------|
    |      CN1      |
|---|               |---------------------------|
|                                               |
|   40174               4068    4072           -|
|           ROM                                ||
|   LS04                4072    4050    7438   C|
|8MHz                                          N|
|                       4049    4075    LS08   2|
|LD1        WD1770                             ||
|   40174               4503    4075    7438   -|
|LD2                                            |
|-----------------------------------------------|

Notes:
    All IC's shown.

    ROM     - "D.O.S. V1.2"
    WD1770  - Western Digital WD1770-xx Floppy Disc Controller @ 8MHz
    CN1     - COMX-35 bus PCB edge connector
    CN2     - 34 pin floppy connector
    LD1     - card selected LED
    LD2     - floppy motor on LED

*/

#include "fdc.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define WD1770_TAG          "wd1770"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMX_FD = &device_creator<comx_fd_device>;


//-------------------------------------------------
//  ROM( comx_fd )
//-------------------------------------------------

ROM_START( comx_fd )
	ROM_REGION( 0x2000, "c000", 0 )
	ROM_LOAD( "d.o.s. v1.2.f4", 0x0000, 0x2000, CRC(cf4ecd2e) SHA1(290e19bdc89e3c8059e63d5ae3cca4daa194e1fe) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *comx_fd_device::device_rom_region() const
{
	return ROM_NAME( comx_fd );
}


FLOPPY_FORMATS_MEMBER( comx_fd_device::floppy_formats )
	FLOPPY_COMX35_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( comx_fd_floppies )
	SLOT_INTERFACE( "525sd35t", FLOPPY_525_SD_35T )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( comx_fd )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( comx_fd )
	MCFG_WD1770_ADD(WD1770_TAG, XTAL_8MHz)

	MCFG_FLOPPY_DRIVE_ADD(WD1770_TAG":0", comx_fd_floppies, "525sd35t", comx_fd_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD1770_TAG":1", comx_fd_floppies, nullptr,       comx_fd_device::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor comx_fd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( comx_fd );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_fd_device - constructor
//-------------------------------------------------

comx_fd_device::comx_fd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COMX_FD, "COMX FD", tag, owner, clock, "comx_fd", __FILE__),
	device_comx_expansion_card_interface(mconfig, *this),
	m_fdc(*this, WD1770_TAG),
	m_floppy0(*this, WD1770_TAG":0"),
	m_floppy1(*this, WD1770_TAG":1"),
	m_rom(*this, "c000"),
	m_q(0),
	m_addr(0),
	m_disb(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void comx_fd_device::device_start()
{
	// state saving
	save_item(NAME(m_ds));
	save_item(NAME(m_q));
	save_item(NAME(m_addr));
	save_item(NAME(m_disb));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void comx_fd_device::device_reset()
{
	m_fdc->reset();
	m_fdc->dden_w(1);
	m_fdc->set_floppy(nullptr);

	m_addr = 0;
	m_disb = 1;
}


//-------------------------------------------------
//  comx_ef4_r - external flag 4 read
//-------------------------------------------------

int comx_fd_device::comx_ef4_r()
{
	int state = CLEAR_LINE;

	if (m_ds && !m_disb)
	{
		state = !m_fdc->drq_r();
	}

	return state;
}


//-------------------------------------------------
//  comx_q_w - Q write
//-------------------------------------------------

void comx_fd_device::comx_q_w(int state)
{
	m_q = state;
}


//-------------------------------------------------
//  comx_mrd_r - memory read
//-------------------------------------------------

UINT8 comx_fd_device::comx_mrd_r(address_space &space, offs_t offset, int *extrom)
{
	UINT8 data = 0xff;

	if (offset >= 0x0dd0 && offset < 0x0de0)
	{
		data = m_rom->base()[offset & 0x1fff];
		*extrom = 0;
	}
	else if (offset >= 0xc000 && offset < 0xe000)
	{
		data = m_rom->base()[offset & 0x1fff];
	}

	return data;
}


//-------------------------------------------------
//  comx_io_r - I/O read
//-------------------------------------------------

UINT8 comx_fd_device::comx_io_r(address_space &space, offs_t offset)
{
	UINT8 data = 0xff;

	if (offset == 2)
	{
		if (m_q)
		{
			data = 0xfe | (m_fdc->intrq_r() ? 1 : 0);
		}
		else
		{
			data = m_fdc->gen_r(m_addr);
			if (m_addr==3) logerror("%s FDC read %u:%02x\n", machine().describe_context(), m_addr,data);
		}
	}

	return data;
}


//-------------------------------------------------
//  comx_io_w - I/O write
//-------------------------------------------------

void comx_fd_device::comx_io_w(address_space &space, offs_t offset, UINT8 data)
{
	if (offset == 2)
	{
		if (m_q)
		{
			/*

			    bit     description

			    0       FDC A0
			    1       FDC A1
			    2       DRIVE0
			    3       DRIVE1
			    4       F9 DISB
			    5       SIDE SELECT

			*/

			// latch data to F3
			m_addr = data & 0x03;

			// drive select
			floppy_image_device *floppy = nullptr;

			if (BIT(data, 2)) floppy = m_floppy0->get_device();
			if (BIT(data, 3)) floppy = m_floppy1->get_device();

			m_fdc->set_floppy(floppy);

			if (floppy) floppy->ss_w(BIT(data, 5));

			m_disb = !BIT(data, 4);
		}
		else
		{
			// write data to WD1770
			m_fdc->gen_w(m_addr, data);
		}
	}
}
