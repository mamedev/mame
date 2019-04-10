// license:BSD-3-Clause
// copyright-holders:Dan Boris
#ifndef MAME_VIDEO_MARIA_H
#define MAME_VIDEO_MARIA_H

#pragma once


// ======================> atari_maria_device

class atari_maria_device :  public device_t
{
public:
	// construction/destruction
	atari_maria_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_dmacpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }

	void interrupt(int lines);
	void startdma(int lines);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	int m_maria_palette[32];
	int m_line_ram[2][160];
	int m_active_buffer;
	int m_write_mode;
	unsigned int m_dll;
	unsigned int m_dl;
	int m_holey;
	int m_offset;
	int m_vblank;
	int m_dmaon;
	int m_dpp;
	int m_wsync;
	int m_color_kill;
	int m_cwidth;
	int m_bcntl;
	int m_kangaroo;
	int m_rm;
	int m_nmi;
	unsigned int m_charbase;
	bitmap_ind16 m_bitmap;

	void draw_scanline();
	int is_holey(unsigned int addr);
	int write_line_ram(int addr, uint8_t offset, int pal);

	required_device<cpu_device> m_cpu; // CPU whose space(AS_PROGRAM) serves as DMA source
	required_device<screen_device> m_screen;
};


// device type definition
DECLARE_DEVICE_TYPE(ATARI_MARIA, atari_maria_device)

#endif // MAME_VIDEO_MARIA_H
