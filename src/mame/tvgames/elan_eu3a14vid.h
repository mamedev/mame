// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_ELAN_EU3A14VID_H
#define MAME_TVGAMES_ELAN_EU3A14VID_H

#include "elan_eu3a05commonvid.h"
#include "cpu/m6502/m6502.h"
#include "machine/bankdev.h"
#include "screen.h"

class elan_eu3a14vid_device : public elan_eu3a05commonvid_device, public device_memory_interface
{
public:
	elan_eu3a14vid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_addrbank(T &&tag) { m_bank.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	void map(address_map &map) ATTR_COLD;

	void set_tilerambase(int tilerambase) { m_tilerambase = tilerambase; }

	void video_start();
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);


protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	required_device<m6502_device> m_cpu;
	required_device<address_map_bank_device> m_bank;
	required_device<screen_device> m_screen;
	const address_space_config      m_space_config;

	inline uint8_t read_gfxdata(int offset, int x);
	inline uint8_t read_vram(int offset);
	inline uint8_t readpix(int baseaddr, int count, int drawfromram);

	uint8_t tilecfg_r(offs_t offset) { return m_tilecfg[offset]; }
	void tilecfg_w(offs_t offset, uint8_t data) { m_tilecfg[offset] = data; }
	uint8_t ramtilecfg_r(offs_t offset) { return m_ramtilecfg[offset]; }
	void ramtilecfg_w(offs_t offset, uint8_t data) { m_ramtilecfg[offset] = data; }
	uint8_t rowscrollcfg_r(offs_t offset) { return m_rowscrollcfg[offset]; }
	void rowscrollcfg_w(offs_t offset, uint8_t data) { m_rowscrollcfg[offset] = data; }
	uint8_t scrollregs_r(offs_t offset) { return m_scrollregs[offset]; }
	void scrollregs_w(offs_t offset, uint8_t data) { m_scrollregs[offset] = data; }
	uint8_t rowscrollregs_r(offs_t offset) { return m_rowscrollregs[offset]; }
	void rowscrollregs_w(offs_t offset, uint8_t data) { m_rowscrollregs[offset] = data; }
	uint8_t spritebase_r(offs_t offset) { return m_spritebase[offset]; }
	void spritebase_w(offs_t offset, uint8_t data) { m_spritebase[offset] = data; }
	uint8_t rowscrollsplit_r(offs_t offset) { return m_rowscrollsplit[offset]; }
	void rowscrollsplit_w(offs_t offset, uint8_t data) { m_rowscrollsplit[offset] = data; }

	uint8_t spriteaddr_r() { return m_spriteaddr; }
	void spriteaddr_w(uint8_t data) { m_spriteaddr = data; }
	uint8_t reg5107_r() { return m_5107; }
	void reg5107_w(uint8_t data) { m_5107 = data; }
	uint8_t reg5108_r() { return m_5108; }
	void reg5108_w(uint8_t data) { m_5108 = data; }
	uint8_t reg5109_r() { return m_5109; }
	void reg5109_w(uint8_t data) { m_5109 = data; }

	uint8_t read_unmapped(offs_t offset);
	void write_unmapped(offs_t offset, uint8_t data);

	void draw_background_ramlayer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect);
	int get_xscroll_for_screenypos(int line);
	void draw_background_tile(bitmap_ind16 &bitmap, const rectangle &cliprect, int bpp, int tileno, int palette, int priority, int flipx, int flipy, int xpos, int ypos, int transpen, int size, int base, int drawfromram);
	void draw_background_page(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int ramstart, int ramend, int xbase, int ybase, int size, int bpp, int base, int pagewidth,int pageheight, int bytespertile, int palettepri, int drawfromram);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprite_pix(const rectangle& cliprect, uint16_t* dst, uint8_t* pridst, int realx, int priority, uint8_t pix, uint8_t mask, uint8_t shift, int palette);
	void draw_sprite_line(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int offset, int line, int pal, int flipx, int pri, int xpos, int ypos, int bpp);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t m_scrollregs[4];
	uint8_t m_tilecfg[6];
	uint8_t m_rowscrollregs[8];
	uint8_t m_rowscrollsplit[5];
	uint8_t m_rowscrollcfg[2];
	uint8_t m_ramtilecfg[6];
	uint8_t m_spriteaddr = 0;
	uint8_t m_spritebase[2];

	uint8_t m_5107 = 0;
	uint8_t m_5108 = 0;
	uint8_t m_5109 = 0;

	int m_tilerambase = 0;
	int m_spriterambase = 0;

	bitmap_ind8 m_prioritybitmap;
};

DECLARE_DEVICE_TYPE(ELAN_EU3A14_VID, elan_eu3a14vid_device)

#endif // MAME_TVGAMES_ELAN_EU3A14VID_H
