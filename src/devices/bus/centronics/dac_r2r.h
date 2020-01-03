// license:BSD-3-Clause
// copyright-holders:Nigel Barnes

#ifndef MAME_BUS_CENTRONICS_DAC_R2R_H
#define MAME_BUS_CENTRONICS_DAC_R2R_H

#pragma once

#include "ctronics.h"
#include "sound/dac.h"

// ======================> centronics_dac_r2r_device

class centronics_dac_r2r_device : public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	centronics_dac_r2r_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual DECLARE_WRITE_LINE_MEMBER( input_data0 ) override { if (state) m_data |= 0x01; else m_data &= ~0x01; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data1 ) override { if (state) m_data |= 0x02; else m_data &= ~0x02; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data2 ) override { if (state) m_data |= 0x04; else m_data &= ~0x04; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data3 ) override { if (state) m_data |= 0x08; else m_data &= ~0x08; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data4 ) override { if (state) m_data |= 0x10; else m_data &= ~0x10; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data5 ) override { if (state) m_data |= 0x20; else m_data &= ~0x20; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data6 ) override { if (state) m_data |= 0x40; else m_data &= ~0x40; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data7 ) override { if (state) m_data |= 0x80; else m_data &= ~0x80; update_dac(); }

private:
	required_device<dac_byte_interface> m_dac;

	void update_dac();

	uint8_t m_data;
};

// device type definition
DECLARE_DEVICE_TYPE(CENTRONICS_DAC_R2R, centronics_dac_r2r_device)


#endif // MAME_BUS_CENTRONICS_DAC_R2R_H
