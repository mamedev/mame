// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_EVO_TSCONF_COPPER_H
#define MAME_SINCLAIR_EVO_TSCONF_COPPER_H

#pragma once

class tsconf_copper_device : public device_t, public device_memory_interface, public device_disasm_interface, public device_state_interface
{

public:
	tsconf_copper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto out_wreg_cb() { return m_out_wreg_cb.bind(); }
	auto out_int_cb() { return m_out_int_cb.bind(); }
	auto out_rdy_cb() { return m_out_rdy_cb.bind(); }
	template <typename... T> void set_in_until_pos_cb(T &&... args) { return m_in_until_pos_cb.set(std::forward<T>(args)...); }

	void copper_en_w(u8 data);
	void cl_data_w(u16 addr, u8 data);
	void dma_ready_w(int status);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	TIMER_CALLBACK_MEMBER(timer_callback);

private:
	void copper_map(address_map &map) ATTR_COLD;

	address_space_config m_space_config;
	memory_share_creator<u16> m_cl_data;

	devcb_write8 m_out_wreg_cb;
	devcb_write_line m_out_int_cb;
	devcb_write_line m_out_rdy_cb;
	device_delegate<attotime (u16)> m_in_until_pos_cb;

	emu_timer *m_timer;

	bool m_en;
	u8 m_pc;
	u8 m_ret_pc;
	u8 m_a;
	u16 m_b; // u9
};


DECLARE_DEVICE_TYPE(TSCONF_COPPER, tsconf_copper_device)

#endif // MAME_SINCLAIR_EVO_TSCONF_COPPER_H
