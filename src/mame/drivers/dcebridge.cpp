// license:BSD-3-Clause
// copyright-holders:Vas Crabb
// Simple RS-232 DCE-DCE bridge
#include "emu.h"

#include "bus/rs232/rs232.h"

#include "dcebridge.lh"


namespace {

enum : ioport_value
{
	DTR_SOURCE_MASK         = 0x07,
	DTR_SOURCE_DCD_REMOTE   = 0x00,
	DTR_SOURCE_DSR_REMOTE   = 0x01,
	DTR_SOURCE_CTS_REMOTE   = 0x02,
	DTR_SOURCE_DCD_LOCAL    = 0x03,
	DTR_SOURCE_DSR_LOCAL    = 0x04,
	DTR_SOURCE_CTS_LOCAL    = 0x05,
	DTR_SOURCE_ASSERT       = 0x06,
	DTR_SOURCE_DEASSERT     = 0x07,

	RTS_SOURCE_MASK         = 0x38,
	RTS_SOURCE_DCD_REMOTE   = 0x00,
	RTS_SOURCE_DSR_REMOTE   = 0x08,
	RTS_SOURCE_CTS_REMOTE   = 0x10,
	RTS_SOURCE_DCD_LOCAL    = 0x18,
	RTS_SOURCE_DSR_LOCAL    = 0x20,
	RTS_SOURCE_CTS_LOCAL    = 0x28,
	RTS_SOURCE_ASSERT       = 0x30,
	RTS_SOURCE_DEASSERT     = 0x38,
};


class dcebridge_state : public driver_device
{
public:
	dcebridge_state(machine_config const &mconfig, device_type &type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_ports(*this, "%c", 'a')
		, m_conf(*this, "CONF_%c", 'A')
	{
	}

	template <unsigned N> DECLARE_INPUT_CHANGED_MEMBER(dtr_source);
	template <unsigned N> DECLARE_INPUT_CHANGED_MEMBER(rts_source);

	void dcebridge(machine_config &config);

protected:
	virtual void driver_start() override;
	virtual void driver_reset() override;

private:
	template <unsigned N> void dcd(int state);
	template <unsigned N> void dsr(int state);
	template <unsigned N> void cts(int state);

	template <unsigned N> void update_dtr();
	template <unsigned N> void update_rts();

	required_device_array<rs232_port_device, 2> m_ports;
	required_ioport_array<2> m_conf;

	u8 m_dcd[2] = { 0U, 0U };
	u8 m_dsr[2] = { 0U, 0U };
	u8 m_cts[2] = { 0U, 0U };
};


template <unsigned N> INPUT_CHANGED_MEMBER(dcebridge_state::dtr_source)
{
	update_dtr<N>();
}


template <unsigned N> INPUT_CHANGED_MEMBER(dcebridge_state::rts_source)
{
	update_rts<N>();
}


void dcebridge_state::dcebridge(machine_config &config)
{
	RS232_PORT(config, m_ports[0], default_rs232_devices, nullptr);
	m_ports[0]->rxd_handler().set(m_ports[1], FUNC(rs232_port_device::write_txd));
	m_ports[0]->dcd_handler().set(FUNC(dcebridge_state::dcd<0>));
	m_ports[0]->dcd_handler().append_output("dcd_a");
	m_ports[0]->dsr_handler().set(FUNC(dcebridge_state::dsr<0>));
	m_ports[0]->dsr_handler().append_output("dsr_a");
	m_ports[0]->ri_handler().set_output("ri_a");
	m_ports[0]->cts_handler().set(FUNC(dcebridge_state::cts<0>));
	m_ports[0]->cts_handler().append_output("cts_a");

	RS232_PORT(config, m_ports[1], default_rs232_devices, "null_modem");
	m_ports[1]->rxd_handler().set(m_ports[0], FUNC(rs232_port_device::write_txd));
	m_ports[1]->dcd_handler().set(FUNC(dcebridge_state::dcd<1>));
	m_ports[1]->dcd_handler().append_output("dcd_b");
	m_ports[1]->dsr_handler().set(FUNC(dcebridge_state::dsr<1>));
	m_ports[1]->dsr_handler().append_output("dsr_b");
	m_ports[1]->ri_handler().set_output("ri_b");
	m_ports[1]->cts_handler().set(FUNC(dcebridge_state::cts<1>));
	m_ports[1]->cts_handler().append_output("cts_b");

	config.set_default_layout(layout_dcebridge);
}


void dcebridge_state::driver_start()
{
	save_item(NAME(m_dcd));
	save_item(NAME(m_dsr));
	save_item(NAME(m_cts));
}


void dcebridge_state::driver_reset()
{
	update_dtr<0>();
	update_rts<0>();
	update_dtr<1>();
	update_rts<1>();
}


template <unsigned N> void dcebridge_state::dcd(int state)
{
	if (m_dcd[N] != state)
	{
		m_dcd[N] = state;
		if (started())
		{
			ioport_value const conf_local(m_conf[N]->read());
			if ((conf_local & DTR_SOURCE_MASK) == DTR_SOURCE_DCD_LOCAL)
				m_ports[N]->write_dtr(state);
			if ((conf_local & RTS_SOURCE_MASK) == RTS_SOURCE_DCD_LOCAL)
				m_ports[N]->write_rts(state);

			ioport_value const conf_remote(m_conf[N ^ 1U]->read());
			if ((conf_remote & DTR_SOURCE_MASK) == DTR_SOURCE_DCD_REMOTE)
				m_ports[N ^ 1U]->write_dtr(state);
			if ((conf_remote & RTS_SOURCE_MASK) == RTS_SOURCE_DCD_REMOTE)
				m_ports[N ^ 1U]->write_rts(state);
		}
	}
}


template <unsigned N> void dcebridge_state::dsr(int state)
{
	if (m_dsr[N] != state)
	{
		m_dsr[N] = state;
		if (started())
		{
			ioport_value const conf_local(m_conf[N]->read());
			if ((conf_local & DTR_SOURCE_MASK) == DTR_SOURCE_DSR_LOCAL)
				m_ports[N]->write_dtr(state);
			if ((conf_local & RTS_SOURCE_MASK) == RTS_SOURCE_DSR_LOCAL)
				m_ports[N]->write_rts(state);

			ioport_value const conf_remote(m_conf[N ^ 1U]->read());
			if ((conf_remote & DTR_SOURCE_MASK) == DTR_SOURCE_DSR_REMOTE)
				m_ports[N ^ 1U]->write_dtr(state);
			if ((conf_remote & RTS_SOURCE_MASK) == RTS_SOURCE_DSR_REMOTE)
				m_ports[N ^ 1U]->write_rts(state);
		}
	}
}


template <unsigned N> void dcebridge_state::cts(int state)
{
	if (m_cts[N] != state)
	{
		m_cts[N] = state;
		if (started())
		{
			ioport_value const conf_local(m_conf[N]->read());
			if ((conf_local & DTR_SOURCE_MASK) == DTR_SOURCE_CTS_LOCAL)
				m_ports[N]->write_dtr(state);
			if ((conf_local & RTS_SOURCE_MASK) == RTS_SOURCE_CTS_LOCAL)
				m_ports[N]->write_rts(state);

			ioport_value const conf_remote(m_conf[N ^ 1U]->read());
			if ((conf_remote & DTR_SOURCE_MASK) == DTR_SOURCE_CTS_REMOTE)
				m_ports[N ^ 1U]->write_dtr(state);
			if ((conf_remote & RTS_SOURCE_MASK) == RTS_SOURCE_CTS_REMOTE)
				m_ports[N ^ 1U]->write_rts(state);
		}
	}
}


template <unsigned N> void dcebridge_state::update_dtr()
{
	ioport_value const conf(m_conf[N]->read());
	switch (conf & DTR_SOURCE_MASK)
	{
	case DTR_SOURCE_DCD_REMOTE:
		m_ports[N]->write_dtr(m_dcd[N ^ 1U]);
		break;
	case DTR_SOURCE_DSR_REMOTE:
		m_ports[N]->write_dtr(m_dsr[N ^ 1U]);
		break;
	case DTR_SOURCE_CTS_REMOTE:
		m_ports[N]->write_dtr(m_cts[N ^ 1U]);
		break;
	case DTR_SOURCE_DCD_LOCAL:
		m_ports[N]->write_dtr(m_dcd[N]);
		break;
	case DTR_SOURCE_DSR_LOCAL:
		m_ports[N]->write_dtr(m_dsr[N]);
		break;
	case DTR_SOURCE_CTS_LOCAL:
		m_ports[N]->write_dtr(m_cts[N]);
		break;
	case DTR_SOURCE_ASSERT:
		m_ports[N]->write_dtr(0);
		break;
	case DTR_SOURCE_DEASSERT:
		m_ports[N]->write_dtr(1);
		break;
	}
}


template <unsigned N> void dcebridge_state::update_rts()
{
	ioport_value const conf(m_conf[N]->read());
	switch (conf & RTS_SOURCE_MASK)
	{
	case RTS_SOURCE_DCD_REMOTE:
		m_ports[N]->write_rts(m_dcd[N ^ 1U]);
		break;
	case RTS_SOURCE_DSR_REMOTE:
		m_ports[N]->write_rts(m_dsr[N ^ 1U]);
		break;
	case RTS_SOURCE_CTS_REMOTE:
		m_ports[N]->write_rts(m_cts[N ^ 1U]);
		break;
	case RTS_SOURCE_DCD_LOCAL:
		m_ports[N]->write_rts(m_dcd[N]);
		break;
	case RTS_SOURCE_DSR_LOCAL:
		m_ports[N]->write_rts(m_dsr[N]);
		break;
	case RTS_SOURCE_CTS_LOCAL:
		m_ports[N]->write_rts(m_cts[N]);
		break;
	case RTS_SOURCE_ASSERT:
		m_ports[N]->write_rts(0);
		break;
	case RTS_SOURCE_DEASSERT:
		m_ports[N]->write_rts(1);
		break;
	}
}


INPUT_PORTS_START(dcebridge)
	PORT_START("CONF_A")
	PORT_CONFNAME(DTR_SOURCE_MASK, DTR_SOURCE_DSR_REMOTE, "DTR A Source") PORT_CHANGED_MEMBER(DEVICE_SELF, dcebridge_state, dtr_source<0>, 0)
	PORT_CONFSETTING(DTR_SOURCE_DCD_REMOTE, "DCD B")
	PORT_CONFSETTING(DTR_SOURCE_DSR_REMOTE, "DSR B")
	PORT_CONFSETTING(DTR_SOURCE_CTS_REMOTE, "CTS B")
	PORT_CONFSETTING(DTR_SOURCE_DCD_LOCAL,  "DCD A Loopback")
	PORT_CONFSETTING(DTR_SOURCE_DSR_LOCAL,  "DSR A Loopback")
	PORT_CONFSETTING(DTR_SOURCE_CTS_LOCAL,  "CTS A Loopback")
	PORT_CONFSETTING(DTR_SOURCE_ASSERT,     "Asserted")
	PORT_CONFSETTING(DTR_SOURCE_DEASSERT,   "Deasserted")
	PORT_CONFNAME(RTS_SOURCE_MASK, RTS_SOURCE_CTS_REMOTE, "RTS A Source") PORT_CHANGED_MEMBER(DEVICE_SELF, dcebridge_state, rts_source<0>, 0)
	PORT_CONFSETTING(RTS_SOURCE_DCD_REMOTE, "DCD B")
	PORT_CONFSETTING(RTS_SOURCE_DSR_REMOTE, "DSR B")
	PORT_CONFSETTING(RTS_SOURCE_CTS_REMOTE, "CTS B")
	PORT_CONFSETTING(RTS_SOURCE_DCD_LOCAL,  "DCD A Loopback")
	PORT_CONFSETTING(RTS_SOURCE_DSR_LOCAL,  "DSR A Loopback")
	PORT_CONFSETTING(RTS_SOURCE_CTS_LOCAL,  "CTS A Loopback")
	PORT_CONFSETTING(RTS_SOURCE_ASSERT,     "Asserted")
	PORT_CONFSETTING(RTS_SOURCE_DEASSERT,   "Deasserted")

