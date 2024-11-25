// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    Iskra Delta SASI Adapter

**********************************************************************/

#include "emu.h"
#include "sasi.h"

#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"

namespace {

class idpartner_sasi_device :
	public device_t,
	public bus::idpartner::device_exp_card_interface
{
public:
	// construction/destruction
	idpartner_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	u8 stat_r();
	u8 data_r();
	void ctrl_w(u8 data);
	void data_w(u8 data);
	void reset_w(u8 data);

	void req_w(int state);
	void io_w(int state);

private:
	required_device<nscsi_bus_device> m_sasibus;
	required_device<nscsi_callback_device> m_sasi;

	int m_drq_enable;
	int m_data_enable;
	int m_prev_daq;
};


idpartner_sasi_device::idpartner_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IDPARTNER_SASI, tag, owner, clock)
	, bus::idpartner::device_exp_card_interface(mconfig, *this)
	, m_sasibus(*this, "sasibus")
	, m_sasi(*this, "sasibus:7:scsicb")
	, m_drq_enable(0)
	, m_data_enable(0)
	, m_prev_daq(0)
{
}

/*----------------------------------
  device_t implementation
----------------------------------*/

void idpartner_sasi_device::device_start()
{
	m_bus->io().install_read_handler (0x10, 0x10, emu::rw_delegate(*this, FUNC(idpartner_sasi_device::stat_r))); // RDSTAT
	m_bus->io().install_read_handler (0x11, 0x11, emu::rw_delegate(*this, FUNC(idpartner_sasi_device::data_r))); // RDDATA
	m_bus->io().install_write_handler(0x10, 0x10, emu::rw_delegate(*this, FUNC(idpartner_sasi_device::ctrl_w))); // WRCONTR
	m_bus->io().install_write_handler(0x11, 0x11, emu::rw_delegate(*this, FUNC(idpartner_sasi_device::data_w))); // WRDATA
	m_bus->io().install_write_handler(0x12, 0x12, emu::rw_delegate(*this, FUNC(idpartner_sasi_device::reset_w))); // RESET

	save_item(NAME(m_drq_enable));
	save_item(NAME(m_data_enable));
	save_item(NAME(m_prev_daq));
}

void idpartner_sasi_device::device_reset()
{
	m_drq_enable = 0;
	m_data_enable = 0;
	m_prev_daq = 0;
	m_bus->drq_w(0);
}

u8 idpartner_sasi_device::stat_r()
{
	u8 data = (m_sasi->req_r() << 7)
			| (m_sasi->io_r() << 6)
			| (m_sasi->msg_r() << 5)
			| (m_sasi->cd_r() << 4)
			| (m_sasi->bsy_r() << 3);
	return data;
}

u8 idpartner_sasi_device::data_r()
{
	u8 data = m_sasi->read();
	if (m_data_enable)
	{
		m_sasi->ack_w(1);
		if (m_drq_enable)
		{
			m_bus->drq_w(0);
			m_prev_daq = 0;
		}
	}
	return data;
}

void idpartner_sasi_device::ctrl_w(u8 data)
{
	m_sasi->write(BIT(data,0));
	m_sasi->sel_w(BIT(data,0));
	m_data_enable = BIT(data,1);
	m_drq_enable = BIT(data,5);
	if (m_data_enable && m_drq_enable)
	{
		m_bus->drq_w(m_sasi->req_r());
		m_prev_daq = m_sasi->req_r();
	} else {
		if (m_prev_daq)
			m_bus->drq_w(0);
		m_prev_daq = 0;
	}
}

void idpartner_sasi_device::data_w(u8 data)
{
	m_sasi->write(data);
	if (m_data_enable)
	{
		m_sasi->ack_w(1);
		if (m_drq_enable)
		{
			m_bus->drq_w(0);
			m_prev_daq = 0;
		}
	}
}

void idpartner_sasi_device::reset_w(u8 data)
{
	m_sasi->rst_w(1);
	m_sasi->rst_w(0);
}

void idpartner_sasi_device::req_w(int state)
{
	m_sasi->ack_w(0);
	if (m_data_enable && m_drq_enable)
	{
		m_bus->drq_w(state);
		m_prev_daq = state;
	}
}

void idpartner_sasi_device::io_w(int state)
{
	if (state)
		m_sasi->write(0); // clears lateched data
}

void idpartner_sasi_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_sasibus);
	NSCSI_CONNECTOR(config, "sasibus:0", default_scsi_devices, "s1410", true);
	NSCSI_CONNECTOR(config, "sasibus:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB)
		.machine_config(
				[this] (device_t* device)
				{
					downcast<nscsi_callback_device&>(*device).req_callback().set(*this, FUNC(idpartner_sasi_device::req_w));
					downcast<nscsi_callback_device&>(*device).io_callback().set(*this, FUNC(idpartner_sasi_device::io_w));
				});
	}
} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(IDPARTNER_SASI, bus::idpartner::device_exp_card_interface, idpartner_sasi_device, "partner_sai", "Iskra Delta Partner SASI card")
