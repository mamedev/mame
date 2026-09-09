// license:BSD-3-Clause
// copyright-holders:superctr
/****************************************************************************

    Roland SC-88 sub CPU (M38881M2, Roland R00232667) - high level emulation.

    The chip is a Mitsubishi 3888 group protocol controller whose program
    is not dumped.  On the SC-88 it receives MIDI IN A, MIDI IN B and the
    computer port on its three UARTs, parses the streams and hands the main
    CPU one message per interrupt through the 3888's system bus interface;
    the main CPU scans the front panel switch matrix through the same
    window and sends MIDI OUT data back through a ring in the dual port RAM.

    System bus side of the chip (3888 group data sheet, fig. 16):

      00-d7  dual port RAM         dc-df  IPC error registers (sub -> main)
      d8-db  IPC mode registers    e0-fa  access flags
             (main -> sub)         fd     semaphore, bit 7 = ready flag
                                   fe/ff  port data / port control

    Message protocol as reconstructed from the main CPU program (v1.04):

      IPCER0  code    01-07 channel voice message, (status >> 4) - 7
                      10-12 display exclusive (model 45), selecting the block
                      20-bf GS exclusive (model 42), selecting the parameter block
                      e7    continuation of an exclusive block
                      ee/ef universal exclusive (GM on / master volume)
      IPCER1  flags   bits 0-3 channel, bits 4-5 source (0 IN A, 1 IN B,
                      2 computer), bit 6 more data follows, bit 7 payload
                      is a block in the dual port RAM
      IPCER2/3        data bytes, or block length / offset for a payload

    Payload blocks are the exclusive bytes after F0 up to and including
    the checksum (the F7 is not counted); they are guarded by block semaphore bit 0/1/2 (the source), which
    the main CPU clears after copying the block.  Bit 7 of IPCER2 marks the
    block as a request (RQ1) rather than a data set (DT1); the rest of the
    byte is the length either way.

    The code does not carry the address MSB directly.  It names one of the
    parameter blocks of the address map, and for most of them the low three
    bits are the high nibble of the address middle byte, so a block covers
    eight of them:

      20  00 0n   system            60  40 0n   patch common (A)
      28  08 0n                     68  48 0n
      30  08 0n / 0c 0n             70  50 0n   patch common (B)
      40  20 0n   user tone bank    78  58 0n
      48  28 0n                     80  41 0n   drum setup (A)
      50  21 0n   user drum set     88  49 0n
      58  29 0n                     90  51 0n   drum setup (B)
                                    98  59 0n
      a0  22 0n   user EFX          b0  23 00   user patch common
      a8  2a 0n                     b1  24 00   user patch part
                                    b8  2b 0n

    The display messages of model 45 follow the same scheme from one block:

      10  10 0n   text (00) and the ten dot pages (01-05, 64 bytes each)
      12  10 2n   display page and display time

    The low nibble of the middle byte is not in the block at all: it travels
    in the channel field of IPCER1, which is how a request names one of the
    sixteen parts.  b0-b4 are the exception, a run of address MSBs rather
    than a run of middle bytes; the SC-88 has neither them nor a0/a8/b8.
    30 answers to two addresses depending on the parameter asked for and 38
    to none that could be found, so neither is listed below; both are
    outside the published map.

    A request is answered on MIDI OUT.  The main CPU sends through the ring
    at 24-bf, D5 being its write pointer and D4 the sub CPU's read pointer,
    under block semaphore bit 5.  The ring carries packets, not a byte
    stream: a length byte followed by that many bytes of MIDI, the next
    packet starting at the following multiple of four.  A packet may
    straddle the wrap at c0.

****************************************************************************/

#include "emu.h"
#include "sc88_sub.h"

#define LOG_MSG     (1U << 1)
#define LOG_TX      (1U << 2)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(SC88_SUB, sc88_sub_device, "sc88_sub", "Roland SC-88 sub CPU (HLE)")

