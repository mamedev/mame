// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Thomson EF9369

    Single Chip Color Palette

               ___ ___
      VSS   1 |*  u   | 28  HP
     VDDC   2 |       | 27  P3
      SMI   3 |       | 26  P2
       CC   4 |       | 25  P1
       CA   5 |       | 24  P0
       CB   6 |       | 23  BLK
        M   7 |       | 22  AS
      AD0   8 |       | 21  R/W
      VCC   9 |       | 20  DS
    RESET  10 |       | 19  CS0
      AD1  11 |       | 18  /CS
      AD2  12 |       | 17  AD7
      AD3  13 |       | 16  AD6
      AD4  14 |_______| 15  AD5

***************************************************************************/

#ifndef MAME_VIDEO_EF9369_H
#define MAME_VIDEO_EF9369_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define EF9369_COLOR_UPDATE(name)   void name(int entry, bool m, uint8_t ca, uint8_t cb, uint8_t cc)

// ======================> ef9369_device

class ef9369_device : public device_t
{
public:
	typedef device_delegate<void (int entry, bool m, uint8_t ca, uint8_t cb, uint8_t cc)> color_update_delegate;

	// construction/destruction
	ef9369_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	template <typename... T> void set_color_update_callback(T &&... args) { m_color_update_cb.set(std::forward<T>(args)...); }

	uint8_t data_r();
	void data_w(uint8_t data);
	void address_w(uint8_t data);

	static constexpr int NUMCOLORS = 16;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	color_update_delegate m_color_update_cb;

	// state
	uint8_t m_ca[NUMCOLORS], m_cb[NUMCOLORS], m_cc[NUMCOLORS];  // actually 4-bit
	bool m_m[NUMCOLORS];
	int m_address;
};

// device type definition
DECLARE_DEVICE_TYPE(EF9369, ef9369_device)

#endif // MAME_VIDEO_EF9369_H
