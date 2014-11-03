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
#include "machine/6522via.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VICTOR_9000_FDC_IRQ_CB(_write) \
	devcb = &victor_9000_fdc_t::set_irq_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> victor_9000_fdc_t

class victor_9000_fdc_t :  public device_t
{
public:
	// construction/destruction
	victor_9000_fdc_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<victor_9000_fdc_t &>(device).m_irq_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( cs5_r ) { return m_via4->read(space, offset); }
	DECLARE_WRITE8_MEMBER( cs5_w ) { return m_via4->write(space, offset, data); }
	DECLARE_READ8_MEMBER( cs6_r ) { return m_via6->read(space, offset); }
	DECLARE_WRITE8_MEMBER( cs6_w ) { return m_via6->write(space, offset, data); }
	DECLARE_READ8_MEMBER( cs7_r ) { return m_via5->read(space, offset); }
	DECLARE_WRITE8_MEMBER( cs7_w ) { return m_via5->write(space, offset, data); }

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_READ8_MEMBER( floppy_p1_r );
	DECLARE_READ8_MEMBER( floppy_p2_r );
	DECLARE_WRITE8_MEMBER( floppy_p2_w );
	DECLARE_READ8_MEMBER( tach0_r );
	DECLARE_READ8_MEMBER( tach1_r );
	DECLARE_WRITE8_MEMBER( da_w );

	DECLARE_WRITE8_MEMBER( via4_pa_w );
	DECLARE_WRITE8_MEMBER( via4_pb_w );
	DECLARE_WRITE_LINE_MEMBER( mode_w );
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

	devcb_write_line m_irq_cb;

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via4;
	required_device<via6522_device> m_via5;
	required_device<via6522_device> m_via6;
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

	int m_via4_irq;
	int m_via5_irq;
	int m_via6_irq;
};



// device type definition
extern const device_type VICTOR_9000_FDC;



#endif
