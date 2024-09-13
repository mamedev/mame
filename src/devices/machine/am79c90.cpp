// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * AMD Am7990 Local Area Network Controller for Ethernet (LANCE) and Am79C90
 * CMOS Local Area Network Controller for Ethernet (C-LANCE).
 *
 * Sources:
 *
 *  http://bitsavers.org/components/amd/Am7990/Am7990.pdf
 *  http://bitsavers.org/components/amd/Am7990/Am79c90.pdf
 *
 * TODO
 *   - external loopback
 *   - hp9k/3xx diagnostic failures
 *
 *                          _____   _____
 *                 Vss   1 |*    \_/     | 48  Vdd
 *                DAL7   2 |             | 47  DAL8
 *                DAL6   3 |             | 46  DAL9
 *                DAL5   4 |             | 45  DAL10
 *                DAL4   5 |             | 44  DAL11
 *                DAL3   6 |             | 43  DAL12
 *                DAL2   7 |             | 42  DAL13
 *                DAL1   8 |             | 41  DAL14
 *                DAL0   9 |             | 40  DAL15
 *                READ  10 |             | 39  A16
 *               /INTR  11 |             | 38  A17
 *               /DALI  12 |   Am79C90   | 37  A18
 *               /DALI  13 |             | 36  A19
 *                /DAS  14 |             | 35  A20
 *           /BM0,BYTE  15 |             | 34  A21
 *        /BM1,/BUSAKO  16 |             | 33  A22
 *        /HOLD,/BUSRQ  17 |             | 32  A23
 *             ALE,/AS  18 |             | 31  RX
 *               /HLDA  19 |             | 30  RENA
 *                 /CS  20 |             | 29  TX
 *                 ADR  21 |             | 28  CLSN
 *              /READY  22 |             | 27  RCLK
 *              /RESET  23 |             | 26  TENA
 *                 Vss  24 |_____________| 25  TCLK
 */

#include "emu.h"
#include "am79c90.h"

#include "multibyte.h"

#define LOG_REG     (1U << 1)
#define LOG_INIT    (1U << 2)
#define LOG_RXTX    (1U << 3)
#define LOG_FILTER  (1U << 4)
#define LOG_PACKETS (1U << 5)

//#define VERBOSE (LOG_GENERAL|LOG_REG|LOG_INIT|LOG_RXTX|LOG_FILTER|LOG_PACKETS)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(AM7990, am7990_device, "am7990", "Am7990 LANCE Ethernet Controller")
DEFINE_DEVICE_TYPE(AM79C90, am79c90_device, "am79c90", "Am79C90 C-LANCE Ethernet Controller")

am7990_device_base::am7990_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_network_interface(mconfig, *this, 10)
	, m_intr_out_cb(*this)
	, m_dma_in_cb(*this, 0)
	, m_dma_out_cb(*this)
	, m_transmit_poll(nullptr)
	, m_intr_out_state(1)
{
}

am7990_device::am7990_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: am7990_device_base(mconfig, AM7990, tag, owner, clock)
{
}

am79c90_device::am79c90_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: am7990_device_base(mconfig, AM79C90, tag, owner, clock)
{
}

