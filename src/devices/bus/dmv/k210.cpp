// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    K210 Centronics module

***************************************************************************/

#include "emu.h"
#include "k210.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/


static MACHINE_CONFIG_FRAGMENT( dmv_k210 )
	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(dmv_k210_device, porta_r))
	MCFG_I8255_IN_PORTB_CB(READ8(dmv_k210_device, portb_r))
	MCFG_I8255_IN_PORTC_CB(READ8(dmv_k210_device, portc_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(dmv_k210_device, porta_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(dmv_k210_device, portb_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(dmv_k210_device, portc_w))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_DATA_INPUT_BUFFER("cent_data_in")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(dmv_k210_device, cent_ack_w))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(dmv_k210_device, cent_busy_w))
	MCFG_CENTRONICS_SELECT_IN_HANDLER(WRITELINE(dmv_k210_device, cent_slct_w))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(dmv_k210_device, cent_pe_w))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(dmv_k210_device, cent_fault_w))
	MCFG_CENTRONICS_AUTOFD_HANDLER(WRITELINE(dmv_k210_device, cent_autofd_w))
	MCFG_CENTRONICS_INIT_HANDLER(WRITELINE(dmv_k210_device, cent_init_w))

	MCFG_DEVICE_ADD("cent_data_in", INPUT_BUFFER, 0)
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type DMV_K210 = &device_creator<dmv_k210_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_k210_device - constructor
//-------------------------------------------------

dmv_k210_device::dmv_k210_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, DMV_K210, "K210 Centronics", tag, owner, clock, "dmv_k210", __FILE__),
		device_dmvslot_interface( mconfig, *this ),
		m_ppi(*this, "ppi8255"),
		m_centronics(*this, "centronics"),
		m_cent_data_in(*this, "cent_data_in"),
		m_cent_data_out(*this, "cent_data_out"), m_bus(nullptr), m_clk1_timer(nullptr), m_portb(0), m_portc(0)
	{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_k210_device::device_start()
{
	m_clk1_timer = timer_alloc(0, nullptr);
	m_bus = static_cast<dmvcart_slot_device*>(owner());
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
//  device_timer - handler timer events
//-------------------------------------------------

void dmv_k210_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	m_centronics->write_strobe(CLEAR_LINE);
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor dmv_k210_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dmv_k210 );
}

void dmv_k210_device::io_read(address_space &space, int ifsel, offs_t offset, UINT8 &data)
{
	if (ifsel == 0)
		data = m_ppi->read(space, offset & 0x03);
}

void dmv_k210_device::io_write(address_space &space, int ifsel, offs_t offset, UINT8 data)
{
	if (ifsel == 0)
		m_ppi->write(space, offset & 0x03, data);
}

READ8_MEMBER( dmv_k210_device::porta_r )
{
	return m_cent_data_in->read();
}

READ8_MEMBER( dmv_k210_device::portb_r )
{
	return m_portb;
}

READ8_MEMBER( dmv_k210_device::portc_r )
{
	return m_portc;
}

WRITE8_MEMBER( dmv_k210_device::porta_w )
{
	m_cent_data_out->write(data);
}

WRITE8_MEMBER( dmv_k210_device::portb_w )
{
	m_centronics->write_ack(BIT(data, 2));
	m_centronics->write_select(BIT(data, 4));
	m_centronics->write_busy(BIT(data, 5));
	m_centronics->write_perror(BIT(data, 6));
	m_centronics->write_fault(BIT(data, 7));
}

WRITE8_MEMBER( dmv_k210_device::portc_w )
{
	if (!(data & 0x80))
	{
		m_centronics->write_strobe(ASSERT_LINE);
		m_clk1_timer->adjust(attotime::from_hz(XTAL_1MHz));
	}

	m_centronics->write_init(!BIT(data, 1));
	m_centronics->write_autofd(!BIT(data, 2));
	m_centronics->write_ack(BIT(data, 6));
	m_bus->m_out_irq_cb(BIT(data, 3));
}

WRITE_LINE_MEMBER( dmv_k210_device::cent_ack_w )     { if (state) m_portb |= 0x04; else m_portb &= ~0x04; m_ppi->pc6_w(state); }
WRITE_LINE_MEMBER( dmv_k210_device::cent_slct_w )    { if (state) m_portb |= 0x10; else m_portb &= ~0x10; }
WRITE_LINE_MEMBER( dmv_k210_device::cent_busy_w )    { if (state) m_portb |= 0x20; else m_portb &= ~0x20; }
WRITE_LINE_MEMBER( dmv_k210_device::cent_pe_w )      { if (state) m_portb |= 0x40; else m_portb &= ~0x40; }
WRITE_LINE_MEMBER( dmv_k210_device::cent_fault_w )   { if (state) m_portb |= 0x80; else m_portb &= ~0x80; }

WRITE_LINE_MEMBER( dmv_k210_device::cent_autofd_w )  { if (state) m_portc |= 0x02; else m_portc &= ~0x02; }
WRITE_LINE_MEMBER( dmv_k210_device::cent_init_w )    { if (state) m_portc |= 0x04; else m_portc &= ~0x04; }
