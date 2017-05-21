// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_AMIGAFDC_H
#define MAME_MACHINE_AMIGAFDC_H

#pragma once

#include "imagedev/floppy.h"

#define MCFG_AMIGA_FDC_INDEX_CALLBACK(_write) \
	devcb = &amiga_fdc_device::set_index_wr_callback(*device, DEVCB_##_write);

class amiga_fdc_device : public device_t {
public:
	amiga_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_index_wr_callback(device_t &device, Object &&cb) { return downcast<amiga_fdc_device &>(device).m_write_index.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE8_MEMBER(ciaaprb_w);

	uint8_t ciaapra_r();
	uint16_t dskbytr_r();
	uint16_t dskpth_r();
	uint16_t dskptl_r();

	void dsksync_w(uint16_t data);
	void dskpth_w(uint16_t data);
	void dskptl_w(uint16_t data);
	void dsklen_w(uint16_t data);
	void adkcon_set(uint16_t data);
	void dmacon_set(uint16_t data);
	uint16_t adkcon_r(void);

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// Running states
	enum {
		IDLE,
		RUNNING,
		RUNNING_SYNCPOINT
	};

	// DMA states
	enum {
		DMA_IDLE,
		DMA_WAIT_START,
		DMA_RUNNING_BYTE_0,
		DMA_RUNNING_BYTE_1
	};

	struct pll_t {
		uint16_t counter;
		uint16_t increment;
		uint16_t transition_time;
		uint8_t history;
		uint8_t slot;
		uint8_t phase_add, phase_sub, freq_add, freq_sub;
		attotime ctime;

		attotime delays[38];

		attotime write_start_time;
		attotime write_buffer[32];
		int write_position;

		void set_clock(const attotime &period);
		void reset(const attotime &when);
		int get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit);
		bool write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit);
		void start_writing(const attotime &tm);
		void commit(floppy_image_device *floppy, const attotime &tm);
		void stop_writing(floppy_image_device *floppy, const attotime &tm);
	};

	struct live_info {
		attotime tm;
		int state, next_state;
		uint16_t shift_reg;
		int bit_counter;
		pll_t pll;
	};

	devcb_write_line m_write_index;

	floppy_image_device *floppy;
	floppy_image_device *floppy_devices[4];

	live_info cur_live, checkpoint_live;

	emu_timer *t_gen;
	uint16_t dsklen, pre_dsklen, dsksync, dskbyt, adkcon, dmacon;
	uint32_t dskpt;
	uint16_t dma_value;

	int dma_state;

	void setup_leds();
	void index_callback(floppy_image_device *floppy, int state);
	bool dma_enabled();
	void dma_check();
	void dma_done();
	void dma_write(uint16_t value);
	uint16_t dma_read();

	void live_start();
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_abort();
	void live_run(const attotime &limit = attotime::never);
};

DECLARE_DEVICE_TYPE(AMIGA_FDC, amiga_fdc_device)

#endif // MAME_MACHINE_AMIGAFDC_H
