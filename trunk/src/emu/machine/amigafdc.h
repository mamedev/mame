#ifndef AMIGAFDC_H
#define AMIGAFDC_H

#include "emu.h"
#include "imagedev/floppy.h"

#define MCFG_AMIGA_FDC_ADD(_tag, _clock)	\
	MCFG_DEVICE_ADD(_tag, AMIGA_FDC, _clock)

class amiga_fdc : public device_t {
public:
	amiga_fdc(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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

	static const floppy_format_type floppy_formats[];

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

		void set_clock(attotime period);
		void reset(attotime when);
		int get_next_bit(attotime &tm, floppy_image_device *floppy, attotime limit);
	};

	struct live_info {
		attotime tm;
		int state, next_state;
		UINT16 shift_reg;
		int bit_counter;
		pll_t pll;
	};

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

	void live_start();
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_abort();
	void live_run(attotime limit = attotime::never);
};

extern const device_type AMIGA_FDC;

#endif /* AMIGAFDC_H */
