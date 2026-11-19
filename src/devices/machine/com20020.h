// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    SMSC COM20020 Universal Local Area Network Controller (ARCNET)

***************************************************************************/

#ifndef MAME_MACHINE_COM20020_H
#define MAME_MACHINE_COM20020_H

#pragma once

class com20020_device : public device_t
{
public:
	static constexpr feature_type unemulated_features() { return feature::LAN; }

	com20020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto int_callback() { return m_int_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr offs_t RAM_SIZE = 0x800;

	void software_reset();
	void wake_core();
	void command_w(u8 data);
	void update_int();

	devcb_write_line m_int_cb;
	memory_share_creator<u8> m_ram;

	u8 m_status;
	u8 m_imr;
	u8 m_config;
	u8 m_subadr;
	u8 m_tentid;
	u8 m_nodeid;
	u8 m_setup1;
	u8 m_setup2;
	u8 m_ptr_high;
	u16 m_address;
	bool m_int;
};

DECLARE_DEVICE_TYPE(COM20020, com20020_device)

#endif // MAME_MACHINE_COM20020_H
