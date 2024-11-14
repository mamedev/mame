// license:BSD-3-Clause
// copyright-holders:68bit
//
// Motorola M68SFDC floppy disk controller

#ifndef MAME_MACHINE_M68SFDC_H
#define MAME_MACHINE_M68SFDC_H

#pragma once

#include "machine/6821pia.h"
#include "machine/clock.h"
#include "machine/fdc_pll.h"
#include "machine/mc6852.h"
#include "machine/timer.h"
#include "imagedev/floppy.h"

INPUT_PORTS_EXTERN(m68sfdc);

class m68sfdc_device : public device_t {
public:
	m68sfdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t reg, uint8_t val);
	uint8_t read(offs_t reg);

	auto irq_handler() { return m_irq_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }

	void set_floppies_4(floppy_connector*, floppy_connector*, floppy_connector*, floppy_connector*);

private:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	required_device<pia6821_device> m_pia;
	required_device<mc6852_device> m_ssda;
	emu_timer *m_timer_head_load;
	emu_timer *m_timer_timeout;

	devcb_write_line m_irq_handler;
	devcb_write_line m_nmi_handler;

	void handle_irq(int state);
	void handle_nmi(int state);

	uint8_t flip_bits(uint8_t data);
	uint8_t pia_pa_r();
	void pia_pa_w(u8 data);
	void pia_ca2_w(int state);
	uint8_t pia_pb_r();
	void pia_pb_w(u8 data);
	int pia_cb1_r();
	void pia_cb2_w(int state);
	u8 m_select_0;
	u8 m_select_1;
	u8 m_select_2;
	u8 m_select_3;
	u8 m_step;
	u8 m_direction;
	u8 m_head_load1;
	u8 m_head_load2;
	u8 m_head_load;
	u8 m_crc;
	u8 m_last_crc;
	u8 m_pia_cb2;
	u8 m_reset;
	u8 m_enable_drive_write;
	u8 m_enable_read;
	u8 m_shift_crc;
	u8 m_shift_crc_count;
	u8 m_tuf_count;
	u8 m_ssda_reg[5];          // Copy of register writes

	required_ioport m_select2_mode;
	required_ioport m_select3_mode;
	required_ioport m_disk_sides;
	required_ioport m_write_protect_mode;
	required_ioport m_stepper_mode;
	void update_floppy_selection();
	void fdc_index_callback(floppy_image_device *floppy, int state);

	floppy_connector *m_floppy0, *m_floppy1, *m_floppy2, *m_floppy3;
	floppy_image_device *m_floppy; // Currently selected floppy.

	TIMER_CALLBACK_MEMBER(head_load_update);
	TIMER_CALLBACK_MEMBER(timeout_expired);
	TIMER_CALLBACK_MEMBER(general_update);

	enum
	{
		// General "doing nothing" state
		IDLE,

		//
		SYNC1,
		SYNC_BYTE1,
		SYNC2,
		SYNC_BYTE2,
		READ,
		READ_BYTE,
		WRITE,
		WRITE_BITS,
	};

	struct live_info
	{
		enum { PT_NONE, PT_CRC_1, PT_CRC_2 };

		attotime tm;
		int state, next_state;
		uint16_t shift_reg;
		uint16_t crc;
		int bit_counter;
		bool data_separator_phase;
		uint8_t data_reg;
	};

	live_info cur_live, checkpoint_live;
	fdc_pll_t cur_pll, checkpoint_pll;

	void pll_reset(const attotime &when);
	//void pll_start_writing(const attotime &tm);
	void pll_commit(floppy_image_device *floppy, const attotime &tm);
	void pll_stop_writing(floppy_image_device *floppy, const attotime &tm);
	int pll_get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit);
	bool pll_write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit);
	void pll_save_checkpoint();
	void pll_retrieve_checkpoint();

	void live_start(int live_state);
	void live_abort();
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_run(attotime limit = attotime::never);
	bool read_one_bit(const attotime &limit);
	bool write_one_bit(const attotime &limit);

	void live_write_fm(uint8_t fm);

	emu_timer *t_gen;
};

DECLARE_DEVICE_TYPE(M68SFDC, m68sfdc_device)

#endif // MAME_MACHINE_M68SFDC_H
