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
		m_main_ram(*this, "main_ram", 0),
		m_nvram(*this, "nvram", 0),
		m_video(*this, "video", 0),
		m_main_rom(*this, "main_rom", 0),
		m_drivedge_zbuf_control(*this, "drivedge_zctl"),
		m_tms1_boot(*this, "tms1_boot"),
		m_tms1_ram(*this, "tms1_ram"),
		m_tms2_ram(*this, "tms2_ram"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_dsp1(*this, "dsp1"),
		m_dsp2(*this, "dsp2"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	optional_shared_ptr<UINT16> m_main_ram;
	optional_shared_ptr<UINT16> m_nvram;
	optional_shared_ptr<UINT16> m_video;
	optional_shared_ptr<UINT16> m_main_rom;
	optional_shared_ptr<UINT32> m_drivedge_zbuf_control;
	optional_shared_ptr<UINT32> m_tms1_boot;
	optional_shared_ptr<UINT32> m_tms1_ram;
	optional_shared_ptr<UINT32> m_tms2_ram;

	void nvram_init(nvram_device &nvram, void *base, size_t length);

	std::unique_ptr<UINT16[]> m_videoram;
	UINT8 m_vint_state;
	UINT8 m_xint_state;
	UINT8 m_qint_state;
	UINT8 m_sound_data;
	UINT8 m_sound_return;
	UINT8 m_sound_int_state;
	offs_t m_itech020_prot_address;
	UINT8 m_tms_spinning[2];
	int m_special_result;
	int m_p1_effx;
	int m_p1_effy;
	int m_p1_lastresult;
	attotime m_p1_lasttime;
	int m_p2_effx;
	int m_p2_effy;
	int m_p2_lastresult;
	attotime m_p2_lasttime;
	UINT8 m_written[0x8000];
	int m_is_drivedge;
	UINT8 m_planes;
	UINT16 m_vram_height;
	UINT16 m_xfer_xcount;
	UINT16 m_xfer_ycount;
	UINT16 m_xfer_xcur;
	UINT16 m_xfer_ycur;
	rectangle m_clip_rect;
	rectangle m_scaled_clip_rect;
	rectangle m_clip_save;
	emu_timer *m_scanline_timer;
	UINT8 *m_grom_base;
	UINT32 m_grom_size;
	UINT32 m_grom_bank;
	UINT32 m_grom_bank_mask;
	UINT16 m_color_latch[2];
	UINT8 m_enable_latch[2];
	UINT16 *m_videoplane[2];
	UINT32 m_vram_mask;
	UINT32 m_vram_xmask;
	UINT32 m_vram_ymask;
	DECLARE_WRITE16_MEMBER(int1_ack_w);
	DECLARE_READ16_MEMBER(trackball_r);
	DECLARE_READ16_MEMBER(trackball_p2_r);
	DECLARE_READ32_MEMBER(trackball32_8bit_r);
	DECLARE_READ32_MEMBER(trackball32_4bit_p1_r);
	DECLARE_READ32_MEMBER(trackball32_4bit_p2_r);
	DECLARE_READ32_MEMBER(trackball32_4bit_combined_r);
	DECLARE_READ32_MEMBER(drivedge_steering_r);
	DECLARE_READ32_MEMBER(drivedge_gas_r);
	DECLARE_READ16_MEMBER(wcbowl_prot_result_r);
	DECLARE_READ32_MEMBER(itech020_prot_result_r);
	DECLARE_READ32_MEMBER(gt2kp_prot_result_r);
	DECLARE_READ32_MEMBER(gtclass_prot_result_r);
	DECLARE_WRITE8_MEMBER(sound_bank_w);
	DECLARE_WRITE16_MEMBER(sound_data_w);
	DECLARE_READ32_MEMBER(sound_data32_r);
	DECLARE_WRITE32_MEMBER(sound_data32_w);
	DECLARE_READ8_MEMBER(sound_data_r);
	DECLARE_WRITE8_MEMBER(sound_return_w);
	DECLARE_READ8_MEMBER(sound_data_buffer_r);
	DECLARE_WRITE8_MEMBER(firq_clear_w);
	DECLARE_WRITE32_MEMBER(tms_reset_assert_w);
	DECLARE_WRITE32_MEMBER(tms_reset_clear_w);
	DECLARE_WRITE32_MEMBER(tms1_68k_ram_w);
	DECLARE_WRITE32_MEMBER(tms2_68k_ram_w);
	DECLARE_WRITE32_MEMBER(tms1_trigger_w);
	DECLARE_WRITE32_MEMBER(tms2_trigger_w);
	DECLARE_READ32_MEMBER(drivedge_tms1_speedup_r);
	DECLARE_READ32_MEMBER(drivedge_tms2_speedup_r);
	DECLARE_WRITE32_MEMBER(int1_ack32_w);
	DECLARE_WRITE16_MEMBER(timekill_colora_w);
	DECLARE_WRITE16_MEMBER(timekill_colorbc_w);
	DECLARE_WRITE16_MEMBER(timekill_intensity_w);
	DECLARE_WRITE16_MEMBER(bloodstm_color1_w);
	DECLARE_WRITE16_MEMBER(bloodstm_color2_w);
	DECLARE_WRITE16_MEMBER(bloodstm_plane_w);
	DECLARE_WRITE32_MEMBER(drivedge_color0_w);
	DECLARE_WRITE32_MEMBER(itech020_color1_w);
	DECLARE_WRITE32_MEMBER(itech020_color2_w);
	DECLARE_WRITE32_MEMBER(itech020_plane_w);
	DECLARE_WRITE16_MEMBER(bloodstm_paletteram_w);
	DECLARE_WRITE16_MEMBER(itech32_video_w);
	DECLARE_READ16_MEMBER(itech32_video_r);
	DECLARE_WRITE16_MEMBER(bloodstm_video_w);
	DECLARE_READ16_MEMBER(bloodstm_video_r);
	DECLARE_WRITE32_MEMBER(itech020_video_w);
	DECLARE_WRITE32_MEMBER(drivedge_zbuf_control_w);
	DECLARE_READ32_MEMBER(itech020_video_r);
	DECLARE_CUSTOM_INPUT_MEMBER(special_port_r);
	DECLARE_WRITE8_MEMBER(drivedge_portb_out);
	DECLARE_WRITE_LINE_MEMBER(drivedge_turbo_light);
	DECLARE_WRITE8_MEMBER(pia_portb_out);
	DECLARE_DRIVER_INIT(gtclasscp);
	DECLARE_DRIVER_INIT(shufshot);
	DECLARE_DRIVER_INIT(wcbowlt);
	DECLARE_DRIVER_INIT(hardyard);
	DECLARE_DRIVER_INIT(s_ver);
	DECLARE_DRIVER_INIT(sftm110);
	DECLARE_DRIVER_INIT(wcbowln);
	DECLARE_DRIVER_INIT(gt2kp);
	DECLARE_DRIVER_INIT(sftm);
	DECLARE_DRIVER_INIT(drivedge);
	DECLARE_DRIVER_INIT(wcbowl);
	DECLARE_DRIVER_INIT(wcbowlj);
	DECLARE_DRIVER_INIT(aamat);
	DECLARE_DRIVER_INIT(bloodstm);
	DECLARE_DRIVER_INIT(aama);
	DECLARE_DRIVER_INIT(timekill);
	DECLARE_DRIVER_INIT(gt3d);
	DECLARE_DRIVER_INIT(gt3dl);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_MACHINE_RESET(drivedge);
	UINT32 screen_update_itech32(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(generate_int1);
	TIMER_CALLBACK_MEMBER(delayed_sound_data_w);
	TIMER_CALLBACK_MEMBER(scanline_interrupt);
	inline offs_t compute_safe_address(int x, int y);
	inline void disable_clipping();
	inline void enable_clipping();
	void logblit(const char *tag);
	void update_interrupts(int fast);
	void draw_raw(UINT16 *base, UINT16 color);
	void draw_raw_drivedge(UINT16 *base, UINT16 *zbase, UINT16 color);
	inline void draw_rle_fast(UINT16 *base, UINT16 color);
	inline void draw_rle_fast_xflip(UINT16 *base, UINT16 color);
	inline void draw_rle_slow(UINT16 *base, UINT16 color);
	void draw_rle(UINT16 *base, UINT16 color);
	void shiftreg_clear(UINT16 *base, UINT16 *zbase);
	void handle_video_command();
	inline int determine_irq_state(int vint, int xint, int qint);
	void itech32_update_interrupts(int vint, int xint, int qint);
	void init_program_rom();
	void init_sftm_common(int prot_addr);
	void init_shuffle_bowl_common(int prot_addr);
	void install_timekeeper();
	void init_gt_common();
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<cpu_device> m_dsp1;
	optional_device<cpu_device> m_dsp2;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
