// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#pragma once
#ifndef __KVIDEODAC_H__
#define __KVIDEODAC_H__

// The init callback is called at startup, when the bitmap size
// changes for whatever reason, and when loading a state.  The
// (optional) callback can then initialize those of the bitmaps that
// never change, if any.
//
// The update callback is called when updating the frame, and has to
// update the bitmaps according to the cliprect.  The kvideodac device
// itself will not change the bitmap contents from one frame to the
// next.
//
// The class proposes either bitmap_update to call the callbacks as
// needed and compute the new image, or screen_update with the normal
// screen updating interface.

class kvideodac_device : public device_t
{
public:
	enum {
		BITMAP_COLOR,
		BITMAP_ATTRIBUTES,
	};

	typedef device_delegate<void (bitmap_ind16 **)> init_cb;
	typedef device_delegate<void (bitmap_ind16 **, const rectangle &)> update_cb;

	kvideodac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~kvideodac_device();

	void set_info(const char *tag, uint16_t shadow_mask, double shadow_level, uint16_t highlight_mask, double highlight_level);
	void set_init_cb(init_cb _cb) { m_init_cb = _cb; }
	void set_update_cb(update_cb _cb) { m_update_cb = _cb; }

	void set_force_shadow(bool force);
	void set_force_highlight(bool force);
	void set_shadow_level(double level);
	void set_highlight_level(double level);

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

	bitmap_ind16 *m_bitmaps[2];
	uint8_t m_shadow_table[256], m_highlight_table[256], m_shadow_highlight_table[256];
	uint16_t m_shadow_mask, m_highlight_mask;
	double m_shadow_level, m_highlight_level;
	bool m_force_shadow, m_force_highlight;

	static void generate_table(uint8_t *table, double level);
};

extern const device_type KVIDEODAC;


#define MCFG_KVIDEODAC_ADD(_tag, _palette_tag, _shadow_mask, _shadow_level, _highlight_mask, _highlight_level) \
	MCFG_DEVICE_ADD(_tag, KVIDEODAC, 0)		 \
		downcast<kvideodac_device *>(device)->set_info(_palette_tag, _shadow_mask, _shadow_level, _highlight_mask, _highlight_level);

#define MCFG_KVIDEODAC_SET_INIT_CB(_tag, _class, _method)					\
	downcast<kvideodac_device *>(device)->set_init_cb(kvideodac_device::init_cb(&_class::_method, #_class "::" #_method, _tag, (_class *)nullptr));

#define MCFG_KVIDEODAC_SET_UPDATE_CB(_tag, _class, _method)				\
	downcast<kvideodac_device *>(device)->set_update_cb(kvideodac_device::update_cb(&_class::_method, #_class "::" #_method, _tag, (_class *)nullptr));

#define MCFG_KVIDEODAC_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#endif
