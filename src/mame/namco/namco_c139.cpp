// license:BSD-3-Clause
// copyright-holders:Angelo Salese, pocketjazzy
/***************************************************************************

    Namco C139 - Serial I/F Controller

    QFP64 RS-422 link controller used to network arcade cabinets:
    System 2, 21, 22 / Super 22, and System 23 (where the silkscreen
    labels it "C422", believed to be a pin-compatible faster-clocked
    revision of C139).

    The cabinet-to-cabinet serial line is modelled as a TCP connection
    between two MAME instances: one instance listens, the other connects
    (resolved from the -comm_* options, from a per-machine cfg
    <linkplay> node, or from built-in loopback defaults - see
    start_comm()).  Each staged TX frame is read out of the shared RAM
    and delivered into the peer's RX area, from where the game's own
    protocol layer (marker scanner, checksum validator, dispatcher)
    consumes it exactly as on hardware.

    The TCP transport approach derives from SailorSat (Ariane Fugmann)'s
    C139 link work for the System 21 era boards.

    Behaviour verified against the Time Crisis II (System 23) link code:
    - Register 5 (TX size) holds the frame size in HALFWORDS; each
      halfword of shared RAM crosses the wire as 2 bytes, high byte
      first (matching the host CPU's big-endian halfword layout).
    - The game's frames carry their own framing: the last halfword has
      bit 8 set (end marker) with the frame's halfword count in the
      trailer bytes, and the byte-sum checksum is built so the
      receiver's validator sees sum mod 256 == 0.
    - The game's TX routine splits messages larger than 0xFF halfwords
      into TXSIZE=0xFF chunks plus a remainder, writing TXOFFSET only
      for the first chunk and relying on the chip's auto-advancing DMA
      pointer for the rest; the receive-side validator accepts slot
      lengths in [4..0x400] halfwords (a 0x400-halfword TX window slot).
    - The game declares a link timeout when its drift counter (local
      state counter minus the partner counter carried in bytes 0-1 of
      each validated frame) reaches 17 frames; a validated ingest
      resets it.

    TODO:
    - Verify the register map and timing against other C139 games
      (System 2 / 21).
    - Is RAM shared with a specific CPU other than master/slave?
    - is this another MCU with internal ROM?

***************************************************************************/

#include "emu.h"
#include "namco_c139.h"

#include "config.h"     // per-machine cfg <linkplay> node
#include "emuopts.h"

#include "xmlfile.h"    // util::xml::data_node attribute access

#include "asio.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <mutex>
#include <optional>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#define LOG_LINK    (1U << 1)   // connection / session lifecycle events
#define LOG_WARN    (1U << 2)   // abnormal conditions
#define LOG_FRAME   (1U << 3)   // per-event protocol activity

#define VERBOSE (0)
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(NAMCO_C139, namco_c139_device, "namco_c139", "Namco C139 Serial")

namespace {

// Frame-token lockstep tuning.
//
// Control frames ride the internal TCP framing with BIT 15 OF THE 16-bit
// size prefix SET (game frames are capped at 0x4000 bytes, so bit 15 is
// never set on a legitimate game frame).  The low 15 bits are the control
// payload byte count.  Control payload byte 0 is a type code; type 0x01 =
// lockstep frame-token, followed by a 32-bit big-endian vblank counter.
// Control frames are consumed by the network read loop and NEVER enter the
// game RX queue / shared RAM.  Both instances must run the same build - a
// peer without control-frame support will treat the size prefix as invalid
// and close the socket.
constexpr uint8_t  LOCKSTEP_CTRL_TYPE_TOKEN     = 0x01; // payload[0]
constexpr uint16_t LOCKSTEP_CTRL_SIZE_FLAG      = 0x8000; // bit 15 of size prefix
constexpr uint16_t LOCKSTEP_CTRL_MAX_PAYLOAD    = 0x40; // sanity cap
constexpr int32_t  LOCKSTEP_MAX_LEAD            = 2;   // frames we may run ahead of the peer
constexpr int      LOCKSTEP_STALL_TIMEOUT_MS    = 100; // wall-clock cap per stalled vblank
constexpr uint32_t LOCKSTEP_SUSPEND_AFTER       = 6;   // consecutive full timeouts before action
constexpr int32_t  LOCKSTEP_REBASE_DRIFT        = 300; // |drift| beyond this = discontinuity

// Chunked-TX tuning.
//
// The game's TX routine splits a message larger than 0xFF halfwords into
// TXSIZE=0xFF chunks + remainder: it writes TXOFFSET only for the FIRST
// chunk (which also reads the message's TOTAL halfword count from the two
// cells immediately BEFORE the offset it programs) and expects the chip's
// internal DMA pointer to keep advancing for the continuation chunks.
// The receive-side validator accepts slot lengths in [4..0x400], so 0x400
// halfwords is the largest legitimate message.
constexpr uint16_t CHUNK_SAT_HW                 = 0xFF;  // TX routine's saturated chunk size
constexpr uint32_t CHUNK_MAX_HW                 = 0x400; // validator's max message length
constexpr int      CHUNK_STALE_MS               = 500;   // pending multi-chunk message older than this = dead

// Keepalive replay cadence.  The game's protocol layer declares a link
// timeout when its drift counter reaches 17 frames (~283 ms at 60 fps);
// a validated ingest resets it to roughly the transit latency.  Replaying
// the last real TX after this much TX silence keeps the peer's dispatcher
// fed with a healthy margin against that ceiling.
constexpr uint32_t HEARTBEAT_CADENCE_MS         = 33;

// Debounce for the driver-pushed in-game signal: this many CONSECUTIVE
// linked-gameplay vblanks before the in-game state arms (~1 s); any
// non-linked vblank drops it immediately.  Link establishment therefore
// always sees the simple stop-and-wait TX release.
constexpr uint32_t INGAME_DEBOUNCE_VBLANKS      = 60;

// TX-complete release tuning (see the member block in namco_c139.h).
// TX_BUSY_MS models the serialization interval of a staged frame (~one
// vblank), which paces the host's TX pump at its hardware texture.
// HEARTBEAT_STALE_MS ages out keepalive replays while in-game: frames
// larger than 0xFF halfwords are never captured for replay (restamping a
// chunked frame would corrupt its framing), so during long stretches of
// bulk-only traffic the captured payload can grow arbitrarily old - and
// under the in-game self-release the peer no longer needs the replay for
// pacing, so a stale replay only feeds it outdated state.
constexpr uint32_t TX_BUSY_MS                   = 12;
constexpr uint32_t HEARTBEAT_STALE_MS           = 150;

// Announce-latch TTL: the stage->START window the latch protects is 1-2
// frames (the wipe usually lands inside the very register write that
// carries the START edge), and the game's own re-announce of a lost
// message class arrives within ~250 ms - so 150 ms comfortably covers
// every legitimate dispatch while guaranteeing a stale (offset, size) can
// never reach the wire long after the game's TX routine moved on.
// Emulated-time domain (attotime), like every other timeout in this
// device, so lockstep stalls cannot expire a latch mid-race.
constexpr int    AL_TTL_MS                      = 150;

// Capacity of the announce-latch wipe-time payload snapshot, in wire
// BYTES (2 per shared-RAM halfword).  Sized to the send path's frame-size
// RAM cap (0x1000 halfwords, see send_pending_tx_frame) so any size a
// latch dispatch could legally transmit always fits; the realistic
// capture is the TX routine's saturated head chunk (CHUNK_SAT_HW
// halfwords = 510 bytes).  The buffer is reserved once in device_start();
// each capture reuses it (clear + append within the reserved capacity -
// no per-event allocation).
constexpr uint32_t AL_SNAP_MAX_BYTES            = 0x2000;

// Dedupe lookback: a latch dispatch is suppressed when a wire completion
// of the same (offset, expected size) class exists at or after
// (announce_time - AL_DEDUPE_LOOKBACK_MS).  The window extends BEFORE the
// announce because the natural send of a message class can precede the
// wiped cadence re-announce of the same class by a few tens of ms.  One
// cadence hop keeps the PREVIOUS cycle's completion outside the window,
// so each cycle is judged only against its own natural send; and a
// genuinely lost class re-announces later than the window, so a
// suppression can never abandon a class, only delay it by one hop.
constexpr int    AL_DEDUPE_LOOKBACK_MS          = 150;

// FNV-1a 32-bit content hash over the shared-RAM image at the staged read
// pointer, in WIRE byte order (high byte first, low byte second per
// halfword, 0x1fff mask - exactly the readout loop in
// send_pending_tx_frame).  Used as the TX-complete admission key; cheap
// (~2 ops/byte) and collision-safe enough for duplicate detection at this
// volume.
constexpr uint32_t TS_FNV_OFFSET                = 0x811c9dc5;
constexpr uint32_t TS_FNV_PRIME                 = 0x01000193;
inline uint32_t ts_hash_ram(uint16_t const *ram, uint16_t ptr, uint32_t hw)
{
	uint32_t h = TS_FNV_OFFSET;
	for (uint32_t i = 0; i < hw; i++)
	{
		uint16_t const w = ram[uint16_t(ptr + i) & 0x1fff];
		h = (h ^ uint32_t(uint8_t(w >> 8))) * TS_FNV_PRIME;
		h = (h ^ uint32_t(uint8_t(w & 0xff))) * TS_FNV_PRIME;
	}
	return h;
}

} // anonymous namespace


//**************************************************************************
//  NETWORK CONTEXT
//
//  Inner class hosting the asio io_context, listening/connecting sockets,
//  and the dedicated worker thread that runs io_context::run().  The MAME
//  emulation thread interacts with this object only via std::atomic state
//  and m_ioctx.post() lambdas, never by touching asio objects directly.
//**************************************************************************

class namco_c139_device::context
{
public:
	context(namco_c139_device &device,
			std::optional<asio::ip::tcp::endpoint> const &local,
			std::optional<asio::ip::tcp::endpoint> const &remote)
		: m_device(device)
		, m_acceptor(m_ioctx)
		, m_socket(m_ioctx)
		, m_local(local)
		, m_remote(remote)
		, m_stopping(false)
		, m_connected(false)
	{
	}

	std::error_code start()
	{
		std::error_code err;

		if (m_local)
		{
			m_acceptor.open(m_local->protocol(), err);
			if (!err)
				m_acceptor.set_option(asio::socket_base::reuse_address(true), err);
			if (!err)
				m_acceptor.bind(*m_local, err);
			if (!err)
				m_acceptor.listen(1, err);
			if (err)
				return err;
		}

		m_thread = std::thread(
				[this] ()
				{
					if (m_local)
					{
						m_acceptor.async_accept(m_socket,
								[this] (std::error_code const &acc_err)
								{
									if (m_stopping.load(std::memory_order_acquire))
										return;
									if (acc_err)
									{
										LOGMASKED(LOG_WARN, "accept failed: %s\n", acc_err.message().c_str());
										return;
									}
									LOGMASKED(LOG_LINK, "peer connected (incoming)\n");
									m_connected.store(true, std::memory_order_release);
									start_read();
								});
					}

					if (m_remote)
					{
						m_socket.async_connect(*m_remote,
								[this] (std::error_code const &con_err)
								{
									if (m_stopping.load(std::memory_order_acquire))
										return;
									if (con_err)
									{
										LOGMASKED(LOG_WARN, "connect to %s:%u failed: %s\n",
												m_remote->address().to_string().c_str(),
												m_remote->port(),
												con_err.message().c_str());
										return;
									}
									LOGMASKED(LOG_LINK, "connected to peer at %s:%u\n",
											m_remote->address().to_string().c_str(),
											m_remote->port());
									m_connected.store(true, std::memory_order_release);
									start_read();
								});
					}

					m_ioctx.run();
				});

		return {};
	}

	void stop()
	{
		asio::post(m_ioctx,
				[this] ()
				{
					m_stopping.store(true, std::memory_order_release);
					std::error_code err;
					if (m_acceptor.is_open())
						m_acceptor.close(err);
					if (m_socket.is_open())
						m_socket.close(err);
				});
		if (m_thread.joinable())
			m_thread.join();
	}

	bool connected() const { return m_connected.load(std::memory_order_acquire); }

	// Called from emulation thread.  Posts the buffer to the network thread
	// and triggers an async_write chain if one is not already in flight.
	void send_frame(std::vector<uint8_t> data)
	{
		asio::post(m_ioctx,
				[this, payload = std::move(data)] () mutable
				{
					if (m_stopping.load(std::memory_order_acquire))
						return;
					if (!m_socket.is_open())
						return;

					bool const idle = m_outbound.empty();
					m_outbound.push_back(std::move(payload));
					if (idle)
						start_write();
				});
	}

private:
	// forward logging to the owning device so the LOGMASKED idiom works
	// inside this inner class too
	template <typename Format, typename... Params>
	void logerror(Format &&fmt, Params &&... args) const
	{
		m_device.logerror(std::forward<Format>(fmt), std::forward<Params>(args)...);
	}

