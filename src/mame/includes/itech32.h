// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brian Troha
/*************************************************************************

    Incredible Technologies/Strata system
    (32-bit blitter variant)

**************************************************************************/
#ifndef MAME_INCLUDES_ITECH32_H
#define MAME_INCLUDES_ITECH32_H

#pragma once

#include "machine/6522via.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timekpr.h"
#include "sound/es5506.h"
#include "emupal.h"
#include "screen.h"

#define VIDEO_CLOCK     XTAL(8'000'000)           // video (pixel) clock
#define CPU_CLOCK       XTAL(12'000'000)          // clock for 68000-based systems
#define CPU020_CLOCK    XTAL(25'000'000)          // clock for 68EC020-based systems
#define SOUND_CLOCK     XTAL(16'000'000)          // clock for sound board
#define TMS_CLOCK       XTAL(40'000'000)          // TMS320C31 clocks on drivedge


class itech32_state : public driver_device
{
public:
	itech32_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_via(*this, "via6522_0"),
		m_ensoniq(*this, "ensoniq"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ticket(*this, "ticket"),
		m_timekeeper(*this, "m48t02"),
		m_main_ram(*this, "main_ram", 0),
		m_nvram(*this, "nvram", 0),
		m_video(*this, "video", 0),
		m_main_rom(*this, "main_rom", 0)
	{ }

	void base_devices(machine_config &config);
	void via(machine_config &config);
	void tourny(machine_config &config);
	void sftm(machine_config &config);
	void bloodstm(machine_config &config);
	void timekill(machine_config &config);

	void init_gtclasscp();
	void init_shufshot();
	void init_wcbowlt();
	void init_hardyard();
	void init_s_ver();
	void init_sftm110();
	void init_wcbowln();
	void init_gt2kp();
	void init_sftm();
	void init_wcbowl();
	void init_wcbowlj();
	void init_aamat();
	void init_bloodstm();
	void init_aama();
	void init_timekill();
	void init_gt3d();
	void init_gt3dl();

	DECLARE_CUSTOM_INPUT_MEMBER(special_port_r);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<via6522_device> m_via; // sftm, wcbowl and gt games don't have the via
	required_device<es5506_device> m_ensoniq;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ticket_dispenser_device> m_ticket;
	optional_device<timekeeper_device> m_timekeeper;

	optional_shared_ptr<uint16_t> m_main_ram;
	optional_shared_ptr<uint16_t> m_nvram;
	optional_shared_ptr<uint16_t> m_video;
	optional_shared_ptr<uint16_t> m_main_rom;

	virtual void nvram_init(nvram_device &nvram, void *base, size_t length);

	std::unique_ptr<uint16_t[]> m_videoram;
	uint8_t m_vint_state;
	uint8_t m_xint_state;
	uint8_t m_qint_state;
	uint8_t m_irq_base;
	uint8_t m_sound_data;
	uint8_t m_sound_return;
	uint8_t m_sound_int_state;
	offs_t m_itech020_prot_address;
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
	uint8_t m_planes;
	uint16_t m_vram_height;
	uint32_t m_vram_mask;
	uint32_t m_vram_xmask;
	uint32_t m_vram_ymask;
	uint8_t *m_grom_base;
	uint32_t m_grom_size;
	uint32_t m_grom_bank_mask;

	DECLARE_WRITE16_MEMBER(int1_ack_w);
	DECLARE_READ16_MEMBER(trackball_r);
	DECLARE_READ16_MEMBER(trackball_p2_r);
	DECLARE_READ32_MEMBER(trackball32_8bit_r);
	DECLARE_READ32_MEMBER(trackball32_4bit_p1_r);
	DECLARE_READ32_MEMBER(trackball32_4bit_p2_r);
	DECLARE_READ32_MEMBER(trackball32_4bit_combined_r);
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
	DECLARE_WRITE32_MEMBER(int1_ack32_w);
	DECLARE_WRITE16_MEMBER(timekill_colora_w);
	DECLARE_WRITE16_MEMBER(timekill_colorbc_w);
	DECLARE_WRITE16_MEMBER(timekill_intensity_w);
	DECLARE_WRITE16_MEMBER(bloodstm_color1_w);
	DECLARE_WRITE16_MEMBER(bloodstm_color2_w);
	DECLARE_WRITE16_MEMBER(bloodstm_plane_w);
	DECLARE_WRITE32_MEMBER(itech020_color1_w);
	DECLARE_WRITE32_MEMBER(itech020_color2_w);
	DECLARE_WRITE32_MEMBER(itech020_plane_w);
	DECLARE_WRITE16_MEMBER(bloodstm_paletteram_w);
	DECLARE_WRITE16_MEMBER(itech32_video_w);
	DECLARE_READ16_MEMBER(itech32_video_r);
	DECLARE_WRITE16_MEMBER(bloodstm_video_w);
	DECLARE_READ16_MEMBER(bloodstm_video_r);
	DECLARE_WRITE32_MEMBER(itech020_video_w);
	DECLARE_READ32_MEMBER(itech020_video_r);
	DECLARE_WRITE8_MEMBER(pia_portb_out);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void init_program_rom();
	void init_sftm_common(int prot_addr);
	void init_shuffle_bowl_common(int prot_addr);
	void install_timekeeper();
	void init_gt_common();

	uint32_t screen_update_itech32(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	WRITE_LINE_MEMBER(generate_int1);
	TIMER_CALLBACK_MEMBER(delayed_sound_data_w);
	TIMER_CALLBACK_MEMBER(scanline_interrupt);
	inline offs_t compute_safe_address(int x, int y);
	inline void disable_clipping();
	inline void enable_clipping();
	virtual void logblit(const char *tag);
	void update_interrupts(int fast);
	void draw_raw(uint16_t *base, uint16_t color);
	virtual void command_blit_raw();
	virtual void command_shift_reg();
	inline void draw_rle_fast(uint16_t *base, uint16_t color);
	inline void draw_rle_fast_xflip(uint16_t *base, uint16_t color);
	inline void draw_rle_slow(uint16_t *base, uint16_t color);
	void draw_rle(uint16_t *base, uint16_t color);
	virtual void shiftreg_clear(uint16_t *base, uint16_t *zbase);
	void handle_video_command();
	void update_interrupts(int vint, int xint, int qint);
	void bloodstm_map(address_map &map);
	void itech020_map(address_map &map);
	void sound_020_map(address_map &map);
	void sound_map(address_map &map);
	void timekill_map(address_map &map);
};

class drivedge_state : public itech32_state
{
public:
	drivedge_state(const machine_config &mconfig, device_type type, const char *tag) :
		itech32_state(mconfig, type, tag),
		m_dsp1(*this, "dsp1"),
		m_dsp2(*this, "dsp2"),
		m_zbuf_control(*this, "zctl"),
		m_tms1_boot(*this, "tms1_boot"),
		m_tms1_ram(*this, "tms1_ram"),
		m_tms2_ram(*this, "tms2_ram"),
		m_leds(*this, "led%u", 0U),
		m_steer(*this, "STEER"),
		m_gas(*this, "GAS")
	{ }

	void drivedge(machine_config &config);

	virtual void driver_init() override;

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ32_MEMBER(steering_r);
	DECLARE_READ32_MEMBER(gas_r);

	DECLARE_READ32_MEMBER(tms1_speedup_r);
	DECLARE_READ32_MEMBER(tms2_speedup_r);
	DECLARE_WRITE32_MEMBER(tms_reset_assert_w);
	DECLARE_WRITE32_MEMBER(tms_reset_clear_w);
	DECLARE_WRITE32_MEMBER(tms1_68k_ram_w);
	DECLARE_WRITE32_MEMBER(tms2_68k_ram_w);
	DECLARE_WRITE32_MEMBER(tms1_trigger_w);
	DECLARE_WRITE32_MEMBER(tms2_trigger_w);

	DECLARE_WRITE32_MEMBER(color0_w);
	DECLARE_WRITE32_MEMBER(zbuf_control_w);

	DECLARE_WRITE8_MEMBER(portb_out);
	DECLARE_WRITE_LINE_MEMBER(turbo_light);

	void main_map(address_map &map);
	void tms1_map(address_map &map);
	void tms2_map(address_map &map);
	virtual void nvram_init(nvram_device &nvram, void *base, size_t length) override;

	virtual void logblit(const char *tag) override;
	virtual void shiftreg_clear(uint16_t *base, uint16_t *zbase) override;
	virtual void command_blit_raw() override;
	virtual void command_shift_reg() override;
	void draw_raw(uint16_t *base, uint16_t *zbase, uint16_t color);

	required_device<cpu_device> m_dsp1;
	required_device<cpu_device> m_dsp2;
	required_shared_ptr<uint32_t> m_zbuf_control;
	required_shared_ptr<uint32_t> m_tms1_boot;
	required_shared_ptr<uint32_t> m_tms1_ram;
	required_shared_ptr<uint32_t> m_tms2_ram;
	output_finder<4> m_leds;
	required_ioport m_steer;
	required_ioport m_gas;

	uint8_t m_tms_spinning[2];
};

#endif // MAME_INCLUDES_ITECH32_H
