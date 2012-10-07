/**********************************************************************

    Luxor 4105 SASI hard disk controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "lux4105.h"
#include "machine/scsibus.h"
#include "machine/scsicb.h"
#include "machine/scsihd.h"
#include "machine/s1410.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define SASIBUS_TAG		"sasi"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type LUXOR_4105 = &device_creator<luxor_4105_device>;


//-------------------------------------------------
//  SCSICB_interface sasi_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( luxor_4105_device::sasi_bsy_w )
{
	if (!state)
	{
		m_sasibus->scsi_sel_w(1);
	}
}

WRITE_LINE_MEMBER( luxor_4105_device::sasi_io_w )
{
	if (!m_io && state)
	{
		m_sasibus->scsi_data_w(m_data);
	}

	m_io = state;

	update_trrq_int();
}

WRITE_LINE_MEMBER( luxor_4105_device::sasi_req_w )
{
	if (state)
	{
		m_sasibus->scsi_ack_w(1);
	}

	update_trrq_int();
}

static const SCSICB_interface sasi_intf =
{
	NULL,
	DEVCB_DEVICE_LINE_MEMBER("^^", luxor_4105_device, sasi_bsy_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("^^", luxor_4105_device, sasi_io_w),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("^^", luxor_4105_device, sasi_req_w),
	DEVCB_NULL
};


//-------------------------------------------------
//  MACHINE_DRIVER( luxor_4105 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( luxor_4105 )
	MCFG_SCSIBUS_ADD(SASIBUS_TAG)
	MCFG_SCSIDEV_ADD(SASIBUS_TAG ":harddisk0", S1410, SCSI_ID_0)
	MCFG_SCSICB_ADD(SASIBUS_TAG ":host", sasi_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor luxor_4105_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( luxor_4105 );
}


//-------------------------------------------------
//  INPUT_PORTS( luxor_4105 )
//-------------------------------------------------

INPUT_PORTS_START( luxor_4105 )
	PORT_START("1E")
	PORT_DIPNAME( 0x03, 0x00, "Stepping" ) PORT_DIPLOCATION("1E:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "Half (Seagate/Texas)" )
	PORT_DIPSETTING(    0x02, "Half (Tandon)" )
	PORT_DIPSETTING(    0x03, "Buffered" )
	PORT_DIPNAME( 0x0c, 0x00, "Heads" ) PORT_DIPLOCATION("1E:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPSETTING(    0x0c, "8" )
	PORT_DIPNAME( 0xf0, 0x00, "Drive Type" ) PORT_DIPLOCATION("1E:5,6,7,8")
	PORT_DIPSETTING(    0x00, "Seagate ST506" )
	PORT_DIPSETTING(    0x10, "Rodime RO100" )
	PORT_DIPSETTING(    0x20, "Shugart SA600" )
	PORT_DIPSETTING(    0x30, "Seagate ST412" )

	PORT_START("5E")
	PORT_DIPNAME( 0x7f, 0x25, "Card Address" ) PORT_DIPLOCATION("5E:1,2,3,4,5,6,7")
	PORT_DIPSETTING(    0x25, "37" )
	PORT_DIPSETTING(    0x2d, "45" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor luxor_4105_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( luxor_4105 );
}


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline void luxor_4105_device::update_trrq_int()
{
	int cd = m_sasibus->scsi_cd_r();
	int req = m_sasibus->scsi_req_r();
	int trrq = !(cd & !req);

	if (BIT(m_dma, 5))
	{
		m_slot->int_w(trrq ? CLEAR_LINE : ASSERT_LINE);
	}
	else
	{
		m_slot->int_w(CLEAR_LINE);
	}

	if (BIT(m_dma, 6))
	{
		m_slot->trrq_w(trrq);
	}
	else
	{
		m_slot->trrq_w(1);
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  luxor_4105_device - constructor
//-------------------------------------------------

luxor_4105_device::luxor_4105_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, LUXOR_4105, "Luxor 4105", tag, owner, clock),
	  device_abc1600bus_card_interface(mconfig, *this),
	  m_sasibus(*this, SASIBUS_TAG ":host"),
	  m_io(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void luxor_4105_device::device_start()
{
	m_slot = dynamic_cast<abc1600bus_slot_device *>(owner());

	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_io));
	save_item(NAME(m_data));
	save_item(NAME(m_dma));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void luxor_4105_device::device_reset()
{
	m_cs = 0;
	m_data = 0;
	m_dma = 0;

	m_sasibus->scsi_rst_w(0);
	m_sasibus->scsi_rst_w(1);

	m_slot->trrq_w(1);
}



//**************************************************************************
//  ABC 1600 BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abc1600bus_cs -
//-------------------------------------------------

void luxor_4105_device::abc1600bus_cs(UINT8 data)
{
	m_cs = (data == ioport("5E")->read());
}


//-------------------------------------------------
//  abc1600bus_csb -
//-------------------------------------------------

int luxor_4105_device::abc1600bus_csb()
{
	return !m_cs;
}


//-------------------------------------------------
//  abc1600bus_rst -
//-------------------------------------------------

void luxor_4105_device::abc1600bus_brst()
{
	device_reset();
}


//-------------------------------------------------
//  abc1600bus_stat -
//-------------------------------------------------

UINT8 luxor_4105_device::abc1600bus_stat()
{
	UINT8 data = 0xff;

	if (m_cs)
	{
		/*

            bit     description

            0       ?
            1       ?
            2       ?
            3       ?
            4
            5
            6       ? (tested at 014D9A, after command 08 sent and 1 byte read from SASI, should be 1)
            7

        */

		data = !m_sasibus->scsi_bsy_r();
		data |= !m_sasibus->scsi_req_r() << 2;
		data |= !m_sasibus->scsi_cd_r() << 3;
		data |= !m_sasibus->scsi_io_r() << 6;
	}

	return data;
}


