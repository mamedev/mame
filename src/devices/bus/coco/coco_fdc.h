// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    coco_fdc.h

    CoCo/Dragon Floppy Disk Controller base classes

*********************************************************************/

#ifndef MAME_BUS_COCO_COCO_FDC_H
#define MAME_BUS_COCO_COCO_FDC_H

#include "cococart.h"

#include "imagedev/floppy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> coco_family_fdc_device_base

class coco_family_fdc_device_base :
		public device_t,
		public device_cococart_interface
{
public:
	void fdc_intrq_w(int state) { m_intrq = state; update_lines(); }
	void fdc_drq_w(int state) { m_drq = state; update_lines(); }

	static void floppy_formats(format_registration &fr);

protected:
	// construction/destruction
	coco_family_fdc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_cococart_interface(mconfig, *this)
	{
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// FDC overrides
	virtual void update_lines() = 0;
	virtual u8 *get_cart_base() override;
	virtual memory_region *get_cart_memregion() override;

	// accessors
	u8 dskreg() const { return m_dskreg; }
	bool intrq() const { return m_intrq; }
	bool drq() const { return m_drq; }
	void set_dskreg(u8 data) { m_dskreg = data; }

private:
	// registers
	u8 m_dskreg;
	bool m_intrq;
	bool m_drq;
};

// device type definitions - CoCo FDC
DECLARE_DEVICE_TYPE(COCO_FDC,   coco_family_fdc_device_base)
DECLARE_DEVICE_TYPE(COCO_SCII,  coco_family_fdc_device_base)
DECLARE_DEVICE_TYPE(CP450_FDC,  coco_family_fdc_device_base)
DECLARE_DEVICE_TYPE(CD6809_FDC, coco_family_fdc_device_base)

#endif // MAME_BUS_COCO_COCO_FDC_H
