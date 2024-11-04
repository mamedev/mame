// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    TE7750 Super I/O Expander

**********************************************************************/

#ifndef MAME_MACHINE_TE7750_H
#define MAME_MACHINE_TE7750_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> te7750_device

class te7750_device : public device_t
{
public:
	// construction/destruction
	te7750_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration
	auto in_port1_cb() { return m_input_cb[0].bind(); }
	auto in_port2_cb() { return m_input_cb[1].bind(); }
	auto in_port3_cb() { return m_input_cb[2].bind(); }
	auto in_port4_cb() { return m_input_cb[3].bind(); }
	auto in_port5_cb() { return m_input_cb[4].bind(); }
	auto in_port6_cb() { return m_input_cb[5].bind(); }
	auto in_port7_cb() { return m_input_cb[6].bind(); }
	auto in_port8_cb() { return m_input_cb[7].bind(); }
	auto in_port9_cb() { return m_input_cb[8].bind(); }
	auto out_port1_cb() { return m_output_cb[0].bind(); }
	auto out_port2_cb() { return m_output_cb[1].bind(); }
	auto out_port3_cb() { return m_output_cb[2].bind(); }
	auto out_port4_cb() { return m_output_cb[3].bind(); }
	auto out_port5_cb() { return m_output_cb[4].bind(); }
	auto out_port6_cb() { return m_output_cb[5].bind(); }
	auto out_port7_cb() { return m_output_cb[6].bind(); }
	auto out_port8_cb() { return m_output_cb[7].bind(); }
	auto out_port9_cb() { return m_output_cb[8].bind(); }
	auto ios_cb() { return m_ios_cb.bind(); }

	// bus-compatible interface
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	te7750_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal helpers
	void set_port_dir(int port, u8 dir);
	void set_ios();

	// input/output callbacks
	devcb_read8::array<9> m_input_cb;
	devcb_write8::array<9> m_output_cb;

	// mode callback
	devcb_read8         m_ios_cb;

	// internal state
	u8                  m_data_latch[9];
	u8                  m_data_dir[9];
};

// ======================> te7751_device

class te7751_device : public te7750_device
{
public:
	te7751_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

// ======================> te7752_device

class te7752_device : public te7750_device
{
public:
	te7752_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

// device type definitions
DECLARE_DEVICE_TYPE(TE7750, te7750_device)
DECLARE_DEVICE_TYPE(TE7751, te7751_device)
DECLARE_DEVICE_TYPE(TE7752, te7752_device)

#endif // MAME_MACHINE_TE7750_H
