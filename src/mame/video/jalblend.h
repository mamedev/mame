// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia
class jaleco_blend_device : public device_t
{
public:
	jaleco_blend_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~jaleco_blend_device() {}

	void set(int color, UINT8 val);
	rgb_t func(rgb_t dest, rgb_t addMe, UINT8 alpha);
	void drawgfx(palette_device &palette,bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color);
	void drawgfx(palette_device &palette,bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	/* each palette entry contains a fourth 'alpha' value */
	std::unique_ptr<UINT8[]> m_table;

	template<class _BitmapClass>
	void drawgfx_common(palette_device &palette,_BitmapClass &dest_bmp,const rectangle &clip,gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color);
};

extern const device_type JALECO_BLEND;
