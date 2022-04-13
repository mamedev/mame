// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn SCSI Host Adaptor

    Known SCSI controller boards used:
      Adaptec ACB-4000A

    Known SCSI devices using adaptor:
      SCSI ID 0 - Acorn Winchester 110: Seagate ST412 + Adaptec ACB-4000A
                  Acorn Winchester 120: NEC D5126 + Adaptec ACB-4000A
                  Acorn Winchester 130: Rodime R0203E + Adaptec ACB-4000A
      SCSI ID 1 - Digistore Tape Streamer: Archive 2060S

    Compatible 3rd party Winchesters using Akhter Host Adaptor:
      Technomatic 20MB: Seagate ST225 + Adaptec ACB-4070
      Technomatic 30MB:
      Technomatic 40MB:

    Additional notes:
      The Digistore used a modified Acorn Host Adaptor, it re-mapped
      the device to &FC44-47, from &FC40-43. This allows both the
      Digistore and Winchester to co-exist.

    Useful CHD parameters:
      10MB -chs 306,4,33   -ss=256
      20MB -chs 612,4,33   -ss=256
      30MB -chs 640,6,33   -ss=256
     512MB -chs 3971,16,33 -ss 256

    TODO:
    - test formatting, may require specific ACB-4000A commands.

**********************************************************************/

#include "emu.h"
#include "scsi.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/devices.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_SCSI, bbc_scsi_device, "bbc_scsi", "Acorn SCSI Host Adaptor");
DEFINE_DEVICE_TYPE(BBC_AWHD, bbc_awhd_device, "bbc_awhd", "Acorn Winchester Disc");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_scsi_device::device_add_mconfig(machine_config& config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB)
		.machine_config([this](device_t* device) {
			downcast<nscsi_callback_device&>(*device).bsy_callback().set(*this, FUNC(bbc_scsi_device::bsy_w));
			downcast<nscsi_callback_device&>(*device).req_callback().set(*this, FUNC(bbc_scsi_device::req_w));
		});
}

void bbc_awhd_device::device_add_mconfig(machine_config &config)
{
	bbc_scsi_device::device_add_mconfig(config);

	/* Adaptec ACB-4000A */
	subdevice<nscsi_connector>("scsi:0")->set_default_option("harddisk");
	subdevice<nscsi_connector>("scsi:0")->set_fixed(true);

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, DERIVED_CLOCK(1, 1), bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(m_1mhzbus, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_1mhzbus->nmi_handler().set(m_1mhzbus, FUNC(bbc_1mhzbus_slot_device::nmi_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_scsi_device - constructor
//-------------------------------------------------

bbc_scsi_device::bbc_scsi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_scsi(*this, "scsi:7:scsicb")
{
}

bbc_scsi_device::bbc_scsi_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: bbc_scsi_device(mconfig, BBC_SCSI, tag, owner, clock)
{
}

bbc_awhd_device::bbc_awhd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_scsi_device(mconfig, BBC_AWHD, tag, owner, clock)
	, m_1mhzbus(*this, "1mhzbus")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_scsi_device::device_start()
{
	/* register for save states */
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_state));
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_scsi_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0x40:
		data = m_scsi->read();
		m_scsi->ack_w(1);
		break;
	case 0x41:
		data = (m_scsi->msg_r() << 0)
			| (m_scsi->bsy_r() << 1)
			| (m_irq_state << 4)
			| (m_scsi->req_r() << 5)
			| (m_scsi->io_r() << 6)
			| (m_scsi->cd_r() << 7);
		break;
	}

	return data;
}

void bbc_scsi_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x40:
		m_scsi->write(data);
		m_scsi->ack_w(1);
		break;
	case 0x42:
		m_scsi->sel_w(1);
		break;
	case 0x43:
		m_irq_enable = BIT(data, 0);
		break;
	}
}

WRITE_LINE_MEMBER(bbc_scsi_device::bsy_w)
{
	m_scsi->sel_w(0);
}

WRITE_LINE_MEMBER(bbc_scsi_device::req_w)
{
	m_scsi->ack_w(0);

	m_irq_state = (m_irq_enable && !state) ? 0 : 1;
	m_slot->irq_w(m_irq_state ? CLEAR_LINE : ASSERT_LINE);
}


uint8_t bbc_awhd_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	data &= bbc_scsi_device::fred_r(offset);
	data &= m_1mhzbus->fred_r(offset);

	return data;
}

void bbc_awhd_device::fred_w(offs_t offset, uint8_t data)
{
	bbc_scsi_device::fred_w(offset, data);
	m_1mhzbus->fred_w(offset, data);
}

uint8_t bbc_awhd_device::jim_r(offs_t offset)
{
	return m_1mhzbus->jim_r(offset);
}

void bbc_awhd_device::jim_w(offs_t offset, uint8_t data)
{
	m_1mhzbus->jim_w(offset, data);
}
