// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Votrax Type 'N Talk (Serial interface)

******************************************************************************/

#include "emu.h"
#include "votraxtnt.h"

#include "machine/votraxtnt.h"


namespace {

class serial_votraxtnt_device : public device_t, public device_rs232_port_interface
{
public:
	serial_votraxtnt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, SERIAL_VOTRAXTNT, tag, owner, clock)
		, device_rs232_port_interface(mconfig, *this)
		, m_tnt(*this, "tnt")
	{
	}

	virtual void input_txd(int state) override { m_tnt->write_rxd(state); }
	virtual void input_rts(int state) override { m_tnt->write_cts(state); }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<votraxtnt_device> m_tnt;
};


void serial_votraxtnt_device::device_add_mconfig(machine_config &config)
{
	VOTRAXTNT(config, m_tnt);
	m_tnt->txd_handler().set(FUNC(serial_votraxtnt_device::output_rxd));
	m_tnt->rts_handler().set(FUNC(serial_votraxtnt_device::output_cts));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(SERIAL_VOTRAXTNT, device_rs232_port_interface, serial_votraxtnt_device, "serial_votraxtnt", "Votrax Type 'N Talk (Serial Port)")
