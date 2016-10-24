// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brian Troha
/*************************************************************************

    Incredible Technologies/Strata system
    (8-bit blitter variant)

**************************************************************************/

#include "machine/nvram.h"

#define VIDEO_CLOCK     XTAL_8MHz           /* video (pixel) clock */
#define CPU_CLOCK       XTAL_12MHz          /* clock for 68000-based systems */
#define CPU020_CLOCK    XTAL_25MHz          /* clock for 68EC020-based systems */
#define SOUND_CLOCK     XTAL_16MHz          /* clock for sound board */
#define TMS_CLOCK       XTAL_40MHz          /* TMS320C31 clocks on drivedge */


class itech32_state : public driver_device
{
public:
	itech32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_dsp1(*this, "dsp1"),
		m_dsp2(*this, "dsp2"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_main_ram(*this, "main_ram", 0),
		m_nvram(*this, "nvram", 0),
		m_video(*this, "video", 0),
		m_main_rom(*this, "main_rom", 0),
		m_drivedge_zbuf_control(*this, "drivedge_zctl"),
		m_tms1_boot(*this, "tms1_boot"),
		m_tms1_ram(*this, "tms1_ram"),
		m_tms2_ram(*this, "tms2_ram") { }


	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<cpu_device> m_dsp1;
	optional_device<cpu_device> m_dsp2;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint16_t> m_main_ram;
	optional_shared_ptr<uint16_t> m_nvram;
	optional_shared_ptr<uint16_t> m_video;
	optional_shared_ptr<uint16_t> m_main_rom;
	optional_shared_ptr<uint32_t> m_drivedge_zbuf_control;
	optional_shared_ptr<uint32_t> m_tms1_boot;
	optional_shared_ptr<uint32_t> m_tms1_ram;
	optional_shared_ptr<uint32_t> m_tms2_ram;

	void nvram_init(nvram_device &nvram, void *base, size_t length);

	std::unique_ptr<uint16_t[]> m_videoram;
	uint8_t m_vint_state;
	uint8_t m_xint_state;
	uint8_t m_qint_state;
	uint8_t m_sound_data;
	uint8_t m_sound_return;
	uint8_t m_sound_int_state;
	offs_t m_itech020_prot_address;
	uint8_t m_tms_spinning[2];
	int m_special_result;
	int m_p1_effx;
	int m_p1_effy;
	int m_p1_lastresult;
	attotime m_p1_lasttime;
	int m_p2_effx;
	int m_p2_effy;
	int m_p2_lastresult;
	attotime m_p2_lasttime;
	uint8_t m_written[0x8000];
	uint16_t m_xfer_xcount;
	uint16_t m_xfer_ycount;
	uint16_t m_xfer_xcur;
	uint16_t m_xfer_ycur;
	rectangle m_clip_rect;
	rectangle m_scaled_clip_rect;
	rectangle m_clip_save;
	emu_timer *m_scanline_timer;
	uint32_t m_grom_bank;
	uint16_t m_color_latch[2];
	uint8_t m_enable_latch[2];
	uint16_t *m_videoplane[2];

	// configuration at init time
	int m_is_drivedge;
	uint8_t m_planes;
	uint16_t m_vram_height;
	uint32_t m_vram_mask;
	uint32_t m_vram_xmask;
	uint32_t m_vram_ymask;
	uint8_t *m_grom_base;
	uint32_t m_grom_size;
	uint32_t m_grom_bank_mask;

	void int1_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t trackball_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t trackball_p2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t trackball32_8bit_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t trackball32_4bit_p1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t trackball32_4bit_p2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t trackball32_4bit_combined_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t drivedge_steering_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t drivedge_gas_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint16_t wcbowl_prot_result_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t itech020_prot_result_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t gt2kp_prot_result_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t gtclass_prot_result_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void sound_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t sound_data32_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void sound_data32_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t sound_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_return_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_data_buffer_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void firq_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tms_reset_assert_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void tms_reset_clear_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void tms1_68k_ram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void tms2_68k_ram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void tms1_trigger_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void tms2_trigger_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t drivedge_tms1_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t drivedge_tms2_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void int1_ack32_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void timekill_colora_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void timekill_colorbc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void timekill_intensity_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bloodstm_color1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bloodstm_color2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bloodstm_plane_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void drivedge_color0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void itech020_color1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void itech020_color2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void itech020_plane_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void bloodstm_paletteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void itech32_video_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t itech32_video_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bloodstm_video_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t bloodstm_video_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void itech020_video_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void drivedge_zbuf_control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t itech020_video_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	ioport_value special_port_r(ioport_field &field, void *param);
	void drivedge_portb_out(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void drivedge_turbo_light(int state);
	void pia_portb_out(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_gtclasscp();
	void init_shufshot();
	void init_wcbowlt();
	void init_hardyard();
	void init_s_ver();
	void init_sftm110();
	void init_wcbowln();
	void init_gt2kp();
	void init_sftm();
	void init_drivedge();
	void init_wcbowl();
	void init_wcbowlj();
	void init_aamat();
	void init_bloodstm();
	void init_aama();
	void init_timekill();
	void init_gt3d();
	void init_gt3dl();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void machine_reset_drivedge();
	void init_program_rom();
	void init_sftm_common(int prot_addr);
	void init_shuffle_bowl_common(int prot_addr);
	void install_timekeeper();
	void init_gt_common();

	uint32_t screen_update_itech32(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void generate_int1(device_t &device);
	void delayed_sound_data_w(void *ptr, int32_t param);
	void scanline_interrupt(void *ptr, int32_t param);
	inline offs_t compute_safe_address(int x, int y);
	inline void disable_clipping();
	inline void enable_clipping();
	void logblit(const char *tag);
	void update_interrupts(int fast);
	void draw_raw(uint16_t *base, uint16_t color);
	void draw_raw_drivedge(uint16_t *base, uint16_t *zbase, uint16_t color);
	inline void draw_rle_fast(uint16_t *base, uint16_t color);
	inline void draw_rle_fast_xflip(uint16_t *base, uint16_t color);
	inline void draw_rle_slow(uint16_t *base, uint16_t color);
	void draw_rle(uint16_t *base, uint16_t color);
	void shiftreg_clear(uint16_t *base, uint16_t *zbase);
	void handle_video_command();
	inline int determine_irq_state(int vint, int xint, int qint);
	void itech32_update_interrupts(int vint, int xint, int qint);
};
