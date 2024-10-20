// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/**********************************************************************

    i3001.h

    Intel 3001 Microprogram Control Unit

**********************************************************************
             _____   _____
     PX4/  1|*    \_/     |40 VCC
     PX7/  2|             |39 AC0
     PX6/  3|             |38 AC1
     PX5/  4|             |37 AC5
     SX3/  5|             |36 LD
     SX2/  6|             |35 ERA
      PR2  7|             |34 MA8
     SX1/  8|             |33 MA7
      PR1  9|  DIP40      |32 MA6
     SX0/ 10|  i3001      |31 MA5
      PR0 11|             |30 MA4
      FC3 12|             |29 MA0
      FC2 13|             |28 MA3
      FO/ 14|             |27 MA2
      FC0 15|             |26 MA1
      FC1 16|             |25 EN
      FI/ 17|             |24 AC6
      ISE 18|             |23 AC4
      CLK 19|             |22 AC3
      GND 20|_____________|21 AC2

**********************************************************************/

#ifndef MAME_MACHINE_I3001_H
#define MAME_MACHINE_I3001_H

#pragma once

class i3001_device : public device_t
{
public:
	i3001_device(const machine_config &mconfig , const char *tag , device_t *owner , uint32_t clock = 0);

	// Output callbacks
	template <typename... T> void set_fo_w_cb(T &&... args) { m_fo_handler.set(std::forward<T>(args)...); }

	// Input callbacks
	template <typename... T> void set_px_r_cb(T &&... args) { m_px_handler.set(std::forward<T>(args)...); }
	template <typename... T> void set_sx_r_cb(T &&... args) { m_sx_handler.set(std::forward<T>(args)...); }

	// Mask of valid bits in address
	static constexpr uint16_t ADDR_MASK = ((1U << 9) - 1);

	// Read current microprogram address (9 bits)
	uint16_t addr_r() const { return m_addr; }

	// Set Address Control code (7 bits)
	void ac_w(uint16_t ac) { m_ac = ac; }

	// Set Flag Control code (4 bits)
	void fc_w(uint8_t fc);

	// Write Flag Input
	void fi_w(int state) { m_fi = state; }

	// Read Flag Output
	int fo_r() { return m_fo; }

	// Read carry/zero flags
	int carry_r() { return m_carry; }
	int zero_r() { return m_zero; }

	// Load address (in real hw address is loaded through PX/SX buses)
	void addr_w(uint16_t addr) { m_addr = addr & ADDR_MASK; }

	// Clock pulse
	void clk_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	device_delegate<void (int)> m_fo_handler;
	device_delegate<uint8_t ()> m_px_handler;
	device_delegate<uint8_t ()> m_sx_handler;

	uint16_t m_addr;
	uint8_t m_pr;
	uint8_t m_ac;
	uint8_t m_fc;
	bool m_fi;
	bool m_fo;
	bool m_carry;
	bool m_zero;
	bool m_flag;

	void update();
	static uint8_t get_row(uint16_t addr) { return (addr >> 4); }
	static uint8_t get_col(uint16_t addr) { return addr & 0b1111; }
	static uint16_t pack_row_col(uint8_t row , uint8_t col) { return ((uint16_t)row << 4) | col; }
};

// device type definition
DECLARE_DEVICE_TYPE(I3001, i3001_device)

#endif /* MAME_MACHINE_I3001_H */
