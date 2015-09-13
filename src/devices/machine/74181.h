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
	ttl74181_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inputs
	void input_a_w(UINT8 data);
	void input_b_w(UINT8 data);
	void select_w(UINT8 data);
	DECLARE_WRITE_LINE_MEMBER( mode_w );
	DECLARE_WRITE_LINE_MEMBER( carry_w );

	// outputs
	UINT8 function_r() { return m_f; }
	DECLARE_READ_LINE_MEMBER( carry_r ) { return m_cn; }
	DECLARE_READ_LINE_MEMBER( generate_r ) { return m_g; }
	DECLARE_READ_LINE_MEMBER( propagate_r ) { return m_p; }
	DECLARE_READ_LINE_MEMBER( equals_r ) { return m_equals; }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_post_load();

private:
	void update();

	// inputs
	UINT8 m_a;
	UINT8 m_b;
	UINT8 m_s;
	int m_m;
	int m_c;

	// outputs
	UINT8 m_f;
	int m_cn;
	int m_g;
	int m_p;
	int m_equals;
};


// device type definition
extern const device_type TTL74181;


#endif  /* __74181_H__ */
