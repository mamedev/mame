// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_TAITO_GUNBUSTR_LINK_H
#define MAME_TAITO_GUNBUSTR_LINK_H

#pragma once

#include <atomic>
#include <memory>


class gunbustr_link_device : public device_t
{
public:
	gunbustr_link_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
	~gunbustr_link_device();

	void map(address_map &map) ATTR_COLD;

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	class context;

	u8 ram_r(offs_t offset);
	void ram_w(offs_t offset, u8 data);
	template <unsigned Buffer> void handshake_w(u8 data);
	template <unsigned Buffer> u16 sense_r();
	template <unsigned Buffer> void sense_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void start_link() ATTR_COLD;
	void check_pending(unsigned buffer);

	std::unique_ptr<context> m_context;
	required_ioport m_config;
	memory_share_creator<u8> m_shared_ram;

	std::atomic<bool> m_pending[2];
};


DECLARE_DEVICE_TYPE(GUNBUSTR_LINK, gunbustr_link_device)

#endif // MAME_TAITO_GUNBUSTR_LINK_H
