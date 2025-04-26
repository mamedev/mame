// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_SETA_IOX_HLE_H
#define MAME_SETA_IOX_HLE_H

#pragma once

class iox_hle_device : public device_t
{
public:
	iox_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned Which> auto p1_key_input_cb() { return m_p1_key[Which].bind(); }
	template <unsigned Which> auto p2_key_input_cb() { return m_p2_key[Which].bind(); }
	auto p1_direct_input_cb() { return m_direct_in[0].bind(); }
	auto p2_direct_input_cb() { return m_direct_in[1].bind(); }

	u8 data_r(offs_t offset);
	void data_w(offs_t offset, u8 data);
	u8 status_r(offs_t offset);
	void command_w(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_read8 m_p1_key[4];
	devcb_read8 m_p2_key[4];
	devcb_read8 m_direct_in[2];

	u8 get_key_matrix_value(devcb_read8 *key_read, bool is_p2);

	u8 m_data, m_command;
	bool m_direct_mode;
};

DECLARE_DEVICE_TYPE(IOX_HLE, iox_hle_device)

#endif // MAME_SETA_IOX_HLE_H
