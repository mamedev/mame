// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Versatile Interface Board

    Part No. 200,009

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_UIB.html

**********************************************************************/


#include "emu.h"
#include "vib.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ACORN_VIB, acorn_vib_device, "acorn_vib", "Acorn Versatile Interface Board")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void acorn_vib_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set(FUNC(acorn_vib_device::irq_w));

	VIA6522(config, m_via6522, XTAL(1'000'000));
	m_via6522->writepa_handler().set("cent_data_out", FUNC(output_latch_device::bus_w));
	m_via6522->ca2_handler().set(m_centronics, FUNC(centronics_device::write_strobe));
	m_via6522->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_via6522, FUNC(via6522_device::write_ca1));
	m_centronics->busy_handler().set(m_via6522, FUNC(via6522_device::write_pa7));
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	I8255(config, m_ppi8255, 0);

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	m_rs232->cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));
	m_rs232->dcd_handler().set(m_acia, FUNC(acia6850_device::write_dcd));

	CLOCK(config, m_acia_clock, 1.8432_MHz_XTAL);
	m_acia_clock->signal_handler().set(FUNC(acorn_vib_device::write_acia_clock));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  acorn_vib_device - constructor
//-------------------------------------------------

acorn_vib_device::acorn_vib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_VIB, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_ppi8255(*this, "ppi8255")
	, m_via6522(*this, "via6522")
	, m_acia(*this, "acia6850")
	, m_acia_clock(*this, "acia_clock")
	, m_centronics(*this, "centronics")
	, m_rs232(*this, "rs232")
	, m_irqs(*this, "irqs")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_vib_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_vib_device::device_reset()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0x0c00, 0x0c0f, 0, 0x10, 0, read8sm_delegate(FUNC(via6522_device::read), m_via6522.target()), write8sm_delegate(FUNC(via6522_device::write), m_via6522.target()));
	space.install_readwrite_handler(0x0c20, 0x0c21, 0, 0x1e, 0, read8sm_delegate(FUNC(acia6850_device::read), m_acia.target()), write8sm_delegate(FUNC(acia6850_device::write), m_acia.target()));
	space.install_readwrite_handler(0x0c40, 0x0c43, 0, 0x1c, 0, read8sm_delegate(FUNC(i8255_device::read), m_ppi8255.target()), write8sm_delegate(FUNC(i8255_device::write), m_ppi8255.target()));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER(acorn_vib_device::write_acia_clock)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

WRITE_LINE_MEMBER(acorn_vib_device::irq_w)
{
	m_bus->irq_w(state);
}
