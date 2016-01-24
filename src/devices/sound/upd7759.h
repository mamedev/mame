// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Mike Balfour, Howie Cohen, Olivier Galibert, Aaron Giles
#pragma once

#ifndef __UPD7759_H__
#define __UPD7759_H__

/* chip states */
enum
{
	STATE_IDLE,
	STATE_DROP_DRQ,
	STATE_START,
	STATE_FIRST_REQ,
	STATE_LAST_SAMPLE,
	STATE_DUMMY1,
	STATE_ADDR_MSB,
	STATE_ADDR_LSB,
	STATE_DUMMY2,
	STATE_BLOCK_HEADER,
	STATE_NIBBLE_COUNT,
	STATE_NIBBLE_MSN,
	STATE_NIBBLE_LSN
};

/* NEC uPD7759/55/56/P56/57/58 ADPCM Speech Processor */

/* There are two modes for the uPD7759, selected through the !MD pin.
   This is the mode select input.  High is stand alone, low is slave.
   We're making the assumption that nobody switches modes through
   software.
*/

#define UPD7759_STANDARD_CLOCK      XTAL_640kHz

class upd775x_device : public device_t,
									public device_sound_interface
{
public:
	upd775x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~upd775x_device() {}

	template<class _Object> static devcb_base &set_drq_callback(device_t &device, _Object object) { return downcast<upd775x_device &>(device).m_drqcallback.set_callback(object); }

	void set_bank_base(offs_t base);

	DECLARE_WRITE_LINE_MEMBER( reset_w );
	DECLARE_READ_LINE_MEMBER( busy_r );
	virtual DECLARE_WRITE8_MEMBER( port_w );
	void postload();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// internal state
	sound_stream *m_channel;                  /* stream channel for playback */

	/* chip configuration */
	UINT8       m_sample_offset_shift;        /* header sample address shift (access data > 0xffff) */

	/* internal clock to output sample rate mapping */
	UINT32      m_pos;                        /* current output sample position */
	UINT32      m_step;                       /* step value per output sample */
	attotime    m_clock_period;               /* clock period */

	/* I/O lines */
	UINT8       m_fifo_in;                    /* last data written to the sound chip */
	UINT8       m_reset;                      /* current state of the RESET line */
	UINT8       m_start;                      /* current state of the START line */
	UINT8       m_drq;                        /* current state of the DRQ line */

	/* internal state machine */
	INT8        m_state;                      /* current overall chip state */
	INT32       m_clocks_left;                /* number of clocks left in this state */
	UINT16      m_nibbles_left;               /* number of ADPCM nibbles left to process */
	UINT8       m_repeat_count;               /* number of repeats remaining in current repeat block */
	INT8        m_post_drq_state;             /* state we will be in after the DRQ line is dropped */
	INT32       m_post_drq_clocks;            /* clocks that will be left after the DRQ line is dropped */
	UINT8       m_req_sample;                 /* requested sample number */
	UINT8       m_last_sample;                /* last sample number available */
	UINT8       m_block_header;               /* header byte */
	UINT8       m_sample_rate;                /* number of UPD clocks per ADPCM nibble */
	UINT8       m_first_valid_header;         /* did we get our first valid header yet? */
	UINT32      m_offset;                     /* current ROM offset */
	UINT32      m_repeat_offset;              /* current ROM repeat offset */

	/* ADPCM processing */
	INT8        m_adpcm_state;                /* ADPCM state index */
	UINT8       m_adpcm_data;                 /* current byte of ADPCM data */
	INT16       m_sample;                     /* current sample value */

	/* ROM access */
	optional_region_ptr<UINT8> m_rombase;	  /* pointer to ROM data or NULL for slave mode */
	UINT8 *     m_rom;                        /* pointer to ROM data or NULL for slave mode */
	UINT32      m_romoffset;                  /* ROM offset to make save/restore easier */
	UINT32      m_rommask;                    /* maximum address offset */

	devcb_write_line m_drqcallback;

	void update_adpcm(int data);
	virtual void advance_state();
};

class upd7759_device : public upd775x_device
{
public:
	upd7759_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	upd7759_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	enum
	{
		TIMER_SLAVE_UPDATE
	};

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	emu_timer *m_timer;                       /* timer */

	DECLARE_WRITE_LINE_MEMBER( start_w );
};

class upd7756_device : public upd775x_device
{
public:
	upd7756_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	DECLARE_WRITE_LINE_MEMBER( start_w );
};

extern const device_type UPD7759;
extern const device_type UPD7756;

#define MCFG_UPD7759_DRQ_CALLBACK(_write) \
	devcb = &upd7759_device::set_drq_callback(*device, DEVCB_##_write);

#define MCFG_UPD7756_DRQ_CALLBACK(_write) \
	devcb = &upd7756_device::set_drq_callback(*device, DEVCB_##_write);

#endif /* __UPD7759_H__ */
