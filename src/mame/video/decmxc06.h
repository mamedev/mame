/* MXC06 */


class deco_mxc06_device : public device_t
{
public:
	deco_mxc06_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);
	static void set_gfx_region(device_t &device, int region);

	void set_gfxregion(int region) { m_gfxregion = region; };
	void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram16, int pri_mask, int pri_val, int col_mask );
	void draw_sprites_bootleg( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram, int pri_mask, int pri_val, int col_mask );
	void set_pri_type( int type ) { m_priority_type = type; }
protected:
	virtual void device_start();
	virtual void device_reset();

	UINT8 m_gfxregion;
	int m_priority_type; // just so we can support the existing drivers without converting everything to pdrawgfx just yet

private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

extern const device_type DECO_MXC06;

#define MCFG_DECO_MXC06_GFXDECODE(_gfxtag) \
	deco_mxc06_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_DECO_MXC06_PALETTE(_palette_tag) \
	deco_mxc06_device::static_set_palette_tag(*device, "^" _palette_tag);
