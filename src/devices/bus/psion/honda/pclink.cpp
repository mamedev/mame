// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion PC Link cable

**********************************************************************/

#include "emu.h"
#include "pclink.h"

#include "bus/rs232/rs232.h"


namespace {

class psion_pclink_device : public device_t, public device_psion_honda_interface
{
public:
	psion_pclink_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, PSION_PCLINK, tag, owner, clock)
		, device_psion_honda_interface(mconfig, *this)
		, m_rs232(*this, "rs232")
	{
	}

protected:
	virtual void device_start() override ATTR_COLD { }

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
		m_rs232->rxd_handler().set(DEVICE_SELF_OWNER, FUNC(psion_honda_slot_device::write_rxd));
		m_rs232->dcd_handler().set(DEVICE_SELF_OWNER, FUNC(psion_honda_slot_device::write_dcd));
		m_rs232->dsr_handler().set(DEVICE_SELF_OWNER, FUNC(psion_honda_slot_device::write_dsr));
		m_rs232->cts_handler().set(DEVICE_SELF_OWNER, FUNC(psion_honda_slot_device::write_cts));
	}

	virtual void write_txd(int state) override { m_rs232->write_txd(state); }
	virtual void write_dtr(int state) override { m_rs232->write_dtr(state); }
	virtual void write_rts(int state) override { m_rs232->write_rts(state); }

private:
	required_device<rs232_port_device> m_rs232;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(PSION_PCLINK, device_psion_honda_interface, psion_pclink_device, "psion_pclink", "Psion PC Link cable")
