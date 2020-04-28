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

/*

    TODO

    - command execution gets stuck in message in phase

        08A88   move.l  D7,D0               D0=00000025
        08A8A   lsl.l   #5, D0              D0=000004A0
        08A8C   ori.l   #$7e000, D0         D0=0007E4A0
        08A92   movea.l D0, A0              A0=0007E4A0
        08A94   move.b  ($2,A0), D0         D0=0007E4AF
        08A98   btst    #$2, D0
        08A9C   beq     $8aa4
        08AA0   bra     $8a88

        [:bus2:4105:sasi:0:s1410] state=3.1 change
        [:bus2:4105:sasi:0:s1410] state=3.0 change
        [:bus2:4105:sasi] ctrl .....B.CI stat 0005
        [:bus2:4105:sasi] 0=ICB
        [:bus2:4105:sasi] ctrl .....B.CI stat 0005
        [:bus2:4105:sasi] 0=ICB
        [:bus2:4105:sasi] ctrl .....B.CI stat 0000
        [:bus2:4105:sasi] 0=ICB
        [:bus2:4105:sasi] ctrl ...Q.B.CI stat 0000
        [:bus2:4105:sasi] 0=QICB
        [:bus2:4105:sasi] ctrl ...Q.B.CI stat 0000
        [:bus2:4105:sasi] 0=QICB
        [:bus2:4105:sasi:0:s1410] state=3.4 change
        [:bus2:4105:sasi] ctrl ...Q.B.CI stat 0000
        [:bus2:4105:sasi] 0=QICB
        [:] ':3f' (089A8) STAT 25: 45
        [:bus2:4105:sasi] ctrl ..KQ.B.CI stat 0000
        [:bus2:4105:sasi] 0=QICB
        [:bus2:4105:sasi] 1=K
        [:bus2:4105:sasi:0:s1410] state=3.4 change
        [:bus2:4105:sasi] ctrl ..K..B.CI stat 0000
        [:bus2:4105:sasi] 0=ICB
        [:bus2:4105:sasi] 1=K
        [:bus2:4105:sasi] ctrl .....B.CI stat 0000
        [:bus2:4105:sasi] 0=ICB
        [:bus2:4105:sasi:0:s1410] state=3.3 change
        [:bus2:4105:sasi:0:s1410] state=3.0 change
        [:bus2:4105:sasi] ctrl .....BMCI min  0000
        [:bus2:4105:sasi] 0=MICB
        [:bus2:4105:sasi] ctrl .....BMCI min  0000
        [:bus2:4105:sasi] 0=MICB
        [:bus2:4105:sasi] ctrl .....BMCI min  0000
        [:bus2:4105:sasi] 0=MICB
        [:bus2:4105:sasi] ctrl ...Q.BMCI min  0000
        [:bus2:4105:sasi] 0=QMICB
        [:bus2:4105:sasi] ctrl ...Q.BMCI min  0000
        [:bus2:4105:sasi] 0=QMICB
        [:bus2:4105:sasi:0:s1410] state=3.4 change
        [:bus2:4105:sasi] ctrl ...Q.BMCI min  0000
        [:bus2:4105:sasi] 0=QMICB
        [:] ':3f' (089D4) INP 25: 00
        [:] ':3f' (089E8) STAT 25: 05

*/

#include "emu.h"
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
	m_dma(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void luxor_4105_device::device_start()
{
	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_dma));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void luxor_4105_device::device_reset()
{
	m_cs = false;
	m_dma = 0;

	m_sasi->rst_w(1);
	m_sasi->rst_w(0);

	m_slot->trrq_w(1);
}


void luxor_4105_device::update_trrq_int()
{
	bool cd = !m_sasi->cd_r();
	bool req = !m_sasi->req_r();
	int trrq = (cd & !req) ? 0 : 1;

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


WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_bsy )
{
	if (state)
	{
		m_sasi->sel_w(0);
	}
}


WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_cd )
{
	update_trrq_int();
}


WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_req )
{
	if (!state)
	{
		m_sasi->ack_w(0);
	}

	update_trrq_int();
}


WRITE_LINE_MEMBER( luxor_4105_device::write_sasi_io )
{
	if (state)
	{
		m_sasi->write(0);
	}
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

		    0       ?
		    1       ?
		    2       ?
		    3       ?
		    4       0
		    5
		    6       ? (tested at 014D9A, after command 08 sent and 1 byte read from SASI, should be 1)
		    7

		*/

		data = m_sasi->req_r();
		data |= !m_sasi->cd_r() << 1;
		data |= m_sasi->bsy_r() << 2;
		data |= !m_sasi->io_r() << 3;
		data |= !m_sasi->msg_r() << 6;
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
		if (!m_sasi->bsy_r())
		{
			data = m_1e->read();
		}
		else
		{
			data = m_sasi->read();

			m_sasi->ack_w(1);
		}
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
		m_sasi->write(data);

		m_sasi->ack_w(1);
	}
}


//-------------------------------------------------
//  abcbus_c1 -
//-------------------------------------------------

void luxor_4105_device::abcbus_c1(uint8_t data)
{
	if (m_cs)
	{
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
		m_dma = 0;

		m_sasi->sel_w(0);

		m_sasi->rst_w(1);
		m_sasi->rst_w(0);
	}
}


//-------------------------------------------------
//  abcbus_c4 -
//-------------------------------------------------

void luxor_4105_device::abcbus_c4(uint8_t data)
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