	// Network-thread only: pop the front of m_outbound and async_write it.
	// On completion, chain to the next entry if any.
	void start_write()
	{
		auto &front = m_outbound.front();
		asio::async_write(m_socket, asio::buffer(front),
				[this] (std::error_code const &err, std::size_t /*bytes*/)
				{
					if (m_stopping.load(std::memory_order_acquire))
						return;
					if (err)
					{
						LOGMASKED(LOG_WARN, "tx write failed: %s\n", err.message().c_str());
						return;
					}
					m_outbound.pop_front();
					if (!m_outbound.empty())
						start_write();
				});
	}

	// Network-thread only: read a 2-byte big-endian size header followed by
	// the corresponding payload, then hand the frame to the emulation
	// thread and chain to the next read.
	void start_read()
	{
		asio::async_read(m_socket, asio::buffer(m_rx_size_bytes),
				[this] (std::error_code const &err, std::size_t /*bytes*/)
				{
					if (m_stopping.load(std::memory_order_acquire))
						return;
					if (err)
					{
						LOGMASKED(LOG_WARN, "rx size read failed: %s\n", err.message().c_str());
						return;
					}

					uint16_t const size = (uint16_t(m_rx_size_bytes[0]) << 8)
										|  uint16_t(m_rx_size_bytes[1]);

					// Size prefix with bit 15 set = link-layer control frame
					// (see the LOCKSTEP_CTRL_* constants).  Parsed here on
					// the network thread and consumed - control payloads
					// never reach the game RX queue or shared RAM.
					if (size & LOCKSTEP_CTRL_SIZE_FLAG)
					{
						uint16_t const ctl_size = uint16_t(size & ~LOCKSTEP_CTRL_SIZE_FLAG);
						if (ctl_size == 0 || ctl_size > LOCKSTEP_CTRL_MAX_PAYLOAD)
						{
							LOGMASKED(LOG_WARN, "invalid rx control frame size %u; closing\n", ctl_size);
							std::error_code close_err;
							m_socket.close(close_err);
							return;
						}
						m_rx_payload.resize(ctl_size);
						asio::async_read(m_socket, asio::buffer(m_rx_payload),
								[this, ctl_size] (std::error_code const &err2, std::size_t /*bytes2*/)
								{
									if (m_stopping.load(std::memory_order_acquire))
										return;
									if (err2)
									{
										LOGMASKED(LOG_WARN, "rx control payload read failed: %s\n",
												err2.message().c_str());
										return;
									}
									if (ctl_size >= 5 && m_rx_payload[0] == LOCKSTEP_CTRL_TYPE_TOKEN)
									{
										uint32_t const token = (uint32_t(m_rx_payload[1]) << 24)
															 | (uint32_t(m_rx_payload[2]) << 16)
															 | (uint32_t(m_rx_payload[3]) << 8)
															 |  uint32_t(m_rx_payload[4]);
										m_device.lockstep_token_received(token);
									}
									m_rx_payload.clear();
									start_read();
								});
						return;
					}

					if (size == 0 || size > 0x4000)
					{
						LOGMASKED(LOG_WARN, "invalid rx frame size %u; closing\n", size);
						std::error_code close_err;
						m_socket.close(close_err);
						return;
					}

					m_rx_payload.resize(size);
					asio::async_read(m_socket, asio::buffer(m_rx_payload),
							[this, size] (std::error_code const &err2, std::size_t /*bytes2*/)
							{
								if (m_stopping.load(std::memory_order_acquire))
									return;
								if (err2)
								{
									LOGMASKED(LOG_WARN, "rx payload read failed: %s\n",
											err2.message().c_str());
									return;
								}
								// Hand the buffer to the emulation thread which
								// will write it into shared RAM, set the RX flag
								// bits in the status reg, and raise the IRQ.
								on_frame_received(std::move(m_rx_payload));
								m_rx_payload.clear();
								start_read();
							});
				});
	}

	namco_c139_device &m_device;
	asio::io_context m_ioctx;
	asio::ip::tcp::acceptor m_acceptor;
	asio::ip::tcp::socket m_socket;
	std::optional<asio::ip::tcp::endpoint> m_local;
	std::optional<asio::ip::tcp::endpoint> m_remote;
	std::atomic<bool> m_stopping;
	std::atomic<bool> m_connected;
	std::thread m_thread;

	// Outbound TX queue (network thread accesses).  m_outbound.front() is
	// the in-flight async_write; subsequent entries chain on completion.
	std::deque<std::vector<uint8_t>> m_outbound;

	// Inbound RX scratch buffers (network thread accesses).
	std::array<uint8_t, 2> m_rx_size_bytes;
	std::vector<uint8_t>   m_rx_payload;

	// Inbound queue of fully-received frames waiting to be delivered to
	// the emulation thread.  Network thread pushes under m_inbound_mutex,
	// emulation thread drains via drain_rx() in deliver_rx_frames().
	std::mutex                       m_inbound_mutex;
	std::deque<std::vector<uint8_t>> m_inbound;

	// Called from the network thread when a complete frame arrives.
	// Hands the buffer to the inbound queue.  The emulation thread will
	// drain the queue lazily on its next C139 register access (reg_r,
	// reg_w, status_r) - the game polls these continuously during the
	// link busy-wait so latency is microseconds.  We deliberately do NOT
	// call machine().scheduler().synchronize() here: that API is intended
	// for in-emulator timers, not foreign-thread wakeups, and on this
	// driver it caused the listener instance to grind to a halt.
	void on_frame_received(std::vector<uint8_t> data)
	{
		std::lock_guard<std::mutex> lock(m_inbound_mutex);
		m_inbound.push_back(std::move(data));
	}

public:
	// Emulation-thread accessor: atomically drain the inbound queue.
	std::deque<std::vector<uint8_t>> drain_rx()
	{
		std::lock_guard<std::mutex> lock(m_inbound_mutex);
		return std::move(m_inbound);
	}
};


// Endpoint parser shared by the CLI (-comm_*) and cfg/default bring-up
// paths, so both validate hosts/ports through the same code.
static std::optional<asio::ip::tcp::endpoint> parse_comm_endpoint(
		device_t &dev, char const *host, char const *port, char const *what)
{
	if (!host || !*host)  return std::nullopt;
	if (!port || !*port)  return std::nullopt;

	std::error_code err;
	auto addr = asio::ip::make_address(host, err);
	if (err)
	{
		if (VERBOSE & LOG_WARN)
			dev.logerror("invalid %s '%s': %s\n", what, host, err.message().c_str());
		return std::nullopt;
	}

	char *end = nullptr;
	unsigned long port_num = std::strtoul(port, &end, 10);
	if (!end || *end != '\0' || port_num == 0 || port_num > 65535)
	{
		if (VERBOSE & LOG_WARN)
			dev.logerror("invalid %s port '%s'\n", what, port);
		return std::nullopt;
	}

	return asio::ip::tcp::endpoint(addr, static_cast<unsigned short>(port_num));
}


void namco_c139_device::start_comm()
{
	auto const &opts = mconfig().options();
	char const *local_host  = opts.comm_localhost();
	char const *local_port  = opts.comm_localport();
	char const *remote_host = opts.comm_remotehost();
	char const *remote_port = opts.comm_remoteport();

	// MAME's emu_options ship with non-empty defaults for these:
	//   comm_localhost  = "0.0.0.0"
	//   comm_localport  = "15112"
	//   comm_remotehost = "127.0.0.1"
	//   comm_remoteport = "15112"
	// If we see the full default tuple, treat that as "not configured"
	// rather than as an instruction to listen on / connect to localhost
	// (which would have us connecting to ourselves).  To opt in via the
	// CLI, the user must change at least one of the four; to disable a
	// side while overriding only the other, pass an empty host string:
	//   listener:  -comm_localport 9876 -comm_remotehost ""
	//   connector: -comm_remotehost 127.0.0.1 -comm_remoteport 9876 -comm_localhost ""
	bool const at_defaults =
			local_host  && std::strcmp(local_host,  "0.0.0.0")   == 0 &&
			local_port  && std::strcmp(local_port,  "15112")     == 0 &&
			remote_host && std::strcmp(remote_host, "127.0.0.1") == 0 &&
			remote_port && std::strcmp(remote_port, "15112")     == 0;
	if (at_defaults)
	{
		// The CLI didn't configure the link: DEFER the decision to the
		// per-machine cfg / built-in loopback defaults, resolved in
		// start_comm_cfg() at config FINAL time - device_start runs BEFORE
		// the cfg file (and the DIP / Link ID values it carries) is
		// loaded, so nothing can be decided here.
		m_lp_comm_deferred = true;
		return;
	}

	// At least one -comm_* option was changed from MAME defaults =>
	// explicit CLI configuration, which wins over any cfg-stored values.
	auto local  = parse_comm_endpoint(*this, local_host,  local_port,  "comm_localhost");
	auto remote = parse_comm_endpoint(*this, remote_host, remote_port, "comm_remotehost");

	if (!local && !remote)
	{
		// No comm configured; remain in solo mode.  Game still falls back
		// gracefully to single-cabinet play when the link probe times out.
		return;
	}

	m_context = std::make_unique<context>(*this, local, remote);
	auto const err = m_context->start();
	if (err)
	{
		LOGMASKED(LOG_WARN, "failed to start network: %s\n", err.message().c_str());
		m_context.reset();
		return;
	}

	LOGMASKED(LOG_LINK, "network started%s%s\n",
			local  ? " (listening)"  : "",
			remote ? " (connecting)" : "");
}


// Per-machine cfg <linkplay> node + deferred cfg/default comm bring-up.
//
// Schema (cfg\<system>.cfg, next to the DIP/input state that already lives
// there - per working directory, which is per instance):
//   <linkplay listen_host="0.0.0.0" listen_port="9876"
//             connect_host="127.0.0.1" connect_port="9876" />
//
// Load order (configuration_manager::load_settings, which runs AFTER
// device_start): INIT -> default.cfg (DEFAULT) -> <system>.cfg (SYSTEM) ->
// FINAL.  We only consume the SYSTEM node; FINAL - which fires exactly
// once, after the ioport manager has restored the DIP and "Link ID"
// machine-configuration values from the same file - is where the deferred
// bring-up runs.

void namco_c139_device::linkplay_config_load(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode)
{
	if (cfg_type == config_type::SYSTEM && parentnode)
	{
		char const *const lhost = parentnode->get_attribute_string("listen_host", nullptr);
		if (lhost && *lhost)
			m_lp_listen_host = lhost;
		char const *const rhost = parentnode->get_attribute_string("connect_host", nullptr);
		if (rhost && *rhost)
			m_lp_connect_host = rhost;
		long long const lport = parentnode->get_attribute_int("listen_port", m_lp_listen_port);
		if (lport >= 1 && lport <= 65535)
			m_lp_listen_port = uint16_t(lport);
		long long const rport = parentnode->get_attribute_int("connect_port", m_lp_connect_port);
		if (rport >= 1 && rport <= 65535)
			m_lp_connect_port = uint16_t(rport);
	}

	if (cfg_type == config_type::FINAL)
		start_comm_cfg();
}


void namco_c139_device::linkplay_config_save(config_type cfg_type, util::xml::data_node *parentnode)
{
	// system-specific cfg only (per-instance), matching where the values load from
	if (cfg_type != config_type::SYSTEM)
		return;

	parentnode->set_attribute("listen_host", m_lp_listen_host.c_str());
	parentnode->set_attribute_int("listen_port", m_lp_listen_port);
	parentnode->set_attribute("connect_host", m_lp_connect_host.c_str());
	parentnode->set_attribute_int("connect_port", m_lp_connect_port);
}


// The host machine's "Link ID" machine configuration (for the timecrs2
// family: port JVS_PLAYER1, PORT_CONFNAME mask 0x00004000, 0x0000 =
// Left/Red, 0x4000 = Right/Blue).  Returns nullptr on machines that carry
// a C139 but no left/right link identity (where mask 0x4000 may be a live
// button, NOT a config field) - the cfg/default auto-link path only
// applies where this field exists.
ioport_field *namco_c139_device::lp_link_id_field() const
{
	ioport_port *const port = machine().root_device().ioport("JVS_PLAYER1");
	if (!port)
		return nullptr;
	ioport_field *const field = port->field(0x00004000);
	return (field && field->type() == IPT_CONFIG) ? field : nullptr;
}


bool namco_c139_device::lp_role_is_connector() const
{
	ioport_field *const field = lp_link_id_field();
	return field && (field->port().read() & 0x00004000); // 0x4000 = Right/Blue = connector
}


