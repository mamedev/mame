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
		: driver_device(mconfig, type, tag),
		m_master(*this, "master"),
		m_slave(*this, "slave"),
		m_mainram(*this, "mainram"),
		m_eeprom(*this, "eeprom"),
		m_sound(*this, "custom"),
		m_dac0(*this, "dac0"),
		m_dac1(*this, "dac1"),
		m_ay8910(*this, "ay8910"),
		m_ay8912(*this, "ay8912"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")  { }

	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	required_shared_ptr<uint8_t> m_mainram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<leland_80186_sound_device> m_sound;
	optional_device<dac_byte_interface> m_dac0;
	optional_device<dac_byte_interface> m_dac1;
	optional_device<ay8910_device> m_ay8910;
	optional_device<ay8912_device> m_ay8912;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint8_t m_dac_control;
	uint8_t *m_alleymas_kludge_mem;
	std::unique_ptr<uint8_t[]> m_ataxx_qram;
	uint8_t m_gfx_control;
	uint8_t m_wcol_enable;
	emu_timer *m_master_int_timer;
	uint8_t *m_master_base;
	uint8_t *m_slave_base;
	uint8_t *m_xrom_base;
	uint32_t m_master_length;
	uint32_t m_slave_length;
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
	uint8_t m_master_bank;
	void (leland_state::*m_update_master_bank)();
	uint32_t m_xrom1_addr;
	uint32_t m_xrom2_addr;
	uint8_t m_battery_ram_enable;
	uint8_t *m_battery_ram;
	std::unique_ptr<uint8_t[]> m_extra_tram;
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
	DECLARE_READ8_MEMBER(ataxx_trackball_r);
	DECLARE_READ8_MEMBER(indyheat_wheel_r);
	DECLARE_READ8_MEMBER(indyheat_analog_r);
	DECLARE_WRITE8_MEMBER(indyheat_analog_w);
	DECLARE_WRITE8_MEMBER(leland_master_alt_bankswitch_w);
	DECLARE_WRITE8_MEMBER(leland_battery_ram_w);
	DECLARE_WRITE8_MEMBER(ataxx_battery_ram_w);
	DECLARE_READ8_MEMBER(leland_master_analog_key_r);
	DECLARE_WRITE8_MEMBER(leland_master_analog_key_w);
	DECLARE_READ8_MEMBER(leland_master_input_r);
	DECLARE_WRITE8_MEMBER(leland_master_output_w);
	DECLARE_READ8_MEMBER(ataxx_master_input_r);
	DECLARE_WRITE8_MEMBER(ataxx_master_output_w);
	DECLARE_WRITE8_MEMBER(leland_gated_paletteram_w);
	DECLARE_READ8_MEMBER(leland_gated_paletteram_r);
	DECLARE_WRITE8_MEMBER(ataxx_paletteram_and_misc_w);
	DECLARE_READ8_MEMBER(ataxx_paletteram_and_misc_r);
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
	DECLARE_WRITE8_MEMBER(ataxx_mvram_port_w);
	DECLARE_WRITE8_MEMBER(ataxx_svram_port_w);
	DECLARE_READ8_MEMBER(ataxx_mvram_port_r);
	DECLARE_READ8_MEMBER(ataxx_svram_port_r);
	DECLARE_READ8_MEMBER(ataxx_eeprom_r);
	DECLARE_WRITE8_MEMBER(ataxx_eeprom_w);
	DECLARE_READ8_MEMBER(leland_sound_port_r);
	DECLARE_WRITE8_MEMBER(leland_sound_port_w);
	DECLARE_WRITE8_MEMBER(leland_gfx_port_w);

	DECLARE_DRIVER_INIT(dblplay);
	DECLARE_DRIVER_INIT(viper);
	DECLARE_DRIVER_INIT(quarterb);
	DECLARE_DRIVER_INIT(aafb);
	DECLARE_DRIVER_INIT(redlin2p);
	DECLARE_DRIVER_INIT(aafbb);
	DECLARE_DRIVER_INIT(dangerz);
	DECLARE_DRIVER_INIT(mayhem);
	DECLARE_DRIVER_INIT(offroad);
	DECLARE_DRIVER_INIT(pigout);
	DECLARE_DRIVER_INIT(alleymas);
	DECLARE_DRIVER_INIT(offroadt);
	DECLARE_DRIVER_INIT(teamqb);
	DECLARE_DRIVER_INIT(strkzone);
	DECLARE_DRIVER_INIT(wseries);
	DECLARE_DRIVER_INIT(powrplay);
	DECLARE_DRIVER_INIT(basebal2);
	DECLARE_DRIVER_INIT(upyoural);
	DECLARE_DRIVER_INIT(cerberus);
	DECLARE_DRIVER_INIT(aafbd2p);
	DECLARE_DRIVER_INIT(ataxx);
	DECLARE_DRIVER_INIT(ataxxj);
	DECLARE_DRIVER_INIT(wsf);
	DECLARE_DRIVER_INIT(indyheat);
	DECLARE_DRIVER_INIT(brutforc);
	DECLARE_DRIVER_INIT(asylum);
	DECLARE_MACHINE_START(ataxx);
	DECLARE_MACHINE_RESET(ataxx);
	DECLARE_MACHINE_START(leland);
	DECLARE_MACHINE_RESET(leland);
	DECLARE_VIDEO_START(leland);
	DECLARE_VIDEO_START(leland2);
	DECLARE_VIDEO_START(ataxx);

	uint32_t screen_update_leland(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ataxx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
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
	void ataxx_bankswitch();
	void leland_init_eeprom(uint8_t default_val, const uint16_t *data, uint8_t serial_offset, uint8_t serial_type);
	void ataxx_init_eeprom(const uint16_t *data);
	int keycard_r();
	void keycard_w(int data);
	void leland_rotate_memory(const char *cpuname);
	void init_master_ports(uint8_t mvram_base, uint8_t io_base);
	void ataxx(machine_config &config);
	void redline(machine_config &config);
	void lelandi(machine_config &config);
	void leland(machine_config &config);
	void quarterb(machine_config &config);
	void wsf(machine_config &config);
	void leland_video(machine_config &config);
	void ataxx_video(machine_config &config);
	void master_map_io(address_map &map);
	void master_map_io_2(address_map &map);
	void master_map_program(address_map &map);
	void master_map_program_2(address_map &map);
	void master_redline_map_io(address_map &map);
	void slave_large_map_program(address_map &map);
	void slave_map_io(address_map &map);
	void slave_map_io_2(address_map &map);
	void slave_map_program(address_map &map);
	void slave_small_map_program(address_map &map);
	void ataxx_80186_map_io(address_map &map);
	void leland_80186_map_io(address_map &map);
	void leland_80186_map_program(address_map &map);
	void redline_80186_map_io(address_map &map);
};


#define SERIAL_TYPE_NONE        0
#define SERIAL_TYPE_ADD         1
#define SERIAL_TYPE_ADD_XOR     2
#define SERIAL_TYPE_ENCRYPT     3
#define SERIAL_TYPE_ENCRYPT_XOR 4

#endif // MAME_INCLUDES_LELAND_H
