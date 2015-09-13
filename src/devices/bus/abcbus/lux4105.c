// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 4105 SASI hard disk controller emulation

*********************************************************************/

#include "lux4105.h"
#include "bus/scsi/scsihd.h"
#include "bus/scsi/s1410.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define SASIBUS_TAG     "sasi"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type LUXOR_4105 = &device_creator<luxor_4105_device>;


WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_bsy )
{
	m_sasi_bsy = state;

	if (state)
	{
		m_sasibus->write_sel(0);
	}
}

WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_io )
{
	m_sasi_io = state;

	if (!m_sasi_io)
	{
		m_sasi_data_out->write(m_data);
	}

	update_trrq_int();
}

WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_req )
{
	m_sasi_req = state;

	if (m_sasi_req)
	{
		m_sasibus->write_ack(0);
	}

	update_trrq_int();
}

WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_cd )
{
	m_sasi_cd = state;
}


//-------------------------------------------------
//  MACHINE_DRIVER( luxor_4105 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( luxor_4105 )
	MCFG_DEVICE_ADD(SASIBUS_TAG, SCSI_PORT, 0)
	MCFG_SCSI_DATA_INPUT_BUFFER("sasi_data_in")
	MCFG_SCSI_BSY_HANDLER(WRITELINE(luxor_4105_device, write_sasi_bsy))
	MCFG_SCSI_REQ_HANDLER(WRITELINE(luxor_4105_device, write_sasi_req))
	MCFG_SCSI_CD_HANDLER(WRITELINE(luxor_4105_device, write_sasi_cd))
	MCFG_SCSI_IO_HANDLER(WRITELINE(luxor_4105_device, write_sasi_io))
	MCFG_SCSIDEV_ADD(SASIBUS_TAG ":" SCSI_PORT_DEVICE1, "harddisk", S1410, SCSI_ID_0)

	MCFG_SCSI_OUTPUT_LATCH_ADD("sasi_data_out", SASIBUS_TAG)
	MCFG_DEVICE_ADD("sasi_data_in", INPUT_BUFFER, 0)
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
	int cd = !m_sasi_cd;
	int req = !m_sasi_req;
	int trrq = !(cd & !req);

	if (BIT(m_dma, 5))
	{
		m_slot->irq_w(trrq ? CLEAR_LINE : ASSERT_LINE);
	}
	else
	{
		m_slot->irq_w(CLEAR_LINE);
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

luxor_4105_device::luxor_4105_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, LUXOR_4105, "Luxor 4105", tag, owner, clock, "lux4105", __FILE__),
	device_abcbus_card_interface(mconfig, *this),
	m_sasibus(*this, SASIBUS_TAG),
	m_sasi_data_out(*this, "sasi_data_out"),
	m_sasi_data_in(*this, "sasi_data_in"),
	m_1e(*this, "1E"),
	m_5e(*this, "5E"),
	m_cs(false),
	m_data(0),
	m_dma(0),
	m_sasi_bsy(0),
	m_sasi_req(0),
	m_sasi_cd(0),
	m_sasi_io(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void luxor_4105_device::device_start()
{
	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_data));
	save_item(NAME(m_dma));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void luxor_4105_device::device_reset()
{
	m_cs = false;
	m_data = 0;
	m_dma = 0;

	m_sasibus->write_rst(1);
	m_sasibus->write_rst(0);

	m_slot->trrq_w(1);
}


//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void luxor_4105_device::abcbus_cs(UINT8 data)
{
	m_cs = (data == m_5e->read());
}


//-------------------------------------------------
//  abcbus_csb -
//-------------------------------------------------

int luxor_4105_device::abcbus_csb()
{
	return !m_cs;
}


//-------------------------------------------------
//  abcbus_stat -
//-------------------------------------------------

UINT8 luxor_4105_device::abcbus_stat()
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

		data = !m_sasi_bsy;
		data |= !m_sasi_req << 2;
		data |= !m_sasi_cd << 3;
		data |= !m_sasi_io << 6;
	}

	return data;
}


//-------------------------------------------------
//  abcbus_inp -
//-------------------------------------------------

UINT8 luxor_4105_device::abcbus_inp()
{
	UINT8 data = 0xff;

	if (m_cs)
	{
		if (!m_sasi_bsy)
		{
			data = m_1e->read();
		}
		else
		{
			if (m_sasi_io)
			{
				data = m_sasi_data_in->read();

				if (m_sasi_req)
				{
					m_sasibus->write_ack(1);
				}
			}
		}
	}

	return data;
}


//-------------------------------------------------
//  abcbus_utp -
//-------------------------------------------------

void luxor_4105_device::abcbus_out(UINT8 data)
{
	if (m_cs)
	{
		m_data = data;

		if (!m_sasi_io)
		{
			m_sasi_data_out->write(m_data);

			if (m_sasi_req)
			{
				m_sasibus->write_ack(1);
			}
		}
	}
}


//-------------------------------------------------
//  abcbus_c1 -
//-------------------------------------------------

void luxor_4105_device::abcbus_c1(UINT8 data)
{
	if (m_cs)
	{
		m_sasibus->write_sel(1);
	}
}


//-------------------------------------------------
//  abcbus_c3 -
//-------------------------------------------------

void luxor_4105_device::abcbus_c3(UINT8 data)
{
	if (m_cs)
	{
		m_data = 0;
		m_dma = 0;

		m_sasibus->write_rst(1);
		m_sasibus->write_rst(0);
	}
}


//-------------------------------------------------
//  abcbus_c4 -
//-------------------------------------------------

void luxor_4105_device::abcbus_c4(UINT8 data)
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
