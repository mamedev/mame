// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha,Jon Sturm
/*****************************************************************************
 *
 * includes/ti85.h
 *
 ****************************************************************************/

#ifndef MAME_TI_TI85_H
#define MAME_TI_TI85_H

#pragma once

#include "bus/ti8x/ti8x.h"
#include "imagedev/snapquik.h"
#include "machine/bankdev.h"
#include "machine/intelfsh.h"
#include "machine/nvram.h"
#include "video/t6a04.h"
#include "emupal.h"


/* model */
enum ti85_model {
	TI81,
	TI81v2,
	TI82,
	TI83,
	TI85,
	TI86,
	TI83P,
	TI83PSE,
	TI84P,
	TI84PSE
};

struct ti83pse_timer
{
	uint8_t loop = 0;
	uint8_t setup = 0;
	float divsor = 1;
	bool interrupt = false;
	uint8_t max = 0;
	uint8_t count = 0;
};

typedef enum TI83PSE_CTIMER
{
	CRYSTAL_TIMER1 = 0,
	CRYSTAL_TIMER2,
	CRYSTAL_TIMER3,
	HW_TIMER1,
	HW_TIMER2
} ti83pse_ctimers;

class ti85_state : public driver_device
{
public:
	ti85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_link_port(*this, "linkport")
		, m_nvram(*this, "nvram")
		, m_flash(*this, "flash")
		, m_membank(*this, "membank%u", 0U)
		, m_on_button(*this, "ON")
		, m_keys(*this, "BIT%u", 0U)
	{
	}

