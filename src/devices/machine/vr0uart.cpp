// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    MagicEyes VRender0 UART sub-device

    Device by Angelo Salese

    TODO:
    - The only current example (Trivia R Us touchscreen) expects to read
      stuff before transmitting anything, except for loopback test and a
      signal break enabling at POST (!?).

***************************************************************************/

#include "emu.h"
#include "vrender0.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(VRENDER0_UART, vr0uart_device, "vr0uart", "MagicEyes VRender0 UART")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vr0uart_device - constructor
//-------------------------------------------------

vr0uart_device::vr0uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VRENDER0_UART, tag, owner, clock),
	  device_serial_interface(mconfig, *this)
{
}

void vr0uart_device::regs_map(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(vr0uart_device::control_r), FUNC(vr0uart_device::control_w));
	map(0x04, 0x07).r(FUNC(vr0uart_device::status_r));
	map(0x08, 0x0b).w(FUNC(vr0uart_device::transmit_buffer_w));
	map(0x0c, 0x0f).r(FUNC(vr0uart_device::receive_buffer_r));
	map(0x10, 0x13).rw(FUNC(vr0uart_device::baud_rate_div_r), FUNC(vr0uart_device::baud_rate_div_w));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vr0uart_device::device_start()
{
	save_item(NAME(m_ucon));
	save_item(NAME(m_ubdr));
	save_item(NAME(m_ustat));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vr0uart_device::device_reset()
{
	m_ucon = 0x001;
	m_ubdr = 1;
	m_urxb_fifo.clear();

	update_serial_config();
}

inline void vr0uart_device::tx_send_byte(uint8_t val)
{
	transmit_register_setup(val);
	m_ustat |= 0x20;
}

inline uint32_t vr0uart_device::calculate_baud_rate()
{
	uint32_t div_rate = ((m_ubdr & 0xffff) + 1) * 16;
	// TODO: external / internal serial clock config
	return (this->clock() / 2) / div_rate;
}

void vr0uart_device::update_serial_config()
{
	const parity_t parity_modes[4] = { PARITY_NONE, PARITY_NONE, PARITY_EVEN, PARITY_ODD };

	uint8_t word_length = m_ucon & 1 ? 8 : 7;
	parity_t parity_mode = parity_modes[(m_ucon & 0xc) >> 2];
	stop_bits_t stop_bits = m_ucon & 2 ? STOP_BITS_2 : STOP_BITS_1;

	set_data_frame(1, word_length, parity_mode, stop_bits);

	if (m_ucon & 0x100) // UART Enable
	{
		uint32_t clock_rate = calculate_baud_rate();
		set_rcv_rate(clock_rate);
		set_tra_rate(clock_rate);
	}
	else
	{
		set_rcv_rate(0);
		set_tra_rate(0);
	}
}

void vr0uart_device::tra_callback()
{
	int bit = transmit_register_get_data_bit();
	m_ustat |= 0x40;
	m_parent->write_line_tx(m_channel_num, bit);
}

void vr0uart_device::tra_complete()
{
	m_ustat &= ~0x60;
	m_parent->IntReq(m_channel_num ? 18 : 15);
}

void vr0uart_device::rcv_complete()
{
	receive_register_extract();
	if (is_receive_parity_error())
		m_ustat |= 2;
	if (is_receive_framing_error())
		m_ustat |= 4;

	if (!m_urxb_fifo.full())
	{
		// TODO: break detection
		m_urxb_fifo.enqueue(get_received_char());
	}
	else
		m_ustat |= 1; // overrun

	if (m_ucon & 0x20 && m_ustat & 0xf)
		m_parent->IntReq(m_channel_num ? 16 : 13);
	else
		m_parent->IntReq(m_channel_num ? 17 : 14);
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

/*
 * ---x ---- ---- UART enable
 * ---- x--- ---- Loopback Test
 * ---- -x-- ---- Send Break mode
 * ---- --x- ---- Generate interrupt on break or error
 * ---- ---x ---- Serial Clock selection (1=external)
 * ---- ---- xx-- Parity Mode (0x=No Parity, 10=Even, 11=Odd)
 * ---- ---- --x- Stop Bits (1=2 bits, 0=1 Bit)
 * ---- ---- ---x Word Length (1=8 bits, 0=7 bits)
 */
READ32_MEMBER( vr0uart_device::control_r )
{
	return m_ucon;
}

WRITE32_MEMBER( vr0uart_device::control_w )
{
	COMBINE_DATA(&m_ucon);
	update_serial_config();
}

/*
 * xxxx ---- ---- Receive FIFO count
 * ---- -x-- ---- Tx buffer not empty, Tx holding data
 * ---- --x- ---- Tx buffer not empty
 * ---- ---x ---- Rx buffer not empty
 * ---- ---- x--- Break detect
 * ---- ---- -x-- Frame error
 * ---- ---- --x- Parity error
 * ---- ---- ---x Overrun Error
 */
READ32_MEMBER( vr0uart_device::status_r )
{
	uint32_t res = m_ustat;
	if (!m_urxb_fifo.empty())
	{
		res |= 0x10;
		res |= (m_urxb_fifo.queue_length() << 8);
	}
	// Break detect and errors are cleared by reading this
	m_ustat &= ~0xf;
	return res;
}

WRITE32_MEMBER( vr0uart_device::transmit_buffer_w )
{
	if (ACCESSING_BITS_0_7)
		tx_send_byte(data & 0xff);
}

READ32_MEMBER( vr0uart_device::receive_buffer_r )
{
	// TODO: unknown value & behaviour attempting to read this on empty FIFO (stall?)
	uint8_t res = 0;

	if (ACCESSING_BITS_0_7 && !m_urxb_fifo.empty())
		res = m_urxb_fifo.dequeue();

	return res;
}

READ32_MEMBER( vr0uart_device::baud_rate_div_r )
{
	return m_ubdr;
}

WRITE32_MEMBER( vr0uart_device::baud_rate_div_w )
{
	COMBINE_DATA(&m_ubdr);
	update_serial_config();
}
