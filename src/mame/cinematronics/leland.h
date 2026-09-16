// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Cinematronics / Leland Cinemat System driver

*************************************************************************/
#ifndef MAME_CINEMATRONICS_LELAND_H
#define MAME_CINEMATRONICS_LELAND_H

#pragma once

#include "machine/eepromser.h"
#include "sound/dac.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class leland_80186_sound_device;

class leland_state : public driver_device
{
public:
	leland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_master(*this, "master")
		, m_slave(*this, "slave")
		, m_mainram(*this, "mainram")
		, m_master_bankslot(*this, "masterbank")
		, m_master_base(*this, "master")
		, m_slave_bankslot(*this, "slavebank")
		, m_slave_base(*this, "slave")
		, m_eeprom(*this, "eeprom")
		, m_battery_ram(*this, "battery")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette_view(*this, "palette_view")
		, m_battery_ram_view(*this, "battery_ram_view")
		, m_io_in(*this, "IN%u", 0U)
		, m_io_an(*this, "AN%u", 0U)
		, m_dac(*this, "dac%u", 0U)
		, m_ay8910(*this, "ay8910")
		, m_ay8912(*this, "ay8912")
		, m_bg_gfxrom(*this, "bg_gfx")
		, m_bg_prom(*this, "bg_prom")
	{ }

	void leland(machine_config &config) ATTR_COLD;

	void init_dblplay() ATTR_COLD;
	void init_dangerz() ATTR_COLD;
	void init_mayhem() ATTR_COLD;
	void init_alleymas() ATTR_COLD;
	void init_strkzone() ATTR_COLD;
	void init_wseries() ATTR_COLD;
	void init_powrplay() ATTR_COLD;
	void init_basebal2() ATTR_COLD;
	void init_upyoural() ATTR_COLD;
	void init_cerberus() ATTR_COLD;

protected:
	static constexpr u8 SERIAL_TYPE_NONE        = 0;
	static constexpr u8 SERIAL_TYPE_ADD         = 1;
	static constexpr u8 SERIAL_TYPE_ADD_XOR     = 2;
	static constexpr u8 SERIAL_TYPE_ENCRYPT     = 3;
	static constexpr u8 SERIAL_TYPE_ENCRYPT_XOR = 4;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void rotate_memory(const char *cpuname) ATTR_COLD;

	void cerberus_bankswitch();
	void mayhem_bankswitch();
	void dangerz_bankswitch();
	void basebal2_bankswitch();

	void redline_bankswitch();
	void viper_bankswitch();
	void offroad_bankswitch();

	int dial_compute_value(int new_val, int indx);
	void init_master_ports(u8 mvram_base, u8 io_base);

	void update_battery_ram_view(bool enable);

	void init_vram();

	int vram_port_r(offs_t offset, int num);
	void vram_port_w(offs_t offset, u8 data, int num);

	u8 master_analog_key_r(offs_t offset);
	void master_analog_key_w(offs_t offset, u8 data);
	u8 dangerz_input_y_r();
	u8 dangerz_input_x_r();
	u8 dangerz_input_upper_r();
	void scroll_w(offs_t offset, u8 data);
	void leland_master_alt_bankswitch_w(u8 data);

	TIMER_CALLBACK_MEMBER(leland_delayed_mvram_w);

	u8 raster_r();
	void slave_video_addr_w(offs_t offset, u8 data);
	void slave_large_banksw_w(u8 data);
	void master_video_addr_w(offs_t offset, u8 data);

