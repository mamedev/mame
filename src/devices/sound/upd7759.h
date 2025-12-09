// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Mike Balfour, Howie Cohen, Olivier Galibert, Aaron Giles
#ifndef MAME_SOUND_UPD7759_H
#define MAME_SOUND_UPD7759_H

#pragma once

#include "dirom.h"

/* NEC uPD7759/55/56/P56/57/58 ADPCM Speech Processor */

/* There are two modes for the uPD7759, selected through the !MD pin.
   This is the mode select input.  High is stand alone, low is slave.
*/

class upd775x_device : public device_t,
	public device_sound_interface,
	public device_rom_interface<17>
{
public:
	enum : u32 { STANDARD_CLOCK = 640'000 };

	void reset_w(int state);
	void start_w(int state);
	int busy_r();
	virtual void port_w(u8 data);
	void set_start_delay(uint32_t data) { m_start_delay = data; }

protected:
	virtual TIMER_CALLBACK_MEMBER(internal_start_w);
	virtual TIMER_CALLBACK_MEMBER(internal_reset_w);
	virtual TIMER_CALLBACK_MEMBER(internal_port_w);

	enum
	{
		TID_PORT_WRITE,
		TID_START_WRITE,
		TID_RESET_WRITE,
		TID_SLAVE_UPDATE,
		TID_MD_WRITE
	};

	// chip states
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
	// chip modes
	enum
	{
		MODE_SLAVE,
		MODE_STAND_ALONE
	};

	upd775x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	virtual void rom_bank_pre_change() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	void update_adpcm(int data);
	virtual void advance_state();

	// internal state
	sound_stream  *m_channel;                   // stream channel for playback

	// chip configuration
	uint8_t       m_sample_offset_shift;        // header sample address shift (access data > 0xffff)

	// internal clock to output sample rate mapping
	uint32_t      m_pos;                        // current output sample position
	uint32_t      m_step;                       // step value per output sample
	attotime      m_clock_period;               // clock period

	// I/O lines
	uint8_t       m_fifo_in;                    // last data written to the sound chip
	uint8_t       m_reset;                      // current state of the RESET line
	uint8_t       m_start;                      // current state of the START line
	uint8_t       m_drq;                        // current state of the DRQ line

	// internal state machine
	int8_t        m_state;                      // current overall chip state
	int32_t       m_clocks_left;                // number of clocks left in this state
	uint16_t      m_nibbles_left;               // number of ADPCM nibbles left to process
	uint8_t       m_repeat_count;               // number of repeats remaining in current repeat block
	int8_t        m_post_drq_state;             // state we will be in after the DRQ line is dropped
	int32_t       m_post_drq_clocks;            // clocks that will be left after the DRQ line is dropped
	uint8_t       m_req_sample;                 // requested sample number
	uint8_t       m_last_sample;                // last sample number available
	uint8_t       m_block_header;               // header byte
	uint8_t       m_sample_rate;                // number of UPD clocks per ADPCM nibble
	uint8_t       m_first_valid_header;         // did we get our first valid header yet?
	uint32_t      m_offset;                     // current ROM offset
	uint32_t      m_repeat_offset;              // current ROM repeat offset
	uint32_t      m_start_delay;
	int           m_mode;                       // current mode of the sound chip

	// ADPCM processing
	int8_t        m_adpcm_state;                // ADPCM state index
	uint8_t       m_adpcm_data;                 // current byte of ADPCM data
	int16_t       m_sample;                     // current sample value

	// ROM access
	int           m_md;                         // High is stand alone, low is slave.
};

class upd7759_device : public upd775x_device
{
public:
	auto drq() { return m_drqcallback.bind(); }

	upd7759_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = STANDARD_CLOCK);

	void md_w(int state);

protected:
	upd7759_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(drq_update);

	void internal_md_w(int state);

	devcb_write_line m_drqcallback;
	emu_timer *m_timer;
};

class upd7756_device : public upd775x_device
{
public:
	upd7756_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = STANDARD_CLOCK);

protected:
	upd7756_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_reset() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(UPD7759, upd7759_device)
DECLARE_DEVICE_TYPE(UPD7756, upd7756_device)

#endif // MAME_SOUND_UPD7759_H
