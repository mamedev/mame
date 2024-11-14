// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    SWTPC MP-C Serial Control Interface

**********************************************************************/

#include "emu.h"
#include "mpc.h"

#include "bus/rs232/rs232.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "machine/ripple_counter.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ss50_mpc_device

class ss50_mpc_device : public device_t, public ss50_card_interface
{
public:
	// construction/destruction
	ss50_mpc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SS50_MPC, tag, owner, clock)
		, ss50_card_interface(mconfig, *this)
		, m_pia(*this, "pia")
		, m_loopback(*this, "loopback")
		, m_counter(*this, "counter")
		, m_baud_jumper(*this, "BAUD")
		, m_count_select(false)
	{
	}

protected:
	// device-specific overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// interface-specific overrides
	virtual u8 register_read(offs_t offset) override;
	virtual void register_write(offs_t offset, u8 data) override;
	virtual void f110_w(int state) override;
	virtual void f300_w(int state) override;

private:
	void serial_input_w(int state);
	void reader_control_w(int state);
	int count_r();
	void count_select_w(int state);

	required_device<pia6821_device> m_pia;
	required_device<input_merger_device> m_loopback;
	required_device<ripple_counter_device> m_counter;
	required_ioport m_baud_jumper;

	bool m_count_select;
};


static INPUT_PORTS_START( mpc )
	PORT_START("BAUD")
	PORT_DIPNAME(1, 0, "Baud Rate")
	PORT_DIPSETTING(1, "110")
	PORT_DIPSETTING(0, "300")

	PORT_START("STOP")
	PORT_DIPNAME(1, 0, "Stop Bits")
	PORT_DIPSETTING(0, "1")
	PORT_DIPSETTING(1, "2")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ss50_mpc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mpc);
}


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_300)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_300)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void ss50_mpc_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia); // actually MC6820
	m_pia->writepa_handler().set("outgate", FUNC(input_merger_device::in_w<0>)).bit(0);
	m_pia->cb2_handler().set(FUNC(ss50_mpc_device::reader_control_w));
	m_pia->readpb_handler().set_ioport("STOP").mask(0x01).lshift(6);
	m_pia->readpb_handler().append(FUNC(ss50_mpc_device::count_r)).lshift(7);
	m_pia->writepb_handler().set(FUNC(ss50_mpc_device::count_select_w)).bit(2);
	m_pia->writepb_handler().append(m_counter, FUNC(ripple_counter_device::reset_w)).bit(0);
	//m_pia->irqa_handler().set(FUNC(ss50_mpc_device::pia_irq_w));
	//m_pia->irqb_handler().set(FUNC(ss50_mpc_device::pia_irq_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(FUNC(ss50_mpc_device::serial_input_w));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	INPUT_MERGER_ALL_HIGH(config, "outgate").output_handler().set("rs232", FUNC(rs232_port_device::write_txd));

	INPUT_MERGER_ANY_HIGH(config, m_loopback).output_handler().set("outgate", FUNC(input_merger_device::in_w<1>));

	RIPPLE_COUNTER(config, m_counter); // CD4024AE (IC3)
	m_counter->set_stages(7); // only Q5 (÷32) and Q4 (÷16) are actually used
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ss50_mpc_device::device_start()
{
	save_item(NAME(m_count_select));
}

void ss50_mpc_device::serial_input_w(int state)
{
	m_pia->write_porta_line(7, state);
	m_loopback->in_w<0>(state);
}


void ss50_mpc_device::reader_control_w(int state)
{
	m_loopback->in_w<1>(state);
}

int ss50_mpc_device::count_r()
{
	return BIT(m_counter->count(), m_count_select ? 4 : 3);
}

void ss50_mpc_device::count_select_w(int state)
{
	m_count_select = bool(state);
}


//-------------------------------------------------
//  register_read - read from a port register
//-------------------------------------------------

u8 ss50_mpc_device::register_read(offs_t offset)
{
	return m_pia->read(offset & 3);
}

//-------------------------------------------------
//  register_write - write to a port register
//-------------------------------------------------

void ss50_mpc_device::register_write(offs_t offset, u8 data)
{
	m_pia->write(offset & 3, data);
}

void ss50_mpc_device::f110_w(int state)
{
	if (m_baud_jumper->read())
		m_counter->clock_w(state);
}

void ss50_mpc_device::f300_w(int state)
{
	if (!m_baud_jumper->read())
		m_counter->clock_w(state);
}


// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(SS50_MPC, ss50_card_interface, ss50_mpc_device, "ss50_mpc", "MP-C Serial Control Interface")