namespace {

constexpr u8 SOURCE_FLAGS[3] = { 0x00, 0x10, 0x20 };
constexpr int BLOCK_SIZE = 0x24;        // dual port RAM 00-23 carries payload blocks
constexpr int MAX_EXCLUSIVE = 0x8a;     // the main CPU's reassembly buffer
constexpr int TX_RING_START = 0x24;
constexpr int TX_RING_END = 0xc0;
constexpr int TX_WRITE_PTR = 0xd5;
constexpr int TX_READ_PTR = 0xd4;

struct exclusive_block { u8 model; u8 address; u8 code; bool paged; };

constexpr exclusive_block BLOCKS[] = {
	{ 0x42, 0x00, 0x20, true }, { 0x42, 0x08, 0x28, true },
	{ 0x42, 0x20, 0x40, true }, { 0x42, 0x28, 0x48, true },
	{ 0x42, 0x21, 0x50, true }, { 0x42, 0x29, 0x58, true },
	{ 0x42, 0x40, 0x60, true }, { 0x42, 0x48, 0x68, true },
	{ 0x42, 0x50, 0x70, true }, { 0x42, 0x58, 0x78, true },
	{ 0x42, 0x41, 0x80, true }, { 0x42, 0x49, 0x88, true },
	{ 0x42, 0x51, 0x90, true }, { 0x42, 0x59, 0x98, true },
	{ 0x42, 0x22, 0xa0, true }, { 0x42, 0x2a, 0xa8, true },
	{ 0x42, 0x23, 0xb0, false }, { 0x42, 0x24, 0xb1, false }, { 0x42, 0x25, 0xb2, false },
	{ 0x42, 0x26, 0xb3, false }, { 0x42, 0x27, 0xb4, false },
	{ 0x42, 0x2b, 0xb8, true },
	{ 0x45, 0x10, 0x10, true },
};

} // anonymous namespace


//-------------------------------------------------
//  UART receiver, one per MIDI source
//-------------------------------------------------

DEFINE_DEVICE_TYPE(SC88_SUB_RX, sc88_sub_rx_device, "sc88_sub_rx", "Roland SC-88 sub CPU UART")

sc88_sub_rx_device::sc88_sub_rx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SC88_SUB_RX, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_byte_cb(*this)
{
}

void sc88_sub_rx_device::device_start()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rcv_rate(31250);
}

void sc88_sub_rx_device::device_reset()
{
	receive_register_reset();
}

void sc88_sub_rx_device::rcv_complete()
{
	receive_register_extract();
	m_byte_cb(get_received_char());
}


//-------------------------------------------------
//  sub CPU
//-------------------------------------------------

sc88_sub_device::sc88_sub_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SC88_SUB, tag, owner, clock)
	, m_int_cb(*this)
	, m_keys_cb(*this, 0xff)
	, m_tx_cb(*this)
	, m_rx(*this, "rx%u", 0U)
{
}

void sc88_sub_device::device_add_mconfig(machine_config &config)
{
	SC88_SUB_RX(config, m_rx[0], 0).byte_callback().set(FUNC(sc88_sub_device::rx_byte<0>));
	SC88_SUB_RX(config, m_rx[1], 0).byte_callback().set(FUNC(sc88_sub_device::rx_byte<1>));
	SC88_SUB_RX(config, m_rx[2], 0).byte_callback().set(FUNC(sc88_sub_device::rx_byte<2>));
}

void sc88_sub_device::device_start()
{
	m_deliver_timer = timer_alloc(FUNC(sc88_sub_device::deliver_timer), this);
	m_tx_timer = timer_alloc(FUNC(sc88_sub_device::tx_timer), this);

	save_item(NAME(m_dpram));
	save_item(NAME(m_ipcm));
	save_item(NAME(m_ipcer));
	save_item(NAME(m_flags));
	save_item(NAME(m_sem));
	save_item(NAME(m_spcon));
	save_item(NAME(m_pa));
	save_item(NAME(m_pa_dir));
	save_item(NAME(m_pb));
	save_item(NAME(m_pb_dir));
	save_item(NAME(m_int_state));
	save_item(NAME(m_in_reset));
	save_item(NAME(m_busy));
	save_item(NAME(m_tx_rd));
	save_item(NAME(m_tx_left));
	save_item(NAME(m_tx_end));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_tx_bits));
}

void sc88_sub_device::device_reset()
{
	std::fill(std::begin(m_dpram), std::end(m_dpram), 0);
	std::fill(std::begin(m_ipcm), std::end(m_ipcm), 0);
	std::fill(std::begin(m_ipcer), std::end(m_ipcer), 0);
	std::fill(std::begin(m_flags), std::end(m_flags), 0);
	m_sem = 0x80; // ready: the sub CPU has finished its own initialisation
	m_spcon = 0;
	m_pa = m_pa_dir = m_pb = m_pb_dir = 0;
	m_int_state = false;
	m_int_cb(0);

	for (source &s : m_src)
		s = source();
	m_queue.clear();
	m_busy = false;
	m_deliver_timer->adjust(attotime::never);

	m_dpram[TX_READ_PTR] = m_dpram[TX_WRITE_PTR] = TX_RING_START;
	m_tx_rd = TX_RING_START;
	m_tx_left = 0;
	m_tx_end = TX_RING_START;
	m_tx_shift = 0;
	m_tx_bits = 0;
	m_tx_timer->adjust(attotime::never);
	m_tx_cb(1);
}


//-------------------------------------------------
//  system bus interface
//-------------------------------------------------