	PORT_START("CONF_B")
	PORT_CONFNAME(DTR_SOURCE_MASK, DTR_SOURCE_DSR_REMOTE, "DTR B Source") PORT_CHANGED_MEMBER(DEVICE_SELF, dcebridge_state, dtr_source<1>, 0)
	PORT_CONFSETTING(DTR_SOURCE_DCD_REMOTE, "DCD A")
	PORT_CONFSETTING(DTR_SOURCE_DSR_REMOTE, "DSR A")
	PORT_CONFSETTING(DTR_SOURCE_CTS_REMOTE, "CTS A")
	PORT_CONFSETTING(DTR_SOURCE_DCD_LOCAL,  "DCD B Loopback")
	PORT_CONFSETTING(DTR_SOURCE_DSR_LOCAL,  "DSR B Loopback")
	PORT_CONFSETTING(DTR_SOURCE_CTS_LOCAL,  "CTS B Loopback")
	PORT_CONFSETTING(DTR_SOURCE_ASSERT,     "Asserted")
	PORT_CONFSETTING(DTR_SOURCE_DEASSERT,   "Deasserted")
	PORT_CONFNAME(RTS_SOURCE_MASK, RTS_SOURCE_CTS_REMOTE, "RTS B Source") PORT_CHANGED_MEMBER(DEVICE_SELF, dcebridge_state, rts_source<1>, 0)
	PORT_CONFSETTING(RTS_SOURCE_DCD_REMOTE, "DCD A")
	PORT_CONFSETTING(RTS_SOURCE_DSR_REMOTE, "DSR A")
	PORT_CONFSETTING(RTS_SOURCE_CTS_REMOTE, "CTS A")
	PORT_CONFSETTING(RTS_SOURCE_DCD_LOCAL,  "DCD B Loopback")
	PORT_CONFSETTING(RTS_SOURCE_DSR_LOCAL,  "DSR B Loopback")
	PORT_CONFSETTING(RTS_SOURCE_CTS_LOCAL,  "CTS B Loopback")
	PORT_CONFSETTING(RTS_SOURCE_ASSERT,     "Asserted")
	PORT_CONFSETTING(RTS_SOURCE_DEASSERT,   "Deasserted")
INPUT_PORTS_END


ROM_START(dcebridge)
ROM_END

} // anonymous namespace

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY    FULLNAME                 FLAGS
SYST( 197?, dcebridge, 0,      0,      dcebridge, dcebridge, dcebridge_state, empty_init, "generic", "RS-232 DCE-DCE Bridge", MACHINE_NO_SOUND_HW )
