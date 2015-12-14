// license:BSD-3-Clause
// copyright-holders:David Haywood, Olivier Galibert
/* */

#pragma once
#ifndef __K055555_H__
#define __K055555_H__

#define MCFG_K055555_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K055555, 0)

#define MCFG_K055555_SET_INIT_CB(_tag, _class, _method)					\
	downcast<k055555_device *>(device)->set_init_cb(k055555_device::init_cb(&_class::_method, #_class "::" #_method, _tag, (_class *)nullptr));

#define MCFG_K055555_SET_UPDATE_CB(_tag, _class, _method)				\
	downcast<k055555_device *>(device)->set_update_cb(k055555_device::update_cb(&_class::_method, #_class "::" #_method, _tag, (_class *)nullptr));

class k055555_device : public device_t
{
public:
	enum {
		LAYERA_COLOR,
		LAYERB_COLOR,
		LAYERC_COLOR,
		LAYERD_COLOR,
		LAYERO_COLOR,
		LAYERS1_COLOR,
		LAYERS2_COLOR,
		LAYERS3_COLOR,
		LAYERO_ATTR,
		LAYERS1_ATTR,
		LAYERS2_ATTR,
		LAYERS3_ATTR,
		BITMAP_COUNT
	};

	typedef device_delegate<void (bitmap_ind16 **)> init_cb;
	typedef device_delegate<void (bitmap_ind16 **, const rectangle &)> update_cb;

	k055555_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k055555_device();

	void set_init_cb(init_cb _cb) { m_init_cb = _cb; }
	void set_update_cb(update_cb _cb) { m_update_cb = _cb; }

	DECLARE_WRITE8_MEMBER(bgc_cblk_w);
	DECLARE_WRITE8_MEMBER(bgc_set_w);
	DECLARE_WRITE8_MEMBER(colset0_w);
	DECLARE_WRITE8_MEMBER(colset1_w);
	DECLARE_WRITE8_MEMBER(colset2_w);
	DECLARE_WRITE8_MEMBER(colset3_w);
	DECLARE_WRITE8_MEMBER(colchg_on_w);
	DECLARE_WRITE8_MEMBER(a_pri0_w);
	DECLARE_WRITE8_MEMBER(a_pri1_w);
	DECLARE_WRITE8_MEMBER(a_colpri_w);
	DECLARE_WRITE8_MEMBER(b_pri0_w);
	DECLARE_WRITE8_MEMBER(b_pri1_w);
	DECLARE_WRITE8_MEMBER(b_colpri_w);
	DECLARE_WRITE8_MEMBER(c_pri_w);
	DECLARE_WRITE8_MEMBER(d_pri_w);
	DECLARE_WRITE8_MEMBER(o_pri_w);
	DECLARE_WRITE8_MEMBER(s1_pri_w);
	DECLARE_WRITE8_MEMBER(s2_pri_w);
	DECLARE_WRITE8_MEMBER(s3_pri_w);
	DECLARE_WRITE8_MEMBER(o_inpri_on_w);
	DECLARE_WRITE8_MEMBER(s1_inpri_on_w);
	DECLARE_WRITE8_MEMBER(s2_inpri_on_w);
	DECLARE_WRITE8_MEMBER(s3_inpri_on_w);
	DECLARE_WRITE8_MEMBER(a_cblk_w);
	DECLARE_WRITE8_MEMBER(b_cblk_w);
	DECLARE_WRITE8_MEMBER(c_cblk_w);
	DECLARE_WRITE8_MEMBER(d_cblk_w);
	DECLARE_WRITE8_MEMBER(o_cblk_w);
	DECLARE_WRITE8_MEMBER(s1_cblk_w);
	DECLARE_WRITE8_MEMBER(s2_cblk_w);
	DECLARE_WRITE8_MEMBER(s3_cblk_w);
	DECLARE_WRITE8_MEMBER(s2_cblk_on_w);
	DECLARE_WRITE8_MEMBER(s3_cblk_on_w);
	DECLARE_WRITE8_MEMBER(v_inmix_w);
	DECLARE_WRITE8_MEMBER(v_inmix_on_w);
	DECLARE_WRITE8_MEMBER(os_inmix_w);
	DECLARE_WRITE8_MEMBER(os_inmix_on_w);
	DECLARE_WRITE8_MEMBER(shd_pri1_w);
	DECLARE_WRITE8_MEMBER(shd_pri2_w);
	DECLARE_WRITE8_MEMBER(shd_pri3_w);
	DECLARE_WRITE8_MEMBER(shd_on_w);
	DECLARE_WRITE8_MEMBER(shd_pri_sel_w);
	DECLARE_WRITE8_MEMBER(v_inbri_w);
	DECLARE_WRITE8_MEMBER(os_inbri_w);
	DECLARE_WRITE8_MEMBER(os_inbri_on_w);
	DECLARE_WRITE8_MEMBER(disp_w);

	DECLARE_ADDRESS_MAP(map, 8);

	void bitmap_update(bitmap_ind16 **bitmaps, const rectangle &cliprect);

protected:
	// device-level overrides
	void device_start() override;
	void device_reset() override;
	void device_post_load() override;

private:
	init_cb m_init_cb;
	update_cb m_update_cb;

	bitmap_ind16 *m_bitmaps[BITMAP_COUNT];

	uint8_t m_shadow_value[4][256];

	uint16_t m_color_mask[8];
	uint8_t m_colset[4];
	uint8_t m_cblk[8];
	uint8_t m_cblk_on[2];
	uint8_t m_pri[8+2];
	uint8_t m_inpri_on[4];
	uint8_t m_colpri[2];
	uint8_t m_shd_pri[3];
	uint8_t m_shd_on, m_shd_pri_sel;
	uint8_t m_bgc_cblk, m_bgc_set, m_colchg_on;
	uint8_t m_v_inmix, m_v_inmix_on, m_os_inmix, m_os_inmix_on;
	uint8_t m_v_inbri, m_os_inbri, m_os_inbri_on;
	uint8_t m_disp;

	void update_shadow_value_array(int entry);
	void compute_color_mask(int i);
};

extern const device_type K055555;

#endif
