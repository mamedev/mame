// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    AMD Am79C90 CMOS Local Area Network Controller for Ethernet (C-LANCE)

    TODO:
        - Communication with the outside world
        - Error handling
        - Clocks

*****************************************************************************/

#include "emu.h"
#include "am79c90.h"

DEFINE_DEVICE_TYPE(AM7990, am7990_device, "am7990", "Am7990 LANCE Ethernet Controller")
DEFINE_DEVICE_TYPE(AM79C90, am79c90_device, "am79c90", "Am79C90 C-LANCE Ethernet Controller")

am7990_device_base::am7990_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_receive_timer(nullptr)
	//, m_receive_poll_timer(nullptr)
	, m_transmit_timer(nullptr)
	, m_transmit_poll_timer(nullptr)
	, m_irq_out_cb(*this)
	, m_dma_out_cb(*this)
	, m_dma_in_cb(*this)
{
}

am7990_device::am7990_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: am7990_device_base(mconfig, AM7990, tag, owner, clock)
{
}

am79c90_device::am79c90_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: am7990_device_base(mconfig, AM79C90, tag, owner, clock)
{
}

void am7990_device_base::device_start()
{
	m_irq_out_cb.resolve_safe();
	m_dma_out_cb.resolve_safe(); // TODO: Should be read/write16!
	m_dma_in_cb.resolve_safe(0);

	m_transmit_poll_timer = timer_alloc(TIMER_TRANSMIT_POLL);
	m_transmit_poll_timer->adjust(attotime::never);
	m_transmit_timer = timer_alloc(TIMER_TRANSMIT);
	m_transmit_timer->adjust(attotime::never);
	m_receive_timer = timer_alloc(TIMER_RECEIVE);
	m_receive_timer->adjust(attotime::never);

	save_item(NAME(m_curr_transmit_desc.m_tmd01));
	save_item(NAME(m_curr_transmit_desc.m_tmd23));
	save_item(NAME(m_next_transmit_desc.m_tmd01));
	save_item(NAME(m_next_transmit_desc.m_tmd23));
	save_item(NAME(m_curr_recv_desc.m_tmd01));
	save_item(NAME(m_curr_recv_desc.m_tmd23));
	save_item(NAME(m_next_recv_desc.m_tmd01));
	save_item(NAME(m_next_recv_desc.m_tmd23));
	save_item(NAME(m_rap));
	save_item(NAME(m_csr));
	save_item(NAME(m_mode));
	save_item(NAME(m_logical_addr_filter));
	save_item(NAME(m_physical_addr));

	save_item(NAME(m_recv_message_count));
	save_item(NAME(m_recv_ring_addr));
	save_item(NAME(m_recv_buf_addr));
	save_item(NAME(m_recv_buf_count));
	save_item(NAME(m_recv_ring_length));
	save_item(NAME(m_recv_ring_pos));
	save_item(NAME(m_recv_fifo));
	save_item(NAME(m_recv_fifo_write));
	save_item(NAME(m_recv_fifo_read));
	save_item(NAME(m_receiving));

	save_item(NAME(m_transmit_ring_addr));
	save_item(NAME(m_transmit_buf_addr));
	save_item(NAME(m_transmit_buf_count));
	save_item(NAME(m_transmit_ring_length));
	save_item(NAME(m_transmit_ring_pos));
	save_item(NAME(m_transmit_fifo));
	save_item(NAME(m_transmit_fifo_write));
	save_item(NAME(m_transmit_fifo_read));
	save_item(NAME(m_transmitting));
}

