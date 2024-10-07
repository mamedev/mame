// license:BSD-3-Clause
// copyright-holders:r09

#ifndef MAME_BUS_FMT_SCSI_FMT121_H
#define MAME_BUS_FMT_SCSI_FMT121_H

#pragma once

#include "fmt_scsi.h"

#include "machine/fm_scsi.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fmt121_device

class fmt121_device : public device_t, public fmt_scsi_card_interface
{
public:
	// device type constructor
	fmt121_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// fmt_scsi_card_interface overrides
	virtual u8 fmt_scsi_read(offs_t offset) override;
	virtual void fmt_scsi_write(offs_t offset, u8 data) override;

	virtual u8 fmt_scsi_data_read(void) override;
	virtual void fmt_scsi_data_write(u8 data) override;

private:

	required_device<fmscsi_device> m_scsi_ctlr;

	void irq_w(int state);
	void drq_w(int state);

};

// device type declaration
DECLARE_DEVICE_TYPE(FMT121, fmt121_device)

#endif // MAME_BUS_FMT_SCSI_FMT121_H
