// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#pragma once

#ifndef __S97269PB_H__
#define __S97269PB_H__

#define MCFG_S97269PB_ADD(_tag ) \
	MCFG_DEVICE_ADD(_tag, S97269PB, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class s97269pb_device : public device_t
{
public:
	// construction/destruction
	s97269pb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// daughterboard logic
	void palette_bank_w(uint8_t data);
	pen_t choosePen(uint8_t x, uint8_t y, pen_t back_pen);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	// bit 2 enables gradient and starfield 
	// bit 3 seems to be used to flip for cocktail
	uint8_t m_palette_bank;
};

// device type definition
extern const device_type S97269PB;

#endif  /* __S97269PB_H__ */