void am7990_device_base::device_reset()
{
	memset(&m_curr_transmit_desc, 0, sizeof(ring_descriptor));
	memset(&m_next_transmit_desc, 0, sizeof(ring_descriptor));
	memset(&m_curr_recv_desc, 0, sizeof(ring_descriptor));
	memset(&m_next_recv_desc, 0, sizeof(ring_descriptor));
	m_rap = 0;
	memset(m_csr, 0, sizeof(uint16_t) * 4);
	m_csr[0] = CSR0_STOP;
	m_mode = 0;
	m_logical_addr_filter = 0;
	m_physical_addr = 0;

	m_recv_message_count = 0;
	m_recv_ring_addr = 0;
	m_recv_buf_addr = 0;
	m_recv_buf_count = 0;
	m_recv_ring_length = 0;
	m_recv_ring_pos = 0;
	memset(m_recv_fifo, 0, sizeof(uint32_t) * ARRAY_LENGTH(m_recv_fifo));
	m_recv_fifo_write = 0;
	m_recv_fifo_read = 0;
	m_receiving = false;

	m_transmit_ring_addr = 0;
	m_transmit_buf_addr = 0;
	m_transmit_buf_count = 0;
	m_transmit_ring_length = 0;
	m_transmit_ring_pos = 0;
	memset(m_transmit_fifo, 0, sizeof(uint32_t) * ARRAY_LENGTH(m_transmit_fifo));
	m_transmit_fifo_write = 0;
	m_transmit_fifo_read = 0;
	m_transmitting = false;
}

void am7990_device_base::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_TRANSMIT_POLL:
			poll_transmit();
			break;

		case TIMER_TRANSMIT:
			transmit();
			break;

		case TIMER_RECEIVE:
			receive();
			break;
	}
}

void am7990_device_base::fetch_transmit_descriptor()
{
	const uint32_t next_addr = (m_transmit_ring_addr >> 2) + (m_transmit_ring_pos << 1);
	ring_descriptor &next = m_next_transmit_desc;
	next.m_tmd01 = m_dma_in_cb(next_addr, ~0);
	next.m_tmd23 = m_dma_in_cb(next_addr + 1, ~0);
}

void am7990_device_base::fetch_receive_descriptor()
{
	const uint32_t next_addr = (m_recv_ring_addr >> 2) + (m_recv_ring_pos << 1);
	ring_descriptor &next = m_next_recv_desc;
	next.m_rmd01 = m_dma_in_cb(next_addr, ~0);
	next.m_rmd23 = m_dma_in_cb(next_addr + 1, ~0);
}

void am7990_device_base::recv_fifo_push(uint32_t value)
{
	// TODO: Poll for the FIFO at 1.6ms, don't instantly start receiving!
	// "...If the C-LANCE does not own it, it will poll the ring once every 1.6ms until
	//  it owns it."

	logerror("%s: LANCE pushing %08x onto receive FIFO\n", machine().describe_context(), value);
	if (!m_receiving && !(m_mode & MODE_LOOP))
	{
		begin_receiving();
	}

	if (m_recv_fifo_write >= ARRAY_LENGTH(m_recv_fifo))
	{
		logerror("%s: LANCE can't push onto receive FIFO, %d >= %d\n", machine().describe_context(), value, m_recv_fifo_write, ARRAY_LENGTH(m_recv_fifo));
		// TODO: Do something
		return;
	}
	m_recv_fifo[m_recv_fifo_write] = value;
	m_recv_fifo_write++;
	if (m_recv_fifo_write == ARRAY_LENGTH(m_recv_fifo))
	{
		// TODO: Do something
	}
}

void am7990_device_base::begin_receiving()
{
	fetch_receive_descriptor();
	m_curr_recv_desc = m_next_recv_desc;

	ring_descriptor &curr = m_curr_recv_desc;
	if (curr.m_rmd01 & RMD1_OWN)
	{
		logerror("%s: LANCE owns the current buffer, activating receive timer, RMD0123 is %08x %08x\n", machine().describe_context(), curr.m_rmd01, curr.m_rmd23);

		m_receiving = true;
		m_recv_buf_addr = (((curr.m_rmd01 << 16) | (curr.m_rmd01 >> 16)) & 0x00ffffff) | 0xff000000;
		const int32_t rmd2 = (int16_t)(curr.m_rmd23 >> 16);
		m_recv_buf_count = (uint16_t)((-rmd2) & 0x00000fff);
		if (m_recv_buf_count == 0)
			m_recv_buf_count = 0x1000;

		if (m_mode & MODE_LOOP)
		{
			m_recv_buf_count = (m_mode & MODE_DTCR) ? 32 : 36;
		}

		if (curr.m_rmd01 & RMD1_STP)
		{
			m_recv_message_count = 0;
		}
		m_receive_timer->adjust(attotime::from_hz(10'000'000), 0, attotime::from_hz(10'000'000));
	}
	else
	{
		logerror("%s: LANCE does not own the current buffer, deactivating receive timer\n", machine().describe_context());
		m_receiving = false;
		m_receive_timer->adjust(attotime::never);
	}
}

