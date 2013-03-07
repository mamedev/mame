/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Motorola 6854 emulation (network interface).

**********************************************************************/

#ifndef MC6854_H
#define MC6854_H

class mc6854_device : public device_t
{
public:
	mc6854_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mc6854_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type MC6854;


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
   transmitted at all. No start flag, stop flag, or crc bits are trasmitted.
   send_frame makes a frame available to the CPU through the 6854 (it may
   fail and return -1 if the 6854 is not ready to accept the frame; even
   if the frame is accepted and 0 is returned, the CPU may abort it). Ony
   full frames are accepted.
*/


/* ---------- configuration ------------ */

struct mc6854_interface
{
	devcb_write_line  out_irq_func; /* interrupt request */

	/* low-level, bit-based interface */
	devcb_read_line   in_rxd_func; /* receive bit */
	devcb_write_line  out_txd_func; /* transmit bit */

	/* high-level, frame-based interface */
	void ( * out_frame ) ( device_t *device, UINT8* data, int length );

	/* control lines */
	devcb_write_line  out_rts_func; /* 1 = transmitting, 0 = idle */
	devcb_write_line  out_dtr_func; /* 1 = data transmit ready, 0 = busy */
};


#define MCFG_MC6854_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MC6854, 0)          \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MC6854_REMOVE(_tag)        \
	MCFG_DEVICE_REMOVE(_tag)


/* ---------- functions ------------ */
/* interface to CPU via address/data bus*/
extern DECLARE_READ8_DEVICE_HANDLER  ( mc6854_r );
extern DECLARE_WRITE8_DEVICE_HANDLER ( mc6854_w );

/* low-level, bit-based interface */
WRITE_LINE_DEVICE_HANDLER( mc6854_set_rx );

/* high-level, frame-based interface */
extern int mc6854_send_frame( device_t *device, UINT8* data, int length ); /* ret -1 if busy */

/* control lines */
WRITE_LINE_DEVICE_HANDLER( mc6854_set_cts ); /* 1 = clear-to-send, 0 = busy */
WRITE_LINE_DEVICE_HANDLER( mc6854_set_dcd ); /* 1 = carrier, 0 = no carrier */

/* clock */
WRITE_LINE_DEVICE_HANDLER( mc6854_rxc_w );
WRITE_LINE_DEVICE_HANDLER( mc6854_txc_w );

#endif
