// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
#ifndef MAME_VIDEO_SETA001_H
#define MAME_VIDEO_SETA001_H

#pragma once

typedef device_delegate<int (uint16_t code, uint8_t color)> gfxbank_cb_delegate;

#define SETA001_SPRITE_GFXBANK_CB_MEMBER(_name) int _name(uint16_t code, uint8_t color)

class seta001_device : public device_t
{
public:
	seta001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	template <typename... T> void set_gfxbank_callback(T &&... args) { m_gfxbank_cb.set(std::forward<T>(args)...); }

	DECLARE_WRITE8_MEMBER(spritebgflag_w8);

	DECLARE_READ16_MEMBER(spritectrl_r16);
	DECLARE_WRITE16_MEMBER(spritectrl_w16);
	DECLARE_READ8_MEMBER(spritectrl_r8);
	DECLARE_WRITE8_MEMBER(spritectrl_w8);

	DECLARE_READ16_MEMBER(spriteylow_r16);
	DECLARE_WRITE16_MEMBER(spriteylow_w16);
	DECLARE_READ8_MEMBER(spriteylow_r8);
	DECLARE_WRITE8_MEMBER(spriteylow_w8);

	DECLARE_READ8_MEMBER(spritecodelow_r8);
	DECLARE_WRITE8_MEMBER(spritecodelow_w8);
	DECLARE_READ8_MEMBER(spritecodehigh_r8);
	DECLARE_WRITE8_MEMBER(spritecodehigh_w8);
	DECLARE_READ16_MEMBER(spritecode_r16);
	DECLARE_WRITE16_MEMBER(spritecode_w16);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size);

	void setac_eof();
	void tnzs_eof();

	// position kludges for seta.c & srmp2.c
	void set_fg_xoffsets(int flip, int noflip) { m_fg_flipxoffs = flip; m_fg_noflipxoffs = noflip; };
	void set_fg_yoffsets(int flip, int noflip) { m_fg_flipyoffs = flip; m_fg_noflipyoffs = noflip; };
	void set_bg_yoffsets(int flip, int noflip) { m_bg_flipyoffs = flip; m_bg_noflipyoffs = noflip; };
	void set_bg_xoffsets(int flip, int noflip) { m_bg_flipxoffs = flip; m_bg_noflipxoffs = noflip; };

	void set_colorbase(int base) { m_colorbase = base; };
	void set_spritelimit(int limit) { m_spritelimit = limit; };
	void set_transpen(int pen) { m_transpen = pen; };

	int is_flipped() { return ((m_spritectrl[ 0 ] & 0x40) >> 6); };

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size);
	void draw_foreground(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size);
	required_device<gfxdecode_device> m_gfxdecode;

	gfxbank_cb_delegate m_gfxbank_cb;

	// configuration
	int m_fg_flipxoffs, m_fg_noflipxoffs;
	int m_fg_flipyoffs, m_fg_noflipyoffs;
	int m_bg_flipyoffs, m_bg_noflipyoffs;
	int m_bg_flipxoffs, m_bg_noflipxoffs;
	int m_colorbase;
	int m_spritelimit;
	int m_transpen;

	// live state
	uint8_t m_bgflag;
	uint8_t m_spritectrl[4];
	std::unique_ptr<uint8_t[]> m_spriteylow;
	std::unique_ptr<uint8_t[]> m_spritecodelow; // tnzs.cpp stuff only uses half?
	std::unique_ptr<uint8_t[]> m_spritecodehigh; // ^
};

DECLARE_DEVICE_TYPE(SETA001_SPRITE, seta001_device)

#endif // MAME_VIDEO_SETA001_H