void am7990_device_base::receive()
{
	const uint32_t received_value = (m_recv_fifo_write == 0) ? 0 : m_recv_fifo[m_recv_fifo_read];
	m_recv_fifo_read++;
	if (m_recv_fifo_read >= m_recv_fifo_write)
	{
		m_recv_fifo_read = 0;
		m_recv_fifo_write = 0;
	}

	if (m_recv_buf_count >= 4)
	{
		logerror("%s: LANCE receiving %08x to address %08x, remaining %d\n", machine().describe_context(), received_value, m_recv_buf_addr, m_recv_buf_count - 4);
		m_dma_out_cb(m_recv_buf_addr >> 2, received_value, ~0);

		m_recv_buf_addr += 4;
		m_recv_message_count += 4;
		m_recv_buf_count -= 4;
	}
	else
	{
		const uint32_t mask = 0xffffffff << (m_recv_buf_count << 3);
		logerror("%s: LANCE receiving %08x & %08x to address %08x, remaining %d\n", machine().describe_context(), received_value, mask, m_recv_buf_addr, 0);
		m_dma_out_cb(m_recv_buf_addr >> 2, received_value, mask);

		m_recv_buf_addr += m_recv_buf_count;
		m_recv_message_count += m_recv_buf_count;
		m_recv_buf_count = 0;
	}

	if (m_recv_buf_count == 0)
	{
		logerror("%s: LANCE has completed receiving a buffer, clearing OWN bit and advancing receive ring position\n", machine().describe_context());
		ring_descriptor &curr = m_curr_recv_desc;
		curr.m_rmd01 &= ~RMD1_OWN;
		const uint32_t addr = (m_recv_ring_addr >> 2) + (m_recv_ring_pos << 1);
		logerror("%s: LANCE is writing new RMD01: %08x\n", machine().describe_context(), curr.m_rmd01);
		m_dma_out_cb(addr, curr.m_rmd01, ~0);

		if (curr.m_rmd01 & TMD1_ENP)
		{
			curr.m_rmd23 &= 0xfffff000;
			if (m_recv_message_count != 0x1000)
			{
				curr.m_rmd23 |= m_recv_message_count & 0xfff;
			}

			m_dma_out_cb(addr + 1, curr.m_rmd23, ~0);

			logerror("%s: LANCE has completed receiving a message, total message length %d bytes, new RMD23 %08x\n", machine().describe_context(), m_recv_message_count, curr.m_rmd23);
		}
		m_recv_ring_pos++;
		m_recv_ring_pos &= m_recv_ring_length - 1;

		if (!(m_mode & MODE_LOOP))
		{
			begin_receiving();
		}
		else
		{
			logerror("%s: LANCE loopback test receive finished, setting RINT and stopping timer.\n", machine().describe_context());
			m_csr[0] |= CSR0_RINT;
			update_interrupts();
			m_receiving = false;
			m_receive_timer->adjust(attotime::never);
		}
	}
}

