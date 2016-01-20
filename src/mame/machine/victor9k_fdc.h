// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 floppy disk controller emulation

**********************************************************************/

#pragma once

#ifndef __VICTOR_9000_FDC__
#define __VICTOR_9000_FDC__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "formats/victor9k_dsk.h"
#include "imagedev/floppy.h"
#include "machine/6522via.h"
#include "machine/fdc_pll.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VICTOR_9000_FDC_IRQ_CB(_write) \
	devcb = &victor_9000_fdc_t::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_VICTOR_9000_FDC_SYN_CB(_write) \
	devcb = &victor_9000_fdc_t::set_syn_wr_callback(*device, DEVCB_##_write);

#define MCFG_VICTOR_9000_FDC_LBRDY_CB(_write) \
	devcb = &victor_9000_fdc_t::set_lbrdy_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> victor_9000_fdc_t

class victor_9000_fdc_t :  public device_t
{
public:
	// construction/destruction
	victor_9000_fdc_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<victor_9000_fdc_t &>(device).m_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_syn_wr_callback(device_t &device, _Object object) { return downcast<victor_9000_fdc_t &>(device).m_syn_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_lbrdy_wr_callback(device_t &device, _Object object) { return downcast<victor_9000_fdc_t &>(device).m_lbrdy_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( cs5_r ) { return m_via4->read(space, offset); }
	DECLARE_WRITE8_MEMBER( cs5_w ) { m_via4->write(space, offset, data); }
	DECLARE_READ8_MEMBER( cs6_r ) { return m_via6->read(space, offset); }
	DECLARE_WRITE8_MEMBER( cs6_w ) { m_via6->write(space, offset, data); }
	DECLARE_READ8_MEMBER( cs7_r );
	DECLARE_WRITE8_MEMBER( cs7_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_READ8_MEMBER( floppy_p1_r );
	DECLARE_WRITE8_MEMBER( floppy_p1_w );
	DECLARE_READ8_MEMBER( floppy_p2_r );
	DECLARE_WRITE8_MEMBER( floppy_p2_w );
	DECLARE_READ8_MEMBER( tach0_r );
	DECLARE_READ8_MEMBER( tach1_r );
	DECLARE_WRITE8_MEMBER( da_w );

	DECLARE_READ8_MEMBER( via4_pa_r );
	DECLARE_WRITE8_MEMBER( via4_pa_w );
	DECLARE_READ8_MEMBER( via4_pb_r );
	DECLARE_WRITE8_MEMBER( via4_pb_w );
	DECLARE_WRITE_LINE_MEMBER( wrsync_w );
	DECLARE_WRITE_LINE_MEMBER( via4_irq_w );

	DECLARE_READ8_MEMBER( via5_pa_r );
	DECLARE_WRITE8_MEMBER( via5_pb_w );
	DECLARE_WRITE_LINE_MEMBER( via5_irq_w );

	DECLARE_READ8_MEMBER( via6_pa_r );
	DECLARE_READ8_MEMBER( via6_pb_r );
	DECLARE_WRITE8_MEMBER( via6_pa_w );
	DECLARE_WRITE8_MEMBER( via6_pb_w );
	DECLARE_WRITE_LINE_MEMBER( drw_w );
	DECLARE_WRITE_LINE_MEMBER( erase_w );
	DECLARE_WRITE_LINE_MEMBER( via6_irq_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	static const int rpm[0x100];

	enum
	{
		TM_GEN,
		TM_TACH0,
		TM_TACH1
	};

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
		UINT8 e;

		// read
		UINT16 shift_reg;
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
		UINT16 shift_reg_write;
		UINT8 wd;
		int wrsync;
		int gcr_data;
		int erase;
	};

	devcb_write_line m_irq_cb;
	devcb_write_line m_syn_cb;
	devcb_write_line m_lbrdy_cb;

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via4;
	required_device<via6522_device> m_via5;
	required_device<via6522_device> m_via6;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	required_memory_region m_gcr_rom;

	void update_stepper_motor(floppy_image_device *floppy, int stp, int old_st, int st);
	void update_spindle_motor(floppy_image_device *floppy, emu_timer *t_tach, bool start, bool stop, bool sel, UINT8 &da);
	void set_rdy0(int state);
	void set_rdy1(int state);

	int load0_cb(floppy_image_device *device);
	void unload0_cb(floppy_image_device *device);

	int load1_cb(floppy_image_device *device);
	void unload1_cb(floppy_image_device *device);

	UINT8 m_p2;

	/* floppy state */
	UINT8 m_da;
	UINT8 m_da0;
	UINT8 m_da1;
	int m_start0;
	int m_stop0;
	int m_start1;
	int m_stop1;
	int m_sel0;
	int m_sel1;
	int m_tach0;
	int m_tach1;
	int m_rdy0;
	int m_rdy1;
	UINT8 m_l0ms;
	UINT8 m_l1ms;
	int m_st0;
	int m_st1;
	int m_stp0;
	int m_stp1;
	int m_drive;
	int m_side;
	int m_drw;
	int m_erase;
	UINT8 m_wd;
	int m_wrsync;

	int m_via4_irq;
	int m_via5_irq;
	int m_via6_irq;

	attotime m_period;

	live_info cur_live, checkpoint_live;
	fdc_pll_t cur_pll, checkpoint_pll;
	emu_timer *t_gen, *t_tach0, *t_tach1;

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
};



// device type definition
extern const device_type VICTOR_9000_FDC;



#endif
