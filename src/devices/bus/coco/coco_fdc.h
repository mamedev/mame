// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    coco_fdc.h

    CoCo/Dragon Floppy Disk Controller

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
	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w) { m_intrq = state; update_lines(); }
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w) { m_drq = state; update_lines(); }

	DECLARE_FLOPPY_FORMATS(floppy_formats);

protected:
	// construction/destruction
	coco_family_fdc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_cococart_interface(mconfig, *this)
	{
		m_owner = dynamic_cast<cococart_slot_device *>(owner);
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// FDC overrides
	virtual void update_lines() = 0;
	virtual uint8_t* get_cart_base() override;

	// wrapper for setting the cart line
	void cart_set_line(cococart_slot_device::line which, cococart_slot_device::line_value value)
	{
		m_owner->set_line_value(which, value);
	}
	void cart_set_line(cococart_slot_device::line which, bool value)
	{
		cart_set_line(which, value ? cococart_slot_device::line_value::ASSERT : cococart_slot_device::line_value::CLEAR);
	}

	// accessors
	uint8_t dskreg() const { return m_dskreg; }
	bool intrq() const { return m_intrq; }
	bool drq() const { return m_drq; }
	void set_dskreg(uint8_t data) { m_dskreg = data; }

private:
	// internal state
	cococart_slot_device *m_owner;

	// registers
	uint8_t m_dskreg;
	bool m_intrq;
	bool m_drq;
};

// device type definitions - CoCo FDC
extern const device_type COCO_FDC;
extern const device_type COCO_FDC_V11;
extern const device_type COCO3_HDB1;
extern const device_type CP400_FDC;

// device type definitions - Dragon FDC
extern const device_type DRAGON_FDC;
extern const device_type SDTANDY_FDC;

#endif // MAME_BUS_COCO_COCO_FDC_H
