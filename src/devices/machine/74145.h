// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    TTL74145

    BCD-to-Decimal decoder

***************************************************************************/

#ifndef MAME_MACHINE_74145_H
#define MAME_MACHINE_74145_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ttl74145_device

class ttl74145_device :  public device_t
{
public:
	// construction/destruction
	ttl74145_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <std::size_t Bit> auto output_line_callback() { return m_output_line_cb[Bit].bind(); }

	uint16_t read();
	void write(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line m_output_line_cb[10];

	/* decoded number */
	uint16_t m_number;
};

// device type definition
DECLARE_DEVICE_TYPE(TTL74145, ttl74145_device)

#endif // MAME_MACHINE_74145_H
