// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************8

    Sega Mega Drive I/O Port

    TODO:
    * How is the serial clock generated?  What's the base clock
      input?
    * Serial reception - requires peripherals to be reworked.
    * Is the transmit data register separate from the transmit shift
      register?
    * What sets the serial receive error?  Framing error, overrun,
      or both?
    * Can receive errors cause interrupts when the receive buffer
      isn't ready?

***********************************************************************/

#include "emu.h"
#include "mdioport.h"

#include <cassert>

//#define LOG_OUTPUT_FUNC osd_printf_info
//#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(MEGADRIVE_IO_PORT, megadrive_io_port_device, "megadrive_io_port", "Sega Mega Drive I/O Port Controller")


megadrive_io_port_device::megadrive_io_port_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	device_t(mconfig, MEGADRIVE_IO_PORT, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_in_callback(*this),
	m_out_callback(*this),
	m_hl_callback(*this),
	m_th_in(DATA_TH_MASK),
	m_txd(DATA_TXD_MASK),
	m_data(0),
	m_ctrl(0),
	m_s_ctrl(0),
	m_txdata(0),
	m_rxdata(0)
{
}


WRITE_LINE_MEMBER(megadrive_io_port_device::th_w)
{
	u8 const th = state ? DATA_TH_MASK : 0x00;
	if (th != m_th_in)
	{
		LOG(state ? "TH rising\n" : "TH falling\n");
		bool const hlupdate =
				ctrl_int() &&
				!BIT(m_ctrl, DATA_TH_BIT) &&
				!rrdy_int();
		m_th_in = th;
		if (hlupdate)
			m_hl_callback(state ? 0 : 1);
	}
}


u8 megadrive_io_port_device::data_r()
{
	// TODO: confirm what's read from pins set to input and configured for serial I/O
	u8 const serialmask =
			(s_ctrl_sin() ? DATA_RXD_MASK : 0x00) |
			(s_ctrl_sout() ? DATA_TXD_MASK : 0x00);
	u8 const outmask = m_ctrl | 0x80;
	u8 const inmask = ~(serialmask | outmask);

	u8 result = (m_data & outmask) | (serialmask & ~outmask) | (m_th_in & inmask);
	if (inmask & 0x3f)
		result |= (!m_in_callback.isnull() ? m_in_callback() : 0x3f) & inmask;

	return result;
}


u8 megadrive_io_port_device::s_ctrl_r()
{
	// TODO: confirm this has no side effects
	return m_s_ctrl;
}


u8 megadrive_io_port_device::rxdata_r()
{
	if (!machine().side_effects_disabled())
	{
		// TODO: confirm this resets both RERR and RRDY
		LOG(
				"%s: read RxDATA 0x%02X\n%s%s",
				machine().describe_context(),
				m_rxdata,
				s_ctrl_rerr() ? ", clear RERR" : "",
				s_ctrl_rrdy() ? ", clear RRDY" : "");
		bool const hlupdate = rrdy_int() && !th_int();
		m_s_ctrl &= ~(S_CTRL_RERR_MASK | S_CTRL_RRDY_MASK);
		if (hlupdate)
			m_hl_callback(0);
	}

	return m_rxdata;
}


void megadrive_io_port_device::data_w(u8 data)
{
	// TODO: confirm whether TH-INT follows TH as output
	u8 const outmask =
			(m_ctrl & 0x7f) &
			(s_ctrl_sin() ? ~DATA_RXD_MASK : 0xff) &
			(s_ctrl_sout() ? ~DATA_TXD_MASK : 0xff);
	bool const outupdate =
			!m_out_callback.isnull() &&
			((data & outmask) != (m_data & outmask));
	bool const hlupdate =
			ctrl_int() &&
			!rrdy_int() &&
			BIT(m_ctrl & (data ^ m_data), DATA_TH_BIT);

	m_data = data;
	if (outupdate)
		update_out();
	if (hlupdate)
		m_hl_callback(BIT(~data, DATA_TH_BIT));
}


void megadrive_io_port_device::ctrl_w(u8 data)
{
	// TODO: confirm whether TH-INT follows TH as output
	u8 const changed = data ^ m_ctrl;
	u8 const parallelmask =
			0x4f |
			(s_ctrl_sin() ? 0x00 : DATA_RXD_MASK) |
			(s_ctrl_sout() ? 0x00 : DATA_TXD_MASK);
	bool const outupdate = !m_out_callback.isnull() && (changed & parallelmask);
	bool const hlupdate =
			(BIT(changed, 7) || (BIT(data, 7) && BIT(changed & (m_data ^ m_th_in), DATA_TH_BIT))) &&
			!rrdy_int();

	m_ctrl = data;
	if (outupdate)
		update_out();
	if (hlupdate)
		m_hl_callback(th_int() ? 1 : 0);
}


void megadrive_io_port_device::s_ctrl_w(u8 data)
{
	u8 const changed = data ^ m_s_ctrl;

	if (changed & 0xc0)
	{
		int rate;
		switch (data & 0xc0)
		{
		default: // shut up stupid compilers
		case 0x00: rate = 4800; break;
		case 0x40: rate = 2400; break;
		case 0x80: rate = 1200; break;
		case 0xc0: rate = 300; break;
		}
		LOG(
				"%s: serial data rate = %u bits/second\n",
				machine().describe_context(),
				rate);
		set_rate(rate);
	}

	bool const outchanged =
			((changed & S_CTRL_SIN_MASK) && BIT(m_ctrl, 5)) ||
			((changed & S_CTRL_SOUT_MASK) && BIT(~m_ctrl | (m_data ^ m_txd), 4));
	bool const hlupdate =
			(changed & S_CTRL_RINT_MASK) &&
			(s_ctrl_rerr() || s_ctrl_rrdy()) &&
			!th_int();

	m_s_ctrl = (data & 0xf8) | (m_s_ctrl & 0x07);
	if (!m_out_callback.isnull() && outchanged)
		update_out();
	if (hlupdate)
		m_hl_callback(s_ctrl_rint() ? 1 : 0);
}


void megadrive_io_port_device::txdata_w(u8 data)
{
	if (s_ctrl_tful())
	{
		LOG(
				"%s: TxDATA = 0x%02X, previous byte 0x%02X lost\n",
				machine().describe_context(),
				data,
				m_txdata);
	}
	else if (is_transmit_register_empty())
	{
		LOG(
				"%s: TxDATA = 0x%02X, start transmission\n",
				machine().describe_context(),
				data);
		transmit_register_setup(data);
	}
	else
	{
		LOG("%s: TxDATA = 0x%02X\n", machine().describe_context(), data);
		m_s_ctrl |= S_CTRL_TFUL_MASK;
	}

	m_txdata = data;
}


void megadrive_io_port_device::device_resolve_objects()
{
	m_th_in = DATA_TH_MASK;

	m_in_callback.resolve();
	m_out_callback.resolve();
	m_hl_callback.resolve_safe();
}


void megadrive_io_port_device::device_start()
{
	save_item(NAME(m_txd));
	save_item(NAME(m_th_in));
	save_item(NAME(m_data));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_s_ctrl));
	save_item(NAME(m_txdata));
	save_item(NAME(m_rxdata));

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
}


