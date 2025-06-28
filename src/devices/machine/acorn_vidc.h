// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont, Juergen Buchmueller
/***************************************************************************

    Acorn VIDC10 (VIDeo Controller) device chip

***************************************************************************/

#ifndef MAME_MACHINE_ACORN_VIDC_H
#define MAME_MACHINE_ACORN_VIDC_H

#pragma once

#include "speaker.h"
#include "sound/dac.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> acorn_vidc10_device

class acorn_vidc10_device : public device_t,
							public device_memory_interface,
							public device_palette_interface,
							public device_video_interface
{
public:
	// I/O operations
	void write(offs_t offset, u32 data, u32 mem_mask = ~0);
	int flyback_r();
	auto vblank() { return m_vblank_cb.bind(); }
	auto sound_drq() { return m_sound_drq_cb.bind(); }
	// MEMC comms
	void write_vram(u32 offset, u8 data) { m_data_vram[offset & (m_data_vram_mask)] = data; }
	void write_cram(u32 offset, u8 data) { m_cursor_vram[offset & (m_cursor_vram_mask)] = data; }
	void write_dac(u8 channel, u8 data);
	void clear_dac(u8 channel) { m_dac[channel & 7]->write(0); }
	void update_sound_mode(bool state) { m_sound_mode = state; refresh_sound_frequency(); }
	void set_cursor_enable(bool state) { m_cursor_enable = state; }
	u32 get_cursor_size() { return (m_crtc_regs[CRTC_VCER] - m_crtc_regs[CRTC_VCSR]) * (32/4); }

	virtual bool get_dac_mode() { return false; }

protected:
	acorn_vidc10_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int dac_type);

	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual u32 palette_entries() const noexcept override;
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual u32 get_pixel_clock();

	TIMER_CALLBACK_MEMBER(vblank_timer);
	TIMER_CALLBACK_MEMBER(sound_drq_timer);

	address_space_config  m_space_config;

	void regs_map(address_map &map) ATTR_COLD;

	enum {
		CRTC_HCR = 0, CRTC_HSWR, CRTC_HBSR, CRTC_HDSR, CRTC_HDER, CRTC_HBER, CRTC_HCSR, CRTC_HIR,
		CRTC_VCR,     CRTC_VSWR, CRTC_VBSR, CRTC_VDSR, CRTC_VDER, CRTC_VBER, CRTC_VCSR, CRTC_VCER
	};
	u32 m_crtc_regs[16];

	inline void screen_vblank_line_update();
	inline void screen_dynamic_res_change();
	void refresh_sound_frequency();

	u16 m_pal_4bpp_base;
	u16 m_pal_cursor_base;
	u16 m_pal_border_base;

	u8 m_bpp_mode, m_crtc_interlace;
	u8       m_sound_frequency_latch;
	bool     m_sound_mode;

	required_device_array<dac_16bit_r2r_twos_complement_device, 8> m_dac;
	int m_dac_type;

	required_device<speaker_device> m_speaker;

	virtual void refresh_stereo_image(u8 channel);
	const int m_sound_max_channels = 8;
private:
	devcb_write_line m_vblank_cb;
	devcb_write_line m_sound_drq_cb;

	void pal_data_display_w(offs_t offset, u32 data);
	void pal_data_cursor_w(offs_t offset, u32 data);
	void stereo_image_w(offs_t offset, u32 data);
	void crtc_w(offs_t offset, u32 data);
	void sound_frequency_w(u32 data);
	void control_w(u32 data);

	u8 m_pixel_clock;
	bool m_cursor_enable;
	//bool m_flyback;
	emu_timer *m_video_timer;
	emu_timer *m_sound_timer;

	std::unique_ptr<u8[]> m_data_vram;
	std::unique_ptr<u8[]> m_cursor_vram;
	// TODO: correct data vram size
	const u32 m_data_vram_mask = 0x1fffff;
	const u32 m_cursor_vram_mask = 0x7fff;
	const u32 m_data_vram_size = m_data_vram_mask+1;
	const u32 m_cursor_vram_size = m_cursor_vram_mask+1;
	void draw(bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 *vram, u8 bpp, int xstart, int ystart, int xsize, int ysize, bool is_cursor);
	inline void update_4bpp_palette(u16 index, u32 paldata);
	u32 m_crtc_raw_horz[2];
	inline u32 convert_crtc_hdisplay(u8 index);

	bool m_sound_frequency_test_bit;
	u8       m_stereo_image[8];
	const float m_sound_input_gain = 0.05f;
	int16_t  m_ulaw_lookup[256];
};

class acorn_vidc1_device : public acorn_vidc10_device
{
public:
	// construction/destruction
	acorn_vidc1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class acorn_vidc1a_device : public acorn_vidc10_device
{
public:
	// construction/destruction
	acorn_vidc1a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type definition
DECLARE_DEVICE_TYPE(ACORN_VIDC1, acorn_vidc1_device)
DECLARE_DEVICE_TYPE(ACORN_VIDC1A, acorn_vidc1a_device)

class arm_vidc20_device : public acorn_vidc10_device
{
public:
	// construction/destruction
	arm_vidc20_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void write_dac32(u8 channel, u16 data);
	virtual bool get_dac_mode() override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void regs_map(address_map &map) ATTR_COLD;
	virtual u32 palette_entries() const noexcept override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_config_complete() override;
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual u32 get_pixel_clock() override;

private:
	void vidc20_pal_data_display_w(offs_t offset, u32 data);
	void vidc20_pal_data_index_w(u32 data);
	void vidc20_pal_data_cursor_w(offs_t offset, u32 data);
	void vidc20_crtc_w(offs_t offset, u32 data);
	void vidc20_control_w(u32 data);
	void vidc20_sound_frequency_w(u32 data);
	void vidc20_sound_control_w(u32 data);
	void fsynreg_w(u32 data);

	u8 m_pal_data_index;
	inline void update_8bpp_palette(u16 index, u32 paldata);
	bool m_dac_serial_mode;
	u8 m_pixel_source;
	u8 m_pixel_rate;
	u8 m_vco_r_modulo;
	u8 m_vco_v_modulo;

	required_device_array<dac_16bit_r2r_twos_complement_device, 2> m_dac32;

	virtual void refresh_stereo_image(u8 channel) override;
};

DECLARE_DEVICE_TYPE(ARM_VIDC20, arm_vidc20_device)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_ACORN_VIDC_H
