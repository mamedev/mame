// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 4105 SASI hard disk controller emulation

*********************************************************************/

/*

PCB Layout
----------

DATABOARD 4105-10

|-------------------------------------------|
|-  LS367   LS74    LS32    LS00    LS38 LED|
||                                         -|
||  LS367   LS06    LS38            LS14   ||
|C                                         C|
|N  ALS08   LS08    81LS96                 N|
|1                                         2|
||  81LS95  74S373  LS273           74S240 ||
||                                         -|
|-  SW1     DM8131  LS175           SW2     |
|-------------------------------------------|

Notes:
    All IC's shown.

    DM8131  - National Semiconductor DM8131N 6-Bit Unified Bus Comparator
    LED     - Power LED
    SW1     - Drive settings
    SW2     - Card address
    CN1     - 2x32 PCB header, ABC 1600 bus
    CN2     - 2x25 PCB header, Xebec S1410

*/

#include "emu.h"
#include "lux4105.h"
#include "bus/scsi/scsihd.h"
#include "bus/scsi/s1410.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define SASIBUS_TAG     "sasi"
#define DMA_O1  BIT(m_dma, 0)
#define DMA_O2  BIT(m_dma, 1)
#define DMA_O3  BIT(m_dma, 2)


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(LUXOR_4105, luxor_4105_device, "lux4105", "Luxor 4105")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void luxor_4105_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "sasi");
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, "s1410");
	NSCSI_CONNECTOR(config, "sasi:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB)
		.machine_config([this](device_t* device) {
			downcast<nscsi_callback_device&>(*device).cd_callback().set(*this, FUNC(luxor_4105_device::write_sasi_cd));
			downcast<nscsi_callback_device&>(*device).bsy_callback().set(*this, FUNC(luxor_4105_device::write_sasi_bsy));
			downcast<nscsi_callback_device&>(*device).req_callback().set(*this, FUNC(luxor_4105_device::write_sasi_req));
			downcast<nscsi_callback_device&>(*device).msg_callback().set(*this, FUNC(luxor_4105_device::write_sasi_msg));
			downcast<nscsi_callback_device&>(*device).io_callback().set(*this, FUNC(luxor_4105_device::write_sasi_io));
		});
}


//-------------------------------------------------
//  INPUT_PORTS( luxor_4105 )
//-------------------------------------------------

INPUT_PORTS_START( luxor_4105 )
	PORT_START("1E")
	PORT_DIPNAME( 0x03, 0x03, "Stepping" ) PORT_DIPLOCATION("1E:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "Half (Seagate/Texas)" )
	PORT_DIPSETTING(    0x02, "Half (Tandon)" )
	PORT_DIPSETTING(    0x03, "Buffered" )
	PORT_DIPNAME( 0x0c, 0x0c, "Heads" ) PORT_DIPLOCATION("1E:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPSETTING(    0x0c, "8" )
	PORT_DIPNAME( 0xf0, 0x30, "Drive Type" ) PORT_DIPLOCATION("1E:5,6,7,8")
	PORT_DIPSETTING(    0x00, "Seagate ST506" )
	PORT_DIPSETTING(    0x10, "Rodime RO100" )
	PORT_DIPSETTING(    0x20, "Shugart SA600" )
	PORT_DIPSETTING(    0x30, "Seagate ST412" )

	PORT_START("5E")
	PORT_DIPNAME( 0x7f, 0x25, "Card Address" ) PORT_DIPLOCATION("5E:1,2,3,4,5,6,7")
	PORT_DIPSETTING(    0x25, "37" )
	PORT_DIPSETTING(    0x2d, "45" )

	PORT_START("S1")
	PORT_CONFNAME( 0x01, 0x01, "DMA Timing" )
	PORT_CONFSETTING(    0x00, "1043/1044" ) // a. TREN connected via delay circuit to OUT latch enable
	PORT_CONFSETTING(    0x01, "1045/1046" ) // b. TREN connected directly to OUT latch enable
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor luxor_4105_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( luxor_4105 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  luxor_4105_device - constructor
//-------------------------------------------------

luxor_4105_device::luxor_4105_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LUXOR_4105, tag, owner, clock),
	device_abcbus_card_interface(mconfig, *this),
	m_sasi(*this, "sasi:7:scsicb"),
	m_1e(*this, "1E"),
	m_5e(*this, "5E"),
	m_cs(false),
	m_dma(0),
	m_pren(1),
	m_prac(1),
	m_trrq(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void luxor_4105_device::device_start()
{
	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_data_out));
	save_item(NAME(m_dma));
	save_item(NAME(m_req));
	save_item(NAME(m_drq));
	save_item(NAME(m_pren));
	save_item(NAME(m_trrq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void luxor_4105_device::device_reset()
{
	m_cs = false;

	internal_reset();
}


void luxor_4105_device::internal_reset()
{
	write_dma_register(0);

	m_data_out = 0;

	m_sasi->sel_w(0);

	m_sasi->rst_w(1);
	m_sasi->rst_w(0);
}


void luxor_4105_device::update_dma()
{
	// TRRQ
	bool req = m_sasi->req_r() && !m_req;
	bool cd = m_sasi->cd_r();
	m_trrq = !(req && !cd);
	if (DMA_O2)
	{
		if (m_prac && !m_trrq)
		{
			// set REQ FF
			m_req = 1;

			update_ack();

			req = m_sasi->req_r() && !m_req;
			m_trrq = !(req && !cd);
		}
	}
	m_slot->trrq_w(DMA_O2 ? m_trrq : 1);

	// DRQ
	bool io = m_sasi->io_r();
	m_drq = (!((!cd || !io) && !(!cd && !DMA_O2))) && req;

	// IRQ
	bool irq = 1;
	if (DMA_O2)
	{
		if (DMA_O1 && m_drq)
		{
			irq = 0;
		}
	}
	else if (DMA_O3)
	{
		irq = 0;
	}
	m_slot->irq_w(!irq);
}


void luxor_4105_device::update_ack()
{
	m_sasi->ack_w(m_sasi->req_r() && (m_sasi->msg_r() || m_req));
}


void luxor_4105_device::write_dma_register(uint8_t data)
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

	m_dma = BIT(data, 5) << 2 | BIT(data, 6) << 1 | BIT(data, 7);
	if (LOG) logerror("%s DMA O1 %u O2 %u O3 %u\n", machine().describe_context(), DMA_O1, DMA_O2, DMA_O3);

	// PREN
	m_pren = !DMA_O2;
	m_slot->pren_w(m_pren);

	update_dma();
}


void luxor_4105_device::write_sasi_data(uint8_t data)
{
	m_data_out = data;

	if (!m_sasi->io_r())
	{
		m_sasi->write(data);
	}

	// clock REQ FF
	m_req = m_sasi->req_r();
	
	update_ack();
	update_dma();
}


WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_bsy )
{
	if (state)
	{
		m_sasi->sel_w(0);
	}
}


WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_cd )
{
	update_dma();
}


WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_req )
{
	if (LOG) logerror("%s REQ %u\n", machine().describe_context(), state);
	
	if (!state)
	{
		// reset REQ FF
		m_req = 0;
	}

	update_ack();
	update_dma();
}


WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_msg )
{
	update_ack();
}


WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_io )
{
	if (state)
	{
		m_sasi->write(0);
	}
	else
	{
		m_sasi->write(m_data_out);
	}

	update_dma();
}


//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void luxor_4105_device::abcbus_cs(uint8_t data)
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

uint8_t luxor_4105_device::abcbus_stat()
{
	uint8_t data = 0xff;

	if (m_cs)
	{
		/*

		    bit     description

		    0       REQ
		    1       C/D
		    2       BSY
		    3       I/O
		    4       0
		    5       DMA !O3
		    6       PREN
		    7       DMA request

		*/

		data = m_sasi->req_r() && !m_req;
		data |= !m_sasi->cd_r() << 1;
		data |= m_sasi->bsy_r() << 2;
		data |= !m_sasi->io_r() << 3;

		data |= !DMA_O3 << 5;
		data |= !m_pren << 6;
		data |= !m_drq << 7;
	}

	return data;
}


//-------------------------------------------------
//  abcbus_inp -
//-------------------------------------------------

uint8_t luxor_4105_device::abcbus_inp()
{
	uint8_t data = 0xff;

	if (m_cs)
	{
		if (m_sasi->bsy_r())
		{
			data = m_sasi->read();
		}
		else
		{
			data = m_1e->read();
		}

		// clock REQ FF
		m_req = m_sasi->req_r();

		update_ack();
		update_dma();

		if (LOG) logerror("%s INP %02x\n", machine().describe_context(), data);
	}

	return data;
}


//-------------------------------------------------
//  abcbus_utp -
//-------------------------------------------------

void luxor_4105_device::abcbus_out(uint8_t data)
{
	if (m_cs)
	{
		if (LOG) logerror("%s OUT %02x\n", machine().describe_context(), data);

		write_sasi_data(data);
	}
}


//-------------------------------------------------
//  abcbus_c1 -
//-------------------------------------------------

void luxor_4105_device::abcbus_c1(uint8_t data)
{
	if (m_cs)
	{
		if (LOG) logerror("%s SELECT\n", machine().describe_context());

		m_sasi->sel_w(1);
	}
}


//-------------------------------------------------
//  abcbus_c3 -
//-------------------------------------------------

void luxor_4105_device::abcbus_c3(uint8_t data)
{
	if (m_cs)
	{
		if (LOG) logerror("%s RESET\n", machine().describe_context());

		internal_reset();
	}
}


//-------------------------------------------------
//  abcbus_c4 -
//-------------------------------------------------

void luxor_4105_device::abcbus_c4(uint8_t data)
{
	if (m_cs)
	{
		if (LOG) logerror("%s DMA %02x\n", machine().describe_context(), data);

		write_dma_register(data);
	}
}


//-------------------------------------------------
//  abcbus_tren -
//-------------------------------------------------

uint8_t luxor_4105_device::abcbus_tren()
{
	uint8_t data = 0xff;

	if (DMA_O2)
	{
		if (m_sasi->bsy_r())
		{
			data = m_sasi->read();
		}

		// clock REQ FF
		m_req = m_sasi->req_r();

		update_ack();
		update_dma();

		if (LOG) logerror("%s TREN R %02x\n", machine().describe_context(), data);
	}

	return data;
}


//-------------------------------------------------
//  abcbus_tren -
//-------------------------------------------------

void luxor_4105_device::abcbus_tren(uint8_t data)
{
	if (DMA_O2)
	{
		if (LOG) logerror("%s TREN W %02x\n", machine().describe_context(), data);

		write_sasi_data(data);
	}
}


//-------------------------------------------------
//  abcbus_prac -
//-------------------------------------------------

void luxor_4105_device::abcbus_prac(int state)
{
	if (LOG) logerror("%s PRAC %u\n", machine().describe_context(), state);

	m_prac = state;
	
	update_dma();
}
