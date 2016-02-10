// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    TTL74145

    BCD-to-Decimal decoder

***************************************************************************/

#ifndef __TTL74145_H__
#define __TTL74145_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************
#define MCFG_TTL74145_OUTPUT_LINE_0_CB(_devcb) \
	devcb = &ttl74145_device::set_output_line_0_callback(*device, DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_1_CB(_devcb) \
	devcb = &ttl74145_device::set_output_line_1_callback(*device, DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_2_CB(_devcb) \
	devcb = &ttl74145_device::set_output_line_2_callback(*device, DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_3_CB(_devcb) \
	devcb = &ttl74145_device::set_output_line_3_callback(*device, DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_4_CB(_devcb) \
	devcb = &ttl74145_device::set_output_line_4_callback(*device, DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_5_CB(_devcb) \
	devcb = &ttl74145_device::set_output_line_5_callback(*device, DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_6_CB(_devcb) \
	devcb = &ttl74145_device::set_output_line_6_callback(*device, DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_7_CB(_devcb) \
	devcb = &ttl74145_device::set_output_line_7_callback(*device, DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_8_CB(_devcb) \
	devcb = &ttl74145_device::set_output_line_8_callback(*device, DEVCB_##_devcb);

#define MCFG_TTL74145_OUTPUT_LINE_9_CB(_devcb) \
	devcb = &ttl74145_device::set_output_line_9_callback(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ttl74145_device

class ttl74145_device :  public device_t
{
public:
	// construction/destruction
	ttl74145_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_output_line_0_callback(device_t &device, _Object object) { return downcast<ttl74145_device &>(device).m_output_line_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_output_line_1_callback(device_t &device, _Object object) { return downcast<ttl74145_device &>(device).m_output_line_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_output_line_2_callback(device_t &device, _Object object) { return downcast<ttl74145_device &>(device).m_output_line_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_output_line_3_callback(device_t &device, _Object object) { return downcast<ttl74145_device &>(device).m_output_line_3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_output_line_4_callback(device_t &device, _Object object) { return downcast<ttl74145_device &>(device).m_output_line_4_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_output_line_5_callback(device_t &device, _Object object) { return downcast<ttl74145_device &>(device).m_output_line_5_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_output_line_6_callback(device_t &device, _Object object) { return downcast<ttl74145_device &>(device).m_output_line_6_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_output_line_7_callback(device_t &device, _Object object) { return downcast<ttl74145_device &>(device).m_output_line_7_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_output_line_8_callback(device_t &device, _Object object) { return downcast<ttl74145_device &>(device).m_output_line_8_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_output_line_9_callback(device_t &device, _Object object) { return downcast<ttl74145_device &>(device).m_output_line_9_cb.set_callback(object); }

	UINT16 read();
	void write(UINT8 data);
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
	UINT16 m_number;
};

// device type definition
extern const device_type TTL74145;

#endif /* TTL74145 */
