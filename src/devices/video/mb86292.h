// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_VIDEO_MB86292_H
#define MAME_VIDEO_MB86292_H

#pragma once

#include "machine/ram.h"

class mb86292_device : public device_t,
					   public device_video_interface,
					   public device_memory_interface
{
public:
	mb86292_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_vram(T &&tag) { m_vram.set_tag(std::forward<T>(tag)); }
	auto set_xint_cb() { return m_xint_cb.bind(); }

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	virtual void vregs_map(address_map &map) ATTR_COLD;

protected:
	mb86292_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	virtual void draw_io_map(address_map &map) ATTR_COLD;

	address_space_config m_draw_io_space_config;
	required_device<screen_device> m_screen;
	required_device<ram_device> m_vram;
	devcb_write_line m_xint_cb;

	void reconfigure_screen();
	void process_display_opcode(u32 opcode);
private:
	void process_display_list();

	u16 m_dce = 0;
	struct {
		u32 lsa = 0, lco = 0;
		bool lreq = false;
		u32 cur_address = 0;
	} m_displaylist;

	struct {
		u16 htp = 0, hdp = 0, hdb = 0, hsp = 0;
		u16 vtr = 0, vsp = 0, vdp = 0;
		u8 hsw = 0, vsw = 0;
	} m_crtc;

	enum {
		IRQ_CERR = 1 << 0,
		IRQ_CEND = 1 << 1,
		IRQ_VSYNC = 1 << 2,
		IRQ_FSYNC = 1 << 3,
		IRQ_SYNCERR = 1 << 4,
		//bit 16 and 17 <reserved> (?)
	};

	struct {
		u32 ist = 0, mask = 0;
	} m_irq;

	struct {
		u16 xres = 0;
		u32 base = 0;
	} m_fb;

	enum drawing_state_t {
		DRAW_IDLE,
		DRAW_COMMAND,
		DRAW_DATA
	};

	struct {
		u16 fc = 0, bc = 0;
		util::fifo <u32, 32> fifo;
		drawing_state_t state;
		u32 current_command = 0;
		u8 command_count = 0;
		u32 data_count = 0;

		// DrawBitmapP needs these intermediary values
		u16 rx = 0, ry = 0, rxi = 0, ryi = 0, rsizex = 0, rsizey = 0;
	} m_draw;

	void reset_drawing_engine();
	u32 ctr_r(offs_t offset);

	// geometry engine
	u32 gctr_r(offs_t offset);

	struct {
		u32 cm = 0;
		u16 ch = 0;
		u16 cw = 0;
		bool cc = false;
		u32 cda = 0;
		u16 tc = 0;
		u16 transpen = 0;
	} m_c_layer;

	struct {
		u32 blda[2]{};
		u32 blm = 0;
		u16 blh = 0;
		u16 blw = 0;
		u8 blflp = 0;
		bool blc = false;
	} m_bl_layer;

	struct {
		u32 mlda[2]{};
	} m_ml_layer;

	template <unsigned N> u32 blda_r(offs_t offset);
	template <unsigned N> void blda_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <unsigned N> u32 mlda_r(offs_t offset);
	template <unsigned N> void mlda_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	emu_timer* m_vsync_timer;

	void check_irqs();
	TIMER_CALLBACK_MEMBER(vsync_cb);

	bitmap_rgb32     m_fb_bitmap;
	void fb_commit();

	// NOTE: the access must always be aligned
	u32 vram_read_word(offs_t offset) {
		return m_vram->read(offset) | (m_vram->read(offset + 1) << 8);
	}

	void vram_write_word(offs_t offset, u16 data) {
		m_vram->write(offset,     data & 0xff);
		m_vram->write(offset + 1, (data >> 8) & 0xff);
	}

	u32 vram_read_dword(offs_t offset) {
		return m_vram->read(offset) | (m_vram->read(offset + 1) << 8) | (m_vram->read(offset + 2) << 16) | (m_vram->read(offset + 3) << 24);
	}

	void vram_write_dword(offs_t offset, u32 data) {
		m_vram->write(offset,     data & 0xff);
		m_vram->write(offset + 1, (data >> 8) & 0xff);
		m_vram->write(offset + 2, (data >> 16) & 0xff);
		m_vram->write(offset + 3, (data >> 24) & 0xff);
	}
};

DECLARE_DEVICE_TYPE(MB86292, mb86292_device)


#endif // MAME_VIDEO_MB86292_H