// Deferred comm bring-up for the cfg / built-in-loopback-defaults path.
// Runs once, at config FINAL (see linkplay_config_load), and only when
// start_comm() saw the all-MAME-defaults -comm_* tuple (i.e. the CLI did
// not configure the link - CLI keeps absolute priority).  Resolution:
//   - machines without the "Link ID" machine configuration: not
//     link-capable via cfg - stay solo (CLI still works);
//   - link DIP OFF: solo, no socket at all;
//   - Link ID Left/Red:   LISTENER, binds  listen_host:listen_port;
//   - Link ID Right/Blue: CONNECTOR, dials connect_host:connect_port.
// Values consumed once per launch; cfg edits apply on the NEXT launch.
void namco_c139_device::start_comm_cfg()
{
	if (!m_lp_comm_deferred || m_context)
		return;

	ioport_field *const idfield = lp_link_id_field();
	if (!idfield)
		return;

	ioport_port *const dsw = machine().root_device().ioport("DSW");
	if (!dsw || (dsw->read() & 0x08))
	{
		LOGMASKED(LOG_LINK, "link DIP is OFF - solo mode, comm not started\n");
		return;
	}

	bool const connector = lp_role_is_connector();
	std::string const &host = connector ? m_lp_connect_host : m_lp_listen_host;
	std::string const portstr = std::to_string(connector ? m_lp_connect_port : m_lp_listen_port);
	auto ep = parse_comm_endpoint(*this, host.c_str(), portstr.c_str(),
			connector ? "linkplay connect_host" : "linkplay listen_host");
	if (!ep)
	{
		LOGMASKED(LOG_WARN, "linkplay cfg %s '%s:%s' unusable - solo mode\n",
				connector ? "connect endpoint" : "listen endpoint", host.c_str(), portstr.c_str());
		return;
	}

	std::optional<asio::ip::tcp::endpoint> local, remote;
	if (connector)
		remote = ep;
	else
		local = ep;

	m_context = std::make_unique<context>(*this, local, remote);
	auto const err = m_context->start();
	if (err)
	{
		LOGMASKED(LOG_WARN, "failed to start network (linkplay cfg): %s\n", err.message().c_str());
		m_context.reset();
		return;
	}

	LOGMASKED(LOG_LINK, "network started via linkplay cfg/defaults: Link ID %s => %s %s:%s\n",
			connector ? "Right/Blue" : "Left/Red",
			connector ? "connecting to" : "listening on",
			host.c_str(), portstr.c_str());
}


void namco_c139_device::device_stop()
{
	if (m_context)
	{
		m_context->stop();
		m_context.reset();
	}
}


bool namco_c139_device::is_linked() const
{
	return m_context && m_context->connected();
}


// Record a wire completion of a latched bulk class into the dedupe ring.
// Called at exactly two sites, both on the emulation thread: the natural
// head-send retirement (the game's own send won its race against the wipe)
// and the latch dispatch itself (a dispatch is equally a wire completion;
// recording it means an immediately-following wiped re-announce of the
// same chain dedupes instead of double-dispatching).  Deduped consumptions
// are deliberately NOT recorded: nothing went on the wire, and a phantom
// entry could suppress a later genuine rescue.  Fixed-size ring,
// overwrite-oldest, no allocation.
void namco_c139_device::al_dedupe_record(uint16_t offset, uint32_t expected_hw)
{
	m_al_completes[m_al_comp_idx].offset = offset;
	m_al_completes[m_al_comp_idx].expected_hw = expected_hw;
	m_al_completes[m_al_comp_idx].t = machine().time();
	m_al_completes[m_al_comp_idx].valid = true;
	m_al_comp_idx = uint8_t((m_al_comp_idx + 1) % AL_DEDUPE_RING);
}


