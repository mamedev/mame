// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 floppy disk controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __VICTOR_9000_FDC__
#define __VICTOR_9000_FDC__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "formats/victor9k_dsk.h"
#include "imagedev/floppy.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VICTOR_9000_FDC_DS0_CB(_write) \
	devcb = &victor_9000_fdc_t::set_ds0_wr_callback(*device, DEVCB_##_write);

#define MCFG_VICTOR_9000_FDC_DS1_CB(_write) \
	devcb = &victor_9000_fdc_t::set_ds1_wr_callback(*device, DEVCB_##_write);

#define MCFG_VICTOR_9000_FDC_RDY0_CB(_write) \
	devcb = &victor_9000_fdc_t::set_rdy0_wr_callback(*device, DEVCB_##_write);

#define MCFG_VICTOR_9000_FDC_RDY1_CB(_write) \
	devcb = &victor_9000_fdc_t::set_rdy1_wr_callback(*device, DEVCB_##_write);

#define MCFG_VICTOR_9000_FDC_BRDY_CB(_write) \
	devcb = &victor_9000_fdc_t::set_brdy_wr_callback(*device, DEVCB_##_write);

#define MCFG_VICTOR_9000_FDC_GCRERR_CB(_write) \
	devcb = &victor_9000_fdc_t::set_gcrerr_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> victor_9000_fdc_t

class victor_9000_fdc_t :  public device_t
{
public:
	// construction/destruction
	victor_9000_fdc_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_ds0_wr_callback(device_t &device, _Object object) { return downcast<victor_9000_fdc_t &>(device).m_ds0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_ds1_wr_callback(device_t &device, _Object object) { return downcast<victor_9000_fdc_t &>(device).m_ds1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_rdy0_wr_callback(device_t &device, _Object object) { return downcast<victor_9000_fdc_t &>(device).m_rdy0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_rdy1_wr_callback(device_t &device, _Object object) { return downcast<victor_9000_fdc_t &>(device).m_rdy1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_brdy_wr_callback(device_t &device, _Object object) { return downcast<victor_9000_fdc_t &>(device).m_brdy_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_gcrerr_wr_callback(device_t &device, _Object object) { return downcast<victor_9000_fdc_t &>(device).m_gcrerr_cb.set_callback(object); }

	void set_floppy(floppy_image_device *floppy0, floppy_image_device *floppy1);

	void l0ms_w(UINT8 data) { m_lms = (m_lms & 0xf0) | (data & 0x0f); }
	void l1ms_w(UINT8 data) { m_lms = (data << 4) | (m_lms & 0x0f); }
	void st0_w(UINT8 data) { m_st0 = data; }
	void st1_w(UINT8 data) { m_st1 = data; }
	DECLARE_WRITE_LINE_MEMBER( side_select_w ) { m_side = state; }
	DECLARE_WRITE_LINE_MEMBER( drive_select_w ) { m_drive = state; }
	DECLARE_WRITE_LINE_MEMBER( stp0_w ) { m_stp0 = state; }
	DECLARE_WRITE_LINE_MEMBER( stp1_w ) { m_stp1 = state; }
	DECLARE_WRITE_LINE_MEMBER( drw_w ) { }
	DECLARE_WRITE_LINE_MEMBER( erase_w ) { }
	DECLARE_READ_LINE_MEMBER( trk0d0_r ) { return m_floppy0->trk00_r(); }
	DECLARE_READ_LINE_MEMBER( trk0d1_r ) { return m_floppy1->trk00_r(); }
	DECLARE_READ_LINE_MEMBER( wps_r ) { return m_drive ? m_floppy1->wpt_r() : m_floppy0->wpt_r(); }
	DECLARE_READ_LINE_MEMBER( sync_r ) { return 1; }
	DECLARE_WRITE_LINE_MEMBER( led0a_w ) { output_set_led_value(LED_A, state); }
	DECLARE_WRITE_LINE_MEMBER( led1a_w ) { output_set_led_value(LED_B, state); }
	DECLARE_READ_LINE_MEMBER( rdy0_r ) { return m_rdy0; }
	DECLARE_READ_LINE_MEMBER( rdy1_r ) { return m_rdy1; }
	DECLARE_READ_LINE_MEMBER( ds0_r ) { return m_ds0; }
	DECLARE_READ_LINE_MEMBER( ds1_r ) { return m_ds1; }
	DECLARE_READ_LINE_MEMBER( single_double_sided_r ) { return m_drive ? m_floppy1->twosid_r() : m_floppy0->twosid_r(); }
	DECLARE_WRITE_LINE_MEMBER( screset_w ) { if (!state) m_maincpu->reset(); }

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_READ8_MEMBER( floppy_p1_r );
	DECLARE_READ8_MEMBER( floppy_p2_r );
	DECLARE_WRITE8_MEMBER( floppy_p2_w );
	DECLARE_READ8_MEMBER( tach0_r );
	DECLARE_READ8_MEMBER( tach1_r );
	DECLARE_WRITE8_MEMBER( da_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

private:
	enum
	{
		LED_A = 0,
		LED_B
	};

	devcb_write_line m_ds0_cb;
	devcb_write_line m_ds1_cb;
	devcb_write_line m_rdy0_cb;
	devcb_write_line m_rdy1_cb;
	devcb_write_line m_brdy_cb;
	devcb_write_line m_gcrerr_cb;

	required_device<cpu_device> m_maincpu;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	required_memory_region m_gcr_rom;

	void ready0_cb(floppy_image_device *, int device);
	int load0_cb(floppy_image_device *device);
	void unload0_cb(floppy_image_device *device);
	void ready1_cb(floppy_image_device *, int device);
	int load1_cb(floppy_image_device *device);
	void unload1_cb(floppy_image_device *device);

	/* floppy state */
	UINT8 m_da;
	UINT8 m_da0;
	UINT8 m_da1;
	int m_sel0;
	int m_sel1;
	int m_tach0;
	int m_tach1;
	int m_rdy0;
	int m_rdy1;
	int m_ds0;
	int m_ds1;
	UINT8 m_lms;                         /* motor speed */
	int m_st0;                        /* stepper phase */
	int m_st1;                        /* stepper phase */
	int m_stp0;                        /* stepper enable */
	int m_stp1;                        /* stepper enable */
	int m_drive;                        /* selected drive */
	int m_side;                         /* selected side */
	int m_brdy;
	int m_sync;
	int m_gcrerr;
};



// device type definition
extern const device_type VICTOR_9000_FDC;



#endif
