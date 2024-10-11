// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    video/apple2.h  - Video handling for Apple II and IIgs

*********************************************************************/

#ifndef MAME_SHARED_APPLE2VIDEO_H
#define MAME_SHARED_APPLE2VIDEO_H

#include "emupal.h"

#define BORDER_LEFT (32)
#define BORDER_RIGHT    (32)
#define BORDER_TOP  (16)    // (plus bottom)

class a2_video_device : public device_t, public device_palette_interface, public device_video_interface
{
public:
	// construction/destruction
	a2_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u32 m_GSborder_colors[16]{}, m_shr_palette[256]{};
	std::unique_ptr<bitmap_ind16> m_8bit_graphics;

	void txt_w(int state);
	void mix_w(int state);
	void scr_w(int state);
	void res_w(int state);
	void dhires_w(int state);
	void an2_w(int state);

	bool get_graphics() const   { return m_graphics; }
	bool get_hires() const      { return m_hires; }
	bool get_dhires() const     { return m_dhires; }
	bool get_page2() const      { return m_page2; }
	bool get_80col() const      { return m_80col; }
	bool get_80store() const    { return m_80store; }
	bool get_mix() const        { return m_mix; }
	bool get_altcharset() const { return m_altcharset; }
	bool get_monohgr() const    { return m_monohgr; }

	void a80col_w(bool b80Col);
	void a80store_w(bool b80Store)  { m_80store = b80Store; }
	void page2_w(bool page2)        { m_page2 = page2; }
	void altcharset_w(bool altch)   { m_altcharset = altch; }
	void monohgr_w(bool mono)       { m_monohgr = mono; }

	// IIgs-specific accessors
	void set_GS_monochrome(u8 mono) { m_monochrome = mono; }
	void set_GS_foreground(u8 fg)   { m_GSfg = fg; }
	void set_GS_background(u8 bg)   { m_GSbg = bg; }
	const u8 get_GS_border()        { return m_GSborder; }
	void set_GS_border(u8 border)   { m_GSborder = border; }
	const u8 get_newvideo()         { return m_newvideo; }
	void set_newvideo(u8 newvideo)  { m_newvideo = newvideo; }
	void set_SHR_color(u8 color, u32 rgb) { m_shr_palette[color] = rgb; }
	void set_GS_border_color(u8 color, u32 rgb) { m_GSborder_colors[color] = rgb; }

	// Models with different text-mode behavior. II includes the II+ and IIE includes the IIc and IIc Plus.
	enum class model { II, IIE, IIGS, II_J_PLUS, IVEL_ULTRA };

	void set_ram_pointers(u8 *main, u8 *aux)    { m_ram_ptr = main; m_aux_ptr = aux; }
	void set_aux_mask(u16 aux_mask)             { m_aux_mask = aux_mask; }
	void set_char_pointer(u8 *charptr, int size) { m_char_ptr = charptr; m_char_size = size; }
	void setup_GS_graphics() { m_8bit_graphics = std::make_unique<bitmap_ind16>(560, 192); }

	template <model Model, bool Invert, bool Flip>
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint32_t screen_update_GS(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void set_iie_langsw(u8 iie_langsw) { m_iie_langsw = iie_langsw; }
	u8 get_iie_langsw() const { return m_iie_langsw; }

protected:
	a2_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual uint32_t palette_entries() const noexcept override;
	void init_palette();

private:
	template <model Model, bool Invert, bool Flip>
	void text_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);

	template <model Model, bool Invert, bool Flip>
	unsigned get_text_character(uint32_t code, int row);

	template<bool Double>
	void lores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void hgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void dhgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);

	void render_line(uint16_t *out, uint16_t const *in, int startcol, int stopcol, bool monochrome, bool is_80_column);

	bool use_page_2() const;

	bool composite_monitor();
	bool monochrome_monitor();
	bool rgb_monitor();

	int composite_color_mode();
	bool composite_lores_artifacts();
	bool composite_text_color(bool is_80_column);
	bool monochrome_dhr_shift();
	int monochrome_hue();

	u8 *m_ram_ptr = nullptr, *m_aux_ptr = nullptr, *m_char_ptr = nullptr;
	u16 m_aux_mask = 0xffff;
	int m_char_size = 0;
	bool m_page2 = false;
	bool m_flash = false;
	bool m_mix = false;
	bool m_graphics = false;
	bool m_hires = false;
	bool m_dhires = false;
	bool m_80col = false;
	bool m_altcharset = false;
	bool m_an2 = false;
	bool m_80store = false;
	bool m_monohgr = false;
	u8 m_GSfg = 0, m_GSbg = 0, m_GSborder = 0, m_newvideo = 0, m_monochrome = 0, m_rgbmode = 0;
	u8 m_iie_langsw = 0; // language switch/modification on IIe/IIc/IIc+ and clones
	optional_ioport m_vidconfig;
};

// a2_video_device with composite monitor dip switch settings
class a2_video_device_composite : public a2_video_device
{
public:
	a2_video_device_composite(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

// a2_video_device with composite settings plus Video-7 RGB card option
class a2_video_device_composite_rgb : public a2_video_device
{
public:
	a2_video_device_composite_rgb(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(APPLE2_VIDEO, a2_video_device)
DECLARE_DEVICE_TYPE(APPLE2_VIDEO_COMPOSITE, a2_video_device_composite)
DECLARE_DEVICE_TYPE(APPLE2_VIDEO_COMPOSITE_RGB, a2_video_device_composite_rgb)

#endif // MAME_SHARED_APPLE2VIDEO_H
