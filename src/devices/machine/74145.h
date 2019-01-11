// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    TTL74145

    BCD-to-Decimal decoder

***************************************************************************/

#ifndef MAME_MACHINE_74145_H
#define MAME_MACHINE_74145_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************
#define MCFG_TTL74145_OUTPUT_LINE_0_CB(_devcb) \
	devcb = &downcast<ttl74145_device &>(*device).set_output_line_0_callback(DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_1_CB(_devcb) \
	devcb = &downcast<ttl74145_device &>(*device).set_output_line_1_callback(DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_2_CB(_devcb) \
	devcb = &downcast<ttl74145_device &>(*device).set_output_line_2_callback(DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_3_CB(_devcb) \
	devcb = &downcast<ttl74145_device &>(*device).set_output_line_3_callback(DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_4_CB(_devcb) \
	devcb = &downcast<ttl74145_device &>(*device).set_output_line_4_callback(DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_5_CB(_devcb) \
	devcb = &downcast<ttl74145_device &>(*device).set_output_line_5_callback(DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_6_CB(_devcb) \
	devcb = &downcast<ttl74145_device &>(*device).set_output_line_6_callback(DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_7_CB(_devcb) \
	devcb = &downcast<ttl74145_device &>(*device).set_output_line_7_callback(DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_8_CB(_devcb) \
	devcb = &downcast<ttl74145_device &>(*device).set_output_line_8_callback(DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_9_CB(_devcb) \
	devcb = &downcast<ttl74145_device &>(*device).set_output_line_9_callback(DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ttl74145_device

class ttl74145_device :  public device_t
{
public:
	// construction/destruction
	ttl74145_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_output_line_0_callback(Object &&cb) { return m_output_line_0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_output_line_1_callback(Object &&cb) { return m_output_line_1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_output_line_2_callback(Object &&cb) { return m_output_line_2_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_output_line_3_callback(Object &&cb) { return m_output_line_3_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_output_line_4_callback(Object &&cb) { return m_output_line_4_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_output_line_5_callback(Object &&cb) { return m_output_line_5_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_output_line_6_callback(Object &&cb) { return m_output_line_6_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_output_line_7_callback(Object &&cb) { return m_output_line_7_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_output_line_8_callback(Object &&cb) { return m_output_line_8_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_output_line_9_callback(Object &&cb) { return m_output_line_9_cb.set_callback(std::forward<Object>(cb)); }

	uint16_t read();
	void write(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line m_output_line_0_cb;
	devcb_write_line m_output_line_1_cb;
	devcb_write_line m_output_line_2_cb;
	devcb_write_line m_output_line_3_cb;
	devcb_write_line m_output_line_4_cb;
	devcb_write_line m_output_line_5_cb;
	devcb_write_line m_output_line_6_cb;
	devcb_write_line m_output_line_7_cb;
	devcb_write_line m_output_line_8_cb;
	devcb_write_line m_output_line_9_cb;

	/* decoded number */
	uint16_t m_number;
};

// device type definition
DECLARE_DEVICE_TYPE(TTL74145, ttl74145_device)

#endif // MAME_MACHINE_74145_H
