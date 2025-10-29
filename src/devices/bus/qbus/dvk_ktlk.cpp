// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    6-port serial card, decimal 3.057.119

    CSR 176560, vector base 320 (factory default).

    CSR, vector base, frame format and port speed are configurable via
    DIP switches, this is not implemented.

***************************************************************************/

#include "emu.h"
#include "dvk_ktlk.h"

#include "bus/rs232/rs232.h"
#include "machine/dl11.h"


namespace {

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> dvk_ktlk_device

class dvk_ktlk_device : public device_t,
					public device_qbus_card_interface,
					public device_z80daisy_interface
{
public:
	// construction/destruction
	dvk_ktlk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override { }

private:
	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

	required_device_array<k1801vp065_device, 6> m_sart;
	required_device_array<rs232_port_device, 6> m_rs232;

	bool m_installed;
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dvk_ktlk_device - constructor
//-------------------------------------------------

dvk_ktlk_device::dvk_ktlk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DVK_KTLK, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, device_z80daisy_interface(mconfig, *this)
	, m_sart(*this, "sart%u", 1U)
	, m_rs232(*this, "port%u", 1U)
	, m_installed(false)
{
}

static DEVICE_INPUT_DEFAULTS_START( host_rs232_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
	DEVICE_INPUT_DEFAULTS( "FLOW_CONTROL", 0x01, 0x01 )
DEVICE_INPUT_DEFAULTS_END

void dvk_ktlk_device::device_add_mconfig(machine_config &config)
{
	for (int i = 0; i < 6; i++)
	{
		K1801VP065(config, m_sart[i], XTAL(4'608'000));
		m_sart[i]->set_rxc(9600);
		m_sart[i]->set_txc(9600);
		m_sart[i]->set_rxvec(0320 + (010 * i));
		m_sart[i]->set_txvec(0324 + (010 * i));
		m_sart[i]->txd_wr_callback().set(m_rs232[i], FUNC(rs232_port_device::write_txd));
		m_sart[i]->rts_wr_callback().set(m_rs232[i], FUNC(rs232_port_device::write_rts));
		m_sart[i]->txrdy_wr_callback().set([this](int state) { m_bus->birq4_w(state); });
		m_sart[i]->rxrdy_wr_callback().set([this](int state) { m_bus->birq4_w(state); });

		RS232_PORT(config, m_rs232[i], default_rs232_devices, "loopback");
		m_rs232[i]->rxd_handler().set(m_sart[i], FUNC(k1801vp065_device::rx_w));
		m_rs232[i]->set_option_device_input_defaults("loopback", DEVICE_INPUT_DEFAULTS_NAME(host_rs232_defaults));
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dvk_ktlk_device::device_start()
{
	save_item(NAME(m_installed));

	m_installed = false;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dvk_ktlk_device::device_reset()
{
	if (!m_installed)
	{
		m_bus->install_device(0176560, 0176637,
				read16sm_delegate(*this, FUNC(dvk_ktlk_device::read)),
				write16sm_delegate(*this, FUNC(dvk_ktlk_device::write)));
		m_installed = true;
	}
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t dvk_ktlk_device::read(offs_t offset)
{
	uint16_t data = m_sart[offset >> 2]->read(offset % 4);
	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void dvk_ktlk_device::write(offs_t offset, uint16_t data)
{
	m_sart[offset >> 2]->write(offset % 4, data);
}


int dvk_ktlk_device::z80daisy_irq_state()
{
	int irq = 0;
	for (int i = 0; i < 6; i++)
	{
		irq |= m_sart[i]->z80daisy_irq_state();
	}
	return irq;
}

int dvk_ktlk_device::z80daisy_irq_ack()
{
	int vec = -1;

	for (int i = 0; i < 6; i++)
	{
		vec = m_sart[i]->z80daisy_irq_ack();
		if (vec > -1) return vec;
	}

	return vec;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(DVK_KTLK, device_qbus_card_interface, dvk_ktlk_device, "dvk_ktlk", "DVK KTLK 6-port serial");
