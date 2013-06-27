

class deco_karnovsprites_device : public device_t
{
public:
	deco_karnovsprites_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void set_gfxregion(int region) { m_gfxregion = region; };
	void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram, int size, int priority );
	static void set_gfx_region(device_t &device, int region);

protected:
	virtual void device_start();
	virtual void device_reset();

	UINT8 m_gfxregion;
private:

};

extern const device_type DECO_KARNOVSPRITES;
