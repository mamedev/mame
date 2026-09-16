// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn AIV SCSI Host Adaptor

**********************************************************************/

#include "emu.h"
#include "scsiaiv.h"

#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"


namespace {

class bbc_scsiaiv_device : public device_t, public device_bbc_modem_interface
{
public:
	// construction/destruction
	bbc_scsiaiv_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock);

	void bsy_w(int state);
	void req_w(int state);

protected:
	bbc_scsiaiv_device(const machine_config &mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_bbc_modem_interface(mconfig, *this)
		, m_scsi(*this, "scsi:7:scsicb")
	{
	}

	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	required_device<nscsi_callback_device> m_scsi;

	int m_irq_enable;
	int m_irq_state;
};


// ======================> bbc_vp415_device

//class bbc_vp415_device : public bbc_scsiaiv_device
//{
//public:
//  // construction/destruction
//  bbc_vp415_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock);
//
//protected:
//  // optional information overrides
//  virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
//};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_scsiaiv_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB)
		.machine_config([this](device_t* device) {
			downcast<nscsi_callback_device&>(*device).bsy_callback().set(*this, FUNC(bbc_scsiaiv_device::bsy_w));
			downcast<nscsi_callback_device&>(*device).req_callback().set(*this, FUNC(bbc_scsiaiv_device::req_w));
		});
}

//void bbc_vp415_device::device_add_mconfig(machine_config &config)
//{
//  bbc_scsiaiv_device::device_add_mconfig(config);
//
//  // Philips VP415
//  subdevice<nscsi_connector>("scsi:0")->set_default_option("vp415");
//  subdevice<nscsi_connector>("scsi:0")->set_fixed(true);
//}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_scsiaiv_device - constructor
//-------------------------------------------------

bbc_scsiaiv_device::bbc_scsiaiv_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock)
	: bbc_scsiaiv_device(mconfig, BBC_SCSIAIV, tag, owner, clock)
{
}

//bbc_vp415_device::bbc_vp415_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock)
//  : bbc_scsiaiv_device(mconfig, BBC_VP415, tag, owner, clock)
//{
//}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_scsiaiv_device::device_start()
{
	// register for save states
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_state));
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_scsiaiv_device::read(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0x07)
	{
	case 0x00:
		data = m_scsi->read();
		m_scsi->ack_w(1);
		break;
	case 0x01:
		data = (m_scsi->msg_r() << 0)
			| (m_scsi->bsy_r() << 1)
			| (m_scsi->rst_r() << 2)
			| (!m_irq_state << 4)
			| (m_scsi->req_r() << 5)
			| (m_scsi->io_r() << 6)
			| (m_scsi->cd_r() << 7);
		break;
	}

	return data;
}

void bbc_scsiaiv_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x07)
	{
	case 0x00:
		m_scsi->write(~data);
		m_scsi->ack_w(1);
		break;
	case 0x02:
		m_scsi->sel_w(1);
		break;
	case 0x03:
		m_irq_enable = BIT(data, 0);
		break;
	}
}

void bbc_scsiaiv_device::bsy_w(int state)
{
	m_scsi->sel_w(0);
}

void bbc_scsiaiv_device::req_w(int state)
{
	m_scsi->ack_w(0);

	m_irq_state = (m_irq_enable && !state) ? 0 : 1;
	m_slot->irq_w(m_irq_state ? CLEAR_LINE : ASSERT_LINE);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_SCSIAIV, device_bbc_modem_interface, bbc_scsiaiv_device, "bbc_scsiaiv", "Acorn AIV SCSI Host Adaptor");
//DEFINE_DEVICE_TYPE_PRIVATE(BBC_VP415, device_bbc_modem_interface, bbc_vp415_device, "bbc_vp415", "BBC Philips VP415");
