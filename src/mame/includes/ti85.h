// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha,Jon Sturm
/*****************************************************************************
 *
 * includes/ti85.h
 *
 ****************************************************************************/

#ifndef TI85_H_
#define TI85_H_

#include "imagedev/snapquik.h"
#include "video/t6a04.h"
#include "machine/bankdev.h"
#include "sound/speaker.h"
#include "machine/nvram.h"
#include "machine/intelfsh.h"


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

typedef struct
{
	uint8_t loop;
	uint8_t setup;
	float divsor;
	bool interrupt;
	uint8_t max;
	uint8_t count;
} ti83pse_timer;

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
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_speaker(*this, "speaker"),
//        m_serial(*this, "tiserial"),
			m_nvram(*this, "nvram"),
			m_flash(*this, "flash"),
			m_membank1(*this, "membank1"),
			m_membank2(*this, "membank2"),
			m_membank3(*this, "membank3"),
			m_membank4(*this, "membank4")
		{ }

	required_device<cpu_device> m_maincpu;
	optional_device<speaker_sound_device> m_speaker;
	//optional_device<> m_serial;
	optional_shared_ptr<uint8_t>  m_nvram;
	optional_device<intelfsh_device> m_flash;
	optional_device<address_map_bank_device> m_membank1;
	optional_device<address_map_bank_device> m_membank2;
	optional_device<address_map_bank_device> m_membank3;
	optional_device<address_map_bank_device> m_membank4;

	ti85_model m_model;

	uint8_t m_LCD_memory_base;
	uint8_t m_LCD_contrast;
	uint8_t m_LCD_status;
	uint8_t m_timer_interrupt_mask;
	uint8_t m_timer_interrupt_status;
	uint8_t m_ctimer_interrupt_status;
	uint8_t m_ON_interrupt_mask;
	uint8_t m_ON_interrupt_status;
	uint8_t m_ON_pressed;
	uint8_t m_flash_unlocked;
	uint8_t m_ti8x_memory_page_1;
	uint8_t m_ti8x_memory_page_2;
	uint8_t m_ti8x_memory_page_3;
	bool m_booting;
	uint8_t m_LCD_mask;
	uint8_t m_power_mode;
	uint8_t m_cpu_speed;
	uint8_t m_keypad_mask;
	uint8_t m_video_buffer_width;
	uint8_t m_interrupt_speed;
	uint8_t m_port4_bit0;
	uint8_t m_ti81_port_7_data;
	std::unique_ptr<uint8_t[]> m_ti8x_ram;
	uint8_t m_PCR;
	uint8_t m_red_out;
	uint8_t m_white_out;
	uint8_t m_ti8x_port2;
	uint8_t m_ti83p_port4;
	uint8_t m_ti83pse_port21;
	int m_ti_video_memory_size;
	int m_ti_screen_x_size;
	int m_ti_screen_y_size;
	int m_ti_number_of_frames;
	std::unique_ptr<uint8_t[]> m_frames;
	uint8_t * m_bios;
	uint8_t ti85_port_0000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti8x_keypad_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti85_port_0006_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti8x_serial_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti86_port_0005_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti83_port_0000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti8x_plus_serial_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ti81_port_0007_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti85_port_0000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti8x_keypad_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti85_port_0002_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti85_port_0003_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti85_port_0004_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti85_port_0005_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti85_port_0006_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti8x_serial_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti86_port_0005_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti86_port_0006_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti82_port_0002_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83_port_0000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83_port_0002_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83_port_0003_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti8x_plus_serial_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83p_int_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83p_port_0004_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83p_port_0006_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83p_port_0007_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83pse_int_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83pse_port_0004_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83pse_port_0005_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83pse_port_0006_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83pse_port_0007_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83p_port_0014_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83pse_port_0020_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ti83pse_port_0021_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ti85_port_0002_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti85_port_0003_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti85_port_0004_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti85_port_0005_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti86_port_0006_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti82_port_0002_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti83_port_0002_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti83_port_0003_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti83p_port_0002_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti83p_port_0004_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_port_0002_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_port_0005_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_port_0009_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_port_0015_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_port_0020_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_port_0021_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti84pse_port_0055_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ti84pse_port_0056_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_ti85(palette_device &palette);
	void machine_reset_ti85();
	void machine_reset_ti83p();
	void palette_init_ti82(palette_device &palette);
	void machine_start_ti86();
	void machine_start_ti83p();
	void machine_start_ti83pse();
	void machine_start_ti84pse();
	void machine_start_ti84p();
	void ti8xpse_init_common();

	uint32_t screen_update_ti85(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ti85_timer_callback(void *ptr, int32_t param);
	void ti83_timer1_callback(void *ptr, int32_t param);
	void ti83_timer2_callback(void *ptr, int32_t param);

	//crystal timers
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void ti83pse_count( uint8_t timer, uint8_t data);

	emu_timer *m_crystal_timer1;
	emu_timer *m_crystal_timer2;
	emu_timer *m_crystal_timer3;
	uint8_t ti83pse_ctimer1_setup_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ti83pse_ctimer1_setup_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_ctimer1_loop_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ti83pse_ctimer1_loop_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_ctimer1_count_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ti83pse_ctimer1_count_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_ctimer2_setup_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ti83pse_ctimer2_setup_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_ctimer2_loop_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ti83pse_ctimer2_loop_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_ctimer2_count_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ti83pse_ctimer2_count_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_ctimer3_setup_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ti83pse_ctimer3_setup_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_ctimer3_loop_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ti83pse_ctimer3_loop_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ti83pse_ctimer3_count_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ti83pse_ctimer3_count_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);


	void update_ti85_memory ();
	void update_ti83p_memory ();
	void update_ti83pse_memory ();
	void update_ti86_memory ();
	void ti8x_snapshot_setup_registers (uint8_t * data);
	void ti85_setup_snapshot (uint8_t * data);
	void ti86_setup_snapshot (uint8_t * data);
	DECLARE_SNAPSHOT_LOAD_MEMBER( ti8x );
	offs_t ti83p_direct_update_handler(direct_read_data &direct, offs_t address);

	ti83pse_timer m_ctimer[3];

	//address_space &asic;
};

#endif /* TI85_H_ */
