// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "sdlc.h"

#include <sstream>


#define LOG_RXBIT       (1U << 1)
#define LOG_RXFLAG      (1U << 2)
#define LOG_LINESTATE   (1U << 3)
#define LOG_FRAMING     (1U << 4)

//#define VERBOSE (LOG_GENERAL | LOG_RXBIT | LOG_RXFLAG | LOG_LINESTATE | LOG_FRAMING)
#include "logmacro.h"

#define LOGRXBIT(...)       LOGMASKED(LOG_RXBIT, __VA_ARGS__)
#define LOGRXFLAG(...)      LOGMASKED(LOG_RXFLAG, __VA_ARGS__)
#define LOGLINESTATE(...)   LOGMASKED(LOG_LINESTATE, __VA_ARGS__)
#define LOGFRAMING(...)     LOGMASKED(LOG_FRAMING, __VA_ARGS__)


DEFINE_DEVICE_TYPE(SDLC_LOGGER, sdlc_logger_device, "sdlc_logger", "SDLC/HDLC logger")


constexpr std::uint16_t device_sdlc_consumer_interface::POLY_SDLC;


device_sdlc_consumer_interface::device_sdlc_consumer_interface(machine_config const &mconfig, device_t &device) :
	device_interface(device, "sdlc_consumer"),
	m_line_active(0U),
	m_discard_bits(0U),
	m_in_frame(0U),
	m_shift_register(0xffffU),
	m_frame_check(0xffffU)
{
}

device_sdlc_consumer_interface::~device_sdlc_consumer_interface()
{
}

void device_sdlc_consumer_interface::interface_post_start()
{
	device().save_item(NAME(m_line_active));
	device().save_item(NAME(m_discard_bits));
	device().save_item(NAME(m_in_frame));
	device().save_item(NAME(m_shift_register));
	device().save_item(NAME(m_frame_check));
}

void device_sdlc_consumer_interface::rx_bit(bool state)
{
	LOGRXBIT("Received bit %u\n", state ? 1U : 0U);

	m_shift_register = (m_shift_register >> 1) | (state ? 0x8000U : 0x0000U);
	if (!state && !m_line_active)
	{
		// any zero bit means the line has become active
		LOGLINESTATE("Line became active\n");
		m_line_active = 1U;
		line_active();
	}

	if ((m_shift_register & 0xff00U) == 0x7e00U)
	{
		// a flag opens and closes frames
		LOGRXFLAG("Received flag\n");
		if (m_in_frame)
		{
			LOGFRAMING("End of frame\n");
			m_in_frame = 0U;
			frame_end();
		}
		m_discard_bits = 8U;
		m_frame_check = 0xffffU;
	}
	else if ((m_shift_register & 0xfffeU) == 0xfffeU)
	{
		// fifteen consecutive ones is an inactive line condition
		if (m_line_active)
		{
			LOGLINESTATE("Line became inactive\n");
			m_line_active = 0U;
			line_inactive();
		}
	}
	else if ((m_shift_register & 0xfe00U) == 0xfe00U)
	{
		// seven consecutive ones is a frame abort
		if (m_in_frame || m_discard_bits)
		{
			LOGFRAMING("Received frame abort\n");
			m_in_frame = 0U;
			m_discard_bits = 0U;
			frame_abort();
		}
	}
	else
	{
		// discard the flag as it shifts off
		if (m_discard_bits && !--m_discard_bits)
		{
			LOGFRAMING("Start of frame\n");
			m_in_frame = 1U;
			frame_start();
		}

		// discard a zero after five consecutive ones
		if (m_in_frame && ((m_shift_register & 0x01f8U) != 0x00f8U))
		{
			bool const bit(BIT(m_shift_register, 8));
			m_frame_check = update_frame_check(POLY_SDLC, m_frame_check, bit);
			data_bit(bit);
		}
	}
}

void device_sdlc_consumer_interface::rx_reset()
{
	LOG("Receive reset\n");

	m_line_active = 0U;
	m_in_frame = 0U;
	m_discard_bits = 0U;
	m_shift_register = 0xffffU;
	m_frame_check = 0xffffU;
}


sdlc_logger_device::sdlc_logger_device(machine_config const &mconfig, char const *tag, device_t *owner, std::uint32_t clock) :
	device_t(mconfig, SDLC_LOGGER, tag, owner, clock),
	device_sdlc_consumer_interface(mconfig, *this),
	m_data_nrzi(0U),
	m_clock_active(1U),
	m_current_data(1U),
	m_last_data(1U),
	m_current_clock(1U),
	m_frame_bits(0U),
	m_expected_fcs(0U),
	m_buffer()
{
}

sdlc_logger_device::~sdlc_logger_device()
{
}

void sdlc_logger_device::clock_w(int state)
{
	if (bool(state) != bool(m_current_clock))
	{
		m_current_clock = state ? 1U : 0U;
		if (m_current_clock == m_clock_active)
		{
			bool const bit(m_data_nrzi ? (m_current_data == m_last_data) : m_current_data);
			LOGRXBIT("Received bit: %u (%u -> %u)\n", bit ? 1U : 0U, m_last_data, m_current_data);
			m_last_data = m_current_data;
			rx_bit(bit);
		}
	}
}