// Called from the emulation thread when the TX Control register's bit 0
// transitions 0 -> 1 (and from the TXSIZE-commit trigger sites in reg_w).
// Reads the staged TX frame out of the shared RAM, frames it as
// [size_be:2 bytes][payload:N bytes], and hands it to the network thread
// for delivery.  No-op when no peer is connected.
//
// Register 5 (TXSIZE) holds the TX size in HALFWORDS.  Each halfword in
// the shared RAM is transmitted as 2 bytes on the wire (big-endian: high
// byte first, then low byte), so the wire-byte count is 2 * m_regs[5].
// The transmitted bytes carry the sender's own framing intact: the last
// halfword has bit 8 set in its high byte (end marker) with the frame
// size in the trailer bytes, and the byte-sum checksum is built by the
// sender so the receiver's validator sees sum mod 256 == 0.
void namco_c139_device::send_pending_tx_frame()
{
	if (!m_context || !m_context->connected())
		return;

	// Not const: the announce-latch dispatch below may substitute the
	// rx_clear-wiped staged size at the zero-size abort site.  al_dispatch
	// marks that path so the register file is provably never written by it.
	uint16_t frame_size_words = m_regs[5];   // HALFWORD count
	bool al_dispatch = false;
	// true when this send is a latch dispatch whose payload comes from the
	// wipe-time snapshot instead of a dispatch-time shared-RAM re-read
	// (by dispatch time the host may already be recomposing the ring slot)
	bool al_snap_use = false;

	// Drop a tracked bulk message whose continuation never came BEFORE
	// deciding how to read this chunk - so a stale hold can never leave
	// the read pointer at the abandoned message's resume offset.  (The
	// per-vblank sweep in vblank_tick() is the primary staleness path;
	// this is the in-send backstop for the case where the next chunk
	// arrives just past the timeout.)
	if (!m_chunk_accum.empty()
			&& machine().time() - m_chunk_accum_since > attotime::from_msec(CHUNK_STALE_MS))
		chunk_drop("stale-timeout");

	// The game's TX routine writes TXOFFSET only for the FIRST chunk of a
	// message; continuation chunks rely on the chip auto-advancing its DMA
	// pointer and never rewrite TXOFFSET.  m_regs[7] never advances in
	// this device, so continuation chunks would otherwise re-send the
	// message HEAD - the true tail (checksum + length trailer + end
	// marker) would never reach the wire.  Read from the modelled
	// auto-advancing pointer instead (latched on every TXOFFSET write in
	// reg_w(), advanced here per transmitted halfword).
	//
	// When a bulk message is tracked and NO TXOFFSET was reprogrammed
	// since the previous send, this send is one of the game's
	// continuation chunks - read it from the tracked message's resume
	// pointer (where the last associated chunk ended), NOT from
	// m_chunk_tx_ptr (which an interleaved message's TXOFFSET write could
	// have dragged elsewhere).  This is the faithful model of the chip: a
	// continuation is a TXSIZE staged without a preceding TXOFFSET
	// reprogram.
	bool const is_continuation = !m_chunk_accum.empty() && !m_chunk_saw_txoffset;
	uint16_t const fifo_ptr =
			  is_continuation      ? m_chunk_resume_ptr
			: m_chunk_tx_ptr_valid ? m_chunk_tx_ptr
			:                        m_regs[7];

	bool const had_txoffset_reprogram = m_chunk_saw_txoffset;

	// A zero-size trigger (the game's idle TX tick stages TXSIZE=0 and
	// does NOT write TXOFFSET) is a no-op: do not consume the
	// TXOFFSET-reprogram edge on it, or a real chunk that the host staged
	// on a later edge would be mis-classified as a continuation.
	//
	// This is also the abort site of the stage->START rx_clear race the
	// announce latch repairs: the host stages a bulk head's TXSIZE, a
	// pending peer frame is delivered at the top of the register write
	// that carries the START edge and wipes m_regs[5], the edge still
	// fires this function - and it would return right here on size 0,
	// silently sending NOTHING.  The host (whose busy byte reads 0 either
	// way) then stages the remainder into dead air, re-announces the
	// message a few times and abandons it.  When the latch is live, WIPED
	// since its announce, inside its TTL, and the modelled DMA pointer
	// still sits at the latched offset, proceed with the latched size
	// instead: the tracker state (accumulator / resume pointer /
	// saw_txoffset consumption) is populated identically, so the
	// remainder's TXSIZE-commit trigger then fires as on any normal bulk
	// message.  One-shot: the latch is consumed here, so an announce can
	// never dispatch twice.  NOTHING is written to the register file on
	// this path (m_regs[5] stays 0 - see the guarded clear below): the
	// rx_clear release was honoured and stays honoured.
	if (frame_size_words == 0)
	{
		if (m_al_valid && m_al_wiped)
		{
			attotime const al_age = machine().time() - m_al_time;
			unsigned const al_age_ms = unsigned(al_age.as_double() * 1000.0 + 0.5);
			if (al_age > attotime::from_msec(AL_TTL_MS))
			{
				++m_al_expired;
				m_al_valid = false;
				m_al_wiped = false;
				LOGMASKED(LOG_FRAME, "announce-latch: expired offset=0x%04x hw=%u expected_hw=%u age_ms=%u expired=%u (START edge arrived past TTL - stale latch dropped)\n",
						m_al_offset, m_al_wiped_hw, m_al_expected_hw, al_age_ms, m_al_expired);
				return;
			}
			if (m_chunk_tx_ptr_valid && m_chunk_tx_ptr == m_al_offset)
			{
				// DISPATCH DEDUPE.  Every rail above passed (live+wiped
				// latch, inside TTL, DMA pointer at the latched offset),
				// so the latch would dispatch right now.  If this
				// (offset, expected size) class already completed on the
				// wire within AL_DEDUPE_LOOKBACK_MS of the latch's
				// announce anchor, the wiped stage was the host's cadence
				// RE-SEND of content the peer already ingested - the
				// rx_clear wipe was flow control, not loss - and
				// dispatching it would deliver a stale duplicate (head
				// from one compose generation, ring tail from a newer
				// one, still passing the byte-sum checksum).  Consume the
				// latch WITHOUT sending and return - the same
				// nothing-sent register-file-untouched shape as the
				// zero-size abort.  A genuine rescue (no recent same-key
				// completion) falls through and dispatches.
				int best = -1;
				for (unsigned i = 0; i < AL_DEDUPE_RING; i++)
				{
					al_complete_rec const &rec = m_al_completes[i];
					if (rec.valid && rec.offset == m_al_offset
							&& rec.expected_hw == m_al_expected_hw
							&& rec.t + attotime::from_msec(AL_DEDUPE_LOOKBACK_MS) >= m_al_time
							&& (best < 0 || rec.t > m_al_completes[best].t))
						best = int(i);
				}
				if (best >= 0)
				{
					m_al_valid = false;        // consumed like a dispatch (one-shot) - but nothing is sent
					m_al_wiped = false;
					m_al_snap_valid = false;   // the wipe-time snapshot dies with its latch
					++m_al_deduped;
					LOGMASKED(LOG_FRAME, "announce-latch: deduped offset=0x%04x hw=%u expected_hw=%u age_ms=%u deduped=%u (class already completed on the wire near the announce - latch consumed, nothing sent)\n",
							m_al_offset, m_al_wiped_hw, m_al_expected_hw, al_age_ms, m_al_deduped);
					return;
				}
				frame_size_words = m_al_wiped_hw;
				al_dispatch = true;
				// The payload for this dispatch is the snapshot copied at
				// wipe-capture time (when the staged bytes were provably
				// pristine).  The (offset, size) equality below always
				// holds at a genuine dispatch - the snapshot was recorded
				// from the SAME m_al_offset/m_al_wiped_hw this dispatch
				// just consumed, and no site mutates them in between (any
				// arm/refresh clears m_al_wiped first, forcing a fresh
				// capture) - so the re-read fallback is a defensive rail,
				// expected 0.
				al_snap_use = m_al_snap_valid
						&& m_al_snap_offset == m_al_offset
						&& m_al_snap.size() == std::size_t(frame_size_words) * 2;
				m_al_valid = false;   // one-shot: consumed now, never dispatchable twice
				m_al_wiped = false;
				++m_al_dispatched;
				// a latch dispatch is itself a wire completion of this
				// (offset, expected size) class - record it so an
				// immediately-following wiped re-announce of the same
				// chain dedupes instead of double-dispatching
				al_dedupe_record(m_al_offset, m_al_expected_hw);
				if (al_snap_use)
				{
					m_al_snap_valid = false;   // one-shot with its latch: a snapshot never transmits twice
					++m_al_snap_dispatched;
					LOGMASKED(LOG_FRAME, "announce-latch: dispatched offset=0x%04x hw=%u expected_hw=%u age_ms=%u dispatched=%u snap_tx=%u (send reconstructed from the wipe-time payload snapshot)\n",
							m_al_offset, frame_size_words, m_al_expected_hw, al_age_ms,
							m_al_dispatched, m_al_snap_dispatched);
				}
				else
				{
					++m_al_snap_fallback;
					LOGMASKED(LOG_WARN, "announce-latch: dispatched-reread offset=0x%04x hw=%u expected_hw=%u age_ms=%u snap_fallback=%u (snapshot does not match this dispatch - falling back to a shared-RAM re-read; investigate if this ever fires)\n",
							m_al_offset, frame_size_words, m_al_expected_hw, al_age_ms,
							m_al_snap_fallback);
				}
				// fall through: the send proceeds with the latched size
			}
			else
			{
				++m_al_superseded;
				m_al_valid = false;
				m_al_wiped = false;
				LOGMASKED(LOG_FRAME, "announce-latch: superseded offset=0x%04x hw=%u dma_ptr=0x%04x superseded=%u (TX pointer moved since the announce - latch dropped; stale offsets are never dispatched)\n",
						m_al_offset, m_al_wiped_hw, m_chunk_tx_ptr, m_al_superseded);
				return;
			}
		}
		else
			return;   // idle TX tick / nothing latched
	}

	// Consumed the "TXOFFSET reprogrammed since last send" edge for this send.
	m_chunk_saw_txoffset = false;

	// Sanity: cap at half the shared RAM (8 KB == 4096 halfwords).  The
	// realistic max per chunk is 0xFF = 255 halfwords (= 510 bytes) from
	// the game's chunked-TX path, but allow some headroom.
	if (frame_size_words > 0x1000)
	{
		LOGMASKED(LOG_WARN, "tx frame size %u halfwords exceeds RAM cap; dropping\n",
				frame_size_words);
		return;
	}

	uint32_t const frame_size_bytes = uint32_t(frame_size_words) * 2;

	// Read frame_size_words halfwords from shared RAM starting at
	// fifo_ptr, emitting each as 2 bytes (high byte first, low byte
	// second - matches the host CPU's big-endian byte ordering of the
	// in-RAM halfword).  This carries the sender's marker bit and
	// checksum intact to the receiver.  A latch dispatch transmits the
	// wipe-time snapshot verbatim instead - it was copied by the
	// identical loop at the wipe instant (same read pointer, same
	// masking, same byte order), so everything downstream (tracker
	// append, resume/DMA pointer advance, chunk forward, emit_tx_frame)
	// is unchanged and the tracker holds the same bytes that went on the
	// wire.
	std::vector<uint8_t> payload;
	payload.reserve(frame_size_bytes);
	if (al_snap_use)
		payload = m_al_snap;
	else
	{
		for (uint32_t i = 0; i < frame_size_bytes; i++)
		{
			uint16_t const word_idx = uint16_t((fifo_ptr + (i >> 1)) & 0x1fff);
			uint16_t const word     = m_ram[word_idx];
			uint8_t  const b        = (i & 1) ? uint8_t(word & 0xff)
											  : uint8_t(word >> 8);
			payload.push_back(b);
		}
	}

	// Advance the modelled DMA pointer past what we just consumed so the
	// next continuation chunk picks up where this one ended.
	m_chunk_tx_ptr = uint16_t((fifo_ptr + frame_size_words) & 0x1fff);
	m_chunk_tx_ptr_valid = true;

	// Clear TX Frame Size to signal "TX complete" to the host CPU.
	// Without this, the game's link probe function reads Frame Size != 0
	// on every subsequent invocation and exits early thinking a TX is
	// still in progress, blocking all further protocol traffic.  Real
	// hardware clears this when TX finishes; we do it synchronously since
	// our "transmission" is instantaneous.  On a latch dispatch m_regs[5]
	// is ALREADY 0 (the rx_clear wiped it and was honoured) - skip the
	// store so the latch path literally never writes the register file.
	if (!al_dispatch)
		m_regs[5] = 0;

	// Decide whether this staged chunk is a complete message (forward
	// now), the start of an announced bulk message (start tracking), a
	// CONTINUATION of the currently-tracked bulk message (associate;
	// retire the tracker when the accumulated halfword count reaches the
	// total announced in the message's size cells), or an unrelated
	// interleaved message (forward now, keep tracking the bulk message).
	//
	// Completeness is keyed on the sender-side size cells, NOT on a
	// trailer heuristic: the game's bulk builder writes a saturated
	// boundary trailer mid-message, which would fool any end-marker test
	// applied at chunk granularity.
	//
	// Association is purely TX POINTER CONTINUITY: a continuation chunk's
	// read pointer (fifo_ptr) resumes the tracked message's advanced DMA
	// pointer (m_chunk_resume_ptr).  The game's continuation path never
	// rewrites TXOFFSET, so on real hardware the DMA pointer keeps
	// advancing from where the previous chunk ended - exactly
	// m_chunk_resume_ptr.  A NEW message rewrites TXOFFSET to a slot
	// base, so its first chunk's read pointer is the slot base, NOT
	// m_chunk_resume_ptr - that distinguishes "continuation" from "a new
	// bulk message reusing the same slot".
	//
	// Every chunk goes on the wire as its own hardware-shaped frame the
	// moment it is associated: the receiving game's RX machinery
	// (end-marker back-scan + forward drain in its RX ring) reassembles
	// chunk sequences in the ring itself, exactly as on real hardware.
	// The accumulator is kept purely as the association/progress tracker.
	if (!m_chunk_accum.empty())
	{
		// A bulk message is being tracked.  Is THIS chunk its
		// continuation?  is_continuation (computed above, which also
		// forced fifo_ptr to m_chunk_resume_ptr) is the primary test; a
		// defensive secondary accept covers the no-interleaving case
		// where the DMA pointer naturally lands on the resume pointer
		// even though a (same-slot) TXOFFSET write was seen - only
		// accepted when the size does NOT look like a fresh saturated
		// announce (that is the "new bulk reusing the same slot" case,
		// which must supersede, not associate).
		bool const fresh_saturated_announce =
				(frame_size_words == CHUNK_SAT_HW
					&& m_chunk_expected_hw > CHUNK_SAT_HW
					&& m_chunk_expected_hw <= CHUNK_MAX_HW);
		bool const resumes = is_continuation
				|| (!had_txoffset_reprogram && fifo_ptr == m_chunk_resume_ptr
					&& !fresh_saturated_announce);

		if (resumes)
		{
			// Continuation chunk: associate and advance the resume
			// pointer, then forward it to the wire as its own frame.
			// Completion is against the TRACKED message's snapshotted
			// expected total, NOT m_chunk_expected_hw (which an
			// interleaved TXOFFSET write may have re-latched to some
			// other message's size).
			m_chunk_accum.insert(m_chunk_accum.end(), payload.begin(), payload.end());
			uint32_t const have_hw = uint32_t(m_chunk_accum.size() / 2);
			m_chunk_resume_ptr = uint16_t((m_chunk_resume_ptr + frame_size_words) & 0x1fff);
			++m_chunk_msg_chunks;     // per-message chunk count (first + continuations)
			emit_tx_frame(std::move(payload), true);
			if (have_hw == m_chunk_held_expected_hw)
			{
				// Complete: every chunk already went out individually -
				// just retire the tracker.
				m_chunk_accum.clear();
				m_chunk_bulk_pending = false;
				m_chunk_msg_chunks = 0;
			}
			else if (have_hw > m_chunk_held_expected_hw || have_hw > CHUNK_MAX_HW)
			{
				chunk_drop("overshoot");
			}
			return;
		}

		// This chunk does NOT resume the tracked message.  Two cases:
		//  - a NEW saturated bulk message (e.g. reusing the same slot at
		//    its base): supersede the stale tracked message and start
		//    fresh (the old one will never complete now);
		//  - an unrelated interleaved small/exact message: forward it now
		//    and KEEP tracking the bulk message.  Falls through to the
		//    start-or-passthrough logic below WITHOUT clearing the
		//    tracker.
		if (frame_size_words == CHUNK_SAT_HW
				&& m_chunk_expected_hw > CHUNK_SAT_HW && m_chunk_expected_hw <= CHUNK_MAX_HW)
		{
			// New bulk message arriving while one is still tracked: the
			// tracked one is stale (its continuation never resumed) -
			// retire it.
			chunk_drop("superseded");
			// fall through to start a fresh hold below
		}
		else
		{
			// Interleaved unrelated message: pass it straight through,
			// keep tracking the bulk message.
			emit_tx_frame(std::move(payload));
			return;
		}
	}

	if (frame_size_words == CHUNK_SAT_HW
			&& m_chunk_expected_hw > CHUNK_SAT_HW && m_chunk_expected_hw <= CHUNK_MAX_HW)
	{
		// Saturated first chunk of an announced multi-chunk message:
		// start tracking.  (A genuine single-chunk 255-halfword message
		// announces expected == 0xFF and passes through.)  Record the
		// slot pointer this message began at and the pointer the next
		// chunk must resume from (this chunk's read pointer + its size).
		// The head chunk goes on the wire NOW as its own hardware-shaped
		// frame; the accumulator keeps a COPY purely as the
		// association/progress tracker.
		m_chunk_accum = payload;             // copy: payload still needed for the wire
		m_chunk_accum_since = machine().time();
		m_chunk_msg_start_ptr = fifo_ptr;
		m_chunk_resume_ptr = uint16_t((fifo_ptr + frame_size_words) & 0x1fff);
		m_chunk_held_expected_hw = m_chunk_expected_hw;   // snapshot so an interleaved TXOFFSET write can't clobber it
		m_chunk_msg_chunks = 1;

		// The head of this announce is going on the wire - the vulnerable
		// stage->START window is over (the remainder dispatches
		// synchronously at its own TXSIZE write via the commit trigger
		// and can never be wiped).  Retire the latch.  On a latch
		// DISPATCH m_al_valid was already consumed at the abort site, so
		// this counts only heads the game sent on its own.  This natural
		// head send is THE completion class the dedupe ring exists for:
		// recorded with the latch's own key so a wiped cadence
		// re-announce of this class dedupes at its START edge.
		if (m_al_valid && m_al_offset == fifo_ptr)
		{
			m_al_valid = false;
			m_al_wiped = false;
			al_dedupe_record(m_al_offset, m_al_expected_hw);
		}
		emit_tx_frame(std::move(payload), true);
		return;
	}

	// A >255-halfword frame staged as ONE whole burst (frame size ==
	// announced size, != 0xFF) is NOT a saturated chunk head, so it
	// reaches the self-contained passthrough below and goes out whole.
	// Retire any announce-latch its own bulk announce armed (bulk_pending
	// latches on expected > 0xFF, but the saturated-head site above only
	// runs for a 0xFF-halfword head) and record the dedupe key, exactly
	// as that site does.  A WIPED large frame instead dispatched from the
	// latch at the abort site (al_dispatch) already cleared m_al_valid -
	// so the guard is false on that path (no double-retire); clearing
	// bulk_pending is still correct (the frame went out).
	if (frame_size_words > CHUNK_SAT_HW)
	{
		m_chunk_bulk_pending = false;
		if (m_al_valid && m_al_offset == fifo_ptr)
		{
			m_al_valid = false;
			m_al_wiped = false;
			al_dedupe_record(m_al_offset, m_al_expected_hw);
		}
	}
	// else: self-contained message (small/exact frames, or the
	// 1-halfword boot ping) - plain passthrough.

	emit_tx_frame(std::move(payload));
}


// Single exit point for a frame onto the wire (a self-contained frame, or
// a chunk of a bulk message forwarded individually).  Also the keepalive
// capture site, so keepalive replays always describe exactly what went on
// the wire.  bulk_chunk=true = a FRAGMENT of an in-progress
// >255-halfword message - excluded from keepalive capture (replaying or
// restamping a fragment would interleave garbage mid-message).
void namco_c139_device::emit_tx_frame(std::vector<uint8_t> payload, bool bulk_chunk)
{
	if (!m_context || !m_context->connected())
		return;
	if (payload.empty())
		return;

	uint32_t const payload_bytes = uint32_t(payload.size());
	uint32_t const payload_words = payload_bytes / 2;

	std::vector<uint8_t> frame;
	frame.reserve(payload_bytes + 2);
	// Internal TCP framing: 16-bit big-endian byte count, then payload.
	// (The receiver reads the 2-byte size prefix to know how many payload
	// bytes follow on the TCP stream.)
	frame.push_back(static_cast<uint8_t>((payload_bytes >> 8) & 0xff));
	frame.push_back(static_cast<uint8_t>(payload_bytes & 0xff));
	frame.insert(frame.end(), payload.begin(), payload.end());

	// Cache small real TXs (>= 2 halfwords, i.e. proper protocol frames -
	// not the 1-halfword boot ping) for the keepalive timer to replay,
	// skipping the 2-byte length prefix, and re-arm the keepalive to fire
	// HEARTBEAT_CADENCE_MS from now - genuine traffic resets the clock,
	// so a replay fires only after that much real-TX silence.
	//
	// Frames larger than 0xFF halfwords - and individual bulk chunks -
	// are EXCLUDED from capture: a replay is restamped (bytes 0-1
	// rewritten with the current state counter), which would corrupt a
	// bulk frame's framing; and replaying a fragment would interleave
	// device-invented data into a message the peer's RX ring is
	// mid-reassembling.  They still re-arm the clock (else-branch below):
	// they are real wire traffic, and holding the replay back while
	// large-frame traffic flows also stops a stale small replay from
	// out-arriving a fresh frame.
	if (payload_words >= 2 && !bulk_chunk && payload_words <= CHUNK_SAT_HW)
	{
		m_last_tx_payload.assign(frame.begin() + 2, frame.end());
		m_txc_cap_time = machine().time();   // stale-replay age-out anchor
		if (m_heartbeat_timer)
			m_heartbeat_timer->adjust(attotime::from_msec(HEARTBEAT_CADENCE_MS));
	}
	else if (m_heartbeat_timer && (bulk_chunk || payload_words > CHUNK_SAT_HW))
		m_heartbeat_timer->adjust(attotime::from_msec(HEARTBEAT_CADENCE_MS));

	m_context->send_frame(std::move(frame));
}


