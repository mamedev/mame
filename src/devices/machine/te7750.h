// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    TE7750 Super I/O Expander

**********************************************************************/

#ifndef MAME_MACHINE_TE7750_H
#define MAME_MACHINE_TE7750_H

#pragma once

//**************************************************************************
//  CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TE7750_IN_PORT1_CB(_devcb) \
	downcast<te7750_device &>(*device).set_input_cb(0, DEVCB_##_devcb);
#define MCFG_TE7750_IN_PORT2_CB(_devcb) \
	downcast<te7750_device &>(*device).set_input_cb(1, DEVCB_##_devcb);
#define MCFG_TE7750_IN_PORT3_CB(_devcb) \
	downcast<te7750_device &>(*device).set_input_cb(2, DEVCB_##_devcb);
#define MCFG_TE7750_IN_PORT4_CB(_devcb) \
	downcast<te7750_device &>(*device).set_input_cb(3, DEVCB_##_devcb);
#define MCFG_TE7750_IN_PORT5_CB(_devcb) \
	downcast<te7750_device &>(*device).set_input_cb(4, DEVCB_##_devcb);
#define MCFG_TE7750_IN_PORT6_CB(_devcb) \
	downcast<te7750_device &>(*device).set_input_cb(5, DEVCB_##_devcb);
#define MCFG_TE7750_IN_PORT7_CB(_devcb) \
	downcast<te7750_device &>(*device).set_input_cb(6, DEVCB_##_devcb);
#define MCFG_TE7750_IN_PORT8_CB(_devcb) \
	downcast<te7750_device &>(*device).set_input_cb(7, DEVCB_##_devcb);
#define MCFG_TE7750_IN_PORT9_CB(_devcb) \
	downcast<te7750_device &>(*device).set_input_cb(8, DEVCB_##_devcb);

#define MCFG_TE7750_OUT_PORT1_CB(_devcb) \
	downcast<te7750_device &>(*device).set_output_cb(0, DEVCB_##_devcb);
#define MCFG_TE7750_OUT_PORT2_CB(_devcb) \
	downcast<te7750_device &>(*device).set_output_cb(1, DEVCB_##_devcb);
#define MCFG_TE7750_OUT_PORT3_CB(_devcb) \
	downcast<te7750_device &>(*device).set_output_cb(2, DEVCB_##_devcb);
#define MCFG_TE7750_OUT_PORT4_CB(_devcb) \
	downcast<te7750_device &>(*device).set_output_cb(3, DEVCB_##_devcb);
#define MCFG_TE7750_OUT_PORT5_CB(_devcb) \
	downcast<te7750_device &>(*device).set_output_cb(4, DEVCB_##_devcb);
#define MCFG_TE7750_OUT_PORT6_CB(_devcb) \
	downcast<te7750_device &>(*device).set_output_cb(5, DEVCB_##_devcb);
#define MCFG_TE7750_OUT_PORT7_CB(_devcb) \
	downcast<te7750_device &>(*device).set_output_cb(6, DEVCB_##_devcb);
#define MCFG_TE7750_OUT_PORT8_CB(_devcb) \
	downcast<te7750_device &>(*device).set_output_cb(7, DEVCB_##_devcb);
#define MCFG_TE7750_OUT_PORT9_CB(_devcb) \
	downcast<te7750_device &>(*device).set_output_cb(8, DEVCB_##_devcb);

#define MCFG_TE7750_IOS_CB(_devcb) \
	downcast<te7750_device &>(*device).set_ios_cb(DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> te7750_device

class te7750_device : public device_t
{
public:
	// construction/destruction
	te7750_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template<class Object> devcb_base &set_input_cb(int port, Object &&obj)
	{
		assert(port >= 0 && port < 9);
		return m_input_cb[port].set_callback(std::forward<Object>(obj));
	}
	template<class Object> devcb_base &set_output_cb(int port, Object &&obj)
	{
		assert(port >= 0 && port < 9);
		return m_output_cb[port].set_callback(std::forward<Object>(obj));
	}
	template<class Object> devcb_base &set_ios_cb(Object &&obj)
	{
		return m_ios_cb.set_callback(std::forward<Object>(obj));
	}
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
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal helpers
	void set_port_dir(int port, u8 dir);
	void set_ios();

	// input/output callbacks
	devcb_read8         m_input_cb[9];
	devcb_write8        m_output_cb[9];

	// mode callback
	devcb_read8         m_ios_cb;

	// internal state
	u8                  m_data_latch[9];
	u8                  m_data_dir[9];
};

// device type definition
DECLARE_DEVICE_TYPE(TE7750, te7750_device)

#endif // MAME_MACHINE_TE7750_H
