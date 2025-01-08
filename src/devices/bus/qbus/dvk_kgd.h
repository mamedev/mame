// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Monochrome register-addressable framebuffer used in DVK.

***************************************************************************/

#ifndef MAME_BUS_QBUS_DVK_KGD_H
#define MAME_BUS_QBUS_DVK_KGD_H

#pragma once

#include "qbus.h"

#include "screen.h"


static constexpr int KGDCR_WR = 0140000;
static constexpr int KGDDR_WR = 0000377;
static constexpr int KGDAR_WR = 0037777;
static constexpr int KGDCT_WR = 0037777;


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> dvk_kgd_device

class dvk_kgd_device : public device_t,
					public device_qbus_card_interface
{
public:
	// construction/destruction
	dvk_kgd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<screen_device> m_screen;

private:
	std::unique_ptr<uint8_t[]> m_videoram_base;
	uint8_t *m_videoram;

	uint16_t m_cr;
	uint16_t m_dr;
	uint16_t m_ar;
	uint16_t m_ct;
};


// device type definition
DECLARE_DEVICE_TYPE(DVK_KGD, dvk_kgd_device)

#endif