void am7990_device_base::transmit()
{
	logerror("%s: LANCE transmit, fetching from %08x\n", machine().describe_context(), m_transmit_buf_addr >> 2);
	uint32_t transmit_value = 0;
	const bool dtcr = m_mode & MODE_DTCR;
	if (m_transmit_buf_count > 4 || dtcr)
	{
		transmit_value = m_dma_in_cb(m_transmit_buf_addr >> 2, ~0);
		if (!dtcr)
		{
			m_crc32.append(&transmit_value, sizeof(uint32_t));
		}
	}
	else
	{
		transmit_value = (uint32_t)m_crc32.finish();
	}
	const bool loopback = (m_mode & MODE_LOOP);
	if (loopback)
	{
		// TODO: Differentiate between internal and external loopback
		recv_fifo_push(transmit_value);
	}
	else
	{
		// TODO: Send data to an actual network interface and/or our real transmit FIFO
	}

	if (m_transmit_buf_count >= 4)
	{
		m_transmit_buf_addr += 4;
		m_transmit_buf_count -= 4;
	}
	else
	{
		m_transmit_buf_addr += m_transmit_buf_count;
		m_transmit_buf_count = 0;
	}

	if (m_transmit_buf_count == 0)
	{
		const uint32_t base_addr = (m_transmit_ring_addr >> 2) + (m_transmit_ring_pos << 1);
		ring_descriptor &curr = m_curr_transmit_desc;
		curr.m_tmd01 &= ~TMD1_OWN;

		m_dma_out_cb(base_addr, curr.m_tmd01, ~0);

		if (!(curr.m_tmd01 & TMD1_ENP))
		{
			fetch_transmit_descriptor();
			m_curr_transmit_desc = m_next_transmit_desc;

			if (!(curr.m_tmd01 & TMD1_OWN))
			{
				logerror("%s: LANCE is done transmitting a buffer of this descriptor ring, next ring unowned, resuming polling.\n", machine().describe_context());
				m_transmitting = false;
				m_transmit_timer->adjust(attotime::never);
				m_transmit_poll_timer->adjust(attotime::from_usec(1600), 0, attotime::from_usec(1600));
			}
			else
			{
				logerror("%s: LANCE is done transmitting a buffer of this descriptor ring, preparing to transmit next buffer.\n", machine().describe_context());
				prepare_transmit_buf();
			}
		}
		else
		{
			logerror("%s: LANCE is done transmitting the last buffer of this descriptor ring, raising TINT and resuming polling.\n", machine().describe_context());
			if (m_mode & MODE_LOOP)
			{
				begin_receiving();
			}

			m_csr[0] |= CSR0_TINT;
			update_interrupts();
			m_transmitting = false;
			m_transmit_timer->adjust(attotime::never);
			m_transmit_poll_timer->adjust(attotime::from_usec(1600), 0, attotime::from_usec(1600));
		}
	}
}

void am7990_device_base::prepare_transmit_buf()
{
	ring_descriptor &curr = m_curr_transmit_desc;
	m_transmit_buf_addr = ((curr.m_tmd01 << 16) | ((curr.m_tmd01 >> 16) & 0x00ffffff)) | 0xff000000;
	const int32_t tmd2 = (int16_t)(curr.m_tmd23 >> 16);
	m_transmit_buf_count = (uint16_t)((-tmd2) & 0x00000fff);
	if (m_transmit_buf_count == 0)
		m_transmit_buf_count = 0x1000;
	if (!(m_mode & MODE_DTCR))
	{
		m_transmit_buf_count += 4;
	}
	logerror("%s: LANCE: Valid transmit descriptors found, preparing to transmit %d bytes\n", machine().describe_context(), m_transmit_buf_count);
}

