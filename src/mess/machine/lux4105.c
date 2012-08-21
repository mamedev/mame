/**********************************************************************

    Luxor 4105 SASI hard disk controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "lux4105.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define SASIBUS_TAG		"sasi"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type LUXOR_4105 = &device_creator<luxor_4105_device>;


//-------------------------------------------------
//  SCSIBus_interface sasi_intf
//-------------------------------------------------

static const SCSIConfigTable sasi_dev_table =
{
	1,
	{
		{ SCSI_ID_0, "harddisk0" }
	}
};

WRITE_LINE_MEMBER( luxor_4105_device::sasi_bsy_w )
{
	if (!state)
	{
		scsi_sel_w(m_sasibus, 1);
	}
}

WRITE_LINE_MEMBER( luxor_4105_device::sasi_io_w )
{
	if (!m_io && state)
	{
		scsi_data_w(m_sasibus, m_data);
	}

	m_io = state;

	update_trrq_int();
}

WRITE_LINE_MEMBER( luxor_4105_device::sasi_req_w )
{
	if (state)
	{
		scsi_ack_w(m_sasibus, 1);
	}

	update_trrq_int();
}

static const SCSIBus_interface sasi_intf =
{
    &sasi_dev_table,
    NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, luxor_4105_device, sasi_bsy_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, luxor_4105_device, sasi_io_w),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, luxor_4105_device, sasi_req_w),
	DEVCB_NULL
};


//-------------------------------------------------
//  MACHINE_DRIVER( luxor_4105 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( luxor_4105 )
    MCFG_SCSIBUS_ADD(SASIBUS_TAG, sasi_intf)
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
	int cd = scsi_cd_r(m_sasibus);
	int req = scsi_req_r(m_sasibus);
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
	  m_sasibus(*this, SASIBUS_TAG),
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
	init_scsibus(m_sasibus, 512);

	m_cs = 0;
	m_data = 0;
	m_dma = 0;

	scsi_rst_w(m_sasibus, 0);
	scsi_rst_w(m_sasibus, 1);

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

		data = !scsi_bsy_r(m_sasibus);
		data |= !scsi_req_r(m_sasibus) << 2;
		data |= !scsi_cd_r(m_sasibus) << 3;
		data |= !scsi_io_r(m_sasibus) << 6;
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
		if (scsi_bsy_r(m_sasibus))
		{
			ioport("1E")->read();
		}
		else
		{
			if (!scsi_io_r(m_sasibus))
			{
				data = scsi_data_r(m_sasibus);

				if (!scsi_req_r(m_sasibus))
				{
					scsi_ack_w(m_sasibus, 0);
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

		if (scsi_io_r(m_sasibus))
		{
			scsi_data_w(m_sasibus, m_data);

			if (!scsi_req_r(m_sasibus))
			{
				scsi_ack_w(m_sasibus, 0);
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
		scsi_sel_w(m_sasibus, 0);
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

		scsi_rst_w(m_sasibus, 0);
		scsi_rst_w(m_sasibus, 1);
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
