// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Cinemat/Leland driver

*************************************************************************/
#ifndef MAME_INCLUDES_LELAND_H
#define MAME_INCLUDES_LELAND_H

#pragma once

#include "machine/eepromser.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "screen.h"

#define LELAND_BATTERY_RAM_SIZE 0x4000
#define ATAXX_EXTRA_TRAM_SIZE 0x800


struct vram_state_data
{
	uint16_t  m_addr;
	uint8_t   m_latch[2];
};

class leland_80186_sound_device;

class leland_state : public driver_device
{
public:
	leland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_master(*this, "master")
		, m_slave(*this, "slave")
		, m_mainram(*this, "mainram")
		, m_battery_ram(*this, "battery")
		, m_eeprom(*this, "eeprom")
		, m_sound(*this, "custom")
		, m_dac(*this, "dac%u", 0U)
		, m_ay8910(*this, "ay8910")
		, m_ay8912(*this, "ay8912")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_master_base(*this, "master")
		, m_slave_base(*this, "slave")
		, m_bg_gfxrom(*this, "bg_gfx")
		, m_bg_prom(*this, "bg_prom")
		, m_master_bankslot(*this, "masterbank_%u", 0U)
		, m_slave_bankslot(*this, "slavebank")
	{ }

	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_battery_ram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<leland_80186_sound_device> m_sound;
	optional_device_array<dac_byte_interface, 2> m_dac;
	optional_device<ay8910_device> m_ay8910;
	optional_device<ay8912_device> m_ay8912;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_region_ptr<uint8_t> m_master_base;
	required_region_ptr<uint8_t> m_slave_base;
	required_region_ptr<uint8_t> m_bg_gfxrom;
	optional_region_ptr<uint8_t> m_bg_prom;

	required_memory_bank_array<2> m_master_bankslot;
	required_memory_bank m_slave_bankslot;

	uint8_t m_dac_control;
	uint8_t *m_alleymas_kludge_mem;
	uint8_t m_gfx_control;
	uint8_t m_wcol_enable;
	emu_timer *m_master_int_timer;
	int m_dangerz_x;
	int m_dangerz_y;
	uint8_t m_analog_result;
	uint8_t m_dial_last_input[4];
	uint8_t m_dial_last_result[4];
	uint8_t m_keycard_shift;
	uint8_t m_keycard_bit;
	uint8_t m_keycard_state;
	uint8_t m_keycard_clock;
	uint8_t m_keycard_command[3];
	uint8_t m_top_board_bank;
	uint8_t m_sound_port_bank;
	uint8_t m_alternate_bank;
	void (leland_state::*m_update_master_bank)();
	uint8_t m_battery_ram_enable;
	std::unique_ptr<uint8_t[]> m_video_ram;
	struct vram_state_data m_vram_state[2];
	uint16_t m_xscroll;
	uint16_t m_yscroll;
	uint8_t m_gfxbank;
	uint16_t m_last_scanline;
	emu_timer *m_scanline_timer;

	DECLARE_READ8_MEMBER(cerberus_dial_1_r);
	DECLARE_READ8_MEMBER(cerberus_dial_2_r);
	DECLARE_WRITE8_MEMBER(alleymas_joystick_kludge);
	DECLARE_READ8_MEMBER(dangerz_input_y_r);
	DECLARE_READ8_MEMBER(dangerz_input_x_r);
	DECLARE_READ8_MEMBER(dangerz_input_upper_r);
	DECLARE_READ8_MEMBER(redline_pedal_1_r);
	DECLARE_READ8_MEMBER(redline_pedal_2_r);
	DECLARE_READ8_MEMBER(redline_wheel_1_r);
	DECLARE_READ8_MEMBER(redline_wheel_2_r);
	DECLARE_READ8_MEMBER(offroad_wheel_1_r);
	DECLARE_READ8_MEMBER(offroad_wheel_2_r);
	DECLARE_READ8_MEMBER(offroad_wheel_3_r);
	DECLARE_WRITE8_MEMBER(leland_master_alt_bankswitch_w);
	DECLARE_WRITE8_MEMBER(leland_battery_ram_w);
	DECLARE_READ8_MEMBER(leland_master_analog_key_r);
	DECLARE_WRITE8_MEMBER(leland_master_analog_key_w);
	DECLARE_READ8_MEMBER(leland_master_input_r);
	DECLARE_WRITE8_MEMBER(leland_master_output_w);
	DECLARE_WRITE8_MEMBER(leland_gated_paletteram_w);
	DECLARE_READ8_MEMBER(leland_gated_paletteram_r);
	DECLARE_WRITE8_MEMBER(leland_slave_small_banksw_w);
	DECLARE_WRITE8_MEMBER(leland_slave_large_banksw_w);
	DECLARE_WRITE8_MEMBER(ataxx_slave_banksw_w);
	DECLARE_READ8_MEMBER(leland_raster_r);
	DECLARE_WRITE8_MEMBER(leland_scroll_w);
	DECLARE_WRITE8_MEMBER(leland_master_video_addr_w);
	DECLARE_WRITE8_MEMBER(leland_mvram_port_w);
	DECLARE_READ8_MEMBER(leland_mvram_port_r);
	DECLARE_WRITE8_MEMBER(leland_slave_video_addr_w);
	DECLARE_WRITE8_MEMBER(leland_svram_port_w);
	DECLARE_READ8_MEMBER(leland_svram_port_r);
	DECLARE_READ8_MEMBER(leland_sound_port_r);
	DECLARE_WRITE8_MEMBER(leland_sound_port_w);
	DECLARE_WRITE8_MEMBER(leland_gfx_port_w);

