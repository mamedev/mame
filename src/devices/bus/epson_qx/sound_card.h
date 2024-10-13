// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * YM2149 based sound card with header for an external MPU401
 *
 *******************************************************************/

#ifndef MAME_BUS_EPSON_QX_YM2149_H
#define MAME_BUS_EPSON_QX_YM2149_H

#pragma once

#include "option.h"

#include "machine/mpu401.h"
#include "sound/ay8910.h"

namespace bus::epson_qx {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* YM2149 Sound Card Device */

class ym2149_sound_card_device : public device_t, public device_option_expansion_interface
{
public:
	// construction/destruction
	ym2149_sound_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void map(address_map &map) ATTR_COLD;

private:
	uint8_t m_irqline;
	void mpu_irq_out(int state);

	required_device<mpu401_device> m_mpu401;
	required_device<ym2149_device> m_ssg;
	required_ioport m_iobase;
	required_ioport m_irq;
	bool m_installed;
};

} // namespace bus::epson_qx

// device type definition
DECLARE_DEVICE_TYPE_NS(EPSON_QX_OPTION_YM2149, bus::epson_qx, ym2149_sound_card_device)


#endif // MAME_BUS_EPSON_QX_YM2149_H
