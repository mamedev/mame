// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sanyo LC7535

    Electronic Volume/Loudness Control with Serial Data Control

              ___ ___
   L5dBIN  1 |*  u   | 22  R5dBIN
     LCT1  2 |       | 21  RCT1
     LCT2  3 |       | 20  RCT2
  L5dBOUT  4 |       | 19  R5dBOUT
   L1dBIN  5 |       | 18  R1dBIN
  L1dBOUT  6 |LC7535P| 17  R1dBOUT
      LVM  7 |       | 16  RVM
      VEE  8 |       | 16  VCC
        S  9 |       | 14  CE
      VDD 10 |       | 13  DI
      VSS 11 |_______| 12  CLK

***************************************************************************/

#ifndef MAME_SOUND_LC7535_H
#define MAME_SOUND_LC7535_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_LC7535_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, LC7535, 0)

#define MCFG_LC7535_SELECT_CB(_read) \
		devcb = &lc7535_device::set_select_callback(*device, DEVCB_##_read);

#define MCFG_LC7535_VOLUME_CB(_class, _method) \
	lc7535_device::set_volume_callback(*device, lc7535_device::volume_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define LC7535_VOLUME_CHANGED(name) void name(int attenuation_right, int attenuation_left, bool loudness)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class lc7535_device : public device_t
{
public:
	// construction/destruction
	lc7535_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	typedef device_delegate<void (int attenuation_right, int attenuation_left, bool loudness)> volume_delegate;

	static void set_volume_callback(device_t &device, volume_delegate &&cb)
		{ downcast<lc7535_device &>(device).m_volume_cb = std::move(cb); }

	template <class Object> static devcb_base &set_select_callback(device_t &device, Object &&cb)
		{ return downcast<lc7535_device &>(device).m_select_cb.set_callback(std::forward<Object>(cb)); }

	// serial interface
	DECLARE_WRITE_LINE_MEMBER( ce_w );
	DECLARE_WRITE_LINE_MEMBER( di_w );
	DECLARE_WRITE_LINE_MEMBER( clk_w );

	float normalize(int attenuation);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// maximum attenuation is -98 dB for infinity
	enum { MAX = -98 };

	static constexpr int m_5db[16] = { -75, -70, -65, -60, -55, -50, -45, -40, -35, -30, -25, -20, -15, -10, -5, -0 };
	static constexpr int m_1db[8] = { MAX, MAX, -4, -3, -2, -1, 0, MAX };

	// callbacks
	devcb_read_line m_select_cb;
	volume_delegate m_volume_cb;

	// state
	uint8_t m_addr;
	uint16_t m_data;
	int m_count, m_ce, m_di, m_clk;
};

// device type definition
DECLARE_DEVICE_TYPE(LC7535, lc7535_device)

#endif // MAME_SOUND_LC7535_H
