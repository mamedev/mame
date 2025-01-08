// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/**********************************************************************

    i3002.h

    Intel 3002 Central Processing Element

**********************************************************************
             _____   _____
      I0/  1|*    \_/     |28 VCC
      I1/  2|             |27 F2
      K0/  3|             |26 F1
      K1/  4|             |25 F0
        X  5|             |24 F3
        Y  6|             |23 ED/
      CO/  7|             |22 M0/
      RO/  8|             |21 M1/
      LI/  9|  DIP28      |20 D1/
      CI/ 10|  i3002      |19 D0/
      EA/ 11|             |18 CLK
      A1/ 12|             |17 F4
      A0/ 13|             |16 F5
      GND 14|_____________|15 F6

**********************************************************************/

#ifndef MAME_MACHINE_I3002_H
#define MAME_MACHINE_I3002_H

#pragma once

class i3002_device : public device_t
{
public:
	i3002_device(const machine_config &mconfig , const char *tag , device_t *owner , uint32_t clock = 0);

	// Output callbacks
	template <typename... T> void set_co_w_cb(T &&... args) { m_co_handler.set(std::forward<T>(args)...); }
	template <typename... T> void set_ro_w_cb(T &&... args) { m_ro_handler.set(std::forward<T>(args)...); }

	// Input bus callbacks
	template <typename... T> void set_ibus_r_cb(T &&... args) { m_ibus_handler.set(std::forward<T>(args)...); }
	template <typename... T> void set_mbus_r_cb(T &&... args) { m_mbus_handler.set(std::forward<T>(args)...); }

	// Width of bit-slice
	static constexpr unsigned SLICE_SIZE = 2;

	// Mask of significant bits
	static constexpr uint8_t WORD_MASK = ((1U << SLICE_SIZE) - 1);

	// Registers
	enum : unsigned {
		  REG_R0,
		  REG_R1,
		  REG_R2,
		  REG_R3,
		  REG_R4,
		  REG_R5,
		  REG_R6,
		  REG_R7,
		  REG_R8,
		  REG_R9,
		  REG_T,
		  REG_AC,
		  REG_MAR,
		  REG_COUNT
	};

	static const char *reg_name(unsigned reg_idx);

	// Decode function code into function group, register group and register index
	static void decode_fc(uint8_t fc , uint8_t& fg , uint8_t& rg , unsigned& reg);

	// Set function code (7 bits) and K-bus (2 bits)
	void fc_kbus_w(uint8_t fc , uint8_t k);

	// Write Carry Input
	void ci_w(int state) { m_ci = state; }

	// Write Left Input
	void li_w(int state) { m_li = state; }

	// Read Carry Output
	int co_r() const { return m_co; }

	// Read Right Output
	int ro_r() const { return m_ro; }

	// Read output buses
	uint8_t abus_r() const { return m_reg[ REG_MAR ]; }
	uint8_t dbus_r() const { return m_reg[ REG_AC ]; }

	// Clock pulse
	void clk_w(int state);

	// Compute RO if FC is the code of right-shift op (and return true)
	// Return false in all other cases
	bool update_ro();

	// Access to registers (debugging only: registers are not directly accessible in the real hw)
	const uint8_t& get_reg(unsigned idx) const { return m_reg[ idx ]; }
	uint8_t& get_reg(unsigned idx) { return m_reg[ idx ]; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	device_delegate<void (int)> m_co_handler;
	device_delegate<void (int)> m_ro_handler;
	device_delegate<uint8_t ()> m_ibus_handler;
	device_delegate<uint8_t ()> m_mbus_handler;

	uint8_t m_reg[ REG_COUNT ];
	uint8_t m_fc;
	uint8_t m_kbus;
	bool m_ci;
	bool m_li;
	bool m_co;
	bool m_ro;

	void update();
	void set_co(bool co);
	void set_ro(bool ro);
};

// device type definition
DECLARE_DEVICE_TYPE(I3002, i3002_device)

#endif /* MAME_MACHINE_I3002_H */
