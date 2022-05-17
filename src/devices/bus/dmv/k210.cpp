// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
/***************************************************************************

    K210 Centronics module

***************************************************************************/

#include "emu.h"
#include "k210.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DMV_K210, dmv_k210_device, "dmv_k210", "K210 Centronics")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_k210_device - constructor
//-------------------------------------------------

dmv_k210_device::dmv_k210_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DMV_K210, tag, owner, clock)
	, device_dmvslot_interface(mconfig, *this)
	, m_ppi(*this, "ppi8255")
	, m_centronics(*this, "centronics")
	, m_cent_data_in(*this, "cent_data_in")
	, m_cent_data_out(*this, "cent_data_out")
	, m_clk1_timer(nullptr), m_portb(0), m_portc(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_k210_device::device_start()
{
	m_clk1_timer = timer_alloc(FUNC(dmv_k210_device::strobe_tick), this);

	// register for state saving
	save_item(NAME(m_portb));
	save_item(NAME(m_portc));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dmv_k210_device::device_reset()
{
	m_clk1_timer->adjust(attotime::never);
	m_portb = 0x00;
	m_portc = 0x00;
}

//-------------------------------------------------
//  strobe_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(dmv_k210_device::strobe_tick)
{
	m_centronics->write_strobe(CLEAR_LINE);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dmv_k210_device::device_add_mconfig(machine_config &config)
{
	I8255(config, m_ppi, 0);
	m_ppi->in_pa_callback().set(FUNC(dmv_k210_device::porta_r));
	m_ppi->in_pb_callback().set(FUNC(dmv_k210_device::portb_r));
	m_ppi->in_pc_callback().set(FUNC(dmv_k210_device::portc_r));
	m_ppi->out_pa_callback().set(FUNC(dmv_k210_device::porta_w));
	m_ppi->out_pb_callback().set(FUNC(dmv_k210_device::portb_w));
	m_ppi->out_pc_callback().set(FUNC(dmv_k210_device::portc_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_data_input_buffer(m_cent_data_in);
	m_centronics->ack_handler().set(FUNC(dmv_k210_device::cent_ack_w));
	m_centronics->busy_handler().set(FUNC(dmv_k210_device::cent_busy_w));
	m_centronics->select_in_handler().set(FUNC(dmv_k210_device::cent_slct_w));
	m_centronics->perror_handler().set(FUNC(dmv_k210_device::cent_pe_w));
	m_centronics->fault_handler().set(FUNC(dmv_k210_device::cent_fault_w));
	m_centronics->autofd_handler().set(FUNC(dmv_k210_device::cent_autofd_w));
	m_centronics->init_handler().set(FUNC(dmv_k210_device::cent_init_w));

	INPUT_BUFFER(config, m_cent_data_in);

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);
}

void dmv_k210_device::io_read(int ifsel, offs_t offset, uint8_t &data)
{
	if (ifsel == 0)
		data = m_ppi->read(offset & 0x03);
}

void dmv_k210_device::io_write(int ifsel, offs_t offset, uint8_t data)
{
	if (ifsel == 0)
		m_ppi->write(offset & 0x03, data);
}

uint8_t dmv_k210_device::porta_r()
{
	return m_cent_data_in->read();
}

uint8_t dmv_k210_device::portb_r()
{
	return m_portb;
}

uint8_t dmv_k210_device::portc_r()
{
	return m_portc;
}

void dmv_k210_device::porta_w(uint8_t data)
{
	m_cent_data_out->write(data);
}

void dmv_k210_device::portb_w(uint8_t data)
{
	m_centronics->write_ack(BIT(data, 2));
	m_centronics->write_select(BIT(data, 4));
	m_centronics->write_busy(BIT(data, 5));
	m_centronics->write_perror(BIT(data, 6));
	m_centronics->write_fault(BIT(data, 7));
}

void dmv_k210_device::portc_w(uint8_t data)
{
	if (!(data & 0x80))
	{
		m_centronics->write_strobe(ASSERT_LINE);
		m_clk1_timer->adjust(attotime::from_hz(XTAL(1'000'000)));
	}

	m_centronics->write_init(!BIT(data, 1));
	m_centronics->write_autofd(!BIT(data, 2));
	m_centronics->write_ack(BIT(data, 6));
	out_irq(BIT(data, 3));
}

WRITE_LINE_MEMBER( dmv_k210_device::cent_ack_w )     { if (state) m_portb |= 0x04; else m_portb &= ~0x04; m_ppi->pc6_w(state); }
WRITE_LINE_MEMBER( dmv_k210_device::cent_slct_w )    { if (state) m_portb |= 0x10; else m_portb &= ~0x10; }
WRITE_LINE_MEMBER( dmv_k210_device::cent_busy_w )    { if (state) m_portb |= 0x20; else m_portb &= ~0x20; }
WRITE_LINE_MEMBER( dmv_k210_device::cent_pe_w )      { if (state) m_portb |= 0x40; else m_portb &= ~0x40; }
WRITE_LINE_MEMBER( dmv_k210_device::cent_fault_w )   { if (state) m_portb |= 0x80; else m_portb &= ~0x80; }

WRITE_LINE_MEMBER( dmv_k210_device::cent_autofd_w )  { if (state) m_portc |= 0x02; else m_portc &= ~0x02; }
WRITE_LINE_MEMBER( dmv_k210_device::cent_init_w )    { if (state) m_portc |= 0x04; else m_portc &= ~0x04; }