	void ti83(machine_config &config);
	void ti82(machine_config &config);
	void ti83p(machine_config &config);
	void ti81v2(machine_config &config);
	void ti73(machine_config &config);
	void ti85d(machine_config &config);
	void ti83pse(machine_config &config);
	void ti84pse(machine_config &config);
	void ti84pce(machine_config &config);
	void ti86(machine_config &config);
	void ti81(machine_config &config);
	void ti85(machine_config &config);
	void ti84p(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	optional_device<ti8x_link_port_device> m_link_port;
	optional_shared_ptr<uint8_t> m_nvram;
	optional_device<intelfsh8_device> m_flash;
	optional_device_array<address_map_bank_device, 4> m_membank;
	required_ioport m_on_button;
	required_ioport_array<8> m_keys;

	ti85_model m_model{};

	uint8_t m_LCD_memory_base = 0;
	uint8_t m_LCD_contrast = 0;
	uint8_t m_LCD_status = 0;
	uint8_t m_timer_interrupt_mask = 0;
	uint8_t m_timer_interrupt_status = 0;
	uint8_t m_ctimer_interrupt_status = 0;
	uint8_t m_ON_interrupt_mask = 0;
	uint8_t m_ON_interrupt_status = 0;
	uint8_t m_ON_pressed = 0;
	uint8_t m_flash_unlocked = 0;
	uint8_t m_ti8x_memory_page_1 = 0;
	uint8_t m_ti8x_memory_page_2 = 0;
	uint8_t m_ti8x_memory_page_3 = 0;
	bool m_booting = false;
	uint8_t m_LCD_mask = 0;
	uint8_t m_power_mode = 0;
	uint8_t m_cpu_speed = 0;
	uint8_t m_keypad_mask = 0;
	uint8_t m_video_buffer_width = 0;
	uint8_t m_interrupt_speed = 0;
	uint8_t m_port4_bit0 = 0;
	uint8_t m_ti81_port_7_data = 0;
	std::unique_ptr<uint8_t[]> m_ti8x_ram;
	uint8_t m_PCR = 0;
	uint8_t m_ti8x_port2 = 0;
	uint8_t m_ti83p_port4 = 0;
	uint8_t m_ti83pse_port21 = 0;
	int m_ti_video_memory_size = 0;
	int m_ti_screen_x_size = 0;
	int m_ti_screen_y_size = 0;
	int m_ti_number_of_frames = 0;
	std::unique_ptr<uint8_t[]> m_frames;
	uint8_t * m_bios = nullptr;
	emu_timer *m_ti85_timer = nullptr;
	emu_timer *m_ti83_1st_timer = nullptr;
	emu_timer *m_ti83_2nd_timer = nullptr;

	uint8_t ti85_port_0000_r();
	uint8_t ti8x_keypad_r();
	uint8_t ti85_port_0006_r();
	uint8_t ti8x_serial_r();
	uint8_t ti86_port_0005_r();
	uint8_t ti83_port_0000_r();
	uint8_t ti8x_plus_serial_r();
	void ti81_port_0007_w(uint8_t data);
	void ti85_port_0000_w(uint8_t data);
	void ti8x_keypad_w(uint8_t data);
	void ti85_port_0002_w(uint8_t data);
	void ti85_port_0003_w(uint8_t data);
	void ti85_port_0004_w(uint8_t data);
	void ti85_port_0005_w(uint8_t data);
	void ti85_port_0006_w(uint8_t data);
	void ti8x_serial_w(uint8_t data);
	void ti86_port_0005_w(uint8_t data);
	void ti86_port_0006_w(uint8_t data);
	void ti82_port_0002_w(uint8_t data);
	void ti83_port_0000_w(uint8_t data);
	void ti83_port_0002_w(uint8_t data);
	void ti83_port_0003_w(uint8_t data);
	void ti8x_plus_serial_w(uint8_t data);
	void ti83p_int_mask_w(uint8_t data);
	void ti83p_port_0004_w(uint8_t data);
	void ti83p_port_0006_w(uint8_t data);
	void ti83p_port_0007_w(uint8_t data);
	void ti83pse_int_ack_w(uint8_t data);
	void ti83pse_port_0004_w(uint8_t data);
	void ti83pse_port_0005_w(uint8_t data);
	void ti83pse_port_0006_w(uint8_t data);
	void ti83pse_port_0007_w(uint8_t data);
	void ti83p_port_0014_w(uint8_t data);
	void ti83pse_port_0020_w(uint8_t data);
	void ti83pse_port_0021_w(uint8_t data);
	uint8_t ti85_port_0002_r();
	uint8_t ti85_port_0003_r();
	uint8_t ti85_port_0004_r();
	uint8_t ti85_port_0005_r();
	uint8_t ti86_port_0006_r();
	uint8_t ti82_port_0002_r();
	uint8_t ti83_port_0002_r();
	uint8_t ti83_port_0003_r();
	uint8_t ti83p_port_0002_r();
	uint8_t ti83p_port_0004_r();
	uint8_t ti83pse_port_0002_r();
	uint8_t ti83pse_port_0005_r();
	uint8_t ti83pse_port_0009_r();
	uint8_t ti83pse_port_0015_r();
	uint8_t ti83pse_port_0020_r();
	uint8_t ti83pse_port_0021_r();
	uint8_t ti84pse_port_0055_r();
	uint8_t ti84pse_port_0056_r();
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void ti85_palette(palette_device &palette);
	DECLARE_MACHINE_RESET(ti85);
	DECLARE_MACHINE_RESET(ti83p);
	void ti82_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(ti86);
	DECLARE_MACHINE_START(ti83p);
	DECLARE_MACHINE_START(ti83pse);
	DECLARE_MACHINE_START(ti84pse);
	DECLARE_MACHINE_START(ti84p);
	void ti8xpse_init_common();

	uint32_t screen_update_ti85(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(ti85_timer_callback);
	TIMER_CALLBACK_MEMBER(ti83_timer1_callback);
	TIMER_CALLBACK_MEMBER(ti83_timer2_callback);

	TIMER_CALLBACK_MEMBER(crystal_timer_tick);

	//crystal timers
	void ti83pse_count(uint8_t timer, uint8_t data);

	emu_timer *m_crystal_timer1 = nullptr;
	emu_timer *m_crystal_timer2 = nullptr;
	emu_timer *m_crystal_timer3 = nullptr;
	uint8_t ti83pse_ctimer1_setup_r();
	void ti83pse_ctimer1_setup_w(uint8_t data);
	uint8_t ti83pse_ctimer1_loop_r();
	void ti83pse_ctimer1_loop_w(uint8_t data);
	uint8_t ti83pse_ctimer1_count_r();
	void ti83pse_ctimer1_count_w(uint8_t data);
	uint8_t ti83pse_ctimer2_setup_r();
	void ti83pse_ctimer2_setup_w(uint8_t data);
	uint8_t ti83pse_ctimer2_loop_r();
	void ti83pse_ctimer2_loop_w(uint8_t data);
	uint8_t ti83pse_ctimer2_count_r();
	void ti83pse_ctimer2_count_w(uint8_t data);
	uint8_t ti83pse_ctimer3_setup_r();
	void ti83pse_ctimer3_setup_w(uint8_t data);
	uint8_t ti83pse_ctimer3_loop_r();
	void ti83pse_ctimer3_loop_w(uint8_t data);
	uint8_t ti83pse_ctimer3_count_r();
	void ti83pse_ctimer3_count_w(uint8_t data);
	uint8_t ti83p_membank2_r(offs_t offset);
	uint8_t ti83p_membank3_r(offs_t offset);

	void ti8x_update_bank(address_space &space, uint8_t bank, uint8_t *base, uint8_t page, bool is_ram);
	void update_ti85_memory();
	void update_ti83p_memory();
	void update_ti83pse_memory();
	void update_ti86_memory();
	void ti8x_snapshot_setup_registers(uint8_t *data);
	void ti85_setup_snapshot(uint8_t *data);
	void ti86_setup_snapshot(uint8_t *data);
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);

	ti83pse_timer m_ctimer[3];

	void ti81_io(address_map &map) ATTR_COLD;
	void ti81_mem(address_map &map) ATTR_COLD;
	void ti81v2_io(address_map &map) ATTR_COLD;
	void ti82_io(address_map &map) ATTR_COLD;
	void ti83_io(address_map &map) ATTR_COLD;
	void ti83p_asic_mem(address_map &map) ATTR_COLD;
	void ti83p_banked_mem(address_map &map) ATTR_COLD;
	void ti83p_io(address_map &map) ATTR_COLD;
	void ti83pse_banked_mem(address_map &map) ATTR_COLD;
	void ti83pse_io(address_map &map) ATTR_COLD;
	void ti84p_banked_mem(address_map &map) ATTR_COLD;
	void ti85_io(address_map &map) ATTR_COLD;
	void ti86_io(address_map &map) ATTR_COLD;
	void ti86_mem(address_map &map) ATTR_COLD;
	//address_space &asic;
};

#endif // MAME_TI_TI85_H
