// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    74181

    4-Bit Arithmetic Logic Unit

***************************************************************************/

#pragma once

#ifndef __74181_H__
#define __74181_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TTL74181_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TTL74181, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ttl74181_device

class ttl74181_device : public device_t
{
public:
	// construction/destruction
	ttl74181_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	virtual void device_start() override;
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
extern const device_type TTL74181;


#endif  /* __74181_H__ */
