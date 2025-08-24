// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Versatile Interface Board

    Part No. 200,009

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_UIB.html

**********************************************************************/

#include "emu.h"
#include "vib.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "machine/mc14411.h"


namespace {

class acorn_vib_device : public device_t, public device_acorn_bus_interface
{
public:
	acorn_vib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ACORN_VIB, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_ppi8255(*this, "ppi8255")
		, m_via6522(*this, "via6522")
		, m_acia(*this, "acia6850")
		, m_mc14411(*this, "mc14411")
		, m_centronics(*this, "centronics")
		, m_rs232(*this, "rs232")
		, m_irqs(*this, "irqs")
		, m_txc(*this, "TXC")
		, m_rxc(*this, "RXC")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<i8255_device> m_ppi8255;
	required_device<via6522_device> m_via6522;
	required_device<acia6850_device> m_acia;
	required_device<mc14411_device> m_mc14411;
	required_device<centronics_device> m_centronics;
	required_device<rs232_port_device> m_rs232;
	required_device<input_merger_device> m_irqs;
	required_ioport m_txc;
	required_ioport m_rxc;

	template<mc14411_device::timer_id T> void write_acia_clock(int state)
	{
		if (T == m_txc->read())
			m_acia->write_txc(state);
		if (T == m_rxc->read())
			m_acia->write_rxc(state);
	}

	void irq_w(int state)
	{
		m_bus->irq_w(state);
	}
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START(acorn_vib)
	PORT_START("TXC")
	PORT_CONFNAME(0x0f, mc14411_device::TIMER_F1, "Transmit Rate")
	PORT_CONFSETTING(mc14411_device::TIMER_F1,  "9600")
	PORT_CONFSETTING(mc14411_device::TIMER_F2,  "7200")
	PORT_CONFSETTING(mc14411_device::TIMER_F3,  "4800")
	PORT_CONFSETTING(mc14411_device::TIMER_F4,  "3600")
	PORT_CONFSETTING(mc14411_device::TIMER_F5,  "2400")
	PORT_CONFSETTING(mc14411_device::TIMER_F6,  "1800")
	PORT_CONFSETTING(mc14411_device::TIMER_F7,  "1200")
	PORT_CONFSETTING(mc14411_device::TIMER_F8,  "600")
	PORT_CONFSETTING(mc14411_device::TIMER_F9,  "300")
	PORT_CONFSETTING(mc14411_device::TIMER_F10, "200")
	PORT_CONFSETTING(mc14411_device::TIMER_F11, "150")
	PORT_CONFSETTING(mc14411_device::TIMER_F12, "134.5")
	PORT_CONFSETTING(mc14411_device::TIMER_F13, "110")
	PORT_CONFSETTING(mc14411_device::TIMER_F14, "75")

	PORT_START("RXC")
	PORT_CONFNAME(0x0f, mc14411_device::TIMER_F1, "Receive Rate")
	PORT_CONFSETTING(mc14411_device::TIMER_F1,  "9600")
	PORT_CONFSETTING(mc14411_device::TIMER_F2,  "7200")
	PORT_CONFSETTING(mc14411_device::TIMER_F3,  "4800")
	PORT_CONFSETTING(mc14411_device::TIMER_F4,  "3600")
	PORT_CONFSETTING(mc14411_device::TIMER_F5,  "2400")
	PORT_CONFSETTING(mc14411_device::TIMER_F6,  "1800")
	PORT_CONFSETTING(mc14411_device::TIMER_F7,  "1200")
	PORT_CONFSETTING(mc14411_device::TIMER_F8,  "600")
	PORT_CONFSETTING(mc14411_device::TIMER_F9,  "300")
	PORT_CONFSETTING(mc14411_device::TIMER_F10, "200")
	PORT_CONFSETTING(mc14411_device::TIMER_F11, "150")
	PORT_CONFSETTING(mc14411_device::TIMER_F12, "134.5")
	PORT_CONFSETTING(mc14411_device::TIMER_F13, "110")
	PORT_CONFSETTING(mc14411_device::TIMER_F14, "75")
INPUT_PORTS_END

ioport_constructor acorn_vib_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(acorn_vib);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_vib_device::device_start()
{
	m_acia->write_dcd(0);
	m_mc14411->rsa_w(1);
	m_mc14411->rsb_w(1);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_vib_device::device_reset()
{
	address_space &space = m_bus->memspace();
	uint16_t m_blk0 = m_bus->blk0() << 12;

	space.install_readwrite_handler(m_blk0 | 0x0c00, m_blk0 | 0x0c0f, 0, 0x10, 0, emu::rw_delegate(*m_via6522, FUNC(via6522_device::read)), emu::rw_delegate(*m_via6522, FUNC(via6522_device::write)));
	space.install_readwrite_handler(m_blk0 | 0x0c20, m_blk0 | 0x0c21, 0, 0x1e, 0, emu::rw_delegate(*m_acia, FUNC(acia6850_device::read)), emu::rw_delegate(*m_acia, FUNC(acia6850_device::write)));
	space.install_readwrite_handler(m_blk0 | 0x0c40, m_blk0 | 0x0c43, 0, 0x1c, 0, emu::rw_delegate(*m_ppi8255, FUNC(i8255_device::read)), emu::rw_delegate(*m_ppi8255, FUNC(i8255_device::write)));

	m_mc14411->timer_disable_all();
	m_mc14411->timer_enable(mc14411_device::timer_id(m_txc->read()), true);
	m_mc14411->timer_enable(mc14411_device::timer_id(m_rxc->read()), true);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void acorn_vib_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set(FUNC(acorn_vib_device::irq_w));

	MOS6522(config, m_via6522, 1'000'000); // TODO: derive clock from bus (pin 29 = ϕ2)
	m_via6522->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_via6522->ca2_handler().set(m_centronics, FUNC(centronics_device::write_strobe));
	m_via6522->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_via6522, FUNC(via6522_device::write_ca1));
	m_centronics->busy_handler().set(m_via6522, FUNC(via6522_device::write_pa7));
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	I8255(config, m_ppi8255);

	ACIA6850(config, m_acia);
	m_acia->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	m_rs232->cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));

	MC14411(config, m_mc14411, 1.8432_MHz_XTAL);
	m_mc14411->out_f<1>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F1>));
	m_mc14411->out_f<2>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F2>));
	m_mc14411->out_f<3>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F3>));
	m_mc14411->out_f<4>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F4>));
	m_mc14411->out_f<5>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F5>));
	m_mc14411->out_f<6>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F6>));
	m_mc14411->out_f<7>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F7>));
	m_mc14411->out_f<8>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F8>));
	m_mc14411->out_f<9>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F9>));
	m_mc14411->out_f<10>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F10>));
	m_mc14411->out_f<11>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F11>));
	m_mc14411->out_f<12>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F12>));
	m_mc14411->out_f<13>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F13>));
	m_mc14411->out_f<14>().set(FUNC(acorn_vib_device::write_acia_clock<mc14411_device::TIMER_F14>));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ACORN_VIB, device_acorn_bus_interface, acorn_vib_device, "acorn_vib", "Acorn Versatile Interface Board")