void megadrive_io_port_device::device_reset()
{
	// TODO: proper reset state for DATA, TxDATA and RxDATA
	m_txd = DATA_TXD_MASK;
	m_data = 0xff;
	m_ctrl = 0x00;
	m_s_ctrl = 0x00;
	m_txdata = 0x00;
	m_rxdata = 0x00;

	receive_register_reset();
	transmit_register_reset();
	set_rate(4800);

	if (!m_out_callback.isnull())
		m_out_callback(0x7f, 0x00);
	m_hl_callback(0);
}


void megadrive_io_port_device::tra_callback()
{
	u8 const txd = transmit_register_get_data_bit() ? DATA_TXD_MASK : 0x00;
	if (txd != m_txd)
	{
		m_txd = txd;
		if (!m_out_callback.isnull() && s_ctrl_sout())
			update_out();
	}
}


void megadrive_io_port_device::tra_complete()
{
	if (s_ctrl_tful())
	{
		LOG("serial transmit complete, load next byte 0x%02X\n", m_txdata);
		transmit_register_setup(m_txdata);
		m_s_ctrl &= ~S_CTRL_TFUL_MASK;
	}
	else
	{
		LOG("serial transmit complete\n");
	}
}


void megadrive_io_port_device::rcv_complete()
{
	receive_register_extract();
	u8 const data = get_received_char();
	bool const hlupdate =
			s_ctrl_rint() &&
			!s_ctrl_rerr() &&
			!s_ctrl_rrdy() &&
			!th_int();

	if (s_ctrl_rrdy())
	{
		// TODO: confirm whether receive overrun sets RERR
		LOG(
				"received byte 0x%02X%s, previous received byte 0x%02X lost\n",
				data,
				is_receive_framing_error() ? ", framing error" : "",
				m_rxdata);
		m_s_ctrl |= S_CTRL_RERR_MASK;
	}
	else if (is_receive_framing_error())
	{
		// TODO: confirm whether framing error sets RRDY and/or updates RxDATA
		LOG("received byte 0x%02X, framing error\n", data);
		m_s_ctrl |= S_CTRL_RERR_MASK | S_CTRL_RRDY_MASK;
	}
	else
	{
		LOG("received byte 0x%02X\n", data);
		m_s_ctrl |= S_CTRL_RRDY_MASK;
	}

	m_rxdata = data;
	if (hlupdate)
		m_hl_callback(1);
}


inline bool megadrive_io_port_device::th_int() const
{
	// TODO: confirm whether TH-INT follows TH as output
	return
			ctrl_int() &&
			(BIT(m_ctrl, DATA_TH_BIT) ? !BIT(m_data, DATA_TH_BIT) : !m_th_in);
}


inline bool megadrive_io_port_device::rrdy_int() const
{
	// TODO: confirm whether RERR can cause an interrupt
	return s_ctrl_rint() && (s_ctrl_rerr() || s_ctrl_rrdy());
}


inline u8 megadrive_io_port_device::out_drive() const
{
	return
			(m_ctrl & (s_ctrl_sin() ? 0x5f : 0x7f)) |
			(s_ctrl_sout() ? DATA_TXD_MASK : 0x00);
}


inline void megadrive_io_port_device::update_out()
{
	assert(!m_out_callback.isnull());

	u8 const drive = out_drive();
	u8 const data = (m_data | ~drive) & (s_ctrl_sout() ? 0x6f : 0x7f);
	m_out_callback(data | (s_ctrl_sout() ? m_txd : 0x00), drive);
}
