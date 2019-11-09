// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_VIDEO_ELAN_EU3A14VID_H
#define MAME_VIDEO_ELAN_EU3A14VID_H

#include "elan_eu3a05commonvid.h"
#include "cpu/m6502/m6502.h"
#include "machine/bankdev.h"
#include "screen.h"

class elan_eu3a14vid_device : public elan_eu3a05commonvid_device
{
public:
	elan_eu3a14vid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_addrbank(T &&tag) { m_bank.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	void set_tilerambase(int tilerambase) { m_tilerambase = tilerambase; }

	void video_start();
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(tilecfg_r) { return m_tilecfg[offset]; }
	DECLARE_WRITE8_MEMBER(tilecfg_w) { m_tilecfg[offset] = data; }
	DECLARE_READ8_MEMBER(ramtilecfg_r) { return m_ramtilecfg[offset]; }
	DECLARE_WRITE8_MEMBER(ramtilecfg_w) { m_ramtilecfg[offset] = data; }
	DECLARE_READ8_MEMBER(rowscrollcfg_r) { return m_rowscrollcfg[offset]; }
	DECLARE_WRITE8_MEMBER(rowscrollcfg_w) { m_rowscrollcfg[offset] = data; }
	DECLARE_READ8_MEMBER(scrollregs_r) { return m_scrollregs[offset]; }
	DECLARE_WRITE8_MEMBER(scrollregs_w) { m_scrollregs[offset] = data; }
	DECLARE_READ8_MEMBER(rowscrollregs_r) { return m_rowscrollregs[offset]; }
	DECLARE_WRITE8_MEMBER(rowscrollregs_w) { m_rowscrollregs[offset] = data; }

	DECLARE_READ8_MEMBER(spriteaddr_r) { return m_spriteaddr; }
	DECLARE_WRITE8_MEMBER(spriteaddr_w) { m_spriteaddr = data; }

	DECLARE_READ8_MEMBER(spritebase_r) { return m_spritebase[offset]; }
	DECLARE_WRITE8_MEMBER(spritebase_w) { m_spritebase[offset] = data; }

	DECLARE_READ8_MEMBER(rowscrollsplit_r) { return m_rowscrollsplit[offset]; }
	DECLARE_WRITE8_MEMBER(rowscrollsplit_w) { m_rowscrollsplit[offset] = data; }


	int m_tilerambase;
	int m_spriterambase;

	bitmap_ind8 m_prioritybitmap;


	void draw_background_ramlayer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect);
	int get_xscroll_for_screenypos(int line);
	void draw_background_tile(bitmap_ind16 &bitmap, const rectangle &cliprect, int bpp, int tileno, int palette, int priority, int flipx, int flipy, int xpos, int ypos, int transpen, int size, int base, int drawfromram);
	void draw_background_page(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int ramstart, int ramend, int xbase, int ybase, int size, int bpp, int base, int pagewidth,int pageheight, int bytespertile, int palettepri, int drawfromram);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprite_pix(const rectangle& cliprect, uint16_t* dst, uint8_t* pridst, int realx, int priority, uint8_t pix, uint8_t mask, uint8_t shift, int palette);
	void draw_sprite_line(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int offset, int line, int pal, int flipx, int pri, int xpos, int ypos, int bpp);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);


protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<m6502_device> m_cpu;
	required_device<address_map_bank_device> m_bank;
	required_device<screen_device> m_screen;

	uint8_t read_gfxdata(int offset, int x);

	uint8_t read_vram(int offset);

	uint8_t m_scrollregs[4];
	uint8_t m_tilecfg[6];
	uint8_t m_rowscrollregs[8];
	uint8_t m_rowscrollsplit[5];
	uint8_t m_rowscrollcfg[2];
	uint8_t m_ramtilecfg[6];
	uint8_t m_spriteaddr;
	uint8_t m_spritebase[2];

};

DECLARE_DEVICE_TYPE(ELAN_EU3A14_VID, elan_eu3a14vid_device)

#endif // MAME_VIDEO_ELAN_EU3A14VID_H
