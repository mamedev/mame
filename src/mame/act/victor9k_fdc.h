// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 floppy disk controller emulation

**********************************************************************/

#ifndef MAME_ACT_VICTOR9K_FDC_H
#define MAME_ACT_VICTOR9K_FDC_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "formats/victor9k_dsk.h"
#include "imagedev/floppy.h"
#include "machine/6522via.h"
#include "machine/fdc_pll.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> victor_9000_fdc_device

class victor_9000_fdc_device :  public device_t
{
public:
	// construction/destruction
	victor_9000_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_wr_callback() { return m_irq_cb.bind(); }
	auto syn_wr_callback() { return m_syn_cb.bind(); }
	auto lbrdy_wr_callback() { return m_lbrdy_cb.bind(); }

	uint8_t cs5_r(offs_t offset) { return m_via4->read(offset); }
	void cs5_w(offs_t offset, uint8_t data) { m_via4->write(offset, data); }
	uint8_t cs6_r(offs_t offset) { return m_via6->read(offset); }
	void cs6_w(offs_t offset, uint8_t data) { m_via6->write(offset, data); }
	uint8_t cs7_r(offs_t offset);
	void cs7_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(gen_tick);
	TIMER_CALLBACK_MEMBER(tach0_tick);
	TIMER_CALLBACK_MEMBER(tach1_tick);

private:
	static const int rpm[0x100];

	enum
	{
		LED_A = 0,
		LED_B
	};

	enum {
		IDLE,
		RUNNING,
		RUNNING_SYNCPOINT
	};

	struct live_info {
		attotime tm;
		int state, next_state;

		int drive;
		int side;
		int drw;

		// common
		offs_t i;
		uint8_t e;

		// read
		uint16_t shift_reg;
		int bit_counter;
		int sync_bit_counter;
		int sync_byte_counter;
		int brdy;
		bool lbrdy_changed;
		int sync;
		int syn;
		bool syn_changed;
		int gcr_err;

		// write
		uint16_t shift_reg_write;
		uint8_t wd;
		int wrsync;
		int gcr_data;
		int erase;
	};

	devcb_write_line m_irq_cb;
	devcb_write_line m_syn_cb;
	devcb_write_line m_lbrdy_cb;

	required_device<i8048_device> m_maincpu;
	required_device<via6522_device> m_via4;
	required_device<via6522_device> m_via5;
	required_device<via6522_device> m_via6;
	required_device_array<floppy_connector, 2> m_floppy;
	required_memory_region m_gcr_rom;
	output_finder<2> m_leds;

	void update_stepper_motor(floppy_image_device *floppy, int stp, int old_st, int st);
	void update_spindle_motor(floppy_image_device *floppy, emu_timer *t_tach, bool start, bool stop, bool sel, uint8_t &da);
	void update_rpm(floppy_image_device *floppy, emu_timer *t_tach, bool sel, uint8_t &da);
	void update_rdy();

	void load0_cb(floppy_image_device *device);
	void unload0_cb(floppy_image_device *device);

	void load1_cb(floppy_image_device *device);
	void unload1_cb(floppy_image_device *device);

	uint8_t m_p2;

	/* floppy state */
	uint8_t m_data;
	uint8_t m_da[2];
	int m_start[2];
	int m_stop[2];
	int m_sel[2];
	int m_tach[2];
	int m_rdy[2];
	int m_scp_rdy0;
	int m_scp_rdy1;
	int m_via_rdy0;
	int m_via_rdy1;
	uint8_t m_scp_l0ms;
	uint8_t m_scp_l1ms;
	uint8_t m_via_l0ms;
	uint8_t m_via_l1ms;
	int m_st[2];
	int m_stp[2];
	int m_drive;
	int m_side;
	int m_drw;
	int m_erase;
	uint8_t m_wd;
	int m_wrsync;

	int m_via4_irq;
	int m_via5_irq;
	int m_via6_irq;

	attotime m_period;

	live_info cur_live, checkpoint_live;
	fdc_pll_t cur_pll, checkpoint_pll;
	emu_timer *t_gen, *t_tach[2];

	floppy_image_device* get_floppy();
	void live_start();
	void pll_reset(const attotime &when);
	void pll_start_writing(const attotime &tm);
	void pll_commit(floppy_image_device *floppy, const attotime &tm);
	void pll_stop_writing(floppy_image_device *floppy, const attotime &tm);
	int pll_get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit);
	bool pll_write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit);
	void pll_save_checkpoint();
	void pll_retrieve_checkpoint();
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_abort();
	void live_run(const attotime &limit = attotime::never);

	static void floppy_formats(format_registration &fr);

	uint8_t floppy_p1_r();
	void floppy_p1_w(uint8_t data);
	uint8_t floppy_p2_r();
	void floppy_p2_w(uint8_t data);
	int tach0_r();
	int tach1_r();
	void da_w(uint8_t data);

	uint8_t via4_pa_r();
	void via4_pa_w(uint8_t data);
	uint8_t via4_pb_r();
	void via4_pb_w(uint8_t data);
	void wrsync_w(int state);
	void via4_irq_w(int state);

	uint8_t via5_pa_r();
	void via5_pb_w(uint8_t data);
	void via5_irq_w(int state);

	uint8_t via6_pa_r();
	uint8_t via6_pb_r();
	void via6_pa_w(uint8_t data);
	void via6_pb_w(uint8_t data);
	void drw_w(int state);
	void erase_w(int state);
	void via6_irq_w(int state);
};



// device type definition
DECLARE_DEVICE_TYPE(VICTOR_9000_FDC, victor_9000_fdc_device)


#endif // MAME_ACT_VICTOR9K_FDC_H
