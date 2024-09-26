// license:BSD-3-Clause
// copyright-holders:Dan Boris
#ifndef MAME_ATARI_MARIA_H
#define MAME_ATARI_MARIA_H

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

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:

	int m_maria_palette[32]{};
	int m_line_ram[2][160]{};
	int m_active_buffer = 0;
	int m_write_mode = 0;
	unsigned int m_dll = 0;
	unsigned int m_dl = 0;
	int m_holey = 0;
	int m_offset = 0;
	int m_vblank = 0;
	int m_dmaon = 0;
	int m_dpp = 0;
	int m_wsync = 0;
	int m_color_kill = 0;
	int m_cwidth = 0;
	int m_bcntl = 0;
	int m_kangaroo = 0;
	int m_rm = 0;
	int m_nmi = 0;
	unsigned int m_charbase = 0;
	bitmap_ind16 m_bitmap;

	void draw_scanline();
	int is_holey(unsigned int addr);
	int write_line_ram(int addr, uint8_t offset, int pal);

	required_device<cpu_device> m_cpu; // CPU whose space(AS_PROGRAM) serves as DMA source
	required_device<screen_device> m_screen;
};


// device type definition
DECLARE_DEVICE_TYPE(ATARI_MARIA, atari_maria_device)

#endif // MAME_ATARI_MARIA_H
