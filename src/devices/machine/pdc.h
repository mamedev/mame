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
	pdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	/* Optional information overrides */
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	/* Callbacks */
	template<class _Object> static devcb_base &m68k_r_callback(device_t &device, _Object object) { return downcast<pdc_device &>(device).m_m68k_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &m68k_w_callback(device_t &device, _Object object) { return downcast<pdc_device &>(device).m_m68k_w_cb.set_callback(object); }

	/* Read and Write members */
	void i8237_hreq_w(int state);
	void i8237_eop_w(int state);
	uint8_t i8237_dma_mem_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8237_dma_mem_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t i8237_fdc_dma_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8237_fdc_dma_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void hdd_irq(int state);

	uint8_t p0_7_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p0_7_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fdd_68k_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fdd_68k_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p38_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p38_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t p39_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p50_5f_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t m68k_dma_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m68k_dma_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void fdc_irq(int state);
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	/* Main CPU accessible registers */
	uint8_t reg_p0;
	uint8_t reg_p1;
	uint8_t reg_p2;
	uint8_t reg_p3;
	uint8_t reg_p4;
	uint8_t reg_p5;
	uint8_t reg_p6;
	uint8_t reg_p7;
	uint8_t reg_p21;
	uint8_t reg_p38;
	uint32_t fdd_68k_dma_address; /* FDD <-> m68k DMA read/write address */
protected:
	/* Device-level overrides */
	virtual void device_start() override;
	virtual void device_reset() override;

	/* Protected variables */
	//uint32_t fdd_68k_dma_address;
	bool b_fdc_irq;

	/* Attached devices */
	required_device<cpu_device> m_pdccpu;
	required_device<am9517a_device> m_dma8237;
	required_device<upd765a_device> m_fdc;
	//required_device<floppy_connector> m_floppy;
	//required_device<floppy_image_device> m_floppy;
	optional_device<hdc9224_device> m_hdc9224;
	mfm_harddisk_device*    m_harddisk;
	required_shared_ptr<uint8_t> m_pdc_ram;

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
