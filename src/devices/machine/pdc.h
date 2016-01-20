// license:GPL-2.0+
// copyright-holders:Brandon Munger
/**********************************************************************

    ROLM 9751 9005 Peripheral device controller emulation

**********************************************************************/

#pragma once

#ifndef __R9751_PDC_H__
#define __R9751_PDC_H__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/upd765.h"
#include "machine/am9517a.h"
#include "formats/pc_dsk.h"
#include "machine/hdc92x4.h"
#include "imagedev/mfmhd.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define PDC_TAG           "pdc"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pdc_device

class pdc_device :  public device_t
{
public:
		/* Constructor and Destructor */
	pdc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

		/* Optional information overrides */
		virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const rom_entry *device_rom_region() const override;

	/* Callbacks */
	template<class _Object> static devcb_base &m68k_r_callback(device_t &device, _Object object) { return downcast<pdc_device &>(device).m_m68k_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &m68k_w_callback(device_t &device, _Object object) { return downcast<pdc_device &>(device).m_m68k_w_cb.set_callback(object); }

	/* Read and Write members */
	DECLARE_WRITE_LINE_MEMBER(i8237_hreq_w);
	DECLARE_WRITE_LINE_MEMBER(i8237_eop_w);
	DECLARE_READ8_MEMBER(i8237_dma_mem_r);
	DECLARE_WRITE8_MEMBER(i8237_dma_mem_w);
	DECLARE_READ8_MEMBER(i8237_fdc_dma_r);
	DECLARE_WRITE8_MEMBER(i8237_fdc_dma_w);

	DECLARE_WRITE_LINE_MEMBER(hdd_irq);

	DECLARE_READ8_MEMBER(p0_7_r);
	DECLARE_WRITE8_MEMBER(p0_7_w);
	DECLARE_READ8_MEMBER(fdd_68k_r);
	DECLARE_WRITE8_MEMBER(fdd_68k_w);
	DECLARE_WRITE8_MEMBER(p38_w);
	DECLARE_READ8_MEMBER(p38_r);
	DECLARE_READ8_MEMBER(p39_r);
	DECLARE_WRITE8_MEMBER(p50_5f_w);

	DECLARE_READ8_MEMBER(m68k_dma_r);
	DECLARE_WRITE8_MEMBER(m68k_dma_w);

	DECLARE_WRITE_LINE_MEMBER(fdc_irq);
		DECLARE_FLOPPY_FORMATS( floppy_formats );

	/* Main CPU accessible registers */
	UINT8 reg_p0;
	UINT8 reg_p1;
	UINT8 reg_p2;
	UINT8 reg_p3;
	UINT8 reg_p4;
	UINT8 reg_p5;
	UINT8 reg_p6;
	UINT8 reg_p7;
	UINT8 reg_p21;
	UINT8 reg_p38;
	UINT32 fdd_68k_dma_address; /* FDD <-> m68k DMA read/write address */
protected:
		/* Device-level overrides */
		virtual void device_start() override;
		virtual void device_reset() override;

	/* Protected variables */
	//UINT32 fdd_68k_dma_address;
	bool b_fdc_irq;

	/* Attached devices */
		required_device<cpu_device> m_pdccpu;
	required_device<am9517a_device> m_dma8237;
	required_device<upd765a_device> m_fdc;
	//required_device<floppy_connector> m_floppy;
	//required_device<floppy_image_device> m_floppy;
	optional_device<hdc9224_device> m_hdc9224;
	mfm_harddisk_device*    m_harddisk;
	required_shared_ptr<UINT8> m_pdc_ram;

	/* Callbacks */
	devcb_read8 m_m68k_r_cb;
	devcb_write8    m_m68k_w_cb;
};

/* Device type */
extern const device_type PDC;

/* MCFG defines */
#define MCFG_PDC_R_CB(_devcb) \
	devcb = &pdc_device::m68k_r_callback(*device, DEVCB_##_devcb);
#define MCFG_PDC_W_CB(_devcb) \
	devcb = &pdc_device::m68k_w_callback(*device, DEVCB_##_devcb);
#endif