void am7990_device_base::poll_transmit()
{
	ring_descriptor &curr = m_curr_transmit_desc;
	const uint32_t base_addr = (m_transmit_ring_addr >> 2) + (m_transmit_ring_pos << 1);
	//logerror("%s: LANCE polling for packets from %08x\n", machine().describe_context(), base_addr);
	curr.m_tmd01 = m_dma_in_cb(base_addr, ~0);

	const uint16_t tmd1 = (uint16_t)curr.m_tmd01;
	if (!(tmd1 & TMD1_OWN))
		return;

	if (!(tmd1 & TMD1_STP) && !m_transmitting)
	{
		// "The STP bit must be set in the first buffer of the packet, or the C-LANCE will skip over this
		//  descriptor and poll the next descriptor(s) until the OWN and STP bits are set."
		m_transmit_ring_pos++;
		m_transmit_ring_pos &= m_transmit_ring_length - 1;
		logerror("%s: LANCE: No STP on this entry and not transmitting, skipping to next entry\n", machine().describe_context());
		return;
	}

	//logerror("%s: LANCE: Starting transmitting\n", machine().describe_context());

	m_transmit_poll_timer->adjust(attotime::never);
	m_transmitting = true;
	m_crc32.reset();

	// TMD0's value is retrieved from the value fetched above, but per the AMD Am79C90 manual, page 30:
	// "The C-LANCE will read TMD0 and TMD2 to get the rest of the buffer address and the buffer byte count
	//  when it owns the descriptor. Each of these memory reads is done separately with a new arbitration
	//  cycle for each transfer."
	m_dma_in_cb(base_addr, ~0);

	curr.m_tmd23 = m_dma_in_cb(base_addr + 1, ~0);

	m_transmit_ring_pos++;
	m_transmit_ring_pos &= m_transmit_ring_length - 1;

	if (!(tmd1 & TMD1_ENP))
	{
		logerror("%s: LANCE: No EOP on this entry, caching next entry and checking ownership\n", machine().describe_context());
		// "BUFFER ERROR is set by the C-LANCE during transmission when the C-LANCE does not find the ENP
		//  flag in the current buffer and does not own the next buffer."
		fetch_transmit_descriptor();
		ring_descriptor &next = m_next_transmit_desc;

		if (!((next.m_tmd01 >> 16) & TMD1_OWN))
		{
			logerror("%s: LANCE: No EOP on this entry, but we don't own the next one; setting BUFF\n", machine().describe_context());
			curr.m_tmd23 |= TMD3_BUFF;
			m_dma_out_cb(base_addr + 1, curr.m_tmd23, ~0);
			m_csr[0] &= ~CSR0_TXON;
			m_transmitting = false;
			return;
		}
	}

	prepare_transmit_buf();

	m_transmit_timer->adjust(attotime::from_hz(10'000'000), 0, attotime::from_hz(10'000'000));
}

void am7990_device_base::update_interrupts()
{
	if (m_csr[0] & CSR0_ANY_INTR)
	{
		m_csr[0] |= CSR0_INTR;
	}
	else
	{
		m_csr[0] &= ~CSR0_INTR;
	}
	m_irq_out_cb((m_csr[0] & CSR0_INTR) ? 1 : 0);
}

READ16_MEMBER(am7990_device_base::regs_r)
{
	uint16_t ret = 0;
	if (offset)
	{
		ret = m_rap;
		logerror("%s: lance_r: RAP = %04x\n", machine().describe_context(), ret);
	}
	else
	{
		ret = m_csr[m_rap];
		logerror("%s: lance_r: CSR%d = %04x\n", machine().describe_context(), m_rap, ret);
	}
	return ret;
}

