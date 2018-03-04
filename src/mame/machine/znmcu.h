// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_ZNMCU_H
#define MAME_MACHINE_ZNMCU_H

#pragma once


DECLARE_DEVICE_TYPE(ZNMCU, znmcu_device)

#define MCFG_ZNMCU_DATAOUT_HANDLER(_devcb) \
	devcb = &downcast<znmcu_device &>(*device).set_dataout_handler(DEVCB_##_devcb);

#define MCFG_ZNMCU_DSR_HANDLER(_devcb) \
	devcb = &downcast<znmcu_device &>(*device).set_dsr_handler(DEVCB_##_devcb);

#define MCFG_ZNMCU_DSW_HANDLER(_devcb) \
	devcb = &downcast<znmcu_device &>(*device).set_dsw_handler(DEVCB_##_devcb);

#define MCFG_ZNMCU_ANALOG1_HANDLER(_devcb) \
	devcb = &downcast<znmcu_device &>(*device).set_analog1_handler(DEVCB_##_devcb);

#define MCFG_ZNMCU_ANALOG2_HANDLER(_devcb) \
	devcb = &downcast<znmcu_device &>(*device).set_analog2_handler(DEVCB_##_devcb);

class znmcu_device : public device_t
{
public:
	znmcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <class Object> devcb_base &set_dsw_handler(Object &&cb) { return m_dsw_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_analog1_handler(Object &&cb) { return m_analog1_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_analog2_handler(Object &&cb) { return m_analog2_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dataout_handler(Object &&cb) { return m_dataout_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dsr_handler(Object &&cb) { return m_dsr_handler.set_callback(std::forward<Object>(cb)); }

	WRITE_LINE_MEMBER(write_select);
	WRITE_LINE_MEMBER(write_clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	devcb_read8 m_dsw_handler;
	devcb_read8 m_analog1_handler;
	devcb_read8 m_analog2_handler;
	devcb_write_line m_dataout_handler;
	devcb_write_line m_dsr_handler;

	static const int MaxBytes = 3;
	int m_select;
	int m_clk;
	int m_bit;
	int m_byte;
	int m_databytes;
	uint8_t m_send[MaxBytes];
	emu_timer *m_mcu_timer;
};

#endif // MAME_MACHINE_ZNMCU_H
