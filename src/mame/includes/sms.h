// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Charles MacDonald,Mathis Rosenhauer,Brad Oliver,Michael Luong,Fabio Priuli,Enik Land
/*****************************************************************************
 *
 * includes/sms.h
 *
 ****************************************************************************/

#ifndef SMS_H_
#define SMS_H_

#define LOG_REG
#define LOG_PAGING
#define LOG_COLOR

#define NVRAM_SIZE             (0x08000)
#define CPU_ADDRESSABLE_SIZE   (0x10000)

#define MAX_CARTRIDGES        16

#define CONTROL1_TAG   "ctrl1"
#define CONTROL2_TAG   "ctrl2"

#include "bus/sega8/sega8_slot.h"
#include "bus/sg1000_exp/sg1000exp.h"
#include "bus/sms_exp/smsexp.h"
#include "bus/sms_ctrl/smsctrl.h"
#include "bus/gamegear/ggext.h"


class sms_state : public driver_device
{
public:
	sms_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vdp(*this, "sms_vdp"),
		m_main_scr(*this, "screen"),
		m_psg_sms(*this, "segapsg"),
		m_psg_gg(*this, "gamegear"),
		m_ym(*this, "ym2413"),
		m_port_ctrl1(*this, CONTROL1_TAG),
		m_port_ctrl2(*this, CONTROL2_TAG),
		m_port_gg_ext(*this, "ext"),
		m_port_gg_dc(*this, "GG_PORT_DC"),
		m_port_pause(*this, "PAUSE"),
		m_port_reset(*this, "RESET"),
		m_port_rapid(*this, "RAPID"),
		m_port_start(*this, "START"),
		m_port_scope(*this, "SEGASCOPE"),
		m_port_scope_binocular(*this, "SSCOPE_BINOCULAR"),
		m_port_persist(*this, "PERSISTENCE"),
		m_region_maincpu(*this, "maincpu"),
		m_mainram(nullptr),
		m_is_gamegear(0),
		m_is_smsj(0),
		m_is_mark_iii(0),
		m_is_sdisp(0),
		m_ioctrl_region_is_japan(0),
		m_has_bios_0400(0),
		m_has_bios_2000(0),
		m_has_bios_full(0),
		m_has_jpn_sms_cart_slot(0),
		m_store_cart_selection_data(0) { }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<sega315_5124_device> m_vdp;
	required_device<screen_device> m_main_scr;
	optional_device<segapsg_device> m_psg_sms;
	optional_device<gamegear_device> m_psg_gg;
	optional_device<ym2413_device> m_ym;
	optional_device<sms_control_port_device> m_port_ctrl1;
	optional_device<sms_control_port_device> m_port_ctrl2;
	optional_device<gg_ext_port_device> m_port_gg_ext;

	optional_ioport m_port_gg_dc;
	optional_ioport m_port_pause;
	optional_ioport m_port_reset;
	optional_ioport m_port_rapid;
	optional_ioport m_port_start;
	optional_ioport m_port_scope;
	optional_ioport m_port_scope_binocular;
	optional_ioport m_port_persist;

	required_memory_region m_region_maincpu;
	address_space *m_space;
	std::unique_ptr<uint8_t[]> m_mainram;
	uint8_t *m_BIOS;

	// for 3D glass binocular hack
	device_t *m_left_lcd;
	device_t *m_right_lcd;
	bitmap_rgb32 m_prevleft_bitmap;
	bitmap_rgb32 m_prevright_bitmap;

	// for gamegear LCD persistence hack
	bitmap_rgb32 m_prev_bitmap;
	bool m_prev_bitmap_copied;

	// for gamegear SMS mode scaling
	bitmap_rgb32 m_gg_sms_mode_bitmap;
	// line_buffer will be used to hold 4 lines of line data as a kind of cache for
	// vertical scaling in the gamegear sms compatibility mode.
	std::unique_ptr<int[]> m_line_buffer;

	// model identifiers
	uint8_t m_is_gamegear;
	uint8_t m_is_smsj;
	uint8_t m_is_mark_iii;
	uint8_t m_is_sdisp;
	uint8_t m_ioctrl_region_is_japan;
	uint8_t m_has_bios_0400;
	uint8_t m_has_bios_2000;
	uint8_t m_has_bios_full;
	uint8_t m_has_jpn_sms_cart_slot;

	// [0] for 0x400-0x3fff, [1] for 0x4000-0x7fff, [2] for 0x8000-0xffff, [3] for 0x0000-0x0400
	uint8_t m_bios_page[4];