// Abandon a tracked bulk message (stale tail, overshoot, superseded by a
// new bulk message).  Only the TRACKER is retired here - the associated
// chunks already went on the wire individually, so the peer holds a
// truncated message its own staleness/marker machinery ages out, exactly
// as an abandoned mid-message transmission on real hardware would.
void namco_c139_device::chunk_drop(const char *reason)
{
	if (m_chunk_accum.empty() && !m_chunk_bulk_pending)
		return;
	if (!m_chunk_accum.empty())
	{
		LOGMASKED(LOG_WARN, "bulk message dropped: reason=%s had_hw=%u expected=%u chunk_count=%u start_ptr=0x%04x resume_ptr=0x%04x\n",
				reason,
				unsigned(m_chunk_accum.size() / 2), m_chunk_held_expected_hw,
				m_chunk_msg_chunks, m_chunk_msg_start_ptr, m_chunk_resume_ptr);
		m_chunk_accum.clear();
	}
	m_chunk_bulk_pending = false;
	m_chunk_msg_chunks = 0;
	m_chunk_held_expected_hw = 0;
}


// Drains any frames the network thread has pushed onto the inbound queue
// and delivers them into the shared RAM at the RX FIFO Pointer (reg 6),
// advancing the pointer and setting the Status/Control register's RX
// flag bits so the game's busy-wait poll-loop breaks out.  Called on the
// emulation thread from every C139 register access path so RX latency
// is bounded by the game's own polling cadence.
//
// The int32_t param is retained for compatibility with timer callbacks
// in case the emulation thread is later woken via an emu_timer.
void namco_c139_device::deliver_rx_frames(int32_t /*param*/)
{
	if (!m_context)
		return;

	auto pending = m_context->drain_rx();
	if (pending.empty())
		return;

	while (!pending.empty())
	{
		auto frame = std::move(pending.front());
		pending.pop_front();

		uint16_t const fifo_ptr = m_regs[6];   // RX FIFO Pointer (WORDS)
		uint16_t const rx_base = 0x1000;
		std::size_t const num_payload_words = frame.size() / 2;

		// Skip frames smaller than 2 payload words.  Even-byte payloads
		// of 2 bytes (= 1 halfword) are the connection-level greetings
		// (the initial 1-halfword ping the game emits at boot via
		// m_regs[5] = 1), not protocol frames - they don't carry the
		// sender's bit-8 end-of-frame marker so the validator can't do
		// anything with them.  Skipping is also a guard against
		// odd-length frames (frame.size() = 1) which would produce 0
		// payload halfwords here.
		if (num_payload_words < 2)
		{
			continue;
		}

		// Write payload bytes into the high-half RX area at the current
		// FIFO pointer.  fifo_ptr is RX-area-relative; the RX area sits
		// at words 0x1000..0x1FFF (the RX FIFO Pointer has a 12-bit
		// range).  The payload is written faithfully - the sender's
		// message-end framing (last halfword: bit 8 of the high byte set,
		// size in the trailer bytes) crosses the wire intact, so the
		// receiving game's validator sees the sender's intended packet
		// structure and checksum range.  The write index wraps within
		// the RX window (rx_base + 12-bit offset), matching the game's
		// own drain loop which wraps its source index inside the RX
		// window - frames crossing the end of the RX area wrap to its
		// start, not into the TX half.
		for (std::size_t i = 0; i < frame.size(); i++)
		{
			uint16_t const word_idx = uint16_t(rx_base + ((fifo_ptr + (i >> 1)) & 0x0fff));
			uint16_t &w = m_ram[word_idx];
			if (i & 1)
				w = uint16_t((w & 0xff00) | uint16_t(frame[i]));
			else
				w = uint16_t((w & 0x00ff) | (uint16_t(frame[i]) << 8));
		}

		// Advance the RX FIFO Pointer past the last received halfword
		// (RX area is 4096 words, so mask to 0x0FFF).  The game's
		// scanner expects to find the end marker at (m_regs[6] - 1) when
		// scanning backwards.
		m_regs[6] = uint16_t((fifo_ptr + num_payload_words) & 0x0fff);

		// Set the RX flag bits in the Status/Control Flags register so
		// the game's dispatcher picks up the event.
		m_regs[1] |= 0x0006;

		// Clear m_regs[5] (Frame Size).  After staging a TX the game
		// writes m_regs[5] = expected size and busy-polls it, expecting
		// hardware to clear the register once a corresponding RX frame
		// has arrived; without this, the game's link-up busy-wait loop
		// never breaks out despite the RX-flag bits being set.
		//
		// NOTE: on real hardware an RX never clears a staged TX - this
		// release exists for the link-up busy-poll.  It therefore races
		// the in-game TX path: a wipe can land between a bulk head's
		// TXSIZE stage and its START edge, silently deleting the head
		// from the middle of the message.  The wipe itself proceeds
		// untouched (the game's TX routine reads the zero as its green
		// light and must never be starved of it); the announce latch
		// merely REMEMBERS the staged size it is about to destroy -
		// together with a snapshot of the staged payload, taken at the
		// only instant the bytes are provably pristine - so the game's
		// START edge, which still arrives and reads a zeroed size, can
		// dispatch the send from the latch (see the abort site in
		// send_pending_tx_frame).  Only the FIRST wipe per announce is
		// captured: a second staged TXSIZE wiped under the same latch
		// means the host re-staged the register without a TXOFFSET
		// reprogram after a missed dispatch - the latch's model of "the
		// wiped value is the head" is no longer trustworthy, so it drops
		// rather than ever dispatching a mismatched (offset, size).
		if (m_regs[5] != 0)
		{
			bool const staged_bulk = m_chunk_bulk_pending || !m_chunk_accum.empty();
			if (staged_bulk)
			{
				LOGMASKED(LOG_FRAME, "rx delivery wiped staged TXSIZE %u hw while a bulk message is announced/open\n",
						unsigned(m_regs[5] & 0xff));

				if (m_al_valid)
				{
					attotime const al_age = machine().time() - m_al_time;
					unsigned const al_age_ms = unsigned(al_age.as_double() * 1000.0 + 0.5);
					if (al_age > attotime::from_msec(AL_TTL_MS))
					{
						++m_al_expired;
						m_al_valid = false;
						m_al_wiped = false;
						LOGMASKED(LOG_FRAME, "announce-latch: expired offset=0x%04x hw=%u expected_hw=%u age_ms=%u expired=%u (wipe hit a latch past TTL - dropped, never dispatched)\n",
								m_al_offset, unsigned(m_regs[5]), m_al_expected_hw,
								al_age_ms, m_al_expired);
					}
					else if (!m_al_wiped)
					{
						m_al_wiped = true;
						m_al_wiped_hw = m_regs[5];
						++m_al_wipes_captured;
						// Snapshot the staged head payload NOW - the host
						// staged these bytes and the wipe (below,
						// untouched) hits only the register.  Copy
						// exactly the halfwords the START-edge dispatch
						// would otherwise re-read from shared RAM
						// (m_al_wiped_hw of them starting at m_al_offset
						// - the dispatch's read pointer is provably
						// m_al_offset, see the abort site), with the
						// identical masking and wire byte order as the
						// send loop.  READ-ONLY: neither m_ram nor the
						// register file is written here; the rx_clear
						// proceeds identically.  The buffer was reserved
						// once in device_start (clear + append within
						// capacity, no per-event allocation).
						m_al_snap_valid = false;
						m_al_snap.clear();
						uint32_t const snap_bytes = uint32_t(m_al_wiped_hw) * 2;
						if (snap_bytes != 0 && snap_bytes <= AL_SNAP_MAX_BYTES)
						{
							for (uint32_t i = 0; i < snap_bytes; i++)
							{
								uint16_t const word_idx = uint16_t((m_al_offset + (i >> 1)) & 0x1fff);
								uint16_t const word     = m_ram[word_idx];
								uint8_t  const b        = (i & 1) ? uint8_t(word & 0xff)
																  : uint8_t(word >> 8);
								m_al_snap.push_back(b);
							}
							m_al_snap_offset = m_al_offset;
							m_al_snap_valid = true;
						}
						LOGMASKED(LOG_FRAME, "announce-latch: wiped offset=0x%04x hw=%u expected_hw=%u age_ms=%u wiped_seen=%u snap=%d snap_bytes=%u (staged TXSIZE lost to rx delivery; latch holds the stage and its wipe-time payload snapshot for the START edge)\n",
								m_al_offset, unsigned(m_al_wiped_hw), m_al_expected_hw,
								al_age_ms, m_al_wipes_captured,
								m_al_snap_valid ? 1 : 0, unsigned(m_al_snap.size()));
					}
					else
					{
						++m_al_superseded;
						m_al_valid = false;
						m_al_wiped = false;
						LOGMASKED(LOG_FRAME, "announce-latch: superseded offset=0x%04x hw=%u expected_hw=%u superseded=%u (second staged TXSIZE wiped under one announce - unattributable re-stage, latch dropped)\n",
								m_al_offset, unsigned(m_regs[5]), m_al_expected_hw,
								m_al_superseded);
					}
				}
			}
		}
		m_regs[5] = 0;
	}

	// Pulse the IRQ output line so the game's interrupt handler picks
	// up the frame even if its busy-wait poll-loop has already timed out
	// into solo mode.  We auto-clear via m_irq_pulse_timer after a short
	// delay so we never get stuck with a level-asserted IRQ that the game
	// fails to ack with the expected magic value.
	if (m_irq_pulse_timer)
	{
		m_irq_cb(ASSERT_LINE);
		m_irq_pulse_timer->adjust(attotime::from_usec(200));
	}
}


// Link payload structure (from the Time Crisis II protocol layer, for
// reference): the game-level "cell stream" is the LOW byte of each
// halfword; after a small link/app header, the stream is a sequence of
// opcode bytes each followed by a fixed operand count, terminated by
// 0x00.  Observed operand byte counts (consumed AFTER the opcode byte):
//   0x1F despawn=4; 0x20 release=2; 0x3B=2; 0x55 link session state
//   (two halfword operands + 24-bit flags advertised to the partner);
//   0x64/65/66=3; 0x67/68/69=4; 0x6A/6B=5; 0x6C/6D/6E=3; 0x6F=6
//   (play-clock pair); 0x70=2 (cutscene timer); 0x71=1; 0x72=3;
//   0x73=190; 0x74=4.  Intrinsics: 0xFD = 3 bytes total; 0xFE/0xFF =
//   variable self-relative skip.
// A byte value is an opcode only when the stream cursor lands on it.
// The device never parses this stream (it forwards raw frames); the
// table is documented here because it defines what the frames carry.


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

void namco_c139_device::data_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().share("sharedram");
}

void namco_c139_device::regs_map(address_map &map)
{
	map(0x00, 0x01).r(FUNC(namco_c139_device::status_r)); // WRITE clears flags
	map(0x02, 0x0f).rw(FUNC(namco_c139_device::reg_r), FUNC(namco_c139_device::reg_w));
//  map(0x0a, 0x0b) // WRITE tx_w
//  map(0x0c, 0x0d) // READ rx_r
//  map(0x0e, 0x0f) //
}

//-------------------------------------------------
//  namco_c139_device - constructor
//-------------------------------------------------

namco_c139_device::namco_c139_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCO_C139, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_space_config("data", ENDIANNESS_BIG, 16, 14, 0, address_map_constructor(FUNC(namco_c139_device::data_map), this)),
	m_irq_cb(*this)
{
	std::fill(std::begin(m_regs), std::end(m_regs), uint16_t(0));
}


