// license:BSD-3-Clause
// copyright-holders:David Haywood, Olivier Galibert
#pragma once
#ifndef __K054338_H__
#define __K054338_H__

// 054338 "CLTC" is a final blender/modifier
//
// Mixes two images, each composed of:
// - one bitmap_ind16 with a 13-bit color code
// - one bitmap_ind16 with:
//   - bits 0-1: shadow code
//   - bits 2-3: brightness code
//   - bits 4-5: mixing code
//   - bit 15:   pixel present when 1
//
// The bitmaps are in a 4-entry array indexed with the BITMAP_* enum
//
// The init callback is called at startup, when the bitmap size
// changes for whatever reason, and when loading a state.  The
// (optional) callback can then initialize those of the bitmaps that
// never change, if any.
//
// The update callback is called when updating the frame, and has to
// update the bitmaps according to the cliprect.  The k054338 device
// itself will not change the bitmap contents from one frame to the
// next.
//
// The class proposes either bitmap_update to call the callbacks as
// needed and compute the new image, or screen_update with the normal
// screen updating interface.

class k054338_device : public device_t
{
public:
	enum {
		BITMAP_FRONT_COLOR,
		BITMAP_FRONT_ATTRIBUTES,
		BITMAP_LAYER2_COLOR,
		BITMAP_LAYER2_ATTRIBUTES
	};

	typedef device_delegate<void (bitmap_ind16 **)> init_cb;
	typedef device_delegate<void (bitmap_ind16 **, const rectangle &)> update_cb;

	k054338_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k054338_device();

	void set_init_cb(init_cb _cb) { m_init_cb = _cb; }
	void set_update_cb(update_cb _cb) { m_update_cb = _cb; }
	void set_palette_tag(const char *tag) { m_palette_tag = tag; }

	DECLARE_WRITE16_MEMBER(backr_w);
	DECLARE_WRITE16_MEMBER(backgb_w);
	DECLARE_WRITE16_MEMBER(shadow_w);
	DECLARE_WRITE16_MEMBER(shadow2_w); // Need to fix that in the memory system
	DECLARE_WRITE16_MEMBER(bri1_w);
	DECLARE_WRITE16_MEMBER(bri23_w);
	DECLARE_WRITE16_MEMBER(mix1_w);
	DECLARE_WRITE16_MEMBER(mix23_w);
	DECLARE_WRITE16_MEMBER(system_w);

	DECLARE_ADDRESS_MAP(map, 16);

	void bitmap_update(bitmap_rgb32 *bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	init_cb m_init_cb;
	update_cb m_update_cb;
	const char *m_palette_tag;
	palette_device *m_palette;

	bitmap_ind16 *m_bitmaps[4];

	uint8_t m_through_shadow_table[0x300];
	uint8_t m_clip_shadow_table[0x300];
	int32_t m_shadow[3][3];
	uint32_t m_back;
	uint8_t m_brightness[3], m_mix_level[3];
	uint8_t m_system;
	bool m_mix_add[3];
};

extern const device_type K054338;


#define MCFG_K054338_ADD(_tag, _palette_tag) \
	MCFG_DEVICE_ADD(_tag, K054338, 0)		 \
	downcast<k054338_device *>(device)->set_palette_tag(_palette_tag);

#define MCFG_K054338_MIXER(_tag) \
	k054338_device::set_mixer_tag(*device, _tag);

#define MCFG_K054338_ALPHAINV(_alphainv) \
	k054338_device::set_alpha_invert(*device, _alphainv);

#define MCFG_K054338_SET_INIT_CB(_tag, _class, _method)					\
	downcast<k054338_device *>(device)->set_init_cb(k054338_device::init_cb(&_class::_method, #_class "::" #_method, _tag, (_class *)nullptr));

#define MCFG_K054338_SET_UPDATE_CB(_tag, _class, _method)				\
	downcast<k054338_device *>(device)->set_update_cb(k054338_device::update_cb(&_class::_method, #_class "::" #_method, _tag, (_class *)nullptr));

#define MCFG_K054338_PALETTE(_palette_tag) \
	downcast<k054338_device *>(device)->set_palette_tag(_palette_tag);

#define MCFG_K054338_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#endif
