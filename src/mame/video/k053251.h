// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont, Olivier Galibert
#ifndef __K053251_H__
#define __K053251_H__

#define MCFG_K053251_ADD(_tag, _shadow_layer)		\
	MCFG_DEVICE_ADD(_tag, K053251, 0) \
	downcast<k053251_device *>(device)->set_shadow_layer(_shadow_layer);

#define MCFG_K053251_SET_INIT_CB(_tag, _class, _method)					\
	downcast<k053251_device *>(device)->set_init_cb(k053251_device::init_cb(&_class::_method, #_class "::" #_method, _tag, (_class *)nullptr));

#define MCFG_K053251_SET_UPDATE_CB(_tag, _class, _method)				\
	downcast<k053251_device *>(device)->set_update_cb(k053251_device::update_cb(&_class::_method, #_class "::" #_method, _tag, (_class *)nullptr));

	enum
	{
		K053251_CI0 = 0,
		K053251_CI1,
		K053251_CI2,
		K053251_CI3,
		K053251_CI4
	};

class k053251_device : public device_t
{
public:
	enum {
		LAYER0_COLOR,
		LAYER1_COLOR,
		LAYER2_COLOR,
		LAYER3_COLOR,
		LAYER4_COLOR,
		LAYER0_ATTR,
		LAYER1_ATTR,
		LAYER2_ATTR,
		BITMAP_COUNT
	};
		
	typedef device_delegate<void (bitmap_ind16 **)> init_cb;
	typedef device_delegate<void (bitmap_ind16 **, const rectangle &)> update_cb;

	k053251_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k053251_device() {}

	void set_shadow_layer(int layer);
	void set_init_cb(init_cb _cb) { m_init_cb = _cb; }
	void set_update_cb(update_cb _cb) { m_update_cb = _cb; }

	DECLARE_WRITE8_MEMBER(inpri_w);
	DECLARE_WRITE8_MEMBER(cblk_w);

	DECLARE_ADDRESS_MAP(map, 8);

	DECLARE_WRITE8_MEMBER(rega_w);
	DECLARE_WRITE8_MEMBER(regb_w);

	void bitmap_update(bitmap_ind16 **bitmaps, const rectangle &cliprect);


	/*
	Note: k053251_w() automatically does a ALL_TILEMAPS->mark_all_dirty()
	when some palette index changes. If ALL_TILEMAPS is too expensive, use
	k053251_set_tilemaps() to indicate which tilemap is associated with each index.
	*/

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE16_MEMBER( lsb_w );
	DECLARE_WRITE16_MEMBER( msb_w );
	int get_priority(int ci);
	int get_palette_index(int ci);
	int get_tmap_dirty(int tmap_num);
	void set_tmap_dirty(int tmap_num, int data);

	DECLARE_READ16_MEMBER( lsb_r );         // PCU1
	DECLARE_READ16_MEMBER( msb_r );         // PCU1

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	init_cb m_init_cb;
	update_cb m_update_cb;

	bitmap_ind16 *m_bitmaps[BITMAP_COUNT];

	int m_shadow_layer;

	uint8_t m_inpri[5];
	uint8_t m_cblk[2];

	// internal state
	int      m_dirty_tmap[5];

	uint8_t    m_ram[16];
	int      m_tilemaps_set;
	int      m_palette_index[5];

	void reset_indexes();
};

extern const device_type K053251;


#endif
