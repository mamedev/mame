// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    RST interrupt vector buffer

**********************************************************************/
#ifndef MAME_MACHINE_RSTBUF_H
#define MAME_MACHINE_RSTBUF_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rst_buffer_device

class rst_buffer_device : public device_t
{
public:
	// configuration
	auto int_callback() { return m_int_cb.bind(); }

	// getter (required override)
	virtual u8 get_vector() const = 0;

	// INTA handler
	IRQ_CALLBACK_MEMBER(inta_cb);

protected:
	// device base class constructor
	rst_buffer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// synchronization helpers
	void sync_input(bool state, u8 mask);
	TIMER_CALLBACK_MEMBER(sync_set_input);
	TIMER_CALLBACK_MEMBER(sync_clear_input);

	// interrupt output callback
	devcb_write_line m_int_cb;

	// input state (D3-D5 of interrupt vector)
	u8 m_input_buffer;
};

// ======================> rst_pos_buffer_device

class rst_pos_buffer_device : public rst_buffer_device
{
public:
	// device constructor
	rst_pos_buffer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// set RST 1/RST 08H request line (modifies bit 3 of vector)
	void rst1_w(int state) { sync_input(state, 0x08); }

	// set RST 2/RST 10H request line (modifies bit 4 of vector)
	void rst2_w(int state) { sync_input(state, 0x10); }

	// set RST 3/RST 20H request line (modifies bit 5 of vector)
	void rst4_w(int state) { sync_input(state, 0x20); }

protected:
	// getter (required override)
	virtual u8 get_vector() const override { return 0xc7 | m_input_buffer; }
};

// ======================> rst_neg_buffer_device

class rst_neg_buffer_device : public rst_buffer_device
{
public:
	// device constructor
	rst_neg_buffer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// set RST 30H request line (modifies bit 3 of vector)
	void rst30_w(int state) { sync_input(state, 0x08); }

	// set RST 28H request line (modifies bit 4 of vector)
	void rst28_w(int state) { sync_input(state, 0x10); }

	// set RST 18H request line (modifies bit 5 of vector)
	void rst18_w(int state) { sync_input(state, 0x20); }

protected:
	// getter (required override)
	virtual u8 get_vector() const override { return 0xff ^ m_input_buffer; }
};

// device type definitions
DECLARE_DEVICE_TYPE(RST_POS_BUFFER, rst_pos_buffer_device)
DECLARE_DEVICE_TYPE(RST_NEG_BUFFER, rst_neg_buffer_device)

#endif // MAME_MACHINE_RSTBUF_H