//-------------------------------------------------
//  abc1600bus_inp -
//-------------------------------------------------

UINT8 luxor_4105_device::abc1600bus_inp()
{
	UINT8 data = 0xff;

	if (m_cs)
	{
		if (m_sasibus->scsi_bsy_r())
		{
			ioport("1E")->read();
		}
		else
		{
			if (!m_sasibus->scsi_io_r())
			{
				data = m_sasibus->scsi_data_r();

				if (!m_sasibus->scsi_req_r())
				{
					m_sasibus->scsi_ack_w(0);
				}
			}
		}
	}

	return data;
}


//-------------------------------------------------
//  abc1600bus_utp -
//-------------------------------------------------

void luxor_4105_device::abc1600bus_out(UINT8 data)
{
	if (m_cs)
	{
		m_data = data;

		if (m_sasibus->scsi_io_r())
		{
			m_sasibus->scsi_data_w(m_data);

			if (!m_sasibus->scsi_req_r())
			{
				m_sasibus->scsi_ack_w(0);
			}
		}
	}
}


//-------------------------------------------------
//  abc1600bus_c1 -
//-------------------------------------------------

void luxor_4105_device::abc1600bus_c1(UINT8 data)
{
	if (m_cs)
	{
		m_sasibus->scsi_sel_w(0);
	}
}


//-------------------------------------------------
//  abc1600bus_c3 -
//-------------------------------------------------

void luxor_4105_device::abc1600bus_c3(UINT8 data)
{
	if (m_cs)
	{
		m_data = 0;
		m_dma = 0;

		m_sasibus->scsi_rst_w(0);
		m_sasibus->scsi_rst_w(1);
	}
}


//-------------------------------------------------
//  abc1600bus_c4 -
//-------------------------------------------------

void luxor_4105_device::abc1600bus_c4(UINT8 data)
{
	if (m_cs)
	{
		/*

            bit     description

            0
            1
            2
            3
            4
            5       byte interrupt enable?
            6       DMA/CPU mode (1=DMA, 0=CPU)?
            7       error interrupt enable?

        */

		m_dma = data;

		update_trrq_int();
	}
}