	uint8_t m_bios_page_count;
	uint8_t m_mapper[4];
	uint8_t m_io_ctrl_reg;
	uint8_t m_mem_ctrl_reg;
	uint8_t m_mem_device_enabled;
	uint8_t m_smsj_audio_control;
	uint8_t m_port_dc_reg;
	uint8_t m_port_dd_reg;
	uint8_t m_gg_sio[5];
	int m_paused;

	uint8_t m_ctrl1_th_state;
	uint8_t m_ctrl2_th_state;
	uint8_t m_ctrl1_th_latch;
	uint8_t m_ctrl2_th_latch;

	// Data needed for Light Phaser
	int m_lphaser_x_offs;   /* Needed to 'calibrate' lphaser; set at cart loading */

	// Data needed for SegaScope (3D glasses)
	uint8_t m_sscope_state;
	uint8_t m_frame_sscope_state;

	// Data needed for Rapid button (smsj, sms1kr, sms1krfm)
	uint16_t m_csync_counter;
	uint8_t m_rapid_mode;
	uint8_t m_rapid_read_state;
	uint8_t m_rapid_last_dc;
	uint8_t m_rapid_last_dd;

	// slot devices
	sega8_cart_slot_device *m_cartslot;
	sega8_card_slot_device *m_cardslot;
	sms_expansion_slot_device *m_smsexpslot;
	sg1000_expansion_slot_device *m_sgexpslot;

	// these are only used by the Store Display unit, but we keep them here temporarily to avoid the need of separate start/reset
	sega8_cart_slot_device *m_slots[16];
	sega8_card_slot_device *m_cards[16];
	uint8_t m_store_control;
	uint8_t m_store_cart_selection_data;
	void store_post_load();
	void store_select_cart(uint8_t data);

	uint8_t read_0000(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t read_4000(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t read_8000(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write_cart(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t sms_mapper_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sms_mapper_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sms_mem_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sms_io_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sms_count_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sms_input_port_dc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sms_input_port_dd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t gg_input_port_00_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sg1000m3_peripheral_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sg1000m3_peripheral_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gg_sio_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gg_sio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gg_psg_stereo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gg_psg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sms_psg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t smsj_audio_control_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void smsj_audio_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void smsj_ym2413_register_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void smsj_ym2413_data_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sms_sscope_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sms_sscope_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void sms_pause_callback(int state);
	void sms_csync_callback(int state);
	void sms_ctrl1_th_input(int state);
	void sms_ctrl2_th_input(int state);
	void gg_ext_th_input(int state);
	uint32_t sms_pixel_color(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

	void init_sg1000m3();
	void init_gamegear();
	void init_gamegeaj();
	void init_sms1krfm();
	void init_sms1kr();
	void init_smskr();
	void init_smsj();
	void init_sms1();
	void init_sms();
	void machine_start_sms();
	void machine_reset_sms();
	void video_start_gamegear();
	void video_reset_gamegear();
	void video_start_sms1();
	void video_reset_sms1();

	uint32_t screen_update_sms(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sms1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gamegear(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_gg_sms_mode_scaling(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank_sms1(screen_device &screen, bool state);

protected:
	uint8_t read_bus(address_space &space, unsigned int bank, uint16_t base_addr, uint16_t offset);
	void setup_bios();
	void setup_media_slots();
	void setup_enabled_slots();
	void lphaser_hcount_latch();
	void sms_get_inputs();
};

class smssdisp_state : public sms_state
{
public:
	smssdisp_state(const machine_config &mconfig, device_type type, const char *tag)
	: sms_state(mconfig, type, tag),
	m_control_cpu(*this, "control")
	{ }

	required_device<cpu_device> m_control_cpu;

	uint8_t sms_store_cart_select_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sms_store_cart_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sms_store_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_smssdisp();

	uint8_t store_cart_peek(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void sms_store_int_callback(int state);
};


/*----------- defined in machine/sms.c -----------*/

#define IO_EXPANSION    (0x80)  /* Expansion slot enable (1= disabled, 0= enabled) */
#define IO_CARTRIDGE    (0x40)  /* Cartridge slot enable (1= disabled, 0= enabled) */
#define IO_CARD         (0x20)  /* Card slot disabled (1= disabled, 0= enabled) */
#define IO_WORK_RAM     (0x10)  /* Work RAM disabled (1= disabled, 0= enabled) */
#define IO_BIOS_ROM     (0x08)  /* BIOS ROM disabled (1= disabled, 0= enabled) */
#define IO_CHIP         (0x04)  /* I/O chip disabled (1= disabled, 0= enabled) */

#endif /* SMS_H_ */
