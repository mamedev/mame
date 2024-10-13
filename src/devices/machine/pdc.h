// license:GPL-2.0+
// copyright-holders:Brandon Munger
/**********************************************************************

    ROLM 9751 9005 Peripheral device controller emulation

**********************************************************************/

#ifndef MAME_MACHINE_PDC_H
#define MAME_MACHINE_PDC_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/upd765.h"
#include "machine/am9517a.h"
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

class pdc_device : public device_t
{
public:
	/* Constructor and Destructor */
	pdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	/* Callbacks */
	auto m68k_r_callback() { return m_m68k_r_cb.bind(); }
	auto m68k_w_callback() { return m_m68k_w_cb.bind(); }

	/* Read and Write members */
	void hdd_irq(int state);

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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	/* Optional information overrides */
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void i8237_hreq_w(int state);
	void i8237_eop_w(int state);
	uint8_t i8237_dma_mem_r(offs_t offset);
	void i8237_dma_mem_w(offs_t offset, uint8_t data);
	uint8_t i8237_fdc_dma_r(offs_t offset);
	void i8237_fdc_dma_w(offs_t offset, uint8_t data);

	uint8_t m68k_dma_r();
	void m68k_dma_w(uint8_t data);

	void fdc_irq(int state);

	uint8_t p0_7_r(offs_t offset);
	void p0_7_w(offs_t offset, uint8_t data);
	uint8_t fdd_68k_r(offs_t offset);
	void fdd_68k_w(offs_t offset, uint8_t data);
	void p38_w(uint8_t data);
	uint8_t p38_r();
	uint8_t p39_r();
	void p50_5f_w(offs_t offset, uint8_t data);

	void pdc_io(address_map &map) ATTR_COLD;
	void pdc_mem(address_map &map) ATTR_COLD;

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
	//mfm_harddisk_device*    m_harddisk;
	required_shared_ptr<uint8_t> m_pdc_ram;

	/* Callbacks */
	devcb_read8 m_m68k_r_cb;
	devcb_write8 m_m68k_w_cb;
};

/* Device type */
DECLARE_DEVICE_TYPE(PDC, pdc_device)


#endif // MAME_MACHINE_PDC_H