	void leland_video(machine_config &config) ATTR_COLD;
	void master_common_map_program(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	required_shared_ptr<u8> m_mainram;
	required_memory_bank m_master_bankslot;
	required_region_ptr<u8> m_master_base;
	required_memory_bank m_slave_bankslot;
	required_region_ptr<u8> m_slave_base;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_shared_ptr<u8> m_battery_ram;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	memory_view m_palette_view;
	memory_view m_battery_ram_view;

	optional_ioport_array<4> m_io_in;
	optional_ioport_array<6> m_io_an;

	void (leland_state::*m_update_master_bank)();

	emu_timer *m_master_int_timer = nullptr;

	u8 m_dial_last_input[4]{};
	u8 m_dial_last_result[4]{};
	u8 m_analog_result = 0U;

	s32 m_dangerz_x = 0;
	s32 m_dangerz_y = 0;

	tilemap_t *m_tilemap = nullptr;
	u16 m_xscroll = 0U;
	u16 m_yscroll = 0U;

	std::unique_ptr<u8[]> m_video_ram;

	struct vram_state_data
	{
		u8   m_buffer = 0U; // MSB of address
		u16  m_addr = 0U;
		u8   m_latch[2]{};
	};

	vram_state_data m_vram_state[2];

private:
	u8 cerberus_dial_1_r();
	u8 cerberus_dial_2_r();
	void alleymas_joystick_kludge_w(u8 data);
	u8 leland_master_input_r(offs_t offset);
	void leland_master_output_w(offs_t offset, u8 data);
	void slave_small_banksw_w(u8 data);
	void leland_mvram_port_w(offs_t offset, u8 data);
	u8 leland_mvram_port_r(offs_t offset);
	void leland_svram_port_w(offs_t offset, u8 data);
	u8 leland_svram_port_r(offs_t offset);
	u8 sound_port_r();
	void sound_port_w(u8 data);
	void gfx_port_w(u8 data);

	TILEMAP_MAPPER_MEMBER(leland_scan);
	TILE_GET_INFO_MEMBER(leland_get_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(leland_master_interrupt);
	TIMER_CALLBACK_MEMBER(leland_interrupt_callback);
	TIMER_CALLBACK_MEMBER(scanline_callback);

	void video_addr_w(offs_t offset, u8 data, int num);

	void update_dangerz_xy();

	void leland_init_eeprom(u8 default_val, const u16 *data, u8 serial_offset, u8 serial_type);
	void ataxx_init_eeprom(const u16 *data);
	u8 keycard_r();
	void keycard_w(u8 data);
	void master_map_io(address_map &map) ATTR_COLD;
	void master_map_program(address_map &map) ATTR_COLD;
	void slave_map_io(address_map &map) ATTR_COLD;
	void slave_small_map_program(address_map &map) ATTR_COLD;

	optional_device_array<dac_byte_interface, 2> m_dac;
	optional_device<ay8910_device> m_ay8910;
	optional_device<ay8912_device> m_ay8912;

	required_region_ptr<u8> m_bg_gfxrom;
	optional_region_ptr<u8> m_bg_prom;

	u8 m_dac_control = 0U;
	u8 *m_alleymas_kludge_mem = nullptr;
	u8 m_gfx_control = 0U;
	u8 m_keycard_shift = 0U;
	u8 m_keycard_bit = 0U;
	u8 m_keycard_state = 0U;
	u8 m_keycard_clock = 0U;
	u8 m_keycard_command[3]{};
	u8 m_top_board_bank = 0U;
	u8 m_sound_port_bank = 0U;
	u8 m_alternate_bank = 0U;
	u8 m_gfxbank = 0U;
	emu_timer *m_scanline_timer = nullptr;
};


class redline_state : public leland_state
{
public:
	redline_state(const machine_config &mconfig, device_type type, const char *tag)
		: leland_state(mconfig, type, tag)
		, m_sound(*this, "custom")
	{
	}

	void init_redlin2p() ATTR_COLD;
	void init_quarterb() ATTR_COLD;
	void init_viper() ATTR_COLD;
	void init_teamqb() ATTR_COLD;
	void init_aafb() ATTR_COLD;
	void init_aafbb() ATTR_COLD;
	void init_aafbd2p() ATTR_COLD;
	void init_offroad() ATTR_COLD;
	void init_offroadt() ATTR_COLD;
	void init_pigout() ATTR_COLD;

	void redline(machine_config &config) ATTR_COLD;
	void quarterb(machine_config &config) ATTR_COLD;
	void lelandi(machine_config &config) ATTR_COLD;

private:
	u8 redline_pedal_1_r();
	u8 redline_pedal_2_r();
	u8 redline_wheel_1_r();
	u8 redline_wheel_2_r();
	u8 offroad_wheel_1_r();
	u8 offroad_wheel_2_r();
	u8 offroad_wheel_3_r();
	void redline_master_alt_bankswitch_w(u8 data);

	void master_redline_map_program(address_map &map) ATTR_COLD;
	void master_redline_map_io(address_map &map) ATTR_COLD;
	void slave_large_map_program(address_map &map) ATTR_COLD;

	required_device<leland_80186_sound_device> m_sound;
};


class ataxx_state : public leland_state
{
public:
	ataxx_state(const machine_config &mconfig, device_type type, const char *tag)
		: leland_state(mconfig, type, tag)
		, m_sound(*this, "custom")
		, m_xrom_base(*this, "xrom")
		, m_qram(*this, "qram", 0x10000, ENDIANNESS_LITTLE)
		, m_tram(*this, "tram")
	{
	}

	void init_ataxx() ATTR_COLD;
	void init_wsf() ATTR_COLD;
	void init_indyheat() ATTR_COLD;
	void init_brutforc() ATTR_COLD;
	void init_asylum() ATTR_COLD;

	void ataxx(machine_config &config) ATTR_COLD;
	void wsf(machine_config &config) ATTR_COLD;
	void asylum(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	u8 ataxx_trackball_r(offs_t offset);
	u8 indyheat_analog_r(offs_t offset);
	void indyheat_analog_w(offs_t offset, u8 data);

	u8 qram_r(offs_t offset);
	void qram_w(offs_t offset, u8 data);

	u8 ataxx_master_input_r(offs_t offset);
	void ataxx_master_output_w(offs_t offset, u8 data);
	void xrom_w(offs_t offset, u8 data);
	u8 xrom_r(offs_t offset);
	void ataxx_slave_banksw_w(u8 data);
	void ataxx_mvram_port_w(offs_t offset, u8 data);
	void ataxx_svram_port_w(offs_t offset, u8 data);
	u8 ataxx_mvram_port_r(offs_t offset);
	u8 ataxx_svram_port_r(offs_t offset);
	u8 eeprom_r();
	void eeprom_w(u8 data);

	TILEMAP_MAPPER_MEMBER(ataxx_scan);
	TILE_GET_INFO_MEMBER(ataxx_get_tile_info);

	TIMER_CALLBACK_MEMBER(ataxx_interrupt_callback);
	void ataxx_bankswitch();

	void ataxx_video(machine_config &config);

	void asylum_slave_map_program(address_map &map) ATTR_COLD;
	void master_map_io_2(address_map &map) ATTR_COLD;
	void master_map_program_2(address_map &map) ATTR_COLD;
	void slave_map_io_2(address_map &map) ATTR_COLD;
	void slave_map_program(address_map &map) ATTR_COLD;

	required_device<leland_80186_sound_device> m_sound;

	required_region_ptr<u16> m_xrom_base;
	memory_share_creator<u8> m_qram;
	required_shared_ptr<u8> m_tram;

	u8 m_master_bank = 0U;
	u32 m_xrom_addr[2]{};
};

#endif // MAME_CINEMATRONICS_LELAND_H
