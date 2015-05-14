// license:BSD-3-Clause
// copyright-holders:smf
#ifndef __LATCH_H__
#define __LATCH_H__

#define MCFG_OUTPUT_LATCH_BIT0_HANDLER(_devcb) \
	devcb = &output_latch_device::set_bit0_handler(*device, DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT1_HANDLER(_devcb) \
	devcb = &output_latch_device::set_bit1_handler(*device, DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT2_HANDLER(_devcb) \
	devcb = &output_latch_device::set_bit2_handler(*device, DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT3_HANDLER(_devcb) \
	devcb = &output_latch_device::set_bit3_handler(*device, DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT4_HANDLER(_devcb) \
	devcb = &output_latch_device::set_bit4_handler(*device, DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT5_HANDLER(_devcb) \
	devcb = &output_latch_device::set_bit5_handler(*device, DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT6_HANDLER(_devcb) \
	devcb = &output_latch_device::set_bit6_handler(*device, DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT7_HANDLER(_devcb) \
	devcb = &output_latch_device::set_bit7_handler(*device, DEVCB_##_devcb);

class output_latch_device : public device_t
{
public:
	output_latch_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_bit0_handler(device_t &device, _Object object) { return downcast<output_latch_device &>(device).m_bit0_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_bit1_handler(device_t &device, _Object object) { return downcast<output_latch_device &>(device).m_bit1_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_bit2_handler(device_t &device, _Object object) { return downcast<output_latch_device &>(device).m_bit2_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_bit3_handler(device_t &device, _Object object) { return downcast<output_latch_device &>(device).m_bit3_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_bit4_handler(device_t &device, _Object object) { return downcast<output_latch_device &>(device).m_bit4_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_bit5_handler(device_t &device, _Object object) { return downcast<output_latch_device &>(device).m_bit5_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_bit6_handler(device_t &device, _Object object) { return downcast<output_latch_device &>(device).m_bit6_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_bit7_handler(device_t &device, _Object object) { return downcast<output_latch_device &>(device).m_bit7_handler.set_callback(object); }

	void write(UINT8 data);
	DECLARE_WRITE8_MEMBER(write) { write(data); }

protected:
	virtual void device_start();

private:
	bool m_resolved;

	int m_bit0;
	int m_bit1;
	int m_bit2;
	int m_bit3;
	int m_bit4;
	int m_bit5;
	int m_bit6;
	int m_bit7;

	devcb_write_line m_bit0_handler;
	devcb_write_line m_bit1_handler;
	devcb_write_line m_bit2_handler;
	devcb_write_line m_bit3_handler;
	devcb_write_line m_bit4_handler;
	devcb_write_line m_bit5_handler;
	devcb_write_line m_bit6_handler;
	devcb_write_line m_bit7_handler;
};

extern const device_type OUTPUT_LATCH;

#endif