void sdlc_logger_device::device_start()
{
	m_buffer.reset(new std::uint8_t[BUFFER_BYTES]);

	save_item(NAME(m_data_nrzi));
	save_item(NAME(m_clock_active));
	save_item(NAME(m_current_data));
	save_item(NAME(m_last_data));
	save_item(NAME(m_current_clock));
	save_item(NAME(m_frame_bits));
	save_item(NAME(m_expected_fcs));
	save_pointer(NAME(m_buffer), BUFFER_BYTES);
}

void sdlc_logger_device::device_reset()
{
}

void sdlc_logger_device::frame_start()
{
	m_frame_bits = 0U;
	m_expected_fcs = 0xffffU;
}

void sdlc_logger_device::frame_end()
{
	shift_residual_bits();
	log_frame(false);
	m_frame_bits = 0;
}

void sdlc_logger_device::frame_abort()
{
	logerror("Frame aborted!\n");
	shift_residual_bits();
	log_frame(true);
	m_frame_bits = 0U;
}

void sdlc_logger_device::data_bit(bool value)
{
	if (BUFFER_BITS > m_frame_bits)
	{
		m_buffer[m_frame_bits >> 3] >>= 1;
		m_buffer[m_frame_bits >> 3] |= value ? 0x80U : 0x00U;
	}
	else if (BUFFER_BITS == m_frame_bits)
	{
		logerror("Frame buffer overrun!\n");
	}

	if ((16U <= m_frame_bits) && ((BUFFER_BITS + 16U) > m_frame_bits))
		m_expected_fcs = update_frame_check(POLY_SDLC, m_expected_fcs, BIT(m_buffer[(m_frame_bits - 16U) >> 3], m_frame_bits & 0x0007U));

	++m_frame_bits;
}

void sdlc_logger_device::shift_residual_bits()
{
	if (BUFFER_BITS > m_frame_bits)
	{
		uint32_t const residual_bits(m_frame_bits & 0x0007U);
		if (residual_bits)
			m_buffer[m_frame_bits >> 3] >>= 8 - residual_bits;
	}
}

void sdlc_logger_device::log_frame(bool partial) const
{
	if (m_frame_bits)
	{
		std::ostringstream msg;
		std::uint32_t const frame_bytes(m_frame_bits >> 3);
		std::uint32_t const residual_bits(m_frame_bits & 0x0007U);
		util::stream_format(msg, "Received %u-bit %sframe (%u bytes + %u bits)", m_frame_bits, partial ? "partial " : "", frame_bytes, residual_bits);

		if (8U <= m_frame_bits)
		{
			std::uint8_t const addr(m_buffer[0]);
			util::stream_format(msg, " A=%02X%s", addr, (0xffU == addr) ? " (broadcast)" : !addr ? " (no station)" : "");
		}

		if (16U <= m_frame_bits)
		{
			std::uint8_t const ctrl(m_buffer[1]);
			if (!BIT(ctrl, 0))
			{
				msg << " I";
			}
			else if (!BIT(ctrl, 1))
			{
				msg << " S";
				switch (ctrl & 0x0cU)
				{
				case 0x00U: msg << " RR"; break;
				case 0x04U: msg << " RNR"; break;
				case 0x08U: msg << " REJ"; break;
				}
			}
			else
			{
				msg << " U";
				switch (ctrl & 0xecU)
				{
				case 0x00U: msg << " UI"; break;
				case 0x04U: msg << " RIM/SIM"; break;
				case 0x0cU: msg << " DM"; break;
				case 0x20U: msg << " UP"; break;
				case 0x40U: msg << " DISC/RD"; break;
				case 0x60U: msg << " UA"; break;
				case 0x80U: msg << " SNRM"; break;
				case 0x84U: msg << " FRMR"; break;
				case 0x9cU: msg << " XID"; break;
				case 0xc4U: msg << " CFGR"; break;
				case 0xccU: msg << " SNRME"; break;
				case 0xe0U: msg << " TEST"; break;
				case 0xecU: msg << " BCN"; break;
				}
			}

			if (!partial && (BUFFER_BITS >= m_frame_bits))
			{
				std::uint16_t fcs;
				fcs = std::uint16_t(m_buffer[frame_bytes - 2]) >> residual_bits;
				fcs |= std::uint16_t(m_buffer[frame_bytes - 1]) << (8 - residual_bits);
				if (residual_bits)
					fcs |= (std::uint16_t(m_buffer[frame_bytes]) & ((1U << residual_bits) - 1U)) << (16 - residual_bits);
				fcs = ~bitswap<16>(fcs, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
				util::stream_format(msg, " FCS=%04X", fcs);
				if (!is_frame_check_good())
					util::stream_format(msg, " (expected %04X)", m_expected_fcs);
			}
		}

		if (!partial)
			msg << (is_frame_check_good() ? " (good)" : " (bad)");

		for (std::uint32_t i = 0U; (frame_bytes > i) && (BUFFER_BYTES > i); ++i)
			util::stream_format(msg, (i & 0x000fU) ? " %02X" : "\n    %02X", m_buffer[i]);
		if (residual_bits && (BUFFER_BITS >= m_frame_bits))
			util::stream_format(msg, (residual_bits > 4) ? "%s %02X&%02X" : "%s %01X&%01X", (frame_bytes & 0x000fU) ? "" : "\n   ", m_buffer[frame_bytes], (1U << residual_bits) - 1);

		logerror("%s\n", msg.str());
	}
}
