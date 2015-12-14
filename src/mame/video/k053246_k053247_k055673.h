// license:BSD-3-Clause
// copyright-holders:David Haywood, Olivier Galibert
/* */

#ifndef MAME_INCLUDES_K053246_K053247_K055673_H
#define MAME_INCLUDES_K053246_K053247_K055673_H

#pragma once

#include "screen.h"

#define MCFG_K053246_055673_ADD(_tag, _dotclock, _palette_tag, _spriteram_tag) \
	MCFG_DEVICE_ADD(_tag, K053246_055673, _dotclock)					       \
	downcast<k053246_055673_device *>(device)->set_info(_palette_tag, _spriteram_tag);

#define MCFG_K053246_053247_ADD(_tag, _dotclock, _palette_tag, _spriteram_tag) \
	MCFG_DEVICE_ADD(_tag, K053246_053247, _dotclock)                           \
	downcast<k053246_053247_device *>(device)->set_info(_palette_tag, _spriteram_tag);

#define MCFG_K053246_055673_DMAACT_CB(_cb) \
	devcb = &k053246_055673_device::set_dmaact_cb(*device, DEVCB_##_cb);

#define MCFG_K053246_053247_DMAACT_CB(_cb) \
	devcb = &k053246_053247_device::set_dmaact_cb(*device, DEVCB_##_cb);

#define MCFG_K053246_055673_DMAIRQ_CB(_cb) \
	devcb = &k053246_055673_device::set_dmairq_cb(*device, DEVCB_##_cb);

#define MCFG_K053246_053247_DMAIRQ_CB(_cb) \
	devcb = &k053246_053247_device::set_dmairq_cb(*device, DEVCB_##_cb);

#define MCFG_K053246_055673_WIRING_CB(_class, _method)	\
	downcast<k053246_055673_device *>(device)->set_wiring_cb(k053246_055673_device::wiring_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_K053246_053247_WIRING_CB(_class, _method)	\
	downcast<k053246_053247_device *>(device)->set_wiring_cb(k053246_055673_device::wiring_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

class k053246_055673_device : public device_t,
							  public device_gfx_interface
{
public:
	typedef device_delegate<void (uint32_t output, uint16_t &color, uint16_t &attr)> wiring_delegate;

	k053246_055673_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	k053246_055673_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	~k053246_055673_device() { }

	void set_info(const char *palette_tag, const char *spriteram_tag);
	void set_wiring_cb(wiring_delegate cb);

	template<class _cb> static devcb_base &set_dmaact_cb(device_t &device, _cb cb) { return downcast<k053246_055673_device &>(device).m_dmaact_cb.set_callback(cb); }
	template<class _cb> static devcb_base &set_dmairq_cb(device_t &device, _cb cb) { return downcast<k053246_055673_device &>(device).m_dmairq_cb.set_callback(cb); }

	void set_objdma_offset(uint32_t offset);
	void set_objcrbk(bool active);
	void set_objcha(bool active);

	DECLARE_WRITE_LINE_MEMBER(vblank_w);

	DECLARE_ADDRESS_MAP(objset1, 16);
	DECLARE_ADDRESS_MAP(objset2, 16);

	DECLARE_ADDRESS_MAP(objset1_8, 8);

	DECLARE_WRITE16_MEMBER(hscr_w);
	DECLARE_WRITE16_MEMBER(vscr_w);
	DECLARE_WRITE16_MEMBER(oms_w);
	DECLARE_WRITE16_MEMBER(ocha_w);

	DECLARE_WRITE16_MEMBER(atrbk_w);
	DECLARE_WRITE16_MEMBER(vrcbk_w);
	DECLARE_WRITE16_MEMBER(opset_w);

	virtual DECLARE_READ8_MEMBER (rom8_r);
	virtual DECLARE_READ16_MEMBER(rom16_r);
	virtual DECLARE_READ32_MEMBER(rom32_r);

	void bitmap_update(bitmap_ind16 *bcolor, bitmap_ind16 *battr, const rectangle &cliprect);

protected:
	// device-level overrides
	void device_start() override;
	void device_reset() override;
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void device_post_load() override;

	enum {
		OD_IDLE,
		OD_WAIT_START,
		OD_WAIT_END
	};

	devcb_write_line m_dmairq_cb;
	devcb_write_line m_dmaact_cb;
	wiring_delegate m_wiring_cb;
	required_memory_region m_region;
	const char *m_spriteram_tag;
	memory_share *m_spriteram;
	emu_timer *m_timer_objdma;
	int m_timer_objdma_state;
	uint16_t m_sram[0x800];

	uint32_t m_ocha;
	uint16_t m_hscr, m_vscr, m_atrbk[4];
	uint8_t m_vrcbk[4], m_oms, m_coreg, m_opset;

	bool m_is_053247;

	bool m_dmairq_on;

	struct tile_layout {
		// Top/left and bottom/right screen position of the tile
		int sc_min, sc_max;
		// Top/left position in the tile, 3.6 fixed-point format
		int32_t tile_min;
		// Step on the tile position, 3.6 fixed-point format
		int32_t step;
		// Delta on tile id
		int tileid_delta;
	};

	void generate_tile_layout(tile_layout *tl, int &tl_count, uint32_t minp, uint32_t maxp, int screen_center, int offset, int width_order, int zoom, const int *tile_id_steps, bool flip, bool mirror, bool gflip);

	template<uint16_t mask> static void draw_tile_keep_shadow(bitmap_ind16 *bcolor, bitmap_ind16 *battr,
															gfx_element *g, uint32_t tile_base_id,
															const tile_layout &lx, const tile_layout &ly,
															uint16_t attr, uint16_t palette, uint8_t trans_mask);
	static void draw_tile_force_shadow(bitmap_ind16 *bcolor, bitmap_ind16 *battr,
									   gfx_element *g, uint32_t tile_base_id,
									   const tile_layout &lx, const tile_layout &ly,
									   uint16_t attr, uint16_t palette, uint8_t trans_mask);
	template<uint16_t mask> static void draw_tile_keep_detect_shadow(bitmap_ind16 *bcolor, bitmap_ind16 *battr,
																   gfx_element *g, uint32_t tile_base_id,
																   const tile_layout &lx, const tile_layout &ly,
																   uint16_t noshadow_attr, uint16_t shadow_attr,
																   uint16_t palette, uint8_t trans_mask, uint8_t shadow_color);
	static void draw_tile_force_detect_shadow(bitmap_ind16 *bcolor, bitmap_ind16 *battr,
											  gfx_element *g, uint32_t tile_base_id,
											  const tile_layout &lx, const tile_layout &ly,
											  uint16_t noshadow_attr, uint16_t shadow_attr,
											  uint16_t palette, uint8_t trans_mask, uint8_t shadow_color);

	void decode_sprite_roms();
};


class k053246_053247_device : public k053246_055673_device
{
public:
	k053246_053247_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k053246_053247_device() { }

	DECLARE_READ8_MEMBER (rom8_r)  override;
	DECLARE_READ16_MEMBER(rom16_r) override;
	DECLARE_READ32_MEMBER(rom32_r) override;
};

extern const device_type K053246_053247;
extern const device_type K053246_055673;

#endif // MAME_INCLUDES_K053246_K053247_K055673_H
