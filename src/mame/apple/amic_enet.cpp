// license:BSD-3-Clause
// copyright-holders:R. Belmont

// AMIC Ethernet DMA, derived from Apple's AMICEqu.a and PDMMace.a.
// RX is a 48 KiB page ring with a packed, eight-byte header.
// TX has two fixed 2 KiB buffers.  All addresses use the physical DMA base.
#include "emu.h"
#include "amic_enet.h"

#define LOG_REG (1U << 1)
#define LOG_PACKET (1U << 2)
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(AMIC_ENET, amic_enet_device, "amic_enet", "AMIC Ethernet DMA")

amic_enet_device::amic_enet_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AMIC_ENET, tag, owner, clock)
	, m_space(*this, finder_base::DUMMY_TAG, -1)
	, m_mace(*this, finder_base::DUMMY_TAG)
	, m_rx_irq_out(*this)
	, m_tx_irq_out(*this)
	, m_timer(nullptr)
	, m_dma_base(0)
	, m_rx_control(0)
	, m_tx_control(0)
	, m_rx_head(0)
	, m_rx_tail(0)
	, m_rx_full(false)
	, m_rx_drq(false)
	, m_tx_drq(false)
	, m_tx_count{}
	, m_tx_position{}
	, m_tx_set(0)
	, m_rx_packet{}
	, m_rx_status{}
	, m_rx_length(0)
	, m_rx_status_count(0)
{
}

void amic_enet_device::map(address_map &map)
{
	// Offsets from the AMIC DMA base register at 50f31000
	map(0x0c20, 0x0c20).rw(FUNC(amic_enet_device::tx_control_r), FUNC(amic_enet_device::tx_control_w));
	map(0x1028, 0x1028).rw(FUNC(amic_enet_device::rx_control_r), FUNC(amic_enet_device::rx_control_w));
	map(0x1030, 0x1030).r(FUNC(amic_enet_device::rx_head_r));
	map(0x1034, 0x1034).rw(FUNC(amic_enet_device::rx_tail_r), FUNC(amic_enet_device::rx_tail_w));
	map(0x1044, 0x1045).select(0x10).rw(FUNC(amic_enet_device::tx_count_r), FUNC(amic_enet_device::tx_count_w));
}

void amic_enet_device::device_start()
{
	m_timer = timer_alloc(FUNC(amic_enet_device::dma_tick), this);
	save_item(NAME(m_dma_base));
	save_item(NAME(m_rx_control));
	save_item(NAME(m_tx_control));
	save_item(NAME(m_rx_head));
	save_item(NAME(m_rx_tail));
	save_item(NAME(m_rx_full));
	save_item(NAME(m_rx_drq));
	save_item(NAME(m_tx_drq));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_tx_position));
	save_item(NAME(m_tx_set));
	save_item(NAME(m_rx_packet));
	save_item(NAME(m_rx_status));
	save_item(NAME(m_rx_length));
	save_item(NAME(m_rx_status_count));
}

void amic_enet_device::device_reset()
{
	m_timer->enable(false);
	m_dma_base = 0;
	m_rx_control = m_tx_control = 0;
	m_rx_head = m_rx_tail = 0;
	m_rx_full = false;
	m_rx_drq = m_tx_drq = false;
	std::fill(std::begin(m_tx_count), std::end(m_tx_count), 0);
	std::fill(std::begin(m_tx_position), std::end(m_tx_position), 0);
	m_tx_set = 0;
	m_rx_length = m_rx_status_count = 0;
	update_irqs();
}

void amic_enet_device::device_post_load()
{
	update_irqs();
	kick();
}

void amic_enet_device::update_irqs()
{
	m_rx_irq_out((m_rx_control & (IE | IF)) == (IE | IF));
	m_tx_irq_out((m_tx_control & (IE | IF)) == (IE | IF));
}

