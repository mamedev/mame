// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion Siena SSD Drive

**********************************************************************/

#include "emu.h"
#include "ssd.h"

#include "bus/psion/honda/slot.h"
#include "machine/psion_ssd.h"


namespace {

class psion_siena_ssd_device : public device_t, public device_psion_honda_interface
{
public:
	psion_siena_ssd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, PSION_SIENA_SSD, tag, owner, clock)
		, device_psion_honda_interface(mconfig, *this)
		, m_ssd(*this, "ssd")
		, m_honda(*this, "honda")
	{
	}

protected:
	virtual void device_start() override ATTR_COLD { }

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		PSION_SSD(config, m_ssd);
		m_ssd->door_cb().set(DEVICE_SELF_OWNER, FUNC(psion_honda_slot_device::write_sdoe));

		PSION_HONDA_SLOT(config, m_honda, psion_honda_devices, nullptr);
		m_honda->rxd_handler().set(DEVICE_SELF_OWNER, FUNC(psion_honda_slot_device::write_rxd));
		m_honda->dcd_handler().set(DEVICE_SELF_OWNER, FUNC(psion_honda_slot_device::write_dcd));
		m_honda->dsr_handler().set(DEVICE_SELF_OWNER, FUNC(psion_honda_slot_device::write_dsr));
		m_honda->cts_handler().set(DEVICE_SELF_OWNER, FUNC(psion_honda_slot_device::write_cts));
	}

	virtual uint8_t data_r() override { return m_ssd->data_r(); }
	virtual void data_w(uint16_t data) override { m_ssd->data_w(data); }

	virtual void write_txd(int state) override { m_honda->write_txd(state); }
	virtual void write_dtr(int state) override { m_honda->write_dtr(state); }
	virtual void write_rts(int state) override { m_honda->write_rts(state); }

private:
	required_device<psion_ssd_device> m_ssd;
	required_device<psion_honda_slot_device> m_honda;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(PSION_SIENA_SSD, device_psion_honda_interface, psion_siena_ssd_device, "psion_siena_ssd", "Psion Siena SSD Drive")