WRITE16_MEMBER(am7990_device_base::regs_w)
{
	if (offset)
	{
		logerror("%s: lance_w: RAP = %d\n", machine().describe_context(), data & 3);
		m_rap = data & 3;
	}
	else
	{
		logerror("%s: lance_w: CSR%d = %04x\n", machine().describe_context(), m_rap, data);
		switch (m_rap)
		{
			case 0: // Control/Status
				m_csr[0] &= ~(data & (CSR0_ANY_ERR | CSR0_IDON));
				if (m_csr[0] & CSR0_ANY_ERR)
					m_csr[0] |= CSR0_ERR;
				else
					m_csr[0] &= ~CSR0_ERR;
				if (data & CSR0_STOP)
				{
					data &= ~(CSR0_RXON | CSR0_TXON | CSR0_TDMD | CSR0_STRT | CSR0_INIT);
					m_csr[0] &= ~(CSR0_IDON | CSR0_RXON | CSR0_TXON | CSR0_TDMD | CSR0_STRT | CSR0_INIT);
					m_csr[0] |= CSR0_STOP;
					m_csr[3] = 0;
					m_receive_timer->adjust(attotime::never);
					m_transmit_timer->adjust(attotime::never);
					m_transmit_poll_timer->adjust(attotime::never);
				}
				if (data & CSR0_INIT)
				{
					uint32_t init_addr = 0xff000000 | m_csr[1] | (m_csr[2] << 16);
					uint16_t init_block[12];

					logerror("%s: LANCE Init block:\n", machine().describe_context());

					for (uint32_t i = 0; i < 6; i++)
					{
						uint32_t value = m_dma_in_cb((init_addr >> 2) + i, ~0);
						init_block[i*2 + 0] = (uint16_t)(value >> 16);
						init_block[i*2 + 1] = (uint16_t)value;
						logerror("%s: IADR +%02d: %04x\n", machine().describe_context(), i*4, init_block[i*2 + 0]);
						logerror("%s: IADR +%02d: %04x\n", machine().describe_context(), i*4 + 2, init_block[i*2 + 1]);
					}

					m_mode = init_block[0];
					m_physical_addr = ((uint64_t)init_block[3] << 32) | ((uint64_t)init_block[2] << 16) | (uint64_t)init_block[1];
					m_logical_addr_filter = ((uint64_t)init_block[7] << 48) | ((uint64_t)init_block[6] << 32)
										  | ((uint64_t)init_block[5] << 16) | (uint64_t)init_block[4];
					m_recv_ring_addr = (((uint32_t)init_block[9] << 16) | (uint32_t)init_block[8]) & 0x00fffff8;
					m_recv_ring_addr |= 0xff000000;
					m_transmit_ring_addr = (((uint32_t)init_block[11] << 16) | (uint32_t)init_block[10]) & 0x00fffff8;
					m_transmit_ring_addr |= 0xff000000;
					m_recv_ring_length = 1 << ((init_block[9] >> 13) & 7);
					m_transmit_ring_length = 1 << ((init_block[11] >> 13) & 7);

					m_transmit_ring_pos = 0;
					m_recv_ring_pos = 0;

					logerror("%s: Mode: %04x\n", machine().describe_context(), m_mode);
					logerror("%s: Physical Address: %08x%08x\n", machine().describe_context(),
						(uint32_t)(m_physical_addr >> 32), (uint32_t)m_physical_addr);
					logerror("%s: Logical Address Filter: %08x%08x\n", machine().describe_context(),
						(uint32_t)(m_logical_addr_filter >> 32), (uint32_t)m_logical_addr_filter);
					logerror("%s: Receive Ring Address: %08x\n", machine().describe_context(), m_recv_ring_addr);
					logerror("%s: Receive Ring Length: %04x\n", machine().describe_context(), m_recv_ring_length);
					logerror("%s: Transmit Ring Address: %08x\n", machine().describe_context(), m_transmit_ring_addr);
					logerror("%s: Transmit Ring Length: %04x\n", machine().describe_context(), m_transmit_ring_length);

					m_csr[0] &= ~CSR0_STOP;
					m_csr[0] |= CSR0_IDON | CSR0_INIT | CSR0_TXON | CSR0_RXON;

					m_receive_timer->adjust(attotime::never);
					m_transmit_timer->adjust(attotime::never);
					m_transmit_poll_timer->adjust(attotime::never);
				}
				if (data & CSR0_STRT)
				{
					m_csr[0] &= ~CSR0_STOP;
					if (m_mode & MODE_DRX)
						m_csr[0] &= ~CSR0_RXON;
					if (m_mode & MODE_DTX)
						m_csr[0] &= ~CSR0_TXON;
					if (m_csr[0] & CSR0_TXON)
						m_transmit_poll_timer->adjust(attotime::from_usec(1600), 0, attotime::from_usec(1600));
				}
				update_interrupts();
				if (data & CSR0_TDMD)
				{
					// TODO: Handle transmit demand
				}
				break;
			case 1: // Least significant 15 bits of the Initialization Block
				// Datasheet says "must be zero", but doesn't indicate what
				// happens if it's written non-zero. Must be writable to pass
				// system diagnostic on MIPS RS2030.
				m_csr[1] = data;
				break;
			case 2: // Most significant 8 bits of the Initialization Block
				// The C-LANCE datasheet explicitly states these bits read and
				// write as zero, while LANCE datasheet just says "reserved".
				// MIPS RS2030 diagnostic requires these bits to be writable,
				// so assuming this is older device behaviour.
				m_csr[2] = (type() == AM7990) ? data : (data & 0x00ff);
				break;
			case 3: // Bus master interface
				m_csr[3] = data & 0x0007;
				break;
		}
	}
}
