// license:BSD-3-Clause
// copyright-holders:Niko Nieminen
#include "emu.h"
#include "cable.h"

namespace {

// record layout: type, value, 64-bit little endian time (ns)
constexpr unsigned RECORD = 10;

// from the DTE side to the line (serial_link sends these)
constexpr uint8_t T_HEARTBEAT = 0x00;
constexpr uint8_t T_TXD       = 0x01;
constexpr uint8_t T_RTS       = 0x02;
constexpr uint8_t T_DTR       = 0x03;

// from the line to the DTE (null_cable sends these, both devices receive them)
constexpr uint8_t T_RXD       = 0x11;
constexpr uint8_t T_CTS       = 0x12;
constexpr uint8_t T_DSR       = 0x13;
constexpr uint8_t T_DCD       = 0x14;

constexpr unsigned POLL_US = 500;        // stream polling period (emulated time)
constexpr unsigned HEARTBEAT_TICKS = 4;  // every 2 ms
constexpr unsigned REFRESH_TICKS = 100;  // every 50 ms
constexpr uint32_t MAX_BUFFER = 1 << 20;

} // anonymous namespace


static INPUT_PORTS_START(rs232_cable)
	PORT_START("LATENCY")
	PORT_CONFNAME(0x07, 0x05, "Latency")
	PORT_CONFSETTING(   0x00, "5 ms")
	PORT_CONFSETTING(   0x01, "10 ms")
	PORT_CONFSETTING(   0x02, "20 ms")
	PORT_CONFSETTING(   0x03, "50 ms")
	PORT_CONFSETTING(   0x04, "100 ms")
	PORT_CONFSETTING(   0x05, "200 ms")
	PORT_CONFSETTING(   0x06, "500 ms")
	PORT_CONFSETTING(   0x07, "1000 ms")
INPUT_PORTS_END

ioport_constructor rs232_cable_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(rs232_cable);
}

rs232_cable_device::rs232_cable_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool crossed) :
	device_t(mconfig, type, tag, owner, clock),
	device_rs232_port_interface(mconfig, *this),
	m_stream(*this, "stream"),
	m_latency(*this, "LATENCY"),
	m_poll(nullptr),
	m_play(nullptr),
	m_crossed(crossed),
	m_offset(0),
	m_offset_est(0),
	m_last_play(0),
	m_synced(false),
	m_tick(0),
	m_late(0),
	m_txd(1),
	m_rts(1),
	m_dtr(1)
{
}

void rs232_cable_device::device_add_mconfig(machine_config &config)
{
	BITBANGER(config, m_stream);
}

void rs232_cable_device::device_start()
{
	m_poll = timer_alloc(FUNC(rs232_cable_device::poll), this);
	m_play = timer_alloc(FUNC(rs232_cable_device::play), this);

	save_item(NAME(m_txd));
	save_item(NAME(m_rts));
	save_item(NAME(m_dtr));
}

void rs232_cable_device::device_reset()
{
	m_rxbuf.clear();
	m_events.clear();
	m_synced = false;
	m_offset = 0;
	m_offset_est = 0;
	m_last_play = 0;
	m_tick = 0;
	m_late = 0;
	m_txd = m_rts = m_dtr = 1;

	// idle line, handshake inputs inactive until the other end says otherwise
	output_rxd(1);
	output_dcd(1);
	output_dsr(1);
	output_cts(1);
	output_ri(1);

	m_play->adjust(attotime::never);
	m_poll->adjust(attotime::from_usec(POLL_US), 0, attotime::from_usec(POLL_US));
}

int64_t rs232_cable_device::now_ns() const
{
	attotime const t = machine().time();
	return int64_t(t.seconds()) * 1000000000 + int64_t(t.attoseconds() / 1000000000);
}

void rs232_cable_device::send(uint8_t type, int value)
{
	uint8_t record[RECORD];
	int64_t const t = now_ns();
	record[0] = type;
	record[1] = value ? 1 : 0;
	for (int i = 0; i < 8; i++)
		record[2 + i] = uint8_t(uint64_t(t) >> (8 * i));
	for (unsigned i = 0; i < RECORD; i++)
		m_stream->output(record[i]);
}

void rs232_cable_device::input_txd(int state)
{
	if (state == m_txd)
		return;
	m_txd = state;
	send(m_crossed ? T_RXD : T_TXD, state);
}

void rs232_cable_device::input_rts(int state)
{
	if (state == m_rts)
		return;
	m_rts = state;
	send(m_crossed ? T_CTS : T_RTS, state);
}

void rs232_cable_device::input_dtr(int state)
{
	if (state == m_dtr)
		return;
	m_dtr = state;
	if (m_crossed)
	{
		send(T_DSR, state);
		send(T_DCD, state);
	}
	else
	{
		send(T_DTR, state);
	}
}