// Out-of-line so the compiler can emit the unique_ptr<context> destructor
// at a point where class context is a complete type.
namco_c139_device::~namco_c139_device() = default;


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_c139_device::device_start()
{
	m_ram = (uint16_t*)memshare("sharedram")->ptr();

	save_item(NAME(m_regs));

	// One-shot timer that auto-clears the IRQ output line a short time
	// after we assert it on RX delivery.  This implements pulse-style IRQ
	// semantics so we never get stuck with a level-asserted IRQ line that
	// the game's handler fails to ack.
	m_irq_pulse_timer = timer_alloc(FUNC(namco_c139_device::irq_pulse_off), this);

	// Keepalive timer that periodically replays the last real TX to keep
	// the partner's link-state dispatcher firing (which resets the
	// partner's timeout counter via state-counter sync).
	m_heartbeat_timer = timer_alloc(FUNC(namco_c139_device::heartbeat_tick), this);

	// announce-latch snapshot buffer, reserved once (captures reuse it)
	m_al_snap.reserve(AL_SNAP_MAX_BYTES);

	// Register the per-machine cfg <linkplay> node.  Registration must
	// happen here in device_start - configuration_manager::load_settings
	// runs right after start_all_devices and only calls handlers
	// registered by then.  The load handler also hosts the deferred comm
	// bring-up at config FINAL (start_comm_cfg).
	machine().configuration().config_register(
			"linkplay",
			configuration_manager::load_delegate(&namco_c139_device::linkplay_config_load, this),
			configuration_manager::save_delegate(&namco_c139_device::linkplay_config_save, this));

	// Bring up the asio context if -comm_localhost / -comm_remotehost are
	// configured on the command line; when the CLI is at MAME defaults
	// this defers to the cfg/loopback-defaults path (start_comm_cfg at
	// config FINAL).
	start_comm();
}


// Called by the host driver once per frame on the vblank rising edge, on
// the emulation thread.  Two jobs:
//
//   1. Send a frame-token control frame carrying our vblank counter so
//      the peer can measure how far ahead/behind we are in EMULATED time.
//   2. Barrier: if we are more than LOCKSTEP_MAX_LEAD frames ahead of the
//      peer's last token (after subtracting the launch-stagger baseline
//      offset captured at link-up), block this thread until the peer
//      catches up or LOCKSTEP_STALL_TIMEOUT_MS of wall-clock passes.
//
// Stall mechanism: we simply sleep-poll on the emulation thread inside
// this callback.  While we are blocked here the MAME scheduler cannot
// advance, so emulated time (CPUs, timers, our own TX) is frozen - which
// is exactly the semantics a hardware-crystal-locked cabinet pair has.
// Trade-offs: video/input/audio hitch for the stall duration, and after a
// stall MAME's throttle sees emulated time behind wall clock and runs the
// next frames unthrottled until it catches up - the barrier (not the
// throttle) remains the authority on how far ahead we may get, so mutual
// drift stays capped at LOCKSTEP_MAX_LEAD either way.
//
// Robustness:
//   - Inert unless connected and at least one peer token has arrived (so
//     a solo/booting peer can never stall us).
//   - A launch stagger between the two instances makes raw frame
//     counters differ; we baseline the offset on the first peer token
//     and cap RELATIVE drift from link-up.
//   - Every stalled vblank is capped at LOCKSTEP_STALL_TIMEOUT_MS.  After
//     LOCKSTEP_SUSPEND_AFTER consecutive full timeouts we look at whether
//     the peer's token advanced over the streak: if it did not (peer
//     hung, died, or was closed) we SUSPEND the barrier and free-run,
//     resuming (with a fresh baseline) only when peer tokens flow again;
//     if it did advance (pathological mutual-stall / baseline skew) we
//     re-baseline instead.  |drift| > LOCKSTEP_REBASE_DRIFT is treated
//     as a discontinuity and also re-baselines.
void namco_c139_device::vblank_tick()
{
	// Per-vblank stale sweep for a tracked bulk message whose
	// continuation never arrived (e.g. peer disconnected mid-message, or
	// the continuation never resumed the pointer).  A real continuation
	// resumes within a few frames, so a hold that survives the
	// CHUNK_STALE_MS window is a genuine abandon.
	if (!m_chunk_accum.empty()
			&& machine().time() - m_chunk_accum_since > attotime::from_msec(CHUNK_STALE_MS))
		chunk_drop("stale-sweep");

	if (!m_context || !m_context->connected())
		return;

	// 1. Send our frame token (always, even if we are about to stall, so
	// the peer's view of us is current while it decides whether to stall).
	uint32_t const frame_no = ++m_lockstep_local_frame;
	std::vector<uint8_t> token;
	token.reserve(7);
	token.push_back(uint8_t(((LOCKSTEP_CTRL_SIZE_FLAG | 5) >> 8) & 0xff)); // size prefix 0x8005
	token.push_back(uint8_t((LOCKSTEP_CTRL_SIZE_FLAG | 5) & 0xff));
	token.push_back(LOCKSTEP_CTRL_TYPE_TOKEN);
	token.push_back(uint8_t((frame_no >> 24) & 0xff));
	token.push_back(uint8_t((frame_no >> 16) & 0xff));
	token.push_back(uint8_t((frame_no >> 8) & 0xff));
	token.push_back(uint8_t(frame_no & 0xff));
	m_context->send_frame(std::move(token));

	// 2. Barrier - engage only once the peer has sent us at least one token.
	if (m_lockstep_tokens_rx.load(std::memory_order_acquire) == 0)
		return;

	uint32_t const peer_now = m_lockstep_peer_token.load(std::memory_order_acquire);

	if (!m_lockstep_have_baseline)
	{
		m_lockstep_offset = int32_t(frame_no - peer_now);
		m_lockstep_have_baseline = true;
		LOGMASKED(LOG_LINK, "lockstep baseline local=%u peer=%u offset=%d\n",
				frame_no, peer_now, m_lockstep_offset);
		return;
	}

	// Effective drift = how many frames WE are ahead of the peer, relative
	// to the link-up baseline.  Negative = peer is ahead (its problem).
	auto effective_drift =
			[this] () -> int32_t
			{
				return int32_t(m_lockstep_local_frame
						- m_lockstep_peer_token.load(std::memory_order_acquire))
						- m_lockstep_offset;
			};

	int32_t const drift = effective_drift();

	// Discontinuity guard (peer reset / reconnected / we were suspended
	// for a long time): re-baseline rather than stalling toward a huge gap.
	if (drift > LOCKSTEP_REBASE_DRIFT || drift < -LOCKSTEP_REBASE_DRIFT)
	{
		m_lockstep_offset = int32_t(frame_no - peer_now);
		LOGMASKED(LOG_LINK, "lockstep drift discontinuity %d - re-baseline (local=%u peer=%u offset=%d)\n",
				drift, frame_no, peer_now, m_lockstep_offset);
		return;
	}

	if (m_lockstep_suspended)
	{
		if (peer_now != m_lockstep_peer_at_suspend)
		{
			// Peer tokens flowing again: fresh baseline, resume the barrier.
			m_lockstep_offset = int32_t(frame_no - peer_now);
			m_lockstep_suspended = false;
			LOGMASKED(LOG_LINK, "lockstep resumed (peer token %u, new offset=%d)\n",
					peer_now, m_lockstep_offset);
		}
		return;
	}

	if (drift <= LOCKSTEP_MAX_LEAD)
	{
		m_lockstep_consec_timeouts = 0;
		return;
	}

	// We are ahead: stall (bounded) until the peer catches up.
	uint32_t const peer_at_entry = peer_now;
	auto const t0 = std::chrono::steady_clock::now();
	bool timed_out = false;
	while (effective_drift() > LOCKSTEP_MAX_LEAD)
	{
		if (std::chrono::steady_clock::now() - t0
				>= std::chrono::milliseconds(LOCKSTEP_STALL_TIMEOUT_MS))
		{
			timed_out = true;
			break;
		}
		std::this_thread::sleep_for(std::chrono::microseconds(500));
	}

	if (!timed_out)
	{
		m_lockstep_consec_timeouts = 0;
		return;
	}

	++m_lockstep_consec_timeouts;
	if (m_lockstep_consec_timeouts == 1)
		m_lockstep_peer_at_streak = peer_at_entry;
	if (m_lockstep_consec_timeouts < LOCKSTEP_SUSPEND_AFTER)
		return;

	// LOCKSTEP_SUSPEND_AFTER straight full timeouts (~0.6 s of degraded
	// running).  Decide between "peer is gone" and "mutual-stall/skew".
	uint32_t const peer_after = m_lockstep_peer_token.load(std::memory_order_acquire);
	m_lockstep_consec_timeouts = 0;
	if (int32_t(peer_after - m_lockstep_peer_at_streak) < 3)
	{
		// Peer token barely moved across the whole streak: peer hung or
		// disconnected.  Free-run until its tokens resume.
		m_lockstep_suspended = true;
		m_lockstep_peer_at_suspend = peer_after;
		LOGMASKED(LOG_WARN, "lockstep suspended after %u stalled vblanks with idle peer (local=%u peer=%u) - free-running\n",
				LOCKSTEP_SUSPEND_AFTER, m_lockstep_local_frame, peer_after);
	}
	else
	{
		// Peer IS advancing yet we kept timing out: baseline skew or both
		// sides stalling on each other.  Re-baseline to break the cycle.
		m_lockstep_offset = int32_t(m_lockstep_local_frame - peer_after);
		LOGMASKED(LOG_LINK, "lockstep re-baseline after persistent stall (peer advancing; local=%u peer=%u new offset=%d)\n",
				m_lockstep_local_frame, peer_after, m_lockstep_offset);
	}
}


// Auto-clear the IRQ line after the pulse delay set by deliver_rx_frames.
TIMER_CALLBACK_MEMBER(namco_c139_device::irq_pulse_off)
{
	m_irq_cb(CLEAR_LINE);
}


// Driver-pushed link-session phase signal + debounce.  Called from the
// host driver's vblank handler once per frame with mode2 = (linked
// gameplay staged).  HYSTERESIS (anti-flap): the debounced in-game state
// arms only after INGAME_DEBOUNCE_VBLANKS (~1 s) CONSECUTIVE mode-2
// vblanks; ANY other vblank drops it immediately and zeroes the streak -
// so attract mode, the link handshake, mode select and any
// re-establishment across resets/area transitions ALWAYS see the simple
// stop-and-wait behaviour, even if the mode word flickers.
// Emulation-thread only.  Consumer: the TX-complete release gate.
void namco_c139_device::set_ingame(bool mode2, uint32_t mode_word)
{
	if (mode2)
	{
		if (m_ingame_streak < INGAME_DEBOUNCE_VBLANKS)
			++m_ingame_streak;
		if (m_ingame_streak >= INGAME_DEBOUNCE_VBLANKS && !m_ingame)
		{
			m_ingame = true;
			// The TX-complete release goes active on the same debounced
			// edge.  The first stage of the stretch always dispatches
			// (prev-hash/ring were invalidated at the last drop/reset).
			LOGMASKED(LOG_LINK, "tx-complete release ACTIVE (mode_word=%u stable %u vblanks)\n",
					mode_word, INGAME_DEBOUNCE_VBLANKS);
		}
	}
	else
	{
		m_ingame_streak = 0;
		if (m_ingame)
		{
			m_ingame = false;
			// Drop the release IMMEDIATELY on mode loss - the gate
			// key/ring die (the next stretch's first stage must always
			// dispatch), the park dies (a stage sitting in TXSIZE is now
			// the stop-and-wait's business: rx delivery releases it), and
			// the busy window dies (no synthesized busy read may leak
			// into establishment - the very next poll reads the raw
			// register).
			m_txc_prev_valid = false;
			for (unsigned i = 0; i < TXC_HIST; i++)
				m_txc_hist[i].valid = false;
			m_txc_parked_dup = false;
			m_txc_busy_until = attotime::zero;
			m_txc_busy_hw = 0;
			LOGMASKED(LOG_LINK, "tx-complete release INACTIVE (mode_word=%u - stop-and-wait resumes)\n",
					mode_word);
		}
	}
}


