/**********************************************************************

    COMX-35 Disk Controller Card emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

#include "comx_fd.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define WD1770_TAG			"wd1770"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMX_FD = &device_creator<comx_fd_device>;


//-------------------------------------------------
//  ROM( comx_fd )
//-------------------------------------------------

ROM_START( comx_fd )
	ROM_REGION( 0x2000, "c000", 0 )
	ROM_LOAD( "d.o.s. v1.2.f4",	0x0000, 0x2000, CRC(cf4ecd2e) SHA1(290e19bdc89e3c8059e63d5ae3cca4daa194e1fe) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *comx_fd_device::device_rom_region() const
{
	return ROM_NAME( comx_fd );
}


//-------------------------------------------------
//  wd17xx_interface fdc_intf
//-------------------------------------------------

static const floppy_interface floppy_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSSD,
	LEGACY_FLOPPY_OPTIONS_NAME(comx35),
	"floppy_5_25",
	NULL
};

WRITE_LINE_MEMBER( comx_fd_device::intrq_w )
{
	m_intrq = state;
}

WRITE_LINE_MEMBER( comx_fd_device::drq_w )
{
	m_drq = state;

	update_ef4();
}

static const wd17xx_interface fdc_intf =
{
	DEVCB_LINE_VCC,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, comx_fd_device, intrq_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, comx_fd_device, drq_w),
	{ FLOPPY_0, FLOPPY_1, NULL, NULL }
};


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( comx_fd )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( comx_fd )
	MCFG_WD1770_ADD(WD1770_TAG, fdc_intf)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(floppy_intf)
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
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_ef4 -
//-------------------------------------------------

inline void comx_fd_device::update_ef4()
{
	if (m_ds && !m_disb)
	{
		m_slot->ef4_w(!m_drq);
	}
	else
	{
		m_slot->ef4_w(CLEAR_LINE);
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_fd_device - constructor
//-------------------------------------------------

comx_fd_device::comx_fd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COMX_FD, "COMX FD", tag, owner, clock),
	device_comx_expansion_card_interface(mconfig, *this),
	m_fdc(*this, WD1770_TAG),
	m_floppy0(*this, FLOPPY_0),
	m_floppy1(*this, FLOPPY_1),
	m_ds(0),
	m_q(0),
	m_addr(0),
	m_intrq(0),
	m_drq(0),
	m_disb(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void comx_fd_device::device_start()
{
	m_rom = memregion("c000")->base();

	// state saving
	save_item(NAME(m_ds));
	save_item(NAME(m_q));
	save_item(NAME(m_addr));
	save_item(NAME(m_intrq));
	save_item(NAME(m_drq));
	save_item(NAME(m_disb));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void comx_fd_device::device_reset()
{
	wd17xx_reset(m_fdc);
}


//-------------------------------------------------
//  comx_q_w - Q write
//-------------------------------------------------

void comx_fd_device::comx_q_w(int state)
{
	m_q = state;
}


//-------------------------------------------------
//  comx_ds_w - device select write
//-------------------------------------------------

void comx_fd_device::comx_ds_w(int state)
{
	m_ds = state;

	update_ef4();
}


//-------------------------------------------------
//  comx_mrd_r - memory read
//-------------------------------------------------

UINT8 comx_fd_device::comx_mrd_r(offs_t offset, int *extrom)
{
	UINT8 data = 0xff;

	if (offset >= 0x0dd0 && offset < 0x0de0)
	{
		data = m_rom[offset & 0x1fff];
		*extrom = 0;
	}
	if (offset >= 0xc000 && offset < 0xe000)
	{
		data = m_rom[offset & 0x1fff];
	}

	return data;
}


//-------------------------------------------------
//  comx_io_r - I/O read
//-------------------------------------------------

UINT8 comx_fd_device::comx_io_r(offs_t offset)
{
	UINT8 data = 0xff;

	if (offset == 2)
	{
		if (m_q)
		{
			data = m_intrq;
		}
		else
		{
			data = wd17xx_r(m_fdc, m_addr);
		}
	}

	return data;
}


//-------------------------------------------------
//  comx_io_w - I/O write
//-------------------------------------------------

void comx_fd_device::comx_io_w(offs_t offset, UINT8 data)
{
	if (offset == 2)
	{
		if (m_q)
		{
			/*

                bit     description

                0       A0
                1       A1
                2       DRIVE0
                3       DRIVE1
                4       F9 DISB
                5       SIDE SELECT

            */

			// latch data to F3
			m_addr = data & 0x03;

			if (BIT(data, 2))
			{
				wd17xx_set_drive(m_fdc, 0);
			}
			else if (BIT(data, 3))
			{
				wd17xx_set_drive(m_fdc, 1);
			}

			m_disb = !BIT(data, 4);
			update_ef4();

			wd17xx_set_side(m_fdc, BIT(data, 5));
		}
		else
		{
			// write data to WD1770
			wd17xx_w(m_fdc, m_addr, data);
		}
	}
}
