// license:BSD-3-Clause
// copyright-holders:r09
/**********************************************************************

    Fujitsu FM Towns SCSI card slot

**********************************************************************/

#ifndef MAME_BUS_FMT_SCSI_FMT_SCSI_H
#define MAME_BUS_FMT_SCSI_FMT_SCSI_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class fmt_scsi_card_interface;

// ======================> fmt_scsi_slot_device

class fmt_scsi_slot_device : public device_t, public device_single_card_slot_interface<fmt_scsi_card_interface>
{
	friend class fmt_scsi_card_interface;

public:
	// construction/destruction
	template <typename T>
	fmt_scsi_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: fmt_scsi_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	fmt_scsi_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto irq_handler() { return m_irq_handler.bind(); }
	auto drq_handler() { return m_drq_handler.bind(); }

	u8 read(address_space &space, offs_t offset);
	void write(offs_t offset, u8 data);

	uint8_t data_read(void);
	void data_write(uint8_t data);

	void irq_w(int state);
	void drq_w(int state);

protected:
	// device-specific overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	fmt_scsi_card_interface *m_card;

	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;

};


// ======================> fmt_scsi_card_interface

class fmt_scsi_card_interface : public device_interface
{
	friend class fmt_scsi_slot_device;

protected:
	// construction/destruction
	fmt_scsi_card_interface(const machine_config &mconfig, device_t &device);

	// required overrides
	virtual u8 fmt_scsi_read(offs_t offset) = 0;
	virtual void fmt_scsi_write(offs_t offset, u8 data) = 0;

	virtual u8 fmt_scsi_data_read(void) = 0;
	virtual void fmt_scsi_data_write(u8 data) = 0;

	fmt_scsi_slot_device *m_slot;

private:
	virtual void interface_pre_start() override;

};


// device type definition
DECLARE_DEVICE_TYPE(FMT_SCSI_SLOT, fmt_scsi_slot_device)

void fmt_scsi_default_devices(device_slot_interface &device);

#endif // MAME_BUS_FMT_SCSI_FMT_SCSI_H