TIMER_CALLBACK_MEMBER(rs232_cable_device::poll)
{
	uint8_t buffer[512];
	for (;;)
	{
		uint32_t const n = m_stream->input(buffer, sizeof(buffer));
		if (n == 0)
			break;
		if (m_rxbuf.size() + n <= MAX_BUFFER)
			m_rxbuf.insert(m_rxbuf.end(), buffer, buffer + n);
		if (n < sizeof(buffer))
			break;
	}
	parse();

	if ((m_tick % HEARTBEAT_TICKS) == 0)
		send(T_HEARTBEAT, 0);

	// Control lines are levels, not events: refresh them every 50 ms so that a peer that was reset
	// (which puts its inputs back to inactive) learns the current state again.
	if ((m_tick % REFRESH_TICKS) == 0)
	{
		if (m_crossed)
		{
			send(T_CTS, m_rts);
			send(T_DSR, m_dtr);
			send(T_DCD, m_dtr);
		}
		else
		{
			send(T_RTS, m_rts);
			send(T_DTR, m_dtr);
		}
	}
	m_tick++;
}

void rs232_cable_device::parse()
{
	size_t pos = 0;
	while (m_rxbuf.size() - pos >= RECORD)
	{
		uint8_t const *r = &m_rxbuf[pos];
		uint64_t t = 0;
		for (int i = 0; i < 8; i++)
			t |= uint64_t(r[2 + i]) << (8 * i);
		schedule(int64_t(t), r[0], r[1]);
		pos += RECORD;
	}
	if (pos)
		m_rxbuf.erase(m_rxbuf.begin(), m_rxbuf.begin() + pos);
}

void rs232_cable_device::schedule(int64_t remote_time, uint8_t type, uint8_t value)
{
	int64_t const now = now_ns();

	// Only heartbeats are stamped with the sender's current time, so only they tell the clock offset;
	// edge records may carry future timestamps (a bridge pre-computes whole frames).
	// The two emulated clocks are throttled to real time in frame-sized steps, so the apparent offset
	// jitters by tens of milliseconds.  Estimate the worst case (maximum, slowly decaying) and apply a new
	// value only while no edges are pending: changing it in the middle of a frame would distort the bit times.
	if (type == T_HEARTBEAT)
	{
		int64_t const candidate = now - remote_time;   // true offset plus transport delay and jitter
		if (!m_synced)
		{
			m_offset_est = m_offset = candidate;
			m_synced = true;
		}
		else if (candidate > m_offset_est)
		{
			m_offset_est = candidate;
		}
		else
		{
			m_offset_est -= (m_offset_est - candidate) >> 12;
		}
		return;
	}

	if (!m_synced)
	{
		m_offset_est = m_offset = now - remote_time;
		m_synced = true;
	}
	else if (m_events.empty() && m_last_play + 5000000 < now)
	{
		if (std::abs(m_offset_est - m_offset) > 1000000)
			osd_printf_warning("cable: resync offset %lld -> %lld ns at %lld\n", (long long)m_offset, (long long)m_offset_est, (long long)now);
		m_offset = m_offset_est;                        // line has been idle: safe to re-sync
	}

	static constexpr int64_t latency_ms[8] = { 5, 10, 20, 50, 100, 200, 500, 1000 };
	int64_t play_at = remote_time + m_offset + latency_ms[m_latency->read() & 7] * 1000000;

	if (play_at < now)
	{
		// too late: widen the margin so that later edges are not squeezed
		int64_t const widen = (now - play_at) + 2000000;
		m_offset += widen;
		m_offset_est += widen;
		play_at = now;
		m_late++;
		if (m_late <= 20 || (m_late % 1000) == 0)
			osd_printf_warning("cable: late edge #%u by %lld ns, offset now %lld\n", m_late, (long long)(widen - 2000000), (long long)m_offset);
	}
	if (play_at <= m_last_play)
	{
		if (play_at < m_last_play - 1000)
			osd_printf_warning("cable: edge order clamp, %lld ns back (type %02x) at %lld\n", (long long)(m_last_play - play_at), type, (long long)now);
		play_at = m_last_play + 1;
	}
	m_last_play = play_at;

	bool const was_empty = m_events.empty();
	m_events.push_back({ play_at, type, value });
	if (was_empty)
		arm_play();
}

void rs232_cable_device::arm_play()
{
	if (m_events.empty())
	{
		m_play->adjust(attotime::never);
		return;
	}
	int64_t const delta = m_events.front().time - now_ns();
	m_play->adjust(attotime::from_nsec(delta > 0 ? delta : 0));
}

void rs232_cable_device::apply(uint8_t type, uint8_t value)
{
	switch (type)
	{
	case T_RXD: output_rxd(value); break;
	case T_CTS: output_cts(value); break;
	case T_DSR: output_dsr(value); break;
	case T_DCD: output_dcd(value); break;
	default: break;
	}
}

TIMER_CALLBACK_MEMBER(rs232_cable_device::play)
{
	int64_t const now = now_ns();
	while (!m_events.empty() && m_events.front().time <= now)
	{
		event const e = m_events.front();
		m_events.pop_front();
		apply(e.type, e.value);
	}
	arm_play();
}


null_cable_device::null_cable_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	rs232_cable_device(mconfig, NULL_CABLE, tag, owner, clock, true)
{
}

serial_link_device::serial_link_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	rs232_cable_device(mconfig, SERIAL_LINK, tag, owner, clock, false)
{
}

DEFINE_DEVICE_TYPE(NULL_CABLE, null_cable_device, "null_cable", "RS-232 Null Modem Cable (bit level)")
DEFINE_DEVICE_TYPE(SERIAL_LINK, serial_link_device, "serial_link", "RS-232 Serial Link (bit level bridge)")