// Keepalive replay - keeps the partner's link-state dispatcher firing
// during periods of no game-initiated TX.  The partner's protocol layer
// declares a link timeout when its drift counter climbs past 17 frames
// without a validated frame arriving; replaying the last real TX triggers
// the partner's validator->dispatcher path and resets its timeout.
//
// Bytes 0-1 of the replayed frame are the partner counter the dispatcher
// subtracts from its own state counter to compute the timeout delta.  A
// replay of a stale frame with stale bytes 0-1 would make that delta
// large and leave the timeout above threshold, so the current local
// counter (pushed in by the driver each vblank) is stamped into bytes 0-1
// on every replay.
TIMER_CALLBACK_MEMBER(namco_c139_device::heartbeat_tick)
{
	if (!m_context || !m_context->connected())
		return;
	if (m_last_tx_payload.empty())
		return;

	// While a bulk chunk sequence is open (head forwarded, remainder
	// still to come), HOLD the replay - a device-invented frame
	// interleaved between the game's chunks would land in the peer's RX
	// ring mid-message, and the peer's drain would then checksum a
	// chunk+replay span (guaranteed checksum failure) and desync the
	// message.  The game's own TX routine never interleaves anything
	// mid-message; keep the wire that shape.  Re-arm so the replay
	// resumes once the sequence completes/drops (the stale sweep bounds
	// the wait).
	if (!m_chunk_accum.empty())
	{
		if (m_heartbeat_timer)
			m_heartbeat_timer->adjust(attotime::from_msec(HEARTBEAT_CADENCE_MS));
		return;
	}

	// STALE-REPLAY AGE-OUT.  Frames larger than 0xFF halfwords are
	// capture-excluded (restamping them would corrupt their framing), so
	// during long stretches of bulk-only traffic the captured replay
	// payload can grow arbitrarily old while replays keep firing.  While
	// the in-game TX-complete release is active the peer no longer needs
	// the replay for pacing (it self-releases at its own stage instants),
	// so a stale replay only feeds it outdated state: SUPPRESS the replay
	// when the capture is older than HEARTBEAT_STALE_MS - re-arm and
	// return, the same hold shape as the open-sequence block above.
	// Outside the in-game state the replay is untouched - it still paces
	// link establishment.
	if (m_ingame
			&& machine().time() - m_txc_cap_time > attotime::from_msec(HEARTBEAT_STALE_MS))
	{
		if (!m_txc_first_stale_logged)
		{
			m_txc_first_stale_logged = true;
			LOGMASKED(LOG_FRAME, "first stale-replay suppression (cap_age_ms=%u) - keepalive replay withheld, captured payload too old to restamp; one-shot line, later suppressions are silent\n",
					unsigned((machine().time() - m_txc_cap_time).as_double() * 1000.0 + 0.5));
		}
		if (m_heartbeat_timer)
			m_heartbeat_timer->adjust(attotime::from_msec(HEARTBEAT_CADENCE_MS));
		return;
	}

	uint32_t const payload_size = uint32_t(m_last_tx_payload.size());

	// Build the on-wire frame: 16-bit big-endian length prefix + payload.
	std::vector<uint8_t> frame;
	frame.reserve(payload_size + 2);
	frame.push_back(uint8_t((payload_size >> 8) & 0xff));
	frame.push_back(uint8_t(payload_size & 0xff));
	frame.insert(frame.end(), m_last_tx_payload.begin(), m_last_tx_payload.end());

	// Stamp our current state counter into bytes 0-1 of the payload
	// (frame[2..3] after the 2-byte length prefix).
	uint16_t const stamp = m_local_counter;
	if (payload_size >= 2)
	{
		frame[2] = uint8_t((stamp >> 8) & 0xff);
		frame[3] = uint8_t(stamp & 0xff);
	}

	m_context->send_frame(std::move(frame));

	// Re-arm.
	if (m_heartbeat_timer)
		m_heartbeat_timer->adjust(attotime::from_msec(HEARTBEAT_CADENCE_MS));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namco_c139_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), uint16_t(0));

	// clear the chunked-TX tracking state
	m_chunk_tx_ptr = 0;
	m_chunk_tx_ptr_valid = false;
	m_chunk_expected_hw = 0;
	m_chunk_held_expected_hw = 0;
	m_chunk_bulk_pending = false;
	m_chunk_accum.clear();
	m_chunk_accum_since = attotime::zero;
	m_chunk_msg_start_ptr = 0;
	m_chunk_resume_ptr = 0;
	m_chunk_msg_chunks = 0;
	m_chunk_saw_txoffset = false;

	// Drop the debounced in-game state across a reset - link
	// re-establishment must ALWAYS see the stop-and-wait behaviour; the
	// debounce then requires a fresh ~1 s of stable linked gameplay
	// before it re-arms.
	if (m_ingame)
		LOGMASKED(LOG_LINK, "tx-complete release INACTIVE (device reset) - stop-and-wait resumes; debounce restarts\n");
	m_ingame = false;
	m_ingame_streak = 0;

	// Clear the TX-complete release runtime state: gate key, history
	// ring, park and busy window all die here, and the capture-time
	// anchor restarts with the first post-reset capture.
	m_txc_prev_valid = false;
	m_txc_prev_hash = 0;
	m_txc_prev_len = 0;
	for (unsigned i = 0; i < TXC_HIST; i++)
	{
		m_txc_hist[i].hash = 0;
		m_txc_hist[i].len = 0;
		m_txc_hist[i].valid = false;
	}
	m_txc_hist_idx = 0;
	m_txc_busy_until = attotime::zero;
	m_txc_busy_hw = 0;
	m_txc_parked_dup = false;
	m_txc_cap_time = attotime::zero;
	m_txc_first_dispatch_logged = false;
	m_txc_first_release_logged = false;
	m_txc_first_stale_logged = false;

	// clear the announce-latch runtime state
	m_al_valid = false;
	m_al_wiped = false;
	m_al_offset = 0;
	m_al_expected_hw = 0;
	m_al_wiped_hw = 0;
	m_al_time = attotime::zero;
	m_al_latched = 0;
	m_al_refreshed = 0;
	m_al_wipes_captured = 0;
	m_al_dispatched = 0;
	m_al_expired = 0;
	m_al_superseded = 0;

	// clear the snapshot (clear() keeps the reserved capacity)
	m_al_snap_valid = false;
	m_al_snap_offset = 0;
	m_al_snap.clear();
	m_al_snap_dispatched = 0;
	m_al_snap_fallback = 0;

	// clear the dedupe ring + counters
	for (auto &rec : m_al_completes)
		rec = al_complete_rec();
	m_al_comp_idx = 0;
	m_al_deduped = 0;
	m_al_refresh_retired = 0;
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector namco_c139_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_DATA, &m_space_config)
	};
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint16_t namco_c139_device::ram_r(offs_t offset)
{
	// Drain any pending RX frames before serving the read.  The link
	// probe window is short (~300 game ticks) and the game RAMs are
	// accessed heavily during it; piggy-backing the drain on RAM reads
	// gets received frames into the game's hands faster than waiting
	// for the next reg_r / status_r call.
	deliver_rx_frames(0);
	return m_ram[offset];
}

void namco_c139_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ram[offset]);
}

uint16_t namco_c139_device::status_r()
{
	// Without a configured transport only the bare status skeleton is
	// modelled: a constant "ready" status, nothing else.
	if (!m_context)
		return 4;

	deliver_rx_frames(0);
	/*
	 x-- RX READY or irq pending?
	 -x- IRQ direction: 1 RX cause - 0 TX cause
	*/
	uint16_t result = 4;   // "ready" bit

	// Mirror the RX-flag bit from Status/Control (m_regs[1] bit 1) so an
	// IRQ handler reading the RX-Status register sees "frame received"
	// when one's been delivered into RAM.  Cleared when the game writes
	// to Status/Control to clear bits 1+2.
	if (m_regs[1] & 0x02)
		result |= 0x02;

	return result;
}

uint16_t namco_c139_device::reg_r(offs_t offset)
{
	// Without a configured transport the register file is not modelled;
	// these reads return 0, matching the no-op/unmapped reads of the bare
	// register skeleton (the surrounding buses read 0 for unmapped).
	if (!m_context)
		return 0;

	deliver_rx_frames(0);
	// regs_map calls us with offset relative to the 0x02..0x0f range, i.e.
	// host byte address 0x02 -> offset 0, address 0x04 -> offset 1, etc.
	// We mirror that into m_regs[1..7], leaving m_regs[0] reserved for the
	// status register handled by status_r().
	const offs_t reg_idx = (offset + 1) & 0x7;

	// MODELLED TX-BUSY (in-game TX-complete release, part 2).  While the
	// modelled serialization window of the last dispatch (or of a
	// duplicate re-stage that landed on an idle serializer - see reg_w)
	// is open, every TXSIZE read answers a synthesized BUSY value (the
	// in-flight TXSIZE, low byte forced non-zero for the game's low-byte
	// busy test) WITHOUT touching the register file - the game's TX pump
	// bails at its entry gate exactly as on real hardware while the frame
	// serializes.  The FIRST poll after the window closes releases a
	// parked duplicate (m_regs[5] -> 0, the one deliberate TXSIZE write
	// of this path - the parked bytes were hash-verified already-crossed
	// content at stage time; if the host meanwhile recomposed the slot,
	// the pump re-stages the new content on the very passage this release
	// enables and the gate dispatches it fresh).  A park whose register
	// was meanwhile wiped by rx delivery / the game's own TXSIZE=0 write
	// is dropped silently (nothing to clear).  Gated on the debounced
	// in-game state: during establishment no synthesized value is ever
	// returned (set_ingame/device_reset also clear the window, so none
	// can leak in).
	bool txc_busy_read = false;
	uint16_t txc_busy_val = 0;
	if (m_ingame && reg_idx == 5)
	{
		if (machine().time() < m_txc_busy_until)
		{
			txc_busy_read = true;
			txc_busy_val = ((m_txc_busy_hw & 0xff) != 0)
					? m_txc_busy_hw : uint16_t(m_txc_busy_hw | 0x0001);
		}
		else if (m_txc_parked_dup)
		{
			m_txc_parked_dup = false;
			if (m_regs[5] != 0)
			{
				m_regs[5] = 0;
				if (!m_txc_first_release_logged)
				{
					m_txc_first_release_logged = true;
					LOGMASKED(LOG_FRAME, "first parked-duplicate release (TXSIZE cleared at the first busy-poll after the modelled window; one-shot line, later releases are silent)\n");
				}
			}
		}
	}

	// While the modelled TX-busy window is open the game reads the
	// synthesized busy value (register file untouched); every other read
	// returns the stored register.
	return txc_busy_read ? txc_busy_val : m_regs[reg_idx];
}