u8 amic_enet_device::tx_control_r()
{
	return m_tx_control | (!m_tx_count[0] ? 0x20 : 0) | (!m_tx_count[1] ? 0x40 : 0);
}

void amic_enet_device::rx_control_w(u8 data)
{
	LOGMASKED(LOG_REG, "RX control %02x (head %02x tail %02x)\n", data, m_rx_head, m_rx_tail);
	if (data & RESET)
	{
		m_rx_control = m_rx_head = m_rx_tail = 0;
		m_rx_full = false;
		m_rx_length = m_rx_status_count = 0;
	}
	else
	{
		m_rx_control = (m_rx_control & (IF | OVERRUN) & ~data) | (data & (RUN | IE));
	}
	update_irqs();
	kick();
}

void amic_enet_device::tx_control_w(u8 data)
{
	LOGMASKED(LOG_REG, "TX control %02x (counts %04x/%04x)\n", data, m_tx_count[0], m_tx_count[1]);
	if (data & RESET)
	{
		m_tx_control = m_tx_set = 0;
		std::fill(std::begin(m_tx_count), std::end(m_tx_count), 0);
		std::fill(std::begin(m_tx_position), std::end(m_tx_position), 0);
	}
	else
	{
		m_tx_control = (m_tx_control & IF & ~data) | (data & (RUN | IE));
	}
	update_irqs();
	kick();
}

void amic_enet_device::rx_tail_w(u8 data)
{
	LOGMASKED(LOG_REG, "RX tail %02x (head %02x)\n", data, m_rx_head);
	// Ignore invalid page indices rather than allowing DMA outside the ring.
	if (data < RING_PAGES)
	{
		if (data != m_rx_tail)
		{
			m_rx_full = false;
		}
		m_rx_tail = data;
	}
}

u8 amic_enet_device::tx_count_r(offs_t offset)
{
	return m_tx_count[BIT(offset, 4)] >> (BIT(offset, 0) ? 0 : 8);
}

void amic_enet_device::tx_count_w(offs_t offset, u8 data)
{
	const unsigned set = BIT(offset, 4);
	const unsigned shift = BIT(offset, 0) ? 0 : 8;
	m_tx_count[set] = (m_tx_count[set] & ~(0xff << shift)) | (u16(data) << shift);
	// Software stops RUN and writes low then high to prime a fixed buffer.
	// The physical buffer is 2 KiB, so its address counter wraps at that size.
	m_tx_position[set] = 0;
	kick();
}

void amic_enet_device::rx_drq_w(int state)
{
	m_rx_drq = bool(state);
	kick();
}

void amic_enet_device::tx_drq_w(int state)
{
	m_tx_drq = bool(state);
	kick();
}

void amic_enet_device::kick()
{
	// Requests may arrive from within MACE's FIFO update.  Service later.
	if (m_timer && !m_timer->enabled() &&
		(((m_tx_control & RUN) && m_tx_drq && (m_tx_count[0] || m_tx_count[1])) ||
		 ((m_rx_control & RUN) && !(m_rx_control & OVERRUN) && (m_rx_drq || m_rx_length || m_rx_status_count))))
	{
		m_timer->adjust(attotime::from_usec(1));
	}
}

