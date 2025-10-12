// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    BK serial port adapter (decimal 2.165.001)

    CSR 176560, vector base 360.

    Has software support in FOCAL ROM 084.

    To do: break detection, periodic timer outputs, DIP switch block

***************************************************************************/

#include "emu.h"
#include "bk_irps.h"

#include "bus/rs232/rs232.h"
#include "machine/dl11.h"
#include "machine/pdp11.h"


namespace {

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> bk_irps_device

class bk_irps_device : public device_t,
					public device_qbus_card_interface,
					public device_z80daisy_interface
{
public:
	// construction/destruction
	bk_irps_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override { return m_sart->z80daisy_irq_state(); }
	virtual int z80daisy_irq_ack() override { return m_sart->z80daisy_irq_ack(); }
	virtual void z80daisy_irq_reti() override { }

	required_device<k1801vp065_device> m_sart;
	required_device<rs232_port_device> m_rs232;
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bk_irps_device - constructor
//-------------------------------------------------

bk_irps_device::bk_irps_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BK_IRPS, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, device_z80daisy_interface(mconfig, *this)
	, m_sart(*this, "sart")
	, m_rs232(*this, "rs232")
{
}

void bk_irps_device::device_add_mconfig(machine_config &config)
{
	K1801VP065(config, m_sart, XTAL(4'608'000));
	m_sart->set_rxc(9600);
	m_sart->set_txc(9600);
	m_sart->set_rxvec(0360);
	m_sart->set_txvec(0364);
	m_sart->txd_wr_callback().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_sart->txrdy_wr_callback().set([this](int state) { m_bus->birq4_w(state); });
	m_sart->rxrdy_wr_callback().set([this](int state) { m_bus->birq4_w(state); });

	RS232_PORT(config, m_rs232, default_rs232_devices, "null_modem");
	m_rs232->rxd_handler().set(m_sart, FUNC(k1801vp065_device::rx_w));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bk_irps_device::device_start()
{
	m_bus->install_device(0176560, 0176567,
			emu::rw_delegate(m_sart, FUNC(k1801vp065_device::read)),
			emu::rw_delegate(m_sart, FUNC(k1801vp065_device::write)));
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(BK_IRPS, device_qbus_card_interface, bk_irps_device, "bk_irps", "BK serial port")