u8 sc88_sub_device::port_r()
{
	switch ((m_spcon >> 2) & 7)
	{
	case 0: return m_pa;
	case 1: return m_pa_dir;
	case 2: return m_pb;
	case 3: return m_pb_dir;
	case 4:
	{
		// P0 returns the switch matrix rows selected by the strobes on PB0-PB3
		u8 keys = 0xff;
		for (int row = 0; row < 4; row++)
			if (BIT(m_pb, row))
				keys &= m_keys_cb[row]();
		return keys;
	}
	default: return 0xff;
	}
}

void sc88_sub_device::port_w(u8 data)
{
	switch ((m_spcon >> 2) & 7)
	{
	case 0: m_pa = data; break;
	case 1: m_pa_dir = data; break;
	case 2: m_pb = data; break;
	case 3: m_pb_dir = data; break;
	default: break;
	}
}

void sc88_sub_device::reset_w(int state)
{
	if (m_in_reset == !state)
		return;
	m_in_reset = !state;
	if (m_in_reset)
		reset();
}

u8 sc88_sub_device::read(offs_t offset)
{
	if (offset < 0xd8)
	{
		if (!machine().side_effects_disabled())
			m_flags[offset >> 3] &= ~(1 << (offset & 7));
		return m_dpram[offset];
	}
	if (offset < 0xdc)
		return m_ipcm[offset & 3];
	if (offset < 0xe0)
	{
		const u8 data = m_ipcer[offset & 3];
		if (!machine().side_effects_disabled())
		{
			m_ipcer[offset & 3] = 0;
			if (offset == 0xdc && m_int_state)
			{
				// the message has been taken; the next one may follow
				m_int_state = false;
				m_int_cb(0);
				m_deliver_timer->adjust(attotime::from_usec(20));
			}
		}
		return data;
	}
	if (offset < 0xfb)
		return m_flags[offset - 0xe0];
	if (offset == 0xfd)
		return m_sem;
	if (offset == 0xfe)
		return port_r();
	if (offset == 0xff)
		return m_spcon;
	return 0xff;
}

void sc88_sub_device::write(offs_t offset, u8 data)
{
	if (offset < 0xd8)
	{
		m_dpram[offset] = data;
		m_flags[offset >> 3] |= 1 << (offset & 7);
		if (offset == TX_WRITE_PTR && m_tx_timer->expire().is_never())
			m_tx_timer->adjust(attotime::zero);
		return;
	}

	if (offset < 0xdc)
	{
		m_ipcm[offset & 3] = data;
		if (offset == 0xd8)
			m_sem &= 0x7f;
		LOG("%s: IPC mode register %d = %02x\n", machine().describe_context(), offset & 3, data);
	}
	else if (offset == 0xfd)
	{
		const u8 bit = 1 << (data & 7);
		if (BIT(data, 7))
			m_sem |= bit;
		else
			m_sem &= ~bit;
		if (!m_busy && !m_queue.empty())
			m_deliver_timer->adjust(attotime::from_usec(20));
	}
	else if (offset == 0xfe)
		port_w(data);
	else if (offset == 0xff)
		m_spcon = data;
}


//-------------------------------------------------
//  MIDI input parsing
//-------------------------------------------------

void sc88_sub_device::midi_byte(int src, u8 data)
{
	source &s = m_src[src];

	if (data >= 0xf8)
		return; // system realtime

	if (data >= 0x80)
	{
		if (s.in_sysex)
		{
			if (data == 0xf7)
			{
				s.sysex.push_back(data);
				send_sysex(src, s);
			}
			s.in_sysex = false;
			s.sysex.clear();
		}
		if (data == 0xf0)
		{
			s.in_sysex = true;
			s.status = 0;
		}
		else if (data < 0xf0)
		{
			s.status = data;
			s.count = 0;
		}
		else
			s.status = 0; // other system common messages cancel running status
		return;
	}

	if (s.in_sysex)
	{
		if (s.sysex.size() < MAX_EXCLUSIVE + 8)
			s.sysex.push_back(data);
		return;
	}
	if (s.status == 0)
		return;

	s.data[s.count++] = data;
	const int type = s.status >> 4;
	const int length = (type == 0xc || type == 0xd) ? 1 : 2;
	if (s.count < length)
		return;
	s.count = 0;

	queue(type - 7, (s.status & 0x0f) | SOURCE_FLAGS[src], s.data[0], (length == 2) ? s.data[1] : 0);
}

