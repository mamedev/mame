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

class pdc_device : public device_t
{
public:
	/* Constructor and Destructor */
	pdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	/* Callbacks */
	auto m68k_r_callback() { return m_m68k_r_cb.bind(); }
	auto m68k_w_callback() { return m_m68k_w_cb.bind(); }

	/* Read and Write members */
	DECLARE_WRITE_LINE_MEMBER(hdd_irq);

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
	/* Optional information overrides */
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	DECLARE_WRITE_LINE_MEMBER(i8237_hreq_w);
	DECLARE_WRITE_LINE_MEMBER(i8237_eop_w);
	DECLARE_READ8_MEMBER(i8237_dma_mem_r);
	DECLARE_WRITE8_MEMBER(i8237_dma_mem_w);
	DECLARE_READ8_MEMBER(i8237_fdc_dma_r);
	DECLARE_WRITE8_MEMBER(i8237_fdc_dma_w);

	DECLARE_READ8_MEMBER(m68k_dma_r);
	DECLARE_WRITE8_MEMBER(m68k_dma_w);

	DECLARE_WRITE_LINE_MEMBER(fdc_irq);
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_READ8_MEMBER(p0_7_r);
	DECLARE_WRITE8_MEMBER(p0_7_w);
	DECLARE_READ8_MEMBER(fdd_68k_r);
	DECLARE_WRITE8_MEMBER(fdd_68k_w);
	DECLARE_WRITE8_MEMBER(p38_w);
	DECLARE_READ8_MEMBER(p38_r);
	DECLARE_READ8_MEMBER(p39_r);
	DECLARE_WRITE8_MEMBER(p50_5f_w);

	void pdc_io(address_map &map);
	void pdc_mem(address_map &map);

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
