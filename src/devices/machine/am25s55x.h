// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    am25s55x.h
    AMD Am25S557/Am25S558 8x8-bit Combinatorial Multiplier

***************************************************************************/

#ifndef MAME_MACHINE_AM25S55X_H
#define MAME_MACHINE_AM25S55X_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> am25s55x_device

class am25s55x_device : public device_t
{
public:
	// construction/destruction
	am25s55x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	void x_w(uint8_t data);
	void y_w(uint8_t data);
	virtual void xm_w(int state);
	virtual void ym_w(int state);
	void oe_w(int state);

	auto s() { return m_s.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void multiply();

	union input_reg
	{
		int8_t s;
		uint8_t u;
	};

	union output_reg
	{
		int16_t s;
		uint16_t u;
	};

	uint8_t m_x_in;
	uint8_t m_y_in;
	bool m_xm;
	bool m_ym;
	bool m_oe;

	bool m_round_s;
	bool m_round_u;

	input_reg m_x;
	input_reg m_y;
	output_reg m_s_out;

	devcb_write16 m_s;
};


// ======================> am25s557_device

class am25s557_device : public am25s55x_device
{
public:
	// construction/destruction
	am25s557_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void xm_w(int state) override;
	virtual void ym_w(int state) override;

	void r_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_rounding();

	bool m_r;
};


// ======================> am25s558_device

class am25s558_device : public am25s55x_device
{
public:
	// construction/destruction
	am25s558_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void rs_w(int state);
	void ru_w(int state);
};

// device type definition
DECLARE_DEVICE_TYPE(AM25S557, am25s557_device)
DECLARE_DEVICE_TYPE(AM25S558, am25s558_device)

#endif // MAME_MACHINE_AM25S55X_H
