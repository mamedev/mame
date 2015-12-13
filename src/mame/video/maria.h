// license:BSD-3-Clause
// copyright-holders:Dan Boris
#ifndef __ATARI_MARIA__
#define __ATARI_MARIA__

#include "emu.h"

// ======================> atari_maria_device

class atari_maria_device :  public device_t
{
public:
	// construction/destruction
	atari_maria_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_cpu_tag(device_t &device, const char *tag) { downcast<atari_maria_device &>(device).m_cpu_tag = tag; }

	void interrupt(int lines);
	void startdma(int lines);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

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
	int write_line_ram(int addr, UINT8 offset, int pal);

	const char *m_cpu_tag;
	cpu_device *m_cpu;  // CPU whose space(AS_PROGRAM) serves as DMA source
	screen_device *m_screen;
};


// device type definition
extern const device_type ATARI_MARIA;


#define MCFG_MARIA_DMACPU(_tag) \
	atari_maria_device::set_cpu_tag(*device, _tag);


#endif
