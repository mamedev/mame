// license: BSD-3-Clause
// copyright-holders: Angelo Salese, superctr, Nathan Misner

#ifndef MAME_BUS_MEGADRIVE_CART_SEGACH_H
#define MAME_BUS_MEGADRIVE_CART_SEGACH_H

#pragma once

#include "slot.h"

#include "imagedev/snapquik.h"
#include "machine/timer.h"

class megadrive_segach_jp_device : public device_t,
								   public device_megadrive_cart_interface
{
public:
	megadrive_segach_jp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	memory_bank_creator m_rom;
	memory_view m_ram_view;
	std::vector<u16> m_ram;
};


class megadrive_segach_us_device : public device_t,
								   public device_megadrive_cart_interface,
								   public device_memory_interface
{
public:
	megadrive_segach_us_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(menu_pressed);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	void tcu_map(address_map &map);

	void load_packets();
	TIMER_DEVICE_CALLBACK_MEMBER(send_packets);

	void sram_enable_w(offs_t offset, u16 data, u16 mem_mask);
	u16 sram_r(offs_t offset);
	void sram_w(offs_t offset, u16 data, u16 mem_mask);

	void crc_write(u16 data);
	u32 crc_read() const;

private:
	struct packet
	{
		u16 file_id;
		u16 service_id;
		u16 game_time;
		u16 address;
		std::array<u16, 246 / 2> data;
	};

	required_device<timer_device> m_packet_timer;

	memory_bank_creator m_rom;
	std::vector<u16> m_dram;
	std::vector<u8> m_sram;
	std::vector<std::array<packet, 10>> m_broadcast;
	std::array<u8, 0x200> m_nvm;

	address_space_config m_space_tcu_config;

	memory_view m_sram_view;
	memory_view m_game_view;
	memory_view m_game_sram_view;

	size_t m_broadcast_count;

	u16 m_game_id;
	u16 m_curr_packet, m_packet_match;

	u16 m_gen_control, m_gen_status;
	u32 m_crc;

	u16 m_tcu_index, m_tcu_dir;
};


DECLARE_DEVICE_TYPE(MEGADRIVE_SEGACH_JP, megadrive_segach_jp_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_SEGACH_US, megadrive_segach_us_device)


#endif // MAME_BUS_MEGADRIVE_CART_SEGACH_H
