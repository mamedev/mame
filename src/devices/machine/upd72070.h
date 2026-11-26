// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_MACHINE_UPD72070_H
#define MAME_MACHINE_UPD72070_H

#pragma once

#include "fdc_pll.h"

#include "imagedev/floppy.h"

class upd72070_device : public device_t
{
public:
	static constexpr feature_type imperfect_features() { return feature::DISK; }

	upd72070_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq_cb() { return m_irq_cb.bind(); }
	auto drq_cb() { return m_drq_cb.bind(); }

	void set_revision(u8 firmware, u8 hardware) { m_firmware_revision = firmware; m_hardware_revision = hardware; }

	void set_xb_clock(u32 clock) { m_xb_clock = clock; }

	void map(address_map &map) ATTR_COLD;

	u8 status_r();
	u8 data_r();
	void data_w(u8 data);
	void rate_w(u8 data);
	u8 dma_r();
	void dma_w(u8 data);
	void tc_w(int state);
	void reset_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum class phase : u8 { IDLE, COMMAND, EXECUTION, RESULT, DRIVE };
	enum class interrupt_cause : u8 { NONE, SEEK, CONTROL, ENABLE, MEDIA };
	enum class drive_state : u8 { IDLE, READY, MOTOR_OFF, SEEK_STEP, SEEK_WAIT, RECAL_OUT, RECAL_OUT_WAIT, RECAL_IN_STEP, RECAL_IN_WAIT, RECAL_LAST_STEP, RECAL_LAST_WAIT };
	enum class media_state : u8 { IDLE, SEARCH_ID, ID, SEARCH_DATA, DATA, WRITE_GAP, WRITE_DATA, READ_GAP, NEXT_GAP, FORMAT_WAIT, FORMAT_PREFIX, FORMAT_SECTOR, FORMAT_GAP, RAW_INDEX, RAW_SKIP, RAW_MARK, RAW_DATA, DRAIN };

	void soft_reset();
	void wake();
	void execute_command();
	void command_end();
	void begin_result(u8 count, bool interrupt = false);
	void invalid_command();
	void sense_interrupt();
	void queue_interrupt(u32 drive, interrupt_cause cause, u8 status, u8 cylinder);
	void update_outputs(bool force = false);
	mac_floppy_device *floppy(u32 drive) const;
	bool drive_sense(u32 drive, u8 address);
	void drive_command(u32 drive, u8 address);
	void enable_drive(s8 drive);
	u8 drive_status(u32 drive);
	u8 interrupt_status(u32 drive, u8 flags);
	void execute_drive_command();
	void finish_drive_command(bool error);
	void wait_ready(drive_state next, attotime timeout);
	bool interrupts_pending() const;
	void standby();
	TIMER_CALLBACK_MEMBER(drive_tick);
	TIMER_CALLBACK_MEMBER(poll_tick);
	void execute_media_command();
	void search_id();
	void read_bit(int bit);
	void read_byte(u8 data);
	bool gcr() const { return !BIT(m_command[0], 6); }
	void read_gcr(u8 data);
	void write_gcr();
	void gcr_data_reset();
	u8 write_host_byte();
	void raw_mark();
	void raw_word(u16 word);
	void raw_byte(u8 data);
	void raw_timeout();
	bool read_track() const { return (m_command[0] & 0x1f) == 2; }
	bool read_id() const { return (m_command[0] & 0x1f) == 0x0a; }
	bool raw_dump() const { return (m_command[0] & 0x1f) == 0x1e; }
	u32 mfm_gap2() const { return (m_perpendicular == 3) ? 41 : 22; }
	u32 mfm_write_gap2() const { return BIT(m_perpendicular, 0) ? mfm_gap2() - 3 : 0; }
	void write_word();
	void encode_word(u8 data);
	void flush_write();
	void sector_done();
	void advance_id();
	void finish_media(u8 st1 = 0, u8 st2 = 0);
	void media_result();
	attotime cell_period() const;
	TIMER_CALLBACK_MEMBER(media_tick);

	void start_transfer(u32 size, bool to_host);
	bool fifo_push(u8 data);
	bool fifo_pop(u8 &data);
	u8 transfer_r();
	void transfer_w(u8 data);
	void update_transfer_request();
	u8 fifo_capacity() const { return BIT(m_config, 5) ? 1 : 16; }
	bool non_dma() const { return BIT(m_specify, 0); }

	devcb_write_line m_irq_cb;
	devcb_write_line m_drq_cb;
	optional_device_array<floppy_connector, 2> m_connectors;
	u32 m_xb_clock;
	u8 m_firmware_revision;
	u8 m_hardware_revision;

	phase m_phase;
	u8 m_command[9];
	u8 m_command_pos;
	u8 m_command_size;
	u8 m_result[7];
	u8 m_result_pos;
	u8 m_result_size;
	u8 m_fifo[16];
	u8 m_fifo_head;
	u8 m_fifo_count;
	u32 m_transfer_remaining;
	bool m_to_host;
	bool m_transfer_request;
	bool m_tc;
	bool m_tc_seen;
	bool m_reset;
	u8 m_drr;
	u8 m_config;
	u8 m_pretrk;
	u8 m_specify;
	u8 m_drive_type;
	u8 m_perpendicular;
	u8 m_drive_busy;
	interrupt_cause m_pending[2];
	u8 m_pending_status[2];
	u8 m_pending_pcn[2];
	s8 m_sense_drive;
	bool m_result_irq;
	bool m_async_irq;
	bool m_irq;
	bool m_drq;
	emu_timer *m_drive_timer;
	emu_timer *m_poll_timer;
	drive_state m_drive_state;
	u8 m_active_drive;
	u8 m_pcn[2];
	u16 m_steps;
	bool m_outward;
	bool m_seek_command;
	bool m_dpll[2];
	bool m_status_read[2];
	bool m_media_out[2];
	bool m_media_pending[2];
	bool m_sense_seek;
	s8 m_enabled;
	u8 m_poll_drive;
	attotime m_deadline;
	attotime m_drive_delay;
	attotime m_poll_delay;
	attotime m_timeout_delay;

	emu_timer *m_media_timer;
	media_state m_media_state;
	fdc_pll_t m_pll;
	attotime m_media_deadline;
	attotime m_next_index;
	attotime m_write_start;
	attotime m_write_edges[1024];
	u16 m_write_count;
	bool m_writing;
	u8 m_media_drive;
	u8 m_head;
	u8 m_id[7];
	u8 m_chrn[4];
	u8 m_st0;
	u8 m_st1;
	u8 m_st2;
	u16 m_shift;
	u16 m_crc;
	u8 m_bits;
	u8 m_sync;
	u32 m_byte;
	u32 m_sector_size;
	u32 m_media_bytes;
	u32 m_transfer_total;
	u8 m_sector;
	u8 m_indexes;
	bool m_saw_id;
	bool m_previous_bit;
	bool m_result_advance;

	u8 m_raw_clock;
	u8 m_raw_delay;
	bool m_raw_c2;

	u8 m_gcr_checksum[3];
	u8 m_gcr_group[4];
	u8 m_gcr_data[3];
	u8 m_gcr_count;
	u8 m_gcr_hi;
	u8 m_gcr_carry;
	u16 m_gcr_pos;
	u8 m_gcr_header_pos;
};

DECLARE_DEVICE_TYPE(UPD72070, upd72070_device)

#endif // MAME_MACHINE_UPD72070_H