void namco_c139_device::reg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	const offs_t reg_idx = (offset + 1) & 0x7;

	// Without a configured transport only the IRQ control below acts -
	// the IRQ line needs no link partner (System 23 exercises it during
	// boot); every other write is discarded, like the bare register
	// skeleton's.
	if (!m_context)
	{
		if (reg_idx == 1)
		{
			if (data == 0xfffb)
				m_irq_cb(ASSERT_LINE);
			else if (data == 0x000f)
				m_irq_cb(CLEAR_LINE);
		}
		return;
	}

	deliver_rx_frames(0);

	// Register 1 (host byte 0x02..0x03) doubles as an IRQ control register
	// in addition to a status mirror.  Two magic values were established
	// empirically:
	//   0xfffb -> assert IRQ output line
	//   0x000f -> deassert IRQ output line
	// Other values just store and let the host poll bits.
	if (reg_idx == 1)
	{
		if (data == 0xfffb)
			m_irq_cb(ASSERT_LINE);
		else if (data == 0x000f)
			m_irq_cb(CLEAR_LINE);
	}

	// Snapshot register 3 (host byte 0x06, TX Control)'s bit-0 state
	// before the update so we can detect a 0 -> 1 edge after.
	uint16_t const old_tx_bit0 = (reg_idx == 3) ? (m_regs[3] & 0x01) : 0;

	COMBINE_DATA(&m_regs[reg_idx]);

	// A TXOFFSET write is the game TX routine's "new message" event
	// (continuation chunks never write it).  Latch the modelled
	// auto-advancing DMA pointer and read the new message's TOTAL
	// halfword count from the two size cells the routine just consumed
	// at TXOFFSET-2 / TXOFFSET-1 (it reads the size cells at the slot
	// base, then programs TXOFFSET = base+2).  expected > 0xFF announces
	// a multi-chunk bulk message.
	//
	// A tracked bulk message is NOT torn down here: it survives unrelated
	// interleaved TXOFFSET writes and is retired only when it completes,
	// overshoots, goes stale, or is explicitly superseded by a NEW
	// saturated bulk message in send_pending_tx_frame().  The new pointer
	// latched here becomes the read pointer for the next triggered send;
	// if that send turns out to be the tracked message's continuation it
	// will resume m_chunk_resume_ptr, otherwise it is an interleaved
	// message that passes through while the bulk message stays tracked.
	if (reg_idx == 7)
	{
		m_chunk_tx_ptr = m_regs[7];
		m_chunk_tx_ptr_valid = true;
		m_chunk_saw_txoffset = true;   // new-message reprogram - the next send is NOT a continuation
		uint16_t const ptr = m_regs[7];
		m_chunk_expected_hw = (uint32_t(m_ram[(ptr - 2) & 0x1fff] & 0xff) << 8)
							|  uint32_t(m_ram[(ptr - 1) & 0x1fff] & 0xff);
		m_chunk_bulk_pending = (m_chunk_expected_hw > CHUNK_SAT_HW
							 && m_chunk_expected_hw <= CHUNK_MAX_HW);

		// Announce latch ARM / REFRESH / SUPERSEDE at the game's
		// new-message announce.  A BULK announce (expected in
		// (0xFF..0x400]) arms the latch with (offset, expected, now); the
		// SAME class re-announced (same offset+expected - the game's own
		// retry of a lost class) REFRESHES it (new TTL anchor, fresh
		// one-shot); any OTHER announce SUPERSEDES it - the newest stage
		// always wins, and a stale (offset, size) must never reach the
		// wire.  A NON-bulk announce while a latch is live also
		// supersedes (the game moved on to another message).
		if (m_chunk_bulk_pending)
		{
			bool const al_refresh = m_al_valid && m_al_offset == m_regs[7]
					&& m_al_expected_hw == m_chunk_expected_hw;
			if (m_al_valid && !al_refresh)
			{
				++m_al_superseded;
				LOGMASKED(LOG_FRAME, "announce-latch: superseded offset=0x%04x expected_hw=%u new_offset=0x%04x new_expected_hw=%u superseded=%u (new bulk announce replaces the latch)\n",
						m_al_offset, m_al_expected_hw,
						m_regs[7], m_chunk_expected_hw, m_al_superseded);
			}
			// A same-class re-announce refreshing a latch with a pending
			// wiped dispatch retires that older wiped stage (the game's
			// own newer announce wins) - counted for observability.
			if (al_refresh && m_al_wiped)
			{
				++m_al_refresh_retired;
				LOGMASKED(LOG_FRAME, "announce-latch: refresh retired a pending wiped dispatch offset=0x%04x hw=%u expected_hw=%u refresh_retired=%u\n",
						m_al_offset, m_al_wiped_hw, m_al_expected_hw, m_al_refresh_retired);
			}
			m_al_valid = true;
			m_al_wiped = false;
			m_al_wiped_hw = 0;
			m_al_offset = m_regs[7];
			m_al_expected_hw = m_chunk_expected_hw;
			m_al_time = machine().time();
			if (al_refresh)
				++m_al_refreshed;
			else
				++m_al_latched;
			LOGMASKED(LOG_FRAME, "announce-latch: latched offset=0x%04x expected_hw=%u refresh=%d latched=%u refreshed=%u\n",
					m_al_offset, m_al_expected_hw,
					al_refresh ? 1 : 0, m_al_latched, m_al_refreshed);
		}
		else if (m_al_valid)
		{
			++m_al_superseded;
			m_al_valid = false;
			m_al_wiped = false;
			LOGMASKED(LOG_FRAME, "announce-latch: superseded offset=0x%04x expected_hw=%u new_offset=0x%04x new_expected_hw=%u superseded=%u (non-bulk announce - latch dropped)\n",
					m_al_offset, m_al_expected_hw,
					m_regs[7], m_chunk_expected_hw, m_al_superseded);
		}
	}

	// IRQ-ack write (0x000f) also clears the pending RX-status bits (1, 2).
	// Without this, status_r keeps reporting bit 1 set after the game's
	// IRQ handler runs; the dispatcher re-fires the RX handler on stale
	// data, the missed-marker counter ticks up, and the link times out
	// even though one frame did successfully validate.
	if (reg_idx == 1 && data == 0x000f)
		m_regs[1] &= ~uint16_t(0x06);

	// TXSIZE-commit TX trigger for a bulk REMAINDER chunk.
	//
	// The game programs a bulk message's remainder correctly (right size
	// = announced - 0xFF, right resume DMA pointer) but writes it as
	// `TXSIZE=<remainder>` followed by a TX Control write with bit 0 LOW
	// (no rising edge), so the START-rising-edge trigger below never
	// fires for it - on the real chip a non-zero TXSIZE written while
	// the chip is armed is itself a transmit trigger.  Model that second
	// trigger condition for the held-continuation case.  We key on the
	// pending-continuation STATE, not on the raw START bit: the head's
	// START rising edge clears bit 0 synchronously BEFORE the game
	// writes the remainder TXSIZE, so the raw bit is 0 at the remainder
	// write.  Fire only when a bulk continuation is genuinely held and
	// resumable:
	//   - !m_chunk_accum.empty()  : a head (or earlier remainder) is tracked;
	//   - !m_chunk_saw_txoffset   : NO TXOFFSET reprogram since the last
	//                               send => continuation, NOT a new
	//                               message (the head write has
	//                               saw_txoffset=1 and an EMPTY tracker,
	//                               so it is excluded on both counts and
	//                               can never be re-fired here);
	//   - (m_regs[5] & 0xff) != 0 : an actual remainder size was just
	//                               staged (an idle TXSIZE=0 tick is
	//                               ignored).
	// send_pending_tx_frame()'s is_continuation branch (computed from the
	// SAME flags) then reads from m_chunk_resume_ptr and associates; it
	// also synchronously clears m_regs[5] -> 0, so a duplicate fire on
	// the same write is impossible and the trailing TX Control writes see
	// TXSIZE == 0 (no stale re-send).  A multi-remainder message
	// (>510 halfwords) is handled naturally: each remainder is its own
	// TXSIZE write, so each gets its own commit trigger.
	if (reg_idx == 5
			&& (m_regs[5] & 0xff) != 0
			&& !m_chunk_accum.empty()
			&& !m_chunk_saw_txoffset)
	{
		send_pending_tx_frame();
	}

	// In-game TX-complete release, part 1: the ADMISSION GATE at the
	// TXSIZE stage site.
	//
	// The game's TX pump writes TX Control BEFORE TXOFFSET/TXSIZE in
	// every passage, so a TX Control-edge send always fires one stage
	// early (the zero-size abort beat); under the emulated stop-and-wait
	// a staged frame reaches the wire only when a peer delivery lands
	// exactly at a pump gate read.  The real chip needs no such
	// interleave: a non-zero TXSIZE written while armed IS its transmit
	// trigger, and TXSIZE->0 plus the TX-done IRQ follow from the
	// serialization itself - hence the synchronous dispatch at the stage
	// instant modelled here.
	//
	// THE GATE: the pump re-offers pending content as strictly adjacent
	// re-stages, so dispatch IFF the staged image's FNV-1a hash differs
	// from the previously dispatched stage (no sequence numbers, no
	// per-class state; the TXC_HIST ring is belt-and-braces for rare
	// A-B-A transients).  TXSIZE=0 writes are idle pump passages
	// (bursting to thousands per second at phase transitions): ignored
	// entirely - no dispatch, no gate/window/park perturbation.  A
	// hash-identical re-stage is PARKED (released at the first busy-poll
	// after the modelled window closes, see reg_r) and - when it lands on
	// an IDLE serializer - opens a busy window itself, so the passage
	// that staged it can never be instantly re-released (the pump keeps
	// its hardware pacing instead of spinning).
	//
	// Structurally disjoint from the commit trigger above:
	// fresh_standalone requires saw_txoffset + an empty tracker = the
	// exact complement of the continuation predicate.
	if (m_ingame && reg_idx == 5)
	{
		if (m_regs[5] == 0)
		{
			// idle pump passage - ignored entirely
		}
		else
		{
			uint16_t const stage_hw = m_regs[5];
			// Structural admission: a fresh standalone stage (including a
			// >255-halfword whole-frame burst); chunk-train heads and
			// continuations stay with the stop-and-wait/commit machinery.
			bool const single_burst_whole = m_chunk_bulk_pending
					&& stage_hw == m_chunk_expected_hw;
			bool const fresh_standalone = stage_hw >= 4 && stage_hw <= CHUNK_MAX_HW
					&& m_chunk_saw_txoffset && m_chunk_accum.empty()
					&& (!m_chunk_bulk_pending || single_burst_whole);
			if (!fresh_standalone)
			{
				// Not this gate's business - and the register now holds a
				// stage the stop-and-wait path owns, so a stale park must
				// never clear it at a poll.
				m_txc_parked_dup = false;
			}
			else
			{
				uint16_t const ptr = m_chunk_tx_ptr_valid ? m_chunk_tx_ptr : m_regs[7];
				// The game's strict trailer invariant on the staged RAM
				// image: the final halfword carries the bit-8 end marker
				// AND the trailer's claimed length equals the frame's own
				// halfword count.  A multi-chunk message FRAGMENT never
				// satisfies it - an intermediate chunk has no trailer,
				// and a last chunk's claimed length is the whole-message
				// total, not the chunk's own size.
				uint16_t const endhw  = m_ram[uint16_t(ptr + stage_hw - 1) & 0x1fff];
				uint16_t const sizehi = m_ram[uint16_t(ptr + stage_hw - 2) & 0x1fff];
				uint32_t const claimed = (uint32_t(sizehi & 0xff) << 8) | (endhw & 0xff);
				if (!((endhw & 0x100) && claimed == stage_hw))
				{
					// Incomplete/torn stage: leave it to the stop-and-wait
					// path (gate state untouched, so a later completed
					// re-stage of this content hashes differently and
					// dispatches fresh).
					m_txc_parked_dup = false;
				}
				else
				{
					uint32_t const h = ts_hash_ram(m_ram, ptr, uint32_t(stage_hw));
					bool const dup_prev = m_txc_prev_valid
							&& h == m_txc_prev_hash && stage_hw == m_txc_prev_len;
					bool dup_ring = false;
					for (unsigned i = 0; i < TXC_HIST && !dup_ring; i++)
						dup_ring = m_txc_hist[i].valid
								&& m_txc_hist[i].hash == h
								&& m_txc_hist[i].len == stage_hw;
					if (dup_prev || dup_ring)
					{
						// Already-crossed content re-offered by the pump.
						// PARK; the first post-window busy-poll releases
						// it (reg_r).  On an idle serializer the re-stage
						// opens its own window - on real hardware these
						// bytes WOULD serialize for ~a frame before
						// TXSIZE cleared, and without that hold the poll
						// right after this write would release a new
						// passage instantly.
						m_txc_parked_dup = true;
						if (machine().time() >= m_txc_busy_until)
						{
							m_txc_busy_until = machine().time()
									+ attotime::from_msec(TX_BUSY_MS);
							m_txc_busy_hw = stage_hw;
						}
						// window already open: pure no-op (never extended -
						// a same-vblank duplicate beat rides through)
					}
					else
					{
						// FRESH content - dispatch synchronously at the
						// stage instant (the TXSIZE-commit trigger; the
						// staged bytes are pristine right now).
						send_pending_tx_frame();
						// The emit path's own synchronous TXSIZE clear is
						// the TX handoff; if it did not run (no peer
						// connected / size-cap abort), nothing crossed -
						// gate state stays untouched and the stop-and-wait
						// behaviour owns the stage.  FULL 16-bit compare
						// (not the low byte): a 0x100/0x200-halfword
						// stage has a zero low byte, which would fake a
						// clear on an aborted send.
						if (m_regs[5] == 0)
						{
							m_txc_prev_hash = h;
							m_txc_prev_len = stage_hw;
							m_txc_prev_valid = true;
							m_txc_hist[m_txc_hist_idx].hash = h;
							m_txc_hist[m_txc_hist_idx].len = stage_hw;
							m_txc_hist[m_txc_hist_idx].valid = true;
							m_txc_hist_idx = uint8_t((m_txc_hist_idx + 1) % TXC_HIST);
							m_txc_parked_dup = false;
							m_txc_busy_until = machine().time()
									+ attotime::from_msec(TX_BUSY_MS);
							m_txc_busy_hw = stage_hw;
							if (!m_txc_first_dispatch_logged)
							{
								m_txc_first_dispatch_logged = true;
								LOGMASKED(LOG_FRAME, "first stage-instant dispatch (hw=%u hash=%08x ptr=0x%04x) - fresh-content stage transmitted at its stage instant, busy-poll reads BUSY for %u ms; one-shot line, later dispatches are silent\n",
										unsigned(stage_hw), h, unsigned(ptr), TX_BUSY_MS);
							}
						}
					}
				}
			}
		}
	}

	// TX trigger: writing register 3 with bit 0 transitioning 0 -> 1
	// initiates a frame send pulled from RAM at TX FIFO Pointer (reg 7)
	// of length TX Frame Size (reg 5).
	if (reg_idx == 3 && !old_tx_bit0 && (m_regs[3] & 0x01))
	{
		send_pending_tx_frame();
		// Real hardware clears the TX trigger bit on completion.  We
		// model this synchronously since our "transmission" finishes
		// the moment we hand off to asio.  Leaving the bit set would
		// have the game's link probe interpret "TX still in flight"
		// and refuse to advance its TX script position.
		m_regs[3] &= ~uint16_t(0x01);
	}
}