	void init_dblplay();
	void init_viper();
	void init_quarterb();
	void init_aafb();
	void init_redlin2p();
	void init_aafbb();
	void init_dangerz();
	void init_mayhem();
	void init_offroad();
	void init_pigout();
	void init_alleymas();
	void init_offroadt();
	void init_teamqb();
	void init_strkzone();
	void init_wseries();
	void init_powrplay();
	void init_basebal2();
	void init_upyoural();
	void init_cerberus();
	void init_aafbd2p();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	tilemap_t      *m_tilemap;

	TILEMAP_MAPPER_MEMBER(leland_scan);
	TILE_GET_INFO_MEMBER(leland_get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(leland_master_interrupt);
	TIMER_CALLBACK_MEMBER(leland_interrupt_callback);
	TIMER_CALLBACK_MEMBER(ataxx_interrupt_callback);
	TIMER_CALLBACK_MEMBER(scanline_callback);
	TIMER_CALLBACK_MEMBER(leland_delayed_mvram_w);

	void leland_video_addr_w(address_space &space, int offset, int data, int num);
	int leland_vram_port_r(address_space &space, int offset, int num);
	void leland_vram_port_w(address_space &space, int offset, int data, int num);
	int dial_compute_value(int new_val, int indx);

	void update_dangerz_xy();
	void cerberus_bankswitch();
	void mayhem_bankswitch();
	void dangerz_bankswitch();
	void basebal2_bankswitch();
	void redline_bankswitch();
	void viper_bankswitch();
	void offroad_bankswitch();
	void leland_init_eeprom(uint8_t default_val, const uint16_t *data, uint8_t serial_offset, uint8_t serial_type);
	void ataxx_init_eeprom(const uint16_t *data);
	int keycard_r();
	void keycard_w(int data);
	void leland_rotate_memory(const char *cpuname);
	void init_master_ports(uint8_t mvram_base, uint8_t io_base);
	void redline(machine_config &config);
	void lelandi(machine_config &config);
	void leland(machine_config &config);
	void quarterb(machine_config &config);
	void leland_video(machine_config &config);
	void master_map_io(address_map &map);
	void master_map_program(address_map &map);
	void master_redline_map_io(address_map &map);
	void slave_large_map_program(address_map &map);
	void slave_map_io(address_map &map);
	void slave_map_program(address_map &map);
	void slave_small_map_program(address_map &map);
};


class ataxx_state : public leland_state
{
public:
	ataxx_state(const machine_config &mconfig, device_type type, const char *tag)
		: leland_state(mconfig, type, tag)
		, m_xrom_base(*this, "xrom")
	{
	}

	void init_ataxx();
	void init_ataxxj();
	void init_wsf();
	void init_indyheat();
	void init_brutforc();
	void init_asylum();

	void ataxx(machine_config &config);
	void wsf(machine_config &config);

protected:
	DECLARE_READ8_MEMBER(ataxx_trackball_r);
	DECLARE_READ8_MEMBER(indyheat_wheel_r);
	DECLARE_READ8_MEMBER(indyheat_analog_r);
	DECLARE_WRITE8_MEMBER(indyheat_analog_w);

	DECLARE_WRITE8_MEMBER(ataxx_battery_ram_w);
	DECLARE_READ8_MEMBER(ataxx_master_input_r);
	DECLARE_WRITE8_MEMBER(ataxx_master_output_w);
	DECLARE_WRITE8_MEMBER(ataxx_paletteram_and_misc_w);
	DECLARE_READ8_MEMBER(ataxx_paletteram_and_misc_r);
	DECLARE_WRITE8_MEMBER(ataxx_mvram_port_w);
	DECLARE_WRITE8_MEMBER(ataxx_svram_port_w);
	DECLARE_READ8_MEMBER(ataxx_mvram_port_r);
	DECLARE_READ8_MEMBER(ataxx_svram_port_r);
	DECLARE_READ8_MEMBER(ataxx_eeprom_r);
	DECLARE_WRITE8_MEMBER(ataxx_eeprom_w);

	TILEMAP_MAPPER_MEMBER(ataxx_scan);
	TILE_GET_INFO_MEMBER(ataxx_get_tile_info);

	void ataxx_bankswitch();

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void ataxx_video(machine_config &config);

	void master_map_program_2(address_map &map);
	void master_map_io_2(address_map &map);
	void slave_map_io_2(address_map &map);

private:
	required_region_ptr<uint8_t> m_xrom_base;

	std::unique_ptr<uint8_t[]> m_ataxx_qram;
	uint8_t m_master_bank;
	uint32_t m_xrom1_addr;
	uint32_t m_xrom2_addr;
	std::unique_ptr<uint8_t[]> m_extra_tram;
};


#define SERIAL_TYPE_NONE        0
#define SERIAL_TYPE_ADD         1
#define SERIAL_TYPE_ADD_XOR     2
#define SERIAL_TYPE_ENCRYPT     3
#define SERIAL_TYPE_ENCRYPT_XOR 4

#endif // MAME_INCLUDES_LELAND_H
