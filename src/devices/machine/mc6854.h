// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Motorola 6854 emulation (network interface).

**********************************************************************/

#ifndef MAME_MACHINE_MC6854_H
#define MAME_MACHINE_MC6854_H

#pragma once


class mc6854_device : public device_t
{
public:
	static constexpr unsigned MAX_FRAME_LENGTH = 65536; // arbitrary value, you may need to enlarge it if you get truncated frames

	typedef device_delegate<void (uint8_t *data, int length)> out_frame_delegate;

	mc6854_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto out_irq_cb()  { return m_out_irq_cb.bind(); }
	auto out_rdsr_cb() { return m_out_rdsr_cb.bind(); }
	auto out_tdsr_cb() { return m_out_tdsr_cb.bind(); }
	auto out_txd_cb()  { return m_out_txd_cb.bind(); }
	auto out_rts_cb()  { return m_out_rts_cb.bind(); }
	auto out_dtr_cb()  { return m_out_dtr_cb.bind(); }

	template <typename... T> void set_out_frame_callback(T &&... args) { m_out_frame_cb.set(std::forward<T>(args)...); }

	/* interface to CPU via address/data bus*/
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t dma_r(){ return read(2); }

	/* low-level, bit-based interface */
	void set_rx(int state);

	/* high-level, frame-based interface */
	int send_frame( uint8_t* data, int length ); /* ret -1 if busy */

	/* control lines */
	void set_cts(int state); /* 1 = clear-to-send, 0 = busy */
	void set_dcd(int state); /* 1 = carrier, 0 = no carrier */

	/* clock */
	void rxc_w(int state);
	void txc_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	bool receive_allowed() const;

private:
	static constexpr unsigned FIFO_SIZE = 3; // hardcoded size of the 6854 FIFO (this is a hardware limit)

	// internal state
	devcb_write_line  m_out_irq_cb; /* interrupt request */
	devcb_write_line  m_out_rdsr_cb; /* Rx fifo DMA request */
	devcb_write_line  m_out_tdsr_cb; /* Tx fifo DMA request */

	/* low-level, bit-based interface */
	devcb_write_line  m_out_txd_cb; /* transmit bit */

	/* high-level, frame-based interface */
	out_frame_delegate   m_out_frame_cb;

	/* control lines */
	devcb_write_line  m_out_rts_cb; /* 1 = transmitting, 0 = idle */
	devcb_write_line  m_out_dtr_cb; /* 1 = data transmit ready, 0 = busy */

	/* registers */
	uint8_t m_cr1, m_cr2, m_cr3, m_cr4; /* control registers */
	uint8_t m_sr1, m_sr2;           /* status registers */

	uint8_t m_cts, m_dcd;

	/* transmit state */
	uint8_t  m_tstate;
	uint16_t m_tfifo[FIFO_SIZE];  /* X x 8-bit FIFO + full & last marker bits */
	uint8_t  m_tones;             /* counter for zero-insertion */
	emu_timer *m_ttimer;       /* when to ask for more data */

	/* receive state */
	uint8_t  m_rstate;
	uint32_t m_rreg;              /* shift register */
	uint8_t  m_rones;             /* count '1 bits */
	uint8_t  m_rsize;             /* bits in the shift register */
	uint16_t m_rfifo[FIFO_SIZE];  /* X x 8-bit FIFO + full & addr marker bits */
	bool     m_rxd;
	bool     m_rxc;

	/* frame-based interface*/
	uint8_t  m_frame[MAX_FRAME_LENGTH];
	uint32_t m_flen, m_fpos;


	/* meaning of tstate / rtate:
	   0 = idle / waiting for frame flag
	   1 = flag sync
	   2 = 8-bit address field(s)
	   3-4 = 8-bit control field(s)
	   5 = 8-bit logical control field(s)
	   6 = variable-length data field(s)
	*/

	void send_bits( uint32_t data, int len, int zi );
	void tfifo_push( uint8_t data );
	void tfifo_terminate( );
	TIMER_CALLBACK_MEMBER(tfifo_cb);
	void tfifo_clear( );

	void rfifo_push( uint8_t d );
	void rfifo_terminate( );
	uint8_t rfifo_pop( );
	void rfifo_clear( );

	void update_sr2( );
	void update_sr1( );
};

DECLARE_DEVICE_TYPE(MC6854, mc6854_device)


/* we provide two interfaces:
   - a bit-based interface:   out_tx, set_rx
   - a frame-based interface: out_frame, send_frame

   The bit-based interface is low-level and slow.
   Use it to simulate the actual bits sent into the wires, e.g., to connect
   the emulator to another bit-based emulated network device, or an actual
   device.

   The frame-based interface is higher-level and faster.
   It passes bytes directly from one end to the other without bothering with
   the actual bit-encoding, synchronization, and CRC.

   Once completed, a frame is sent through out_frame. Aborted frames are not
   transmitted at all. No start flag, stop flag, or crc bits are transmitted.
   send_frame makes a frame available to the CPU through the 6854 (it may
   fail and return -1 if the 6854 is not ready to accept the frame; even
   if the frame is accepted and 0 is returned, the CPU may abort it). Ony
   full frames are accepted.
*/

#endif // MAME_MACHINE_MC6854_H
