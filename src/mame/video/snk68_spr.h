// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Acho A. Tang, Nicola Salmoria
typedef device_delegate<void (int&, int&, int&, int&)> snk68_tile_indirection_delegate;

#define MCFG_SNK68_SPR_GFXDECODE(_gfxtag) \
	snk68_spr_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);
#define MCFG_SNK68_SPR_SET_TILE_INDIRECT( _class, _method) \
	snk68_spr_device::set_tile_indirect_cb(*device, snk68_tile_indirection_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));
#define MCFG_SNK68_SPR_NO_PARTIAL \
	snk68_spr_device::static_set_no_partial(*device);


class snk68_spr_device : public device_t
{
public:
	snk68_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void set_tile_indirect_cb(device_t &device,snk68_tile_indirection_delegate newtilecb);
	static void static_set_no_partial(device_t &device);

	DECLARE_READ16_MEMBER(spriteram_r);
	DECLARE_WRITE16_MEMBER(spriteram_w);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int group);
	void draw_sprites_all(bitmap_ind16 &bitmap, const rectangle &cliprect);

	snk68_tile_indirection_delegate m_newtilecb;

	void tile_callback_noindirect(int& tile, int& fx, int& fy, int& region);
	void set_flip(int flip);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<UINT16> m_spriteram;
	required_device<screen_device> m_screen;
	int m_flipscreen;
	int m_partialupdates; // the original hardware needs this, the cloned hardware does not.
};


extern const device_type SNK68_SPR;