const u8 am7990_device_base::ETH_BROADCAST[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
constexpr attotime am7990_device_base::TX_POLL_PERIOD;

void am7990_device_base::device_start()
{
	m_transmit_poll = timer_alloc(FUNC(am7990_device_base::transmit_poll), this);
	m_transmit_poll->adjust(TX_POLL_PERIOD, 0, TX_POLL_PERIOD);

	save_item(NAME(m_rap));
	save_item(NAME(m_csr));

	save_item(NAME(m_mode));
	save_item(NAME(m_logical_addr_filter));
	save_item(NAME(m_physical_addr));

	save_item(NAME(m_rx_ring_base));
	save_item(NAME(m_rx_ring_mask));
	save_item(NAME(m_rx_ring_pos));
	save_item(NAME(m_rx_md));

	save_item(NAME(m_tx_ring_base));
	save_item(NAME(m_tx_ring_mask));
	save_item(NAME(m_tx_ring_pos));
	save_item(NAME(m_tx_md));

	save_item(NAME(m_intr_out_state));
	save_item(NAME(m_idon));
	save_item(NAME(m_lb_buf));
	save_item(NAME(m_lb_length));
}

void am7990_device_base::device_reset()
{
	m_rap = 0;
	m_csr[0] = CSR0_STOP;
	m_csr[3] = 0;

	m_mode = 0;
	m_lb_length = 0;
	m_idon = false;

	update_interrupts();
}

void am7990_device_base::update_interrupts()
{
	if (m_csr[0] & CSR0_INEA)
	{
		// update intr
		if (bool(m_csr[0] & CSR0_INTR) == m_intr_out_state)
		{
			m_intr_out_state = !m_intr_out_state;
			m_intr_out_cb(m_intr_out_state);

			if (!m_intr_out_state)
				LOG("interrupt asserted\n");
		}
	}
	else
	{
		// deassert intr
		if (!m_intr_out_state)
		{
			m_intr_out_state = !m_intr_out_state;
			m_intr_out_cb(m_intr_out_state);
		}
	}
}

int am7990_device_base::recv_start_cb(u8 *buf, int length)
{
	// check internal loopback
	if ((m_mode & MODE_LOOP) && (m_mode & MODE_INTL))
	{
		LOGMASKED(LOG_RXTX, "receive internal loopback mode, external packet discarded\n");

		return 0;
	}

	// discard runt packets
	if (length < 64)
	{
		LOGMASKED(LOG_RXTX, "receive runt packet length %d discarded\n", length);

		return 0;
	}

	return receive(buf, length);
}

int am7990_device_base::receive(u8 *buf, int length)
{
	// check receiver enabled
	if (!(m_csr[0] & CSR0_RXON))
	{
		LOGMASKED(LOG_RXTX, "receive disabled, external packet discarded\n");

		return -1;
	}

	// address filter
	if (!address_filter(buf))
		return -1;

	LOGMASKED(LOG_RXTX, "receive packet length %d\n", length);
	dump_bytes(buf, length);

	// check we have a buffer
	u32 ring_address = (m_rx_ring_base + (m_rx_ring_pos << 3)) & RING_ADDR_MASK;
	m_rx_md[1] = m_dma_in_cb(ring_address | 2);

	if (!(m_rx_md[1] & RMD1_OWN))
		return -2;

	// flag start of packet
	m_rx_md[1] |= RMD1_STP;

	int offset = 0;
	while (offset < length)
	{
		// read rmd0 and rmd2
		m_rx_md[0] = m_dma_in_cb(ring_address | 0);
		m_rx_md[2] = m_dma_in_cb(ring_address | 4);

		u32 const rx_buf_address = (u32(m_rx_md[1] & 0xff) << 16) | m_rx_md[0];
		int const rx_buf_length = get_buf_length(m_rx_md[2]);

		// FIXME: In the C-LANCE device, the case of all 0's in the receive
		// descriptor may produce unpredictable results.

		// write the data to memory
		int const count = std::min(length - offset, rx_buf_length);
		dma_out(rx_buf_address, &buf[offset], count);
		offset += count;

		LOGMASKED(LOG_RXTX, "receive buffer address 0x%06x length %d wrote %d\n", rx_buf_address, rx_buf_length, count);

		// clear ownership
		m_rx_md[1] &= ~RMD1_OWN;

		if (offset < length)
		{
			// look ahead to next descriptor
			u8 const next_ring_pos = (m_rx_ring_pos + 1) & m_rx_ring_mask;
			u32 const next_ring_address = (m_rx_ring_base + (next_ring_pos << 3)) & RING_ADDR_MASK;

			// only read the next descriptor if there's more than one in the ring
			u16 const next_rmd1 = (next_ring_address != ring_address) ? m_dma_in_cb(next_ring_address | 2) : m_rx_md[1];

			// check ownership of the next descriptor
			if (next_rmd1 & RMD1_OWN)
			{
				// update the descriptor
				m_dma_out_cb(ring_address | 2, m_rx_md[1]);

				// advance the ring
				m_rx_ring_pos = next_ring_pos;
				ring_address = next_ring_address;
				m_rx_md[1] = next_rmd1;
			}
			else
			{
				// overflow error
				m_rx_md[1] |= RMD1_ERR | RMD1_BUFF;
				break;
			}
		}
	}

	if (offset == length)
	{
		// check fcs
		if (!(m_mode & MODE_LOOP) || (m_mode & MODE_DTCR))
		{
			u32 const crc = util::crc32_creator::simple(buf, length);

			if (~crc != FCS_RESIDUE)
			{
				LOGMASKED(LOG_RXTX, "receive incorrect fcs 0x%08x\n", ~crc);

				m_rx_md[1] |= RMD1_ERR | RMD1_CRC;
			}
		}

		m_rx_md[1] |= RMD1_ENP;
	}

	return offset;
}

void am7990_device_base::recv_complete_cb(int result)
{
	switch (result)
	{
	case -2: // missed packet
		m_csr[0] |= CSR0_ERR | CSR0_MISS;
		break;

	case -1: // packet discarded or filtered
		return;

	default: // received something
		{
			// update the final descriptor
			u32 const ring_address = (m_rx_ring_base + (m_rx_ring_pos << 3)) & RING_ADDR_MASK;

			LOGMASKED(LOG_RXTX, "receive complete rmd1 0x%04x rmd3 %d\n", m_rx_md[1], result & RMD3_MCNT);

			m_dma_out_cb(ring_address | 2, m_rx_md[1]);
			m_dma_out_cb(ring_address | 6, (m_rx_md[1] & RMD1_ERR) ? 0 : (result & RMD3_MCNT));

			// advance the ring
			m_rx_ring_pos = (m_rx_ring_pos + 1) & m_rx_ring_mask;
		}
		break;
	}

	// generate receive interrupt
	m_csr[0] |= CSR0_RINT | CSR0_INTR;
	update_interrupts();
}

void am7990_device_base::transmit_poll(s32 param)
{
	// check transmitter enabled
	if (m_csr[0] & CSR0_TXON)
	{
		// clear transmit demand
		m_csr[0] &= ~CSR0_TDMD;

		// read a transmit descriptor
		u32 const ring_address = (m_tx_ring_base + (m_tx_ring_pos << 3)) & RING_ADDR_MASK;
		m_tx_md[1] = m_dma_in_cb(ring_address | 2);

		// check ownership
		if (m_tx_md[1] & TMD1_OWN)
		{
			// check for start of packet
			if (!(m_tx_md[1] & TMD1_STP))
			{
				// clear ownership
				m_dma_out_cb(ring_address | 2, m_tx_md[1] & ~TMD1_OWN);

				// advance the ring
				m_tx_ring_pos = (m_tx_ring_pos + 1) & m_tx_ring_mask;
			}
			else
				transmit();
		}
	}

	// receive pending loopback data
	if (m_lb_length && (m_mode & MODE_LOOP))
	{
		LOGMASKED(LOG_RXTX, "receive loopback packet length %d\n", m_lb_length);

		int const result = receive(m_lb_buf, m_lb_length);
		m_lb_length = 0;
		recv_complete_cb(result);
	}
}

void am7990_device_base::transmit()
{
	// stop transmit polling
	m_transmit_poll->enable(false);

	// check whether to append fcs
	bool append_fcs = !(m_mode & MODE_DTCR);

	// this bit can be used to detect C-LANCE
	if (type() == AM79C90)
		append_fcs = append_fcs || bool(m_tx_md[1] & TMD1_ADD_FCS);
	else
		m_tx_md[1] &= ~TMD1_ADD_FCS;

	u32 ring_address = (m_tx_ring_base + (m_tx_ring_pos << 3)) & RING_ADDR_MASK;
	u8 buf[4096];
	int length = 0;

	while (true)
	{
		// read tmd0 and tmd2
		m_tx_md[0] = m_dma_in_cb(ring_address | 0);
		m_tx_md[2] = m_dma_in_cb(ring_address | 4);
		m_tx_md[3] = 0;

		u32 const tx_buf_address = (u32(m_tx_md[1] & TMD1_HADR) << 16) | m_tx_md[0];
		int const tx_buf_length = get_buf_length(m_tx_md[2]);

		LOGMASKED(LOG_RXTX, "transmit buffer address 0x%06x length %d%s%s%s\n", tx_buf_address, tx_buf_length,
			m_tx_md[1] & TMD1_OWN ? " OWN" : "", m_tx_md[1] & TMD1_STP ? " STP" : "", m_tx_md[1] & TMD1_ENP ? " ENP" : "");

		// clear ownership
		m_tx_md[1] &= ~TMD1_OWN;

		// FIXME: zero length transmit buffer
		if (tx_buf_length == 0)
		{
			// update the descriptor
			m_dma_out_cb(ring_address | 2, m_tx_md[1]);

			// advance the ring
			m_tx_ring_pos = (m_tx_ring_pos + 1) & m_tx_ring_mask;

			return;
		}

		// minimum length 100 when chaining, or 64 when not, except in loopback mode
		if (!length && !(m_mode & MODE_LOOP) && tx_buf_length < ((m_tx_md[1] & TMD1_ENP) ? (append_fcs ? 60 : 64) : 100))
			logerror("first transmit buffer length %d less than required minimum\n", tx_buf_length);

		// read the data from memory
		dma_in(tx_buf_address, &buf[length], tx_buf_length);
		length += tx_buf_length;

		// check for end of packet
		if (!(m_tx_md[1] & TMD1_ENP))
		{
			// look ahead to next descriptor
			u8 const next_ring_pos = (m_tx_ring_pos + 1) & m_tx_ring_mask;
			u32 const next_ring_address = (m_tx_ring_base + (next_ring_pos << 3)) & RING_ADDR_MASK;

			// only read the next descriptor if there's more than one in the ring
			u16 const next_tmd1 = (next_ring_address != ring_address) ? m_dma_in_cb(next_ring_address | 2) : m_tx_md[1];

			if (next_tmd1 & TMD1_OWN)
			{
				// update the descriptor
				m_dma_out_cb(ring_address | 2, m_tx_md[1]);

				// advance the ring
				m_tx_ring_pos = next_ring_pos;
				ring_address = next_ring_address;
				m_tx_md[1] = next_tmd1;
			}
			else
			{
				// buffer error
				m_tx_md[1] |= TMD1_ERR;
				m_tx_md[3] |= TMD3_BUFF | TMD3_UFLO;
				m_dma_out_cb(ring_address | 6, m_tx_md[3]);

				// turn off the transmitter
				m_csr[0] &= ~CSR0_TXON;
				break;
			}
		}
		else
			break;
	}

	// check for babble
	if (length > 1518)
		m_csr[0] |= CSR0_ERR | CSR0_BABL;

	// compute and append the fcs
	if (append_fcs)
	{
		u32 const crc = util::crc32_creator::simple(buf, length);

		// insert the fcs
		put_u32le(&buf[length], crc);
		length += 4;
	}

	LOGMASKED(LOG_RXTX, "transmit sending packet length %d\n", length);
	dump_bytes(buf, length);

	// handle loopback
	if (m_mode & MODE_LOOP)
	{
		// forced collision
		if ((m_mode & MODE_COLL) && (m_mode & MODE_INTL))
		{
			send_complete_cb(-1);
			return;
		}

		int const fcs_length = append_fcs ? 4 : 0;

		if ((length - fcs_length) < 8 || (length - fcs_length) > 32)
		{
			logerror("transmit invalid loopback packet length %d\n", length - fcs_length);

			// FIXME: don't know what to do, so just drop the packet
			send_complete_cb(-2);
			return;
		}

		memcpy(m_lb_buf, buf, length);
		m_lb_length = length;

		if (m_mode & MODE_INTL)
		{
			send_complete_cb(length);
			return;
		}
	}

	send(buf, length, 4);
}

void am7990_device_base::send_complete_cb(int result)
{
	u32 const ring_address = (m_tx_ring_base + (m_tx_ring_pos << 3)) & RING_ADDR_MASK;

	// update tmd3 on error
	switch (result)
	{
	case -2: // invalid loopback packet
		m_tx_md[1] |= TMD1_ERR;
		break;

	case -1: // forced collision
		m_tx_md[1] |= TMD1_ERR;
		m_tx_md[3] |= TMD3_RTRY;
		m_dma_out_cb(ring_address | 6, m_tx_md[3]);
		break;

	case 0: // failure to transmit (assume loss of carrier)
		m_tx_md[1] |= TMD1_ERR;
		m_tx_md[3] |= TMD3_LCAR;
		m_dma_out_cb(ring_address | 6, m_tx_md[3]);
		break;
	}

	LOGMASKED(LOG_RXTX, "transmit complete tmd1 0x%04x tmd3 0x%04x\n", m_tx_md[1], m_tx_md[3]);

	// update the last descriptor
	m_dma_out_cb(ring_address | 2, m_tx_md[1]);

	// advance the ring
	m_tx_ring_pos = (m_tx_ring_pos + 1) & m_tx_ring_mask;

	// generate transmit interrupt
	m_csr[0] |= CSR0_TINT | CSR0_INTR;
	update_interrupts();

	// resume transmit polling (back-to-back)
	m_transmit_poll->adjust(attotime::zero, 0, TX_POLL_PERIOD);
}

u16 am7990_device_base::regs_r(address_space &space, offs_t offset)
{
	if (!offset)
	{
		LOGMASKED(LOG_REG, "regs_r csr%d data 0x%04x (%s)\n", m_rap, m_csr[m_rap], machine().describe_context());

		if (m_rap && !(m_csr[0] & CSR0_STOP))
			return space.unmap();
		else
			return m_csr[m_rap];
	}
	else
		return m_rap;
}

void am7990_device_base::regs_w(offs_t offset, u16 data)
{
	if (!offset)
	{
		LOGMASKED(LOG_REG, "regs_w csr%d data 0x%04x (%s)\n", m_rap, data, machine().describe_context());

		switch (m_rap)
		{
		case 0: // Control/Status
			/*
			 * All bits are cleared by reset or STOP, except for STOP which is set.
			 *
			 *   INIT, STRT, STOP, TDMD - read/write with 1 only
			 *   TXON, RXON, INTR, ERR - read only
			 *   INEA - read/write
			 *   IDON, TINT, RINT, MERR, MISS, CERR, BABL - read/clear only
			 *
			 */

			// STOP takes priority over all other bits
			if (data & CSR0_STOP)
			{
				if (!(m_csr[0] & CSR0_STOP))
					device_reset();
				break;
			}

			// interrupt/error flags are all cleared by writing 1
			m_csr[0] &= ~(data & (CSR0_BABL | CSR0_CERR | CSR0_MISS | CSR0_MERR | CSR0_RINT | CSR0_TINT | CSR0_IDON));

			// handle INIT
			if ((data & CSR0_INIT) && !(m_csr[0] & CSR0_INIT))
			{
				if (m_csr[0] & CSR0_STOP)
					initialize();
				else
					m_csr[0] |= m_idon ? CSR0_IDON : CSR0_INIT;
			}

			/*
			 * From the Am7990 datasheet:
			 *
			 * The STOP bit must be set prior to setting the STRT bit. INIT
			 * and STRT must not be set at the same time. The LANCE must be
			 * initialized first and the user must wait for the IDON bit to
			 * be set (IDON=1) before setting the STRT bit.
			 *
			 * And:
			 *
			 * The STOP bit must be set prior to setting the INIT bit.
			 * Setting INIT clears the STOP bit.
			 *
			 * This is clearly contradictory; driver code sets the STRT bit
			 * after INIT, so assume STRT does not require STOP to be set.
			 *
			 * HP9000/3xx diagnostic sets INIT and STRT simultaneously.
			 */
			// handle STRT
			if ((data & CSR0_STRT) && !(m_csr[0] & CSR0_STRT))
			{
				LOG("START receiver %s transmitter %s\n",
					(m_mode & MODE_DRX) ? "OFF" : "ON", (m_mode & MODE_DTX) ? "OFF" : "ON");

				m_csr[0] |= CSR0_STRT;
				m_csr[0] &= ~CSR0_STOP;

				if (m_mode & MODE_DRX)
					m_csr[0] &= ~CSR0_RXON;
				else
					m_csr[0] |= CSR0_RXON;

				if (m_mode & MODE_DTX)
					m_csr[0] &= ~CSR0_TXON;
				else
					m_csr[0] |= CSR0_TXON;

				// trigger an immediate transmit poll
				m_transmit_poll->adjust(attotime::zero, 0, TX_POLL_PERIOD);
			}

			// transmit demand
			if ((data & CSR0_TDMD) && !(m_csr[0] & CSR0_TDMD))
			{
				m_csr[0] |= CSR0_TDMD;
				m_transmit_poll->adjust(attotime::zero, 0, TX_POLL_PERIOD);
			}

			// interrupt enable
			if (!(m_csr[0] & CSR0_STOP) || type() == AM79C90)
			{
				// interrupt enable is read/write
				if ((data ^ m_csr[0]) & CSR0_INEA)
					LOG("interrupts %s\n", data & CSR0_INEA ? "enabled" : "disabled");

				if (data & CSR0_INEA)
					m_csr[0] |= CSR0_INEA;
				else
					m_csr[0] &= ~CSR0_INEA;
			}

			// ERR == BABL || CERR || MISS || MERR
			if (m_csr[0] & CSR0_ANY_ERR)
				m_csr[0] |= CSR0_ERR;
			else
				m_csr[0] &= ~CSR0_ERR;

			// INTR == BABL || MISS || MERR || RINT || TINT || IDON
			if (m_csr[0] & CSR0_ANY_INTR)
				m_csr[0] |= CSR0_INTR;
			else
				m_csr[0] &= ~CSR0_INTR;

			update_interrupts();
			break;

		case 1: // Least significant 15 bits of the Initialization Block
			// Datasheet says "must be zero", but doesn't indicate what
			// happens if it's written non-zero. Must be writable to pass
			// system diagnostic on MIPS RS2030.
			if (m_csr[0] & CSR0_STOP)
				m_csr[1] = data;
			break;

		case 2: // Most significant 8 bits of the Initialization Block
			// The C-LANCE datasheet explicitly states these bits read and
			// write as zero, while LANCE datasheet just says "reserved".
			// MIPS RS2030 diagnostic requires these bits to be writable,
			// so assuming this is older device behaviour.
			if (m_csr[0] & CSR0_STOP)
				m_csr[2] = (type() == AM7990) ? data : (data & 0x00ff);
			break;

		case 3: // Bus master interface
			if (m_csr[0] & CSR0_STOP)
				m_csr[3] = data & CSR3_MASK;
			break;
		}
	}
	else
		m_rap = data & 3;
}

void am7990_device_base::initialize()
{
	u32 init_addr = ((u32(m_csr[2]) << 16) | m_csr[1]) & INIT_ADDR_MASK;
	u16 init_block[12];

	LOG("INITIALIZE initialization block address 0x%08x\n", init_addr);

	for (int i = 0; i < 12; i++)
		init_block[i] = m_dma_in_cb(init_addr + i * 2);

	m_mode = init_block[0];

	set_promisc(m_mode & MODE_PROM);

	put_u16le(&m_physical_addr[0], init_block[1]);
	put_u16le(&m_physical_addr[2], init_block[2]);
	put_u16le(&m_physical_addr[4], init_block[3]);
	set_mac(m_physical_addr);

	m_logical_addr_filter = (u64(init_block[7]) << 48) | (u64(init_block[6]) << 32) | (u32(init_block[5]) << 16) | init_block[4];

	m_rx_ring_base = ((u32(init_block[9]) << 16) | init_block[8]) & RING_ADDR_MASK;
	m_tx_ring_base = ((u32(init_block[11]) << 16) | init_block[10]) & RING_ADDR_MASK;
	m_rx_ring_mask = ~u8(1 << ((init_block[9] >> 13) & 7));
	m_tx_ring_mask = ~u8(1 << ((init_block[11] >> 13) & 7));

	m_tx_ring_pos = 0;
	m_rx_ring_pos = 0;

	LOGMASKED(LOG_INIT, "mode 0x%04x physical address %02x-%02x-%02x-%02x-%02x-%02x\n", m_mode,
		m_physical_addr[0], m_physical_addr[1], m_physical_addr[2], m_physical_addr[3], m_physical_addr[4], m_physical_addr[5]);
	LOGMASKED(LOG_INIT, "logical address filter 0x%016x\n", m_logical_addr_filter);
	LOGMASKED(LOG_INIT, "receive ring address 0x%08x length %d\n", m_rx_ring_base, 1 << ((init_block[9] >> 13) & 7));
	LOGMASKED(LOG_INIT, "transmit ring address 0x%08x length %d\n", m_tx_ring_base, 1 << ((init_block[11] >> 13) & 7));

	m_csr[0] |= CSR0_IDON | CSR0_INIT;
	m_csr[0] &= ~CSR0_STOP;

	m_idon = true;
}

void am7990_device_base::dma_in(u32 address, u8 *buf, int length)
{
	// odd address start
	if (address & 1)
	{
		u16 const word = m_dma_in_cb(address & ~1);

		if (m_csr[3] & CSR3_BSWP)
			buf[0] = word & 0xff;
		else
			buf[0] = word >> 8;

		buf++;
		address++;
		length--;
	}

	// word loop
	while (length > 1)
	{
		u16 const word = m_dma_in_cb(address);

		if (m_csr[3] & CSR3_BSWP)
			put_u16be(&buf[0], word);
		else
			put_u16le(&buf[0], word);

		buf += 2;
		address += 2;
		length -= 2;
	}

	// trailing byte
	if (length)
	{
		u16 const word = m_dma_in_cb(address);

		if (m_csr[3] & CSR3_BSWP)
			buf[0] = word >> 8;
		else
			buf[0] = word & 0xff;

		buf++;
		address++;
		length--;
	}
}

void am7990_device_base::dma_out(u32 address, u8 *buf, int length)
{
	// odd address start
	if (address & 1)
	{
		if (m_csr[3] & CSR3_BSWP)
			m_dma_out_cb(address & ~1, buf[0], 0x00ff);
		else
			m_dma_out_cb(address & ~1, buf[0] << 8, 0xff00);

		buf++;
		address++;
		length--;
	}

	// word loop
	while (length > 1)
	{
		u16 const word = (m_csr[3] & CSR3_BSWP) ? get_u16be(&buf[0]) : get_u16le(&buf[0]);

		m_dma_out_cb(address, word);

		buf += 2;
		address += 2;
		length -= 2;
	}

	// trailing byte
	if (length)
	{
		if (m_csr[3] & CSR3_BSWP)
			m_dma_out_cb(address, buf[0] << 8, 0xff00);
		else
			m_dma_out_cb(address, buf[0], 0x00ff);

		buf++;
		address++;
		length--;
	}
}

void am7990_device_base::dump_bytes(u8 *buf, int length)
{
	if (VERBOSE & LOG_PACKETS)
	{
		// pad with zeros to 8-byte boundary
		for (int i = 0; i < 8 - (length % 8); i++)
			buf[length + i] = 0;

		// dump length / 8 (rounded up) groups of 8 bytes
		for (int i = 0; i < (length + 7) / 8; i++)
			LOGMASKED(LOG_PACKETS, "%02x %02x %02x %02x %02x %02x %02x %02x\n",
				buf[i * 8 + 0], buf[i * 8 + 1], buf[i * 8 + 2], buf[i * 8 + 3],
				buf[i * 8 + 4], buf[i * 8 + 5], buf[i * 8 + 6], buf[i * 8 + 7]);
	}
}

bool am7990_device_base::address_filter(u8 *buf)
{
	if (m_mode & MODE_PROM)
	{
		LOGMASKED(LOG_FILTER, "address_filter accepted (promiscuous mode)\n");

		return true;
	}

	if (buf[0] & 1)
	{
		// broadcast
		if (!memcmp(ETH_BROADCAST, buf, 6))
		{
			LOGMASKED(LOG_FILTER, "address_filter accepted (broadcast) %02x-%02x-%02x-%02x-%02x-%02x\n",
				buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

			return true;
		}

		// multicast
		/*
		 * Multicast address matching is performed by computing the fcs crc of
		 * the destination address, and then using the upper 6 bits as an index
		 * into the 64-bit logical address filter.
		 */
		u32 const crc = util::crc32_creator::simple(buf, 6);
		if (BIT(m_logical_addr_filter, 63 - (crc >> 26)))
		{
			LOGMASKED(LOG_FILTER, "address_filter accepted (logical address match) %02x-%02x-%02x-%02x-%02x-%02x\n",
				buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

			return true;
		}
	}
	else
		// unicast
		if (!memcmp(m_physical_addr, buf, 6))
		{
			LOGMASKED(LOG_FILTER, "address_filter accepted (physical address match)\n");

			return true;
		}

	return false;
}
