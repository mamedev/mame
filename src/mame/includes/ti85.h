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
	UINT8 loop;
	UINT8 setup;
	float divsor;
	bool interrupt;
	UINT8 max;
	UINT8 count;
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
	optional_shared_ptr<UINT8>  m_nvram;
	optional_device<intelfsh_device> m_flash;
	optional_device<address_map_bank_device> m_membank1;
	optional_device<address_map_bank_device> m_membank2;
	optional_device<address_map_bank_device> m_membank3;
	optional_device<address_map_bank_device> m_membank4;

	ti85_model m_model;

	UINT8 m_LCD_memory_base;
	UINT8 m_LCD_contrast;
	UINT8 m_LCD_status;
	UINT8 m_timer_interrupt_mask;
	UINT8 m_timer_interrupt_status;
	UINT8 m_ctimer_interrupt_status;
	UINT8 m_ON_interrupt_mask;
	UINT8 m_ON_interrupt_status;
	UINT8 m_ON_pressed;
	UINT8 m_flash_unlocked;
	UINT8 m_ti8x_memory_page_1;
	UINT8 m_ti8x_memory_page_2;
	UINT8 m_ti8x_memory_page_3;
	bool m_booting;
	UINT8 m_LCD_mask;
	UINT8 m_power_mode;
	UINT8 m_cpu_speed;
	UINT8 m_keypad_mask;
	UINT8 m_video_buffer_width;
	UINT8 m_interrupt_speed;
	UINT8 m_port4_bit0;
	UINT8 m_ti81_port_7_data;
	std::unique_ptr<UINT8[]> m_ti8x_ram;
	UINT8 m_PCR;
	UINT8 m_red_out;
	UINT8 m_white_out;
	UINT8 m_ti8x_port2;
	UINT8 m_ti83p_port4;
	UINT8 m_ti83pse_port21;
	int m_ti_video_memory_size;
	int m_ti_screen_x_size;
	int m_ti_screen_y_size;
	int m_ti_number_of_frames;
	UINT8 * m_frames;
	UINT8 * m_bios;
	DECLARE_READ8_MEMBER(ti85_port_0000_r);
	DECLARE_READ8_MEMBER(ti8x_keypad_r);
	DECLARE_READ8_MEMBER(ti85_port_0006_r);
	DECLARE_READ8_MEMBER(ti8x_serial_r);
	DECLARE_READ8_MEMBER(ti86_port_0005_r);
	DECLARE_READ8_MEMBER(ti83_port_0000_r);
	DECLARE_READ8_MEMBER(ti8x_plus_serial_r);
	DECLARE_WRITE8_MEMBER(ti81_port_0007_w);
	DECLARE_WRITE8_MEMBER(ti85_port_0000_w);
	DECLARE_WRITE8_MEMBER(ti8x_keypad_w);
	DECLARE_WRITE8_MEMBER(ti85_port_0002_w);
	DECLARE_WRITE8_MEMBER(ti85_port_0003_w);
	DECLARE_WRITE8_MEMBER(ti85_port_0004_w);
	DECLARE_WRITE8_MEMBER(ti85_port_0005_w);
	DECLARE_WRITE8_MEMBER(ti85_port_0006_w);
	DECLARE_WRITE8_MEMBER(ti8x_serial_w);
	DECLARE_WRITE8_MEMBER(ti86_port_0005_w);
	DECLARE_WRITE8_MEMBER(ti86_port_0006_w);
	DECLARE_WRITE8_MEMBER(ti82_port_0002_w);
	DECLARE_WRITE8_MEMBER(ti83_port_0000_w);
	DECLARE_WRITE8_MEMBER(ti83_port_0002_w);
	DECLARE_WRITE8_MEMBER(ti83_port_0003_w);
	DECLARE_WRITE8_MEMBER(ti8x_plus_serial_w);
	DECLARE_WRITE8_MEMBER(ti83p_int_mask_w);
	DECLARE_WRITE8_MEMBER(ti83p_port_0004_w);
	DECLARE_WRITE8_MEMBER(ti83p_port_0006_w);
	DECLARE_WRITE8_MEMBER(ti83p_port_0007_w);
	DECLARE_WRITE8_MEMBER(ti83pse_int_ack_w);
	DECLARE_WRITE8_MEMBER(ti83pse_port_0004_w);
	DECLARE_WRITE8_MEMBER(ti83pse_port_0005_w);
	DECLARE_WRITE8_MEMBER(ti83pse_port_0006_w);
	DECLARE_WRITE8_MEMBER(ti83pse_port_0007_w);
	DECLARE_WRITE8_MEMBER(ti83p_port_0014_w);
	DECLARE_WRITE8_MEMBER(ti83pse_port_0020_w);
	DECLARE_WRITE8_MEMBER(ti83pse_port_0021_w);
	DECLARE_READ8_MEMBER( ti85_port_0002_r );
	DECLARE_READ8_MEMBER( ti85_port_0003_r );
	DECLARE_READ8_MEMBER( ti85_port_0004_r );
	DECLARE_READ8_MEMBER( ti85_port_0005_r );
	DECLARE_READ8_MEMBER( ti86_port_0006_r );
	DECLARE_READ8_MEMBER( ti82_port_0002_r );
	DECLARE_READ8_MEMBER( ti83_port_0002_r );
	DECLARE_READ8_MEMBER( ti83_port_0003_r );
	DECLARE_READ8_MEMBER( ti83p_port_0002_r );
	DECLARE_READ8_MEMBER( ti83p_port_0004_r );
	DECLARE_READ8_MEMBER( ti83pse_port_0002_r );
	DECLARE_READ8_MEMBER( ti83pse_port_0005_r );
	DECLARE_READ8_MEMBER( ti83pse_port_0009_r );
	DECLARE_READ8_MEMBER( ti83pse_port_0015_r );
	DECLARE_READ8_MEMBER( ti83pse_port_0020_r );
	DECLARE_READ8_MEMBER( ti83pse_port_0021_r );
	DECLARE_READ8_MEMBER( ti84pse_port_0055_r );
	DECLARE_READ8_MEMBER( ti84pse_port_0056_r );
	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(ti85);
	DECLARE_MACHINE_RESET(ti85);
	DECLARE_MACHINE_RESET(ti83p);
	DECLARE_PALETTE_INIT(ti82);
	DECLARE_MACHINE_START(ti86);
	DECLARE_MACHINE_START(ti83p);
	DECLARE_MACHINE_START(ti83pse);
	DECLARE_MACHINE_START(ti84pse);
	DECLARE_MACHINE_START(ti84p);
	void ti8xpse_init_common();

	UINT32 screen_update_ti85(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(ti85_timer_callback);
	TIMER_CALLBACK_MEMBER(ti83_timer1_callback);
	TIMER_CALLBACK_MEMBER(ti83_timer2_callback);

	//crystal timers
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void ti83pse_count( UINT8 timer, UINT8 data);

	emu_timer *m_crystal_timer1;
	emu_timer *m_crystal_timer2;
	emu_timer *m_crystal_timer3;
	DECLARE_READ8_MEMBER( ti83pse_ctimer1_setup_r );
	DECLARE_WRITE8_MEMBER( ti83pse_ctimer1_setup_w );
	DECLARE_READ8_MEMBER( ti83pse_ctimer1_loop_r );
	DECLARE_WRITE8_MEMBER( ti83pse_ctimer1_loop_w );
	DECLARE_READ8_MEMBER( ti83pse_ctimer1_count_r );
	DECLARE_WRITE8_MEMBER( ti83pse_ctimer1_count_w );
	DECLARE_READ8_MEMBER( ti83pse_ctimer2_setup_r );
	DECLARE_WRITE8_MEMBER( ti83pse_ctimer2_setup_w );
	DECLARE_READ8_MEMBER( ti83pse_ctimer2_loop_r );
	DECLARE_WRITE8_MEMBER( ti83pse_ctimer2_loop_w );
	DECLARE_READ8_MEMBER( ti83pse_ctimer2_count_r );
	DECLARE_WRITE8_MEMBER( ti83pse_ctimer2_count_w );
	DECLARE_READ8_MEMBER( ti83pse_ctimer3_setup_r );
	DECLARE_WRITE8_MEMBER( ti83pse_ctimer3_setup_w );
	DECLARE_READ8_MEMBER( ti83pse_ctimer3_loop_r );
	DECLARE_WRITE8_MEMBER( ti83pse_ctimer3_loop_w );
	DECLARE_READ8_MEMBER( ti83pse_ctimer3_count_r );
	DECLARE_WRITE8_MEMBER( ti83pse_ctimer3_count_w );


	void update_ti85_memory ();
	void update_ti83p_memory ();
	void update_ti83pse_memory ();
	void update_ti86_memory ();
	void ti8x_snapshot_setup_registers (UINT8 * data);
	void ti85_setup_snapshot (UINT8 * data);
	void ti86_setup_snapshot (UINT8 * data);
	DECLARE_SNAPSHOT_LOAD_MEMBER( ti8x );
	DECLARE_DIRECT_UPDATE_MEMBER( ti83p_direct_update_handler );

	ti83pse_timer m_ctimer[3];

	//address_space &asic;
};

#endif /* TI85_H_ */
