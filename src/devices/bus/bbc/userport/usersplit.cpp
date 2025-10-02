// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Watford Electronics User Port Splitter

**********************************************************************/

#include "emu.h"
#include "usersplit.h"


namespace {

class bbc_usersplit_device : public device_t, public device_bbc_userport_interface
{
public:
	bbc_usersplit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_USERSPLIT, tag, owner, clock)
		, device_bbc_userport_interface(mconfig, *this)
		, m_userport(*this, "userport%u", 1)
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(userport_changed) { m_selected = newval; }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t pb_r() override;
	virtual void pb_w(uint8_t data) override;
	virtual void write_cb1(int state) override;
	virtual void write_cb2(int state) override;

private:
	void cb1a_w(int state);
	void cb2a_w(int state);
	void cb1b_w(int state);
	void cb2b_w(int state);

	required_device_array<bbc_userport_slot_device, 2> m_userport;
	uint8_t m_selected;
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( usersplit )
	PORT_START("SELECT")
	PORT_CONFNAME(0x01, 0x00, "User Port") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bbc_usersplit_device::userport_changed), 0)
	PORT_CONFSETTING(0x00, "A")
	PORT_CONFSETTING(0x01, "B")
INPUT_PORTS_END

ioport_constructor bbc_usersplit_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( usersplit );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_usersplit_device::device_add_mconfig(machine_config &config)
{
	// user port A
	BBC_USERPORT_SLOT(config, m_userport[0], bbc_userport_devices, nullptr);
	m_userport[0]->cb1_handler().set(FUNC(bbc_usersplit_device::cb1a_w));
	m_userport[0]->cb2_handler().set(FUNC(bbc_usersplit_device::cb2a_w));

	// user port B
	BBC_USERPORT_SLOT(config, m_userport[1], bbc_userport_devices, nullptr);
	m_userport[1]->cb1_handler().set(FUNC(bbc_usersplit_device::cb1b_w));
	m_userport[1]->cb2_handler().set(FUNC(bbc_usersplit_device::cb2b_w));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_usersplit_device::device_start()
{
	m_selected = 0;

	save_item(NAME(m_selected));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_usersplit_device::pb_r()
{
	return m_userport[m_selected]->pb_r();
}

void bbc_usersplit_device::pb_w(uint8_t data)
{
	m_userport[m_selected]->pb_w(data);
}

void bbc_usersplit_device::write_cb1(int state)
{
	m_userport[m_selected]->write_cb1(state);
}

void bbc_usersplit_device::write_cb2(int state)
{
	m_userport[m_selected]->write_cb2(state);
}


void bbc_usersplit_device::cb1a_w(int state)
{
	if (m_selected == 0x00)
		m_slot->cb1_w(state);
}

void bbc_usersplit_device::cb2a_w(int state)
{
	if (m_selected == 0x00)
		m_slot->cb2_w(state);
}

void bbc_usersplit_device::cb1b_w(int state)
{
	if (m_selected == 0x01)
		m_slot->cb1_w(state);
}

void bbc_usersplit_device::cb2b_w(int state)
{
	if (m_selected == 0x01)
		m_slot->cb2_w(state);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_USERSPLIT, device_bbc_userport_interface, bbc_usersplit_device, "bbc_usersplit", "BBC Micro User Port Splitter")
