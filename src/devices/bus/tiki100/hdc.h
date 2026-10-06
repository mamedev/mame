// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIKI-100 Winchester controller card emulation

**********************************************************************/

#ifndef MAME_BUS_TIKI100_HDC_H
#define MAME_BUS_TIKI100_HDC_H

#pragma once

#include "bus/tiki100/exp.h"
#include "imagedev/harddriv.h"
#include "machine/wd2010.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tiki100_hdc_device

class tiki100_hdc_device : public device_t, public device_tiki100bus_card_interface
{
public:
	// construction/destruction
	tiki100_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_tiki100bus_card_interface overrides
	virtual uint8_t iorq_r(offs_t offset, uint8_t data) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

private:
	required_device<wd2010_device> m_hdc;
	required_device_array<harddisk_image_device, 2> m_hd;

	uint8_t m_buffer[0x400];
	uint16_t m_counter;
	bool m_bdrq;
	bool m_bcs;
	bool m_wg;

	harddisk_image_device *selected_drive();
	uint32_t selected_lba(harddisk_image_device *hd);
	int sector_size();

	void bdrq_w(int state);
	void bcr_w(int state);
	void bcs_w(int state);
	void wg_w(int state);
	int drdy_r();
};


// device type definition
DECLARE_DEVICE_TYPE(TIKI100_HDC, tiki100_hdc_device)

#endif // MAME_BUS_TIKI100_HDC_H