void amic_enet_device::receive_complete()
{
	const unsigned pages = (m_rx_length + 8 + 255) / 256;
	const unsigned available = m_rx_full ? 0 : (m_rx_tail + RING_PAGES - m_rx_head - 1) % RING_PAGES + 1;
	if (pages <= available)
	{
		log_packet("RX", m_rx_packet, m_rx_length);
		const u32 start = u32(m_rx_head) * 256;
		for (unsigned i = 0; i < m_rx_length; ++i)
		{
			m_space->write_byte(m_dma_base + (start + 8 + i) % RING_SIZE, m_rx_packet[i]);
		}
		// MACE presents low count, high count/status, runt and collision bytes.
		// AMIC puts each pair in big-endian order for the host driver.
		for (unsigned i = 0; i < 4; ++i)
		{
			m_space->write_byte(m_dma_base + start + (i ^ 1), m_rx_status[i]);
		}
		m_rx_head = (m_rx_head + pages) % RING_PAGES;
		m_rx_full = m_rx_head == m_rx_tail;
		LOGMASKED(LOG_REG, "RX complete: base %08x head %02x tail %02x bytes %u status %02x%02x%02x%02x\n",
			m_dma_base, m_rx_head, m_rx_tail, m_rx_length, m_rx_status[1], m_rx_status[0], m_rx_status[3], m_rx_status[2]);
	}
	// A full ring is distinguishable from an empty one by OVERRUN.  Stop before
	// overwriting unread pages; Apple's driver frees pages and writes 4a to resume.
	if (pages > available || m_rx_full)
	{
		m_rx_control = (m_rx_control & ~RUN) | OVERRUN;
	}
	m_rx_control |= IF;
	m_rx_length = m_rx_status_count = 0;
	update_irqs();
}

void amic_enet_device::log_packet(const char *direction, const u8 *data, unsigned length)
{
	if (VERBOSE & LOG_PACKET)
	{
		std::string bytes;
		for (unsigned i = 0; i < length; ++i)
		{
			bytes += util::string_format("%02x", data[i]);
		}
		LOGMASKED(LOG_PACKET, "AMIC_PACKET %s %s %u %s\n", machine().time().as_string(), direction, length, bytes);
	}
}

TIMER_CALLBACK_MEMBER(amic_enet_device::dma_tick)
{
	// Bounded bursts; MACE supplies wire timing and FIFO backpressure.
	for (unsigned word = 0; word < 8 && (m_tx_control & RUN) && m_tx_drq; ++word)
	{
		if (!m_tx_count[m_tx_set])
		{
			m_tx_set ^= 1;
		}
		if (!m_tx_count[m_tx_set])
		{
			break;
		}
		u16 &count = m_tx_count[m_tx_set];
		u16 &position = m_tx_position[m_tx_set];
		const u32 base = m_dma_base + 0x14000 + m_tx_set * 0x800;
		if ((VERBOSE & LOG_PACKET) && !position)
		{
			u8 packet[2048];
			const unsigned length = std::min<unsigned>(count, sizeof(packet));
			for (unsigned i = 0; i < length; ++i)
			{
				packet[i] = m_space->read_byte(base + i);
			}
			log_packet("TX", packet, length);
		}
		const unsigned bytes = std::min<unsigned>(2, count);
		u16 data = m_space->read_byte(base + (position & 0x7ff));
		if (bytes == 2)
		{
			data |= u16(m_space->read_byte(base + ((position + 1) & 0x7ff))) << 8;
		}
		if (!m_mace->tx_dma_w(data, bytes == 2 ? 0xffff : 0x00ff, count == bytes))
		{
			break;
		}
		position += bytes;
		count -= bytes;
		if (!count)
		{
			m_tx_control |= IF;
			m_tx_set ^= 1;
			update_irqs();
		}
	}
	for (unsigned word = 0; word < 8 && (m_rx_control & RUN) && !(m_rx_control & OVERRUN) &&
		(m_rx_drq || m_rx_length || m_rx_status_count); ++word)
	{
		const auto result = m_mace->rx_dma_r();
		if (!result.valid)
		{
			break;
		}
		if (result.status)
		{
			if (m_rx_status_count < 4)
			{
				m_rx_status[m_rx_status_count++] = result.data;
			}
		}
		else
		{
			for (unsigned byte = 0; byte < result.bytes; ++byte)
			{
				if (m_rx_length < sizeof(m_rx_packet))
				{
					m_rx_packet[m_rx_length++] = result.data >> (8 * byte);
				}
			}
		}
		if (result.frame_done)
		{
			receive_complete();
		}
	}
	kick();
}
