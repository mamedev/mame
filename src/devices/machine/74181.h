// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    74181

    4-Bit Arithmetic Logic Unit

***************************************************************************/

#ifndef MAME_MACHINE_74181_H
#define MAME_MACHINE_74181_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ttl74181_device

class ttl74181_device : public device_t
{
public:
	// construction/destruction
	ttl74181_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// inputs
	void input_a_w(uint8_t data);
	void input_b_w(uint8_t data);
	void select_w(uint8_t data);
	void mode_w(int state);
	void carry_w(int state);

	// outputs
	uint8_t function_r() { return m_f; }
	int carry_r() { return m_cn; }
	int generate_r() { return m_g; }
	int propagate_r() { return m_p; }
	int equals_r() { return m_equals; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	void update();

	// inputs
	uint8_t m_a;
	uint8_t m_b;
	uint8_t m_s;
	int m_m;
	int m_c;

	// outputs
	uint8_t m_f;
	int m_cn;
	int m_g;
	int m_p;
	int m_equals;
};


// device type definition
DECLARE_DEVICE_TYPE(TTL74181, ttl74181_device)

#endif // MAME_MACHINE_74181_H