// s.sysex holds the bytes after F0, ending with F7
void sc88_sub_device::send_sysex(int src, source &s)
{
	const std::vector<u8> &x = s.sysex;
	u8 code;
	u8 chan = 0;
	u8 request = 0;
	if (x.size() >= 8 && x[0] == 0x41 && (x[2] == 0x42 || x[2] == 0x45) && (x[3] == 0x12 || x[3] == 0x11))
	{
		const exclusive_block *block = nullptr;
		for (const exclusive_block &b : BLOCKS)
			if (b.model == x[2] && b.address == x[4] && (b.paged || (x[5] & 0xf0) == 0))
				block = &b;
		if (!block)
		{
			LOGMASKED(LOG_MSG, "exclusive model %02x address %02x %02x has no block\n", x[2], x[4], x[5]);
			return;
		}
		code = block->code + (block->paged ? (x[5] >> 4) : 0);
		chan = x[5] & 0x0f;
		request = (x[3] == 0x11) ? 0x80 : 0x00;
	}
	else if (x.size() >= 5 && x[0] == 0x7e)
		code = 0xee;
	else if (x.size() >= 5 && x[0] == 0x7f)
		code = 0xef;
	else
		return;

	if (x.size() > MAX_EXCLUSIVE)
	{
		LOGMASKED(LOG_MSG, "exclusive message of %d bytes dropped\n", int(x.size()));
		return;
	}

	// long messages are split into blocks the main CPU reassembles; the
	// length counts the bytes up to the checksum, not the F7
	const size_t total = x.size() - 1;
	for (size_t pos = 0; pos < total; pos += BLOCK_SIZE)
	{
		const size_t len = std::min<size_t>(BLOCK_SIZE, total - pos);
		const bool more = pos + len < total;
		queue(pos ? 0xe7 : code, SOURCE_FLAGS[src] | 0x80 | (more ? 0x40 : 0) | chan,
				u8(len) | (pos ? 0 : request), 0,
				std::vector<u8>(x.begin() + pos, x.begin() + pos + len));
	}
}


//-------------------------------------------------
//  IPC messages to the main CPU
//-------------------------------------------------

void sc88_sub_device::queue(u8 code, u8 flags, u8 d1, u8 d2, std::vector<u8> &&block)
{
	m_queue.push_back(message{ code, flags, d1, d2, std::move(block) });
	if (!m_busy)
		deliver();
}

void sc88_sub_device::deliver()
{
	if (m_busy || m_queue.empty())
		return;

	message &m = m_queue.front();
	const int src = (m.flags >> 4) & 3;
	if (!m.block.empty())
	{
		// the previous block of this source must have been consumed
		if (BIT(m_sem, src))
		{
			m_deliver_timer->adjust(attotime::from_usec(100));
			return;
		}
		std::copy(m.block.begin(), m.block.end(), &m_dpram[0]);
		m_sem |= 1 << src;
	}

	LOGMASKED(LOG_MSG, "message %02x %02x %02x %02x%s\n", m.code, m.flags, m.d1, m.d2, m.block.empty() ? "" : " (block)");
	m_ipcer[0] = m.code;
	m_ipcer[1] = m.flags;
	m_ipcer[2] = m.d1;
	m_ipcer[3] = m.d2;
	m_queue.pop_front();
	m_busy = true;
	m_int_state = true;
	m_int_cb(1);
}

TIMER_CALLBACK_MEMBER(sc88_sub_device::deliver_timer)
{
	m_busy = false;
	deliver();
}


//-------------------------------------------------
//  MIDI OUT: bytes queued by the main CPU in the dual port RAM ring
//-------------------------------------------------

u8 sc88_sub_device::ring_advance(u8 offset, u8 count)
{
	offset += count;
	if (offset >= TX_RING_END)
		offset -= TX_RING_END - TX_RING_START;
	return offset;
}

TIMER_CALLBACK_MEMBER(sc88_sub_device::tx_timer)
{
	if (m_tx_bits)
	{
		m_tx_cb(BIT(m_tx_shift, 0));
		m_tx_shift >>= 1;
		m_tx_bits--;
		m_tx_timer->adjust(attotime::from_hz(31250));
		return;
	}

	while (!m_tx_left)
	{
		if (m_tx_rd == m_dpram[TX_WRITE_PTR])
			return;
		m_tx_left = m_dpram[m_tx_rd];
		m_tx_end = ring_advance(m_tx_rd, (m_tx_left + 4) & ~3);
		m_tx_rd = ring_advance(m_tx_rd, 1);
		if (!m_tx_left)
			m_dpram[TX_READ_PTR] = m_tx_rd = m_tx_end;
	}

	const u8 data = m_dpram[m_tx_rd];
	LOGMASKED(LOG_TX, "MIDI OUT %02x\n", data);
	m_tx_rd = ring_advance(m_tx_rd, 1);
	if (!--m_tx_left)
		m_dpram[TX_READ_PTR] = m_tx_rd = m_tx_end;

	// start bit, 8 data bits, stop bit
	m_tx_shift = (u16(data) << 1) | 0x200;
	m_tx_bits = 10;
	m_tx_timer->adjust(attotime::zero);
}
