// license:BSD-3-Clause
// copyright-holders:Angelo Salese, pocketjazzy
/***************************************************************************

    Namco C139 - Serial I/F Controller

    Inter-cabinet serial link controller (RS-422).  On System 23 PCBs the
    part is silkscreened "C422", believed to be a pin-compatible
    faster-clocked revision of C139.

    The link is emulated as a TCP bridge between two MAME instances; the
    transport approach derives from SailorSat (Ariane Fugmann)'s C139
    link work for the System 21 era boards.

***************************************************************************/
#ifndef MAME_NAMCO_NAMCO_C139_H
#define MAME_NAMCO_NAMCO_C139_H

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// the per-machine cfg handlers below take the configuration-manager types
// by value / pointer only
enum class config_type : int;
enum class config_level : int;
namespace util::xml { class data_node; }


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> namco_c139_device

class namco_c139_device : public device_t,
						  public device_memory_interface
{
public:
	// construction/destruction
	namco_c139_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	~namco_c139_device();   // out-of-line: m_context (unique_ptr<context>) needs class context to be complete

	// configuration
	auto irq_handler() { return m_irq_cb.bind(); }

	// I/O operations
	void regs_map(address_map &map) ATTR_COLD;

	uint16_t status_r();

	uint16_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void data_map(address_map &map) ATTR_COLD;

	// Host driver pushes the game's link state-machine call counter here
	// every vblank so the keepalive replay timer can stamp it into bytes
	// 0-1 of replayed frames - the peer's protocol dispatcher reads those
	// bytes as the partner counter when computing its link-timeout delta.
	void set_local_counter(uint16_t counter) { m_local_counter = counter; }

	// Host driver pushes the game's link-session phase here every vblank:
	// mode2 = linked gameplay/cutscene staged (only the driver can read the
	// game's mode word - it maps main RAM).  The device debounces the
	// signal (INGAME_DEBOUNCE_VBLANKS consecutive mode-2 vblanks before the
	// in-game state arms, dropped immediately on loss) and models the
	// chip's TX-complete release only while the debounced state holds;
	// link establishment always sees the simple stop-and-wait behaviour.
	// Emulation-thread only (written at vblank, read on the emulation
	// thread - no atomics needed).
	void set_ingame(bool mode2, uint32_t mode_word);

	// Per-vblank frame-token barrier.  The host driver calls vblank_tick()
	// once per frame (vblank rising edge); the device sends a frame-token
	// control frame to the peer and stalls (bounded, wall-clock) whenever
	// the local frame count runs more than LOCKSTEP_MAX_LEAD frames ahead
	// of the last token received from the peer - modelling the mutual
	// crystal-locked frame pacing two linked cabinets have.
	void vblank_tick();

	// true while a transport is configured AND its peer connection is
	// established - the host driver keeps its per-frame link service away
	// from the game's RAM unless this holds.  Out-of-line: needs the
	// complete context type.
	bool is_linked() const;

protected:
	// inner class hosting the asio io_context, sockets, and worker thread
	class context;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	uint16_t reg_r(offs_t offset);
	void reg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void start_comm() ATTR_COLD;
	void send_pending_tx_frame();
	void deliver_rx_frames(int32_t param);

	// TX-side handling of the host CPU's chunked bulk transmissions.  The
	// game's TX routine splits any message larger than 0xFF halfwords into
	// successive TXSIZE=0xFF chunks plus a remainder, writing TXOFFSET only
	// for the FIRST chunk and relying on the chip auto-advancing its DMA
	// pointer for the rest.  The device models that pointer and tracks the
	// multi-chunk message so each chunk goes on the wire correctly.
	void chunk_drop(const char *reason);
	// record a wire completion of a latched bulk class into the dedupe ring
	// (called at the natural head-send retirement and at a latch dispatch;
	// the definition documents why)
	void al_dedupe_record(uint16_t offset, uint32_t expected_hw);
	// bulk_chunk=true marks a chunk of an in-progress >255-halfword message
	// forwarded individually - excluded from keepalive capture (a chunk is
	// a message FRAGMENT; replaying or restamping one would interleave
	// garbage mid-message).  Default false for every other caller.
	void emit_tx_frame(std::vector<uint8_t> payload, bool bulk_chunk = false);

	TIMER_CALLBACK_MEMBER(irq_pulse_off);
	TIMER_CALLBACK_MEMBER(heartbeat_tick);

	const address_space_config m_space_config;
	devcb_write_line m_irq_cb;
	uint16_t* m_ram = nullptr;
	uint16_t m_regs[8];
	std::unique_ptr<context> m_context;
	emu_timer *m_irq_pulse_timer = nullptr;

	// Keepalive replay state: the last real TX is replayed after a cadence
	// of TX silence so the peer's link-state dispatcher keeps firing (the
	// game declares a link timeout when its drift counter reaches 17
	// frames; see heartbeat_tick).
	emu_timer *m_heartbeat_timer = nullptr;
	std::vector<uint8_t> m_last_tx_payload;   // last real TX, replayed periodically
	uint16_t m_local_counter = 0;             // pushed in by host vblank

	// Frame-token lockstep state.
	// Ownership: everything here is emulation-thread-only EXCEPT
	// m_lockstep_peer_token / m_lockstep_tokens_rx, which the asio network
	// thread writes (via lockstep_token_received()) and the emulation
	// thread reads - hence the atomics.
	void lockstep_token_received(uint32_t token)   // network thread
	{
		m_lockstep_peer_token.store(token, std::memory_order_release);
		m_lockstep_tokens_rx.fetch_add(1, std::memory_order_release);
	}

	uint32_t m_lockstep_local_frame = 0;           // our vblank count = token payload
	std::atomic<uint32_t> m_lockstep_peer_token{0};// last token from peer
	std::atomic<uint32_t> m_lockstep_tokens_rx{0}; // count of tokens from peer
	bool m_lockstep_have_baseline = false;         // launch-stagger offset captured
	int32_t m_lockstep_offset = 0;                 // local - peer at link-up
	bool m_lockstep_suspended = false;             // free-running (peer token flow died)
	uint32_t m_lockstep_peer_at_suspend = 0;       // peer token when we suspended
	uint32_t m_lockstep_peer_at_streak = 0;        // peer token at start of timeout streak
	uint32_t m_lockstep_consec_timeouts = 0;       // consecutive full-timeout stalls

	// Chunked-TX tracking (all emulation-thread only).
	//
	// A chunk is associated with its message by TX POINTER CONTINUITY: a
	// held bulk message records the slot pointer it began from
	// (m_chunk_msg_start_ptr) and the advanced DMA pointer the next chunk
	// must resume from (m_chunk_resume_ptr).  A later chunk whose read
	// pointer resumes m_chunk_resume_ptr is the continuation and is
	// tracked as such until the accumulated halfword count reaches the
	// total announced in the message's size cells.  A chunk staged after a
	// fresh TXOFFSET write is a NEW message, not a continuation, even if
	// it reuses the same slot - the host's continuation path never
	// rewrites TXOFFSET, so on the real chip the DMA pointer keeps
	// advancing from where the previous chunk ended.
	uint16_t m_chunk_tx_ptr = 0;                   // auto-advancing TX DMA pointer (halfword index)
	bool m_chunk_tx_ptr_valid = false;             // latched at least once (host wrote TXOFFSET)
	uint32_t m_chunk_expected_hw = 0;              // total message halfwords from the 2 size cells before TXOFFSET (LAST announced)
	uint32_t m_chunk_held_expected_hw = 0;         // expected total of the HELD message, snapshotted at hold (an interleaved TXOFFSET write would otherwise clobber m_chunk_expected_hw)
	bool m_chunk_bulk_pending = false;             // expected > 0xFF announced, message not yet completed/dropped
	std::vector<uint8_t> m_chunk_accum;            // association/progress tracker for the held bulk message (chunks also go on the wire individually as they are associated)
	attotime m_chunk_accum_since;                  // emulated time of the first held chunk (staleness)
	uint16_t m_chunk_msg_start_ptr = 0;            // slot pointer (TXOFFSET) the held bulk message began at
	uint16_t m_chunk_resume_ptr = 0;               // advanced DMA pointer the next continuation chunk must resume from
	uint32_t m_chunk_msg_chunks = 0;               // chunks (first + continuations) accumulated for the CURRENT held message
	bool m_chunk_saw_txoffset = false;             // a TXOFFSET write happened since the last send (new-message reprogram); a send with this clear is a continuation

	// Debounced in-game state fed by the driver's set_ingame() push (gates
	// the TX-complete release below).
	bool m_ingame = false;                         // debounced phase: true = stable linked-gameplay staging
	uint32_t m_ingame_streak = 0;                  // consecutive mode-2 vblanks seen (debounce counter)

	// Announce latch.
	//
	// On the emulated link, delivering a peer frame clears a staged TXSIZE
	// (the "rx_clear" release that the game's link-up busy-poll expects).
	// On the real chip an RX never clears a staged TX, so this emulation
	// compromise can race the host: it stages a bulk message head's
	// TXSIZE, a pending peer frame is delivered at the top of the very
	// register write that carries the START edge and wipes the staged
	// size, and the send would silently read size 0 - the host (whose
	// busy byte reads 0 either way) then stages the remainder into dead
	// air and eventually abandons the message.  The latch remembers a
	// staged bulk announce across that wipe - (offset, expected size,
	// time) recorded at each bulk TXOFFSET announce - and reconstructs
	// the send when the START edge arrives with a wiped size:
	//  - the payload transmitted is a SNAPSHOT copied at wipe-capture
	//    time, the one instant the staged bytes are provably pristine
	//    (the host staged them; the wipe hits only the register) - by
	//    dispatch time the host may already be recomposing the same ring
	//    slot for its next frame;
	//  - a dispatch whose (offset, expected size) class already completed
	//    on the wire within AL_DEDUPE_LOOKBACK_MS of its announce is
	//    consumed WITHOUT sending: the wiped stage was the host's cadence
	//    re-send of content the peer already ingested (the wipe was flow
	//    control, not loss), and re-sending it would deliver a stale
	//    duplicate assembled from two compose generations;
	//  - one-shot per announce; TTL'd; superseded by any newer host
	//    stage; the register file is NEVER written by the latch (the
	//    rx_clear release stays honoured).
	bool m_al_valid = false;                       // a live latch exists (announce -> head-send window)
	bool m_al_wiped = false;                       // the staged TXSIZE was rx_clear-wiped since the announce
	uint16_t m_al_offset = 0;                      // TXOFFSET of the latched bulk announce (head read pointer)
	uint32_t m_al_expected_hw = 0;                 // total announced halfwords (from the size cells)
	uint16_t m_al_wiped_hw = 0;                    // the staged TXSIZE value the wipe destroyed (dispatch size)
	attotime m_al_time;                            // announce/refresh emulated time (TTL anchor)
	uint32_t m_al_latched = 0;                     // cumulative event counters (reported on their per-event log lines)
	uint32_t m_al_refreshed = 0;                   //   same-class re-announces that re-timed the latch
	uint32_t m_al_wipes_captured = 0;              //   staged-TXSIZE wipes remembered by a live latch
	uint32_t m_al_dispatched = 0;                  //   sends reconstructed from the latch (== wiped-then-dispatched)
	uint32_t m_al_expired = 0;                     //   latches dropped past TTL (never dispatched)
	uint32_t m_al_superseded = 0;                  //   latches dropped by a newer/other host stage

	// wipe-time payload snapshot (see above)
	bool m_al_snap_valid = false;                  // snapshot below was captured for the CURRENT wiped latch
	uint16_t m_al_snap_offset = 0;                 // shared-RAM word offset the snapshot was copied from (== m_al_offset at capture)
	std::vector<uint8_t> m_al_snap;                // wipe-time payload copy, wire byte order (reserved once, reused)
	uint32_t m_al_snap_dispatched = 0;             // dispatches that transmitted the snapshot
	uint32_t m_al_snap_fallback = 0;               // dispatches that had to re-read RAM (defensive rail; expect 0)

	// dispatch dedupe ring (see above).  Key choice (offset, expected_hw),
	// not offset alone: the host's TX ring reuses 4 slots, so offset alone
	// would false-match a DIFFERENT message class that rotated into the
	// slot; the expected size separates the classes.
	static constexpr unsigned AL_DEDUPE_RING = 16; // recent-completion entries (>1.5 s of history at peak observed message rate)
	struct al_complete_rec
	{
		uint16_t offset = 0;                       // shared-RAM word offset (TXOFFSET) of the completed head
		uint32_t expected_hw = 0;                  // announced total halfwords (the class key's second half)
		attotime t;                                // emulated time the head went on the wire
		bool valid = false;                        // entry holds a real completion
	};
	al_complete_rec m_al_completes[AL_DEDUPE_RING];// completion ring (overwrite-oldest, fixed size, no allocation)
	uint8_t m_al_comp_idx = 0;                     // next ring write slot
	uint32_t m_al_deduped = 0;                     // dispatches suppressed: class completed within the lookback
	uint32_t m_al_refresh_retired = 0;             // same-class re-announces that retired a PENDING WIPED dispatch

private:
	// Per-machine link configuration, stored in the system cfg file:
	//   <linkplay listen_host="0.0.0.0" listen_port="9876"
	//             connect_host="127.0.0.1" connect_port="9876" />
	// Resolution priority at comm bring-up: (1) explicit -comm_* CLI
	// options (any of the four changed from MAME defaults); (2) these
	// cfg-stored values; (3) the built-in loopback defaults below (so two
	// bare instances on one PC link out-of-the-box).  Under (2)/(3) the
	// ROLE comes from the "Link ID" machine configuration (Left/Red =
	// listener binds listen_host:listen_port, Right/Blue = connector
	// dials connect_host:connect_port) and the link DIP gates whether any
	// comm starts at all.  Values are consumed ONCE at machine start
	// (config FINAL) - no live socket rebinding.
	static constexpr uint16_t     LP_DEFAULT_PORT         = 9876;
	static constexpr char const  *LP_DEFAULT_LISTEN_HOST  = "0.0.0.0";
	static constexpr char const  *LP_DEFAULT_CONNECT_HOST = "127.0.0.1";

	void linkplay_config_load(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode);
	void linkplay_config_save(config_type cfg_type, util::xml::data_node *parentnode);
	void start_comm_cfg() ATTR_COLD;               // deferred cfg/default comm bring-up (config FINAL)
	ioport_field *lp_link_id_field() const;        // the host machine's "Link ID" PORT_CONFNAME, or nullptr
	bool lp_role_is_connector() const;             // "Link ID" machine config: Right/Blue = connector

	std::string m_lp_listen_host  = LP_DEFAULT_LISTEN_HOST;   // left/red: bind address
	uint16_t    m_lp_listen_port  = LP_DEFAULT_PORT;          // left/red: bind port
	std::string m_lp_connect_host = LP_DEFAULT_CONNECT_HOST;  // right/blue: target address
	uint16_t    m_lp_connect_port = LP_DEFAULT_PORT;          // right/blue: target port
	bool        m_lp_comm_deferred = false;        // start_comm saw all-MAME-defaults => resolve at config FINAL

	// a (hash, length) pair identifying staged/dispatched TX content, used
	// by the TX-complete admission gate's history ring below
	struct ts_hist_rec
	{
		uint32_t hash = 0;                         // FNV-1a 32 of the staged wire bytes
		uint16_t len = 0;                          // staged TXSIZE (halfwords)
		bool valid = false;
	};

	// TX-complete release (in-game only; establishment keeps the simple
	// stop-and-wait behaviour).
	//
	// The real chip's transmit trigger is a non-zero TXSIZE written while
	// the chip is armed; TXSIZE->0 and the TX-done IRQ follow from the
	// serialization itself, so the host's TX pump sees its staged frame
	// leave within about a frame.  The emulated stop-and-wait (a staged
	// TXSIZE survives until a delivered peer frame wipes it) throttles the
	// host's compose rate to the peer's delivery cadence, so while the
	// debounced in-game state holds the device models the real release:
	//  1. ADMISSION GATE (reg_w, the TXSIZE stage site): a freshly staged
	//     standalone frame whose FNV-1a content hash differs from the
	//     previously dispatched stage is transmitted SYNCHRONOUSLY at its
	//     stage instant.  The host's pump re-offers pending content on
	//     later passages, always as strictly adjacent re-stages - so a
	//     previous-content-hash key suffices (plus a small history ring
	//     covering rare A-B-A transients).  TXSIZE=0 writes are idle pump
	//     passages and are ignored entirely.  A hash-identical re-stage is
	//     PARKED, not dispatched.
	//  2. MODELLED TX-BUSY (reg_r, the pump's TXSIZE busy-poll): after
	//     each dispatch - and for a duplicate re-stage landing with the
	//     modelled serializer idle - TXSIZE reads return a synthesized
	//     BUSY value for TX_BUSY_MS (~one vblank), WITHOUT writing the
	//     register file; the first poll AFTER the window closes releases a
	//     parked duplicate (TXSIZE -> 0).  This reproduces the pump's
	//     hardware pacing (roughly one passage per serialization interval)
	//     instead of letting it spin.
	//  3. STALE-REPLAY AGE-OUT (heartbeat_tick): when in-game, the
	//     keepalive replay is suppressed when the captured payload is
	//     older than HEARTBEAT_STALE_MS - under self-release the peer no
	//     longer needs the replay for pacing, and a stale replay only
	//     feeds the peer outdated state.
	// Leaving the in-game state (or device_reset) clears the gate key,
	// ring, park and busy window immediately, so establishment never sees
	// a synthesized read.
	static constexpr unsigned TXC_HIST = 4;        // dispatched-content (hash,len) ring depth (A-B-A transient cover)
	bool m_txc_prev_valid = false;                 // prev hash/len below hold real dispatched content
	uint32_t m_txc_prev_hash = 0;                  // FNV-1a-32 of the last dispatched staged wire image (the admission key)
	uint16_t m_txc_prev_len = 0;                   // its staged length (halfwords)
	ts_hist_rec m_txc_hist[TXC_HIST];              // last TXC_HIST dispatched (hash,len) - belt-and-braces ring
	uint8_t m_txc_hist_idx = 0;                    // next ring write slot
	attotime m_txc_busy_until;                     // modelled busy window end (dispatch/park instant + TX_BUSY_MS)
	uint16_t m_txc_busy_hw = 0;                    // TXSIZE of the modelled in-flight frame (the synthesized busy read value)
	bool m_txc_parked_dup = false;                 // a hash-verified duplicate re-stage sits in TXSIZE (cleared at the first post-window poll)
	attotime m_txc_cap_time;                       // emulated time of the last keepalive capture (age-out anchor)
	bool m_txc_first_dispatch_logged = false;      // one-shot first-dispatch log line
	bool m_txc_first_release_logged = false;       // one-shot first parked-dup release log line
	bool m_txc_first_stale_logged = false;         // one-shot first stale-replay suppression log line
};


// device type definition
DECLARE_DEVICE_TYPE(NAMCO_C139, namco_c139_device)

#endif // MAME_NAMCO_NAMCO_C139_H
