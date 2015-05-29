// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef AMIGAFDC_H
#define AMIGAFDC_H

#include "emu.h"
#include "imagedev/floppy.h"

#define MCFG_AMIGA_FDC_INDEX_CALLBACK(_write) \
	devcb = &amiga_fdc::set_index_wr_callback(*device, DEVCB_##_write);

class amiga_fdc : public device_t {
public:
	amiga_fdc(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_index_wr_callback(device_t &device, _Object object) { return downcast<amiga_fdc &>(device).m_write_index.set_callback(object); }

	DECLARE_WRITE8_MEMBER(ciaaprb_w);

	UINT8 ciaapra_r();
	UINT16 dskbytr_r();
	UINT16 dskpth_r();
	UINT16 dskptl_r();

	void dsksync_w(UINT16 data);
	void dskpth_w(UINT16 data);
	void dskptl_w(UINT16 data);
	void dsklen_w(UINT16 data);
	void adkcon_set(UINT16 data);
	void dmacon_set(UINT16 data);
	UINT16 adkcon_r(void);

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

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
		UINT16 counter;
		UINT16 increment;
		UINT16 transition_time;
		UINT8 history;
		UINT8 slot;
		UINT8 phase_add, phase_sub, freq_add, freq_sub;
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
		UINT16 shift_reg;
		int bit_counter;
		pll_t pll;
	};

	devcb_write_line m_write_index;

	floppy_image_device *floppy;
	floppy_image_device *floppy_devices[4];

	live_info cur_live, checkpoint_live;

	emu_timer *t_gen;
	UINT16 dsklen, pre_dsklen, dsksync, dskbyt, adkcon, dmacon;
	UINT32 dskpt;
	UINT16 dma_value;

	int dma_state;

	void setup_leds();
	void index_callback(floppy_image_device *floppy, int state);
	bool dma_enabled();
	void dma_check();
	void dma_done();
	void dma_write(UINT16 value);
	UINT16 dma_read();

	void live_start();
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_abort();
	void live_run(const attotime &limit = attotime::never);
};

extern const device_type AMIGA_FDC;

#endif /* AMIGAFDC_H */
