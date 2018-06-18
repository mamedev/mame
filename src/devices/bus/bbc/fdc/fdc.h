// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

        BBC Micro Floppy Disc Controller slot emulation

**********************************************************************/

#ifndef MAME_BUS_BBC_FDC_FDC_H
#define MAME_BUS_BBC_FDC_FDC_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BBC_FDC_SLOT_ADD(_tag, _slot_intf, _def_slot, _fixed) \
	MCFG_DEVICE_ADD(_tag, BBC_FDC_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _fixed)

#define MCFG_BBC_FDC_SLOT_INTRQ_HANDLER(_devcb) \
	devcb = &downcast<bbc_fdc_slot_device &>(*device).set_intrq_handler(DEVCB_##_devcb);

#define MCFG_BBC_FDC_SLOT_DRQ_HANDLER(_devcb) \
	devcb = &downcast<bbc_fdc_slot_device &>(*device).set_drq_handler(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_fdc_slot_device

class device_bbc_fdc_interface;

class bbc_fdc_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	bbc_fdc_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~bbc_fdc_slot_device();

	// callbacks
	template <class Object> devcb_base &set_intrq_handler(Object &&cb) { return m_intrq_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_drq_handler(Object &&cb) { return m_drq_handler.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE_LINE_MEMBER( intrq_w ) { m_intrq_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( drq_w) { m_drq_handler(state); }

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	device_bbc_fdc_interface *m_card;

private:
	devcb_write_line m_intrq_handler;
	devcb_write_line m_drq_handler;
};


// ======================> device_bbc_fdc_interface

class device_bbc_fdc_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_bbc_fdc_interface();

protected:
	device_bbc_fdc_interface(const machine_config &mconfig, device_t &device);

	bbc_fdc_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_FDC_SLOT, bbc_fdc_slot_device)

void bbc_fdc_devices(device_slot_interface &device);


#endif // MAME_BUS_BBC_FDC_FDC_H
